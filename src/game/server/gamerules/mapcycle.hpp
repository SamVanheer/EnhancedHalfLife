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

constexpr int MAX_RULE_BUFFER = 1024;

struct mapcycle_item_t
{
	mapcycle_item_t* next;

	char mapname[MAX_MAPNAME_LENGTH];
	int  minplayers, maxplayers;
	char rulebuffer[MAX_RULE_BUFFER];
};

struct mapcycle_t
{
	mapcycle_item_t* items;
	mapcycle_item_t* next_item;
};

void ExtractCommandString(char* s, char* szCommand, std::size_t commandSize);
bool ReloadMapCycleFile(const char* filename, mapcycle_t* cycle);
void DestroyMapCycle(mapcycle_t* cycle);
