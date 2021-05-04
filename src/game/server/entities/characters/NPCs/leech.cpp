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

//
// UNDONE:
// DONE:Steering force model for attack
// DONE:Attack animation control / damage
// DONE:Establish range of up/down motion and steer around vertical obstacles
// DONE:Re-evaluate height periodically
// DONE:Fall (Movetype::Toss) and play different anim if out of water
// Test in complex room (c2a3?)
// DONE:Sounds? - Kelly will fix
// Blood cloud? Hurt effect?
// Group behavior?
// DONE:Save/restore
// Flop animation - just bind to ACT_TWITCH
// Fix fatal push into wall case
//
// Try this on a bird
// Try this on a model with hulls/tracehull?
//

// Animation events
constexpr int LEECH_AE_ATTACK = 1;
constexpr int LEECH_AE_FLOP = 2;

// Movement constants

constexpr int LEECH_ACCELERATE = 10;
constexpr int LEECH_CHECK_DIST = 45;
constexpr int LEECH_SWIM_SPEED = 50;
constexpr int LEECH_SWIM_ACCEL = 80;
constexpr int LEECH_SWIM_DECEL = 10;
constexpr int LEECH_TURN_RATE = 90;
constexpr int LEECH_SIZEX = 10;
constexpr float LEECH_FRAMETIME = 0.1;

#define DEBUG_BEAMS		0

/**
*	@brief basic little swimming monster
*/
class CLeech : public CBaseMonster
{
public:
	void Spawn() override;
	void Precache() override;

	void EXPORT SwimThink();
	void EXPORT DeadThink();
	void Touch(CBaseEntity* pOther) override
	{
		if (pOther->IsPlayer())
		{
			// If the client is pushing me, give me some base velocity
			if (gpGlobals->trace_ent && InstanceOrNull(gpGlobals->trace_ent) == this)
			{
				pev->basevelocity = pOther->GetAbsVelocity();
				pev->flags |= FL_BASEVELOCITY;
			}
		}
	}

	void SetObjectCollisionBox() override
	{
		pev->absmin = GetAbsOrigin() + Vector(-8, -8, 0);
		pev->absmax = GetAbsOrigin() + Vector(8, 8, 2);
	}

	void AttackSound();
	void AlertSound() override;
	void UpdateMotion();

	/**
	*	@brief returns normalized distance to obstacle
	*/
	float ObstacleDistance(CBaseEntity* pTarget);
	void MakeVectors();
	void RecalculateWaterlevel();
	void SwitchLeechState();

	// Base entity functions
	void HandleAnimEvent(AnimationEvent& event) override;
	int	BloodColor() override { return DONT_BLEED; }
	void Killed(const KilledInfo& info) override;
	void Activate() override;
	bool TakeDamage(const TakeDamageInfo& info) override;
	int	Classify() override { return CLASS_INSECT; }
	Relationship GetRelationship(CBaseEntity* pTarget) override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	static const char* pAttackSounds[];
	static const char* pAlertSounds[];

private:
	// UNDONE: Remove unused boid vars, do group behavior
	float	m_flTurning;// is this boid turning?
	bool	m_fPathBlocked;// true if there is an obstacle ahead
	float	m_flAccelerate;
	float	m_obstacle;
	float	m_top;
	float	m_bottom;
	float	m_height;
	float	m_waterTime;
	float	m_sideTime;		// Timer to randomly check clearance on sides
	float	m_zTime;
	float	m_stateTime;
	float	m_attackSoundTime;

#if DEBUG_BEAMS
	CBeam* m_pb;
	CBeam* m_pt;
#endif
};

LINK_ENTITY_TO_CLASS(monster_leech, CLeech);

