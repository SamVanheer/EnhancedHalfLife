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

#include "game.h"

constexpr int SF_INFOBM_RUN = 0x0001;
constexpr int SF_INFOBM_WAIT = 0x0002;

// AI Nodes for Big Momma
class CInfoBM : public CPointEntity
{
public:
	void Spawn() override;
	void KeyValue(KeyValueData* pkvd) override;

	// name in pev->targetname
	// next in pev->target
	// radius in pev->scale
	// health in pev->health
	// Reach target in pev->message
	// Reach delay in pev->speed
	// Reach sequence in pev->netname

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	string_t m_preSequence = iStringNull;
};

LINK_ENTITY_TO_CLASS(info_bigmomma, CInfoBM);

TYPEDESCRIPTION	CInfoBM::m_SaveData[] =
{
	DEFINE_FIELD(CInfoBM, m_preSequence, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CInfoBM, CPointEntity);

void CInfoBM::Spawn()
{
}

void CInfoBM::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "radius"))
	{
		pev->scale = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "reachdelay"))
	{
		pev->speed = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "reachtarget"))
	{
		pev->message = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "reachsequence"))
	{
		pev->netname = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "presequence"))
	{
		m_preSequence = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CPointEntity::KeyValue(pkvd);
}

/**
*	@brief Mortar shot entity
*/
class CBMortar : public CBaseEntity
{
public:
	void Spawn() override;

	static CBMortar* Shoot(CBaseEntity* pOwner, const Vector& vecStart, const Vector& vecVelocity);
	void Touch(CBaseEntity* pOther) override;
	void EXPORT Animate();

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	int m_maxFrame = 0;
};

LINK_ENTITY_TO_CLASS(bmortar, CBMortar);

