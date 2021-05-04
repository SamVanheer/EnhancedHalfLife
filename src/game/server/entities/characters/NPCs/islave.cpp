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

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
constexpr int ISLAVE_AE_CLAW = 1;
constexpr int ISLAVE_AE_CLAWRAKE = 2;
constexpr int ISLAVE_AE_ZAP_POWERUP = 3;
constexpr int ISLAVE_AE_ZAP_SHOOT = 4;
constexpr int ISLAVE_AE_ZAP_DONE = 5;

constexpr int ISLAVE_MAX_BEAMS = 8;

constexpr int SF_ISLAVE_IS_REVIVED_SLAVE = 1 << 0;

/**
*	@brief Alien slave monster
*/
class CISlave : public CSquadMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int	 SoundMask() override;
	int  Classify() override;
	Relationship GetRelationship(CBaseEntity* pTarget) override;
	void HandleAnimEvent(AnimationEvent& event) override;

	/**
	*	@brief normal beam attack
	*/
	bool CheckRangeAttack1(float flDot, float flDist) override;

	/**
	*	@brief check bravery and try to resurect dead comrades
	*/
	bool CheckRangeAttack2(float flDot, float flDist) override;
	void CallForHelp(const char* szClassname, float flDist, EHANDLE hEnemy, Vector& vecLocation);
	void TraceAttack(const TraceAttackInfo& info) override;

	/**
	*	@brief get provoked when injured
	*/
	bool TakeDamage(const TakeDamageInfo& info) override;

	void DeathSound() override;
	void PainSound() override;
	void AlertSound() override;
	void IdleSound() override;

	void Killed(const KilledInfo& info) override;

	void StartTask(Task_t* pTask) override;
	Schedule_t* GetSchedule() override;
	Schedule_t* GetScheduleOfType(int Type) override;
	CUSTOM_SCHEDULES;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	void ClearBeams();

	/**
	*	@brief small beam from arm to nearby geometry
	*/
	void ArmBeam(int side);

	/**
	*	@brief regenerate dead colleagues
	*/
	void WackBeam(int side, CBaseEntity* pEntity);

	/**
	*	@brief heavy damage directly forward
	*/
	void ZapBeam(int side);

	/**
	*	@brief brighten all beams
	*/
	void BeamGlow();

	int m_iBravery;

	EHandle<CBeam> m_hBeam[ISLAVE_MAX_BEAMS];

	int m_iBeams;
	float m_flNextAttack;

	int	m_voicePitch;

	EHANDLE m_hDead;

	static const char* pAttackHitSounds[];
	static const char* pAttackMissSounds[];
	static const char* pPainSounds[];
	static const char* pDeathSounds[];
};

LINK_ENTITY_TO_CLASS(monster_alien_slave, CISlave);
LINK_ENTITY_TO_CLASS(monster_vortigaunt, CISlave);

TYPEDESCRIPTION	CISlave::m_SaveData[] =
{
	DEFINE_FIELD(CISlave, m_iBravery, FIELD_INTEGER),

	DEFINE_ARRAY(CISlave, m_hBeam, FIELD_EHANDLE, ISLAVE_MAX_BEAMS),
	DEFINE_FIELD(CISlave, m_iBeams, FIELD_INTEGER),
	DEFINE_FIELD(CISlave, m_flNextAttack, FIELD_TIME),

	DEFINE_FIELD(CISlave, m_voicePitch, FIELD_INTEGER),

	DEFINE_FIELD(CISlave, m_hDead, FIELD_EHANDLE),

};

IMPLEMENT_SAVERESTORE(CISlave, CSquadMonster);

const char* CISlave::pAttackHitSounds[] =
{
	"zombie/claw_strike1.wav",
	"zombie/claw_strike2.wav",
	"zombie/claw_strike3.wav",
};

const char* CISlave::pAttackMissSounds[] =
{
	"zombie/claw_miss1.wav",
	"zombie/claw_miss2.wav",
};

const char* CISlave::pPainSounds[] =
{
	"aslave/slv_pain1.wav",
	"aslave/slv_pain2.wav",
};

const char* CISlave::pDeathSounds[] =
{
	"aslave/slv_die1.wav",
	"aslave/slv_die2.wav",
};

