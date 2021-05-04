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

#include "steam/steamtypes.h"

/*
*	@brief MD5 Hash
*/
struct MD5Context_t
{
	unsigned int buf[4];
	unsigned int bits[2];
	unsigned char in[64];
};

typedef std::uint32_t CRC32_t;
