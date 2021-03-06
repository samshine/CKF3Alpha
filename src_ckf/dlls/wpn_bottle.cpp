#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#define	KNIFE_BODYHIT_VOLUME 128
#define	KNIFE_WALLHIT_VOLUME 512

LINK_ENTITY_TO_CLASS(weapon_bottle, CBottle);

enum bottle_e
{
	BOTTLE_IDLE,
	BOTTLE_SLASH1_BREAK,
	BOTTLE_SLASH2_BREAK,
	BOTTLE_DRAW,
	BOTTLE_SLASH1_BROKEN,
	BOTTLE_SLASH2_BROKEN,
	BOTTLE_SLASH1,
	BOTTLE_SLASH2,
	BOTTLE_IDLE_BROKEN,
	BOTTLE_DRAW_BROKEN
};

void CBottle::Spawn(void)
{
	Precache();
	m_iId = WEAPON_BOTTLE;
	SET_MODEL(ENT(pev), "models/CKF_III/wp_group_rf.mdl");
	pev->sequence = 6;
	m_iClip = WEAPON_NOCLIP;
	m_bBroken = FALSE;
	FallInit();
}

void CBottle::Reset(void)
{
	if(m_bBroken && m_pPlayer->m_pActiveItem == this)
	{
		m_pPlayer->pev->scale = 18;
		PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, m_iId, 1, 0, 0);
	}
	m_bBroken = FALSE;
}

void CBottle::Precache(void)
{
	PRECACHE_MODEL("models/CKF_III/v_bottle.mdl");
	PRECACHE_MODEL("models/CKF_III/wp_group_rf.mdl");
	PRECACHE_SOUND("CKF_III/bottle_hit.wav");
	PRECACHE_SOUND("CKF_III/bottle_hitbod.wav");
	PRECACHE_SOUND("CKF_III/bottle_break.wav");

	m_usFireScript = PRECACHE_EVENT(1, "events/ckf_melee.sc");
}

int CBottle::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 2;
	p->iPosition = 1;
	p->iId = WEAPON_BOTTLE;
	p->iFlags = 0;
	p->iWeight = BOTTLE_WEIGHT;

	return 1;
}

BOOL CBottle::Deploy(void)
{
	m_iShotsFired = 0;

	return GroupDeploy("models/CKF_III/v_bottle.mdl", "models/CKF_III/wp_group_rf.mdl", (m_bBroken) ? BOTTLE_DRAW_BROKEN : BOTTLE_DRAW, 0, 0, "knife", (m_bBroken) ? 21 : 18);
}

void CBottle::Holster(int skiplocal)
{
	m_bMeleeAttack = FALSE;
}

void CBottle::PrimaryAttack(void)
{
	m_flMeleeAttack = UTIL_WeaponTimeBase() + 0.3;
	m_iMeleeCrit = m_pPlayer->GetCriticalFire(2, m_pPlayer->random_seed);
	m_bMeleeAttack = TRUE;

	int iAnimBase = (m_bBroken) ? BOTTLE_SLASH1_BROKEN : BOTTLE_SLASH1;
	SendWeaponAnim(iAnimBase + (m_iShotsFired++) % 2, UseDecrement() != FALSE);

	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.8;
}

void CBottle::SecondaryAttack(void)
{
	CBasePlayerItem *pWeapon = m_pPlayer->m_rgpPlayerItems[WEAPON_SLOT_SECONDARY];
	if(pWeapon && pWeapon->m_iId == WEAPON_STICKYLAUNCHER)
	{
		CStickyLauncher *pLauncher = (CStickyLauncher *)pWeapon;
		if( pLauncher->m_flNextSecondaryAttack < UTIL_WeaponTimeBase())
			pLauncher->SecondaryAttack();
	}
}

void CBottle::Swing(void)
{
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 48;

	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

	PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, m_iMeleeCrit, 0, m_iId, 0, (m_iMeleeCrit >= 2) ? TRUE : FALSE, 0);

	if (tr.flFraction >= 1)
	{
		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(m_pPlayer->pev), &tr);

		if (tr.flFraction < 1)
		{
			CBaseEntity *pHit = CBaseEntity::Instance(tr.pHit);

			if (!pHit || pHit->IsBSPModel())
				FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, ENT(m_pPlayer->pev));

			vecEnd = tr.vecEndPos;
		}
	}

	if (tr.flFraction < 1)
	{
		BOOL bOldBroken = m_bBroken;

		if(m_iMeleeCrit >= 2)
			m_bBroken = TRUE;

		float flVol = 1;
		int fHitWorld = TRUE;

		CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);

		if (pEntity)
		{		
			m_iMeleeCrit += pEntity->GetCriticalHit();

			float fDmg = 65;

			if(m_iMeleeCrit >= 2) fDmg *= 3;
			else if(m_iMeleeCrit) fDmg *= 1.35f;
			else fDmg += RANDOM_LONG(-6, 7);

			ClearMultiDamage();

			pEntity->TraceAttack(m_pPlayer->pev, fDmg, gpGlobals->v_forward, &tr, DMG_NEVERGIB | DMG_SLASH, m_iMeleeCrit);

			ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

			if (pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MOVEABLE)
			{
				EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_STATIC, "CKF_III/bottle_hitbod.wav", VOL_NORM, ATTN_NORM);

				m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;

				if (!pEntity->IsAlive())
					return;

				flVol = 0.1;
				fHitWorld = FALSE;
			}
		}

		if (fHitWorld)
		{
			TEXTURETYPE_PlaySound(&tr, vecSrc, vecSrc + (vecEnd - vecSrc) * 2, BULLET_PLAYER_CROWBAR);
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_STATIC, "CKF_III/bottle_hit.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
		}

		if(m_bBroken && !bOldBroken)
		{
			MESSAGE_BEGIN(MSG_PVS, gmsgDrawFX, m_pPlayer->GetGunPosition());
			WRITE_BYTE(FX_BOTTLEBREAK);
			WRITE_SHORT(m_pPlayer->entindex());
			MESSAGE_END();
			
			m_pPlayer->pev->scale = 21;
		}

		m_pPlayer->m_iWeaponVolume = flVol * KNIFE_WALLHIT_VOLUME;
		DecalGunshot(&tr, BULLET_PLAYER_CROWBAR, FALSE, m_pPlayer->pev, FALSE);
	}
}