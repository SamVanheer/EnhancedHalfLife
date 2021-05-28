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

#include "CBullsquid.hpp"
#include "CSquidSpit.hpp"
#include "navigation/nodes.hpp"
#include "game.hpp"

LINK_ENTITY_TO_CLASS(monster_bullchicken, CBullsquid);

int CBullsquid::IgnoreConditions()
{
	int iIgnore = CBaseMonster::IgnoreConditions();

	if (gpGlobals->time - m_flLastHurtTime <= 20)
	{
		// haven't been hurt in 20 seconds, so let the squid care about stink. 
		iIgnore = bits_COND_SMELL | bits_COND_SMELL_FOOD;
	}

	if (m_hEnemy != nullptr)
	{
		if (m_hEnemy->ClassnameIs("monster_headcrab"))
		{
			// (Unless after a tasty headcrab)
			iIgnore = bits_COND_SMELL | bits_COND_SMELL_FOOD;
		}
	}

	return iIgnore;
}

Relationship CBullsquid::GetRelationship(CBaseEntity* pTarget)
{
	if (gpGlobals->time - m_flLastHurtTime < 5 && pTarget->ClassnameIs("monster_headcrab"))
	{
		// if squid has been hurt in the last 5 seconds, and is getting relationship for a headcrab, 
		// tell squid to disregard crab. 
		return Relationship::None;
	}

	return CBaseMonster::GetRelationship(pTarget);
}

bool CBullsquid::TakeDamage(const TakeDamageInfo& info)
{
	// if the squid is running, has an enemy, was hurt by the enemy, hasn't been hurt in the last 3 seconds, and isn't too close to the enemy,
	// it will swerve. (whew).
	if (m_hEnemy != nullptr && IsMoving() && info.GetAttacker() == m_hEnemy && gpGlobals->time - m_flLastHurtTime > 3)
	{
		float flDist = (GetAbsOrigin() - m_hEnemy->GetAbsOrigin()).Length2D();

		if (flDist > SQUID_SPRINT_DIST)
		{
			flDist = (GetAbsOrigin() - m_Route[m_iRouteIndex].vecLocation).Length2D();// reusing flDist. 

			Vector vecApex;
			if (Triangulate(GetAbsOrigin(), m_Route[m_iRouteIndex].vecLocation, flDist * 0.5, m_hEnemy, &vecApex))
			{
				InsertWaypoint(vecApex, bits_MF_TO_DETOUR | bits_MF_DONT_SIMPLIFY);
			}
		}
	}

	if (!info.GetAttacker()->ClassnameIs("monster_headcrab"))
	{
		// don't forget about headcrabs if it was a headcrab that hurt the squid.
		m_flLastHurtTime = gpGlobals->time;
	}

	return CBaseMonster::TakeDamage(info);
}

bool CBullsquid::CheckRangeAttack1(float flDot, float flDist)
{
	if (IsMoving() && flDist >= 512)
	{
		// squid will far too far behind if he stops running to spit at this distance from the enemy.
		return false;
	}

	if (flDist > 64 && flDist <= 784 && flDot >= 0.5 && gpGlobals->time >= m_flNextSpitTime)
	{
		if (m_hEnemy != nullptr)
		{
			if (fabs(GetAbsOrigin().z - m_hEnemy->GetAbsOrigin().z) > 256)
			{
				// don't try to spit at someone up really high or down really low.
				return false;
			}
		}

		if (IsMoving())
		{
			// don't spit again for a long time, resume chasing enemy.
			m_flNextSpitTime = gpGlobals->time + 5;
		}
		else
		{
			// not moving, so spit again pretty soon.
			m_flNextSpitTime = gpGlobals->time + 0.5;
		}

		return true;
	}

	return false;
}

bool CBullsquid::CheckMeleeAttack1(float flDot, float flDist)
{
	if (m_hEnemy->pev->health <= gSkillData.bullsquidDmgWhip && flDist <= 85 && flDot >= 0.7)
	{
		return true;
	}
	return false;
}

