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

#include "CBaseWeapon.hpp"
#include "CWeaponBox.hpp"

void CWeaponBox::Precache()
{
	PRECACHE_MODEL("models/w_weaponbox.mdl");
}

void CWeaponBox::KeyValue(KeyValueData* pkvd)
{
	if (m_cAmmoTypes < MAX_AMMO_TYPES)
	{
		PackAmmo(ALLOC_STRING(pkvd->szKeyName), atoi(pkvd->szValue));
		m_cAmmoTypes++;// count this new ammo type.

		pkvd->fHandled = true;
	}
	else
	{
		ALERT(at_console, "WeaponBox too full! only %d ammotypes allowed\n", MAX_AMMO_TYPES);
	}
}

void CWeaponBox::Spawn()
{
	Precache();

	SetMovetype(Movetype::Toss);
	SetSolidType(Solid::Trigger);

	SetSize(vec3_origin, vec3_origin);

	SetModel("models/w_weaponbox.mdl");
}

void CWeaponBox::Kill()
{
	CBaseWeapon* pWeapon;
	int i;

	// destroy the weapons
	for (i = 0; i < MAX_WEAPON_TYPES; i++)
	{
		pWeapon = m_hPlayerWeapons[i];

		while (pWeapon)
		{
			pWeapon->SetThink(&CBaseWeapon::SUB_Remove);
			pWeapon->pev->nextthink = gpGlobals->time + 0.1;
			pWeapon = pWeapon->m_hNext;
		}
	}

	// remove the box
	UTIL_Remove(this);
}

void CWeaponBox::Touch(CBaseEntity* pOther)
{
	if (!(pev->flags & FL_ONGROUND))
	{
		return;
	}

	if (!pOther->IsPlayer())
	{
		// only players may touch a weaponbox.
		return;
	}

	if (!pOther->IsAlive())
	{
		// no dead guys.
		return;
	}

	CBasePlayer* pPlayer = (CBasePlayer*)pOther;
	int i;

	// dole out ammo
	for (i = 0; i < MAX_AMMO_TYPES; i++)
	{
		if (!IsStringNull(m_rgiszAmmo[i]))
		{
			// there's some ammo of this type. 
			pPlayer->GiveAmmo(m_rgAmmo[i], STRING(m_rgiszAmmo[i]));

			//ALERT ( at_console, "Gave %d rounds of %s\n", m_rgAmmo[i], STRING(m_rgiszAmmo[i]) );

			// now empty the ammo from the weaponbox since we just gave it to the player
			m_rgiszAmmo[i] = iStringNull;
			m_rgAmmo[i] = 0;
		}
	}

	// go through my weapons and try to give the usable ones to the player. 
	// it's important the the player be given ammo first, so the weapons code doesn't refuse 
	// to deploy a better weapon that the player may pick up because he has no ammo for it.
	for (i = 0; i < MAX_WEAPON_TYPES; i++)
	{
		CBaseWeapon* weapon = m_hPlayerWeapons[i];

		while (weapon)
		{
			//ALERT ( at_console, "trying to give %s\n", m_hPlayerWeapons[ i ]->GetClassname() );

			auto next = m_hPlayerWeapons[i]->m_hNext;

			m_hPlayerWeapons[i] = next;// unlink this weapon from the box

			if (pPlayer->AddPlayerWeapon(weapon).Action == ItemApplyAction::AttachedToPlayer)
			{
				weapon->AttachToPlayer(pPlayer);
			}
			else
			{
				//Player didn't pick it up, remove the weapon
				UTIL_Remove(weapon);
			}

			weapon = next;
		}
	}

	pOther->EmitSound(SoundChannel::Item, "items/gunpickup2.wav");
	SetTouch(nullptr);
	UTIL_Remove(this);
}

