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

#include "CBaseDoor.hpp"
#include "CRotDoor.generated.hpp"

/**
*	@details if two doors touch, they are assumed to be connected and operate as a unit.
*	TOGGLE causes the door to wait in both the start and end states for a trigger event.
*	START_OPEN causes the door to move to its destination when spawned, and operate in reverse.
*	It is used to temporarily or permanently close off an area when triggered (not useful for touch or takedamage doors).
*	You need to have an origin brush as part of this entity.
*	The center of that brush will be the point around which it is rotated.
*	It will rotate around the Z axis by default.
*	You can check either the X_AXIS or Y_AXIS box to change that.
*
*	"distance" is how many degrees the door will be rotated.
*	"speed" determines how fast the door moves; default value is 100.
*	REVERSE will cause the door to rotate in the opposite direction.
*	"movedir"		determines the opening direction
*	"targetname"	if set, no touch field will be spawned and a remote button or trigger field activates the door.
*	"speed"			movement speed (100 default)
*	"wait"			wait before returning (0 default, -1 = never return)
*/
class EHL_CLASS("EntityName": "func_door_rotating") CRotDoor : public CBaseDoor
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
};
