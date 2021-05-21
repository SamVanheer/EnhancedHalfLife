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

#include <string_view>

#include "mathlib.hpp"

/**
*	@file
*
*	this file is included by both the game-dll and the client-dll
*/

constexpr std::string_view GAME_NAME{"Half-Life"};

constexpr int MAX_CLIENTS = 32;

constexpr int MAX_WEAPONS = 32;	// ???

//TODO: these two should be one constant
constexpr int MAX_WEAPON_SLOTS = 5;	// hud weapon selection slots
constexpr int MAX_WEAPON_TYPES = 6;	// hud weapon selection slots

constexpr int MAX_ITEMS = 1;	// hard coded item types

constexpr int HIDEHUD_WEAPONS = 1 << 0;
constexpr int HIDEHUD_FLASHLIGHT = 1 << 1;
constexpr int HIDEHUD_ALL = 1 << 2;
constexpr int HIDEHUD_HEALTH = 1 << 3;

constexpr int MAX_AMMO_TYPES = 32;

constexpr int TEAM_NAME_LENGTH = 16;

constexpr int HUD_PRINTNOTIFY = 1;
constexpr int HUD_PRINTCONSOLE = 2;
constexpr int HUD_PRINTTALK = 3;
constexpr int HUD_PRINTCENTER = 4;

constexpr char HUD_SAYTEXT_PRINTTALK = 2;

constexpr int WEAPON_SUIT = 31;

constexpr int CLIMB_SHAKE_FREQUENCY = 22;			//!< how many frames in between screen shakes when climbing
constexpr int MAX_CLIMB_SPEED = 200;				//!< fastest vertical climbing speed possible
constexpr int CLIMB_SPEED_DEC = 15;					//!< climbing deceleration rate
constexpr int CLIMB_PUNCH_X = -7;					//!< how far to 'punch' client X axis when climbing
constexpr int CLIMB_PUNCH_Z = 7;					//!< how far to 'punch' client Z axis when climbing

constexpr int PLAYER_FATAL_FALL_SPEED = 1024;		//!< approx 60 feet
constexpr int PLAYER_MAX_SAFE_FALL_SPEED = 580;		//!< approx 20 feet
constexpr float DAMAGE_FOR_FALL_SPEED
= 100.0f / (PLAYER_FATAL_FALL_SPEED - PLAYER_MAX_SAFE_FALL_SPEED);	//!< damage per unit per second.
constexpr int PLAYER_MIN_BOUNCE_SPEED = 200;
constexpr float PLAYER_FALL_PUNCH_THRESHHOLD = 350; //!< won't punch player's screen/make scrape noise unless player falling at least this fast.

constexpr int PLAYER_LONGJUMP_SPEED = 350;			//!< how fast we longjump
constexpr double PLAYER_DUCKING_MULTIPLIER = 0.333;

constexpr int PLAYER_WATERJUMP_HEIGHT = 8;			//!< how high the player jumps when exiting water

constexpr float BUNNYJUMP_MAX_SPEED_FACTOR = 1.7f;	//!< Only allow bunny jumping up to 1.7x server / player maxspeed setting

constexpr float PLAYER_AIRTIME = 12;				//!< lung full of air lasts this many seconds

constexpr double PLAYER_ARMOR_RATIO = 0.2;			//!< Armor Takes 80% of the damage
constexpr double PLAYER_ARMOR_BONUS = 0.5;			//!< Each Point of Armor is work 1/x points of health

constexpr std::string_view SOUND_FLASHLIGHT_ON{"items/flashlight1.wav"};
constexpr std::string_view SOUND_FLASHLIGHT_OFF{"items/flashlight1.wav"};

constexpr float FLASH_DRAIN_TIME = 1.2;				//!< 100 units/3 minutes
constexpr float FLASH_CHARGE_TIME = 0.2;			//!< 100 units/20 seconds  (seconds per unit)

constexpr float PLAYER_USE_SEARCH_RADIUS = 64;		//!< Player +use search radius

constexpr float PLAYER_GEIGER_DELAY = 0.25f;		//!< Delay between geiger checks

constexpr float PLAYER_SUIT_UPDATE_TIME = 3.5;
constexpr float PLAYER_SUIT_FIRST_UPDATE_TIME = 0.1;

constexpr double AUTOAIM_2DEGREES = 0.0348994967025;
constexpr double AUTOAIM_5DEGREES = 0.08715574274766;
constexpr double AUTOAIM_8DEGREES = 0.1391731009601;
constexpr double AUTOAIM_10DEGREES = 0.1736481776669;

// used by suit voice to indicate damage sustained and repaired type to player

// instant damage

constexpr int DMG_GENERIC = 0;;					//!< generic damage was done
constexpr int DMG_CRUSH = 1 << 0;				//!< crushed by falling or moving object
constexpr int DMG_BULLET = 1 << 1;				//!< shot
constexpr int DMG_SLASH = 1 << 2;				//!< cut, clawed, stabbed
constexpr int DMG_BURN = 1 << 3;				//!< heat burned
constexpr int DMG_FREEZE = 1 << 4;				//!< frozen
constexpr int DMG_FALL = 1 << 5;				//!< fell too far
constexpr int DMG_BLAST = 1 << 6;				//!< explosive blast damage
constexpr int DMG_CLUB = 1 << 7;				//!< crowbar, punch, headbutt
constexpr int DMG_SHOCK = 1 << 8;				//!< electric shock
constexpr int DMG_SONIC = 1 << 9;				//!< sound pulse shockwave
constexpr int DMG_ENERGYBEAM = 1 << 10;			//!< laser or other high energy beam 
constexpr int DMG_NEVERGIB = 1 << 12;			//!< with this bit OR'd in, no damage type will be able to gib victims upon death
constexpr int DMG_ALWAYSGIB = 1 << 13;			//!< with this bit OR'd in, any damage type can be made to gib victims upon death.

