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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "talkmonster.h"

/**
*	@brief runtime pitch shift and volume fadein/out structure
*	NOTE: IF YOU CHANGE THIS STRUCT YOU MUST CHANGE THE SAVE/RESTORE VERSION NUMBER
*	SEE BELOW (in the typedescription for the class)
*/
struct dynpitchvol_t
{
	// NOTE: do not change the order of these parameters 
	// NOTE: unless you also change order of rgdpvpreset array elements!
	int preset;

	int pitchrun;		// pitch shift % when sound is running 0 - 255
	int pitchstart;		// pitch shift % when sound stops or starts 0 - 255
	int spinup;			// spinup time 0 - 100
	int spindown;		// spindown time 0 - 100

	int volrun;			// volume change % when sound is running 0 - 10
	int volstart;		// volume change % when sound stops or starts 0 - 10
	int fadein;			// volume fade in time 0 - 100
	int fadeout;		// volume fade out time 0 - 100

						// Low Frequency Oscillator
	int	lfotype;		// 0) off 1) square 2) triangle 3) random
	int lforate;		// 0 - 1000, how fast lfo osciallates

	int lfomodpitch;	// 0-100 mod of current pitch. 0 is off.
	int lfomodvol;		// 0-100 mod of current volume. 0 is off.

	int cspinup;		// each trigger hit increments counter and spinup pitch


	int	cspincount;

	int pitch;
	int spinupsav;
	int spindownsav;
	int pitchfrac;

	int vol;
	int fadeinsav;
	int fadeoutsav;
	int volfrac;

	int	lfofrac;
	int	lfomult;
};

constexpr int CDPVPRESETMAX = 27;

/**
*	@brief presets for runtime pitch and vol modulation of ambient sounds
*/
constexpr dynpitchvol_t rgdpvpreset[CDPVPRESETMAX] =
{
	// pitch	pstart	spinup	spindwn	volrun	volstrt	fadein	fadeout	lfotype	lforate	modptch modvol	cspnup		
	{1,	255,	 75,	95,		95,		10,		1,		50,		95, 	0,		0,		0,		0,		0,		0,0,0,0,0,0,0,0,0,0},
	{2,	255,	 85,	70,		88,		10,		1,		20,		88,		0,		0,		0,		0,		0,		0,0,0,0,0,0,0,0,0,0},
	{3,	255,	100,	50,		75,		10,		1,		10,		75,		0,		0,		0,		0,		0,		0,0,0,0,0,0,0,0,0,0},
	{4,	100,	100,	0,		0,		10,		1,		90,		90,		0,		0,		0,		0,		0,		0,0,0,0,0,0,0,0,0,0},
	{5,	100,	100,	0,		0,		10,		1,		80,		80,		0,		0,		0,		0,		0,		0,0,0,0,0,0,0,0,0,0},
	{6,	100,	100,	0,		0,		10,		1,		50,		70,		0,		0,		0,		0,		0,		0,0,0,0,0,0,0,0,0,0},
	{7,	100,	100,	0,		0,		 5,		1,		40,		50,		1,		50,		0,		10,		0,		0,0,0,0,0,0,0,0,0,0},
	{8,	100,	100,	0,		0,		 5,		1,		40,		50,		1,		150,	0,		10,		0,		0,0,0,0,0,0,0,0,0,0},
	{9,	100,	100,	0,		0,		 5,		1,		40,		50,		1,		750,	0,		10,		0,		0,0,0,0,0,0,0,0,0,0},
	{10,128,	100,	50,		75,		10,		1,		30,		40,		2,		 8,		20,		0,		0,		0,0,0,0,0,0,0,0,0,0},
	{11,128,	100,	50,		75,		10,		1,		30,		40,		2,		25,		20,		0,		0,		0,0,0,0,0,0,0,0,0,0},
	{12,128,	100,	50,		75,		10,		1,		30,		40,		2,		70,		20,		0,		0,		0,0,0,0,0,0,0,0,0,0},
	{13,50,		 50,	0,		0,		10,		1,		20,		50,		0,		0,		0,		0,		0,		0,0,0,0,0,0,0,0,0,0},
	{14,70,		 70,	0,		0,		10,		1,		20,		50,		0,		0,		0,		0,		0,		0,0,0,0,0,0,0,0,0,0},
	{15,90,		 90,	0,		0,		10,		1,		20,		50,		0,		0,		0,		0,		0,		0,0,0,0,0,0,0,0,0,0},
	{16,120,	120,	0,		0,		10,		1,		20,		50,		0,		0,		0,		0,		0,		0,0,0,0,0,0,0,0,0,0},
	{17,180,	180,	0,		0,		10,		1,		20,		50,		0,		0,		0,		0,		0,		0,0,0,0,0,0,0,0,0,0},
	{18,255,	255,	0,		0,		10,		1,		20,		50,		0,		0,		0,		0,		0,		0,0,0,0,0,0,0,0,0,0},
	{19,200,	 75,	90,		90,		10,		1,		50,		90,		2,		100,	20,		0,		0,		0,0,0,0,0,0,0,0,0,0},
	{20,255,	 75,	97,		90,		10,		1,		50,		90,		1,		40,		50,		0,		0,		0,0,0,0,0,0,0,0,0,0},
	{21,100,	100,	0,		0,		10,		1,		30,		50,		3,		15,		20,		0,		0,		0,0,0,0,0,0,0,0,0,0},
	{22,160,	160,	0,		0,		10,		1,		50,		50,		3,		500,	25,		0,		0,		0,0,0,0,0,0,0,0,0,0},
	{23,255,	 75,	88,		0,		10,		1,		40,		0,		0,		0,		0,		0,		5,		0,0,0,0,0,0,0,0,0,0},
	{24,200,	 20,	95,	    70,		10,		1,		70,		70,		3,		20,		50,		0,		0,		0,0,0,0,0,0,0,0,0,0},
	{25,180,	100,	50,		60,		10,		1,		40,		60,		2,		90,		100,	100,	0,		0,0,0,0,0,0,0,0,0,0},
	{26,60,		 60,	0,		0,		10,		1,		40,		70,		3,		80,		20,		50,		0,		0,0,0,0,0,0,0,0,0,0},
	{27,128,	 90,	10,		10,		10,		1,		20,		40,		1,		5,		10,		20,		0,		0,0,0,0,0,0,0,0,0,0}
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
class CAmbientGeneric : public CBaseEntity
{
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

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];
	int	ObjectCaps() override { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION); }

	float m_flAttenuation;		// attenuation value
	dynpitchvol_t m_dpv;

	bool	m_fActive;	// only true when the entity is playing a looping sound
	bool	m_fLooping;	// true when the sound played will loop
};

