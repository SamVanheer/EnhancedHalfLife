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

// Custom Entities

// Start/End Entity is encoded as 12 bits of entity index, and 4 bits of attachment (4:12)
constexpr int BEAMENT_ENTITY(int x)
{
	return x & 0xFFF;
}

constexpr int BEAMENT_ATTACHMENT(int x)
{
	return (x >> 12) & 0xF;
}

// Beam types, encoded as a byte
enum 
{
	BEAM_POINTS = 0,
	BEAM_ENTPOINT,
	BEAM_ENTS,
	BEAM_HOSE,
};

constexpr int BEAM_FSINE = 0x10;
constexpr int BEAM_FSOLID = 0x20;
constexpr int BEAM_FSHADEIN = 0x40;
constexpr int BEAM_FSHADEOUT = 0x80;