int	CISlave::Classify()
{
	return	CLASS_ALIEN_MILITARY;
}

Relationship CISlave::GetRelationship(CBaseEntity* pTarget)
{
	if ((pTarget->IsPlayer()))
		if ((pev->spawnflags & SF_MONSTER_WAIT_UNTIL_PROVOKED) && !(m_afMemory & bits_MEMORY_PROVOKED))
			return Relationship::None;
	return CBaseMonster::GetRelationship(pTarget);
}

void CISlave::CallForHelp(const char* szClassname, float flDist, EHANDLE hEnemy, Vector& vecLocation)
{
	// ALERT( at_aiconsole, "help " );

	// skip ones not on my netname
	if (IsStringNull(pev->netname))
		return;

	CBaseEntity* pEntity = nullptr;

	while ((pEntity = UTIL_FindEntityByString(pEntity, "netname", STRING(pev->netname))) != nullptr)
	{
		const float d = (GetAbsOrigin() - pEntity->GetAbsOrigin()).Length();
		if (d < flDist)
		{
			CBaseMonster* pMonster = pEntity->MyMonsterPointer();
			if (pMonster)
			{
				pMonster->m_afMemory |= bits_MEMORY_PROVOKED;
				pMonster->PushEnemy(hEnemy, vecLocation);
			}
		}
	}
}

void CISlave::AlertSound()
{
	if (m_hEnemy != nullptr)
	{
		SENTENCEG_PlayRndSz(this, "SLV_ALERT", 0.85, ATTN_NORM, m_voicePitch);

		CallForHelp("monster_alien_slave", 512, m_hEnemy, m_vecEnemyLKP);
	}
}

void CISlave::IdleSound()
{
	if (RANDOM_LONG(0, 2) == 0)
	{
		SENTENCEG_PlayRndSz(this, "SLV_IDLE", 0.85, ATTN_NORM, m_voicePitch);
	}

#if 0
	int side = RANDOM_LONG(0, 1) * 2 - 1;

	ClearBeams();
	ArmBeam(side);

	UTIL_MakeAimVectors(GetAbsAngles());
	Vector vecSrc = GetAbsOrigin() + gpGlobals->v_right * 2 * side;
	MESSAGE_BEGIN(MessageDest::PVS, SVC_TEMPENTITY, vecSrc);
	WRITE_BYTE(TE_DLIGHT);
	WRITE_COORD(vecSrc.x);	// X
	WRITE_COORD(vecSrc.y);	// Y
	WRITE_COORD(vecSrc.z);	// Z
	WRITE_BYTE(8);		// radius * 0.1
	WRITE_BYTE(255);		// r
	WRITE_BYTE(180);		// g
	WRITE_BYTE(96);		// b
	WRITE_BYTE(10);		// time * 10
	WRITE_BYTE(0);		// decay * 0.1
	MESSAGE_END();

	EmitSound(SoundChannel::Weapon, "debris/zap1.wav");
#endif
}

void CISlave::PainSound()
{
	if (RANDOM_LONG(0, 2) == 0)
	{
		EmitSound(SoundChannel::Weapon, pPainSounds[RANDOM_LONG(0, ArraySize(pPainSounds) - 1)], VOL_NORM, ATTN_NORM, m_voicePitch);
	}
}

void CISlave::DeathSound()
{
	EmitSound(SoundChannel::Weapon, pDeathSounds[RANDOM_LONG(0, ArraySize(pDeathSounds) - 1)], VOL_NORM, ATTN_NORM, m_voicePitch);
}

int CISlave::SoundMask()
{
	return bits_SOUND_WORLD |
		bits_SOUND_COMBAT |
		bits_SOUND_DANGER |
		bits_SOUND_PLAYER;
}


void CISlave::Killed(const KilledInfo& info)
{
	ClearBeams();
	CSquadMonster::Killed(info);
}

void CISlave::SetYawSpeed()
{
	int ys;

	switch (m_Activity)
	{
	case ACT_WALK:
		ys = 50;
		break;
	case ACT_RUN:
		ys = 70;
		break;
	case ACT_IDLE:
		ys = 50;
		break;
	default:
		ys = 90;
		break;
	}

	pev->yaw_speed = ys;
}

