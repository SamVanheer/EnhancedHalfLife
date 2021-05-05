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

#include "navigation/nodes.h"

class CNihilanthHVR;

constexpr int N_SCALE = 15;
constexpr int N_SPHERES = 20;

/**
*	@brief Nihilanth, final Boss monster
*/
class CNihilanth : public CBaseMonster
{
public:
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	void Spawn() override;
	void Precache() override;
	int  Classify() override { return CLASS_ALIEN_MILITARY; }
	int  BloodColor() override { return BLOOD_COLOR_YELLOW; }
	void Killed(const KilledInfo& info) override;
	void GibMonster() override;

	void SetObjectCollisionBox() override
	{
		pev->absmin = GetAbsOrigin() + Vector(-16 * N_SCALE, -16 * N_SCALE, -48 * N_SCALE);
		pev->absmax = GetAbsOrigin() + Vector(16 * N_SCALE, 16 * N_SCALE, 28 * N_SCALE);
	}

	void HandleAnimEvent(AnimationEvent& event) override;

	void EXPORT StartupThink();
	void EXPORT HuntThink();
	void EXPORT CrashTouch(CBaseEntity* pOther);
	void EXPORT DyingThink();
	void EXPORT StartupUse(const UseInfo& info);
	void EXPORT NullThink();
	void EXPORT CommandUse(const UseInfo& info);

	void FloatSequence();
	void NextActivity();

	void Flight();

	bool AbsorbSphere();
	bool EmitSphere();
	void TargetSphere(UseType useType, float value);
	CBaseEntity* RandomTargetname(const char* szName);
	void ShootBalls();
	void MakeFriend(Vector vecPos);

	bool TakeDamage(const TakeDamageInfo& info) override;
	void TraceAttack(const TraceAttackInfo& info) override;

	void PainSound() override;
	void DeathSound() override;

	static const char* pAttackSounds[];	// vocalization: play sometimes when he launches an attack
	static const char* pBallSounds[];	// the sound of the lightening ball launch
	static const char* pShootSounds[];	// grunting vocalization: play sometimes when he launches an attack
	static const char* pRechargeSounds[];	// vocalization: play when he recharges
	static const char* pLaughSounds[];	// vocalization: play sometimes when hit and still has lots of health
	static const char* pPainSounds[];	// vocalization: play sometimes when hit and has much less health and no more chargers
	static const char* pDeathSounds[];	// vocalization: play as he dies

	// x_teleattack1.wav	the looping sound of the teleport attack ball.

	float m_flForce = 0;

	float m_flNextPainSound = 0;

	Vector m_velocity;
	Vector m_avelocity;

	Vector m_vecTarget;
	Vector m_posTarget;

	Vector m_vecDesired;
	Vector m_posDesired;

	float  m_flMinZ = 0;
	float  m_flMaxZ = 0;

	Vector m_vecGoal;

	float m_flLastSeen = 0;
	float m_flPrevSeen = 0;

	int m_irritation = 0;

	int m_iLevel = 0;
	int m_iTeleport = 0;

	EHANDLE m_hRecharger;

	EHandle<CNihilanthHVR> m_hSphere[N_SPHERES];
	int	m_iActiveSpheres = 0;

	float m_flAdj = 0;

	EHandle<CSprite> m_hBall;

	char m_szRechargerTarget[64]{};
	char m_szDrawUse[64]{};
	char m_szTeleportUse[64]{};
	char m_szTeleportTouch[64]{};
	char m_szDeadUse[64]{};
	char m_szDeadTouch[64]{};

	float m_flShootEnd = 0;
	float m_flShootTime = 0;

	EHANDLE m_hFriend[3];
};

LINK_ENTITY_TO_CLASS(monster_nihilanth, CNihilanth);

TYPEDESCRIPTION	CNihilanth::m_SaveData[] =
{
	DEFINE_FIELD(CNihilanth, m_flForce, FIELD_FLOAT),
	DEFINE_FIELD(CNihilanth, m_flNextPainSound, FIELD_TIME),
	DEFINE_FIELD(CNihilanth, m_velocity, FIELD_VECTOR),
	DEFINE_FIELD(CNihilanth, m_avelocity, FIELD_VECTOR),
	DEFINE_FIELD(CNihilanth, m_vecTarget, FIELD_VECTOR),
	DEFINE_FIELD(CNihilanth, m_posTarget, FIELD_POSITION_VECTOR),
	DEFINE_FIELD(CNihilanth, m_vecDesired, FIELD_VECTOR),
	DEFINE_FIELD(CNihilanth, m_posDesired, FIELD_POSITION_VECTOR),
	DEFINE_FIELD(CNihilanth, m_flMinZ, FIELD_FLOAT),
	DEFINE_FIELD(CNihilanth, m_flMaxZ, FIELD_FLOAT),
	DEFINE_FIELD(CNihilanth, m_vecGoal, FIELD_VECTOR),
	DEFINE_FIELD(CNihilanth, m_flLastSeen, FIELD_TIME),
	DEFINE_FIELD(CNihilanth, m_flPrevSeen, FIELD_TIME),
	DEFINE_FIELD(CNihilanth, m_irritation, FIELD_INTEGER),
	DEFINE_FIELD(CNihilanth, m_iLevel, FIELD_INTEGER),
	DEFINE_FIELD(CNihilanth, m_iTeleport, FIELD_INTEGER),
	DEFINE_FIELD(CNihilanth, m_hRecharger, FIELD_EHANDLE),
	DEFINE_ARRAY(CNihilanth, m_hSphere, FIELD_EHANDLE, N_SPHERES),
	DEFINE_FIELD(CNihilanth, m_iActiveSpheres, FIELD_INTEGER),
	DEFINE_FIELD(CNihilanth, m_flAdj, FIELD_FLOAT),
	DEFINE_FIELD(CNihilanth, m_hBall, FIELD_EHANDLE),
	DEFINE_ARRAY(CNihilanth, m_szRechargerTarget, FIELD_CHARACTER, 64),
	DEFINE_ARRAY(CNihilanth, m_szDrawUse, FIELD_CHARACTER, 64),
	DEFINE_ARRAY(CNihilanth, m_szTeleportUse, FIELD_CHARACTER, 64),
	DEFINE_ARRAY(CNihilanth, m_szTeleportTouch, FIELD_CHARACTER, 64),
	DEFINE_ARRAY(CNihilanth, m_szDeadUse, FIELD_CHARACTER, 64),
	DEFINE_ARRAY(CNihilanth, m_szDeadTouch, FIELD_CHARACTER, 64),
	DEFINE_FIELD(CNihilanth, m_flShootEnd, FIELD_TIME),
	DEFINE_FIELD(CNihilanth, m_flShootTime, FIELD_TIME),
	DEFINE_ARRAY(CNihilanth, m_hFriend, FIELD_EHANDLE, 3),
};

IMPLEMENT_SAVERESTORE(CNihilanth, CBaseMonster);

/**
*	@brief Controller bouncy ball attack
*/
class CNihilanthHVR : public CBaseMonster
{
public:
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	void Spawn() override;
	void Precache() override;

	void CircleInit(CBaseEntity* pTarget);
	void AbsorbInit();
	void TeleportInit(CNihilanth* pOwner, CBaseEntity* pEnemy, CBaseEntity* pTarget, CBaseEntity* pTouch);
	void GreenBallInit();
	void ZapInit(CBaseEntity* pEnemy);

	void EXPORT HoverThink();
	bool CircleTarget(Vector vecTarget);
	void EXPORT DissipateThink();

	void EXPORT ZapThink();
	void EXPORT TeleportThink();
	void EXPORT TeleportTouch(CBaseEntity* pOther);

	void EXPORT RemoveTouch(CBaseEntity* pOther);
	void EXPORT BounceTouch(CBaseEntity* pOther);
	void EXPORT ZapTouch(CBaseEntity* pOther);

