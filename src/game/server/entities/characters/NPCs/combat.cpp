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

/**
*	@file
*
*	functions dealing with damage infliction & death
*/

#include "animation.h"
#include "func_break.h"

constexpr int GERMAN_GIB_COUNT = 4;
constexpr int HUMAN_GIB_COUNT = 6;
constexpr int ALIEN_GIB_COUNT = 4;

// HACKHACK -- The gib velocity equations don't work
void CGib::LimitVelocity()
{
	float length = GetAbsVelocity().Length();

	// ceiling at 1500.  The gib velocity equation is not bounded properly.  Rather than tune it
	// in 3 separate places again, I'll just limit it here.
	if (length > 1500.0)
		SetAbsVelocity(GetAbsVelocity().Normalize() * 1500);		// This should really be sv_maxvelocity * 0.75 or something
}

void CGib::SpawnStickyGibs(CBaseEntity* pVictim, const Vector& vecOrigin, int cGibs)
{
	if (g_Language == LANGUAGE_GERMAN)
	{
		// no sticky gibs in germany right now!
		return;
	}

	for (int i = 0; i < cGibs; i++)
	{
		CGib* pGib = GetClassPtr((CGib*)nullptr);

		pGib->Spawn("models/stickygib.mdl");
		pGib->pev->body = RANDOM_LONG(0, 2);

		if (pVictim)
		{
			pGib->SetAbsOrigin(vecOrigin + Vector{RANDOM_FLOAT(-3, 3), RANDOM_FLOAT(-3, 3), RANDOM_FLOAT(-3, 3)});

			/*
			Vector origin = pVictim->pev->absmin;

			for (int i = 0; i < 3; ++i)
			{
				origin[i] = origin[i] + pVictim->pev->size[i] * RANDOM_FLOAT(0, 1);
			}

			pGib->SetAbsOrigin(origin);
			*/

			// make the gib fly away from the attack vector
			Vector gibVelocity = g_vecAttackDir * -1;

			// mix in some noise
			gibVelocity.x += RANDOM_FLOAT(-0.15, 0.15);
			gibVelocity.y += RANDOM_FLOAT(-0.15, 0.15);
			gibVelocity.z += RANDOM_FLOAT(-0.15, 0.15);

			gibVelocity = gibVelocity * 900;

			pGib->pev->avelocity.x = RANDOM_FLOAT(250, 400);
			pGib->pev->avelocity.y = RANDOM_FLOAT(250, 400);

			// copy owner's blood color
			pGib->m_bloodColor = pVictim->BloodColor();

			if (pVictim->pev->health > -50)
			{
				gibVelocity = gibVelocity * 0.7;
			}
			else if (pVictim->pev->health > -200)
			{
				gibVelocity = gibVelocity * 2;
			}
			else
			{
				gibVelocity = gibVelocity * 4;
			}

			pGib->SetAbsVelocity(gibVelocity);

			pGib->SetMovetype(Movetype::Toss);
			pGib->SetSolidType(Solid::BBox);
			pGib->SetSize(vec3_origin, vec3_origin);
			pGib->SetTouch(&CGib::StickyGibTouch);
			pGib->SetThink(nullptr);
		}
		pGib->LimitVelocity();
	}
}

void CGib::SpawnHeadGib(CBaseEntity* pVictim)
{
	CGib* pGib = GetClassPtr((CGib*)nullptr);

	if (g_Language == LANGUAGE_GERMAN)
	{
		pGib->Spawn("models/germangibs.mdl");// throw one head
		pGib->pev->body = 0;
	}
	else
	{
		pGib->Spawn("models/hgibs.mdl");// throw one head
		pGib->pev->body = 0;
	}

	if (pVictim)
	{
		pGib->SetAbsOrigin(pVictim->GetAbsOrigin() + pVictim->pev->view_ofs);

		CBaseEntity* pPlayer = UTIL_FindClientInPVS(pGib);

		Vector velocity;

		if (RANDOM_LONG(0, 100) <= 5 && !IsNullEnt(pPlayer))
		{
			// 5% chance head will be thrown at player's face.
			velocity = ((pPlayer->GetAbsOrigin() + pPlayer->pev->view_ofs) - pGib->GetAbsOrigin()).Normalize() * 300;
			velocity.z += 100;
		}
		else
		{
			velocity = {RANDOM_FLOAT(-100, 100), RANDOM_FLOAT(-100, 100), RANDOM_FLOAT(200, 300)};
		}

		pGib->SetAbsVelocity(velocity);

		pGib->pev->avelocity.x = RANDOM_FLOAT(100, 200);
		pGib->pev->avelocity.y = RANDOM_FLOAT(100, 300);

		// copy owner's blood color
		pGib->m_bloodColor = pVictim->BloodColor();

		if (pVictim->pev->health > -50)
		{
			pGib->SetAbsVelocity(pGib->GetAbsVelocity() * 0.7);
		}
		else if (pVictim->pev->health > -200)
		{
			pGib->SetAbsVelocity(pGib->GetAbsVelocity() * 2);
		}
		else
		{
			pGib->SetAbsVelocity(pGib->GetAbsVelocity() * 4);
		}
	}
	pGib->LimitVelocity();
}