void CISlave::HandleAnimEvent(AnimationEvent& event)
{
	// ALERT( at_console, "event %d : %f\n", pEvent->event, pev->frame );
	switch (event.event)
	{
	case ISLAVE_AE_CLAW:
	{
		// SOUND HERE!
		if (CBaseEntity* pHurt = CheckTraceHullAttack(70, gSkillData.slaveDmgClaw, DMG_SLASH); pHurt)
		{
			if (pHurt->pev->flags & (FL_MONSTER | FL_CLIENT))
			{
				pHurt->pev->punchangle.z = -18;
				pHurt->pev->punchangle.x = 5;
			}
			// Play a random attack hit sound
			EmitSound(SoundChannel::Weapon, pAttackHitSounds[RANDOM_LONG(0, ArraySize(pAttackHitSounds) - 1)], VOL_NORM, ATTN_NORM, m_voicePitch);
		}
		else
		{
			// Play a random attack miss sound
			EmitSound(SoundChannel::Weapon, pAttackMissSounds[RANDOM_LONG(0, ArraySize(pAttackMissSounds) - 1)], VOL_NORM, ATTN_NORM, m_voicePitch);
		}
	}
	break;

	case ISLAVE_AE_CLAWRAKE:
	{
		if (CBaseEntity* pHurt = CheckTraceHullAttack(70, gSkillData.slaveDmgClawrake, DMG_SLASH); pHurt)
		{
			if (pHurt->pev->flags & (FL_MONSTER | FL_CLIENT))
			{
				pHurt->pev->punchangle.z = -18;
				pHurt->pev->punchangle.x = 5;
			}
			EmitSound(SoundChannel::Weapon, pAttackHitSounds[RANDOM_LONG(0, ArraySize(pAttackHitSounds) - 1)], VOL_NORM, ATTN_NORM, m_voicePitch);
		}
		else
		{
			EmitSound(SoundChannel::Weapon, pAttackMissSounds[RANDOM_LONG(0, ArraySize(pAttackMissSounds) - 1)], VOL_NORM, ATTN_NORM, m_voicePitch);
		}
	}
	break;

	case ISLAVE_AE_ZAP_POWERUP:
	{
		// speed up attack when on hard
		if (g_SkillLevel == SkillLevel::Hard)
			pev->framerate = 1.5;

		UTIL_MakeAimVectors(GetAbsAngles());

		if (m_iBeams == 0)
		{
			const Vector vecSrc = GetAbsOrigin() + gpGlobals->v_forward * 2;
			MESSAGE_BEGIN(MessageDest::PVS, SVC_TEMPENTITY, vecSrc);
			WRITE_BYTE(TE_DLIGHT);
			WRITE_COORD(vecSrc.x);	// X
			WRITE_COORD(vecSrc.y);	// Y
			WRITE_COORD(vecSrc.z);	// Z
			WRITE_BYTE(12);		// radius * 0.1
			WRITE_BYTE(255);		// r
			WRITE_BYTE(180);		// g
			WRITE_BYTE(96);		// b
			WRITE_BYTE(20 / pev->framerate);		// time * 10
			WRITE_BYTE(0);		// decay * 0.1
			MESSAGE_END();
		}

		if (m_hDead != nullptr)
		{
			WackBeam(-1, m_hDead);
			WackBeam(1, m_hDead);
		}
		else
		{
			ArmBeam(-1);
			ArmBeam(1);
			BeamGlow();
		}

		EmitSound(SoundChannel::Weapon, "debris/zap4.wav", VOL_NORM, ATTN_NORM, PITCH_NORM + m_iBeams * 10);
		pev->skin = m_iBeams / 2;
	}
	break;

	case ISLAVE_AE_ZAP_SHOOT:
	{
		ClearBeams();

		if (m_hDead != nullptr)
		{
			const Vector vecDest = m_hDead->GetAbsOrigin() + Vector(0, 0, 38);
			TraceResult trace;
			UTIL_TraceHull(vecDest, vecDest, IgnoreMonsters::No, Hull::Human, m_hDead, &trace);

			if (!trace.fStartSolid)
			{
				CBaseEntity* pNew = Create("monster_alien_slave", m_hDead->GetAbsOrigin(), m_hDead->GetAbsAngles());
				CBaseMonster* pNewMonster = pNew->MyMonsterPointer();
				pNew->pev->spawnflags |= SF_ISLAVE_IS_REVIVED_SLAVE;
				WackBeam(-1, pNew);
				WackBeam(1, pNew);
				UTIL_Remove(m_hDead);
				EmitSound(SoundChannel::Weapon, "hassault/hw_shoot1.wav", VOL_NORM, ATTN_NORM, RANDOM_LONG(130, 160));

				/*
				CBaseEntity *pEffect = Create("test_effect", pNew->Center(), GetAbsAngles());
				pEffect->Use( this, this, UseType::On, 1 );
				*/
				break;
			}
		}
		ClearMultiDamage();

		UTIL_MakeAimVectors(GetAbsAngles());

		ZapBeam(-1);
		ZapBeam(1);

		EmitSound(SoundChannel::Weapon, "hassault/hw_shoot1.wav", VOL_NORM, ATTN_NORM, RANDOM_LONG(130, 160));
		// StopSound(SoundChannel::Weapon, "debris/zap4.wav" );
		ApplyMultiDamage(this, this);

		m_flNextAttack = gpGlobals->time + RANDOM_FLOAT(0.5, 4.0);
	}
	break;

	case ISLAVE_AE_ZAP_DONE:
	{
		ClearBeams();
	}
	break;

	default:
		CSquadMonster::HandleAnimEvent(event);
		break;
	}
}

