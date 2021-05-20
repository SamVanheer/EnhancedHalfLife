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

#include "dll_functions.hpp"

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
constexpr int CONTROLLER_AE_HEAD_OPEN = 1;
constexpr int CONTROLLER_AE_BALL_SHOOT = 2;
constexpr int CONTROLLER_AE_SMALL_SHOOT = 3;
constexpr int CONTROLLER_AE_POWERUP_FULL = 4;
constexpr int CONTROLLER_AE_POWERUP_HALF = 5;

constexpr int CONTROLLER_FLINCH_DELAY = 2;		// at most one flinch every n secs

class CController : public CSquadMonster
{
public:
	void OnRemove() override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int  Classify() override;
	void HandleAnimEvent(AnimationEvent& event) override;

	void RunAI() override;
	bool CheckRangeAttack1(float flDot, float flDist) override;	//!< shoot a bigass energy ball out of their head
	bool CheckRangeAttack2(float flDot, float flDist) override;	//!< head
	bool CheckMeleeAttack1(float flDot, float flDist) override;	//!< block, throw
	Schedule_t* GetSchedule() override;
	Schedule_t* GetScheduleOfType(int Type) override;
	void StartTask(Task_t* pTask) override;
	void RunTask(Task_t* pTask) override;
	CUSTOM_SCHEDULES;

	void Stop() override;
	void Move(float flInterval) override;
	LocalMoveResult CheckLocalMove(const Vector& vecStart, const Vector& vecEnd, CBaseEntity* pTarget, float* pflDist) override;
	void MoveExecute(CBaseEntity* pTargetEnt, const Vector& vecDir, float flInterval) override;
	void SetActivity(Activity NewActivity) override;
	bool ShouldAdvanceRoute(float flWaypointDist) override;
	int LookupFloat();

	float m_flNextFlinch = 0;

	float m_flShootTime = 0;
	float m_flShootEnd = 0;

	void PainSound() override;
	void AlertSound() override;
	void IdleSound() override;
	void AttackSound();
	void DeathSound() override;

	static const char* pAttackSounds[];
	static const char* pIdleSounds[];
	static const char* pAlertSounds[];
	static const char* pPainSounds[];
	static const char* pDeathSounds[];

	bool TakeDamage(const TakeDamageInfo& info) override;
	void Killed(const KilledInfo& info) override;
	void GibMonster() override;

	EHandle<CSprite> m_hBall[2];	// hand balls
	int m_iBall[2]{};				// how bright it should be
	float m_iBallTime[2]{};			// when it should be that color
	int m_iBallCurrent[2]{};		// current brightness

	Vector m_vecEstVelocity;

	Vector m_velocity;
	bool m_fInCombat = false;
};

LINK_ENTITY_TO_CLASS(monster_alien_controller, CController);

TYPEDESCRIPTION	CController::m_SaveData[] =
{
	DEFINE_ARRAY(CController, m_hBall, FIELD_EHANDLE, 2),
	DEFINE_ARRAY(CController, m_iBall, FIELD_INTEGER, 2),
	DEFINE_ARRAY(CController, m_iBallTime, FIELD_TIME, 2),
	DEFINE_ARRAY(CController, m_iBallCurrent, FIELD_INTEGER, 2),
	DEFINE_FIELD(CController, m_vecEstVelocity, FIELD_VECTOR),
};

IMPLEMENT_SAVERESTORE(CController, CSquadMonster);

const char* CController::pAttackSounds[] =
{
	"controller/con_attack1.wav",
	"controller/con_attack2.wav",
	"controller/con_attack3.wav",
};

const char* CController::pIdleSounds[] =
{
	"controller/con_idle1.wav",
	"controller/con_idle2.wav",
	"controller/con_idle3.wav",
	"controller/con_idle4.wav",
	"controller/con_idle5.wav",
};

const char* CController::pAlertSounds[] =
{
	"controller/con_alert1.wav",
	"controller/con_alert2.wav",
	"controller/con_alert3.wav",
};

const char* CController::pPainSounds[] =
{
	"controller/con_pain1.wav",
	"controller/con_pain2.wav",
	"controller/con_pain3.wav",
};

const char* CController::pDeathSounds[] =
{
	"controller/con_die1.wav",
	"controller/con_die2.wav",
};

int	CController::Classify()
{
	return CLASS_ALIEN_MILITARY;
}

void CController::SetYawSpeed()
{
	pev->yaw_speed = 120;
}

bool CController::TakeDamage(const TakeDamageInfo& info)
{
	// HACK HACK -- until we fix this.
	if (IsAlive())
		PainSound();
	return CBaseMonster::TakeDamage(info);
}

void CController::OnRemove()
{
	for (auto& ballHandle : m_hBall)
	{
		ballHandle.Remove();
	}

	CSquadMonster::OnRemove();
}

void CController::Killed(const KilledInfo& info)
{
	// fade balls
	for (auto& ballHandle : m_hBall)
	{
		if (auto ball = ballHandle.Get(); ball)
		{
			ball->SUB_StartFadeOut();
			ballHandle = nullptr;
		}
	}

	CSquadMonster::Killed(info);
}

