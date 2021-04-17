//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#pragma once

enum
{
	MAX_PLAYERS = 64,
	MAX_TEAMS = 64,
	MAX_TEAM_NAME = 16,
};

constexpr int MAX_SCORES = 10;
constexpr int MAX_SCOREBOARD_TEAMS = 5;

constexpr int NUM_ROWS = MAX_PLAYERS + (MAX_SCOREBOARD_TEAMS * 2);

constexpr int MAX_SERVERNAME_LENGTH = 64;
constexpr int MAX_TEAMNAME_SIZE = 32;