	CBaseEntity* RandomClassname(const char* szName);

	// void EXPORT SphereUse(const UseInfo& info);

	void MovetoTarget(Vector vecTarget);
	virtual void Crawl();

	//TODO: unused?
	void Zap();
	void Teleport();

	float m_flIdealVel = 0;
	Vector m_vecIdeal;
	EHandle<CNihilanth> m_hNihilanth;
	EHANDLE m_hTouch;
	int m_nFrames = 0;
};

LINK_ENTITY_TO_CLASS(nihilanth_energy_ball, CNihilanthHVR);

TYPEDESCRIPTION	CNihilanthHVR::m_SaveData[] =
{
	DEFINE_FIELD(CNihilanthHVR, m_flIdealVel, FIELD_FLOAT),
	DEFINE_FIELD(CNihilanthHVR, m_vecIdeal, FIELD_VECTOR),
	DEFINE_FIELD(CNihilanthHVR, m_hNihilanth, FIELD_EHANDLE),
	DEFINE_FIELD(CNihilanthHVR, m_hTouch, FIELD_EHANDLE),
	DEFINE_FIELD(CNihilanthHVR, m_nFrames, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CNihilanthHVR, CBaseMonster);

const char* CNihilanth::pAttackSounds[] =
{
	"X/x_attack1.wav",
	"X/x_attack2.wav",
	"X/x_attack3.wav",
};

const char* CNihilanth::pBallSounds[] =
{
	"X/x_ballattack1.wav",
};

const char* CNihilanth::pShootSounds[] =
{
	"X/x_shoot1.wav",
};

const char* CNihilanth::pRechargeSounds[] =
{
	"X/x_recharge1.wav",
	"X/x_recharge2.wav",
	"X/x_recharge3.wav",
};

const char* CNihilanth::pLaughSounds[] =
{
	"X/x_laugh1.wav",
	"X/x_laugh2.wav",
};

const char* CNihilanth::pPainSounds[] =
{
	"X/x_pain1.wav",
	"X/x_pain2.wav",
};

const char* CNihilanth::pDeathSounds[] =
{
	"X/x_die1.wav",
};

void CNihilanth::Spawn()
{
	Precache();
	// motor
	SetMovetype(Movetype::Fly);
	SetSolidType(Solid::BBox);

	SetModel("models/nihilanth.mdl");
	// SetSize( Vector( -300, -300, 0), Vector(300, 300, 512));
	SetSize(Vector(-32, -32, 0), Vector(32, 32, 64));
	SetAbsOrigin(GetAbsOrigin());

	pev->flags |= FL_MONSTER;
	SetDamageMode(DamageMode::Aim);
	pev->health = gSkillData.nihilanthHealth;
	pev->view_ofs = Vector(0, 0, 300);

	m_flFieldOfView = -1; // 360 degrees

	pev->sequence = 0;
	ResetSequenceInfo();

	InitBoneControllers();

	SetThink(&CNihilanth::StartupThink);
	pev->nextthink = gpGlobals->time + 0.1;

	m_vecDesired = vec3_forward;
	m_posDesired = Vector(GetAbsOrigin().x, GetAbsOrigin().y, 512);

	m_iLevel = 1;
	m_iTeleport = 1;

	if (m_szRechargerTarget[0] == '\0')	safe_strcpy(m_szRechargerTarget, "n_recharger");
	if (m_szDrawUse[0] == '\0')			safe_strcpy(m_szDrawUse, "n_draw");
	if (m_szTeleportUse[0] == '\0')		safe_strcpy(m_szTeleportUse, "n_leaving");
	if (m_szTeleportTouch[0] == '\0')	safe_strcpy(m_szTeleportTouch, "n_teleport");
	if (m_szDeadUse[0] == '\0')			safe_strcpy(m_szDeadUse, "n_dead");
	if (m_szDeadTouch[0] == '\0')		safe_strcpy(m_szDeadTouch, "n_ending");

	// near death
	/*
	m_iTeleport = 10;
	m_iLevel = 10;
	m_irritation = 2;
	pev->health = 100;
	*/
}

void CNihilanth::Precache()
{
	PRECACHE_MODEL("models/nihilanth.mdl");
	PRECACHE_MODEL("sprites/lgtning.spr");
	UTIL_PrecacheOther("nihilanth_energy_ball");
	UTIL_PrecacheOther("monster_alien_controller");
	UTIL_PrecacheOther("monster_alien_slave");

	PRECACHE_SOUND_ARRAY(pAttackSounds);
	PRECACHE_SOUND_ARRAY(pBallSounds);
	PRECACHE_SOUND_ARRAY(pShootSounds);
	PRECACHE_SOUND_ARRAY(pRechargeSounds);
	PRECACHE_SOUND_ARRAY(pLaughSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pDeathSounds);
	PRECACHE_SOUND("debris/beamstart7.wav");
}

void CNihilanth::PainSound()
{
	if (m_flNextPainSound > gpGlobals->time)
		return;

	m_flNextPainSound = gpGlobals->time + RANDOM_FLOAT(2, 5);

	if (pev->health > gSkillData.nihilanthHealth / 2)
	{
		EmitSound(SoundChannel::Voice, RANDOM_SOUND_ARRAY(pLaughSounds), VOL_NORM, 0.2);
	}
	else if (m_irritation >= 2)
	{
		EmitSound(SoundChannel::Voice, RANDOM_SOUND_ARRAY(pPainSounds), VOL_NORM, 0.2);
	}
}

void CNihilanth::DeathSound()
{
	EmitSound(SoundChannel::Voice, RANDOM_SOUND_ARRAY(pDeathSounds), VOL_NORM, 0.1);
}

void CNihilanth::NullThink()
{
	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.5;
}

void CNihilanth::StartupUse(const UseInfo& info)
{
	SetThink(&CNihilanth::HuntThink);
	pev->nextthink = gpGlobals->time + 0.1;
	SetUse(&CNihilanth::CommandUse);
}

void CNihilanth::StartupThink()
{
	m_irritation = 0;
	m_flAdj = 512;

	CBaseEntity* pEntity = UTIL_FindEntityByTargetname(nullptr, "n_min");
	if (pEntity)
		m_flMinZ = pEntity->GetAbsOrigin().z;
	else
		m_flMinZ = -WORLD_BOUNDARY;

	pEntity = UTIL_FindEntityByTargetname(nullptr, "n_max");
	if (pEntity)
		m_flMaxZ = pEntity->GetAbsOrigin().z;
	else
		m_flMaxZ = WORLD_BOUNDARY;

	m_hRecharger = this;
	for (int i = 0; i < N_SPHERES; i++)
	{
		EmitSphere();
	}
	m_hRecharger = nullptr;

	SetThink(&CNihilanth::HuntThink);
	SetUse(&CNihilanth::CommandUse);
	pev->nextthink = gpGlobals->time + 0.1;
}

void CNihilanth::Killed(const KilledInfo& info)
{
	CBaseMonster::Killed(info);
}

void CNihilanth::DyingThink()
{
	pev->nextthink = gpGlobals->time + 0.1;
	DispatchAnimEvents();
	StudioFrameAdvance();

	if (pev->deadflag == DeadFlag::No)
	{
		DeathSound();
		pev->deadflag = DeadFlag::Dying;

		m_posDesired.z = m_flMaxZ;
	}

	if (pev->deadflag == DeadFlag::Dying)
	{
		Flight();

		if (fabs(GetAbsOrigin().z - m_flMaxZ) < 16)
		{
			SetAbsVelocity(vec3_origin);
			FireTargets(m_szDeadUse, this, this, UseType::On, 1.0);
			pev->deadflag = DeadFlag::Dead;
		}
	}

	if (m_fSequenceFinished)
	{
		pev->avelocity.y += RANDOM_FLOAT(-100, 100);
		if (pev->avelocity.y < -100)
			pev->avelocity.y = -100;
		if (pev->avelocity.y > 100)
			pev->avelocity.y = 100;

		pev->sequence = LookupSequence("die1");
	}

	if (auto ball = m_hBall.Get(); ball)
	{
		if (ball->GetRenderAmount() > 0)
		{
			ball->SetRenderAmount(std::max(0.0f, ball->GetRenderAmount() - 2));
		}
		else
		{
			UTIL_Remove(m_hBall);
			m_hBall = nullptr;
		}
	}

	Vector vecDir;

	UTIL_MakeAimVectors(GetAbsAngles());
	const int iAttachment = RANDOM_LONG(1, 4);

	do
	{
		vecDir = Vector(RANDOM_FLOAT(-1, 1), RANDOM_FLOAT(-1, 1), RANDOM_FLOAT(-1, 1));
	}
	while (DotProduct(vecDir, vecDir) > 1.0);

	switch (RANDOM_LONG(1, 4))
	{
	case 1: // head
		vecDir.z = fabs(vecDir.z) * 0.5;
		vecDir = vecDir + 2 * gpGlobals->v_up;
		break;
	case 2: // eyes
		if (DotProduct(vecDir, gpGlobals->v_forward) < 0)
			vecDir = vecDir * -1;

		vecDir = vecDir + 2 * gpGlobals->v_forward;
		break;
	case 3: // left hand
		if (DotProduct(vecDir, gpGlobals->v_right) > 0)
			vecDir = vecDir * -1;
		vecDir = vecDir - 2 * gpGlobals->v_right;
		break;
	case 4: // right hand
		if (DotProduct(vecDir, gpGlobals->v_right) < 0)
			vecDir = vecDir * -1;
		vecDir = vecDir + 2 * gpGlobals->v_right;
		break;
	}

	Vector vecSrc, vecAngles;
	GetAttachment(iAttachment - 1, vecSrc, vecAngles);

	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecSrc + vecDir * WORLD_BOUNDARY, IgnoreMonsters::Yes, this, &tr);

	MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMENTPOINT);
	WRITE_SHORT(entindex() + 0x1000 * iAttachment);
	WRITE_COORD(tr.vecEndPos.x);
	WRITE_COORD(tr.vecEndPos.y);
	WRITE_COORD(tr.vecEndPos.z);
	WRITE_SHORT(g_sModelIndexLaser);
	WRITE_BYTE(0); // frame start
	WRITE_BYTE(10); // framerate
	WRITE_BYTE(5); // life
	WRITE_BYTE(100);  // width
	WRITE_BYTE(120);   // noise
	WRITE_BYTE(64);   // r, g, b
	WRITE_BYTE(128);   // r, g, b
	WRITE_BYTE(255);   // r, g, b
	WRITE_BYTE(255);	// brightness
	WRITE_BYTE(10);		// speed
	MESSAGE_END();

	GetAttachment(0, vecSrc, vecAngles);
	CNihilanthHVR* pEntity = (CNihilanthHVR*)Create("nihilanth_energy_ball", vecSrc, GetAbsAngles(), this);
	pEntity->SetAbsVelocity(Vector(RANDOM_FLOAT(-0.7, 0.7), RANDOM_FLOAT(-0.7, 0.7), 1.0) * 600.0);
	pEntity->GreenBallInit();
}