void CGib::SpawnRandomGibs(CBaseEntity* pVictim, int cGibs, bool human)
{
	for (int cSplat = 0; cSplat < cGibs; cSplat++)
	{
		CGib* pGib = GetClassPtr((CGib*)nullptr);

		if (g_Language == LANGUAGE_GERMAN)
		{
			pGib->Spawn("models/germangibs.mdl");
			pGib->pev->body = RANDOM_LONG(0, GERMAN_GIB_COUNT - 1);
		}
		else
		{
			if (human)
			{
				// human pieces
				pGib->Spawn("models/hgibs.mdl");
				pGib->pev->body = RANDOM_LONG(1, HUMAN_GIB_COUNT - 1);// start at one to avoid throwing random amounts of skulls (0th gib)
			}
			else
			{
				// aliens
				pGib->Spawn("models/agibs.mdl");
				pGib->pev->body = RANDOM_LONG(0, ALIEN_GIB_COUNT - 1);
			}
		}

		if (pVictim)
		{
			// spawn the gib somewhere in the monster's bounding volume
			// absmin.z is in the floor because the engine subtracts 1 to enlarge the box
			Vector origin = pVictim->pev->absmin + Vector(0, 0, 1);

			for (int i = 0; i < 3; ++i)
			{
				origin[i] = origin[i] + pVictim->pev->size[i] * RANDOM_FLOAT(0, 1);
			}

			pGib->SetAbsOrigin(origin);

			// make the gib fly away from the attack vector
			Vector gibVelocity = g_vecAttackDir * -1;

			// mix in some noise
			gibVelocity.x += RANDOM_FLOAT(-0.25, 0.25);
			gibVelocity.y += RANDOM_FLOAT(-0.25, 0.25);
			gibVelocity.z += RANDOM_FLOAT(-0.25, 0.25);

			gibVelocity = gibVelocity * RANDOM_FLOAT(300, 400);

			pGib->pev->avelocity.x = RANDOM_FLOAT(100, 200);
			pGib->pev->avelocity.y = RANDOM_FLOAT(100, 300);

			// copy owner's blood color
			pGib->m_bloodColor = pVictim->BloodColor();

			if (pVictim->pev->health > -50)
			{
				gibVelocity = gibVelocity * 0.7;
			}
			else if (pVictim->pev->health > -200)
			{
				gibVelocity = gibVelocity * 2;
			}
			else
			{
				gibVelocity = gibVelocity * 4;
			}

			pGib->SetAbsVelocity(gibVelocity);

			pGib->SetSolidType(Solid::BBox);
			pGib->SetSize(vec3_origin, vec3_origin);
		}
		pGib->LimitVelocity();
	}
}

bool CBaseMonster::HasHumanGibs()
{
	const int myClass = Classify();

	if (myClass == CLASS_HUMAN_MILITARY ||
		myClass == CLASS_PLAYER_ALLY ||
		myClass == CLASS_HUMAN_PASSIVE ||
		myClass == CLASS_PLAYER)

		return true;

	return false;
}

bool CBaseMonster::HasAlienGibs()
{
	const int myClass = Classify();

	if (myClass == CLASS_ALIEN_MILITARY ||
		myClass == CLASS_ALIEN_MONSTER ||
		myClass == CLASS_ALIEN_PASSIVE ||
		myClass == CLASS_INSECT ||
		myClass == CLASS_ALIEN_PREDATOR ||
		myClass == CLASS_ALIEN_PREY)

		return true;

	return false;
}

void CBaseMonster::FadeMonster()
{
	StopAnimation();
	SetAbsVelocity(vec3_origin);
	SetMovetype(Movetype::None);
	pev->avelocity = vec3_origin;
	pev->animtime = gpGlobals->time;
	pev->effects |= EF_NOINTERP;
	SUB_StartFadeOut();
}

void CBaseMonster::GibMonster()
{
	bool gibbed = false;

	EmitSound(SoundChannel::Weapon, "common/bodysplat.wav");

	// only humans throw skulls !!!UNDONE - eventually monsters will have their own sets of gibs
	if (HasHumanGibs())
	{
		if (CVAR_GET_FLOAT("violence_hgibs") != 0)	// Only the player will ever get here
		{
			CGib::SpawnHeadGib(this);
			CGib::SpawnRandomGibs(this, 4, true);	// throw some human gibs.
		}
		gibbed = true;
	}
	else if (HasAlienGibs())
	{
		if (CVAR_GET_FLOAT("violence_agibs") != 0)	// Should never get here, but someone might call it directly
		{
			CGib::SpawnRandomGibs(this, 4, false);	// Throw alien gibs
		}
		gibbed = true;
	}

	if (!IsPlayer())
	{
		if (gibbed)
		{
			// don't remove players!
			SetThink(&CBaseMonster::SUB_Remove);
			pev->nextthink = gpGlobals->time;
		}
		else
		{
			FadeMonster();
		}
	}
}

Activity CBaseMonster::GetDeathActivity()
{
	if (pev->deadflag != DeadFlag::No)
	{
		// don't run this while dying.
		return m_IdealActivity;
	}

	const Vector vecSrc = Center();

	UTIL_MakeVectors(GetAbsAngles());
	const float flDot = DotProduct(gpGlobals->v_forward, g_vecAttackDir * -1);

	bool fTriedDirection = false;
	Activity deathActivity = ACT_DIESIMPLE;// in case we can't find any special deaths to do.

	switch (m_LastHitGroup)
	{
		// try to pick a region-specific death.
	case HITGROUP_HEAD:
		deathActivity = ACT_DIE_HEADSHOT;
		break;

	case HITGROUP_STOMACH:
		deathActivity = ACT_DIE_GUTSHOT;
		break;

	case HITGROUP_GENERIC:
		// try to pick a death based on attack direction
		fTriedDirection = true;

		if (flDot > 0.3)
		{
			deathActivity = ACT_DIEFORWARD;
		}
		else if (flDot <= -0.3)
		{
			deathActivity = ACT_DIEBACKWARD;
		}
		break;

	default:
		// try to pick a death based on attack direction
		fTriedDirection = true;

		if (flDot > 0.3)
		{
			deathActivity = ACT_DIEFORWARD;
		}
		else if (flDot <= -0.3)
		{
			deathActivity = ACT_DIEBACKWARD;
		}
		break;
	}

	// can we perform the prescribed death?
	if (LookupActivity(deathActivity) == ACTIVITY_NOT_AVAILABLE)
	{
		// no! did we fail to perform a directional death? 
		if (fTriedDirection)
		{
			// if yes, we're out of options. Go simple.
			deathActivity = ACT_DIESIMPLE;
		}
		else
		{
			// cannot perform the ideal region-specific death, so try a direction.
			if (flDot > 0.3)
			{
				deathActivity = ACT_DIEFORWARD;
			}
			else if (flDot <= -0.3)
			{
				deathActivity = ACT_DIEBACKWARD;
			}
		}
	}

	if (LookupActivity(deathActivity) == ACTIVITY_NOT_AVAILABLE)
	{
		// if we're still invalid, simple is our only option.
		deathActivity = ACT_DIESIMPLE;
	}

	if (deathActivity == ACT_DIEFORWARD)
	{
		// make sure there's room to fall forward
		TraceResult	tr;
		UTIL_TraceHull(vecSrc, vecSrc + gpGlobals->v_forward * 64, IgnoreMonsters::No, Hull::Head, this, &tr);

		if (tr.flFraction != 1.0)
		{
			deathActivity = ACT_DIESIMPLE;
		}
	}

	if (deathActivity == ACT_DIEBACKWARD)
	{
		// make sure there's room to fall backward
		TraceResult	tr;
		UTIL_TraceHull(vecSrc, vecSrc - gpGlobals->v_forward * 64, IgnoreMonsters::No, Hull::Head, this, &tr);

		if (tr.flFraction != 1.0)
		{
			deathActivity = ACT_DIESIMPLE;
		}
	}

	return deathActivity;
}

