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
#include "scriptevent.hpp"
#include "CCineMonster.generated.hpp"

constexpr int SF_SCRIPT_WAITTILLSEEN = 1;
constexpr int SF_SCRIPT_EXITAGITATED = 2;
constexpr int SF_SCRIPT_REPEATABLE = 4;
constexpr int SF_SCRIPT_LEAVECORPSE = 8;
//constexpr int SF_SCRIPT_INTERPOLATE = 16; // don't use, old bug
constexpr int SF_SCRIPT_NOINTERRUPT = 32;
constexpr int SF_SCRIPT_OVERRIDESTATE = 64;
constexpr int SF_SCRIPT_NOSCRIPTMOVEMENT = 128;

constexpr int SCRIPT_BREAK_CONDITIONS = bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE;

enum SS_INTERRUPT
{
	SS_INTERRUPT_IDLE = 0,
	SS_INTERRUPT_BY_NAME,
	SS_INTERRUPT_AI,
};

// when a monster finishes an AI scripted sequence, we can choose
// a schedule to place them in. These defines are the aliases to
// resolve worldcraft input to real schedules (sjb)
constexpr int SCRIPT_FINISHSCHED_DEFAULT = 0;
constexpr int SCRIPT_FINISHSCHED_AMBUSH = 1;

enum class ScriptedMoveTo
{
	No = 0,
	Walk = 1,
	Run = 2,
	Instantaneous = 4,
	TurnToFace = 5
};

/**
*	@details Not sure if this is still accurate:
*	targetname "me" - there can be more than one with the same name, and they act in concert
*	target "the_entity_I_want_to_start_playing" or "class entity_classname" will pick the closest inactive scientist
*	play "name_of_sequence"
*	idle "name of idle sequence to play before starting"
*	donetrigger "whatever" - can be any other triggerable entity such as another sequence, train, door, or a special case like "die" or "remove"
*	moveto - if set the monster first moves to this nodes position
*	range # - only search this far to find the target
*	spawnflags - (stop if blocked, stop if player seen)
*/
class EHL_CLASS("EntityName": "scripted_sequence") CCineMonster : public CBaseMonster
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	void KeyValue(KeyValueData* pkvd) override;
	void Use(const UseInfo& info) override;

	/**
	*	@brief This doesn't really make sense since only Movetype::Push get 'Blocked' events
	*/
	void Blocked(CBaseEntity* pOther) override;
	void Touch(CBaseEntity* pOther) override;
	int	 ObjectCaps() override { return (CBaseMonster::ObjectCaps() & ~FCAP_ACROSS_TRANSITION); }

	/**
	*	@brief Find an entity that I'm interested in and precache the sounds he'll need in the sequence.
	*/
	void Activate() override;

	// void EXPORT CineSpawnThink();
	void EXPORT CineThink();

	/**
	*	@brief find all the cinematic entities with my targetname and tell them to wait before starting
	*/
	void DelayStart(int state);

	/**
	*	@brief find a viable entity
	*/
	bool FindEntity();

	/**
	*	@brief make the entity enter a scripted sequence
	*/
	virtual void PossessEntity();

	/**
	*	@brief find all the cinematic entities with my targetname and stop them from playing
	*/
	void CancelScript();

	/**
	*	@brief lookup a sequence name and setup the target monster to play it
	*/
	virtual bool StartSequence(CBaseMonster* pTarget, string_t iszSeq, bool completeOnEmpty);

	/**
	*	@brief returns false, scripted sequences cannot possess entities regardless of state.
	*/
	virtual bool CanOverrideState();

	/**
	*	@brief called when a scripted sequence animation sequence is done playing
	*	( or when an AI Scripted Sequence doesn't supply an animation sequence to play ).
	*	Expects the CBaseMonster pointer to the monster that the sequence possesses.
	*/
	void SequenceDone(CBaseMonster* pMonster);

	/**
	*	@brief When a monster finishes a scripted sequence,
	*	we have to fix up its state and schedule for it to return to a normal AI monster.
	*	Scripted sequences just dirty the Schedule and drop the monster in Idle State.
	*/
	virtual void FixScriptMonsterSchedule(CBaseMonster* pMonster);
	bool	CanInterrupt();
	void	AllowInterrupt(bool fAllow);
	int		IgnoreConditions() override;

	EHL_FIELD("Persisted": true)
	string_t m_iszIdle = iStringNull;		// string index for idle animation

	EHL_FIELD("Persisted": true)
	string_t m_iszPlay = iStringNull;		// string index for scripted animation

	EHL_FIELD("Persisted": true)
	string_t m_iszEntity = iStringNull;		// entity that is wanted for this script

	EHL_FIELD("Persisted": true)
	ScriptedMoveTo m_fMoveTo = ScriptedMoveTo::No;

	EHL_FIELD("Persisted": true)
	int m_iFinishSchedule = 0;

	EHL_FIELD("Persisted": true)
	float m_flRadius = 0;		// range to search

	EHL_FIELD("Persisted": true)
	float m_flRepeat = 0;	// repeat rate

	EHL_FIELD("Persisted": true)
	int m_iDelay = 0;

	EHL_FIELD("Persisted": true, "Type": "Time")
	float m_startTime = 0;

	EHL_FIELD("Persisted": true)
	Movetype m_saved_movetype = Movetype::None;

	EHL_FIELD("Persisted": true)
	Solid m_saved_solid = Solid::Not;

	EHL_FIELD("Persisted": true)
	int m_saved_effects = 0;
	//	Vector m_vecOrigOrigin;

	EHL_FIELD("Persisted": true)
	bool m_interruptable = false;
};