void CController::GibMonster()
{
	// delete balls
	for (auto& ballHandle : m_hBall)
	{
		ballHandle.Remove();
	}
	CSquadMonster::GibMonster();
}

void CController::PainSound()
{
	if (RANDOM_LONG(0, 5) < 2)
		EMIT_SOUND_ARRAY_DYN(SoundChannel::Voice, pPainSounds);
}

void CController::AlertSound()
{
	EMIT_SOUND_ARRAY_DYN(SoundChannel::Voice, pAlertSounds);
}

void CController::IdleSound()
{
	EMIT_SOUND_ARRAY_DYN(SoundChannel::Voice, pIdleSounds);
}

void CController::AttackSound()
{
	EMIT_SOUND_ARRAY_DYN(SoundChannel::Voice, pAttackSounds);
}

void CController::DeathSound()
{
	EMIT_SOUND_ARRAY_DYN(SoundChannel::Voice, pDeathSounds);
}

void CController::HandleAnimEvent(AnimationEvent& event)
{
	switch (event.event)
	{
	case CONTROLLER_AE_HEAD_OPEN:
	{
		Vector vecStart, angleGun;
		GetAttachment(0, vecStart, angleGun);

		MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
		WRITE_BYTE(TE_ELIGHT);
		WRITE_SHORT(entindex() + 0x1000);		// entity, attachment
		WRITE_COORD(vecStart.x);		// origin
		WRITE_COORD(vecStart.y);
		WRITE_COORD(vecStart.z);
		WRITE_COORD(1);	// radius
		WRITE_BYTE(255);	// R
		WRITE_BYTE(192);	// G
		WRITE_BYTE(64);	// B
		WRITE_BYTE(20);	// life * 10
		WRITE_COORD(-32); // decay
		MESSAGE_END();

		m_iBall[0] = 192;
		m_iBallTime[0] = gpGlobals->time + atoi(event.options) / 15.0;
		m_iBall[1] = 255;
		m_iBallTime[1] = gpGlobals->time + atoi(event.options) / 15.0;
	}
	break;

	case CONTROLLER_AE_BALL_SHOOT:
	{
		Vector vecStart, angleGun;
		GetAttachment(0, vecStart, angleGun);

		MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
		WRITE_BYTE(TE_ELIGHT);
		WRITE_SHORT(entindex() + 0x1000);		// entity, attachment
		WRITE_COORD(0);		// origin
		WRITE_COORD(0);
		WRITE_COORD(0);
		WRITE_COORD(32);	// radius
		WRITE_BYTE(255);	// R
		WRITE_BYTE(192);	// G
		WRITE_BYTE(64);	// B
		WRITE_BYTE(10);	// life * 10
		WRITE_COORD(32); // decay
		MESSAGE_END();

		CBaseMonster* pBall = (CBaseMonster*)Create("controller_head_ball", vecStart, GetAbsAngles(), this);

		pBall->SetAbsVelocity(Vector(0, 0, 32));
		pBall->m_hEnemy = m_hEnemy;

		m_iBall[0] = 0;
		m_iBall[1] = 0;
	}
	break;

	case CONTROLLER_AE_SMALL_SHOOT:
	{
		AttackSound();
		m_flShootTime = gpGlobals->time;
		m_flShootEnd = m_flShootTime + atoi(event.options) / 15.0;
	}
	break;
	case CONTROLLER_AE_POWERUP_FULL:
	{
		m_iBall[0] = 255;
		m_iBallTime[0] = gpGlobals->time + atoi(event.options) / 15.0;
		m_iBall[1] = 255;
		m_iBallTime[1] = gpGlobals->time + atoi(event.options) / 15.0;
	}
	break;
	case CONTROLLER_AE_POWERUP_HALF:
	{
		m_iBall[0] = 192;
		m_iBallTime[0] = gpGlobals->time + atoi(event.options) / 15.0;
		m_iBall[1] = 192;
		m_iBallTime[1] = gpGlobals->time + atoi(event.options) / 15.0;
	}
	break;
	default:
		CBaseMonster::HandleAnimEvent(event);
		break;
	}
}

void CController::Spawn()
{
	Precache();

	SetModel("models/controller.mdl");
	SetSize(Vector(-32, -32, 0), Vector(32, 32, 64));

	SetSolidType(Solid::SlideBox);
	SetMovetype(Movetype::Fly);
	pev->flags |= FL_FLY;
	m_bloodColor = BLOOD_COLOR_GREEN;
	pev->health = gSkillData.controllerHealth;
	pev->view_ofs = Vector(0, 0, -2);// position of the eyes relative to monster's origin.
	m_flFieldOfView = VIEW_FIELD_FULL;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState = NPCState::None;

	MonsterInit();
}

void CController::Precache()
{
	PRECACHE_MODEL("models/controller.mdl");

	PRECACHE_SOUND_ARRAY(pAttackSounds);
	PRECACHE_SOUND_ARRAY(pIdleSounds);
	PRECACHE_SOUND_ARRAY(pAlertSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pDeathSounds);

	PRECACHE_MODEL("sprites/xspark4.spr");

	UTIL_PrecacheOther("controller_energy_ball");
	UTIL_PrecacheOther("controller_head_ball");
}