bool CWeaponBox::PackWeapon(CBaseWeapon* pWeapon)
{
	// is one of these weapons already packed in this box?
	if (HasWeapon(pWeapon))
	{
		return false;// box can only hold one of each weapon type
	}

	if (auto player = pWeapon->m_hPlayer.Get(); player)
	{
		if (!player->RemovePlayerWeapon(pWeapon))
		{
			// failed to unhook the weapon from the player!
			return false;
		}
	}

	int iWeaponSlot = pWeapon->WeaponSlot();

	if (m_hPlayerWeapons[iWeaponSlot])
	{
		// there's already one weapon in this slot, so link this into the slot's column
		pWeapon->m_hNext = m_hPlayerWeapons[iWeaponSlot];
		m_hPlayerWeapons[iWeaponSlot] = pWeapon;
	}
	else
	{
		// first weapon we have for this slot
		m_hPlayerWeapons[iWeaponSlot] = pWeapon;
		pWeapon->m_hNext = nullptr;
	}

	pWeapon->m_RespawnMode = ItemRespawnMode::Never;// never respawn
	pWeapon->SetMovetype(Movetype::None);
	pWeapon->SetSolidType(Solid::Not);
	pWeapon->pev->effects = EF_NODRAW;
	pWeapon->pev->modelindex = 0;
	pWeapon->pev->model = iStringNull;
	pWeapon->SetOwner(this);
	pWeapon->SetThink(nullptr);// crowbar may be trying to swing again, etc.
	pWeapon->SetTouch(nullptr);
	pWeapon->m_hPlayer = nullptr;

	//ALERT ( at_console, "packed %s\n", pWeapon->GetClassname() );

	return true;
}

bool CWeaponBox::PackAmmo(string_t iszName, int iCount)
{
	int iMaxCarry;

	if (IsStringNull(iszName))
	{
		// error here
		ALERT(at_console, "NULL String in PackAmmo!\n");
		return false;
	}

	iMaxCarry = MaxAmmoCarry(iszName);

	if (iMaxCarry != -1 && iCount > 0)
	{
		//ALERT ( at_console, "Packed %d rounds of %s\n", iCount, STRING(iszName) );
		GiveAmmo(iCount, STRING(iszName), iMaxCarry);
		return true;
	}

	return false;
}

int CWeaponBox::GiveAmmo(int iCount, const char* szName, int iMax, int* pIndex)
{
	int i;

	for (i = 1; i < MAX_AMMO_TYPES && !IsStringNull(m_rgiszAmmo[i]); i++)
	{
		if (stricmp(szName, STRING(m_rgiszAmmo[i])) == 0)
		{
			if (pIndex)
				*pIndex = i;

			int iAdd = std::min(iCount, iMax - m_rgAmmo[i]);
			if (iCount == 0 || iAdd > 0)
			{
				m_rgAmmo[i] += iAdd;

				return i;
			}
			return -1;
		}
	}
	if (i < MAX_AMMO_TYPES)
	{
		if (pIndex)
			*pIndex = i;

		m_rgiszAmmo[i] = MAKE_STRING(szName);
		m_rgAmmo[i] = iCount;

		return i;
	}
	ALERT(at_console, "out of named ammo slots\n");
	return i;
}

bool CWeaponBox::HasWeapon(CBaseWeapon* pCheckWeapon)
{
	CBaseWeapon* weapon = m_hPlayerWeapons[pCheckWeapon->WeaponSlot()];

	while (weapon)
	{
		if (weapon->ClassnameIs(pCheckWeapon->GetClassname()))
		{
			return true;
		}
		weapon = weapon->m_hNext;
	}

	return false;
}

bool CWeaponBox::IsEmpty()
{
	int i;

	for (i = 0; i < MAX_WEAPON_TYPES; i++)
	{
		if (m_hPlayerWeapons[i])
		{
			return false;
		}
	}

	for (i = 0; i < MAX_AMMO_TYPES; i++)
	{
		if (!IsStringNull(m_rgiszAmmo[i]))
		{
			// still have a bit of this type of ammo
			return false;
		}
	}

	return true;
}

void CWeaponBox::SetObjectCollisionBox()
{
	pev->absmin = GetAbsOrigin() + Vector(-16, -16, 0);
	pev->absmax = GetAbsOrigin() + Vector(16, 16, 16);
}
