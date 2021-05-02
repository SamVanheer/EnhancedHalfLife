//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: Contains implementation of various VGUI-derived objects
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================

#include "VGUI_Font.h"

#include "camera.h"
#include "kbutton.h"
#include "cvardef.h"
#include "usercmd.h"
#include "camera.h"

#include "vgui_loadtga.h"

// Arrow filenames
const char* sArrowFilenames[] =
{
	"arrowup",
	"arrowdn",
	"arrowlt",
	"arrowrt",
};

/**
*	@brief Get the name of TGA file, without a gamedir
*/
char* GetTGANameForRes(const char* pszName)
{
	int i;
	char sz[256];
	static char gd[sizeof(sz) * 2];
	if (ScreenWidth < 640)
		i = 320;
	else
		i = 640;
	snprintf(sz, sizeof(sz), pszName, i);
	snprintf(gd, sizeof(gd), "gfx/vgui/%s.tga", sz);
	return gd;
}

BitmapTGA* LoadTGAForRes(const char* pImageName)
{
	BitmapTGA* pTGA;

	char sz[256];
	snprintf(sz, sizeof(sz), "%%d_%s", pImageName);
	pTGA = vgui_LoadTGA(GetTGANameForRes(sz));

	return pTGA;
}

CommandButton::CommandButton(const char* text, int x, int y, int wide, int tall, bool bNoHighlight) : Button("", x, y, wide, tall)
{
	m_iPlayerClass = 0;
	m_bNoHighlight = bNoHighlight;
	m_bFlat = false;
	Init();
	setText(text);
}

CommandButton::CommandButton(int iPlayerClass, const char* text, int x, int y, int wide, int tall, bool bFlat) : Button("", x, y, wide, tall)
{
	m_iPlayerClass = iPlayerClass;
	m_bNoHighlight = false;
	m_bFlat = bFlat;
	Init();
	setText(text);
}

CommandButton::CommandButton(const char* text, int x, int y, int wide, int tall, bool bNoHighlight, bool bFlat) : Button("", x, y, wide, tall)
{
	m_iPlayerClass = 0;
	m_bFlat = bFlat;
	m_bNoHighlight = bNoHighlight;
	Init();
	setText(text);
}

void CommandButton::Init()
{
	m_pSubMenu = nullptr;
	m_pSubLabel = nullptr;
	m_pParentMenu = nullptr;

	// Set text color to orange
	setFgColor(Scheme::sc_primary1);

	// left align
	setContentAlignment(vgui::Label::a_west);

	// Add the Highlight signal
	if (!m_bNoHighlight)
		addInputSignal(new CHandler_CommandButtonHighlight(this));

	// not bound to any button yet
	m_cBoundKey = 0;
}

void CommandButton::RecalculateText()
{
	char szBuf[128];

	if (m_cBoundKey != 0)
	{
		if (m_cBoundKey == (char)255)
		{
			safe_strcpy(szBuf, m_sMainText);
		}
		else
		{
			snprintf(szBuf, sizeof(szBuf), "  %c  %s", m_cBoundKey, m_sMainText);
		}
		szBuf[MAX_BUTTON_SIZE - 1] = 0;
	}
	else
	{
		// just draw a space if no key bound
		snprintf(szBuf, sizeof(szBuf), "     %s", m_sMainText);
		szBuf[MAX_BUTTON_SIZE - 1] = 0;
	}

	Button::setText(szBuf);
}

void CommandButton::setText(const char* text)
{
	safe_strcpy(m_sMainText, text);

	RecalculateText();
}

void CommandButton::setBoundKey(char boundKey)
{
	m_cBoundKey = boundKey;
	RecalculateText();
}

char CommandButton::getBoundKey()
{
	return m_cBoundKey;
}

void CommandButton::AddSubMenu(CCommandMenu* pNewMenu)
{
	m_pSubMenu = pNewMenu;

	// Prevent this button from being pushed
	setMouseClickEnabled(MOUSE_LEFT, false);
}

void CommandButton::UpdateSubMenus(int iAdjustment)
{
	if (m_pSubMenu)
		m_pSubMenu->RecalculatePositions(iAdjustment);
}

