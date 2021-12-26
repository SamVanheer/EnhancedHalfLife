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

#include "PersistenceDefs.hpp"

class CBaseEntity;
struct edict_t;
struct SAVERESTOREDATA;

namespace persistence
{
/**
*	@brief Provides an interface to read and write data from the saverestore buffer
*/
class SaveRestoreBuffer
{
public:
	SaveRestoreBuffer(SAVERESTOREDATA* data)
		: m_data(data)
	{
	}

	~SaveRestoreBuffer() = default;

	int EntityIndex(const edict_t* entLookup) const;
	int EntityIndex(CBaseEntity* entity) const;

	edict_t* EntityFromIndex(int entityIndex) const;

	int GetEntityFlags(int entityIndex) const;
	void SetEntityFlags(int entityIndex, int flags);

	float GetTimeOffset() const
	{
		if (!m_data)
		{
			return 0;
		}

		return m_data->time;
	}

	Vector GetLandmarkOffset() const
	{
		if (!m_data || !m_data->fUseLandmark)
		{
			return vec3_origin;
		}

		return m_data->vecLandmarkOffset;
	}

protected:
	void BufferRewind(std::size_t size);

	unsigned int HashString(const char* token);
	unsigned short TokenHash(const char* token);

protected:
	SAVERESTOREDATA* const m_data = nullptr;
};
}
