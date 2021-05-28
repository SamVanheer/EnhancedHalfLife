#pragma once

#include<VGUI_Panel.h>
#include<VGUI_Frame.h>
#include<VGUI_TextPanel.h>
#include<VGUI_Label.h>
#include<VGUI_Button.h>
#include<VGUI_ActionSignal.h>
#include<VGUI_InputSignal.h>
#include<VGUI_Scheme.h>
#include<VGUI_Image.h>
#include<VGUI_FileInputStream.h>
#include<VGUI_BitmapTGA.h>
#include<VGUI_DesktopIcon.h>
#include<VGUI_App.h>
#include<VGUI_MiniApp.h>
#include<VGUI_LineBorder.h>
#include<VGUI_String.h>
#include<VGUI_ScrollPanel.h>
#include<VGUI_ScrollBar.h>
#include<VGUI_Slider.h>

// custom scheme handling
#include "vgui_SchemeManager.hpp"

constexpr int PC_LASTCLASS = 10;
constexpr int PC_UNDEFINED = 0;
constexpr int MENU_DEFAULT = 1;
constexpr int MENU_MAPBRIEFING = 4;
constexpr int MENU_INTRO = 5;
constexpr int MENU_CLASSHELP = 6;
constexpr int MENU_CLASSHELP2 = 7;
constexpr int MENU_REPEATHELP = 8;
constexpr int MENU_SPECHELP = 9;

using namespace vgui;

class ScorePanel;
class SpectatorPanel;
class CCommandMenu;
class CommandLabel;
class CommandButton;
class CMenuPanel;
class DragNDropPanel;
class CTransparentPanel;
class TeamFortressViewport;

/**
*	@brief Loads a .tga file and returns a pointer to the VGUI tga object
*/
BitmapTGA* LoadTGAForRes(const char* pImageName);
void ScaleColors(int& r, int& g, int& b, int a);
const Vector& GetClientColor(int clientIndex);
constexpr Vector g_ColorBlue{0.6, 0.8, 1.0};
constexpr Vector g_ColorRed{1.0, 0.25, 0.25};
constexpr Vector g_ColorGreen{0.6, 1.0, 0.6};
constexpr Vector g_ColorYellow{1.0, 0.7, 0.0};
constexpr Vector g_ColorGrey{0.8, 0.8, 0.8};
extern int iTeamColors[5][3];
extern int iNumberOfTeamColors;
extern TeamFortressViewport* gViewPort;


// Command Menu positions 
constexpr int MAX_MENUS = 80;
constexpr int MAX_BUTTONS = 100;

#define BUTTON_SIZE_Y			YRES(30)
#define CMENU_SIZE_X			XRES(160)

#define SUBMENU_SIZE_X			(CMENU_SIZE_X / 8)
#define SUBMENU_SIZE_Y			(BUTTON_SIZE_Y / 6)

#define CMENU_TOP				(BUTTON_SIZE_Y * 4)

//constexpr int MAX_TEAMNAME_SIZE = 64;
constexpr int MAX_BUTTON_SIZE = 32;

// Map Briefing Window
constexpr int MAPBRIEF_INDENT = 30;

// Team Menu
#define TMENU_INDENT_X			(30 * ((float)ScreenHeight / 640))
constexpr int TMENU_HEADER = 100;
#define TMENU_SIZE_X			(ScreenWidth - (TMENU_INDENT_X * 2))
#define TMENU_SIZE_Y			(TMENU_HEADER + BUTTON_SIZE_Y * 7)
#define TMENU_PLAYER_INDENT		(((float)TMENU_SIZE_X / 3) * 2)
#define TMENU_INDENT_Y			(((float)ScreenHeight - TMENU_SIZE_Y) / 2)

// Class Menu
#define CLMENU_INDENT_X			(30 * ((float)ScreenHeight / 640))
constexpr int CLMENU_HEADER = 100;
#define CLMENU_SIZE_X			(ScreenWidth - (CLMENU_INDENT_X * 2))
#define CLMENU_SIZE_Y			(CLMENU_HEADER + BUTTON_SIZE_Y * 11)
#define CLMENU_PLAYER_INDENT	(((float)CLMENU_SIZE_X / 3) * 2)
#define CLMENU_INDENT_Y			(((float)ScreenHeight - CLMENU_SIZE_Y) / 2)

// Arrows
enum
{
	ARROW_UP,
	ARROW_DOWN,
	ARROW_LEFT,
	ARROW_RIGHT,
};

//==============================================================================
// VIEWPORT PIECES
//============================================================
// Wrapper for an Image Label without a background
class CImageLabel : public Label
{
public:
	BitmapTGA* m_pTGA;

public:
	void LoadImage(const char* pImageName);

	/**
	*	@brief Button with Class image beneath it
	*/
	CImageLabel(const char* pImageName, int x, int y);
	CImageLabel(const char* pImageName, int x, int y, int wide, int tall);

	virtual int getImageTall();
	virtual int getImageWide();

	virtual void paintBackground()
	{
		// Do nothing, so the background's left transparent.
	}
};

