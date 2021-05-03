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

/**
*	@file
*
*	spawn, think, and use functions for entities that use brush models
*/

#include "doors.h"

extern DLL_GLOBAL Vector		g_vecAttackDir;

constexpr int SF_BRUSH_ACCDCC = 16;			// brush should accelerate and decelerate when toggled
constexpr int SF_BRUSH_HURT = 32;			// rotating brush that inflicts pain based on rotation speed
constexpr int SF_ROTATING_NOT_SOLID = 64;	// some special rotating objects are not solid.

// covering cheesy noise1, noise2, & noise3 fields so they make more sense (for rotating fans)
#define		noiseStart		noise1
#define		noiseStop		noise2
#define		noiseRunning	noise3

constexpr int SF_PENDULUM_SWING = 2;	// spawnflag that makes a pendulum a rope swing.

/**
*	@brief This is just a solid wall if not inhibited
*/
class CFuncWall : public CBaseEntity
{
public:
	void	Spawn() override;
	void	Use(const UseInfo& info) override;

	/**
	*	@brief Bmodels don't go across transitions
	*/
	int	ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
};

LINK_ENTITY_TO_CLASS(func_wall, CFuncWall);

void CFuncWall::Spawn()
{
	pev->angles = vec3_origin;
	SetMovetype(Movetype::Push);  // so it doesn't get pushed by anything
	SetSolidType(Solid::BSP);
	SetModel(STRING(pev->model));

	// If it can't move/go away, it's really part of the world
	pev->flags |= FL_WORLDBRUSH;
}

void CFuncWall::Use(const UseInfo& info)
{
	if (ShouldToggle(info.GetUseType(), (int)(pev->frame)))
		pev->frame = 1 - pev->frame;
}

constexpr int SF_WALL_START_OFF = 0x0001;

class CFuncWallToggle : public CFuncWall
{
public:
	void	Spawn() override;
	void	Use(const UseInfo& info) override;
	void	TurnOff();
	void	TurnOn();
	bool	IsOn();
};

LINK_ENTITY_TO_CLASS(func_wall_toggle, CFuncWallToggle);

void CFuncWallToggle::Spawn()
{
	CFuncWall::Spawn();
	if (pev->spawnflags & SF_WALL_START_OFF)
		TurnOff();
}

void CFuncWallToggle::TurnOff()
{
	SetSolidType(Solid::Not);
	pev->effects |= EF_NODRAW;
	SetAbsOrigin(GetAbsOrigin());
}

void CFuncWallToggle::TurnOn()
{
	SetSolidType(Solid::BSP);
	pev->effects &= ~EF_NODRAW;
	SetAbsOrigin(GetAbsOrigin());
}

bool CFuncWallToggle::IsOn()
{
	return GetSolidType() != Solid::Not;
}

void CFuncWallToggle::Use(const UseInfo& info)
{
	const bool status = IsOn();

	if (ShouldToggle(info.GetUseType(), status))
	{
		if (status)
			TurnOff();
		else
			TurnOn();
	}
}

constexpr int SF_CONVEYOR_VISUAL = 0x0001;
constexpr int SF_CONVEYOR_NOTSOLID = 0x0002;

class CFuncConveyor : public CFuncWall
{
public:
	void	Spawn() override;
	void	Use(const UseInfo& info) override;
	void	UpdateSpeed(float speed);
};

LINK_ENTITY_TO_CLASS(func_conveyor, CFuncConveyor);

void CFuncConveyor::Spawn()
{
	SetMovedir(this);
	CFuncWall::Spawn();

	if (!(pev->spawnflags & SF_CONVEYOR_VISUAL))
		SetBits(pev->flags, FL_CONVEYOR);

	// HACKHACK - This is to allow for some special effects
	if (pev->spawnflags & SF_CONVEYOR_NOTSOLID)
	{
		SetSolidType(Solid::Not);
		pev->skin = 0;		// Don't want the engine thinking we've got special contents on this brush
	}

	if (pev->speed == 0)
		pev->speed = 100;

	UpdateSpeed(pev->speed);
}

