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

#include "CBaseMonster.hpp"
#include "CFlyingMonster.generated.hpp"

/**
*	@brief Base class for flying monsters.  This overrides the movement test & execution code from CBaseMonster
*/
class EHL_CLASS() CFlyingMonster : public CBaseMonster
{
	EHL_GENERATED_BODY()

public:
	LocalMoveResult CheckLocalMove(const Vector& vecStart, const Vector& vecEnd, CBaseEntity* pTarget, float* pflDist) override;
	bool		Triangulate(const Vector& vecStart, const Vector& vecEnd, float flDist, CBaseEntity* pTargetEnt, Vector* pApex) override;
	Activity	GetStoppedActivity() override;
	void		Killed(const KilledInfo& info) override;
	void		Stop() override;
	float		ChangeYaw(int speed) override;
	void		HandleAnimEvent(AnimationEvent& event) override;
	void		MoveExecute(CBaseEntity* pTargetEnt, const Vector& vecDir, float flInterval) override;
	void		Move(float flInterval = 0.1) override;
	bool		ShouldAdvanceRoute(float flWaypointDist) override;

	inline void	SetFlyingMomentum(float momentum) { m_momentum = momentum; }
	inline void	SetFlyingFlapSound(const char* pFlapSound) { m_pFlapSound = pFlapSound; }
	inline void	SetFlyingSpeed(float speed) { m_flightSpeed = speed; }
	float		CeilingZ(const Vector& position);
	float		FloorZ(const Vector& position);
	bool		ProbeZ(const Vector& position, const Vector& probe, float& fraction);

	// UNDONE:  Save/restore this stuff!!!
protected:
	Vector m_vecTravel;				// Current direction
	float m_flightSpeed = 0;		// Current flight speed (decays when not flapping or gliding)
	float m_stopTime = 0;			// Last time we stopped (to avoid switching states too soon)
	float m_momentum = 0;			// Weight for desired vs. momentum velocity
	const char* m_pFlapSound = nullptr;
	float m_flLastZYawTime = 0;
};
