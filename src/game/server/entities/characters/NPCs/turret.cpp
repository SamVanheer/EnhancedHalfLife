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

constexpr int TURRET_SHOTS = 2;
constexpr int TURRET_RANGE = 100 * 12;
constexpr Vector TURRET_SPREAD(0, 0, 0);
constexpr int TURRET_TURNRATE = 30;				//angles per 0.1 second
constexpr int TURRET_MAXWAIT = 15;				// seconds turret will stay active w/o a target
constexpr int TURRET_MAXSPIN = 5;				// seconds turret barrel will spin w/o a target
constexpr float TURRET_MACHINE_VOLUME = 0.5;

enum class TurretAnim
{
	None = 0,
	Fire,
	Spin,
	Deploy,
	Retire,
	Die,
};

enum class TurretOrientation
{
	Floor = 0,
	Ceiling
};

class CBaseTurret : public CBaseMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void KeyValue(KeyValueData* pkvd) override;
	void EXPORT TurretUse(const UseInfo& info);

	void TraceAttack(const TraceAttackInfo& info) override;
	bool TakeDamage(const TakeDamageInfo& info) override;

	/**
	*	@brief ID as a machine
	*/
	int	 Classify() override;

	int BloodColor() override { return DONT_BLEED; }
	void GibMonster() override {}	// UNDONE: Throw turret gibs?

	// Think functions

	void EXPORT ActiveThink();

	/**
	*	@brief This search function will sit with the turret deployed and look for a new target.
	*	After a set amount of time, the barrel will spin down. After m_flMaxWait, the turret will retract.
	*/
	void EXPORT SearchThink();

	/**
	*	@brief This think function will deploy the turret when something comes into range.
	*	This is for automatically activated turrets.
	*/
	void EXPORT AutoSearchThink();
	void EXPORT TurretDeath();

	virtual void EXPORT SpinDownCall() { m_iSpin = false; }
	virtual void EXPORT SpinUpCall() { m_iSpin = true; }

	void EXPORT Deploy();
	void EXPORT Retire();

	void EXPORT Initialize();

	virtual void Ping();
	virtual void EyeOn();
	virtual void EyeOff();

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];

	// other functions
	void SetTurretAnim(TurretAnim anim);
	bool MoveTurret();
	virtual void Shoot(Vector& vecSrc, Vector& vecDirToEnemy) {}

	float m_flMaxSpin;		// Max time to spin the barrel w/o a target
	bool m_iSpin;

	EHandle<CSprite> m_hEyeGlow; //TODO: need to remove this entity on death
	int		m_eyeBrightness;

	int	m_iDeployHeight;
	int	m_iRetractHeight;
	int m_iMinPitch;

	int m_iBaseTurnRate;	// angles per second
	float m_fTurnRate;		// actual turn rate
	TurretOrientation m_iOrientation;
	bool m_iOn;
	bool m_fBeserk;			// Sometimes this bitch will just freak out
	bool m_iAutoStart;		// true if the turret auto deploys when a target
							// enters its range

	Vector m_vecLastSight;
	float m_flLastSight;	// Last time we saw a target
	float m_flMaxWait;		// Max time to seach w/o a target
	int m_iSearchSpeed;		// Not Used!

	// movement
	float	m_flStartYaw;
	Vector	m_vecCurAngles;
	Vector	m_vecGoalAngles;

	float	m_flPingTime;	// Time until the next ping, used when searching
	float	m_flSpinUpTime;	// Amount of time until the barrel should spin down when searching
};

TYPEDESCRIPTION	CBaseTurret::m_SaveData[] =
{
	DEFINE_FIELD(CBaseTurret, m_flMaxSpin, FIELD_FLOAT),
	DEFINE_FIELD(CBaseTurret, m_iSpin, FIELD_BOOLEAN),

	DEFINE_FIELD(CBaseTurret, m_hEyeGlow, FIELD_EHANDLE),
	DEFINE_FIELD(CBaseTurret, m_eyeBrightness, FIELD_INTEGER),
	DEFINE_FIELD(CBaseTurret, m_iDeployHeight, FIELD_INTEGER),
	DEFINE_FIELD(CBaseTurret, m_iRetractHeight, FIELD_INTEGER),
	DEFINE_FIELD(CBaseTurret, m_iMinPitch, FIELD_INTEGER),

	DEFINE_FIELD(CBaseTurret, m_iBaseTurnRate, FIELD_INTEGER),
	DEFINE_FIELD(CBaseTurret, m_fTurnRate, FIELD_FLOAT),
	DEFINE_FIELD(CBaseTurret, m_iOrientation, FIELD_INTEGER),
	DEFINE_FIELD(CBaseTurret, m_iOn, FIELD_BOOLEAN),
	DEFINE_FIELD(CBaseTurret, m_fBeserk, FIELD_BOOLEAN),
	DEFINE_FIELD(CBaseTurret, m_iAutoStart, FIELD_BOOLEAN),

	DEFINE_FIELD(CBaseTurret, m_vecLastSight, FIELD_POSITION_VECTOR),
	DEFINE_FIELD(CBaseTurret, m_flLastSight, FIELD_TIME),
	DEFINE_FIELD(CBaseTurret, m_flMaxWait, FIELD_FLOAT),
	DEFINE_FIELD(CBaseTurret, m_iSearchSpeed, FIELD_INTEGER),

	DEFINE_FIELD(CBaseTurret, m_flStartYaw, FIELD_FLOAT),
	DEFINE_FIELD(CBaseTurret, m_vecCurAngles, FIELD_VECTOR),
	DEFINE_FIELD(CBaseTurret, m_vecGoalAngles, FIELD_VECTOR),

	DEFINE_FIELD(CBaseTurret, m_flPingTime, FIELD_TIME),
	DEFINE_FIELD(CBaseTurret, m_flSpinUpTime, FIELD_TIME),
};

