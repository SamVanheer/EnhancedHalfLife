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

#include "CBaseTrigger.hpp"
#include "CTriggerPush.generated.hpp"

constexpr int SF_TRIGGER_PUSH_ONCE = 1;
constexpr int SF_TRIGGER_PUSH_START_OFF = 2;		//!< spawnflag that makes trigger_push spawn turned OFF

/**
*	@brief Pushes the player and other entities
*/
class EHL_CLASS(EntityName=trigger_push) CTriggerPush : public CBaseTrigger
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	void KeyValue(KeyValueData * pkvd) override;
	void Touch(CBaseEntity * pOther) override;
};
