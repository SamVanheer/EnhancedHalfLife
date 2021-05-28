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

#include "CBarnacle.hpp"

LINK_ENTITY_TO_CLASS(monster_barnacle, CBarnacle);

int	CBarnacle::Classify()
{
	return	CLASS_ALIEN_MONSTER;
}

void CBarnacle::HandleAnimEvent(AnimationEvent& event)
{
	switch (event.event)
	{
	case BARNACLE_AE_PUKEGIB:
		CGib::SpawnRandomGibs(this, 1, true);
		break;
	default:
		CBaseMonster::HandleAnimEvent(event);
		break;
	}
}

void CBarnacle::Spawn()
{
	Precache();

	SetModel("models/barnacle.mdl");
	SetSize(Vector(-16, -16, -32), Vector(16, 16, 0));

	SetSolidType(Solid::SlideBox);
	SetMovetype(Movetype::None);
	SetDamageMode(DamageMode::Aim);
	m_bloodColor = BLOOD_COLOR_RED;
	pev->effects = EF_INVLIGHT; // take light from the ceiling 
	pev->health = 25;
	m_flFieldOfView = 0.5;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState = NPCState::None;
	m_flKillVictimTime = 0;
	m_cGibs = 0;
	m_fLiftingPrey = false;
	m_flTongueAdj = -100;

	InitBoneControllers();

	SetActivity(ACT_IDLE);

	SetThink(&CBarnacle::BarnacleThink);
	pev->nextthink = gpGlobals->time + 0.5;

	SetAbsOrigin(GetAbsOrigin());
}

bool CBarnacle::TakeDamage(const TakeDamageInfo& info)
{
	TakeDamageInfo adjustedInfo = info;

	if (adjustedInfo.GetDamageTypes() & DMG_CLUB)
	{
		adjustedInfo.SetDamage(pev->health);
	}

	return CBaseMonster::TakeDamage(adjustedInfo);
}