IMPLEMENT_SAVERESTORE(CBaseTurret, CBaseMonster);

class CTurret : public CBaseTurret
{
public:
	void Spawn() override;
	void Precache() override;
	// Think functions
	void SpinUpCall() override;
	void SpinDownCall() override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];

	// other functions
	void Shoot(Vector& vecSrc, Vector& vecDirToEnemy) override;

private:
	bool m_iStartSpin;

};

TYPEDESCRIPTION	CTurret::m_SaveData[] =
{
	DEFINE_FIELD(CTurret, m_iStartSpin, FIELD_BOOLEAN),
};

IMPLEMENT_SAVERESTORE(CTurret, CBaseTurret);

class CMiniTurret : public CBaseTurret
{
public:
	void Spawn() override;
	void Precache() override;
	// other functions
	void Shoot(Vector& vecSrc, Vector& vecDirToEnemy) override;
};

LINK_ENTITY_TO_CLASS(monster_turret, CTurret);
LINK_ENTITY_TO_CLASS(monster_miniturret, CMiniTurret);

void CBaseTurret::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "maxsleep"))
	{
		m_flMaxWait = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "orientation"))
	{
		//TODO: validate input
		m_iOrientation = static_cast<TurretOrientation>(atoi(pkvd->szValue));
		pkvd->fHandled = true;

	}
	else if (AreStringsEqual(pkvd->szKeyName, "searchspeed"))
	{
		m_iSearchSpeed = atoi(pkvd->szValue);
		pkvd->fHandled = true;

	}
	else if (AreStringsEqual(pkvd->szKeyName, "turnrate"))
	{
		m_iBaseTurnRate = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "style") ||
		AreStringsEqual(pkvd->szKeyName, "height") ||
		AreStringsEqual(pkvd->szKeyName, "value1") ||
		AreStringsEqual(pkvd->szKeyName, "value2") ||
		AreStringsEqual(pkvd->szKeyName, "value3"))
		pkvd->fHandled = true;
	else
		CBaseMonster::KeyValue(pkvd);
}

void CBaseTurret::Spawn()
{
	Precache();
	pev->nextthink = gpGlobals->time + 1;
	pev->movetype = Movetype::Fly;
	pev->sequence = 0;
	pev->frame = 0;
	SetSolidType(Solid::SlideBox);
	SetDamageMode(DamageMode::Aim);

	SetBits(pev->flags, FL_MONSTER);
	SetUse(&CBaseTurret::TurretUse);

	if ((pev->spawnflags & SF_MONSTER_TURRET_AUTOACTIVATE)
		&& !(pev->spawnflags & SF_MONSTER_TURRET_STARTINACTIVE))
	{
		m_iAutoStart = true;
	}

	ResetSequenceInfo();
	SetBoneController(0, 0);
	SetBoneController(1, 0);
	m_flFieldOfView = VIEW_FIELD_FULL;
	// m_flSightRange = TURRET_RANGE;
}

void CBaseTurret::Precache()
{
	PRECACHE_SOUND("turret/tu_fire1.wav");
	PRECACHE_SOUND("turret/tu_ping.wav");
	PRECACHE_SOUND("turret/tu_active2.wav");
	PRECACHE_SOUND("turret/tu_die.wav");
	PRECACHE_SOUND("turret/tu_die2.wav");
	PRECACHE_SOUND("turret/tu_die3.wav");
	// PRECACHE_SOUND ("turret/tu_retract.wav"); // just use deploy sound to save memory
	PRECACHE_SOUND("turret/tu_deploy.wav");
	PRECACHE_SOUND("turret/tu_spinup.wav");
	PRECACHE_SOUND("turret/tu_spindown.wav");
	PRECACHE_SOUND("turret/tu_search.wav");
	PRECACHE_SOUND("turret/tu_alert.wav");
}

constexpr std::string_view TURRET_GLOW_SPRITE{"sprites/flare3.spr"};

void CTurret::Spawn()
{
	Precache();
	SetModel("models/turret.mdl");
	pev->health = gSkillData.turretHealth;
	m_HackedGunPos = Vector(0, 0, 12.75);
	m_flMaxSpin = TURRET_MAXSPIN;
	pev->view_ofs.z = 12.75;

	CBaseTurret::Spawn();

	m_iRetractHeight = 16;
	m_iDeployHeight = 32;
	m_iMinPitch = -15;
	SetSize(Vector(-32, -32, -m_iRetractHeight), Vector(32, 32, m_iRetractHeight));

	SetThink(&CTurret::Initialize);

	auto glow = m_hEyeGlow = CSprite::SpriteCreate(TURRET_GLOW_SPRITE.data(), pev->origin, false);
	glow->SetTransparency(RenderMode::Glow, 255, 0, 0, 0, RenderFX::NoDissipation);
	glow->SetAttachment(this, 2);
	m_eyeBrightness = 0;

	pev->nextthink = gpGlobals->time + 0.3;
}

