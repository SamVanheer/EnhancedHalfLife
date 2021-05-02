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

DECLARE_MESSAGE(m_DeathNotice, DeathMsg);

struct DeathNoticeItem {
	char szKiller[MAX_PLAYER_NAME_LENGTH * 2];
	char szVictim[MAX_PLAYER_NAME_LENGTH * 2];
	int iId;	// the index number of the associated sprite
	int iSuicide;
	int iTeamKill;
	int iNonPlayerKill;
	float flDisplayTime;
	const Vector* KillerColor;
	const Vector* VictimColor;
};

constexpr int MAX_DEATHNOTICES = 4;
static int DEATHNOTICE_DISPLAY_TIME = 6;

constexpr int DEATHNOTICE_TOP = 64;

DeathNoticeItem rgDeathNoticeList[MAX_DEATHNOTICES + 1];

const Vector& GetClientColor(int clientIndex)
{
	switch (g_PlayerExtraInfo[clientIndex].teamnumber)
	{
	case 1:	return g_ColorBlue;
	case 2: return g_ColorRed;
	case 3: return g_ColorYellow;
	case 4: return g_ColorGreen;
	case 0: return g_ColorYellow;

	default: return g_ColorGrey;
	}
}

bool CHudDeathNotice::Init()
{
	gHUD.AddHudElem(this);

	HOOK_MESSAGE(DeathMsg);

	CVAR_CREATE("hud_deathnotice_time", "6", 0);

	return true;
}

void CHudDeathNotice::InitHUDData()
{
	memset(rgDeathNoticeList, 0, sizeof(rgDeathNoticeList));
}

bool CHudDeathNotice::VidInit()
{
	m_HUD_d_skull = gHUD.GetSpriteIndex("d_skull");

	return true;
}

bool CHudDeathNotice::Draw(float flTime)
{
	int x, y, r, g, b;

	for (int i = 0; i < MAX_DEATHNOTICES; i++)
	{
		if (rgDeathNoticeList[i].iId == 0)
			break;  // we've gone through them all

		if (rgDeathNoticeList[i].flDisplayTime < flTime)
		{ // display time has expired
			// remove the current item from the list
			memmove(&rgDeathNoticeList[i], &rgDeathNoticeList[i + 1], sizeof(DeathNoticeItem) * (MAX_DEATHNOTICES - i));
			i--;  // continue on the next item;  stop the counter getting incremented
			continue;
		}

		rgDeathNoticeList[i].flDisplayTime = std::min(rgDeathNoticeList[i].flDisplayTime, gHUD.m_flTime + DEATHNOTICE_DISPLAY_TIME);

		// Only draw if the viewport will let me
		if (gViewPort && gViewPort->AllowedToPrintText())
		{
			// Draw the death notice
			y = DEATHNOTICE_TOP + 2 + (20 * i);  //!!!

			int id = (rgDeathNoticeList[i].iId == -1) ? m_HUD_d_skull : rgDeathNoticeList[i].iId;
			x = ScreenWidth - ConsoleStringLen(rgDeathNoticeList[i].szVictim) - (gHUD.GetSpriteRect(id).right - gHUD.GetSpriteRect(id).left);

			if (!rgDeathNoticeList[i].iSuicide)
			{
				x -= (5 + ConsoleStringLen(rgDeathNoticeList[i].szKiller));

				// Draw killers name
				if (rgDeathNoticeList[i].KillerColor)
					gEngfuncs.pfnDrawSetTextColor(rgDeathNoticeList[i].KillerColor->x, rgDeathNoticeList[i].KillerColor->y, rgDeathNoticeList[i].KillerColor->z);
				x = 5 + DrawConsoleString(x, y, rgDeathNoticeList[i].szKiller);
			}

			r = 255;  g = 80;	b = 0;
			if (rgDeathNoticeList[i].iTeamKill)
			{
				r = 10;	g = 240; b = 10;  // display it in sickly green
			}

			// Draw death weapon
			SPR_Set(gHUD.GetSprite(id), r, g, b);
			SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(id));

			x += (gHUD.GetSpriteRect(id).right - gHUD.GetSpriteRect(id).left);

			// Draw victims name (if it was a player that was killed)
			if (rgDeathNoticeList[i].iNonPlayerKill == false)
			{
				if (rgDeathNoticeList[i].VictimColor)
					gEngfuncs.pfnDrawSetTextColor(rgDeathNoticeList[i].VictimColor->x, rgDeathNoticeList[i].VictimColor->y, rgDeathNoticeList[i].VictimColor->z);
				x = DrawConsoleString(x, y, rgDeathNoticeList[i].szVictim);
			}
		}
	}

	return true;
}