Activity CBaseMonster::GetSmallFlinchActivity()
{
	bool fTriedDirection = false;
	UTIL_MakeVectors(GetAbsAngles());
	const float flDot = DotProduct(gpGlobals->v_forward, g_vecAttackDir * -1);

	Activity flinchActivity;

	switch (m_LastHitGroup)
	{
		// pick a region-specific flinch
	case HITGROUP_HEAD:
		flinchActivity = ACT_FLINCH_HEAD;
		break;
	case HITGROUP_STOMACH:
		flinchActivity = ACT_FLINCH_STOMACH;
		break;
	case HITGROUP_LEFTARM:
		flinchActivity = ACT_FLINCH_LEFTARM;
		break;
	case HITGROUP_RIGHTARM:
		flinchActivity = ACT_FLINCH_RIGHTARM;
		break;
	case HITGROUP_LEFTLEG:
		flinchActivity = ACT_FLINCH_LEFTLEG;
		break;
	case HITGROUP_RIGHTLEG:
		flinchActivity = ACT_FLINCH_RIGHTLEG;
		break;
	case HITGROUP_GENERIC:
	default:
		// just get a generic flinch.
		flinchActivity = ACT_SMALL_FLINCH;
		break;
	}

	// do we have a sequence for the ideal activity?
	if (LookupActivity(flinchActivity) == ACTIVITY_NOT_AVAILABLE)
	{
		flinchActivity = ACT_SMALL_FLINCH;
	}

	return flinchActivity;
}

void CBaseMonster::BecomeDead()
{
	SetDamageMode(DamageMode::Yes);// don't let autoaim aim at corpses.

	// give the corpse half of the monster's original maximum health. 
	pev->health = pev->max_health / 2;
	pev->max_health = 5; // max_health now becomes a counter for how many blood decals the corpse can place.

	// make the corpse fly away from the attack vector
	SetMovetype(Movetype::Toss);
	//pev->flags &= ~FL_ONGROUND;
	//SetAbsOrigin(GetAbsOrigin() + Vector(0, 0, 2));
	//SetAbsVelocity(g_vecAttackDir * -1);
	//SetAbsVelocity(GetAbsVelocity() * RANDOM_FLOAT( 300, 400 ));
}

bool CBaseMonster::ShouldGibMonster(GibType gibType)
{
	if ((gibType == GibType::Normal && pev->health < GIB_HEALTH_VALUE) || (gibType == GibType::Always))
		return true;

	return false;
}

void CBaseMonster::CallGibMonster()
{
	bool fade = false;

	if (HasHumanGibs())
	{
		if (CVAR_GET_FLOAT("violence_hgibs") == 0)
			fade = true;
	}
	else if (HasAlienGibs())
	{
		if (CVAR_GET_FLOAT("violence_agibs") == 0)
			fade = true;
	}

	SetDamageMode(DamageMode::No);
	SetSolidType(Solid::Not);// do something with the body. while monster blows up

	if (fade)
	{
		FadeMonster();
	}
	else
	{
		pev->effects = EF_NODRAW; // make the model invisible.
		GibMonster();
	}

	pev->deadflag = DeadFlag::Dead;
	CheckAITrigger();

	// don't let the status bar glitch for players.with <0 health.
	if (pev->health < -99)
	{
		pev->health = 0;
	}

	if (ShouldFadeOnDeath() && !fade)
		UTIL_Remove(this);
}

void CBaseMonster::Killed(const KilledInfo& info)
{
	if (HasMemory(bits_MEMORY_KILLED))
	{
		if (ShouldGibMonster(info.GetGibType()))
			CallGibMonster();
		return;
	}

	Remember(bits_MEMORY_KILLED);

	// clear the deceased's sound channels.(may have been firing or reloading when killed)
	EmitSound(SoundChannel::Weapon, "common/null.wav");
	m_IdealMonsterState = NPCState::Dead;
	// Make sure this condition is fired too (TakeDamage breaks out before this happens on death)
	SetConditions(bits_COND_LIGHT_DAMAGE);

	// tell owner ( if any ) that we're dead.This is mostly for MonsterMaker functionality.
	if (CBaseEntity* pOwner = GetOwner(); pOwner)
	{
		pOwner->DeathNotice(this);
	}

	if (ShouldGibMonster(info.GetGibType()))
	{
		CallGibMonster();
		return;
	}
	else if (pev->flags & FL_MONSTER)
	{
		SetTouch(nullptr);
		BecomeDead();
	}

	// don't let the status bar glitch for players.with <0 health.
	if (pev->health < -99)
	{
		pev->health = 0;
	}

	m_IdealMonsterState = NPCState::Dead;
}

