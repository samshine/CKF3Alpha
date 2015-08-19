#include "hud.h"
#include "tfteammenu.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <FileSystem.h>

#include <cl_entity.h>
#include <cdll_dll.h>

#include <vgui_controls/Button.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/ImagePanel.h>

#include "CounterStrikeViewport.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
TFTeamButton::TFTeamButton( vgui::Panel *parent, const char *panelName ) : Button( parent, panelName, "" )
{
	m_szModelPanel[0] = '\0';
	m_iTeam = TEAM_UNASSIGNED;
	m_flHoverTimeToWait = -1;
	m_flHoverTime = -1;
	m_bMouseEntered = false;
	m_bTeamDisabled = false;

	vgui::ivgui()->AddTickSignal( GetVPanel() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TFTeamButton::ApplySettings( KeyValues *inResourceData )
{
	BaseClass::ApplySettings( inResourceData );

	Q_strncpy( m_szModelPanel, inResourceData->GetString( "associated_model", "" ), sizeof( m_szModelPanel ) );
	m_iTeam = inResourceData->GetInt( "team", TEAM_UNASSIGNED );
	m_flHoverTimeToWait = inResourceData->GetFloat( "hover", -1 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TFTeamButton::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetDefaultColor( GetFgColor(), Color( 0, 0, 0, 0 ) );
	SetArmedColor( GetButtonFgColor(), Color( 0, 0, 0, 0 ) );
	SetDepressedColor( GetButtonFgColor(), Color( 0, 0, 0, 0 ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TFTeamButton::SendAnimation( const char * pszAnimation, bool reverse )
{
	Panel *pParent = GetParent();
	if ( pParent )
	{
		ModelPanel *pModel = dynamic_cast< ModelPanel* >(pParent->FindChildByName( m_szModelPanel ));
		if ( pModel )
		{
			if(strstr(pszAnimation, "opening"))
			{
				pModel->SetAnimation(pszAnimation);
				if(m_flHoverTime > 0)
				{
					float frame;
					frame = 255 -  255 * (m_flHoverTime - gEngfuncs.GetClientTime()) / m_flHoverTimeToWait;
					pModel->GetEntity()->curstate.frame = frame;
				}
				if(reverse)
				{
					pModel->GetEntity()->curstate.framerate = -1;
					if(m_flHoverTime <= 0)
					{
						pModel->GetEntity()->curstate.frame = 255;;
					}
				}
			}
			else
			{
				pModel->SetAnimation(pszAnimation);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool TFTeamButton::IsDisabled()
{
	bool bRetVal = false;

	if ( ( m_iTeam > TEAM_UNASSIGNED ) && GetParent() )
	{
		CTFTeamMenu *pTeamMenu = dynamic_cast< CTFTeamMenu* >( GetParent() );
		if ( pTeamMenu )
		{
			bRetVal = pTeamMenu->IsTeamDisabled(m_iTeam);
		}
	}

	return bRetVal;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TFTeamButton::OnCursorEntered()
{
	BaseClass::OnCursorEntered();

	SetMouseEnteredState( true );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TFTeamButton::OnCursorExited()
{
	BaseClass::OnCursorExited();

	SetMouseEnteredState( false );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TFTeamButton::SetMouseEnteredState( bool state )
{
	if ( state )
	{
		m_bMouseEntered = true;

		if(strstr(m_szModelPanel, "door"))
			SendAnimation("door_opening");
		else
			SendAnimation("TV_ON");

		if ( m_flHoverTimeToWait > 0 )
		{
			m_flHoverTime = gEngfuncs.GetClientTime() + m_flHoverTimeToWait;
		}
		else
		{
			m_flHoverTime = -1;
		}
	}
	else
	{
		m_bMouseEntered = false;

		if(strstr(m_szModelPanel, "door"))
			SendAnimation("door_opening", true);
		else
			SendAnimation("TV_OFF");

		if ( m_flHoverTimeToWait > 0 )
		{
			m_flHoverTime = gEngfuncs.GetClientTime() + m_flHoverTimeToWait;
		}
		else
		{
			m_flHoverTime = -1;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TFTeamButton::OnTick()
{
	// check to see if our state has changed
	bool bDisabled = IsDisabled();

	if ( bDisabled != m_bTeamDisabled )
	{
		m_bTeamDisabled = bDisabled;

		if ( m_bMouseEntered )
		{
			// something has changed, so reset our state
			SetMouseEnteredState( true );
		}
		else
		{
			// the mouse isn't currently over the button, but we should update the status
			if ( m_bTeamDisabled )
			{
				if(strstr(m_szModelPanel, "door"))
					SendAnimation("disabled");
				else
					SendAnimation("TV_disabled");
			}
			else
			{
				if(strstr(m_szModelPanel, "door"))
					SendAnimation("door_closed");
				else
					SendAnimation("TV_OFF");
			}
		}
	}

	if ( ( m_flHoverTime > 0 ) && ( m_flHoverTime < gEngfuncs.GetClientTime() ) )
	{
		m_flHoverTime = -1;

		if ( m_bTeamDisabled )
		{
			if(strstr(m_szModelPanel, "door"))
				SendAnimation("disabled");
			else
				SendAnimation("TV_disabled");
		}
		else
		{
			if(m_bMouseEntered)
			{
				if(strstr(m_szModelPanel, "door"))
					SendAnimation( "door_opened" );
				else
					SendAnimation( "TV_ON" );
			}
			else
			{
				if(strstr(m_szModelPanel, "door"))
					SendAnimation( "door_closed" );
				else
					SendAnimation( "TV_OFF" );
			}
		}
	}
}

CTFTeamMenu::CTFTeamMenu(void) : CTeamMenu()
{
	m_pBlueTeamButton = new TFTeamButton( this, "teambutton0" );
	m_pRedTeamButton = new TFTeamButton( this, "teambutton1" );
	m_pAutoTeamButton = new TFTeamButton( this, "teambutton2" );
	m_pSpecTeamButton = new TFTeamButton( this, "teambutton3" );
	m_pSpecLabel = new Label( this, "TeamMenuSpectate", "" );

	m_pCancelButton = new Button( this, "CancelButton", "#CKF3_Cancel" );

	m_pModelPanel[0] = new ModelPanel( this, "autodoor");
	m_pModelPanel[1] = new ModelPanel( this, "bluedoor");
	m_pModelPanel[2] = new ModelPanel( this, "reddoor");
	m_pModelPanel[3] = new ModelPanel( this, "spectate");

	vgui::ivgui()->AddTickSignal( GetVPanel() );

	SetPaintBackgroundEnabled(false);

	LoadControlSettings("Resource/UI/TFTeamMenu.res");
	InvalidateLayout();
}

bool CTFTeamMenu::IsTeamDisabled(int iTeam)
{
	if(iTeam == TEAM_RED && g_iTeamNumber == TEAM_RED)
	{
		return true;
	}
	if(iTeam == TEAM_BLUE && g_iTeamNumber == TEAM_BLUE)
	{
		return true;
	}
	return false;
}

void CTFTeamMenu::ShowPanel(bool bShow)
{
	if (bShow)
	{
		if (gHUD.m_iIntermission || gEngfuncs.IsSpectateOnly())
			return;

		m_pModelPanel[0]->LoadModel();
		m_pModelPanel[1]->LoadModel();
		m_pModelPanel[2]->LoadModel();
		m_pModelPanel[3]->LoadModel();
	}

	BaseClass::ShowPanel(bShow);
}

void CTFTeamMenu::Update(void)
{
	if (gViewPortInterface->GetAllowSpectators())
	{
		if (g_iTeamNumber == TEAM_UNASSIGNED || g_iFreezeTimeOver || (g_PlayerExtraInfo[gHUD.m_iPlayerNum].dead))
			m_pSpecTeamButton->SetVisible( true );
		else
			m_pSpecTeamButton->SetVisible( false );
	}
	else
		m_pSpecTeamButton->SetVisible( false );

	if ( g_iTeamNumber != TEAM_UNASSIGNED )
	{
		if ( m_pCancelButton )
		{
			m_pCancelButton->SetVisible( true );
		}
	}
	else
	{
		if ( m_pCancelButton && m_pCancelButton->IsVisible() )
		{
			m_pCancelButton->SetVisible( false );
		}
	}
}

void CTFTeamMenu::SetVisible(bool state)
{
	BaseClass::SetVisible(state);
}

void CTFTeamMenu::OnCommand(const char *command)
{
	if (Q_stricmp(command, "vguicancel"))
	{
		//engine->pfnClientCmd(const_cast<char *>(command));

		// we're selecting a team, so make sure it's not the team we're already on before sending to the server
		if ( Q_strstr( command, "jointeam " ) )
		{
			const char *pTeam = command + Q_strlen( "jointeam " );
			int iTeam = TEAM_UNASSIGNED;

			if ( Q_stricmp( pTeam, "spectate" ) == 0 || Q_stricmp( pTeam, "3" ) == 0)
			{
				iTeam = TEAM_SPECTATOR;
			}
			else if ( Q_stricmp( pTeam, "red" ) == 0 || Q_stricmp( pTeam, "1" ) == 0)
			{
				iTeam = TEAM_RED;
			}
			else if ( Q_stricmp( pTeam, "blue" ) == 0 || Q_stricmp( pTeam, "2" ) == 0)
			{
				iTeam = TEAM_BLUE;
			}
			else if ( Q_stricmp( pTeam, "random" ) == 0 || Q_stricmp( pTeam, "4" ) == 0 || Q_stricmp( pTeam, "0" ) == 0)
			{
				iTeam = TEAM_RANDOM;
			}
			// are we selecting the team we're already on?
			if ( g_iTeamNumber != iTeam )
			{
				engine->pfnClientCmd( (char *)command );
				engine->pfnClientCmd( "\n" );
			}
		}
		else if ( Q_strstr( command, "jointeam_nomenus " ) )
		{
			engine->pfnClientCmd( (char *)command );
			engine->pfnClientCmd( "\n" );
		}
	}

	BaseClass::OnCommand(command);
	ShowPanel( false );
	OnClose();
}

void CTFTeamMenu::PerformLayout(void)
{
	BaseClass::PerformLayout();
}

void CTFTeamMenu::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	Update();
}