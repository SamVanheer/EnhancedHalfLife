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

#include "CBigMomma.hpp"
#include "CBMortar.hpp"

LINK_ENTITY_TO_CLASS(bmortar, CBMortar);

// UNDONE: right now this is pretty much a copy of the squid spit with minor changes to the way it does damage
void CBMortar::Spawn()
{
	SetMovetype(Movetype::Toss);

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
	auto pSpit = static_cast<CBMortar*>(g_EntityList.Create("bmortar"));
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