void CBaseEntity::SUB_StartFadeOut()
{
	if (GetRenderMode() == RenderMode::Normal)
	{
		SetRenderAmount(255);
		SetRenderMode(RenderMode::TransTexture);
	}

	SetSolidType(Solid::Not);
	pev->avelocity = vec3_origin;

	pev->nextthink = gpGlobals->time + 0.1;
	SetThink(&CBaseEntity::SUB_FadeOut);
}

void CBaseEntity::SUB_FadeOut()
{
	if (GetRenderAmount() > 7)
	{
		SetRenderAmount(GetRenderAmount() - 7);
		pev->nextthink = gpGlobals->time + 0.1;
	}
	else
	{
		SetRenderAmount(0);
		pev->nextthink = gpGlobals->time + 0.2;
		SetThink(&CBaseEntity::SUB_Remove);
	}
}

void CGib::WaitTillLand()
{
	if (!IsInWorld())
	{
		UTIL_Remove(this);
		return;
	}

	if (GetAbsVelocity() == vec3_origin)
	{
		SetThink(&CGib::SUB_StartFadeOut);
		pev->nextthink = gpGlobals->time + m_lifeTime;

		// If you bleed, you stink!
		if (m_bloodColor != DONT_BLEED)
		{
			// ok, start stinkin!
			CSoundEnt::InsertSound(bits_SOUND_MEAT, GetAbsOrigin(), 384, 25);
		}
	}
	else
	{
		// wait and check again in another half second.
		pev->nextthink = gpGlobals->time + 0.5;
	}
}

void CGib::BounceGibTouch(CBaseEntity* pOther)
{
	//if ( RANDOM_LONG(0,1) )
	//	return;// don't bleed everytime

	if (pev->flags & FL_ONGROUND)
	{
		SetAbsVelocity(GetAbsVelocity() * 0.9);
		SetAbsAngles({0, GetAbsAngles().y, 0});
		pev->avelocity.x = 0;
		pev->avelocity.z = 0;
	}
	else
	{
		if (g_Language != LANGUAGE_GERMAN && m_cBloodDecals > 0 && m_bloodColor != DONT_BLEED)
		{
			const Vector vecSpot = GetAbsOrigin() + Vector(0, 0, 8);//move up a bit, and trace down.
			TraceResult	tr;
			UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 0, -24), IgnoreMonsters::Yes, this, &tr);

			UTIL_BloodDecalTrace(&tr, m_bloodColor);

			m_cBloodDecals--;
		}

		if (m_material != Materials::None && RANDOM_LONG(0, 2) == 0)
		{
			float volume;
			float zvel = fabs(GetAbsVelocity().z);

			volume = 0.8 * std::min(1.0, ((float)zvel) / 450.0);

			CBreakable::MaterialSoundRandom(this, m_material, volume);
		}
	}
}

void CGib::StickyGibTouch(CBaseEntity* pOther)
{
	SetThink(&CGib::SUB_Remove);
	pev->nextthink = gpGlobals->time + 10;

	if (!pOther->ClassnameIs("worldspawn"))
	{
		pev->nextthink = gpGlobals->time;
		return;
	}

	TraceResult	tr;
	UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + GetAbsVelocity() * 32, IgnoreMonsters::Yes, this, &tr);

	UTIL_BloodDecalTrace(&tr, m_bloodColor);

	SetAbsVelocity(tr.vecPlaneNormal * -1);
	SetAbsAngles(VectorAngles(GetAbsVelocity()));
	SetAbsVelocity(vec3_origin);
	pev->avelocity = vec3_origin;
	SetMovetype(Movetype::None);
}

void CGib::Spawn(const char* szGibModel)
{
	SetMovetype(Movetype::Bounce);
	pev->friction = 0.55; // deading the bounce a bit

	// sometimes an entity inherits the edict from a former piece of glass,
	// and will spawn using the same render FX or rendermode! bad!
	SetRenderAmount(255);
	SetRenderMode(RenderMode::Normal);
	SetRenderFX(RenderFX::None);
	SetSolidType(Solid::SlideBox);/// hopefully this will fix the VELOCITY TOO LOW crap
	SetClassname("gib");

	SetModel(szGibModel);
	SetSize(vec3_origin, vec3_origin);

	pev->nextthink = gpGlobals->time + 4;
	m_lifeTime = 25;
	SetThink(&CGib::WaitTillLand);
	SetTouch(&CGib::BounceGibTouch);

	m_material = Materials::None;
	m_cBloodDecals = 5;// how many blood decals this gib can place (1 per bounce until none remain). 
}

bool CBaseMonster::GiveHealth(float flHealth, int bitsDamageType)
{
	if (!pev->takedamage)
		return false;

	// clear out any damage types we healed.
	// UNDONE: generic health should not heal any
	// UNDONE: time-based damage

	m_bitsDamageType &= ~(bitsDamageType & ~DMG_TIMEBASED);

	return CBaseEntity::GiveHealth(flHealth, bitsDamageType);
}