TYPEDESCRIPTION	CBMortar::m_SaveData[] =
{
	DEFINE_FIELD(CBMortar, m_maxFrame, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CBMortar, CBaseEntity);

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
constexpr int BIG_AE_STEP1 = 1;			//!< Footstep left
constexpr int BIG_AE_STEP2 = 2;			//!< Footstep right
constexpr int BIG_AE_STEP3 = 3;			//!< Footstep back left
constexpr int BIG_AE_STEP4 = 4;			//!< Footstep back right
constexpr int BIG_AE_SACK = 5;			//!< Sack slosh
constexpr int BIG_AE_DEATHSOUND = 6;	//!< Death sound

constexpr int BIG_AE_MELEE_ATTACKBR = 8;	//!< Leg attack
constexpr int BIG_AE_MELEE_ATTACKBL = 9;	//!< Leg attack
constexpr int BIG_AE_MELEE_ATTACK1 = 10;	//!< Leg attack
constexpr int BIG_AE_MORTAR_ATTACK1 = 11;	//!< Launch a mortar
constexpr int BIG_AE_LAY_CRAB = 12;			//!< Lay a headcrab
constexpr int BIG_AE_JUMP_FORWARD = 13;		//!< Jump up and forward
constexpr int BIG_AE_SCREAM = 14;			//!< alert sound
constexpr int BIG_AE_PAIN_SOUND = 15;		//!< pain sound
constexpr int BIG_AE_ATTACK_SOUND = 16;		//!< attack sound
constexpr int BIG_AE_BIRTH_SOUND = 17;		//!< birth sound
constexpr int BIG_AE_EARLY_TARGET = 50;		//!< Fire target early

// User defined conditions
constexpr int bits_COND_NODE_SEQUENCE = bits_COND_SPECIAL1; 		// pev->netname contains the name of a sequence to play

// Attack distance constants
constexpr int BIG_ATTACKDIST = 170;
constexpr int BIG_MORTARDIST = 800;
constexpr int BIG_MAXCHILDREN = 20;			// Max # of live headcrab children


constexpr int bits_MEMORY_CHILDPAIR = bits_MEMORY_CUSTOM1;
constexpr int bits_MEMORY_ADVANCE_NODE = bits_MEMORY_CUSTOM2;
constexpr int bits_MEMORY_COMPLETED_NODE = bits_MEMORY_CUSTOM3;
constexpr int bits_MEMORY_FIRED_NODE = bits_MEMORY_CUSTOM4;

int gSpitSprite, gSpitDebrisSprite;
Vector CheckSplatToss(CBaseEntity* pEntity, const Vector& vecSpot1, const Vector& vecSpot2, float maxHeight);
void MortarSpray(const Vector& position, const Vector& direction, int spriteModel, int count);

// UNDONE:	
//
constexpr std::string_view BIG_CHILDCLASS{"monster_babycrab"};

class CBigMomma : public CBaseMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void KeyValue(KeyValueData* pkvd) override;
	void Activate() override;
	bool TakeDamage(const TakeDamageInfo& info) override;

	void		RunTask(Task_t* pTask) override;
	void		StartTask(Task_t* pTask) override;
	Schedule_t* GetSchedule() override;
	Schedule_t* GetScheduleOfType(int Type) override;
	void		TraceAttack(const TraceAttackInfo& info) override;

	void NodeStart(string_t iszNextNode);
	void NodeReach();
	bool ShouldGoToNode();

	void SetYawSpeed() override;
	int  Classify() override;
	void HandleAnimEvent(AnimationEvent& event) override;
	void LayHeadcrab();

	string_t GetNodeSequence()
	{
		CBaseEntity* pTarget = m_hTargetEnt;
		if (pTarget)
		{
			return pTarget->pev->netname;	// netname holds node sequence
		}
		return iStringNull;
	}

	string_t GetNodePresequence()
	{
		CInfoBM* pTarget = (CInfoBM*)m_hTargetEnt.Get();
		if (pTarget)
		{
			return pTarget->m_preSequence;
		}
		return iStringNull;
	}

	float GetNodeDelay()
	{
		CBaseEntity* pTarget = m_hTargetEnt;
		if (pTarget)
		{
			return pTarget->pev->speed;	// Speed holds node delay
		}
		return 0;
	}

	float GetNodeRange()
	{
		CBaseEntity* pTarget = m_hTargetEnt;
		if (pTarget)
		{
			return pTarget->pev->scale;	// Scale holds node delay
		}
		return 1e6;
	}

	float GetNodeYaw()
	{
		CBaseEntity* pTarget = m_hTargetEnt;
		if (pTarget)
		{
			if (pTarget->GetAbsAngles().y != 0)
				return pTarget->GetAbsAngles().y;
		}
		return GetAbsAngles().y;
	}

	// Restart the crab count on each new level
	void OverrideReset() override
	{
		m_crabCount = 0;
	}

	void DeathNotice(CBaseEntity* pChild) override;

	bool CanLayCrab()
	{
		if (m_crabTime < gpGlobals->time && m_crabCount < BIG_MAXCHILDREN)
		{
			// Don't spawn crabs inside each other
			Vector mins = GetAbsOrigin() - Vector(32, 32, 0);
			Vector maxs = GetAbsOrigin() + Vector(32, 32, 0);

			CBaseEntity* pList[2];
			int count = UTIL_EntitiesInBox(pList, ArraySize(pList), mins, maxs, FL_MONSTER);
			for (int i = 0; i < count; i++)
			{
				if (pList[i] != this)	// Don't hurt yourself!
					return false;
			}
			return true;
		}

		return false;
	}

	void LaunchMortar();

	void SetObjectCollisionBox() override
	{
		pev->absmin = GetAbsOrigin() + Vector(-95, -95, 0);
		pev->absmax = GetAbsOrigin() + Vector(95, 95, 190);
	}

	bool CheckMeleeAttack1(float flDot, float flDist) override;	//!< Slash
	bool CheckMeleeAttack2(float flDot, float flDist) override;	//!< Lay a crab
	bool CheckRangeAttack1(float flDot, float flDist) override;	//!< Mortar launch

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	static const char* pChildDieSounds[];
	static const char* pSackSounds[];
	static const char* pDeathSounds[];
	static const char* pAttackSounds[];
	static const char* pAttackHitSounds[];
	static const char* pBirthSounds[];
	static const char* pAlertSounds[];
	static const char* pPainSounds[];
	static const char* pFootSounds[];

	CUSTOM_SCHEDULES;

private:
	float m_nodeTime = 0;
	float m_crabTime = 0;
	float m_mortarTime = 0;
	float m_painSoundTime = 0;
	int m_crabCount = 0;
};

LINK_ENTITY_TO_CLASS(monster_bigmomma, CBigMomma);

