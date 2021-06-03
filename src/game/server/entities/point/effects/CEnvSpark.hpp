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
#include "CEnvSpark.generated.hpp"

constexpr int SF_SPARK_TOGGLE = 1 << 5;
constexpr int SF_SPARK_START_ON = 1 << 6;

class EHL_CLASS("EntityName": "env_spark", "EntityNameAliases": ["env_debris"]) CEnvSpark : public CBaseEntity
{
	EHL_GENERATED_BODY()

public:
	void	Spawn() override;
	void	Precache() override;
	void	EXPORT SparkThink();
	void	EXPORT SparkStart(const UseInfo& info);
	void	EXPORT SparkStop(const UseInfo& info);
	void	KeyValue(KeyValueData* pkvd) override;

	EHL_FIELD("Persisted": true)
	float m_flMaxDelay = 0;
};

/**
*	@brief Makes flagged buttons spark when turned off
*/
void DoSpark(CBaseEntity* entity, const Vector& location);