void CommandButton::paint()
{
	// Make the sub label paint the same as the button
	if (m_pSubLabel)
	{
		if (isSelected())
			m_pSubLabel->PushDown();
		else
			m_pSubLabel->PushUp();
	}

	// draw armed button text in white
	if (isArmed())
	{
		setFgColor(Scheme::sc_secondary2);
	}
	else
	{
		setFgColor(Scheme::sc_primary1);
	}

	Button::paint();
}

void CommandButton::paintBackground()
{
	if (m_bFlat)
	{
		if (isArmed())
		{
			// Orange Border
			drawSetColor(Scheme::sc_secondary1);
			drawOutlinedRect(0, 0, _size[0], _size[1]);
		}
	}
	else
	{
		if (isArmed())
		{
			// Orange highlight background
			drawSetColor(Scheme::sc_primary2);
			drawFilledRect(0, 0, _size[0], _size[1]);
		}

		// Orange Border
		drawSetColor(Scheme::sc_secondary1);
		drawOutlinedRect(0, 0, _size[0], _size[1]);
	}
}

void CommandButton::cursorEntered()
{
	// unarm all the other buttons in this menu
	CCommandMenu* containingMenu = getParentMenu();
	if (containingMenu)
	{
		containingMenu->ClearButtonsOfArmedState();

		// make all our higher buttons armed
		CCommandMenu* pCParent = containingMenu->GetParentMenu();
		if (pCParent)
		{
			CommandButton* pParentButton = pCParent->FindButtonWithSubmenu(containingMenu);

			pParentButton->cursorEntered();
		}
	}

	// arm ourselves
	setArmed(true);
}

void CommandButton::cursorExited()
{
	// only clear ourselves if we have do not have a containing menu
	// only stay armed if we have a sub menu
	// the buttons only unarm themselves when another button is armed instead
	if (!getParentMenu() || !GetSubMenu())
	{
		setArmed(false);
	}
}

CCommandMenu* CommandButton::getParentMenu()
{
	return m_pParentMenu;
}

void CommandButton::setParentMenu(CCommandMenu* pParentMenu)
{
	m_pParentMenu = pParentMenu;
}

CImageLabel::CImageLabel(const char* pImageName, int x, int y) : Label("", x, y)
{
	setContentFitted(true);
	m_pTGA = LoadTGAForRes(pImageName);
	setImage(m_pTGA);
}

CImageLabel::CImageLabel(const char* pImageName, int x, int y, int wide, int tall) : Label("", x, y, wide, tall)
{
	setContentFitted(true);
	m_pTGA = LoadTGAForRes(pImageName);
	setImage(m_pTGA);
}

int CImageLabel::getImageWide()
{
	if (m_pTGA)
	{
		int iXSize, iYSize;
		m_pTGA->getSize(iXSize, iYSize);
		return iXSize;
	}
	else
	{
		return 1;
	}
}

int CImageLabel::getImageTall()
{
	if (m_pTGA)
	{
		int iXSize, iYSize;
		m_pTGA->getSize(iXSize, iYSize);
		return iYSize;
	}
	else
	{
		return 1;
	}
}

void CImageLabel::LoadImage(const char* pImageName)
{
	if (m_pTGA)
		delete m_pTGA;

	// Load the Image
	m_pTGA = LoadTGAForRes(pImageName);

	if (m_pTGA == nullptr)
	{
		// we didn't find a matching image file for this resolution
		// try to load file resolution independent
		m_pTGA = vgui_LoadTGA(pImageName);
	}

	if (m_pTGA == nullptr)
		return;	// unable to load image

	int w, t;

	m_pTGA->getSize(w, t);

	setSize(XRES(w), YRES(t));
	setImage(m_pTGA);
}

void CCommandMenu::paintBackground()
{
	// Transparent black background

	if (m_iSpectCmdMenu)
		drawSetColor(0, 0, 0, 64);
	else
		drawSetColor(Scheme::sc_primary3);

	drawFilledRect(0, 0, _size[0], _size[1]);
}

