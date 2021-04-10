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

#pragma once

/**
*	@file
* 
*	Used to detect the texture the player is standing on, map the texture name to a material type.
*	Play footstep sound based on material type.
*/

constexpr int CTEXTURESMAX = 512;			//!< max number of textures loaded
constexpr int CBTEXTURENAMEMAX = 13;		//!< only load first n chars of name

// texture types
constexpr char CHAR_TEX_CONCRETE = 'C';
constexpr char CHAR_TEX_METAL = 'M';
constexpr char CHAR_TEX_DIRT = 'D';
constexpr char CHAR_TEX_VENT = 'V';
constexpr char CHAR_TEX_GRATE = 'G';
constexpr char CHAR_TEX_TILE = 'T';
constexpr char CHAR_TEX_SLOSH = 'S';
constexpr char CHAR_TEX_WOOD = 'W';
constexpr char CHAR_TEX_COMPUTER = 'P';
constexpr char CHAR_TEX_GLASS = 'Y';
constexpr char CHAR_TEX_FLESH = 'F';
constexpr char CHAR_TEX_SNOW = 'N';

void TEXTURETYPE_Init();
char TEXTURETYPE_Find(char* name);
