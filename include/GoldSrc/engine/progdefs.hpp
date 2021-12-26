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

#include "Platform.hpp"
#include "codegen/codegen_api.hpp"
#include "const.hpp"
#include "mathlib.hpp"

struct edict_t;

struct globalvars_t
{
	float		time = 0;
	float		frametime = 0;
	float		force_retouch = 0;
	string_t	mapname;
	string_t	startspot;
	float		deathmatch = 0;
	float		coop = 0;
	float		teamplay = 0;
	float		serverflags = 0;
	float		found_secrets = 0;
	Vector		v_forward;
	Vector		v_up;
	Vector		v_right;
	float		trace_allsolid = 0;
	float		trace_startsolid = 0;
	float		trace_fraction = 0;
	Vector		trace_endpos;
	Vector		trace_plane_normal;
	float		trace_plane_dist = 0;
	edict_t*	trace_ent = nullptr;
	float		trace_inopen = 0;
	float		trace_inwater = 0;
	int			trace_hitgroup = 0;
	int			trace_flags = 0;
	int			msg_entity = 0;
	int			cdAudioTrack = 0;
	int			maxClients = 0;
	int			maxEntities = 0;
	const char* pStringBase = "";

	void* pSaveData = nullptr;
	Vector		vecLandmarkOffset;
};

#include "progdefs.generated.hpp"

struct EHL_CLASS() entvars_t
{
	EHL_GENERATED_BODY()

public:
	EHL_FIELD("Persisted": true)
	string_t	classname;

	EHL_FIELD("Persisted": true, "IsGlobal": true)
	string_t	globalname;

	EHL_FIELD("Persisted": true, "Type": "Position")
	Vector		origin;

	EHL_FIELD("Persisted": true, "Type" : "Position")
	Vector		oldorigin;

	EHL_FIELD("Persisted": true)
	Vector		velocity;

	EHL_FIELD("Persisted": true)
	Vector		basevelocity;

	Vector      clbasevelocity;  // Base velocity that was passed in to server physics so 
								 //  client can predict conveyors correctly.  Server zeroes it, so we need to store here, too.
	EHL_FIELD("Persisted": true)
	Vector		movedir;

	EHL_FIELD("Persisted": true)
	Vector		angles;			// Model angles

	EHL_FIELD("Persisted": true)
	Vector		avelocity;		// angle velocity (degrees per second)

	EHL_FIELD("Persisted": true)
	Vector		punchangle;		// auto-decaying view angle adjustment

	EHL_FIELD("Persisted": true)
	Vector		v_angle;		// Viewing angle (player only)

	// For parametric entities
	Vector		endpos;
	Vector		startpos;
	float		impacttime = 0;
	float		starttime = 0;

	EHL_FIELD("Persisted": true)
	FixAngleMode fixangle = FixAngleMode::None;

	EHL_FIELD("Persisted": true)
	float		idealpitch = 0;

	EHL_FIELD("Persisted": true)
	float		pitch_speed = 0;

	EHL_FIELD("Persisted": true)
	float		ideal_yaw = 0;

	EHL_FIELD("Persisted": true)
	float		yaw_speed = 0;

	EHL_FIELD("Persisted": true)
	int			modelindex = 0;

	EHL_FIELD("Persisted": true, "IsGlobal": true, "Type": "ModelName")
	string_t	model;

	EHL_FIELD("Persisted": true, "Type" : "ModelName")
	string_t viewmodel;		// player's viewmodel

	EHL_FIELD("Persisted": true, "Type" : "ModelName")
	string_t weaponmodel;	// what other players see

	EHL_FIELD("Persisted": true, "Type" : "Position")
	Vector		absmin;		// BB max translated to world coord

	EHL_FIELD("Persisted": true, "Type" : "Position")
	Vector		absmax;		// BB max translated to world coord

	EHL_FIELD("Persisted": true, "IsGlobal" : true)
	Vector		mins;		// local BB min

	EHL_FIELD("Persisted": true, "IsGlobal" : true)
	Vector		maxs;		// local BB max

	EHL_FIELD("Persisted": true, "IsGlobal" : true)
	Vector		size;		// maxs - mins

	EHL_FIELD("Persisted": true, "Type" : "Time")
	float		ltime = 0;

	EHL_FIELD("Persisted": true, "Type" : "Time")
	float		nextthink = 0;

	EHL_FIELD("Persisted": true)
	Movetype movetype = Movetype::None;

	EHL_FIELD("Persisted": true)
	Solid solid = Solid::Not;

	EHL_FIELD("Persisted": true)
	int			skin = 0;

	EHL_FIELD("Persisted": true)
	int			body = 0;			// sub-model selection for studiomodels

	EHL_FIELD("Persisted": true)
	int 		effects = 0;

	EHL_FIELD("Persisted": true)
	float		gravity = 0;		// % of "normal" gravity

	EHL_FIELD("Persisted": true)
	float		friction = 0;		// inverse elasticity of Movetype::Bounce

	EHL_FIELD("Persisted": true)
	int			light_level = 0;

	EHL_FIELD("Persisted": true)
	int			sequence = 0;		// animation sequence
	int			gaitsequence = 0;	// movement animation sequence for player (0 for none)

	EHL_FIELD("Persisted": true)
	float		frame = 0;			// % playback position in animation sequences (0..255)

