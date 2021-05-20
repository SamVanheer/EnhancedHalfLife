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

#include "CHeadCrab.hpp"
#include "game.h"

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
constexpr int HC_AE_JUMPATTACK = 2;

Task_t	tlHCRangeAttack1[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_FACE_IDEAL,			(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE	},
	{ TASK_FACE_IDEAL,			(float)0		},
	{ TASK_WAIT_RANDOM,			(float)0.5		},
};

Schedule_t	slHCRangeAttack1[] =
{
	{
		tlHCRangeAttack1,
		ArraySize(tlHCRangeAttack1),
		bits_COND_ENEMY_OCCLUDED |
		bits_COND_NO_AMMO_LOADED,
		0,
		"HCRangeAttack1"
	},
};

Task_t	tlHCRangeAttack1Fast[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_FACE_IDEAL,			(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE	},
};

Schedule_t	slHCRangeAttack1Fast[] =
{
	{
		tlHCRangeAttack1Fast,
		ArraySize(tlHCRangeAttack1Fast),
		bits_COND_ENEMY_OCCLUDED |
		bits_COND_NO_AMMO_LOADED,
		0,
		"HCRAFast"
	},
};

LINK_ENTITY_TO_CLASS(monster_headcrab, CHeadCrab);

DEFINE_CUSTOM_SCHEDULES(CHeadCrab)
{
	slHCRangeAttack1,
		slHCRangeAttack1Fast,
};

IMPLEMENT_CUSTOM_SCHEDULES(CHeadCrab, CBaseMonster);

const char* CHeadCrab::pIdleSounds[] =
{
	"headcrab/hc_idle1.wav",
	"headcrab/hc_idle2.wav",
	"headcrab/hc_idle3.wav",
};
const char* CHeadCrab::pAlertSounds[] =
{
	"headcrab/hc_alert1.wav",
};
const char* CHeadCrab::pPainSounds[] =
{
	"headcrab/hc_pain1.wav",
	"headcrab/hc_pain2.wav",
	"headcrab/hc_pain3.wav",
};
const char* CHeadCrab::pAttackSounds[] =
{
	"headcrab/hc_attack1.wav",
	"headcrab/hc_attack2.wav",
	"headcrab/hc_attack3.wav",
};

const char* CHeadCrab::pDeathSounds[] =
{
	"headcrab/hc_die1.wav",
	"headcrab/hc_die2.wav",
};

const char* CHeadCrab::pBiteSounds[] =
{
	"headcrab/hc_headbite.wav",
};

int	CHeadCrab::Classify()
{
	return	CLASS_ALIEN_PREY;
}

Vector CHeadCrab::Center()
{
	return Vector(GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z + 6);
}

Vector CHeadCrab::BodyTarget(const Vector& posSrc)
{
	return Center();
}

void CHeadCrab::SetYawSpeed()
{
	int ys;

	switch (m_Activity)
	{
	case ACT_IDLE:
		ys = 30;
		break;
	case ACT_RUN:
	case ACT_WALK:
		ys = 20;
		break;
	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		ys = 60;
		break;
	case ACT_RANGE_ATTACK1:
		ys = 30;
		break;
	default:
		ys = 30;
		break;
	}

	pev->yaw_speed = ys;
}

void CHeadCrab::HandleAnimEvent(AnimationEvent& event)
{
	switch (event.event)
	{
	case HC_AE_JUMPATTACK:
	{
		ClearBits(pev->flags, FL_ONGROUND);

		SetAbsOrigin(GetAbsOrigin() + Vector(0, 0, 1));// take him off ground so engine doesn't instantly reset onground 
		UTIL_MakeVectors(GetAbsAngles());

		Vector vecJumpDir;
		if (m_hEnemy != nullptr)
		{
			const float gravity = std::max(1.0f, g_psv_gravity->value);

			// How fast does the headcrab need to travel to reach that height given gravity?
			const float height = std::max(16.0f, (m_hEnemy->GetAbsOrigin().z + m_hEnemy->pev->view_ofs.z - GetAbsOrigin().z));
			const float speed = sqrt(2 * gravity * height);
			const float time = speed / gravity;

			// Scale the sideways velocity to get there at the right time
			vecJumpDir = (m_hEnemy->GetAbsOrigin() + m_hEnemy->pev->view_ofs - GetAbsOrigin());
			vecJumpDir = vecJumpDir * (1.0 / time);

			// Speed to offset gravity at the desired height
			vecJumpDir.z = speed;

			// Don't jump too far/fast
			const float distance = vecJumpDir.Length();

			if (distance > 650)
			{
				vecJumpDir = vecJumpDir * (650.0 / distance);
			}
		}
		else
		{
			// jump hop, don't care where
			vecJumpDir = Vector(gpGlobals->v_forward.x, gpGlobals->v_forward.y, gpGlobals->v_up.z) * 350;
		}

		const int iSound = RANDOM_LONG(0, 2);
		if (iSound != 0)
			EmitSound(SoundChannel::Voice, pAttackSounds[iSound], GetSoundVolue(), ATTN_IDLE, GetVoicePitch());

		SetAbsVelocity(vecJumpDir);
		m_flNextAttack = gpGlobals->time + 2;
	}
	break;

	default:
		CBaseMonster::HandleAnimEvent(event);
		break;
	}
}

