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

constexpr int SF_TRIPMINE_INSTANT_ON = 1 << 0;

class EHL_CLASS() CTripmineGrenade : public CGrenade
{
	void OnRemove() override;
	void Spawn() override;
	void Precache() override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];

	bool TakeDamage(const TakeDamageInfo& info) override;

	void EXPORT WarningThink();
	void EXPORT PowerupThink();
	void EXPORT BeamBreakThink();
	void EXPORT DelayDeathThink();
	void Killed(const KilledInfo& info) override;

	void MakeBeam();
	void KillBeam();

	float		m_flPowerUp = 0;
	Vector		m_vecDir;
	Vector		m_vecEnd;
	float		m_flBeamLength = 0;

	EHANDLE		m_hOwner;
	EHandle<CBeam> m_hBeam;
	Vector		m_posOwner;
	Vector		m_angleOwner;
	EHANDLE m_hRealOwner;// tracelines don't hit PEV->OWNER, which means a player couldn't detonate his own trip mine, so we store the owner here.
};
