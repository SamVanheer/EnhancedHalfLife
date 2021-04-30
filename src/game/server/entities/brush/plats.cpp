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
*	spawn, think, and touch functions for trains, etc
*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "trains.h"

static void PlatSpawnInsideTrigger(entvars_t* pevPlatform);

constexpr int SF_PLAT_TOGGLE = 0x0001;

class CBasePlatTrain : public CBaseToggle
{
public:
	int	ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	void KeyValue(KeyValueData* pkvd) override;
	void Precache() override;

	// This is done to fix spawn flag collisions between this class and a derived class
	virtual bool IsTogglePlat() { return (pev->spawnflags & SF_PLAT_TOGGLE) != 0; }

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	byte	m_bMoveSnd;			// sound a plat makes while moving
	byte	m_bStopSnd;			// sound a plat makes when it stops
	float	m_volume;			// Sound volume
};

TYPEDESCRIPTION	CBasePlatTrain::m_SaveData[] =
{
	DEFINE_FIELD(CBasePlatTrain, m_bMoveSnd, FIELD_CHARACTER),
	DEFINE_FIELD(CBasePlatTrain, m_bStopSnd, FIELD_CHARACTER),
	DEFINE_FIELD(CBasePlatTrain, m_volume, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CBasePlatTrain, CBaseToggle);

void CBasePlatTrain::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "lip"))
	{
		m_flLip = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "wait"))
	{
		m_flWait = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "height"))
	{
		m_flHeight = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "rotation"))
	{
		m_vecFinalAngle.x = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "movesnd"))
	{
		m_bMoveSnd = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "stopsnd"))
	{
		m_bStopSnd = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "volume"))
	{
		m_volume = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CBaseToggle::KeyValue(pkvd);
}

#define noiseMoving noise
#define noiseArrived noise1

void CBasePlatTrain::Precache()
{
	// set the plat's "in-motion" sound
	switch (m_bMoveSnd)
	{
	case	0:
		pev->noiseMoving = MAKE_STRING("common/null.wav");
		break;
	case	1:
		PRECACHE_SOUND("plats/bigmove1.wav");
		pev->noiseMoving = MAKE_STRING("plats/bigmove1.wav");
		break;
	case	2:
		PRECACHE_SOUND("plats/bigmove2.wav");
		pev->noiseMoving = MAKE_STRING("plats/bigmove2.wav");
		break;
	case	3:
		PRECACHE_SOUND("plats/elevmove1.wav");
		pev->noiseMoving = MAKE_STRING("plats/elevmove1.wav");
		break;
	case	4:
		PRECACHE_SOUND("plats/elevmove2.wav");
		pev->noiseMoving = MAKE_STRING("plats/elevmove2.wav");
		break;
	case	5:
		PRECACHE_SOUND("plats/elevmove3.wav");
		pev->noiseMoving = MAKE_STRING("plats/elevmove3.wav");
		break;
	case	6:
		PRECACHE_SOUND("plats/freightmove1.wav");
		pev->noiseMoving = MAKE_STRING("plats/freightmove1.wav");
		break;
	case	7:
		PRECACHE_SOUND("plats/freightmove2.wav");
		pev->noiseMoving = MAKE_STRING("plats/freightmove2.wav");
		break;
	case	8:
		PRECACHE_SOUND("plats/heavymove1.wav");
		pev->noiseMoving = MAKE_STRING("plats/heavymove1.wav");
		break;
	case	9:
		PRECACHE_SOUND("plats/rackmove1.wav");
		pev->noiseMoving = MAKE_STRING("plats/rackmove1.wav");
		break;
	case	10:
		PRECACHE_SOUND("plats/railmove1.wav");
		pev->noiseMoving = MAKE_STRING("plats/railmove1.wav");
		break;
	case	11:
		PRECACHE_SOUND("plats/squeekmove1.wav");
		pev->noiseMoving = MAKE_STRING("plats/squeekmove1.wav");
		break;
	case	12:
		PRECACHE_SOUND("plats/talkmove1.wav");
		pev->noiseMoving = MAKE_STRING("plats/talkmove1.wav");
		break;
	case	13:
		PRECACHE_SOUND("plats/talkmove2.wav");
		pev->noiseMoving = MAKE_STRING("plats/talkmove2.wav");
		break;
	default:
		pev->noiseMoving = MAKE_STRING("common/null.wav");
		break;
	}

	// set the plat's 'reached destination' stop sound
	switch (m_bStopSnd)
	{
	case	0:
		pev->noiseArrived = MAKE_STRING("common/null.wav");
		break;
	case	1:
		PRECACHE_SOUND("plats/bigstop1.wav");
		pev->noiseArrived = MAKE_STRING("plats/bigstop1.wav");
		break;
	case	2:
		PRECACHE_SOUND("plats/bigstop2.wav");
		pev->noiseArrived = MAKE_STRING("plats/bigstop2.wav");
		break;
	case	3:
		PRECACHE_SOUND("plats/freightstop1.wav");
		pev->noiseArrived = MAKE_STRING("plats/freightstop1.wav");
		break;
	case	4:
		PRECACHE_SOUND("plats/heavystop2.wav");
		pev->noiseArrived = MAKE_STRING("plats/heavystop2.wav");
		break;
	case	5:
		PRECACHE_SOUND("plats/rackstop1.wav");
		pev->noiseArrived = MAKE_STRING("plats/rackstop1.wav");
		break;
	case	6:
		PRECACHE_SOUND("plats/railstop1.wav");
		pev->noiseArrived = MAKE_STRING("plats/railstop1.wav");
		break;
	case	7:
		PRECACHE_SOUND("plats/squeekstop1.wav");
		pev->noiseArrived = MAKE_STRING("plats/squeekstop1.wav");
		break;
	case	8:
		PRECACHE_SOUND("plats/talkstop1.wav");
		pev->noiseArrived = MAKE_STRING("plats/talkstop1.wav");
		break;

	default:
		pev->noiseArrived = MAKE_STRING("common/null.wav");
		break;
	}
}

#define noiseMovement noise
#define noiseStopMoving noise1

/**
*	@details Plats are always drawn in the extended position, so they will light correctly.
*	speed	default 150
*	If the plat has a targetname, it will start out disabled in the extended position until it is triggered,
*	when it will lower and become a normal plat.
*	If the "height" key is set, that will determine the amount the plat moves,
*	instead ofbeing implicitly determined by the model's height.
*/
class CFuncPlat : public CBasePlatTrain
{
public:
	void Spawn() override;
	void Precache() override;
	void Setup();

	void Blocked(CBaseEntity* pOther) override;

	/**
	*	@brief Start bringing platform down.
	*/
	void EXPORT PlatUse(const UseInfo& info);

	void	EXPORT CallGoDown() { GoDown(); }
	void	EXPORT CallHitTop() { HitTop(); }
	void	EXPORT CallHitBottom() { HitBottom(); }

	/**
	*	@brief Platform is at bottom, now starts moving up
	*/
	virtual void GoUp();

	/**
	*	@brief Platform is at top, now starts moving down.
	*/
	virtual void GoDown();

	/**
	*	@brief Platform has hit top. Pauses, then starts back down again.
	*/
	virtual void HitTop();

	/**
	*	@brief Platform has hit bottom. Stops and waits forever.
	*/
	virtual void HitBottom();
};

LINK_ENTITY_TO_CLASS(func_plat, CFuncPlat);

// UNDONE: Need to save this!!! It needs class & linkage
class CPlatTrigger : public CBaseEntity
{
public:
	int	ObjectCaps() override { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_DONT_SAVE; }

	/**
	*	@brief Create a trigger entity for a platform.
	*/
	void SpawnInsideTrigger(CFuncPlat* pPlatform);

	/**
	*	@brief When the platform's trigger field is touched, the platform ???
	*/
	void Touch(CBaseEntity* pOther) override;
	EHANDLE m_hPlatform;
};

void CFuncPlat::Setup()
{
	//pev->noiseMovement = MAKE_STRING("plats/platmove1.wav");
	//pev->noiseStopMoving = MAKE_STRING("plats/platstop1.wav");

	if (m_flTLength == 0)
		m_flTLength = 80;
	if (m_flTWidth == 0)
		m_flTWidth = 10;

	pev->angles = vec3_origin;

	pev->solid = Solid::BSP;
	pev->movetype = Movetype::Push;

	UTIL_SetOrigin(pev, pev->origin);		// set size and link into world
	UTIL_SetSize(pev, pev->mins, pev->maxs);
	SET_MODEL(ENT(pev), STRING(pev->model));

	// vecPosition1 is the top position, vecPosition2 is the bottom
	m_vecPosition1 = pev->origin;
	m_vecPosition2 = pev->origin;
	if (m_flHeight != 0)
		m_vecPosition2.z = pev->origin.z - m_flHeight;
	else
		m_vecPosition2.z = pev->origin.z - pev->size.z + 8;
	if (pev->speed == 0)
		pev->speed = 150;

	if (m_volume == 0)
		m_volume = 0.85;
}

