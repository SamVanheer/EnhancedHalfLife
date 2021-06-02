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

#include "CBaseMonster.hpp"
#include "CNihilanthHVR.generated.hpp"

class CNihilanth;

/**
*	@brief Controller bouncy ball attack
*/
class EHL_CLASS(EntityName=nihilanth_energy_ball) CNihilanthHVR : public CBaseMonster
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	void Precache() override;

	void CircleInit(CBaseEntity* pTarget);
	void AbsorbInit();
	void TeleportInit(CNihilanth* pOwner, CBaseEntity* pEnemy, CBaseEntity* pTarget, CBaseEntity* pTouch);
	void GreenBallInit();
	void ZapInit(CBaseEntity* pEnemy);

	void EXPORT HoverThink();
	bool CircleTarget(Vector vecTarget);
	void EXPORT DissipateThink();

	void EXPORT ZapThink();
	void EXPORT TeleportThink();
	void EXPORT TeleportTouch(CBaseEntity* pOther);

	void EXPORT RemoveTouch(CBaseEntity* pOther);
	void EXPORT BounceTouch(CBaseEntity* pOther);
	void EXPORT ZapTouch(CBaseEntity* pOther);

	CBaseEntity* RandomClassname(const char* szName);

	// void EXPORT SphereUse(const UseInfo& info);

	void MovetoTarget(Vector vecTarget);
	virtual void Crawl();

	EHL_FIELD(Persisted)
	float m_flIdealVel = 0;

	EHL_FIELD(Persisted)
	Vector m_vecIdeal;

	EHL_FIELD(Persisted)
	EHandle<CNihilanth> m_hNihilanth;

	EHL_FIELD(Persisted)
	EHANDLE m_hTouch;

	EHL_FIELD(Persisted)
	int m_nFrames = 0;
};