void CHeadCrab::Spawn()
{
	Precache();

	SetModel("models/headcrab.mdl");
	SetSize(Vector(-12, -12, 0), Vector(12, 12, 24));

	SetSolidType(Solid::SlideBox);
	SetMovetype(Movetype::Step);
	m_bloodColor = BLOOD_COLOR_GREEN;
	pev->effects = 0;
	pev->health = gSkillData.headcrabHealth;
	pev->view_ofs = Vector(0, 0, 20);// position of the eyes relative to monster's origin.
	pev->yaw_speed = 5;//!!! should we put this in the monster's changeanim function since turn rates may vary with state/anim?
	m_flFieldOfView = 0.5;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState = NPCState::None;

	MonsterInit();
}

void CHeadCrab::Precache()
{
	PRECACHE_SOUND_ARRAY(pIdleSounds);
	PRECACHE_SOUND_ARRAY(pAlertSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pAttackSounds);
	PRECACHE_SOUND_ARRAY(pDeathSounds);
	PRECACHE_SOUND_ARRAY(pBiteSounds);

	PRECACHE_MODEL("models/headcrab.mdl");
}

void CHeadCrab::RunTask(Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_RANGE_ATTACK1:
	case TASK_RANGE_ATTACK2:
	{
		if (m_fSequenceFinished)
		{
			TaskComplete();
			SetTouch(nullptr);
			m_IdealActivity = ACT_IDLE;
		}
		break;
	}
	default:
	{
		CBaseMonster::RunTask(pTask);
	}
	}
}

void CHeadCrab::LeapTouch(CBaseEntity* pOther)
{
	if (!pOther->pev->takedamage)
	{
		return;
	}

	if (pOther->Classify() == Classify())
	{
		return;
	}

	// Don't hit if back on ground
	if (!IsBitSet(pev->flags, FL_ONGROUND))
	{
		EmitSound(SoundChannel::Weapon, RANDOM_SOUND_ARRAY(pBiteSounds), GetSoundVolue(), ATTN_IDLE, GetVoicePitch());

		pOther->TakeDamage({this, this, GetDamageAmount(), DMG_SLASH});
	}

	SetTouch(nullptr);
}

void CHeadCrab::PrescheduleThink()
{
	// make the crab coo a little bit in combat state
	if (m_MonsterState == NPCState::Combat && RANDOM_FLOAT(0, 5) < 0.1)
	{
		IdleSound();
	}
}

void CHeadCrab::StartTask(Task_t* pTask)
{
	m_iTaskStatus = TaskStatus::Running;

	switch (pTask->iTask)
	{
	case TASK_RANGE_ATTACK1:
	{
		EmitSound(SoundChannel::Weapon, pAttackSounds[0], GetSoundVolue(), ATTN_IDLE, GetVoicePitch());
		m_IdealActivity = ACT_RANGE_ATTACK1;
		SetTouch(&CHeadCrab::LeapTouch);
		break;
	}
	default:
	{
		CBaseMonster::StartTask(pTask);
	}
	}
}

bool CHeadCrab::CheckRangeAttack1(float flDot, float flDist)
{
	if (IsBitSet(pev->flags, FL_ONGROUND) && flDist <= 256 && flDot >= 0.65)
	{
		return true;
	}
	return false;
}

bool CHeadCrab::CheckRangeAttack2(float flDot, float flDist)
{
	return false;
	// BUGBUG: Why is this code here?  There is no ACT_RANGE_ATTACK2 animation.  I've disabled it for now.
#if 0
	if (IsBitSet(pev->flags, FL_ONGROUND) && flDist > 64 && flDist <= 256 && flDot >= 0.5)
	{
		return true;
	}
	return false;
#endif
}

bool CHeadCrab::TakeDamage(const TakeDamageInfo& info)
{
	TakeDamageInfo adjustedInfo = info;
	// Don't take any acid damage -- BigMomma's mortar is acid
	if (adjustedInfo.GetDamageTypes() & DMG_ACID)
		adjustedInfo.SetDamage(0);

	return CBaseMonster::TakeDamage(adjustedInfo);
}

void CHeadCrab::IdleSound()
{
	EmitSound(SoundChannel::Voice, RANDOM_SOUND_ARRAY(pIdleSounds), GetSoundVolue(), ATTN_IDLE, GetVoicePitch());
}

void CHeadCrab::AlertSound()
{
	EmitSound(SoundChannel::Voice, RANDOM_SOUND_ARRAY(pAlertSounds), GetSoundVolue(), ATTN_IDLE, GetVoicePitch());
}

void CHeadCrab::PainSound()
{
	EmitSound(SoundChannel::Voice, RANDOM_SOUND_ARRAY(pPainSounds), GetSoundVolue(), ATTN_IDLE, GetVoicePitch());
}

void CHeadCrab::DeathSound()
{
	EmitSound(SoundChannel::Voice, RANDOM_SOUND_ARRAY(pDeathSounds), GetSoundVolue(), ATTN_IDLE, GetVoicePitch());
}

Schedule_t* CHeadCrab::GetScheduleOfType(int Type)
{
	switch (Type)
	{
	case SCHED_RANGE_ATTACK1:
	{
		return &slHCRangeAttack1[0];
	}
	break;
	}

	return CBaseMonster::GetScheduleOfType(Type);
}
