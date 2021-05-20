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

class CXenTreeTrigger;

constexpr int TREE_AE_ATTACK = 1;

class EHL_CLASS() CXenTree : public CActAnimating
{
public:
	void OnRemove() override;
	void		Spawn() override;
	void		Precache() override;
	void		Touch(CBaseEntity* pOther) override;
	void		Think() override;
	bool TakeDamage(const TakeDamageInfo& info) override { Attack(); return false; }
	void		HandleAnimEvent(AnimationEvent& event) override;
	void		Attack();
	int			Classify() override { return CLASS_BARNACLE; }

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	static const char* pAttackHitSounds[];
	static const char* pAttackMissSounds[];

private:
	EHandle<CXenTreeTrigger> m_hTrigger;
};