TYPEDESCRIPTION	CBigMomma::m_SaveData[] =
{
	DEFINE_FIELD(CBigMomma, m_nodeTime, FIELD_TIME),
	DEFINE_FIELD(CBigMomma, m_crabTime, FIELD_TIME),
	DEFINE_FIELD(CBigMomma, m_mortarTime, FIELD_TIME),
	DEFINE_FIELD(CBigMomma, m_painSoundTime, FIELD_TIME),
	DEFINE_FIELD(CBigMomma, m_crabCount, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CBigMomma, CBaseMonster);

const char* CBigMomma::pChildDieSounds[] =
{
	"gonarch/gon_childdie1.wav",
	"gonarch/gon_childdie2.wav",
	"gonarch/gon_childdie3.wav",
};

const char* CBigMomma::pSackSounds[] =
{
	"gonarch/gon_sack1.wav",
	"gonarch/gon_sack2.wav",
	"gonarch/gon_sack3.wav",
};

const char* CBigMomma::pDeathSounds[] =
{
	"gonarch/gon_die1.wav",
};

const char* CBigMomma::pAttackSounds[] =
{
	"gonarch/gon_attack1.wav",
	"gonarch/gon_attack2.wav",
	"gonarch/gon_attack3.wav",
};
const char* CBigMomma::pAttackHitSounds[] =
{
	"zombie/claw_strike1.wav",
	"zombie/claw_strike2.wav",
	"zombie/claw_strike3.wav",
};

const char* CBigMomma::pBirthSounds[] =
{
	"gonarch/gon_birth1.wav",
	"gonarch/gon_birth2.wav",
	"gonarch/gon_birth3.wav",
};

const char* CBigMomma::pAlertSounds[] =
{
	"gonarch/gon_alert1.wav",
	"gonarch/gon_alert2.wav",
	"gonarch/gon_alert3.wav",
};

const char* CBigMomma::pPainSounds[] =
{
	"gonarch/gon_pain2.wav",
	"gonarch/gon_pain4.wav",
	"gonarch/gon_pain5.wav",
};

const char* CBigMomma::pFootSounds[] =
{
	"gonarch/gon_step1.wav",
	"gonarch/gon_step2.wav",
	"gonarch/gon_step3.wav",
};

void CBigMomma::KeyValue(KeyValueData* pkvd)
{
#if 0
	if (AreStringsEqual(pkvd->szKeyName, "volume"))
	{
		m_volume = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
#endif
		CBaseMonster::KeyValue(pkvd);
}

int	CBigMomma::Classify()
{
	return	CLASS_ALIEN_MONSTER;
}

void CBigMomma::SetYawSpeed()
{
	int ys;

	switch (m_Activity)
	{
	case ACT_IDLE:
		ys = 100;
		break;
	default:
		ys = 90;
	}
	pev->yaw_speed = ys;
}

void CBigMomma::HandleAnimEvent(AnimationEvent& event)
{
	switch (event.event)
	{
	case BIG_AE_MELEE_ATTACKBR:
	case BIG_AE_MELEE_ATTACKBL:
	case BIG_AE_MELEE_ATTACK1:
	{
		Vector forward, right;
		AngleVectors(GetAbsAngles(), &forward, &right, nullptr);

		const Vector center = GetAbsOrigin() + forward * 128;
		const Vector mins = center - Vector(64, 64, 0);
		const Vector maxs = center + Vector(64, 64, 64);

		CBaseEntity* pList[8];
		int count = UTIL_EntitiesInBox(pList, ArraySize(pList), mins, maxs, FL_MONSTER | FL_CLIENT);
		CBaseEntity* pHurt = nullptr;

		for (int i = 0; i < count && !pHurt; i++)
		{
			if (pList[i] != this)
			{
				if (pList[i]->GetOwner() != this)
					pHurt = pList[i];
			}
		}

		if (pHurt)
		{
			pHurt->TakeDamage({this, this, gSkillData.bigmommaDmgSlash, DMG_CRUSH | DMG_SLASH});
			pHurt->pev->punchangle.x = 15;
			switch (event.event)
			{
			case BIG_AE_MELEE_ATTACKBR:
				pHurt->SetAbsVelocity(pHurt->GetAbsVelocity() + (forward * 150) + Vector(0, 0, 250) - (right * 200));
				break;

			case BIG_AE_MELEE_ATTACKBL:
				pHurt->SetAbsVelocity(pHurt->GetAbsVelocity() + (forward * 150) + Vector(0, 0, 250) + (right * 200));
				break;

			case BIG_AE_MELEE_ATTACK1:
				pHurt->SetAbsVelocity(pHurt->GetAbsVelocity() + (forward * 220) + Vector(0, 0, 200));
				break;
			}

			pHurt->pev->flags &= ~FL_ONGROUND;
			EmitSound(SoundChannel::Weapon, RANDOM_SOUND_ARRAY(pAttackHitSounds), VOL_NORM, ATTN_NORM, PITCH_NORM + RANDOM_LONG(-5, 5));
		}
	}
	break;

	case BIG_AE_SCREAM:
		EMIT_SOUND_ARRAY_DYN(SoundChannel::Voice, pAlertSounds);
		break;

	case BIG_AE_PAIN_SOUND:
		EMIT_SOUND_ARRAY_DYN(SoundChannel::Voice, pPainSounds);
		break;

	case BIG_AE_ATTACK_SOUND:
		EMIT_SOUND_ARRAY_DYN(SoundChannel::Weapon, pAttackSounds);
		break;

	case BIG_AE_BIRTH_SOUND:
		EMIT_SOUND_ARRAY_DYN(SoundChannel::Body, pBirthSounds);
		break;

	case BIG_AE_SACK:
		if (RANDOM_LONG(0, 100) < 30)
			EMIT_SOUND_ARRAY_DYN(SoundChannel::Body, pSackSounds);
		break;

	case BIG_AE_DEATHSOUND:
		EMIT_SOUND_ARRAY_DYN(SoundChannel::Voice, pDeathSounds);
		break;

	case BIG_AE_STEP1:		// Footstep left
	case BIG_AE_STEP3:		// Footstep back left
		EMIT_SOUND_ARRAY_DYN(SoundChannel::Item, pFootSounds);
		break;

	case BIG_AE_STEP4:		// Footstep back right
	case BIG_AE_STEP2:		// Footstep right
		EMIT_SOUND_ARRAY_DYN(SoundChannel::Body, pFootSounds);
		break;

	case BIG_AE_MORTAR_ATTACK1:
		LaunchMortar();
		break;

	case BIG_AE_LAY_CRAB:
		LayHeadcrab();
		break;

	case BIG_AE_JUMP_FORWARD:
		ClearBits(pev->flags, FL_ONGROUND);

		SetAbsOrigin(GetAbsOrigin() + Vector(0, 0, 1));// take him off ground so engine doesn't instantly reset onground 
		UTIL_MakeVectors(GetAbsAngles());

		SetAbsVelocity((gpGlobals->v_forward * 200) + gpGlobals->v_up * 500);
		break;

	case BIG_AE_EARLY_TARGET:
	{
		if (CBaseEntity* pTarget = m_hTargetEnt; pTarget && !IsStringNull(pTarget->pev->message))
			FireTargets(STRING(pTarget->pev->message), this, this, UseType::Toggle, 0);
		Remember(bits_MEMORY_FIRED_NODE);
	}
	break;

	default:
		CBaseMonster::HandleAnimEvent(event);
		break;
	}
}

void CBigMomma::TraceAttack(const TraceAttackInfo& info)
{
	TraceAttackInfo adjustedInfo = info;

	if (adjustedInfo.GetTraceResult().iHitgroup != 1)
	{
		// didn't hit the sack?

		if (pev->dmgtime != gpGlobals->time || (RANDOM_LONG(0, 10) < 1))
		{
			UTIL_Ricochet(adjustedInfo.GetTraceResult().vecEndPos, RANDOM_FLOAT(1, 2));
			pev->dmgtime = gpGlobals->time;
		}

		adjustedInfo.SetDamage(0.1f);// don't hurt the monster much, but allow bits_COND_LIGHT_DAMAGE to be generated
	}
	else if (gpGlobals->time > m_painSoundTime)
	{
		m_painSoundTime = gpGlobals->time + RANDOM_LONG(1, 3);
		EMIT_SOUND_ARRAY_DYN(SoundChannel::Voice, pPainSounds);
	}

	CBaseMonster::TraceAttack(adjustedInfo);
}

bool CBigMomma::TakeDamage(const TakeDamageInfo& info)
{
	TakeDamageInfo adjustedInfo = info;

	// Don't take any acid damage -- BigMomma's mortar is acid
	if (adjustedInfo.GetDamageTypes() & DMG_ACID)
		adjustedInfo.SetDamage(0);

	if (!HasMemory(bits_MEMORY_PATH_FINISHED))
	{
		if (pev->health <= adjustedInfo.GetDamage())
		{
			pev->health = adjustedInfo.GetDamage() + 1;
			Remember(bits_MEMORY_ADVANCE_NODE | bits_MEMORY_COMPLETED_NODE);
			ALERT(at_aiconsole, "BM: Finished node health!!!\n");
		}
	}

	return CBaseMonster::TakeDamage(adjustedInfo);
}

void CBigMomma::LayHeadcrab()
{
	CBaseEntity* pChild = CBaseEntity::Create(BIG_CHILDCLASS.data(), GetAbsOrigin(), GetAbsAngles(), this);

	pChild->pev->spawnflags |= SF_MONSTER_FALL_TO_GROUND;

	// Is this the second crab in a pair?
	if (HasMemory(bits_MEMORY_CHILDPAIR))
	{
		m_crabTime = gpGlobals->time + RANDOM_FLOAT(5, 10);
		Forget(bits_MEMORY_CHILDPAIR);
	}
	else
	{
		m_crabTime = gpGlobals->time + RANDOM_FLOAT(0.5, 2.5);
		Remember(bits_MEMORY_CHILDPAIR);
	}

	TraceResult tr;
	UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() - Vector(0, 0, 100), IgnoreMonsters::Yes, this, &tr);
	UTIL_DecalTrace(&tr, DECAL_MOMMABIRTH);

	EmitSound(SoundChannel::Weapon, RANDOM_SOUND_ARRAY(pBirthSounds), VOL_NORM, ATTN_NORM, PITCH_NORM + RANDOM_LONG(-5, 5));
	m_crabCount++;
}

void CBigMomma::DeathNotice(CBaseEntity* pChild)
{
	if (m_crabCount > 0)		// Some babies may cross a transition, but we reset the count then
		m_crabCount--;
	if (IsAlive())
	{
		// Make the "my baby's dead" noise!
		EMIT_SOUND_ARRAY_DYN(SoundChannel::Weapon, pChildDieSounds);
	}
}

void CBigMomma::LaunchMortar()
{
	m_mortarTime = gpGlobals->time + RANDOM_FLOAT(2, 15);

	Vector startPos = GetAbsOrigin();
	startPos.z += 180;

	EmitSound(SoundChannel::Weapon, RANDOM_SOUND_ARRAY(pSackSounds), VOL_NORM, ATTN_NORM, PITCH_NORM + RANDOM_LONG(-5, 5));
	CBMortar* pBomb = CBMortar::Shoot(this, startPos, pev->movedir);
	pBomb->pev->gravity = 1.0;
	MortarSpray(startPos, vec3_up, gSpitSprite, 24);
}

void CBigMomma::Spawn()
{
	Precache();

	SetModel("models/big_mom.mdl");
	SetSize(Vector(-32, -32, 0), Vector(32, 32, 64));

	SetSolidType(Solid::SlideBox);
	SetMovetype(Movetype::Step);
	m_bloodColor = BLOOD_COLOR_GREEN;
	pev->health = 150 * gSkillData.bigmommaHealthFactor;
	pev->view_ofs = Vector(0, 0, 128);// position of the eyes relative to monster's origin.
	m_flFieldOfView = 0.3;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState = NPCState::None;

	MonsterInit();
}

void CBigMomma::Precache()
{
	PRECACHE_MODEL("models/big_mom.mdl");

	PRECACHE_SOUND_ARRAY(pChildDieSounds);
	PRECACHE_SOUND_ARRAY(pSackSounds);
	PRECACHE_SOUND_ARRAY(pDeathSounds);
	PRECACHE_SOUND_ARRAY(pAttackSounds);
	PRECACHE_SOUND_ARRAY(pAttackHitSounds);
	PRECACHE_SOUND_ARRAY(pBirthSounds);
	PRECACHE_SOUND_ARRAY(pAlertSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pFootSounds);

	UTIL_PrecacheOther(BIG_CHILDCLASS.data());

	// TEMP: Squid
	PRECACHE_MODEL("sprites/mommaspit.spr");// spit projectile.
	gSpitSprite = PRECACHE_MODEL("sprites/mommaspout.spr");// client side spittle.
	gSpitDebrisSprite = PRECACHE_MODEL("sprites/mommablob.spr");

	PRECACHE_SOUND("bullchicken/bc_acid1.wav");
	PRECACHE_SOUND("bullchicken/bc_spithit1.wav");
	PRECACHE_SOUND("bullchicken/bc_spithit2.wav");
}

void CBigMomma::Activate()
{
	if (m_hTargetEnt == nullptr)
		Remember(bits_MEMORY_ADVANCE_NODE);	// Start 'er up
}

void CBigMomma::NodeStart(string_t iszNextNode)
{
	pev->netname = iszNextNode;

	CBaseEntity* pTarget = nullptr;

	if (!IsStringNull(pev->netname))
	{
		pTarget = UTIL_FindEntityByTargetname(nullptr, STRING(pev->netname));
	}

	if (!pTarget)
	{
		ALERT(at_aiconsole, "BM: Finished the path!!\n");
		Remember(bits_MEMORY_PATH_FINISHED);
		return;
	}
	Remember(bits_MEMORY_ON_PATH);
	m_hTargetEnt = pTarget;
}

void CBigMomma::NodeReach()
{
	CBaseEntity* pTarget = m_hTargetEnt;

	Forget(bits_MEMORY_ADVANCE_NODE);

	if (!pTarget)
		return;

	if (pTarget->pev->health)
		pev->max_health = pev->health = pTarget->pev->health * gSkillData.bigmommaHealthFactor;

	if (!HasMemory(bits_MEMORY_FIRED_NODE))
	{
		if (!IsStringNull(pTarget->pev->message))
			FireTargets(STRING(pTarget->pev->message), this, this, UseType::Toggle, 0);
	}
	Forget(bits_MEMORY_FIRED_NODE);

	pev->netname = pTarget->pev->target;
	if (pTarget->pev->health == 0)
		Remember(bits_MEMORY_ADVANCE_NODE);	// Move on if no health at this node
}

bool CBigMomma::CheckMeleeAttack1(float flDot, float flDist)
{
	if (flDot >= 0.7)
	{
		if (flDist <= BIG_ATTACKDIST)
			return true;
	}
	return false;
}

bool CBigMomma::CheckMeleeAttack2(float flDot, float flDist)
{
	return CanLayCrab();
}

bool CBigMomma::CheckRangeAttack1(float flDot, float flDist)
{
	if (flDist <= BIG_MORTARDIST && m_mortarTime < gpGlobals->time)
	{
		if (CBaseEntity* pEnemy = m_hEnemy; pEnemy)
		{
			Vector startPos = GetAbsOrigin();
			startPos.z += 180;
			pev->movedir = CheckSplatToss(this, startPos, pEnemy->BodyTarget(GetAbsOrigin()), RANDOM_FLOAT(150, 500));
			if (pev->movedir != vec3_origin)
				return true;
		}
	}
	return false;
}

//=========================================================
// AI Schedules Specific to this monster
//=========================================================

enum
{
	SCHED_BIG_NODE = LAST_COMMON_SCHEDULE + 1,
	SCHED_NODE_FAIL,
};

enum
{
	TASK_MOVE_TO_NODE_RANGE = LAST_COMMON_TASK + 1,	// Move within node range
	TASK_FIND_NODE,									// Find my next node
	TASK_PLAY_NODE_PRESEQUENCE,						// Play node pre-script
	TASK_PLAY_NODE_SEQUENCE,						// Play node script
	TASK_PROCESS_NODE,								// Fire targets, etc.
	TASK_WAIT_NODE,									// Wait at the node
	TASK_NODE_DELAY,								// Delay walking toward node for a bit. You've failed to get there
	TASK_NODE_YAW,									// Get the best facing direction for this node
};

Task_t	tlBigNode[] =
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_NODE_FAIL },
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_FIND_NODE,			(float)0		},	// Find my next node
	{ TASK_PLAY_NODE_PRESEQUENCE,(float)0		},	// Play the pre-approach sequence if any
	{ TASK_MOVE_TO_NODE_RANGE,	(float)0		},	// Move within node range
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_NODE_YAW,			(float)0		},
	{ TASK_FACE_IDEAL,			(float)0		},
	{ TASK_WAIT_NODE,			(float)0		},	// Wait for node delay
	{ TASK_PLAY_NODE_SEQUENCE,	(float)0		},	// Play the sequence if one exists
	{ TASK_PROCESS_NODE,		(float)0		},	// Fire targets, etc.
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE	},
};

