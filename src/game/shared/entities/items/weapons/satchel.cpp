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
	SetSolidType(Solid::Not);
	UTIL_Remove(this);
}

void CSatchelCharge::Spawn()
{
	Precache();
	// motor
	SetMovetype(Movetype::Bounce);
	SetSolidType(Solid::BBox);

	SetModel("models/w_satchel.mdl");
	//SetSize( Vector( -16, -16, -4), Vector(16, 16, 32));	// Old box -- size of headcrab monsters/players get blocked by this
	SetSize(Vector(-4, -4, -4), Vector(4, 4, 4));	// Uses point-sized, and can be stepped over
	SetAbsOrigin(GetAbsOrigin());

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
	if (pOther == GetOwner())
		return;

	// pev->avelocity = Vector (300, 300, 300);
	pev->gravity = 1;// normal gravity now

	// HACKHACK - On ground isn't always set, so look for ground underneath
	TraceResult tr;
	UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() - Vector(0, 0, 10), IgnoreMonsters::Yes, this, &tr);

	if (tr.flFraction < 1.0)
	{
		// add a bit of static friction
		SetAbsVelocity(GetAbsVelocity() * 0.95);
		pev->avelocity = pev->avelocity * 0.9;
		// play sliding sound, volume based on velocity
	}
	if (!(pev->flags & FL_ONGROUND) && GetAbsVelocity().Length2D() > 10)
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
		SetMovetype(Movetype::Fly);

		Vector velocity = GetAbsVelocity() * 0.8;
		velocity.z += 8;
		SetAbsVelocity(velocity);

		pev->avelocity = pev->avelocity * 0.9;
	}
	else if (pev->waterlevel == WaterLevel::Dry)
	{
		SetMovetype(Movetype::Bounce);
	}
	else
	{
		SetAbsVelocity(GetAbsVelocity() - Vector{0, 0, 8});
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
	case 0:	EmitSound(SoundChannel::Voice, "weapons/g_bounce1.wav"); break;
	case 1:	EmitSound(SoundChannel::Voice, "weapons/g_bounce2.wav"); break;
	case 2:	EmitSound(SoundChannel::Voice, "weapons/g_bounce3.wav"); break;
	}
}

LINK_ENTITY_TO_CLASS(weapon_satchel, CSatchel);

bool CSatchel::AddDuplicate(CBasePlayerWeapon* pOriginal)
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
	const bool bResult = CBasePlayerWeapon::AddToPlayer(pPlayer);

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
	FallInit();// get ready to fall down.
}

void CSatchel::Precache()
{
	CBasePlayerWeapon::Precache();

	PRECACHE_MODEL("models/v_satchel.mdl");
	PRECACHE_MODEL("models/v_satchel_radio.mdl");
	PRECACHE_MODEL("models/w_satchel.mdl");
	PRECACHE_MODEL("models/p_satchel.mdl");
	PRECACHE_MODEL("models/p_satchel_radio.mdl");

	UTIL_PrecacheOther("monster_satchel");
}

bool CSatchel::GetWeaponInfo(WeaponInfo* p)
{
	p->pszName = GetClassname();
	p->pszAmmo1 = "Satchel Charge";
	p->iMaxAmmo1 = SATCHEL_MAX_CARRY;
	p->pszAmmo2 = nullptr;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 4;
	p->iPosition = 1;
	p->iFlags = WEAPON_FLAG_SELECTONEMPTY | WEAPON_FLAG_LIMITINWORLD | WEAPON_FLAG_EXHAUSTIBLE;
	p->iId = m_iId = WEAPON_SATCHEL;
	p->iWeight = SATCHEL_WEIGHT;

	return true;
}

