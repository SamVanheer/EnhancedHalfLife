/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/

constexpr int AFLOCK_MAX_RECRUIT_RADIUS = 1024;
constexpr int AFLOCK_FLY_SPEED = 125;
constexpr int AFLOCK_TURN_RATE = 75;
constexpr int AFLOCK_ACCELERATE = 10;
constexpr int AFLOCK_CHECK_DIST = 192;
constexpr int AFLOCK_TOO_CLOSE = 100;
constexpr int AFLOCK_TOO_FAR = 256;

class CFlockingFlyerFlock : public CBaseMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void KeyValue(KeyValueData* pkvd) override;
	void SpawnFlock();

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	// Sounds are shared by the flock
	static  void PrecacheFlockSounds();

	int		m_cFlockSize;
	float	m_flFlockRadius;
};

TYPEDESCRIPTION	CFlockingFlyerFlock::m_SaveData[] =
{
	DEFINE_FIELD(CFlockingFlyerFlock, m_cFlockSize, FIELD_INTEGER),
	DEFINE_FIELD(CFlockingFlyerFlock, m_flFlockRadius, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CFlockingFlyerFlock, CBaseMonster);

//TODO: should probably make this inherit from CSquadMonster and remove the squad code from this class
class CFlockingFlyer : public CBaseMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void SpawnCommonCode();
	void EXPORT IdleThink();
	void BoidAdvanceFrame();

	/**
	*	@brief Leader boid calls this to form a flock from surrounding boids
	*/
	void EXPORT FormFlock();

	/**
	*	@brief player enters the pvs, so get things going.
	*/
	void EXPORT Start();

	/**
	*	@brief Leader boids use this think every tenth
	*/
	void EXPORT FlockLeaderThink();

	/**
	*	@brief follower boids execute this code when flocking
	*/
	void EXPORT FlockFollowerThink();
	void EXPORT FallHack();
	void MakeSound();

	/**
	*	@brief Searches for boids that are too close and pushes them away
	*/
	void SpreadFlock();

	/**
	*	@brief Alters the caller's course if he's too close to others
	*	This function should **ONLY** be called when Caller's velocity is normalized!!
	*/
	void SpreadFlock2();
	void Killed(const KilledInfo& info) override;

	/**
	*	@brief returns true if there is an obstacle ahead
	*/
	bool PathBlocked();
	//void KeyValue( KeyValueData *pkvd ) override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	int IsLeader() { return m_hSquadLeader == this; }
	int	InSquad() { return m_hSquadLeader != nullptr; }

	/**
	*	@brief return the number of members of this squad
	*	callable from leaders & followers
	*/
	int	SquadCount();

	/**
	*	@brief remove pRemove from my squad.
	*	If I am pRemove, promote m_pSquadNext to leader
	*/
	void SquadRemove(CFlockingFlyer* pRemove);

	/**
	*	@brief Unlink the squad pointers.
	*/
	void SquadUnlink();

	/**
	*	@brief add pAdd to my squad
	*/
	void SquadAdd(CFlockingFlyer* pAdd);

	/**
	*	@brief Unlink all squad members
	*/
	void SquadDisband();

	EHandle<CFlockingFlyer> m_hSquadLeader;
	EHandle<CFlockingFlyer> m_hSquadNext;
	bool	m_fTurning;// is this boid turning?
	bool	m_fCourseAdjust;// followers set this flag true to override flocking while they avoid something
	bool	m_fPathBlocked;// true if there is an obstacle ahead
	Vector	m_vecReferencePoint;// last place we saw leader
	Vector	m_vecAdjustedVelocity;// adjusted velocity (used when fCourseAdjust is true)
	float	m_flGoalSpeed;
	float	m_flLastBlockedTime;
	float	m_flFakeBlockedTime;
	float	m_flAlertTime;
	float	m_flFlockNextSoundTime;
};
LINK_ENTITY_TO_CLASS(monster_flyer, CFlockingFlyer);
LINK_ENTITY_TO_CLASS(monster_flyer_flock, CFlockingFlyerFlock);

