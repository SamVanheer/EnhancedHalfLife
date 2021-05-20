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

#include "CGrenade.hpp"

class EHL_CLASS() CApacheHVR : public CGrenade
{
	void Spawn() override;
	void Precache() override;
	void EXPORT IgniteThink();
	void EXPORT AccelerateThink();

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	int m_iTrail = 0;
	Vector m_vecForward;
};
