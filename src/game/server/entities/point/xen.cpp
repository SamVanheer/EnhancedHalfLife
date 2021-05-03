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

#include "animation.h"

constexpr std::string_view XEN_PLANT_GLOW_SPRITE{"sprites/flare3.spr"};
constexpr int XEN_PLANT_HIDE_TIME = 5;

class CActAnimating : public CBaseAnimating
{
public:
	void			SetActivity(Activity act);
	inline Activity	GetActivity() { return m_Activity; }

	int	ObjectCaps() override { return CBaseAnimating::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

private:
	Activity	m_Activity;
};

TYPEDESCRIPTION	CActAnimating::m_SaveData[] =
{
	DEFINE_FIELD(CActAnimating, m_Activity, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CActAnimating, CBaseAnimating);

void CActAnimating::SetActivity(Activity act)
{
	int sequence = LookupActivity(act);
	if (sequence != ACTIVITY_NOT_AVAILABLE)
	{
		pev->sequence = sequence;
		m_Activity = act;
		pev->frame = 0;
		ResetSequenceInfo();
	}
}

class CXenPLight : public CActAnimating
{
public:
	void		Spawn() override;
	void		Precache() override;
	void		Touch(CBaseEntity* pOther) override;
	void		Think() override;

	void		LightOn();
	void		LightOff();

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

private:
	EHandle<CSprite> m_hGlow;
};

LINK_ENTITY_TO_CLASS(xen_plantlight, CXenPLight);

TYPEDESCRIPTION	CXenPLight::m_SaveData[] =
{
	DEFINE_FIELD(CXenPLight, m_hGlow, FIELD_EHANDLE),
};

IMPLEMENT_SAVERESTORE(CXenPLight, CActAnimating);

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

	auto glow = m_hGlow = CSprite::SpriteCreate(XEN_PLANT_GLOW_SPRITE.data(), pev->origin + Vector(0, 0, (pev->mins.z + pev->maxs.z) * 0.5), false);
	glow->SetTransparency(RenderMode::Glow, pev->rendercolor.x, pev->rendercolor.y, pev->rendercolor.z, pev->renderamt, pev->renderfx);
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

class CXenHair : public CActAnimating
{
public:
	void		Spawn() override;
	void		Precache() override;
	void		Think() override;
};

LINK_ENTITY_TO_CLASS(xen_hair, CXenHair);

constexpr int SF_HAIR_SYNC = 0x0001;

void CXenHair::Spawn()
{
	Precache();
	SetModel("models/hair.mdl");
	SetSize(Vector(-4, -4, 0), Vector(4, 4, 32));
	pev->sequence = 0;

	if (!(pev->spawnflags & SF_HAIR_SYNC))
	{
		pev->frame = RANDOM_FLOAT(0, 255);
		pev->framerate = RANDOM_FLOAT(0.7, 1.4);
	}
	ResetSequenceInfo();

	SetSolidType(Solid::Not);
	SetMovetype(Movetype::None);
	pev->nextthink = gpGlobals->time + RANDOM_FLOAT(0.1, 0.4);	// Load balance these a bit
}

void CXenHair::Think()
{
	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.5;
}

void CXenHair::Precache()
{
	PRECACHE_MODEL("models/hair.mdl");
}

class CXenTreeTrigger : public CBaseEntity
{
public:
	void		Touch(CBaseEntity* pOther) override;
	static CXenTreeTrigger* TriggerCreate(CBaseEntity* pOwner, const Vector& position);
};

LINK_ENTITY_TO_CLASS(xen_ttrigger, CXenTreeTrigger);

CXenTreeTrigger* CXenTreeTrigger::TriggerCreate(CBaseEntity* pOwner, const Vector& position)
{
	CXenTreeTrigger* pTrigger = GetClassPtr((CXenTreeTrigger*)nullptr);
	pTrigger->pev->origin = position;
	pTrigger->SetClassname("xen_ttrigger");
	pTrigger->SetSolidType(Solid::Trigger);
	pTrigger->SetMovetype(Movetype::None);
	pTrigger->SetOwner(pOwner);

	return pTrigger;
}

void CXenTreeTrigger::Touch(CBaseEntity* pOther)
{
	if (auto entity = GetOwner(); entity)
	{
		entity->Touch(pOther);
	}
}

constexpr int TREE_AE_ATTACK = 1;

class CXenTree : public CActAnimating
{
public:
	void		Spawn() override;
	void		Precache() override;
	void		Touch(CBaseEntity* pOther) override;
	void		Think() override;
	bool TakeDamage(const TakeDamageInfo& info) override { Attack(); return false; }
	void		HandleAnimEvent(AnimationEvent& event) override;
	void		Attack();
	int			Classify() override { return CLASS_BARNACLE; }

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	static const char* pAttackHitSounds[];
	static const char* pAttackMissSounds[];

private:
	EHandle<CXenTreeTrigger> m_hTrigger;
};

LINK_ENTITY_TO_CLASS(xen_tree, CXenTree);

TYPEDESCRIPTION	CXenTree::m_SaveData[] =
{
	DEFINE_FIELD(CXenTree, m_hTrigger, FIELD_EHANDLE),
};

IMPLEMENT_SAVERESTORE(CXenTree, CActAnimating);

void CXenTree::Spawn()
{
	Precache();

	SetModel("models/tree.mdl");
	SetMovetype(Movetype::None);
	SetSolidType(Solid::BBox);

	SetDamageMode(DamageMode::Yes);

	SetSize(Vector(-30, -30, 0), Vector(30, 30, 188));
	SetActivity(ACT_IDLE);
	pev->nextthink = gpGlobals->time + 0.1;
	pev->frame = RANDOM_FLOAT(0, 255);
	pev->framerate = RANDOM_FLOAT(0.7, 1.4);

	Vector triggerPosition;
	AngleVectors(pev->angles, &triggerPosition, nullptr, nullptr);
	triggerPosition = pev->origin + (triggerPosition * 64);
	// Create the trigger
	auto trigger = m_hTrigger = CXenTreeTrigger::TriggerCreate(this, triggerPosition);
	trigger->SetSize(Vector(-24, -24, 0), Vector(24, 24, 128));
}

const char* CXenTree::pAttackHitSounds[] =
{
	"zombie/claw_strike1.wav",
	"zombie/claw_strike2.wav",
	"zombie/claw_strike3.wav",
};

const char* CXenTree::pAttackMissSounds[] =
{
	"zombie/claw_miss1.wav",
	"zombie/claw_miss2.wav",
};

void CXenTree::Precache()
{
	PRECACHE_MODEL("models/tree.mdl");
	PRECACHE_MODEL(XEN_PLANT_GLOW_SPRITE.data());
	PRECACHE_SOUND_ARRAY(pAttackHitSounds);
	PRECACHE_SOUND_ARRAY(pAttackMissSounds);
}

void CXenTree::Touch(CBaseEntity* pOther)
{
	if (!pOther->IsPlayer() && pOther->ClassnameIs("monster_bigmomma"))
		return;

	Attack();
}

void CXenTree::Attack()
{
	if (GetActivity() == ACT_IDLE)
	{
		SetActivity(ACT_MELEE_ATTACK1);
		pev->framerate = RANDOM_FLOAT(1.0, 1.4);
		EMIT_SOUND_ARRAY_DYN(SoundChannel::Weapon, pAttackMissSounds);
	}
}

void CXenTree::HandleAnimEvent(AnimationEvent& event)
{
	switch (event.event)
	{
	case TREE_AE_ATTACK:
	{
		CBaseEntity* pList[8];
		bool sound = false;
		int count = UTIL_EntitiesInBox(pList, ArraySize(pList), m_hTrigger->pev->absmin, m_hTrigger->pev->absmax, FL_MONSTER | FL_CLIENT);
		Vector forward;

		AngleVectors(pev->angles, &forward, nullptr, nullptr);

		for (int i = 0; i < count; i++)
		{
			if (pList[i] != this)
			{
				if (pList[i]->GetOwner() != this)
				{
					sound = true;
					pList[i]->TakeDamage({this, this, 25, DMG_CRUSH | DMG_SLASH});
					pList[i]->pev->punchangle.x = 15;
					pList[i]->pev->velocity = pList[i]->pev->velocity + forward * 100;
				}
			}
		}

		if (sound)
		{
			EMIT_SOUND_ARRAY_DYN(SoundChannel::Weapon, pAttackHitSounds);
		}
	}
	return;
	}

	CActAnimating::HandleAnimEvent(event);
}

void CXenTree::Think()
{
	float flInterval = StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1;
	DispatchAnimEvents(flInterval);

	switch (GetActivity())
	{
	case ACT_MELEE_ATTACK1:
		if (m_fSequenceFinished)
		{
			SetActivity(ACT_IDLE);
			pev->framerate = RANDOM_FLOAT(0.6, 1.4);
		}
		break;

	default:
	case ACT_IDLE:
		break;

	}
}

// UNDONE:	These need to smoke somehow when they take damage
//			Touch behavior?
//			Cause damage in smoke area
class CXenSpore : public CActAnimating
{
public:
	void		Spawn() override;
	void		Precache() override;
	void		Touch(CBaseEntity* pOther) override;
	void		Think() override;
	bool TakeDamage(const TakeDamageInfo& info) override { Attack(); return false; }
	//	void HandleAnimEvent(AnimationEvent& event);
	void		Attack() {}

	static const char* pModelNames[];
};

class CXenSporeSmall : public CXenSpore
{
	void		Spawn() override;
};

class CXenSporeMed : public CXenSpore
{
	void		Spawn() override;
};

class CXenSporeLarge : public CXenSpore
{
	void		Spawn() override;

	static const Vector m_hullSizes[];
};

/**
*	@brief Fake collision box for big spores
*/
class CXenHull : public CPointEntity
{
public:
	static CXenHull* CreateHull(CBaseEntity* source, const Vector& mins, const Vector& maxs, const Vector& offset);
	int			Classify() override { return CLASS_BARNACLE; }
};

CXenHull* CXenHull::CreateHull(CBaseEntity* source, const Vector& mins, const Vector& maxs, const Vector& offset)
{
	CXenHull* pHull = GetClassPtr((CXenHull*)nullptr);

	pHull->SetAbsOrigin(source->pev->origin + offset);
	pHull->SetModel(STRING(source->pev->model));
	pHull->SetSolidType(Solid::BBox);
	pHull->SetClassname("xen_hull");
	pHull->SetMovetype(Movetype::None);
	pHull->SetOwner(source);
	pHull->SetSize(mins, maxs);
	pHull->pev->renderamt = 0;
	pHull->pev->rendermode = RenderMode::TransTexture;
	//	pHull->pev->effects = EF_NODRAW;

	return pHull;
}

LINK_ENTITY_TO_CLASS(xen_spore_small, CXenSporeSmall);
LINK_ENTITY_TO_CLASS(xen_spore_medium, CXenSporeMed);
LINK_ENTITY_TO_CLASS(xen_spore_large, CXenSporeLarge);
LINK_ENTITY_TO_CLASS(xen_hull, CXenHull);

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

	AngleVectors(pev->angles, &forward, &right, nullptr);

	// Rotate the leg hulls into position
	for (std::size_t i = 0; i < ArraySize(m_hullSizes); i++)
		CXenHull::CreateHull(this, Vector(-12, -12, 0), Vector(12, 12, 120), (m_hullSizes[i].x * forward) + (m_hullSizes[i].y * right));
}

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
