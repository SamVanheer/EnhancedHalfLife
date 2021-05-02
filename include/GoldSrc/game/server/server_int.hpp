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

#include "steam/steamtypes.h"
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
	/**
	*	@brief Initialize/shutdown the game (one-time call after loading of game .dll )
	*/
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

	/**
	*	@brief called when a player connects to a server
	*/
	qboolean(*pfnClientConnect)		(edict_t* pEntity, const char* pszName, const char* pszAddress, char szRejectReason[128]);

	/**
	*	@brief called when a player disconnects from a server
	*/
	void			(*pfnClientDisconnect)	(edict_t* pEntity);

	/**
	*	@brief Player entered the suicide command
	*/
	void			(*pfnClientKill)		(edict_t* pEntity);

	/**
	*	@brief called each time a player is spawned
	*/
	void			(*pfnClientPutInServer)	(edict_t* pEntity);

	/**
	*	@brief called each time a player uses a "cmd" command
	*/
	void			(*pfnClientCommand)		(edict_t* pEntity);

	/**
	*	@brief called after the player changes userinfo -
	*	gives dll a chance to modify it before it gets sent into the rest of the engine.
	*/
	void			(*pfnClientUserInfoChanged)(edict_t* pEntity, char* infobuffer);

	void			(*pfnServerActivate)	(edict_t* pEdictList, int edictCount, int clientMax);
	void			(*pfnServerDeactivate)	();

	/**
	*	@brief Called every frame before physics are run
	*/
	void			(*pfnPlayerPreThink)	(edict_t* pEntity);

	/**
	*	@brief Called every frame after physics are run
	*/
	void			(*pfnPlayerPostThink)	(edict_t* pEntity);

	void			(*pfnStartFrame)		();
	void			(*pfnParmsNewLevel)		();
	void			(*pfnParmsChangeLevel)	();

	/**
	*	@brief Returns the descriptive name of this .dll.  E.g., Half-Life, or Team Fortress 2
	*/
	const char* (*pfnGetGameDescription)();

	/**
	*	@brief A new player customization has been registered on the server
	*	UNDONE:  This only sets the # of frames of the spray can logo animation right now.
	*/
	void            (*pfnPlayerCustomization) (edict_t* pEntity, customization_t* pCustom);

	// Spectator funcs
	/**
	*	@brief A spectator has joined the game
	*/
	void			(*pfnSpectatorConnect)		(edict_t* pEntity);

	/**
	*	@brief A spectator has left the game
	*/
	void			(*pfnSpectatorDisconnect)	(edict_t* pEntity);

	/**
	*	@brief A spectator has sent a usercmd
	*/
	void			(*pfnSpectatorThink)		(edict_t* pEntity);

	/**
	*	@brief Notify game .dll that engine is going to shut down.  Allows mod authors to set a breakpoint.
	*/
	void			(*pfnSys_Error)			(const char* error_string);

	void			(*pfnPM_Move) (playermove_t* ppmove, qboolean server);
	void			(*pfnPM_Init) (playermove_t* ppmove);
	char			(*pfnPM_FindTextureType)(const char* name);

	/**
	*	@details A client can have a separate "view entity" indicating that his/her view should depend on the origin of that view entity.
	*	If that's the case, then pViewEntity will be non-nullptr and will be used.
	*	Otherwise, the current entity's origin is used.
	*	Either is offset by the view_ofs to get the eye position.
	*	From the eye position, we set up the PAS and PVS to use for filtering network messages to the client.
	*	At this point, we could override the actual PAS or PVS values, or use a different origin.
	*	NOTE: Do not cache the values of pas and pvs, as they depend on reusable memory in the engine, they are only good for this one frame
	*/
	void			(*pfnSetupVisibility)(edict_t* pViewEntity, edict_t* pClient, unsigned char** pvs, unsigned char** pas);

	/**
	*	@brief Data sent to current client only
	*	engine sets cd to 0 before calling.
	*/
	void			(*pfnUpdateClientData) (const edict_t* ent, int sendweapons, clientdata_t* cd);

	/**
	*	@brief Return true if the entity state has been filled in for the ent and the entity will be propagated to the client, false otherwise
	*	@details state is the server maintained copy of the state info that is transmitted to the client
	*	a MOD could alter values copied into state to send the "host" a different look for a particular entity update, etc.
	*	e and ent are the entity that is being added to the update, if true is returned
	*	host is the player's edict of the player whom we are sending the update to
	*	player is true if the ent/e is a player and false otherwise
	*	pSet is either the PAS or PVS that we previous set up.
	*	We can use it to ask the engine to filter the entity against the PAS or PVS.
	*	we could also use the pas/ pvs that we set in SetupVisibility, if we wanted to.
	*	Caching the value is valid in that case, but still only for the current frame
	*/
	int				(*pfnAddToFullPack)(entity_state_t* state, int e, edict_t* ent, edict_t* host, int hostflags, int player, unsigned char* pSet);

	/**
	*	@brief Creates baselines used for network encoding, especially for player data since players are not spawned until connect time.
	*/
	void			(*pfnCreateBaseline) (int player, int eindex, entity_state_t* baseline, edict_t* entity, int playermodelindex, Vector* player_mins, Vector* player_maxs);

	/**
	*	@brief Allows game .dll to override network encoding of certain types of entities and tweak values, etc.
	*/
	void			(*pfnRegisterEncoders)	();
	int				(*pfnGetWeaponData)		(edict_t* player, weapon_data_t* info);

	/**
	*	@brief We're about to run this usercmd for the specified player. We can set up groupinfo and masking here, etc.
	*	This is the time to examine the usercmd for anything extra. This call happens even if think does not.
	*/
	void			(*pfnCmdStart)			(const edict_t* player, const usercmd_t* cmd, unsigned int random_seed);

	/**
	*	@brief Each cmdstart is exactly matched with a cmd end, clean up any group trace flags, etc. here
	*/
	void			(*pfnCmdEnd)			(const edict_t* player);

	/**
	*	@brief  Return true if the packet is valid. Set response_buffer_size if you want to send a response packet.
	*	Incoming, it holds the max size of the response_buffer, so you must zero it out if you choose not to respond.
	*/
	int				(*pfnConnectionlessPacket)	(const netadr_t* net_from, const char* args, char* response_buffer, int* response_buffer_size);

	/**
	*	@brief Engine calls this to enumerate player collision hulls, for prediction. Return false if the hullnumber doesn't exist.
	*/
	int				(*pfnGetHullBounds)	(int hullnumber, Vector* mins, Vector* maxs);

	/**
	*	@brief Create pseudo-baselines for items that aren't placed in the map at spawn time,
	*	but which are likely to be created during play ( e.g., grenades, ammo packs, projectiles, corpses, etc. )
	*/
	void			(*pfnCreateInstancedBaselines) ();

	/**
	*	@brief One of the ENGINE_FORCE_UNMODIFIED files failed the consistency check for the specified player
	*	@return false to allow the client to continue, true to force immediate disconnection
	*	( with an optional disconnect message of up to 256 characters )
	*/
	int				(*pfnInconsistentFile)(const edict_t* player, const char* filename, char* disconnect_message);

	// The game .dll should return 1 if lag compensation should be allowed ( could also just set
	//  the sv_unlag cvar.
	// Most games right now should return 0, until client-side weapon prediction code is written
	//  and tested for them.
	/**
	*	@brief  The game .dll should return true if lag compensation should be allowed ( could also just set the sv_unlag cvar.)
	*	Most games right now should return false, until client-side weapon prediction code is written and tested for them
	*	( note you can predict weapons, but not do lag compensation, too, if you want.)
	*/
	int				(*pfnAllowLagCompensation)();
};

