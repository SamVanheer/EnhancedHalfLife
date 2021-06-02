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
#include "CControllerHeadBall.generated.hpp"

/**
*	@brief Controller bouncy ball attack
*/
class EHL_CLASS(EntityName=controller_head_ball) CControllerHeadBall : public CBaseMonster
{
	EHL_GENERATED_BODY()

	void Spawn() override;
	void Precache() override;
	void EXPORT HuntThink();
	void EXPORT DieThink();
	void EXPORT BounceTouch(CBaseEntity* pOther);
	void MovetoTarget(Vector vecTarget);
	void Crawl();
	int m_iTrail = 0;
	int m_flNextAttack = 0;
	Vector m_vecIdeal;
	EHANDLE m_hOwner;
};