	EHL_FIELD("Persisted": true, "Type": "Time")
	float		animtime = 0;		// world time when frame was set

	EHL_FIELD("Persisted": true)
	float		framerate = 0;		// animation playback rate (-8x to 8x)

	EHL_FIELD("Persisted": true)
	byte		controller[4]{};	// bone controller setting (0..255)

	EHL_FIELD("Persisted": true)
	byte		blending[2]{};		// blending amount between sub-sequences (0..255)

	EHL_FIELD("Persisted": true)
	float		scale = 0;			// sprite rendering scale (0..255)

	EHL_FIELD("Persisted": true)
	RenderMode rendermode = RenderMode::Normal;

	EHL_FIELD("Persisted": true)
	float		renderamt = 0;

	EHL_FIELD("Persisted": true)
	Vector		rendercolor;

	EHL_FIELD("Persisted": true)
	RenderFX renderfx = RenderFX::None;

	EHL_FIELD("Persisted": true)
	float		health = 0;

	EHL_FIELD("Persisted": true)
	float		frags = 0;

	EHL_FIELD("Persisted": true)
	int			weapons = 0;  // bit mask for available weapons

	EHL_FIELD("Persisted": true)
	float		takedamage = 0;

	EHL_FIELD("Persisted": true)
	DeadFlag deadflag = DeadFlag::No;

	EHL_FIELD("Persisted": true)
	Vector		view_ofs;	// eye position

	EHL_FIELD("Persisted": true)
	int			button = 0;

	EHL_FIELD("Persisted": true)
	int			impulse = 0;

	EHL_FIELD("Persisted": true)
	edict_t* chain = nullptr;			// Entity pointer when linked into a linked list

	EHL_FIELD("Persisted": true)
	edict_t* dmg_inflictor = nullptr;

	EHL_FIELD("Persisted": true)
	edict_t* enemy = nullptr;

	EHL_FIELD("Persisted": true)
	edict_t* aiment = nullptr;			// entity pointer when MOVETYPE_FOLLOW

	EHL_FIELD("Persisted": true)
	edict_t* owner = nullptr;

	EHL_FIELD("Persisted": true)
	edict_t* groundentity = nullptr;

	EHL_FIELD("Persisted": true)
	int			spawnflags = 0;

	EHL_FIELD("Persisted": true)
	int			flags = 0;

	EHL_FIELD("Persisted": true)
	int			colormap = 0;			// lowbyte topcolor, highbyte bottomcolor

	EHL_FIELD("Persisted": true)
	int			team = 0;

	EHL_FIELD("Persisted": true)
	float		max_health = 0;

	EHL_FIELD("Persisted": true, "Type": "Time")
	float		teleport_time = 0;

	EHL_FIELD("Persisted": true)
	float		armortype = 0;

	EHL_FIELD("Persisted": true)
	float		armorvalue = 0;

	EHL_FIELD("Persisted": true)
	WaterLevel waterlevel = WaterLevel::Dry;

	EHL_FIELD("Persisted": true)
	Contents watertype = Contents::None;

	EHL_FIELD("Persisted": true, "IsGlobal" : true)
	string_t	target;

	EHL_FIELD("Persisted": true, "IsGlobal" : true)
	string_t	targetname;

	EHL_FIELD("Persisted": true)
	string_t	netname;

	EHL_FIELD("Persisted": true)
	string_t	message;

	EHL_FIELD("Persisted": true)
	float		dmg_take = 0;

	EHL_FIELD("Persisted": true)
	float		dmg_save = 0;

	EHL_FIELD("Persisted": true)
	float		dmg = 0;

	EHL_FIELD("Persisted": true, "Type": "Time")
	float		dmgtime = 0;

	string_t	noise;
	string_t	noise1;
	string_t	noise2;
	string_t	noise3;

	EHL_FIELD("Persisted": true)
	float		speed = 0;

	EHL_FIELD("Persisted": true, "Type" : "Time")
	float		air_finished = 0;

	EHL_FIELD("Persisted": true, "Type" : "Time")
	float		pain_finished = 0;

	EHL_FIELD("Persisted": true, "Type" : "Time")
	float		radsuit_finished = 0;

	edict_t* pContainingEntity = nullptr;

	int			playerclass = 0;
	float		maxspeed = 0;

	float		fov = 0;
	int			weaponanim = 0;

	int			pushmsec = 0;

	int			bInDuck = 0;
	int			flTimeStepSound = 0;
	int			flSwimTime= 0;
	int			flDuckTime= 0;
	int			iStepLeft = 0;
	float		flFallVelocity = 0;

	int			gamestate = 0;

	int			oldbuttons = 0;

	int			groupinfo = 0;

	// For mods
	int			iuser1 = 0;
	int			iuser2 = 0;
	int			iuser3 = 0;
	int			iuser4 = 0;
	float		fuser1 = 0;
	float		fuser2 = 0;
	float		fuser3 = 0;
	float		fuser4 = 0;
	Vector		vuser1;
	Vector		vuser2;
	Vector		vuser3;
	Vector		vuser4;
	edict_t* euser1 = nullptr;
	edict_t* euser2 = nullptr;
	edict_t* euser3 = nullptr;
	edict_t* euser4 = nullptr;
};