void CTurret::Precache()
{
	CBaseTurret::Precache();
	PRECACHE_MODEL("models/turret.mdl");
	PRECACHE_MODEL(TURRET_GLOW_SPRITE.data());
}

void CMiniTurret::Spawn()
{
	Precache();
	SetModel("models/miniturret.mdl");
	pev->health = gSkillData.miniturretHealth;
	m_HackedGunPos = Vector(0, 0, 12.75);
	m_flMaxSpin = 0;
	pev->view_ofs.z = 12.75;

	CBaseTurret::Spawn();
	m_iRetractHeight = 16;
	m_iDeployHeight = 32;
	m_iMinPitch = -15;
	SetSize(Vector(-16, -16, -m_iRetractHeight), Vector(16, 16, m_iRetractHeight));

	SetThink(&CMiniTurret::Initialize);
	pev->nextthink = gpGlobals->time + 0.3;
}

void CMiniTurret::Precache()
{
	CBaseTurret::Precache();
	PRECACHE_MODEL("models/miniturret.mdl");
	PRECACHE_SOUND("weapons/hks1.wav");
	PRECACHE_SOUND("weapons/hks2.wav");
	PRECACHE_SOUND("weapons/hks3.wav");
}

void CBaseTurret::Initialize()
{
	m_iOn = false;
	m_fBeserk = false;
	m_iSpin = false;

	SetBoneController(0, 0);
	SetBoneController(1, 0);

	if (m_iBaseTurnRate == 0) m_iBaseTurnRate = TURRET_TURNRATE;
	if (m_flMaxWait == 0) m_flMaxWait = TURRET_MAXWAIT;
	m_flStartYaw = pev->angles.y;
	if (m_iOrientation == TurretOrientation::Ceiling)
	{
		pev->idealpitch = 180;
		pev->angles.x = 180;
		pev->view_ofs.z = -pev->view_ofs.z;
		pev->effects |= EF_INVLIGHT;
		pev->angles.y = pev->angles.y + 180;
		if (pev->angles.y > 360)
			pev->angles.y = pev->angles.y - 360;
	}

	m_vecGoalAngles.x = 0;

	if (m_iAutoStart)
	{
		m_flLastSight = gpGlobals->time + m_flMaxWait;
		SetThink(&CBaseTurret::AutoSearchThink);
		pev->nextthink = gpGlobals->time + .1;
	}
	else
		SetThink(&CBaseTurret::SUB_DoNothing);
}

void CBaseTurret::TurretUse(const UseInfo& info)
{
	if (!ShouldToggle(info.GetUseType(), m_iOn))
		return;

	if (m_iOn)
	{
		m_hEnemy = nullptr;
		pev->nextthink = gpGlobals->time + 0.1;
		m_iAutoStart = false;// switching off a turret disables autostart
		//!!!! this should spin down first!!BUGBUG
		SetThink(&CBaseTurret::Retire);
	}
	else
	{
		pev->nextthink = gpGlobals->time + 0.1; // turn on delay

		// if the turret is flagged as an autoactivate turret, re-enable it's ability open self.
		if (pev->spawnflags & SF_MONSTER_TURRET_AUTOACTIVATE)
		{
			m_iAutoStart = true;
		}

		SetThink(&CBaseTurret::Deploy);
	}
}

void CBaseTurret::Ping()
{
	// make the pinging noise every second while searching
	if (m_flPingTime == 0)
		m_flPingTime = gpGlobals->time + 1;
	else if (m_flPingTime <= gpGlobals->time)
	{
		m_flPingTime = gpGlobals->time + 1;
		EmitSound(SoundChannel::Item, "turret/tu_ping.wav");
		EyeOn();
	}
	else if (m_eyeBrightness > 0)
	{
		EyeOff();
	}
}

void CBaseTurret::EyeOn()
{
	if (auto glow = m_hEyeGlow.Get(); glow)
	{
		if (m_eyeBrightness != 255)
		{
			m_eyeBrightness = 255;
		}
		glow->SetBrightness(m_eyeBrightness);
	}
}

void CBaseTurret::EyeOff()
{
	if (auto glow = m_hEyeGlow.Get(); glow)
	{
		if (m_eyeBrightness > 0)
		{
			m_eyeBrightness = std::max(0, m_eyeBrightness - 30);
			glow->SetBrightness(m_eyeBrightness);
		}
	}
}

