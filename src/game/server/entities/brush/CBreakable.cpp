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

#include "CBreakable.hpp"
#include "effects/CEnvExplosion.hpp"

extern DLL_GLOBAL Vector		g_vecAttackDir;

/**
*	@brief Just add more items to the bottom of this array and they will automagically be supported
*	This is done instead of just a classname in the FGD so we can control which entities can be spawned,
*	and still remain fairly flexible
*/
const char* CBreakable::pSpawnObjects[] =
{
	nullptr,			// 0
	"item_battery",		// 1
	"item_healthkit",	// 2
	"weapon_9mmhandgun",// 3
	"ammo_9mmclip",		// 4
	"weapon_9mmAR",		// 5
	"ammo_9mmAR",		// 6
	"ammo_ARgrenades",	// 7
	"weapon_shotgun",	// 8
	"ammo_buckshot",	// 9
	"weapon_crossbow",	// 10
	"ammo_crossbow",	// 11
	"weapon_357",		// 12
	"ammo_357",			// 13
	"weapon_rpg",		// 14
	"ammo_rpgclip",		// 15
	"ammo_gaussclip",	// 16
	"weapon_handgrenade",// 17
	"weapon_tripmine",	// 18
	"weapon_satchel",	// 19
	"weapon_snark",		// 20
	"weapon_hornetgun",	// 21
};

void CBreakable::KeyValue(KeyValueData* pkvd)
{
	// UNDONE_WC: explicitly ignoring these fields, but they shouldn't be in the map file!
	if (AreStringsEqual(pkvd->szKeyName, "explosion"))
	{
		if (!stricmp(pkvd->szValue, "1"))
		{
			m_Explosion = Explosions::Directed;
		}
		else
		{
			m_Explosion = Explosions::Random;
		}

		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "material"))
	{
		const int i = atoi(pkvd->szValue);

		// 0:glass, 1:metal, 2:flesh, 3:wood

		if ((i < 0) || (i >= static_cast<int>(Materials::LastMaterial)))
			m_Material = Materials::Wood;
		else
			m_Material = (Materials)i;

		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "deadmodel"))
	{
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "shards"))
	{
		//			m_iShards = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "gibmodel"))
	{
		m_iszGibModel = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "spawnobject"))
	{
		const std::size_t object = atoi(pkvd->szValue);
		if (object > 0 && object < ArraySize(pSpawnObjects))
			m_iszSpawnObject = MAKE_STRING(pSpawnObjects[object]);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "explodemagnitude"))
	{
		ExplosionSetMagnitude(atoi(pkvd->szValue));
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "lip"))
		pkvd->fHandled = true;
	else
		BaseClass::KeyValue(pkvd);
}

void CBreakable::Spawn()
{
	Precache();

	if (IsBitSet(pev->spawnflags, SF_BREAK_TRIGGER_ONLY))
		SetDamageMode(DamageMode::No);
	else
		SetDamageMode(DamageMode::Yes);

	SetSolidType(Solid::BSP);
	SetMovetype(Movetype::Push);

	Vector angles = GetAbsAngles();
	m_angle = angles.y;
	angles.y = 0;
	SetAbsAngles(angles);

	// HACK:  matGlass can receive decals, we need the client to know about this
	//  so use class to store the material flag
	if (m_Material == Materials::Glass)
	{
		pev->playerclass = 1;
	}

	SetModel(STRING(pev->model));//set size and link into world.

	SetTouch(&CBreakable::BreakTouch);
	if (IsBitSet(pev->spawnflags, SF_BREAK_TRIGGER_ONLY))		// Only break on trigger
		SetTouch(nullptr);

	// Flag unbreakable glass as "worldbrush" so it will block ALL tracelines
	if (!IsBreakable() && GetRenderMode() != RenderMode::Normal)
		pev->flags |= FL_WORLDBRUSH;
}

const char* CBreakable::pSoundsWood[] =
{
	"debris/wood1.wav",
	"debris/wood2.wav",
	"debris/wood3.wav",
};

const char* CBreakable::pSoundsFlesh[] =
{
	"debris/flesh1.wav",
	"debris/flesh2.wav",
	"debris/flesh3.wav",
	"debris/flesh5.wav",
	"debris/flesh6.wav",
	"debris/flesh7.wav",
};

const char* CBreakable::pSoundsMetal[] =
{
	"debris/metal1.wav",
	"debris/metal2.wav",
	"debris/metal3.wav",
};