TYPEDESCRIPTION	CFlockingFlyer::m_SaveData[] =
{
	DEFINE_FIELD(CFlockingFlyer, m_hSquadLeader, FIELD_EHANDLE),
	DEFINE_FIELD(CFlockingFlyer, m_hSquadNext, FIELD_EHANDLE),
	DEFINE_FIELD(CFlockingFlyer, m_fTurning, FIELD_BOOLEAN),
	DEFINE_FIELD(CFlockingFlyer, m_fCourseAdjust, FIELD_BOOLEAN),
	DEFINE_FIELD(CFlockingFlyer, m_fPathBlocked, FIELD_BOOLEAN),
	DEFINE_FIELD(CFlockingFlyer, m_vecReferencePoint, FIELD_POSITION_VECTOR),
	DEFINE_FIELD(CFlockingFlyer, m_vecAdjustedVelocity, FIELD_VECTOR),
	DEFINE_FIELD(CFlockingFlyer, m_flGoalSpeed, FIELD_FLOAT),
	DEFINE_FIELD(CFlockingFlyer, m_flLastBlockedTime, FIELD_TIME),
	DEFINE_FIELD(CFlockingFlyer, m_flFakeBlockedTime, FIELD_TIME),
	DEFINE_FIELD(CFlockingFlyer, m_flAlertTime, FIELD_TIME),
	//	DEFINE_FIELD( CFlockingFlyer, m_flFlockNextSoundTime, FIELD_TIME ),	// don't need to save
};

IMPLEMENT_SAVERESTORE(CFlockingFlyer, CBaseMonster);

void CFlockingFlyerFlock::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "iFlockSize"))
	{
		m_cFlockSize = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "flFlockRadius"))
	{
		m_flFlockRadius = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
}

void CFlockingFlyerFlock::Spawn()
{
	Precache();
	SpawnFlock();

	UTIL_RemoveNow(this);		// dump the spawn ent
}

void CFlockingFlyerFlock::Precache()
{
	//PRECACHE_MODEL("models/aflock.mdl");		
	PRECACHE_MODEL("models/boid.mdl");

	PrecacheFlockSounds();
}

void CFlockingFlyerFlock::PrecacheFlockSounds()
{
	PRECACHE_SOUND("boid/boid_alert1.wav");
	PRECACHE_SOUND("boid/boid_alert2.wav");

	PRECACHE_SOUND("boid/boid_idle1.wav");
	PRECACHE_SOUND("boid/boid_idle2.wav");
}

void CFlockingFlyerFlock::SpawnFlock()
{
	const float R = m_flFlockRadius;

	CFlockingFlyer* pLeader = nullptr;

	for (int iCount = 0; iCount < m_cFlockSize; iCount++)
	{
		CFlockingFlyer* pBoid = GetClassPtr((CFlockingFlyer*)nullptr);

		if (!pLeader)
		{
			// make this guy the leader.
			pLeader = pBoid;

			pLeader->m_hSquadLeader = pLeader;
			pLeader->m_hSquadNext = nullptr;
		}

		const Vector vecSpot{GetAbsOrigin() + Vector{RANDOM_FLOAT(-R, R), RANDOM_FLOAT(-R, R), RANDOM_FLOAT(0, 16)}};

		pBoid->SetAbsOrigin(vecSpot);
		pBoid->SetMovetype(Movetype::Fly);
		pBoid->SpawnCommonCode();
		pBoid->pev->flags &= ~FL_ONGROUND;
		pBoid->SetAbsVelocity(vec3_origin);
		pBoid->SetAbsAngles(GetAbsAngles());

		pBoid->pev->frame = 0;
		pBoid->pev->nextthink = gpGlobals->time + 0.2;
		pBoid->SetThink(&CFlockingFlyer::IdleThink);

		if (pBoid != pLeader)
		{
			pLeader->SquadAdd(pBoid);
		}
	}
}

void CFlockingFlyer::Spawn()
{
	Precache();
	SpawnCommonCode();

	pev->frame = 0;
	pev->nextthink = gpGlobals->time + 0.1;
	SetThink(&CFlockingFlyer::IdleThink);
}

void CFlockingFlyer::Precache()
{
	//PRECACHE_MODEL("models/aflock.mdl");
	PRECACHE_MODEL("models/boid.mdl");
	CFlockingFlyerFlock::PrecacheFlockSounds();
}

void CFlockingFlyer::MakeSound()
{
	if (m_flAlertTime > gpGlobals->time)
	{
		// make agitated sounds
		switch (RANDOM_LONG(0, 1))
		{
		case 0:	EmitSound(SoundChannel::Weapon, "boid/boid_alert1.wav"); break;
		case 1:	EmitSound(SoundChannel::Weapon, "boid/boid_alert2.wav"); break;
		}

		return;
	}

	// make normal sound
	switch (RANDOM_LONG(0, 1))
	{
	case 0:	EmitSound(SoundChannel::Weapon, "boid/boid_idle1.wav"); break;
	case 1:	EmitSound(SoundChannel::Weapon, "boid/boid_idle2.wav"); break;
	}
}