// Command Label
// Overridden label so we can darken it when submenus open
class CommandLabel : public Label
{
private:
	int		m_iState;

public:
	CommandLabel(const char* text, int x, int y, int wide, int tall) : Label(text, x, y, wide, tall)
	{
		m_iState = false;
	}

	void PushUp()
	{
		m_iState = false;
		repaint();
	}

	void PushDown()
	{
		m_iState = true;
		repaint();
	}
};

//============================================================
// Command Buttons
/**
*	@brief All TFC Hud buttons are derived from this one.
*/
class CommandButton : public Button
{
private:
	int		m_iPlayerClass;
	bool	m_bFlat;

	// Submenus under this button
	CCommandMenu* m_pSubMenu;
	CCommandMenu* m_pParentMenu;
	CommandLabel* m_pSubLabel;

	char m_sMainText[MAX_BUTTON_SIZE];
	char m_cBoundKey;

	SchemeHandle_t m_hTextScheme;

	/**
	*	@brief Prepends the button text with the current bound key
	*	if no bound key, then a clear space ' ' instead
	*/
	void RecalculateText();

public:
	bool	m_bNoHighlight;

public:
	CommandButton(const char* text, int x, int y, int wide, int tall, bool bNoHighlight, bool bFlat);
	// Constructors
	CommandButton(const char* text, int x, int y, int wide, int tall, bool bNoHighlight = false);
	CommandButton(int iPlayerClass, const char* text, int x, int y, int wide, int tall, bool bFlat);

	void Init();

	// Menu Handling
	void AddSubMenu(CCommandMenu* pNewMenu);
	void AddSubLabel(CommandLabel* pSubLabel)
	{
		m_pSubLabel = pSubLabel;
	}

	virtual int IsNotValid()
	{
		return false;
	}

	void UpdateSubMenus(int iAdjustment);
	int GetPlayerClass() { return m_iPlayerClass; };
	CCommandMenu* GetSubMenu() { return m_pSubMenu; };

	/**
	*	@brief Returns the command menu that the button is part of, if any
	*/
	CCommandMenu* getParentMenu();

	/**
	*	@brief Sets the menu that contains this button
	*/
	void setParentMenu(CCommandMenu* pParentMenu);

	// Overloaded vgui functions
	virtual void paint();
	virtual void setText(const char* text);
	virtual void paintBackground();

	/**
	*	@brief Highlights the current button, and all it's parent menus
	*/
	void cursorEntered();
	void cursorExited();

	void setBoundKey(char boundKey);
	char getBoundKey();
};

class ColorButton : public CommandButton
{
private:

	Color* ArmedColor;
	Color* UnArmedColor;

	Color* ArmedBorderColor;
	Color* UnArmedBorderColor;

public:
	ColorButton(const char* text, int x, int y, int wide, int tall, bool bNoHighlight, bool bFlat) :
		CommandButton(text, x, y, wide, tall, bNoHighlight, bFlat)
	{
		ArmedColor = nullptr;
		UnArmedColor = nullptr;
		ArmedBorderColor = nullptr;
		UnArmedBorderColor = nullptr;
	}


	virtual void paintBackground()
	{
		int r, g, b, a;
		Color bgcolor;

		if (isArmed())
		{
			// Highlight background
		/*	getBgColor(bgcolor);
			bgcolor.getColor(r, g, b, a);
			drawSetColor( r,g,b,a );
			drawFilledRect(0,0,_size[0],_size[1]);*/

			if (ArmedBorderColor)
			{
				ArmedBorderColor->getColor(r, g, b, a);
				drawSetColor(r, g, b, a);
				drawOutlinedRect(0, 0, _size[0], _size[1]);
			}
		}
		else
		{
			if (UnArmedBorderColor)
			{
				UnArmedBorderColor->getColor(r, g, b, a);
				drawSetColor(r, g, b, a);
				drawOutlinedRect(0, 0, _size[0], _size[1]);
			}
		}
	}
	void paint()
	{
		int r, g, b, a;
		if (isArmed())
		{
			if (ArmedColor)
			{
				ArmedColor->getColor(r, g, b, a);
				setFgColor(r, g, b, a);
			}
			else
				setFgColor(Scheme::sc_secondary2);
		}
		else
		{
			if (UnArmedColor)
			{
				UnArmedColor->getColor(r, g, b, a);
				setFgColor(r, g, b, a);
			}
			else
				setFgColor(Scheme::sc_primary1);
		}

		Button::paint();
	}

	void setArmedColor(int r, int g, int b, int a)
	{
		ArmedColor = new Color(r, g, b, a);
	}

	void setUnArmedColor(int r, int g, int b, int a)
	{
		UnArmedColor = new Color(r, g, b, a);
	}

	void setArmedBorderColor(int r, int g, int b, int a)
	{
		ArmedBorderColor = new Color(r, g, b, a);
	}

	void setUnArmedBorderColor(int r, int g, int b, int a)
	{
		UnArmedBorderColor = new Color(r, g, b, a);
	}
};

class SpectButton : public CommandButton
{
private:

public:
	SpectButton(int iPlayerClass, const char* text, int x, int y, int wide, int tall) :
		CommandButton(text, x, y, wide, tall, false)
	{
		Init();

		setText(text);
	}