void CBaseTurret::ActiveThink()
{
	pev->nextthink = gpGlobals->time + 0.1;
	StudioFrameAdvance();

	if ((!m_iOn) || (m_hEnemy == nullptr))
	{
		m_hEnemy = nullptr;
		m_flLastSight = gpGlobals->time + m_flMaxWait;
		SetThink(&CBaseTurret::SearchThink);
		return;
	}

	// if it's dead, look for something new
	if (!m_hEnemy->IsAlive())
	{
		if (!m_flLastSight)
		{
			m_flLastSight = gpGlobals->time + 0.5; // continue-shooting timeout
		}
		else
		{
			if (gpGlobals->time > m_flLastSight)
			{
				m_hEnemy = nullptr;
				m_flLastSight = gpGlobals->time + m_flMaxWait;
				SetThink(&CBaseTurret::SearchThink);
				return;
			}
		}
	}

	const Vector vecMid = pev->origin + pev->view_ofs;
	Vector vecMidEnemy = m_hEnemy->BodyTarget(vecMid);

	// Look for our current enemy
	bool fEnemyVisible = IsBoxVisible(this, m_hEnemy, vecMidEnemy);

	const Vector vecDirToEnemy = vecMidEnemy - vecMid;	// calculate dir and dist to enemy
	float flDistToEnemy = vecDirToEnemy.Length();

	Vector vec = VectorAngles(vecMidEnemy - vecMid);

	// Current enmey is not visible.
	if (!fEnemyVisible || (flDistToEnemy > TURRET_RANGE))
	{
		if (!m_flLastSight)
			m_flLastSight = gpGlobals->time + 0.5;
		else
		{
			// Should we look for a new target?
			if (gpGlobals->time > m_flLastSight)
			{
				m_hEnemy = nullptr;
				m_flLastSight = gpGlobals->time + m_flMaxWait;
				SetThink(&CBaseTurret::SearchThink);
				return;
			}
		}
		fEnemyVisible = false;
	}
	else
	{
		m_vecLastSight = vecMidEnemy;
	}

	UTIL_MakeAimVectors(m_vecCurAngles);

	/*
	ALERT( at_console, "%.0f %.0f : %.2f %.2f %.2f\n",
		m_vecCurAngles.x, m_vecCurAngles.y,
		gpGlobals->v_forward.x, gpGlobals->v_forward.y, gpGlobals->v_forward.z );
	*/

	const Vector vecLOS = vecDirToEnemy.Normalize(); //vecMid - m_vecLastSight;

	// Is the Gun looking at the target
	const bool fAttack = DotProduct(vecLOS, gpGlobals->v_forward) > 0.866; // 30 degree slop

	// fire the gun
	if (m_iSpin && ((fAttack) || (m_fBeserk)))
	{
		Vector vecSrc, vecAng;
		GetAttachment(0, vecSrc, vecAng);
		SetTurretAnim(TurretAnim::Fire);
		Shoot(vecSrc, gpGlobals->v_forward);
	}
	else
	{
		SetTurretAnim(TurretAnim::Spin);
	}

	//move the gun
	if (m_fBeserk)
	{
		if (RANDOM_LONG(0, 9) == 0)
		{
			m_vecGoalAngles.y = RANDOM_FLOAT(0, 360);
			m_vecGoalAngles.x = RANDOM_FLOAT(0, 90) - 90 * static_cast<int>(m_iOrientation);
			TakeDamage({this, this, 1, DMG_GENERIC}); // don't beserk forever
			return;
		}
	}
	else if (fEnemyVisible)
	{
		if (vec.y > 360)
			vec.y -= 360;

		if (vec.y < 0)
			vec.y += 360;

		//ALERT(at_console, "[%.2f]", vec.x);

		if (vec.x < -180)
			vec.x += 360;

		if (vec.x > 180)
			vec.x -= 360;

		// now all numbers should be in [1...360]
		// pin to turret limitations to [-90...15]

		if (m_iOrientation == TurretOrientation::Floor)
		{
			if (vec.x > 90)
				vec.x = 90;
			else if (vec.x < m_iMinPitch)
				vec.x = m_iMinPitch;
		}
		else
		{
			if (vec.x < -90)
				vec.x = -90;
			else if (vec.x > -m_iMinPitch)
				vec.x = -m_iMinPitch;
		}

		// ALERT(at_console, "->[%.2f]\n", vec.x);

		m_vecGoalAngles.y = vec.y;
		m_vecGoalAngles.x = vec.x;

	}

	SpinUpCall();
	MoveTurret();
}

void CTurret::Shoot(Vector& vecSrc, Vector& vecDirToEnemy)
{
	FireBullets(1, vecSrc, vecDirToEnemy, TURRET_SPREAD, TURRET_RANGE, BULLET_MONSTER_12MM, 1);
	EmitSound(SoundChannel::Weapon, "turret/tu_fire1.wav", VOL_NORM, 0.6);
	pev->effects = pev->effects | EF_MUZZLEFLASH;
}

void CMiniTurret::Shoot(Vector& vecSrc, Vector& vecDirToEnemy)
{
	FireBullets(1, vecSrc, vecDirToEnemy, TURRET_SPREAD, TURRET_RANGE, BULLET_MONSTER_9MM, 1);

	switch (RANDOM_LONG(0, 2))
	{
	case 0: EmitSound(SoundChannel::Weapon, "weapons/hks1.wav"); break;
	case 1: EmitSound(SoundChannel::Weapon, "weapons/hks2.wav"); break;
	case 2: EmitSound(SoundChannel::Weapon, "weapons/hks3.wav"); break;
	}
	pev->effects = pev->effects | EF_MUZZLEFLASH;
}