void CFuncPlat::Precache()
{
	CBasePlatTrain::Precache();
	//PRECACHE_SOUND("plats/platmove1.wav");
	//PRECACHE_SOUND("plats/platstop1.wav");
	if (!IsTogglePlat())
		PlatSpawnInsideTrigger(pev);		// the "start moving" trigger
}

void CFuncPlat::Spawn()
{
	Setup();

	Precache();

	// If this platform is the target of some button, it starts at the TOP position,
	// and is brought down by that button.  Otherwise, it starts at BOTTOM.
	if (!IsStringNull(pev->targetname))
	{
		UTIL_SetOrigin(pev, m_vecPosition1);
		m_toggle_state = ToggleState::AtTop;
		SetUse(&CFuncPlat::PlatUse);
	}
	else
	{
		UTIL_SetOrigin(pev, m_vecPosition2);
		m_toggle_state = ToggleState::AtBottom;
	}
}

static void PlatSpawnInsideTrigger(entvars_t* pevPlatform)
{
	GetClassPtr((CPlatTrigger*)nullptr)->SpawnInsideTrigger(GetClassPtr((CFuncPlat*)pevPlatform));
}

void CPlatTrigger::SpawnInsideTrigger(CFuncPlat* pPlatform)
{
	m_hPlatform = pPlatform;
	// Create trigger entity, "point" it at the owning platform, give it a touch method
	pev->solid = Solid::Trigger;
	pev->movetype = Movetype::None;
	pev->origin = pPlatform->pev->origin;

	// Establish the trigger field's size
	Vector vecTMin = pPlatform->pev->mins + Vector(25, 25, 0);
	Vector vecTMax = pPlatform->pev->maxs + Vector(25, 25, 8);
	vecTMin.z = vecTMax.z - (pPlatform->m_vecPosition1.z - pPlatform->m_vecPosition2.z + 8);
	if (pPlatform->pev->size.x <= 50)
	{
		vecTMin.x = (pPlatform->pev->mins.x + pPlatform->pev->maxs.x) / 2;
		vecTMax.x = vecTMin.x + 1;
	}
	if (pPlatform->pev->size.y <= 50)
	{
		vecTMin.y = (pPlatform->pev->mins.y + pPlatform->pev->maxs.y) / 2;
		vecTMax.y = vecTMin.y + 1;
	}
	UTIL_SetSize(pev, vecTMin, vecTMax);
}

void CPlatTrigger::Touch(CBaseEntity* pOther)
{
	//Platform was removed, remove trigger
	if (!m_hPlatform || !m_hPlatform->pev)
	{
		UTIL_Remove(this);
		return;
	}

	// Ignore touches by non-players
	if (!pOther->IsPlayer())
		return;

	// Ignore touches by corpses
	if (!pOther->IsAlive())
		return;

	CFuncPlat* platform = static_cast<CFuncPlat*>(static_cast<CBaseEntity*>(m_hPlatform));

	// Make linked platform go up/down.
	if (platform->m_toggle_state == ToggleState::AtBottom)
		platform->GoUp();
	else if (platform->m_toggle_state == ToggleState::AtTop)
		platform->pev->nextthink = platform->pev->ltime + 1;// delay going down
}

void CFuncPlat::PlatUse(const UseInfo& info)
{
	if (IsTogglePlat())
	{
		// Top is off, bottom is on
		const bool on = m_toggle_state == ToggleState::AtBottom;

		if (!ShouldToggle(info.GetUseType(), on))
			return;

		if (m_toggle_state == ToggleState::AtTop)
			GoDown();
		else if (m_toggle_state == ToggleState::AtBottom)
			GoUp();
	}
	else
	{
		SetUse(nullptr);

		if (m_toggle_state == ToggleState::AtTop)
			GoDown();
	}
}

void CFuncPlat::GoDown()
{
	if (!IsStringNull(pev->noiseMovement))
		EmitSound(SoundChannel::Static, STRING(pev->noiseMovement), m_volume);

	ASSERT(m_toggle_state == ToggleState::AtTop || m_toggle_state == ToggleState::GoingUp);
	m_toggle_state = ToggleState::GoingDown;
	SetMoveDone(&CFuncPlat::CallHitBottom);
	LinearMove(m_vecPosition2, pev->speed);
}

void CFuncPlat::HitBottom()
{
	if (!IsStringNull(pev->noiseMovement))
		StopSound(SoundChannel::Static, STRING(pev->noiseMovement));

	if (!IsStringNull(pev->noiseStopMoving))
		EmitSound(SoundChannel::Weapon, STRING(pev->noiseStopMoving), m_volume);

	ASSERT(m_toggle_state == ToggleState::GoingDown);
	m_toggle_state = ToggleState::AtBottom;
}

void CFuncPlat::GoUp()
{
	if (!IsStringNull(pev->noiseMovement))
		EmitSound(SoundChannel::Static, STRING(pev->noiseMovement), m_volume);

	ASSERT(m_toggle_state == ToggleState::AtBottom || m_toggle_state == ToggleState::GoingDown);
	m_toggle_state = ToggleState::GoingUp;
	SetMoveDone(&CFuncPlat::CallHitTop);
	LinearMove(m_vecPosition1, pev->speed);
}

void CFuncPlat::HitTop()
{
	if (!IsStringNull(pev->noiseMovement))
		StopSound(SoundChannel::Static, STRING(pev->noiseMovement));

	if (!IsStringNull(pev->noiseStopMoving))
		EmitSound(SoundChannel::Weapon, STRING(pev->noiseStopMoving), m_volume);

	ASSERT(m_toggle_state == ToggleState::GoingUp);
	m_toggle_state = ToggleState::AtTop;

	if (!IsTogglePlat())
	{
		// After a delay, the platform will automatically start going down again.
		SetThink(&CFuncPlat::CallGoDown);
		pev->nextthink = pev->ltime + 3;
	}
}

void CFuncPlat::Blocked(CBaseEntity* pOther)
{
	ALERT(at_aiconsole, "%s Blocked by %s\n", STRING(pev->classname), STRING(pOther->pev->classname));
	// Hurt the blocker a little
	pOther->TakeDamage({pev, pev, 1, DMG_CRUSH});

	if (!IsStringNull(pev->noiseMovement))
		StopSound(SoundChannel::Static, STRING(pev->noiseMovement));

	// Send the platform back where it came from
	ASSERT(m_toggle_state == ToggleState::GoingUp || m_toggle_state == ToggleState::GoingDown);
	if (m_toggle_state == ToggleState::GoingUp)
		GoDown();
	else if (m_toggle_state == ToggleState::GoingDown)
		GoUp();
}

class CFuncPlatRot : public CFuncPlat
{
public:
	void Spawn() override;
	void SetupRotation();

	void	GoUp() override;
	void	GoDown() override;
	void	HitTop() override;
	void	HitBottom() override;

	void			RotMove(const Vector& destAngle, float time);
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	Vector	m_end, m_start;
};

LINK_ENTITY_TO_CLASS(func_platrot, CFuncPlatRot);

TYPEDESCRIPTION	CFuncPlatRot::m_SaveData[] =
{
	DEFINE_FIELD(CFuncPlatRot, m_end, FIELD_VECTOR),
	DEFINE_FIELD(CFuncPlatRot, m_start, FIELD_VECTOR),
};

IMPLEMENT_SAVERESTORE(CFuncPlatRot, CFuncPlat);

void CFuncPlatRot::SetupRotation()
{
	if (m_vecFinalAngle.x != 0)		// This plat rotates too!
	{
		CBaseToggle::AxisDir(pev);
		m_start = pev->angles;
		m_end = pev->angles + pev->movedir * m_vecFinalAngle.x;
	}
	else
	{
		m_start = vec3_origin;
		m_end = vec3_origin;
	}
	if (!IsStringNull(pev->targetname))	// Start at top
	{
		pev->angles = m_end;
	}
}

void CFuncPlatRot::Spawn()
{
	CFuncPlat::Spawn();
	SetupRotation();
}

