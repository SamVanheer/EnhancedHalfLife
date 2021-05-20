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

#include "CCineMonster.hpp"

class EHL_CLASS() CCineAI : public CCineMonster
{
	/**
	*	@brief lookup a sequence name and setup the target monster to play it
	*	overridden for CCineAI because it's ok for them to not have an animation sequence for the monster to play.
	*	For a regular Scripted Sequence, that situation is an error.
	*/
	bool StartSequence(CBaseMonster * pTarget, string_t iszSeq, bool completeOnEmpty) override;

	/**
	*	@brief make the entity carry out the scripted sequence instructions,
	*	but without destroying the monster's state.
	*/
	void PossessEntity() override;

	/**
	*	@brief returns true because scripted AI can possess entities regardless of their state.
	*/
	bool CanOverrideState() override;

	/**
	*	@brief When a monster finishes a scripted sequence,
	*	we have to fix up its state and schedule for it to return to a normal AI monster.
	*
	*	@details AI Scripted sequences will, depending on what the level designer selects:
	*	-Dirty the monster's schedule and drop out of the sequence in their current state.
	*	-Select a specific AMBUSH schedule, regardless of state.
	*/
	void FixScriptMonsterSchedule(CBaseMonster * pMonster) override;
};