Schedule_t	slBigNode[] =
{
	{
		tlBigNode,
		ArraySize(tlBigNode),
		0,
		0,
		"Big Node"
	},
};

Task_t	tlNodeFail[] =
{
	{ TASK_NODE_DELAY,			(float)10		},	// Try to do something else for 10 seconds
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE	},
};

Schedule_t	slNodeFail[] =
{
	{
		tlNodeFail,
		ArraySize(tlNodeFail),
		0,
		0,
		"NodeFail"
	},
};

DEFINE_CUSTOM_SCHEDULES(CBigMomma)
{
	slBigNode,
		slNodeFail,
};

IMPLEMENT_CUSTOM_SCHEDULES(CBigMomma, CBaseMonster);

Schedule_t* CBigMomma::GetScheduleOfType(int Type)
{
	switch (Type)
	{
	case SCHED_BIG_NODE:
		return slBigNode;
		break;

	case SCHED_NODE_FAIL:
		return slNodeFail;
		break;
	}

	return CBaseMonster::GetScheduleOfType(Type);
}

bool CBigMomma::ShouldGoToNode()
{
	if (HasMemory(bits_MEMORY_ADVANCE_NODE))
	{
		if (m_nodeTime < gpGlobals->time)
			return true;
	}
	return false;
}

