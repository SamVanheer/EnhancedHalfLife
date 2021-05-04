/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#include "hornet.h"

int iHornetTrail;
int iHornetPuff;

LINK_ENTITY_TO_CLASS(hornet, CHornet);

TYPEDESCRIPTION	CHornet::m_SaveData[] =
{
	DEFINE_FIELD(CHornet, m_flStopAttack, FIELD_TIME),
	DEFINE_FIELD(CHornet, m_iHornetType, FIELD_INTEGER),
	DEFINE_FIELD(CHornet, m_flFlySpeed, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CHornet, CBaseMonster);

bool CHornet::TakeDamage(const TakeDamageInfo& info)
{
	int damageTypes = info.GetDamageTypes();
	// filter these bits a little.
	damageTypes &= ~(DMG_ALWAYSGIB);
	damageTypes |= DMG_NEVERGIB;

	return CBaseMonster::TakeDamage({info.GetInflictor(), info.GetAttacker() , info.GetDamage(), damageTypes});
}

void CHornet::Spawn()
{
	Precache();

	SetMovetype(Movetype::Fly);
	SetSolidType(Solid::BBox);
	SetDamageMode(DamageMode::Yes);
	pev->flags |= FL_MONSTER;
	pev->health = 1;// weak!

	if (g_pGameRules->IsMultiplayer())
	{
		// hornets don't live as long in multiplayer
		m_flStopAttack = gpGlobals->time + 3.5;
	}
	else
	{
		m_flStopAttack = gpGlobals->time + 5.0;
	}

	m_flFieldOfView = 0.9; // +- 25 degrees

	if (RANDOM_LONG(1, 5) <= 2)
	{
		m_iHornetType = HORNET_TYPE_RED;
		m_flFlySpeed = HORNET_RED_SPEED;
	}
	else
	{
		m_iHornetType = HORNET_TYPE_ORANGE;
		m_flFlySpeed = HORNET_ORANGE_SPEED;
	}

	SetModel("models/hornet.mdl");
	SetSize(Vector(-4, -4, -4), Vector(4, 4, 4));

	SetTouch(&CHornet::DieTouch);
	SetThink(&CHornet::StartTrack);

	if (auto owner = GetOwner(); !IsNullEnt(owner) && (owner->pev->flags & FL_CLIENT))
	{
		pev->dmg = gSkillData.plrDmgHornet;
	}
	else
	{
		// no real owner, or owner isn't a client. 
		pev->dmg = gSkillData.monDmgHornet;
	}

	pev->nextthink = gpGlobals->time + 0.1;
	ResetSequenceInfo();
}

void CHornet::Precache()
{
	PRECACHE_MODEL("models/hornet.mdl");

	PRECACHE_SOUND("agrunt/ag_fire1.wav");
	PRECACHE_SOUND("agrunt/ag_fire2.wav");
	PRECACHE_SOUND("agrunt/ag_fire3.wav");

	PRECACHE_SOUND("hornet/ag_buzz1.wav");
	PRECACHE_SOUND("hornet/ag_buzz2.wav");
	PRECACHE_SOUND("hornet/ag_buzz3.wav");

	PRECACHE_SOUND("hornet/ag_hornethit1.wav");
	PRECACHE_SOUND("hornet/ag_hornethit2.wav");
	PRECACHE_SOUND("hornet/ag_hornethit3.wav");

	iHornetPuff = PRECACHE_MODEL("sprites/muz1.spr");
	iHornetTrail = PRECACHE_MODEL("sprites/laserbeam.spr");
}

Relationship CHornet::GetRelationship(CBaseEntity* pTarget)
{
	if (pTarget->pev->modelindex == pev->modelindex)
	{
		return Relationship::None;
	}

	return CBaseMonster::GetRelationship(pTarget);
}

int CHornet::Classify()
{
	if (auto owner = GetOwner(); owner && owner->pev->flags & FL_CLIENT)
	{
		return CLASS_PLAYER_BIOWEAPON;
	}

	return	CLASS_ALIEN_BIOWEAPON;
}

void CHornet::StartTrack()
{
	IgniteTrail();

	SetTouch(&CHornet::TrackTouch);
	SetThink(&CHornet::TrackTarget);

	pev->nextthink = gpGlobals->time + 0.1;
}

void CHornet::StartDart()
{
	IgniteTrail();

	SetTouch(&CHornet::DartTouch);

	SetThink(&CHornet::SUB_Remove);
	pev->nextthink = gpGlobals->time + 4;
}

void CHornet::IgniteTrail()
{
	/*

	  ted's suggested trail colors:

	r161
	g25
	b97

	r173
	g39
	b14

	old colors
			case HORNET_TYPE_RED:
				WRITE_BYTE( 255 );   // r, g, b
				WRITE_BYTE( 128 );   // r, g, b
				WRITE_BYTE( 0 );   // r, g, b
				break;
			case HORNET_TYPE_ORANGE:
				WRITE_BYTE( 0   );   // r, g, b
				WRITE_BYTE( 100 );   // r, g, b
				WRITE_BYTE( 255 );   // r, g, b
				break;

	*/

	// trail
	MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMFOLLOW);
	WRITE_SHORT(entindex());	// entity
	WRITE_SHORT(iHornetTrail);	// model
	WRITE_BYTE(10); // life
	WRITE_BYTE(2);  // width

	switch (m_iHornetType)
	{
	case HORNET_TYPE_RED:
		WRITE_BYTE(179);   // r, g, b
		WRITE_BYTE(39);   // r, g, b
		WRITE_BYTE(14);   // r, g, b
		break;
	case HORNET_TYPE_ORANGE:
		WRITE_BYTE(255);   // r, g, b
		WRITE_BYTE(128);   // r, g, b
		WRITE_BYTE(0);   // r, g, b
		break;
	}

	WRITE_BYTE(128);	// brightness

	MESSAGE_END();
}