void CBaseTurret::Deploy()
{
	pev->nextthink = gpGlobals->time + 0.1;
	StudioFrameAdvance();

	if (pev->sequence != static_cast<int>(TurretAnim::Deploy))
	{
		m_iOn = true;
		SetTurretAnim(TurretAnim::Deploy);
		EmitSound(SoundChannel::Body, "turret/tu_deploy.wav", TURRET_MACHINE_VOLUME);
		SUB_UseTargets(this, UseType::On, 0);
	}

	if (m_fSequenceFinished)
	{
		pev->maxs.z = m_iDeployHeight;
		pev->mins.z = -m_iDeployHeight;
		SetSize(pev->mins, pev->maxs);

		m_vecCurAngles.x = 0;

		if (m_iOrientation == TurretOrientation::Ceiling)
		{
			m_vecCurAngles.y = UTIL_AngleMod(pev->angles.y + 180);
		}
		else
		{
			m_vecCurAngles.y = UTIL_AngleMod(pev->angles.y);
		}

		SetTurretAnim(TurretAnim::Spin);
		pev->framerate = 0;
		SetThink(&CBaseTurret::SearchThink);
	}

	m_flLastSight = gpGlobals->time + m_flMaxWait;
}

void CBaseTurret::Retire()
{
	// make the turret level
	m_vecGoalAngles.x = 0;
	m_vecGoalAngles.y = m_flStartYaw;

	pev->nextthink = gpGlobals->time + 0.1;

	StudioFrameAdvance();

	EyeOff();

	if (!MoveTurret())
	{
		if (m_iSpin)
		{
			SpinDownCall();
		}
		else if (pev->sequence != static_cast<int>(TurretAnim::Retire))
		{
			SetTurretAnim(TurretAnim::Retire);
			EmitSound(SoundChannel::Body, "turret/tu_deploy.wav", TURRET_MACHINE_VOLUME, ATTN_NORM, 120);
			SUB_UseTargets(this, UseType::Off, 0);
		}
		else if (m_fSequenceFinished)
		{
			m_iOn = false;
			m_flLastSight = 0;
			SetTurretAnim(TurretAnim::None);
			pev->maxs.z = m_iRetractHeight;
			pev->mins.z = -m_iRetractHeight;
			SetSize(pev->mins, pev->maxs);
			if (m_iAutoStart)
			{
				SetThink(&CBaseTurret::AutoSearchThink);
				pev->nextthink = gpGlobals->time + .1;
			}
			else
				SetThink(&CBaseTurret::SUB_DoNothing);
		}
	}
	else
	{
		SetTurretAnim(TurretAnim::Spin);
	}
}

void CTurret::SpinUpCall()
{
	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1;

	// Are we already spun up? If not start the two stage process.
	if (!m_iSpin)
	{
		SetTurretAnim(TurretAnim::Spin);
		// for the first pass, spin up the the barrel
		if (!m_iStartSpin)
		{
			pev->nextthink = gpGlobals->time + 1.0; // spinup delay
			EmitSound(SoundChannel::Body, "turret/tu_spinup.wav", TURRET_MACHINE_VOLUME);
			m_iStartSpin = true;
			pev->framerate = 0.1;
		}
		// after the barrel is spun up, turn on the hum
		else if (pev->framerate >= 1.0)
		{
			pev->nextthink = gpGlobals->time + 0.1; // retarget delay
			EmitSound(SoundChannel::Static, "turret/tu_active2.wav", TURRET_MACHINE_VOLUME);
			SetThink(&CTurret::ActiveThink);
			m_iStartSpin = false;
			m_iSpin = true;
		}
		else
		{
			pev->framerate += 0.075;
		}
	}

	if (m_iSpin)
	{
		SetThink(&CTurret::ActiveThink);
	}
}

void CTurret::SpinDownCall()
{
	if (m_iSpin)
	{
		SetTurretAnim(TurretAnim::Spin);
		if (pev->framerate == 1.0)
		{
			StopSound(SoundChannel::Static, "turret/tu_active2.wav");
			EmitSound(SoundChannel::Item, "turret/tu_spindown.wav", TURRET_MACHINE_VOLUME);
		}
		pev->framerate -= 0.02;
		if (pev->framerate <= 0)
		{
			pev->framerate = 0;
			m_iSpin = false;
		}
	}
}

void CBaseTurret::SetTurretAnim(TurretAnim anim)
{
	const TurretAnim currentSequence = static_cast<TurretAnim>(pev->sequence);

	if (currentSequence != anim)
	{
		switch (anim)
		{
		case TurretAnim::Fire:
		case TurretAnim::Spin:
			if (currentSequence != TurretAnim::Fire && currentSequence != TurretAnim::Spin)
			{
				pev->frame = 0;
			}
			break;
		default:
			pev->frame = 0;
			break;
		}

		pev->sequence = static_cast<int>(anim);
		ResetSequenceInfo();

		switch (anim)
		{
		case TurretAnim::Retire:
			pev->frame = 255;
			pev->framerate = -1.0;
			break;
		case TurretAnim::Die:
			pev->framerate = 1.0;
			break;
		}
		//ALERT(at_console, "Turret anim #%d\n", anim);
	}
}

