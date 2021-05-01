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

#include <limits>

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "monsters.h"
#include "schedule.h"
#include "talkmonster.h"
#include "defaultai.h"
#include "scripted.h"
#include "soundent.h"
#include "animation.h"

// NOTE: m_voicePitch & m_szGrp should be fixed up by precache each save/restore

TYPEDESCRIPTION	CTalkMonster::m_SaveData[] =
{
	DEFINE_FIELD(CTalkMonster, m_bitsSaid, FIELD_INTEGER),
	DEFINE_FIELD(CTalkMonster, m_nSpeak, FIELD_INTEGER),

	// Recalc'ed in Precache()
	//	DEFINE_FIELD( CTalkMonster, m_voicePitch, FIELD_INTEGER ),
	//	DEFINE_FIELD( CTalkMonster, m_szGrp, FIELD_??? ),
	DEFINE_FIELD(CTalkMonster, m_useTime, FIELD_TIME),
	DEFINE_FIELD(CTalkMonster, m_iszUse, FIELD_STRING),
	DEFINE_FIELD(CTalkMonster, m_iszUnUse, FIELD_STRING),
	DEFINE_FIELD(CTalkMonster, m_flLastSaidSmelled, FIELD_TIME),
	DEFINE_FIELD(CTalkMonster, m_flStopTalkTime, FIELD_TIME),
	DEFINE_FIELD(CTalkMonster, m_hTalkTarget, FIELD_EHANDLE),
};

IMPLEMENT_SAVERESTORE(CTalkMonster, CBaseMonster);

const char* CTalkMonster::m_szFriends[TLK_CFRIENDS] =
{
	"monster_barney",
	"monster_scientist",
	"monster_sitting_scientist",
};

//=========================================================
// AI Schedules Specific to talking monsters
//=========================================================

Task_t	tlIdleResponse[] =
{
	{ TASK_SET_ACTIVITY,	(float)ACT_IDLE	},// Stop and listen
	{ TASK_WAIT,			(float)0.5		},// Wait until sure it's me they are talking to
	{ TASK_TLK_EYECONTACT,	(float)0		},// Wait until speaker is done
	{ TASK_TLK_RESPOND,		(float)0		},// Wait and then say my response
	{ TASK_TLK_IDEALYAW,	(float)0		},// look at who I'm talking to
	{ TASK_FACE_IDEAL,		(float)0		},
	{ TASK_SET_ACTIVITY,	(float)ACT_SIGNAL3	},
	{ TASK_TLK_EYECONTACT,	(float)0		},// Wait until speaker is done
};

Schedule_t	slIdleResponse[] =
{
	{
		tlIdleResponse,
		ArraySize(tlIdleResponse),
		bits_COND_NEW_ENEMY |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE,
		0,
		"Idle Response"
	},
};

Task_t	tlIdleSpeak[] =
{
	{ TASK_TLK_SPEAK,		(float)0		},// question or remark
	{ TASK_TLK_IDEALYAW,	(float)0		},// look at who I'm talking to
	{ TASK_FACE_IDEAL,		(float)0		},
	{ TASK_SET_ACTIVITY,	(float)ACT_SIGNAL3	},
	{ TASK_TLK_EYECONTACT,	(float)0		},
	{ TASK_WAIT_RANDOM,		(float)0.5		},
};

Schedule_t	slIdleSpeak[] =
{
	{
		tlIdleSpeak,
		ArraySize(tlIdleSpeak),
		bits_COND_NEW_ENEMY |
		bits_COND_CLIENT_PUSH |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE,
		0,
		"Idle Speak"
	},
};

Task_t	tlIdleSpeakWait[] =
{
	{ TASK_SET_ACTIVITY,	(float)ACT_SIGNAL3	},// Stop and talk
	{ TASK_TLK_SPEAK,		(float)0		},// question or remark
	{ TASK_TLK_EYECONTACT,	(float)0		},// 
	{ TASK_WAIT,			(float)2		},// wait - used when sci is in 'use' mode to keep head turned
};

Schedule_t	slIdleSpeakWait[] =
{
	{
		tlIdleSpeakWait,
		ArraySize(tlIdleSpeakWait),
		bits_COND_NEW_ENEMY |
		bits_COND_CLIENT_PUSH |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE,
		0,
		"Idle Speak Wait"
	},
};

Task_t	tlIdleHello[] =
{
	{ TASK_SET_ACTIVITY,	(float)ACT_SIGNAL3	},// Stop and talk
	{ TASK_TLK_HELLO,		(float)0		},// Try to say hello to player
	{ TASK_TLK_EYECONTACT,	(float)0		},
	{ TASK_WAIT,			(float)0.5		},// wait a bit
	{ TASK_TLK_HELLO,		(float)0		},// Try to say hello to player
	{ TASK_TLK_EYECONTACT,	(float)0		},
	{ TASK_WAIT,			(float)0.5		},// wait a bit
	{ TASK_TLK_HELLO,		(float)0		},// Try to say hello to player
	{ TASK_TLK_EYECONTACT,	(float)0		},
	{ TASK_WAIT,			(float)0.5		},// wait a bit
	{ TASK_TLK_HELLO,		(float)0		},// Try to say hello to player
	{ TASK_TLK_EYECONTACT,	(float)0		},
	{ TASK_WAIT,			(float)0.5		},// wait a bit

};