void CFuncPlatRot::GoDown()
{
	CFuncPlat::GoDown();
	RotMove(m_start, pev->nextthink - pev->ltime);
}

void CFuncPlatRot::HitBottom()
{
	CFuncPlat::HitBottom();
	pev->avelocity = vec3_origin;
	pev->angles = m_start;
}

void CFuncPlatRot::GoUp()
{
	CFuncPlat::GoUp();
	RotMove(m_end, pev->nextthink - pev->ltime);
}

void CFuncPlatRot::HitTop()
{
	CFuncPlat::HitTop();
	pev->avelocity = vec3_origin;
	pev->angles = m_end;
}

void CFuncPlatRot::RotMove(const Vector& destAngle, float time)
{
	// set destdelta to the vector needed to move
	const Vector vecDestDelta = destAngle - pev->angles;

	// Travel time is so short, we're practically there already;  so make it so.
	if (time >= 0.1)
		pev->avelocity = vecDestDelta / time;
	else
	{
		pev->avelocity = vecDestDelta;
		pev->nextthink = pev->ltime + 1;
	}
}

/**
*	@brief Trains are moving platforms that players can ride.
*	@details The targets origin specifies the min point of the train at each corner.
*	The train spawns at the first target it is pointing at.
*	If the train is the target of a button or trigger, it will not begin moving until activated.
*	speed	default 100
*	dmg		default	2
*/
class CFuncTrain : public CBasePlatTrain
{
public:
	void Spawn() override;
	void Precache() override;
	void Activate() override;
	void OverrideReset() override;

	void Blocked(CBaseEntity* pOther) override;
	void Use(const UseInfo& info) override;
	void KeyValue(KeyValueData* pkvd) override;


	void EXPORT Wait();

	/**
	*	@brief path corner needs to change to next target 
	*/
	void EXPORT Next();
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	EHANDLE m_hCurrentTarget;
	int			m_sounds;
	bool		m_activated;
};

LINK_ENTITY_TO_CLASS(func_train, CFuncTrain);

TYPEDESCRIPTION	CFuncTrain::m_SaveData[] =
{
	DEFINE_FIELD(CFuncTrain, m_sounds, FIELD_INTEGER),
	DEFINE_FIELD(CFuncTrain, m_hCurrentTarget, FIELD_EHANDLE),
	DEFINE_FIELD(CFuncTrain, m_activated, FIELD_BOOLEAN),
};

IMPLEMENT_SAVERESTORE(CFuncTrain, CBasePlatTrain);

void CFuncTrain::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "sounds"))
	{
		m_sounds = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CBasePlatTrain::KeyValue(pkvd);
}

void CFuncTrain::Blocked(CBaseEntity* pOther)
{
	if (gpGlobals->time < m_flActivateFinished)
		return;

	m_flActivateFinished = gpGlobals->time + 0.5;

	pOther->TakeDamage({pev, pev, pev->dmg, DMG_CRUSH});
}

void CFuncTrain::Use(const UseInfo& info)
{
	if (pev->spawnflags & SF_TRAIN_WAIT_RETRIGGER)
	{
		// Move toward my target
		pev->spawnflags &= ~SF_TRAIN_WAIT_RETRIGGER;
		Next();
	}
	else
	{
		pev->spawnflags |= SF_TRAIN_WAIT_RETRIGGER;
		// Pop back to last target if it's available
		if (CBaseEntity* last = m_hCurrentTarget; last)
			pev->target = last->pev->targetname;
		pev->nextthink = 0;
		pev->velocity = vec3_origin;
		if (!IsStringNull(pev->noiseStopMoving))
			EmitSound(SoundChannel::Voice, STRING(pev->noiseStopMoving), m_volume);
	}
}

void CFuncTrain::Wait()
{
	//TODO: can be null if killtargeted
	CBaseEntity* pTarget = m_hCurrentTarget;

	// Fire the pass target if there is one
	if (!IsStringNull(pTarget->pev->message))
	{
		FireTargets(STRING(pTarget->pev->message), this, this, USE_TOGGLE, 0);
		if (IsBitSet(pTarget->pev->spawnflags, SF_CORNER_FIREONCE))
			pTarget->pev->message = iStringNull;
	}

	// need pointer to LAST target.
	if (IsBitSet(pTarget->pev->spawnflags, SF_TRAIN_WAIT_RETRIGGER) || (pev->spawnflags & SF_TRAIN_WAIT_RETRIGGER))
	{
		pev->spawnflags |= SF_TRAIN_WAIT_RETRIGGER;
		// clear the sound channel.
		if (!IsStringNull(pev->noiseMovement))
			StopSound(SoundChannel::Static, STRING(pev->noiseMovement));
		if (!IsStringNull(pev->noiseStopMoving))
			EmitSound(SoundChannel::Voice, STRING(pev->noiseStopMoving), m_volume);
		pev->nextthink = 0;
		return;
	}

	// ALERT ( at_console, "%f\n", m_flWait );

	if (m_flWait != 0)
	{// -1 wait will wait forever!		
		pev->nextthink = pev->ltime + m_flWait;
		if (!IsStringNull(pev->noiseMovement))
			StopSound(SoundChannel::Static, STRING(pev->noiseMovement));
		if (!IsStringNull(pev->noiseStopMoving))
			EmitSound(SoundChannel::Voice, STRING(pev->noiseStopMoving), m_volume);
		SetThink(&CFuncTrain::Next);
	}
	else
	{
		Next();// do it RIGHT now!
	}
}

void CFuncTrain::Next()
{
	// now find our next target
	CBaseEntity* pTarg = GetNextTarget();

	if (!pTarg)
	{
		if (!IsStringNull(pev->noiseMovement))
			StopSound(SoundChannel::Static, STRING(pev->noiseMovement));
		// Play stop sound
		if (!IsStringNull(pev->noiseStopMoving))
			EmitSound(SoundChannel::Voice, STRING(pev->noiseStopMoving), m_volume);
		return;
	}

	// Save last target in case we need to find it again
	pev->message = pev->target;

	pev->target = pTarg->pev->target;
	m_flWait = pTarg->GetDelay();

	if (CBaseEntity* pCurrentTarget = m_hCurrentTarget; pCurrentTarget && pCurrentTarget->pev->speed != 0)
	{// don't copy speed from target if it is 0 (uninitialized)
		pev->speed = pCurrentTarget->pev->speed;
		ALERT(at_aiconsole, "Train %s speed to %4.2f\n", STRING(pev->targetname), pev->speed);
	}
	m_hCurrentTarget = pTarg;// keep track of this since path corners change our target for us.

	if (IsBitSet(pTarg->pev->spawnflags, SF_CORNER_TELEPORT))
	{
		// Path corner has indicated a teleport to the next corner.
		SetBits(pev->effects, EF_NOINTERP);
		UTIL_SetOrigin(pev, pTarg->pev->origin - (pev->mins + pev->maxs) * 0.5);
		Wait(); // Get on with doing the next path corner.
	}
	else
	{
		// Normal linear move.

		// CHANGED this from SoundChannel::Voice to SoundChannel::Static around OEM beta time because trains should
		// use SoundChannel::Static for their movement sounds to prevent sound field problems.
		// this is not a hack or temporary fix, this is how things should be. (sjb).
		if (!IsStringNull(pev->noiseMovement))
			StopSound(SoundChannel::Static, STRING(pev->noiseMovement));
		if (!IsStringNull(pev->noiseMovement))
			EmitSound(SoundChannel::Static, STRING(pev->noiseMovement), m_volume);
		ClearBits(pev->effects, EF_NOINTERP);
		SetMoveDone(&CFuncTrain::Wait);
		LinearMove(pTarg->pev->origin - (pev->mins + pev->maxs) * 0.5, pev->speed);
	}
}

void CFuncTrain::Activate()
{
	// Not yet active, so teleport to first target
	if (!m_activated)
	{
		m_activated = true;
		CBaseEntity* pTarget = UTIL_FindEntityByTargetname(nullptr, STRING(pev->target));

		//TODO: currently mimics old behavior where it uses the world by default. Needs to handle null targets better
		if (!pTarget)
		{
			pTarget = Instance(INDEXENT(0));
		}

		pev->target = pTarget->pev->target;
		m_hCurrentTarget = pTarget;// keep track of this since path corners change our target for us.

		UTIL_SetOrigin(pev, pTarget->pev->origin - (pev->mins + pev->maxs) * 0.5);

		if (IsStringNull(pev->targetname))
		{	// not triggered, so start immediately
			pev->nextthink = pev->ltime + 0.1;
			SetThink(&CFuncTrain::Next);
		}
		else
			pev->spawnflags |= SF_TRAIN_WAIT_RETRIGGER;
	}
}

