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

constexpr int ACT_T_IDLE = 1010;
constexpr int ACT_T_TAP = 1020;
constexpr int ACT_T_STRIKE = 1030;
constexpr int ACT_T_REARIDLE = 1040;

/**
*	@brief silo of death tentacle monster (half life)
*/
class CTentacle : public CBaseMonster
{
public:
	CTentacle();

	void Spawn() override;
	void Precache() override;
	void KeyValue(KeyValueData* pkvd) override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	// Don't allow the tentacle to go across transitions!!!
	int	ObjectCaps() override { return CBaseMonster::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	void SetObjectCollisionBox() override
	{
		pev->absmin = GetAbsOrigin() + Vector(-400, -400, 0);
		pev->absmax = GetAbsOrigin() + Vector(400, 400, 850);
	}

	void EXPORT Cycle();
	void EXPORT CommandUse(const UseInfo& info);
	void EXPORT Start();
	void EXPORT DieThink();

	void EXPORT Test();

	void EXPORT HitTouch(CBaseEntity* pOther);

	float HearingSensitivity() override { return 2.0; }

	bool TakeDamage(const TakeDamageInfo& info) override;
	void HandleAnimEvent(AnimationEvent& event) override;
	void Killed(const KilledInfo& info) override;

	NPCState GetIdealState() override { return NPCState::Idle; }
	//TODO: should override base, but has different signature
	bool CanPlaySequence(bool fDisregardState) { return true; }

	int Classify() override;

	int Level(float dz);
	int MyLevel();
	float MyHeight();

	float m_flInitialYaw = 0;
	int m_iGoalAnim = 0;
	int m_iLevel = 0;
	int m_iDir = 0;
	float m_flFramerateAdj = 0;
	float m_flSoundYaw = 0;
	int m_iSoundLevel = 0;
	float m_flSoundTime = 0;
	float m_flSoundRadius = 0;
	int m_iHitDmg = 0;
	float m_flHitTime = 0;

	float m_flTapRadius = 0;

	float m_flNextSong = 0;

	static inline bool g_fFlySound = false;
	static inline bool g_fSquirmSound = false;

	float m_flMaxYaw = 0;
	int m_iTapSound = 0;

	Vector m_vecPrevSound;
	float m_flPrevSoundTime = 0;

	static const char* pHitSilo[];
	static const char* pHitDirt[];
	static const char* pHitWater[];
};

LINK_ENTITY_TO_CLASS(monster_tentacle, CTentacle);

// stike sounds
constexpr int TE_NONE = -1;
constexpr int TE_SILO = 0;
constexpr int TE_DIRT = 1;
constexpr int TE_WATER = 2;

const char* CTentacle::pHitSilo[] =
{
	"tentacle/te_strike1.wav",
	"tentacle/te_strike2.wav",
};

const char* CTentacle::pHitDirt[] =
{
	"player/pl_dirt1.wav",
	"player/pl_dirt2.wav",
	"player/pl_dirt3.wav",
	"player/pl_dirt4.wav",
};

const char* CTentacle::pHitWater[] =
{
	"player/pl_slosh1.wav",
	"player/pl_slosh2.wav",
	"player/pl_slosh3.wav",
	"player/pl_slosh4.wav",
};

TYPEDESCRIPTION	CTentacle::m_SaveData[] =
{
	DEFINE_FIELD(CTentacle, m_flInitialYaw, FIELD_FLOAT),
	DEFINE_FIELD(CTentacle, m_iGoalAnim, FIELD_INTEGER),
	DEFINE_FIELD(CTentacle, m_iLevel, FIELD_INTEGER),
	DEFINE_FIELD(CTentacle, m_iDir, FIELD_INTEGER),
	DEFINE_FIELD(CTentacle, m_flFramerateAdj, FIELD_FLOAT),
	DEFINE_FIELD(CTentacle, m_flSoundYaw, FIELD_FLOAT),
	DEFINE_FIELD(CTentacle, m_iSoundLevel, FIELD_INTEGER),
	DEFINE_FIELD(CTentacle, m_flSoundTime, FIELD_TIME),
	DEFINE_FIELD(CTentacle, m_flSoundRadius, FIELD_FLOAT),
	DEFINE_FIELD(CTentacle, m_iHitDmg, FIELD_INTEGER),
	DEFINE_FIELD(CTentacle, m_flHitTime, FIELD_TIME),
	DEFINE_FIELD(CTentacle, m_flTapRadius, FIELD_FLOAT),
	DEFINE_FIELD(CTentacle, m_flNextSong, FIELD_TIME),
	DEFINE_FIELD(CTentacle, m_iTapSound, FIELD_INTEGER),
	DEFINE_FIELD(CTentacle, m_flMaxYaw, FIELD_FLOAT),
	DEFINE_FIELD(CTentacle, m_vecPrevSound, FIELD_POSITION_VECTOR),
	DEFINE_FIELD(CTentacle, m_flPrevSoundTime, FIELD_TIME),
};

IMPLEMENT_SAVERESTORE(CTentacle, CBaseMonster);

// animation sequence aliases 
enum TENTACLE_ANIM
{
	TENTACLE_ANIM_Pit_Idle,