TYPEDESCRIPTION	CLeech::m_SaveData[] =
{
	DEFINE_FIELD(CLeech, m_flTurning, FIELD_FLOAT),
	DEFINE_FIELD(CLeech, m_fPathBlocked, FIELD_BOOLEAN),
	DEFINE_FIELD(CLeech, m_flAccelerate, FIELD_FLOAT),
	DEFINE_FIELD(CLeech, m_obstacle, FIELD_FLOAT),
	DEFINE_FIELD(CLeech, m_top, FIELD_FLOAT),
	DEFINE_FIELD(CLeech, m_bottom, FIELD_FLOAT),
	DEFINE_FIELD(CLeech, m_height, FIELD_FLOAT),
	DEFINE_FIELD(CLeech, m_waterTime, FIELD_TIME),
	DEFINE_FIELD(CLeech, m_sideTime, FIELD_TIME),
	DEFINE_FIELD(CLeech, m_zTime, FIELD_TIME),
	DEFINE_FIELD(CLeech, m_stateTime, FIELD_TIME),
	DEFINE_FIELD(CLeech, m_attackSoundTime, FIELD_TIME),
};

IMPLEMENT_SAVERESTORE(CLeech, CBaseMonster);

const char* CLeech::pAttackSounds[] =
{
	"leech/leech_bite1.wav",
	"leech/leech_bite2.wav",
	"leech/leech_bite3.wav",
};

const char* CLeech::pAlertSounds[] =
{
	"leech/leech_alert1.wav",
	"leech/leech_alert2.wav",
};

void CLeech::Spawn()
{
	Precache();
	SetModel("models/leech.mdl");
	// Just for fun
	//	SetModel( "models/icky.mdl");

//	SetSize( vec3_origin, vec3_origin );
	SetSize(Vector(-1, -1, 0), Vector(1, 1, 2));
	// Don't push the minz down too much or the water check will fail because this entity is really point-sized
	SetSolidType(Solid::SlideBox);
	SetMovetype(Movetype::Fly);
	SetBits(pev->flags, FL_SWIM);
	pev->health = gSkillData.leechHealth;

	m_flFieldOfView = -0.5;	// 180 degree FOV
	m_flDistLook = 750;
	MonsterInit();
	SetThink(&CLeech::SwimThink);
	SetUse(nullptr);
	SetTouch(nullptr);
	pev->view_ofs = vec3_origin;

	m_flTurning = 0;
	m_fPathBlocked = false;
	SetActivity(ACT_SWIM);
	SetState(NPCState::Idle);
	m_stateTime = gpGlobals->time + RANDOM_FLOAT(1, 5);
}

void CLeech::Activate()
{
	RecalculateWaterlevel();
}

void CLeech::RecalculateWaterlevel()
{
	// Calculate boundaries
	const Vector vecTest = GetAbsOrigin() - Vector(0, 0, 400);

	TraceResult tr;

	UTIL_TraceLine(GetAbsOrigin(), vecTest, IgnoreMonsters::No, this, &tr);
	if (tr.flFraction != 1.0)
		m_bottom = tr.vecEndPos.z + 1;
	else
		m_bottom = vecTest.z;

	m_top = UTIL_WaterLevel(GetAbsOrigin(), GetAbsOrigin().z, GetAbsOrigin().z + 400) - 1;

	// Chop off 20% of the outside range
	const float newBottom = m_bottom * 0.8 + m_top * 0.2;
	m_top = m_bottom * 0.2 + m_top * 0.8;
	m_bottom = newBottom;
	m_height = RANDOM_FLOAT(m_bottom, m_top);
	m_waterTime = gpGlobals->time + RANDOM_FLOAT(5, 7);
}

void CLeech::SwitchLeechState()
{
	m_stateTime = gpGlobals->time + RANDOM_FLOAT(3, 6);
	if (m_MonsterState == NPCState::Combat)
	{
		m_hEnemy = nullptr;
		SetState(NPCState::Idle);
		// We may be up against the player, so redo the side checks
		m_sideTime = 0;
	}
	else
	{
		Look(m_flDistLook);

		if (CBaseEntity* pEnemy = BestVisibleEnemy(); pEnemy && pEnemy->pev->waterlevel != WaterLevel::Dry)
		{
			m_hEnemy = pEnemy;
			SetState(NPCState::Combat);
			m_stateTime = gpGlobals->time + RANDOM_FLOAT(18, 25);
			AlertSound();
		}
	}
}