Schedule_t	slIdleHello[] =
{
	{
		tlIdleHello,
		ArraySize(tlIdleHello),
		bits_COND_NEW_ENEMY |
		bits_COND_CLIENT_PUSH |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_HEAR_SOUND |
		bits_COND_PROVOKED,

		bits_SOUND_COMBAT,
		"Idle Hello"
	},
};

Task_t	tlIdleStopShooting[] =
{
	{ TASK_TLK_STOPSHOOTING,	(float)0		},// tell player to stop shooting friend
	// { TASK_TLK_EYECONTACT,		(float)0		},// look at the player
};

Schedule_t	slIdleStopShooting[] =
{
	{
		tlIdleStopShooting,
		ArraySize(tlIdleStopShooting),
		bits_COND_NEW_ENEMY |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_HEAR_SOUND,
		0,
		"Idle Stop Shooting"
	},
};

Task_t	tlMoveAway[] =
{
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_MOVE_AWAY_FAIL },
	{ TASK_STORE_LASTPOSITION,		(float)0		},
	{ TASK_MOVE_AWAY_PATH,			(float)100		},
	{ TASK_WALK_PATH_FOR_UNITS,		(float)100		},
	{ TASK_STOP_MOVING,				(float)0		},
	{ TASK_FACE_PLAYER,				(float)0.5 },
};

Schedule_t	slMoveAway[] =
{
	{
		tlMoveAway,
		ArraySize(tlMoveAway),
		0,
		0,
		"MoveAway"
	},
};

Task_t	tlMoveAwayFail[] =
{
	{ TASK_STOP_MOVING,				(float)0		},
	{ TASK_FACE_PLAYER,				(float)0.5		},
};

Schedule_t	slMoveAwayFail[] =
{
	{
		tlMoveAwayFail,
		ArraySize(tlMoveAwayFail),
		0,
		0,
		"MoveAwayFail"
	},
};

Task_t	tlMoveAwayFollow[] =
{
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_TARGET_FACE },
	{ TASK_STORE_LASTPOSITION,		(float)0		},
	{ TASK_MOVE_AWAY_PATH,			(float)100				},
	{ TASK_WALK_PATH_FOR_UNITS,		(float)100		},
	{ TASK_STOP_MOVING,				(float)0		},
	{ TASK_SET_SCHEDULE,			(float)SCHED_TARGET_FACE },
};

Schedule_t	slMoveAwayFollow[] =
{
	{
		tlMoveAwayFollow,
		ArraySize(tlMoveAwayFollow),
		0,
		0,
		"MoveAwayFollow"
	},
};

Task_t	tlTlkIdleWatchClient[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE	},
	{ TASK_TLK_LOOK_AT_CLIENT,	(float)6		},
};

Task_t	tlTlkIdleWatchClientStare[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE	},
	{ TASK_TLK_CLIENT_STARE,	(float)6		},
	{ TASK_TLK_STARE,			(float)0		},
	{ TASK_TLK_IDEALYAW,		(float)0		},// look at who I'm talking to
	{ TASK_FACE_IDEAL,			(float)0		},
	{ TASK_SET_ACTIVITY,		(float)ACT_SIGNAL3	},
	{ TASK_TLK_EYECONTACT,		(float)0		},
};

Schedule_t	slTlkIdleWatchClient[] =
{
	{
		tlTlkIdleWatchClient,
		ArraySize(tlTlkIdleWatchClient),
		bits_COND_NEW_ENEMY |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_HEAR_SOUND |
		bits_COND_SMELL |
		bits_COND_CLIENT_PUSH |
		bits_COND_CLIENT_UNSEEN |
		bits_COND_PROVOKED,

		bits_SOUND_COMBAT |// sound flags - change these, and you'll break the talking code.
		//bits_SOUND_PLAYER		|
		//bits_SOUND_WORLD		|

		bits_SOUND_DANGER |
		bits_SOUND_MEAT |// scents
		bits_SOUND_CARCASS |
		bits_SOUND_GARBAGE,
		"TlkIdleWatchClient"
	},

	{
		tlTlkIdleWatchClientStare,
		ArraySize(tlTlkIdleWatchClientStare),
		bits_COND_NEW_ENEMY |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_HEAR_SOUND |
		bits_COND_SMELL |
		bits_COND_CLIENT_PUSH |
		bits_COND_CLIENT_UNSEEN |
		bits_COND_PROVOKED,

		bits_SOUND_COMBAT |// sound flags - change these, and you'll break the talking code.
		//bits_SOUND_PLAYER		|
		//bits_SOUND_WORLD		|

		bits_SOUND_DANGER |
		bits_SOUND_MEAT |// scents
		bits_SOUND_CARCASS |
		bits_SOUND_GARBAGE,
		"TlkIdleWatchClientStare"
	},
};

Task_t	tlTlkIdleEyecontact[] =
{
	{ TASK_TLK_IDEALYAW,	(float)0		},// look at who I'm talking to
	{ TASK_FACE_IDEAL,		(float)0		},
	{ TASK_SET_ACTIVITY,	(float)ACT_SIGNAL3	},
	{ TASK_TLK_EYECONTACT,	(float)0		},// Wait until speaker is done
};

Schedule_t	slTlkIdleEyecontact[] =
{
	{
		tlTlkIdleEyecontact,
		ArraySize(tlTlkIdleEyecontact),
		bits_COND_NEW_ENEMY |
		bits_COND_CLIENT_PUSH |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE,
		0,
		"TlkIdleEyecontact"
	},
};