CTFScrollButton::CTFScrollButton(int iArrow, const char* text, int x, int y, int wide, int tall) : CommandButton(text, x, y, wide, tall)
{
	// Set text color to orange
	setFgColor(Scheme::sc_primary1);

	// Load in the arrow
	m_pTGA = LoadTGAForRes(sArrowFilenames[iArrow]);
	setImage(m_pTGA);

	// Highlight signal
	InputSignal* pISignal = new CHandler_CommandButtonHighlight(this);
	addInputSignal(pISignal);
}

void CTFScrollButton::paint()
{
	if (!m_pTGA)
		return;

	// draw armed button text in white
	if (isArmed())
	{
		m_pTGA->setColor(Color(255, 255, 255, 0));
	}
	else
	{
		m_pTGA->setColor(Color(255, 255, 255, 128));
	}

	m_pTGA->doPaint(this);
}

void CTFScrollButton::paintBackground()
{
	/*
		if ( isArmed() )
		{
			// Orange highlight background
			drawSetColor( Scheme::sc_primary2 );
			drawFilledRect(0,0,_size[0],_size[1]);
		}

		// Orange Border
		drawSetColor( Scheme::sc_secondary1 );
		drawOutlinedRect(0,0,_size[0]-1,_size[1]);
	*/
}

void CTFSlider::paintBackground()
{
	int wide, tall, nobx, noby;
	getPaintSize(wide, tall);
	getNobPos(nobx, noby);

	// Border
	drawSetColor(Scheme::sc_secondary1);
	drawOutlinedRect(0, 0, wide, tall);

	if (isVertical())
	{
		// Nob Fill
		drawSetColor(Scheme::sc_primary2);
		drawFilledRect(0, nobx, wide, noby);

		// Nob Outline
		drawSetColor(Scheme::sc_primary1);
		drawOutlinedRect(0, nobx, wide, noby);
	}
	else
	{
		// Nob Fill
		drawSetColor(Scheme::sc_primary2);
		drawFilledRect(nobx, 0, noby, tall);

		// Nob Outline
		drawSetColor(Scheme::sc_primary1);
		drawOutlinedRect(nobx, 0, noby, tall);
	}
}

CTFScrollPanel::CTFScrollPanel(int x, int y, int wide, int tall) : ScrollPanel(x, y, wide, tall)
{
	ScrollBar* pScrollBar = getVerticalScrollBar();
	pScrollBar->setButton(new CTFScrollButton(ARROW_UP, "", 0, 0, 16, 16), 0);
	pScrollBar->setButton(new CTFScrollButton(ARROW_DOWN, "", 0, 0, 16, 16), 1);
	pScrollBar->setSlider(new CTFSlider(0, wide - 1, wide, (tall - (wide * 2)) + 2, true));
	pScrollBar->setPaintBorderEnabled(false);
	pScrollBar->setPaintBackgroundEnabled(false);
	pScrollBar->setPaintEnabled(false);

	pScrollBar = getHorizontalScrollBar();
	pScrollBar->setButton(new CTFScrollButton(ARROW_LEFT, "", 0, 0, 16, 16), 0);
	pScrollBar->setButton(new CTFScrollButton(ARROW_RIGHT, "", 0, 0, 16, 16), 1);
	pScrollBar->setSlider(new CTFSlider(tall, 0, wide - (tall * 2), tall, false));
	pScrollBar->setPaintBorderEnabled(false);
	pScrollBar->setPaintBackgroundEnabled(false);
	pScrollBar->setPaintEnabled(false);
}

void CHandler_MenuButtonOver::cursorEntered(Panel* panel)
{
	if (gViewPort && m_pMenuPanel)
	{
		m_pMenuPanel->SetActiveInfo(m_iButton);
	}
}

void CMenuHandler_StringCommandClassSelect::actionPerformed(Panel* panel)
{
	CMenuHandler_StringCommand::actionPerformed(panel);

	bool bAutoKill = CVAR_GET_FLOAT("hud_classautokill") != 0;
	if (bAutoKill && g_iPlayerClass != 0)
		gEngfuncs.pfnClientCmd("kill");
}
