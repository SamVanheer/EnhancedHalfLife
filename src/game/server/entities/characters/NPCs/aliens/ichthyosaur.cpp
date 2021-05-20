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

#include "CFlyingMonster.hpp"
#include "animation.h"

constexpr int SEARCH_RETRY = 16;

constexpr int ICHTHYOSAUR_SPEED = 150;

constexpr int EYE_MAD = 0;
constexpr int EYE_BASE = 1;
constexpr int EYE_CLOSED = 2;
constexpr int EYE_BACK = 3;
constexpr int EYE_LOOK = 4;

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
constexpr int ICHTHYOSAUR_AE_SHAKE_RIGHT = 1;
constexpr int ICHTHYOSAUR_AE_SHAKE_LEFT = 2;

/**
*	@brief evil, satan fish monster
*/
class CIchthyosaur : public CFlyingMonster
{
public:
	void  Spawn() override;
	void  Precache() override;
	void  SetYawSpeed() override;
	int   Classify() override;
	void  HandleAnimEvent(AnimationEvent& event) override;
	CUSTOM_SCHEDULES;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	Schedule_t* GetSchedule() override;
	Schedule_t* GetScheduleOfType(int Type) override;

	void Killed(const KilledInfo& info) override;
	void BecomeDead() override;

	void EXPORT CombatUse(const UseInfo& info);
	void EXPORT BiteTouch(CBaseEntity* pOther);

	void  StartTask(Task_t* pTask) override;
	void  RunTask(Task_t* pTask) override;

	bool  CheckMeleeAttack1(float flDot, float flDist) override;

	/**
	*	@brief swim in for a chomp
	*/
	bool  CheckRangeAttack1(float flDot, float flDist) override;

	float ChangeYaw(int speed) override;
	Activity GetStoppedActivity() override;

	void  Move(float flInterval) override;
	void  MoveExecute(CBaseEntity* pTargetEnt, const Vector& vecDir, float flInterval) override;
	void  MonsterThink() override;
	void  Stop() override;
	void  Swim();
	Vector DoProbe(const Vector& Probe);

	float VectorToPitch(const Vector& vec);
	float PitchDiff();
	float ChangePitch(int speed);

	Vector m_SaveVelocity;
	float m_idealDist = 0;

	float m_flBlink = 0;

	float m_flEnemyTouched = 0;
	bool  m_bOnAttack = false;

	float m_flMaxSpeed = 0;
	float m_flMinSpeed = 0;
	float m_flMaxDist = 0;

	EHandle<CBeam> m_hBeam;

	float m_flNextAlert = 0;

	float m_flLastPitchTime = 0;

	static const char* pIdleSounds[];
	static const char* pAlertSounds[];
	static const char* pAttackSounds[];
	static const char* pBiteSounds[];
	static const char* pDieSounds[];
	static const char* pPainSounds[];

	void IdleSound() override;
	void AlertSound() override;
	void AttackSound();
	void BiteSound();
	void DeathSound() override;
	void PainSound() override;

	template<std::size_t Size>
	void EMIT_ICKY_SOUND(SoundChannel chan, const char* (&array)[Size])
	{
		EmitSound(chan, array[RANDOM_LONG(0, ArraySize(array) - 1)], VOL_NORM, 0.6, RANDOM_LONG(95, 105));
	}
};

LINK_ENTITY_TO_CLASS(monster_ichthyosaur, CIchthyosaur);

TYPEDESCRIPTION	CIchthyosaur::m_SaveData[] =
{
	DEFINE_FIELD(CIchthyosaur, m_SaveVelocity, FIELD_VECTOR),
	DEFINE_FIELD(CIchthyosaur, m_idealDist, FIELD_FLOAT),
	DEFINE_FIELD(CIchthyosaur, m_flBlink, FIELD_FLOAT),
	DEFINE_FIELD(CIchthyosaur, m_flEnemyTouched, FIELD_FLOAT),
	DEFINE_FIELD(CIchthyosaur, m_bOnAttack, FIELD_BOOLEAN),
	DEFINE_FIELD(CIchthyosaur, m_flMaxSpeed, FIELD_FLOAT),
	DEFINE_FIELD(CIchthyosaur, m_flMinSpeed, FIELD_FLOAT),
	DEFINE_FIELD(CIchthyosaur, m_flMaxDist, FIELD_FLOAT),
	DEFINE_FIELD(CIchthyosaur, m_flNextAlert, FIELD_TIME),
};