LINK_ENTITY_TO_CLASS(ambient_generic, CAmbientGeneric);

TYPEDESCRIPTION	CAmbientGeneric::m_SaveData[] =
{
	DEFINE_FIELD(CAmbientGeneric, m_flAttenuation, FIELD_FLOAT),
	DEFINE_FIELD(CAmbientGeneric, m_fActive, FIELD_BOOLEAN),
	DEFINE_FIELD(CAmbientGeneric, m_fLooping, FIELD_BOOLEAN),

	// HACKHACK - This is not really in the spirit of the save/restore design, but save this
	// out as a binary data block.  If the dynpitchvol_t is changed, old saved games will NOT
	// load these correctly, so bump the save/restore version if you change the size of the struct
	// The right way to do this is to split the input parms (read in keyvalue) into members and re-init this
	// struct in Precache(), but it's unlikely that the struct will change, so it's not worth the time right now.
	DEFINE_ARRAY(CAmbientGeneric, m_dpv, FIELD_CHARACTER, sizeof(dynpitchvol_t)),
};

IMPLEMENT_SAVERESTORE(CAmbientGeneric, CBaseEntity);

void CAmbientGeneric::Spawn()
{
	/*
			-1 : "Default"
			0  : "Everywhere"
			200 : "Small Radius"
			125 : "Medium Radius"
			80  : "Large Radius"
	*/

	if (IsBitSet(pev->spawnflags, AMBIENT_SOUND_EVERYWHERE))
	{
		m_flAttenuation = ATTN_NONE;
	}
	else if (IsBitSet(pev->spawnflags, AMBIENT_SOUND_SMALLRADIUS))
	{
		m_flAttenuation = ATTN_IDLE;
	}
	else if (IsBitSet(pev->spawnflags, AMBIENT_SOUND_MEDIUMRADIUS))
	{
		m_flAttenuation = ATTN_STATIC;
	}
	else if (IsBitSet(pev->spawnflags, AMBIENT_SOUND_LARGERADIUS))
	{
		m_flAttenuation = ATTN_NORM;
	}
	else
	{// if the designer didn't set a sound attenuation, default to one.
		m_flAttenuation = ATTN_STATIC;
	}

	const char* szSoundFile = STRING(pev->message);

	if (IsStringNull(pev->message) || strlen(szSoundFile) < 1)
	{
		ALERT(at_error, "EMPTY AMBIENT AT: %f, %f, %f\n", pev->origin.x, pev->origin.y, pev->origin.z);
		pev->nextthink = gpGlobals->time + 0.1;
		SetThink(&CAmbientGeneric::SUB_Remove);
		return;
	}
	pev->solid = Solid::Not;
	pev->movetype = Movetype::None;

	// Set up think function for dynamic modification 
	// of ambient sound's pitch or volume. Don't
	// start thinking yet.

	SetThink(&CAmbientGeneric::RampThink);
	pev->nextthink = 0;

	// allow on/off switching via 'use' function.

	SetUse(&CAmbientGeneric::ToggleUse);

	m_fActive = false;

	m_fLooping = !IsBitSet(pev->spawnflags, AMBIENT_SOUND_NOT_LOOPING);

	Precache();
}