// time-based damage

constexpr int DMG_DROWN = 1 << 14;				//!< Drowning
constexpr int DMG_FIRSTTIMEBASED = DMG_DROWN;

constexpr int DMG_PARALYZE = 1 << 15;			//!< slows affected creature down
constexpr int DMG_NERVEGAS = 1 << 16;			//!< nerve toxins, very bad
constexpr int DMG_POISON = 1 << 17;				//!< blood poisioning
constexpr int DMG_RADIATION = 1 << 18;			//!< radiation exposure
constexpr int DMG_DROWNRECOVER = 1 << 19;		//!< drowning recovery
constexpr int DMG_ACID = 1 << 20;				//!< toxic chemicals or acid burns
constexpr int DMG_SLOWBURN = 1 << 21;			//!< in an oven
constexpr int DMG_SLOWFREEZE = 1 << 22;			//!< in a subzero freezer
constexpr int DMG_MORTAR = 1 << 23;				//!< Hit by air raid (done to distinguish grenade from mortar)

//mask off TF-specific stuff too
/**
*	@brief mask for time-based damage
*/
constexpr int DMG_TIMEBASED = DMG_DROWN
| DMG_PARALYZE
| DMG_NERVEGAS
| DMG_POISON
| DMG_RADIATION
| DMG_DROWNRECOVER
| DMG_ACID
| DMG_SLOWBURN
| DMG_SLOWFREEZE
| DMG_MORTAR;

//TF ADDITIONS
constexpr int DMG_IGNITE = 1 << 24;				//!< Players hit by this begin to burn
constexpr int DMG_RADIUS_MAX = 1 << 25;			//!< Radius damage with this flag doesn't decrease over distance
constexpr int DMG_RADIUS_QUAKE = 1 << 26;		//!< Radius damage is done like Quake. 1/2 damage at 1/2 radius.
constexpr int DMG_IGNOREARMOR = 1 << 27;		//!< Damage ignores target's armor
constexpr int DMG_AIMED = 1 << 28;				//!< Does Hit location damage
constexpr int DMG_WALLPIERCING = 1 << 29;		//!< Blast Damages ents through walls

constexpr int DMG_CALTROP = 1 << 30;
constexpr int DMG_HALLUC = 1 << 31;

// TF Healing Additions for GiveHealth
constexpr int DMG_IGNORE_MAXHEALTH = DMG_IGNITE;
// TF Redefines since we never use the originals
constexpr int DMG_NAIL = DMG_SLASH;
constexpr int DMG_NOT_SELF = DMG_FREEZE;


constexpr int DMG_TRANQ = DMG_MORTAR;
constexpr int DMG_CONCUSS = DMG_SONIC;

// these are the damage types that are allowed to gib corpses
constexpr int DMG_GIB_CORPSE = DMG_CRUSH | DMG_FALL | DMG_BLAST | DMG_SONIC | DMG_CLUB;

// these are the damage types that have client hud art
constexpr int DMG_SHOWNHUD = DMG_POISON | DMG_ACID | DMG_FREEZE | DMG_SLOWFREEZE | DMG_DROWN | DMG_BURN | DMG_SLOWBURN | DMG_NERVEGAS | DMG_RADIATION | DMG_SHOCK;

// NOTE: tweak these values based on gameplay feedback:

constexpr int PARALYZE_DURATION = 2;		//!< number of 2 second intervals to take damage
constexpr double PARALYZE_DAMAGE = 1.0;		//!< damage to take each 2 second interval

constexpr int NERVEGAS_DURATION = 2;
constexpr double NERVEGAS_DAMAGE = 5.0;

constexpr int POISON_DURATION = 5;
constexpr double POISON_DAMAGE = 2.0;

constexpr int RADIATION_DURATION = 2;
constexpr double RADIATION_DAMAGE = 1.0;

constexpr int ACID_DURATION = 2;
constexpr double ACID_DAMAGE = 5.0;

constexpr int SLOWBURN_DURATION = 2;
constexpr double SLOWBURN_DAMAGE = 1.0;

constexpr int SLOWFREEZE_DURATION = 2;
constexpr double SLOWFREEZE_DAMAGE = 1.0;

constexpr int itbd_Paralyze = 0;
constexpr int itbd_NerveGas = 1;
constexpr int itbd_Poison = 2;
constexpr int itbd_Radiation = 3;
constexpr int itbd_DrownRecover = 4;
constexpr int itbd_Acid = 5;
constexpr int itbd_SlowBurn = 6;
constexpr int itbd_SlowFreeze = 7;
constexpr int CDMG_TIMEBASED = 8;

/**
*	@brief people gib if their health is <= this at the time of death
*/
constexpr int GIB_HEALTH_VALUE = -30;

constexpr Vector VEC_HULL_MIN(-16, -16, -36);
constexpr Vector VEC_HULL_MAX(16, 16, 36);
constexpr Vector VEC_HUMAN_HULL_MIN(-16, -16, 0);
constexpr Vector VEC_HUMAN_HULL_MAX(16, 16, 72);
constexpr Vector VEC_HUMAN_HULL_DUCK(16, 16, 36);

constexpr Vector VEC_VIEW(0, 0, 28);

constexpr Vector VEC_DUCK_HULL_MIN(-16, -16, -18);
constexpr Vector VEC_DUCK_HULL_MAX(16, 16, 18);
constexpr Vector VEC_DUCK_VIEW(0, 0, 12);

constexpr Vector VEC_DEAD_VIEW(0, 0, -8);
