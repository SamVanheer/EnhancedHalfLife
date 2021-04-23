/***
*
*	Copyright (c) 1999, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#pragma once

constexpr int RGB_YELLOWISH = 0x00FFA000; //255,160,0
constexpr int RGB_REDISH = 0x00FF1010; //255,160,0
constexpr int RGB_GREENISH = 0x0000A000; //0,160,0

#include "wrect.h"
#include "cl_dll.h"
#include "ammo.h"

constexpr int DHN_DRAWZERO = 1;
constexpr int DHN_2DIGITS = 2;
constexpr int DHN_3DIGITS = 4;
constexpr int MIN_ALPHA = 100;

struct POSITION
{
	int x, y;
};

#include "global_consts.h"

struct RGBA
{
	unsigned char r, g, b, a;
};

struct cvar_t;


constexpr int HUD_ACTIVE = 1;
constexpr int HUD_INTERMISSION = 2;

constexpr int MAX_PLAYER_NAME_LENGTH = 32;

constexpr int MAX_MOTD_LENGTH = 1536;

class CHudBase
{
public:
	POSITION m_pos{};
	int m_type = 0;
	int m_iFlags = 0; // active, moving, 
	virtual ~CHudBase() {}
	virtual bool Init() { return false; }
	virtual bool VidInit() { return false; }
	virtual void Shutdown() {}
	virtual bool Draw(float flTime) { return false; }
	virtual void Think() {}
	virtual void Reset() {}
	virtual void InitHUDData() {}		// called every time a server is connected to

};

struct HUDLIST
{
	CHudBase* p;
	HUDLIST* pNext;
};

#include "voice_status.h" // base voice handling class
#include "hud_spectator.h"

class CHudAmmo : public CHudBase
{
public:
	bool Init() override;
	bool VidInit() override;
	bool Draw(float flTime) override;

	/**
	*	@brief Used for selection of weapon menu item.
	*/
	void Think() override;
	void Reset() override;
	bool DrawWList(float flTime);

	/**
	*	@brief Update hud state with the current weapon and clip count.
	*	Ammo counts are updated with AmmoX. Server assures that the Weapon ammo type numbers match a real ammo type.
	*/
	bool MsgFunc_CurWeapon(const char* pszName, int iSize, void* pbuf);

	/**
	*	@brief Tells the hud about a new weapon type.
	*/
	bool MsgFunc_WeaponList(const char* pszName, int iSize, void* pbuf);

	/**
	*	@brief Update the count of a known type of ammo
	*/
	bool MsgFunc_AmmoX(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_AmmoPickup(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_WeapPickup(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_ItemPickup(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_HideWeapon(const char* pszName, int iSize, void* pbuf);

	void SlotInput(int iSlot);
	void UserCmd_Slot1();
	void UserCmd_Slot2();
	void UserCmd_Slot3();
	void UserCmd_Slot4();
	void UserCmd_Slot5();
	void UserCmd_Slot6();
	void UserCmd_Slot7();
	void UserCmd_Slot8();
	void UserCmd_Slot9();
	void UserCmd_Slot10();
	void UserCmd_Close();

	/**
	*	@brief Selects the next item in the weapon menu
	*/
	void UserCmd_NextWeapon();

	/**
	*	@brief Selects the previous item in the menu
	*/
	void UserCmd_PrevWeapon();

private:
	/**
	*	@brief Menu selection code
	*/
	void SelectSlot(int iSlot, bool fAdvance, int iDirection);

private:
	enum class SelectionType
	{
		Off,
		MenuBar,
		ActiveWeapon
	};

	struct WeaponSelection
	{
		SelectionType Type = SelectionType::Off;
		WEAPON* Selection = nullptr;
	};

	float m_fFade = 0;
	RGBA  m_rgba{};

	WeaponSelection m_ActiveSel;
	WeaponSelection m_LastSel;		//!< Last weapon menu selection
	WEAPON* m_pWeapon = nullptr;
	int	m_HUD_bucket0 = 0;
	int m_HUD_selection = 0;
};

#include "health.h"

constexpr int FADE_TIME = 100;

class CHudGeiger : public CHudBase
{
public:
	bool Init() override;
	bool VidInit() override;
	bool Draw(float flTime) override;
	bool MsgFunc_Geiger(const char* pszName, int iSize, void* pbuf);

private:
	int m_iGeigerRange = 0;
};

class CHudTrain : public CHudBase
{
public:
	bool Init() override;
	bool VidInit() override;
	bool Draw(float flTime) override;
	bool MsgFunc_Train(const char* pszName, int iSize, void* pbuf);

private:
	HSPRITE m_hSprite{};
	int m_iPos = 0;
};

/**
*	@brief generic text status bar, set by game dll
*
*	runs across bottom of screen
*/
class CHudStatusBar : public CHudBase
{
public:
	bool Init() override;
	bool VidInit() override;
	bool Draw(float flTime) override;
	void Reset() override;
	void ParseStatusString(int line_num);

	/**
	*	@brief Message handler for StatusText message
	*	@details accepts two values:
	*		byte: line number of status bar text 
	*		string: status bar text
	*	this string describes how the status bar should be drawn
	*	a semi-regular expression:
	*	( slotnum ([a..z] [%pX] [%iX])*)*
	*	where slotnum is an index into the Value table (see below)
	*	if slotnum is 0, the string is always drawn
	*	if StatusValue[slotnum] != 0, the following string is drawn, upto the next newline - otherwise the text is skipped upto next newline
	*	%pX, where X is an integer, will substitute a player name here, getting the player index from StatusValue[X]
	*	%iX, where X is an integer, will substitute a number here, getting the number from StatusValue[X]
	*/
	bool MsgFunc_StatusText(const char* pszName, int iSize, void* pbuf);

	/**
	*	@brief Message handler for StatusText message
	*	@details accepts two values:
	*		byte: index into the status value array
	*		short: value to store
	*/
	bool MsgFunc_StatusValue(const char* pszName, int iSize, void* pbuf);

protected:
	enum
	{
		MAX_STATUSTEXT_LENGTH = 128,
		MAX_STATUSBAR_VALUES = 8,
		MAX_STATUSBAR_LINES = 3,
	};

	char m_szStatusText[MAX_STATUSBAR_LINES][MAX_STATUSTEXT_LENGTH]{};  // a text string describing how the status bar is to be drawn
	char m_szStatusBar[MAX_STATUSBAR_LINES][MAX_STATUSTEXT_LENGTH]{};	// the constructed bar that is drawn
	int m_iStatusValues[MAX_STATUSBAR_VALUES]{};  // an array of values for use in the status bar

	bool m_bReparseString = false; // set to true whenever the m_szStatusBar needs to be recalculated

	// an array of colors...one color for each line
	const Vector* m_pflNameColors[MAX_STATUSBAR_LINES]{};
};

struct extra_player_info_t
{
	short frags;
	short deaths;
	short playerclass;
	short health; // UNUSED currently, spectator UI would like this
	bool dead; // UNUSED currently, spectator UI would like this
	short teamnumber;
	char teamname[MAX_TEAM_NAME];
};

struct team_info_t
{
	char name[MAX_TEAM_NAME];
	short frags;
	short deaths;
	short ping;
	short packetloss;
	short ownteam;
	short players;
	int already_drawn;
	int scores_overriden;
	int teamnumber;
};

#include "player_info.h"

class CHudDeathNotice : public CHudBase
{
public:
	bool Init() override;
	void InitHUDData() override;
	bool VidInit() override;
	bool Draw(float flTime) override;

	/**
	*	@brief This message handler may be better off elsewhere
	*/
	bool MsgFunc_DeathMsg(const char* pszName, int iSize, void* pbuf);

private:
	int m_HUD_d_skull = 0;  // sprite index of skull icon
};

/**
*	@brief generic menu handler
*/
class CHudMenu : public CHudBase
{
public:
	bool Init() override;
	void InitHUDData() override;
	bool VidInit() override;
	void Reset() override;
	bool Draw(float flTime) override;

	/**
	*	@brief Message handler for ShowMenu message
	*	@details takes four values:
	*		short: a bitfield of keys that are valid input
	*		char : the duration, in seconds, the menu should stay up. -1 means is stays until something is chosen.
	*		byte : a boolean, true if there is more string yet to be received before displaying the menu, false if it's the last string
	*		string: menu string to display
	*	if this message is never received, then scores will simply be the combined totals of the players.
	*/
	bool MsgFunc_ShowMenu(const char* pszName, int iSize, void* pbuf);

	void SelectMenuItem(int menu_item);

	bool m_fMenuDisplayed = false;
	int m_bitsValidSlots = 0;
	float m_flShutoffTime = 0;
	bool m_fWaitingForMore = false;
};

class CHudSayText : public CHudBase
{
public:
	bool Init() override;
	void InitHUDData() override;
	bool VidInit() override;
	bool Draw(float flTime) override;
	bool MsgFunc_SayText(const char* pszName, int iSize, void* pbuf);
	void SayTextPrint(const char* pszBuf, int iBufSize, int clientIndex = -1);
	void EnsureTextFitsInOneLineAndWrapIfHaveTo(int line);
	friend class CHudSpectator;

private:
	cvar_t* m_HUD_saytext = nullptr;
	cvar_t* m_HUD_saytext_time = nullptr;
};

class CHudBattery : public CHudBase
{
public:
	bool Init() override;
	bool VidInit() override;
	bool Draw(float flTime) override;
	bool MsgFunc_Battery(const char* pszName, int iSize, void* pbuf);

private:
	HSPRITE m_hSprite1{};
	HSPRITE m_hSprite2{};
	wrect_t* m_prc1 = nullptr;
	wrect_t* m_prc2 = nullptr;
	int m_iBat = 0;
	float m_fFade = 0;
	int m_iHeight = 0;		// width of the battery innards
};

class CHudFlashlight : public CHudBase
{
public:
	bool Init() override;
	bool VidInit() override;
	bool Draw(float flTime) override;
	void Reset() override;
	bool MsgFunc_Flashlight(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_FlashBat(const char* pszName, int iSize, void* pbuf);

private:
	HSPRITE m_hSprite1{};
	HSPRITE m_hSprite2{};
	HSPRITE m_hBeam{};
	wrect_t* m_prc1 = nullptr;
	wrect_t* m_prc2 = nullptr;
	wrect_t* m_prcBeam = nullptr;
	float m_flBat = 0;
	int m_iBat = 0;
	bool m_fOn = false;
	int m_iWidth = 0;		// width of the battery innards
};

const int maxHUDMessages = 16;
struct message_parms_t
{
	client_textmessage_t* pMessage;
	float time;
	int x, y;
	int	totalWidth, totalHeight;
	int width;
	int lines;
	int lineLength;
	int length;
	int r, g, b;
	int text;
	int fadeBlend;
	float charTime;
	float fadeTime;
};

/**
*	@brief this class routes messages through titles.txt for localisation
*/
class CHudTextMessage : public CHudBase
{
public:
	bool Init() override;

	/**
	*	@brief Searches through the string for any msg names (indicated by a '#')
	*	any found are looked up in titles.txt and the new message substituted
	*	the new value is pushed into dst_buffer
	*/
	static char* LocaliseTextString(const char* msg, char* dst_buffer, int buffer_size);

	/**
	*	@brief As LocaliseTextString, but with a local static buffer
	*/
	static char* BufferedLocaliseTextString(const char* msg);

	/**
	*	@brief Simplified version of LocaliseTextString; assumes string is only one word
	*/
	const char* LookupString(const char* msg_name, int* msg_dest = nullptr);

	/**
	*	@brief Message handler for text messages
	*	@details displays a string, looking them up from the titles.txt file, which can be localised
	*	parameters:
	*	  byte:   message direction  ( HUD_PRINTCONSOLE, HUD_PRINTNOTIFY, HUD_PRINTCENTER, HUD_PRINTTALK )
	*	  string: message
	*	optional parameters:
	*	  string: message parameter 1
	*	  string: message parameter 2
	*	  string: message parameter 3
	*	  string: message parameter 4
	*	any string that starts with the character '#' is a message name, and is used to look up the real message in titles.txt
	*	the next (optional) one to four strings are parameters for that string (which can also be message names if they begin with '#')
	*/
	bool MsgFunc_TextMsg(const char* pszName, int iSize, void* pbuf);
};

class CHudMessage : public CHudBase
{
public:
	bool Init() override;
	bool VidInit() override;
	bool Draw(float flTime) override;
	bool MsgFunc_HudText(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_GameTitle(const char* pszName, int iSize, void* pbuf);

	float FadeBlend(float fadein, float fadeout, float hold, float localTime);
	int	XPosition(float x, int width, int lineWidth);
	int YPosition(float y, int height);

	void MessageAdd(const char* pName, float time);
	void MessageAdd(client_textmessage_t* newMessage);
	void MessageDrawScan(client_textmessage_t* pMessage, float time);
	void MessageScanStart();
	void MessageScanNextChar();
	void Reset() override;

private:
	client_textmessage_t* m_pMessages[maxHUDMessages]{};
	float m_startTime[maxHUDMessages]{};
	message_parms_t m_parms{};
	float m_gameTitleTime = 0;
	client_textmessage_t* m_pGameTitle = nullptr;

	int m_HUD_title_life = 0;
	int m_HUD_title_half = 0;
};

constexpr int MAX_SPRITE_NAME_LENGTH = 24;

class CHudStatusIcons : public CHudBase
{
public:
	bool Init() override;
	bool VidInit() override;
	void Reset() override;

	/**
	*	@brief Draw status icons along the left-hand side of the screen
	*/
	bool Draw(float flTime) override;

	/**
	*	@brief Message handler for StatusIcon message
	*	@details accepts five values:
	*		byte   : true = ENABLE icon, false = DISABLE icon
	*		string : the sprite name to display
	*		byte   : red
	*		byte   : green
	*		byte   : blue
	*/
	bool MsgFunc_StatusIcon(const char* pszName, int iSize, void* pbuf);

	enum
	{
		MAX_ICONSPRITENAME_LENGTH = MAX_SPRITE_NAME_LENGTH,
		MAX_ICONSPRITES = 4,
	};


	//had to make these public so CHud could access them (to enable concussion icon)
	//could use a friend declaration instead...
	/**
	*	@brief add the icon to the icon list, and set it's drawing color
	*/
	void EnableIcon(const char* pszIconName, unsigned char red, unsigned char green, unsigned char blue);
	void DisableIcon(const char* pszIconName);

private:
	struct icon_sprite_t
	{
		char szSpriteName[MAX_ICONSPRITENAME_LENGTH];
		HSPRITE spr;
		wrect_t rc;
		unsigned char r, g, b;
	};

	icon_sprite_t m_IconList[MAX_ICONSPRITES]{};
};

/**
*	@brief CHud handles the message, calculation, and drawing the HUD
*/
class CHud
{
public:
	CHud() = default;

	/**
	*	@brief cleans up memory allocated for m_rg* arrays
	*/
	~CHud();

	int DrawHudNumber(int x, int y, int iFlags, int iNumber, int r, int g, int b);
	int DrawHudString(int x, int y, int iMaxX, char* szString, int r, int g, int b);

	/**
	*	@brief draws a string from right to left (right-aligned)
	*/
	int DrawHudStringReverse(int xpos, int ypos, int iMinX, char* szString, int r, int g, int b);
	int DrawHudNumberString(int xpos, int ypos, int iMinX, int iNumber, int r, int g, int b);
	int GetNumWidth(int iNumber, int iFlags);

	int GetHudNumberWidth(int number, int width, int flags);
	int DrawHudNumberReverse(int x, int y, int number, int flags, int r, int g, int b);

	HSPRITE GetSprite(int index)
	{
		return (index < 0) ? 0 : m_rghSprites[index];
	}

	wrect_t& GetSpriteRect(int index)
	{
		return m_rgrcRects[index];
	}

	/**
	*	@brief searches through the sprite list loaded from hud.txt for a name matching SpriteName
	*	returns an index into the gHUD.m_rghSprites[] array
	*	returns -1 if sprite not found
	*/
	int GetSpriteIndex(const char* SpriteName);

	/**
	*	@brief This is called every time the DLL is loaded
	*/
	void Init();
	void VidInit();
	void Shutdown();
	void Think();

	/**
	*	@brief step through the local data, placing the appropriate graphics & text as appropriate
	*	@return true if they've changed, false otherwise
	*/
	bool Redraw(float flTime, bool intermission);
	bool UpdateClientData(client_data_t* cdata, float time);

	// user messages
	bool MsgFunc_Damage(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_GameMode(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_Logo(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_ResetHUD(const char* pszName, int iSize, void* pbuf);
	void MsgFunc_InitHUD(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_SetFOV(const char* pszName, int iSize, void* pbuf);

	void AddHudElem(CHudBase* p);

	float GetSensitivity();

public:
	HSPRITE m_hsprCursor{};
	float m_flTime = 0;	   //!< the current client time
	float m_fOldTime = 0;  //!< the time at which the HUD was last redrawn
	double m_flTimeDelta = 0; //!< the difference between flTime and fOldTime
	Vector m_vecOrigin{vec3_origin};
	Vector m_vecAngles{vec3_origin};
	int m_iKeyBits = 0;
	int m_iHideHUDDisplay = 0;
	int m_iFOV = 0;
	int m_Teamplay = 0;
	int m_iRes = 0;
	cvar_t* m_pCvarStealMouse = nullptr;
	cvar_t* m_pCvarDraw = nullptr;

	int m_iFontHeight = 0;

	// Screen information
	SCREENINFO m_scrinfo{};

	int	m_iWeaponBits = 0;
	bool m_fPlayerDead = false;
	bool m_iIntermission = false;

	// sprite indexes
	int m_HUD_number_0 = 0;

	CHudAmmo		m_Ammo;
	CHudHealth		m_Health;
	CHudSpectator	m_Spectator;
	CHudGeiger		m_Geiger;
	CHudBattery		m_Battery;
	CHudTrain		m_Train;
	CHudFlashlight	m_Flash;
	CHudMessage		m_Message;
	CHudStatusBar   m_StatusBar;
	CHudDeathNotice m_DeathNotice;
	CHudSayText		m_SayText;
	CHudMenu		m_Menu;
	CHudTextMessage m_TextMessage;
	CHudStatusIcons m_StatusIcons;

private:
	HUDLIST* m_pHudList = nullptr;
	HSPRITE m_hsprLogo{};
	int m_iLogo = 0;
	client_sprite_t* m_pSpriteList = nullptr;
	int m_iSpriteCount = 0;
	int m_iSpriteCountAllRes = 0;
	float m_flMouseSensitivity = 0;

	// the memory for these arrays are allocated in the first call to CHud::VidInit(), when the hud.txt and associated sprites are loaded.
	// freed in ~CHud()
	HSPRITE* m_rghSprites = nullptr;	/*[HUD_SPRITE_COUNT]*/			// the sprites loaded from hud.txt
	wrect_t* m_rgrcRects = nullptr;	/*[HUD_SPRITE_COUNT]*/
	char* m_rgszSpriteNames = nullptr; /*[HUD_SPRITE_COUNT][MAX_SPRITE_NAME_LENGTH]*/

	cvar_t* default_fov = nullptr;
};

inline CHud gHUD;

extern int g_iPlayerClass;
extern int g_iTeamNumber;
extern int g_iUser1;
extern int g_iUser2;
extern int g_iUser3;