void CAmbientGeneric::Precache()
{
	const char* szSoundFile = STRING(pev->message);

	if (!IsStringNull(pev->message) && strlen(szSoundFile) > 1)
	{
		if (*szSoundFile != '!')
			PRECACHE_SOUND(szSoundFile);
	}
	// init all dynamic modulation parms
	InitModulationParms();

	if (!IsBitSet(pev->spawnflags, AMBIENT_SOUND_START_SILENT))
	{
		// start the sound ASAP
		if (m_fLooping)
			m_fActive = true;
	}
	if (m_fActive)
	{
		UTIL_EmitAmbientSound(edict(), pev->origin, szSoundFile,
			(m_dpv.vol * 0.01), m_flAttenuation, SND_SPAWNING, m_dpv.pitch);

		pev->nextthink = gpGlobals->time + 0.1;
	}
}

void CAmbientGeneric::RampThink()
{
	const char* szSoundFile = STRING(pev->message);
	int pitch = m_dpv.pitch;
	int vol = m_dpv.vol;
	int flags = 0;
	bool fChanged = false; // FALSE if pitch and vol remain unchanged this round

	if (!m_dpv.spinup && !m_dpv.spindown && !m_dpv.fadein && !m_dpv.fadeout && !m_dpv.lfotype)
		return;						// no ramps or lfo, stop thinking

	// ==============
	// pitch envelope
	// ==============
	if (m_dpv.spinup || m_dpv.spindown)
	{
		const int prev = m_dpv.pitchfrac >> 8;

		if (m_dpv.spinup > 0)
			m_dpv.pitchfrac += m_dpv.spinup;
		else if (m_dpv.spindown > 0)
			m_dpv.pitchfrac -= m_dpv.spindown;

		pitch = m_dpv.pitchfrac >> 8;

		if (pitch > m_dpv.pitchrun)
		{
			pitch = m_dpv.pitchrun;
			m_dpv.spinup = 0;				// done with ramp up
		}

		if (pitch < m_dpv.pitchstart)
		{
			pitch = m_dpv.pitchstart;
			m_dpv.spindown = 0;				// done with ramp down

			// shut sound off
			UTIL_EmitAmbientSound(edict(), pev->origin, szSoundFile,
				0, 0, SND_STOP, 0);

			// return without setting nextthink
			return;
		}

		if (pitch > 255) pitch = 255;
		if (pitch < 1) pitch = 1;

		m_dpv.pitch = pitch;

		fChanged |= (prev != pitch);
		flags |= SND_CHANGE_PITCH;
	}

	// ==================
	// amplitude envelope
	// ==================
	if (m_dpv.fadein || m_dpv.fadeout)
	{
		const int prev = m_dpv.volfrac >> 8;

		if (m_dpv.fadein > 0)
			m_dpv.volfrac += m_dpv.fadein;
		else if (m_dpv.fadeout > 0)
			m_dpv.volfrac -= m_dpv.fadeout;

		vol = m_dpv.volfrac >> 8;

		if (vol > m_dpv.volrun)
		{
			vol = m_dpv.volrun;
			m_dpv.fadein = 0;				// done with ramp up
		}

		if (vol < m_dpv.volstart)
		{
			vol = m_dpv.volstart;
			m_dpv.fadeout = 0;				// done with ramp down

			// shut sound off
			UTIL_EmitAmbientSound(edict(), pev->origin, szSoundFile,
				0, 0, SND_STOP, 0);

			// return without setting nextthink
			return;
		}

		if (vol > 100) vol = 100;
		if (vol < 1) vol = 1;

		m_dpv.vol = vol;

		fChanged |= (prev != vol);
		flags |= SND_CHANGE_VOL;
	}

	// ===================
	// pitch/amplitude LFO
	// ===================
	if (m_dpv.lfotype)
	{
		int pos;

		if (m_dpv.lfofrac > 0x6fffffff)
			m_dpv.lfofrac = 0;

		// update lfo, lfofrac/255 makes a triangle wave 0-255
		m_dpv.lfofrac += m_dpv.lforate;
		pos = m_dpv.lfofrac >> 8;

		if (m_dpv.lfofrac < 0)
		{
			m_dpv.lfofrac = 0;
			m_dpv.lforate = abs(m_dpv.lforate);
			pos = 0;
		}
		else if (pos > 255)
		{
			pos = 255;
			m_dpv.lfofrac = (255 << 8);
			m_dpv.lforate = -abs(m_dpv.lforate);
		}

		switch (m_dpv.lfotype)
		{
		case LFO_SQUARE:
			if (pos < 128)
				m_dpv.lfomult = 255;
			else
				m_dpv.lfomult = 0;

			break;
		case LFO_RANDOM:
			if (pos == 255)
				m_dpv.lfomult = RANDOM_LONG(0, 255);
			break;
		case LFO_TRIANGLE:
		default:
			m_dpv.lfomult = pos;
			break;
		}

		if (m_dpv.lfomodpitch)
		{
			const int prev = pitch;

			// pitch 0-255
			pitch += ((m_dpv.lfomult - 128) * m_dpv.lfomodpitch) / 100;

			if (pitch > 255) pitch = 255;
			if (pitch < 1) pitch = 1;


			fChanged |= (prev != pitch);
			flags |= SND_CHANGE_PITCH;
		}

		if (m_dpv.lfomodvol)
		{
			// vol 0-100
			const int prev = vol;

			vol += ((m_dpv.lfomult - 128) * m_dpv.lfomodvol) / 100;

			if (vol > 100) vol = 100;
			if (vol < 0) vol = 0;

			fChanged |= (prev != vol);
			flags |= SND_CHANGE_VOL;
		}

	}

	// Send update to playing sound only if we actually changed
	// pitch or volume in this routine.

	if (flags && fChanged)
	{
		if (pitch == PITCH_NORM)
			pitch = PITCH_NORM + 1; // don't send 'no pitch' !

		UTIL_EmitAmbientSound(edict(), pev->origin, szSoundFile,
			(vol * 0.01), m_flAttenuation, flags, pitch);
	}

	// update ramps at 5hz
	pev->nextthink = gpGlobals->time + 0.2;
}

