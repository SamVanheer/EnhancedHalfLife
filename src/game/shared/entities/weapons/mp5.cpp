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
#include "soundent.h"
#include "gamerules.h"
#include "UserMessages.h"

LINK_ENTITY_TO_CLASS(weapon_mp5, CMP5);
LINK_ENTITY_TO_CLASS(weapon_9mmAR, CMP5);

void CMP5::Spawn()
{
	pev->classname = MAKE_STRING("weapon_9mmAR"); // hack to allow for old names
	Precache();
	SetModel("models/w_9mmAR.mdl");
	m_iId = WEAPON_MP5;

	m_iDefaultAmmo = MP5_DEFAULT_GIVE;

	FallInit();// get ready to fall down.
}

void CMP5::Precache()
{
	PRECACHE_MODEL("models/v_9mmAR.mdl");
	PRECACHE_MODEL("models/w_9mmAR.mdl");
	PRECACHE_MODEL("models/p_9mmAR.mdl");

	m_iShell = PRECACHE_MODEL("models/shell.mdl");// brass shellTE_MODEL

	PRECACHE_MODEL("models/grenade.mdl");	// grenade

	PRECACHE_MODEL("models/w_9mmARclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");

	PRECACHE_SOUND("items/clipinsert1.wav");
	PRECACHE_SOUND("items/cliprelease1.wav");

	PRECACHE_SOUND("weapons/hks1.wav");// H to the K
	PRECACHE_SOUND("weapons/hks2.wav");// H to the K
	PRECACHE_SOUND("weapons/hks3.wav");// H to the K

	PRECACHE_SOUND("weapons/glauncher.wav");
	PRECACHE_SOUND("weapons/glauncher2.wav");

	PRECACHE_SOUND("weapons/357_cock1.wav");

	m_usMP5 = PRECACHE_EVENT(1, "events/mp5.sc");
	m_usMP52 = PRECACHE_EVENT(1, "events/mp52.sc");
}

bool CMP5::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = _9MM_MAX_CARRY;
	p->pszAmmo2 = "ARgrenades";
	p->iMaxAmmo2 = M203_GRENADE_MAX_CARRY;
	p->iMaxClip = MP5_MAX_CLIP;
	p->iSlot = 2;
	p->iPosition = 0;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_MP5;
	p->iWeight = MP5_WEIGHT;

	return true;
}

bool CMP5::AddToPlayer(CBasePlayer* pPlayer)
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

bool CMP5::Deploy()
{
	return DefaultDeploy("models/v_9mmAR.mdl", "models/p_9mmAR.mdl", MP5_DEPLOY, "mp5");
}

void CMP5::PrimaryAttack()
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
		PlayEmptySound();
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	m_hPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_hPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	m_iClip--;

	m_hPlayer->pev->effects = (int)(m_hPlayer->pev->effects) | EF_MUZZLEFLASH;

	// player "shoot" animation
	m_hPlayer->SetAnimation(PlayerAnim::Attack1);

	const Vector vecSrc = m_hPlayer->GetGunPosition();
	const Vector vecAiming = m_hPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);
	Vector vecDir;

#ifdef CLIENT_DLL
	if (bIsMultiplayer())
#else
	if (g_pGameRules->IsMultiplayer())
#endif
	{
		// optimized multiplayer. Widened to make it easier to hit a moving player
		vecDir = m_hPlayer->FireBulletsPlayer(1, vecSrc, vecAiming, VECTOR_CONE_6DEGREES, WORLD_SIZE, BULLET_PLAYER_MP5, 2, 0, m_hPlayer, m_hPlayer->random_seed);
	}
	else
	{
		// single player spread
		vecDir = m_hPlayer->FireBulletsPlayer(1, vecSrc, vecAiming, VECTOR_CONE_3DEGREES, WORLD_SIZE, BULLET_PLAYER_MP5, 2, 0, m_hPlayer, m_hPlayer->random_seed);
	}

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	UTIL_PlaybackEvent(flags, m_hPlayer, m_usMP5, {.fparam1 = vecDir.x, .fparam2 = vecDir.y});

	if (!m_iClip && m_hPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_hPlayer->SetSuitUpdate("!HEV_AMO0", SuitSoundType::Sentence, 0);

	m_flNextPrimaryAttack = GetNextAttackDelay(0.1);

	if (m_flNextPrimaryAttack < UTIL_WeaponTimeBase())
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_hPlayer->random_seed, 10, 15);
}

