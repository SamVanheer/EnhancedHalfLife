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

#include "CGib.hpp"

constexpr int GERMAN_GIB_COUNT = 4;
constexpr int HUMAN_GIB_COUNT = 6;
constexpr int ALIEN_GIB_COUNT = 4;

LINK_ENTITY_TO_CLASS(gib, CGib);

CGib* CGib::GibCreate()
{
	return static_cast<CGib*>(g_EntityList.Create("gib"));
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

	SetModel(szGibModel);
	SetSize(vec3_origin, vec3_origin);

	pev->nextthink = gpGlobals->time + 4;
	m_lifeTime = 25;
	SetThink(&CGib::WaitTillLand);
	SetTouch(&CGib::BounceGibTouch);

	m_material = Materials::None;
	m_cBloodDecals = 5;// how many blood decals this gib can place (1 per bounce until none remain). 
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

// HACKHACK -- The gib velocity equations don't work
void CGib::LimitVelocity()
{
	float length = GetAbsVelocity().Length();

	// ceiling at 1500.  The gib velocity equation is not bounded properly.  Rather than tune it
	// in 3 separate places again, I'll just limit it here.
	if (length > 1500.0)
		SetAbsVelocity(GetAbsVelocity().Normalize() * 1500);		// This should really be sv_maxvelocity * 0.75 or something
}

void CGib::SpawnHeadGib(CBaseEntity* pVictim)
{
	CGib* pGib = CGib::GibCreate();

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
		CGib* pGib = CGib::GibCreate();

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

void CGib::SpawnStickyGibs(CBaseEntity* pVictim, const Vector& vecOrigin, int cGibs)
{
	if (g_Language == LANGUAGE_GERMAN)
	{
		// no sticky gibs in germany right now!
		return;
	}

	for (int i = 0; i < cGibs; i++)
	{
		CGib* pGib = CGib::GibCreate();

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