//=========================================================
// AI Schedules Specific to this monster
//=========================================================
Task_t tlControllerChaseEnemy[] =
{
	{ TASK_GET_PATH_TO_ENEMY,	(float)128		},
	{ TASK_WAIT_FOR_MOVEMENT,	(float)0		},
};

/**
*	@brief Chase enemy schedule
*/
Schedule_t slControllerChaseEnemy[] =
{
	{
		tlControllerChaseEnemy,
		ArraySize(tlControllerChaseEnemy),
		bits_COND_NEW_ENEMY |
		bits_COND_TASK_FAILED,
		0,
		"ControllerChaseEnemy"
	},
};

Task_t	tlControllerStrafe[] =
{
	{ TASK_WAIT,					(float)0.2					},
	{ TASK_GET_PATH_TO_ENEMY,		(float)128					},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0					},
	{ TASK_WAIT,					(float)1					},
};

Schedule_t	slControllerStrafe[] =
{
	{
		tlControllerStrafe,
		ArraySize(tlControllerStrafe),
		bits_COND_NEW_ENEMY,
		0,
		"ControllerStrafe"
	},
};

Task_t	tlControllerTakeCover[] =
{
	{ TASK_WAIT,					(float)0.2					},
	{ TASK_FIND_COVER_FROM_ENEMY,	(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0					},
	{ TASK_WAIT,					(float)1					},
};

Schedule_t	slControllerTakeCover[] =
{
	{
		tlControllerTakeCover,
		ArraySize(tlControllerTakeCover),
		bits_COND_NEW_ENEMY,
		0,
		"ControllerTakeCover"
	},
};

Task_t	tlControllerFail[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT,				(float)2		},
	{ TASK_WAIT_PVS,			(float)0		},
};

Schedule_t	slControllerFail[] =
{
	{
		tlControllerFail,
		ArraySize(tlControllerFail),
		0,
		0,
		"ControllerFail"
	},
};

DEFINE_CUSTOM_SCHEDULES(CController)
{
	slControllerChaseEnemy,
		slControllerStrafe,
		slControllerTakeCover,
		slControllerFail,
};

IMPLEMENT_CUSTOM_SCHEDULES(CController, CSquadMonster);

void CController::StartTask(Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_RANGE_ATTACK1:
		CSquadMonster::StartTask(pTask);
		break;
	case TASK_GET_PATH_TO_ENEMY_LKP:
	{
		if (BuildNearestRoute(m_vecEnemyLKP, pev->view_ofs, pTask->flData, (m_vecEnemyLKP - GetAbsOrigin()).Length() + 1024))
		{
			TaskComplete();
		}
		else
		{
			// no way to get there =(
			ALERT(at_aiconsole, "GetPathToEnemyLKP failed!!\n");
			TaskFail();
		}
		break;
	}
	case TASK_GET_PATH_TO_ENEMY:
	{
		CBaseEntity* pEnemy = m_hEnemy;

		if (pEnemy == nullptr)
		{
			TaskFail();
			return;
		}

		if (BuildNearestRoute(pEnemy->GetAbsOrigin(), pEnemy->pev->view_ofs, pTask->flData, (pEnemy->GetAbsOrigin() - GetAbsOrigin()).Length() + 1024))
		{
			TaskComplete();
		}
		else
		{
			// no way to get there =(
			ALERT(at_aiconsole, "GetPathToEnemy failed!!\n");
			TaskFail();
		}
		break;
	}
	default:
		CSquadMonster::StartTask(pTask);
		break;
	}
}

Vector Intersect(const Vector& vecSrc, const Vector& vecDst, const Vector& vecMove, float flSpeed)
{
	const Vector vecTo = vecDst - vecSrc;

	const float a = DotProduct(vecMove, vecMove) - flSpeed * flSpeed;
	const float b = 0 * DotProduct(vecTo, vecMove); // why does this work?
	const float c = DotProduct(vecTo, vecTo);

	float t;
	if (a == 0)
	{
		t = c / (flSpeed * flSpeed);
	}
	else
	{
		t = b * b - 4 * a * c;
		t = sqrt(t) / (2.0 * a);
		const float t1 = -b + t;
		const float t2 = -b - t;

		if (t1 < 0 || t2 < t1)
			t = t2;
		else
			t = t1;
	}

	// ALERT( at_console, "Intersect %f\n", t );

	if (t < 0.1)
		t = 0.1;
	if (t > 10.0)
		t = 10.0;

	const Vector vecHit = vecTo + vecMove * t;
	return vecHit.Normalize() * flSpeed;
}

