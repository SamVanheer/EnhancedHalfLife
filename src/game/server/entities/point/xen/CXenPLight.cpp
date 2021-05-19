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

#include "CXenPLight.hpp"

LINK_ENTITY_TO_CLASS(xen_plantlight, CXenPLight);

TYPEDESCRIPTION	CXenPLight::m_SaveData[] =
{
	DEFINE_FIELD(CXenPLight, m_hGlow, FIELD_EHANDLE),
};

IMPLEMENT_SAVERESTORE(CXenPLight, CActAnimating);

void CXenPLight::OnRemove()
{
	m_hGlow.Remove();
}

void CXenPLight::Spawn()
{
	Precache();

	SetModel("models/light.mdl");
	SetMovetype(Movetype::None);
	SetSolidType(Solid::Trigger);

	SetSize(Vector(-80, -80, 0), Vector(80, 80, 32));
	SetActivity(ACT_IDLE);
	pev->nextthink = gpGlobals->time + 0.1;
	pev->frame = RANDOM_FLOAT(0, 255);

	auto glow = m_hGlow = CSprite::SpriteCreate(XEN_PLANT_GLOW_SPRITE.data(), GetAbsOrigin() + Vector(0, 0, (pev->mins.z + pev->maxs.z) * 0.5), false);
	glow->SetTransparency(RenderMode::Glow, GetRenderColor(), GetRenderAmount(), GetRenderFX());
	glow->SetAttachment(this, 1);
}

void CXenPLight::Precache()
{
	PRECACHE_MODEL("models/light.mdl");
	PRECACHE_MODEL(XEN_PLANT_GLOW_SPRITE.data());
}

void CXenPLight::Think()
{
	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1;

	switch (GetActivity())
	{
	case ACT_CROUCH:
		if (m_fSequenceFinished)
		{
			SetActivity(ACT_CROUCHIDLE);
			LightOff();
		}
		break;

	case ACT_CROUCHIDLE:
		if (gpGlobals->time > pev->dmgtime)
		{
			SetActivity(ACT_STAND);
			LightOn();
		}
		break;

	case ACT_STAND:
		if (m_fSequenceFinished)
			SetActivity(ACT_IDLE);
		break;

	case ACT_IDLE:
	default:
		break;
	}
}

void CXenPLight::Touch(CBaseEntity* pOther)
{
	if (pOther->IsPlayer())
	{
		pev->dmgtime = gpGlobals->time + XEN_PLANT_HIDE_TIME;
		if (GetActivity() == ACT_IDLE || GetActivity() == ACT_STAND)
		{
			SetActivity(ACT_CROUCH);
		}
	}
}

void CXenPLight::LightOn()
{
	SUB_UseTargets(this, UseType::On, 0);
	if (auto glow = m_hGlow.Get(); glow)
		glow->pev->effects &= ~EF_NODRAW;
}

void CXenPLight::LightOff()
{
	SUB_UseTargets(this, UseType::Off, 0);
	if (auto glow = m_hGlow.Get(); glow)
		glow->pev->effects |= EF_NODRAW;
}