Relationship CLeech::GetRelationship(CBaseEntity* pTarget)
{
	if (pTarget->IsPlayer())
		return Relationship::Dislike;
	return CBaseMonster::GetRelationship(pTarget);
}

void CLeech::AttackSound()
{
	if (gpGlobals->time > m_attackSoundTime)
	{
		EmitSound(SoundChannel::Voice, pAttackSounds[RANDOM_LONG(0, ArraySize(pAttackSounds) - 1)]);
		m_attackSoundTime = gpGlobals->time + 0.5;
	}
}

void CLeech::AlertSound()
{
	EmitSound(SoundChannel::Voice, pAlertSounds[RANDOM_LONG(0, ArraySize(pAlertSounds) - 1)], VOL_NORM, ATTN_NORM * 0.5);
}

void CLeech::Precache()
{
	//PRECACHE_MODEL("models/icky.mdl");
	PRECACHE_MODEL("models/leech.mdl");

	PRECACHE_SOUND_ARRAY(pAttackSounds);
	PRECACHE_SOUND_ARRAY(pAlertSounds);
}

bool CLeech::TakeDamage(const TakeDamageInfo& info)
{
	SetAbsVelocity(vec3_origin);

	// Nudge the leech away from the damage
	if (info.GetInflictor())
	{
		SetAbsVelocity((GetAbsOrigin() - info.GetInflictor()->GetAbsOrigin()).Normalize() * 25);
	}

	return CBaseMonster::TakeDamage(info);
}

void CLeech::HandleAnimEvent(AnimationEvent& event)
{
	switch (event.event)
	{
	case LEECH_AE_ATTACK:
		AttackSound();

		if (CBaseEntity* pEnemy = m_hEnemy; pEnemy != nullptr)
		{
			Vector face;
			AngleVectors(GetAbsAngles(), &face, nullptr, nullptr);
			face.z = 0;

			Vector dir = pEnemy->GetAbsOrigin() - GetAbsOrigin();
			dir.z = 0;
			dir = dir.Normalize();
			face = face.Normalize();

			if (DotProduct(dir, face) > 0.9)		// Only take damage if the leech is facing the prey
				pEnemy->TakeDamage({this, this, gSkillData.leechDmgBite, DMG_SLASH});
		}
		m_stateTime -= 2;
		break;

	case LEECH_AE_FLOP:
		// Play flop sound
		break;

	default:
		CBaseMonster::HandleAnimEvent(event);
		break;
	}
}

void CLeech::MakeVectors()
{
	Vector tmp = GetAbsAngles();
	tmp.x = -tmp.x;
	UTIL_MakeVectors(tmp);
}

