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

#include "archtypes.h"     // DAL
#include <stdio.h>
#include "custom.h"

struct clientdata_t;
struct entity_state_t;
struct netadr_t;
struct playermove_t;
struct usercmd_t;
struct weapon_data_t;

constexpr int INTERFACE_VERSION = 140;
constexpr int MAX_INCONSISTENT_FILE_MESSAGE_LENGTH = 256;

// ONLY ADD NEW FUNCTIONS TO THE END OF THIS STRUCT.  INTERFACE VERSION IS FROZEN AT 138

struct DLL_FUNCTIONS
{
	// Initialize/shutdown the game (one-time call after loading of game .dll )
	void			(*pfnGameInit)			();
	int				(*pfnSpawn)				(edict_t* pent);
	void			(*pfnThink)				(edict_t* pent);
	void			(*pfnUse)				(edict_t* pentUsed, edict_t* pentOther);
	void			(*pfnTouch)				(edict_t* pentTouched, edict_t* pentOther);
	void			(*pfnBlocked)			(edict_t* pentBlocked, edict_t* pentOther);
	void			(*pfnKeyValue)			(edict_t* pentKeyvalue, KeyValueData* pkvd);
	void			(*pfnSave)				(edict_t* pent, SAVERESTOREDATA* pSaveData);
	int 			(*pfnRestore)			(edict_t* pent, SAVERESTOREDATA* pSaveData, int globalEntity);
	void			(*pfnSetAbsBox)			(edict_t* pent);

	void			(*pfnSaveWriteFields)	(SAVERESTOREDATA*, const char*, void*, TYPEDESCRIPTION*, int);
	void			(*pfnSaveReadFields)	(SAVERESTOREDATA*, const char*, void*, TYPEDESCRIPTION*, int);

	void			(*pfnSaveGlobalState)		(SAVERESTOREDATA*);
	void			(*pfnRestoreGlobalState)	(SAVERESTOREDATA*);
	void			(*pfnResetGlobalState)		();

	qboolean(*pfnClientConnect)		(edict_t* pEntity, const char* pszName, const char* pszAddress, char szRejectReason[128]);

	void			(*pfnClientDisconnect)	(edict_t* pEntity);
	void			(*pfnClientKill)		(edict_t* pEntity);
	void			(*pfnClientPutInServer)	(edict_t* pEntity);
	void			(*pfnClientCommand)		(edict_t* pEntity);
	void			(*pfnClientUserInfoChanged)(edict_t* pEntity, char* infobuffer);

	void			(*pfnServerActivate)	(edict_t* pEdictList, int edictCount, int clientMax);
	void			(*pfnServerDeactivate)	();

	void			(*pfnPlayerPreThink)	(edict_t* pEntity);
	void			(*pfnPlayerPostThink)	(edict_t* pEntity);

	void			(*pfnStartFrame)		();
	void			(*pfnParmsNewLevel)		();
	void			(*pfnParmsChangeLevel)	();

	// Returns string describing current .dll.  E.g., TeamFotrress 2, Half-Life
	const char* (*pfnGetGameDescription)();

	// Notify dll about a player customization.
	void            (*pfnPlayerCustomization) (edict_t* pEntity, customization_t* pCustom);

	// Spectator funcs
	void			(*pfnSpectatorConnect)		(edict_t* pEntity);
	void			(*pfnSpectatorDisconnect)	(edict_t* pEntity);
	void			(*pfnSpectatorThink)		(edict_t* pEntity);

	// Notify game .dll that engine is going to shut down.  Allows mod authors to set a breakpoint.
	void			(*pfnSys_Error)			(const char* error_string);

	void			(*pfnPM_Move) (playermove_t* ppmove, qboolean server);
	void			(*pfnPM_Init) (playermove_t* ppmove);
	char			(*pfnPM_FindTextureType)(const char* name);
	void			(*pfnSetupVisibility)(edict_t* pViewEntity, edict_t* pClient, unsigned char** pvs, unsigned char** pas);
	void			(*pfnUpdateClientData) (const edict_t* ent, int sendweapons, clientdata_t* cd);
	int				(*pfnAddToFullPack)(entity_state_t* state, int e, edict_t* ent, edict_t* host, int hostflags, int player, unsigned char* pSet);
	void			(*pfnCreateBaseline) (int player, int eindex, entity_state_t* baseline, edict_t* entity, int playermodelindex, Vector* player_mins, Vector* player_maxs);
	void			(*pfnRegisterEncoders)	();
	int				(*pfnGetWeaponData)		(edict_t* player, weapon_data_t* info);

	void			(*pfnCmdStart)			(const edict_t* player, const usercmd_t* cmd, unsigned int random_seed);
	void			(*pfnCmdEnd)			(const edict_t* player);

	// Return 1 if the packet is valid.  Set response_buffer_size if you want to send a response packet.  Incoming, it holds the max
	//  size of the response_buffer, so you must zero it out if you choose not to respond.
	int				(*pfnConnectionlessPacket)	(const netadr_t* net_from, const char* args, char* response_buffer, int* response_buffer_size);

	// Enumerates player hulls.  Returns 0 if the hull number doesn't exist, 1 otherwise
	int				(*pfnGetHullBounds)	(int hullnumber, Vector* mins, Vector* maxs);

	// Create baselines for certain "unplaced" items.
	void			(*pfnCreateInstancedBaselines) ();

	// One of the pfnForceUnmodified files failed the consistency check for the specified player
	// Return 0 to allow the client to continue, 1 to force immediate disconnection ( with an optional disconnect message of up to 256 characters )
	int				(*pfnInconsistentFile)(const edict_t* player, const char* filename, char* disconnect_message);

	// The game .dll should return 1 if lag compensation should be allowed ( could also just set
	//  the sv_unlag cvar.
	// Most games right now should return 0, until client-side weapon prediction code is written
	//  and tested for them.
	int				(*pfnAllowLagCompensation)();
};

extern DLL_FUNCTIONS		gEntityInterface;

// Current version.
constexpr int NEW_DLL_FUNCTIONS_VERSION = 1;

struct NEW_DLL_FUNCTIONS
{
	// Called right before the object's memory is freed. 
	// Calls its destructor.
	void			(*pfnOnFreeEntPrivateData)(edict_t* pEnt);
	void			(*pfnGameShutdown)();
	int				(*pfnShouldCollide)(edict_t* pentTouched, edict_t* pentOther);
	void			(*pfnCvarValue)(const edict_t* pEnt, const char* value);
	void			(*pfnCvarValue2)(const edict_t* pEnt, int requestID, const char* cvarName, const char* value);
};

typedef int	(*NEW_DLL_FUNCTIONS_FN)(NEW_DLL_FUNCTIONS* pFunctionTable, int* interfaceVersion);

// Pointers will be null if the game DLL doesn't support this API.
extern NEW_DLL_FUNCTIONS	gNewDLLFunctions;

typedef int	(*APIFUNCTION)(DLL_FUNCTIONS* pFunctionTable, int interfaceVersion);
typedef int	(*APIFUNCTION2)(DLL_FUNCTIONS* pFunctionTable, int* interfaceVersion);
