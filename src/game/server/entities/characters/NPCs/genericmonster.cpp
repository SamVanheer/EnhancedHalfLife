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

// For holograms, make them not solid so the player can walk through them
constexpr int	SF_GENERICMONSTER_NOTSOLID = 4;

/**
*	@brief purely for scripted sequence work.
*/
class CGenericMonster : public CBaseMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int  Classify() override;
	void HandleAnimEvent(AnimationEvent& event) override;

	/**
	*	@brief generic monster can't hear.
	*/
	int SoundMask() override;
};
LINK_ENTITY_TO_CLASS(monster_generic, CGenericMonster);

int	CGenericMonster::Classify()
{
	return CLASS_PLAYER_ALLY;
}

void CGenericMonster::SetYawSpeed()
{
	int ys;

	switch (m_Activity)
	{
	case ACT_IDLE:
	default:
		ys = 90;
	}

	pev->yaw_speed = ys;
}

void CGenericMonster::HandleAnimEvent(AnimationEvent& event)
{
	switch (event.event)
	{
	case 0:
	default:
		CBaseMonster::HandleAnimEvent(event);
		break;
	}
}

int CGenericMonster::SoundMask()
{
	return bits_SOUND_NONE;
}

void CGenericMonster::Spawn()
{
	Precache();

	SetModel(STRING(pev->model));

	/*
		if ( AreStringsEqual( STRING(pev->model), "models/player.mdl" ) )
			SetSize(VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);
		else
			SetSize(VEC_HULL_MIN, VEC_HULL_MAX);
	*/

	if (AreStringsEqual(STRING(pev->model), "models/player.mdl") || AreStringsEqual(STRING(pev->model), "models/holo.mdl"))
		SetSize(VEC_HULL_MIN, VEC_HULL_MAX);
	else
		SetSize(VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	SetSolidType(Solid::SlideBox);
	SetMovetype(Movetype::Step);
	m_bloodColor = BLOOD_COLOR_RED;
	pev->health = 8;
	m_flFieldOfView = 0.5;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState = NPCState::None;

	MonsterInit();

	if (pev->spawnflags & SF_GENERICMONSTER_NOTSOLID)
	{
		SetSolidType(Solid::Not);
		SetDamageMode(DamageMode::No);
	}
}

void CGenericMonster::Precache()
{
	PRECACHE_MODEL(STRING(pev->model));
}