float CLeech::ObstacleDistance(CBaseEntity* pTarget)
{
	// use VELOCITY, not angles, not all boids point the direction they are flying
	//Vector vecDir = VectorAngles(GetAbsVelocity());
	MakeVectors();

	// check for obstacle ahead
	Vector vecTest = GetAbsOrigin() + gpGlobals->v_forward * LEECH_CHECK_DIST;
	TraceResult tr;
	UTIL_TraceLine(GetAbsOrigin(), vecTest, IgnoreMonsters::No, this, &tr);

	if (tr.fStartSolid)
	{
		pev->speed = -LEECH_SWIM_SPEED * 0.5;
		//		ALERT( at_console, "Stuck from (%f %f %f) to (%f %f %f)\n", pev->oldorigin.x, pev->oldorigin.y, pev->oldorigin.z, GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z );
		//		SetAbsOrigin( pev->oldorigin );
	}

	if (tr.flFraction != 1.0)
	{
		if (pTarget == nullptr || InstanceOrNull(tr.pHit) != pTarget)
		{
			return tr.flFraction;
		}
		else
		{
			if (fabs(m_height - GetAbsOrigin().z) > 10)
				return tr.flFraction;
		}
	}

	if (m_sideTime < gpGlobals->time)
	{
		// extra wide checks
		vecTest = GetAbsOrigin() + gpGlobals->v_right * LEECH_SIZEX * 2 + gpGlobals->v_forward * LEECH_CHECK_DIST;
		UTIL_TraceLine(GetAbsOrigin(), vecTest, IgnoreMonsters::No, this, &tr);
		if (tr.flFraction != 1.0)
			return tr.flFraction;

		vecTest = GetAbsOrigin() - gpGlobals->v_right * LEECH_SIZEX * 2 + gpGlobals->v_forward * LEECH_CHECK_DIST;
		UTIL_TraceLine(GetAbsOrigin(), vecTest, IgnoreMonsters::No, this, &tr);
		if (tr.flFraction != 1.0)
			return tr.flFraction;

		// Didn't hit either side, so stop testing for another 0.5 - 1 seconds
		m_sideTime = gpGlobals->time + RANDOM_FLOAT(0.5, 1);
	}
	return 1.0;
}

void CLeech::DeadThink()
{
	if (m_fSequenceFinished)
	{
		if (m_Activity == ACT_DIEFORWARD)
		{
			SetThink(nullptr);
			StopAnimation();
			return;
		}
		else if (pev->flags & FL_ONGROUND)
		{
			SetSolidType(Solid::Not);
			SetActivity(ACT_DIEFORWARD);
		}
	}
	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1;

	// Apply damage velocity, but keep out of the walls
	if (GetAbsVelocity().x != 0 || GetAbsVelocity().y != 0)
	{
		TraceResult tr;

		// Look 0.5 seconds ahead
		UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + GetAbsVelocity() * 0.5, IgnoreMonsters::No, this, &tr);
		if (tr.flFraction != 1.0)
		{
			SetAbsVelocity({0, 0, GetAbsVelocity().z});
		}
	}
}

void CLeech::UpdateMotion()
{
	m_flAccelerate = m_flAccelerate * 0.8 + pev->speed * 0.2;

	const float flapspeed = std::clamp(std::abs((pev->speed - m_flAccelerate) / LEECH_ACCELERATE) + 1, 0.5f, 1.9f);

	pev->framerate = flapspeed;

	if (!m_fPathBlocked)
		pev->avelocity.y = pev->ideal_yaw;
	else
		pev->avelocity.y = pev->ideal_yaw * m_obstacle;

	if (pev->avelocity.y > 150)
		m_IdealActivity = ACT_TURN_LEFT;
	else if (pev->avelocity.y < -150)
		m_IdealActivity = ACT_TURN_RIGHT;
	else
		m_IdealActivity = ACT_SWIM;

	// lean
	const float delta = m_height - GetAbsOrigin().z;

	float targetPitch;
	if (delta < -10)
		targetPitch = -30;
	else if (delta > 10)
		targetPitch = 30;
	else
		targetPitch = 0;

	Vector myAngles = GetAbsAngles();

	myAngles.x = UTIL_Approach(targetPitch, myAngles.x, 60 * LEECH_FRAMETIME);

	// bank
	pev->avelocity.z = -(myAngles.z + (pev->avelocity.y * 0.25));

	if (m_MonsterState == NPCState::Combat && HasConditions(bits_COND_CAN_MELEE_ATTACK1))
		m_IdealActivity = ACT_MELEE_ATTACK1;

	// Out of water check
	if (pev->waterlevel == WaterLevel::Dry)
	{
		SetMovetype(Movetype::Toss);
		m_IdealActivity = ACT_TWITCH;
		SetAbsVelocity(vec3_origin);

		// Animation will intersect the floor if either of these is non-zero
		myAngles.x = 0;
		myAngles.z = 0;

		if (pev->framerate < 1.0)
			pev->framerate = 1.0;
	}
	else if (GetMovetype() == Movetype::Toss)
	{
		SetMovetype(Movetype::Fly);
		pev->flags &= ~FL_ONGROUND;
		RecalculateWaterlevel();
		m_waterTime = gpGlobals->time + 2;	// Recalc again soon, water may be rising
	}

	SetAbsAngles(myAngles);

	if (m_Activity != m_IdealActivity)
	{
		SetActivity(m_IdealActivity);
	}
	const float flInterval = StudioFrameAdvance();
	DispatchAnimEvents(flInterval);

#if DEBUG_BEAMS
	if (!m_pb)
		m_pb = CBeam::BeamCreate("sprites/laserbeam.spr", 5);
	if (!m_pt)
		m_pt = CBeam::BeamCreate("sprites/laserbeam.spr", 5);
	m_pb->PointsInit(GetAbsOrigin(), GetAbsOrigin() + gpGlobals->v_forward * LEECH_CHECK_DIST);
	m_pt->PointsInit(GetAbsOrigin(), GetAbsOrigin() - gpGlobals->v_right * (pev->avelocity.y * 0.25));
	if (m_fPathBlocked)
	{
		float color = m_obstacle * 30;
		if (m_obstacle == 1.0)
			color = 0;
		color = std::min(255.0f, color);
		m_pb->SetColor(255, (int)color, (int)color);
	}
	else
		m_pb->SetColor(255, 255, 0);
	m_pt->SetColor(0, 0, 255);
#endif
}

