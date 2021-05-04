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

#include "defaultai.h"
#include "scripted.h"
#include "animation.h"

constexpr int SCIENTIST_BODYGROUP_HEAD = 1;
constexpr int SCIENTIST_BODYGROUP_NEEDLE = 2;

constexpr int SCIENTIST_HEAD_GLASSES = 0;
constexpr int SCIENTIST_HEAD_EINSTEIN = 1;
constexpr int SCIENTIST_HEAD_LUTHER = 2;
constexpr int SCIENTIST_HEAD_SLICK = 3;

constexpr int SCIENTIST_NEEDLE_OFF = 0;
constexpr int SCIENTIST_NEEDLE_ON = 1;

constexpr int NUM_SCIENTIST_HEADS = 4; // four heads available for scientist model

enum
{
	SCHED_HIDE = LAST_TALKMONSTER_SCHEDULE + 1,
	SCHED_FEAR,
	SCHED_PANIC,
	SCHED_STARTLE,
	SCHED_TARGET_CHASE_SCARED,
	SCHED_TARGET_FACE_SCARED,
};

enum
{
	TASK_SAY_HEAL = LAST_TALKMONSTER_TASK + 1,
	TASK_HEAL,
	TASK_SAY_FEAR,
	TASK_RUN_PATH_SCARED,
	TASK_SCREAM,
	TASK_RANDOM_SCREAM,
	TASK_MOVE_TO_TARGET_RANGE_SCARED,
};

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
constexpr int SCIENTIST_AE_HEAL = 1;
constexpr int SCIENTIST_AE_NEEDLEON = 2;
constexpr int SCIENTIST_AE_NEEDLEOFF = 3;

/**
*	@brief human scientist (passive lab worker)
*/
class CScientist : public CTalkMonster
{
public:
	void Spawn() override;
	void Precache() override;

	void SetYawSpeed() override;
	int  Classify() override;
	void HandleAnimEvent(AnimationEvent& event) override;
	void RunTask(Task_t* pTask) override;
	void StartTask(Task_t* pTask) override;
	int	ObjectCaps() override { return CTalkMonster::ObjectCaps() | FCAP_IMPULSE_USE; }
	bool TakeDamage(const TakeDamageInfo& info) override;
	int FriendNumber(int arrayNumber) override;
	void SetActivity(Activity newActivity) override;
	Activity GetStoppedActivity() override;
	int SoundMask() override;
	void DeclineFollowing() override;

	float	CoverRadius() override { return 1200; }		// Need more room for cover because scientists want to get far away!
	bool	DisregardEnemy(CBaseEntity* pEnemy) { return !pEnemy->IsAlive() || (gpGlobals->time - m_fearTime) > 15; }

	bool	CanHeal();
	void	Heal();
	void	Scream();

	// Override these to set behavior
	Schedule_t* GetScheduleOfType(int Type) override;
	Schedule_t* GetSchedule() override;
	NPCState GetIdealState() override;

	void DeathSound() override;
	void PainSound() override;

	void TalkInit();

	void Killed(const KilledInfo& info) override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	CUSTOM_SCHEDULES;

private:
	float m_painTime;
	float m_healTime;
	float m_fearTime;
};

LINK_ENTITY_TO_CLASS(monster_scientist, CScientist);

TYPEDESCRIPTION	CScientist::m_SaveData[] =
{
	DEFINE_FIELD(CScientist, m_painTime, FIELD_TIME),
	DEFINE_FIELD(CScientist, m_healTime, FIELD_TIME),
	DEFINE_FIELD(CScientist, m_fearTime, FIELD_TIME),
};

IMPLEMENT_SAVERESTORE(CScientist, CTalkMonster);

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

class CDeadScientist : public CBaseMonster
{
public:
	void Spawn() override;
	int	Classify() override { return	CLASS_HUMAN_PASSIVE; }

	void KeyValue(KeyValueData* pkvd) override;
	int	m_iPose;// which sequence to display
	static const char* m_szPoses[7];
};

const char* CDeadScientist::m_szPoses[] = {"lying_on_back", "lying_on_stomach", "dead_sitting", "dead_hang", "dead_table1", "dead_table2", "dead_table3"};

void CDeadScientist::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "pose"))
	{
		m_iPose = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CBaseMonster::KeyValue(pkvd);
}

LINK_ENTITY_TO_CLASS(monster_scientist_dead, CDeadScientist);