void CFuncTrain::Spawn()
{
	Precache();
	if (pev->speed == 0)
		pev->speed = 100;

	if (IsStringNull(pev->target))
		ALERT(at_console, "FuncTrain with no target");

	if (pev->dmg == 0)
		pev->dmg = 2;

	pev->movetype = Movetype::Push;

	if (IsBitSet(pev->spawnflags, SF_TRACKTRAIN_PASSABLE))
		pev->solid = Solid::Not;
	else
		pev->solid = Solid::BSP;

	SET_MODEL(ENT(pev), STRING(pev->model));
	UTIL_SetSize(pev, pev->mins, pev->maxs);
	UTIL_SetOrigin(pev, pev->origin);

	m_activated = false;

	if (m_volume == 0)
		m_volume = 0.85;
}

void CFuncTrain::Precache()
{
	CBasePlatTrain::Precache();

#if 0  // obsolete
	// otherwise use preset sound
	switch (m_sounds)
	{
	case 0:
		pev->noise = 0;
		pev->noise1 = 0;
		break;

	case 1:
		PRECACHE_SOUND("plats/train2.wav");
		PRECACHE_SOUND("plats/train1.wav");
		pev->noise = MAKE_STRING("plats/train2.wav");
		pev->noise1 = MAKE_STRING("plats/train1.wav");
		break;

	case 2:
		PRECACHE_SOUND("plats/platmove1.wav");
		PRECACHE_SOUND("plats/platstop1.wav");
		pev->noise = MAKE_STRING("plats/platstop1.wav");
		pev->noise1 = MAKE_STRING("plats/platmove1.wav");
		break;
	}
#endif
}

void CFuncTrain::OverrideReset()
{
	// Are we moving?
	if (pev->velocity != vec3_origin && pev->nextthink != 0)
	{
		pev->target = pev->message;
		// now find our next target
		CBaseEntity* pTarg = GetNextTarget();
		if (!pTarg)
		{
			pev->nextthink = 0;
			pev->velocity = vec3_origin;
		}
		else	// Keep moving for 0.1 secs, then find path_corner again and restart
		{
			SetThink(&CFuncTrain::Next);
			pev->nextthink = pev->ltime + 0.1;
		}
	}
}