	TENTACLE_ANIM_rise_to_Temp1,
	TENTACLE_ANIM_Temp1_to_Floor,
	TENTACLE_ANIM_Floor_Idle,
	TENTACLE_ANIM_Floor_Fidget_Pissed,
	TENTACLE_ANIM_Floor_Fidget_SmallRise,
	TENTACLE_ANIM_Floor_Fidget_Wave,
	TENTACLE_ANIM_Floor_Strike,
	TENTACLE_ANIM_Floor_Tap,
	TENTACLE_ANIM_Floor_Rotate,
	TENTACLE_ANIM_Floor_Rear,
	TENTACLE_ANIM_Floor_Rear_Idle,
	TENTACLE_ANIM_Floor_to_Lev1,

	TENTACLE_ANIM_Lev1_Idle,
	TENTACLE_ANIM_Lev1_Fidget_Claw,
	TENTACLE_ANIM_Lev1_Fidget_Shake,
	TENTACLE_ANIM_Lev1_Fidget_Snap,
	TENTACLE_ANIM_Lev1_Strike,
	TENTACLE_ANIM_Lev1_Tap,
	TENTACLE_ANIM_Lev1_Rotate,
	TENTACLE_ANIM_Lev1_Rear,
	TENTACLE_ANIM_Lev1_Rear_Idle,
	TENTACLE_ANIM_Lev1_to_Lev2,

	TENTACLE_ANIM_Lev2_Idle,
	TENTACLE_ANIM_Lev2_Fidget_Shake,
	TENTACLE_ANIM_Lev2_Fidget_Swing,
	TENTACLE_ANIM_Lev2_Fidget_Tut,
	TENTACLE_ANIM_Lev2_Strike,
	TENTACLE_ANIM_Lev2_Tap,
	TENTACLE_ANIM_Lev2_Rotate,
	TENTACLE_ANIM_Lev2_Rear,
	TENTACLE_ANIM_Lev2_Rear_Idle,
	TENTACLE_ANIM_Lev2_to_Lev3,

	TENTACLE_ANIM_Lev3_Idle,
	TENTACLE_ANIM_Lev3_Fidget_Shake,
	TENTACLE_ANIM_Lev3_Fidget_Side,
	TENTACLE_ANIM_Lev3_Fidget_Swipe,
	TENTACLE_ANIM_Lev3_Strike,
	TENTACLE_ANIM_Lev3_Tap,
	TENTACLE_ANIM_Lev3_Rotate,
	TENTACLE_ANIM_Lev3_Rear,
	TENTACLE_ANIM_Lev3_Rear_Idle,

	TENTACLE_ANIM_Lev1_Door_reach,

	TENTACLE_ANIM_Lev3_to_Engine,
	TENTACLE_ANIM_Engine_Idle,
	TENTACLE_ANIM_Engine_Sway,
	TENTACLE_ANIM_Engine_Swat,
	TENTACLE_ANIM_Engine_Bob,
	TENTACLE_ANIM_Engine_Death1,
	TENTACLE_ANIM_Engine_Death2,
	TENTACLE_ANIM_Engine_Death3,

