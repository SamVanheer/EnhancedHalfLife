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

constexpr int	MAX_WORLD_SOUNDS = 64;		//!< maximum number of sounds handled by the world at one time.

constexpr int bits_SOUND_NONE = 0;
constexpr int bits_SOUND_COMBAT = 1 << 0;	//!< gunshots, explosions
constexpr int bits_SOUND_WORLD = 1 << 1;	//!< door opening/closing, glass breaking
constexpr int bits_SOUND_PLAYER = 1 << 2;	//!< all noises generated by player. walking, shooting, falling, splashing
constexpr int bits_SOUND_CARCASS = 1 << 3;	//!< dead body
constexpr int bits_SOUND_MEAT = 1 << 4;		//!< gib or pork chop
constexpr int bits_SOUND_DANGER = 1 << 5;	//!< pending danger. Grenade that is about to explode, explosive barrel that is damaged, falling crate
constexpr int bits_SOUND_GARBAGE = 1 << 6;	//!< trash cans, banana peels, old fast food bags.

constexpr int bits_ALL_SOUNDS = 0xFFFFFFFF;

constexpr int SOUNDLIST_EMPTY = -1;

constexpr int SOUNDLISTTYPE_FREE = 1;		//!< identifiers passed to functions that can operate on either list, to indicate which list to operate on.
constexpr int SOUNDLISTTYPE_ACTIVE = 2;

constexpr int SOUND_NEVER_EXPIRE = -1;		//!< with this set as a sound's ExpireTime, the sound will never expire.

/**
*	@brief an instance of a sound in the world.
*/
class CSound
{
public:
	/**
	*	@brief zeroes all fields for a sound
	*/
	void	Clear();

	/**
	*	@brief clears the volume, origin, and type for a sound, but doesn't expire or unlink it. 
	*/
	void	Reset();

	Vector	m_vecOrigin;	//!< sound's location in space
	int		m_iType;		//!< what type of sound this is
	int		m_iVolume;		//!< how loud the sound is
	float	m_flExpireTime;	//!< when the sound should be purged from the list
	int		m_iNext;		//!< index of next sound in this list ( Active or Free )
	int		m_iNextAudible;	//!< temporary link that monsters use to build a list of audible sounds

	bool	IsSound();
	bool	IsScent();
};

/**
*	@brief a single instance of this entity spawns when the world spawns.
*	The SoundEnt's job is to update the world's Free and Active sound lists.
*/
class CSoundEnt : public CBaseEntity
{
public:

	void Precache() override;
	void Spawn() override;

	/**
	*	@brief at interval, the entire active sound list is checked for sounds
	*	that have ExpireTimes less than or equal to the current world time,
	*	and these sounds are deallocated.
	*/
	void Think() override;

	/**
	*	@brief clears all sounds and moves them into the free sound list.
	*/
	void Initialize();

	/**
	*	@brief Allocates a free sound and fills it with sound info.
	*/
	static void		InsertSound(int iType, const Vector& vecOrigin, int iVolume, float flDuration);

	/**
	*	@brief clears the passed active sound and moves it to the top of the free list.
	*	TAKE CARE to only call this function for sounds in the Active list!!
	*/
	static void		FreeSound(int iSound, int iPrevious);
	static int		ActiveList();// return the head of the active list
	static int		FreeList();// return the head of the free list
	static CSound* SoundPointerForIndex(int iIndex);// return a pointer for this index in the sound list

	/**
	*	@brief Clients are numbered from 1 to MAXCLIENTS, but the client reserved sounds in the soundlist are from 0 to MAXCLIENTS - 1,
	*	so this function ensures that a client gets the proper index to his reserved sound in the soundlist.
	*/
	static int		ClientSoundIndex(edict_t* pClient);

	bool	IsEmpty() { return m_iActiveSound == SOUNDLIST_EMPTY; }

	/**
	*	@brief returns the number of sounds in the desired sound list.
	*/
	int		SoundsInList(int iListType);

	/**
	*	@brief moves a sound from the Free list to the Active list returns the index of the alloc'd sound
	*/
	int		AllocSound();
	int		ObjectCaps() override { return FCAP_DONT_SAVE; }

	int		m_iFreeSound;	//!< index of the first sound in the free sound list
	int		m_iActiveSound; //!< indes of the first sound in the active sound list
	int		m_cLastActiveSounds; //!< keeps track of the number of active sounds at the last update. (for diagnostic work)
	bool	m_fShowReport; //!< if true, dump information about free/active sounds.

private:
	CSound		m_SoundPool[MAX_WORLD_SOUNDS];
};

inline CSoundEnt* pSoundEnt = nullptr;