bool CBullsquid::CheckMeleeAttack2(float flDot, float flDist)
{
	if (flDist <= 85 && flDot >= 0.7 && !HasConditions(bits_COND_CAN_MELEE_ATTACK1))		// The player & bullsquid can be as much as their bboxes 
	{										// apart (48 * sqrt(3)) and he can still attack (85 is a little more than 48*sqrt(3))
		return true;
	}
	return false;
}

bool CBullsquid::ValidateHintType(short sHint)
{
	static constexpr short sSquidHints[] =
	{
		HINT_WORLD_HUMAN_BLOOD,
	};

	for (std::size_t i = 0; i < ArraySize(sSquidHints); i++)
	{
		if (sSquidHints[i] == sHint)
		{
			return true;
		}
	}

	ALERT(at_aiconsole, "Couldn't validate hint type");
	return false;
}

int CBullsquid::SoundMask()
{
	return bits_SOUND_WORLD |
		bits_SOUND_COMBAT |
		bits_SOUND_CARCASS |
		bits_SOUND_MEAT |
		bits_SOUND_GARBAGE |
		bits_SOUND_PLAYER;
}

int	CBullsquid::Classify()
{
	return CLASS_ALIEN_PREDATOR;
}

constexpr float SQUID_ATTN_IDLE = 1.5;
void CBullsquid::IdleSound()
{
	switch (RANDOM_LONG(0, 4))
	{
	case 0:
		EmitSound(SoundChannel::Voice, "bullchicken/bc_idle1.wav", VOL_NORM, SQUID_ATTN_IDLE);
		break;
	case 1:
		EmitSound(SoundChannel::Voice, "bullchicken/bc_idle2.wav", VOL_NORM, SQUID_ATTN_IDLE);
		break;
	case 2:
		EmitSound(SoundChannel::Voice, "bullchicken/bc_idle3.wav", VOL_NORM, SQUID_ATTN_IDLE);
		break;
	case 3:
		EmitSound(SoundChannel::Voice, "bullchicken/bc_idle4.wav", VOL_NORM, SQUID_ATTN_IDLE);
		break;
	case 4:
		EmitSound(SoundChannel::Voice, "bullchicken/bc_idle5.wav", VOL_NORM, SQUID_ATTN_IDLE);
		break;
	}
}

void CBullsquid::PainSound()
{
	const int iPitch = RANDOM_LONG(85, 120);

	switch (RANDOM_LONG(0, 3))
	{
	case 0:
		EmitSound(SoundChannel::Voice, "bullchicken/bc_pain1.wav", VOL_NORM, ATTN_NORM, iPitch);
		break;
	case 1:
		EmitSound(SoundChannel::Voice, "bullchicken/bc_pain2.wav", VOL_NORM, ATTN_NORM, iPitch);
		break;
	case 2:
		EmitSound(SoundChannel::Voice, "bullchicken/bc_pain3.wav", VOL_NORM, ATTN_NORM, iPitch);
		break;
	case 3:
		EmitSound(SoundChannel::Voice, "bullchicken/bc_pain4.wav", VOL_NORM, ATTN_NORM, iPitch);
		break;
	}
}

void CBullsquid::AlertSound()
{
	const int iPitch = RANDOM_LONG(140, 160);

	switch (RANDOM_LONG(0, 1))
	{
	case 0:
		EmitSound(SoundChannel::Voice, "bullchicken/bc_idle1.wav", VOL_NORM, ATTN_NORM, iPitch);
		break;
	case 1:
		EmitSound(SoundChannel::Voice, "bullchicken/bc_idle2.wav", VOL_NORM, ATTN_NORM, iPitch);
		break;
	}
}

void CBullsquid::SetYawSpeed()
{
	int ys = 0;

	switch (m_Activity)
	{
	case	ACT_WALK:			ys = 90;	break;
	case	ACT_RUN:			ys = 90;	break;
	case	ACT_IDLE:			ys = 90;	break;
	case	ACT_RANGE_ATTACK1:	ys = 90;	break;
	default:
		ys = 90;
		break;
	}

	pev->yaw_speed = ys;
}

