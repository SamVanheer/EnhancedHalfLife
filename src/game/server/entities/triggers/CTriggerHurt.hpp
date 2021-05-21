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
#include "CTriggerHurt.generated.hpp"

constexpr int SF_TRIGGER_HURT_TARGETONCE = 1;		//!< Only fire hurt target once
constexpr int SF_TRIGGER_HURT_START_OFF = 2;		//!< spawnflag that makes trigger_push spawn turned OFF
constexpr int SF_TRIGGER_HURT_NO_CLIENTS = 8;		//!< spawnflag that makes trigger_push spawn turned OFF
constexpr int SF_TRIGGER_HURT_CLIENTONLYFIRE = 16;	//!< trigger hurt will only fire its target if it is hurting a client
constexpr int SF_TRIGGER_HURT_CLIENTONLYTOUCH = 32;	//!< only clients may touch this trigger.

/**
*	@brief hurts anything that touches it. if the trigger has a targetname, firing it will toggle state
*	@details trigger hurt that causes radiation will do a radius check
*	and set the player's geiger counter level according to distance from center of trigger
*/
class EHL_CLASS() CTriggerHurt : public CBaseTrigger
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	/**
	*	@brief When touched, a hurt trigger does DMG points of damage each half-second
	*/
	void EXPORT HurtTouch(CBaseEntity * pOther);
	void EXPORT RadiationThink();
};
