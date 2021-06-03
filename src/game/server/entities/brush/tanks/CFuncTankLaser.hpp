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

#include "CFuncTank.hpp"
#include "CFuncTankLaser.generated.hpp"

class CLaser;

class EHL_CLASS("EntityName": "func_tanklaser") CFuncTankLaser : public CFuncTank
{
	EHL_GENERATED_BODY()

public:
	void	Activate() override;
	void	KeyValue(KeyValueData* pkvd) override;
	void	Fire(const Vector& barrelEnd, const Vector& forward, CBaseEntity* pAttacker) override;
	void	Think() override;
	CLaser* GetLaser();

private:
	EHL_FIELD("Persisted": true)
	EHandle<CLaser> m_hLaser;

	EHL_FIELD("Persisted": true, "Type": "Time")
	float m_laserTime = 0;
};