void CMP5::SecondaryAttack()
{
	// don't fire underwater
	if (m_hPlayer->pev->waterlevel == WaterLevel::Head)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	if (m_hPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 0)
	{
		PlayEmptySound();
		return;
	}

	m_hPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_hPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_hPlayer->m_iExtraSoundTypes = bits_SOUND_DANGER;
	m_hPlayer->m_flStopExtraSoundTime = UTIL_WeaponTimeBase() + 0.2;

	m_hPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;

	// player "shoot" animation
	m_hPlayer->SetAnimation(PlayerAnim::Attack1);

	UTIL_MakeVectors(m_hPlayer->pev->v_angle + m_hPlayer->pev->punchangle);

	// we don't add in player velocity anymore.
	CGrenade::ShootContact(m_hPlayer,
		m_hPlayer->pev->origin + m_hPlayer->pev->view_ofs + gpGlobals->v_forward * 16,
		gpGlobals->v_forward * 800);

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	UTIL_PlaybackEvent(flags, m_hPlayer, m_usMP52);

	m_flNextPrimaryAttack = GetNextAttackDelay(1);
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5;// idle pretty soon after shooting.

	if (!m_hPlayer->m_rgAmmo[m_iSecondaryAmmoType])
		// HEV suit - indicate out of ammo condition
		m_hPlayer->SetSuitUpdate("!HEV_AMO0", SuitSoundType::Sentence, 0);
}

void CMP5::Reload()
{
	if (m_hPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	DefaultReload(MP5_MAX_CLIP, MP5_RELOAD, 1.5);
}

void CMP5::WeaponIdle()
{
	ResetEmptySound();

	m_hPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	int iAnim;
	switch (RANDOM_LONG(0, 1))
	{
	case 0:
		iAnim = MP5_LONGIDLE;
		break;

	default:
	case 1:
		iAnim = MP5_IDLE1;
		break;
	}

	SendWeaponAnim(iAnim);

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_hPlayer->random_seed, 10, 15); // how long till we do this again.
}

class CMP5AmmoClip : public CBasePlayerAmmo
{
	void Spawn() override
	{
		Precache();
		SetModel("models/w_9mmARclip.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_9mmARclip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	bool AddAmmo(CBasePlayer* pOther) override
	{
		int bResult = (pOther->GiveAmmo(AMMO_MP5CLIP_GIVE, "9mm", _9MM_MAX_CARRY) != -1);
		if (bResult)
		{
			EmitSound(SoundChannel::Item, "items/9mmclip1.wav");
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS(ammo_mp5clip, CMP5AmmoClip);
LINK_ENTITY_TO_CLASS(ammo_9mmAR, CMP5AmmoClip);

class CMP5Chainammo : public CBasePlayerAmmo
{
	void Spawn() override
	{
		Precache();
		SetModel("models/w_chainammo.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_chainammo.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	bool AddAmmo(CBasePlayer* pOther) override
	{
		int bResult = (pOther->GiveAmmo(AMMO_CHAINBOX_GIVE, "9mm", _9MM_MAX_CARRY) != -1);
		if (bResult)
		{
			EmitSound(SoundChannel::Item, "items/9mmclip1.wav");
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS(ammo_9mmbox, CMP5Chainammo);

class CMP5AmmoGrenade : public CBasePlayerAmmo
{
	void Spawn() override
	{
		Precache();
		SetModel("models/w_ARgrenade.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_ARgrenade.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	bool AddAmmo(CBasePlayer* pOther) override
	{
		int bResult = (pOther->GiveAmmo(AMMO_M203BOX_GIVE, "ARgrenades", M203_GRENADE_MAX_CARRY) != -1);

		if (bResult)
		{
			EmitSound(SoundChannel::Item, "items/9mmclip1.wav");
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS(ammo_mp5grenades, CMP5AmmoGrenade);
LINK_ENTITY_TO_CLASS(ammo_ARgrenades, CMP5AmmoGrenade);
