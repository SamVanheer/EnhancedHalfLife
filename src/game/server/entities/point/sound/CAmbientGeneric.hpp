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
#include "CAmbientGeneric.generated.hpp"

/**
*	@brief runtime pitch shift and volume fadein/out structure
*	NOTE: IF YOU CHANGE THIS STRUCT YOU MUST CHANGE THE SAVE/RESTORE VERSION NUMBER
*	SEE BELOW (in the typedescription for the class)
*/
struct dynpitchvol_t
{
	// NOTE: do not change the order of these parameters 
	// NOTE: unless you also change order of rgdpvpreset array elements!
	int preset = 0;

	int pitchrun = 0;		// pitch shift % when sound is running 0 - 255
	int pitchstart = 0;		// pitch shift % when sound stops or starts 0 - 255
	int spinup = 0;			// spinup time 0 - 100
	int spindown = 0;		// spindown time 0 - 100

	int volrun = 0;			// volume change % when sound is running 0 - 10
	int volstart = 0;		// volume change % when sound stops or starts 0 - 10
	int fadein = 0;			// volume fade in time 0 - 100
	int fadeout = 0;		// volume fade out time 0 - 100

							// Low Frequency Oscillator
	int	lfotype = 0;		// 0) off 1) square 2) triangle 3) random
	int lforate = 0;		// 0 - 1000, how fast lfo osciallates

	int lfomodpitch = 0;	// 0-100 mod of current pitch. 0 is off.
	int lfomodvol = 0;		// 0-100 mod of current volume. 0 is off.

	int cspinup = 0;		// each trigger hit increments counter and spinup pitch


	int	cspincount = 0;

	int pitch = 0;
	int spinupsav = 0;
	int spindownsav = 0;
	int pitchfrac = 0;

	int vol = 0;
	int fadeinsav = 0;
	int fadeoutsav = 0;
	int volfrac = 0;

	int	lfofrac = 0;
	int	lfomult = 0;
};

constexpr int AMBIENT_SOUND_STATIC = 0;	//!< medium radius attenuation
constexpr int AMBIENT_SOUND_EVERYWHERE = 1;
constexpr int AMBIENT_SOUND_SMALLRADIUS = 2;
constexpr int AMBIENT_SOUND_MEDIUMRADIUS = 4;
constexpr int AMBIENT_SOUND_LARGERADIUS = 8;
constexpr int AMBIENT_SOUND_START_SILENT = 16;
constexpr int AMBIENT_SOUND_NOT_LOOPING = 32;

constexpr int LFO_SQUARE = 1;
constexpr int LFO_TRIANGLE = 2;
constexpr int LFO_RANDOM = 3;

/**
*	@brief Generic ambient sound
*	general-purpose user-defined static sound
*/
class EHL_CLASS() CAmbientGeneric : public CBaseEntity
{
	EHL_GENERATED_BODY()

public:
	void KeyValue(KeyValueData* pkvd) override;
	void Spawn() override;
	void Precache() override;

	/**
	*	@brief turns an ambient sound on or off.
	*	If the ambient is a looping sound, mark sound as active (m_fActive) if it's playing, innactive if not.
	*	If the sound is not a looping sound, never mark it as active.
	*/
	void EXPORT ToggleUse(const UseInfo& info);

	/**
	*	@brief Think at 5hz if we are dynamically modifying pitch or volume of the playing sound.
	*	This function will ramp pitch and/or volume up or down, modify pitch/volume with lfo if active.
	*/
	void EXPORT RampThink();

	/**
	*	@brief Init all ramp params in preparation to play a new sound
	*/
	void InitModulationParms();

	int	ObjectCaps() override { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION); }

	EHL_FIELD(Persisted)
	float m_flAttenuation = 0;		// attenuation value

	// HACKHACK - This is not really in the spirit of the save/restore design, but save this
	// out as a binary data block.  If the dynpitchvol_t is changed, old saved games will NOT
	// load these correctly, so bump the save/restore version if you change the size of the struct
	// The right way to do this is to split the input parms (read in keyvalue) into members and re-init this
	// struct in Precache(), but it's unlikely that the struct will change, so it's not worth the time right now.
	EHL_FIELD(Persisted)
	dynpitchvol_t m_dpv;

	EHL_FIELD(Persisted)
	bool m_fActive = false;		// only true when the entity is playing a looping sound

	EHL_FIELD(Persisted)
	bool m_fLooping = false;	// true when the sound played will loop
};
