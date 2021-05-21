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

/**
*	@file
*
*	4-23-98
*	JOHN:  client dll interface declarations
*	this file is included by both the engine and the client-dll,
*	so make sure engine declarations aren't done twice
*/

#include <cstdint>

#include "const.hpp"
#include "ref_params.hpp"
#include "r_efx.hpp"
#include "wrect.hpp"
#include "in_buttons.hpp"
#include "cl_engine_int.hpp"

constexpr int SCRINFO_SCREENFLASH = 1;
constexpr int SCRINFO_STRETCHED = 2;

struct SCREENINFO
{
	int		iSize;
	int		iWidth;
	int		iHeight;
	int		iFlags;
	int		iCharHeight;
	short	charWidths[256];
};

struct client_data_t
{
	// fields that cannot be modified  (ie. have no effect if changed)
	Vector origin;

	// fields that can be changed by the cldll
	Vector viewangles;
	int		iWeaponBits;
	//	int		iAccessoryBits;
	float	fov;	// field of view
};

struct client_sprite_t
{
	char szName[64];
	char szSprite[64];
	int hspr;
	int iRes;
	wrect_t rc;
};

struct hud_player_info_t
{
	char* name;
	short ping;
	byte thisplayer;  //!< true if this is the calling player

	byte spectator;
	byte packetloss;

	char* model;
	short topcolor;
	short bottomcolor;

	std::uint64_t m_nSteamID;
};
