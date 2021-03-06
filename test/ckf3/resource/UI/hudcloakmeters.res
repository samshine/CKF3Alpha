"Resource/UI/HudCloakMeters.res"
{
	"HudCloakMeter"
	{
		"fieldName"		"HudCloakMeter"
		"visible"		"1"
		"enabled"		"1"
		"xpos"			"r175"
		"ypos"			"r50"
		"wide"			"100"
		"tall"			"50"
	}

	"HudCloakMeterBG"
	{
		"ControlName"	"ImagePanel"
		"fieldName"		"HudCloakMeterBG"
		"xpos"			"12"
		"ypos"			"0"
		"zpos"			"0"
		"wide"			"50"
		"tall"			"40"
		"visible"		"1"
		"enabled"		"1"
		"image"			"resource/tga/mask_spy_red"
		"scaleImage"	"1"			
	}
	
	"CloakMeterLabel"
	{
		"ControlName"			"Label"
		"fieldName"				"CloakMeterLabel"
		"xpos"					"21"
		"ypos"					"24"
		"zpos"					"2"
		"wide"					"41"
		"tall"					"15"
		"autoResize"			"1"
		"pinCorner"				"2"
		"visible"				"1"
		"enabled"				"1"
		"tabPosition"			"0"
		"labelText"				"#CKF3_Cloak"
		"textAlignment"			"center"
		"dulltext"				"0"
		"brighttext"			"0"
		"font"					"TFFontSmall"
	}

	"CloakMeterMeter"
	{	
		"ControlName"			"ContinuousProgressBar"
		"fieldName"				"CloakMeterMeter"
		"font"					"Default"
		"xpos"					"26"
		"ypos"					"23"
		"zpos"					"2"
		"wide"					"28"
		"tall"					"4"
		"autoResize"			"0"
		"pinCorner"				"0"
		"visible"				"0"
		"enabled"				"0"
		"textAlignment"			"Left"
		"dulltext"				"0"
		"brighttext"			"0"
	}					
	
	"CloakMeterCount"
	{
		"ControlName"			"Label"
		"fieldName"				"CloakMeterCount"
		"xpos"					"16"
		"ypos"					"6"
		"zpos"					"2"
		"wide"					"44"
		"tall"					"20"	
		"pinCorner"				"2"
		"visible"				"1"
		"enabled"				"1"
		"tabPosition"			"0"
		"labelText"				"%cloakenergy%"
		"textAlignment"			"north"
		"dulltext"				"0"
		"brighttext"			"0"
		"font"					"HudFontMediumSmall"
	}
}
