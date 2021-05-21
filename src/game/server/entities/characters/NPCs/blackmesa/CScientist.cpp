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

#include "CBaseMonster.defaultai.hpp"
#include "CScientist.hpp"
#include "animation.h"

LINK_ENTITY_TO_CLASS(monster_scientist, CScientist);

//=========================================================
// AI Schedules Specific to this monster
//=========================================================
Task_t	tlFollow[] =
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_CANT_FOLLOW },	// If you fail, bail out of follow
	{ TASK_MOVE_TO_TARGET_RANGE,(float)128		},	// Move within 128 of target ent (client)
//	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_FACE },
};

Schedule_t	slFollow[] =
{
	{
		tlFollow,
		ArraySize(tlFollow),
		bits_COND_NEW_ENEMY |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_HEAR_SOUND,
		bits_SOUND_COMBAT |
		bits_SOUND_DANGER,
		"Follow"
	},
};

Task_t	tlFollowScared[] =
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_TARGET_CHASE },// If you fail, follow normally
	{ TASK_MOVE_TO_TARGET_RANGE_SCARED,(float)128		},	// Move within 128 of target ent (client)
//	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_FACE_SCARED },
};

Schedule_t	slFollowScared[] =
{
	{
		tlFollowScared,
		ArraySize(tlFollowScared),
		bits_COND_NEW_ENEMY |
		bits_COND_HEAR_SOUND |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE,
		bits_SOUND_DANGER,
		"FollowScared"
	},
};

Task_t	tlFaceTargetScared[] =
{
	{ TASK_FACE_TARGET,			(float)0		},
	{ TASK_SET_ACTIVITY,		(float)ACT_CROUCHIDLE },
	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_CHASE_SCARED },
};

Schedule_t	slFaceTargetScared[] =
{
	{
		tlFaceTargetScared,
		ArraySize(tlFaceTargetScared),
		bits_COND_HEAR_SOUND |
		bits_COND_NEW_ENEMY,
		bits_SOUND_DANGER,
		"FaceTargetScared"
	},
};

Task_t	tlStopFollowing[] =
{
	{ TASK_CANT_FOLLOW,		(float)0 },
};

Schedule_t	slStopFollowing[] =
{
	{
		tlStopFollowing,
		ArraySize(tlStopFollowing),
		0,
		0,
		"StopFollowing"
	},
};

Task_t	tlHeal[] =
{
	{ TASK_MOVE_TO_TARGET_RANGE,(float)50		},	// Move within 60 of target ent (client)
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_TARGET_CHASE },	// If you fail, catch up with that guy! (change this to put syringe away and then chase)
	{ TASK_FACE_IDEAL,			(float)0		},
	{ TASK_SAY_HEAL,			(float)0		},
	{ TASK_PLAY_SEQUENCE_FACE_TARGET,		(float)ACT_ARM	},			// Whip out the needle
	{ TASK_HEAL,				(float)0	},	// Put it in the player
	{ TASK_PLAY_SEQUENCE_FACE_TARGET,		(float)ACT_DISARM	},			// Put away the needle
};

Schedule_t	slHeal[] =
{
	{
		tlHeal,
		ArraySize(tlHeal),
		0,	// Don't interrupt or he'll end up running around with a needle all the time
		0,
		"Heal"
	},
};

Task_t	tlFaceTarget[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_FACE_TARGET,			(float)0		},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_CHASE },
};

Schedule_t	slFaceTarget[] =
{
	{
		tlFaceTarget,
		ArraySize(tlFaceTarget),
		bits_COND_CLIENT_PUSH |
		bits_COND_NEW_ENEMY |
		bits_COND_HEAR_SOUND,
		bits_SOUND_COMBAT |
		bits_SOUND_DANGER,
		"FaceTarget"
	},
};

Task_t	tlSciPanic[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_SCREAM,				(float)0		},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,		(float)ACT_EXCITED	},	// This is really fear-stricken excitement
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE	},
};