	TENTACLE_ANIM_none
};

int	CTentacle::Classify()
{
	return	CLASS_ALIEN_MONSTER;
}

void CTentacle::Spawn()
{
	Precache();

	SetSolidType(Solid::BBox);
	SetMovetype(Movetype::Fly);
	pev->effects = 0;
	pev->health = 75;
	pev->sequence = 0;

	SetModel("models/tentacle2.mdl");
	SetSize(Vector(-32, -32, 0), Vector(32, 32, 64));

	SetDamageMode(DamageMode::Aim);
	pev->flags |= FL_MONSTER;

	m_bloodColor = BLOOD_COLOR_GREEN;

	SetThink(&CTentacle::Start);
	SetTouch(&CTentacle::HitTouch);
	SetUse(&CTentacle::CommandUse);

	pev->nextthink = gpGlobals->time + 0.2;

	ResetSequenceInfo();
	m_iDir = 1;

	pev->yaw_speed = 18;
	m_flInitialYaw = GetAbsAngles().y;
	pev->ideal_yaw = m_flInitialYaw;

	g_fFlySound = false;
	g_fSquirmSound = false;

	m_iHitDmg = 20;

	if (m_flMaxYaw <= 0)
		m_flMaxYaw = 65;

	m_MonsterState = NPCState::Idle;

	// SetThink( Test );
	SetAbsOrigin(GetAbsOrigin());
}

void CTentacle::Precache()
{
	PRECACHE_MODEL("models/tentacle2.mdl");

	PRECACHE_SOUND("ambience/flies.wav");
	PRECACHE_SOUND("ambience/squirm2.wav");

	PRECACHE_SOUND("tentacle/te_alert1.wav");
	PRECACHE_SOUND("tentacle/te_alert2.wav");
	PRECACHE_SOUND("tentacle/te_flies1.wav");
	PRECACHE_SOUND("tentacle/te_move1.wav");
	PRECACHE_SOUND("tentacle/te_move2.wav");
	PRECACHE_SOUND("tentacle/te_roar1.wav");
	PRECACHE_SOUND("tentacle/te_roar2.wav");
	PRECACHE_SOUND("tentacle/te_search1.wav");
	PRECACHE_SOUND("tentacle/te_search2.wav");
	PRECACHE_SOUND("tentacle/te_sing1.wav");
	PRECACHE_SOUND("tentacle/te_sing2.wav");
	PRECACHE_SOUND("tentacle/te_squirm2.wav");
	PRECACHE_SOUND("tentacle/te_strike1.wav");
	PRECACHE_SOUND("tentacle/te_strike2.wav");
	PRECACHE_SOUND("tentacle/te_swing1.wav");
	PRECACHE_SOUND("tentacle/te_swing2.wav");

	PRECACHE_SOUND_ARRAY(pHitSilo);
	PRECACHE_SOUND_ARRAY(pHitDirt);
	PRECACHE_SOUND_ARRAY(pHitWater);
}

CTentacle::CTentacle()
{
	m_flMaxYaw = 65;
	m_iTapSound = 0;
}

void CTentacle::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "sweeparc"))
	{
		m_flMaxYaw = atof(pkvd->szValue) / 2.0;
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "sound"))
	{
		m_iTapSound = atoi(pkvd->szValue);
		pkvd->fHandled = true;

	}
	else
		CBaseMonster::KeyValue(pkvd);
}

int CTentacle::Level(float dz)
{
	if (dz < 216)
		return 0;
	if (dz < 408)
		return 1;
	if (dz < 600)
		return 2;
	return 3;
}

float CTentacle::MyHeight()
{
	switch (MyLevel())
	{
	case 1:
		return 256;
	case 2:
		return 448;
	case 3:
		return 640;
	}
	return 0;
}