void CLeech::SwimThink()
{
	if (IsNullEnt(UTIL_FindClientInPVS(this)))
	{
		pev->nextthink = gpGlobals->time + RANDOM_FLOAT(1, 1.5);
		SetAbsVelocity(vec3_origin);
		return;
	}
	else
		pev->nextthink = gpGlobals->time + 0.1;

	float targetSpeed = LEECH_SWIM_SPEED;

	if (m_waterTime < gpGlobals->time)
		RecalculateWaterlevel();

	if (m_stateTime < gpGlobals->time)
		SwitchLeechState();

	ClearConditions(bits_COND_CAN_MELEE_ATTACK1);

	TraceResult tr;
	float targetYaw = 0;
	CBaseEntity* pTarget;

	switch (m_MonsterState)
	{
	case NPCState::Combat:
		pTarget = m_hEnemy;
		if (!pTarget)
			SwitchLeechState();
		else
		{
			// Chase the enemy's eyes
			m_height = pTarget->GetAbsOrigin().z + pTarget->pev->view_ofs.z - 5;
			// Clip to viable water area
			if (m_height < m_bottom)
				m_height = m_bottom;
			else if (m_height > m_top)
				m_height = m_top;
			Vector location = pTarget->GetAbsOrigin() - GetAbsOrigin();
			location.z += (pTarget->pev->view_ofs.z);
			if (location.Length() < 40)
				SetConditions(bits_COND_CAN_MELEE_ATTACK1);
			// Turn towards target ent
			targetYaw = UTIL_VecToYaw(location);

			targetYaw = UTIL_AngleDiff(targetYaw, UTIL_AngleMod(GetAbsAngles().y));

			if (targetYaw < (-LEECH_TURN_RATE * 0.75))
				targetYaw = (-LEECH_TURN_RATE * 0.75);
			else if (targetYaw > (LEECH_TURN_RATE * 0.75))
				targetYaw = (LEECH_TURN_RATE * 0.75);
			else
				targetSpeed *= 2;
		}

		break;

	default:
		if (m_zTime < gpGlobals->time)
		{
			float newHeight = RANDOM_FLOAT(m_bottom, m_top);
			m_height = 0.5 * m_height + 0.5 * newHeight;
			m_zTime = gpGlobals->time + RANDOM_FLOAT(1, 4);
		}
		if (RANDOM_LONG(0, 100) < 10)
			targetYaw = RANDOM_LONG(-30, 30);
		pTarget = nullptr;
		// oldorigin test
		if ((GetAbsOrigin() - pev->oldorigin).Length() < 1)
		{
			// If leech didn't move, there must be something blocking it, so try to turn
			m_sideTime = 0;
		}

		break;
	}

	m_obstacle = ObstacleDistance(pTarget);
	pev->oldorigin = GetAbsOrigin();
	if (m_obstacle < 0.1)
		m_obstacle = 0.1;

	// is the way ahead clear?
	if (m_obstacle == 1.0)
	{
		// if the leech is turning, stop the trend.
		if (m_flTurning != 0)
		{
			m_flTurning = 0;
		}

		m_fPathBlocked = false;
		pev->speed = UTIL_Approach(targetSpeed, pev->speed, LEECH_SWIM_ACCEL * LEECH_FRAMETIME);
		SetAbsVelocity(gpGlobals->v_forward * pev->speed);
	}
	else
	{
		m_obstacle = 1.0 / m_obstacle;
		// IF we get this far in the function, the leader's path is blocked!
		m_fPathBlocked = true;

		if (m_flTurning == 0)// something in the way and leech is not already turning to avoid
		{
			// measure clearance on left and right to pick the best dir to turn
			Vector vecTest = GetAbsOrigin() + (gpGlobals->v_right * LEECH_SIZEX) + (gpGlobals->v_forward * LEECH_CHECK_DIST);
			UTIL_TraceLine(GetAbsOrigin(), vecTest, IgnoreMonsters::No, this, &tr);
			const float flRightSide = tr.flFraction;

			vecTest = GetAbsOrigin() + (gpGlobals->v_right * -LEECH_SIZEX) + (gpGlobals->v_forward * LEECH_CHECK_DIST);
			UTIL_TraceLine(GetAbsOrigin(), vecTest, IgnoreMonsters::No, this, &tr);
			const float flLeftSide = tr.flFraction;

			// turn left, right or random depending on clearance ratio
			const float delta = flRightSide - flLeftSide;
			if (delta > 0.1 || (delta > -0.1 && RANDOM_LONG(0, 100) < 50))
				m_flTurning = -LEECH_TURN_RATE;
			else
				m_flTurning = LEECH_TURN_RATE;
		}
		pev->speed = UTIL_Approach(-(LEECH_SWIM_SPEED * 0.5), pev->speed, LEECH_SWIM_DECEL * LEECH_FRAMETIME * m_obstacle);
		SetAbsVelocity(gpGlobals->v_forward * pev->speed);
	}
	pev->ideal_yaw = m_flTurning + targetYaw;
	UpdateMotion();
}

void CLeech::Killed(const KilledInfo& info)
{
	//ALERT(at_aiconsole, "Leech: killed\n");
	// tell owner ( if any ) that we're dead.This is mostly for MonsterMaker functionality.
	if (CBaseEntity* pOwner = GetOwner(); pOwner)
		pOwner->DeathNotice(this);

	// When we hit the ground, play the "death_end" activity
	if (pev->waterlevel != WaterLevel::Dry)
	{
		SetAbsAngles({0, GetAbsAngles().y, 0});
		SetAbsOrigin(GetAbsOrigin() + Vector(0, 0, 1));
		pev->avelocity = vec3_origin;
		if (RANDOM_LONG(0, 99) < 70)
			pev->avelocity.y = RANDOM_LONG(-720, 720);

		pev->gravity = 0.02;
		ClearBits(pev->flags, FL_ONGROUND);
		SetActivity(ACT_DIESIMPLE);
	}
	else
		SetActivity(ACT_DIEFORWARD);

	SetMovetype(Movetype::Toss);
	SetDamageMode(DamageMode::No);
	SetThink(&CLeech::DeadThink);
}