bool CSatchel::IsUseable()
{
	auto player = m_hPlayer.Get();

	if (player->m_rgAmmo[PrimaryAmmoIndex()] > 0)
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
	auto player = m_hPlayer.Get();

	if (player->m_rgAmmo[PrimaryAmmoIndex()] > 0)
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
	auto player = m_hPlayer.Get();

	player->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;
	//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( player->random_seed, 10, 15 );

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
	auto player = m_hPlayer.Get();

	player->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	if (m_chargeReady != ChargeState::NoSatchelsDeployed)
	{
		SendWeaponAnim(SATCHEL_RADIO_HOLSTER);
	}
	else
	{
		SendWeaponAnim(SATCHEL_DROP);
	}
	player->EmitSound(SoundChannel::Weapon, "common/null.wav");

	if (!player->m_rgAmmo[m_iPrimaryAmmoType] && m_chargeReady == ChargeState::NoSatchelsDeployed)
	{
		player->pev->weapons &= ~(1 << WEAPON_SATCHEL);
		SetThink(&CSatchel::DestroyWeapon);
		pev->nextthink = gpGlobals->time + 0.1;
	}
}

void CSatchel::PrimaryAttack()
{
	auto player = m_hPlayer.Get();

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

		CBaseEntity* pSatchel = nullptr;

		while ((pSatchel = UTIL_FindEntityInSphere(pSatchel, player->GetAbsOrigin(), WORLD_BOUNDARY)) != nullptr)
		{
			if (pSatchel->ClassnameIs("monster_satchel"))
			{
				if (pSatchel->GetOwner() == player)
				{
					pSatchel->Use({player, player, UseType::On});
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
	auto player = m_hPlayer.Get();

	if (player->m_rgAmmo[m_iPrimaryAmmoType])
	{
		const Vector vecSrc = player->GetAbsOrigin();

		const Vector vecThrow = gpGlobals->v_forward * 274 + player->GetAbsVelocity();

#ifndef CLIENT_DLL
		CBaseEntity* pSatchel = Create("monster_satchel", vecSrc, vec3_origin, player);
		pSatchel->SetAbsVelocity(vecThrow);
		pSatchel->pev->avelocity.y = 400;

		player->pev->viewmodel = MAKE_STRING("models/v_satchel_radio.mdl");
		player->pev->weaponmodel = MAKE_STRING("models/p_satchel_radio.mdl");
#else
		LoadVModel("models/v_satchel_radio.mdl", player);
#endif

		SendWeaponAnim(SATCHEL_RADIO_DRAW);

		// player "shoot" animation
		player->SetAnimation(PlayerAnim::Attack1);

		m_chargeReady = ChargeState::SatchelsDeployed;

		player->m_rgAmmo[m_iPrimaryAmmoType]--;

		m_flNextPrimaryAttack = GetNextAttackDelay(1.0);
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;
	}
}

void CSatchel::WeaponIdle()
{
	auto player = m_hPlayer.Get();

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	switch (m_chargeReady)
	{
	case ChargeState::NoSatchelsDeployed:
		SendWeaponAnim(SATCHEL_FIDGET1);
		// use tripmine animations
		safe_strcpy(player->m_szAnimExtension, "trip");
		break;
	case ChargeState::SatchelsDeployed:
		SendWeaponAnim(SATCHEL_RADIO_FIDGET1);
		// use hivehand animations
		safe_strcpy(player->m_szAnimExtension, "hive");
		break;
	case ChargeState::Reloading:
		if (!player->m_rgAmmo[m_iPrimaryAmmoType])
		{
			m_chargeReady = ChargeState::NoSatchelsDeployed;
			RetireWeapon();
			return;
		}

#ifndef CLIENT_DLL
		player->pev->viewmodel = MAKE_STRING("models/v_satchel.mdl");
		player->pev->weaponmodel = MAKE_STRING("models/p_satchel.mdl");
#else
		LoadVModel("models/v_satchel.mdl", player);
#endif

		SendWeaponAnim(SATCHEL_DRAW);

		// use tripmine animations
		safe_strcpy(player->m_szAnimExtension, "trip");

		m_flNextPrimaryAttack = GetNextAttackDelay(0.5);
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;
		m_chargeReady = ChargeState::NoSatchelsDeployed;
		break;
	}
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(player->random_seed, 10, 15);// how long till we do this again.
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
	CBaseEntity* pFind = nullptr;

	while ((pFind = UTIL_FindEntityByClassname(pFind, "monster_satchel")) != nullptr)
	{
		CSatchelCharge* pSatchel = (CSatchelCharge*)pFind;

		if (pSatchel->GetOwner() == pOwner)
		{
			pSatchel->Deactivate();
		}
	}
}
