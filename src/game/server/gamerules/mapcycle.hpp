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

#include <cstddef>

#include "sv_engine_int.hpp"

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

/**
*	@brief Parse commands/key value pairs to issue right after map xxx command is issued on server level transition
*/
void ExtractCommandString(char* s, char* szCommand, std::size_t commandSize);

/**
*	@brief Parses mapcycle.txt file into mapcycle_t structure
*/
bool ReloadMapCycleFile(const char* filename, mapcycle_t* cycle);

/**
*	@brief Clean up memory used by mapcycle when switching it
*/
void DestroyMapCycle(mapcycle_t* cycle);
