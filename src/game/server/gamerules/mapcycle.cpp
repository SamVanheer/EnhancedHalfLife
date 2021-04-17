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

#include "extdll.h"
#include "util.h"
#include "mapcycle.hpp"
#include "CTokenizer.hpp"

/*
==============
ExtractCommandString

Parse commands/key value pairs to issue right after map xxx command is issued on server
 level transition
==============
*/
void ExtractCommandString(char* s, char* szCommand, std::size_t commandSize)
{
	// Now make rules happen
	char	pkey[512];
	char	value[512];	// use two buffers so compares
								// work without stomping on each other
	char* o;

	if (*s == '\\')
		s++;

	while (true)
	{
		o = pkey;
		while (*s != '\\')
		{
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;

		while (*s != '\\' && *s)
		{
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;

		safe_strcat(szCommand, pkey, commandSize);
		if (strlen(value) > 0)
		{
			safe_strcat(szCommand, " ", commandSize);
			safe_strcat(szCommand, value, commandSize);
		}
		safe_strcat(szCommand, "\n", commandSize);

		if (!*s)
			return;
		s++;
	}
}

/*
==============
ReloadMapCycleFile


Parses mapcycle.txt file into mapcycle_t structure
==============
*/
bool ReloadMapCycleFile(const char* filename, mapcycle_t* cycle)
{
	//TODO: clean this up
	char szBuffer[MAX_RULE_BUFFER];
	char szMap[MAX_MAPNAME_LENGTH];

	auto [fileBuffer, length] = FileSystem_LoadFileIntoBuffer(filename);

	bool hasbuffer;
	mapcycle_item_t* item, * newlist = nullptr, * next;

	if (fileBuffer && length)
	{
		CTokenizer tokenizer{reinterpret_cast<char*>(fileBuffer.get())};

		// the first map name in the file becomes the default
		while (true)
		{
			hasbuffer = false;
			memset(szBuffer, 0, MAX_RULE_BUFFER);

			tokenizer.Next();
			if (tokenizer.GetToken().empty())
				break;

			safe_strcpy(szMap, tokenizer.GetToken());

			// Any more tokens on this line?
			if (tokenizer.TokenWaiting())
			{
				tokenizer.Next();
				if (!tokenizer.GetToken().empty())
				{
					hasbuffer = true;
					safe_strcpy(szBuffer, tokenizer.GetToken());
				}
			}

			// Check map
			if (IS_MAP_VALID(szMap))
			{
				// Create entry
				char* s;

				item = new mapcycle_item_t;

				safe_strcpy(item->mapname, szMap);

				item->minplayers = 0;
				item->maxplayers = 0;

				memset(item->rulebuffer, 0, MAX_RULE_BUFFER);

				if (hasbuffer)
				{
					s = g_engfuncs.pfnInfoKeyValue(szBuffer, "minplayers");
					if (s && s[0])
					{
						item->minplayers = atoi(s);
						item->minplayers = std::max(item->minplayers, 0);
						item->minplayers = std::min(item->minplayers, gpGlobals->maxClients);
					}
					s = g_engfuncs.pfnInfoKeyValue(szBuffer, "maxplayers");
					if (s && s[0])
					{
						item->maxplayers = atoi(s);
						item->maxplayers = std::max(item->maxplayers, 0);
						item->maxplayers = std::min(item->maxplayers, gpGlobals->maxClients);
					}

					// Remove keys
					//
					g_engfuncs.pfnInfo_RemoveKey(szBuffer, "minplayers");
					g_engfuncs.pfnInfo_RemoveKey(szBuffer, "maxplayers");

					safe_strcpy(item->rulebuffer, szBuffer);
				}

				item->next = cycle->items;
				cycle->items = item;
			}
			else
			{
				ALERT(at_console, "Skipping %s from mapcycle, not a valid map\n", szMap);
			}

		}

		fileBuffer.reset();
	}

	// Fixup circular list pointer
	item = cycle->items;

	// Reverse it to get original order
	while (item)
	{
		next = item->next;
		item->next = newlist;
		newlist = item;
		item = next;
	}
	cycle->items = newlist;
	item = cycle->items;

	// Didn't parse anything
	if (!item)
	{
		return false;
	}

	while (item->next)
	{
		item = item->next;
	}
	item->next = cycle->items;

	cycle->next_item = item->next;

	return true;
}

/*
==============
DestroyMapCycle

Clean up memory used by mapcycle when switching it
==============
*/
void DestroyMapCycle(mapcycle_t* cycle)
{
	mapcycle_item_t* p, * n, * start;
	p = cycle->items;
	if (p)
	{
		start = p;
		p = p->next;
		while (p != start)
		{
			n = p->next;
			delete p;
			p = n;
		}

		delete cycle->items;
	}
	cycle->items = nullptr;
	cycle->next_item = nullptr;
}