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

// UNDONE: Holster weapon?

#include "CBarney.hpp"
#include "CBaseMonster.defaultai.hpp"

//=========================================================
// AI Schedules Specific to this monster
//=========================================================
Task_t	tlBaFollow[] =
{
	{ TASK_MOVE_TO_TARGET_RANGE,(float)128		},	// Move within 128 of target ent (client)
	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_FACE },
};

Schedule_t	slBaFollow[] =
{
	{
		tlBaFollow,
		ArraySize(tlBaFollow),
		bits_COND_NEW_ENEMY |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_HEAR_SOUND |
		bits_COND_PROVOKED,
		bits_SOUND_DANGER,
		"Follow"
	},
};

Task_t	tlBarneyEnemyDraw[] =
{
	{ TASK_STOP_MOVING,					0				},
	{ TASK_FACE_ENEMY,					0				},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,	(float)ACT_ARM },
};

/**
*	@brief much better looking draw schedule for when barney knows who he's gonna attack.
*/
Schedule_t slBarneyEnemyDraw[] =
{
	{
		tlBarneyEnemyDraw,
		ArraySize(tlBarneyEnemyDraw),
		0,
		0,
		"Barney Enemy Draw"
	}
};

Task_t	tlBaFaceTarget[] =
{
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_FACE_TARGET,			(float)0		},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_CHASE },
};

Schedule_t	slBaFaceTarget[] =
{
	{
		tlBaFaceTarget,
		ArraySize(tlBaFaceTarget),
		bits_COND_CLIENT_PUSH |
		bits_COND_NEW_ENEMY |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_HEAR_SOUND |
		bits_COND_PROVOKED,
		bits_SOUND_DANGER,
		"FaceTarget"
	},
};

Task_t	tlIdleBaStand[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT,				(float)2		}, // repick IDLESTAND every two seconds.
	{ TASK_TLK_HEADRESET,		(float)0		}, // reset head position
};

Schedule_t	slIdleBaStand[] =
{
	{
		tlIdleBaStand,
		ArraySize(tlIdleBaStand),
		bits_COND_NEW_ENEMY |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_HEAR_SOUND |
		bits_COND_SMELL |
		bits_COND_PROVOKED,

		bits_SOUND_COMBAT |// sound flags - change these, and you'll break the talking code.
		//bits_SOUND_PLAYER		|
		//bits_SOUND_WORLD		|

		bits_SOUND_DANGER |
		bits_SOUND_MEAT |// scents
		bits_SOUND_CARCASS |
		bits_SOUND_GARBAGE,
		"IdleStand"
	},
};

DEFINE_CUSTOM_SCHEDULES(CBarney)
{
	slBaFollow,
		slBarneyEnemyDraw,
		slBaFaceTarget,
		slIdleBaStand,
};

IMPLEMENT_CUSTOM_SCHEDULES(CBarney, CTalkMonster);

void CBarney::StartTask(Task_t* pTask)
{
	CTalkMonster::StartTask(pTask);
}

void CBarney::RunTask(Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_RANGE_ATTACK1:
		if (m_hEnemy != nullptr && (m_hEnemy->IsPlayer()))
		{
			pev->framerate = 1.5;
		}
		CTalkMonster::RunTask(pTask);
		break;
	default:
		CTalkMonster::RunTask(pTask);
		break;
	}
}

int CBarney::SoundMask()
{
	return	bits_SOUND_WORLD |
		bits_SOUND_COMBAT |
		bits_SOUND_CARCASS |
		bits_SOUND_MEAT |
		bits_SOUND_GARBAGE |
		bits_SOUND_DANGER |
		bits_SOUND_PLAYER;
}

int	CBarney::Classify()
{
	return	CLASS_PLAYER_ALLY;
}

void CBarney::AlertSound()
{
	if (m_hEnemy != nullptr)
	{
		if (OkToSpeak())
		{
			PlaySentence("BA_ATTACK", RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_IDLE);
		}
	}
}