Schedule_t* CBigMomma::GetSchedule()
{
	if (ShouldGoToNode())
	{
		return GetScheduleOfType(SCHED_BIG_NODE);
	}

	return CBaseMonster::GetSchedule();
}

void CBigMomma::StartTask(Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_FIND_NODE:
	{
		if (!HasMemory(bits_MEMORY_ADVANCE_NODE))
		{
			if (CBaseEntity* pTarget = m_hTargetEnt; pTarget)
				pev->netname = m_hTargetEnt->pev->target;
		}
		NodeStart(pev->netname);
		TaskComplete();
		ALERT(at_aiconsole, "BM: Found node %s\n", STRING(pev->netname));
	}
	break;

	case TASK_NODE_DELAY:
		m_nodeTime = gpGlobals->time + pTask->flData;
		TaskComplete();
		ALERT(at_aiconsole, "BM: FAIL! Delay %.2f\n", pTask->flData);
		break;

	case TASK_PROCESS_NODE:
		ALERT(at_aiconsole, "BM: Reached node %s\n", STRING(pev->netname));
		NodeReach();
		TaskComplete();
		break;

	case TASK_PLAY_NODE_PRESEQUENCE:
	case TASK_PLAY_NODE_SEQUENCE:
	{
		string_t sequence;
		if (pTask->iTask == TASK_PLAY_NODE_SEQUENCE)
			sequence = GetNodeSequence();
		else
			sequence = GetNodePresequence();

		ALERT(at_aiconsole, "BM: Playing node sequence %s\n", STRING(sequence));
		if (!IsStringNull(sequence))
		{
			const int sequenceIndex = LookupSequence(STRING(sequence));
			if (sequenceIndex != -1)
			{
				pev->sequence = sequenceIndex;
				pev->frame = 0;
				ResetSequenceInfo();
				ALERT(at_aiconsole, "BM: Sequence %s\n", STRING(GetNodeSequence()));
				return;
			}
		}
		TaskComplete();
	}
	break;

	case TASK_NODE_YAW:
		pev->ideal_yaw = GetNodeYaw();
		TaskComplete();
		break;

	case TASK_WAIT_NODE:
		m_flWait = gpGlobals->time + GetNodeDelay();
		if (m_hTargetEnt->pev->spawnflags & SF_INFOBM_WAIT)
			ALERT(at_aiconsole, "BM: Wait at node %s forever\n", STRING(pev->netname));
		else
			ALERT(at_aiconsole, "BM: Wait at node %s for %.2f\n", STRING(pev->netname), GetNodeDelay());
		break;


	case TASK_MOVE_TO_NODE_RANGE:
	{
		CBaseEntity* pTarget = m_hTargetEnt;
		if (!pTarget)
			TaskFail();
		else
		{
			if ((pTarget->GetAbsOrigin() - GetAbsOrigin()).Length() < GetNodeRange())
				TaskComplete();
			else
			{
				Activity act = ACT_WALK;
				if (pTarget->pev->spawnflags & SF_INFOBM_RUN)
					act = ACT_RUN;

				m_vecMoveGoal = pTarget->GetAbsOrigin();
				if (!MoveToTarget(act, 2))
				{
					TaskFail();
				}
			}
		}
	}
	ALERT(at_aiconsole, "BM: Moving to node %s\n", STRING(pev->netname));

	break;

	case TASK_MELEE_ATTACK1:
		// Play an attack sound here
		EmitSound(SoundChannel::Voice, RANDOM_SOUND_ARRAY(pAttackSounds));
		CBaseMonster::StartTask(pTask);
		break;

	default:
		CBaseMonster::StartTask(pTask);
		break;
	}
}