void CAmbientGeneric::InitModulationParms()
{
	m_dpv.volrun = pev->health * 10;	// 0 - 100
	if (m_dpv.volrun > 100) m_dpv.volrun = 100;
	if (m_dpv.volrun < 0) m_dpv.volrun = 0;

	// get presets
	if (m_dpv.preset != 0 && m_dpv.preset <= CDPVPRESETMAX)
	{
		// load preset values
		m_dpv = rgdpvpreset[m_dpv.preset - 1];

		// fixup preset values, just like
		// fixups in KeyValue routine.
		if (m_dpv.spindown > 0)
			m_dpv.spindown = (101 - m_dpv.spindown) * 64;
		if (m_dpv.spinup > 0)
			m_dpv.spinup = (101 - m_dpv.spinup) * 64;

		m_dpv.volstart *= 10;
		m_dpv.volrun *= 10;

		if (m_dpv.fadein > 0)
			m_dpv.fadein = (101 - m_dpv.fadein) * 64;
		if (m_dpv.fadeout > 0)
			m_dpv.fadeout = (101 - m_dpv.fadeout) * 64;

		m_dpv.lforate *= 256;

		m_dpv.fadeinsav = m_dpv.fadein;
		m_dpv.fadeoutsav = m_dpv.fadeout;
		m_dpv.spinupsav = m_dpv.spinup;
		m_dpv.spindownsav = m_dpv.spindown;
	}

	m_dpv.fadein = m_dpv.fadeinsav;
	m_dpv.fadeout = 0;

	if (m_dpv.fadein)
		m_dpv.vol = m_dpv.volstart;
	else
		m_dpv.vol = m_dpv.volrun;

	m_dpv.spinup = m_dpv.spinupsav;
	m_dpv.spindown = 0;

	if (m_dpv.spinup)
		m_dpv.pitch = m_dpv.pitchstart;
	else
		m_dpv.pitch = m_dpv.pitchrun;

	if (m_dpv.pitch == 0)
		m_dpv.pitch = PITCH_NORM;

	m_dpv.pitchfrac = m_dpv.pitch << 8;
	m_dpv.volfrac = m_dpv.vol << 8;

	m_dpv.lfofrac = 0;
	m_dpv.lforate = abs(m_dpv.lforate);

	m_dpv.cspincount = 1;

	if (m_dpv.cspinup)
	{
		const int pitchinc = (255 - m_dpv.pitchstart) / m_dpv.cspinup;

		m_dpv.pitchrun = m_dpv.pitchstart + pitchinc;
		if (m_dpv.pitchrun > 255) m_dpv.pitchrun = 255;
	}

	if ((m_dpv.spinupsav || m_dpv.spindownsav || (m_dpv.lfotype && m_dpv.lfomodpitch))
		&& (m_dpv.pitch == PITCH_NORM))
		m_dpv.pitch = PITCH_NORM + 1; // must never send 'no pitch' as first pitch
									  // if we intend to pitch shift later!
}