DEFINE_CUSTOM_SCHEDULES(CTalkMonster)
{
	slIdleResponse,
		slIdleSpeak,
		slIdleHello,
		slIdleSpeakWait,
		slIdleStopShooting,
		slMoveAway,
		slMoveAwayFollow,
		slMoveAwayFail,
		slTlkIdleWatchClient,
		& slTlkIdleWatchClient[1],
		slTlkIdleEyecontact,
};

IMPLEMENT_CUSTOM_SCHEDULES(CTalkMonster, CBaseMonster);

void CTalkMonster::SetActivity(Activity newActivity)
{
	if (newActivity == ACT_IDLE && IsTalking())
		newActivity = ACT_SIGNAL3;

	if (newActivity == ACT_SIGNAL3 && (LookupActivity(ACT_SIGNAL3) == ACTIVITY_NOT_AVAILABLE))
		newActivity = ACT_IDLE;

	CBaseMonster::SetActivity(newActivity);
}

void CTalkMonster::StartTask(Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_TLK_SPEAK:
		// ask question or make statement
		IdleSpeak();
		TaskComplete();
		break;

	case TASK_TLK_RESPOND:
		// respond to question
		IdleRespond();
		TaskComplete();
		break;

	case TASK_TLK_HELLO:
		// greet player
		IdleHello();
		TaskComplete();
		break;

	case TASK_TLK_STARE:
		// let the player know I know he's staring at me.
		IdleStare();
		TaskComplete();
		break;

	case TASK_FACE_PLAYER:
	case TASK_TLK_LOOK_AT_CLIENT:
	case TASK_TLK_CLIENT_STARE:
		// track head to the client for a while.
		m_flWaitFinished = gpGlobals->time + pTask->flData;
		break;

	case TASK_TLK_EYECONTACT:
		break;

	case TASK_TLK_IDEALYAW:
		if (m_hTalkTarget != nullptr)
		{
			pev->yaw_speed = 60;
			float yaw = VecToYaw(m_hTalkTarget->pev->origin - pev->origin) - pev->angles.y;

			if (yaw > 180) yaw -= 360;
			if (yaw < -180) yaw += 360;

			if (yaw < 0)
			{
				pev->ideal_yaw = std::min(yaw + 45, 0.0f) + pev->angles.y;
			}
			else
			{
				pev->ideal_yaw = std::max(yaw - 45, 0.0f) + pev->angles.y;
			}
		}
		TaskComplete();
		break;

	case TASK_TLK_HEADRESET:
		// reset head position after looking at something
		m_hTalkTarget = nullptr;
		TaskComplete();
		break;

	case TASK_TLK_STOPSHOOTING:
		// tell player to stop shooting
		PlaySentence(m_szGrp[TLK_NOSHOOT], RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_NORM);
		TaskComplete();
		break;

	case TASK_CANT_FOLLOW:
		StopFollowing(false);
		PlaySentence(m_szGrp[TLK_STOP], RANDOM_FLOAT(2, 2.5), VOL_NORM, ATTN_NORM);
		TaskComplete();
		break;

	case TASK_WALK_PATH_FOR_UNITS:
		m_movementActivity = ACT_WALK;
		break;

	case TASK_MOVE_AWAY_PATH:
	{
		Vector dir = pev->angles;
		dir.y = pev->ideal_yaw + 180;
		Vector move;

		AngleVectors(dir, &move, nullptr, nullptr);
		dir = pev->origin + move * pTask->flData;
		if (MoveToLocation(ACT_WALK, 2, dir))
		{
			TaskComplete();
		}
		else if (FindCover(pev->origin, pev->view_ofs, 0, CoverRadius()))
		{
			// then try for plain ole cover
			m_flMoveWaitFinished = gpGlobals->time + 2;
			TaskComplete();
		}
		else
		{
			// nowhere to go?
			TaskFail();
		}
	}
	break;

	case TASK_PLAY_SCRIPT:
		m_hTalkTarget = nullptr;
		CBaseMonster::StartTask(pTask);
		break;

	default:
		CBaseMonster::StartTask(pTask);
	}
}