IMPLEMENT_SAVERESTORE(CIchthyosaur, CFlyingMonster);

const char* CIchthyosaur::pIdleSounds[] =
{
	"ichy/ichy_idle1.wav",
	"ichy/ichy_idle2.wav",
	"ichy/ichy_idle3.wav",
	"ichy/ichy_idle4.wav",
};

const char* CIchthyosaur::pAlertSounds[] =
{
	"ichy/ichy_alert2.wav",
	"ichy/ichy_alert3.wav",
};

const char* CIchthyosaur::pAttackSounds[] =
{
	"ichy/ichy_attack1.wav",
	"ichy/ichy_attack2.wav",
};

const char* CIchthyosaur::pBiteSounds[] =
{
	"ichy/ichy_bite1.wav",
	"ichy/ichy_bite2.wav",
};

const char* CIchthyosaur::pPainSounds[] =
{
	"ichy/ichy_pain2.wav",
	"ichy/ichy_pain3.wav",
	"ichy/ichy_pain5.wav",
};

const char* CIchthyosaur::pDieSounds[] =
{
	"ichy/ichy_die2.wav",
	"ichy/ichy_die4.wav",
};

void CIchthyosaur::IdleSound()
{
	EMIT_ICKY_SOUND(SoundChannel::Voice, pIdleSounds);
}

void CIchthyosaur::AlertSound()
{
	EMIT_ICKY_SOUND(SoundChannel::Voice, pAlertSounds);
}

void CIchthyosaur::AttackSound()
{
	EMIT_ICKY_SOUND(SoundChannel::Voice, pAttackSounds);
}

void CIchthyosaur::BiteSound()
{
	EMIT_ICKY_SOUND(SoundChannel::Weapon, pBiteSounds);
}

void CIchthyosaur::DeathSound()
{
	EMIT_ICKY_SOUND(SoundChannel::Voice, pDieSounds);
}

void CIchthyosaur::PainSound()
{
	EMIT_ICKY_SOUND(SoundChannel::Voice, pPainSounds);
}

//=========================================================
// monster-specific tasks and states
//=========================================================
enum
{
	TASK_ICHTHYOSAUR_CIRCLE_ENEMY = LAST_COMMON_TASK + 1,
	TASK_ICHTHYOSAUR_SWIM,
	TASK_ICHTHYOSAUR_FLOAT,
};

//=========================================================
// AI Schedules Specific to this monster
//=========================================================
static Task_t	tlSwimAround[] =
{
	{ TASK_SET_ACTIVITY,			(float)ACT_WALK },
	{ TASK_ICHTHYOSAUR_SWIM,		0.0 },
};

static Schedule_t	slSwimAround[] =
{
	{
		tlSwimAround,
		ArraySize(tlSwimAround),
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_SEE_ENEMY |
		bits_COND_NEW_ENEMY |
		bits_COND_HEAR_SOUND,
		bits_SOUND_PLAYER |
		bits_SOUND_COMBAT,
		"SwimAround"
	},
};

static Task_t	tlSwimAgitated[] =
{
	{ TASK_STOP_MOVING,				(float)0 },
	{ TASK_SET_ACTIVITY,			(float)ACT_RUN },
	{ TASK_WAIT,					(float)2.0 },
};

static Schedule_t	slSwimAgitated[] =
{
	{
		tlSwimAgitated,
		ArraySize(tlSwimAgitated),
		0,
		0,
		"SwimAgitated"
	},
};

static Task_t	tlCircleEnemy[] =
{
	{ TASK_SET_ACTIVITY,			(float)ACT_WALK },
	{ TASK_ICHTHYOSAUR_CIRCLE_ENEMY, 0.0 },
};

static Schedule_t	slCircleEnemy[] =
{
	{
		tlCircleEnemy,
		ArraySize(tlCircleEnemy),
		bits_COND_NEW_ENEMY |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_CAN_MELEE_ATTACK1 |
		bits_COND_CAN_RANGE_ATTACK1,
		0,
		"CircleEnemy"
	},
};

