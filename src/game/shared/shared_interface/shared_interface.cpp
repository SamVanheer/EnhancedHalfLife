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

#include "Platform.h"
#include "mathlib.h"
#include "cdll_dll.h"

bool Shared_GetHullBounds(int hullnumber, Vector& mins, Vector& maxs)
{
	switch (hullnumber)
	{
	case 0:	// Normal player
		mins = VEC_HULL_MIN;
		maxs = VEC_HULL_MAX;
		return true;

	case 1:	// Crouched player
		mins = VEC_DUCK_HULL_MIN;
		maxs = VEC_DUCK_HULL_MAX;
		return true;

	case 2:	// Point based hull
		mins = vec3_origin;
		maxs = vec3_origin;
		return true;
	}

	return false;
}