void CDeadScientist::Spawn()
{
	PRECACHE_MODEL("models/scientist.mdl");
	SetModel("models/scientist.mdl");

	pev->effects = 0;
	pev->sequence = 0;
	// Corpses have less health
	pev->health = 8;//gSkillData.scientistHealth;

	m_bloodColor = BLOOD_COLOR_RED;

	if (pev->body == -1)
	{// -1 chooses a random head
		pev->body = RANDOM_LONG(0, NUM_SCIENTIST_HEADS - 1);// pick a head, any head
	}

	//Remap old bodies to new ones
	SetBodygroup(SCIENTIST_BODYGROUP_HEAD, pev->body);

	// Luther is black, make his hands black
	if (GetBodygroup(SCIENTIST_BODYGROUP_HEAD) == SCIENTIST_HEAD_LUTHER)
		pev->skin = 1;
	else
		pev->skin = 0;

	pev->sequence = LookupSequence(m_szPoses[m_iPose]);
	if (pev->sequence == -1)
	{
		ALERT(at_console, "Dead scientist with bad pose\n");
	}

	//	pev->skin += 2; // use bloody skin -- UNDONE: Turn this back on when we have a bloody skin again!
	MonsterInitDead();
}

/**
*	@brief kdb: changed from public CBaseMonster so he can speak
*/
class CSittingScientist : public CScientist
{
public:
	void Spawn() override;
	void  Precache() override;

	/**
	*	@brief sit, do stuff
	*/
	void EXPORT SittingThink();

	/**
	*	@brief ID as a passive human
	*/
	int	Classify() override;
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	/**
	*	@brief prepare sitting scientist to answer a question
	*/
	void SetAnswerQuestion(CTalkMonster* pSpeaker) override;
	int FriendNumber(int arrayNumber) override;

	/**
	*	@brief ask question of nearby friend, or make statement
	*/
	int IdleSpeak();
	int		m_baseSequence;
	int		m_headTurn;
	float	m_flResponseDelay;
};