void CNihilanth::CrashTouch(CBaseEntity* pOther)
{
	// only crash if we hit something solid
	if (pOther->GetSolidType() == Solid::BSP)
	{
		SetTouch(nullptr);
		pev->nextthink = gpGlobals->time;
	}
}

void CNihilanth::GibMonster()
{
	// EmitSound(SoundChannel::Voice, "common/bodysplat.wav", 0.75, ATTN_NORM, 200);		
}

void CNihilanth::FloatSequence()
{
	if (m_irritation >= 2)
	{
		pev->sequence = LookupSequence("float_open");
	}
	else if (m_avelocity.y > 30)
	{
		pev->sequence = LookupSequence("walk_r");
	}
	else if (m_avelocity.y < -30)
	{
		pev->sequence = LookupSequence("walk_l");
	}
	else if (m_velocity.z > 30)
	{
		pev->sequence = LookupSequence("walk_u");
	}
	else if (m_velocity.z < -30)
	{
		pev->sequence = LookupSequence("walk_d");
	}
	else
	{
		pev->sequence = LookupSequence("float");
	}
}

void CNihilanth::ShootBalls()
{
	if (m_flShootEnd > gpGlobals->time)
	{
		Vector vecHand, vecAngle;

		while (m_flShootTime < m_flShootEnd && m_flShootTime < gpGlobals->time)
		{
			if (m_hEnemy != nullptr)
			{
				GetAttachment(2, vecHand, vecAngle);
				Vector vecSrc = vecHand + GetAbsVelocity() * (m_flShootTime - gpGlobals->time);
				// vecDir = (m_posTarget - vecSrc).Normalize( );
				Vector vecDir = (m_posTarget - GetAbsOrigin()).Normalize();
				vecSrc = vecSrc + vecDir * (gpGlobals->time - m_flShootTime);
				CNihilanthHVR* pEntity = (CNihilanthHVR*)Create("nihilanth_energy_ball", vecSrc, GetAbsAngles(), this);
				pEntity->SetAbsVelocity(vecDir * 200.0);
				pEntity->ZapInit(m_hEnemy);

				GetAttachment(3, vecHand, vecAngle);
				vecSrc = vecHand + GetAbsVelocity() * (m_flShootTime - gpGlobals->time);
				// vecDir = (m_posTarget - vecSrc).Normalize( );
				vecDir = (m_posTarget - GetAbsOrigin()).Normalize();
				vecSrc = vecSrc + vecDir * (gpGlobals->time - m_flShootTime);
				pEntity = (CNihilanthHVR*)Create("nihilanth_energy_ball", vecSrc, GetAbsAngles(), this);
				pEntity->SetAbsVelocity(vecDir * 200.0);
				pEntity->ZapInit(m_hEnemy);
			}
			m_flShootTime += 0.2;
		}
	}
}

void CNihilanth::MakeFriend(Vector vecStart)
{
	for (int i = 0; i < 3; i++)
	{
		if (m_hFriend[i] != nullptr && !m_hFriend[i]->IsAlive())
		{
			if (GetRenderMode() == RenderMode::Normal) // don't do it if they are already fading
				m_hFriend[i]->MyMonsterPointer()->FadeMonster();
			m_hFriend[i] = nullptr;
		}

		if (m_hFriend[i] == nullptr)
		{
			if (RANDOM_LONG(0, 1) == 0)
			{
				const int iNode = WorldGraph.FindNearestNode(vecStart, bits_NODE_AIR);
				if (iNode != NO_NODE)
				{
					const CNode& node = WorldGraph.Node(iNode);
					TraceResult tr;
					UTIL_TraceHull(node.m_vecOrigin + Vector(0, 0, 32), node.m_vecOrigin + Vector(0, 0, 32), IgnoreMonsters::No, Hull::Large, nullptr, &tr);
					if (tr.fStartSolid == 0)
						m_hFriend[i] = Create("monster_alien_controller", node.m_vecOrigin, GetAbsAngles());
				}
			}
			else
			{
				const int iNode = WorldGraph.FindNearestNode(vecStart, bits_NODE_LAND | bits_NODE_WATER);
				if (iNode != NO_NODE)
				{
					const CNode& node = WorldGraph.Node(iNode);
					TraceResult tr;
					UTIL_TraceHull(node.m_vecOrigin + Vector(0, 0, 36), node.m_vecOrigin + Vector(0, 0, 36), IgnoreMonsters::No, Hull::Human, nullptr, &tr);
					if (tr.fStartSolid == 0)
						m_hFriend[i] = Create("monster_alien_slave", node.m_vecOrigin, GetAbsAngles());
				}
			}
			if (m_hFriend[i] != nullptr)
			{
				m_hFriend[i]->EmitSound(SoundChannel::Weapon, "debris/beamstart7.wav");
			}

			return;
		}
	}
}

