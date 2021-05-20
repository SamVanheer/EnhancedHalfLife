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

#include "CBreakable.hpp"

// func_pushable (it's also func_breakable, so don't collide with those flags)
constexpr int SF_PUSH_BREAKABLE = 128;

class EHL_CLASS() CPushable : public CBreakable
{
public:
	void	Spawn() override;
	void	Precache() override;
	void	Touch(CBaseEntity* pOther) override;
	void	Move(CBaseEntity* pMover, bool push);
	void	KeyValue(KeyValueData* pkvd) override;

	/**
	*	@brief Pull the func_pushable
	*/
	void	Use(const UseInfo& info) override;
	void	EXPORT StopMovementSound();
	//	virtual void	SetActivator( CBaseEntity *pActivator ) { m_pPusher = pActivator; }

	int	ObjectCaps() override { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_CONTINUOUS_USE; }
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	inline float MaxSpeed() { return m_maxSpeed; }

	// breakables use an overridden takedamage
	bool TakeDamage(const TakeDamageInfo& info)  override;

	static	TYPEDESCRIPTION m_SaveData[];

	static const char* m_soundNames[3];
	int m_lastSound = 0;	// no need to save/restore, just keeps the same sound from playing twice in a row
	float m_maxSpeed = 0;
	float m_soundTime = 0;
};
