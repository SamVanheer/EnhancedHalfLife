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

#include "CBaseEntity.hpp"
#include "CBaseDelay.generated.hpp"

/**
*	@brief generic Delay entity.
*/
class EHL_CLASS() CBaseDelay : public CBaseEntity
{
	EHL_GENERATED_BODY()

public:
	EHL_FIELD(Persisted)
	float m_flDelay = 0;

	EHL_FIELD(Persisted)
	string_t m_iszKillTarget = iStringNull;

	void KeyValue(KeyValueData* pkvd) override;

	// common member functions
	//TODO: this is a non-virtual override of the same function in CBaseEntity. Should probably just merge this class into CBaseEntity
	void SUB_UseTargets(CBaseEntity* pActivator, UseType useType, float value);
	void EXPORT DelayThink();
};