void CBullsquid::HandleAnimEvent(AnimationEvent& event)
{
	switch (event.event)
	{
	case BSQUID_AE_SPIT:
	{
		if (m_hEnemy)
		{
			UTIL_MakeVectors(GetAbsAngles());

			// !!!HACKHACK - the spot at which the spit originates (in front of the mouth) was measured in 3ds and hardcoded here.
			// we should be able to read the position of bones at runtime for this info.
			Vector vecSpitOffset = (gpGlobals->v_right * 8 + gpGlobals->v_forward * 37 + gpGlobals->v_up * 23);
			vecSpitOffset = (GetAbsOrigin() + vecSpitOffset);
			Vector vecSpitDir = ((m_hEnemy->GetAbsOrigin() + m_hEnemy->pev->view_ofs) - vecSpitOffset).Normalize();

			vecSpitDir.x += RANDOM_FLOAT(-0.05, 0.05);
			vecSpitDir.y += RANDOM_FLOAT(-0.05, 0.05);
			vecSpitDir.z += RANDOM_FLOAT(-0.05, 0);


			// do stuff for this event.
			AttackSound();

			// spew the spittle temporary ents.
			MESSAGE_BEGIN(MessageDest::PVS, SVC_TEMPENTITY, vecSpitOffset);
			WRITE_BYTE(TE_SPRITE_SPRAY);
			WRITE_COORD(vecSpitOffset.x);	// pos
			WRITE_COORD(vecSpitOffset.y);
			WRITE_COORD(vecSpitOffset.z);
			WRITE_COORD(vecSpitDir.x);	// dir
			WRITE_COORD(vecSpitDir.y);
			WRITE_COORD(vecSpitDir.z);
			WRITE_SHORT(iSquidSpitSprite);	// model
			WRITE_BYTE(15);			// count
			WRITE_BYTE(210);			// speed
			WRITE_BYTE(25);			// noise ( client will divide by 100 )
			MESSAGE_END();

			CSquidSpit::Shoot(this, vecSpitOffset, vecSpitDir * 900);
		}
	}
	break;

	case BSQUID_AE_BITE:
	{
		// SOUND HERE!
		if (CBaseEntity* pHurt = CheckTraceHullAttack(70, gSkillData.bullsquidDmgBite, DMG_SLASH); pHurt)
		{
			//pHurt->pev->punchangle.z = -15;
			//pHurt->pev->punchangle.x = -45;
			pHurt->SetAbsVelocity(pHurt->GetAbsVelocity() - gpGlobals->v_forward * 100);
			pHurt->SetAbsVelocity(pHurt->GetAbsVelocity() + gpGlobals->v_up * 100);
		}
	}
	break;

	case BSQUID_AE_TAILWHIP:
	{
		if (CBaseEntity* pHurt = CheckTraceHullAttack(70, gSkillData.bullsquidDmgWhip, DMG_CLUB | DMG_ALWAYSGIB); pHurt)
		{
			pHurt->pev->punchangle.z = -20;
			pHurt->pev->punchangle.x = 20;
			pHurt->SetAbsVelocity(pHurt->GetAbsVelocity() + gpGlobals->v_right * 200);
			pHurt->SetAbsVelocity(pHurt->GetAbsVelocity() + gpGlobals->v_up * 100);
		}
	}
	break;

	case BSQUID_AE_BLINK:
	{
		// close eye. 
		pev->skin = 1;
	}
	break;

	case BSQUID_AE_HOP:
	{
		const float flGravity = g_psv_gravity->value;

		// throw the squid up into the air on this frame.
		if (IsBitSet(pev->flags, FL_ONGROUND))
		{
			pev->flags -= FL_ONGROUND;
		}

		// jump into air for 0.8 (24/30) seconds
		// SetAbsVelocity(GetAbsVelocity() + Vector(0, 0, (0.875 * flGravity) * 0.5));
		SetAbsVelocity(GetAbsVelocity() + Vector(0, 0, (0.625 * flGravity) * 0.5));
	}
	break;

	case BSQUID_AE_THROW:
	{
		// squid throws its prey IF the prey is a client. 
		if (CBaseEntity* pHurt = CheckTraceHullAttack(70, 0, 0); pHurt)
		{
			// croonchy bite sound
			const int iPitch = RANDOM_FLOAT(90, 110);
			switch (RANDOM_LONG(0, 1))
			{
			case 0:
				EmitSound(SoundChannel::Weapon, "bullchicken/bc_bite2.wav", VOL_NORM, ATTN_NORM, iPitch);
				break;
			case 1:
				EmitSound(SoundChannel::Weapon, "bullchicken/bc_bite3.wav", VOL_NORM, ATTN_NORM, iPitch);
				break;
			}


			//pHurt->pev->punchangle.x = RANDOM_LONG(0,34) - 5;
			//pHurt->pev->punchangle.z = RANDOM_LONG(0,49) - 25;
			//pHurt->pev->punchangle.y = RANDOM_LONG(0,89) - 45;

			// screeshake transforms the viewmodel as well as the viewangle. No problems with seeing the ends of the viewmodels.
			UTIL_ScreenShake(pHurt->GetAbsOrigin(), 25.0, 1.5, 0.7, 2);

			if (pHurt->IsPlayer())
			{
				UTIL_MakeVectors(GetAbsAngles());
				pHurt->SetAbsVelocity(pHurt->GetAbsVelocity() + gpGlobals->v_forward * 300 + gpGlobals->v_up * 300);
			}
		}
	}
	break;

	default:
		CBaseMonster::HandleAnimEvent(event);
	}
}

