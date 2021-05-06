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

class CBaseEntity;
class CBasePlayer;
class Vector;

/**
*	@file
*
*	Functions and types for iterating over entities
*/

CBaseEntity* UTIL_EntityByIndex(int index);

//TODO: optimize this so it doesn't have to query every time
inline CBaseEntity* UTIL_GetWorld()
{
	return UTIL_EntityByIndex(0);
}

/**
*	@brief Returns a CBasePlayer pointer to a player by index
*
*	Only returns if the player is spawned and connected, otherwise returns nullptr
*	Index is 1 based
*/
CBasePlayer* UTIL_PlayerByIndex(int playerIndex);

/**
*	@brief In singleplayer this gets the local player.
*	In multiplayer this returns null.
*/
CBasePlayer* UTIL_GetLocalPlayer();

/**
*	@brief Find a player with a case-insensitive name search.
*/
CBasePlayer* FindPlayerByName(const char* pTestName);

/**
*	@brief Determine the current # of active players on the server for map cycling logic
*/
int UTIL_CountPlayers();

CBaseEntity* UTIL_FindEntityInSphere(CBaseEntity* pStartEntity, const Vector& vecCenter, float flRadius);
CBaseEntity* UTIL_FindEntityByString(CBaseEntity* pStartEntity, const char* szKeyword, const char* szValue);
CBaseEntity* UTIL_FindEntityByClassname(CBaseEntity* pStartEntity, const char* szName);
CBaseEntity* UTIL_FindEntityByTargetname(CBaseEntity* pStartEntity, const char* szName);

/**
*	@brief for doing a reverse lookup. Say you have a door, and want to find its button.
*/
CBaseEntity* UTIL_FindEntityByTarget(CBaseEntity* pStartEntity, const char* szName);

CBaseEntity* UTIL_FindEntityGeneric(const char* szName, const Vector& vecSrc, float flRadius);

CBaseEntity* UTIL_FindClientInPVS(CBaseEntity* pPVSEntity);

/**
*	@brief Pass in an array of pointers and an array size, it fills the array and returns the number inserted
*/
int UTIL_MonstersInSphere(CBaseEntity** pList, int listMax, const Vector& center, float radius);
int UTIL_EntitiesInBox(CBaseEntity** pList, int listMax, const Vector& mins, const Vector& maxs, int flagMask);

#define UTIL_EntitiesInPVS(pent) (*g_engfuncs.pfnEntitiesInPVS)(pent)