bool CBaseMonster::TakeDamage(const TakeDamageInfo& info)
{
	if (!pev->takedamage)
		return false;

	if (!IsAlive())
	{
		return DeadTakeDamage(info);
	}

	if (pev->deadflag == DeadFlag::No)
	{
		// no pain sound during death animation.
		PainSound();// "Ouch!"
	}

	//!!!LATER - make armor consideration here!
	const float flTake = info.GetDamage();

	// set damage type sustained
	m_bitsDamageType |= info.GetDamageTypes();

	// grab the vector of the incoming attack. ( pretend that the inflictor is a little lower than it really is, so the body will tend to fly upward a bit).
	Vector vecDir = vec3_origin;
	if (!IsNullEnt(info.GetInflictor()))
	{
		if (CBaseEntity* pInflictor = info.GetInflictor(); pInflictor)
		{
			vecDir = (pInflictor->Center() - Vector(0, 0, 10) - Center()).Normalize();
			vecDir = g_vecAttackDir = vecDir.Normalize();
		}
	}

	// add to the damage total for clients, which will be sent as a single
	// message at the end of the frame
	if (IsPlayer())
	{
		if (info.GetInflictor())
			pev->dmg_inflictor = info.GetInflictor()->edict();

		pev->dmg_take += flTake;

		// check for godmode or invincibility
		if (pev->flags & FL_GODMODE)
		{
			return false;
		}
	}

	// if this is a player, move him around!
	if ((!IsNullEnt(info.GetInflictor())) && (GetMovetype() == Movetype::Walk) && (!info.GetAttacker() || info.GetAttacker()->GetSolidType() != Solid::Trigger))
	{
		SetAbsVelocity(GetAbsVelocity() + vecDir * -DamageForce(info.GetDamage()));
	}

	// do the damage
	pev->health -= flTake;

	// HACKHACK Don't kill monsters in a script.  Let them break their scripts first
	if (m_MonsterState == NPCState::Script)
	{
		SetConditions(bits_COND_LIGHT_DAMAGE);
		return false;
	}

	if (pev->health <= 0)
	{
		GibType gibType = GibType::Normal;

		if (info.GetDamageTypes() & DMG_ALWAYSGIB)
		{
			gibType = GibType::Always;
		}
		else if (info.GetDamageTypes() & DMG_NEVERGIB)
		{
			gibType = GibType::Never;
		}

		Killed({info.GetInflictor(), info.GetAttacker(), gibType});

		return false;
	}

	// react to the damage (get mad)
	if ((pev->flags & FL_MONSTER) && !IsNullEnt(info.GetAttacker()))
	{
		if (info.GetAttacker()->pev->flags & (FL_MONSTER | FL_CLIENT))
		{// only if the attack was a monster or client!
			// enemy's last known position is somewhere down the vector that the attack came from.
			if (info.GetInflictor())
			{
				if (m_hEnemy == nullptr || info.GetInflictor() == m_hEnemy || !HasConditions(bits_COND_SEE_ENEMY))
				{
					m_vecEnemyLKP = info.GetInflictor()->GetAbsOrigin();
				}
			}
			else
			{
				m_vecEnemyLKP = GetAbsOrigin() + (g_vecAttackDir * 64);
			}

			MakeIdealYaw(m_vecEnemyLKP);

			// add pain to the conditions 
			// !!!HACKHACK - fudged for now. Do we want to have a virtual function to determine what is light and 
			// heavy damage per monster class?
			if (info.GetDamage() > 0)
			{
				SetConditions(bits_COND_LIGHT_DAMAGE);
			}

			if (info.GetDamage() >= 20)
			{
				SetConditions(bits_COND_HEAVY_DAMAGE);
			}
		}
	}

	return true;
}

bool CBaseMonster::DeadTakeDamage(const TakeDamageInfo& info)
{
	// grab the vector of the incoming attack. ( pretend that the inflictor is a little lower than it really is, so the body will tend to fly upward a bit).
	Vector vecDir = vec3_origin;
	if (!IsNullEnt(info.GetInflictor()))
	{
		if (CBaseEntity* pInflictor = info.GetInflictor(); pInflictor)
		{
			vecDir = (pInflictor->Center() - Vector(0, 0, 10) - Center()).Normalize();
			vecDir = g_vecAttackDir = vecDir.Normalize();
		}
	}

#if 0// turn this back on when the bounding box issues are resolved.
	pev->flags &= ~FL_ONGROUND;
	SetAbsOrigin(GetAbsOrigin() + Vector(0, 0, 1));

	// let the damage scoot the corpse around a bit.
	if (!IsNullEnt(pevInflictor) && (pevAttacker->solid != Solid::Trigger))
	{
		SetAbsVelocity(GetAbsVelocity() + vecDir * -DamageForce(flDamage));
	}
#endif

	// kill the corpse if enough damage was done to destroy the corpse and the damage is of a type that is allowed to destroy the corpse.
	if (info.GetDamageTypes() & DMG_GIB_CORPSE)
	{
		if (pev->health <= info.GetDamage())
		{
			pev->health = -50;
			Killed({info.GetInflictor(), info.GetAttacker(), GibType::Always});
			return false;
		}
		// Accumulate corpse gibbing damage, so you can gib with multiple hits
		pev->health -= info.GetDamage() * 0.1;
	}

	return true;
}

float CBaseMonster::DamageForce(float damage)
{
	const float force = std::min(1000.0f, damage * ((32 * 32 * 72.0f) / (pev->size.x * pev->size.y * pev->size.z)) * 5);

	return force;
}

void RadiusDamage(Vector vecSrc, CBaseEntity* pInflictor, CBaseEntity* pAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType)
{
	const float falloff = flRadius ? flDamage / flRadius : 1.0;

	const bool bInWater = UTIL_PointContents(vecSrc) == Contents::Water;

	vecSrc.z += 1;// in case grenade is lying on the ground

	if (!pAttacker)
		pAttacker = pInflictor;

	CBaseEntity* pEntity = nullptr;
	TraceResult	tr;
	// iterate on all entities in the vicinity.
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != nullptr)
	{
		if (pEntity->GetDamageMode() != DamageMode::No)
		{
			// UNDONE: this should check a damage mask, not an ignore
			if (iClassIgnore != CLASS_NONE && pEntity->Classify() == iClassIgnore)
			{// houndeyes don't hurt other houndeyes with their attack
				continue;
			}

			// blast's don't tavel into or out of water
			if (bInWater && pEntity->pev->waterlevel == WaterLevel::Dry)
				continue;
			if (!bInWater && pEntity->pev->waterlevel == WaterLevel::Head)
				continue;

			const Vector vecSpot = pEntity->BodyTarget(vecSrc);

			UTIL_TraceLine(vecSrc, vecSpot, IgnoreMonsters::No, pInflictor, &tr);

			if (tr.flFraction == 1.0 || CBaseEntity::InstanceOrNull(tr.pHit) == pEntity)
			{// the explosion can 'see' this entity, so hurt them!
				if (tr.fStartSolid)
				{
					// if we're stuck inside them, fixup the position and distance
					tr.vecEndPos = vecSrc;
					tr.flFraction = 0.0;
				}

				// decrease damage for an ent that's farther from the bomb.
				float flAdjustedDamage = (vecSrc - tr.vecEndPos).Length() * falloff;
				flAdjustedDamage = flDamage - flAdjustedDamage;

				if (flAdjustedDamage < 0)
				{
					flAdjustedDamage = 0;
				}

				// ALERT( at_console, "hit %s\n", pEntity->GetClassname() );
				if (tr.flFraction != 1.0)
				{
					ClearMultiDamage();
					pEntity->TraceAttack({pInflictor, flAdjustedDamage, (tr.vecEndPos - vecSrc).Normalize(), tr, bitsDamageType});
					ApplyMultiDamage(pInflictor, pAttacker);
				}
				else
				{
					pEntity->TakeDamage({pInflictor, pAttacker, flAdjustedDamage, bitsDamageType});
				}
			}
		}
	}
}

