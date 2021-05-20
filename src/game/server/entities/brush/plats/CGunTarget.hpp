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

#include "CBaseMonster.hpp"

constexpr int FGUNTARGET_START_ON = 0x0001;

/**
*	@details pev->speed is the travel speed
*	pev->health is current health
*	pev->max_health is the amount to reset to each time it starts
*/
class EHL_CLASS() CGunTarget : public CBaseMonster
{
public:
	void			Spawn() override;
	void			Activate() override;
	void EXPORT		Next();
	void EXPORT		Start();
	void EXPORT		Wait();
	void			Stop() override;

	int				BloodColor() override { return DONT_BLEED; }
	int				Classify() override { return CLASS_MACHINE; }
	bool TakeDamage(const TakeDamageInfo& info) override;
	void			Use(const UseInfo& info) override;
	Vector			BodyTarget(const Vector& posSrc) override { return GetAbsOrigin(); }

	int	ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];

private:
	bool m_on = false;
};