Schedule_t	slSciPanic[] =
{
	{
		tlSciPanic,
		ArraySize(tlSciPanic),
		0,
		0,
		"SciPanic"
	},
};

Task_t	tlIdleSciStand[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT,				(float)2		}, // repick IDLESTAND every two seconds.
	{ TASK_TLK_HEADRESET,		(float)0		}, // reset head position
};

Schedule_t	slIdleSciStand[] =
{
	{
		tlIdleSciStand,
		ArraySize(tlIdleSciStand),
		bits_COND_NEW_ENEMY |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_HEAR_SOUND |
		bits_COND_SMELL |
		bits_COND_CLIENT_PUSH |
		bits_COND_PROVOKED,

		bits_SOUND_COMBAT |// sound flags
		//bits_SOUND_PLAYER		|
		//bits_SOUND_WORLD		|
		bits_SOUND_DANGER |
		bits_SOUND_MEAT |// scents
		bits_SOUND_CARCASS |
		bits_SOUND_GARBAGE,
		"IdleSciStand"

	},
};

Task_t	tlScientistCover[] =
{
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_PANIC },		// If you fail, just panic!
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_FIND_COVER_FROM_ENEMY,	(float)0					},
	{ TASK_RUN_PATH_SCARED,			(float)0					},
	{ TASK_TURN_LEFT,				(float)179					},
	{ TASK_SET_SCHEDULE,			(float)SCHED_HIDE			},
};

Schedule_t	slScientistCover[] =
{
	{
		tlScientistCover,
		ArraySize(tlScientistCover),
		bits_COND_NEW_ENEMY,
		0,
		"ScientistCover"
	},
};

Task_t	tlScientistHide[] =
{
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_PANIC },		// If you fail, just panic!
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_PLAY_SEQUENCE,			(float)ACT_CROUCH			},
	{ TASK_SET_ACTIVITY,			(float)ACT_CROUCHIDLE		},	// FIXME: This looks lame
	{ TASK_WAIT_RANDOM,				(float)10.0					},
};

Schedule_t	slScientistHide[] =
{
	{
		tlScientistHide,
		ArraySize(tlScientistHide),
		bits_COND_NEW_ENEMY |
		bits_COND_HEAR_SOUND |
		bits_COND_SEE_ENEMY |
		bits_COND_SEE_HATE |
		bits_COND_SEE_FEAR |
		bits_COND_SEE_DISLIKE,
		bits_SOUND_DANGER,
		"ScientistHide"
	},
};

Task_t	tlScientistStartle[] =
{
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_PANIC },		// If you fail, just panic!
	{ TASK_RANDOM_SCREAM,			(float)0.3 },				// Scream 30% of the time
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,			(float)ACT_CROUCH			},
	{ TASK_RANDOM_SCREAM,			(float)0.1 },				// Scream again 10% of the time
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,			(float)ACT_CROUCHIDLE		},
	{ TASK_WAIT_RANDOM,				(float)1.0					},
};

Schedule_t	slScientistStartle[] =
{
	{
		tlScientistStartle,
		ArraySize(tlScientistStartle),
		bits_COND_NEW_ENEMY |
		bits_COND_SEE_ENEMY |
		bits_COND_SEE_HATE |
		bits_COND_SEE_FEAR |
		bits_COND_SEE_DISLIKE,
		0,
		"ScientistStartle"
	},
};

Task_t	tlFear[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_FACE_ENEMY,				(float)0					},
	{ TASK_SAY_FEAR,				(float)0					},
	//	{ TASK_PLAY_SEQUENCE,			(float)ACT_FEAR_DISPLAY		},
};

Schedule_t	slFear[] =
{
	{
		tlFear,
		ArraySize(tlFear),
		bits_COND_NEW_ENEMY,
		0,
		"Fear"
	},
};

DEFINE_CUSTOM_SCHEDULES(CScientist)
{
	slFollow,
		slFaceTarget,
		slIdleSciStand,
		slFear,
		slScientistCover,
		slScientistHide,
		slScientistStartle,
		slHeal,
		slStopFollowing,
		slSciPanic,
		slFollowScared,
		slFaceTargetScared,
};

