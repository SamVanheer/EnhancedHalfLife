/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/

#pragma once

#include "CBaseTurret.hpp"
#include "CTurret.generated.hpp"

class EHL_CLASS("EntityName": "monster_turret") CTurret : public CBaseTurret
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	void Precache() override;
	// Think functions
	void SpinUpCall() override;
	void SpinDownCall() override;

	// other functions
	void Shoot(const Vector& vecSrc, const Vector& vecDirToEnemy) override;

private:
	EHL_FIELD("Persisted": true)
	bool m_iStartSpin = false;

};
