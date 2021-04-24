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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "player.h"
#include "gamerules.h"
#include "weaponinfo.h"

class CSatchelCharge : public CGrenade
{
	void Spawn() override;
	void Precache() override;
	void BounceSound() override;

	void EXPORT SatchelSlide(CBaseEntity* pOther);
	void EXPORT SatchelThink();

public:
	/**
	*	@brief do whatever it is we do to an orphaned satchel when we don't want it in the world anymore.
	*/
	void Deactivate();
};
LINK_ENTITY_TO_CLASS(monster_satchel, CSatchelCharge);

void CSatchelCharge::Deactivate()
{
	pev->solid = SOLID_NOT;
	UTIL_Remove(this);
}

void CSatchelCharge::Spawn()
{
	Precache();
	// motor
	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_BBOX;

	SET_MODEL(ENT(pev), "models/w_satchel.mdl");
	//UTIL_SetSize(pev, Vector( -16, -16, -4), Vector(16, 16, 32));	// Old box -- size of headcrab monsters/players get blocked by this
	UTIL_SetSize(pev, Vector(-4, -4, -4), Vector(4, 4, 4));	// Uses point-sized, and can be stepped over
	UTIL_SetOrigin(pev, pev->origin);

	SetTouch(&CSatchelCharge::SatchelSlide);
	SetUse(&CSatchelCharge::DetonateUse);
	SetThink(&CSatchelCharge::SatchelThink);
	pev->nextthink = gpGlobals->time + 0.1;

	pev->gravity = 0.5;
	pev->friction = 0.8;

	pev->dmg = gSkillData.plrDmgSatchel;
	// ResetSequenceInfo( );
	pev->sequence = 1;
}

void CSatchelCharge::SatchelSlide(CBaseEntity* pOther)
{
	// don't hit the guy that launched this grenade
	if (pOther->edict() == pev->owner)
		return;

	// pev->avelocity = Vector (300, 300, 300);
	pev->gravity = 1;// normal gravity now

	// HACKHACK - On ground isn't always set, so look for ground underneath
	TraceResult tr;
	UTIL_TraceLine(pev->origin, pev->origin - Vector(0, 0, 10), IgnoreMonsters::Yes, edict(), &tr);

	if (tr.flFraction < 1.0)
	{
		// add a bit of static friction
		pev->velocity = pev->velocity * 0.95;
		pev->avelocity = pev->avelocity * 0.9;
		// play sliding sound, volume based on velocity
	}
	if (!(pev->flags & FL_ONGROUND) && pev->velocity.Length2D() > 10)
	{
		BounceSound();
	}
	StudioFrameAdvance();
}

void CSatchelCharge::SatchelThink()
{
	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1;

	if (!IsInWorld())
	{
		UTIL_Remove(this);
		return;
	}

	if (pev->waterlevel == WaterLevel::Head)
	{
		pev->movetype = MOVETYPE_FLY;
		pev->velocity = pev->velocity * 0.8;
		pev->avelocity = pev->avelocity * 0.9;
		pev->velocity.z += 8;
	}
	else if (pev->waterlevel == WaterLevel::Dry)
	{
		pev->movetype = MOVETYPE_BOUNCE;
	}
	else
	{
		pev->velocity.z -= 8;
	}
}

void CSatchelCharge::Precache()
{
	PRECACHE_MODEL("models/grenade.mdl");
	PRECACHE_SOUND("weapons/g_bounce1.wav");
	PRECACHE_SOUND("weapons/g_bounce2.wav");
	PRECACHE_SOUND("weapons/g_bounce3.wav");
}

void CSatchelCharge::BounceSound()
{
	switch (RANDOM_LONG(0, 2))
	{
	case 0:	EmitSound(CHAN_VOICE, "weapons/g_bounce1.wav"); break;
	case 1:	EmitSound(CHAN_VOICE, "weapons/g_bounce2.wav"); break;
	case 2:	EmitSound(CHAN_VOICE, "weapons/g_bounce3.wav"); break;
	}
}

LINK_ENTITY_TO_CLASS(weapon_satchel, CSatchel);