void CNihilanth::NextActivity()
{
	UTIL_MakeAimVectors(GetAbsAngles());

	if (m_irritation >= 2)
	{
		auto ball = m_hBall.Get();
		if (!ball)
		{
			ball = m_hBall = CSprite::SpriteCreate("sprites/tele1.spr", GetAbsOrigin(), true);
			if (ball)
			{
				ball->SetTransparency(RenderMode::TransAdd, {255, 255, 255}, 255, RenderFX::NoDissipation);
				ball->SetAttachment(this, 1);
				ball->SetScale(4.0);
				ball->pev->framerate = 10.0;
				ball->TurnOn();
			}
		}

		if (ball)
		{
			MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
			WRITE_BYTE(TE_ELIGHT);
			WRITE_SHORT(entindex() + 0x1000);		// entity, attachment
			WRITE_COORD(GetAbsOrigin().x);		// origin
			WRITE_COORD(GetAbsOrigin().y);
			WRITE_COORD(GetAbsOrigin().z);
			WRITE_COORD(256);	// radius
			WRITE_BYTE(255);	// R
			WRITE_BYTE(192);	// G
			WRITE_BYTE(64);	// B
			WRITE_BYTE(200);	// life * 10
			WRITE_COORD(0); // decay
			MESSAGE_END();
		}
	}

	if ((pev->health < gSkillData.nihilanthHealth / 2 || m_iActiveSpheres < N_SPHERES / 2) && m_hRecharger == nullptr && m_iLevel <= 9)
	{
		char szName[128];

		CBaseEntity* pEnt = nullptr;
		CBaseEntity* pRecharger = nullptr;
		float flDist = WORLD_SIZE;

		snprintf(szName, sizeof(szName), "%s%d", m_szRechargerTarget, m_iLevel);

		while ((pEnt = UTIL_FindEntityByTargetname(pEnt, szName)) != nullptr)
		{
			float flLocal = (pEnt->GetAbsOrigin() - GetAbsOrigin()).Length();
			if (flLocal < flDist)
			{
				flDist = flLocal;
				pRecharger = pEnt;
			}
		}

		if (pRecharger)
		{
			m_hRecharger = pRecharger;
			m_posDesired = Vector(GetAbsOrigin().x, GetAbsOrigin().y, pRecharger->GetAbsOrigin().z);
			m_vecDesired = (pRecharger->GetAbsOrigin() - m_posDesired).Normalize();
			m_vecDesired.z = 0;
			m_vecDesired = m_vecDesired.Normalize();
		}
		else
		{
			m_hRecharger = nullptr;
			ALERT(at_aiconsole, "nihilanth can't find %s\n", szName);
			m_iLevel++;
			if (m_iLevel > 9)
				m_irritation = 2;
		}
	}

	const float flDist = (m_posDesired - GetAbsOrigin()).Length();
	const float flDot = DotProduct(m_vecDesired, gpGlobals->v_forward);

	if (m_hRecharger != nullptr)
	{
		// at we at power up yet?
		if (flDist < 128.0)
		{
			int iseq = LookupSequence("recharge");

			if (iseq != pev->sequence)
			{
				char szText[128];

				snprintf(szText, sizeof(szText), "%s%d", m_szDrawUse, m_iLevel);
				FireTargets(szText, this, this, UseType::On, 1.0);

				ALERT(at_console, "fireing %s\n", szText);
			}
			pev->sequence = LookupSequence("recharge");
		}
		else
		{
			FloatSequence();
		}
		return;
	}

	if (m_hEnemy != nullptr && !m_hEnemy->IsAlive())
	{
		m_hEnemy = nullptr;
	}

	if (m_flLastSeen + 15 < gpGlobals->time)
	{
		m_hEnemy = nullptr;
	}

	if (m_hEnemy == nullptr)
	{
		Look(WORLD_BOUNDARY);
		m_hEnemy = BestVisibleEnemy();
	}

	if (m_hEnemy != nullptr && m_irritation != 0)
	{
		if (m_flLastSeen + 5 > gpGlobals->time && flDist < 256 && flDot > 0)
		{
			if (m_irritation >= 2 && pev->health < gSkillData.nihilanthHealth / 2.0)
			{
				pev->sequence = LookupSequence("attack1_open");
			}
			else
			{
				if (RANDOM_LONG(0, 1) == 0)
				{
					pev->sequence = LookupSequence("attack1"); // zap
				}
				else
				{
					char szText[128];

					snprintf(szText, sizeof(szText), "%s%d", m_szTeleportTouch, m_iTeleport);
					CBaseEntity* pTouch = UTIL_FindEntityByTargetname(nullptr, szText);

					snprintf(szText, sizeof(szText), "%s%d", m_szTeleportUse, m_iTeleport);
					CBaseEntity* pTrigger = UTIL_FindEntityByTargetname(nullptr, szText);

					if (pTrigger != nullptr || pTouch != nullptr)
					{
						pev->sequence = LookupSequence("attack2"); // teleport
					}
					else
					{
						m_iTeleport++;
						pev->sequence = LookupSequence("attack1"); // zap
					}
				}
			}
			return;
		}
	}

	FloatSequence();
}

void CNihilanth::HuntThink()
{
	pev->nextthink = gpGlobals->time + 0.1;
	DispatchAnimEvents();
	StudioFrameAdvance();

	ShootBalls();

	// if dead, force cancelation of current animation
	if (pev->health <= 0)
	{
		SetThink(&CNihilanth::DyingThink);
		m_fSequenceFinished = true;
		return;
	}

	// ALERT( at_console, "health %.0f\n", pev->health );

	// if damaged, try to abosorb some spheres
	if (pev->health < gSkillData.nihilanthHealth && AbsorbSphere())
	{
		pev->health += gSkillData.nihilanthHealth / N_SPHERES;
	}

	// get new sequence
	if (m_fSequenceFinished)
	{
		// if (!m_fSequenceLoops)
		pev->frame = 0;
		NextActivity();
		ResetSequenceInfo();
		pev->framerate = 2.0 - 1.0 * (pev->health / gSkillData.nihilanthHealth);
	}

	// look for current enemy	
	if (m_hEnemy != nullptr && m_hRecharger == nullptr)
	{
		if (IsVisible(m_hEnemy))
		{
			if (m_flLastSeen < gpGlobals->time - 5)
				m_flPrevSeen = gpGlobals->time;
			m_flLastSeen = gpGlobals->time;
			m_posTarget = m_hEnemy->GetAbsOrigin();
			m_vecTarget = (m_posTarget - GetAbsOrigin()).Normalize();
			m_vecDesired = m_vecTarget;
			m_posDesired = Vector(GetAbsOrigin().x, GetAbsOrigin().y, m_posTarget.z + m_flAdj);
		}
		else
		{
			m_flAdj = std::min(m_flAdj + 10, 1000.0f);
		}
	}

	// don't go too high
	if (m_posDesired.z > m_flMaxZ)
		m_posDesired.z = m_flMaxZ;

	// don't go too low
	if (m_posDesired.z < m_flMinZ)
		m_posDesired.z = m_flMinZ;

	Flight();
}

