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

class CLaser;

class CFuncTankLaser : public CFuncTank
{
public:
	void	Activate() override;
	void	KeyValue(KeyValueData* pkvd) override;
	void	Fire(const Vector& barrelEnd, const Vector& forward, CBaseEntity* pAttacker) override;
	void	Think() override;
	CLaser* GetLaser();

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

private:
	EHandle<CLaser> m_hLaser;
	float m_laserTime = 0;
};