void CAmbientGeneric::ToggleUse(const UseInfo& info)
{
	const char* szSoundFile = STRING(pev->message);

	if (info.GetUseType() != UseType::Toggle)
	{
		if ((m_fActive && info.GetUseType() == UseType::On) || (!m_fActive && info.GetUseType() == UseType::Off))
			return;
	}
	// Directly change pitch if arg passed. Only works if sound is already playing.

	if (info.GetUseType() == UseType::Set && m_fActive)		// Momentary buttons will pass down a float in here
	{
		float fraction = info.GetValue();

		if (fraction > 1.0)
			fraction = 1.0;
		if (fraction < 0.0)
			fraction = 0.01;

		m_dpv.pitch = fraction * 255;

		UTIL_EmitAmbientSound(edict(), pev->origin, szSoundFile,
			0, 0, SND_CHANGE_PITCH, m_dpv.pitch);

		return;
	}

	// Toggle

	// m_fActive is true only if a looping sound is playing.

	if (m_fActive)
	{// turn sound off

		if (m_dpv.cspinup)
		{
			// Don't actually shut off. Each toggle causes
			// incremental spinup to max pitch

			if (m_dpv.cspincount <= m_dpv.cspinup)
			{
				// start a new spinup
				m_dpv.cspincount++;

				const int pitchinc = (255 - m_dpv.pitchstart) / m_dpv.cspinup;

				m_dpv.spinup = m_dpv.spinupsav;
				m_dpv.spindown = 0;

				m_dpv.pitchrun = m_dpv.pitchstart + pitchinc * m_dpv.cspincount;
				if (m_dpv.pitchrun > 255) m_dpv.pitchrun = 255;

				pev->nextthink = gpGlobals->time + 0.1;
			}

		}
		else
		{
			m_fActive = false;

			// HACKHACK - this makes the code in Precache() work properly after a save/restore
			pev->spawnflags |= AMBIENT_SOUND_START_SILENT;

			if (m_dpv.spindownsav || m_dpv.fadeoutsav)
			{
				// spin it down (or fade it) before shutoff if spindown is set
				m_dpv.spindown = m_dpv.spindownsav;
				m_dpv.spinup = 0;

				m_dpv.fadeout = m_dpv.fadeoutsav;
				m_dpv.fadein = 0;
				pev->nextthink = gpGlobals->time + 0.1;
			}
			else
				UTIL_EmitAmbientSound(edict(), pev->origin, szSoundFile,
					0, 0, SND_STOP, 0);
		}
	}
	else
	{// turn sound on

		// only toggle if this is a looping sound.  If not looping, each
		// trigger will cause the sound to play.  If the sound is still
		// playing from a previous trigger press, it will be shut off
		// and then restarted.

		if (m_fLooping)
			m_fActive = true;
		else
			// shut sound off now - may be interrupting a long non-looping sound
			UTIL_EmitAmbientSound(edict(), pev->origin, szSoundFile,
				0, 0, SND_STOP, 0);

		// init all ramp params for startup

		InitModulationParms();

		UTIL_EmitAmbientSound(edict(), pev->origin, szSoundFile,
			(m_dpv.vol * 0.01), m_flAttenuation, 0, m_dpv.pitch);

		pev->nextthink = gpGlobals->time + 0.1;
	}
}

