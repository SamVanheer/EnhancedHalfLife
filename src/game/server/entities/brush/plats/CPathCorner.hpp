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

#include "CPointEntity.hpp"
#include "CPathCorner.generated.hpp"

// Spawnflags of CPathCorner
constexpr int SF_CORNER_WAITFORTRIG = 0x001;
constexpr int SF_CORNER_TELEPORT = 0x002;
constexpr int SF_CORNER_FIREONCE = 0x004;

class EHL_CLASS(EntityName=path_corner) CPathCorner : public CPointEntity
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	void KeyValue(KeyValueData* pkvd) override;
	float GetDelay() override { return m_flWait; }
	//	void Touch( CBaseEntity *pOther ) override;

private:
	EHL_FIELD(Persisted)
	float m_flWait = 0;
};