Task_t tlTwitchDie[] =
{
	{ TASK_STOP_MOVING,			0		 },
	{ TASK_SOUND_DIE,			(float)0 },
	{ TASK_DIE,					(float)0 },
	{ TASK_ICHTHYOSAUR_FLOAT,	(float)0 },
};

Schedule_t slTwitchDie[] =
{
	{
		tlTwitchDie,
		ArraySize(tlTwitchDie),
		0,
		0,
		"Die"
	},
};

DEFINE_CUSTOM_SCHEDULES(CIchthyosaur)
{
	slSwimAround,
		slSwimAgitated,
		slCircleEnemy,
		slTwitchDie,
};
IMPLEMENT_CUSTOM_SCHEDULES(CIchthyosaur, CFlyingMonster);

int	CIchthyosaur::Classify()
{
	return	CLASS_ALIEN_MONSTER;
}

bool CIchthyosaur::CheckMeleeAttack1(float flDot, float flDist)
{
	if (flDot >= 0.7 && m_flEnemyTouched > gpGlobals->time - 0.2)
	{
		return true;
	}
	return false;
}

void CIchthyosaur::BiteTouch(CBaseEntity* pOther)
{
	// bite if we hit who we want to eat
	if (pOther == m_hEnemy)
	{
		m_flEnemyTouched = gpGlobals->time;
		m_bOnAttack = true;
	}
}

void CIchthyosaur::CombatUse(const UseInfo& info)
{
	if (!ShouldToggle(info.GetUseType(), m_bOnAttack))
		return;

	m_bOnAttack = !m_bOnAttack;
}

bool CIchthyosaur::CheckRangeAttack1(float flDot, float flDist)
{
	if (flDot > -0.7 && (m_bOnAttack || (flDist <= 192 && m_idealDist <= 192)))
	{
		return true;
	}

	return false;
}

void CIchthyosaur::SetYawSpeed()
{
	pev->yaw_speed = 100;
}

void CIchthyosaur::Killed(const KilledInfo& info)
{
	CBaseMonster::Killed(info);
	SetAbsVelocity(vec3_origin);
}

void CIchthyosaur::BecomeDead()
{
	SetDamageMode(DamageMode::Yes);// don't let autoaim aim at corpses.

	// give the corpse half of the monster's original maximum health. 
	pev->health = pev->max_health / 2;
	pev->max_health = 5; // max_health now becomes a counter for how many blood decals the corpse can place.
}

void CIchthyosaur::HandleAnimEvent(AnimationEvent& event)
{
	bool bDidAttack = false;
	switch (event.event)
	{
	case ICHTHYOSAUR_AE_SHAKE_RIGHT:
	case ICHTHYOSAUR_AE_SHAKE_LEFT:
	{
		if (m_hEnemy != nullptr && IsVisible(m_hEnemy))
		{
			CBaseEntity* pHurt = m_hEnemy;

			if (m_flEnemyTouched < gpGlobals->time - 0.2 && (m_hEnemy->BodyTarget(GetAbsOrigin()) - GetAbsOrigin()).Length() >(32 + 16 + 32))
				break;

			const Vector vecShootDir = ShootAtEnemy(GetAbsOrigin());
			UTIL_MakeAimVectors(GetAbsAngles());

			if (DotProduct(vecShootDir, gpGlobals->v_forward) > 0.707)
			{
				m_bOnAttack = true;
				pHurt->pev->punchangle.z = -18;
				pHurt->pev->punchangle.x = 5;
				pHurt->SetAbsVelocity(pHurt->GetAbsVelocity() - gpGlobals->v_right * 300);
				if (pHurt->IsPlayer())
				{
					Vector angles = pHurt->GetAbsAngles();
					angles.x += RANDOM_FLOAT(-35, 35);
					angles.y += RANDOM_FLOAT(-90, 90);
					angles.z = 0;
					pHurt->SetAbsAngles(angles);

					pHurt->pev->fixangle = FixAngleMode::Absolute;
				}
				pHurt->TakeDamage({this, this, gSkillData.ichthyosaurDmgShake, DMG_SLASH});
			}
		}
		BiteSound();

		bDidAttack = true;
	}
	break;
	default:
		CFlyingMonster::HandleAnimEvent(event);
		break;
	}

	if (bDidAttack)
	{
		const Vector vecSrc = GetAbsOrigin() + gpGlobals->v_forward * 32;
		UTIL_Bubbles(vecSrc - Vector(8, 8, 8), vecSrc + Vector(8, 8, 8), 16);
	}
}