void CTalkMonster::RunTask(Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_TLK_CLIENT_STARE:
	case TASK_TLK_LOOK_AT_CLIENT:

		CBasePlayer* pPlayer;

		// track head to the client for a while.
		if (m_MonsterState == NPCState::Idle &&
			!IsMoving() &&
			!IsTalking())
		{
			// Get edict for one player
			pPlayer = UTIL_PlayerByIndex(1);

			if (pPlayer)
			{
				IdleHeadTurn(pPlayer->pev->origin);
			}
		}
		else
		{
			// started moving or talking
			TaskFail();
			return;
		}

		if (pTask->iTask == TASK_TLK_CLIENT_STARE)
		{
			// fail out if the player looks away or moves away.
			if ((pPlayer->pev->origin - pev->origin).Length2D() > TLK_STARE_DIST)
			{
				// player moved away.
				TaskFail();
			}

			UTIL_MakeVectors(pPlayer->pev->angles);
			if (UTIL_DotPoints(pPlayer->pev->origin, pev->origin, gpGlobals->v_forward) < m_flFieldOfView)
			{
				// player looked away
				TaskFail();
			}
		}

		if (gpGlobals->time > m_flWaitFinished)
		{
			TaskComplete();
		}
		break;

	case TASK_FACE_PLAYER:
	{
		// Get edict for one player
		if (CBasePlayer* pPlayer = UTIL_PlayerByIndex(1); pPlayer)
		{
			MakeIdealYaw(pPlayer->pev->origin);
			ChangeYaw(pev->yaw_speed);
			IdleHeadTurn(pPlayer->pev->origin);
			if (gpGlobals->time > m_flWaitFinished && YawDiff() < 10)
			{
				TaskComplete();
			}
		}
		else
		{
			TaskFail();
		}
	}
	break;

	case TASK_TLK_EYECONTACT:
		if (!IsMoving() && IsTalking() && m_hTalkTarget != nullptr)
		{
			// ALERT( at_console, "waiting %f\n", m_flStopTalkTime - gpGlobals->time );
			IdleHeadTurn(m_hTalkTarget->pev->origin);
		}
		else
		{
			TaskComplete();
		}
		break;

	case TASK_WALK_PATH_FOR_UNITS:
	{
		const float distance = (m_vecLastPosition - pev->origin).Length2D();

		// Walk path until far enough away
		if (distance > pTask->flData || MovementIsComplete())
		{
			TaskComplete();
			RouteClear();		// Stop moving
		}
	}
	break;
	case TASK_WAIT_FOR_MOVEMENT:
		if (IsTalking() && m_hTalkTarget != nullptr)
		{
			// ALERT(at_console, "walking, talking\n");
			IdleHeadTurn(m_hTalkTarget->pev->origin);
		}
		else
		{
			IdleHeadTurn(pev->origin);
			// override so that during walk, a scientist may talk and greet player
			IdleHello();
			if (RANDOM_LONG(0, m_nSpeak * 20) == 0)
			{
				IdleSpeak();
			}
		}

		CBaseMonster::RunTask(pTask);
		if (TaskIsComplete())
			IdleHeadTurn(pev->origin);
		break;

	default:
		if (IsTalking() && m_hTalkTarget != nullptr)
		{
			IdleHeadTurn(m_hTalkTarget->pev->origin);
		}
		else
		{
			SetBoneController(0, 0);
		}
		CBaseMonster::RunTask(pTask);
	}
}

void CTalkMonster::Killed(const KilledInfo& info)
{
	// If a client killed me (unless I was already Barnacle'd), make everyone else mad/afraid of him
	if ((info.GetAttacker()->flags & FL_CLIENT) && m_MonsterState != NPCState::Prone)
	{
		AlertFriends();
		LimitFollowers(CBaseEntity::Instance(info.GetAttacker()), 0);
	}

	m_hTargetEnt = nullptr;
	// Don't finish that sentence
	StopTalking();
	SetUse(nullptr);
	CBaseMonster::Killed(info);
}

CBaseEntity* CTalkMonster::EnumFriends(CBaseEntity* pPrevious, int listNumber, bool bTrace)
{
	CBaseEntity* pFriend = pPrevious;
	TraceResult tr;

	const char* pszFriend = m_szFriends[FriendNumber(listNumber)];
	while (pFriend = UTIL_FindEntityByClassname(pFriend, pszFriend))
	{
		if (pFriend == this || !pFriend->IsAlive())
			// don't talk to self or dead people
			continue;
		if (bTrace)
		{
			Vector vecCheck = pFriend->pev->origin;
			vecCheck.z = pFriend->pev->absmax.z;

			UTIL_TraceLine(pev->origin, vecCheck, IgnoreMonsters::Yes, ENT(pev), &tr);
		}
		else
			tr.flFraction = 1.0;

		if (tr.flFraction == 1.0)
		{
			return pFriend;
		}
	}

	return nullptr;
}

void CTalkMonster::AlertFriends()
{
	CBaseEntity* pFriend = nullptr;

	// for each friend in this bsp...
	for (int i = 0; i < TLK_CFRIENDS; i++)
	{
		while (pFriend = EnumFriends(pFriend, i, true))
		{
			if (CBaseMonster* pMonster = pFriend->MyMonsterPointer(); pMonster->IsAlive())
			{
				// don't provoke a friend that's playing a death animation. They're a goner
				pMonster->m_afMemory |= bits_MEMORY_PROVOKED;
			}
		}
	}
}

void CTalkMonster::ShutUpFriends()
{
	CBaseEntity* pFriend = nullptr;

	// for each friend in this bsp...
	for (int i = 0; i < TLK_CFRIENDS; i++)
	{
		while (pFriend = EnumFriends(pFriend, i, true))
		{
			if (CBaseMonster* pMonster = pFriend->MyMonsterPointer(); pMonster)
			{
				pMonster->SentenceStop();
			}
		}
	}
}

void CTalkMonster::LimitFollowers(CBaseEntity* pPlayer, int maxFollowers)
{
	CBaseEntity* pFriend = nullptr;

	int count = 0;
	// for each friend in this bsp...
	for (int i = 0; i < TLK_CFRIENDS; i++)
	{
		while (pFriend = EnumFriends(pFriend, i, false))
		{
			if (CBaseMonster* pMonster = pFriend->MyMonsterPointer(); pMonster)
			{
				if (pMonster->m_hTargetEnt == pPlayer)
				{
					count++;
					if (count > maxFollowers)
						pMonster->StopFollowing(true);
				}
			}
		}
	}
}

