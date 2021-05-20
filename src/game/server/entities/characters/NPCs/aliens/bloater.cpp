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

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
constexpr int BLOATER_AE_ATTACK_MELEE1 = 0x01;

class CBloater : public CBaseMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int  Classify() override;
	void HandleAnimEvent(AnimationEvent& event) override;

	void PainSound() override;
	void AlertSound() override;
	void IdleSound() override;
	void AttackSnd();

	// No range attacks
	bool CheckRangeAttack1(float flDot, float flDist) override { return false; }
	bool CheckRangeAttack2(float flDot, float flDist) override { return false; }
	bool TakeDamage(const TakeDamageInfo& info) override;
};

LINK_ENTITY_TO_CLASS(monster_bloater, CBloater);

int	CBloater::Classify()
{
	return	CLASS_ALIEN_MONSTER;
}

void CBloater::SetYawSpeed()
{
	pev->yaw_speed = 120;
}

bool CBloater::TakeDamage(const TakeDamageInfo& info)
{
	PainSound();
	return CBaseMonster::TakeDamage(info);
}

void CBloater::PainSound()
{
#if 0	
	int pitch = 95 + RANDOM_LONG(0, 9);

	switch (RANDOM_LONG(0, 5))
	{
	case 0:
		EmitSound(SoundChannel::Voice, "zombie/zo_pain1.wav", VOL_NORM, ATTN_NORM, pitch);
		break;
	case 1:
		EmitSound(SoundChannel::Voice, "zombie/zo_pain2.wav", VOL_NORM, ATTN_NORM, pitch);
		break;
	default:
		break;
	}
#endif
}

void CBloater::AlertSound()
{
#if 0
	int pitch = 95 + RANDOM_LONG(0, 9);

	switch (RANDOM_LONG(0, 2))
	{
	case 0:
		EmitSound(SoundChannel::Voice, "zombie/zo_alert10.wav", VOL_NORM, ATTN_NORM, pitch);
		break;
	case 1:
		EmitSound(SoundChannel::Voice, "zombie/zo_alert20.wav", VOL_NORM, ATTN_NORM, pitch);
		break;
	case 2:
		EmitSound(SoundChannel::Voice, "zombie/zo_alert30.wav", VOL_NORM, ATTN_NORM, pitch);
		break;
	}
#endif
}

void CBloater::IdleSound()
{
#if 0
	int pitch = 95 + RANDOM_LONG(0, 9);

	switch (RANDOM_LONG(0, 2))
	{
	case 0:
		EmitSound(SoundChannel::Voice, "zombie/zo_idle1.wav", VOL_NORM, ATTN_NORM, pitch);
		break;
	case 1:
		EmitSound(SoundChannel::Voice, "zombie/zo_idle2.wav", VOL_NORM, ATTN_NORM, pitch);
		break;
	case 2:
		EmitSound(SoundChannel::Voice, "zombie/zo_idle3.wav", VOL_NORM, ATTN_NORM, pitch);
		break;
	}
#endif
}

void CBloater::AttackSnd()
{
#if 0
	int pitch = 95 + RANDOM_LONG(0, 9);

	switch (RANDOM_LONG(0, 1))
	{
	case 0:
		EmitSound(SoundChannel::Voice, "zombie/zo_attack1.wav", VOL_NORM, ATTN_NORM, pitch);
		break;
	case 1:
		EmitSound(SoundChannel::Voice, "zombie/zo_attack2.wav", VOL_NORM, ATTN_NORM, pitch);
		break;
	}
#endif
}

void CBloater::HandleAnimEvent(AnimationEvent& event)
{
	switch (event.event)
	{
	case BLOATER_AE_ATTACK_MELEE1:
	{
		// do stuff for this event.
		AttackSnd();
	}
	break;

	default:
		CBaseMonster::HandleAnimEvent(event);
		break;
	}
}

void CBloater::Spawn()
{
	Precache();

	SetModel("models/floater.mdl");
	SetSize(VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	SetSolidType(Solid::SlideBox);
	SetMovetype(Movetype::Fly);
	pev->spawnflags |= FL_FLY;
	m_bloodColor = BLOOD_COLOR_GREEN;
	pev->health = 40;
	pev->view_ofs = VEC_VIEW;// position of the eyes relative to monster's origin.
	m_flFieldOfView = 0.5;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState = NPCState::None;

	MonsterInit();
}

void CBloater::Precache()
{
	PRECACHE_MODEL("models/floater.mdl");
}