bool CSatchel::AddDuplicate(CBasePlayerItem* pOriginal)
{
#ifdef CLIENT_DLL
	if (bIsMultiplayer())
#else
	if (g_pGameRules->IsMultiplayer())
#endif
	{
		CSatchel* pSatchel = (CSatchel*)pOriginal;

		if (pSatchel->m_chargeReady != ChargeState::NoSatchelsDeployed)
		{
			// player has some satchels deployed. Refuse to add more.
			return false;
		}
	}

	return CBasePlayerWeapon::AddDuplicate(pOriginal);
}

bool CSatchel::AddToPlayer(CBasePlayer* pPlayer)
{
	const bool bResult = CBasePlayerItem::AddToPlayer(pPlayer);

	pPlayer->pev->weapons |= (1 << m_iId);
	m_chargeReady = ChargeState::NoSatchelsDeployed;// this satchel charge weapon now forgets that any satchels are deployed by it.

	if (bResult)
	{
		return AddWeapon();
	}
	return false;
}

void CSatchel::Spawn()
{
	Precache();
	m_iId = WEAPON_SATCHEL;
	SET_MODEL(ENT(pev), "models/w_satchel.mdl");

	m_iDefaultAmmo = SATCHEL_DEFAULT_GIVE;

	FallInit();// get ready to fall down.
}

void CSatchel::Precache()
{
	PRECACHE_MODEL("models/v_satchel.mdl");
	PRECACHE_MODEL("models/v_satchel_radio.mdl");
	PRECACHE_MODEL("models/w_satchel.mdl");
	PRECACHE_MODEL("models/p_satchel.mdl");
	PRECACHE_MODEL("models/p_satchel_radio.mdl");

	UTIL_PrecacheOther("monster_satchel");
}

bool CSatchel::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "Satchel Charge";
	p->iMaxAmmo1 = SATCHEL_MAX_CARRY;
	p->pszAmmo2 = nullptr;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 4;
	p->iPosition = 1;
	p->iFlags = ITEM_FLAG_SELECTONEMPTY | ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;
	p->iId = m_iId = WEAPON_SATCHEL;
	p->iWeight = SATCHEL_WEIGHT;

	return true;
}

bool CSatchel::IsUseable()
{
	if (m_pPlayer->m_rgAmmo[PrimaryAmmoIndex()] > 0)
	{
		// player is carrying some satchels
		return true;
	}

	if (m_chargeReady != ChargeState::NoSatchelsDeployed)
	{
		// player isn't carrying any satchels, but has some out
		return true;
	}

	return false;
}

bool CSatchel::CanDeploy()
{
	if (m_pPlayer->m_rgAmmo[PrimaryAmmoIndex()] > 0)
	{
		// player is carrying some satchels
		return true;
	}

	if (m_chargeReady != ChargeState::NoSatchelsDeployed)
	{
		// player isn't carrying any satchels, but has some out
		return true;
	}

	return false;
}

bool CSatchel::Deploy()
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;
	//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );

	bool result;

	if (m_chargeReady != ChargeState::NoSatchelsDeployed)
		result = DefaultDeploy("models/v_satchel_radio.mdl", "models/p_satchel_radio.mdl", SATCHEL_RADIO_DRAW, "hive");
	else
		result = DefaultDeploy("models/v_satchel.mdl", "models/p_satchel.mdl", SATCHEL_DRAW, "trip");

	if (result)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2;
	}

	return result;
}

void CSatchel::Holster()
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	if (m_chargeReady != ChargeState::NoSatchelsDeployed)
	{
		SendWeaponAnim(SATCHEL_RADIO_HOLSTER);
	}
	else
	{
		SendWeaponAnim(SATCHEL_DROP);
	}
	m_pPlayer->EmitSound(CHAN_WEAPON, "common/null.wav");

	if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] && m_chargeReady == ChargeState::NoSatchelsDeployed)
	{
		m_pPlayer->pev->weapons &= ~(1 << WEAPON_SATCHEL);
		SetThink(&CSatchel::DestroyItem);
		pev->nextthink = gpGlobals->time + 0.1;
	}
}