void CBaseMonster::RadiusDamage(CBaseEntity* pInflictor, CBaseEntity* pAttacker, float flDamage, int iClassIgnore, int bitsDamageType)
{
	::RadiusDamage(GetAbsOrigin(), pInflictor, pAttacker, flDamage, flDamage * 2.5, iClassIgnore, bitsDamageType);
}

void CBaseMonster::RadiusDamage(const Vector& vecSrc, CBaseEntity* pInflictor, CBaseEntity* pAttacker, float flDamage, int iClassIgnore, int bitsDamageType)
{
	::RadiusDamage(vecSrc, pInflictor, pAttacker, flDamage, flDamage * 2.5, iClassIgnore, bitsDamageType);
}

CBaseEntity* CBaseMonster::CheckTraceHullAttack(float flDist, int iDamage, int iDmgType)
{
	if (IsPlayer())
		UTIL_MakeVectors(GetAbsAngles());
	else
		UTIL_MakeAimVectors(GetAbsAngles());

	Vector vecStart = GetAbsOrigin();
	vecStart.z += pev->size.z * 0.5;
	const Vector vecEnd = vecStart + (gpGlobals->v_forward * flDist);

	TraceResult tr;
	UTIL_TraceHull(vecStart, vecEnd, IgnoreMonsters::No, Hull::Head, this, &tr);

	if (tr.pHit)
	{
		CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

		if (iDamage > 0)
		{
			pEntity->TakeDamage({this, this, static_cast<float>(iDamage), iDmgType});
		}

		return pEntity;
	}

	return nullptr;
}

bool CBaseMonster::IsInViewCone(CBaseEntity* pEntity)
{
	return IsInViewCone(pEntity->GetAbsOrigin());
}

bool CBaseMonster::IsInViewCone(const Vector& origin)
{
	UTIL_MakeVectors(GetAbsAngles());

	const Vector2D vec2LOS = (origin - GetAbsOrigin()).Make2D().Normalize();

	const float flDot = DotProduct(vec2LOS, gpGlobals->v_forward.Make2D());

	return flDot > m_flFieldOfView;
}

bool CBaseEntity::IsVisible(CBaseEntity* pEntity)
{
	if (IsBitSet(pEntity->pev->flags, FL_NOTARGET))
		return false;

	// don't look through water
	if ((pev->waterlevel != WaterLevel::Head && pEntity->pev->waterlevel == WaterLevel::Head)
		|| (pev->waterlevel == WaterLevel::Head && pEntity->pev->waterlevel == WaterLevel::Dry))
		return false;

	const Vector vecLookerOrigin = GetAbsOrigin() + pev->view_ofs;//look through the caller's 'eyes'
	const Vector vecTargetOrigin = pEntity->EyePosition();

	TraceResult tr;
	UTIL_TraceLine(vecLookerOrigin, vecTargetOrigin, IgnoreMonsters::Yes, IgnoreGlass::Yes, this, &tr);

	if (tr.flFraction != 1.0)
	{
		return false;// Line of sight is not established
	}
	else
	{
		return true;// line of sight is valid.
	}
}

bool CBaseEntity::IsVisible(const Vector& vecOrigin)
{
	const Vector vecLookerOrigin = EyePosition();//look through the caller's 'eyes'

	TraceResult tr;
	UTIL_TraceLine(vecLookerOrigin, vecOrigin, IgnoreMonsters::Yes, IgnoreGlass::Yes, this, &tr);

	if (tr.flFraction != 1.0)
	{
		return false;// Line of sight is not established
	}
	else
	{
		return true;// line of sight is valid.
	}
}

void CBaseEntity::TraceAttack(const TraceAttackInfo& info)
{
	const Vector vecOrigin = info.GetTraceResult().vecEndPos - info.GetDirection() * 4;

	if (pev->takedamage)
	{
		AddMultiDamage(info.GetAttacker(), this, info.GetDamage(), info.GetDamageTypes());

		const int blood = BloodColor();

		if (blood != DONT_BLEED)
		{
			SpawnBlood(vecOrigin, blood, info.GetDamage());// a little surface blood.
			TraceBleed(info);
		}
	}
}

void CBaseMonster::TraceAttack(const TraceAttackInfo& info)
{
	TraceAttackInfo adjustedInfo = info;

	if (pev->takedamage)
	{
		m_LastHitGroup = adjustedInfo.GetTraceResult().iHitgroup;

		{
			float damage = adjustedInfo.GetDamage();

			switch (adjustedInfo.GetTraceResult().iHitgroup)
			{
			case HITGROUP_GENERIC:
				break;
			case HITGROUP_HEAD:
				damage *= gSkillData.monHead;
				break;
			case HITGROUP_CHEST:
				damage *= gSkillData.monChest;
				break;
			case HITGROUP_STOMACH:
				damage *= gSkillData.monStomach;
				break;
			case HITGROUP_LEFTARM:
			case HITGROUP_RIGHTARM:
				damage *= gSkillData.monArm;
				break;
			case HITGROUP_LEFTLEG:
			case HITGROUP_RIGHTLEG:
				damage *= gSkillData.monLeg;
				break;
			default:
				break;
			}

			adjustedInfo.SetDamage(damage);
		}

		SpawnBlood(adjustedInfo.GetTraceResult().vecEndPos, BloodColor(), adjustedInfo.GetDamage());// a little surface blood.
		TraceBleed(adjustedInfo);
		AddMultiDamage(adjustedInfo.GetAttacker(), this, adjustedInfo.GetDamage(), adjustedInfo.GetDamageTypes());
	}
}

