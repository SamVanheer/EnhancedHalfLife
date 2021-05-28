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

#include "CBasePlatTrain.hpp"
#include "CFuncTrain.generated.hpp"

// Trains
constexpr int SF_TRAIN_WAIT_RETRIGGER = 1;
constexpr int SF_TRAIN_START_ON = 4;		//!< Train is initially moving
constexpr int SF_TRAIN_PASSABLE = 8;		//!< Train is not solid -- used to make water trains

/**
*	@brief Trains are moving platforms that players can ride.
*	@details The targets origin specifies the min point of the train at each corner.
*	The train spawns at the first target it is pointing at.
*	If the train is the target of a button or trigger, it will not begin moving until activated.
*	speed	default 100
*	dmg		default	2
*/
class EHL_CLASS() CFuncTrain : public CBasePlatTrain
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	void Activate() override;
	void OverrideReset() override;

	void Blocked(CBaseEntity* pOther) override;
	void Use(const UseInfo& info) override;
	void KeyValue(KeyValueData* pkvd) override;


	void EXPORT Wait();

	/**
	*	@brief path corner needs to change to next target
	*/
	void EXPORT Next();

	EHL_FIELD(Persisted)
	EHANDLE m_hCurrentTarget;

	EHL_FIELD(Persisted)
	int m_sounds = 0;

	EHL_FIELD(Persisted)
	bool m_activated = false;
};