int CTentacle::MyLevel()
{
	switch (pev->sequence)
	{
	case TENTACLE_ANIM_Pit_Idle:
		return -1;

	case TENTACLE_ANIM_rise_to_Temp1:
	case TENTACLE_ANIM_Temp1_to_Floor:
	case TENTACLE_ANIM_Floor_to_Lev1:
		return 0;

	case TENTACLE_ANIM_Floor_Idle:
	case TENTACLE_ANIM_Floor_Fidget_Pissed:
	case TENTACLE_ANIM_Floor_Fidget_SmallRise:
	case TENTACLE_ANIM_Floor_Fidget_Wave:
	case TENTACLE_ANIM_Floor_Strike:
	case TENTACLE_ANIM_Floor_Tap:
	case TENTACLE_ANIM_Floor_Rotate:
	case TENTACLE_ANIM_Floor_Rear:
	case TENTACLE_ANIM_Floor_Rear_Idle:
		return 0;

	case TENTACLE_ANIM_Lev1_Idle:
	case TENTACLE_ANIM_Lev1_Fidget_Claw:
	case TENTACLE_ANIM_Lev1_Fidget_Shake:
	case TENTACLE_ANIM_Lev1_Fidget_Snap:
	case TENTACLE_ANIM_Lev1_Strike:
	case TENTACLE_ANIM_Lev1_Tap:
	case TENTACLE_ANIM_Lev1_Rotate:
	case TENTACLE_ANIM_Lev1_Rear:
	case TENTACLE_ANIM_Lev1_Rear_Idle:
		return 1;

	case TENTACLE_ANIM_Lev1_to_Lev2:
		return 1;

	case TENTACLE_ANIM_Lev2_Idle:
	case TENTACLE_ANIM_Lev2_Fidget_Shake:
	case TENTACLE_ANIM_Lev2_Fidget_Swing:
	case TENTACLE_ANIM_Lev2_Fidget_Tut:
	case TENTACLE_ANIM_Lev2_Strike:
	case TENTACLE_ANIM_Lev2_Tap:
	case TENTACLE_ANIM_Lev2_Rotate:
	case TENTACLE_ANIM_Lev2_Rear:
	case TENTACLE_ANIM_Lev2_Rear_Idle:
		return 2;

	case TENTACLE_ANIM_Lev2_to_Lev3:
		return 2;

	case TENTACLE_ANIM_Lev3_Idle:
	case TENTACLE_ANIM_Lev3_Fidget_Shake:
	case TENTACLE_ANIM_Lev3_Fidget_Side:
	case TENTACLE_ANIM_Lev3_Fidget_Swipe:
	case TENTACLE_ANIM_Lev3_Strike:
	case TENTACLE_ANIM_Lev3_Tap:
	case TENTACLE_ANIM_Lev3_Rotate:
	case TENTACLE_ANIM_Lev3_Rear:
	case TENTACLE_ANIM_Lev3_Rear_Idle:
		return 3;

	case TENTACLE_ANIM_Lev1_Door_reach:
		return -1;
	}
	return -1;
}

void CTentacle::Test()
{
	pev->sequence = TENTACLE_ANIM_Floor_Strike;
	pev->framerate = 0;
	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1;
}

