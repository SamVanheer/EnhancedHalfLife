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

#include "CNihilanth.hpp"
#include "CNihilanthHVR.hpp"
#include "navigation/nodes.hpp"

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

void CNihilanth::OnRemove()
{
	for (auto& sphereHandle : m_hSphere)
	{
		sphereHandle.Remove();
	}

	m_hBall.Remove();

	CBaseMonster::OnRemove();
}

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
			m_hBall.Remove();
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
			else if (!g_pGameRules->IsMultiplayer())
			{
				if (CBaseEntity* pEntity = UTIL_GetLocalPlayer(); pEntity != nullptr && pEntity->IsAlive())
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
