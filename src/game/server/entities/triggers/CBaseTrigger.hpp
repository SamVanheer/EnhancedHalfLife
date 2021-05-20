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

#include "CBaseToggle.hpp"

constexpr int SF_TRIGGER_ALLOWMONSTERS = 1;	//!< monsters allowed to fire this trigger
constexpr int SF_TRIGGER_NOCLIENTS = 2;		//!< players not allowed to fire this trigger
constexpr int SF_TRIGGER_PUSHABLES = 4;		//!< only pushables can fire this trigger

class EHL_CLASS() CBaseTrigger : public CBaseToggle
{
public:
	int	ObjectCaps() override { return CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	void KeyValue(KeyValueData * pkvd) override;

	void InitTrigger();
	void ActivateMultiTrigger(CBaseEntity * pActivator);

	/**
	*	@brief the wait time has passed, so set back up for another activation
	*/
	void EXPORT MultiWaitOver();

	/**
	*	@brief If this is the USE function for a trigger, its state will toggle every time it's fired
	*/
	void EXPORT ToggleUse(const UseInfo & info);
};