void CTentacle::Cycle()
{
	// ALERT( at_console, "%s %.2f %d %d\n", STRING( pev->targetname ), GetAbsOrigin().z, m_MonsterState, m_IdealMonsterState );
	pev->nextthink = gpGlobals->time + 0.1;

	// ALERT( at_console, "%s %d %d %d %f %f\n", STRING( pev->targetname ), pev->sequence, m_iGoalAnim, m_iDir, pev->framerate, pev->health );

	if (m_MonsterState == NPCState::Script || m_IdealMonsterState == NPCState::Script)
	{
		SetAbsAngles({GetAbsAngles().x, m_flInitialYaw, GetAbsAngles().z});
		pev->ideal_yaw = m_flInitialYaw;
		ClearConditions(IgnoreConditions());
		MonsterThink();
		m_iGoalAnim = TENTACLE_ANIM_Pit_Idle;
		return;
	}

	DispatchAnimEvents();
	StudioFrameAdvance();

	ChangeYaw(pev->yaw_speed);

	Listen();

	// Listen will set this if there's something in my sound list
	if (CSound* pSound = HasConditions(bits_COND_HEAR_SOUND) ? BestSound() : nullptr; pSound)
	{
		Vector vecDir;
		if (gpGlobals->time - m_flPrevSoundTime < 0.5f)
		{
			const float dt = gpGlobals->time - m_flPrevSoundTime;
			vecDir = pSound->m_vecOrigin + (pSound->m_vecOrigin - m_vecPrevSound) / dt - GetAbsOrigin();
		}
		else
		{
			vecDir = pSound->m_vecOrigin - GetAbsOrigin();
		}
		m_flPrevSoundTime = gpGlobals->time;
		m_vecPrevSound = pSound->m_vecOrigin;

		m_flSoundYaw = UTIL_VecToYaw(vecDir) - m_flInitialYaw;
		m_iSoundLevel = Level(vecDir.z);

		if (m_flSoundYaw < -180)
			m_flSoundYaw += 360;
		if (m_flSoundYaw > 180)
			m_flSoundYaw -= 360;

		// ALERT( at_console, "sound %d %.0f\n", m_iSoundLevel, m_flSoundYaw );
		if (m_flSoundTime < gpGlobals->time)
		{
			// play "I hear new something" sound
			const char* sound;

			switch (RANDOM_LONG(0, 1))
			{
			case 0: sound = "tentacle/te_alert1.wav"; break;
			case 1: sound = "tentacle/te_alert2.wav"; break;
			}

			// UTIL_EmitAmbientSound(this, GetAbsOrigin() + Vector( 0, 0, MyHeight()), sound, 1.0, ATTN_NORM, 0, 100);
		}
		m_flSoundTime = gpGlobals->time + RANDOM_FLOAT(5.0, 10.0);
	}

	// clip ideal_yaw
	float dy = m_flSoundYaw;
	switch (pev->sequence)
	{
	case TENTACLE_ANIM_Floor_Rear:
	case TENTACLE_ANIM_Floor_Rear_Idle:
	case TENTACLE_ANIM_Lev1_Rear:
	case TENTACLE_ANIM_Lev1_Rear_Idle:
	case TENTACLE_ANIM_Lev2_Rear:
	case TENTACLE_ANIM_Lev2_Rear_Idle:
	case TENTACLE_ANIM_Lev3_Rear:
	case TENTACLE_ANIM_Lev3_Rear_Idle:
		if (dy < 0 && dy > -m_flMaxYaw)
			dy = -m_flMaxYaw;
		if (dy > 0 && dy < m_flMaxYaw)
			dy = m_flMaxYaw;
		break;
	default:
		if (dy < -m_flMaxYaw)
			dy = -m_flMaxYaw;
		if (dy > m_flMaxYaw)
			dy = m_flMaxYaw;
	}
	pev->ideal_yaw = m_flInitialYaw + dy;

	if (m_fSequenceFinished)
	{
		// ALERT( at_console, "%s done %d %d\n", STRING( pev->targetname ), pev->sequence, m_iGoalAnim );
		if (pev->health <= 1)
		{
			m_iGoalAnim = TENTACLE_ANIM_Pit_Idle;
			if (pev->sequence == TENTACLE_ANIM_Pit_Idle)
			{
				pev->health = 75;
			}
		}
		else if (m_flSoundTime > gpGlobals->time)
		{
			if (m_flSoundYaw >= -(m_flMaxYaw + 30) && m_flSoundYaw <= (m_flMaxYaw + 30))
			{
				// strike
				m_iGoalAnim = LookupActivity(ACT_T_STRIKE + m_iSoundLevel);
			}
			else if (m_flSoundYaw >= -m_flMaxYaw * 2 && m_flSoundYaw <= m_flMaxYaw * 2)
			{
				// tap
				m_iGoalAnim = LookupActivity(ACT_T_TAP + m_iSoundLevel);
			}
			else
			{
				// go into rear idle
				m_iGoalAnim = LookupActivity(ACT_T_REARIDLE + m_iSoundLevel);
			}
		}
		else if (pev->sequence == TENTACLE_ANIM_Pit_Idle)
		{
			// stay in pit until hear noise
			m_iGoalAnim = TENTACLE_ANIM_Pit_Idle;
		}
		else if (pev->sequence == m_iGoalAnim)
		{
			if (MyLevel() >= 0 && gpGlobals->time < m_flSoundTime)
			{
				if (RANDOM_LONG(0, 9) < m_flSoundTime - gpGlobals->time)
				{
					// continue stike
					m_iGoalAnim = LookupActivity(ACT_T_STRIKE + m_iSoundLevel);
				}
				else
				{
					// tap
					m_iGoalAnim = LookupActivity(ACT_T_TAP + m_iSoundLevel);
				}
			}
			else if (MyLevel() < 0)
			{
				m_iGoalAnim = LookupActivity(ACT_T_IDLE + 0);
			}
			else
			{
				if (m_flNextSong < gpGlobals->time)
				{
					// play "I hear new something" sound
					const char* sound;

					switch (RANDOM_LONG(0, 1))
					{
					case 0: sound = "tentacle/te_sing1.wav"; break;
					case 1: sound = "tentacle/te_sing2.wav"; break;
					}

					EmitSound(SoundChannel::Voice, sound);

					m_flNextSong = gpGlobals->time + RANDOM_FLOAT(10, 20);
				}

				if (RANDOM_LONG(0, 15) == 0)
				{
					// idle on new level
					m_iGoalAnim = LookupActivity(ACT_T_IDLE + RANDOM_LONG(0, 3));
				}
				else if (RANDOM_LONG(0, 3) == 0)
				{
					// tap
					m_iGoalAnim = LookupActivity(ACT_T_TAP + MyLevel());
				}
				else
				{
					// idle
					m_iGoalAnim = LookupActivity(ACT_T_IDLE + MyLevel());
				}
			}
			if (m_flSoundYaw < 0)
				m_flSoundYaw += RANDOM_FLOAT(2, 8);
			else
				m_flSoundYaw -= RANDOM_FLOAT(2, 8);
		}

		pev->sequence = FindTransition(pev->sequence, m_iGoalAnim, m_iDir);

		if (m_iDir > 0)
		{
			pev->frame = 0;
		}
		else
		{
			m_iDir = -1; // just to safe
			pev->frame = 255;
		}
		ResetSequenceInfo();

		m_flFramerateAdj = RANDOM_FLOAT(-0.2, 0.2);
		pev->framerate = m_iDir * 1.0 + m_flFramerateAdj;

		switch (pev->sequence)
		{
		case TENTACLE_ANIM_Floor_Tap:
		case TENTACLE_ANIM_Lev1_Tap:
		case TENTACLE_ANIM_Lev2_Tap:
		case TENTACLE_ANIM_Lev3_Tap:
		{
			UTIL_MakeVectors(GetAbsAngles());

			TraceResult tr1, tr2;

			Vector vecSrc = GetAbsOrigin() + Vector(0, 0, MyHeight() - 4);
			UTIL_TraceLine(vecSrc, vecSrc + gpGlobals->v_forward * 512, IgnoreMonsters::Yes, this, &tr1);

			vecSrc = GetAbsOrigin() + Vector(0, 0, MyHeight() + 8);
			UTIL_TraceLine(vecSrc, vecSrc + gpGlobals->v_forward * 512, IgnoreMonsters::Yes, this, &tr2);

			// ALERT( at_console, "%f %f\n", tr1.flFraction * 512, tr2.flFraction * 512 );

			m_flTapRadius = SetBlending(0, RANDOM_FLOAT(tr1.flFraction * 512, tr2.flFraction * 512));
		}
		break;
		default:
			m_flTapRadius = 336; // 400 - 64
			break;
		}
		pev->view_ofs.z = MyHeight();
		// ALERT( at_console, "seq %d\n", pev->sequence );
	}

	if (m_flPrevSoundTime + 2.0 > gpGlobals->time)
	{
		// 1.5 normal speed if hears sounds
		pev->framerate = m_iDir * 1.5 + m_flFramerateAdj;
	}
	else if (m_flPrevSoundTime + 5.0 > gpGlobals->time)
	{
		// slowdown to normal
		pev->framerate = m_iDir + m_iDir * (5 - (gpGlobals->time - m_flPrevSoundTime)) / 2 + m_flFramerateAdj;
	}
}

