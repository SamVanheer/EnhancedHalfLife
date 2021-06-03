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

#include "CPointEntity.hpp"
#include "CTriggerVolume.generated.hpp"

/**
*	@brief Define space that travels across a level transition
*	Derive from point entity so this doesn't move across levels
*/
class EHL_CLASS("EntityName": "trigger_transition") CTriggerVolume : public CPointEntity
{
	EHL_GENERATED_BODY()

public:
	void		Spawn() override;
};