void CBaseTurret::SearchThink()
{
	// ensure rethink
	SetTurretAnim(TurretAnim::Spin);
	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1;

	if (m_flSpinUpTime == 0 && m_flMaxSpin)
		m_flSpinUpTime = gpGlobals->time + m_flMaxSpin;

	Ping();

	// If we have a target and we're still healthy
	if (m_hEnemy != nullptr)
	{
		if (!m_hEnemy->IsAlive())
			m_hEnemy = nullptr;// Dead enemy forces a search for new one
	}


	// Acquire Target
	if (m_hEnemy == nullptr)
	{
		Look(TURRET_RANGE);
		m_hEnemy = BestVisibleEnemy();
	}

	// If we've found a target, spin up the barrel and start to attack
	if (m_hEnemy != nullptr)
	{
		m_flLastSight = 0;
		m_flSpinUpTime = 0;
		SetThink(&CBaseTurret::ActiveThink);
	}
	else
	{
		// Are we out of time, do we need to retract?
		if (gpGlobals->time > m_flLastSight)
		{
			//Before we retrace, make sure that we are spun down.
			m_flLastSight = 0;
			m_flSpinUpTime = 0;
			SetThink(&CBaseTurret::Retire);
		}
		// should we stop the spin?
		else if ((m_flSpinUpTime) && (gpGlobals->time > m_flSpinUpTime))
		{
			SpinDownCall();
		}

		// generic hunt for new victims
		m_vecGoalAngles.y = (m_vecGoalAngles.y + 0.1 * m_fTurnRate);
		if (m_vecGoalAngles.y >= 360)
			m_vecGoalAngles.y -= 360;
		MoveTurret();
	}
}

void CBaseTurret::AutoSearchThink()
{
	// ensure rethink
	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.3;

	// If we have a target and we're still healthy

	if (m_hEnemy != nullptr)
	{
		if (!m_hEnemy->IsAlive())
			m_hEnemy = nullptr;// Dead enemy forces a search for new one
	}

	// Acquire Target

	if (m_hEnemy == nullptr)
	{
		Look(TURRET_RANGE);
		m_hEnemy = BestVisibleEnemy();
	}

	if (m_hEnemy != nullptr)
	{
		SetThink(&CBaseTurret::Deploy);
		EmitSound(SoundChannel::Body, "turret/tu_alert.wav", TURRET_MACHINE_VOLUME);
	}
}

void CBaseTurret::TurretDeath()
{
	bool iActive = false;

	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1;

	if (pev->deadflag != DeadFlag::Dead)
	{
		pev->deadflag = DeadFlag::Dead;

		const float flRndSound = RANDOM_FLOAT(0, 1);

		if (flRndSound <= 0.33)
			EmitSound(SoundChannel::Body, "turret/tu_die.wav");
		else if (flRndSound <= 0.66)
			EmitSound(SoundChannel::Body, "turret/tu_die2.wav");
		else
			EmitSound(SoundChannel::Body, "turret/tu_die3.wav");

		StopSound(SoundChannel::Static, "turret/tu_active2.wav");

		if (m_iOrientation == TurretOrientation::Floor)
			m_vecGoalAngles.x = -15;
		else
			m_vecGoalAngles.x = -90;

		SetTurretAnim(TurretAnim::Die);

		EyeOn();
	}

	EyeOff();

	if (pev->dmgtime + RANDOM_FLOAT(0, 2) > gpGlobals->time)
	{
		// lots of smoke
		MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
		WRITE_BYTE(TE_SMOKE);
		WRITE_COORD(RANDOM_FLOAT(pev->absmin.x, pev->absmax.x));
		WRITE_COORD(RANDOM_FLOAT(pev->absmin.y, pev->absmax.y));
		WRITE_COORD(pev->origin.z - static_cast<int>(m_iOrientation) * 64);
		WRITE_SHORT(g_sModelIndexSmoke);
		WRITE_BYTE(25); // scale * 10
		WRITE_BYTE(10 - static_cast<int>(m_iOrientation) * 5); // framerate
		MESSAGE_END();
	}

	if (pev->dmgtime + RANDOM_FLOAT(0, 5) > gpGlobals->time)
	{
		Vector vecSrc = Vector(RANDOM_FLOAT(pev->absmin.x, pev->absmax.x), RANDOM_FLOAT(pev->absmin.y, pev->absmax.y), 0);
		if (m_iOrientation == TurretOrientation::Floor)
			vecSrc = vecSrc + Vector(0, 0, RANDOM_FLOAT(pev->origin.z, pev->absmax.z));
		else
			vecSrc = vecSrc + Vector(0, 0, RANDOM_FLOAT(pev->absmin.z, pev->origin.z));

		UTIL_Sparks(vecSrc);
	}

	if (m_fSequenceFinished && !MoveTurret() && pev->dmgtime + 5 < gpGlobals->time)
	{
		pev->framerate = 0;
		SetThink(nullptr);
	}
}