void CTentacle::CommandUse(const UseInfo& info)
{
	// ALERT( at_console, "%s triggered %d\n", STRING( pev->targetname ), useType ); 
	switch (info.GetUseType())
	{
	case UseType::Off:
		SetDamageMode(DamageMode::No);
		SetThink(&CTentacle::DieThink);
		m_iGoalAnim = TENTACLE_ANIM_Engine_Death1;
		break;
	case UseType::On:
		if (info.GetActivator())
		{
			// ALERT( at_console, "insert sound\n");
			CSoundEnt::InsertSound(bits_SOUND_WORLD, info.GetActivator()->GetAbsOrigin(), 1024, 1.0);
		}
		break;
	case UseType::Set:
		break;
	case UseType::Toggle:
		SetDamageMode(DamageMode::No);
		SetThink(&CTentacle::DieThink);
		m_iGoalAnim = TENTACLE_ANIM_Engine_Idle;
		break;
	}
}

void CTentacle::DieThink()
{
	pev->nextthink = gpGlobals->time + 0.1;

	DispatchAnimEvents();
	StudioFrameAdvance();

	ChangeYaw(24);

	if (m_fSequenceFinished)
	{
		if (pev->sequence == m_iGoalAnim)
		{
			switch (m_iGoalAnim)
			{
			case TENTACLE_ANIM_Engine_Idle:
			case TENTACLE_ANIM_Engine_Sway:
			case TENTACLE_ANIM_Engine_Swat:
			case TENTACLE_ANIM_Engine_Bob:
				m_iGoalAnim = TENTACLE_ANIM_Engine_Sway + RANDOM_LONG(0, 2);
				break;
			case TENTACLE_ANIM_Engine_Death1:
			case TENTACLE_ANIM_Engine_Death2:
			case TENTACLE_ANIM_Engine_Death3:
				UTIL_Remove(this);
				return;
			}
		}

		// ALERT( at_console, "%d : %d => ", pev->sequence, m_iGoalAnim );
		pev->sequence = FindTransition(pev->sequence, m_iGoalAnim, m_iDir);
		// ALERT( at_console, "%d\n", pev->sequence );

		if (m_iDir > 0)
		{
			pev->frame = 0;
		}
		else
		{
			pev->frame = 255;
		}
		ResetSequenceInfo();

		float dy;
		switch (pev->sequence)
		{
		case TENTACLE_ANIM_Floor_Rear:
		case TENTACLE_ANIM_Floor_Rear_Idle:
		case TENTACLE_ANIM_Lev1_Rear:
		case TENTACLE_ANIM_Lev1_Rear_Idle:
		case TENTACLE_ANIM_Lev2_Rear:
		case TENTACLE_ANIM_Lev2_Rear_Idle:
		case TENTACLE_ANIM_Lev3_Rear:
		case TENTACLE_ANIM_Lev3_Rear_Idle:
		case TENTACLE_ANIM_Engine_Idle:
		case TENTACLE_ANIM_Engine_Sway:
		case TENTACLE_ANIM_Engine_Swat:
		case TENTACLE_ANIM_Engine_Bob:
		case TENTACLE_ANIM_Engine_Death1:
		case TENTACLE_ANIM_Engine_Death2:
		case TENTACLE_ANIM_Engine_Death3:
			pev->framerate = RANDOM_FLOAT(m_iDir - 0.2, m_iDir + 0.2);
			dy = 180;
			break;
		default:
			pev->framerate = 1.5;
			dy = 0;
			break;
		}
		pev->ideal_yaw = m_flInitialYaw + dy;
	}
}