void CSatchel::PrimaryAttack()
{
	switch (m_chargeReady)
	{
	case ChargeState::NoSatchelsDeployed:
	{
		Throw();
	}
	break;
	case ChargeState::SatchelsDeployed:
	{
		SendWeaponAnim(SATCHEL_RADIO_FIRE);

		edict_t* pPlayer = m_pPlayer->edict();

		CBaseEntity* pSatchel = nullptr;

		while ((pSatchel = UTIL_FindEntityInSphere(pSatchel, m_pPlayer->pev->origin, WORLD_BOUNDARY)) != nullptr)
		{
			if (ClassnameIs(pSatchel->pev, "monster_satchel"))
			{
				if (pSatchel->pev->owner == pPlayer)
				{
					pSatchel->Use(m_pPlayer, m_pPlayer, USE_ON, 0);
					m_chargeReady = ChargeState::Reloading;
				}
			}
		}

		m_chargeReady = ChargeState::Reloading;
		m_flNextPrimaryAttack = GetNextAttackDelay(0.5);
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
		break;
	}

	case ChargeState::Reloading:
		// we're reloading, don't allow fire
	{
	}
	break;
	}
}

void CSatchel::SecondaryAttack()
{
	if (m_chargeReady != ChargeState::Reloading)
	{
		Throw();
	}
}

void CSatchel::Throw()
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		const Vector vecSrc = m_pPlayer->pev->origin;

		const Vector vecThrow = gpGlobals->v_forward * 274 + m_pPlayer->pev->velocity;

#ifndef CLIENT_DLL
		CBaseEntity* pSatchel = Create("monster_satchel", vecSrc, vec3_origin, m_pPlayer->edict());
		pSatchel->pev->velocity = vecThrow;
		pSatchel->pev->avelocity.y = 400;

		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_satchel_radio.mdl");
		m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_satchel_radio.mdl");
#else
		LoadVModel("models/v_satchel_radio.mdl", m_pPlayer);
#endif

		SendWeaponAnim(SATCHEL_RADIO_DRAW);

		// player "shoot" animation
		m_pPlayer->SetAnimation(PlayerAnim::Attack1);

		m_chargeReady = ChargeState::SatchelsDeployed;

		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

		m_flNextPrimaryAttack = GetNextAttackDelay(1.0);
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;
	}
}

void CSatchel::WeaponIdle()
{
	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	switch (m_chargeReady)
	{
	case ChargeState::NoSatchelsDeployed:
		SendWeaponAnim(SATCHEL_FIDGET1);
		// use tripmine animations
		safe_strcpy(m_pPlayer->m_szAnimExtension, "trip");
		break;
	case ChargeState::SatchelsDeployed:
		SendWeaponAnim(SATCHEL_RADIO_FIDGET1);
		// use hivehand animations
		safe_strcpy(m_pPlayer->m_szAnimExtension, "hive");
		break;
	case ChargeState::Reloading:
		if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			m_chargeReady = ChargeState::NoSatchelsDeployed;
			RetireWeapon();
			return;
		}

#ifndef CLIENT_DLL
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_satchel.mdl");
		m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_satchel.mdl");
#else
		LoadVModel("models/v_satchel.mdl", m_pPlayer);
#endif

		SendWeaponAnim(SATCHEL_DRAW);

		// use tripmine animations
		safe_strcpy(m_pPlayer->m_szAnimExtension, "trip");

		m_flNextPrimaryAttack = GetNextAttackDelay(0.5);
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;
		m_chargeReady = ChargeState::NoSatchelsDeployed;
		break;
	}
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);// how long till we do this again.
}

void CSatchel::GetWeaponData(weapon_data_t& data)
{
	data.iuser1 = static_cast<int>(m_chargeReady);
}

void CSatchel::SetWeaponData(const weapon_data_t& data)
{
	m_chargeReady = static_cast<ChargeState>(data.iuser1);
}

void DeactivateSatchels(CBasePlayer* pOwner)
{
	edict_t* pFind = FIND_ENTITY_BY_CLASSNAME(nullptr, "monster_satchel");

	while (!IsNullEnt(pFind))
	{
		if (CSatchelCharge* pSatchel = (CSatchelCharge*)CBaseEntity::Instance(pFind); pSatchel)
		{
			if (pSatchel->pev->owner == pOwner->edict())
			{
				pSatchel->Deactivate();
			}
		}

		pFind = FIND_ENTITY_BY_CLASSNAME(pFind, "monster_satchel");
	}
}
