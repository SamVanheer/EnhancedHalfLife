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

#include "extdll.h"

/**
*	@file
* 
*	Declares the server library interface to the engine
*/

extern "C"
{
DLLEXPORT int GetEntityAPI(DLL_FUNCTIONS * pFunctionTable, int interfaceVersion);
DLLEXPORT int GetEntityAPI2(DLL_FUNCTIONS * pFunctionTable, int* interfaceVersion);
DLLEXPORT int GetNewDLLFunctions(NEW_DLL_FUNCTIONS * pFunctionTable, int* interfaceVersion);
}