float CTalkMonster::TargetDistance()
{
	// If we lose the player, or he dies, return a really large distance
	if (m_hTargetEnt == nullptr || !m_hTargetEnt->IsAlive())
		return std::numeric_limits<float>::max();

	return (m_hTargetEnt->pev->origin - pev->origin).Length();
}

void CTalkMonster::HandleAnimEvent(AnimationEvent& event)
{
	switch (event.event)
	{
	case SCRIPT_EVENT_SENTENCE_RND1:		// Play a named sentence group 25% of the time
		if (RANDOM_LONG(0, 99) < 75)
			break;

		[[fallthrough]];

	case SCRIPT_EVENT_SENTENCE:				// Play a named sentence group
		ShutUpFriends();
		PlaySentence(event.options, RANDOM_FLOAT(2.8, 3.4), VOL_NORM, ATTN_IDLE);
		//ALERT(at_console, "script event speak\n");
		break;

	default:
		CBaseMonster::HandleAnimEvent(event);
		break;
	}
}

void CTalkMonster::TalkInit()
{
	// every new talking monster must reset this global, otherwise
	// when a level is loaded, nobody will talk (time is reset to 0)

	CTalkMonster::g_talkWaitTime = 0;

	m_voicePitch = PITCH_NORM;
}

CBaseEntity* CTalkMonster::FindNearestFriend(bool fPlayer)
{
	CBaseEntity* pFriend = nullptr;
	CBaseEntity* pNearest = nullptr;
	float range = std::numeric_limits<float>::max();
	TraceResult tr;

	Vector vecStart = pev->origin;
	vecStart.z = pev->absmax.z;

	const int cfriends = fPlayer ? 1 : TLK_CFRIENDS;

	// for each type of friend...

	for (int i = cfriends - 1; i > -1; i--)
	{
		const char* pszFriend = fPlayer ? "player" : m_szFriends[FriendNumber(i)];

		if (!pszFriend)
			continue;

		// for each friend in this bsp...
		while (pFriend = UTIL_FindEntityByClassname(pFriend, pszFriend))
		{
			if (pFriend == this || !pFriend->IsAlive())
				// don't talk to self or dead people
				continue;

			CBaseMonster* pMonster = pFriend->MyMonsterPointer();

			// If not a monster for some reason, or in a script, or prone
			if (!pMonster || pMonster->m_MonsterState == NPCState::Script || pMonster->m_MonsterState == NPCState::Prone)
				continue;

			Vector vecCheck = pFriend->pev->origin;
			vecCheck.z = pFriend->pev->absmax.z;

			// if closer than previous friend, and in range, see if he's visible

			if (range > (vecStart - vecCheck).Length())
			{
				UTIL_TraceLine(vecStart, vecCheck, IgnoreMonsters::Yes, ENT(pev), &tr);

				if (tr.flFraction == 1.0)
				{
					// visible and in range, this is the new nearest scientist
					if ((vecStart - vecCheck).Length() < TALKRANGE_MIN)
					{
						pNearest = pFriend;
						range = (vecStart - vecCheck).Length();
					}
				}
			}
		}
	}
	return pNearest;
}

int CTalkMonster::GetVoicePitch()
{
	return m_voicePitch + RANDOM_LONG(0, 3);
}

void CTalkMonster::Touch(CBaseEntity* pOther)
{
	// Did the player touch me?
	if (pOther->IsPlayer())
	{
		// Ignore if pissed at player
		if (m_afMemory & bits_MEMORY_PROVOKED)
			return;

		// Stay put during speech
		if (IsTalking())
			return;

		// Heuristic for determining if the player is pushing me away
		const float speed = fabs(pOther->pev->velocity.x) + fabs(pOther->pev->velocity.y);
		if (speed > 50)
		{
			SetConditions(bits_COND_CLIENT_PUSH);
			MakeIdealYaw(pOther->pev->origin);
		}
	}
}

void CTalkMonster::IdleRespond()
{
	// play response
	PlaySentence(m_szGrp[TLK_ANSWER], RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_IDLE);
}

bool CTalkMonster::OkToSpeak()
{
	// if in the grip of a barnacle, don't speak
	if (m_MonsterState == NPCState::Prone || m_IdealMonsterState == NPCState::Prone)
	{
		return false;
	}

	// if not alive, certainly don't speak
	if (pev->deadflag != DeadFlag::No)
	{
		return false;
	}

	// if someone else is talking, don't speak
	if (gpGlobals->time <= CTalkMonster::g_talkWaitTime)
		return false;

	if (pev->spawnflags & SF_MONSTER_GAG)
		return false;

	if (m_MonsterState == NPCState::Prone)
		return false;

	// if player is not in pvs, don't speak
	if (!IsAlive() || IsNullEnt(FIND_CLIENT_IN_PVS(edict())))
		return false;

	// don't talk if you're in combat
	if (m_hEnemy != nullptr && IsVisible(m_hEnemy))
		return false;

	return true;
}

bool CTalkMonster::CanPlaySentence(bool fDisregardState)
{
	if (fDisregardState)
		return CBaseMonster::CanPlaySentence(fDisregardState);
	return OkToSpeak();
}

bool CTalkMonster::IdleStare()
{
	if (!OkToSpeak())
		return false;

	PlaySentence(m_szGrp[TLK_STARE], RANDOM_FLOAT(5, 7.5), VOL_NORM, ATTN_IDLE);

	m_hTalkTarget = FindNearestFriend(true);
	return true;
}

