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

constexpr int SF_SPARK_TOGGLE = 1 << 5;
constexpr int SF_SPARK_START_ON = 1 << 6;

class EHL_CLASS() CEnvSpark : public CBaseEntity
{
public:
	void	Spawn() override;
	void	Precache() override;
	void	EXPORT SparkThink();
	void	EXPORT SparkStart(const UseInfo& info);
	void	EXPORT SparkStop(const UseInfo& info);
	void	KeyValue(KeyValueData* pkvd) override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];

	float m_flDelay = 0;
};

/**
*	@brief Makes flagged buttons spark when turned off
*/
void DoSpark(CBaseEntity* entity, const Vector& location);
