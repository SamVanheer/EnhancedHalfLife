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

#include "CBaseAnimating.hpp"

#include "CBaseToggle.generated.hpp"

class CBaseToggle;

using CallWhenMoveDoneFn = void (CBaseToggle::*)();

/**
*	@brief generic Toggle entity.
*/
class EHL_CLASS() CBaseToggle : public CBaseAnimating
{
	EHL_GENERATED_BODY()

public:
	void KeyValue(KeyValueData* pkvd) override;

	EHL_FIELD("Persisted": true)
	ToggleState m_toggle_state = ToggleState::AtTop;

	EHL_FIELD("Persisted": true, "Type": "Time")
	float m_flActivateFinished = 0;	//!< like attack_finished, but for doors

	EHL_FIELD("Persisted": true)
	float m_flMoveDistance = 0;		//!< how far a door should slide or rotate

	EHL_FIELD("Persisted": true)
	float m_flWait = 0;

	EHL_FIELD("Persisted": true)
	float m_flLip = 0;

	EHL_FIELD("Persisted": true, "Type": "Position")
	Vector m_vecPosition1;

	EHL_FIELD("Persisted": true, "Type": "Position")
	Vector m_vecPosition2;

	EHL_FIELD("Persisted": true)
	Vector m_vecAngle1; // UNDONE: Position could go through transition, but also angle?

	EHL_FIELD("Persisted": true)
	Vector m_vecAngle2; // UNDONE: Position could go through transition, but also angle?

	EHL_FIELD("Persisted": true)
	CallWhenMoveDoneFn m_pfnCallWhenMoveDone = nullptr;

	EHL_FIELD("Persisted": true, "Type": "Position")
	Vector m_vecFinalDest;

	EHL_FIELD("Persisted": true)
	Vector m_vecFinalAngle;

	float GetDelay() override { return m_flWait; }

	// common member functions
	/**
	*	@brief calculate pev->velocity and pev->nextthink to reach vecDest from GetAbsOrigin() traveling at flSpeed
	*/
	void LinearMove(const Vector& vecDest, float flSpeed);

	/**
	*	@brief After moving, set origin to exact final destination, call "move done" function
	*/
	void EXPORT LinearMoveDone();

	/**
	*	@brief calculate pev->velocity and pev->nextthink to reach vecDest from GetAbsOrigin() traveling at flSpeed
	*
	*	Just like LinearMove, but rotational.
	*/
	void AngularMove(const Vector& vecDestAngle, float flSpeed);

	/**
	*	@brief After rotating, set angle to exact final angle, call "move done" function
	*/
	void EXPORT AngularMoveDone();

	static float AxisValue(int flags, const Vector& angles);
	static void AxisDir(CBaseEntity* pEntity);
	static float AxisDelta(int flags, const Vector& angle1, const Vector& angle2);
};

#define SetMoveDone(a) m_pfnCallWhenMoveDone = static_cast<CallWhenMoveDoneFn>(a)