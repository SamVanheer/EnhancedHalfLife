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

class EHL_CLASS() CTurret : public CBaseTurret
{
public:
	void Spawn() override;
	void Precache() override;
	// Think functions
	void SpinUpCall() override;
	void SpinDownCall() override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];

	// other functions
	void Shoot(const Vector& vecSrc, const Vector& vecDirToEnemy) override;

private:
	bool m_iStartSpin = false;

};