void CBaseTurret::TraceAttack(const TraceAttackInfo& info)
{
	TraceAttackInfo adjustedInfo = info;

	if (adjustedInfo.GetTraceResult().iHitgroup == 10)
	{
		// hit armor
		if (pev->dmgtime != gpGlobals->time || (RANDOM_LONG(0, 10) < 1))
		{
			UTIL_Ricochet(adjustedInfo.GetTraceResult().vecEndPos, RANDOM_FLOAT(1, 2));
			pev->dmgtime = gpGlobals->time;
		}

		adjustedInfo.SetDamage(0.1f);// don't hurt the monster much, but allow bits_COND_LIGHT_DAMAGE to be generated
	}

	if (!pev->takedamage)
		return;

	AddMultiDamage(adjustedInfo.GetAttacker(), this, adjustedInfo.GetDamage(), adjustedInfo.GetDamageTypes());
}

bool CBaseTurret::TakeDamage(const TakeDamageInfo& info)
{
	if (!pev->takedamage)
		return false;

	TakeDamageInfo adjustedInfo = info;

	if (!m_iOn)
		adjustedInfo.SetDamage(adjustedInfo.GetDamage() / 10.0f);

	pev->health -= adjustedInfo.GetDamage();
	if (pev->health <= 0)
	{
		pev->health = 0;
		SetDamageMode(DamageMode::No);
		pev->dmgtime = gpGlobals->time;

		ClearBits(pev->flags, FL_MONSTER); // why are they set in the first place???

		SetUse(nullptr);
		SetThink(&CBaseTurret::TurretDeath);
		SUB_UseTargets(this, UseType::On, 0); // wake up others
		pev->nextthink = gpGlobals->time + 0.1;

		return false;
	}

	if (pev->health <= 10)
	{
		if (m_iOn && (true || RANDOM_LONG(0, 0x7FFF) > 800))
		{
			m_fBeserk = true;
			SetThink(&CBaseTurret::SearchThink);
		}
	}

	return true;
}

bool CBaseTurret::MoveTurret()
{
	bool state = false;
	// any x movement?

	if (m_vecCurAngles.x != m_vecGoalAngles.x)
	{
		const float flDir = m_vecGoalAngles.x > m_vecCurAngles.x ? 1 : -1;

		m_vecCurAngles.x += 0.1 * m_fTurnRate * flDir;

		// if we started below the goal, and now we're past, peg to goal
		if (flDir == 1)
		{
			if (m_vecCurAngles.x > m_vecGoalAngles.x)
				m_vecCurAngles.x = m_vecGoalAngles.x;
		}
		else
		{
			if (m_vecCurAngles.x < m_vecGoalAngles.x)
				m_vecCurAngles.x = m_vecGoalAngles.x;
		}

		if (m_iOrientation == TurretOrientation::Floor)
			SetBoneController(1, -m_vecCurAngles.x);
		else
			SetBoneController(1, m_vecCurAngles.x);
		state = true;
	}

	if (m_vecCurAngles.y != m_vecGoalAngles.y)
	{
		float flDir = m_vecGoalAngles.y > m_vecCurAngles.y ? 1 : -1;
		float flDist = fabs(m_vecGoalAngles.y - m_vecCurAngles.y);

		if (flDist > 180)
		{
			flDist = 360 - flDist;
			flDir = -flDir;
		}
		if (flDist > 30)
		{
			if (m_fTurnRate < m_iBaseTurnRate * 10)
			{
				m_fTurnRate += m_iBaseTurnRate;
			}
		}
		else if (m_fTurnRate > 45)
		{
			m_fTurnRate -= m_iBaseTurnRate;
		}
		else
		{
			m_fTurnRate += m_iBaseTurnRate;
		}

		m_vecCurAngles.y += 0.1 * m_fTurnRate * flDir;

		if (m_vecCurAngles.y < 0)
			m_vecCurAngles.y += 360;
		else if (m_vecCurAngles.y >= 360)
			m_vecCurAngles.y -= 360;

		if (flDist < (0.05 * m_iBaseTurnRate))
			m_vecCurAngles.y = m_vecGoalAngles.y;

		//ALERT(at_console, "%.2f -> %.2f\n", m_vecCurAngles.y, y);
		if (m_iOrientation == TurretOrientation::Floor)
			SetBoneController(0, m_vecCurAngles.y - pev->angles.y);
		else
			SetBoneController(0, pev->angles.y - 180 - m_vecCurAngles.y);
		state = true;
	}

	if (!state)
		m_fTurnRate = m_iBaseTurnRate;

	//ALERT(at_console, "(%.2f, %.2f)->(%.2f, %.2f)\n", m_vecCurAngles.x, 
	//	m_vecCurAngles.y, m_vecGoalAngles.x, m_vecGoalAngles.y);
	return state;
}

int	CBaseTurret::Classify()
{
	if (m_iOn || m_iAutoStart)
		return	CLASS_MACHINE;
	return CLASS_NONE;
}

