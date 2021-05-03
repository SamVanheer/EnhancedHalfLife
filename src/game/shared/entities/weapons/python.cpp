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
#include "weapons.h"
#include "monsters.h"
#include "player.h"
#include "gamerules.h"
#include "UserMessages.h"

LINK_ENTITY_TO_CLASS(weapon_python, CPython);
LINK_ENTITY_TO_CLASS(weapon_357, CPython);

bool CPython::GetItemInfo(ItemInfo* p)
{
	p->pszName = GetClassname();
	p->pszAmmo1 = "357";
	p->iMaxAmmo1 = _357_MAX_CARRY;
	p->pszAmmo2 = nullptr;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = PYTHON_MAX_CLIP;
	p->iFlags = 0;
	p->iSlot = 1;
	p->iPosition = 1;
	p->iId = m_iId = WEAPON_PYTHON;
	p->iWeight = PYTHON_WEIGHT;

	return true;
}

bool CPython::AddToPlayer(CBasePlayer* pPlayer)
{
	if (CBasePlayerWeapon::AddToPlayer(pPlayer))
	{
		MESSAGE_BEGIN(MessageDest::One, gmsgWeapPickup, pPlayer);
		WRITE_BYTE(m_iId);
		MESSAGE_END();
		return true;
	}
	return false;
}

void CPython::Spawn()
{
	SetClassname("weapon_357"); // hack to allow for old names
	Precache();
	m_iId = WEAPON_PYTHON;
	SetModel("models/w_357.mdl");

	m_iDefaultAmmo = PYTHON_DEFAULT_GIVE;

	FallInit();// get ready to fall down.
}

void CPython::Precache()
{
	PRECACHE_MODEL("models/v_357.mdl");
	PRECACHE_MODEL("models/w_357.mdl");
	PRECACHE_MODEL("models/p_357.mdl");

	PRECACHE_MODEL("models/w_357ammobox.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");

	PRECACHE_SOUND("weapons/357_reload1.wav");
	PRECACHE_SOUND("weapons/357_cock1.wav");
	PRECACHE_SOUND("weapons/357_shot1.wav");
	PRECACHE_SOUND("weapons/357_shot2.wav");

	m_usFirePython = PRECACHE_EVENT(1, "events/python.sc");
}

bool CPython::Deploy()
{
#ifdef CLIENT_DLL
	if (bIsMultiplayer())
#else
	if (g_pGameRules->IsMultiplayer())
#endif
	{
		// enable laser sight geometry.
		pev->body = 1;
	}
	else
	{
		pev->body = 0;
	}

	return DefaultDeploy("models/v_357.mdl", "models/p_357.mdl", PYTHON_DRAW, "python", pev->body);
}

void CPython::Holster()
{
	m_fInReload = false;// cancel any reload in progress.

	if (m_hPlayer->m_iFOV != 0)
	{
		SecondaryAttack();
	}

	m_hPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;
	m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_hPlayer->random_seed, 10, 15);
	SendWeaponAnim(PYTHON_HOLSTER);
}

void CPython::SecondaryAttack()
{
#ifdef CLIENT_DLL
	if (!bIsMultiplayer())
#else
	if (!g_pGameRules->IsMultiplayer())
#endif
	{
		return;
	}

	if (m_hPlayer->m_iFOV != 0)
	{
		m_hPlayer->m_iFOV = 0;  // 0 means reset to default fov
	}
	else if (m_hPlayer->m_iFOV != 40)
	{
		m_hPlayer->m_iFOV = 40;
	}

	m_flNextSecondaryAttack = 0.5;
}

void CPython::PrimaryAttack()
{
	// don't fire underwater
	if (m_hPlayer->pev->waterlevel == WaterLevel::Head)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = 0.15;
		}

		return;
	}

	m_hPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_hPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_iClip--;

	m_hPlayer->pev->effects = (int)(m_hPlayer->pev->effects) | EF_MUZZLEFLASH;

	// player "shoot" animation
	m_hPlayer->SetAnimation(PlayerAnim::Attack1);


	UTIL_MakeVectors(m_hPlayer->pev->v_angle + m_hPlayer->pev->punchangle);

	const Vector vecSrc = m_hPlayer->GetGunPosition();
	const Vector vecAiming = m_hPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	const Vector vecDir = m_hPlayer->FireBulletsPlayer(1, vecSrc, vecAiming, VECTOR_CONE_1DEGREES, WORLD_SIZE, BULLET_PLAYER_357, 0, 0, m_hPlayer, m_hPlayer->random_seed);

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	UTIL_PlaybackEvent(flags, m_hPlayer, m_usFirePython, {.fparam1 = vecDir.x, .fparam2 = vecDir.y});

	if (!m_iClip && m_hPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_hPlayer->SetSuitUpdate("!HEV_AMO0", SuitSoundType::Sentence, 0);

	m_flNextPrimaryAttack = 0.75;
	m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_hPlayer->random_seed, 10, 15);
}

void CPython::Reload()
{
	if (m_hPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	if (m_hPlayer->m_iFOV != 0)
	{
		m_hPlayer->m_iFOV = 0;  // 0 means reset to default fov
	}

	bool bUseScope = false;
#ifdef CLIENT_DLL
	bUseScope = bIsMultiplayer();
#else
	bUseScope = g_pGameRules->IsMultiplayer();
#endif

	DefaultReload(PYTHON_MAX_CLIP, PYTHON_RELOAD, 2.0, bUseScope);
}

void CPython::WeaponIdle()
{
	ResetEmptySound();

	m_hPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	int iAnim;
	const float flRand = UTIL_SharedRandomFloat(m_hPlayer->random_seed, 0, 1);
	if (flRand <= 0.5)
	{
		iAnim = PYTHON_IDLE1;
		m_flTimeWeaponIdle = (70.0 / 30.0);
	}
	else if (flRand <= 0.7)
	{
		iAnim = PYTHON_IDLE2;
		m_flTimeWeaponIdle = (60.0 / 30.0);
	}
	else if (flRand <= 0.9)
	{
		iAnim = PYTHON_IDLE3;
		m_flTimeWeaponIdle = (88.0 / 30.0);
	}
	else
	{
		iAnim = PYTHON_FIDGET;
		m_flTimeWeaponIdle = (170.0 / 30.0);
	}

	bool bUseScope = false;
#ifdef CLIENT_DLL
	bUseScope = bIsMultiplayer();
#else
	bUseScope = g_pGameRules->IsMultiplayer();
#endif

	SendWeaponAnim(iAnim, bUseScope);
}

class CPythonAmmo : public CBasePlayerAmmo
{
	void Spawn() override
	{
		Precache();
		SetModel("models/w_357ammobox.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_357ammobox.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	bool AddAmmo(CBasePlayer* pOther) override
	{
		if (pOther->GiveAmmo(AMMO_357BOX_GIVE, "357", _357_MAX_CARRY) != -1)
		{
			EmitSound(SoundChannel::Item, "items/9mmclip1.wav");
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(ammo_357, CPythonAmmo);