int CController::LookupFloat()
{
	if (m_velocity.Length() < 32.0)
	{
		return LookupSequence("up");
	}

	UTIL_MakeAimVectors(GetAbsAngles());
	const float x = DotProduct(gpGlobals->v_forward, m_velocity);
	const float y = DotProduct(gpGlobals->v_right, m_velocity);
	const float z = DotProduct(gpGlobals->v_up, m_velocity);

	if (fabs(x) > fabs(y) && fabs(x) > fabs(z))
	{
		if (x > 0)
			return LookupSequence("forward");
		else
			return LookupSequence("backward");
	}
	else if (fabs(y) > fabs(z))
	{
		if (y > 0)
			return LookupSequence("right");
		else
			return LookupSequence("left");
	}
	else
	{
		if (z > 0)
			return LookupSequence("up");
		else
			return LookupSequence("down");
	}
}

void CController::RunTask(Task_t* pTask)
{
	if (m_flShootEnd > gpGlobals->time)
	{
		Vector vecHand, vecAngle;
		GetAttachment(2, vecHand, vecAngle);

		while (m_flShootTime < m_flShootEnd && m_flShootTime < gpGlobals->time)
		{
			Vector vecSrc = vecHand + GetAbsVelocity() * (m_flShootTime - gpGlobals->time);

			if (m_hEnemy != nullptr)
			{
				if (HasConditions(bits_COND_SEE_ENEMY))
				{
					m_vecEstVelocity = m_vecEstVelocity * 0.5 + m_hEnemy->GetAbsVelocity() * 0.5;
				}
				else
				{
					m_vecEstVelocity = m_vecEstVelocity * 0.8;
				}
				Vector vecDir = Intersect(vecSrc, m_hEnemy->BodyTarget(GetAbsOrigin()), m_vecEstVelocity, gSkillData.controllerSpeedBall);
				const float delta = 0.03490; // +-2 degree
				vecDir = vecDir + Vector(RANDOM_FLOAT(-delta, delta), RANDOM_FLOAT(-delta, delta), RANDOM_FLOAT(-delta, delta)) * gSkillData.controllerSpeedBall;

				vecSrc = vecSrc + vecDir * (gpGlobals->time - m_flShootTime);
				CBaseMonster* pBall = (CBaseMonster*)Create("controller_energy_ball", vecSrc, GetAbsAngles(), this);
				pBall->SetAbsVelocity(vecDir);
			}
			m_flShootTime += 0.2;
		}

		if (m_flShootTime > m_flShootEnd)
		{
			m_iBall[0] = 64;
			m_iBallTime[0] = m_flShootEnd;
			m_iBall[1] = 64;
			m_iBallTime[1] = m_flShootEnd;
			m_fInCombat = false;
		}
	}

	switch (pTask->iTask)
	{
	case TASK_WAIT_FOR_MOVEMENT:
	case TASK_WAIT:
	case TASK_WAIT_FACE_ENEMY:
	case TASK_WAIT_PVS:
		MakeIdealYaw(m_vecEnemyLKP);
		ChangeYaw(pev->yaw_speed);

		if (m_fSequenceFinished)
		{
			m_fInCombat = false;
		}

		CSquadMonster::RunTask(pTask);

		if (!m_fInCombat)
		{
			if (HasConditions(bits_COND_CAN_RANGE_ATTACK1))
			{
				pev->sequence = LookupActivity(ACT_RANGE_ATTACK1);
				pev->frame = 0;
				ResetSequenceInfo();
				m_fInCombat = true;
			}
			else if (HasConditions(bits_COND_CAN_RANGE_ATTACK2))
			{
				pev->sequence = LookupActivity(ACT_RANGE_ATTACK2);
				pev->frame = 0;
				ResetSequenceInfo();
				m_fInCombat = true;
			}
			else
			{
				const int iFloat = LookupFloat();
				if (m_fSequenceFinished || iFloat != pev->sequence)
				{
					pev->sequence = iFloat;
					pev->frame = 0;
					ResetSequenceInfo();
				}
			}
		}
		break;
	default:
		CSquadMonster::RunTask(pTask);
		break;
	}
}

Schedule_t* CController::GetSchedule()
{
	switch (m_MonsterState)
	{
	case NPCState::Idle:
		break;

	case NPCState::Alert:
		break;

	case NPCState::Combat:
	{
		const Vector vecTmp = Intersect(vec3_origin, Vector(100, 4, 7), Vector(2, 10, -3), 20.0);

		// dead enemy
		if (HasConditions(bits_COND_LIGHT_DAMAGE))
		{
			// m_iFrustration++;
		}
		if (HasConditions(bits_COND_HEAVY_DAMAGE))
		{
			// m_iFrustration++;
		}
	}
	break;
	}

	return CSquadMonster::GetSchedule();
}

Schedule_t* CController::GetScheduleOfType(int Type)
{
	// ALERT( at_console, "%d\n", m_iFrustration );
	switch (Type)
	{
	case SCHED_CHASE_ENEMY:
		return slControllerChaseEnemy;
	case SCHED_RANGE_ATTACK1:
		return slControllerStrafe;
	case SCHED_RANGE_ATTACK2:
	case SCHED_MELEE_ATTACK1:
	case SCHED_MELEE_ATTACK2:
	case SCHED_TAKE_COVER_FROM_ENEMY:
		return slControllerTakeCover;
	case SCHED_FAIL:
		return slControllerFail;
	}

	return CBaseMonster::GetScheduleOfType(Type);
}