void CIchthyosaur::Spawn()
{
	Precache();

	SetModel("models/icky.mdl");
	SetSize(Vector(-32, -32, -32), Vector(32, 32, 32));

	SetSolidType(Solid::BBox);
	SetMovetype(Movetype::Fly);
	m_bloodColor = BLOOD_COLOR_GREEN;
	pev->health = gSkillData.ichthyosaurHealth;
	pev->view_ofs = Vector(0, 0, 16);
	m_flFieldOfView = VIEW_FIELD_WIDE;
	m_MonsterState = NPCState::None;
	SetBits(pev->flags, FL_SWIM);
	SetFlyingSpeed(ICHTHYOSAUR_SPEED);
	SetFlyingMomentum(2.5);	// Set momentum constant

	m_afCapability = bits_CAP_RANGE_ATTACK1 | bits_CAP_SWIM;

	MonsterInit();

	SetTouch(&CIchthyosaur::BiteTouch);
	SetUse(&CIchthyosaur::CombatUse);

	m_idealDist = 384;
	m_flMinSpeed = 80;
	m_flMaxSpeed = 300;
	m_flMaxDist = 384;

	Vector Forward;
	AngleVectors(GetAbsAngles(), &Forward, nullptr, nullptr);
	SetAbsVelocity(m_flightSpeed * Forward.Normalize());
	m_SaveVelocity = GetAbsVelocity();
}

void CIchthyosaur::Precache()
{
	PRECACHE_MODEL("models/icky.mdl");

	PRECACHE_SOUND_ARRAY(pIdleSounds);
	PRECACHE_SOUND_ARRAY(pAlertSounds);
	PRECACHE_SOUND_ARRAY(pAttackSounds);
	PRECACHE_SOUND_ARRAY(pBiteSounds);
	PRECACHE_SOUND_ARRAY(pDieSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
}

Schedule_t* CIchthyosaur::GetSchedule()
{
	// ALERT( at_console, "GetSchedule( )\n" );
	switch (m_MonsterState)
	{
	case NPCState::Idle:
		m_flightSpeed = 80;
		return GetScheduleOfType(SCHED_IDLE_WALK);

	case NPCState::Alert:
		m_flightSpeed = 150;
		return GetScheduleOfType(SCHED_IDLE_WALK);

	case NPCState::Combat:
		m_flMaxSpeed = 400;
		// eat them
		if (HasConditions(bits_COND_CAN_MELEE_ATTACK1))
		{
			return GetScheduleOfType(SCHED_MELEE_ATTACK1);
		}
		// chase them down and eat them
		if (HasConditions(bits_COND_CAN_RANGE_ATTACK1))
		{
			return GetScheduleOfType(SCHED_CHASE_ENEMY);
		}
		if (HasConditions(bits_COND_HEAVY_DAMAGE))
		{
			m_bOnAttack = true;
		}
		if (pev->health < pev->max_health - 20)
		{
			m_bOnAttack = true;
		}

		return GetScheduleOfType(SCHED_STANDOFF);
	}

	return CFlyingMonster::GetSchedule();
}

Schedule_t* CIchthyosaur::GetScheduleOfType(int Type)
{
	// ALERT( at_console, "GetScheduleOfType( %d ) %d\n", Type, m_bOnAttack );
	switch (Type)
	{
	case SCHED_IDLE_WALK:
		return slSwimAround;
	case SCHED_STANDOFF:
		return slCircleEnemy;
	case SCHED_FAIL:
		return slSwimAgitated;
	case SCHED_DIE:
		return slTwitchDie;
	case SCHED_CHASE_ENEMY:
		AttackSound();
	}

	return CBaseMonster::GetScheduleOfType(Type);
}

void CIchthyosaur::StartTask(Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_ICHTHYOSAUR_CIRCLE_ENEMY:
		break;
	case TASK_ICHTHYOSAUR_SWIM:
		break;
	case TASK_SMALL_FLINCH:
		if (m_idealDist > 128)
		{
			m_flMaxDist = 512;
			m_idealDist = 512;
		}
		else
		{
			m_bOnAttack = true;
		}
		CFlyingMonster::StartTask(pTask);
		break;

	case TASK_ICHTHYOSAUR_FLOAT:
		pev->skin = EYE_BASE;
		SetSequenceByName("bellyup");
		break;

	default:
		CFlyingMonster::StartTask(pTask);
		break;
	}
}

