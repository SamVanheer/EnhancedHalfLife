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

#include "CBaseEntity.hpp"
#include "CTriggerRelay.generated.hpp"

constexpr int SF_RELAY_FIREONCE = 0x0001;

class EHL_CLASS("EntityName": "trigger_relay") CTriggerRelay : public CBaseEntity
{
	EHL_GENERATED_BODY()

public:
	void KeyValue(KeyValueData * pkvd) override;
	void Spawn() override;
	void Use(const UseInfo & info) override;

	int ObjectCaps() override { return BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

private:
	EHL_FIELD("Persisted": true)
	UseType	triggerType = UseType::Off;
};