void CBigMomma::RunTask(Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_MOVE_TO_NODE_RANGE:
	{
		if (m_hTargetEnt == nullptr)
			TaskFail();
		else
		{
			const float distance = (m_vecMoveGoal - GetAbsOrigin()).Length2D();
			// Set the appropriate activity based on an overlapping range
			// overlap the range to prevent oscillation
			if ((distance < GetNodeRange()) || MovementIsComplete())
			{
				ALERT(at_aiconsole, "BM: Reached node!\n");
				TaskComplete();
				RouteClear();		// Stop moving
			}
		}
	}

	break;

	case TASK_WAIT_NODE:
		if (m_hTargetEnt != nullptr && (m_hTargetEnt->pev->spawnflags & SF_INFOBM_WAIT))
			return;

		if (gpGlobals->time > m_flWaitFinished)
			TaskComplete();
		ALERT(at_aiconsole, "BM: The WAIT is over!\n");
		break;

	case TASK_PLAY_NODE_PRESEQUENCE:
	case TASK_PLAY_NODE_SEQUENCE:
		if (m_fSequenceFinished)
		{
			m_Activity = ACT_RESET;
			TaskComplete();
		}
		break;

	default:
		CBaseMonster::RunTask(pTask);
		break;
	}
}

Vector CheckSplatToss(CBaseEntity* pEntity, const Vector& vecSpot1, const Vector& vecSpot2, float maxHeight)
{
	TraceResult		tr;
	const float flGravity = g_psv_gravity->value;

	// calculate the midpoint and apex of the 'triangle'
	const Vector vecMidPoint = vecSpot1 + (vecSpot2 - vecSpot1) * 0.5; // halfway point between Spot1 and Spot2
	UTIL_TraceLine(vecMidPoint, vecMidPoint + Vector(0, 0, maxHeight), IgnoreMonsters::Yes, pEntity, &tr);
	const Vector vecApex = tr.vecEndPos; // highest point 

	UTIL_TraceLine(vecSpot1, vecApex, IgnoreMonsters::No, pEntity, &tr);
	if (tr.flFraction != 1.0)
	{
		// fail!
		return vec3_origin;
	}

	// Don't worry about actually hitting the target, this won't hurt us!

	// How high should the grenade travel (subtract 15 so the grenade doesn't hit the ceiling)?
	const float height = (vecApex.z - vecSpot1.z) - 15;
	// How fast does the grenade need to travel to reach that height given gravity?
	const float speed = sqrt(2 * flGravity * height);

	// How much time does it take to get there?
	const float time = speed / flGravity;
	Vector vecGrenadeVel = vecSpot2 - vecSpot1;
	vecGrenadeVel.z = 0;
	const float distance = vecGrenadeVel.Length();

	// Travel half the distance to the target in that time (apex is at the midpoint)
	vecGrenadeVel = vecGrenadeVel * (0.5 / time);
	// Speed to offset gravity at the desired height
	vecGrenadeVel.z = speed;

	return vecGrenadeVel;
}