TYPEDESCRIPTION	CFuncTrackTrain::m_SaveData[] =
{
	DEFINE_FIELD(CFuncTrackTrain, m_ppath, FIELD_CLASSPTR),
	DEFINE_FIELD(CFuncTrackTrain, m_length, FIELD_FLOAT),
	DEFINE_FIELD(CFuncTrackTrain, m_height, FIELD_FLOAT),
	DEFINE_FIELD(CFuncTrackTrain, m_speed, FIELD_FLOAT),
	DEFINE_FIELD(CFuncTrackTrain, m_dir, FIELD_FLOAT),
	DEFINE_FIELD(CFuncTrackTrain, m_startSpeed, FIELD_FLOAT),
	DEFINE_FIELD(CFuncTrackTrain, m_controlMins, FIELD_VECTOR),
	DEFINE_FIELD(CFuncTrackTrain, m_controlMaxs, FIELD_VECTOR),
	DEFINE_FIELD(CFuncTrackTrain, m_sounds, FIELD_INTEGER),
	DEFINE_FIELD(CFuncTrackTrain, m_flVolume, FIELD_FLOAT),
	DEFINE_FIELD(CFuncTrackTrain, m_flBank, FIELD_FLOAT),
	DEFINE_FIELD(CFuncTrackTrain, m_oldSpeed, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CFuncTrackTrain, CBaseEntity);
LINK_ENTITY_TO_CLASS(func_tracktrain, CFuncTrackTrain);

void CFuncTrackTrain::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "wheels"))
	{
		m_length = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "height"))
	{
		m_height = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "startspeed"))
	{
		m_startSpeed = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "sounds"))
	{
		m_sounds = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "volume"))
	{
		m_flVolume = (float)(atoi(pkvd->szValue));
		m_flVolume *= 0.1;
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "bank"))
	{
		m_flBank = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

void CFuncTrackTrain::NextThink(float thinkTime, bool alwaysThink)
{
	if (alwaysThink)
		pev->flags |= FL_ALWAYSTHINK;
	else
		pev->flags &= ~FL_ALWAYSTHINK;

	pev->nextthink = thinkTime;
}

void CFuncTrackTrain::Blocked(CBaseEntity* pOther)
{
	entvars_t* pevOther = pOther->pev;

	// Blocker is on-ground on the train
	if (IsBitSet(pevOther->flags, FL_ONGROUND) && VARS(pevOther->groundentity) == pev)
	{
		const float deltaSpeed = std::min(fabs(pev->speed), 50.0f);
		if (!pevOther->velocity.z)
			pevOther->velocity.z += deltaSpeed;
		return;
	}
	else
		pevOther->velocity = (pevOther->origin - pev->origin).Normalize() * pev->dmg;

	ALERT(at_aiconsole, "TRAIN(%s): Blocked by %s (dmg:%.2f)\n", STRING(pev->targetname), STRING(pOther->pev->classname), pev->dmg);
	if (pev->dmg <= 0)
		return;
	// we can't hurt this thing, so we're not concerned with it
	pOther->TakeDamage({pev, pev, pev->dmg, DMG_CRUSH});
}

void CFuncTrackTrain::Use(const UseInfo& info)
{
	if (info.GetUseType() != USE_SET)
	{
		if (!ShouldToggle(info.GetUseType(), (pev->speed != 0)))
			return;

		if (pev->speed == 0)
		{
			pev->speed = m_speed * m_dir;

			Next();
		}
		else
		{
			pev->speed = 0;
			pev->velocity = vec3_origin;
			pev->avelocity = vec3_origin;
			StopSound();
			SetThink(nullptr);
		}
	}
	else
	{
		float delta = info.GetValue();

		delta = ((int)(pev->speed * 4) / (int)m_speed) * 0.25 + 0.25 * delta;
		if (delta > 1)
			delta = 1;
		else if (delta < -1)
			delta = -1;
		if (pev->spawnflags & SF_TRACKTRAIN_FORWARDONLY)
		{
			if (delta < 0)
				delta = 0;
		}
		pev->speed = m_speed * delta;
		Next();
		ALERT(at_aiconsole, "TRAIN(%s), speed to %.2f\n", STRING(pev->targetname), pev->speed);
	}
}

constexpr int TRAIN_STARTPITCH = 60;
constexpr int TRAIN_MAXPITCH = 200;
constexpr int TRAIN_MAXSPEED = 1000;	// approx max speed for sound pitch calculation

void CFuncTrackTrain::StopSound()
{
	// if sound playing, stop it
	if (m_soundPlaying && !IsStringNull(pev->noise))
	{
		const unsigned short us_sound = ((unsigned short)(m_sounds) & 0x0007) << 12;

		const unsigned short us_encode = us_sound;

		PLAYBACK_EVENT_FULL(FEV_RELIABLE | FEV_UPDATE, edict(), m_usAdjustPitch, 0.0,
			vec3_origin, vec3_origin, 0.0, 0.0, us_encode, 0, 1, 0);

		/*
		StopSound(SoundChannel::Static, STRING(pev->noise));
		*/
		EmitSound(SoundChannel::Item, "plats/ttrain_brake1.wav", m_flVolume);
	}

	m_soundPlaying = false;
}

void CFuncTrackTrain::UpdateSound()
{
	if (IsStringNull(pev->noise))
		return;

	const float flpitch = TRAIN_STARTPITCH + (fabs(pev->speed) * (TRAIN_MAXPITCH - TRAIN_STARTPITCH) / TRAIN_MAXSPEED);

	if (!m_soundPlaying)
	{
		// play startup sound for train
		EmitSound(SoundChannel::Item, "plats/ttrain_start1.wav", m_flVolume);
		EmitSound(SoundChannel::Static, STRING(pev->noise), m_flVolume, ATTN_NORM, (int)flpitch);
		m_soundPlaying = true;
	}
	else
	{
		/*
				// update pitch
				EmitSound(SoundChannel::Static, STRING(pev->noise), m_flVolume, ATTN_NORM, (int) flpitch, SND_CHANGE_PITCH);
		*/
		// volume 0.0 - 1.0 - 6 bits
		// m_sounds 3 bits
		// flpitch = 6 bits
		// 15 bits total

		const unsigned short us_sound = ((unsigned short)(m_sounds) & 0x0007) << 12;
		const unsigned short us_pitch = ((unsigned short)(flpitch / 10.0) & 0x003f) << 6;
		const unsigned short us_volume = ((unsigned short)(m_flVolume * 40.0) & 0x003f);

		const unsigned short us_encode = us_sound | us_pitch | us_volume;

		PLAYBACK_EVENT_FULL(FEV_RELIABLE | FEV_UPDATE, edict(), m_usAdjustPitch, 0.0,
			vec3_origin, vec3_origin, 0.0, 0.0, us_encode, 0, 0, 0);
	}
}

void CFuncTrackTrain::Next()
{
	if (!pev->speed)
	{
		ALERT(at_aiconsole, "TRAIN(%s): Speed is 0\n", STRING(pev->targetname));
		StopSound();
		return;
	}

	/*
	if (!m_ppath)
	{
		m_ppath = CPathTrack::Instance(UTIL_FindEntityByTargetname(nullptr, STRING(pev->target)));
	}
	*/
	if (!m_ppath)
	{
		ALERT(at_aiconsole, "TRAIN(%s): Lost path\n", STRING(pev->targetname));
		StopSound();
		return;
	}

	UpdateSound();

	Vector nextPos = pev->origin;

	nextPos.z -= m_height;
	CPathTrack* pnext = m_ppath->LookAhead(&nextPos, pev->speed * 0.1, 1);
	nextPos.z += m_height;

	pev->velocity = (nextPos - pev->origin) * 10;
	Vector nextFront = pev->origin;

	nextFront.z -= m_height;
	if (m_length > 0)
		m_ppath->LookAhead(&nextFront, m_length, 0);
	else
		m_ppath->LookAhead(&nextFront, 100, 0);
	nextFront.z += m_height;

	const Vector delta = nextFront - pev->origin;
	Vector angles = VectorAngles(delta);
	// The train actually points west
	angles.y += 180;

	// !!!  All of this crap has to be done to make the angles not wrap around, revisit this.
	UTIL_FixupAngles(angles);
	UTIL_FixupAngles(pev->angles);

	if (!pnext || (delta.x == 0 && delta.y == 0))
		angles = pev->angles;

	float vx;
	if (!(pev->spawnflags & SF_TRACKTRAIN_NOPITCH))
		vx = UTIL_AngleDistance(angles.x, pev->angles.x);
	else
		vx = 0;
	const float vy = UTIL_AngleDistance(angles.y, pev->angles.y);

	pev->avelocity.y = vy * 10;
	pev->avelocity.x = vx * 10;

	if (m_flBank != 0)
	{
		if (pev->avelocity.y < -5)
			pev->avelocity.z = UTIL_AngleDistance(UTIL_ApproachAngle(-m_flBank, pev->angles.z, m_flBank * 2), pev->angles.z);
		else if (pev->avelocity.y > 5)
			pev->avelocity.z = UTIL_AngleDistance(UTIL_ApproachAngle(m_flBank, pev->angles.z, m_flBank * 2), pev->angles.z);
		else
			pev->avelocity.z = UTIL_AngleDistance(UTIL_ApproachAngle(0, pev->angles.z, m_flBank * 4), pev->angles.z) * 4;
	}

	if (pnext)
	{
		if (pnext != m_ppath)
		{
			CPathTrack* pFire;
			if (pev->speed >= 0)
				pFire = pnext;
			else
				pFire = m_ppath;

			m_ppath = pnext;
			// Fire the pass target if there is one
			if (!IsStringNull(pFire->pev->message))
			{
				FireTargets(STRING(pFire->pev->message), this, this, USE_TOGGLE, 0);
				if (IsBitSet(pFire->pev->spawnflags, SF_PATH_FIREONCE))
					pFire->pev->message = iStringNull;
			}

			if (pFire->pev->spawnflags & SF_PATH_DISABLE_TRAIN)
				pev->spawnflags |= SF_TRACKTRAIN_NOCONTROL;

			// Don't override speed if under user control
			if (pev->spawnflags & SF_TRACKTRAIN_NOCONTROL)
			{
				if (pFire->pev->speed != 0)
				{// don't copy speed from target if it is 0 (uninitialized)
					pev->speed = pFire->pev->speed;
					ALERT(at_aiconsole, "TrackTrain %s speed to %4.2f\n", STRING(pev->targetname), pev->speed);
				}
			}

		}
		SetThink(&CFuncTrackTrain::Next);
		NextThink(pev->ltime + 0.5f, true);
	}
	else	// end of path, stop
	{
		StopSound();
		pev->velocity = (nextPos - pev->origin);
		pev->avelocity = vec3_origin;
		const float distance = pev->velocity.Length();
		m_oldSpeed = pev->speed;

		pev->speed = 0;

		// Move to the dead end

		// Are we there yet?
		if (distance > 0)
		{
			// no, how long to get there?
			const float time = distance / m_oldSpeed;
			pev->velocity = pev->velocity * (m_oldSpeed / distance);
			SetThink(&CFuncTrackTrain::DeadEnd);
			NextThink(pev->ltime + time, false);
		}
		else
		{
			DeadEnd();
		}
	}
}

void CFuncTrackTrain::DeadEnd()
{
	// Fire the dead-end target if there is one
	CPathTrack* pTrack = m_ppath;

	ALERT(at_aiconsole, "TRAIN(%s): Dead end ", STRING(pev->targetname));
	// Find the dead end path node
	// HACKHACK -- This is bugly, but the train can actually stop moving at a different node depending on its speed
	// so we have to traverse the list to it's end.
	if (pTrack)
	{
		if (m_oldSpeed < 0)
		{
			CPathTrack* pNext;
			do
			{
				pNext = pTrack->ValidPath(pTrack->GetPrevious(), true);
				if (pNext)
					pTrack = pNext;
			}
			while (pNext);
		}
		else
		{
			CPathTrack* pNext;
			do
			{
				pNext = pTrack->ValidPath(pTrack->GetNext(), true);
				if (pNext)
					pTrack = pNext;
			}
			while (pNext);
		}
	}

	pev->velocity = vec3_origin;
	pev->avelocity = vec3_origin;
	if (pTrack)
	{
		ALERT(at_aiconsole, "at %s\n", STRING(pTrack->pev->targetname));
		if (!IsStringNull(pTrack->pev->netname))
			FireTargets(STRING(pTrack->pev->netname), this, this, USE_TOGGLE, 0);
	}
	else
		ALERT(at_aiconsole, "\n");
}

void CFuncTrackTrain::SetControls(entvars_t* pevControls)
{
	const Vector offset = pevControls->origin - pev->oldorigin;

	m_controlMins = pevControls->mins + offset;
	m_controlMaxs = pevControls->maxs + offset;
}

bool CFuncTrackTrain::OnControls(entvars_t* pevTest)
{
	const Vector offset = pevTest->origin - pev->origin;

	if (pev->spawnflags & SF_TRACKTRAIN_NOCONTROL)
		return false;

	// Transform offset into local coordinates
	UTIL_MakeVectors(pev->angles);
	const Vector local
	{
		DotProduct(offset, gpGlobals->v_forward),
		-DotProduct(offset, gpGlobals->v_right),
		DotProduct(offset, gpGlobals->v_up)
	};

	if (local.x >= m_controlMins.x && local.y >= m_controlMins.y && local.z >= m_controlMins.z &&
		local.x <= m_controlMaxs.x && local.y <= m_controlMaxs.y && local.z <= m_controlMaxs.z)
		return true;

	return false;
}

void CFuncTrackTrain::Find()
{
	m_ppath = CPathTrack::Instance(UTIL_FindEntityByTargetname(nullptr, STRING(pev->target)));
	if (!m_ppath)
		return;

	entvars_t* pevTarget = m_ppath->pev;
	if (!ClassnameIs(pevTarget, "path_track"))
	{
		ALERT(at_error, "func_track_train must be on a path of path_track\n");
		m_ppath = nullptr;
		return;
	}

	Vector nextPos = pevTarget->origin;
	nextPos.z += m_height;

	Vector look = nextPos;
	look.z -= m_height;
	m_ppath->LookAhead(&look, m_length, 0);
	look.z += m_height;

	pev->angles = VectorAngles(look - nextPos);
	// The train actually points west
	pev->angles.y += 180;

	if (pev->spawnflags & SF_TRACKTRAIN_NOPITCH)
		pev->angles.x = 0;
	UTIL_SetOrigin(pev, nextPos);
	NextThink(pev->ltime + 0.1, false);
	SetThink(&CFuncTrackTrain::Next);
	pev->speed = m_startSpeed;

	UpdateSound();
}

void CFuncTrackTrain::NearestPath()
{
	CBaseEntity* pTrack = nullptr;
	CBaseEntity* pNearest = nullptr;
	float closest = 1024;

	while ((pTrack = UTIL_FindEntityInSphere(pTrack, pev->origin, 1024)) != nullptr)
	{
		// filter out non-tracks
		if (!(pTrack->pev->flags & (FL_CLIENT | FL_MONSTER)) && ClassnameIs(pTrack->pev, "path_track"))
		{
			const float dist = (pev->origin - pTrack->pev->origin).Length();
			if (dist < closest)
			{
				closest = dist;
				pNearest = pTrack;
			}
		}
	}

	if (!pNearest)
	{
		ALERT(at_console, "Can't find a nearby track !!!\n");
		SetThink(nullptr);
		return;
	}

	ALERT(at_aiconsole, "TRAIN: %s, Nearest track is %s\n", STRING(pev->targetname), STRING(pNearest->pev->targetname));
	// If I'm closer to the next path_track on this path, then it's my real path
	pTrack = ((CPathTrack*)pNearest)->GetNext();
	if (pTrack)
	{
		if ((pev->origin - pTrack->pev->origin).Length() < (pev->origin - pNearest->pev->origin).Length())
			pNearest = pTrack;
	}

	m_ppath = (CPathTrack*)pNearest;

	if (pev->speed != 0)
	{
		NextThink(pev->ltime + 0.1, false);
		SetThink(&CFuncTrackTrain::Next);
	}
}

void CFuncTrackTrain::OverrideReset()
{
	NextThink(pev->ltime + 0.1, false);
	SetThink(&CFuncTrackTrain::NearestPath);
}

CFuncTrackTrain* CFuncTrackTrain::Instance(CBaseEntity* pent)
{
	if (pent && ClassnameIs(pent->pev, "func_tracktrain"))
		return (CFuncTrackTrain*) pent;
	return nullptr;
}

void CFuncTrackTrain::Spawn()
{
	if (pev->speed == 0)
		m_speed = 100;
	else
		m_speed = pev->speed;

	pev->speed = 0;
	pev->velocity = vec3_origin;
	pev->avelocity = vec3_origin;
	pev->impulse = m_speed;

	m_dir = 1;

	if (IsStringNull(pev->target))
		ALERT(at_console, "FuncTrain with no target");

	if (pev->spawnflags & SF_TRACKTRAIN_PASSABLE)
		pev->solid = Solid::Not;
	else
		pev->solid = Solid::BSP;
	pev->movetype = Movetype::Push;

	SET_MODEL(ENT(pev), STRING(pev->model));

	UTIL_SetSize(pev, pev->mins, pev->maxs);
	UTIL_SetOrigin(pev, pev->origin);

	// Cache off placed origin for train controls
	pev->oldorigin = pev->origin;

	m_controlMins = pev->mins;
	m_controlMaxs = pev->maxs;
	m_controlMaxs.z += 72;
	// start trains on the next frame, to make sure their targets have had
	// a chance to spawn/activate
	NextThink(pev->ltime + 0.1, false);
	SetThink(&CFuncTrackTrain::Find);
	Precache();
}

void CFuncTrackTrain::Precache()
{
	if (m_flVolume == 0.0)
		m_flVolume = 1.0;

	switch (m_sounds)
	{
	default:
		// no sound
		pev->noise = iStringNull;
		break;
	case 1: PRECACHE_SOUND("plats/ttrain1.wav"); pev->noise = MAKE_STRING("plats/ttrain1.wav"); break;
	case 2: PRECACHE_SOUND("plats/ttrain2.wav"); pev->noise = MAKE_STRING("plats/ttrain2.wav"); break;
	case 3: PRECACHE_SOUND("plats/ttrain3.wav"); pev->noise = MAKE_STRING("plats/ttrain3.wav"); break;
	case 4: PRECACHE_SOUND("plats/ttrain4.wav"); pev->noise = MAKE_STRING("plats/ttrain4.wav"); break;
	case 5: PRECACHE_SOUND("plats/ttrain6.wav"); pev->noise = MAKE_STRING("plats/ttrain6.wav"); break;
	case 6: PRECACHE_SOUND("plats/ttrain7.wav"); pev->noise = MAKE_STRING("plats/ttrain7.wav"); break;
	}

	PRECACHE_SOUND("plats/ttrain_brake1.wav");
	PRECACHE_SOUND("plats/ttrain_start1.wav");

	m_usAdjustPitch = PRECACHE_EVENT(1, "events/train.sc");
}

/**
*	@brief This class defines the volume of space that the player must stand in to control the train
*/
class CFuncTrainControls : public CBaseEntity
{
public:
	int	ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	void Spawn() override;
	void EXPORT Find();
};

LINK_ENTITY_TO_CLASS(func_traincontrols, CFuncTrainControls);

void CFuncTrainControls::Find()
{
	CBaseEntity* pTarget = nullptr;

	do
	{
		pTarget = UTIL_FindEntityByTargetname(pTarget, STRING(pev->target));
	}
	while (!IsNullEnt(pTarget) && !ClassnameIs(pTarget->pev, "func_tracktrain"));

	if (IsNullEnt(pTarget))
	{
		ALERT(at_console, "No train %s\n", STRING(pev->target));
		return;
	}

	CFuncTrackTrain* ptrain = CFuncTrackTrain::Instance(pTarget);
	ptrain->SetControls(pev);
	UTIL_Remove(this);
}

void CFuncTrainControls::Spawn()
{
	pev->solid = Solid::Not;
	pev->movetype = Movetype::None;
	SET_MODEL(ENT(pev), STRING(pev->model));

	UTIL_SetSize(pev, pev->mins, pev->maxs);
	UTIL_SetOrigin(pev, pev->origin);

	SetThink(&CFuncTrainControls::Find);
	pev->nextthink = gpGlobals->time;
}

constexpr int SF_TRACK_ACTIVATETRAIN = 0x00000001;
constexpr int SF_TRACK_RELINK = 0x00000002;
constexpr int SF_TRACK_ROTMOVE = 0x00000004;
constexpr int SF_TRACK_STARTBOTTOM = 0x00000008;
constexpr int SF_TRACK_DONT_MOVE = 0x00000010;

enum class TrainCode
{
	Safe,
	Blocking,
	Following
};

/**
*	@brief Track changer / Train elevator
*	@details This entity is a rotating/moving platform that will carry a train to a new track.
*	It must be larger in X-Y planar area than the train,
*	since it must contain the train within these dimensions in order to operate when the train is near it.
*/
class CFuncTrackChange : public CFuncPlatRot
{
public:
	void Spawn() override;
	void Precache() override;

	//	void	Blocked() override;
	void	EXPORT GoUp() override;
	void	EXPORT GoDown() override;

	void			KeyValue(KeyValueData* pkvd) override;
	void			Use(const UseInfo& info) override;
	void			EXPORT Find();
	TrainCode		EvaluateTrain(CPathTrack* pcurrent);
	void			UpdateTrain(Vector& dest);
	void	HitBottom() override;
	void	HitTop() override;
	void			Touch(CBaseEntity* pOther) override;

	/**
	*	@brief Normal track change
	*/
	virtual void	UpdateAutoTargets(ToggleState toggleState);
	bool	IsTogglePlat() override { return true; }

	void			DisableUse() { m_use = false; }
	void			EnableUse() { m_use = true; }
	bool UseEnabled() { return m_use; }

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	void	OverrideReset() override;


	CPathTrack* m_trackTop;
	CPathTrack* m_trackBottom;

	CFuncTrackTrain* m_train;

	string_t m_trackTopName;
	string_t m_trackBottomName;
	string_t m_trainName;
	TrainCode m_code;
	ToggleState m_targetState;
	bool m_use;
};

LINK_ENTITY_TO_CLASS(func_trackchange, CFuncTrackChange);

TYPEDESCRIPTION	CFuncTrackChange::m_SaveData[] =
{
	DEFINE_GLOBAL_FIELD(CFuncTrackChange, m_trackTop, FIELD_CLASSPTR),
	DEFINE_GLOBAL_FIELD(CFuncTrackChange, m_trackBottom, FIELD_CLASSPTR),
	DEFINE_GLOBAL_FIELD(CFuncTrackChange, m_train, FIELD_CLASSPTR),
	DEFINE_GLOBAL_FIELD(CFuncTrackChange, m_trackTopName, FIELD_STRING),
	DEFINE_GLOBAL_FIELD(CFuncTrackChange, m_trackBottomName, FIELD_STRING),
	DEFINE_GLOBAL_FIELD(CFuncTrackChange, m_trainName, FIELD_STRING),
	DEFINE_FIELD(CFuncTrackChange, m_code, FIELD_INTEGER),
	DEFINE_FIELD(CFuncTrackChange, m_targetState, FIELD_INTEGER),
	DEFINE_FIELD(CFuncTrackChange, m_use, FIELD_BOOLEAN),
};

IMPLEMENT_SAVERESTORE(CFuncTrackChange, CFuncPlatRot);

void CFuncTrackChange::Spawn()
{
	Setup();
	if (IsBitSet(pev->spawnflags, SF_TRACK_DONT_MOVE))
		m_vecPosition2.z = pev->origin.z;

	SetupRotation();

	if (IsBitSet(pev->spawnflags, SF_TRACK_STARTBOTTOM))
	{
		UTIL_SetOrigin(pev, m_vecPosition2);
		m_toggle_state = ToggleState::AtBottom;
		pev->angles = m_start;
		m_targetState = ToggleState::AtTop;
	}
	else
	{
		UTIL_SetOrigin(pev, m_vecPosition1);
		m_toggle_state = ToggleState::AtTop;
		pev->angles = m_end;
		m_targetState = ToggleState::AtBottom;
	}

	EnableUse();
	pev->nextthink = pev->ltime + 2.0;
	SetThink(&CFuncTrackChange::Find);
	Precache();
}

void CFuncTrackChange::Precache()
{
	// Can't trigger sound
	PRECACHE_SOUND("buttons/button11.wav");

	CFuncPlatRot::Precache();
}

// UNDONE: Filter touches before re-evaluating the train.
void CFuncTrackChange::Touch(CBaseEntity* pOther)
{
#if 0
	TRAIN_CODE code;
	entvars_t* pevToucher = pOther->pev;
#endif
}

void CFuncTrackChange::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "train"))
	{
		m_trainName = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "toptrack"))
	{
		m_trackTopName = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "bottomtrack"))
	{
		m_trackBottomName = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
	{
		CFuncPlatRot::KeyValue(pkvd);		// Pass up to base class
	}
}

