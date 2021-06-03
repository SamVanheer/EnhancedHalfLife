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
#include "CFireAndDie.generated.hpp"

/**
*	@brief Fires a target after level transition and then dies
*/
class EHL_CLASS("EntityName": "fireanddie") CFireAndDie : public CBaseEntity
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	void Precache() override;
	void Think() override;
	int ObjectCaps() override { return BaseClass::ObjectCaps() | FCAP_FORCE_TRANSITION; }	// Always go across transitions
};
