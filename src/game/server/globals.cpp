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
/*
  DLL-wide global variable definitions.
  They're all defined here, for convenient centralization.
  Source files that need them should "extern ..." declare each
  variable, to better document what globals they care about.
  TODO: this is a bad way to handle this, it results in a lot of redundant forward declarations. Clean this up
*/

DLL_GLOBAL std::uint32_t g_ulFrameCount;
DLL_GLOBAL std::uint32_t g_ulModelIndexEyes;
DLL_GLOBAL std::uint32_t g_ulModelIndexPlayer;
DLL_GLOBAL Vector		g_vecAttackDir;
DLL_GLOBAL int			gDisplayTitle;
DLL_GLOBAL bool			g_fGameOver;
DLL_GLOBAL int			g_Language;