bool CHudDeathNotice::MsgFunc_DeathMsg(const char* pszName, int iSize, void* pbuf)
{
	m_iFlags |= HUD_ACTIVE;

	BufferReader reader{pbuf, iSize};

	int killer = reader.ReadByte();
	int victim = reader.ReadByte();

	char killedwith[32];
	safe_strcpy(killedwith, "d_");
	safe_strcat(killedwith, reader.ReadString());

	if (gViewPort)
		gViewPort->DeathMsg(killer, victim);

	gHUD.m_Spectator.DeathMessage(victim);
	int i;
	for (i = 0; i < MAX_DEATHNOTICES; i++)
	{
		if (rgDeathNoticeList[i].iId == 0)
			break;
	}
	if (i == MAX_DEATHNOTICES)
	{ // move the rest of the list forward to make room for this item
		memmove(rgDeathNoticeList, rgDeathNoticeList + 1, sizeof(DeathNoticeItem) * MAX_DEATHNOTICES);
		i = MAX_DEATHNOTICES - 1;
	}

	if (gViewPort)
		gViewPort->GetAllPlayersInfo();

	// Get the Killer's name
	const char* killer_name = g_PlayerInfoList[killer].name;
	if (!killer_name)
	{
		killer_name = "";
		rgDeathNoticeList[i].szKiller[0] = 0;
	}
	else
	{
		rgDeathNoticeList[i].KillerColor = &GetClientColor(killer);
		safe_strcpy(rgDeathNoticeList[i].szKiller, killer_name);
	}

	// Get the Victim's name
	const char* victim_name = nullptr;
	// If victim is -1, the killer killed a specific, non-player object (like a sentrygun)
	if (((char)victim) != -1)
		victim_name = g_PlayerInfoList[victim].name;
	if (!victim_name)
	{
		victim_name = "";
		rgDeathNoticeList[i].szVictim[0] = 0;
	}
	else
	{
		rgDeathNoticeList[i].VictimColor = &GetClientColor(victim);
		safe_strcpy(rgDeathNoticeList[i].szVictim, victim_name);
	}

	// Is it a non-player object kill?
	if (((char)victim) == -1)
	{
		rgDeathNoticeList[i].iNonPlayerKill = true;

		// Store the object's name in the Victim slot (skip the d_ bit)
		safe_strcpy(rgDeathNoticeList[i].szVictim, killedwith + 2);
	}
	else
	{
		if (killer == victim || killer == 0)
			rgDeathNoticeList[i].iSuicide = true;

		if (!strcmp(killedwith, "d_teammate"))
			rgDeathNoticeList[i].iTeamKill = true;
	}

	// Find the sprite in the list
	int spr = gHUD.GetSpriteIndex(killedwith);

	rgDeathNoticeList[i].iId = spr;

	DEATHNOTICE_DISPLAY_TIME = CVAR_GET_FLOAT("hud_deathnotice_time");
	rgDeathNoticeList[i].flDisplayTime = gHUD.m_flTime + DEATHNOTICE_DISPLAY_TIME;

	if (rgDeathNoticeList[i].iNonPlayerKill)
	{
		ConsolePrint(rgDeathNoticeList[i].szKiller);
		ConsolePrint(" killed a ");
		ConsolePrint(rgDeathNoticeList[i].szVictim);
		ConsolePrint("\n");
	}
	else
	{
		// record the death notice in the console
		if (rgDeathNoticeList[i].iSuicide)
		{
			ConsolePrint(rgDeathNoticeList[i].szVictim);

			if (!strcmp(killedwith, "d_world"))
			{
				ConsolePrint(" died");
			}
			else
			{
				ConsolePrint(" killed self");
			}
		}
		else if (rgDeathNoticeList[i].iTeamKill)
		{
			ConsolePrint(rgDeathNoticeList[i].szKiller);
			ConsolePrint(" killed his teammate ");
			ConsolePrint(rgDeathNoticeList[i].szVictim);
		}
		else
		{
			ConsolePrint(rgDeathNoticeList[i].szKiller);
			ConsolePrint(" killed ");
			ConsolePrint(rgDeathNoticeList[i].szVictim);
		}

		if (killedwith && *killedwith && (*killedwith > 13) && strcmp(killedwith, "d_world") && !rgDeathNoticeList[i].iTeamKill)
		{
			ConsolePrint(" with ");

			// replace the code names with the 'real' names
			if (!strcmp(killedwith + 2, "egon"))
				safe_strcpy(killedwith, "d_gluon gun");
			if (!strcmp(killedwith + 2, "gauss"))
				safe_strcpy(killedwith, "d_tau cannon");

			ConsolePrint(killedwith + 2); // skip over the "d_" part
		}

		ConsolePrint("\n");
	}

	return true;
}