void CFlockingFlyer::Killed(const KilledInfo& info)
{
	for (CFlockingFlyer* pSquad = m_hSquadLeader; pSquad; pSquad = pSquad->m_hSquadNext)
	{
		pSquad->m_flAlertTime = gpGlobals->time + 15;
	}

	if (m_hSquadLeader)
	{
		m_hSquadLeader->SquadRemove(this);
	}

	pev->deadflag = DeadFlag::Dead;

	pev->framerate = 0;
	pev->effects = EF_NOINTERP;

	SetSize(vec3_origin, vec3_origin);
	SetMovetype(Movetype::Toss);

	SetThink(&CFlockingFlyer::FallHack);
	pev->nextthink = gpGlobals->time + 0.1;
}

void CFlockingFlyer::FallHack()
{
	if (pev->flags & FL_ONGROUND)
	{
		if (!Instance(pev->groundentity)->ClassnameIs("worldspawn"))
		{
			pev->flags &= ~FL_ONGROUND;
			pev->nextthink = gpGlobals->time + 0.1;
		}
		else
		{
			SetAbsVelocity(vec3_origin);
			SetThink(nullptr);
		}
	}
}

void CFlockingFlyer::SpawnCommonCode()
{
	pev->deadflag = DeadFlag::No;
	SetClassname("monster_flyer");
	SetSolidType(Solid::SlideBox);
	SetMovetype(Movetype::Fly);
	SetDamageMode(DamageMode::No);
	pev->health = 1;

	m_fPathBlocked = false;// obstacles will be detected
	m_flFieldOfView = 0.2;

	//SetModel( "models/aflock.mdl");
	SetModel("models/boid.mdl");

	//	SetSize( vec3_origin, vec3_origin);
	SetSize(Vector(-5, -5, 0), Vector(5, 5, 2));
}

void CFlockingFlyer::BoidAdvanceFrame()
{
	pev->armorvalue = pev->armorvalue * .8 + pev->speed * .2;

	const float flapspeed = std::clamp(std::abs((pev->speed - pev->armorvalue) / AFLOCK_ACCELERATE), 0.25f, 1.9f);

	pev->framerate = flapspeed;

	// lean
	pev->avelocity.x = -(GetAbsAngles().x + flapspeed * 5);

	// bank
	pev->avelocity.z = -(GetAbsAngles().z + pev->avelocity.y);

	// pev->framerate		= flapspeed;
	StudioFrameAdvance(0.1);
}

void CFlockingFlyer::IdleThink()
{
	pev->nextthink = gpGlobals->time + 0.2;

	// see if there's a client in the same pvs as the monster
	if (!IsNullEnt(UTIL_FindClientInPVS(this)))
	{
		SetThink(&CFlockingFlyer::Start);
		pev->nextthink = gpGlobals->time + 0.1;
	}
}

void CFlockingFlyer::Start()
{
	pev->nextthink = gpGlobals->time + 0.1;

	if (IsLeader())
	{
		SetThink(&CFlockingFlyer::FlockLeaderThink);
	}
	else
	{
		SetThink(&CFlockingFlyer::FlockFollowerThink);
	}

	/*
		Vector	vecTakeOff;
		vecTakeOff = Vector ( 0 , 0 , 0 );

		vecTakeOff.z = 50 + RANDOM_FLOAT ( 0, 100 );
		vecTakeOff.x = 20 - RANDOM_FLOAT ( 0, 40);
		vecTakeOff.y = 20 - RANDOM_FLOAT ( 0, 40);

		SetAbsVelocity(vecTakeOff);


		pev->speed = GetAbsVelocity().Length();
		pev->sequence = 0;
	*/
	SetActivity(ACT_FLY);
	ResetSequenceInfo();
	BoidAdvanceFrame();

	pev->speed = AFLOCK_FLY_SPEED;// no delay!
}