const char* CBreakable::pSoundsConcrete[] =
{
	"debris/concrete1.wav",
	"debris/concrete2.wav",
	"debris/concrete3.wav",
};


const char* CBreakable::pSoundsGlass[] =
{
	"debris/glass1.wav",
	"debris/glass2.wav",
	"debris/glass3.wav",
};

const char** CBreakable::MaterialSoundList(Materials precacheMaterial, int& soundCount)
{
	const char** pSoundList = nullptr;

	switch (precacheMaterial)
	{
	case Materials::Wood:
		pSoundList = pSoundsWood;
		soundCount = ArraySize(pSoundsWood);
		break;
	case Materials::Flesh:
		pSoundList = pSoundsFlesh;
		soundCount = ArraySize(pSoundsFlesh);
		break;
	case Materials::Computer:
	case Materials::UnbreakableGlass:
	case Materials::Glass:
		pSoundList = pSoundsGlass;
		soundCount = ArraySize(pSoundsGlass);
		break;

	case Materials::Metal:
		pSoundList = pSoundsMetal;
		soundCount = ArraySize(pSoundsMetal);
		break;

	case Materials::CinderBlock:
	case Materials::Rocks:
		pSoundList = pSoundsConcrete;
		soundCount = ArraySize(pSoundsConcrete);
		break;

	case Materials::CeilingTile:
	case Materials::None:
	default:
		soundCount = 0;
		break;
	}

	return pSoundList;
}

void CBreakable::MaterialSoundPrecache(Materials precacheMaterial)
{
	int soundCount = 0;

	const char** pSoundList = MaterialSoundList(precacheMaterial, soundCount);

	for (int i = 0; i < soundCount; i++)
	{
		PRECACHE_SOUND(pSoundList[i]);
	}
}

void CBreakable::MaterialSoundRandom(CBaseEntity* entity, Materials soundMaterial, float volume)
{
	int soundCount = 0;

	const char** pSoundList = MaterialSoundList(soundMaterial, soundCount);

	if (soundCount)
		entity->EmitSound(SoundChannel::Body, pSoundList[RANDOM_LONG(0, soundCount - 1)], volume, 1.0);
}

void CBreakable::Precache()
{
	const char* pGibName = nullptr;

	switch (m_Material)
	{
	case Materials::Wood:
		pGibName = "models/woodgibs.mdl";

		PRECACHE_SOUND("debris/bustcrate1.wav");
		PRECACHE_SOUND("debris/bustcrate2.wav");
		break;
	case Materials::Flesh:
		pGibName = "models/fleshgibs.mdl";

		PRECACHE_SOUND("debris/bustflesh1.wav");
		PRECACHE_SOUND("debris/bustflesh2.wav");
		break;
	case Materials::Computer:
		PRECACHE_SOUND("buttons/spark5.wav");
		PRECACHE_SOUND("buttons/spark6.wav");
		pGibName = "models/computergibs.mdl";

		PRECACHE_SOUND("debris/bustmetal1.wav");
		PRECACHE_SOUND("debris/bustmetal2.wav");
		break;

	case Materials::UnbreakableGlass:
	case Materials::Glass:
		pGibName = "models/glassgibs.mdl";

		PRECACHE_SOUND("debris/bustglass1.wav");
		PRECACHE_SOUND("debris/bustglass2.wav");
		break;
	case Materials::Metal:
		pGibName = "models/metalplategibs.mdl";

		PRECACHE_SOUND("debris/bustmetal1.wav");
		PRECACHE_SOUND("debris/bustmetal2.wav");
		break;
	case Materials::CinderBlock:
		pGibName = "models/cindergibs.mdl";

		PRECACHE_SOUND("debris/bustconcrete1.wav");
		PRECACHE_SOUND("debris/bustconcrete2.wav");
		break;
	case Materials::Rocks:
		pGibName = "models/rockgibs.mdl";

		PRECACHE_SOUND("debris/bustconcrete1.wav");
		PRECACHE_SOUND("debris/bustconcrete2.wav");
		break;
	case Materials::CeilingTile:
		pGibName = "models/ceilinggibs.mdl";

		PRECACHE_SOUND("debris/bustceiling.wav");
		break;
	}
	MaterialSoundPrecache(m_Material);
	if (!IsStringNull(m_iszGibModel))
		pGibName = STRING(m_iszGibModel);

	if (pGibName)
	{
		m_idShard = PRECACHE_MODEL(pGibName);
	}

	// Precache the spawn item's data
	if (!IsStringNull(m_iszSpawnObject))
		UTIL_PrecacheOther(STRING(m_iszSpawnObject));
}