bool CController::CheckRangeAttack1(float flDot, float flDist)
{
	if (flDot > 0.5 && flDist > 256 && flDist <= 2048)
	{
		return true;
	}
	return false;
}

bool CController::CheckRangeAttack2(float flDot, float flDist)
{
	if (flDot > 0.5 && flDist > 64 && flDist <= 2048)
	{
		return true;
	}
	return false;
}

bool CController::CheckMeleeAttack1(float flDot, float flDist)
{
	return false;
}

void CController::SetActivity(Activity NewActivity)
{
	CBaseMonster::SetActivity(NewActivity);

	switch (m_Activity)
	{
	case ACT_WALK:
		m_flGroundSpeed = 100;
		break;
	default:
		m_flGroundSpeed = 100;
		break;
	}
}

void CController::RunAI()
{
	CBaseMonster::RunAI();

	if (HasMemory(bits_MEMORY_KILLED))
		return;

	for (int i = 0; i < 2; i++)
	{
		auto ball = m_hBall[i].Get();

		if (ball == nullptr)
		{
			ball = m_hBall[i] = CSprite::SpriteCreate("sprites/xspark4.spr", GetAbsOrigin(), true);
			ball->SetTransparency(RenderMode::Glow, {255, 255, 255}, 255, RenderFX::NoDissipation);
			ball->SetAttachment(this, (i + 3));
			ball->SetScale(1.0);
		}

		float t = m_iBallTime[i] - gpGlobals->time;
		if (t > 0.1)
			t = 0.1 / t;
		else
			t = 1.0;

		m_iBallCurrent[i] += (m_iBall[i] - m_iBallCurrent[i]) * t;

		ball->SetBrightness(m_iBallCurrent[i]);

		Vector vecStart, angleGun;
		GetAttachment(i + 2, vecStart, angleGun);
		ball->SetAbsOrigin(vecStart);

		MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
		WRITE_BYTE(TE_ELIGHT);
		WRITE_SHORT(entindex() + 0x1000 * (i + 3));		// entity, attachment
		WRITE_COORD(vecStart.x);		// origin
		WRITE_COORD(vecStart.y);
		WRITE_COORD(vecStart.z);
		WRITE_COORD(m_iBallCurrent[i] / 8);	// radius
		WRITE_BYTE(255);	// R
		WRITE_BYTE(192);	// G
		WRITE_BYTE(64);	// B
		WRITE_BYTE(5);	// life * 10
		WRITE_COORD(0); // decay
		MESSAGE_END();
	}
}

void CController::Stop()
{
	m_IdealActivity = GetStoppedActivity();
}

