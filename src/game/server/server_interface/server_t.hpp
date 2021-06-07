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

#include "consistency_t.hpp"
#include "crc.hpp"
#include "custom.hpp"
#include "event_t.hpp"
#include "progdefs.hpp"
#include "shared_utils.hpp"
#include "sizebuf_t.hpp"
#include "util.hpp"
#include "WorldLimits.hpp"

struct edict_t;
struct entity_state_t;
struct extra_baselines_t;
struct model_t;

constexpr int MAX_DATAGRAM_BUFFER = 4000;
constexpr int MAX_MULTICAST_BUFFER = 1024;
constexpr int MAX_SPECTATOR_BUFFER = 1024;
constexpr int MAX_SIGNON_BUFFER = 32768;

enum server_state_t
{
	ss_dead = 0,
	ss_loading,
	ss_active,
};

struct server_t
{
	qboolean active;
	qboolean paused;
	qboolean loadgame;

	double time;
	double oldtime;

	int lastcheck;
	double lastchecktime;

	char name[MAX_QPATH];
	char oldname[MAX_QPATH];
	char startspot[MAX_QPATH];
	char modelname[MAX_QPATH];

	model_t* worldmodel;
	CRC32_t worldmapCRC;

	byte clientdllmd5[16];

	resource_t resourcelist[MAX_RESOURCES];
	int num_resources;

	consistency_t consistency_list[MAX_MODELS];
	int num_consistency;

	char* model_precache[MAX_MODELS];
	model_t* models[MAX_MODELS];

	unsigned char model_precache_flags[MAX_MODELS];

	event_t event_precache[MAX_EVENTS];

	char* sound_precache[MAX_SOUNDS];
	short sound_precache_hashedlookup[(MAX_SOUNDS * 2) - 1];
	qboolean sound_precache_hashedlookup_built;

	char* generic_precache[MAX_GENERIC_FILES];
	char generic_precache_names[MAX_GENERIC_FILES][MAX_QPATH];
	int num_generic_names;

	char* lightstyles[MAX_LIGHT_STYLES];

	int num_edicts;
	int max_edicts;
	edict_t* edicts;

	entity_state_t* baselines;
	extra_baselines_t* instance_baselines;

	server_state_t state;

	sizebuf_t datagram;
	byte datagram_buf[MAX_DATAGRAM_BUFFER];

	sizebuf_t reliable_datagram;
	byte reliable_datagram_buf[MAX_DATAGRAM_BUFFER];

	sizebuf_t multicast;
	byte multicast_buf[MAX_MULTICAST_BUFFER];

	sizebuf_t spectator;
	byte spectator_buf[MAX_SPECTATOR_BUFFER];

	sizebuf_t signon;
	byte signon_data[MAX_SIGNON_BUFFER];
};

/**
*	@brief Gets the server_t global from the engine
*	Only returns a valid pointer if map loading has started
*/
inline server_t* SV_GetServerGlobal()
{
	if (!gpGlobals || IsStringNull(gpGlobals->mapname))
	{
		return nullptr;
	}

	auto mapNameAddress = reinterpret_cast<byte*>(const_cast<char*>(STRING(gpGlobals->mapname)));

	auto server = reinterpret_cast<server_t*>(mapNameAddress - offsetof(server_t, name));

	return server;
}