void CBaseEntity::FireBullets(std::uint32_t cShots, const Vector& vecSrc, const Vector& vecDirShooting, const Vector& vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, CBaseEntity* pAttacker)
{
	static int tracerCount = 0;
	TraceResult tr;
	const Vector vecRight = gpGlobals->v_right;
	const Vector vecUp = gpGlobals->v_up;

	if (pAttacker == nullptr)
		pAttacker = this;  // the default attacker is ourselves

	ClearMultiDamage();
	gMultiDamage.type = DMG_BULLET | DMG_NEVERGIB;

	for (std::uint32_t iShot = 1; iShot <= cShots; iShot++)
	{
		// get circular gaussian spread
		float x, y, z;
		do {
			x = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
			y = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
			z = x * x + y * y;
		}
		while (z > 1);

		const Vector vecDir = vecDirShooting +
			x * vecSpread.x * vecRight +
			y * vecSpread.y * vecUp;

		const Vector vecEnd = vecSrc + vecDir * flDistance;
		UTIL_TraceLine(vecSrc, vecEnd, IgnoreMonsters::No, this, &tr);

		bool tracer = false;
		if (iTracerFreq != 0 && (tracerCount++ % iTracerFreq) == 0)
		{
			Vector vecTracerSrc;

			if (IsPlayer())
			{// adjust tracer position for player
				vecTracerSrc = vecSrc + Vector(0, 0, -4) + gpGlobals->v_right * 2 + gpGlobals->v_forward * 16;
			}
			else
			{
				vecTracerSrc = vecSrc;
			}

			if (iTracerFreq != 1)		// guns that always trace also always decal
				tracer = true;
			switch (iBulletType)
			{
			case BULLET_MONSTER_MP5:
			case BULLET_MONSTER_9MM:
			case BULLET_MONSTER_12MM:
			default:
				MESSAGE_BEGIN(MessageDest::PAS, SVC_TEMPENTITY, vecTracerSrc);
				WRITE_BYTE(TE_TRACER);
				WRITE_COORD(vecTracerSrc.x);
				WRITE_COORD(vecTracerSrc.y);
				WRITE_COORD(vecTracerSrc.z);
				WRITE_COORD(tr.vecEndPos.x);
				WRITE_COORD(tr.vecEndPos.y);
				WRITE_COORD(tr.vecEndPos.z);
				MESSAGE_END();
				break;
			}
		}
		// do damage, paint decals
		if (tr.flFraction != 1.0)
		{
			CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

			if (iDamage)
			{
				pEntity->TraceAttack({pAttacker, static_cast<float>(iDamage), vecDir, tr, DMG_BULLET | ((iDamage > 16) ? DMG_ALWAYSGIB : DMG_NEVERGIB)});

				TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
				DecalGunshot(&tr, iBulletType);
			}
			else switch (iBulletType)
			{
			case BULLET_PLAYER_BUCKSHOT:
				// make distance based!
				pEntity->TraceAttack({pAttacker, gSkillData.plrDmgBuckshot, vecDir, tr, DMG_BULLET});

				TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
				DecalGunshot(&tr, iBulletType);
				break;

			default:
			case BULLET_MONSTER_9MM:
				pEntity->TraceAttack({pAttacker, gSkillData.monDmg9MM, vecDir, tr, DMG_BULLET});

				TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
				DecalGunshot(&tr, iBulletType);

				break;

			case BULLET_MONSTER_MP5:
				pEntity->TraceAttack({pAttacker, gSkillData.monDmgMP5, vecDir, tr, DMG_BULLET});

				TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
				DecalGunshot(&tr, iBulletType);

				break;

			case BULLET_MONSTER_12MM:
				pEntity->TraceAttack({pAttacker, gSkillData.monDmg12MM, vecDir, tr, DMG_BULLET});
				if (!tracer)
				{
					TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
					DecalGunshot(&tr, iBulletType);
				}
				break;

			case BULLET_NONE: // FIX 
				pEntity->TraceAttack({pAttacker, 50, vecDir, tr, DMG_CLUB});
				TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
				// only decal glass
				if (auto hit = InstanceOrNull(tr.pHit); !IsNullEnt(hit) && hit->GetRenderMode() != RenderMode::Normal)
				{
					UTIL_DecalTrace(&tr, DECAL_GLASSBREAK1 + RANDOM_LONG(0, 2));
				}

				break;
			}
		}
		// make bullet trails
		UTIL_BubbleTrail(vecSrc, tr.vecEndPos, (flDistance * tr.flFraction) / 64.0f);
	}
	ApplyMultiDamage(this, pAttacker);
}