void CIchthyosaur::RunTask(Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_ICHTHYOSAUR_CIRCLE_ENEMY:
		if (m_hEnemy == nullptr)
		{
			TaskComplete();
		}
		else if (IsVisible(m_hEnemy))
		{
			const Vector vecFrom = m_hEnemy->EyePosition();

			const Vector vecDelta = (GetAbsOrigin() - vecFrom).Normalize();
			Vector vecSwim = CrossProduct(vecDelta, vec3_up).Normalize();

			if (DotProduct(vecSwim, m_SaveVelocity) < 0)
				vecSwim = vecSwim * -1.0;

			Vector vecPos = vecFrom + vecDelta * m_idealDist + vecSwim * 32;

			// ALERT( at_console, "vecPos %.0f %.0f %.0f\n", vecPos.x, vecPos.y, vecPos.z );

			TraceResult tr;

			UTIL_TraceHull(vecFrom, vecPos, IgnoreMonsters::Yes, Hull::Large, m_hEnemy, &tr);

			if (tr.flFraction > 0.5)
				vecPos = tr.vecEndPos;

			m_SaveVelocity = m_SaveVelocity * 0.8 + 0.2 * (vecPos - GetAbsOrigin()).Normalize() * m_flightSpeed;

			// ALERT( at_console, "m_SaveVelocity %.2f %.2f %.2f\n", m_SaveVelocity.x, m_SaveVelocity.y, m_SaveVelocity.z );

			if (HasConditions(bits_COND_ENEMY_FACING_ME) && m_hEnemy->IsVisible(this))
			{
				m_flNextAlert -= 0.1;

				if (m_idealDist < m_flMaxDist)
				{
					m_idealDist += 4;
				}
				if (m_flightSpeed > m_flMinSpeed)
				{
					m_flightSpeed -= 2;
				}
				else if (m_flightSpeed < m_flMinSpeed)
				{
					m_flightSpeed += 2;
				}
				if (m_flMinSpeed < m_flMaxSpeed)
				{
					m_flMinSpeed += 0.5;
				}
			}
			else
			{
				m_flNextAlert += 0.1;

				if (m_idealDist > 128)
				{
					m_idealDist -= 4;
				}
				if (m_flightSpeed < m_flMaxSpeed)
				{
					m_flightSpeed += 4;
				}
			}
			// ALERT( at_console, "%.0f\n", m_idealDist );
		}
		else
		{
			m_flNextAlert = gpGlobals->time + 0.2;
		}

		if (m_flNextAlert < gpGlobals->time)
		{
			// ALERT( at_console, "AlertSound()\n");
			AlertSound();
			m_flNextAlert = gpGlobals->time + RANDOM_FLOAT(3, 5);
		}

		break;
	case TASK_ICHTHYOSAUR_SWIM:
		if (m_fSequenceFinished)
		{
			TaskComplete();
		}
		break;
	case TASK_DIE:
		if (m_fSequenceFinished)
		{
			pev->deadflag = DeadFlag::Dead;

			TaskComplete();
		}
		break;

	case TASK_ICHTHYOSAUR_FLOAT:
	{
		Vector angles = GetAbsAngles();
		angles.x = UTIL_ApproachAngle(0, angles.x, 20);
		SetAbsAngles(angles);

		Vector newVelocity = GetAbsVelocity() * 0.8;
		if (pev->waterlevel > WaterLevel::Feet && newVelocity.z < 64)
		{
			newVelocity.z += 8;
		}
		else
		{
			newVelocity.z -= 8;
		}
		SetAbsVelocity(newVelocity);
		// ALERT( at_console, "%f\n", newVelocity.z );
	}
	break;

	default:
		CFlyingMonster::RunTask(pTask);
		break;
	}
}

