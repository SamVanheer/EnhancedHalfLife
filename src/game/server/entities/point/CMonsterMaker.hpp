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
#include "CMonsterMaker.generated.hpp"

constexpr int SF_MONSTERMAKER_START_ON = 1;		//!< start active ( if has targetname )
constexpr int SF_MONSTERMAKER_CYCLIC = 4;		//!< drop one monster every time fired.
constexpr int SF_MONSTERMAKER_MONSTERCLIP = 8;	//!< Children are blocked by monsterclip

/**
*	@brief this ent creates monsters during the game.
*/
class EHL_CLASS(EntityName=monstermaker) CMonsterMaker : public CBaseMonster
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	void Precache() override;
	void KeyValue(KeyValueData* pkvd) override;

	/**
	*	@brief activates/deactivates the monster maker
	*/
	void EXPORT ToggleUse(const UseInfo& info);

	/**
	*	@brief drops one monster from the monstermaker each time we call this.
	*/
	void EXPORT CyclicUse(const UseInfo& info);

	/**
	*	@brief creates a new monster every so often
	*/
	void EXPORT MakerThink();
	void DeathNotice(CBaseEntity* pChild) override;//!< monster maker children use this to tell the monster maker that they have died.

	/**
	*	@brief this is the code that drops the monster
	*/
	void MakeMonster();

	EHL_FIELD(Persisted)
	string_t m_iszMonsterClassname = iStringNull;//!< classname of the monster(s) that will be created.

	EHL_FIELD(Persisted)
	int m_cNumMonsters = 0;//!< max number of monsters this ent can create

	EHL_FIELD(Persisted)
	int m_cLiveChildren = 0;//!< how many monsters made by this monster maker that are currently alive

	EHL_FIELD(Persisted)
	int m_iMaxLiveChildren = 0;//!< max number of monsters that this maker may have out at one time.

	EHL_FIELD(Persisted)
	float m_flGround = 0; //!< z coord of the ground under me, used to make sure no monsters are under the maker when it drops a new child

	EHL_FIELD(Persisted)
	bool m_fActive = false;

	EHL_FIELD(Persisted)
	bool m_fFadeChildren = false;//!< should we make the children fadeout?
};