void CBullsquid::Spawn()
{
	Precache();

	SetModel("models/bullsquid.mdl");
	SetSize(Vector(-32, -32, 0), Vector(32, 32, 64));

	SetSolidType(Solid::SlideBox);
	SetMovetype(Movetype::Step);
	m_bloodColor = BLOOD_COLOR_GREEN;
	pev->effects = 0;
	pev->health = gSkillData.bullsquidHealth;
	m_flFieldOfView = 0.2;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState = NPCState::None;

	m_fCanThreatDisplay = true;
	m_flNextSpitTime = gpGlobals->time;

	MonsterInit();
}

void CBullsquid::Precache()
{
	PRECACHE_MODEL("models/bullsquid.mdl");

	PRECACHE_MODEL("sprites/bigspit.spr");// spit projectile.

	iSquidSpitSprite = PRECACHE_MODEL("sprites/tinyspit.spr");// client side spittle.

	PRECACHE_SOUND("zombie/claw_miss2.wav");// because we use the basemonster SWIPE animation event

	PRECACHE_SOUND("bullchicken/bc_attack2.wav");
	PRECACHE_SOUND("bullchicken/bc_attack3.wav");

	PRECACHE_SOUND("bullchicken/bc_die1.wav");
	PRECACHE_SOUND("bullchicken/bc_die2.wav");
	PRECACHE_SOUND("bullchicken/bc_die3.wav");

	PRECACHE_SOUND("bullchicken/bc_idle1.wav");
	PRECACHE_SOUND("bullchicken/bc_idle2.wav");
	PRECACHE_SOUND("bullchicken/bc_idle3.wav");
	PRECACHE_SOUND("bullchicken/bc_idle4.wav");
	PRECACHE_SOUND("bullchicken/bc_idle5.wav");

	PRECACHE_SOUND("bullchicken/bc_pain1.wav");
	PRECACHE_SOUND("bullchicken/bc_pain2.wav");
	PRECACHE_SOUND("bullchicken/bc_pain3.wav");
	PRECACHE_SOUND("bullchicken/bc_pain4.wav");

	PRECACHE_SOUND("bullchicken/bc_attackgrowl.wav");
	PRECACHE_SOUND("bullchicken/bc_attackgrowl2.wav");
	PRECACHE_SOUND("bullchicken/bc_attackgrowl3.wav");

	PRECACHE_SOUND("bullchicken/bc_acid1.wav");

	PRECACHE_SOUND("bullchicken/bc_bite2.wav");
	PRECACHE_SOUND("bullchicken/bc_bite3.wav");

	PRECACHE_SOUND("bullchicken/bc_spithit1.wav");
	PRECACHE_SOUND("bullchicken/bc_spithit2.wav");
}

