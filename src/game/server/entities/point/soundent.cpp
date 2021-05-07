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

#include "soundent.h"

LINK_ENTITY_TO_CLASS(soundent, CSoundEnt);

void CSound::Clear()
{
	m_vecOrigin = vec3_origin;
	m_iType = 0;
	m_iVolume = 0;
	m_flExpireTime = 0;
	m_iNext = SOUNDLIST_EMPTY;
	m_iNextAudible = 0;
}

void CSound::Reset()
{
	m_vecOrigin = vec3_origin;
	m_iType = 0;
	m_iVolume = 0;
	m_iNext = SOUNDLIST_EMPTY;
}

bool CSound::IsSound()
{
	if (m_iType & (bits_SOUND_COMBAT | bits_SOUND_WORLD | bits_SOUND_PLAYER | bits_SOUND_DANGER))
	{
		return true;
	}

	return false;
}

bool CSound::IsScent()
{
	if (m_iType & (bits_SOUND_CARCASS | bits_SOUND_MEAT | bits_SOUND_GARBAGE))
	{
		return true;
	}

	return false;
}

void CSoundEnt::Spawn()
{
	SetSolidType(Solid::Not);
	Initialize();

	pev->nextthink = gpGlobals->time + 1;
}

void CSoundEnt::Think()
{
	pev->nextthink = gpGlobals->time + 0.3;// how often to check the sound list.

	int iPreviousSound = SOUNDLIST_EMPTY;
	int iSound = m_iActiveSound;

	while (iSound != SOUNDLIST_EMPTY)
	{
		if (m_SoundPool[iSound].m_flExpireTime <= gpGlobals->time && m_SoundPool[iSound].m_flExpireTime != SOUND_NEVER_EXPIRE)
		{
			int iNext = m_SoundPool[iSound].m_iNext;

			// move this sound back into the free list
			FreeSound(iSound, iPreviousSound);

			iSound = iNext;
		}
		else
		{
			iPreviousSound = iSound;
			iSound = m_SoundPool[iSound].m_iNext;
		}
	}

	if (m_fShowReport)
	{
		ALERT(at_aiconsole, "Soundlist: %d / %d  (%d)\n", SoundsInList(SOUNDLISTTYPE_ACTIVE), SoundsInList(SOUNDLISTTYPE_FREE), SoundsInList(SOUNDLISTTYPE_ACTIVE) - m_cLastActiveSounds);
		m_cLastActiveSounds = SoundsInList(SOUNDLISTTYPE_ACTIVE);
	}
}

void CSoundEnt::Precache()
{
}

void CSoundEnt::FreeSound(int iSound, int iPrevious)
{
	if (!pSoundEnt)
	{
		// no sound ent!
		return;
	}

	if (iPrevious != SOUNDLIST_EMPTY)
	{
		// iSound is not the head of the active list, so
		// must fix the index for the Previous sound
//		pSoundEnt->m_SoundPool[ iPrevious ].m_iNext = m_SoundPool[ iSound ].m_iNext;
		pSoundEnt->m_SoundPool[iPrevious].m_iNext = pSoundEnt->m_SoundPool[iSound].m_iNext;
	}
	else
	{
		// the sound we're freeing IS the head of the active list.
		pSoundEnt->m_iActiveSound = pSoundEnt->m_SoundPool[iSound].m_iNext;
	}

	// make iSound the head of the Free list.
	pSoundEnt->m_SoundPool[iSound].m_iNext = pSoundEnt->m_iFreeSound;
	pSoundEnt->m_iFreeSound = iSound;
}

int CSoundEnt::AllocSound()
{
	if (m_iFreeSound == SOUNDLIST_EMPTY)
	{
		// no free sound!
		ALERT(at_console, "Free Sound List is full!\n");
		return SOUNDLIST_EMPTY;
	}

	// there is at least one sound available, so move it to the
	// Active sound list, and return its SoundPool index.

	const int iNewSound = m_iFreeSound;// copy the index of the next free sound

	m_iFreeSound = m_SoundPool[m_iFreeSound].m_iNext;// move the index down into the free list. 

	m_SoundPool[iNewSound].m_iNext = m_iActiveSound;// point the new sound at the top of the active list.

	m_iActiveSound = iNewSound;// now make the new sound the top of the active list. You're done.

	return iNewSound;
}

