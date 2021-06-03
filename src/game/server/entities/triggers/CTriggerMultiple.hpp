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
#include "CTriggerMultiple.generated.hpp"

/**
*	@brief Variable sized repeatable trigger. Must be targeted at one or more entities.
*	@details "wait" : Seconds between triggerings. (.2 default)
*/
class EHL_CLASS("EntityName": "trigger_multiple") CTriggerMultiple : public CBaseTrigger
{
	EHL_GENERATED_BODY()

public:
	void KeyValue(KeyValueData* pkvd) override;
	void Spawn() override;

	void EXPORT MultiTouch(CBaseEntity * pOther);

private:
	void ActivateMultiTrigger(CBaseEntity * pActivator);

	/**
	*	@brief the wait time has passed, so set back up for another activation
	*/
	void EXPORT MultiWaitOver();

protected:
	EHL_FIELD("Persisted": true)
	float m_flWait = 0;

private:
	EHL_FIELD("Persisted": true, "Type": "SoundName")
	string_t m_ActivateSound = iStringNull;
};