void CNihilanth::Flight()
{
	// estimate where I'll be facing in one seconds
	UTIL_MakeAimVectors(GetAbsAngles() + m_avelocity);
	// Vector vecEst1 = GetAbsOrigin() + m_velocity + gpGlobals->v_up * m_flForce - Vector( 0, 0, 384 );
	// float flSide = DotProduct( m_posDesired - vecEst1, gpGlobals->v_right );

	const float flSide = DotProduct(m_vecDesired, gpGlobals->v_right);

	if (flSide < 0)
	{
		if (m_avelocity.y < 180)
		{
			m_avelocity.y += 6; // 9 * (3.0/2.0);
		}
	}
	else
	{
		if (m_avelocity.y > -180)
		{
			m_avelocity.y -= 6; // 9 * (3.0/2.0);
		}
	}
	m_avelocity.y *= 0.98;

	// estimate where I'll be in two seconds
	const Vector vecEst = GetAbsOrigin() + m_velocity * 2.0 + gpGlobals->v_up * m_flForce * 20;

	// add immediate force
	UTIL_MakeAimVectors(GetAbsAngles());
	m_velocity.x += gpGlobals->v_up.x * m_flForce;
	m_velocity.y += gpGlobals->v_up.y * m_flForce;
	m_velocity.z += gpGlobals->v_up.z * m_flForce;

	/*
	float flSpeed = m_velocity.Length();
	const float flDir = DotProduct(Vector(gpGlobals->v_forward.x, gpGlobals->v_forward.y, 0), Vector(m_velocity.x, m_velocity.y, 0));
	if (flDir < 0)
		flSpeed = -flSpeed;

	const float flDist = DotProduct(m_posDesired - vecEst, gpGlobals->v_forward);
	*/

	// sideways drag
	m_velocity.x = m_velocity.x * (1.0 - fabs(gpGlobals->v_right.x) * 0.05);
	m_velocity.y = m_velocity.y * (1.0 - fabs(gpGlobals->v_right.y) * 0.05);
	m_velocity.z = m_velocity.z * (1.0 - fabs(gpGlobals->v_right.z) * 0.05);

	// general drag
	m_velocity = m_velocity * 0.995;

	// apply power to stay correct height
	if (m_flForce < 100 && vecEst.z < m_posDesired.z)
	{
		m_flForce += 10;
	}
	else if (m_flForce > -100 && vecEst.z > m_posDesired.z)
	{
		if (vecEst.z > m_posDesired.z)
			m_flForce -= 10;
	}

	SetAbsOrigin(GetAbsOrigin() + m_velocity * 0.1);
	SetAbsAngles(GetAbsAngles() + m_avelocity * 0.1);

	// ALERT( at_console, "%5.0f %5.0f : %4.0f : %3.0f : %2.0f\n", m_posDesired.z, GetAbsOrigin().z, m_velocity.z, m_avelocity.y, m_flForce ); 
}

bool CNihilanth::AbsorbSphere()
{
	for (int i = 0; i < N_SPHERES; i++)
	{
		if (m_hSphere[i] != nullptr)
		{
			CNihilanthHVR* pSphere = m_hSphere[i];
			pSphere->AbsorbInit();
			m_hSphere[i] = nullptr;
			m_iActiveSpheres--;
			return true;
		}
	}
	return false;
}

bool CNihilanth::EmitSphere()
{
	m_iActiveSpheres = 0;
	int empty = 0;

	for (int i = 0; i < N_SPHERES; i++)
	{
		if (m_hSphere[i] != nullptr)
		{
			m_iActiveSpheres++;
		}
		else
		{
			empty = i;
		}
	}

	if (m_iActiveSpheres >= N_SPHERES)
		return false;

	const Vector vecSrc = m_hRecharger->GetAbsOrigin();
	CNihilanthHVR* pEntity = (CNihilanthHVR*)Create("nihilanth_energy_ball", vecSrc, GetAbsAngles(), this);
	pEntity->SetAbsVelocity(GetAbsOrigin() - vecSrc);
	pEntity->CircleInit(this);

	m_hSphere[empty] = pEntity;
	return true;
}

void CNihilanth::TargetSphere(UseType useType, float value)
{
	CNihilanthHVR* pSphere;
	int i;
	for (i = 0; i < N_SPHERES; i++)
	{
		if (m_hSphere[i] != nullptr)
		{
			pSphere = m_hSphere[i];
			if (pSphere->m_hEnemy == nullptr)
				break;
		}
	}
	if (i == N_SPHERES)
	{
		return;
	}

	Vector vecSrc, vecAngles;
	GetAttachment(2, vecSrc, vecAngles);
	pSphere->SetAbsOrigin(vecSrc);
	pSphere->Use({this, this, useType, value});
	pSphere->SetAbsVelocity(m_vecDesired * RANDOM_FLOAT(50, 100) + Vector(RANDOM_FLOAT(-50, 50), RANDOM_FLOAT(-50, 50), RANDOM_FLOAT(-50, 50)));
}

void CNihilanth::HandleAnimEvent(AnimationEvent& event)
{
	switch (event.event)
	{
	case 1:	// shoot 
		break;
	case 2:	// zen
		if (m_hEnemy != nullptr)
		{
			if (RANDOM_LONG(0, 4) == 0)
				EmitSound(SoundChannel::Voice, RANDOM_SOUND_ARRAY(pAttackSounds), VOL_NORM, 0.2);

			EmitSound(SoundChannel::Weapon, RANDOM_SOUND_ARRAY(pBallSounds), VOL_NORM, 0.2);

			MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
			WRITE_BYTE(TE_ELIGHT);
			WRITE_SHORT(entindex() + 0x3000);		// entity, attachment
			WRITE_COORD(GetAbsOrigin().x);		// origin
			WRITE_COORD(GetAbsOrigin().y);
			WRITE_COORD(GetAbsOrigin().z);
			WRITE_COORD(256);	// radius
			WRITE_BYTE(128);	// R
			WRITE_BYTE(128);	// G
			WRITE_BYTE(255);	// B
			WRITE_BYTE(10);	// life * 10
			WRITE_COORD(128); // decay
			MESSAGE_END();

			MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
			WRITE_BYTE(TE_ELIGHT);
			WRITE_SHORT(entindex() + 0x4000);		// entity, attachment
			WRITE_COORD(GetAbsOrigin().x);		// origin
			WRITE_COORD(GetAbsOrigin().y);
			WRITE_COORD(GetAbsOrigin().z);
			WRITE_COORD(256);	// radius
			WRITE_BYTE(128);	// R
			WRITE_BYTE(128);	// G
			WRITE_BYTE(255);	// B
			WRITE_BYTE(10);	// life * 10
			WRITE_COORD(128); // decay
			MESSAGE_END();

			m_flShootTime = gpGlobals->time;
			m_flShootEnd = gpGlobals->time + 1.0;
		}
		break;
	case 3:	// prayer
		if (m_hEnemy != nullptr)
		{
			char szText[128];

			snprintf(szText, sizeof(szText), "%s%d", m_szTeleportTouch, m_iTeleport);
			CBaseEntity* pTouch = UTIL_FindEntityByTargetname(nullptr, szText);

			snprintf(szText, sizeof(szText), "%s%d", m_szTeleportUse, m_iTeleport);
			CBaseEntity* pTrigger = UTIL_FindEntityByTargetname(nullptr, szText);

			if (pTrigger != nullptr || pTouch != nullptr)
			{
				EmitSound(SoundChannel::Voice, RANDOM_SOUND_ARRAY(pAttackSounds), VOL_NORM, 0.2);

				Vector vecSrc, vecAngles;
				GetAttachment(2, vecSrc, vecAngles);
				CNihilanthHVR* pEntity = (CNihilanthHVR*)Create("nihilanth_energy_ball", vecSrc, GetAbsAngles(), this);
				pEntity->SetAbsVelocity(GetAbsOrigin() - vecSrc);
				pEntity->TeleportInit(this, m_hEnemy, pTrigger, pTouch);
			}
			else
			{
				m_iTeleport++; // unexpected failure

				EmitSound(SoundChannel::Weapon, RANDOM_SOUND_ARRAY(pBallSounds), VOL_NORM, 0.2);

				ALERT(at_aiconsole, "nihilanth can't target %s\n", szText);

				MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
				WRITE_BYTE(TE_ELIGHT);
				WRITE_SHORT(entindex() + 0x3000);		// entity, attachment
				WRITE_COORD(GetAbsOrigin().x);		// origin
				WRITE_COORD(GetAbsOrigin().y);
				WRITE_COORD(GetAbsOrigin().z);
				WRITE_COORD(256);	// radius
				WRITE_BYTE(128);	// R
				WRITE_BYTE(128);	// G
				WRITE_BYTE(255);	// B
				WRITE_BYTE(10);	// life * 10
				WRITE_COORD(128); // decay
				MESSAGE_END();

				MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
				WRITE_BYTE(TE_ELIGHT);
				WRITE_SHORT(entindex() + 0x4000);		// entity, attachment
				WRITE_COORD(GetAbsOrigin().x);		// origin
				WRITE_COORD(GetAbsOrigin().y);
				WRITE_COORD(GetAbsOrigin().z);
				WRITE_COORD(256);	// radius
				WRITE_BYTE(128);	// R
				WRITE_BYTE(128);	// G
				WRITE_BYTE(255);	// B
				WRITE_BYTE(10);	// life * 10
				WRITE_COORD(128); // decay
				MESSAGE_END();

				m_flShootTime = gpGlobals->time;
				m_flShootEnd = gpGlobals->time + 1.0;
			}
		}
		break;
	case 4:	// get a sphere
	{
		if (m_hRecharger != nullptr)
		{
			if (!EmitSphere())
			{
				m_hRecharger = nullptr;
			}
		}
	}
	break;
	case 5:	// start up sphere machine
	{
		EmitSound(SoundChannel::Voice, RANDOM_SOUND_ARRAY(pRechargeSounds), VOL_NORM, 0.2);
	}
	break;
	case 6:
		if (m_hEnemy != nullptr)
		{
			Vector vecSrc, vecAngles;
			GetAttachment(2, vecSrc, vecAngles);
			CNihilanthHVR* pEntity = (CNihilanthHVR*)Create("nihilanth_energy_ball", vecSrc, GetAbsAngles(), this);
			pEntity->SetAbsVelocity(GetAbsOrigin() - vecSrc);
			pEntity->ZapInit(m_hEnemy);
		}
		break;
	case 7:
		/*
		Vector vecSrc, vecAngles;
		GetAttachment( 0, vecSrc, vecAngles );
		CNihilanthHVR *pEntity = (CNihilanthHVR *)Create( "nihilanth_energy_ball", vecSrc, GetAbsAngles(), this );
		pEntity->SetAbsVelocity(Vector ( RANDOM_FLOAT( -0.7, 0.7 ), RANDOM_FLOAT( -0.7, 0.7 ), 1.0 ) * 600.0);
		pEntity->GreenBallInit( );
		*/
		break;
	}
}