// HACKHACK -- This is ugly, but encode the speed in the rendercolor to avoid adding more data to the network stream
void CFuncConveyor::UpdateSpeed(float speed)
{
	// Encode it as an integer with 4 fractional bits
	const int speedCode = (int)(fabs(speed) * 16.0);

	if (speed < 0)
		pev->rendercolor.x = 1;
	else
		pev->rendercolor.x = 0;

	pev->rendercolor.y = (speedCode >> 8);
	pev->rendercolor.z = (speedCode & 0xFF);
}

void CFuncConveyor::Use(const UseInfo& info)
{
	pev->speed = -pev->speed;
	UpdateSpeed(pev->speed);
}

/**
*	@brief A simple entity that looks solid but lets you walk through it.
*/
class CFuncIllusionary : public CBaseToggle
{
public:
	void Spawn() override;
	void KeyValue(KeyValueData* pkvd) override;
	int	ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
};

LINK_ENTITY_TO_CLASS(func_illusionary, CFuncIllusionary);

void CFuncIllusionary::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "skin"))//skin is used for content type
	{
		pev->skin = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CBaseToggle::KeyValue(pkvd);
}

void CFuncIllusionary::Spawn()
{
	pev->angles = vec3_origin;
	SetMovetype(Movetype::None);
	SetSolidType(Solid::Not);// always solid_not 
	SetModel(STRING(pev->model));

	// I'd rather eat the network bandwidth of this than figure out how to save/restore
	// these entities after they have been moved to the client, or respawn them ala Quake
	// Perhaps we can do this in deathmatch only.
	//	MAKE_STATIC(ENT(pev));
}

/**
*	@brief Monster only clip brush
*
*	@details This brush will be solid for any entity who has the FL_MONSTERCLIP flag set in pev->flags
*	otherwise it will be invisible and not solid. This can be used to keep  specific monsters out of certain areas
*/
class CFuncMonsterClip : public CFuncWall
{
public:
	void	Spawn() override;
	void	Use(const UseInfo& info) override {}		//!< Clear out func_wall's use function
};

LINK_ENTITY_TO_CLASS(func_monsterclip, CFuncMonsterClip);

void CFuncMonsterClip::Spawn()
{
	CFuncWall::Spawn();
	if (CVAR_GET_FLOAT("showtriggers") == 0)
		pev->effects = EF_NODRAW;
	pev->flags |= FL_MONSTERCLIP;
}

constexpr int SF_BRUSH_ROTATE_Y_AXIS = 0;
constexpr int SF_BRUSH_ROTATE_INSTANT = 1;
constexpr int SF_BRUSH_ROTATE_BACKWARDS = 2;
constexpr int SF_BRUSH_ROTATE_Z_AXIS = 4;
constexpr int SF_BRUSH_ROTATE_X_AXIS = 8;
constexpr int SF_BRUSH_ROTATE_SMALLRADIUS = 128;
constexpr int SF_BRUSH_ROTATE_MEDIUMRADIUS = 256;
constexpr int SF_BRUSH_ROTATE_LARGERADIUS = 512;

/**
*	@details You need to have an origin brush as part of this entity.
*	The center of that brush will be the point around which it is rotated.
*	It will rotate around the Z axis by default.
*	You can check either the X_AXIS or Y_AXIS box to change that.
*/
class CFuncRotating : public CBaseEntity
{
public:
	// basic functions
	void Spawn() override;
	void Precache() override;

	/**
	*	@brief accelerates a non-moving func_rotating up to it's speed
	*/
	void EXPORT SpinUp();

	/**
	*	@brief decelerates a moving func_rotating to a standstill.
	*/
	void EXPORT SpinDown();
	void KeyValue(KeyValueData* pkvd) override;

	/**
	*	@brief will hurt others based on how fast the brush is spinning
	*/
	void EXPORT HurtTouch(CBaseEntity* pOther);

	/**
	*	@brief when a rotating brush is triggered
	*/
	void EXPORT RotatingUse(const UseInfo& info);
	void EXPORT Rotate();