LINK_ENTITY_TO_CLASS(monster_sitting_scientist, CSittingScientist);
TYPEDESCRIPTION	CSittingScientist::m_SaveData[] =
{
	// Don't need to save/restore m_baseSequence (recalced)
	DEFINE_FIELD(CSittingScientist, m_headTurn, FIELD_INTEGER),
	DEFINE_FIELD(CSittingScientist, m_flResponseDelay, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CSittingScientist, CScientist);

// animation sequence aliases 
enum SITTING_ANIM
{
	SITTING_ANIM_sitlookleft,
	SITTING_ANIM_sitlookright,
	SITTING_ANIM_sitscared,
	SITTING_ANIM_sitting2,
	SITTING_ANIM_sitting3
};

void CSittingScientist::Spawn()
{
	PRECACHE_MODEL("models/scientist.mdl");
	SetModel("models/scientist.mdl");
	Precache();
	InitBoneControllers();

	SetSize(Vector(-14, -14, 0), Vector(14, 14, 36));

	SetSolidType(Solid::SlideBox);
	SetMovetype(Movetype::Step);
	pev->effects = 0;
	pev->health = 50;

	m_bloodColor = BLOOD_COLOR_RED;
	m_flFieldOfView = VIEW_FIELD_WIDE; // indicates the width of this monster's forward view cone ( as a dotproduct result )

	m_afCapability = bits_CAP_HEAR | bits_CAP_TURN_HEAD;

	SetBits(pev->spawnflags, SF_MONSTER_PREDISASTER); // predisaster only!

	if (pev->body == -1)
	{// -1 chooses a random head
		pev->body = RANDOM_LONG(0, NUM_SCIENTIST_HEADS - 1);// pick a head, any head
	}

	//Remap old bodies to new ones
	SetBodygroup(SCIENTIST_BODYGROUP_HEAD, pev->body);

	// Luther is black, make his hands black
	if (GetBodygroup(SCIENTIST_BODYGROUP_HEAD) == SCIENTIST_HEAD_LUTHER)
		pev->skin = 1;

	m_baseSequence = LookupSequence("sitlookleft");
	pev->sequence = m_baseSequence + RANDOM_LONG(0, 4);
	ResetSequenceInfo();

	SetThink(&CSittingScientist::SittingThink);
	pev->nextthink = gpGlobals->time + 0.1;

	DROP_TO_FLOOR(edict());
}

void CSittingScientist::Precache()
{
	m_baseSequence = LookupSequence("sitlookleft");
	TalkInit();
}

int	CSittingScientist::Classify()
{
	return CLASS_HUMAN_PASSIVE;
}

int CSittingScientist::FriendNumber(int arrayNumber)
{
	static constexpr int array[3] = {2, 1, 0};
	if (arrayNumber < 3)
		return array[arrayNumber];
	return arrayNumber;
}

void CSittingScientist::SittingThink()
{
	StudioFrameAdvance();

	// try to greet player
	if (IdleHello())
	{
		CBaseEntity* pent = FindNearestFriend(true);
		if (pent)
		{
			float yaw = VecToYaw(pent->GetAbsOrigin() - GetAbsOrigin()) - GetAbsAngles().y;

			if (yaw > 180) yaw -= 360;
			if (yaw < -180) yaw += 360;

			if (yaw > 0)
				pev->sequence = m_baseSequence + SITTING_ANIM_sitlookleft;
			else
				pev->sequence = m_baseSequence + SITTING_ANIM_sitlookright;

			ResetSequenceInfo();
			pev->frame = 0;
			SetBoneController(0, 0);
		}
	}
	else if (m_fSequenceFinished)
	{
		int i = RANDOM_LONG(0, 99);
		m_headTurn = 0;

		if (m_flResponseDelay && gpGlobals->time > m_flResponseDelay)
		{
			// respond to question
			IdleRespond();
			pev->sequence = m_baseSequence + SITTING_ANIM_sitscared;
			m_flResponseDelay = 0;
		}
		else if (i < 30)
		{
			pev->sequence = m_baseSequence + SITTING_ANIM_sitting3;

			// turn towards player or nearest friend and speak

			CBaseEntity* pent;
			if (!IsBitSet(m_bitsSaid, bit_saidHelloPlayer))
				pent = FindNearestFriend(true);
			else
				pent = FindNearestFriend(false);

			if (!IdleSpeak() || !pent)
			{
				m_headTurn = RANDOM_LONG(0, 8) * 10 - 40;
				pev->sequence = m_baseSequence + SITTING_ANIM_sitting3;
			}
			else
			{
				// only turn head if we spoke
				float yaw = VecToYaw(pent->GetAbsOrigin() - GetAbsOrigin()) - GetAbsAngles().y;

				//TODO: this keeps showing up everywhere
				if (yaw > 180) yaw -= 360;
				if (yaw < -180) yaw += 360;

				if (yaw > 0)
					pev->sequence = m_baseSequence + SITTING_ANIM_sitlookleft;
				else
					pev->sequence = m_baseSequence + SITTING_ANIM_sitlookright;

				//ALERT(at_console, "sitting speak\n");
			}
		}
		else if (i < 60)
		{
			pev->sequence = m_baseSequence + SITTING_ANIM_sitting3;
			m_headTurn = RANDOM_LONG(0, 8) * 10 - 40;
			if (RANDOM_LONG(0, 99) < 5)
			{
				//ALERT(at_console, "sitting speak2\n");
				IdleSpeak();
			}
		}
		else if (i < 80)
		{
			pev->sequence = m_baseSequence + SITTING_ANIM_sitting2;
		}
		else if (i < 100)
		{
			pev->sequence = m_baseSequence + SITTING_ANIM_sitscared;
		}

		ResetSequenceInfo();
		pev->frame = 0;
		SetBoneController(0, m_headTurn);
	}
	pev->nextthink = gpGlobals->time + 0.1;
}

void CSittingScientist::SetAnswerQuestion(CTalkMonster* pSpeaker)
{
	m_flResponseDelay = gpGlobals->time + RANDOM_FLOAT(3, 4);
	m_hTalkTarget = (CBaseMonster*)pSpeaker;
}

int CSittingScientist::IdleSpeak()
{
	// try to start a conversation, or make statement
	if (!OkToSpeak())
		return false;

	// set global min delay for next conversation
	CTalkMonster::g_talkWaitTime = gpGlobals->time + RANDOM_FLOAT(4.8, 5.2);

	const int pitch = GetVoicePitch();

	// if there is a friend nearby to speak to, play sentence, set friend's response time, return

	// try to talk to any standing or sitting scientists nearby
	if (CBaseEntity* pentFriend = FindNearestFriend(false); pentFriend && RANDOM_LONG(0, 1))
	{
		CTalkMonster* pTalkMonster = GetClassPtr((CTalkMonster*)pentFriend->pev);
		pTalkMonster->SetAnswerQuestion(this);

		IdleHeadTurn(pentFriend->GetAbsOrigin());
		SENTENCEG_PlayRndSz(this, m_szGrp[TLK_PQUESTION], 1.0, ATTN_IDLE, pitch);
		// set global min delay for next conversation
		CTalkMonster::g_talkWaitTime = gpGlobals->time + RANDOM_FLOAT(4.8, 5.2);
		return true;
	}

	// otherwise, play an idle statement
	if (RANDOM_LONG(0, 1))
	{
		SENTENCEG_PlayRndSz(this, m_szGrp[TLK_PIDLE], 1.0, ATTN_IDLE, pitch);
		// set global min delay for next conversation
		CTalkMonster::g_talkWaitTime = gpGlobals->time + RANDOM_FLOAT(4.8, 5.2);
		return true;
	}

	// never spoke
	CTalkMonster::g_talkWaitTime = 0;
	return false;
}