IMPLEMENT_CUSTOM_SCHEDULES(CScientist, CTalkMonster);

void CScientist::DeclineFollowing()
{
	Talk(10);
	m_hTalkTarget = m_hEnemy;
	PlaySentence("SC_POK", 2, VOL_NORM, ATTN_NORM);
}

void CScientist::Scream()
{
	if (OkToSpeak())
	{
		Talk(10);
		m_hTalkTarget = m_hEnemy;
		PlaySentence("SC_SCREAM", RANDOM_FLOAT(3, 6), VOL_NORM, ATTN_NORM);
	}
}

Activity CScientist::GetStoppedActivity()
{
	if (m_hEnemy != nullptr)
		return ACT_EXCITED;
	return CTalkMonster::GetStoppedActivity();
}

void CScientist::StartTask(Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_SAY_HEAL:
		//		if ( OkToSpeak() )
		Talk(2);
		m_hTalkTarget = m_hTargetEnt;
		PlaySentence("SC_HEAL", 2, VOL_NORM, ATTN_IDLE);

		TaskComplete();
		break;

	case TASK_SCREAM:
		Scream();
		TaskComplete();
		break;

	case TASK_RANDOM_SCREAM:
		if (RANDOM_FLOAT(0, 1) < pTask->flData)
			Scream();
		TaskComplete();
		break;

	case TASK_SAY_FEAR:
		if (OkToSpeak())
		{
			Talk(2);
			m_hTalkTarget = m_hEnemy;
			if (m_hEnemy->IsPlayer())
				PlaySentence("SC_PLFEAR", 5, VOL_NORM, ATTN_NORM);
			else
				PlaySentence("SC_FEAR", 5, VOL_NORM, ATTN_NORM);
		}
		TaskComplete();
		break;

	case TASK_HEAL:
		m_IdealActivity = ACT_MELEE_ATTACK1;
		break;

	case TASK_RUN_PATH_SCARED:
		m_movementActivity = ACT_RUN_SCARED;
		break;

	case TASK_MOVE_TO_TARGET_RANGE_SCARED:
	{
		if ((m_hTargetEnt->GetAbsOrigin() - GetAbsOrigin()).Length() < 1)
			TaskComplete();
		else
		{
			m_vecMoveGoal = m_hTargetEnt->GetAbsOrigin();
			if (!MoveToTarget(ACT_WALK_SCARED, 0.5))
				TaskFail();
		}
	}
	break;

	default:
		CTalkMonster::StartTask(pTask);
		break;
	}
}

void CScientist::RunTask(Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_RUN_PATH_SCARED:
		if (MovementIsComplete())
			TaskComplete();
		if (RANDOM_LONG(0, 31) < 8)
			Scream();
		break;

	case TASK_MOVE_TO_TARGET_RANGE_SCARED:
	{
		if (RANDOM_LONG(0, 63) < 8)
			Scream();

		if (m_hEnemy == nullptr)
		{
			TaskFail();
		}
		else
		{
			float distance = (m_vecMoveGoal - GetAbsOrigin()).Length2D();
			// Re-evaluate when you think your finished, or the target has moved too far
			if ((distance < pTask->flData) || (m_vecMoveGoal - m_hTargetEnt->GetAbsOrigin()).Length() > pTask->flData * 0.5)
			{
				m_vecMoveGoal = m_hTargetEnt->GetAbsOrigin();
				distance = (m_vecMoveGoal - GetAbsOrigin()).Length2D();
				RefreshRoute();
			}

			// Set the appropriate activity based on an overlapping range
			// overlap the range to prevent oscillation
			if (distance < pTask->flData)
			{
				TaskComplete();
				RouteClear();		// Stop moving
			}
			else if (distance < 190 && m_movementActivity != ACT_WALK_SCARED)
				m_movementActivity = ACT_WALK_SCARED;
			else if (distance >= 270 && m_movementActivity != ACT_RUN_SCARED)
				m_movementActivity = ACT_RUN_SCARED;
		}
	}
	break;

	case TASK_HEAL:
		if (m_fSequenceFinished)
		{
			TaskComplete();
		}
		else
		{
			if (TargetDistance() > 90)
				TaskComplete();
			pev->ideal_yaw = UTIL_VecToYaw(m_hTargetEnt->GetAbsOrigin() - GetAbsOrigin());
			ChangeYaw(pev->yaw_speed);
		}
		break;
	default:
		CTalkMonster::RunTask(pTask);
		break;
	}
}