void CBreakable::DamageSound()
{
	Materials material = m_Material;
	if (material == Materials::Computer && RANDOM_LONG(0, 1))
		material = Materials::Metal;

	const char* rgpsz[6]{};
	int i = 0;

	switch (material)
	{
	case Materials::Computer:
	case Materials::Glass:
	case Materials::UnbreakableGlass:
		rgpsz[0] = "debris/glass1.wav";
		rgpsz[1] = "debris/glass2.wav";
		rgpsz[2] = "debris/glass3.wav";
		i = 3;
		break;

	case Materials::Wood:
		rgpsz[0] = "debris/wood1.wav";
		rgpsz[1] = "debris/wood2.wav";
		rgpsz[2] = "debris/wood3.wav";
		i = 3;
		break;

	case Materials::Metal:
		rgpsz[0] = "debris/metal1.wav";
		rgpsz[1] = "debris/metal3.wav";
		rgpsz[2] = "debris/metal2.wav";
		i = 2;
		break;

	case Materials::Flesh:
		rgpsz[0] = "debris/flesh1.wav";
		rgpsz[1] = "debris/flesh2.wav";
		rgpsz[2] = "debris/flesh3.wav";
		rgpsz[3] = "debris/flesh5.wav";
		rgpsz[4] = "debris/flesh6.wav";
		rgpsz[5] = "debris/flesh7.wav";
		i = 6;
		break;

	case Materials::Rocks:
	case Materials::CinderBlock:
		rgpsz[0] = "debris/concrete1.wav";
		rgpsz[1] = "debris/concrete2.wav";
		rgpsz[2] = "debris/concrete3.wav";
		i = 3;
		break;

	case Materials::CeilingTile:
		// UNDONE: no ceiling tile shard sound yet
		i = 0;
		break;
	}

	if (i)
	{
		int pitch;
		if (RANDOM_LONG(0, 2))
			pitch = PITCH_NORM;
		else
			pitch = 95 + RANDOM_LONG(0, 34);

		const float fvol = RANDOM_FLOAT(0.75, 1.0);

		EmitSound(SoundChannel::Voice, rgpsz[RANDOM_LONG(0, i - 1)], fvol, ATTN_NORM, pitch);
	}
}

void CBreakable::BreakTouch(CBaseEntity* pOther)
{
	// only players can break these right now
	if (!pOther->IsPlayer() || !IsBreakable())
	{
		return;
	}

	if (IsBitSet(pev->spawnflags, SF_BREAK_TOUCH))
	{// can be broken when run into 
		const float flDamage = pOther->GetAbsVelocity().Length() * 0.01;

		if (flDamage >= pev->health)
		{
			SetTouch(nullptr);
			TakeDamage({pOther, pOther, flDamage, DMG_CRUSH});

			// do a little damage to player if we broke glass or computer
			pOther->TakeDamage({this, this, flDamage / 4, DMG_SLASH});
		}
	}

	if (IsBitSet(pev->spawnflags, SF_BREAK_PRESSURE) && pOther->pev->absmin.z >= pev->maxs.z - 2)
	{// can be broken when stood upon

		// play creaking sound here.
		DamageSound();

		SetThink(&CBreakable::Die);
		SetTouch(nullptr);

		if (m_flDelay == 0)
		{// !!!BUGBUG - why doesn't zero delay work?
			m_flDelay = 0.1;
		}

		pev->nextthink = pev->ltime + m_flDelay;
	}
}

void CBreakable::Use(const UseInfo& info)
{
	if (IsBreakable())
	{
		Vector angles = GetAbsAngles();
		angles.y = m_angle;
		SetAbsAngles(angles);
		UTIL_MakeVectors(angles);
		g_vecAttackDir = gpGlobals->v_forward;

		Die();
	}
}

void CBreakable::TraceAttack(const TraceAttackInfo& info)
{
	// random spark if this is a 'computer' object
	if (RANDOM_LONG(0, 1))
	{
		switch (m_Material)
		{
		case Materials::Computer:
		{
			UTIL_Sparks(info.GetTraceResult().vecEndPos);

			const float flVolume = RANDOM_FLOAT(0.7, 1.0);//random volume range
			switch (RANDOM_LONG(0, 1))
			{
			case 0: EmitSound(SoundChannel::Voice, "buttons/spark5.wav", flVolume); break;
			case 1: EmitSound(SoundChannel::Voice, "buttons/spark6.wav", flVolume); break;
			}
		}
		break;

		case Materials::UnbreakableGlass:
			UTIL_Ricochet(info.GetTraceResult().vecEndPos, RANDOM_FLOAT(0.5, 1.5));
			break;
		}
	}

	BaseClass::TraceAttack(info);
}