extern DLL_FUNCTIONS		gEntityInterface;

/**
*	@brief Current version.
*/
constexpr int NEW_DLL_FUNCTIONS_VERSION = 1;

struct NEW_DLL_FUNCTIONS
{
	/**
	*	@brief Called right before the object's memory is freed.
	*	Calls its destructor.
	*/
	void			(*pfnOnFreeEntPrivateData)(edict_t* pEnt);
	void			(*pfnGameShutdown)();
	int				(*pfnShouldCollide)(edict_t* pentTouched, edict_t* pentOther);
	void			(*pfnCvarValue)(const edict_t* pEnt, const char* value);
	void			(*pfnCvarValue2)(const edict_t* pEnt, int requestID, const char* cvarName, const char* value);
};

typedef int	(*NEW_DLL_FUNCTIONS_FN)(NEW_DLL_FUNCTIONS* pFunctionTable, int* interfaceVersion);

/**
*	@brief Pointers will be null if the game DLL doesn't support this API.
*/
extern NEW_DLL_FUNCTIONS	gNewDLLFunctions;

typedef int	(*APIFUNCTION)(DLL_FUNCTIONS* pFunctionTable, int interfaceVersion);
typedef int	(*APIFUNCTION2)(DLL_FUNCTIONS* pFunctionTable, int* interfaceVersion);