void CFuncTrackChange::OverrideReset()
{
	pev->nextthink = pev->ltime + 1.0;
	SetThink(&CFuncTrackChange::Find);
}

void CFuncTrackChange::Find()
{
	// Find track entities
	CBaseEntity* target = UTIL_FindEntityByTargetname(nullptr, STRING(m_trackTopName));
	if (!IsNullEnt(target))
	{
		m_trackTop = CPathTrack::Instance(target);
		target = UTIL_FindEntityByTargetname(nullptr, STRING(m_trackBottomName));
		if (!IsNullEnt(target))
		{
			m_trackBottom = CPathTrack::Instance(target);
			target = UTIL_FindEntityByTargetname(nullptr, STRING(m_trainName));
			if (!IsNullEnt(target))
			{
				//TODO: redundant lookup
				m_train = CFuncTrackTrain::Instance(UTIL_FindEntityByTargetname(nullptr, STRING(m_trainName)));
				if (!m_train)
				{
					ALERT(at_error, "Can't find train for track change! %s\n", STRING(m_trainName));
					return;
				}
				const Vector center = (pev->absmin + pev->absmax) * 0.5;
				m_trackBottom = m_trackBottom->Nearest(center);
				m_trackTop = m_trackTop->Nearest(center);
				UpdateAutoTargets(m_toggle_state);
				SetThink(nullptr);
				return;
			}
			else
			{
				ALERT(at_error, "Can't find train for track change! %s\n", STRING(m_trainName));
				target = UTIL_FindEntityByTargetname(nullptr, STRING(m_trainName)); //TODO: pointless?
			}
		}
		else
			ALERT(at_error, "Can't find bottom track for track change! %s\n", STRING(m_trackBottomName));
	}
	else
		ALERT(at_error, "Can't find top track for track change! %s\n", STRING(m_trackTopName));
}