void CHornet::TrackTarget()
{
	StudioFrameAdvance();

	if (gpGlobals->time > m_flStopAttack)
	{
		SetTouch(nullptr);
		SetThink(&CHornet::SUB_Remove);
		pev->nextthink = gpGlobals->time + 0.1;
		return;
	}

	// UNDONE: The player pointer should come back after returning from another level
	if (m_hEnemy == nullptr)
	{// enemy is dead.
		Look(512);
		m_hEnemy = BestVisibleEnemy();
	}

	if (m_hEnemy != nullptr && IsVisible(m_hEnemy))
	{
		m_vecEnemyLKP = m_hEnemy->BodyTarget(GetAbsOrigin());
	}
	else
	{
		m_vecEnemyLKP = m_vecEnemyLKP + GetAbsVelocity() * m_flFlySpeed * 0.1;
	}

	const Vector vecDirToEnemy = (m_vecEnemyLKP - GetAbsOrigin()).Normalize();

	Vector vecFlightDir;
	if (GetAbsVelocity().Length() < 0.1)
		vecFlightDir = vecDirToEnemy;
	else
		vecFlightDir = GetAbsVelocity().Normalize();

	// measure how far the turn is, the wider the turn, the slow we'll go this time.
	float flDelta = DotProduct(vecFlightDir, vecDirToEnemy);

	if (flDelta < 0.5)
	{// hafta turn wide again. play sound
		switch (RANDOM_LONG(0, 2))
		{
		case 0:	EmitSound(SoundChannel::Voice, "hornet/ag_buzz1.wav", HORNET_BUZZ_VOLUME); break;
		case 1:	EmitSound(SoundChannel::Voice, "hornet/ag_buzz2.wav", HORNET_BUZZ_VOLUME); break;
		case 2:	EmitSound(SoundChannel::Voice, "hornet/ag_buzz3.wav", HORNET_BUZZ_VOLUME); break;
		}
	}

	if (flDelta <= 0 && m_iHornetType == HORNET_TYPE_RED)
	{// no flying backwards, but we don't want to invert this, cause we'd go fast when we have to turn REAL far.
		flDelta = 0.25;
	}

	SetAbsVelocity((vecFlightDir + vecDirToEnemy).Normalize());

	if (auto owner = GetOwner(); owner && (owner->pev->flags & FL_MONSTER))
	{
		// random pattern only applies to hornets fired by monsters, not players. 
		// scramble the flight dir a bit.
		SetAbsVelocity(GetAbsVelocity() + Vector{RANDOM_FLOAT(-0.10, 0.10), RANDOM_FLOAT(-0.10, 0.10), RANDOM_FLOAT(-0.10, 0.10)});
	}

	switch (m_iHornetType)
	{
	case HORNET_TYPE_RED:
		SetAbsVelocity(GetAbsVelocity() * (m_flFlySpeed * flDelta));// scale the dir by the ( speed * width of turn )
		pev->nextthink = gpGlobals->time + RANDOM_FLOAT(0.1, 0.3);
		break;
	case HORNET_TYPE_ORANGE:
		SetAbsVelocity(GetAbsVelocity() * m_flFlySpeed);// do not have to slow down to turn.
		pev->nextthink = gpGlobals->time + 0.1;// fixed think time
		break;
	}

	pev->angles = VectorAngles(GetAbsVelocity());

	SetSolidType(Solid::BBox);

	// if hornet is close to the enemy, jet in a straight line for a half second.
	// (only in the single player game)
	if (m_hEnemy != nullptr && !g_pGameRules->IsMultiplayer())
	{
		if (flDelta >= 0.4 && (GetAbsOrigin() - m_vecEnemyLKP).Length() <= 300)
		{
			MESSAGE_BEGIN(MessageDest::PVS, SVC_TEMPENTITY, GetAbsOrigin());
			WRITE_BYTE(TE_SPRITE);
			WRITE_COORD(GetAbsOrigin().x);	// pos
			WRITE_COORD(GetAbsOrigin().y);
			WRITE_COORD(GetAbsOrigin().z);
			WRITE_SHORT(iHornetPuff);		// model
			// WRITE_BYTE( 0 );				// life * 10
			WRITE_BYTE(2);				// size * 10
			WRITE_BYTE(128);			// brightness
			MESSAGE_END();

			switch (RANDOM_LONG(0, 2))
			{
			case 0:	EmitSound(SoundChannel::Voice, "hornet/ag_buzz1.wav", HORNET_BUZZ_VOLUME); break;
			case 1:	EmitSound(SoundChannel::Voice, "hornet/ag_buzz2.wav", HORNET_BUZZ_VOLUME); break;
			case 2:	EmitSound(SoundChannel::Voice, "hornet/ag_buzz3.wav", HORNET_BUZZ_VOLUME); break;
			}
			SetAbsVelocity(GetAbsVelocity() * 2);
			pev->nextthink = gpGlobals->time + 1.0;
			// don't attack again
			m_flStopAttack = gpGlobals->time;
		}
	}
}

