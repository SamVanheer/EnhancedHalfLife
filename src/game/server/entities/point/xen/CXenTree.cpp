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
#include "CXenTree.hpp"
#include "CXenTreeTrigger.hpp"

LINK_ENTITY_TO_CLASS(xen_tree, CXenTree);

TYPEDESCRIPTION	CXenTree::m_SaveData[] =
{
	DEFINE_FIELD(CXenTree, m_hTrigger, FIELD_EHANDLE),
};

IMPLEMENT_SAVERESTORE(CXenTree, CActAnimating);

void CXenTree::OnRemove()
{
	m_hTrigger.Remove();
	CActAnimating::OnRemove();
}

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
	AngleVectors(GetAbsAngles(), &triggerPosition, nullptr, nullptr);
	triggerPosition = GetAbsOrigin() + (triggerPosition * 64);
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

		AngleVectors(GetAbsAngles(), &forward, nullptr, nullptr);

		for (int i = 0; i < count; i++)
		{
			if (pList[i] != this)
			{
				if (pList[i]->GetOwner() != this)
				{
					sound = true;
					pList[i]->TakeDamage({this, this, 25, DMG_CRUSH | DMG_SLASH});
					pList[i]->pev->punchangle.x = 15;
					pList[i]->SetAbsVelocity(pList[i]->GetAbsVelocity() + forward * 100);
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