void CTentacle::HandleAnimEvent(AnimationEvent& event)
{
	const char* sound = nullptr;

	switch (event.event)
	{
	case 1:	// bang 
	{
		Vector vecSrc, vecAngles;
		GetAttachment(0, vecSrc, vecAngles);

		// Vector vecSrc = GetAbsOrigin() + m_flTapRadius * Vector( cos(GetAbsAngles().y * (3.14192653 / 180.0) ), sin(GetAbsAngles().y * (M_PI / 180.0) ), 0.0 );

		// vecSrc.z += MyHeight( );

		switch (m_iTapSound)
		{
		case TE_SILO:
			UTIL_EmitAmbientSound(this, vecSrc, RANDOM_SOUND_ARRAY(pHitSilo), 1.0, ATTN_NORM, 0, 100);
			break;
		case TE_NONE:
			break;
		case TE_DIRT:
			UTIL_EmitAmbientSound(this, vecSrc, RANDOM_SOUND_ARRAY(pHitDirt), 1.0, ATTN_NORM, 0, 100);
			break;
		case TE_WATER:
			UTIL_EmitAmbientSound(this, vecSrc, RANDOM_SOUND_ARRAY(pHitWater), 1.0, ATTN_NORM, 0, 100);
			break;
		}
		gpGlobals->force_retouch++;
	}
	break;

	case 3: // start killing swing
		m_iHitDmg = 200;
		// UTIL_EmitAmbientSound(this, GetAbsOrigin() + Vector( 0, 0, MyHeight()), "tentacle/te_swing1.wav", 1.0, ATTN_NORM, 0, 100);
		break;

	case 4: // end killing swing
		m_iHitDmg = 25;
		break;

	case 5: // just "whoosh" sound
		// UTIL_EmitAmbientSound(this, GetAbsOrigin() + Vector( 0, 0, MyHeight()), "tentacle/te_swing2.wav", 1.0, ATTN_NORM, 0, 100);
		break;

	case 2:	// tap scrape
	case 6: // light tap
	{
		Vector vecSrc = GetAbsOrigin() + m_flTapRadius * Vector(cos(GetAbsAngles().y * (M_PI / 180.0)), sin(GetAbsAngles().y * (M_PI / 180.0)), 0.0);

		vecSrc.z += MyHeight();

		const float flVol = RANDOM_FLOAT(0.3, 0.5);

		switch (m_iTapSound)
		{
		case TE_SILO:
			UTIL_EmitAmbientSound(this, vecSrc, RANDOM_SOUND_ARRAY(pHitSilo), flVol, ATTN_NORM, 0, 100);
			break;
		case TE_NONE:
			break;
		case TE_DIRT:
			UTIL_EmitAmbientSound(this, vecSrc, RANDOM_SOUND_ARRAY(pHitDirt), flVol, ATTN_NORM, 0, 100);
			break;
		case TE_WATER:
			UTIL_EmitAmbientSound(this, vecSrc, RANDOM_SOUND_ARRAY(pHitWater), flVol, ATTN_NORM, 0, 100);
			break;
		}
	}
	break;


	case 7: // roar
		switch (RANDOM_LONG(0, 1))
		{
		case 0: sound = "tentacle/te_roar1.wav"; break;
		case 1: sound = "tentacle/te_roar2.wav"; break;
		}

		UTIL_EmitAmbientSound(this, GetAbsOrigin() + Vector(0, 0, MyHeight()), sound, 1.0, ATTN_NORM, 0, 100);
		break;

	case 8: // search
		switch (RANDOM_LONG(0, 1))
		{
		case 0: sound = "tentacle/te_search1.wav"; break;
		case 1: sound = "tentacle/te_search2.wav"; break;
		}

		UTIL_EmitAmbientSound(this, GetAbsOrigin() + Vector(0, 0, MyHeight()), sound, 1.0, ATTN_NORM, 0, 100);
		break;

	case 9: // swing
		switch (RANDOM_LONG(0, 1))
		{
		case 0: sound = "tentacle/te_move1.wav"; break;
		case 1: sound = "tentacle/te_move2.wav"; break;
		}

		UTIL_EmitAmbientSound(this, GetAbsOrigin() + Vector(0, 0, MyHeight()), sound, 1.0, ATTN_NORM, 0, 100);
		break;

	default:
		CBaseMonster::HandleAnimEvent(event);
	}
}