void CNihilanth::CommandUse(const UseInfo& info)
{
	switch (info.GetUseType())
	{
	case UseType::Off:
	{
		if (CBaseEntity* pTouch = UTIL_FindEntityByTargetname(nullptr, m_szDeadTouch); pTouch)
		{
			if (m_hEnemy != nullptr)
			{
				pTouch->Touch(m_hEnemy);
			}
			// if the player is using "notarget", the ending sequence won't fire unless we catch it here
			else
			{
				if (CBaseEntity* pEntity = UTIL_FindEntityByClassname(nullptr, "player"); pEntity != nullptr && pEntity->IsAlive())
				{
					pTouch->Touch(pEntity);
				}
			}
		}
	}
	break;
	case UseType::On:
		if (m_irritation == 0)
		{
			m_irritation = 1;
		}
		break;
	case UseType::Set:
		break;
	case UseType::Toggle:
		break;
	}
}

bool CNihilanth::TakeDamage(const TakeDamageInfo& info)
{
	if (info.GetInflictor()->GetOwner() == this)
		return false;

	if (info.GetDamage() >= pev->health)
	{
		pev->health = 1;
		if (m_irritation != 3)
			return false;
	}

	PainSound();

	pev->health -= info.GetDamage();
	return false;
}

void CNihilanth::TraceAttack(const TraceAttackInfo& info)
{
	if (m_irritation == 3)
		m_irritation = 2;

	if (m_irritation == 2 && info.GetTraceResult().iHitgroup == 2 && info.GetDamage() > 2)
		m_irritation = 3;

	if (m_irritation != 3)
	{
		const Vector vecBlood = (info.GetTraceResult().vecEndPos - GetAbsOrigin()).Normalize();

		UTIL_BloodStream(info.GetTraceResult().vecEndPos, vecBlood, BloodColor(), info.GetDamage() + (100 - 100 * (pev->health / gSkillData.nihilanthHealth)));
	}

	// SpawnBlood(ptr->vecEndPos, BloodColor(), flDamage * 5.0);// a little surface blood.
	AddMultiDamage(info.GetAttacker(), this, info.GetDamage(), info.GetDamageTypes());
}

CBaseEntity* CNihilanth::RandomTargetname(const char* szName)
{
	int total = 0;

	CBaseEntity* pEntity = nullptr;
	CBaseEntity* pNewEntity = nullptr;
	while ((pNewEntity = UTIL_FindEntityByTargetname(pNewEntity, szName)) != nullptr)
	{
		total++;
		if (RANDOM_LONG(0, total - 1) < 1)
			pEntity = pNewEntity;
	}
	return pEntity;
}

void CNihilanthHVR::Spawn()
{
	Precache();

	SetRenderMode(RenderMode::TransAdd);
	SetRenderAmount(255);
	pev->scale = 3.0;
}

void CNihilanthHVR::Precache()
{
	PRECACHE_MODEL("sprites/flare6.spr");
	PRECACHE_MODEL("sprites/nhth1.spr");
	PRECACHE_MODEL("sprites/exit1.spr");
	PRECACHE_MODEL("sprites/tele1.spr");
	PRECACHE_MODEL("sprites/animglow01.spr");
	PRECACHE_MODEL("sprites/xspark4.spr");
	PRECACHE_MODEL("sprites/muzzleflash3.spr");
	PRECACHE_SOUND("debris/zap4.wav");
	PRECACHE_SOUND("weapons/electro4.wav");
	PRECACHE_SOUND("x/x_teleattack1.wav");
}

void CNihilanthHVR::CircleInit(CBaseEntity* pTarget)
{
	SetMovetype(Movetype::Noclip);
	SetSolidType(Solid::Not);

	// SetModel( "sprites/flare6.spr");
	// pev->scale = 3.0;
	// SetModel( "sprites/xspark4.spr");
	SetModel("sprites/muzzleflash3.spr");
	SetRenderColor({255, 224, 192});
	pev->scale = 2.0;
	m_nFrames = 1;
	SetRenderAmount(255);

	SetSize(vec3_origin, vec3_origin);
	SetAbsOrigin(GetAbsOrigin());

	SetThink(&CNihilanthHVR::HoverThink);
	SetTouch(&CNihilanthHVR::BounceTouch);
	pev->nextthink = gpGlobals->time + 0.1;

	m_hTargetEnt = pTarget;
}

CBaseEntity* CNihilanthHVR::RandomClassname(const char* szName)
{
	int total = 0;

	CBaseEntity* pEntity = nullptr;
	CBaseEntity* pNewEntity = nullptr;
	while ((pNewEntity = UTIL_FindEntityByClassname(pNewEntity, szName)) != nullptr)
	{
		total++;
		if (RANDOM_LONG(0, total - 1) < 1)
			pEntity = pNewEntity;
	}
	return pEntity;
}