bool CTalkMonster::IdleHello()
{
	if (!OkToSpeak())
		return false;

	// if this is first time scientist has seen player, greet him
	if (!IsBitSet(m_bitsSaid, bit_saidHelloPlayer))
	{
		// get a player
		if (CBaseEntity* pPlayer = FindNearestFriend(true); pPlayer)
		{
			if (IsInViewCone(pPlayer) && IsVisible(pPlayer))
			{
				m_hTalkTarget = pPlayer;

				if (IsBitSet(pev->spawnflags, SF_MONSTER_PREDISASTER))
					PlaySentence(m_szGrp[TLK_PHELLO], RANDOM_FLOAT(3, 3.5), VOL_NORM, ATTN_IDLE);
				else
					PlaySentence(m_szGrp[TLK_HELLO], RANDOM_FLOAT(3, 3.5), VOL_NORM, ATTN_IDLE);

				SetBits(m_bitsSaid, bit_saidHelloPlayer);

				return true;
			}
		}
	}
	return false;
}

void CTalkMonster::IdleHeadTurn(Vector& vecFriend)
{
	// turn head in desired direction only if ent has a turnable head
	if (m_afCapability & bits_CAP_TURN_HEAD)
	{
		float yaw = VecToYaw(vecFriend - pev->origin) - pev->angles.y;

		if (yaw > 180) yaw -= 360;
		if (yaw < -180) yaw += 360;

		// turn towards vector
		SetBoneController(0, yaw);
	}
}

bool CTalkMonster::IdleSpeak()
{
	if (!OkToSpeak())
		return false;

	// try to start a conversation, or make statement
	const char* szIdleGroup;
	const char* szQuestionGroup;
	float duration;
	// set idle groups based on pre/post disaster
	if (IsBitSet(pev->spawnflags, SF_MONSTER_PREDISASTER))
	{
		szIdleGroup = m_szGrp[TLK_PIDLE];
		szQuestionGroup = m_szGrp[TLK_PQUESTION];
		// set global min delay for next conversation
		duration = RANDOM_FLOAT(4.8, 5.2);
	}
	else
	{
		szIdleGroup = m_szGrp[TLK_IDLE];
		szQuestionGroup = m_szGrp[TLK_QUESTION];
		// set global min delay for next conversation
		duration = RANDOM_FLOAT(2.8, 3.2);

	}

	// player using this entity is alive and wounded?
	if (CBaseEntity* pTarget = m_hTargetEnt; pTarget != nullptr)
	{
		if (pTarget->IsPlayer())
		{
			if (pTarget->IsAlive())
			{
				m_hTalkTarget = m_hTargetEnt;
				if (!IsBitSet(m_bitsSaid, bit_saidDamageHeavy) &&
					(m_hTargetEnt->pev->health <= m_hTargetEnt->pev->max_health / 8))
				{
					//EmitSound(SoundChannel::Voice, m_szGrp[TLK_PLHURT3], VOL_NORM, ATTN_IDLE, pitch);
					PlaySentence(m_szGrp[TLK_PLHURT3], duration, VOL_NORM, ATTN_IDLE);
					SetBits(m_bitsSaid, bit_saidDamageHeavy);
					return true;
				}
				else if (!IsBitSet(m_bitsSaid, bit_saidDamageMedium) &&
					(m_hTargetEnt->pev->health <= m_hTargetEnt->pev->max_health / 4))
				{
					//EmitSound(SoundChannel::Voice, m_szGrp[TLK_PLHURT2], VOL_NORM, ATTN_IDLE, pitch);
					PlaySentence(m_szGrp[TLK_PLHURT2], duration, VOL_NORM, ATTN_IDLE);
					SetBits(m_bitsSaid, bit_saidDamageMedium);
					return true;
				}
				else if (!IsBitSet(m_bitsSaid, bit_saidDamageLight) &&
					(m_hTargetEnt->pev->health <= m_hTargetEnt->pev->max_health / 2))
				{
					//EmitSound(SoundChannel::Voice, m_szGrp[TLK_PLHURT1], VOL_NORM, ATTN_IDLE, pitch);
					PlaySentence(m_szGrp[TLK_PLHURT1], duration, VOL_NORM, ATTN_IDLE);
					SetBits(m_bitsSaid, bit_saidDamageLight);
					return true;
				}
			}
			else
			{
				//!!!KELLY - here's a cool spot to have the talkmonster talk about the dead player if we want.
				// "Oh dear, Gordon Freeman is dead!" -Scientist
				// "Damn, I can't do this without you." -Barney
			}
		}
	}

	// if there is a friend nearby to speak to, play sentence, set friend's response time, return
	if (CBaseEntity* pFriend = FindNearestFriend(false); pFriend && !(pFriend->IsMoving()) && (RANDOM_LONG(0, 99) < 75))
	{
		PlaySentence(szQuestionGroup, duration, VOL_NORM, ATTN_IDLE);
		//SENTENCEG_PlayRndSz( ENT(pev), szQuestionGroup, 1.0, ATTN_IDLE, 0, pitch );

		// force friend to answer
		CTalkMonster* pTalkMonster = (CTalkMonster*)pFriend;
		m_hTalkTarget = pFriend;
		pTalkMonster->SetAnswerQuestion(this); // UNDONE: This is EVIL!!!
		pTalkMonster->m_flStopTalkTime = m_flStopTalkTime;

		m_nSpeak++;
		return true;
	}

	// otherwise, play an idle statement, try to face client when making a statement.
	if (RANDOM_LONG(0, 1))
	{
		//SENTENCEG_PlayRndSz( ENT(pev), szIdleGroup, 1.0, ATTN_IDLE, 0, pitch );
		if (CBaseEntity* pFriend = FindNearestFriend(true); pFriend)
		{
			m_hTalkTarget = pFriend;
			PlaySentence(szIdleGroup, duration, VOL_NORM, ATTN_IDLE);
			m_nSpeak++;
			return true;
		}
	}

	// didn't speak
	Talk(0);
	CTalkMonster::g_talkWaitTime = 0;
	return false;
}