constexpr int DIST_TO_CHECK = 200;
void CController::Move(float flInterval)
{
	// Don't move if no valid route
	if (IsRouteClear())
	{
		ALERT(at_aiconsole, "Tried to move with no route!\n");
		TaskFail();
		return;
	}

	if (m_flMoveWaitFinished > gpGlobals->time)
		return;

	// Debug, test movement code
#if 0
//	if ( CVAR_GET_FLOAT("stopmove" ) != 0 )
	{
		if (m_movementGoal == MOVEGOAL_ENEMY)
			RouteSimplify(m_hEnemy);
		else
			RouteSimplify(m_hTargetEnt);
		RefreshRoute();
		return;
	}
#else
// Debug, draw the route
//	DrawRoute( this, m_Route, m_iRouteIndex, 0, 0, 255 );
#endif

	// if the monster is moving directly towards an entity (enemy for instance), we'll set this pointer
	// to that entity for the CheckLocalMove and Triangulate functions.
	if (m_flGroundSpeed == 0)
	{
		m_flGroundSpeed = 100;
		// TaskFail( );
		// return;
	}

	float flMoveDist = m_flGroundSpeed * flInterval;

	float flWaypointDist = 0;
	float flCheckDist = 0;

	CBaseEntity* pTargetEnt = nullptr;

	do
	{
		// local move to waypoint.
		const Vector vecDir = (m_Route[m_iRouteIndex].vecLocation - GetAbsOrigin()).Normalize();
		flWaypointDist = (m_Route[m_iRouteIndex].vecLocation - GetAbsOrigin()).Length();

		// MakeIdealYaw ( m_Route[ m_iRouteIndex ].vecLocation );
		// ChangeYaw ( pev->yaw_speed );

		// if the waypoint is closer than CheckDist, CheckDist is the dist to waypoint
		if (flWaypointDist < DIST_TO_CHECK)
		{
			flCheckDist = flWaypointDist;
		}
		else
		{
			flCheckDist = DIST_TO_CHECK;
		}

		if ((m_Route[m_iRouteIndex].iType & (~bits_MF_NOT_TO_MASK)) == bits_MF_TO_ENEMY)
		{
			// only on a PURE move to enemy ( i.e., ONLY MF_TO_ENEMY set, not MF_TO_ENEMY and DETOUR )
			pTargetEnt = m_hEnemy;
		}
		else if ((m_Route[m_iRouteIndex].iType & ~bits_MF_NOT_TO_MASK) == bits_MF_TO_TARGETENT)
		{
			pTargetEnt = m_hTargetEnt;
		}

		// !!!BUGBUG - CheckDist should be derived from ground speed.
		// If this fails, it should be because of some dynamic entity blocking this guy.
		// We've already checked this path, so we should wait and time out if the entity doesn't move
		float flDist = 0; // how far the lookahead check got before hitting an object.
		if (CheckLocalMove(GetAbsOrigin(), GetAbsOrigin() + vecDir * flCheckDist, pTargetEnt, &flDist) != LocalMoveResult::Valid)
		{
			// Can't move, stop
			Stop();
			// Blocking entity is in global trace_ent
			CBaseEntity* pBlocker = CBaseEntity::Instance(gpGlobals->trace_ent);
			if (pBlocker)
			{
				Blocked(pBlocker);
			}
			if (pBlocker && m_moveWaitTime > 0 && pBlocker->IsMoving() && !pBlocker->IsPlayer() && (gpGlobals->time - m_flMoveWaitFinished) > 3.0f)
			{
				// Can we still move toward our target?
				if (flDist < m_flGroundSpeed)
				{
					// Wait for a second
					m_flMoveWaitFinished = gpGlobals->time + m_moveWaitTime;
					//				ALERT( at_aiconsole, "Move %s!!!\n", pBlocker->GetClassname() );
					return;
				}
			}
			else
			{
				// try to triangulate around whatever is in the way.
				Vector vecApex;
				if (Triangulate(GetAbsOrigin(), m_Route[m_iRouteIndex].vecLocation, flDist, pTargetEnt, &vecApex))
				{
					InsertWaypoint(vecApex, bits_MF_TO_DETOUR);
					RouteSimplify(pTargetEnt);
				}
				else
				{
					ALERT(at_aiconsole, "Couldn't Triangulate\n");
					Stop();
					if (m_moveWaitTime > 0)
					{
						RefreshRoute();
						m_flMoveWaitFinished = gpGlobals->time + m_moveWaitTime * 0.5;
					}
					else
					{
						TaskFail();
						ALERT(at_aiconsole, "Failed to move!\n");
						//ALERT( at_aiconsole, "%f, %f, %f\n", GetAbsOrigin().z, (GetAbsOrigin() + (vecDir * flCheckDist)).z, m_Route[m_iRouteIndex].vecLocation.z );
					}
					return;
				}
			}
		}

		// UNDONE: this is a hack to quit moving farther than it has looked ahead.
		if (flCheckDist < flMoveDist)
		{
			MoveExecute(pTargetEnt, vecDir, flCheckDist / m_flGroundSpeed);

			// ALERT( at_console, "%.02f\n", flInterval );
			AdvanceRoute(flWaypointDist);
			flMoveDist -= flCheckDist;
		}
		else
		{
			MoveExecute(pTargetEnt, vecDir, flMoveDist / m_flGroundSpeed);

			if (ShouldAdvanceRoute(flWaypointDist - flMoveDist))
			{
				AdvanceRoute(flWaypointDist);
			}
			flMoveDist = 0;
		}

		if (MovementIsComplete())
		{
			Stop();
			RouteClear();
		}
	}
	while (flMoveDist > 0 && flCheckDist > 0);

	// cut corner?
	if (flWaypointDist < 128)
	{
		if (m_movementGoal == MOVEGOAL_ENEMY)
			RouteSimplify(m_hEnemy);
		else
			RouteSimplify(m_hTargetEnt);
		RefreshRoute();

		if (m_flGroundSpeed > 100)
			m_flGroundSpeed -= 40;
	}
	else
	{
		if (m_flGroundSpeed < 400)
			m_flGroundSpeed += 10;
	}
}

bool CController::ShouldAdvanceRoute(float flWaypointDist)
{
	if (flWaypointDist <= 32)
	{
		return true;
	}

	return false;
}

LocalMoveResult CController::CheckLocalMove(const Vector& vecStart, const Vector& vecEnd, CBaseEntity* pTarget, float* pflDist)
{
	TraceResult tr;

	UTIL_TraceHull(vecStart + Vector(0, 0, 32), vecEnd + Vector(0, 0, 32), IgnoreMonsters::No, Hull::Large, this, &tr);

	// ALERT( at_console, "%.0f %.0f %.0f : ", vecStart.x, vecStart.y, vecStart.z );
	// ALERT( at_console, "%.0f %.0f %.0f\n", vecEnd.x, vecEnd.y, vecEnd.z );

	if (pflDist)
	{
		*pflDist = ((tr.vecEndPos - Vector(0, 0, 32)) - vecStart).Length();// get the distance.
	}

	// ALERT( at_console, "check %d %d %f\n", tr.fStartSolid, tr.fAllSolid, tr.flFraction );
	if (tr.fStartSolid || tr.flFraction < 1.0)
	{
		if (pTarget && pTarget == InstanceOrNull(gpGlobals->trace_ent))
			return LocalMoveResult::Valid;
		return LocalMoveResult::Invalid;
	}

	return LocalMoveResult::Valid;
}

