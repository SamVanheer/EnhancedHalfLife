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

#include "CTriggerCounter.generated.hpp"

constexpr int SF_COUNTER_NOMESSAGE = 1;

/**
*	@brief Acts as an intermediary for an action that takes multiple inputs.
*	@details If nomessage is not set, it will print "1 more.. " etc when triggered and "sequence complete" when finished.
*	After the counter has been triggered "cTriggersLeft" times (default 2), it will fire all of it's targets and remove itself.
*/
class EHL_CLASS() CTriggerCounter : public CBaseTrigger
{
	EHL_GENERATED_BODY()

public:
	void KeyValue(KeyValueData * pkvd) override;
	void Spawn() override;

	void EXPORT CounterUse(const UseInfo & info);

	EHL_FIELD(Persisted)
	int m_cTriggersLeft = 0; //!< # of activations remaining
};