	virtual void paintBackground()
	{
		if (isArmed())
		{
			drawSetColor(143, 143, 54, 125);
			drawFilledRect(5, 0, _size[0] - 5, _size[1]);
		}
	}

	virtual void paint()
	{

		if (isArmed())
		{
			setFgColor(194, 202, 54, 0);
		}
		else
		{
			setFgColor(143, 143, 54, 15);
		}

		Button::paint();
	}
};
//============================================================
// Command Menus
class CCommandMenu : public Panel
{
private:
	CCommandMenu* m_pParentMenu;
	int			  m_iXOffset;
	int			  m_iYOffset;

	// Buttons in this menu
	CommandButton* m_aButtons[MAX_BUTTONS]{};
	int			  m_iButtons = 0;

	// opens menu from top to bottom (0 = default), or from bottom to top (1)?
	int				m_iDirection;
public:
	CCommandMenu(CCommandMenu* pParentMenu, int x, int y, int wide, int tall) : Panel(x, y, wide, tall)
	{
		m_pParentMenu = pParentMenu;
		m_iXOffset = x;
		m_iYOffset = y;
		m_iDirection = 0;
	}


	CCommandMenu(CCommandMenu* pParentMenu, int direction, int x, int y, int wide, int tall) : Panel(x, y, wide, tall)
	{
		m_pParentMenu = pParentMenu;
		m_iXOffset = x;
		m_iYOffset = y;
		m_iDirection = direction;
	}

	float		m_flButtonSizeY = 0;
	int			m_iSpectCmdMenu = 0;
	void		AddButton(CommandButton* pButton);

	/**
	*	@brief Recalculate the visible buttons
	*/
	bool		RecalculateVisibles(int iNewYPos, bool bHideAll);

	/**
	*	@brief Make sure all submenus can fit on the screen
	*/
	void		RecalculatePositions(int iYOffset);

	/**
	*	@brief Make this menu and all menus above it in the chain visible
	*/
	void		MakeVisible(CCommandMenu* pChildMenu);

	CCommandMenu* GetParentMenu() { return m_pParentMenu; };
	int			GetXOffset() { return m_iXOffset; };
	int			GetYOffset() { return m_iYOffset; };
	int			GetDirection() { return m_iDirection; };
	int			GetNumButtons() { return m_iButtons; };
	CommandButton* FindButtonWithSubmenu(CCommandMenu* pSubMenu);

	/**
	*	@brief clears the current menus buttons of any armed (highlighted)  state, and all their sub buttons
	*/
	void		ClearButtonsOfArmedState();

	void		RemoveAllButtons();

	/**
	*	@brief Tries to find a button that has a key bound to the input, and presses the button if found
	*	@param keyNum the character number of the input key
	*	@return true if the command menu should close, false otherwise
	*/
	bool		KeyInput(int keyNum);

	/**
	*	@brief Various overloaded paint functions for Custom VGUI objects
	*/
	virtual void paintBackground();
};

//==============================================================================
// Command menu root button (drop down box style)

class DropDownButton : public ColorButton
{
private:
	CImageLabel* m_pOpenButton;

public:

	DropDownButton(const char* text, int x, int y, int wide, int tall, bool bNoHighlight, bool bFlat) :
		ColorButton(text, x, y, wide, tall, bNoHighlight, bFlat)
	{
		// Put a > to show it's a submenu
		m_pOpenButton = new CImageLabel("arrowup", XRES(CMENU_SIZE_X - 2), YRES(BUTTON_SIZE_Y - 2));
		m_pOpenButton->setParent(this);

		int textwide, texttall;
		getSize(textwide, texttall);

		// Reposition
		m_pOpenButton->setPos(textwide - (m_pOpenButton->getImageWide() + 6), -2 /*(tall - m_pOpenButton->getImageTall()*2) / 2*/);
		m_pOpenButton->setVisible(true);

	}

	virtual void   setVisible(bool state)
	{
		m_pOpenButton->setVisible(state);
		ColorButton::setVisible(state);
	}


};

//==============================================================================
// Button with image instead of text

class CImageButton : public ColorButton
{
private:
	CImageLabel* m_pOpenButton;

public:

	CImageButton(const char* text, int x, int y, int wide, int tall, bool bNoHighlight, bool bFlat) :
		ColorButton(" ", x, y, wide, tall, bNoHighlight, bFlat)
	{
		m_pOpenButton = new CImageLabel(text, 1, 1, wide - 2, tall - 2);
		m_pOpenButton->setParent(this);

		// Reposition
	//	m_pOpenButton->setPos( x+1,y+1 );
	//	m_pOpenButton->setSize(wide-2,tall-2);

		m_pOpenButton->setVisible(true);
	}

	virtual void   setVisible(bool state)
	{
		m_pOpenButton->setVisible(state);
		ColorButton::setVisible(state);
	}


};

//==============================================================================
class TeamFortressViewport : public Panel
{
private:
	vgui::Cursor* _cursorNone;
	vgui::Cursor* _cursorArrow;

	int			 m_iInitialized;