TrainCode CFuncTrackChange::EvaluateTrain(CPathTrack* pcurrent)
{
	// Go ahead and work, we don't have anything to switch, so just be an elevator
	if (!pcurrent || !m_train)
		return TrainCode::Safe;

	if (m_train->m_ppath == pcurrent || (pcurrent->m_pprevious && m_train->m_ppath == pcurrent->m_pprevious) ||
		(pcurrent->m_pnext && m_train->m_ppath == pcurrent->m_pnext))
	{
		if (m_train->pev->speed != 0)
			return TrainCode::Blocking;

		const Vector dist = pev->origin - m_train->pev->origin;
		const float length = dist.Length2D();
		if (length < m_train->m_length)		// Empirically determined close distance
			return TrainCode::Following;
		else if (length > (150 + m_train->m_length))
			return TrainCode::Safe;

		return TrainCode::Blocking;
	}

	return TrainCode::Safe;
}

void CFuncTrackChange::UpdateTrain(Vector& dest)
{
	const float time = pev->nextthink - pev->ltime;

	m_train->pev->velocity = pev->velocity;
	m_train->pev->avelocity = pev->avelocity;
	m_train->NextThink(m_train->pev->ltime + time, false);

	// Attempt at getting the train to rotate properly around the origin of the trackchange
	if (time <= 0)
		return;

	const Vector offset = m_train->pev->origin - pev->origin;
	const Vector delta = dest - pev->angles;
	// Transform offset into local coordinates
	UTIL_MakeInvVectors(delta, gpGlobals);
	Vector local
	{
		DotProduct(offset, gpGlobals->v_forward),
		DotProduct(offset, gpGlobals->v_right),
		DotProduct(offset, gpGlobals->v_up)
	};

	local = local - offset;
	m_train->pev->velocity = pev->velocity + (local * (1.0 / time));
}

void CFuncTrackChange::GoDown()
{
	if (m_code == TrainCode::Blocking)
		return;

	// HitBottom may get called during CFuncPlat::GoDown(), so set up for that
	// before you call GoDown()

	UpdateAutoTargets(ToggleState::GoingDown);
	// If ROTMOVE, move & rotate
	if (IsBitSet(pev->spawnflags, SF_TRACK_DONT_MOVE))
	{
		SetMoveDone(&CFuncTrackChange::CallHitBottom);
		m_toggle_state = ToggleState::GoingDown;
		AngularMove(m_start, pev->speed);
	}
	else
	{
		CFuncPlat::GoDown();
		SetMoveDone(&CFuncTrackChange::CallHitBottom);
		RotMove(m_start, pev->nextthink - pev->ltime);
	}
	// Otherwise, rotate first, move second

	// If the train is moving with the platform, update it
	if (m_code == TrainCode::Following)
	{
		UpdateTrain(m_start);
		m_train->m_ppath = nullptr;
	}
}

void CFuncTrackChange::GoUp()
{
	if (m_code == TrainCode::Blocking)
		return;

	// HitTop may get called during CFuncPlat::GoUp(), so set up for that
	// before you call GoUp();

	UpdateAutoTargets(ToggleState::GoingUp);
	if (IsBitSet(pev->spawnflags, SF_TRACK_DONT_MOVE))
	{
		m_toggle_state = ToggleState::GoingUp;
		SetMoveDone(&CFuncTrackChange::CallHitTop);
		AngularMove(m_end, pev->speed);
	}
	else
	{
		// If ROTMOVE, move & rotate
		CFuncPlat::GoUp();
		SetMoveDone(&CFuncTrackChange::CallHitTop);
		RotMove(m_end, pev->nextthink - pev->ltime);
	}

	// Otherwise, move first, rotate second

	// If the train is moving with the platform, update it
	if (m_code == TrainCode::Following)
	{
		UpdateTrain(m_end);
		m_train->m_ppath = nullptr;
	}
}

void CFuncTrackChange::UpdateAutoTargets(ToggleState toggleState)
{
	if (!m_trackTop || !m_trackBottom)
		return;

	if (toggleState == ToggleState::AtTop)
		ClearBits(m_trackTop->pev->spawnflags, SF_PATH_DISABLED);
	else
		SetBits(m_trackTop->pev->spawnflags, SF_PATH_DISABLED);

	if (toggleState == ToggleState::AtBottom)
		ClearBits(m_trackBottom->pev->spawnflags, SF_PATH_DISABLED);
	else
		SetBits(m_trackBottom->pev->spawnflags, SF_PATH_DISABLED);
}

