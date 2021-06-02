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
#include "CBubbling.generated.hpp"

class EHL_CLASS(EntityName=env_bubbles) CBubbling : public CBaseEntity
{
	EHL_GENERATED_BODY()

public:
	void	Spawn() override;
	void	Precache() override;
	void	KeyValue(KeyValueData* pkvd) override;

	void	EXPORT FizzThink();
	void	Use(const UseInfo& info) override;

	int		ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	EHL_FIELD(Persisted)
	int m_density = 0;

	EHL_FIELD(Persisted)
	int m_frequency = 0;

	//let spawn restore this!
	int m_bubbleModel = 0;

	EHL_FIELD(Persisted)
	bool m_state = false;
};