void MortarSpray(const Vector& position, const Vector& direction, int spriteModel, int count)
{
	MESSAGE_BEGIN(MessageDest::PVS, SVC_TEMPENTITY, position);
	WRITE_BYTE(TE_SPRITE_SPRAY);
	WRITE_COORD(position.x);	// pos
	WRITE_COORD(position.y);
	WRITE_COORD(position.z);
	WRITE_COORD(direction.x);	// dir
	WRITE_COORD(direction.y);
	WRITE_COORD(direction.z);
	WRITE_SHORT(spriteModel);	// model
	WRITE_BYTE(count);			// count
	WRITE_BYTE(130);			// speed
	WRITE_BYTE(80);			// noise ( client will divide by 100 )
	MESSAGE_END();
}

// UNDONE: right now this is pretty much a copy of the squid spit with minor changes to the way it does damage
void CBMortar::Spawn()
{
	SetMovetype(Movetype::Toss);
	SetClassname("bmortar");

	SetSolidType(Solid::BBox);
	SetRenderMode(RenderMode::TransAlpha);
	SetRenderAmount(255);

	SetModel("sprites/mommaspit.spr");
	pev->frame = 0;
	pev->scale = 0.5;

	SetSize(vec3_origin, vec3_origin);

	m_maxFrame = (float)MODEL_FRAMES(pev->modelindex) - 1;
	pev->dmgtime = gpGlobals->time + 0.4;
}