void CController::MoveExecute(CBaseEntity* pTargetEnt, const Vector& vecDir, float flInterval)
{
	if (m_IdealActivity != m_movementActivity)
		m_IdealActivity = m_movementActivity;

	// ALERT( at_console, "move %.4f %.4f %.4f : %f\n", vecDir.x, vecDir.y, vecDir.z, flInterval );

	// float flTotal = m_flGroundSpeed * pev->framerate * flInterval;
	// UTIL_MoveToOrigin ( ENT(pev), m_Route[ m_iRouteIndex ].vecLocation, flTotal, MoveToOriginType::Strafe );

	m_velocity = m_velocity * 0.8 + m_flGroundSpeed * vecDir * 0.2;

	UTIL_MoveToOrigin(this, GetAbsOrigin() + m_velocity, m_velocity.Length() * flInterval, MoveToOriginType::Strafe);
}

/**
*	@brief Controller bouncy ball attack
*/
class CControllerHeadBall : public CBaseMonster
{
	void Spawn() override;
	void Precache() override;
	void EXPORT HuntThink();
	void EXPORT DieThink();
	void EXPORT BounceTouch(CBaseEntity* pOther);
	void MovetoTarget(Vector vecTarget);
	void Crawl();
	int m_iTrail = 0;
	int m_flNextAttack = 0;
	Vector m_vecIdeal;
	EHANDLE m_hOwner;
};

LINK_ENTITY_TO_CLASS(controller_head_ball, CControllerHeadBall);

void CControllerHeadBall::Spawn()
{
	Precache();
	// motor
	SetMovetype(Movetype::Fly);
	SetSolidType(Solid::BBox);

	SetModel("sprites/xspark4.spr");
	SetRenderMode(RenderMode::TransAdd);
	SetRenderColor({255, 255, 255});
	SetRenderAmount(255);
	pev->scale = 2.0;

	SetSize(vec3_origin, vec3_origin);
	SetAbsOrigin(GetAbsOrigin());

	SetThink(&CControllerHeadBall::HuntThink);
	SetTouch(&CControllerHeadBall::BounceTouch);

	m_vecIdeal = vec3_origin;

	pev->nextthink = gpGlobals->time + 0.1;

	m_hOwner = InstanceOrWorld(pev->owner);
	pev->dmgtime = gpGlobals->time;
}

void CControllerHeadBall::Precache()
{
	PRECACHE_MODEL("sprites/xspark1.spr");
	PRECACHE_SOUND("debris/zap4.wav");
	PRECACHE_SOUND("weapons/electro4.wav");
}

void CControllerHeadBall::HuntThink()
{
	pev->nextthink = gpGlobals->time + 0.1;

	SetRenderAmount(GetRenderAmount() - 5);

	MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
	WRITE_BYTE(TE_ELIGHT);
	WRITE_SHORT(entindex());		// entity, attachment
	WRITE_COORD(GetAbsOrigin().x);		// origin
	WRITE_COORD(GetAbsOrigin().y);
	WRITE_COORD(GetAbsOrigin().z);
	WRITE_COORD(GetRenderAmount() / 16);	// radius
	WRITE_BYTE(255);	// R
	WRITE_BYTE(255);	// G
	WRITE_BYTE(255);	// B
	WRITE_BYTE(2);	// life * 10
	WRITE_COORD(0); // decay
	MESSAGE_END();

	// check world boundaries
	if (gpGlobals->time - pev->dmgtime > 5 || GetRenderAmount() < 64 || m_hEnemy == nullptr || m_hOwner == nullptr || !UTIL_IsInWorld(GetAbsOrigin()))
	{
		SetTouch(nullptr);
		UTIL_Remove(this);
		return;
	}

	MovetoTarget(m_hEnemy->Center());

	if ((m_hEnemy->Center() - GetAbsOrigin()).Length() < 64)
	{
		TraceResult tr;
		UTIL_TraceLine(GetAbsOrigin(), m_hEnemy->Center(), IgnoreMonsters::No, this, &tr);

		if (CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit); pEntity != nullptr && pEntity->pev->takedamage)
		{
			ClearMultiDamage();
			pEntity->TraceAttack({m_hOwner, gSkillData.controllerDmgZap, GetAbsVelocity(), tr, DMG_SHOCK});
			ApplyMultiDamage(this, m_hOwner);
		}

		MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
		WRITE_BYTE(TE_BEAMENTPOINT);
		WRITE_SHORT(entindex());
		WRITE_COORD(tr.vecEndPos.x);
		WRITE_COORD(tr.vecEndPos.y);
		WRITE_COORD(tr.vecEndPos.z);
		WRITE_SHORT(g_sModelIndexLaser);
		WRITE_BYTE(0); // frame start
		WRITE_BYTE(10); // framerate
		WRITE_BYTE(3); // life
		WRITE_BYTE(20);  // width
		WRITE_BYTE(0);   // noise
		WRITE_BYTE(255);   // r, g, b
		WRITE_BYTE(255);   // r, g, b
		WRITE_BYTE(255);   // r, g, b
		WRITE_BYTE(255);	// brightness
		WRITE_BYTE(10);		// speed
		MESSAGE_END();

		UTIL_EmitAmbientSound(this, tr.vecEndPos, "weapons/electro4.wav", 0.5, ATTN_NORM, 0, RANDOM_LONG(140, 160));

		m_flNextAttack = gpGlobals->time + 3.0;

		SetThink(&CControllerHeadBall::DieThink);
		pev->nextthink = gpGlobals->time + 0.3;
	}

	// Crawl( );
}