void CBarney::SetYawSpeed()
{
	int ys = 0;

	switch (m_Activity)
	{
	case ACT_IDLE:
		ys = 70;
		break;
	case ACT_WALK:
		ys = 70;
		break;
	case ACT_RUN:
		ys = 90;
		break;
	default:
		ys = 70;
		break;
	}

	pev->yaw_speed = ys;
}

bool CBarney::CheckRangeAttack1(float flDot, float flDist)
{
	if (flDist <= 1024 && flDot >= 0.5)
	{
		if (gpGlobals->time > m_checkAttackTime)
		{
			TraceResult tr;

			const Vector shootOrigin = GetAbsOrigin() + Vector(0, 0, 55);
			CBaseEntity* pEnemy = m_hEnemy;
			const Vector shootTarget = ((pEnemy->BodyTarget(shootOrigin) - pEnemy->GetAbsOrigin()) + m_vecEnemyLKP);
			UTIL_TraceLine(shootOrigin, shootTarget, IgnoreMonsters::No, this, &tr);
			m_checkAttackTime = gpGlobals->time + 1; //TODO: done twice

			m_lastAttackCheck = tr.flFraction == 1.0 || (tr.pHit != nullptr && CBaseEntity::Instance(tr.pHit) == pEnemy);

			m_checkAttackTime = gpGlobals->time + 1.5;
		}
		return m_lastAttackCheck;
	}
	return false;
}

void CBarney::BarneyFirePistol()
{
	UTIL_MakeVectors(GetAbsAngles());
	const Vector vecShootOrigin = GetAbsOrigin() + Vector(0, 0, 55);
	const Vector vecShootDir = ShootAtEnemy(vecShootOrigin);

	const Vector angDir = VectorAngles(vecShootDir);
	SetBlending(0, angDir.x);
	pev->effects = EF_MUZZLEFLASH;

	FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_2DEGREES, 1024, BULLET_MONSTER_9MM);

	int pitchShift = RANDOM_LONG(0, 20);

	// Only shift about half the time
	if (pitchShift > 10)
		pitchShift = 0;
	else
		pitchShift -= 5;
	EmitSound(SoundChannel::Weapon, "barney/ba_attack2.wav", VOL_NORM, ATTN_NORM, PITCH_NORM + pitchShift);

	CSoundEnt::InsertSound(bits_SOUND_COMBAT, GetAbsOrigin(), 384, 0.3);

	// UNDONE: Reload?
	m_cAmmoLoaded--;// take away a bullet!
}

void CBarney::HandleAnimEvent(AnimationEvent& event)
{
	switch (event.event)
	{
	case BARNEY_AE_SHOOT:
		BarneyFirePistol();
		break;

	case BARNEY_AE_DRAW:
		// barney's bodygroup switches here so he can pull gun from holster
		SetBodygroup(BARNEY_BODYGROUP_GUN, BARNEY_BODY_GUNDRAWN);
		m_fGunDrawn = true;
		break;

	case BARNEY_AE_HOLSTER:
		// change bodygroup to replace gun in holster
		SetBodygroup(BARNEY_BODYGROUP_GUN, BARNEY_BODY_GUNHOLSTERED);
		m_fGunDrawn = false;
		break;

	default:
		CTalkMonster::HandleAnimEvent(event);
	}
}