void CHornet::TrackTouch(CBaseEntity* pOther)
{
	if (pOther == GetOwner() || pOther->pev->modelindex == pev->modelindex)
	{// bumped into the guy that shot it.
		SetSolidType(Solid::Not);
		return;
	}

	if (GetRelationship(pOther) <= Relationship::None)
	{
		// hit something we don't want to hurt, so turn around.
		Vector newVelocity = GetAbsVelocity().Normalize();

		newVelocity.x *= -1;
		newVelocity.y *= -1;

		SetAbsOrigin(GetAbsOrigin() + newVelocity * 4); // bounce the hornet off a bit.
		SetAbsVelocity(newVelocity * m_flFlySpeed);

		return;
	}

	DieTouch(pOther);
}

void CHornet::DartTouch(CBaseEntity* pOther)
{
	DieTouch(pOther);
}

void CHornet::DieTouch(CBaseEntity* pOther)
{
	if (pOther && pOther->pev->takedamage)
	{// do the damage

		switch (RANDOM_LONG(0, 2))
		{// buzz when you plug someone
		case 0:	EmitSound(SoundChannel::Voice, "hornet/ag_hornethit1.wav"); break;
		case 1:	EmitSound(SoundChannel::Voice, "hornet/ag_hornethit2.wav"); break;
		case 2:	EmitSound(SoundChannel::Voice, "hornet/ag_hornethit3.wav"); break;
		}

		pOther->TakeDamage({this, GetOwner(), pev->dmg, DMG_BULLET});
	}

	pev->modelindex = 0;// so will disappear for the 0.1 secs we wait until NEXTTHINK gets rid
	SetSolidType(Solid::Not);

	SetThink(&CHornet::SUB_Remove);
	pev->nextthink = gpGlobals->time + 1;// stick around long enough for the sound to finish!
}