void CBarnacle::BarnacleThink()
{
	pev->nextthink = gpGlobals->time + 0.1;

	if (m_hEnemy != nullptr)
	{
		// barnacle has prey.

		if (!m_hEnemy->IsAlive())
		{
			// someone (maybe even the barnacle) killed the prey. Reset barnacle.
			m_fLiftingPrey = false;// indicate that we're not lifting prey.
			m_hEnemy = nullptr;
			return;
		}

		if (m_fLiftingPrey)
		{
			if (m_hEnemy != nullptr && m_hEnemy->pev->deadflag != DeadFlag::No)
			{
				// crap, someone killed the prey on the way up.
				m_hEnemy = nullptr;
				m_fLiftingPrey = false;
				return;
			}

			// still pulling prey.
			Vector vecNewEnemyOrigin = m_hEnemy->GetAbsOrigin();
			vecNewEnemyOrigin.x = GetAbsOrigin().x;
			vecNewEnemyOrigin.y = GetAbsOrigin().y;

			// guess as to where their neck is
			vecNewEnemyOrigin.x -= 6 * cos(m_hEnemy->GetAbsAngles().y * M_PI / 180.0);
			vecNewEnemyOrigin.y -= 6 * sin(m_hEnemy->GetAbsAngles().y * M_PI / 180.0);

			m_flAltitude -= BARNACLE_PULL_SPEED;
			vecNewEnemyOrigin.z += BARNACLE_PULL_SPEED;

			if (fabs(GetAbsOrigin().z - (vecNewEnemyOrigin.z + m_hEnemy->pev->view_ofs.z - 8)) < BARNACLE_BODY_HEIGHT)
			{
				// prey has just been lifted into position ( if the victim origin + eye height + 8 is higher than the bottom of the barnacle, it is assumed that the head is within barnacle's body )
				m_fLiftingPrey = false;

				EmitSound(SoundChannel::Weapon, "barnacle/bcl_bite3.wav");

				m_flKillVictimTime = gpGlobals->time + 10;// now that the victim is in place, the killing bite will be administered in 10 seconds.

				if (CBaseMonster* pVictim = m_hEnemy->MyMonsterPointer(); pVictim)
				{
					pVictim->BarnacleVictimBitten(this);
					SetActivity(ACT_EAT);
				}
			}

			m_hEnemy->SetAbsOrigin(vecNewEnemyOrigin);
		}
		else
		{
			// prey is lifted fully into feeding position and is dangling there.

			CBaseMonster* pVictim = m_hEnemy->MyMonsterPointer();

			if (m_flKillVictimTime != -1 && gpGlobals->time > m_flKillVictimTime)
			{
				// kill!
				if (pVictim)
				{
					pVictim->TakeDamage({this, this, pVictim->pev->health, DMG_SLASH | DMG_ALWAYSGIB});
					m_cGibs = 3;
				}

				return;
			}

			// bite prey every once in a while
			if (pVictim && (RANDOM_LONG(0, 49) == 0))
			{
				switch (RANDOM_LONG(0, 2))
				{
				case 0:	EmitSound(SoundChannel::Weapon, "barnacle/bcl_chew1.wav");	break;
				case 1:	EmitSound(SoundChannel::Weapon, "barnacle/bcl_chew2.wav");	break;
				case 2:	EmitSound(SoundChannel::Weapon, "barnacle/bcl_chew3.wav");	break;
				}

				pVictim->BarnacleVictimBitten(this);
			}
		}
	}
	else
	{
		// barnacle has no prey right now, so just idle and check to see if anything is touching the tongue.

		// If idle and no nearby client, don't think so often
		if (IsNullEnt(UTIL_FindClientInPVS(this)))
			pev->nextthink = gpGlobals->time + RANDOM_FLOAT(1, 1.5);	// Stagger a bit to keep barnacles from thinking on the same frame

		if (m_fSequenceFinished)
		{// this is done so barnacle will fidget.
			SetActivity(ACT_IDLE);
			m_flTongueAdj = -100;
		}

		if (m_cGibs && RANDOM_LONG(0, 99) == 1)
		{
			// cough up a gib.
			CGib::SpawnRandomGibs(this, 1, true);
			m_cGibs--;

			switch (RANDOM_LONG(0, 2))
			{
			case 0:	EmitSound(SoundChannel::Weapon, "barnacle/bcl_chew1.wav");	break;
			case 1:	EmitSound(SoundChannel::Weapon, "barnacle/bcl_chew2.wav");	break;
			case 2:	EmitSound(SoundChannel::Weapon, "barnacle/bcl_chew3.wav");	break;
			}
		}

		float flLength;
		CBaseEntity* pTouchEnt = TongueTouchEnt(&flLength);

		if (pTouchEnt != nullptr && m_fTongueExtended)
		{
			// tongue is fully extended, and is touching someone.
			if (pTouchEnt->BecomeProne())
			{
				EmitSound(SoundChannel::Weapon, "barnacle/bcl_alert2.wav");

				SetSequenceByName("attack1");
				m_flTongueAdj = -20;

				m_hEnemy = pTouchEnt;

				pTouchEnt->SetMovetype(Movetype::Fly);
				pTouchEnt->SetAbsVelocity(vec3_origin);
				pTouchEnt->pev->basevelocity = vec3_origin;
				pTouchEnt->SetAbsOrigin({GetAbsOrigin().x, GetAbsOrigin().y, pTouchEnt->GetAbsOrigin().z});

				m_fLiftingPrey = true;// indicate that we should be lifting prey.
				m_flKillVictimTime = -1;// set this to a bogus time while the victim is lifted.

				m_flAltitude = GetAbsOrigin().z - pTouchEnt->EyePosition().z;
			}
		}
		else
		{
			// calculate a new length for the tongue to be clear of anything else that moves under it. 
			if (m_flAltitude < flLength)
			{
				// if tongue is higher than is should be, lower it kind of slowly.
				m_flAltitude += BARNACLE_PULL_SPEED;
				m_fTongueExtended = false;
			}
			else
			{
				m_flAltitude = flLength;
				m_fTongueExtended = true;
			}
		}
	}

	// ALERT( at_console, "tounge %f\n", m_flAltitude + m_flTongueAdj );
	SetBoneController(0, -(m_flAltitude + m_flTongueAdj));
	StudioFrameAdvance(0.1);
}

