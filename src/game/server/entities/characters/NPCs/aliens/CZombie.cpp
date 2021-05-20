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

// UNDONE: Don't flinch every time you get hit

#include "CZombie.hpp"

LINK_ENTITY_TO_CLASS(monster_zombie, CZombie);

const char* CZombie::pAttackHitSounds[] =
{
	"zombie/claw_strike1.wav",
	"zombie/claw_strike2.wav",
	"zombie/claw_strike3.wav",
};

const char* CZombie::pAttackMissSounds[] =
{
	"zombie/claw_miss1.wav",
	"zombie/claw_miss2.wav",
};

const char* CZombie::pAttackSounds[] =
{
	"zombie/zo_attack1.wav",
	"zombie/zo_attack2.wav",
};

const char* CZombie::pIdleSounds[] =
{
	"zombie/zo_idle1.wav",
	"zombie/zo_idle2.wav",
	"zombie/zo_idle3.wav",
	"zombie/zo_idle4.wav",
};

const char* CZombie::pAlertSounds[] =
{
	"zombie/zo_alert10.wav",
	"zombie/zo_alert20.wav",
	"zombie/zo_alert30.wav",
};

const char* CZombie::pPainSounds[] =
{
	"zombie/zo_pain1.wav",
	"zombie/zo_pain2.wav",
};

int	CZombie::Classify()
{
	return	CLASS_ALIEN_MONSTER;
}

void CZombie::SetYawSpeed()
{
	pev->yaw_speed = 120;
}

bool CZombie::TakeDamage(const TakeDamageInfo& info)
{
	TakeDamageInfo adjustedInfo = info;

	// Take 30% damage from bullets
	if (adjustedInfo.GetDamageTypes() == DMG_BULLET)
	{
		const Vector vecDir = (GetAbsOrigin() - (adjustedInfo.GetInflictor()->pev->absmin + adjustedInfo.GetInflictor()->pev->absmax) * 0.5).Normalize();
		const float flForce = DamageForce(adjustedInfo.GetDamage());
		SetAbsVelocity(GetAbsVelocity() + vecDir * flForce);
		adjustedInfo.SetDamage(adjustedInfo.GetDamage() * 0.3);
	}

	// HACK HACK -- until we fix this.
	if (IsAlive())
		PainSound();
	return CBaseMonster::TakeDamage(adjustedInfo);
}

void CZombie::PainSound()
{
	const int pitch = 95 + RANDOM_LONG(0, 9);

	if (RANDOM_LONG(0, 5) < 2)
		EmitSound(SoundChannel::Voice, pPainSounds[RANDOM_LONG(0, ArraySize(pPainSounds) - 1)], VOL_NORM, ATTN_NORM, pitch);
}

void CZombie::AlertSound()
{
	const int pitch = 95 + RANDOM_LONG(0, 9);

	EmitSound(SoundChannel::Voice, pAlertSounds[RANDOM_LONG(0, ArraySize(pAlertSounds) - 1)], VOL_NORM, ATTN_NORM, pitch);
}

void CZombie::IdleSound()
{
	const int pitch = 100 + RANDOM_LONG(-5, 5);

	// Play a random idle sound
	EmitSound(SoundChannel::Voice, pIdleSounds[RANDOM_LONG(0, ArraySize(pIdleSounds) - 1)], VOL_NORM, ATTN_NORM, pitch);
}

void CZombie::AttackSound()
{
	const int pitch = 100 + RANDOM_LONG(-5, 5);

	// Play a random attack sound
	EmitSound(SoundChannel::Voice, pAttackSounds[RANDOM_LONG(0, ArraySize(pAttackSounds) - 1)], VOL_NORM, ATTN_NORM, pitch);
}