void CControllerHeadBall::DieThink()
{
	UTIL_Remove(this);
}

void CControllerHeadBall::MovetoTarget(Vector vecTarget)
{
	// accelerate
	float flSpeed = m_vecIdeal.Length();
	if (flSpeed == 0)
	{
		m_vecIdeal = GetAbsVelocity();
		flSpeed = m_vecIdeal.Length();
	}

	if (flSpeed > 400)
	{
		m_vecIdeal = m_vecIdeal.Normalize() * 400;
	}
	m_vecIdeal = m_vecIdeal + (vecTarget - GetAbsOrigin()).Normalize() * 100;
	SetAbsVelocity(m_vecIdeal);
}

void CControllerHeadBall::Crawl()
{
	const Vector vecAim = Vector(RANDOM_FLOAT(-1, 1), RANDOM_FLOAT(-1, 1), RANDOM_FLOAT(-1, 1)).Normalize();
	const Vector vecPnt = GetAbsOrigin() + GetAbsVelocity() * 0.3 + vecAim * 64;

	MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMENTPOINT);
	WRITE_SHORT(entindex());
	WRITE_COORD(vecPnt.x);
	WRITE_COORD(vecPnt.y);
	WRITE_COORD(vecPnt.z);
	WRITE_SHORT(g_sModelIndexLaser);
	WRITE_BYTE(0); // frame start
	WRITE_BYTE(10); // framerate
	WRITE_BYTE(3); // life
	WRITE_BYTE(20);  // width
	WRITE_BYTE(0);   // noise
	WRITE_BYTE(255);   // r, g, b
	WRITE_BYTE(255);   // r, g, b
	WRITE_BYTE(255);   // r, g, b
	WRITE_BYTE(255);	// brightness
	WRITE_BYTE(10);		// speed
	MESSAGE_END();
}

void CControllerHeadBall::BounceTouch(CBaseEntity* pOther)
{
	Vector vecDir = m_vecIdeal.Normalize();

	const TraceResult tr = UTIL_GetGlobalTrace();

	const float n = -DotProduct(tr.vecPlaneNormal, vecDir);

	vecDir = 2.0 * tr.vecPlaneNormal * n + vecDir;

	m_vecIdeal = vecDir * m_vecIdeal.Length();
}

class CControllerZapBall : public CBaseMonster
{
	void Spawn() override;
	void Precache() override;
	void EXPORT AnimateThink();
	void EXPORT ExplodeTouch(CBaseEntity* pOther);

	EHANDLE m_hOwner;
};

LINK_ENTITY_TO_CLASS(controller_energy_ball, CControllerZapBall);

void CControllerZapBall::Spawn()
{
	Precache();
	// motor
	SetMovetype(Movetype::Fly);
	SetSolidType(Solid::BBox);

	SetModel("sprites/xspark4.spr");
	SetRenderMode(RenderMode::TransAdd);
	SetRenderColor({255, 255, 255});
	SetRenderAmount(255);
	pev->scale = 0.5;

	SetSize(vec3_origin, vec3_origin);
	SetAbsOrigin(GetAbsOrigin());

	SetThink(&CControllerZapBall::AnimateThink);
	SetTouch(&CControllerZapBall::ExplodeTouch);

	m_hOwner = InstanceOrWorld(pev->owner);
	pev->dmgtime = gpGlobals->time; // keep track of when ball spawned
	pev->nextthink = gpGlobals->time + 0.1;
}

void CControllerZapBall::Precache()
{
	PRECACHE_MODEL("sprites/xspark4.spr");
	// PRECACHE_SOUND("debris/zap4.wav");
	// PRECACHE_SOUND("weapons/electro4.wav");
}

void CControllerZapBall::AnimateThink()
{
	pev->nextthink = gpGlobals->time + 0.1;

	pev->frame = ((int)pev->frame + 1) % 11;

	if (gpGlobals->time - pev->dmgtime > 5 || GetAbsVelocity().Length() < 10)
	{
		SetTouch(nullptr);
		UTIL_Remove(this);
	}
}

void CControllerZapBall::ExplodeTouch(CBaseEntity* pOther)
{
	if (pOther->pev->takedamage)
	{
		TraceResult tr = UTIL_GetGlobalTrace();

		CBaseEntity* pOwner = m_hOwner ? m_hOwner : this;

		ClearMultiDamage();
		pOther->TraceAttack({pOwner, gSkillData.controllerDmgBall, GetAbsVelocity().Normalize(), tr, DMG_ENERGYBEAM});
		ApplyMultiDamage(pOwner, pOwner);

		UTIL_EmitAmbientSound(this, tr.vecEndPos, "weapons/electro4.wav", 0.3, ATTN_NORM, 0, RANDOM_LONG(90, 99));
	}

	UTIL_Remove(this);
}