void CBarnacle::Killed(const KilledInfo& info)
{
	SetSolidType(Solid::Not);
	SetDamageMode(DamageMode::No);

	if (m_hEnemy != nullptr)
	{
		if (CBaseMonster* pVictim = m_hEnemy->MyMonsterPointer(); pVictim)
		{
			pVictim->BarnacleVictimReleased();
		}
	}

	//	CGib::SpawnRandomGibs( pev, 4, true );

	switch (RANDOM_LONG(0, 1))
	{
	case 0:	EmitSound(SoundChannel::Weapon, "barnacle/bcl_die1.wav");	break;
	case 1:	EmitSound(SoundChannel::Weapon, "barnacle/bcl_die3.wav");	break;
	}

	SetActivity(ACT_DIESIMPLE);
	SetBoneController(0, 0);

	StudioFrameAdvance(0.1);

	pev->nextthink = gpGlobals->time + 0.1;
	SetThink(&CBarnacle::WaitTillDead);
}

void CBarnacle::WaitTillDead()
{
	pev->nextthink = gpGlobals->time + 0.1;

	float flInterval = StudioFrameAdvance(0.1);
	DispatchAnimEvents(flInterval);

	if (m_fSequenceFinished)
	{
		// death anim finished. 
		StopAnimation();
		SetThink(nullptr);
	}
}

void CBarnacle::Precache()
{
	PRECACHE_MODEL("models/barnacle.mdl");

	PRECACHE_SOUND("barnacle/bcl_alert2.wav");//happy, lifting food up
	PRECACHE_SOUND("barnacle/bcl_bite3.wav");//just got food to mouth
	PRECACHE_SOUND("barnacle/bcl_chew1.wav");
	PRECACHE_SOUND("barnacle/bcl_chew2.wav");
	PRECACHE_SOUND("barnacle/bcl_chew3.wav");
	PRECACHE_SOUND("barnacle/bcl_die1.wav");
	PRECACHE_SOUND("barnacle/bcl_die3.wav");
}

constexpr int BARNACLE_CHECK_SPACING = 8;
CBaseEntity* CBarnacle::TongueTouchEnt(float* pflLength)
{
	TraceResult	tr;

	// trace once to hit architecture and see if the tongue needs to change position.
	UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() - Vector(0, 0, 2048), IgnoreMonsters::Yes, this, &tr);
	const float length = fabs(GetAbsOrigin().z - tr.vecEndPos.z);
	if (pflLength)
	{
		*pflLength = length;
	}

	const Vector delta = Vector(BARNACLE_CHECK_SPACING, BARNACLE_CHECK_SPACING, 0);
	Vector mins = GetAbsOrigin() - delta;
	Vector maxs = GetAbsOrigin() + delta;
	maxs.z = GetAbsOrigin().z;
	mins.z -= length;

	CBaseEntity* pList[10];
	const int count = UTIL_EntitiesInBox(pList, ArraySize(pList), mins, maxs, (FL_CLIENT | FL_MONSTER));
	if (count)
	{
		for (int i = 0; i < count; i++)
		{
			// only clients and monsters
			if (pList[i] != this && GetRelationship(pList[i]) > Relationship::None && pList[i]->pev->deadflag == DeadFlag::No)	// this ent is one of our enemies. Barnacle tries to eat it.
			{
				return pList[i];
			}
		}
	}

	return nullptr;
}