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

#include "animation.hpp"
#include "CWeaponCycler.hpp"

void CWeaponCycler::Spawn()
{
	SetSolidType(Solid::SlideBox);
	SetMovetype(Movetype::None);

	PRECACHE_MODEL(STRING(pev->model));
	SetModel(STRING(pev->model));
	m_iszModel = pev->model;
	m_iModel = pev->modelindex;

	SetAbsOrigin(GetAbsOrigin());
	SetSize(Vector(-16, -16, 0), Vector(16, 16, 16));
	SetTouch(&CWeaponCycler::ItemTouch);
}

bool CWeaponCycler::Deploy()
{
	m_hPlayer->pev->viewmodel = m_iszModel;
	m_hPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;
	SendWeaponAnim(0);
	m_iClip = 0;
	return true;
}

void CWeaponCycler::Holster()
{
	m_hPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
}

void CWeaponCycler::PrimaryAttack()
{

	SendWeaponAnim(pev->sequence);

	m_flNextPrimaryAttack = gpGlobals->time + 0.3;
}

void CWeaponCycler::SecondaryAttack()
{
	float flFrameRate, flGroundSpeed;

	pev->sequence = (pev->sequence + 1) % 8;

	pev->modelindex = m_iModel;
	GetSequenceInfo(GetModelPointer(), this, flFrameRate, flGroundSpeed);
	pev->modelindex = 0;

	if (flFrameRate == 0.0)
	{
		pev->sequence = 0;
	}

	SendWeaponAnim(pev->sequence);

	m_flNextSecondaryAttack = gpGlobals->time + 0.3;
}