float CIchthyosaur::VectorToPitch(const Vector& vec)
{
	//TODO: should be a helper function like for yaw
	float pitch;
	if (vec.z == 0 && vec.x == 0)
		pitch = 0;
	else
	{
		pitch = (int)(atan2(vec.z, sqrt(vec.x * vec.x + vec.y * vec.y)) * 180.0 / M_PI);
		if (pitch < 0)
			pitch += 360;
	}
	return pitch;
}

void CIchthyosaur::Move(float flInterval)
{
	CFlyingMonster::Move(flInterval);
}

float CIchthyosaur::PitchDiff()
{
	const float flCurrentPitch = UTIL_AngleMod(GetAbsAngles().z);

	if (flCurrentPitch == pev->idealpitch)
	{
		return 0;
	}

	float flPitchDiff = pev->idealpitch - flCurrentPitch;

	if (pev->idealpitch > flCurrentPitch)
	{
		if (flPitchDiff >= 180)
			flPitchDiff = flPitchDiff - 360;
	}
	else
	{
		if (flPitchDiff <= -180)
			flPitchDiff = flPitchDiff + 360;
	}
	return flPitchDiff;
}

float CIchthyosaur::ChangePitch(int speed)
{
	if (GetMovetype() == Movetype::Fly)
	{
		const float diff = PitchDiff();
		float target = 0;
		if (m_IdealActivity != GetStoppedActivity())
		{
			if (diff < -20)
				target = 45;
			else if (diff > 20)
				target = -45;
		}

		if (m_flLastPitchTime == 0)
		{
			m_flLastPitchTime = gpGlobals->time - gpGlobals->frametime;
		}

		const float delta = std::min(0.25f, gpGlobals->time - m_flLastPitchTime);

		m_flLastPitchTime = gpGlobals->time;

		Vector angles = GetAbsAngles();
		angles.x = UTIL_Approach(target, angles.x, 220.0 * delta);
		SetAbsAngles(angles);
	}
	return 0;
}

float CIchthyosaur::ChangeYaw(int speed)
{
	if (GetMovetype() == Movetype::Fly)
	{
		const float diff = YawDiff();
		float target = 0;

		if (m_IdealActivity != GetStoppedActivity())
		{
			if (diff < -20)
				target = 20;
			else if (diff > 20)
				target = -20;
		}

		if (m_flLastZYawTime == 0)
		{
			m_flLastZYawTime = gpGlobals->time - gpGlobals->frametime;
		}

		const float delta = std::min(0.25f, gpGlobals->time - m_flLastZYawTime);

		m_flLastZYawTime = gpGlobals->time;

		Vector angles = GetAbsAngles();
		angles.z = UTIL_Approach(target, angles.z, 220.0 * delta);
		SetAbsAngles(angles);
	}
	return CFlyingMonster::ChangeYaw(speed);
}

Activity CIchthyosaur::GetStoppedActivity()
{
	if (GetMovetype() != Movetype::Fly)		// UNDONE: Ground idle here, IDLE may be something else
		return ACT_IDLE;
	return ACT_WALK;
}

void CIchthyosaur::MoveExecute(CBaseEntity* pTargetEnt, const Vector& vecDir, float flInterval)
{
	m_SaveVelocity = vecDir * m_flightSpeed;
}

void CIchthyosaur::MonsterThink()
{
	CFlyingMonster::MonsterThink();

	if (pev->deadflag == DeadFlag::No)
	{
		if (m_MonsterState != NPCState::Script)
		{
			Swim();

			// blink the eye
			if (m_flBlink < gpGlobals->time)
			{
				pev->skin = EYE_CLOSED;
				if (m_flBlink + 0.2 < gpGlobals->time)
				{
					m_flBlink = gpGlobals->time + RANDOM_FLOAT(3, 4);
					if (m_bOnAttack)
						pev->skin = EYE_MAD;
					else
						pev->skin = EYE_BASE;
				}
			}
		}
	}
}

void CIchthyosaur::Stop()
{
	if (!m_bOnAttack)
		m_flightSpeed = 80.0;
}