void CAmbientGeneric::KeyValue(KeyValueData* pkvd)
{
	// NOTE: changing any of the modifiers in this code
	// NOTE: also requires changing InitModulationParms code.

	// preset
	if (AreStringsEqual(pkvd->szKeyName, "preset"))
	{
		m_dpv.preset = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}

	// pitchrun
	else if (AreStringsEqual(pkvd->szKeyName, "pitch"))
	{
		m_dpv.pitchrun = atoi(pkvd->szValue);
		pkvd->fHandled = true;

		if (m_dpv.pitchrun > 255) m_dpv.pitchrun = 255;
		if (m_dpv.pitchrun < 0) m_dpv.pitchrun = 0;
	}

	// pitchstart
	else if (AreStringsEqual(pkvd->szKeyName, "pitchstart"))
	{
		m_dpv.pitchstart = atoi(pkvd->szValue);
		pkvd->fHandled = true;

		if (m_dpv.pitchstart > 255) m_dpv.pitchstart = 255;
		if (m_dpv.pitchstart < 0) m_dpv.pitchstart = 0;
	}

	// spinup
	else if (AreStringsEqual(pkvd->szKeyName, "spinup"))
	{
		m_dpv.spinup = atoi(pkvd->szValue);

		if (m_dpv.spinup > 100) m_dpv.spinup = 100;
		if (m_dpv.spinup < 0) m_dpv.spinup = 0;

		if (m_dpv.spinup > 0)
			m_dpv.spinup = (101 - m_dpv.spinup) * 64;
		m_dpv.spinupsav = m_dpv.spinup;
		pkvd->fHandled = true;
	}

	// spindown
	else if (AreStringsEqual(pkvd->szKeyName, "spindown"))
	{
		m_dpv.spindown = atoi(pkvd->szValue);

		if (m_dpv.spindown > 100) m_dpv.spindown = 100;
		if (m_dpv.spindown < 0) m_dpv.spindown = 0;

		if (m_dpv.spindown > 0)
			m_dpv.spindown = (101 - m_dpv.spindown) * 64;
		m_dpv.spindownsav = m_dpv.spindown;
		pkvd->fHandled = true;
	}

	// volstart
	else if (AreStringsEqual(pkvd->szKeyName, "volstart"))
	{
		m_dpv.volstart = atoi(pkvd->szValue);

		if (m_dpv.volstart > 10) m_dpv.volstart = 10;
		if (m_dpv.volstart < 0) m_dpv.volstart = 0;

		m_dpv.volstart *= 10;	// 0 - 100

		pkvd->fHandled = true;
	}

	// fadein
	else if (AreStringsEqual(pkvd->szKeyName, "fadein"))
	{
		m_dpv.fadein = atoi(pkvd->szValue);

		if (m_dpv.fadein > 100) m_dpv.fadein = 100;
		if (m_dpv.fadein < 0) m_dpv.fadein = 0;

		if (m_dpv.fadein > 0)
			m_dpv.fadein = (101 - m_dpv.fadein) * 64;
		m_dpv.fadeinsav = m_dpv.fadein;
		pkvd->fHandled = true;
	}

	// fadeout
	else if (AreStringsEqual(pkvd->szKeyName, "fadeout"))
	{
		m_dpv.fadeout = atoi(pkvd->szValue);

		if (m_dpv.fadeout > 100) m_dpv.fadeout = 100;
		if (m_dpv.fadeout < 0) m_dpv.fadeout = 0;

		if (m_dpv.fadeout > 0)
			m_dpv.fadeout = (101 - m_dpv.fadeout) * 64;
		m_dpv.fadeoutsav = m_dpv.fadeout;
		pkvd->fHandled = true;
	}

	// lfotype
	else if (AreStringsEqual(pkvd->szKeyName, "lfotype"))
	{
		m_dpv.lfotype = atoi(pkvd->szValue);
		if (m_dpv.lfotype > 4) m_dpv.lfotype = LFO_TRIANGLE;
		pkvd->fHandled = true;
	}

	// lforate
	else if (AreStringsEqual(pkvd->szKeyName, "lforate"))
	{
		m_dpv.lforate = atoi(pkvd->szValue);

		if (m_dpv.lforate > 1000) m_dpv.lforate = 1000;
		if (m_dpv.lforate < 0) m_dpv.lforate = 0;

		m_dpv.lforate *= 256;

		pkvd->fHandled = true;
	}
	// lfomodpitch
	else if (AreStringsEqual(pkvd->szKeyName, "lfomodpitch"))
	{
		m_dpv.lfomodpitch = atoi(pkvd->szValue);
		if (m_dpv.lfomodpitch > 100) m_dpv.lfomodpitch = 100;
		if (m_dpv.lfomodpitch < 0) m_dpv.lfomodpitch = 0;


		pkvd->fHandled = true;
	}

	// lfomodvol
	else if (AreStringsEqual(pkvd->szKeyName, "lfomodvol"))
	{
		m_dpv.lfomodvol = atoi(pkvd->szValue);
		if (m_dpv.lfomodvol > 100) m_dpv.lfomodvol = 100;
		if (m_dpv.lfomodvol < 0) m_dpv.lfomodvol = 0;

		pkvd->fHandled = true;
	}

	// cspinup
	else if (AreStringsEqual(pkvd->szKeyName, "cspinup"))
	{
		m_dpv.cspinup = atoi(pkvd->szValue);
		if (m_dpv.cspinup > 100) m_dpv.cspinup = 100;
		if (m_dpv.cspinup < 0) m_dpv.cspinup = 0;

		pkvd->fHandled = true;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

/**
*	@brief Room sound FX
*	@details spawn a sound entity that will set player roomtype when player moves in range and sight.
*	A client that is visible and in range of a sound entity will have its room_type set by that sound entity.
*	If two or more sound entities are contending for a client, then the nearest sound entity to the client will set the client's room_type.
*	A client's room_type will remain set to its prior value until a new in-range, visible sound entity resets a new room_type.
*/
class CEnvSound : public CPointEntity
{
public:
	void KeyValue(KeyValueData* pkvd) override;
	void Spawn() override;

	void Think() override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	float m_flRadius;
	float m_flRoomtype;
};

LINK_ENTITY_TO_CLASS(env_sound, CEnvSound);

TYPEDESCRIPTION	CEnvSound::m_SaveData[] =
{
	DEFINE_FIELD(CEnvSound, m_flRadius, FIELD_FLOAT),
	DEFINE_FIELD(CEnvSound, m_flRoomtype, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CEnvSound, CBaseEntity);

void CEnvSound::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "radius"))
	{
		m_flRadius = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	if (AreStringsEqual(pkvd->szKeyName, "roomtype"))
	{
		m_flRoomtype = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
}

/**
*	@brief returns true if the given sound entity (pev) is in range and can see the given player entity (pevTarget)
*/
bool IsEnvSoundInRange(CEnvSound* pSound, CBaseEntity* pTarget, float* pflRange)
{
	const Vector vecSpot1 = pSound->pev->origin + pSound->pev->view_ofs;
	const Vector vecSpot2 = pTarget->pev->origin + pTarget->pev->view_ofs;
	TraceResult tr;

	UTIL_TraceLine(vecSpot1, vecSpot2, IgnoreMonsters::Yes, pSound->edict(), &tr);

	// check if line of sight crosses water boundary, or is blocked

	if ((tr.fInOpen && tr.fInWater) || tr.flFraction != 1)
		return false;

	// calc range from sound entity to player

	const Vector vecRange = tr.vecEndPos - vecSpot1;
	const float flRange = vecRange.Length();

	if (pSound->m_flRadius < flRange)
		return false;

	if (pflRange)
		*pflRange = flRange;

	return true;
}

// CONSIDER: if player in water state, autoset roomtype to 14,15 or 16. 

void CEnvSound::Think()
{
	const float SlowThinkInterval = 0.75f;
	const float FastThinkInterval = 0.25f;

	// get pointer to client if visible; FIND_CLIENT_IN_PVS will
	// cycle through visible clients on consecutive calls.

	auto pPlayer = static_cast<CBasePlayer*>(InstanceOrNull(FIND_CLIENT_IN_PVS(edict())));

	if (IsNullEnt(pPlayer))
	{
		// no player in pvs of sound entity, slow it down
		pev->nextthink = gpGlobals->time + SlowThinkInterval;
		return;
	}

	float flRange;

	// check to see if this is the sound entity that is 
	// currently affecting this player

	if (CBaseEntity* lastSound = pPlayer->m_hSndLast; lastSound && lastSound == this)
	{
		// this is the entity currently affecting player, check
		// for validity

		if (pPlayer->m_flSndRoomtype != 0 && pPlayer->m_flSndRange != 0) {

			// we're looking at a valid sound entity affecting
			// player, make sure it's still valid, update range

			if (IsEnvSoundInRange(this, pPlayer, &flRange)) {
				pPlayer->m_flSndRange = flRange;
				pev->nextthink = gpGlobals->time + FastThinkInterval;
				return;
			}
			else {

				// current sound entity affecting player is no longer valid,
				// flag this state by clearing room_type and range.
				// NOTE: we do not actually change the player's room_type
				// NOTE: until we have a new valid room_type to change it to.

				pPlayer->m_flSndRange = 0;
				pPlayer->m_flSndRoomtype = 0;
			}
		}
		else {
			// entity is affecting player but is out of range,
			// wait passively for another entity to usurp it...
		}

		pev->nextthink = gpGlobals->time + SlowThinkInterval;
		return;
	}

	// if we got this far, we're looking at an entity that is contending
	// for current player sound. the closest entity to player wins.

	if (IsEnvSoundInRange(this, pPlayer, &flRange))
	{
		if (flRange < pPlayer->m_flSndRange || pPlayer->m_flSndRange == 0)
		{
			// new entity is closer to player, so it wins.
			pPlayer->m_hSndLast = this;
			pPlayer->m_flSndRoomtype = m_flRoomtype;
			pPlayer->m_flSndRange = flRange;

			// send room_type command to player's server.
			// this should be a rare event - once per change of room_type
			// only!

			//CLIENT_COMMAND(pentPlayer, "room_type %f", m_flRoomtype);

			MESSAGE_BEGIN(MessageDest::One, SVC_ROOMTYPE, nullptr, pPlayer->edict());		// use the magic #1 for "one client"
			WRITE_SHORT((short)m_flRoomtype);					// sequence number
			MESSAGE_END();

			// crank up nextthink rate for new active sound entity
			// by falling through to think_fast...
		}
		// player is not closer to the contending sound entity,
		// just fall through to think_fast. this effectively
		// cranks up the think_rate of entities near the player.
	}

	// player is in pvs of sound entity, but either not visible or
	// not in range. do nothing, fall through to think_fast...

	pev->nextthink = gpGlobals->time + FastThinkInterval;
}

void CEnvSound::Spawn()
{
	// spread think times
	pev->nextthink = gpGlobals->time + RANDOM_FLOAT(0.0, 0.5);
}

constexpr int SPEAKER_START_SILENT = 1;	//!< wait for trigger 'on' to start announcements

/**
*	@brief Used for announcements per level, for door lock/unlock spoken voice.
*/
class CSpeaker : public CBaseEntity
{
public:
	void KeyValue(KeyValueData* pkvd) override;
	void Spawn() override;
	void Precache() override;

	/**
	*	@brief if an announcement is pending, cancel it.  If no announcement is pending, start one.
	*/
	void EXPORT ToggleUse(const UseInfo& info);
	void EXPORT SpeakerThink();

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	int	ObjectCaps() override { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION); }

	int	m_preset;			// preset number
};

LINK_ENTITY_TO_CLASS(speaker, CSpeaker);

TYPEDESCRIPTION	CSpeaker::m_SaveData[] =
{
	DEFINE_FIELD(CSpeaker, m_preset, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CSpeaker, CBaseEntity);

void CSpeaker::Spawn()
{
	const char* szSoundFile = STRING(pev->message);

	if (!m_preset && (IsStringNull(pev->message) || strlen(szSoundFile) < 1))
	{
		ALERT(at_error, "SPEAKER with no Level/Sentence! at: %f, %f, %f\n", pev->origin.x, pev->origin.y, pev->origin.z);
		pev->nextthink = gpGlobals->time + 0.1;
		SetThink(&CSpeaker::SUB_Remove);
		return;
	}
	pev->solid = Solid::Not;
	pev->movetype = Movetype::None;

	SetThink(&CSpeaker::SpeakerThink);
	pev->nextthink = 0.0;

	// allow on/off switching via 'use' function.

	SetUse(&CSpeaker::ToggleUse);

	Precache();
}

constexpr float ANNOUNCE_MINUTES_MIN = 0.25;
constexpr float ANNOUNCE_MINUTES_MAX = 2.25;

void CSpeaker::Precache()
{
	if (!IsBitSet(pev->spawnflags, SPEAKER_START_SILENT))
		// set first announcement time for random n second
		pev->nextthink = gpGlobals->time + RANDOM_FLOAT(5.0, 15.0);
}

void CSpeaker::SpeakerThink()
{
	// Wait for the talkmonster to finish first.
	if (gpGlobals->time <= CTalkMonster::g_talkWaitTime)
	{
		pev->nextthink = CTalkMonster::g_talkWaitTime + RANDOM_FLOAT(5, 10);
		return;
	}

	const char* szSoundFile = nullptr;

	if (m_preset)
	{
		// go lookup preset text, assign szSoundFile
		switch (m_preset)
		{
		case 1: szSoundFile = "C1A0_"; break;
		case 2: szSoundFile = "C1A1_"; break;
		case 3: szSoundFile = "C1A2_"; break;
		case 4: szSoundFile = "C1A3_"; break;
		case 5: szSoundFile = "C1A4_"; break;
		case 6: szSoundFile = "C2A1_"; break;
		case 7: szSoundFile = "C2A2_"; break;
		case 8: szSoundFile = "C2A3_"; break;
		case 9: szSoundFile = "C2A4_"; break;
		case 10: szSoundFile = "C2A5_"; break;
		case 11: szSoundFile = "C3A1_"; break;
		case 12: szSoundFile = "C3A2_"; break;
		}
	}
	else
		szSoundFile = STRING(pev->message);

	//No sound to play
	if (!szSoundFile)
	{
		return;
	}

	const float flvolume = pev->health * 0.1;
	const float flattenuation = 0.3;
	const int flags = 0;
	const int pitch = PITCH_NORM;

	if (szSoundFile[0] == '!')
	{
		// play single sentence, one shot
		UTIL_EmitAmbientSound(edict(), pev->origin, szSoundFile,
			flvolume, flattenuation, flags, pitch);

		// shut off and reset
		pev->nextthink = 0.0;
	}
	else
	{
		// make random announcement from sentence group

		if (SENTENCEG_PlayRndSz(this, szSoundFile, flvolume, flattenuation, pitch, flags) < 0)
			ALERT(at_console, "Level Design Error!\nSPEAKER has bad sentence group name: %s\n", szSoundFile);

		// set next announcement time for random 5 to 10 minute delay
		pev->nextthink = gpGlobals->time +
			RANDOM_FLOAT(ANNOUNCE_MINUTES_MIN * 60.0, ANNOUNCE_MINUTES_MAX * 60.0);

		CTalkMonster::g_talkWaitTime = gpGlobals->time + 5;		// time delay until it's ok to speak: used so that two NPCs don't talk at once
	}
}

void CSpeaker::ToggleUse(const UseInfo& info)
{
	const bool fActive = (pev->nextthink > 0.0);

	// fActive is true only if an announcement is pending

	if (info.GetUseType() != UseType::Toggle)
	{
		// ignore if we're just turning something on that's already on, or
		// turning something off that's already off.
		if ((fActive && info.GetUseType() == UseType::On) || (!fActive && info.GetUseType() == UseType::Off))
			return;
	}

	if (info.GetUseType() == UseType::On)
	{
		// turn on announcements
		pev->nextthink = gpGlobals->time + 0.1;
		return;
	}

	if (info.GetUseType() == UseType::Off)
	{
		// turn off announcements
		pev->nextthink = 0.0;
		return;

	}

	// Toggle announcements
	if (fActive)
	{
		// turn off announcements
		pev->nextthink = 0.0;
	}
	else
	{
		// turn on announcements
		pev->nextthink = gpGlobals->time + 0.1;
	}
}

void CSpeaker::KeyValue(KeyValueData* pkvd)
{
	// preset
	if (AreStringsEqual(pkvd->szKeyName, "preset"))
	{
		m_preset = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}
