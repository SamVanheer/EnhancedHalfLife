/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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

bool KB_ConvertString(char* in, char** ppout);

DECLARE_MESSAGE(m_Menu, ShowMenu);

bool CHudMenu::Init()
{
	gHUD.AddHudElem(this);

	HOOK_MESSAGE(ShowMenu);

	InitHUDData();

	return true;
}

void CHudMenu::InitHUDData()
{
	m_fMenuDisplayed = false;
	m_bitsValidSlots = 0;
	Reset();
}

void CHudMenu::Reset()
{
	m_szPrelocalisedMenuString[0] = 0;
	m_fWaitingForMore = false;
}

bool CHudMenu::VidInit()
{
	return true;
}

/**
*	@brief Interprets the given escape token (backslash followed by a letter).
*	@details The first character of the token must be a backslash.
*	The second character specifies the operation to perform:
*		\w : White text (this is the default)
*		\d : Dim (gray) text
*		\y : Yellow text
*		\r : Red text
*		\R : Right-align (just for the remainder of the current line)
*/
static inline const char* ParseEscapeToken(const char* token, int& menuR, int& menuG, int& menuB, int& menuX, bool& menuRAlign)
{
	if (*token != '\\')
		return token;

	token++;

	switch (*token)
	{
	case '\0':
		return token;

	case 'w':
		menuR = 255;
		menuG = 255;
		menuB = 255;
		break;

	case 'd':
		menuR = 100;
		menuG = 100;
		menuB = 100;
		break;

	case 'y':
		menuR = 255;
		menuG = 210;
		menuB = 64;
		break;

	case 'r':
		menuR = 210;
		menuG = 24;
		menuB = 0;
		break;

	case 'R':
		menuX = ScreenWidth / 2;
		menuRAlign = true;
		break;
	}

	return ++token;
}

bool CHudMenu::Draw(float flTime)
{
	// check for if menu is set to disappear
	if (m_flShutoffTime > 0)
	{
		if (m_flShutoffTime <= gHUD.m_flTime)
		{  // times up, shutoff
			m_fMenuDisplayed = false;
			m_iFlags &= ~HUD_ACTIVE;
			return true;
		}
	}

	// don't draw the menu if the scoreboard is being shown
	if (gViewPort && gViewPort->IsScoreBoardVisible())
		return true;

	// draw the menu, along the left-hand side of the screen

	// count the number of newlines
	int nlc = 0;
	int i;
	for (i = 0; i < MAX_MENU_STRING && m_szMenuString[i] != '\0'; i++)
	{
		if (m_szMenuString[i] == '\n')
			nlc++;
	}

	// center it
	int y = (ScreenHeight / 2) - ((nlc / 2) * 12) - 40; // make sure it is above the say text

	int menu_r = 255;
	int menu_g = 255;
	int menu_b = 255;
	int menu_x = 20;
	bool menu_ralign = false;

	const char* sptr = m_szMenuString;

	while (*sptr != '\0')
	{
		if (*sptr == '\\')
		{
			sptr = ParseEscapeToken(sptr, menu_r, menu_g, menu_b, menu_x, menu_ralign);
		}
		else if (*sptr == '\n')
		{
			menu_ralign = false;
			menu_x = 20;
			y += (12);

			sptr++;
		}
		else
		{
			char menubuf[80];
			const char* ptr = sptr;
			while (*sptr != '\0' && *sptr != '\n' && *sptr != '\\')
			{
				sptr++;
			}

			safe_strcpy(menubuf, std::string_view{ptr, static_cast<std::size_t>(sptr - ptr)});

			if (menu_ralign)
			{
				// IMPORTANT: Right-to-left rendered text does not parse escape tokens!
				menu_x = gHUD.DrawHudStringReverse(menu_x, y, 0, menubuf, menu_r, menu_g, menu_b);
			}
			else
			{
				menu_x = gHUD.DrawHudString(menu_x, y, 320, menubuf, menu_r, menu_g, menu_b);
			}
		}
	}

	return true;
}

void CHudMenu::SelectMenuItem(int menu_item)
{
	// if menu_item is in a valid slot,  send a menuselect command to the server
	if ((menu_item > 0) && (m_bitsValidSlots & (1 << (menu_item - 1))))
	{
		char szbuf[32];
		snprintf(szbuf, sizeof(szbuf), "menuselect %d\n", menu_item);
		EngineClientCmd(szbuf);

		// remove the menu
		m_fMenuDisplayed = false;
		m_iFlags &= ~HUD_ACTIVE;
	}
}

bool CHudMenu::MsgFunc_ShowMenu(const char* pszName, int iSize, void* pbuf)
{
	BufferReader reader{pbuf, iSize};

	m_bitsValidSlots = reader.ReadShort();
	const int DisplayTime = reader.ReadChar();
	const bool NeedMore = reader.ReadByte() != 0;

	if (DisplayTime > 0)
		m_flShutoffTime = DisplayTime + gHUD.m_flTime;
	else
		m_flShutoffTime = -1;

	if (m_bitsValidSlots)
	{
		if (!m_fWaitingForMore) // this is the start of a new menu
		{
			safe_strcpy(m_szPrelocalisedMenuString, reader.ReadString());
		}
		else
		{  // append to the current menu string
			safe_strcat(m_szPrelocalisedMenuString, reader.ReadString());
		}

		if (!NeedMore)
		{  // we have the whole string, so we can localise it now
			safe_strcpy(m_szMenuString, gHUD.m_TextMessage.BufferedLocaliseTextString(m_szPrelocalisedMenuString));

			// Swap in characters
			char* temp = nullptr;
			if (KB_ConvertString(m_szMenuString, &temp))
			{
				safe_strcpy(m_szMenuString, temp);
				free(temp);
			}
		}

		m_fMenuDisplayed = true;
		m_iFlags |= HUD_ACTIVE;
	}
	else
	{
		m_fMenuDisplayed = false; // no valid slots means that the menu should be turned off
		m_iFlags &= ~HUD_ACTIVE;
	}

	m_fWaitingForMore = NeedMore;

	return true;
}
