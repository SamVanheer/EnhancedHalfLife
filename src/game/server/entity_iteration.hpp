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

#include "shared_utils.hpp"
#include "progdefs.h"

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

CBasePlayer* UTIL_FindClientInPVS(CBaseEntity* pPVSEntity);

/**
*	@brief Pass in an array of pointers and an array size, it fills the array and returns the number inserted
*/
int UTIL_MonstersInSphere(CBaseEntity** pList, int listMax, const Vector& center, float radius);
int UTIL_EntitiesInBox(CBaseEntity** pList, int listMax, const Vector& mins, const Vector& maxs, int flagMask);

#define UTIL_EntitiesInPVS(pent) (*g_engfuncs.pfnEntitiesInPVS)(pent)

template<typename T>
struct FindByClassnameFunctor
{
	static T* Find(T* pStartEntity, const char* pszClassname)
	{
		return static_cast<T*>(UTIL_FindEntityByClassname(pStartEntity, pszClassname));
	}
};

template<typename T>
struct FindByTargetnameFunctor
{
	static T* Find(T* pStartEntity, const char* pszName)
	{
		return static_cast<T*>(UTIL_FindEntityByTargetname(pStartEntity, pszName));
	}
};

template<typename T, typename FINDER>
class CEntityIterator
{
public:
	CEntityIterator()
		: m_pszName("")
		, m_pEntity(nullptr)
	{
	}

	CEntityIterator(const CEntityIterator&) = default;

	CEntityIterator(const char* const pszName, T* pEntity)
		: m_pszName(pszName)
		, m_pEntity(pEntity)
	{
	}

	CEntityIterator& operator=(const CEntityIterator&) = default;

	const T* operator*() const { return m_pEntity; }

	T* operator*() { return m_pEntity; }

	T* operator->() { return m_pEntity; }

	void operator++()
	{
		m_pEntity = static_cast<T*>(FINDER::Find(m_pEntity, m_pszName));
	}

	void operator++(int)
	{
		++* this;
	}

	bool operator==(const CEntityIterator& other) const
	{
		return m_pEntity == other.m_pEntity;
	}

	bool operator!=(const CEntityIterator& other) const
	{
		return !(*this == other);
	}

private:
	const char* const m_pszName;
	T* m_pEntity;
};

/**
*	@brief Entity enumerator optimized for iteration from start
*/
template<typename T, typename FINDER>
class CEntityEnumerator
{
public:
	using Functor = FINDER;
	using iterator = CEntityIterator<T, Functor>;

public:
	CEntityEnumerator(const char* pszClassName)
		: m_pszName(pszClassName)
	{
	}

	iterator begin()
	{
		return {m_pszName, static_cast<T*>(Functor::Find(nullptr, m_pszName))};
	}

	iterator end()
	{
		return {m_pszName, nullptr};
	}

private:
	const char* const m_pszName;
};

/**
*	@brief Entity enumerator for iteration from a given start entity
*/
template<typename T, typename FINDER>
class CEntityEnumeratorWithStart
{
public:
	using Functor = FINDER;
	using iterator = CEntityIterator<T, Functor>;

public:
	CEntityEnumeratorWithStart(const char* pszClassName, T* pStartEntity)
		: m_pszName(pszClassName)
		, m_pStartEntity(pStartEntity)
	{
	}

	iterator begin()
	{
		return {m_pszName, static_cast<T*>(Functor::Find(m_pStartEntity, m_pszName))};
	}

	iterator end()
	{
		return {m_pszName, nullptr};
	}

private:
	const char* const m_pszName;
	T* m_pStartEntity = nullptr;
};

template<typename T = CBaseEntity>
inline CEntityEnumerator<T, FindByClassnameFunctor<T>> UTIL_FindEntitiesByClassname(const char* pszClassName)
{
	return {pszClassName};
}

template<typename T = CBaseEntity>
inline CEntityEnumeratorWithStart<T, FindByClassnameFunctor<T>> UTIL_FindEntitiesByClassname(const char* pszClassName, T* pStartEntity)
{
	return {pszClassName, pStartEntity};
}

template<typename T = CBaseEntity>
inline CEntityEnumerator<T, FindByTargetnameFunctor<T>> UTIL_FindEntitiesByTargetname(const char* pszName)
{
	return {pszName};
}

template<typename T = CBaseEntity>
inline CEntityEnumeratorWithStart<T, FindByTargetnameFunctor<T>> UTIL_FindEntitiesByTargetname(const char* pszName, T* pStartEntity)
{
	return {pszName, pStartEntity};
}

class CPlayerIterator
{
public:
	CPlayerIterator() = default;
	CPlayerIterator(const CPlayerIterator&) = default;
	CPlayerIterator& operator=(const CPlayerIterator&) = default;

	CPlayerIterator(int index)
		: m_iIndex(index)
	{
		FindNextPlayer(); //Find the first player to start off with
	}

	const CBasePlayer* operator*() const { return m_pEntity; }

	CBasePlayer* operator*() { return m_pEntity; }

	CBasePlayer* operator->() { return m_pEntity; }

	void operator++()
	{
		++m_iIndex;
		FindNextPlayer();
	}

	void operator++(int)
	{
		++* this;
	}

	bool operator==(const CPlayerIterator& other) const
	{
		return m_iIndex == other.m_iIndex;
	}

	bool operator!=(const CPlayerIterator& other) const
	{
		return !(*this == other);
	}

private:
	void FindNextPlayer()
	{
		//Find the next valid player
		for (; m_iIndex <= gpGlobals->maxClients; ++m_iIndex)
		{
			if (auto player = UTIL_PlayerByIndex(m_iIndex); player)
			{
				m_pEntity = player;
				return;
			}
		}

		//No more players to check, now end iterator
		m_pEntity = nullptr;
	}

private:
	int m_iIndex = gpGlobals->maxClients + 1; //Default to end iterator, which is one past the last player slot
	CBasePlayer* m_pEntity = nullptr; //Cached for faster access
};

/**
*	@brief Player enumerator
*/
class CPlayerEnumerator
{
public:
	using iterator = CPlayerIterator;

public:
	CPlayerEnumerator() = default;

	iterator begin()
	{
		return {1}; //Start at first player
	}

	iterator end()
	{
		return {};
	}
};

/**
*	@brief Iterates over all players currently fully connected to the server
*/
inline CPlayerEnumerator UTIL_AllPlayers()
{
	return {};
}
