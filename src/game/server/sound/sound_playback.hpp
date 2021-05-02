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

class CBaseEntity;
struct TraceResult;

void UTIL_EmitAmbientSound(CBaseEntity* entity, const Vector& vecOrigin, const char* samp, float vol, float attenuation, int fFlags, int pitch);

/**
*	@brief for this entity, for the given sentence within the sentence group, stop the sentence.
*/
void SENTENCEG_Stop(CBaseEntity* entity, int isentenceg, int ipick);

/**
*	@brief given sentence group index, play random sentence for given entity.
*	@return ipick - which sentence was picked to play from the group.
*	@details Ipick is only needed if you plan on stopping the sound before playback is done (see SENTENCEG_Stop).
*/
int SENTENCEG_PlayRndI(CBaseEntity* entity, int isentenceg, float volume, float attenuation, int pitch, int flags = 0);

/**
*	@brief same as SENTENCEG_PlayRndI, but takes sentence group name instead of index
*/
int SENTENCEG_PlayRndSz(CBaseEntity* entity, const char* szrootname, float volume, float attenuation, int pitch, int flags = 0);

/**
*	@brief play sentences in sequential order from sentence group. Reset after last sentence.
*/
int SENTENCEG_PlaySequentialSz(CBaseEntity* entity, const char* szrootname, float volume, float attenuation, int pitch, int ipick, int freset, int flags = 0);

/**
*	@brief play a strike sound based on the texture that was hit by the attack traceline.
*	VecSrc/VecEnd are the original traceline endpoints used by the attacker
*	@param iBulletType the type of bullet that hit the texture.
*	@return volume of strike instrument (crowbar) to play
*/
float TEXTURETYPE_PlaySound(TraceResult* ptr, Vector vecSrc, Vector vecEnd, int iBulletType);

/**
*	@brief use EMIT_SOUND_DYN to set the pitch of a sound.
*
*	Pitch of 100 is no pitch shift. Pitch > 100 up to 255 is a higher pitch, pitch < 100 down to 1 is a lower pitch.
*	150 to 70 is the realistic range.
*	EMIT_SOUND_DYN with pitch != 100 should be used sparingly,
*	as it's not quite as fast as EMIT_SOUND (the pitchshift mixer is not native coded).
*/
void EMIT_SOUND_DYN(edict_t* entity, SoundChannel channel, const char* sample, float volume, float attenuation,
	int flags, int pitch);

/**
*	@brief play a specific sentence over the HEV suit speaker - just pass player entity, and !sentencename
*/
void EMIT_SOUND_SUIT(CBaseEntity* entity, const char* sample);

/**
*	@brief play a sentence, randomly selected from the passed in group id, over the HEV suit speaker
*/
void EMIT_GROUPID_SUIT(CBaseEntity* entity, int isentenceg);

/**
*	@brief play a sentence, randomly selected from the passed in groupname
*/
void EMIT_GROUPNAME_SUIT(CBaseEntity* entity, const char* groupname);

template<std::size_t Size>
void PRECACHE_SOUND_ARRAY(const char* (&a)[Size])
{
	for (std::size_t i = 0; i < ArraySize(a); i++)
	{
		PRECACHE_SOUND(a[i]);
	}
}

template<std::size_t Size>
const char* RANDOM_SOUND_ARRAY(const char* (&array)[Size])
{
	return array[RANDOM_LONG(0, ArraySize(array) - 1)];
}
