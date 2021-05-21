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

#include "extdll.hpp"

class CBaseEntity;

/**
*	@brief sounds that doors and buttons make when locked/unlocked
*/
struct locksound_t
{
	string_t sLockedSound = iStringNull;		//!< sound a door makes when it's locked
	string_t sLockedSentence = iStringNull;		//!< sentence group played when door is locked
	string_t sUnlockedSound = iStringNull;		//!< sound a door makes when it's unlocked
	string_t sUnlockedSentence = iStringNull;	//!< sentence group played when door is unlocked

	int iLockedSentence = 0;		//!< which sentence in sentence group to play next
	int iUnlockedSentence = 0;		//!< which sentence in sentence group to play next

	float flwaitSound = 0;			//!< time delay between playing consecutive 'locked/unlocked' sounds
	float flwaitSentence = 0;		//!< time delay between playing consecutive sentences
	bool bEOFLocked = false;		//!< true if hit end of list of locked sentences
	bool bEOFUnlocked = false;		//!< true if hit end of list of unlocked sentences
};

/**
*	@brief play door or button locked or unlocked sounds.
*	@details pass in pointer to valid locksound struct.
*	NOTE: this routine is shared by doors and buttons
*	@param flocked if true, play 'door is locked' sound, otherwise play 'door is unlocked' sound
*/
void PlayLockSounds(CBaseEntity* entity, locksound_t* pls, int flocked, int fbutton); //TODO: use pass by ref

/**
*	@brief Button sound table. get string of button sound number
*	@details Also used by CBaseDoor to get 'touched' door lock/unlock sounds
*/
const char* ButtonSound(int sound);
