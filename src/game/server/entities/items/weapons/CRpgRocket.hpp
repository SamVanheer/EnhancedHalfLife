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

#include "CGrenade.hpp"
#include "CRpgRocket.generated.hpp"

class CRpg;

class EHL_CLASS() CRpgRocket : public CGrenade
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	void Precache() override;
	void EXPORT FollowThink();
	void EXPORT IgniteThink();
	void EXPORT RocketTouch(CBaseEntity* pOther);
	static CRpgRocket* CreateRpgRocket(const Vector& vecOrigin, const Vector & vecAngles, CBaseEntity* pOwner, CRpg* pLauncher);

	int m_iTrail = 0;

	EHL_FIELD(Persisted, Type=Time)
	float m_flIgniteTime = 0;

	EHL_FIELD(Persisted)
	EHandle<CRpg> m_hLauncher;// handle back to the launcher that fired me. 
};
