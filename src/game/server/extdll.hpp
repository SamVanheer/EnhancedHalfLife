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

/**
*	@file
*
*	Global header file for extension DLLs
*/

#include "Platform.hpp"

// Header file containing definition of globalvars_t and entvars_t

// Vector class
#include "mathlib.hpp"

// Shared engine/DLL constants
#include "const.hpp"
#include "edict.hpp"

// Shared header describing protocol between engine and DLLs
#include "sv_engine_int.hpp"

// Shared header between the client DLL and the game DLLs
#include "cdll_dll.hpp"