void CBullsquid::DeathSound()
{
	switch (RANDOM_LONG(0, 2))
	{
	case 0:
		EmitSound(SoundChannel::Voice, "bullchicken/bc_die1.wav");
		break;
	case 1:
		EmitSound(SoundChannel::Voice, "bullchicken/bc_die2.wav");
		break;
	case 2:
		EmitSound(SoundChannel::Voice, "bullchicken/bc_die3.wav");
		break;
	}
}

void CBullsquid::AttackSound()
{
	switch (RANDOM_LONG(0, 1))
	{
	case 0:
		EmitSound(SoundChannel::Weapon, "bullchicken/bc_attack2.wav");
		break;
	case 1:
		EmitSound(SoundChannel::Weapon, "bullchicken/bc_attack3.wav");
		break;
	}
}

void CBullsquid::RunAI()
{
	// first, do base class stuff
	CBaseMonster::RunAI();

	if (pev->skin != 0)
	{
		// close eye if it was open.
		pev->skin = 0;
	}

	if (RANDOM_LONG(0, 39) == 0)
	{
		pev->skin = 1;
	}

	if (m_hEnemy != nullptr && m_Activity == ACT_RUN)
	{
		// chasing enemy. Sprint for last bit
		if ((GetAbsOrigin() - m_hEnemy->GetAbsOrigin()).Length2D() < SQUID_SPRINT_DIST)
		{
			pev->framerate = 1.25;
		}
	}
}

//========================================================
// AI Schedules Specific to this monster
//=========================================================
Task_t	tlSquidRangeAttack1[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_FACE_IDEAL,			(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE	},
};

/**
*	@brief primary range attack
*/
Schedule_t	slSquidRangeAttack1[] =
{
	{
		tlSquidRangeAttack1,
		ArraySize(tlSquidRangeAttack1),
		bits_COND_NEW_ENEMY |
		bits_COND_ENEMY_DEAD |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_ENEMY_OCCLUDED |
		bits_COND_NO_AMMO_LOADED,
		0,
		"Squid Range Attack1"
	},
};

Task_t tlSquidChaseEnemy1[] =
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_RANGE_ATTACK1	},// !!!OEM - this will stop nasty squid oscillation.
	{ TASK_GET_PATH_TO_ENEMY,	(float)0					},
	{ TASK_RUN_PATH,			(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,	(float)0					},
};

/**
*	@brief Chase enemy schedule
*/
Schedule_t slSquidChaseEnemy[] =
{
	{
		tlSquidChaseEnemy1,
		ArraySize(tlSquidChaseEnemy1),
		bits_COND_NEW_ENEMY |
		bits_COND_ENEMY_DEAD |
		bits_COND_SMELL_FOOD |
		bits_COND_CAN_RANGE_ATTACK1 |
		bits_COND_CAN_MELEE_ATTACK1 |
		bits_COND_CAN_MELEE_ATTACK2 |
		bits_COND_TASK_FAILED |
		bits_COND_HEAR_SOUND,

		bits_SOUND_DANGER |
		bits_SOUND_MEAT,
		"Squid Chase Enemy"
	},
};

Task_t tlSquidHurtHop[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_SOUND_WAKE,			(float)0		},
	{ TASK_SQUID_HOPTURN,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},// in case squid didn't turn all the way in the air.
};