bool CBreakable::TakeDamage(const TakeDamageInfo& info)
{
	if (!IsBreakable())
		return false;

	TakeDamageInfo adjustedInfo = info;

	Vector vecTemp;

	// if Attacker == Inflictor, the attack was a melee or other instant-hit attack.
	// (that is, no actual entity projectile was involved in the attack so use the shooter's origin). 
	if (adjustedInfo.GetAttacker() == adjustedInfo.GetInflictor())
	{
		vecTemp = adjustedInfo.GetInflictor()->GetAbsOrigin() - (pev->absmin + (pev->size * 0.5));

		// if a client hit the breakable with a crowbar, and breakable is crowbar-sensitive, break it now.
		if (IsBitSet(adjustedInfo.GetAttacker()->pev->flags, FL_CLIENT) &&
			IsBitSet(pev->spawnflags, SF_BREAK_CROWBAR) && (adjustedInfo.GetDamageTypes() & DMG_CLUB))
			adjustedInfo.SetDamage(pev->health);
	}
	else
		// an actual missile was involved.
	{
		vecTemp = adjustedInfo.GetInflictor()->GetAbsOrigin() - (pev->absmin + (pev->size * 0.5));
	}

	// Breakables take double damage from the crowbar
	if (adjustedInfo.GetDamageTypes() & DMG_CLUB)
		adjustedInfo.SetDamage(adjustedInfo.GetDamage() * 2);

	// Boxes / glass / etc. don't take much poison damage, just the impact of the dart - consider that 10%
	if (adjustedInfo.GetDamageTypes() & DMG_POISON)
		adjustedInfo.SetDamage(adjustedInfo.GetDamage() * 0.1);

	// this global is still used for glass and other non-monster killables, along with decals.
	g_vecAttackDir = vecTemp.Normalize();

	// do the damage
	pev->health -= adjustedInfo.GetDamage();
	if (pev->health <= 0)
	{
		Killed({adjustedInfo.GetInflictor(), adjustedInfo.GetAttacker(), GibType::Normal});
		Die();
		return false;
	}

	// Make a shard noise each time func breakable is hit.
	// Don't play shard noise if cbreakable actually died.

	DamageSound();

	return true;
}

