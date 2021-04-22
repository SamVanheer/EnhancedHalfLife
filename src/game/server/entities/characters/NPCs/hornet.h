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

constexpr int HORNET_TYPE_RED = 0;
constexpr int HORNET_TYPE_ORANGE = 1;
constexpr float HORNET_RED_SPEED = 600;
constexpr float HORNET_ORANGE_SPEED = 800;
constexpr float HORNET_BUZZ_VOLUME = 0.8;

/**
*	@brief this is the projectile that the Alien Grunt fires.
*/
class CHornet : public CBaseMonster
{
public:
	void Spawn() override;
	void Precache() override;

	/**
	*	@brief ID's Hornet as their owner
	*/
	int	 Classify() override;

	/**
	*	@brief hornets will never get mad at each other, no matter who the owner is.
	*/
	int  IRelationship(CBaseEntity* pTarget) override;
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	void IgniteTrail();

	/**
	*	@brief starts a hornet out tracking its target
	*/
	void EXPORT StartTrack();

	/**
	*	@brief starts a hornet out just flying straight.
	*/
	void EXPORT StartDart();

	/**
	*	@brief Hornet is flying, gently tracking target
	*/
	void EXPORT TrackTarget();

	/**
	*	@brief Tracking Hornet hit something
	*/
	void EXPORT TrackTouch(CBaseEntity* pOther);
	void EXPORT DartTouch(CBaseEntity* pOther);
	void EXPORT DieTouch(CBaseEntity* pOther);

	/**
	*	@brief don't let hornets gib, ever.
	*/
	bool TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;

	float			m_flStopAttack;
	int				m_iHornetType;
	float			m_flFlySpeed;
};
