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

#include "CBaseToggle.hpp"

#include "locksounds.hpp"

/**
*	@brief Generic Button
*	@details When a button is touched, it moves some distance in the direction of its angle,
*	triggers all of its targets, waits some time, then returns to its original position where it can be triggered again.
*	"movedir"	determines the opening direction
*	"target"	all entities with a matching targetname will be used
*	"speed"		override the default 40 speed
*	"wait"		override the default 1 second wait (-1 = never return)
*	"lip"		override the default 4 unit lip remaining at end of move
*	"health"	if set, the button must be killed instead of touched
*/
class CBaseButton : public CBaseToggle
{
public:
	void Spawn() override;
	void Precache() override;
	void KeyValue(KeyValueData* pkvd) override;

	/**
	*	@brief Starts the button moving "in/up".
	*/
	void ButtonActivate();

	/**
	*	@brief Touching a button simply "activates" it.
	*/
	void EXPORT ButtonTouch(CBaseEntity* pOther);
	void EXPORT ButtonSpark();

	/**
	*	@brief Button has reached the "in/up" position.  Activate its "targets", and pause before "popping out".
	*/
	void EXPORT TriggerAndWait();

	/**
	*	@brief Starts the button moving "out/down".
	*/
	void EXPORT ButtonReturn();

	/**
	*	@brief Button has returned to start state. Quiesce it.
	*/
	void EXPORT ButtonBackHome();
	void EXPORT ButtonUse(const UseInfo& info);
	bool TakeDamage(const TakeDamageInfo& info) override;
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	enum class ButtonCode
	{
		Nothing,
		Activate,
		Return
	};

	ButtonCode ButtonResponseToTouch();

	static TYPEDESCRIPTION m_SaveData[];
	/**
	*	@brief Buttons that don't take damage can be IMPULSE used
	*/
	int	ObjectCaps() override { return (CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | (pev->takedamage ? 0 : FCAP_IMPULSE_USE); }

	bool m_fStayPushed = false;		//!< button stays pushed in until touched again?
	bool m_fRotating = false;		//!< a rotating button?  default is a sliding button.

	locksound_t m_ls;			//!< door lock sounds

	// ordinals from entity selection
	byte m_bLockedSound = 0;
	byte m_bLockedSentence = 0;
	byte m_bUnlockedSound = 0;
	byte m_bUnlockedSentence = 0;
	int m_sounds = 0;
};
