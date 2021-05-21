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

#include "vector.h"

struct model_t;
struct particle_t;

constexpr int FBEAM_STARTENTITY = 0x00000001;
constexpr int FBEAM_ENDENTITY = 0x00000002;
constexpr int FBEAM_FADEIN = 0x00000004;
constexpr int FBEAM_FADEOUT = 0x00000008;
constexpr int FBEAM_SINENOISE = 0x00000010;
constexpr int FBEAM_SOLID = 0x00000020;
constexpr int FBEAM_SHADEIN = 0x00000040;
constexpr int FBEAM_SHADEOUT = 0x00000080;
constexpr int FBEAM_STARTVISIBLE = 0x10000000;		//!< Has this client actually seen this beam's start entity yet?
constexpr int FBEAM_ENDVISIBLE = 0x20000000;		//!< Has this client actually seen this beam's end entity yet?
constexpr int FBEAM_ISACTIVE = 0x40000000;
constexpr int FBEAM_FOREVER = 0x80000000;

struct BEAM
{
	BEAM* next;
	int			type;
	int			flags;
	Vector		source;
	Vector		target;
	Vector		delta;
	float		t;		//!< 0 .. 1 over lifetime of beam
	float		freq;
	float		die;
	float		width;
	float		amplitude;
	float		r, g, b;
	float		brightness;
	float		speed;
	float		frameRate;
	float		frame;
	int			segments;
	int			startEntity;
	int			endEntity;
	int			modelIndex;
	int			frameCount;
	model_t* pFollowModel;
	particle_t* particles;
};