void CBarney::Spawn()
{
	Precache();

	SetModel("models/barney.mdl");
	SetSize(VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	SetSolidType(Solid::SlideBox);
	SetMovetype(Movetype::Step);
	m_bloodColor = BLOOD_COLOR_RED;
	pev->health = gSkillData.barneyHealth;
	pev->view_ofs = Vector(0, 0, 50);// position of the eyes relative to monster's origin.
	m_flFieldOfView = VIEW_FIELD_WIDE; // NOTE: we need a wide field of view so npc will notice player and say hello
	m_MonsterState = NPCState::None;

	SetBodygroup(BARNEY_BODYGROUP_GUN, BARNEY_BODY_GUNHOLSTERED);
	m_fGunDrawn = false;

	m_afCapability = bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP;

	MonsterInit();
	SetUse(&CBarney::FollowerUse);
}

void CBarney::Precache()
{
	PRECACHE_MODEL("models/barney.mdl");

	PRECACHE_SOUND("barney/ba_attack1.wav");
	PRECACHE_SOUND("barney/ba_attack2.wav");

	PRECACHE_SOUND("barney/ba_pain1.wav");
	PRECACHE_SOUND("barney/ba_pain2.wav");
	PRECACHE_SOUND("barney/ba_pain3.wav");

	PRECACHE_SOUND("barney/ba_die1.wav");
	PRECACHE_SOUND("barney/ba_die2.wav");
	PRECACHE_SOUND("barney/ba_die3.wav");

	// every new barney must call this, otherwise
	// when a level is loaded, nobody will talk (time is reset to 0)
	TalkInit();
	CTalkMonster::Precache();
}

void CBarney::TalkInit()
{
	CTalkMonster::TalkInit();

	// scientists speach group names (group names are in sentences.txt)

	m_szGrp[TLK_ANSWER] = "BA_ANSWER";
	m_szGrp[TLK_QUESTION] = "BA_QUESTION";
	m_szGrp[TLK_IDLE] = "BA_IDLE";
	m_szGrp[TLK_STARE] = "BA_STARE";
	m_szGrp[TLK_USE] = "BA_OK";
	m_szGrp[TLK_UNUSE] = "BA_WAIT";
	m_szGrp[TLK_STOP] = "BA_STOP";

	m_szGrp[TLK_NOSHOOT] = "BA_SCARED";
	m_szGrp[TLK_HELLO] = "BA_HELLO";

	m_szGrp[TLK_PLHURT1] = "!BA_CUREA";
	m_szGrp[TLK_PLHURT2] = "!BA_CUREB";
	m_szGrp[TLK_PLHURT3] = "!BA_CUREC";

	m_szGrp[TLK_PHELLO] = nullptr;	//"BA_PHELLO";		// UNDONE
	m_szGrp[TLK_PIDLE] = nullptr;	//"BA_PIDLE";			// UNDONE
	m_szGrp[TLK_PQUESTION] = "BA_PQUEST";		// UNDONE

	m_szGrp[TLK_SMELL] = "BA_SMELL";

	m_szGrp[TLK_WOUND] = "BA_WOUND";
	m_szGrp[TLK_MORTAL] = "BA_MORTAL";

	// get voice for head - just one barney voice for now
	m_voicePitch = 100;
}

bool IsFacing(CBaseEntity* pTest, const Vector& reference)
{
	Vector vecDir = reference - pTest->GetAbsOrigin();
	vecDir.z = 0;
	vecDir = vecDir.Normalize();
	Vector angle = pTest->pev->v_angle;
	angle.x = 0;
	Vector forward;
	AngleVectors(angle, &forward, nullptr, nullptr);
	// He's facing me, he meant it
	return DotProduct(forward, vecDir) > 0.96; // +/- 15 degrees or so
}

bool CBarney::TakeDamage(const TakeDamageInfo& info)
{
	// make sure friends talk about it if player hurts talkmonsters...
	const bool ret = CTalkMonster::TakeDamage(info);
	if (!IsAlive() || pev->deadflag == DeadFlag::Dying)
		return ret;

	if (m_MonsterState != NPCState::Prone && (info.GetAttacker()->pev->flags & FL_CLIENT))
	{
		m_flPlayerDamage += info.GetDamage();

		// This is a heurstic to determine if the player intended to harm me
		// If I have an enemy, we can't establish intent (may just be crossfire)
		if (m_hEnemy == nullptr)
		{
			// If the player was facing directly at me, or I'm already suspicious, get mad
			if ((m_afMemory & bits_MEMORY_SUSPICIOUS) || IsFacing(info.GetAttacker(), GetAbsOrigin()))
			{
				// Alright, now I'm pissed!
				PlaySentence("BA_MAD", 4, VOL_NORM, ATTN_NORM);

				Remember(bits_MEMORY_PROVOKED);
				StopFollowing(true);
			}
			else
			{
				// Hey, be careful with that
				PlaySentence("BA_SHOT", 4, VOL_NORM, ATTN_NORM);
				Remember(bits_MEMORY_SUSPICIOUS);
			}
		}
		else if (!(m_hEnemy->IsPlayer()) && pev->deadflag == DeadFlag::No)
		{
			PlaySentence("BA_SHOT", 4, VOL_NORM, ATTN_NORM);
		}
	}

	return ret;
}

void CBarney::PainSound()
{
	if (gpGlobals->time < m_painTime)
		return;

	m_painTime = gpGlobals->time + RANDOM_FLOAT(0.5, 0.75);

	switch (RANDOM_LONG(0, 2))
	{
	case 0: EmitSound(SoundChannel::Voice, "barney/ba_pain1.wav", VOL_NORM, ATTN_NORM, GetVoicePitch()); break;
	case 1: EmitSound(SoundChannel::Voice, "barney/ba_pain2.wav", VOL_NORM, ATTN_NORM, GetVoicePitch()); break;
	case 2: EmitSound(SoundChannel::Voice, "barney/ba_pain3.wav", VOL_NORM, ATTN_NORM, GetVoicePitch()); break;
	}
}

void CBarney::DeathSound()
{
	switch (RANDOM_LONG(0, 2))
	{
	case 0: EmitSound(SoundChannel::Voice, "barney/ba_die1.wav", VOL_NORM, ATTN_NORM, GetVoicePitch()); break;
	case 1: EmitSound(SoundChannel::Voice, "barney/ba_die2.wav", VOL_NORM, ATTN_NORM, GetVoicePitch()); break;
	case 2: EmitSound(SoundChannel::Voice, "barney/ba_die3.wav", VOL_NORM, ATTN_NORM, GetVoicePitch()); break;
	}
}

void CBarney::TraceAttack(const TraceAttackInfo& info)
{
	TraceAttackInfo adjustedInfo = info;

	switch (info.GetTraceResult().iHitgroup)
	{
	case HITGROUP_CHEST:
	case HITGROUP_STOMACH:
		if (adjustedInfo.GetDamageTypes() & (DMG_BULLET | DMG_SLASH | DMG_BLAST))
		{
			adjustedInfo.SetDamage(adjustedInfo.GetDamage() / 2);
		}
		break;
	case 10:
		if (adjustedInfo.GetDamageTypes() & (DMG_BULLET | DMG_SLASH | DMG_CLUB))
		{
			adjustedInfo.SetDamage(adjustedInfo.GetDamage() - 20);
			if (adjustedInfo.GetDamage() <= 0)
			{
				UTIL_Ricochet(adjustedInfo.GetTraceResult().vecEndPos, 1.0);
				adjustedInfo.SetDamage(0.01f);
			}
		}
		// always a head shot
		adjustedInfo.GetTraceResult().iHitgroup = HITGROUP_HEAD;
		break;
	}

	CTalkMonster::TraceAttack(adjustedInfo);
}

void CBarney::Killed(const KilledInfo& info)
{
	if (GetBodygroup(BARNEY_BODYGROUP_GUN) != BARNEY_BODY_GUNGONE)
	{// drop the gun!
		SetBodygroup(BARNEY_BODYGROUP_GUN, BARNEY_BODY_GUNGONE);

		Vector vecGunPos, vecGunAngles;
		GetAttachment(0, vecGunPos, vecGunAngles);

		CBaseEntity* pGun = DropItem("weapon_9mmhandgun", vecGunPos, vecGunAngles);
	}

	SetUse(nullptr);
	CTalkMonster::Killed(info);
}

Schedule_t* CBarney::GetScheduleOfType(int Type)
{
	Schedule_t* psched;

	switch (Type)
	{
	case SCHED_ARM_WEAPON:
		if (m_hEnemy != nullptr)
		{
			// face enemy, then draw.
			return slBarneyEnemyDraw;
		}
		break;

		// Hook these to make a looping schedule
	case SCHED_TARGET_FACE:
		// call base class default so that barney will talk
		// when 'used' 
		psched = CTalkMonster::GetScheduleOfType(Type);

		if (psched == slIdleStand)
			return slBaFaceTarget;	// override this for different target face behavior
		else
			return psched;

	case SCHED_TARGET_CHASE:
		return slBaFollow;

	case SCHED_IDLE_STAND:
		// call base class default so that scientist will talk
		// when standing during idle
		psched = CTalkMonster::GetScheduleOfType(Type);

		if (psched == slIdleStand)
		{
			// just look straight ahead.
			return slIdleBaStand;
		}
		else
			return psched;
	}

	return CTalkMonster::GetScheduleOfType(Type);
}

Schedule_t* CBarney::GetSchedule()
{
	if (HasConditions(bits_COND_HEAR_SOUND))
	{
		CSound* pSound = BestSound();

		ASSERT(pSound != nullptr);
		if (pSound && (pSound->m_iType & bits_SOUND_DANGER))
			return GetScheduleOfType(SCHED_TAKE_COVER_FROM_BEST_SOUND);
	}
	if (HasConditions(bits_COND_ENEMY_DEAD) && OkToSpeak())
	{
		PlaySentence("BA_KILL", 4, VOL_NORM, ATTN_NORM);
	}

	switch (m_MonsterState)
	{
	case NPCState::Combat:
	{
		// dead enemy
		if (HasConditions(bits_COND_ENEMY_DEAD))
		{
			// call base class, all code to handle dead enemies is centralized there.
			return CBaseMonster::GetSchedule();
		}

		// always act surprized with a new enemy
		if (HasConditions(bits_COND_NEW_ENEMY) && HasConditions(bits_COND_LIGHT_DAMAGE))
			return GetScheduleOfType(SCHED_SMALL_FLINCH);

		// wait for one schedule to draw gun
		if (!m_fGunDrawn)
			return GetScheduleOfType(SCHED_ARM_WEAPON);

		if (HasConditions(bits_COND_HEAVY_DAMAGE))
			return GetScheduleOfType(SCHED_TAKE_COVER_FROM_ENEMY);
	}
	break;

	case NPCState::Alert:
	case NPCState::Idle:
		if (HasConditions(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE))
		{
			// flinch if hurt
			return GetScheduleOfType(SCHED_SMALL_FLINCH);
		}

		if (m_hEnemy == nullptr && IsFollowing())
		{
			if (!m_hTargetEnt->IsAlive())
			{
				// UNDONE: Comment about the recently dead player here?
				StopFollowing(false);
				break;
			}
			else
			{
				if (HasConditions(bits_COND_CLIENT_PUSH))
				{
					return GetScheduleOfType(SCHED_MOVE_AWAY_FOLLOW);
				}
				return GetScheduleOfType(SCHED_TARGET_FACE);
			}
		}

		if (HasConditions(bits_COND_CLIENT_PUSH))
		{
			return GetScheduleOfType(SCHED_MOVE_AWAY);
		}

		// try to say something about smells
		TrySmellTalk();
		break;
	}

	return CTalkMonster::GetSchedule();
}

NPCState CBarney::GetIdealState()
{
	return CTalkMonster::GetIdealState();
}

void CBarney::DeclineFollowing()
{
	PlaySentence("BA_POK", 2, VOL_NORM, ATTN_NORM);
}
