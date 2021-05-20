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

#pragma once

#include "CGrenade.hpp"

enum w_squeak_e
{
	WSQUEAK_IDLE1 = 0,
	WSQUEAK_FIDGET,
	WSQUEAK_JUMP,
	WSQUEAK_RUN,
};

class EHL_CLASS() CSqueakGrenade : public CGrenade
{
	void Spawn() override;
	void Precache() override;
	int  Classify() override;
	void EXPORT SuperBounceTouch(CBaseEntity* pOther);
	void EXPORT HuntThink();
	int  BloodColor() override { return BLOOD_COLOR_YELLOW; }
	void Killed(const KilledInfo& info) override;
	void GibMonster() override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];

	static inline float m_flNextBounceSoundTime = 0;

	// CBaseEntity *m_pTarget;
	float m_flDie = 0;
	Vector m_vecTarget;
	float m_flNextHunt = 0;
	float m_flNextHit = 0;
	Vector m_posPrev;
	EHANDLE m_hOwner;
	int  m_iMyClass = 0;
};
