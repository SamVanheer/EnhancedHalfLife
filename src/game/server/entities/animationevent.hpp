/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
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

struct MonsterEvent_t
{
	int			event;
	char* options;
};

constexpr int EVENT_SPECIFIC = 0;
constexpr int EVENT_SCRIPTED = 1000;
constexpr int EVENT_SHARED = 2000;
constexpr int EVENT_CLIENT = 5000;

constexpr int MONSTER_EVENT_BODYDROP_LIGHT = 2001;
constexpr int MONSTER_EVENT_BODYDROP_HEAVY = 2002;

constexpr int MONSTER_EVENT_SWISHSOUND = 2010;