Schedule_t slSquidHurtHop[] =
{
	{
		tlSquidHurtHop,
		ArraySize(tlSquidHurtHop),
		0,
		0,
		"SquidHurtHop"
	}
};

Task_t tlSquidSeeCrab[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_SOUND_WAKE,			(float)0			},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_EXCITED	},
	{ TASK_FACE_ENEMY,			(float)0			},
};

Schedule_t slSquidSeeCrab[] =
{
	{
		tlSquidSeeCrab,
		ArraySize(tlSquidSeeCrab),
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE,
		0,
		"SquidSeeCrab"
	}
};

Task_t tlSquidEat[] =
{
	{ TASK_STOP_MOVING,				(float)0				},
	{ TASK_EAT,						(float)10				},// this is in case the squid can't get to the food
	{ TASK_STORE_LASTPOSITION,		(float)0				},
	{ TASK_GET_PATH_TO_BESTSCENT,	(float)0				},
	{ TASK_WALK_PATH,				(float)0				},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0				},
	{ TASK_PLAY_SEQUENCE,			(float)ACT_EAT			},
	{ TASK_PLAY_SEQUENCE,			(float)ACT_EAT			},
	{ TASK_PLAY_SEQUENCE,			(float)ACT_EAT			},
	{ TASK_EAT,						(float)50				},
	{ TASK_GET_PATH_TO_LASTPOSITION,(float)0				},
	{ TASK_WALK_PATH,				(float)0				},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0				},
	{ TASK_CLEAR_LASTPOSITION,		(float)0				},
};

/**
*	@brief squid walks to something tasty and eats it.
*/
Schedule_t slSquidEat[] =
{
	{
		tlSquidEat,
		ArraySize(tlSquidEat),
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_NEW_ENEMY	,

	// even though HEAR_SOUND/SMELL FOOD doesn't break this schedule, we need this mask
	// here or the monster won't detect these sounds at ALL while running this schedule.
	bits_SOUND_MEAT |
	bits_SOUND_CARCASS,
	"SquidEat"
}
};

Task_t tlSquidSniffAndEat[] =
{
	{ TASK_STOP_MOVING,				(float)0				},
	{ TASK_EAT,						(float)10				},// this is in case the squid can't get to the food
	{ TASK_PLAY_SEQUENCE,			(float)ACT_DETECT_SCENT },
	{ TASK_STORE_LASTPOSITION,		(float)0				},
	{ TASK_GET_PATH_TO_BESTSCENT,	(float)0				},
	{ TASK_WALK_PATH,				(float)0				},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0				},
	{ TASK_PLAY_SEQUENCE,			(float)ACT_EAT			},
	{ TASK_PLAY_SEQUENCE,			(float)ACT_EAT			},
	{ TASK_PLAY_SEQUENCE,			(float)ACT_EAT			},
	{ TASK_EAT,						(float)50				},
	{ TASK_GET_PATH_TO_LASTPOSITION,(float)0				},
	{ TASK_WALK_PATH,				(float)0				},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0				},
	{ TASK_CLEAR_LASTPOSITION,		(float)0				},
};

/**
*	@brief this is a bit different than just Eat.
*	We use this schedule when the food is far away, occluded, or behind the squid.
*	This schedule plays a sniff animation before going to the source of food.
*/
Schedule_t slSquidSniffAndEat[] =
{
	{
		tlSquidSniffAndEat,
		ArraySize(tlSquidSniffAndEat),
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_NEW_ENEMY	,

	// even though HEAR_SOUND/SMELL FOOD doesn't break this schedule, we need this mask
	// here or the monster won't detect these sounds at ALL while running this schedule.
	bits_SOUND_MEAT |
	bits_SOUND_CARCASS,
	"SquidSniffAndEat"
}
};