bool CISlave::CheckRangeAttack1(float flDot, float flDist)
{
	if (m_flNextAttack > gpGlobals->time)
	{
		return false;
	}

	return CSquadMonster::CheckRangeAttack1(flDot, flDist);
}

bool CISlave::CheckRangeAttack2(float flDot, float flDist)
{
	return false;

	if (m_flNextAttack > gpGlobals->time)
	{
		return false;
	}

	m_hDead = nullptr;
	m_iBravery = 0;

	CBaseEntity* pEntity = nullptr;
	while ((pEntity = UTIL_FindEntityByClassname(pEntity, "monster_alien_slave")) != nullptr)
	{
		TraceResult tr;

		UTIL_TraceLine(EyePosition(), pEntity->EyePosition(), IgnoreMonsters::Yes, this, &tr);
		if (tr.flFraction == 1.0 || InstanceOrNull(tr.pHit) == pEntity)
		{
			if (pEntity->pev->deadflag == DeadFlag::Dead)
			{
				const float d = (GetAbsOrigin() - pEntity->GetAbsOrigin()).Length();
				if (d < flDist)
				{
					m_hDead = pEntity;
					flDist = d;
				}
				m_iBravery--;
			}
			else
			{
				m_iBravery++;
			}
		}
	}

	return m_hDead != nullptr;
}

void CISlave::StartTask(Task_t* pTask)
{
	ClearBeams();

	CSquadMonster::StartTask(pTask);
}

