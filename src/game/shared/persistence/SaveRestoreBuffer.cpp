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

#include <bit>

#include "cbase.hpp"
#include "SaveRestoreBuffer.hpp"

namespace persistence
{
int SaveRestoreBuffer::EntityIndex(CBaseEntity* entity) const
{
	if (entity == nullptr)
	{
		return -1;
	}

	return EntityIndex(entity->edict());
}

int SaveRestoreBuffer::EntityIndex(const edict_t* entLookup) const
{
	if (!m_data || entLookup == nullptr)
	{
		return -1;
	}

	for (int i = 0; i < m_data->tableCount; i++)
	{
		ENTITYTABLE* pTable = m_data->pTable + i;

		if (pTable->pent == entLookup)
		{
			return i;
		}
	}

	return -1;
}

edict_t* SaveRestoreBuffer::EntityFromIndex(int entityIndex) const
{
	if (!m_data || entityIndex < 0)
	{
		return nullptr;
	}

	for (int i = 0; i < m_data->tableCount; i++)
	{
		ENTITYTABLE* pTable = m_data->pTable + i;

		if (pTable->id == entityIndex)
		{
			return pTable->pent;
		}
	}

	return nullptr;
}

int SaveRestoreBuffer::GetEntityFlags(int entityIndex) const
{
	if (!m_data || entityIndex < 0)
	{
		return 0;
	}

	if (entityIndex > m_data->tableCount)
	{
		return 0;
	}

	return m_data->pTable[entityIndex].flags;
}

void SaveRestoreBuffer::SetEntityFlags(int entityIndex, int flags)
{
	if (!m_data || entityIndex < 0)
	{
		return;
	}

	if (entityIndex > m_data->tableCount)
	{
		return;
	}

	m_data->pTable[entityIndex].flags |= flags;
}

void SaveRestoreBuffer::BufferRewind(std::size_t size)
{
	if (!m_data)
	{
		return;
	}

	if (static_cast<std::size_t>(m_data->size) < size)
	{
		size = m_data->size;
	}

	m_data->pCurrentData -= size;
	m_data->size -= size;
}

unsigned int SaveRestoreBuffer::HashString(const char* token)
{
	unsigned int hash = 0;

	while (*token)
	{
		hash = std::rotr(hash, 4) ^ *token++;
	}

	return hash;
}

unsigned short SaveRestoreBuffer::TokenHash(const char* token)
{
	const unsigned short hash = static_cast<unsigned short>(HashString(token) % static_cast<unsigned int>(m_data->tokenCount));

#if _DEBUG
	static int tokensparsed = 0;
	tokensparsed++;

	if (!m_data->tokenCount || !m_data->pTokens)
	{
		ALERT(at_error, "No token table array in CSaveRestoreBuffer::TokenHash()!");
		return 0;
	}

#endif

	for (int i = 0; i < m_data->tokenCount; i++)
	{
#if _DEBUG
		static bool beentheredonethat = false;

		if (i > 50 && !beentheredonethat)
		{
			beentheredonethat = true;
			ALERT(at_error, "CSaveRestoreBuffer::TokenHash() is getting too full!");
		}
#endif

		int	index = hash + i;

		if (index >= m_data->tokenCount)
		{
			index -= m_data->tokenCount;
		}

		if (!m_data->pTokens[index] || strcmp(token, m_data->pTokens[index]) == 0)
		{
			m_data->pTokens[index] = const_cast<char*>(token);
			return index;
		}
	}

	// Token hash table full!!! 
	// [Consider doing overflow table(s) after the main table & limiting linear hash table search]
	ALERT(at_error, "CSaveRestoreBuffer::TokenHash() is COMPLETELY FULL!");
	return 0;
}
}