void CFlockingFlyer::FormFlock()
{
	if (!InSquad())
	{
		// I am my own leader
		m_hSquadLeader = this;
		m_hSquadNext = nullptr;
		int squadCount = 1;

		CBaseEntity* pEntity = nullptr;

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, GetAbsOrigin(), AFLOCK_MAX_RECRUIT_RADIUS)) != nullptr)
		{
			if (CBaseMonster* pRecruit = pEntity->MyMonsterPointer();
				pRecruit && pRecruit != this && pRecruit->IsAlive() && !pRecruit->m_hCine)
			{
				// Can we recruit this guy?
				if (pRecruit->ClassnameIs("monster_flyer"))
				{
					squadCount++;
					SquadAdd((CFlockingFlyer*)pRecruit);
				}
			}
		}
	}

	SetThink(&CFlockingFlyer::IdleThink);// now that flock is formed, go to idle and wait for a player to come along.
	pev->nextthink = gpGlobals->time;
}

void CFlockingFlyer::SpreadFlock()
{
	for (CFlockingFlyer* pList = m_hSquadLeader; pList; pList = pList->m_hSquadNext)
	{
		if (pList != this && (GetAbsOrigin() - pList->GetAbsOrigin()).Length() <= AFLOCK_TOO_CLOSE)
		{
			// push the other away
			const Vector vecDir = (pList->GetAbsOrigin() - GetAbsOrigin()).Normalize();

			// store the magnitude of the other boid's velocity, and normalize it so we
			// can average in a course that points away from the leader.
			const float flSpeed = pList->GetAbsVelocity().Length();
			pList->SetAbsVelocity((pList->GetAbsVelocity().Normalize() + vecDir) * 0.5 * flSpeed);
		}
	}
}

void CFlockingFlyer::SpreadFlock2()
{
	for (CFlockingFlyer* pList = m_hSquadLeader; pList; pList = pList->m_hSquadNext)
	{
		if (pList != this && (GetAbsOrigin() - pList->GetAbsOrigin()).Length() <= AFLOCK_TOO_CLOSE)
		{
			const Vector vecDir = (GetAbsOrigin() - pList->GetAbsOrigin()).Normalize();

			SetAbsVelocity(GetAbsVelocity() + vecDir);
		}
	}
}

bool CFlockingFlyer::PathBlocked()
{
	if (m_flFakeBlockedTime > gpGlobals->time)
	{
		m_flLastBlockedTime = gpGlobals->time;
		return true;
	}

	// use VELOCITY, not angles, not all boids point the direction they are flying
	//vecDir = VectorAngles( pevBoid->velocity );
	UTIL_MakeVectors(GetAbsAngles());

	bool fBlocked = false;// assume the way ahead is clear

	// check for obstacle ahead
	TraceResult tr;
	UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + gpGlobals->v_forward * AFLOCK_CHECK_DIST, IgnoreMonsters::Yes, this, &tr);
	if (tr.flFraction != 1.0)
	{
		m_flLastBlockedTime = gpGlobals->time;
		fBlocked = true;
	}

	// extra wide checks
	UTIL_TraceLine(GetAbsOrigin() + gpGlobals->v_right * 12, GetAbsOrigin() + gpGlobals->v_right * 12 + gpGlobals->v_forward * AFLOCK_CHECK_DIST, IgnoreMonsters::Yes, this, &tr);
	if (tr.flFraction != 1.0)
	{
		m_flLastBlockedTime = gpGlobals->time;
		fBlocked = true;
	}

	UTIL_TraceLine(GetAbsOrigin() - gpGlobals->v_right * 12, GetAbsOrigin() - gpGlobals->v_right * 12 + gpGlobals->v_forward * AFLOCK_CHECK_DIST, IgnoreMonsters::Yes, this, &tr);
	if (tr.flFraction != 1.0)
	{
		m_flLastBlockedTime = gpGlobals->time;
		fBlocked = true;
	}

	if (!fBlocked && gpGlobals->time - m_flLastBlockedTime > 6)
	{
		// not blocked, and it's been a few seconds since we've actually been blocked.
		m_flFakeBlockedTime = gpGlobals->time + RANDOM_LONG(1, 3);
	}

	return	fBlocked;
}