Task_t tlSquidWallow[] =
{
	{ TASK_STOP_MOVING,				(float)0				},
	{ TASK_EAT,						(float)10				},// this is in case the squid can't get to the stinkiness
	{ TASK_STORE_LASTPOSITION,		(float)0				},
	{ TASK_GET_PATH_TO_BESTSCENT,	(float)0				},
	{ TASK_WALK_PATH,				(float)0				},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0				},
	{ TASK_PLAY_SEQUENCE,			(float)ACT_INSPECT_FLOOR},
	{ TASK_EAT,						(float)50				},// keeps squid from eating or sniffing anything else for a while.
	{ TASK_GET_PATH_TO_LASTPOSITION,(float)0				},
	{ TASK_WALK_PATH,				(float)0				},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0				},
	{ TASK_CLEAR_LASTPOSITION,		(float)0				},
};

/**
*	@brief squid does this to stinky things.
*/
Schedule_t slSquidWallow[] =
{
	{
		tlSquidWallow,
		ArraySize(tlSquidWallow),
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_NEW_ENEMY	,

	// even though HEAR_SOUND/SMELL FOOD doesn't break this schedule, we need this mask
	// here or the monster won't detect these sounds at ALL while running this schedule.
	bits_SOUND_GARBAGE,

	"SquidWallow"
}
};

DEFINE_CUSTOM_SCHEDULES(CBullsquid)
{
	slSquidRangeAttack1,
		slSquidChaseEnemy,
		slSquidHurtHop,
		slSquidSeeCrab,
		slSquidEat,
		slSquidSniffAndEat,
		slSquidWallow
};

IMPLEMENT_CUSTOM_SCHEDULES(CBullsquid, CBaseMonster);

Schedule_t* CBullsquid::GetSchedule()
{
	switch (m_MonsterState)
	{
	case NPCState::Alert:
	{
		if (HasConditions(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE))
		{
			return GetScheduleOfType(SCHED_SQUID_HURTHOP);
		}

		if (HasConditions(bits_COND_SMELL_FOOD))
		{
			if (CSound* pSound = BestScent(); pSound && (!IsInViewCone(pSound->m_vecOrigin) || !IsVisible(pSound->m_vecOrigin)))
			{
				// scent is behind or occluded
				return GetScheduleOfType(SCHED_SQUID_SNIFF_AND_EAT);
			}

			// food is right out in the open. Just go get it.
			return GetScheduleOfType(SCHED_SQUID_EAT);
		}

		if (HasConditions(bits_COND_SMELL))
		{
			// there's something stinky. 
			if (CSound* pSound = BestScent(); pSound)
				return GetScheduleOfType(SCHED_SQUID_WALLOW);
		}

		break;
	}
	case NPCState::Combat:
	{
		// dead enemy
		if (HasConditions(bits_COND_ENEMY_DEAD))
		{
			// call base class, all code to handle dead enemies is centralized there.
			return CBaseMonster::GetSchedule();
		}

		if (HasConditions(bits_COND_NEW_ENEMY))
		{
			if (m_fCanThreatDisplay && GetRelationship(m_hEnemy) == Relationship::Hate)
			{
				// this means squid sees a headcrab!
				m_fCanThreatDisplay = false;// only do the headcrab dance once per lifetime.
				return GetScheduleOfType(SCHED_SQUID_SEECRAB);
			}
			else
			{
				return GetScheduleOfType(SCHED_WAKE_ANGRY);
			}
		}

		if (HasConditions(bits_COND_SMELL_FOOD))
		{
			if (CSound* pSound = BestScent(); pSound && (!IsInViewCone(pSound->m_vecOrigin) || !IsVisible(pSound->m_vecOrigin)))
			{
				// scent is behind or occluded
				return GetScheduleOfType(SCHED_SQUID_SNIFF_AND_EAT);
			}

			// food is right out in the open. Just go get it.
			return GetScheduleOfType(SCHED_SQUID_EAT);
		}

		if (HasConditions(bits_COND_CAN_RANGE_ATTACK1))
		{
			return GetScheduleOfType(SCHED_RANGE_ATTACK1);
		}

		if (HasConditions(bits_COND_CAN_MELEE_ATTACK1))
		{
			return GetScheduleOfType(SCHED_MELEE_ATTACK1);
		}

		if (HasConditions(bits_COND_CAN_MELEE_ATTACK2))
		{
			return GetScheduleOfType(SCHED_MELEE_ATTACK2);
		}

		return GetScheduleOfType(SCHED_CHASE_ENEMY);
	}
	}

	return CBaseMonster::GetSchedule();
}