void CFuncTrackChange::Use(const UseInfo& info)
{
	if (m_toggle_state != ToggleState::AtTop && m_toggle_state != ToggleState::AtBottom)
		return;

	// If train is in "safe" area, but not on the elevator, play alarm sound
	if (m_toggle_state == ToggleState::AtTop)
		m_code = EvaluateTrain(m_trackTop);
	else if (m_toggle_state == ToggleState::AtBottom)
		m_code = EvaluateTrain(m_trackBottom);
	else
		m_code = TrainCode::Blocking;
	if (m_code == TrainCode::Blocking)
	{
		// Play alarm and return
		EmitSound(SoundChannel::Voice, "buttons/button11.wav");
		return;
	}

	// Otherwise, it's safe to move
	// If at top, go down
	// at bottom, go up

	DisableUse();
	if (m_toggle_state == ToggleState::AtTop)
		GoDown();
	else
		GoUp();
}

void CFuncTrackChange::HitBottom()
{
	CFuncPlatRot::HitBottom();
	if (m_code == TrainCode::Following)
	{
		//		UpdateTrain();
		m_train->SetTrack(m_trackBottom);
	}
	SetThink(nullptr);
	pev->nextthink = -1;

	UpdateAutoTargets(m_toggle_state);

	EnableUse();
}

void CFuncTrackChange::HitTop()
{
	CFuncPlatRot::HitTop();
	if (m_code == TrainCode::Following)
	{
		//		UpdateTrain();
		m_train->SetTrack(m_trackTop);
	}

	// Don't let the plat go back down
	SetThink(nullptr);
	pev->nextthink = -1;
	UpdateAutoTargets(m_toggle_state);
	EnableUse();
}

/**
*	@brief Auto track change
*/
class CFuncTrackAuto : public CFuncTrackChange
{
public:
	void			Use(const UseInfo& info) override;
	void	UpdateAutoTargets(ToggleState toggleState) override;
};

LINK_ENTITY_TO_CLASS(func_trackautochange, CFuncTrackAuto);

void CFuncTrackAuto::UpdateAutoTargets(ToggleState toggleState)
{
	if (!m_trackTop || !m_trackBottom)
		return;

	CPathTrack* pTarget, * pNextTarget;
	if (m_targetState == ToggleState::AtTop)
	{
		pTarget = m_trackTop->GetNext();
		pNextTarget = m_trackBottom->GetNext();
	}
	else
	{
		pTarget = m_trackBottom->GetNext();
		pNextTarget = m_trackTop->GetNext();
	}
	if (pTarget)
	{
		ClearBits(pTarget->pev->spawnflags, SF_PATH_DISABLED);
		if (m_code == TrainCode::Following && m_train && m_train->pev->speed == 0)
			m_train->Use({this, this, USE_ON});
	}

	if (pNextTarget)
		SetBits(pNextTarget->pev->spawnflags, SF_PATH_DISABLED);
}

void CFuncTrackAuto::Use(const UseInfo& info)
{
	if (!UseEnabled())
		return;

	CPathTrack* pTarget;
	if (m_toggle_state == ToggleState::AtTop)
		pTarget = m_trackTop;
	else if (m_toggle_state == ToggleState::AtBottom)
		pTarget = m_trackBottom;
	else
		pTarget = nullptr;

	if (ClassnameIs(info.GetActivator()->pev, "func_tracktrain"))
	{
		m_code = EvaluateTrain(pTarget);
		// Safe to fire?
		if (m_code == TrainCode::Following && m_toggle_state != m_targetState)
		{
			DisableUse();
			if (m_toggle_state == ToggleState::AtTop)
				GoDown();
			else
				GoUp();
		}
	}
	else
	{
		if (pTarget)
			pTarget = pTarget->GetNext();
		if (pTarget && m_train->m_ppath != pTarget && ShouldToggle(info.GetUseType(), m_targetState != ToggleState::AtTop))
		{
			if (m_targetState == ToggleState::AtTop)
				m_targetState = ToggleState::AtBottom;
			else
				m_targetState = ToggleState::AtTop;
		}

		UpdateAutoTargets(m_targetState);
	}
}

constexpr int FGUNTARGET_START_ON = 0x0001;

/**
*	@details pev->speed is the travel speed
*	pev->health is current health
*	pev->max_health is the amount to reset to each time it starts
*/
class CGunTarget : public CBaseMonster
{
public:
	void			Spawn() override;
	void			Activate() override;
	void EXPORT		Next();
	void EXPORT		Start();
	void EXPORT		Wait();
	void			Stop() override;

	int				BloodColor() override { return DONT_BLEED; }
	int				Classify() override { return CLASS_MACHINE; }
	bool TakeDamage(const TakeDamageInfo& info) override;
	void			Use(const UseInfo& info) override;
	Vector			BodyTarget(const Vector& posSrc) override { return pev->origin; }

	int	ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];

private:
	bool			m_on;
};

LINK_ENTITY_TO_CLASS(func_guntarget, CGunTarget);

TYPEDESCRIPTION	CGunTarget::m_SaveData[] =
{
	DEFINE_FIELD(CGunTarget, m_on, FIELD_BOOLEAN),
};

IMPLEMENT_SAVERESTORE(CGunTarget, CBaseMonster);

void CGunTarget::Spawn()
{
	pev->solid = Solid::BSP;
	pev->movetype = Movetype::Push;

	UTIL_SetOrigin(pev, pev->origin);
	SET_MODEL(ENT(pev), STRING(pev->model));

	if (pev->speed == 0)
		pev->speed = 100;

	// Don't take damage until "on"
	SetDamageMode(DamageMode::No);
	pev->flags |= FL_MONSTER;

	m_on = false;
	pev->max_health = pev->health;

	if (pev->spawnflags & FGUNTARGET_START_ON)
	{
		SetThink(&CGunTarget::Start);
		pev->nextthink = pev->ltime + 0.3;
	}
}

void CGunTarget::Activate()
{
	// now find our next target
	if (CBaseEntity* pTarg = GetNextTarget(); pTarg)
	{
		m_hTargetEnt = pTarg;
		UTIL_SetOrigin(pev, pTarg->pev->origin - (pev->mins + pev->maxs) * 0.5);
	}
}

void CGunTarget::Start()
{
	Use({this, this, USE_ON});
}

void CGunTarget::Next()
{
	SetThink(nullptr);

	m_hTargetEnt = GetNextTarget();
	CBaseEntity* pTarget = m_hTargetEnt;

	if (!pTarget)
	{
		Stop();
		return;
	}
	SetMoveDone(&CGunTarget::Wait);
	LinearMove(pTarget->pev->origin - (pev->mins + pev->maxs) * 0.5, pev->speed);
}

void CGunTarget::Wait()
{
	CBaseEntity* pTarget = m_hTargetEnt;

	if (!pTarget)
	{
		Stop();
		return;
	}

	// Fire the pass target if there is one
	if (!IsStringNull(pTarget->pev->message))
	{
		FireTargets(STRING(pTarget->pev->message), this, this, USE_TOGGLE, 0);
		if (IsBitSet(pTarget->pev->spawnflags, SF_CORNER_FIREONCE))
			pTarget->pev->message = iStringNull;
	}

	m_flWait = pTarget->GetDelay();

	pev->target = pTarget->pev->target;
	SetThink(&CGunTarget::Next);
	if (m_flWait != 0)
	{// -1 wait will wait forever!		
		pev->nextthink = pev->ltime + m_flWait;
	}
	else
	{
		Next();// do it RIGHT now!
	}
}

void CGunTarget::Stop()
{
	pev->velocity = vec3_origin;
	pev->nextthink = 0;
	SetDamageMode(DamageMode::No);
}

bool	CGunTarget::TakeDamage(const TakeDamageInfo& info)
{
	if (pev->health > 0)
	{
		pev->health -= info.GetDamage();
		if (pev->health <= 0)
		{
			pev->health = 0;
			Stop();
			if (!IsStringNull(pev->message))
				FireTargets(STRING(pev->message), this, this, USE_TOGGLE, 0);
		}
	}
	return false;
}

void CGunTarget::Use(const UseInfo& info)
{
	if (!ShouldToggle(info.GetUseType(), m_on))
		return;

	if (m_on)
	{
		Stop();
	}
	else
	{
		SetDamageMode(DamageMode::Aim);
		m_hTargetEnt = GetNextTarget();
		if (m_hTargetEnt == nullptr)
			return;
		pev->health = pev->max_health;
		Next();
	}
}