void CBreakable::Die()
{
	int pitch = 95 + RANDOM_LONG(0, 29);

	if (pitch > 97 && pitch < 103)
		pitch = 100;

	// The more negative pev->health, the louder
	// the sound should be.

	float fvol = RANDOM_FLOAT(0.85, 1.0) + (fabs(pev->health) / 100.0);

	if (fvol > 1.0)
		fvol = 1.0;

	char cFlag = 0;

	switch (m_Material)
	{
	case Materials::Glass:
		switch (RANDOM_LONG(0, 1))
		{
		case 0:	EmitSound(SoundChannel::Voice, "debris/bustglass1.wav", fvol, ATTN_NORM, pitch);
			break;
		case 1:	EmitSound(SoundChannel::Voice, "debris/bustglass2.wav", fvol, ATTN_NORM, pitch);
			break;
		}
		cFlag = BREAK_GLASS;
		break;

	case Materials::Wood:
		switch (RANDOM_LONG(0, 1))
		{
		case 0:	EmitSound(SoundChannel::Voice, "debris/bustcrate1.wav", fvol, ATTN_NORM, pitch);
			break;
		case 1:	EmitSound(SoundChannel::Voice, "debris/bustcrate2.wav", fvol, ATTN_NORM, pitch);
			break;
		}
		cFlag = BREAK_WOOD;
		break;

	case Materials::Computer:
	case Materials::Metal:
		switch (RANDOM_LONG(0, 1))
		{
		case 0:	EmitSound(SoundChannel::Voice, "debris/bustmetal1.wav", fvol, ATTN_NORM, pitch);
			break;
		case 1:	EmitSound(SoundChannel::Voice, "debris/bustmetal2.wav", fvol, ATTN_NORM, pitch);
			break;
		}
		cFlag = BREAK_METAL;
		break;

	case Materials::Flesh:
		switch (RANDOM_LONG(0, 1))
		{
		case 0:	EmitSound(SoundChannel::Voice, "debris/bustflesh1.wav", fvol, ATTN_NORM, pitch);
			break;
		case 1:	EmitSound(SoundChannel::Voice, "debris/bustflesh2.wav", fvol, ATTN_NORM, pitch);
			break;
		}
		cFlag = BREAK_FLESH;
		break;

	case Materials::Rocks:
	case Materials::CinderBlock:
		switch (RANDOM_LONG(0, 1))
		{
		case 0:	EmitSound(SoundChannel::Voice, "debris/bustconcrete1.wav", fvol, ATTN_NORM, pitch);
			break;
		case 1:	EmitSound(SoundChannel::Voice, "debris/bustconcrete2.wav", fvol, ATTN_NORM, pitch);
			break;
		}
		cFlag = BREAK_CONCRETE;
		break;

	case Materials::CeilingTile:
		EmitSound(SoundChannel::Voice, "debris/bustceiling.wav", fvol, ATTN_NORM, pitch);
		break;
	}

	Vector vecVelocity;// shard velocity
	if (m_Explosion == Explosions::Directed)
		vecVelocity = -g_vecAttackDir * 200;
	else
	{
		vecVelocity.x = 0;
		vecVelocity.y = 0;
		vecVelocity.z = 0;
	}

	// shard origin
	const Vector vecSpot = GetAbsOrigin() + (pev->mins + pev->maxs) * 0.5;
	MESSAGE_BEGIN(MessageDest::PVS, SVC_TEMPENTITY, vecSpot);
	WRITE_BYTE(TE_BREAKMODEL);

	// position
	WRITE_COORD(vecSpot.x);
	WRITE_COORD(vecSpot.y);
	WRITE_COORD(vecSpot.z);

	// size
	WRITE_COORD(pev->size.x);
	WRITE_COORD(pev->size.y);
	WRITE_COORD(pev->size.z);

	// velocity
	WRITE_COORD(vecVelocity.x);
	WRITE_COORD(vecVelocity.y);
	WRITE_COORD(vecVelocity.z);

	// randomization
	WRITE_BYTE(10);

	// Model
	WRITE_SHORT(m_idShard);	//model id#

	// # of shards
	WRITE_BYTE(0);	// let client decide

	// duration
	WRITE_BYTE(25);// 2.5 seconds

	// flags
	WRITE_BYTE(cFlag);
	MESSAGE_END();

	float size = pev->size.x;
	if (size < pev->size.y)
		size = pev->size.y;
	if (size < pev->size.z)
		size = pev->size.z;

	// !!! HACK  This should work!
	// Build a box above the entity that looks like an 8 pixel high sheet
	Vector mins = pev->absmin;
	Vector maxs = pev->absmax;
	mins.z = pev->absmax.z;
	maxs.z += 8;

	// BUGBUG -- can only find 256 entities on a breakable -- should be enough
	CBaseEntity* pList[256];
	int count = UTIL_EntitiesInBox(pList, ArraySize(pList), mins, maxs, FL_ONGROUND);
	if (count)
	{
		for (int i = 0; i < count; i++)
		{
			ClearBits(pList[i]->pev->flags, FL_ONGROUND);
			pList[i]->pev->groundentity = nullptr;
		}
	}

	// Don't fire something that could fire myself
	ClearTargetname();

	SetSolidType(Solid::Not);
	// Fire targets on break
	SUB_UseTargets(nullptr, UseType::Toggle, 0);

	SetThink(&CBreakable::SUB_Remove);
	pev->nextthink = pev->ltime + 0.1;
	if (!IsStringNull(m_iszSpawnObject))
		CBaseEntity::Create(STRING(m_iszSpawnObject), GetBrushModelOrigin(this), GetAbsAngles(), this);

	if (Explodable())
	{
		UTIL_CreateExplosion(Center(), GetAbsAngles(), this, ExplosionMagnitude(), true);
	}
}

bool CBreakable::IsBreakable()
{
	return m_Material != Materials::UnbreakableGlass;
}

int	CBreakable::DamageDecal(int bitsDamageType)
{
	if (m_Material == Materials::Glass)
		return DECAL_GLASSBREAK1 + RANDOM_LONG(0, 2);

	if (m_Material == Materials::UnbreakableGlass)
		return DECAL_BPROOF1;

	return CBaseEntity::DamageDecal(bitsDamageType);
}