	CCommandMenu* m_pCommandMenus[MAX_MENUS];
	CCommandMenu* m_pCurrentCommandMenu;
	float		 m_flMenuOpenTime;
	float		 m_flScoreBoardLastUpdated;
	float		 m_flSpectatorPanelLastUpdated;
	int			 m_iNumMenus;
	int			 m_iCurrentTeamNumber;
	int			 m_iCurrentPlayerClass;
	int			 m_iUser1;
	int			 m_iUser2;
	int			 m_iUser3;

	// VGUI Menus
	/**
	*	@brief Spectator "Menu" explaining the Spectator buttons
	*/
	void		 CreateSpectatorMenu();

	// Scheme handler
	CSchemeManager m_SchemeManager;

	// MOTD
	int		m_iGotAllMOTD;
	char	m_szMOTD[MAX_MOTD_LENGTH];

	int					m_iAllowSpectators;

	// Data for specific sections of the Command Menu
	int			m_iNumberOfTeams;
	char		m_sTeamNames[5][MAX_TEAMNAME_SIZE];

	// Localisation strings
	char		m_sMapName[64];

	// helper function to update the player menu entries
	void UpdatePlayerMenu(int menuIndex);

public:
	TeamFortressViewport(int x, int y, int wide, int tall);

	/**
	*	@brief Called everytime a new level is started. Viewport clears out it's data.
	*/
	void Initialize();

	/**
	*	@brief Read the Command Menu structure from the txt file and create the menu.
	*	@return Index of menu in m_pCommandMenus
	*/
	int		CreateCommandMenu(const char* menuFile, int direction, int yOffset, bool flatDesign, float flButtonSizeX, float flButtonSizeY, int xOffset);
	void	CreateScoreBoard();

	void UpdateCursorState();
	void UpdateCommandMenu(int menuIndex);

	/**
	*	@brief We've got an update on player info
	*	Recalculate any menus that use it.
	*/
	void UpdateOnPlayerInfo();
	void UpdateHighlights();
	void UpdateSpectatorPanel();

	/**
	*	@brief Direct Key Input
	*/
	bool KeyInput(int down, int keynum, const char* pszCurrentBinding);

	/**
	*	@brief Activate's the player special ability
	*	called when the player hits their "special" key
	*/
	void InputPlayerSpecial();
	void GetAllPlayersInfo();
	void DeathMsg(int killer, int victim);

	void ShowCommandMenu(int menuIndex);

	/**
	*	@brief Handles the key input of "-commandmenu"
	*/
	void InputSignalHideCommandMenu();
	void HideCommandMenu();
	void SetCurrentCommandMenu(CCommandMenu* pNewMenu);
	void SetCurrentMenu(CMenuPanel* pMenu);

	void ShowScoreBoard();
	void HideScoreBoard();
	bool IsScoreBoardVisible();

	/**
	*	@brief Return true if the HUD's allowed to print text messages
	*/
	bool AllowedToPrintText();

	void ShowVGUIMenu(int iMenu);

	/**
	*	@brief Removes all VGUI Menu's onscreen
	*/
	void HideVGUIMenu();

	/**
	*	@brief Remove the top VGUI menu, and bring up the next one
	*/
	void HideTopMenu();

	CMenuPanel* CreateTextWindow(int iTextToShow);

	CCommandMenu* CreateSubMenu(CommandButton* pButton, CCommandMenu* pParentMenu, int iYOffset, int iXOffset = 0);

	// Data Handlers
	int GetNumberOfTeams() { return m_iNumberOfTeams; }
	char* GetTeamName(int iTeam) { return m_sTeamNames[iTeam]; }
	int GetAllowSpectators() { return m_iAllowSpectators; }