void CTentacle::Start()
{
	SetThink(&CTentacle::Cycle);

	if (!g_fFlySound)
	{
		EmitSound(SoundChannel::Body, "ambience/flies.wav");
		g_fFlySound = true;
		//		pev->nextthink = gpGlobals-> time + 0.1;
	}
	else if (!g_fSquirmSound)
	{
		EmitSound(SoundChannel::Body, "ambience/squirm2.wav");
		g_fSquirmSound = true;
	}

	pev->nextthink = gpGlobals->time + 0.1;
}

void CTentacle::HitTouch(CBaseEntity* pOther)
{
	const TraceResult tr = UTIL_GetGlobalTrace();

	if (pOther->pev->modelindex == pev->modelindex)
		return;

	if (m_flHitTime > gpGlobals->time)
		return;

	// only look at the ones where the player hit me
	if (tr.pHit == nullptr || tr.pHit->v.modelindex != pev->modelindex)
		return;

	if (tr.iHitgroup >= 3)
	{
		pOther->TakeDamage({this, this, static_cast<float>(m_iHitDmg), DMG_CRUSH});
		// ALERT( at_console, "wack %3d : ", m_iHitDmg );
	}
	else if (tr.iHitgroup != 0)
	{
		pOther->TakeDamage({this, this, 20, DMG_CRUSH});
		// ALERT( at_console, "tap  %3d : ", 20 );
	}
	else
	{
		return; // Huh?
	}

	m_flHitTime = gpGlobals->time + 0.5;

	// ALERT( at_console, "%s : ", STRING( tr.pHit->v.classname ) );

	// ALERT( at_console, "%.0f : %s : %d\n", GetAbsAngles().y, pOther->GetClassname(), tr.iHitgroup );
}

bool CTentacle::TakeDamage(const TakeDamageInfo& info)
{
	if (info.GetDamage() > pev->health)
	{
		pev->health = 1;
	}
	else
	{
		pev->health -= info.GetDamage();
	}
	return true;
}

void CTentacle::Killed(const KilledInfo& info)
{
	m_iGoalAnim = TENTACLE_ANIM_Pit_Idle;
	return;
}

class CTentacleMaw : public CBaseMonster
{
public:
	void Spawn() override;
	void Precache() override;
};

LINK_ENTITY_TO_CLASS(monster_tentaclemaw, CTentacleMaw);

void CTentacleMaw::Spawn()
{
	Precache();
	SetModel("models/maw.mdl");
	SetSize(Vector(-32, -32, 0), Vector(32, 32, 64));

	SetSolidType(Solid::Not);
	SetMovetype(Movetype::Step);
	pev->effects = 0;
	pev->health = 75;
	pev->yaw_speed = 8;
	pev->sequence = 0;

	SetAbsAngles({90, GetAbsAngles().y, GetAbsAngles().z});

	// ResetSequenceInfo( );
}

void CTentacleMaw::Precache()
{
	PRECACHE_MODEL("models/maw.mdl");
}
