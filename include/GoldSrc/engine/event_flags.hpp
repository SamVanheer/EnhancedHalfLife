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

/**
*	@brief Skip local host for event send.
*/
constexpr int FEV_NOTHOST = 1 << 0;

/**
*	@brief Send the event reliably.
*	You must specify the origin and angles and use g_engfuncs.pfnPlaybackEvent for this to work correctly on the server
*	for anything that depends on the event origin/angles.
*	I.e., the origin/angles are not taken from the invoking edict for reliable events.
*/
constexpr int FEV_RELIABLE = 1 << 1;

/**
*	@brief Don't restrict to PAS/PVS, send this event to _everybody_ on the server
*	( useful for stopping SoundChannel::Static sounds started by client event when client is not in PVS anymore ( hwguy in TFC e.g. ).
*/
constexpr int FEV_GLOBAL = 1 << 2;

/**
*	@brief If this client already has one of these events in its queue, just update the event instead of sending it as a duplicate
*/
constexpr int FEV_UPDATE = 1 << 3;

/**
*	@brief Only send to entity specified as the invoker
*/
constexpr int	FEV_HOSTONLY = 1 << 4;

/**
*	@brief Only send if the event was created on the server.
*/
constexpr int FEV_SERVER = 1 << 5;

/**
*	@brief Only issue event client side ( from shared code )
*/
constexpr int FEV_CLIENT = 1 << 6;
