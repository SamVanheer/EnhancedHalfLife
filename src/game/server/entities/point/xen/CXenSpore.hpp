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

#include "CActAnimating.hpp"

// UNDONE:	These need to smoke somehow when they take damage
//			Touch behavior?
//			Cause damage in smoke area
class CXenSpore : public CActAnimating
{
public:
	void		Spawn() override;
	void		Precache() override;
	void		Touch(CBaseEntity* pOther) override;
	void		Think() override;
	bool TakeDamage(const TakeDamageInfo& info) override { Attack(); return false; }
	//	void HandleAnimEvent(AnimationEvent& event);
	void		Attack() {}

	static const char* pModelNames[];
};

class CXenSporeSmall : public CXenSpore
{
	void		Spawn() override;
};

class CXenSporeMed : public CXenSpore
{
	void		Spawn() override;
};

class CXenSporeLarge : public CXenSpore
{
	void		Spawn() override;

	static const Vector m_hullSizes[];
};