/**
*	@brief smallest turret, placed near grunt entrenchments
*/
class CSentry : public CBaseTurret
{
public:
	void Spawn() override;
	void Precache() override;
	// other functions
	void Shoot(Vector& vecSrc, Vector& vecDirToEnemy) override;
	bool TakeDamage(const TakeDamageInfo& info) override;
	void EXPORT SentryTouch(CBaseEntity* pOther);
	void EXPORT SentryDeath();
};

LINK_ENTITY_TO_CLASS(monster_sentry, CSentry);

void CSentry::Precache()
{
	CBaseTurret::Precache();
	PRECACHE_MODEL("models/sentry.mdl");
}

void CSentry::Spawn()
{
	Precache();
	SetModel("models/sentry.mdl");
	pev->health = gSkillData.sentryHealth;
	m_HackedGunPos = Vector(0, 0, 48);
	pev->view_ofs.z = 48;
	m_flMaxWait = 1E6;
	m_flMaxSpin = 1E6;

	CBaseTurret::Spawn();
	m_iRetractHeight = 64;
	m_iDeployHeight = 64;
	m_iMinPitch = -60;
	SetSize(Vector(-16, -16, -m_iRetractHeight), Vector(16, 16, m_iRetractHeight));

	SetTouch(&CSentry::SentryTouch);
	SetThink(&CSentry::Initialize);
	pev->nextthink = gpGlobals->time + 0.3;
}

void CSentry::Shoot(Vector& vecSrc, Vector& vecDirToEnemy)
{
	FireBullets(1, vecSrc, vecDirToEnemy, TURRET_SPREAD, TURRET_RANGE, BULLET_MONSTER_MP5, 1);

	switch (RANDOM_LONG(0, 2))
	{
	case 0: EmitSound(SoundChannel::Weapon, "weapons/hks1.wav"); break;
	case 1: EmitSound(SoundChannel::Weapon, "weapons/hks2.wav"); break;
	case 2: EmitSound(SoundChannel::Weapon, "weapons/hks3.wav"); break;
	}
	pev->effects = pev->effects | EF_MUZZLEFLASH;
}

bool CSentry::TakeDamage(const TakeDamageInfo& info)
{
	if (!pev->takedamage)
		return false;

	if (!m_iOn)
	{
		SetThink(&CSentry::Deploy);
		SetUse(nullptr);
		pev->nextthink = gpGlobals->time + 0.1;
	}

	pev->health -= info.GetDamage();
	if (pev->health <= 0)
	{
		pev->health = 0;
		SetDamageMode(DamageMode::No);
		pev->dmgtime = gpGlobals->time;

		ClearBits(pev->flags, FL_MONSTER); // why are they set in the first place???

		SetUse(nullptr);
		SetThink(&CSentry::SentryDeath);
		SUB_UseTargets(this, UseType::On, 0); // wake up others
		pev->nextthink = gpGlobals->time + 0.1;

		return false;
	}

	return true;
}

void CSentry::SentryTouch(CBaseEntity* pOther)
{
	if (pOther && (pOther->IsPlayer() || (pOther->pev->flags & FL_MONSTER)))
	{
		TakeDamage({pOther, pOther, 0, 0});
	}
}

void CSentry::SentryDeath()
{
	bool iActive = false;

	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1;

	if (pev->deadflag != DeadFlag::Dead)
	{
		pev->deadflag = DeadFlag::Dead;

		const float flRndSound = RANDOM_FLOAT(0, 1);

		if (flRndSound <= 0.33)
			EmitSound(SoundChannel::Body, "turret/tu_die.wav");
		else if (flRndSound <= 0.66)
			EmitSound(SoundChannel::Body, "turret/tu_die2.wav");
		else
			EmitSound(SoundChannel::Body, "turret/tu_die3.wav");

		StopSound(SoundChannel::Static, "turret/tu_active2.wav");

		SetBoneController(0, 0);
		SetBoneController(1, 0);

		SetTurretAnim(TurretAnim::Die);

		SetSolidType(Solid::Not);
		pev->angles.y = UTIL_AngleMod(pev->angles.y + RANDOM_LONG(0, 2) * 120);

		EyeOn();
	}

	EyeOff();

	Vector vecSrc, vecAng;
	GetAttachment(1, vecSrc, vecAng);

	if (pev->dmgtime + RANDOM_FLOAT(0, 2) > gpGlobals->time)
	{
		// lots of smoke
		MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
		WRITE_BYTE(TE_SMOKE);
		WRITE_COORD(vecSrc.x + RANDOM_FLOAT(-16, 16));
		WRITE_COORD(vecSrc.y + RANDOM_FLOAT(-16, 16));
		WRITE_COORD(vecSrc.z - 32);
		WRITE_SHORT(g_sModelIndexSmoke);
		WRITE_BYTE(15); // scale * 10
		WRITE_BYTE(8); // framerate
		MESSAGE_END();
	}

	if (pev->dmgtime + RANDOM_FLOAT(0, 8) > gpGlobals->time)
	{
		UTIL_Sparks(vecSrc);
	}

	if (m_fSequenceFinished && pev->dmgtime + 5 < gpGlobals->time)
	{
		pev->framerate = 0;
		SetThink(nullptr);
	}
}
