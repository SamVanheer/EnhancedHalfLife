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

#include "CXenHull.hpp"
#include "CXenSpore.hpp"

LINK_ENTITY_TO_CLASS(xen_spore_small, CXenSporeSmall);
LINK_ENTITY_TO_CLASS(xen_spore_medium, CXenSporeMed);
LINK_ENTITY_TO_CLASS(xen_spore_large, CXenSporeLarge);

void CXenSpore::Spawn()
{
	Precache();

	SetModel(pModelNames[pev->skin]);
	SetMovetype(Movetype::None);
	SetSolidType(Solid::BBox);
	SetDamageMode(DamageMode::Yes);

	//	SetActivity( ACT_IDLE );
	pev->sequence = 0;
	pev->frame = RANDOM_FLOAT(0, 255);
	pev->framerate = RANDOM_FLOAT(0.7, 1.4);
	ResetSequenceInfo();
	pev->nextthink = gpGlobals->time + RANDOM_FLOAT(0.1, 0.4);	// Load balance these a bit
}

const char* CXenSpore::pModelNames[] =
{
	"models/fungus(small).mdl",
	"models/fungus.mdl",
	"models/fungus(large).mdl",
};

void CXenSpore::Precache()
{
	PRECACHE_MODEL(pModelNames[pev->skin]);
}

void CXenSpore::Touch(CBaseEntity* pOther)
{
}

void CXenSpore::Think()
{
	float flInterval = StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1;

#if 0
	DispatchAnimEvents(flInterval);

	switch (GetActivity())
	{
	default:
	case ACT_IDLE:
		break;

	}
#endif
}

void CXenSporeSmall::Spawn()
{
	pev->skin = 0;
	CXenSpore::Spawn();
	SetSize(Vector(-16, -16, 0), Vector(16, 16, 64));
}

void CXenSporeMed::Spawn()
{
	pev->skin = 1;
	CXenSpore::Spawn();
	SetSize(Vector(-40, -40, 0), Vector(40, 40, 120));
}

/**
*	@brief I just eyeballed these -- fill in hulls for the legs
*/
const Vector CXenSporeLarge::m_hullSizes[] =
{
	Vector(90, -25, 0),
	Vector(25, 75, 0),
	Vector(-15, -100, 0),
	Vector(-90, -35, 0),
	Vector(-90, 60, 0),
};

void CXenSporeLarge::Spawn()
{
	pev->skin = 2;
	CXenSpore::Spawn();
	SetSize(Vector(-48, -48, 110), Vector(48, 48, 240));

	Vector forward, right;

	AngleVectors(GetAbsAngles(), &forward, &right, nullptr);

	// Rotate the leg hulls into position
	for (std::size_t i = 0; i < ArraySize(m_hullSizes); i++)
		CXenHull::CreateHull(this, Vector(-12, -12, 0), Vector(12, 12, 120), (m_hullSizes[i].x * forward) + (m_hullSizes[i].y * right));
}
