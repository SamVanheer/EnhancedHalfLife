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

#include "CBaseEntity.hpp"
#include "animationevent.hpp"

#include "CBaseAnimating.generated.hpp"

struct studiohdr_t;

class EHL_CLASS() CBaseAnimating : public CBaseEntity
{
	EHL_GENERATED_BODY()

public:
	/**
	*	@brief Gets the studio model header
	*	Only valid for entities that use studio models
	*/
	studiohdr_t* GetModelPointer();

	// Basic Monster Animation functions
	/**
	*	@brief advance the animation frame up to the current time
	*
	*	accumulate animation frame time from last time called until now
	*	if an flInterval is passed in, only advance animation that number of seconds
	*/
	float StudioFrameAdvance(float flInterval = 0.0);
	int GetSequenceFlags();
	int LookupActivity(int activity);

	/**
	*	@brief Get activity with highest 'weight'
	*/
	int LookupActivityHeaviest(int activity);
	int LookupSequence(const char* label);
	void ResetSequenceInfo();

	/**
	*	@brief Handle events that have happend since last time called up until X seconds into the future
	*/
	void DispatchAnimEvents(float flFutureInterval = 0.1);

	/**
	*	@brief catches the entity-specific messages that occur when tagged animation frames are played.
	*/
	virtual void HandleAnimEvent(AnimationEvent& event) {}
	float SetBoneController(int iController, float flValue);
	void InitBoneControllers();
	float SetBlending(int iBlender, float flValue);
	void GetBonePosition(int iBone, Vector& origin, Vector& angles);
	int FindTransition(int iEndingSequence, int iGoalSequence, int& iDir);
	int FindTransition(int iEndingSequence, int iGoalSequence);
	void GetAttachment(int iAttachment, Vector& origin, Vector& angles);
	void SetBodygroup(int iGroup, int iValue);
	int GetBodygroup(int iGroup);
	bool ExtractBbox(int sequence, Vector& mins, Vector& maxs);
	void SetSequenceBox();

	// animation needs
	EHL_FIELD("Persisted": true)
	float m_flFrameRate = 0;			//!< computed FPS for current sequence

	EHL_FIELD("Persisted": true)
	float m_flGroundSpeed = 0;			//!< computed linear movement rate for current sequence

	EHL_FIELD("Persisted": true, "Type": "Time")
	float m_flLastEventCheck = 0;		//!< last time the event list was checked

	EHL_FIELD("Persisted": true)
	bool m_fSequenceFinished = false;	//!< flag set when StudioAdvanceFrame moves across a frame boundry

	EHL_FIELD("Persisted": true)
	bool m_fSequenceLoops = false;		//!< true if the sequence loops
};