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

#include "CTriggerMultiple.hpp"
#include "CTriggerOnce.generated.hpp"

/**
*	@brief Variable sized trigger. Triggers once, then removes itself.
*	@details You must set the key "target" to the name of another object in the level that has a matching "targetname".
*	if "killtarget" is set, any objects that have a matching "target" will be removed when the trigger is fired.
*	if "angle" is set, the trigger will only fire when someone is facing the direction of the angle.  Use "360" for an angle of 0.
*/
class EHL_CLASS() CTriggerOnce : public CTriggerMultiple
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
};