void CZombie::HandleAnimEvent(AnimationEvent& event)
{
	switch (event.event)
	{
	case ZOMBIE_AE_ATTACK_RIGHT:
	{
		// do stuff for this event.
//		ALERT( at_console, "Slash right!\n" );
		if (CBaseEntity* pHurt = CheckTraceHullAttack(70, gSkillData.zombieDmgOneSlash, DMG_SLASH); pHurt)
		{
			if (pHurt->pev->flags & (FL_MONSTER | FL_CLIENT))
			{
				pHurt->pev->punchangle.z = -18;
				pHurt->pev->punchangle.x = 5;
				pHurt->SetAbsVelocity(pHurt->GetAbsVelocity() - gpGlobals->v_right * 100);
			}
			// Play a random attack hit sound
			EmitSound(SoundChannel::Weapon, pAttackHitSounds[RANDOM_LONG(0, ArraySize(pAttackHitSounds) - 1)], VOL_NORM, ATTN_NORM, PITCH_NORM + RANDOM_LONG(-5, 5));
		}
		else // Play a random attack miss sound
			EmitSound(SoundChannel::Weapon, pAttackMissSounds[RANDOM_LONG(0, ArraySize(pAttackMissSounds) - 1)], VOL_NORM, ATTN_NORM, PITCH_NORM + RANDOM_LONG(-5, 5));

		if (RANDOM_LONG(0, 1))
			AttackSound();
	}
	break;

	case ZOMBIE_AE_ATTACK_LEFT:
	{
		// do stuff for this event.
//		ALERT( at_console, "Slash left!\n" );
		if (CBaseEntity* pHurt = CheckTraceHullAttack(70, gSkillData.zombieDmgOneSlash, DMG_SLASH); pHurt)
		{
			if (pHurt->pev->flags & (FL_MONSTER | FL_CLIENT))
			{
				pHurt->pev->punchangle.z = 18;
				pHurt->pev->punchangle.x = 5;
				pHurt->SetAbsVelocity(pHurt->GetAbsVelocity() + gpGlobals->v_right * 100);
			}
			EmitSound(SoundChannel::Weapon, pAttackHitSounds[RANDOM_LONG(0, ArraySize(pAttackHitSounds) - 1)], VOL_NORM, ATTN_NORM, PITCH_NORM + RANDOM_LONG(-5, 5));
		}
		else
			EmitSound(SoundChannel::Weapon, pAttackMissSounds[RANDOM_LONG(0, ArraySize(pAttackMissSounds) - 1)], VOL_NORM, ATTN_NORM, PITCH_NORM + RANDOM_LONG(-5, 5));

		if (RANDOM_LONG(0, 1))
			AttackSound();
	}
	break;

	case ZOMBIE_AE_ATTACK_BOTH:
	{
		// do stuff for this event.
		if (CBaseEntity* pHurt = CheckTraceHullAttack(70, gSkillData.zombieDmgBothSlash, DMG_SLASH); pHurt)
		{
			if (pHurt->pev->flags & (FL_MONSTER | FL_CLIENT))
			{
				pHurt->pev->punchangle.x = 5;
				pHurt->SetAbsVelocity(pHurt->GetAbsVelocity() + gpGlobals->v_forward * -100);
			}
			EmitSound(SoundChannel::Weapon, pAttackHitSounds[RANDOM_LONG(0, ArraySize(pAttackHitSounds) - 1)], VOL_NORM, ATTN_NORM, PITCH_NORM + RANDOM_LONG(-5, 5));
		}
		else
			EmitSound(SoundChannel::Weapon, pAttackMissSounds[RANDOM_LONG(0, ArraySize(pAttackMissSounds) - 1)], VOL_NORM, ATTN_NORM, PITCH_NORM + RANDOM_LONG(-5, 5));

		if (RANDOM_LONG(0, 1))
			AttackSound();
	}
	break;

	default:
		CBaseMonster::HandleAnimEvent(event);
		break;
	}
}

void CZombie::Spawn()
{
	Precache();

	SetModel("models/zombie.mdl");
	SetSize(VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	SetSolidType(Solid::SlideBox);
	SetMovetype(Movetype::Step);
	m_bloodColor = BLOOD_COLOR_GREEN;
	pev->health = gSkillData.zombieHealth;
	pev->view_ofs = VEC_VIEW;// position of the eyes relative to monster's origin.
	m_flFieldOfView = 0.5;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState = NPCState::None;
	m_afCapability = bits_CAP_DOORS_GROUP;

	MonsterInit();
}

void CZombie::Precache()
{
	PRECACHE_MODEL("models/zombie.mdl");

	PRECACHE_SOUND_ARRAY(pAttackHitSounds);
	PRECACHE_SOUND_ARRAY(pAttackMissSounds);
	PRECACHE_SOUND_ARRAY(pAttackSounds);
	PRECACHE_SOUND_ARRAY(pIdleSounds);
	PRECACHE_SOUND_ARRAY(pAlertSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
}

int CZombie::IgnoreConditions()
{
	int iIgnore = CBaseMonster::IgnoreConditions();

	if ((m_Activity == ACT_MELEE_ATTACK1) || (m_Activity == ACT_MELEE_ATTACK1))
	{
#if 0
		if (pev->health < 20)
			iIgnore |= (bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE);
		else
#endif			
			if (m_flNextFlinch >= gpGlobals->time)
				iIgnore |= (bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE);
	}

	if ((m_Activity == ACT_SMALL_FLINCH) || (m_Activity == ACT_BIG_FLINCH))
	{
		if (m_flNextFlinch < gpGlobals->time)
			m_flNextFlinch = gpGlobals->time + ZOMBIE_FLINCH_DELAY;
	}

	return iIgnore;
}
