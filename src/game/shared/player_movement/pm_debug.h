/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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

#include "mathlib.h"

/**
*	@brief Shows a particle trail from player to entity in crosshair.
*	Shows particles at that entities bbox
*	Tries to shoot a ray out by about 128 units.
*/
void PM_ViewEntity();
void PM_DrawBBox(Vector mins, const Vector& maxs, const Vector& origin, int pcolor, float life);
void PM_ParticleLine(const Vector& start, const Vector& end, int pcolor, float life, float vert);
void PM_ShowClipBox();