int	CScientist::Classify()
{
	return CLASS_HUMAN_PASSIVE;
}

void CScientist::SetYawSpeed()
{
	int ys = 90;

	switch (m_Activity)
	{
	case ACT_IDLE:
		ys = 120;
		break;
	case ACT_WALK:
		ys = 180;
		break;
	case ACT_RUN:
		ys = 150;
		break;
	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		ys = 120;
		break;
	}

	pev->yaw_speed = ys;
}

void CScientist::HandleAnimEvent(AnimationEvent& event)
{
	switch (event.event)
	{
	case SCIENTIST_AE_HEAL:		// Heal my target (if within range)
		Heal();
		break;
	case SCIENTIST_AE_NEEDLEON:
	{
		SetBodygroup(SCIENTIST_BODYGROUP_NEEDLE, SCIENTIST_NEEDLE_ON);
	}
	break;
	case SCIENTIST_AE_NEEDLEOFF:
	{
		SetBodygroup(SCIENTIST_BODYGROUP_NEEDLE, SCIENTIST_NEEDLE_OFF);
	}
	break;

	default:
		CTalkMonster::HandleAnimEvent(event);
	}
}

void CScientist::Spawn()
{
	Precache();

	SetModel("models/scientist.mdl");
	SetSize(VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	SetSolidType(Solid::SlideBox);
	SetMovetype(Movetype::Step);
	m_bloodColor = BLOOD_COLOR_RED;
	pev->health = gSkillData.scientistHealth;
	pev->view_ofs = Vector(0, 0, 50);// position of the eyes relative to monster's origin.
	m_flFieldOfView = VIEW_FIELD_WIDE; // NOTE: we need a wide field of view so scientists will notice player and say hello
	m_MonsterState = NPCState::None;

	//	m_flDistTooFar		= 256.0;

	m_afCapability = bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_OPEN_DOORS | bits_CAP_AUTO_DOORS | bits_CAP_USE;

	// White hands
	pev->skin = 0;

	if (pev->body == -1)
	{// -1 chooses a random head
		pev->body = RANDOM_LONG(0, NUM_SCIENTIST_HEADS - 1);// pick a head, any head
	}

	//Remap old bodies to new ones
	SetBodygroup(SCIENTIST_BODYGROUP_HEAD, pev->body);

	// Luther is black, make his hands black
	if (GetBodygroup(SCIENTIST_BODYGROUP_HEAD) == SCIENTIST_HEAD_LUTHER)
		pev->skin = 1;

	MonsterInit();
	SetUse(&CScientist::FollowerUse);
}

void CScientist::Precache()
{
	PRECACHE_MODEL("models/scientist.mdl");
	PRECACHE_SOUND("scientist/sci_pain1.wav");
	PRECACHE_SOUND("scientist/sci_pain2.wav");
	PRECACHE_SOUND("scientist/sci_pain3.wav");
	PRECACHE_SOUND("scientist/sci_pain4.wav");
	PRECACHE_SOUND("scientist/sci_pain5.wav");

	// every new scientist must call this, otherwise
	// when a level is loaded, nobody will talk (time is reset to 0)
	TalkInit();

	CTalkMonster::Precache();
}

void CScientist::TalkInit()
{
	CTalkMonster::TalkInit();

	// scientist will try to talk to friends in this order:

	m_szFriends[0] = "monster_scientist";
	m_szFriends[1] = "monster_sitting_scientist";
	m_szFriends[2] = "monster_barney";

	// scientists speach group names (group names are in sentences.txt)

	m_szGrp[TLK_ANSWER] = "SC_ANSWER";
	m_szGrp[TLK_QUESTION] = "SC_QUESTION";
	m_szGrp[TLK_IDLE] = "SC_IDLE";
	m_szGrp[TLK_STARE] = "SC_STARE";
	m_szGrp[TLK_USE] = "SC_OK";
	m_szGrp[TLK_UNUSE] = "SC_WAIT";
	m_szGrp[TLK_STOP] = "SC_STOP";
	m_szGrp[TLK_NOSHOOT] = "SC_SCARED";
	m_szGrp[TLK_HELLO] = "SC_HELLO";

	m_szGrp[TLK_PLHURT1] = "!SC_CUREA";
	m_szGrp[TLK_PLHURT2] = "!SC_CUREB";
	m_szGrp[TLK_PLHURT3] = "!SC_CUREC";

	m_szGrp[TLK_PHELLO] = "SC_PHELLO";
	m_szGrp[TLK_PIDLE] = "SC_PIDLE";
	m_szGrp[TLK_PQUESTION] = "SC_PQUEST";
	m_szGrp[TLK_SMELL] = "SC_SMELL";

	m_szGrp[TLK_WOUND] = "SC_WOUND";
	m_szGrp[TLK_MORTAL] = "SC_MORTAL";

	// get voice for head
	switch (GetBodygroup(SCIENTIST_BODYGROUP_HEAD) % 3)
	{
	default:
	case SCIENTIST_HEAD_GLASSES:	m_voicePitch = 105; break;
	case SCIENTIST_HEAD_EINSTEIN:	m_voicePitch = 100; break;
	case SCIENTIST_HEAD_LUTHER:		m_voicePitch = 95; break;
	case SCIENTIST_HEAD_SLICK:		m_voicePitch = 100; break;
	}
}

bool CScientist::TakeDamage(const TakeDamageInfo& info)
{
	if (info.GetInflictor() && info.GetInflictor()->pev->flags & FL_CLIENT)
	{
		Remember(bits_MEMORY_PROVOKED);
		StopFollowing(true);
	}

	// make sure friends talk about it if player hurts scientist...
	return CTalkMonster::TakeDamage(info);
}

int CScientist::SoundMask()
{
	return bits_SOUND_WORLD |
		bits_SOUND_COMBAT |
		bits_SOUND_CARCASS |
		bits_SOUND_MEAT |
		bits_SOUND_GARBAGE |
		bits_SOUND_DANGER |
		bits_SOUND_PLAYER;
}

void CScientist::PainSound()
{
	if (gpGlobals->time < m_painTime)
		return;

	m_painTime = gpGlobals->time + RANDOM_FLOAT(0.5, 0.75);

	switch (RANDOM_LONG(0, 4))
	{
	case 0: EmitSound(SoundChannel::Voice, "scientist/sci_pain1.wav", VOL_NORM, ATTN_NORM, GetVoicePitch()); break;
	case 1: EmitSound(SoundChannel::Voice, "scientist/sci_pain2.wav", VOL_NORM, ATTN_NORM, GetVoicePitch()); break;
	case 2: EmitSound(SoundChannel::Voice, "scientist/sci_pain3.wav", VOL_NORM, ATTN_NORM, GetVoicePitch()); break;
	case 3: EmitSound(SoundChannel::Voice, "scientist/sci_pain4.wav", VOL_NORM, ATTN_NORM, GetVoicePitch()); break;
	case 4: EmitSound(SoundChannel::Voice, "scientist/sci_pain5.wav", VOL_NORM, ATTN_NORM, GetVoicePitch()); break;
	}
}

void CScientist::DeathSound()
{
	PainSound();
}

void CScientist::Killed(const KilledInfo& info)
{
	SetUse(nullptr);
	CTalkMonster::Killed(info);
}

void CScientist::SetActivity(Activity newActivity)
{
	const int iSequence = LookupActivity(newActivity);

	// Set to the desired anim, or default anim if the desired is not present
	if (iSequence == ACTIVITY_NOT_AVAILABLE)
		newActivity = ACT_IDLE;
	CTalkMonster::SetActivity(newActivity);
}

Schedule_t* CScientist::GetScheduleOfType(int Type)
{
	Schedule_t* psched;

	switch (Type)
	{
		// Hook these to make a looping schedule
	case SCHED_TARGET_FACE:
		// call base class default so that scientist will talk
		// when 'used' 
		psched = CTalkMonster::GetScheduleOfType(Type);

		if (psched == slIdleStand)
			return slFaceTarget;	// override this for different target face behavior
		else
			return psched;

	case SCHED_TARGET_CHASE:
		return slFollow;

	case SCHED_CANT_FOLLOW:
		return slStopFollowing;

	case SCHED_PANIC:
		return slSciPanic;

	case SCHED_TARGET_CHASE_SCARED:
		return slFollowScared;

	case SCHED_TARGET_FACE_SCARED:
		return slFaceTargetScared;

	case SCHED_IDLE_STAND:
		// call base class default so that scientist will talk
		// when standing during idle
		psched = CTalkMonster::GetScheduleOfType(Type);

		if (psched == slIdleStand)
			return slIdleSciStand;
		else
			return psched;

	case SCHED_HIDE:
		return slScientistHide;

	case SCHED_STARTLE:
		return slScientistStartle;

	case SCHED_FEAR:
		return slFear;
	}

	return CTalkMonster::GetScheduleOfType(Type);
}

Schedule_t* CScientist::GetSchedule()
{
	if (HasConditions(bits_COND_HEAR_SOUND))
	{
		CSound* pSound = BestSound();

		ASSERT(pSound != nullptr);
		if (pSound && (pSound->m_iType & bits_SOUND_DANGER))
			return GetScheduleOfType(SCHED_TAKE_COVER_FROM_BEST_SOUND);
	}

	switch (m_MonsterState)
	{
	case NPCState::Alert:
	case NPCState::Idle:
		if (CBaseEntity* pEnemy = m_hEnemy; pEnemy)
		{
			if (HasConditions(bits_COND_SEE_ENEMY))
				m_fearTime = gpGlobals->time;
			else if (DisregardEnemy(pEnemy))		// After 15 seconds of being hidden, return to alert
			{
				m_hEnemy = nullptr;
				pEnemy = nullptr;
			}
		}

		if (HasConditions(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE))
		{
			// flinch if hurt
			return GetScheduleOfType(SCHED_SMALL_FLINCH);
		}

		// Cower when you hear something scary
		if (HasConditions(bits_COND_HEAR_SOUND))
		{
			CSound* pSound = BestSound();

			ASSERT(pSound != nullptr);
			if (pSound)
			{
				if (pSound->m_iType & (bits_SOUND_DANGER | bits_SOUND_COMBAT))
				{
					if (gpGlobals->time - m_fearTime > 3)	// Only cower every 3 seconds or so
					{
						m_fearTime = gpGlobals->time;		// Update last fear
						return GetScheduleOfType(SCHED_STARTLE);	// This will just duck for a second
					}
				}
			}
		}

		// Behavior for following the player
		if (IsFollowing())
		{
			if (!m_hTargetEnt->IsAlive())
			{
				// UNDONE: Comment about the recently dead player here?
				StopFollowing(false);
				break;
			}

			Relationship relationship = Relationship::None;

			// Nothing scary, just me and the player
			if (CBaseEntity* pEnemy = m_hEnemy; pEnemy != nullptr)
				relationship = GetRelationship(pEnemy);

			// UNDONE: Model fear properly, fix R_FR and add multiple levels of fear
			if (relationship != Relationship::Dislike && relationship != Relationship::Hate)
			{
				// If I'm already close enough to my target
				if (TargetDistance() <= 128)
				{
					if (CanHeal())	// Heal opportunistically
						return slHeal;
					if (HasConditions(bits_COND_CLIENT_PUSH))	// Player wants me to move
						return GetScheduleOfType(SCHED_MOVE_AWAY_FOLLOW);
				}
				return GetScheduleOfType(SCHED_TARGET_FACE);	// Just face and follow.
			}
			else	// UNDONE: When afraid, scientist won't move out of your way.  Keep This?  If not, write move away scared
			{
				if (HasConditions(bits_COND_NEW_ENEMY)) // I just saw something new and scary, react
					return GetScheduleOfType(SCHED_FEAR);					// React to something scary
				return GetScheduleOfType(SCHED_TARGET_FACE_SCARED);	// face and follow, but I'm scared!
			}
		}

		if (HasConditions(bits_COND_CLIENT_PUSH))	// Player wants me to move
			return GetScheduleOfType(SCHED_MOVE_AWAY);

		// try to say something about smells
		TrySmellTalk();
		break;
	case NPCState::Combat:
		if (HasConditions(bits_COND_NEW_ENEMY))
			return slFear;					// Point and scream!
		if (HasConditions(bits_COND_SEE_ENEMY))
			return slScientistCover;		// Take Cover

		if (HasConditions(bits_COND_HEAR_SOUND))
			return slTakeCoverFromBestSound;	// Cower and panic from the scary sound!

		return slScientistCover;			// Run & Cower
		break;
	}

	return CTalkMonster::GetSchedule();
}

NPCState CScientist::GetIdealState()
{
	switch (m_MonsterState)
	{
	case NPCState::Alert:
	case NPCState::Idle:
		if (HasConditions(bits_COND_NEW_ENEMY))
		{
			if (IsFollowing())
			{
				const Relationship relationship = GetRelationship(m_hEnemy);
				if (relationship != Relationship::Fear || relationship != Relationship::Hate && !HasConditions(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE))
				{
					// Don't go to combat if you're following the player
					m_IdealMonsterState = NPCState::Alert;
					return m_IdealMonsterState;
				}
				StopFollowing(true);
			}
		}
		else if (HasConditions(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE))
		{
			// Stop following if you take damage
			if (IsFollowing())
				StopFollowing(true);
		}
		break;

	case NPCState::Combat:
	{
		if (CBaseEntity* pEnemy = m_hEnemy; pEnemy != nullptr)
		{
			if (DisregardEnemy(pEnemy))		// After 15 seconds of being hidden, return to alert
			{
				// Strip enemy when going to alert
				m_IdealMonsterState = NPCState::Alert;
				m_hEnemy = nullptr;
				return m_IdealMonsterState;
			}
			// Follow if only scared a little
			if (m_hTargetEnt != nullptr)
			{
				m_IdealMonsterState = NPCState::Alert;
				return m_IdealMonsterState;
			}

			if (HasConditions(bits_COND_SEE_ENEMY))
			{
				m_fearTime = gpGlobals->time;
				m_IdealMonsterState = NPCState::Combat;
				return m_IdealMonsterState;
			}
		}
	}
	break;
	}

	return CTalkMonster::GetIdealState();
}

bool CScientist::CanHeal()
{
	if ((m_healTime > gpGlobals->time) || (m_hTargetEnt == nullptr) || (m_hTargetEnt->pev->health > (m_hTargetEnt->pev->max_health * 0.5)))
		return false;

	return true;
}

void CScientist::Heal()
{
	if (!CanHeal())
		return;

	const Vector target = m_hTargetEnt->GetAbsOrigin() - GetAbsOrigin();
	if (target.Length() > 100)
		return;

	m_hTargetEnt->GiveHealth(gSkillData.scientistHeal, DMG_GENERIC);
	// Don't heal again for 1 minute
	m_healTime = gpGlobals->time + 60;
}

int CScientist::FriendNumber(int arrayNumber)
{
	static constexpr int array[3] = {1, 2, 0};
	if (arrayNumber < 3)
		return array[arrayNumber];
	return arrayNumber;
}