void CTalkMonster::PlayScriptedSentence(const char* pszSentence, float duration, float volume, float attenuation, bool bConcurrent, CBaseEntity* pListener)
{
	if (!bConcurrent)
		ShutUpFriends();

	ClearConditions(bits_COND_CLIENT_PUSH);	// Forget about moving!  I've got something to say!
	m_useTime = gpGlobals->time + duration;
	PlaySentence(pszSentence, duration, volume, attenuation);

	m_hTalkTarget = pListener;
}

void CTalkMonster::PlaySentence(const char* pszSentence, float duration, float volume, float attenuation)
{
	if (!pszSentence)
		return;

	Talk(duration);

	CTalkMonster::g_talkWaitTime = gpGlobals->time + duration + 2.0;
	if (pszSentence[0] == '!')
		EmitSound(SoundChannel::Voice, pszSentence, volume, attenuation, GetVoicePitch());
	else
		SENTENCEG_PlayRndSz(this, pszSentence, volume, attenuation, GetVoicePitch());

	// If you say anything, don't greet the player - you may have already spoken to them
	SetBits(m_bitsSaid, bit_saidHelloPlayer);
}

void CTalkMonster::Talk(float flDuration)
{
	if (flDuration <= 0)
	{
		// no duration :( 
		m_flStopTalkTime = gpGlobals->time + 3;
	}
	else
	{
		m_flStopTalkTime = gpGlobals->time + flDuration;
	}
}

void CTalkMonster::SetAnswerQuestion(CTalkMonster* pSpeaker)
{
	if (!m_hCine)
		ChangeSchedule(slIdleResponse);
	m_hTalkTarget = (CBaseMonster*)pSpeaker;
}

bool CTalkMonster::TakeDamage(const TakeDamageInfo& info)
{
	if (IsAlive())
	{
		// if player damaged this entity, have other friends talk about it
		if (info.GetAttacker() && m_MonsterState != NPCState::Prone && IsBitSet(info.GetAttacker()->flags, FL_CLIENT))
		{
			if (CBaseEntity* pFriend = FindNearestFriend(false); pFriend && pFriend->IsAlive())
			{
				// only if not dead or dying!
				CTalkMonster* pTalkMonster = (CTalkMonster*)pFriend;
				pTalkMonster->ChangeSchedule(slIdleStopShooting);
			}
		}
	}
	return CBaseMonster::TakeDamage(info);
}

Schedule_t* CTalkMonster::GetScheduleOfType(int Type)
{
	switch (Type)
	{
	case SCHED_MOVE_AWAY:
		return slMoveAway;

	case SCHED_MOVE_AWAY_FOLLOW:
		return slMoveAwayFollow;

	case SCHED_MOVE_AWAY_FAIL:
		return slMoveAwayFail;

	case SCHED_TARGET_FACE:
		// speak during 'use'
		if (RANDOM_LONG(0, 99) < 2)
			//ALERT ( at_console, "target chase speak\n" );
			return slIdleSpeakWait;
		else
			return slIdleStand;

	case SCHED_IDLE_STAND:
	{
		// if never seen player, try to greet him
		if (!IsBitSet(m_bitsSaid, bit_saidHelloPlayer))
		{
			return slIdleHello;
		}

		// sustained light wounds?
		if (!IsBitSet(m_bitsSaid, bit_saidWoundLight) && (pev->health <= (pev->max_health * 0.75)))
		{
			//SENTENCEG_PlayRndSz( ENT(pev), m_szGrp[TLK_WOUND], 1.0, ATTN_IDLE, 0, GetVoicePitch() );
			//CTalkMonster::g_talkWaitTime = gpGlobals->time + RANDOM_FLOAT(2.8, 3.2);
			PlaySentence(m_szGrp[TLK_WOUND], RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_IDLE);
			SetBits(m_bitsSaid, bit_saidWoundLight);
			return slIdleStand;
		}
		// sustained heavy wounds?
		else if (!IsBitSet(m_bitsSaid, bit_saidWoundHeavy) && (pev->health <= (pev->max_health * 0.5)))
		{
			//SENTENCEG_PlayRndSz( ENT(pev), m_szGrp[TLK_MORTAL], 1.0, ATTN_IDLE, 0, GetVoicePitch() );
			//CTalkMonster::g_talkWaitTime = gpGlobals->time + RANDOM_FLOAT(2.8, 3.2);
			PlaySentence(m_szGrp[TLK_MORTAL], RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_IDLE);
			SetBits(m_bitsSaid, bit_saidWoundHeavy);
			return slIdleStand;
		}

		// talk about world
		if (OkToSpeak() && RANDOM_LONG(0, m_nSpeak * 2) == 0)
		{
			//ALERT ( at_console, "standing idle speak\n" );
			return slIdleSpeak;
		}

		if (!IsTalking() && HasConditions(bits_COND_SEE_CLIENT) && RANDOM_LONG(0, 6) == 0)
		{
			if (CBasePlayer* pPlayer = UTIL_PlayerByIndex(1); pPlayer)
			{
				// watch the client.
				UTIL_MakeVectors(pPlayer->pev->angles);
				if ((pPlayer->pev->origin - pev->origin).Length2D() < TLK_STARE_DIST &&
					UTIL_DotPoints(pPlayer->pev->origin, pev->origin, gpGlobals->v_forward) >= m_flFieldOfView)
				{
					// go into the special STARE schedule if the player is close, and looking at me too.
					return &slTlkIdleWatchClient[1];
				}

				return slTlkIdleWatchClient;
			}
		}
		else
		{
			if (IsTalking())
				// look at who we're talking to
				return slTlkIdleEyecontact;
			else
				// regular standing idle
				return slIdleStand;
		}

		// NOTE - caller must first CTalkMonster::GetScheduleOfType, 
		// then check result and decide what to return ie: if sci gets back
		// slIdleStand, return slIdleSciStand
	}
	break;
	}

	return CBaseMonster::GetScheduleOfType(Type);
}