void CSoundEnt::InsertSound(int iType, const Vector& vecOrigin, int iVolume, float flDuration)
{
	if (!pSoundEnt)
	{
		// no sound ent!
		return;
	}

	const int iThisSound = pSoundEnt->AllocSound();

	if (iThisSound == SOUNDLIST_EMPTY)
	{
		ALERT(at_console, "Could not AllocSound() for InsertSound() (DLL)\n");
		return;
	}

	pSoundEnt->m_SoundPool[iThisSound].m_vecOrigin = vecOrigin;
	pSoundEnt->m_SoundPool[iThisSound].m_iType = iType;
	pSoundEnt->m_SoundPool[iThisSound].m_iVolume = iVolume;
	pSoundEnt->m_SoundPool[iThisSound].m_flExpireTime = gpGlobals->time + flDuration;
}

void CSoundEnt::Initialize()
{
	m_cLastActiveSounds = 0;
	m_iFreeSound = 0;
	m_iActiveSound = SOUNDLIST_EMPTY;

	{
		int i;
		for (i = 0; i < MAX_WORLD_SOUNDS; i++)
		{// clear all sounds, and link them into the free sound list.
			m_SoundPool[i].Clear();
			m_SoundPool[i].m_iNext = i + 1;
		}

		m_SoundPool[i - 1].m_iNext = SOUNDLIST_EMPTY;// terminate the list here.
	}

	// now reserve enough sounds for each client
	for (int i = 0; i < gpGlobals->maxClients; i++)
	{
		const int iSound = pSoundEnt->AllocSound();

		if (iSound == SOUNDLIST_EMPTY)
		{
			ALERT(at_console, "Could not AllocSound() for Client Reserve! (DLL)\n");
			return;
		}

		pSoundEnt->m_SoundPool[iSound].m_flExpireTime = SOUND_NEVER_EXPIRE;
	}

	if (CVAR_GET_FLOAT("displaysoundlist") == 1)
	{
		m_fShowReport = true;
	}
	else
	{
		m_fShowReport = false;
	}
}

int CSoundEnt::SoundsInList(int iListType)
{
	int iThisSound;

	if (iListType == SOUNDLISTTYPE_FREE)
	{
		iThisSound = m_iFreeSound;
	}
	else if (iListType == SOUNDLISTTYPE_ACTIVE)
	{
		iThisSound = m_iActiveSound;
	}
	else
	{
		ALERT(at_console, "Unknown Sound List Type!\n");
		return 0;
	}

	if (iThisSound == SOUNDLIST_EMPTY)
	{
		return 0;
	}

	int i = 0;

	while (iThisSound != SOUNDLIST_EMPTY)
	{
		i++;

		iThisSound = m_SoundPool[iThisSound].m_iNext;
	}

	return i;
}

int CSoundEnt::ActiveList()
{
	if (!pSoundEnt)
	{
		return SOUNDLIST_EMPTY;
	}

	return pSoundEnt->m_iActiveSound;
}

int CSoundEnt::FreeList()
{
	if (!pSoundEnt)
	{
		return SOUNDLIST_EMPTY;
	}

	return pSoundEnt->m_iFreeSound;
}

CSound* CSoundEnt::SoundPointerForIndex(int iIndex)
{
	if (!pSoundEnt)
	{
		return nullptr;
	}

	if (iIndex > (MAX_WORLD_SOUNDS - 1))
	{
		ALERT(at_console, "SoundPointerForIndex() - Index too large!\n");
		return nullptr;
	}

	if (iIndex < 0)
	{
		ALERT(at_console, "SoundPointerForIndex() - Index < 0!\n");
		return nullptr;
	}

	return &pSoundEnt->m_SoundPool[iIndex];
}

int CSoundEnt::ClientSoundIndex(edict_t* pClient)
{
	const int iReturn = ENTINDEX(pClient) - 1;

#ifdef _DEBUG
	if (iReturn < 0 || iReturn > gpGlobals->maxClients)
	{
		ALERT(at_console, "** ClientSoundIndex returning a bogus value! **\n");
	}
#endif // _DEBUG

	return iReturn;
}
