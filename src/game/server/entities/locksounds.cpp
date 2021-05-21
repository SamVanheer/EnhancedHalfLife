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

#include "locksounds.hpp"

constexpr int DOOR_SENTENCEWAIT = 6;
constexpr int DOOR_SOUNDWAIT = 3;
constexpr float BUTTON_SOUNDWAIT = 0.5;

void PlayLockSounds(CBaseEntity* entity, locksound_t* pls, int flocked, int fbutton)
{
	// LOCKED SOUND

	// CONSIDER: consolidate the locksound_t struct (all entries are duplicates for lock/unlock)
	// CONSIDER: and condense this code.
	float flsoundwait;

	if (fbutton)
		flsoundwait = BUTTON_SOUNDWAIT;
	else
		flsoundwait = DOOR_SOUNDWAIT;

	if (flocked)
	{
		const bool fplaysound = (!IsStringNull(pls->sLockedSound) && gpGlobals->time > pls->flwaitSound);
		const bool fplaysentence = (!IsStringNull(pls->sLockedSentence) && !pls->bEOFLocked && gpGlobals->time > pls->flwaitSentence);
		float fvol;

		if (fplaysound && fplaysentence)
			fvol = 0.25;
		else
			fvol = 1.0;

		// if there is a locked sound, and we've debounced, play sound
		if (fplaysound)
		{
			// play 'door locked' sound
			entity->EmitSound(SoundChannel::Item, STRING(pls->sLockedSound), fvol);
			pls->flwaitSound = gpGlobals->time + flsoundwait;
		}

		// if there is a sentence, we've not played all in list, and we've debounced, play sound
		if (fplaysentence)
		{
			// play next 'door locked' sentence in group
			const int iprev = pls->iLockedSentence;

			pls->iLockedSentence = SENTENCEG_PlaySequentialSz(entity, STRING(pls->sLockedSentence),
				0.85, ATTN_NORM, PITCH_NORM, pls->iLockedSentence, false);
			pls->iUnlockedSentence = 0;

			// make sure we don't keep calling last sentence in list
			pls->bEOFLocked = (iprev == pls->iLockedSentence);

			pls->flwaitSentence = gpGlobals->time + DOOR_SENTENCEWAIT;
		}
	}
	else
	{
		// UNLOCKED SOUND

		const bool fplaysound = (!IsStringNull(pls->sUnlockedSound) && gpGlobals->time > pls->flwaitSound);
		const bool fplaysentence = (!IsStringNull(pls->sUnlockedSentence) && !pls->bEOFUnlocked && gpGlobals->time > pls->flwaitSentence);
		float fvol;

		// if playing both sentence and sound, lower sound volume so we hear sentence
		if (fplaysound && fplaysentence)
			fvol = 0.25;
		else
			fvol = 1.0;

		// play 'door unlocked' sound if set
		if (fplaysound)
		{
			entity->EmitSound(SoundChannel::Item, STRING(pls->sUnlockedSound), fvol);
			pls->flwaitSound = gpGlobals->time + flsoundwait;
		}

		// play next 'door unlocked' sentence in group
		if (fplaysentence)
		{
			const int iprev = pls->iUnlockedSentence;

			pls->iUnlockedSentence = SENTENCEG_PlaySequentialSz(entity, STRING(pls->sUnlockedSentence),
				0.85, ATTN_NORM, PITCH_NORM, pls->iUnlockedSentence, false);
			pls->iLockedSentence = 0;

			// make sure we don't keep calling last sentence in list
			pls->bEOFUnlocked = (iprev == pls->iUnlockedSentence);
			pls->flwaitSentence = gpGlobals->time + DOOR_SENTENCEWAIT;
		}
	}
}

const char* ButtonSound(int sound)
{
	switch (sound)
	{
	case 0: return "common/null.wav";
	case 1: return "buttons/button1.wav";
	case 2: return "buttons/button2.wav";
	case 3: return "buttons/button3.wav";
	case 4: return "buttons/button4.wav";
	case 5: return "buttons/button5.wav";
	case 6: return "buttons/button6.wav";
	case 7: return "buttons/button7.wav";
	case 8: return "buttons/button8.wav";
	case 9: return "buttons/button9.wav";
	case 10: return "buttons/button10.wav";
	case 11: return "buttons/button11.wav";
	case 12: return "buttons/latchlocked1.wav";
	case 13: return "buttons/latchunlocked1.wav";
	case 14: return "buttons/lightswitch2.wav";

		// next 6 slots reserved for any additional sliding button sounds we may add

	case 21: return "buttons/lever1.wav";
	case 22: return "buttons/lever2.wav";
	case 23: return "buttons/lever3.wav";
	case 24: return "buttons/lever4.wav";
	case 25: return "buttons/lever5.wav";

	default:return "buttons/button9.wav";
	}
}
