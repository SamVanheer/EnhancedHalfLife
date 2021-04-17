#pragma once

#include <VGUI_Panel.h>
#include <VGUI_Label.h>
#include <VGUI_Button.h>

using namespace vgui;

constexpr int SPECTATOR_PANEL_CMD_NONE = 0;

constexpr int SPECTATOR_PANEL_CMD_OPTIONS = 1;
constexpr int SPECTATOR_PANEL_CMD_PREVPLAYER = 2;
constexpr int SPECTATOR_PANEL_CMD_NEXTPLAYER = 3;
constexpr int SPECTATOR_PANEL_CMD_HIDEMENU = 4;
constexpr int SPECTATOR_PANEL_CMD_TOGGLE_INSET = 5;
constexpr int SPECTATOR_PANEL_CMD_CAMERA = 6;
constexpr int SPECTATOR_PANEL_CMD_PLAYERS = 7;

// spectator panel sizes
constexpr int PANEL_HEIGHT = 64;

constexpr int BANNER_WIDTH = 256;
constexpr int BANNER_HEIGHT = 64;

constexpr int OPTIONS_BUTTON_X = 96;
constexpr int CAMOPTIONS_BUTTON_X = 200;


constexpr int SEPERATOR_WIDTH = 15;
constexpr int SEPERATOR_HEIGHT = 15;


constexpr int TEAM_NUMBER = 2;

class SpectatorPanel : public Panel //, public vgui::CDefaultInputSignal
{

public:
	SpectatorPanel(int x, int y, int wide, int tall);
	virtual ~SpectatorPanel();

	void			ActionSignal(int cmd);

	// InputSignal overrides.
public:
	void Initialize();
	void Update();



public:

	void EnableInsetView(bool isEnabled);
	void ShowMenu(bool isVisible);

	DropDownButton* m_OptionButton;
	//	CommandButton     *	m_HideButton;
		//ColorButton	  *	m_PrevPlayerButton;
		//ColorButton	  *	m_NextPlayerButton;
	CImageButton* m_PrevPlayerButton;
	CImageButton* m_NextPlayerButton;
	DropDownButton* m_CamButton;

	CTransparentPanel* m_TopBorder;
	CTransparentPanel* m_BottomBorder;

	ColorButton* m_InsetViewButton;

	DropDownButton* m_BottomMainButton;
	CImageLabel* m_TimerImage;
	Label* m_BottomMainLabel;
	Label* m_CurrentTime;
	Label* m_ExtraInfo;
	Panel* m_Separator;

	Label* m_TeamScores[TEAM_NUMBER];

	CImageLabel* m_TopBanner;

	bool			m_menuVisible;
	bool			m_insetVisible;
};



class CSpectatorHandler_Command : public ActionSignal
{

private:
	SpectatorPanel* m_pFather;
	int				 m_cmd;

public:
	CSpectatorHandler_Command(SpectatorPanel* panel, int cmd)
	{
		m_pFather = panel;
		m_cmd = cmd;
	}

	void actionPerformed(Panel* panel) override
	{
		m_pFather->ActionSignal(m_cmd);
	}
};
