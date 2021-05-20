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

constexpr int SF_DOOR_ROTATE_Y = 0;
constexpr int SF_DOOR_START_OPEN = 1;
constexpr int SF_DOOR_ROTATE_BACKWARDS = 2;
constexpr int SF_DOOR_PASSABLE = 8;
constexpr int SF_DOOR_ONEWAY = 16;
constexpr int SF_DOOR_NO_AUTO_RETURN = 32;
constexpr int SF_DOOR_ROTATE_Z = 64;
constexpr int SF_DOOR_ROTATE_X = 128;
constexpr int SF_DOOR_USE_ONLY = 256;	// door must be opened by player's use button.
constexpr int SF_DOOR_NOMONSTERS = 512;	// Monster can't open
constexpr int SF_DOOR_SILENT = 0x80000000;

constexpr int SF_ITEM_USE_ONLY = 256; //  ITEM_USE_ONLY = BUTTON_USE_ONLY = DOOR_USE_ONLY!!! 

/**
*	@details if two doors touch, they are assumed to be connected and operate as a unit.
*	TOGGLE causes the door to wait in both the start and end states for a trigger event.
*	START_OPEN causes the door to move to its destination when spawned, and operate in reverse.
*	It is used to temporarily or permanently close off an area when triggered (not useful for touch or takedamage doors).
*	"movedir"        determines the opening direction
*	"targetname"	if set, no touch function will be set and a remote button or trigger field activates the door.
*	"speed"         movement speed (100 default)
*	"wait"          wait before returning (0 default, -1 = never return)
*	"lip"           lip remaining at end of move (0 default)
*	"dmg"           damage to inflict when blocked (0 default)
*/
class EHL_CLASS() CBaseDoor : public CBaseToggle
{
public:
	void Spawn() override;
	void Precache() override;
	void KeyValue(KeyValueData* pkvd) override;
	void Use(const UseInfo& info) override;
	void Blocked(CBaseEntity* pOther) override;


	int	ObjectCaps() override
	{
		if (pev->spawnflags & SF_ITEM_USE_ONLY)
			return (CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_IMPULSE_USE;
		else
			return (CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION);
	}
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];

	/**
	*	@brief used to selectivly override defaults
	*	@details Doors not tied to anything (e.g. button, another door) can be touched, to make them activate.
	*/
	void EXPORT DoorTouch(CBaseEntity* pOther);

	// local functions
	/**
	*	@brief Causes the door to "do its thing", i.e. start moving, and cascade activation.
	*/
	bool DoorActivate();

	/**
	*	@brief Starts the door going to its "up" position (simply m_vecPosition2)
	*/
	void EXPORT DoorGoUp();

	/**
	*	@brief Starts the door going to its "down" position (simply m_vecPosition1).
	*/
	void EXPORT DoorGoDown();

	/**
	*	@brief The door has reached the "up" position. Either go back down, or wait for another activation.
	*/
	void EXPORT DoorHitTop();

	/**
	*	@brief The door has reached the "down" position. Back to quiescence.
	*/
	void EXPORT DoorHitBottom();

	byte m_bHealthValue = 0;//!< some doors are medi-kit doors, they give players health

	byte m_bMoveSnd = 0;		//!< sound a door makes while moving
	byte m_bStopSnd = 0;		//!< sound a door makes when it stops

	locksound_t m_ls;			//!< door lock sounds

	byte m_bLockedSound = 0;	//!< ordinals from entity selection
	byte m_bLockedSentence = 0;
	byte m_bUnlockedSound = 0;
	byte m_bUnlockedSentence = 0;
};