void CIchthyosaur::Swim()
{
	const Vector start = GetAbsOrigin();

	if (IsBitSet(pev->flags, FL_ONGROUND))
	{
		Vector angles = GetAbsAngles();
		angles.x = 0;
		angles.y += RANDOM_FLOAT(-45, 45);
		SetAbsAngles(angles);
		ClearBits(pev->flags, FL_ONGROUND);

		const Vector Angles = Vector(-GetAbsAngles().x, GetAbsAngles().y, GetAbsAngles().z);

		Vector Forward, Up;
		AngleVectors(Angles, &Forward, nullptr, &Up);

		SetAbsVelocity(Forward * 200 + Up * 200);
		return;
	}

	if (m_bOnAttack && m_flightSpeed < m_flMaxSpeed)
	{
		m_flightSpeed += 40;
	}
	if (m_flightSpeed < 180)
	{
		if (m_IdealActivity == ACT_RUN)
			SetActivity(ACT_WALK);
		if (m_IdealActivity == ACT_WALK)
			pev->framerate = m_flightSpeed / 150.0;
		// ALERT( at_console, "walk %.2f\n", pev->framerate );
	}
	else
	{
		if (m_IdealActivity == ACT_WALK)
			SetActivity(ACT_RUN);
		if (m_IdealActivity == ACT_RUN)
			pev->framerate = m_flightSpeed / 150.0;
		// ALERT( at_console, "run  %.2f\n", pev->framerate );
	}

	/*
		if (!m_hBeam)
		{
			m_hBeam = CBeam::BeamCreate( "sprites/laserbeam.spr", 80 );
			m_hBeam->PointEntInit( GetAbsOrigin() + m_SaveVelocity, entindex( ) );
			m_hBeam->SetEndAttachment( 1 );
			m_hBeam->SetColor( 255, 180, 96 );
			m_hBeam->SetBrightness( 192 );
		}
	*/
	constexpr int PROBE_LENGTH = 150;
	Vector Angles = VectorAngles(m_SaveVelocity);
	Angles.x = -Angles.x;
	Vector Forward, Right, Up;
	AngleVectors(Angles, Forward, Right, Up);

	const Vector f = DoProbe(start + PROBE_LENGTH * Forward);
	const Vector r = DoProbe(start + PROBE_LENGTH / 3 * Forward + Right);
	const Vector l = DoProbe(start + PROBE_LENGTH / 3 * Forward - Right);
	const Vector u = DoProbe(start + PROBE_LENGTH / 3 * Forward + Up);
	const Vector d = DoProbe(start + PROBE_LENGTH / 3 * Forward - Up);

	const Vector SteeringVector = f + r + l + u + d;
	m_SaveVelocity = (m_SaveVelocity + SteeringVector / 2).Normalize();

	Vector myAngles = GetAbsAngles();

	Angles = Vector(-myAngles.x, myAngles.y, myAngles.z);
	AngleVectors(Angles, &Forward, nullptr, nullptr);
	// ALERT( at_console, "%f : %f\n", Angles.x, Forward.z );

	const float flDot = DotProduct(Forward, m_SaveVelocity);
	if (flDot > 0.5)
		m_SaveVelocity = m_SaveVelocity * m_flightSpeed;
	else if (flDot > 0)
		m_SaveVelocity = m_SaveVelocity * m_flightSpeed * (flDot + 0.5);
	else
		m_SaveVelocity = m_SaveVelocity * 80;

	SetAbsVelocity(m_SaveVelocity);

	// ALERT( at_console, "%.0f %.0f\n", m_flightSpeed, GetAbsVelocity().Length() );


	// ALERT( at_console, "Steer %f %f %f\n", SteeringVector.x, SteeringVector.y, SteeringVector.z );

/*
	m_hBeam->SetStartPos(GetAbsOrigin() + GetAbsVelocity());
	m_hBeam->RelinkBeam();
*/

// ALERT( at_console, "speed %f\n", m_flightSpeed );

	Angles = VectorAngles(m_SaveVelocity);

	// Smooth Pitch
	//
	if (Angles.x > 180)
		Angles.x = Angles.x - 360;

	myAngles.x = UTIL_Approach(Angles.x, myAngles.x, 50 * 0.1);
	if (myAngles.x < -80) myAngles.x = -80;
	if (myAngles.x > 80) myAngles.x = 80;

	// Smooth Yaw and generate Roll
	//
	float turn = 360;
	// ALERT( at_console, "Y %.0f %.0f\n", Angles.y, myAngles.y );

	if (fabs(Angles.y - myAngles.y) < fabs(turn))
	{
		turn = Angles.y - myAngles.y;
	}
	if (fabs(Angles.y - myAngles.y + 360) < fabs(turn))
	{
		turn = Angles.y - myAngles.y + 360;
	}
	if (fabs(Angles.y - myAngles.y - 360) < fabs(turn))
	{
		turn = Angles.y - myAngles.y - 360;
	}

	float speed = m_flightSpeed * 0.1;

	// ALERT( at_console, "speed %.0f %f\n", turn, speed );
	if (fabs(turn) > speed)
	{
		if (turn < 0.0)
		{
			turn = -speed;
		}
		else
		{
			turn = speed;
		}
	}

	myAngles.y += turn;
	myAngles.z -= turn;
	myAngles.y = fmod((myAngles.y + 360.0), 360.0);

	//TODO: could break if multiple ichtys exist
	static float yaw_adj;

	yaw_adj = yaw_adj * 0.8 + turn;

	// ALERT( at_console, "yaw %f : %f\n", turn, yaw_adj );

	SetBoneController(0, -yaw_adj / 4.0);

	// Roll Smoothing
	//
	turn = 360;
	if (fabs(Angles.z - myAngles.z) < fabs(turn))
	{
		turn = Angles.z - myAngles.z;
	}
	if (fabs(Angles.z - myAngles.z + 360) < fabs(turn))
	{
		turn = Angles.z - myAngles.z + 360;
	}
	if (fabs(Angles.z - myAngles.z - 360) < fabs(turn))
	{
		turn = Angles.z - myAngles.z - 360;
	}
	speed = m_flightSpeed / 2 * 0.1;
	if (fabs(turn) < speed)
	{
		myAngles.z += turn;
	}
	else
	{
		if (turn < 0.0)
		{
			myAngles.z -= speed;
		}
		else
		{
			myAngles.z += speed;
		}
	}
	if (myAngles.z < -20) myAngles.z = -20;
	if (myAngles.z > 20) myAngles.z = 20;

	SetAbsAngles(myAngles);

	//AngleVectors(Vector(-Angles.x, Angles.y, Angles.z), &Forward, nullptr, nullptr);

	// UTIL_MoveToOrigin ( ENT(pev), GetAbsOrigin() + Forward * speed, speed, MoveToOriginType::Strafe );
}