void CISlave::Spawn()
{
	Precache();

	SetModel("models/islave.mdl");
	SetSize(VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	SetSolidType(Solid::SlideBox);
	SetMovetype(Movetype::Step);
	m_bloodColor = BLOOD_COLOR_GREEN;
	pev->effects = 0;
	pev->health = gSkillData.slaveHealth;
	pev->view_ofs = Vector(0, 0, 64);// position of the eyes relative to monster's origin.
	m_flFieldOfView = VIEW_FIELD_WIDE; // NOTE: we need a wide field of view so npc will notice player and say hello
	m_MonsterState = NPCState::None;
	m_afCapability = bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_RANGE_ATTACK2 | bits_CAP_DOORS_GROUP;

	m_voicePitch = RANDOM_LONG(85, 110);

	MonsterInit();
}

void CISlave::Precache()
{
	PRECACHE_MODEL("models/islave.mdl");
	PRECACHE_MODEL("sprites/lgtning.spr");
	PRECACHE_SOUND("debris/zap1.wav");
	PRECACHE_SOUND("debris/zap4.wav");
	PRECACHE_SOUND("weapons/electro4.wav");
	PRECACHE_SOUND("hassault/hw_shoot1.wav");
	PRECACHE_SOUND("zombie/zo_pain2.wav");
	PRECACHE_SOUND("headcrab/hc_headbite.wav");
	PRECACHE_SOUND("weapons/cbar_miss1.wav");

	PRECACHE_SOUND_ARRAY(pAttackHitSounds);
	PRECACHE_SOUND_ARRAY(pAttackMissSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pDeathSounds);

	UTIL_PrecacheOther("test_effect");
}

bool CISlave::TakeDamage(const TakeDamageInfo& info)
{
	// don't slash one of your own
	if ((info.GetDamageTypes() & DMG_SLASH) && info.GetAttacker() && GetRelationship(info.GetAttacker()) < Relationship::Dislike)
		return false;

	m_afMemory |= bits_MEMORY_PROVOKED;
	return CSquadMonster::TakeDamage(info);
}

void CISlave::TraceAttack(const TraceAttackInfo& info)
{
	if (info.GetDamageTypes() & DMG_SHOCK)
		return;

	CSquadMonster::TraceAttack(info);
}

//=========================================================
// AI Schedules Specific to this monster
//=========================================================
Task_t	tlSlaveAttack1[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_FACE_IDEAL,			(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
};

/**
*	@brief primary range attack
*/
Schedule_t	slSlaveAttack1[] =
{
	{
		tlSlaveAttack1,
		ArraySize(tlSlaveAttack1),
		bits_COND_CAN_MELEE_ATTACK1 |
		bits_COND_HEAR_SOUND |
		bits_COND_HEAVY_DAMAGE,

		bits_SOUND_DANGER,
		"Slave Range Attack1"
	},
};

DEFINE_CUSTOM_SCHEDULES(CISlave)
{
	slSlaveAttack1,
};

IMPLEMENT_CUSTOM_SCHEDULES(CISlave, CSquadMonster);

Schedule_t* CISlave::GetSchedule()
{
	ClearBeams();

	/*
		if (pev->spawnflags & SF_ISLAVE_IS_REVIVED_SLAVE)
		{
			pev->spawnflags &= ~SF_ISLAVE_IS_REVIVED_SLAVE;
			return GetScheduleOfType( SCHED_RELOAD );
		}
	*/

	if (HasConditions(bits_COND_HEAR_SOUND))
	{
		CSound* pSound = BestSound();

		ASSERT(pSound != nullptr);

		if (pSound && (pSound->m_iType & bits_SOUND_DANGER))
			return GetScheduleOfType(SCHED_TAKE_COVER_FROM_BEST_SOUND);
		if (pSound->m_iType & bits_SOUND_COMBAT)
			m_afMemory |= bits_MEMORY_PROVOKED;
	}

	switch (m_MonsterState)
	{
	case NPCState::Combat:
		// dead enemy
		if (HasConditions(bits_COND_ENEMY_DEAD))
		{
			// call base class, all code to handle dead enemies is centralized there.
			return CBaseMonster::GetSchedule();
		}

		if (pev->health < 20 || m_iBravery < 0)
		{
			if (!HasConditions(bits_COND_CAN_MELEE_ATTACK1))
			{
				m_failSchedule = SCHED_CHASE_ENEMY;
				if (HasConditions(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE))
				{
					return GetScheduleOfType(SCHED_TAKE_COVER_FROM_ENEMY);
				}
				if (HasConditions(bits_COND_SEE_ENEMY) && HasConditions(bits_COND_ENEMY_FACING_ME))
				{
					// ALERT( at_console, "exposed\n");
					return GetScheduleOfType(SCHED_TAKE_COVER_FROM_ENEMY);
				}
			}
		}
		break;
	}
	return CSquadMonster::GetSchedule();
}

Schedule_t* CISlave::GetScheduleOfType(int Type)
{
	switch (Type)
	{
	case SCHED_FAIL:
		if (HasConditions(bits_COND_CAN_MELEE_ATTACK1))
		{
			return CSquadMonster::GetScheduleOfType(SCHED_MELEE_ATTACK1);
		}
		break;
	case SCHED_RANGE_ATTACK1:
		return slSlaveAttack1;
	case SCHED_RANGE_ATTACK2:
		return slSlaveAttack1;
	}
	return CSquadMonster::GetScheduleOfType(Type);
}

void CISlave::ArmBeam(int side)
{
	TraceResult tr;
	float flDist = 1.0;

	if (m_iBeams >= ISLAVE_MAX_BEAMS)
		return;

	UTIL_MakeAimVectors(GetAbsAngles());
	const Vector vecSrc = GetAbsOrigin() + gpGlobals->v_up * 36 + gpGlobals->v_right * side * 16 + gpGlobals->v_forward * 32;

	for (int i = 0; i < 3; i++)
	{
		const Vector vecAim = gpGlobals->v_right * side * RANDOM_FLOAT(0, 1) + gpGlobals->v_up * RANDOM_FLOAT(-1, 1);
		TraceResult tr1;
		UTIL_TraceLine(vecSrc, vecSrc + vecAim * 512, IgnoreMonsters::No, this, &tr1);
		if (flDist > tr1.flFraction)
		{
			tr = tr1;
			flDist = tr.flFraction;
		}
	}

	// Couldn't find anything close enough
	if (flDist == 1.0)
		return;

	DecalGunshot(&tr, BULLET_PLAYER_CROWBAR);

	auto beam = m_hBeam[m_iBeams] = CBeam::BeamCreate("sprites/lgtning.spr", 30);
	if (!beam)
		return;

	beam->PointEntInit(tr.vecEndPos, entindex());
	beam->SetEndAttachment(side < 0 ? 2 : 1);
	// beam->SetColor( 180, 255, 96 );
	beam->SetColor(96, 128, 16);
	beam->SetBrightness(64);
	beam->SetNoise(80);
	m_iBeams++;
}

void CISlave::BeamGlow()
{
	const int b = std::min(255, m_iBeams * 32);

	for (int i = 0; i < m_iBeams; i++)
	{
		auto beam = m_hBeam[i];
		if (beam->GetBrightness() != 255)
		{
			beam->SetBrightness(b);
		}
	}
}

void CISlave::WackBeam(int side, CBaseEntity* pEntity)
{
	if (m_iBeams >= ISLAVE_MAX_BEAMS)
		return;

	if (pEntity == nullptr)
		return;

	auto beam = m_hBeam[m_iBeams] = CBeam::BeamCreate("sprites/lgtning.spr", 30);
	if (!beam)
		return;

	beam->PointEntInit(pEntity->Center(), entindex());
	beam->SetEndAttachment(side < 0 ? 2 : 1);
	beam->SetColor(180, 255, 96);
	beam->SetBrightness(255);
	beam->SetNoise(80);
	m_iBeams++;
}

void CISlave::ZapBeam(int side)
{
	if (m_iBeams >= ISLAVE_MAX_BEAMS)
		return;

	const Vector vecSrc = GetAbsOrigin() + gpGlobals->v_up * 36;
	Vector vecAim = ShootAtEnemy(vecSrc);
	float deflection = 0.01;
	vecAim = vecAim + side * gpGlobals->v_right * RANDOM_FLOAT(0, deflection) + gpGlobals->v_up * RANDOM_FLOAT(-deflection, deflection);

	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecSrc + vecAim * 1024, IgnoreMonsters::No, this, &tr);

	auto beam = m_hBeam[m_iBeams] = CBeam::BeamCreate("sprites/lgtning.spr", 50);
	if (!beam)
		return;

	beam->PointEntInit(tr.vecEndPos, entindex());
	beam->SetEndAttachment(side < 0 ? 2 : 1);
	beam->SetColor(180, 255, 96);
	beam->SetBrightness(255);
	beam->SetNoise(20);
	m_iBeams++;

	if (CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit); pEntity != nullptr && pEntity->pev->takedamage)
	{
		pEntity->TraceAttack({this, gSkillData.slaveDmgZap, vecAim, tr, DMG_SHOCK});
	}
	UTIL_EmitAmbientSound(this, tr.vecEndPos, "weapons/electro4.wav", 0.5, ATTN_NORM, 0, RANDOM_LONG(140, 160));
}

void CISlave::ClearBeams()
{
	for (int i = 0; i < ISLAVE_MAX_BEAMS; i++)
	{
		if (m_hBeam[i])
		{
			UTIL_Remove(m_hBeam[i]);
			m_hBeam[i] = nullptr;
		}
	}
	m_iBeams = 0;
	pev->skin = 0;

	StopSound(SoundChannel::Weapon, "debris/zap4.wav");
}