void CFlockingFlyer::FlockLeaderThink()
{
	pev->nextthink = gpGlobals->time + 0.1;

	UTIL_MakeVectors(GetAbsAngles());

	// is the way ahead clear?
	if (!PathBlocked())
	{
		// if the boid is turning, stop the trend.
		if (m_fTurning)
		{
			m_fTurning = false;
			pev->avelocity.y = 0;
		}

		m_fPathBlocked = false;

		if (pev->speed <= AFLOCK_FLY_SPEED)
			pev->speed += 5;

		SetAbsVelocity(gpGlobals->v_forward * pev->speed);

		BoidAdvanceFrame();

		return;
	}

	// IF we get this far in the function, the leader's path is blocked!
	m_fPathBlocked = true;

	TraceResult tr;

	if (!m_fTurning)// something in the way and boid is not already turning to avoid
	{
		// measure clearance on left and right to pick the best dir to turn
		UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + gpGlobals->v_right * AFLOCK_CHECK_DIST, IgnoreMonsters::Yes, this, &tr);
		const float flRightSide = (tr.vecEndPos - GetAbsOrigin()).Length();

		UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() - gpGlobals->v_right * AFLOCK_CHECK_DIST, IgnoreMonsters::Yes, this, &tr);
		const float flLeftSide = (tr.vecEndPos - GetAbsOrigin()).Length();

		// turn right if more clearance on right side
		if (flRightSide > flLeftSide)
		{
			pev->avelocity.y = -AFLOCK_TURN_RATE;
			m_fTurning = true;
		}
		// default to left turn :)
		else if (flLeftSide > flRightSide)
		{
			pev->avelocity.y = AFLOCK_TURN_RATE;
			m_fTurning = true;
		}
		else
		{
			// equidistant. Pick randomly between left and right.
			m_fTurning = true;

			if (RANDOM_LONG(0, 1) == 0)
			{
				pev->avelocity.y = AFLOCK_TURN_RATE;
			}
			else
			{
				pev->avelocity.y = -AFLOCK_TURN_RATE;
			}
		}
	}
	SpreadFlock();

	SetAbsVelocity(gpGlobals->v_forward * pev->speed);

	// check and make sure we aren't about to plow into the ground, don't let it happen
	UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() - gpGlobals->v_up * 16, IgnoreMonsters::Yes, this, &tr);
	if (tr.flFraction != 1.0 && GetAbsVelocity().z < 0)
		SetAbsVelocity({GetAbsVelocity().x, GetAbsVelocity().y, 0});

	// maybe it did, though.
	if (IsBitSet(pev->flags, FL_ONGROUND))
	{
		SetAbsOrigin(GetAbsOrigin() + Vector(0, 0, 1));
		SetAbsVelocity({GetAbsVelocity().x, GetAbsVelocity().y, 0});
	}

	if (m_flFlockNextSoundTime < gpGlobals->time)
	{
		MakeSound();
		m_flFlockNextSoundTime = gpGlobals->time + RANDOM_FLOAT(1, 3);
	}

	BoidAdvanceFrame();
}

void CFlockingFlyer::FlockFollowerThink()
{
	pev->nextthink = gpGlobals->time + 0.1;

	if (IsLeader() || !InSquad())
	{
		// the leader has been killed and this flyer suddenly finds himself the leader. 
		SetThink(&CFlockingFlyer::FlockLeaderThink);
		return;
	}

	auto squadLeader = m_hSquadLeader.Get();

	Vector vecDirToLeader = squadLeader->GetAbsOrigin() - GetAbsOrigin();
	const float flDistToLeader = vecDirToLeader.Length();

	// match heading with leader
	SetAbsAngles(squadLeader->GetAbsAngles());

	//
	// We can see the leader, so try to catch up to it
	//
	if (IsInViewCone(squadLeader))
	{
		// if we're too far away, speed up
		if (flDistToLeader > AFLOCK_TOO_FAR)
		{
			m_flGoalSpeed = squadLeader->GetAbsVelocity().Length() * 1.5;
		}

		// if we're too close, slow down
		else if (flDistToLeader < AFLOCK_TOO_CLOSE)
		{
			m_flGoalSpeed = squadLeader->GetAbsVelocity().Length() * 0.5;
		}
	}
	else
	{
		// wait up! the leader isn't out in front, so we slow down to let him pass
		m_flGoalSpeed = squadLeader->GetAbsVelocity().Length() * 0.5;
	}

	SpreadFlock2();

	pev->speed = GetAbsVelocity().Length();
	SetAbsVelocity(GetAbsVelocity().Normalize());

	// if we are too far from leader, average a vector towards it into our current velocity
	if (flDistToLeader > AFLOCK_TOO_FAR)
	{
		vecDirToLeader = vecDirToLeader.Normalize();
		SetAbsVelocity((GetAbsVelocity() + vecDirToLeader) * 0.5);
	}

	// clamp speeds and handle acceleration
	if (m_flGoalSpeed > AFLOCK_FLY_SPEED * 2)
	{
		m_flGoalSpeed = AFLOCK_FLY_SPEED * 2;
	}

	if (pev->speed < m_flGoalSpeed)
	{
		pev->speed += AFLOCK_ACCELERATE;
	}
	else if (pev->speed > m_flGoalSpeed)
	{
		pev->speed -= AFLOCK_ACCELERATE;
	}

	SetAbsVelocity(GetAbsVelocity() * pev->speed);

	BoidAdvanceFrame();
}