	/**
	*	@brief ramp pitch and volume up to final values, based on difference between how fast we're going vs how fast we plan to go
	*/
	void RampPitchVol(int fUp);

	/**
	*	@brief An entity has blocked the brush
	*/
	void Blocked(CBaseEntity* pOther) override;
	int	ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];

	float m_flFanFriction;
	float m_flAttenuation;
	float m_flVolume;
	float m_pitch;
	int	  m_sounds;
};

TYPEDESCRIPTION	CFuncRotating::m_SaveData[] =
{
	DEFINE_FIELD(CFuncRotating, m_flFanFriction, FIELD_FLOAT),
	DEFINE_FIELD(CFuncRotating, m_flAttenuation, FIELD_FLOAT),
	DEFINE_FIELD(CFuncRotating, m_flVolume, FIELD_FLOAT),
	DEFINE_FIELD(CFuncRotating, m_pitch, FIELD_FLOAT),
	DEFINE_FIELD(CFuncRotating, m_sounds, FIELD_INTEGER)
};

IMPLEMENT_SAVERESTORE(CFuncRotating, CBaseEntity);

LINK_ENTITY_TO_CLASS(func_rotating, CFuncRotating);

void CFuncRotating::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "fanfriction"))
	{
		m_flFanFriction = atof(pkvd->szValue) / 100;
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "Volume"))
	{
		m_flVolume = atof(pkvd->szValue) / 10.0;

		if (m_flVolume > 1.0)
			m_flVolume = 1.0;
		if (m_flVolume < 0.0)
			m_flVolume = 0.0;
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "spawnorigin"))
	{
		const Vector tmp = UTIL_StringToVector(pkvd->szValue);
		if (tmp != vec3_origin)
			SetAbsOrigin(tmp);
	}
	else if (AreStringsEqual(pkvd->szKeyName, "sounds"))
	{
		m_sounds = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

void CFuncRotating::Spawn()
{
	// set final pitch.  Must not be PITCH_NORM, since we
	// plan on pitch shifting later.

	m_pitch = PITCH_NORM - 1;

	// maintain compatibility with previous maps
	if (m_flVolume == 0.0)
		m_flVolume = 1.0;

	// if the designer didn't set a sound attenuation, default to one.
	m_flAttenuation = ATTN_NORM;

	if (IsBitSet(pev->spawnflags, SF_BRUSH_ROTATE_SMALLRADIUS))
	{
		m_flAttenuation = ATTN_IDLE;
	}
	else if (IsBitSet(pev->spawnflags, SF_BRUSH_ROTATE_MEDIUMRADIUS))
	{
		m_flAttenuation = ATTN_STATIC;
	}
	else if (IsBitSet(pev->spawnflags, SF_BRUSH_ROTATE_LARGERADIUS))
	{
		m_flAttenuation = ATTN_NORM;
	}

	// prevent divide by zero if level designer forgets friction!
	if (m_flFanFriction == 0)
	{
		m_flFanFriction = 1;
	}

	if (IsBitSet(pev->spawnflags, SF_BRUSH_ROTATE_Z_AXIS))
		pev->movedir = vec3_up;
	else if (IsBitSet(pev->spawnflags, SF_BRUSH_ROTATE_X_AXIS))
		pev->movedir = vec3_forward;
	else
		pev->movedir = vec3_right;	// y-axis

	// check for reverse rotation
	if (IsBitSet(pev->spawnflags, SF_BRUSH_ROTATE_BACKWARDS))
		pev->movedir = pev->movedir * -1;

	// some rotating objects like fake volumetric lights will not be solid.
	if (IsBitSet(pev->spawnflags, SF_ROTATING_NOT_SOLID))
	{
		SetSolidType(Solid::Not);
		pev->skin = static_cast<int>(Contents::Empty);
		SetMovetype(Movetype::Push);
	}
	else
	{
		SetSolidType(Solid::BSP);
		SetMovetype(Movetype::Push);
	}

	SetAbsOrigin(GetAbsOrigin());
	SetModel(STRING(pev->model));

	SetUse(&CFuncRotating::RotatingUse);
	// did level designer forget to assign speed?
	if (pev->speed <= 0)
		pev->speed = 0;

	// Removed this per level designers request.  -- JAY
	//	if (pev->dmg == 0)
	//		pev->dmg = 2;

	// instant-use brush?
	if (IsBitSet(pev->spawnflags, SF_BRUSH_ROTATE_INSTANT))
	{
		SetThink(&CFuncRotating::SUB_CallUseToggle);
		pev->nextthink = pev->ltime + 1.5;	// leave a magic delay for client to start up
	}
	// can this brush inflict pain?
	if (IsBitSet(pev->spawnflags, SF_BRUSH_HURT))
	{
		SetTouch(&CFuncRotating::HurtTouch);
	}

	Precache();
}

void CFuncRotating::Precache()
{
	const char* szSoundFile = STRING(pev->message);

	// set up fan sounds

	if (!IsStringNull(pev->message) && strlen(szSoundFile) > 0)
	{
		// if a path is set for a wave, use it

		PRECACHE_SOUND(szSoundFile);

		pev->noiseRunning = ALLOC_STRING(szSoundFile);
	}
	else
	{
		// otherwise use preset sound
		switch (m_sounds)
		{
		case 1:
			PRECACHE_SOUND("fans/fan1.wav");
			pev->noiseRunning = ALLOC_STRING("fans/fan1.wav");
			break;
		case 2:
			PRECACHE_SOUND("fans/fan2.wav");
			pev->noiseRunning = ALLOC_STRING("fans/fan2.wav");
			break;
		case 3:
			PRECACHE_SOUND("fans/fan3.wav");
			pev->noiseRunning = ALLOC_STRING("fans/fan3.wav");
			break;
		case 4:
			PRECACHE_SOUND("fans/fan4.wav");
			pev->noiseRunning = ALLOC_STRING("fans/fan4.wav");
			break;
		case 5:
			PRECACHE_SOUND("fans/fan5.wav");
			pev->noiseRunning = ALLOC_STRING("fans/fan5.wav");
			break;

		case 0:
		default:
			if (!IsStringNull(pev->message) && strlen(szSoundFile) > 0)
			{
				PRECACHE_SOUND(szSoundFile);

				pev->noiseRunning = ALLOC_STRING(szSoundFile);
				break;
			}
			else
			{
				pev->noiseRunning = ALLOC_STRING("common/null.wav");
				break;
			}
		}
	}

	if (pev->avelocity != vec3_origin)
	{
		// if fan was spinning, and we went through transition or save/restore,
		// make sure we restart the sound.  1.5 sec delay is magic number. KDB

		SetThink(&CFuncRotating::SpinUp);
		pev->nextthink = pev->ltime + 1.5;
	}
}

void CFuncRotating::HurtTouch(CBaseEntity* pOther)
{
	// we can't hurt this thing, so we're not concerned with it
	if (!pOther->pev->takedamage)
		return;

	// calculate damage based on rotation speed
	pev->dmg = pev->avelocity.Length() / 10;

	pOther->TakeDamage({this, this, pev->dmg, DMG_CRUSH});

	pOther->pev->velocity = (pOther->GetAbsOrigin() - GetBrushModelOrigin(this)).Normalize() * pev->dmg;
}

constexpr int FANPITCHMIN = 30;
constexpr int FANPITCHMAX = 100;

void CFuncRotating::RampPitchVol(int fUp)
{
	const Vector vecAVel = pev->avelocity;

	// get current angular velocity

	const vec_t vecCur = fabs(vecAVel.x != 0 ? vecAVel.x : (vecAVel.y != 0 ? vecAVel.y : vecAVel.z));

	// get target angular velocity

	vec_t vecFinal = (pev->movedir.x != 0 ? pev->movedir.x : (pev->movedir.y != 0 ? pev->movedir.y : pev->movedir.z));
	vecFinal *= pev->speed;
	vecFinal = fabs(vecFinal);

	// calc volume and pitch as % of final vol and pitch

	const float fpct = vecCur / vecFinal;
	//	if (fUp)
	//		fvol = m_flVolume * (0.5 + fpct/2.0); // spinup volume ramps up from 50% max vol
	//	else
	const float fvol = m_flVolume * fpct;			  // slowdown volume ramps down to 0

	const float fpitch = FANPITCHMIN + (FANPITCHMAX - FANPITCHMIN) * fpct;

	int pitch = (int)fpitch;
	if (pitch == PITCH_NORM)
		pitch = PITCH_NORM - 1;

	// change the fan's vol and pitch

	EmitSound(SoundChannel::Static, STRING(pev->noiseRunning),
		fvol, m_flAttenuation, pitch, SND_CHANGE_PITCH | SND_CHANGE_VOL);
}

void CFuncRotating::SpinUp()
{
	pev->nextthink = pev->ltime + 0.1;
	pev->avelocity = pev->avelocity + (pev->movedir * (pev->speed * m_flFanFriction));

	const Vector vecAVel = pev->avelocity;// cache entity's rotational velocity

	// if we've met or exceeded target speed, set target speed and stop thinking
	if (fabs(vecAVel.x) >= fabs(pev->movedir.x * pev->speed) &&
		fabs(vecAVel.y) >= fabs(pev->movedir.y * pev->speed) &&
		fabs(vecAVel.z) >= fabs(pev->movedir.z * pev->speed))
	{
		pev->avelocity = pev->movedir * pev->speed;// set speed in case we overshot
		EmitSound(SoundChannel::Static, STRING(pev->noiseRunning),
			m_flVolume, m_flAttenuation, FANPITCHMAX, SND_CHANGE_PITCH | SND_CHANGE_VOL);

		SetThink(&CFuncRotating::Rotate);
		Rotate();
	}
	else
	{
		RampPitchVol(true);
	}
}

void CFuncRotating::SpinDown()
{
	pev->nextthink = pev->ltime + 0.1;

	pev->avelocity = pev->avelocity - (pev->movedir * (pev->speed * m_flFanFriction));//spin down slower than spinup

	const Vector vecAVel = pev->avelocity;// cache entity's rotational velocity

	vec_t vecdir;
	if (pev->movedir.x != 0)
		vecdir = pev->movedir.x;
	else if (pev->movedir.y != 0)
		vecdir = pev->movedir.y;
	else
		vecdir = pev->movedir.z;

	// if we've met or exceeded target speed, set target speed and stop thinking
	// (note: must check for movedir > 0 or < 0)
	if (((vecdir > 0) && (vecAVel.x <= 0 && vecAVel.y <= 0 && vecAVel.z <= 0)) ||
		((vecdir < 0) && (vecAVel.x >= 0 && vecAVel.y >= 0 && vecAVel.z >= 0)))
	{
		pev->avelocity = vec3_origin;// set speed in case we overshot

		// stop sound, we're done
		StopSound(SoundChannel::Static, STRING(pev->noiseRunning /* Stop */));

		SetThink(&CFuncRotating::Rotate);
		Rotate();
	}
	else
	{
		RampPitchVol(false);
	}
}

void CFuncRotating::Rotate()
{
	pev->nextthink = pev->ltime + 10;
}

void CFuncRotating::RotatingUse(const UseInfo& info)
{
	// is this a brush that should accelerate and decelerate when turned on/off (fan)?
	if (IsBitSet(pev->spawnflags, SF_BRUSH_ACCDCC))
	{
		// fan is spinning, so stop it.
		if (pev->avelocity != vec3_origin)
		{
			SetThink(&CFuncRotating::SpinDown);
			//EmitSound(SoundChannel::Weapon, STRING(pev->noiseStop), 
			//	m_flVolume, m_flAttenuation, m_pitch);

			pev->nextthink = pev->ltime + 0.1;
		}
		else// fan is not moving, so start it
		{
			SetThink(&CFuncRotating::SpinUp);
			EmitSound(SoundChannel::Static, STRING(pev->noiseRunning),
				0.01, m_flAttenuation, FANPITCHMIN);

			pev->nextthink = pev->ltime + 0.1;
		}
	}
	else if (!IsBitSet(pev->spawnflags, SF_BRUSH_ACCDCC))//this is a normal start/stop brush.
	{
		if (pev->avelocity != vec3_origin)
		{
			// play stopping sound here
			SetThink(&CFuncRotating::SpinDown);

			// EmitSound(SoundChannel::Weapon, STRING(pev->noiseStop), 
			//	m_flVolume, m_flAttenuation, m_pitch);

			pev->nextthink = pev->ltime + 0.1;
			// pev->avelocity = vec3_origin;
		}
		else
		{
			EmitSound(SoundChannel::Static, STRING(pev->noiseRunning),
				m_flVolume, m_flAttenuation, FANPITCHMAX);
			pev->avelocity = pev->movedir * pev->speed;

			SetThink(&CFuncRotating::Rotate);
			Rotate();
		}
	}
}

void CFuncRotating::Blocked(CBaseEntity* pOther)
{
	pOther->TakeDamage({this, this, pev->dmg, DMG_CRUSH});
}

constexpr int SF_PENDULUM_AUTO_RETURN = 16;
constexpr int SF_PENDULUM_PASSABLE = 32;

class CPendulum : public CBaseEntity
{
public:
	void	Spawn() override;
	void	KeyValue(KeyValueData* pkvd) override;
	void	EXPORT Swing();
	void	EXPORT PendulumUse(const UseInfo& info);
	void	EXPORT Stop();
	void	Touch(CBaseEntity* pOther) override;
	void	EXPORT RopeTouch(CBaseEntity* pOther);// this touch func makes the pendulum a rope
	int	ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	void	Blocked(CBaseEntity* pOther) override;

	static	TYPEDESCRIPTION m_SaveData[];

	float	m_accel;			// Acceleration
	float	m_distance;			// 
	float	m_time;
	float	m_damp;
	float	m_maxSpeed;
	float	m_dampSpeed;
	Vector	m_center;
	Vector	m_start;

	EHandle<CBaseEntity> m_hRopeUser;
};

LINK_ENTITY_TO_CLASS(func_pendulum, CPendulum);

TYPEDESCRIPTION	CPendulum::m_SaveData[] =
{
	DEFINE_FIELD(CPendulum, m_accel, FIELD_FLOAT),
	DEFINE_FIELD(CPendulum, m_distance, FIELD_FLOAT),
	DEFINE_FIELD(CPendulum, m_time, FIELD_TIME),
	DEFINE_FIELD(CPendulum, m_damp, FIELD_FLOAT),
	DEFINE_FIELD(CPendulum, m_maxSpeed, FIELD_FLOAT),
	DEFINE_FIELD(CPendulum, m_dampSpeed, FIELD_FLOAT),
	DEFINE_FIELD(CPendulum, m_center, FIELD_VECTOR),
	DEFINE_FIELD(CPendulum, m_start, FIELD_VECTOR),
	DEFINE_FIELD(CPendulum, m_hRopeUser, FIELD_EHANDLE),
};

IMPLEMENT_SAVERESTORE(CPendulum, CBaseEntity);

void CPendulum::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "distance"))
	{
		m_distance = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "damp"))
	{
		m_damp = atof(pkvd->szValue) * 0.001;
		pkvd->fHandled = true;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

void CPendulum::Spawn()
{
	// set the axis of rotation
	CBaseToggle::AxisDir(this);

	if (IsBitSet(pev->spawnflags, SF_DOOR_PASSABLE))
		SetSolidType(Solid::Not);
	else
		SetSolidType(Solid::BSP);
	SetMovetype(Movetype::Push);
	SetAbsOrigin(GetAbsOrigin());
	SetModel(STRING(pev->model));

	if (m_distance == 0)
		return;

	if (pev->speed == 0)
		pev->speed = 100;

	m_accel = (pev->speed * pev->speed) / (2 * fabs(m_distance));	// Calculate constant acceleration from speed and distance
	m_maxSpeed = pev->speed;
	m_start = pev->angles;
	m_center = pev->angles + (m_distance * 0.5) * pev->movedir;

	if (IsBitSet(pev->spawnflags, SF_BRUSH_ROTATE_INSTANT))
	{
		SetThink(&CPendulum::SUB_CallUseToggle);
		pev->nextthink = gpGlobals->time + 0.1;
	}
	pev->speed = 0;
	SetUse(&CPendulum::PendulumUse);

	if (IsBitSet(pev->spawnflags, SF_PENDULUM_SWING))
	{
		//TODO Doesn't actually work because this class overrides Touch()
		//Probably doesn't work anyway
		SetTouch(&CPendulum::RopeTouch);
	}
}

void CPendulum::PendulumUse(const UseInfo& info)
{
	if (pev->speed)		// Pendulum is moving, stop it and auto-return if necessary
	{
		if (IsBitSet(pev->spawnflags, SF_PENDULUM_AUTO_RETURN))
		{
			const float delta = CBaseToggle::AxisDelta(pev->spawnflags, pev->angles, m_start);

			pev->avelocity = m_maxSpeed * pev->movedir;
			pev->nextthink = pev->ltime + (delta / m_maxSpeed);
			SetThink(&CPendulum::Stop);
		}
		else
		{
			pev->speed = 0;		// Dead stop
			SetThink(nullptr);
			pev->avelocity = vec3_origin;
		}
	}
	else
	{
		pev->nextthink = pev->ltime + 0.1;		// Start the pendulum moving
		m_time = gpGlobals->time;		// Save time to calculate dt
		SetThink(&CPendulum::Swing);
		m_dampSpeed = m_maxSpeed;
	}
}

void CPendulum::Stop()
{
	pev->angles = m_start;
	pev->speed = 0;
	SetThink(nullptr);
	pev->avelocity = vec3_origin;
}

void CPendulum::Blocked(CBaseEntity* pOther)
{
	m_time = gpGlobals->time;
}

void CPendulum::Swing()
{
	const float delta = CBaseToggle::AxisDelta(pev->spawnflags, pev->angles, m_center);
	const float dt = gpGlobals->time - m_time;	// How much time has passed?
	m_time = gpGlobals->time;		// Remember the last time called

	if (delta > 0 && m_accel > 0)
		pev->speed -= m_accel * dt;	// Integrate velocity
	else
		pev->speed += m_accel * dt;

	if (pev->speed > m_maxSpeed)
		pev->speed = m_maxSpeed;
	else if (pev->speed < -m_maxSpeed)
		pev->speed = -m_maxSpeed;
	// scale the destdelta vector by the time spent traveling to get velocity
	pev->avelocity = pev->speed * pev->movedir;

	// Call this again
	pev->nextthink = pev->ltime + 0.1;

	if (m_damp)
	{
		m_dampSpeed -= m_damp * m_dampSpeed * dt;
		if (m_dampSpeed < 30.0)
		{
			pev->angles = m_center;
			pev->speed = 0;
			SetThink(nullptr);
			pev->avelocity = vec3_origin;
		}
		else if (pev->speed > m_dampSpeed)
			pev->speed = m_dampSpeed;
		else if (pev->speed < -m_dampSpeed)
			pev->speed = -m_dampSpeed;
	}
}

void CPendulum::Touch(CBaseEntity* pOther)
{
	if (pev->dmg <= 0)
		return;

	// we can't hurt this thing, so we're not concerned with it
	if (!pOther->pev->takedamage)
		return;

	// calculate damage based on rotation speed
	float damage = pev->dmg * pev->speed * 0.01;

	if (damage < 0)
		damage = -damage;

	pOther->TakeDamage({this, this, damage, DMG_CRUSH});

	pOther->pev->velocity = (pOther->GetAbsOrigin() - GetBrushModelOrigin(this)).Normalize() * damage;
}

void CPendulum::RopeTouch(CBaseEntity* pOther)
{
	if (!pOther->IsPlayer())
	{// not a player!
		ALERT(at_console, "Not a client\n");
		return;
	}

	if (pOther == m_hRopeUser)
	{// this player already on the rope.
		return;
	}

	m_hRopeUser = pOther;
	pOther->pev->velocity = vec3_origin;
	pOther->SetMovetype(Movetype::None);
}