Vector CIchthyosaur::DoProbe(const Vector& Probe)
{
	Vector WallNormal = vec3_down; // WATER normal is Straight Down for fish.
	float frac;
	bool bBumpedSomething = ProbeZ(GetAbsOrigin(), Probe, frac);

	TraceResult tr;
	UTIL_TraceMonsterHull(this, GetAbsOrigin(), Probe, IgnoreMonsters::No, this, &tr);
	if (tr.fAllSolid || tr.flFraction < 0.99)
	{
		if (tr.flFraction < 0.0) tr.flFraction = 0.0;
		if (tr.flFraction > 1.0) tr.flFraction = 1.0;
		if (tr.flFraction < frac)
		{
			frac = tr.flFraction;
			bBumpedSomething = true;
			WallNormal = tr.vecPlaneNormal;
		}
	}

	if (bBumpedSomething && (m_hEnemy == nullptr || InstanceOrNull(tr.pHit) != m_hEnemy))
	{
		const Vector ProbeDir = Probe - GetAbsOrigin();

		const Vector NormalToProbeAndWallNormal = CrossProduct(ProbeDir, WallNormal);
		Vector SteeringVector = CrossProduct(NormalToProbeAndWallNormal, ProbeDir);

		const float SteeringForce = std::abs(m_flightSpeed * (1 - frac) * DotProduct(WallNormal.Normalize(), m_SaveVelocity.Normalize()));

		SteeringVector = SteeringForce * SteeringVector.Normalize();

		return SteeringVector;
	}

	return vec3_origin;
}