/*
	// Is this boid's course blocked?
	if ( FBoidPathBlocked (pev) )
	{
		// course is still blocked from last time. Just keep flying along adjusted
		// velocity
		if ( m_fCourseAdjust )
		{
			SetAbsVelocity(m_vecAdjustedVelocity * pev->speed);
			return;
		}
		else // set course adjust flag and calculate adjusted velocity
		{
			m_fCourseAdjust = true;

			// use VELOCITY, not angles, not all boids point the direction they are flying
			//const Vector vecDir = VectorAngles(GetAbsVelocity());
			//UTIL_MakeVectors ( vecDir );

			UTIL_MakeVectors (GetAbsAngles());

			// measure clearance on left and right to pick the best dir to turn
			TraceResult tr;
			UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + gpGlobals->v_right * AFLOCK_CHECK_DIST, ignore_monsters, ENT(pev), &tr);
			flRightSide = (tr.vecEndPos - GetAbsOrigin()).Length();

			UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() - gpGlobals->v_right * AFLOCK_CHECK_DIST, ignore_monsters, ENT(pev), &tr);
			flLeftSide = (tr.vecEndPos - GetAbsOrigin()).Length();

			// slide right if more clearance on right side
			if ( flRightSide > flLeftSide )
			{
				m_vecAdjustedVelocity = gpGlobals->v_right;
			}
			// else slide left
			else
			{
				m_vecAdjustedVelocity = gpGlobals->v_right * -1;
			}
		}
		return;
	}

	// if we make it this far, boids path is CLEAR!
	m_fCourseAdjust = false;
*/

void CFlockingFlyer::SquadUnlink()
{
	m_hSquadLeader = nullptr;
	m_hSquadNext = nullptr;
}

void CFlockingFlyer::SquadAdd(CFlockingFlyer* pAdd)
{
	ASSERT(pAdd != nullptr);
	ASSERT(!pAdd->InSquad());
	ASSERT(this->IsLeader());

	pAdd->m_hSquadNext = m_hSquadNext;
	m_hSquadNext = pAdd;
	pAdd->m_hSquadLeader = this;
}

void CFlockingFlyer::SquadRemove(CFlockingFlyer* pRemove)
{
	ASSERT(pRemove != nullptr);
	ASSERT(this->IsLeader());
	ASSERT(pRemove->m_hSquadLeader == this);

	if (SquadCount() > 2)
	{
		// Removing the leader, promote m_pSquadNext to leader
		if (pRemove == this)
		{
			CFlockingFlyer* pLeader = m_hSquadNext;

			// copy the enemy LKP to the new leader
			pLeader->m_vecEnemyLKP = m_vecEnemyLKP;

			//TODO: nullcheck after accessing pointer
			if (pLeader)
			{
				for (CFlockingFlyer* pList = pLeader; pList; pList = pList->m_hSquadNext)
				{
					pList->m_hSquadLeader = pLeader;
				}
			}
			SquadUnlink();
		}
		else	// removing a node
		{
			CFlockingFlyer* pList;

			// Find the node before pRemove
			for (pList = this; pList->m_hSquadNext != pRemove; pList = pList->m_hSquadNext)
			{
				// assert to test valid list construction
				ASSERT(pList->m_hSquadNext != nullptr);
			}
			// List validity
			ASSERT(pList->m_hSquadNext == pRemove);

			// Relink without pRemove
			pList->m_hSquadNext = pRemove->m_hSquadNext;

			// Unlink pRemove
			pRemove->SquadUnlink();
		}
	}
	else
		SquadDisband();
}

int CFlockingFlyer::SquadCount()
{
	int squadCount = 0;
	for (CFlockingFlyer* pList = m_hSquadLeader; pList; pList = pList->m_hSquadNext)
	{
		squadCount++;
	}

	return squadCount;
}

void CFlockingFlyer::SquadDisband()
{
	for (CFlockingFlyer* pNext = nullptr, *pList = m_hSquadLeader; pList; pList = pNext)
	{
		pNext = pList->m_hSquadNext;
		pList->SquadUnlink();
	}
}