void CBMortar::Animate()
{
	pev->nextthink = gpGlobals->time + 0.1;

	if (gpGlobals->time > pev->dmgtime)
	{
		pev->dmgtime = gpGlobals->time + 0.2;
		MortarSpray(GetAbsOrigin(), -GetAbsVelocity().Normalize(), gSpitSprite, 3);
	}
	if (pev->frame++)
	{
		if (pev->frame > m_maxFrame)
		{
			pev->frame = 0;
		}
	}
}

CBMortar* CBMortar::Shoot(CBaseEntity* pOwner, const Vector& vecStart, const Vector& vecVelocity)
{
	CBMortar* pSpit = GetClassPtr((CBMortar*)nullptr);
	pSpit->Spawn();

	pSpit->SetAbsOrigin(vecStart);
	pSpit->SetAbsVelocity(vecVelocity);
	pSpit->SetOwner(pOwner);
	pSpit->pev->scale = 2.5;
	pSpit->SetThink(&CBMortar::Animate);
	pSpit->pev->nextthink = gpGlobals->time + 0.1;

	return pSpit;
}

void CBMortar::Touch(CBaseEntity* pOther)
{
	// splat sound
	const int iPitch = RANDOM_FLOAT(90, 110);

	EmitSound(SoundChannel::Voice, "bullchicken/bc_acid1.wav", VOL_NORM, ATTN_NORM, iPitch);

	switch (RANDOM_LONG(0, 1))
	{
	case 0:
		EmitSound(SoundChannel::Weapon, "bullchicken/bc_spithit1.wav", VOL_NORM, ATTN_NORM, iPitch);
		break;
	case 1:
		EmitSound(SoundChannel::Weapon, "bullchicken/bc_spithit2.wav", VOL_NORM, ATTN_NORM, iPitch);
		break;
	}

	TraceResult tr;
	if (pOther->IsBSPModel())
	{
		// make a splat on the wall
		UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + GetAbsVelocity() * 10, IgnoreMonsters::No, this, &tr);
		UTIL_DecalTrace(&tr, DECAL_MOMMASPLAT);
	}
	else
	{
		tr.vecEndPos = GetAbsOrigin();
		tr.vecPlaneNormal = -1 * GetAbsVelocity().Normalize();
	}
	// make some flecks
	MortarSpray(tr.vecEndPos, tr.vecPlaneNormal, gSpitSprite, 24);

	RadiusDamage(GetAbsOrigin(), this, GetOwner(), gSkillData.bigmommaDmgBlast, gSkillData.bigmommaRadiusBlast, CLASS_NONE, DMG_ACID);
	UTIL_Remove(this);
}