void CNihilanthHVR::HoverThink()
{
	pev->nextthink = gpGlobals->time + 0.1;

	if (m_hTargetEnt != nullptr)
	{
		CircleTarget(m_hTargetEnt->GetAbsOrigin() + Vector(0, 0, 16 * N_SCALE));
	}
	else
	{
		UTIL_Remove(this);
	}

	if (RANDOM_LONG(0, 99) < 5)
	{
		/*
				CBaseEntity *pOther = RandomClassname( GetClassname() );

				if (pOther && pOther != this)
				{
					MESSAGE_BEGIN( MessageDest::Broadcast, SVC_TEMPENTITY );
						WRITE_BYTE( TE_BEAMENTS );
						WRITE_SHORT( this->entindex() );
						WRITE_SHORT( pOther->entindex() );
						WRITE_SHORT( g_sModelIndexLaser );
						WRITE_BYTE( 0 ); // framestart
						WRITE_BYTE( 0 ); // framerate
						WRITE_BYTE( 10 ); // life
						WRITE_BYTE( 80 );  // width
						WRITE_BYTE( 80 );   // noise
						WRITE_BYTE( 255 );   // r, g, b
						WRITE_BYTE( 128 );   // r, g, b
						WRITE_BYTE( 64 );   // r, g, b
						WRITE_BYTE( 255 );	// brightness
						WRITE_BYTE( 30 );		// speed
					MESSAGE_END();
				}
		*/
		/*
				MESSAGE_BEGIN( MessageDest::Broadcast, SVC_TEMPENTITY );
					WRITE_BYTE( TE_BEAMENTS );
					WRITE_SHORT( this->entindex() );
					WRITE_SHORT( m_hTargetEnt->entindex() + 0x1000 );
					WRITE_SHORT( g_sModelIndexLaser );
					WRITE_BYTE( 0 ); // framestart
					WRITE_BYTE( 0 ); // framerate
					WRITE_BYTE( 10 ); // life
					WRITE_BYTE( 80 );  // width
					WRITE_BYTE( 80 );   // noise
					WRITE_BYTE( 255 );   // r, g, b
					WRITE_BYTE( 128 );   // r, g, b
					WRITE_BYTE( 64 );   // r, g, b
					WRITE_BYTE( 255 );	// brightness
					WRITE_BYTE( 30 );		// speed
				MESSAGE_END();
		*/
	}

	pev->frame = ((int)pev->frame + 1) % m_nFrames;
}

void CNihilanthHVR::ZapInit(CBaseEntity* pEnemy)
{
	SetMovetype(Movetype::Fly);
	SetSolidType(Solid::BBox);

	SetModel("sprites/nhth1.spr");

	SetRenderColor({255, 255, 255});
	pev->scale = 2.0;

	SetAbsVelocity((pEnemy->GetAbsOrigin() - GetAbsOrigin()).Normalize() * 200);

	m_hEnemy = pEnemy;
	SetThink(&CNihilanthHVR::ZapThink);
	SetTouch(&CNihilanthHVR::ZapTouch);
	pev->nextthink = gpGlobals->time + 0.1;

	EmitSound(SoundChannel::Weapon, "debris/zap4.wav");
}

void CNihilanthHVR::ZapThink()
{
	pev->nextthink = gpGlobals->time + 0.05;

	// check world boundaries
	if (m_hEnemy == nullptr || !UTIL_IsInWorld(GetAbsOrigin()))
	{
		SetTouch(nullptr);
		UTIL_Remove(this);
		return;
	}

	if (GetAbsVelocity().Length() < 2000)
	{
		SetAbsVelocity(GetAbsVelocity() * 1.2);
	}

	// MovetoTarget( m_hEnemy->Center( ) );

	if ((m_hEnemy->Center() - GetAbsOrigin()).Length() < 256)
	{
		TraceResult tr;

		UTIL_TraceLine(GetAbsOrigin(), m_hEnemy->Center(), IgnoreMonsters::No, this, &tr);

		if (CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit); pEntity != nullptr && pEntity->pev->takedamage)
		{
			ClearMultiDamage();
			pEntity->TraceAttack({this, gSkillData.nihilanthZap, GetAbsVelocity(), tr, DMG_SHOCK});
			ApplyMultiDamage(this, this);
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
		WRITE_BYTE(20);   // noise
		WRITE_BYTE(64);   // r, g, b
		WRITE_BYTE(196);   // r, g, b
		WRITE_BYTE(255);   // r, g, b
		WRITE_BYTE(255);	// brightness
		WRITE_BYTE(10);		// speed
		MESSAGE_END();

		UTIL_EmitAmbientSound(this, tr.vecEndPos, "weapons/electro4.wav", 0.5, ATTN_NORM, 0, RANDOM_LONG(140, 160));

		SetTouch(nullptr);
		UTIL_Remove(this);
		pev->nextthink = gpGlobals->time + 0.2;
		return;
	}

	pev->frame = (int)(pev->frame + 1) % 11;

	MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
	WRITE_BYTE(TE_ELIGHT);
	WRITE_SHORT(entindex());		// entity, attachment
	WRITE_COORD(GetAbsOrigin().x);		// origin
	WRITE_COORD(GetAbsOrigin().y);
	WRITE_COORD(GetAbsOrigin().z);
	WRITE_COORD(128);	// radius
	WRITE_BYTE(128);	// R
	WRITE_BYTE(128);	// G
	WRITE_BYTE(255);	// B
	WRITE_BYTE(10);	// life * 10
	WRITE_COORD(128); // decay
	MESSAGE_END();

	// Crawl( );
}

void CNihilanthHVR::ZapTouch(CBaseEntity* pOther)
{
	UTIL_EmitAmbientSound(this, GetAbsOrigin(), "weapons/electro4.wav", 1.0, ATTN_NORM, 0, RANDOM_LONG(90, 95));

	RadiusDamage(this, this, 50, CLASS_NONE, DMG_SHOCK);
	SetAbsVelocity(GetAbsVelocity() * 0);

	/*
	for (int i = 0; i < 10; i++)
	{
		Crawl( );
	}
	*/

	SetTouch(nullptr);
	UTIL_Remove(this);
	pev->nextthink = gpGlobals->time + 0.2;
}

void CNihilanthHVR::TeleportInit(CNihilanth* pOwner, CBaseEntity* pEnemy, CBaseEntity* pTarget, CBaseEntity* pTouch)
{
	SetMovetype(Movetype::Fly);
	SetSolidType(Solid::BBox);

	SetRenderColor({255, 255, 255});
	SetAbsVelocity({GetAbsVelocity().x, GetAbsVelocity().y, GetAbsVelocity().z * 0.2f});

	SetModel("sprites/exit1.spr");

	m_hNihilanth = pOwner;
	m_hEnemy = pEnemy;
	m_hTargetEnt = pTarget;
	m_hTouch = pTouch;

	SetThink(&CNihilanthHVR::TeleportThink);
	SetTouch(&CNihilanthHVR::TeleportTouch);
	pev->nextthink = gpGlobals->time + 0.1;

	EmitSound(SoundChannel::Weapon, "x/x_teleattack1.wav", VOL_NORM, 0.2);
}

void CNihilanthHVR::GreenBallInit()
{
	SetMovetype(Movetype::Fly);
	SetSolidType(Solid::BBox);

	SetRenderColor({255, 255, 255});
	pev->scale = 1.0;

	SetModel("sprites/exit1.spr");

	SetTouch(&CNihilanthHVR::RemoveTouch);
}