	// Message Handlers
	bool MsgFunc_TeamNames(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_VGUIMenu(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_MOTD(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_ServerName(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_ScoreInfo(const char* pszName, int iSize, void* pbuf);

	/**
	*	@brief Message handler for TeamScore message
	*	@details accepts three values:
	*		string: team name
	*		short: teams kills
	*		short: teams deaths
	*	if this message is never received, then scores will simply be the combined totals of the players.
	*/
	bool MsgFunc_TeamScore(const char* pszName, int iSize, void* pbuf);

	/**
	*	@brief Message handler for TeamInfo message
	*	@details accepts two values:
	*		byte: client number
	*		string: client team name
	*/
	bool MsgFunc_TeamInfo(const char* pszName, int iSize, void* pbuf);

	// Input
	/**
	*	@brief Number Key Input
	*/
	bool SlotInput(int iSlot);

	virtual void paintBackground();

	CSchemeManager* GetSchemeManager() { return &m_SchemeManager; }
	ScorePanel* GetScoreBoard() { return m_pScoreBoard; }

	/**
	*	@brief Makes sure the memory allocated for TeamFortressViewport is nulled out
	*/
	void* operator new(size_t stAllocateBlock);

public:
	// VGUI Menus
	CMenuPanel* m_pCurrentMenu;
	int						m_StandardMenu;	// indexs in m_pCommandMenus
	int						m_SpectatorOptionsMenu;
	int						m_SpectatorCameraMenu;
	int						m_PlayerMenu; // a list of current player
	ScorePanel* m_pScoreBoard;
	SpectatorPanel* m_pSpectatorPanel;
	char			m_szServerName[MAX_SERVERNAME_LENGTH];
};

//============================================================
// Command Menu Button Handlers
constexpr int MAX_COMMAND_SIZE = 256;

class CMenuHandler_StringCommand : public ActionSignal
{
protected:
	char	m_pszCommand[MAX_COMMAND_SIZE];
	int		m_iCloseVGUIMenu;
public:
	CMenuHandler_StringCommand(const char* pszCommand)
	{
		safe_strcpy(m_pszCommand, pszCommand);
		m_iCloseVGUIMenu = false;
	}

	CMenuHandler_StringCommand(const char* pszCommand, int iClose)
	{
		safe_strcpy(m_pszCommand, pszCommand);
		m_iCloseVGUIMenu = true;
	}

	virtual void actionPerformed(Panel* panel)
	{
		gEngfuncs.pfnClientCmd(m_pszCommand);

		if (m_iCloseVGUIMenu)
			gViewPort->HideTopMenu();
		else
			gViewPort->HideCommandMenu();
	}
};

// This works the same as CMenuHandler_StringCommand, except it watches the string command 
// for specific commands, and modifies client vars based upon them.
class CMenuHandler_StringCommandWatch : public CMenuHandler_StringCommand
{
private:
public:
	CMenuHandler_StringCommandWatch(const char* pszCommand) : CMenuHandler_StringCommand(pszCommand)
	{
	}

	CMenuHandler_StringCommandWatch(const char* pszCommand, int iClose) : CMenuHandler_StringCommand(pszCommand, iClose)
	{
	}

	virtual void actionPerformed(Panel* panel)
	{
		CMenuHandler_StringCommand::actionPerformed(panel);

		// Try to guess the player's new team (it'll be corrected if it's wrong)
		if (!strcmp(m_pszCommand, "jointeam 1"))
			g_iTeamNumber = 1;
		else if (!strcmp(m_pszCommand, "jointeam 2"))
			g_iTeamNumber = 2;
		else if (!strcmp(m_pszCommand, "jointeam 3"))
			g_iTeamNumber = 3;
		else if (!strcmp(m_pszCommand, "jointeam 4"))
			g_iTeamNumber = 4;
	}
};

// Used instead of CMenuHandler_StringCommand for Class Selection buttons.
// Checks the state of hud_classautokill and kills the player if set
class CMenuHandler_StringCommandClassSelect : public CMenuHandler_StringCommand
{
private:
public:
	CMenuHandler_StringCommandClassSelect(const char* pszCommand) : CMenuHandler_StringCommand(pszCommand)
	{
	}

	CMenuHandler_StringCommandClassSelect(const char* pszCommand, int iClose) : CMenuHandler_StringCommand(pszCommand, iClose)
	{
	}

	virtual void actionPerformed(Panel* panel);
};

class CMenuHandler_PopupSubMenuInput : public InputSignal
{
private:
	CCommandMenu* m_pSubMenu;
	Button* m_pButton;
public:
	CMenuHandler_PopupSubMenuInput(Button* pButton, CCommandMenu* pSubMenu)
	{
		m_pSubMenu = pSubMenu;
		m_pButton = pButton;
	}

	virtual void cursorMoved(int x, int y, Panel* panel)
	{
		//gViewPort->SetCurrentCommandMenu( m_pSubMenu );
	}

	virtual void cursorEntered(Panel* panel)
	{
		gViewPort->SetCurrentCommandMenu(m_pSubMenu);

		if (m_pButton)
			m_pButton->setArmed(true);
	};
	virtual void cursorExited(Panel* Panel) {}
	virtual void mousePressed(MouseCode code, Panel* panel) {}
	virtual void mouseDoublePressed(MouseCode code, Panel* panel) {}
	virtual void mouseReleased(MouseCode code, Panel* panel) {}
	virtual void mouseWheeled(int delta, Panel* panel) {}
	virtual void keyPressed(KeyCode code, Panel* panel) {}
	virtual void keyTyped(KeyCode code, Panel* panel) {}
	virtual void keyReleased(KeyCode code, Panel* panel) {}
	virtual void keyFocusTicked(Panel* panel) {}
};

class CMenuHandler_LabelInput : public InputSignal
{
private:
	ActionSignal* m_pActionSignal;
public:
	CMenuHandler_LabelInput(ActionSignal* pSignal)
	{
		m_pActionSignal = pSignal;
	}

	virtual void mousePressed(MouseCode code, Panel* panel)
	{
		m_pActionSignal->actionPerformed(panel);
	}

	virtual void mouseReleased(MouseCode code, Panel* panel) {}
	virtual void cursorEntered(Panel* panel) {}
	virtual void cursorExited(Panel* Panel) {}
	virtual void cursorMoved(int x, int y, Panel* panel) {}
	virtual void mouseDoublePressed(MouseCode code, Panel* panel) {}
	virtual void mouseWheeled(int delta, Panel* panel) {}
	virtual void keyPressed(KeyCode code, Panel* panel) {}
	virtual void keyTyped(KeyCode code, Panel* panel) {}
	virtual void keyReleased(KeyCode code, Panel* panel) {}
	virtual void keyFocusTicked(Panel* panel) {}
};

constexpr int HIDE_TEXTWINDOW = 0;
constexpr int SHOW_MAPBRIEFING = 1;
constexpr int SHOW_CLASSDESC = 2;
constexpr int SHOW_MOTD = 3;
constexpr int SHOW_SPECHELP = 4;

class CMenuHandler_TextWindow : public ActionSignal
{
private:
	int	m_iState;
public:
	CMenuHandler_TextWindow(int iState)
	{
		m_iState = iState;
	}

	virtual void actionPerformed(Panel* panel)
	{
		if (m_iState == HIDE_TEXTWINDOW)
		{
			gViewPort->HideTopMenu();
		}
		else
		{
			gViewPort->HideCommandMenu();
			gViewPort->ShowVGUIMenu(m_iState);
		}
	}
};

class CMenuHandler_ToggleCvar : public ActionSignal
{
private:
	cvar_t* m_cvar;

public:
	CMenuHandler_ToggleCvar(char* cvarname)
	{
		m_cvar = gEngfuncs.pfnGetCvarPointer(cvarname);
	}

	virtual void actionPerformed(Panel* panel)
	{
		if (m_cvar->value)
			m_cvar->value = 0.0f;
		else
			m_cvar->value = 1.0f;

		// hide the menu 
		gViewPort->HideCommandMenu();

		gViewPort->UpdateSpectatorPanel();
	}


};



class CMenuHandler_SpectateFollow : public ActionSignal
{
protected:
	char	m_szplayer[MAX_COMMAND_SIZE];
public:
	CMenuHandler_SpectateFollow(char* player)
	{
		safe_strcpy(m_szplayer, player);
	}

	virtual void actionPerformed(Panel* panel)
	{
		gHUD.m_Spectator.FindPlayer(m_szplayer);
		gViewPort->HideCommandMenu();
	}
};



class CDragNDropHandler : public InputSignal
{
private:
	DragNDropPanel* m_pPanel;
	bool			m_bDragging = false;
	int				m_iaDragOrgPos[2]{};
	int				m_iaDragStart[2]{};

public:
	CDragNDropHandler(DragNDropPanel* pPanel)
	{
		m_pPanel = pPanel;
	}

	void cursorMoved(int x, int y, Panel* panel);
	void mousePressed(MouseCode code, Panel* panel);
	void mouseReleased(MouseCode code, Panel* panel);

	void mouseDoublePressed(MouseCode code, Panel* panel) {}
	void cursorEntered(Panel* panel) {}
	void cursorExited(Panel* panel) {}
	void mouseWheeled(int delta, Panel* panel) {}
	void keyPressed(KeyCode code, Panel* panel) {}
	void keyTyped(KeyCode code, Panel* panel) {}
	void keyReleased(KeyCode code, Panel* panel) {}
	void keyFocusTicked(Panel* panel) {}
};

class CHandler_MenuButtonOver : public InputSignal
{
private:
	int			m_iButton;
	CMenuPanel* m_pMenuPanel;
public:
	CHandler_MenuButtonOver(CMenuPanel* pPanel, int iButton)
	{
		m_iButton = iButton;
		m_pMenuPanel = pPanel;
	}

	void cursorEntered(Panel* panel);

	void cursorMoved(int x, int y, Panel* panel) {}
	void mousePressed(MouseCode code, Panel* panel) {}
	void mouseReleased(MouseCode code, Panel* panel) {}
	void mouseDoublePressed(MouseCode code, Panel* panel) {}
	void cursorExited(Panel* panel) {}
	void mouseWheeled(int delta, Panel* panel) {}
	void keyPressed(KeyCode code, Panel* panel) {}
	void keyTyped(KeyCode code, Panel* panel) {}
	void keyReleased(KeyCode code, Panel* panel) {}
	void keyFocusTicked(Panel* panel) {}
};

class CHandler_ButtonHighlight : public InputSignal
{
private:
	Button* m_pButton;
public:
	CHandler_ButtonHighlight(Button* pButton)
	{
		m_pButton = pButton;
	}

	virtual void cursorEntered(Panel* panel)
	{
		m_pButton->setArmed(true);
	};
	virtual void cursorExited(Panel* Panel)
	{
		m_pButton->setArmed(false);
	};
	virtual void mousePressed(MouseCode code, Panel* panel) {}
	virtual void mouseReleased(MouseCode code, Panel* panel) {}
	virtual void cursorMoved(int x, int y, Panel* panel) {}
	virtual void mouseDoublePressed(MouseCode code, Panel* panel) {}
	virtual void mouseWheeled(int delta, Panel* panel) {}
	virtual void keyPressed(KeyCode code, Panel* panel) {}
	virtual void keyTyped(KeyCode code, Panel* panel) {}
	virtual void keyReleased(KeyCode code, Panel* panel) {}
	virtual void keyFocusTicked(Panel* panel) {}
};

//-----------------------------------------------------------------------------
// Purpose: Special handler for highlighting of command menu buttons
//-----------------------------------------------------------------------------
class CHandler_CommandButtonHighlight : public CHandler_ButtonHighlight
{
private:
	CommandButton* m_pCommandButton;
public:
	CHandler_CommandButtonHighlight(CommandButton* pButton) : CHandler_ButtonHighlight(pButton)
	{
		m_pCommandButton = pButton;
	}

	virtual void cursorEntered(Panel* panel)
	{
		m_pCommandButton->cursorEntered();
	}

	virtual void cursorExited(Panel* panel)
	{
		m_pCommandButton->cursorExited();
	}
};


//================================================================
// Overidden Command Buttons for special visibilities
class SpectateButton : public CommandButton
{
public:
	SpectateButton(const char* text, int x, int y, int wide, int tall, bool bNoHighlight) : CommandButton(text, x, y, wide, tall, bNoHighlight)
	{
	}

	virtual int IsNotValid()
	{
		// Only visible if the server allows it
		if (gViewPort->GetAllowSpectators() != 0)
			return false;

		return true;
	}
};

constexpr int MAX_MAPNAME = 256;

class MapButton : public CommandButton
{
private:
	char m_szMapName[MAX_MAPNAME];

public:
	MapButton(const char* pMapName, const char* text, int x, int y, int wide, int tall) : CommandButton(text, x, y, wide, tall)
	{
		snprintf(m_szMapName, sizeof(m_szMapName), "maps/%s.bsp", pMapName);
	}

	virtual int IsNotValid()
	{
		const char* level = gEngfuncs.pfnGetLevelName();
		if (!level)
			return true;

		// Does it match the current map name?
		if (strcmp(m_szMapName, level))
			return true;

		return false;
	}
};

//-----------------------------------------------------------------------------
// Purpose: CommandButton which is only displayed if the player is on team X
//-----------------------------------------------------------------------------
class ToggleCommandButton : public CommandButton, public InputSignal
{
private:
	cvar_t* m_cvar;
	CImageLabel* pLabelOn;
	CImageLabel* pLabelOff;


public:
	ToggleCommandButton(const char* cvarname, const char* text, int x, int y, int wide, int tall, bool flat) :
		CommandButton(text, x, y, wide, tall, false, flat)
	{
		m_cvar = gEngfuncs.pfnGetCvarPointer(cvarname);

		// Put a > to show it's a submenu
		pLabelOn = new CImageLabel("checked", 0, 0);
		pLabelOn->setParent(this);
		pLabelOn->addInputSignal(this);

		pLabelOff = new CImageLabel("unchecked", 0, 0);
		pLabelOff->setParent(this);
		pLabelOff->setEnabled(true);
		pLabelOff->addInputSignal(this);

		int textwide, texttall;
		getTextSize(textwide, texttall);

		// Reposition
		pLabelOn->setPos(textwide, (tall - pLabelOn->getTall()) / 2);

		pLabelOff->setPos(textwide, (tall - pLabelOff->getTall()) / 2);

		// Set text color to orange
		setFgColor(Scheme::sc_primary1);
	}

	virtual void cursorEntered(Panel* panel)
	{
		CommandButton::cursorEntered();
	}

	virtual void cursorExited(Panel* panel)
	{
		CommandButton::cursorExited();
	}

	virtual void mousePressed(MouseCode code, Panel* panel)
	{
		doClick();
	};

	virtual void cursorMoved(int x, int y, Panel* panel) {};

	virtual void mouseDoublePressed(MouseCode code, Panel* panel) {}
	virtual void mouseReleased(MouseCode code, Panel* panel) {}
	virtual void mouseWheeled(int delta, Panel* panel) {}
	virtual void keyPressed(KeyCode code, Panel* panel) {}
	virtual void keyTyped(KeyCode code, Panel* panel) {}
	virtual void keyReleased(KeyCode code, Panel* panel) {}
	virtual void keyFocusTicked(Panel* panel) {}

	virtual void paint()
	{
		if (!m_cvar)
		{
			pLabelOff->setVisible(false);
			pLabelOn->setVisible(false);
		}
		else if (m_cvar->value)
		{
			pLabelOff->setVisible(false);
			pLabelOn->setVisible(true);
		}
		else
		{
			pLabelOff->setVisible(true);
			pLabelOn->setVisible(false);
		}

		CommandButton::paint();

	}
};
class SpectToggleButton : public CommandButton, public InputSignal
{
private:
	cvar_t* m_cvar;
	CImageLabel* pLabelOn;

public:
	SpectToggleButton(const char* cvarname, const char* text, int x, int y, int wide, int tall, bool flat) :
		CommandButton(text, x, y, wide, tall, false, flat)
	{
		m_cvar = gEngfuncs.pfnGetCvarPointer(cvarname);

		// Put a > to show it's a submenu
		pLabelOn = new CImageLabel("checked", 0, 0);
		pLabelOn->setParent(this);
		pLabelOn->addInputSignal(this);


		int textwide, texttall;
		getTextSize(textwide, texttall);

		// Reposition
		pLabelOn->setPos(textwide, (tall - pLabelOn->getTall()) / 2);
	}

	virtual void cursorEntered(Panel* panel)
	{
		CommandButton::cursorEntered();
	}

	virtual void cursorExited(Panel* panel)
	{
		CommandButton::cursorExited();
	}

	virtual void mousePressed(MouseCode code, Panel* panel)
	{
		doClick();
	};

	virtual void cursorMoved(int x, int y, Panel* panel) {};

	virtual void mouseDoublePressed(MouseCode code, Panel* panel) {}
	virtual void mouseReleased(MouseCode code, Panel* panel) {}
	virtual void mouseWheeled(int delta, Panel* panel) {}
	virtual void keyPressed(KeyCode code, Panel* panel) {}
	virtual void keyTyped(KeyCode code, Panel* panel) {}
	virtual void keyReleased(KeyCode code, Panel* panel) {}
	virtual void keyFocusTicked(Panel* panel) {}

	virtual void paintBackground()
	{
		if (isArmed())
		{
			drawSetColor(143, 143, 54, 125);
			drawFilledRect(5, 0, _size[0] - 5, _size[1]);
		}
	}

	virtual void paint()
	{
		if (isArmed())
		{
			setFgColor(194, 202, 54, 0);
		}
		else
		{
			setFgColor(143, 143, 54, 15);
		}

		if (!m_cvar)
		{
			pLabelOn->setVisible(false);
		}
		else if (m_cvar->value)
		{
			pLabelOn->setVisible(true);
		}
		else
		{
			pLabelOn->setVisible(false);
		}

		Button::paint();
	}
};

//============================================================
// Panel that can be dragged around
class DragNDropPanel : public Panel
{
private:
	bool		m_bBeingDragged;
	LineBorder* m_pBorder;
public:
	DragNDropPanel(int x, int y, int wide, int tall) : Panel(x, y, wide, tall)
	{
		m_bBeingDragged = false;

		// Create the Drag Handler
		addInputSignal(new CDragNDropHandler(this));

		// Create the border (for dragging)
		m_pBorder = new LineBorder();
	}

	virtual void setDragged(bool bState)
	{
		m_bBeingDragged = bState;

		if (m_bBeingDragged)
			setBorder(m_pBorder);
		else
			setBorder(nullptr);
	}
};

//================================================================
// Panel that draws itself with a transparent black background
class CTransparentPanel : public Panel
{
private:
	int	m_iTransparency;
public:
	CTransparentPanel(int iTrans, int x, int y, int wide, int tall) : Panel(x, y, wide, tall)
	{
		m_iTransparency = iTrans;
	}

	virtual void paintBackground()
	{
		if (m_iTransparency)
		{
			// Transparent black background
			drawSetColor(0, 0, 0, m_iTransparency);
			drawFilledRect(0, 0, _size[0], _size[1]);
		}
	}
};

//================================================================
// Menu Panel that supports buffering of menus
class CMenuPanel : public CTransparentPanel
{
private:
	CMenuPanel* m_pNextMenu;
	int			m_iMenuID;
	int			m_iRemoveMe;
	int			m_iIsActive;
	float		m_flOpenTime;
public:
	CMenuPanel(int iRemoveMe, int x, int y, int wide, int tall) : CTransparentPanel(100, x, y, wide, tall)
	{
		Reset();
		m_iRemoveMe = iRemoveMe;
	}

	CMenuPanel(int iTrans, int iRemoveMe, int x, int y, int wide, int tall) : CTransparentPanel(iTrans, x, y, wide, tall)
	{
		Reset();
		m_iRemoveMe = iRemoveMe;
	}

	virtual void Reset()
	{
		m_pNextMenu = nullptr;
		m_iIsActive = false;
		m_flOpenTime = 0;
	}

	void SetNextMenu(CMenuPanel* pNextPanel)
	{
		if (m_pNextMenu)
			m_pNextMenu->SetNextMenu(pNextPanel);
		else
			m_pNextMenu = pNextPanel;
	}

	void SetMenuID(int iID)
	{
		m_iMenuID = iID;
	}

	void SetActive(int iState)
	{
		m_iIsActive = iState;
	}

	virtual void Open()
	{
		setVisible(true);

		// Note the open time, so we can delay input for a bit
		m_flOpenTime = gHUD.m_flTime;
	}

	virtual void Close()
	{
		setVisible(false);
		m_iIsActive = false;

		if (m_iRemoveMe)
			gViewPort->removeChild(this);

		// This MenuPanel has now been deleted. Don't append code here.
	}

	int			ShouldBeRemoved() { return m_iRemoveMe; }
	CMenuPanel* GetNextMenu() { return m_pNextMenu; }
	int			GetMenuID() { return m_iMenuID; }
	int			IsActive() { return m_iIsActive; }
	float		GetOpenTime() { return m_flOpenTime; }

	// Numeric input
	virtual bool SlotInput(int iSlot) { return false; }
	virtual void SetActiveInfo(int iInput) {}
};

//================================================================
// Custom drawn scroll bars
class CTFScrollButton : public CommandButton
{
private:
	BitmapTGA* m_pTGA;

public:
	CTFScrollButton(int iArrow, const char* text, int x, int y, int wide, int tall);

	virtual void paint();
	virtual void paintBackground();
};

// Custom drawn slider bar
class CTFSlider : public Slider
{
public:
	CTFSlider(int x, int y, int wide, int tall, bool vertical) : Slider(x, y, wide, tall, vertical)
	{
	}

	virtual void paintBackground();
};

// Custom drawn scrollpanel
class CTFScrollPanel : public ScrollPanel
{
public:
	CTFScrollPanel(int x, int y, int wide, int tall);
};