bool CTalkMonster::IsTalking()
{
	return m_flStopTalkTime > gpGlobals->time;
}

void CTalkMonster::PrescheduleThink()
{
	if (!HasConditions(bits_COND_SEE_CLIENT))
	{
		SetConditions(bits_COND_CLIENT_UNSEEN);
	}
}

void CTalkMonster::TrySmellTalk()
{
	if (!OkToSpeak())
		return;

	// clear smell bits periodically
	if (gpGlobals->time > m_flLastSaidSmelled)
	{
		//		ALERT ( at_aiconsole, "Clear smell bits\n" );
		ClearBits(m_bitsSaid, bit_saidSmelled);
	}
	// smelled something?
	if (!IsBitSet(m_bitsSaid, bit_saidSmelled) && HasConditions(bits_COND_SMELL))
	{
		PlaySentence(m_szGrp[TLK_SMELL], RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_IDLE);
		m_flLastSaidSmelled = gpGlobals->time + 60;// don't talk about the stinky for a while.
		SetBits(m_bitsSaid, bit_saidSmelled);
	}
}

Relationship CTalkMonster::GetRelationship(CBaseEntity* pTarget)
{
	if (pTarget->IsPlayer())
		if (m_afMemory & bits_MEMORY_PROVOKED)
			return Relationship::Hate;
	return CBaseMonster::GetRelationship(pTarget);
}

void CTalkMonster::StopFollowing(bool clearSchedule)
{
	if (IsFollowing())
	{
		if (!(m_afMemory & bits_MEMORY_PROVOKED))
		{
			PlaySentence(m_szGrp[TLK_UNUSE], RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_IDLE);
			m_hTalkTarget = m_hTargetEnt;
		}

		if (m_movementGoal == MOVEGOAL_TARGETENT)
			RouteClear(); // Stop him from walking toward the player
		m_hTargetEnt = nullptr;
		if (clearSchedule)
			ClearSchedule();
		if (m_hEnemy != nullptr)
			m_IdealMonsterState = NPCState::Combat;
	}
}

void CTalkMonster::StartFollowing(CBaseEntity* pLeader)
{
	if (auto cine = m_hCine.Get(); cine)
		cine->CancelScript();

	if (m_hEnemy != nullptr)
		m_IdealMonsterState = NPCState::Alert;

	m_hTargetEnt = pLeader;
	PlaySentence(m_szGrp[TLK_USE], RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_IDLE);
	m_hTalkTarget = m_hTargetEnt;
	ClearConditions(bits_COND_CLIENT_PUSH);
	ClearSchedule();
}

bool CTalkMonster::CanFollow()
{
	if (m_MonsterState == NPCState::Script)
	{
		if (!m_hCine->CanInterrupt())
			return false;
	}

	if (!IsAlive())
		return false;

	return !IsFollowing();
}

void CTalkMonster::FollowerUse(const UseInfo& info)
{
	// Don't allow use during a scripted_sentence
	if (m_useTime > gpGlobals->time)
		return;

	if (auto pCaller = info.GetCaller(); pCaller != nullptr && pCaller->IsPlayer())
	{
		// Pre-disaster followers can't be used
		if (pev->spawnflags & SF_MONSTER_PREDISASTER)
		{
			DeclineFollowing();
		}
		else if (CanFollow())
		{
			LimitFollowers(pCaller, 1);

			if (m_afMemory & bits_MEMORY_PROVOKED)
				ALERT(at_console, "I'm not following you, you evil person!\n");
			else
			{
				StartFollowing(pCaller);
				SetBits(m_bitsSaid, bit_saidHelloPlayer);	// Don't say hi after you've started following
			}
		}
		else
		{
			StopFollowing(true);
		}
	}
}

void CTalkMonster::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "UseSentence"))
	{
		m_iszUse = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "UnUseSentence"))
	{
		m_iszUnUse = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CBaseMonster::KeyValue(pkvd);
}

void CTalkMonster::Precache()
{
	if (!IsStringNull(m_iszUse))
		m_szGrp[TLK_USE] = STRING(m_iszUse);
	if (!IsStringNull(m_iszUnUse))
		m_szGrp[TLK_UNUSE] = STRING(m_iszUnUse);
}