Schedule_t* CBullsquid::GetScheduleOfType(int Type)
{
	switch (Type)
	{
	case SCHED_RANGE_ATTACK1:
		return &slSquidRangeAttack1[0];
		break;
	case SCHED_SQUID_HURTHOP:
		return &slSquidHurtHop[0];
		break;
	case SCHED_SQUID_SEECRAB:
		return &slSquidSeeCrab[0];
		break;
	case SCHED_SQUID_EAT:
		return &slSquidEat[0];
		break;
	case SCHED_SQUID_SNIFF_AND_EAT:
		return &slSquidSniffAndEat[0];
		break;
	case SCHED_SQUID_WALLOW:
		return &slSquidWallow[0];
		break;
	case SCHED_CHASE_ENEMY:
		return &slSquidChaseEnemy[0];
		break;
	}

	return CBaseMonster::GetScheduleOfType(Type);
}

void CBullsquid::StartTask(Task_t* pTask)
{
	m_iTaskStatus = TaskStatus::Running;

	switch (pTask->iTask)
	{
	case TASK_MELEE_ATTACK2:
	{
		switch (RANDOM_LONG(0, 2))
		{
		case 0:
			EmitSound(SoundChannel::Voice, "bullchicken/bc_attackgrowl.wav");
			break;
		case 1:
			EmitSound(SoundChannel::Voice, "bullchicken/bc_attackgrowl2.wav");
			break;
		case 2:
			EmitSound(SoundChannel::Voice, "bullchicken/bc_attackgrowl3.wav");
			break;
		}

		CBaseMonster::StartTask(pTask);
		break;
	}
	case TASK_SQUID_HOPTURN:
	{
		SetActivity(ACT_HOP);
		MakeIdealYaw(m_vecEnemyLKP);
		break;
	}
	case TASK_GET_PATH_TO_ENEMY:
	{
		if (BuildRoute(m_hEnemy->GetAbsOrigin(), bits_MF_TO_ENEMY, m_hEnemy))
		{
			m_iTaskStatus = TaskStatus::Complete;
		}
		else
		{
			ALERT(at_aiconsole, "GetPathToEnemy failed!!\n");
			TaskFail();
		}
		break;
	}
	default:
	{
		CBaseMonster::StartTask(pTask);
		break;
	}
	}
}

void CBullsquid::RunTask(Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_SQUID_HOPTURN:
	{
		MakeIdealYaw(m_vecEnemyLKP);
		ChangeYaw(pev->yaw_speed);

		if (m_fSequenceFinished)
		{
			m_iTaskStatus = TaskStatus::Complete;
		}
		break;
	}
	default:
	{
		CBaseMonster::RunTask(pTask);
		break;
	}
	}
}

NPCState CBullsquid::GetIdealState()
{
	const int iConditions = ScheduleFlags();

	// If no schedule conditions, the new ideal state is probably the reason we're in here.
	switch (m_MonsterState)
	{
	case NPCState::Combat:
		/*
		COMBAT goes to ALERT upon death of enemy
		*/
	{
		if (m_hEnemy != nullptr && (iConditions & bits_COND_LIGHT_DAMAGE || iConditions & bits_COND_HEAVY_DAMAGE) && m_hEnemy->ClassnameIs("monster_headcrab"))
		{
			// if the squid has a headcrab enemy and something hurts it, it's going to forget about the crab for a while.
			m_hEnemy = nullptr;
			m_IdealMonsterState = NPCState::Alert;
		}
		break;
	}
	}

	m_IdealMonsterState = CBaseMonster::GetIdealState();

	return m_IdealMonsterState;
}