void CNihilanthHVR::TeleportThink()
{
	pev->nextthink = gpGlobals->time + 0.1;

	// check world boundaries
	if (m_hEnemy == nullptr || !m_hEnemy->IsAlive() || !UTIL_IsInWorld(GetAbsOrigin()))
	{
		StopSound(SoundChannel::Weapon, "x/x_teleattack1.wav");
		UTIL_Remove(this);
		return;
	}

	if ((m_hEnemy->Center() - GetAbsOrigin()).Length() < 128)
	{
		StopSound(SoundChannel::Weapon, "x/x_teleattack1.wav");
		UTIL_Remove(this);

		if (m_hTargetEnt != nullptr)
			m_hTargetEnt->Use({m_hEnemy, m_hEnemy, UseType::On, 1.0});

		if (m_hTouch != nullptr && m_hEnemy != nullptr)
			m_hTouch->Touch(m_hEnemy);
	}
	else
	{
		MovetoTarget(m_hEnemy->Center());
	}

	MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
	WRITE_BYTE(TE_ELIGHT);
	WRITE_SHORT(entindex());		// entity, attachment
	WRITE_COORD(GetAbsOrigin().x);		// origin
	WRITE_COORD(GetAbsOrigin().y);
	WRITE_COORD(GetAbsOrigin().z);
	WRITE_COORD(256);	// radius
	WRITE_BYTE(0);	// R
	WRITE_BYTE(255);	// G
	WRITE_BYTE(0);	// B
	WRITE_BYTE(10);	// life * 10
	WRITE_COORD(256); // decay
	MESSAGE_END();

	pev->frame = (int)(pev->frame + 1) % 20;
}

void CNihilanthHVR::AbsorbInit()
{
	SetThink(&CNihilanthHVR::DissipateThink);
	SetRenderAmount(255);

	MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMENTS);
	WRITE_SHORT(this->entindex());
	WRITE_SHORT(m_hTargetEnt->entindex() + 0x1000);
	WRITE_SHORT(g_sModelIndexLaser);
	WRITE_BYTE(0); // framestart
	WRITE_BYTE(0); // framerate
	WRITE_BYTE(50); // life
	WRITE_BYTE(80);  // width
	WRITE_BYTE(80);   // noise
	WRITE_BYTE(255);   // r, g, b
	WRITE_BYTE(128);   // r, g, b
	WRITE_BYTE(64);   // r, g, b
	WRITE_BYTE(255);	// brightness
	WRITE_BYTE(30);		// speed
	MESSAGE_END();
}

void CNihilanthHVR::TeleportTouch(CBaseEntity* pOther)
{
	CBaseEntity* pEnemy = m_hEnemy;

	if (pOther == pEnemy)
	{
		if (m_hTargetEnt != nullptr)
			m_hTargetEnt->Use({pEnemy, pEnemy, UseType::On, 1.0});

		if (m_hTouch != nullptr && pEnemy != nullptr)
			m_hTouch->Touch(pEnemy);
	}
	else
	{
		m_hNihilanth->MakeFriend(GetAbsOrigin());
	}

	SetTouch(nullptr);
	StopSound(SoundChannel::Weapon, "x/x_teleattack1.wav");
	UTIL_Remove(this);
}

void CNihilanthHVR::DissipateThink()
{
	pev->nextthink = gpGlobals->time + 0.1;

	if (pev->scale > 5.0)
		UTIL_Remove(this);

	SetRenderAmount(GetRenderAmount() - 2);
	pev->scale += 0.1;

	if (m_hTargetEnt != nullptr)
	{
		CircleTarget(m_hTargetEnt->GetAbsOrigin() + Vector(0, 0, WORLD_BOUNDARY));
	}
	else
	{
		UTIL_Remove(this);
	}

	MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
	WRITE_BYTE(TE_ELIGHT);
	WRITE_SHORT(entindex());		// entity, attachment
	WRITE_COORD(GetAbsOrigin().x);		// origin
	WRITE_COORD(GetAbsOrigin().y);
	WRITE_COORD(GetAbsOrigin().z);
	WRITE_COORD(GetRenderAmount());	// radius
	WRITE_BYTE(255);	// R
	WRITE_BYTE(192);	// G
	WRITE_BYTE(64);	// B
	WRITE_BYTE(2);	// life * 10
	WRITE_COORD(0); // decay
	MESSAGE_END();
}

bool CNihilanthHVR::CircleTarget(Vector vecTarget)
{
	bool fClose = false;

	Vector vecDest = vecTarget;
	Vector vecEst = GetAbsOrigin() + GetAbsVelocity() * 0.5;
	Vector vecSrc = GetAbsOrigin();
	vecDest.z = 0;
	vecEst.z = 0;
	vecSrc.z = 0;
	float d1 = (vecDest - vecSrc).Length() - 24 * N_SCALE;
	const float d2 = (vecDest - vecEst).Length() - 24 * N_SCALE;

	if (m_vecIdeal == vec3_origin)
	{
		m_vecIdeal = GetAbsVelocity();
	}

	if (d1 < 0 && d2 <= d1)
	{
		// ALERT( at_console, "too close\n");
		m_vecIdeal = m_vecIdeal - (vecDest - vecSrc).Normalize() * 50;
	}
	else if (d1 > 0 && d2 >= d1)
	{
		// ALERT( at_console, "too far\n");
		m_vecIdeal = m_vecIdeal + (vecDest - vecSrc).Normalize() * 50;
	}
	pev->avelocity.z = d1 * 20;

	if (d1 < 32)
	{
		fClose = true;
	}

	m_vecIdeal = m_vecIdeal + Vector(RANDOM_FLOAT(-2, 2), RANDOM_FLOAT(-2, 2), RANDOM_FLOAT(-2, 2));
	m_vecIdeal = Vector(m_vecIdeal.x, m_vecIdeal.y, 0).Normalize() * 200
		/* + Vector( -m_vecIdeal.y, m_vecIdeal.x, 0 ).Normalize( ) * 32 */
		+ Vector(0, 0, m_vecIdeal.z);
	// m_vecIdeal = m_vecIdeal + Vector( -m_vecIdeal.y, m_vecIdeal.x, 0 ).Normalize( ) * 2;

	// move up/down
	d1 = vecTarget.z - GetAbsOrigin().z;
	if (d1 > 0 && m_vecIdeal.z < 200)
		m_vecIdeal.z += 20;
	else if (d1 < 0 && m_vecIdeal.z > -200)
		m_vecIdeal.z -= 20;

	SetAbsVelocity(m_vecIdeal);

	// ALERT( at_console, "%.0f %.0f %.0f\n", m_vecIdeal.x, m_vecIdeal.y, m_vecIdeal.z );
	return fClose;
}

void CNihilanthHVR::MovetoTarget(Vector vecTarget)
{
	if (m_vecIdeal == vec3_origin)
	{
		m_vecIdeal = GetAbsVelocity();
	}

	// accelerate
	const float flSpeed = m_vecIdeal.Length();
	if (flSpeed > 300)
	{
		m_vecIdeal = m_vecIdeal.Normalize() * 300;
	}
	m_vecIdeal = m_vecIdeal + (vecTarget - GetAbsOrigin()).Normalize() * 300;
	SetAbsVelocity(m_vecIdeal);
}

void CNihilanthHVR::Crawl()
{
	const Vector vecAim = Vector(RANDOM_FLOAT(-1, 1), RANDOM_FLOAT(-1, 1), RANDOM_FLOAT(-1, 1)).Normalize();
	const Vector vecPnt = GetAbsOrigin() + GetAbsVelocity() * 0.2 + vecAim * 128;

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
	WRITE_BYTE(80);   // noise
	WRITE_BYTE(64);   // r, g, b
	WRITE_BYTE(128);   // r, g, b
	WRITE_BYTE(255);   // r, g, b
	WRITE_BYTE(255);	// brightness
	WRITE_BYTE(10);		// speed
	MESSAGE_END();
}

void CNihilanthHVR::RemoveTouch(CBaseEntity* pOther)
{
	StopSound(SoundChannel::Weapon, "x/x_teleattack1.wav");
	UTIL_Remove(this);
}

void CNihilanthHVR::BounceTouch(CBaseEntity* pOther)
{
	Vector vecDir = m_vecIdeal.Normalize();

	const TraceResult tr = UTIL_GetGlobalTrace();

	const float n = -DotProduct(tr.vecPlaneNormal, vecDir);

	vecDir = 2.0 * tr.vecPlaneNormal * n + vecDir;

	m_vecIdeal = vecDir * m_vecIdeal.Length();
}