Vector CBasePlayer::FireBulletsPlayer(std::uint32_t cShots, const Vector& vecSrc, const Vector& vecDirShooting, const Vector& vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage)
{
	static int tracerCount = 0;
	TraceResult tr;
	const Vector vecRight = gpGlobals->v_right;
	const Vector vecUp = gpGlobals->v_up;
	float x = 0, y = 0, z;

	CBasePlayer* pAttacker = this;
	const int shared_rand = random_seed;

	ClearMultiDamage();
	gMultiDamage.type = DMG_BULLET | DMG_NEVERGIB;

	for (std::uint32_t iShot = 1; iShot <= cShots; iShot++)
	{
		//Use player's random seed.
		// get circular gaussian spread
		x = UTIL_SharedRandomFloat(shared_rand + iShot, -0.5, 0.5) + UTIL_SharedRandomFloat(shared_rand + (1 + iShot), -0.5, 0.5);
		y = UTIL_SharedRandomFloat(shared_rand + (2 + iShot), -0.5, 0.5) + UTIL_SharedRandomFloat(shared_rand + (3 + iShot), -0.5, 0.5);
		z = x * x + y * y;

		const Vector vecDir = vecDirShooting +
			x * vecSpread.x * vecRight +
			y * vecSpread.y * vecUp;

		const Vector vecEnd = vecSrc + vecDir * flDistance;
		UTIL_TraceLine(vecSrc, vecEnd, IgnoreMonsters::No, this, &tr);

		// do damage, paint decals
		if (tr.flFraction != 1.0)
		{
			CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

			if (iDamage)
			{
				pEntity->TraceAttack({pAttacker, static_cast<float>(iDamage), vecDir, tr, DMG_BULLET | ((iDamage > 16) ? DMG_ALWAYSGIB : DMG_NEVERGIB)});

				TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
				DecalGunshot(&tr, iBulletType);
			}
			else switch (iBulletType)
			{
			default:
			case BULLET_PLAYER_9MM:
				pEntity->TraceAttack({pAttacker, gSkillData.plrDmg9MM, vecDir, tr, DMG_BULLET});
				break;

			case BULLET_PLAYER_MP5:
				pEntity->TraceAttack({pAttacker, gSkillData.plrDmgMP5, vecDir, tr, DMG_BULLET});
				break;

			case BULLET_PLAYER_BUCKSHOT:
				// make distance based!
				pEntity->TraceAttack({pAttacker, gSkillData.plrDmgBuckshot, vecDir, tr, DMG_BULLET});
				break;

			case BULLET_PLAYER_357:
				pEntity->TraceAttack({pAttacker, gSkillData.plrDmg357, vecDir, tr, DMG_BULLET});
				break;

			case BULLET_NONE: // FIX 
				pEntity->TraceAttack({pAttacker, 50, vecDir, tr, DMG_CLUB});
				TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
				// only decal glass
				if (auto hit = InstanceOrNull(tr.pHit); !IsNullEnt(hit) && hit->GetRenderMode() != RenderMode::Normal)
				{
					UTIL_DecalTrace(&tr, DECAL_GLASSBREAK1 + RANDOM_LONG(0, 2));
				}

				break;
			}
		}
		// make bullet trails
		UTIL_BubbleTrail(vecSrc, tr.vecEndPos, (flDistance * tr.flFraction) / 64.0f);
	}
	ApplyMultiDamage(this, pAttacker);

	return Vector(x * vecSpread.x, y * vecSpread.y, 0.0);
}

void CBaseEntity::TraceBleed(const TraceAttackInfo& info)
{
	if (BloodColor() == DONT_BLEED)
		return;

	if (info.GetDamage() == 0)
		return;

	if (!(info.GetDamageTypes() & (DMG_CRUSH | DMG_BULLET | DMG_SLASH | DMG_BLAST | DMG_CLUB | DMG_MORTAR)))
		return;

	// make blood decal on the wall! 
	float flNoise;
	int cCount;

	/*
		if ( !IsAlive() )
		{
			// dealing with a dead monster.
			if ( pev->max_health <= 0 )
			{
				// no blood decal for a monster that has already decalled its limit.
				return;
			}
			else
			{
				pev->max_health--;
			}
		}
	*/

	if (info.GetDamage() < 10)
	{
		flNoise = 0.1;
		cCount = 1;
	}
	else if (info.GetDamage() < 25)
	{
		flNoise = 0.2;
		cCount = 2;
	}
	else
	{
		flNoise = 0.3;
		cCount = 4;
	}

	TraceResult Bloodtr;
	for (int i = 0; i < cCount; i++)
	{
		Vector vecTraceDir = info.GetDirection() * -1;// trace in the opposite direction the shot came from (the direction the shot is going)

		vecTraceDir.x += RANDOM_FLOAT(-flNoise, flNoise);
		vecTraceDir.y += RANDOM_FLOAT(-flNoise, flNoise);
		vecTraceDir.z += RANDOM_FLOAT(-flNoise, flNoise);

		UTIL_TraceLine(info.GetTraceResult().vecEndPos, info.GetTraceResult().vecEndPos + vecTraceDir * -172, IgnoreMonsters::Yes, this, &Bloodtr);

		if (Bloodtr.flFraction != 1.0)
		{
			UTIL_BloodDecalTrace(&Bloodtr, BloodColor());
		}
	}
}

void CBaseMonster::MakeDamageBloodDecal(int cCount, float flNoise, TraceResult* ptr, const Vector& vecDir)
{
	// make blood decal on the wall! 

	if (!IsAlive())
	{
		// dealing with a dead monster. 
		if (pev->max_health <= 0)
		{
			// no blood decal for a monster that has already decalled its limit.
			return;
		}
		else
		{
			pev->max_health--;
		}
	}

	TraceResult Bloodtr;
	for (int i = 0; i < cCount; i++)
	{
		Vector vecTraceDir = vecDir;

		vecTraceDir.x += RANDOM_FLOAT(-flNoise, flNoise);
		vecTraceDir.y += RANDOM_FLOAT(-flNoise, flNoise);
		vecTraceDir.z += RANDOM_FLOAT(-flNoise, flNoise);

		UTIL_TraceLine(ptr->vecEndPos, ptr->vecEndPos + vecTraceDir * 172, IgnoreMonsters::Yes, this, &Bloodtr);

		/*
				MESSAGE_BEGIN( MessageDest::Broadcast, SVC_TEMPENTITY );
					WRITE_BYTE( TE_SHOWLINE);
					WRITE_COORD( ptr->vecEndPos.x );
					WRITE_COORD( ptr->vecEndPos.y );
					WRITE_COORD( ptr->vecEndPos.z );

					WRITE_COORD( Bloodtr.vecEndPos.x );
					WRITE_COORD( Bloodtr.vecEndPos.y );
					WRITE_COORD( Bloodtr.vecEndPos.z );
				MESSAGE_END();
		*/

		if (Bloodtr.flFraction != 1.0)
		{
			UTIL_BloodDecalTrace(&Bloodtr, BloodColor());
		}
	}
}
