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

#include "dll_functions.hpp"
#include "util.h"
#include "cbase.h"
#include "game.h"
#include "client.h"
#include "pm_shared.h"

#include "gamerules.h"

void SpectatorDummyFunction(edict_t*)
{
	//Nothing. Never called
}

DLL_FUNCTIONS gEntityInterface =
{
	GameDLLInit,				//pfnGameInit
	DispatchSpawn,				//pfnSpawn
	DispatchThink,				//pfnThink
	DispatchUse,				//pfnUse
	DispatchTouch,				//pfnTouch
	DispatchBlocked,			//pfnBlocked
	DispatchKeyValue,			//pfnKeyValue
	DispatchSave,				//pfnSave
	DispatchRestore,			//pfnRestore
	DispatchObjectCollsionBox,	//pfnAbsBox TODO typo

	SaveWriteFields,			//pfnSaveWriteFields
	SaveReadFields,				//pfnSaveReadFields

	SaveGlobalState,			//pfnSaveGlobalState
	RestoreGlobalState,			//pfnRestoreGlobalState
	ResetGlobalState,			//pfnResetGlobalState

	ClientConnect,				//pfnClientConnect
	ClientDisconnect,			//pfnClientDisconnect
	ClientKill,					//pfnClientKill
	ClientPutInServer,			//pfnClientPutInServer
	ClientCommand,				//pfnClientCommand
	ClientUserInfoChanged,		//pfnClientUserInfoChanged
	ServerActivate,				//pfnServerActivate
	ServerDeactivate,			//pfnServerDeactivate

	PlayerPreThink,				//pfnPlayerPreThink
	PlayerPostThink,			//pfnPlayerPostThink

	StartFrame,					//pfnStartFrame
	ParmsNewLevel,				//pfnParmsNewLevel
	ParmsChangeLevel,			//pfnParmsChangeLevel

	GetGameDescription,         //pfnGetGameDescription    Returns string describing current .dll game.
	PlayerCustomization,        //pfnPlayerCustomization   Notifies .dll of new customization for player.

	SpectatorDummyFunction,		//pfnSpectatorConnect      Called when spectator joins server
	SpectatorDummyFunction,		//pfnSpectatorDisconnect   Called when spectator leaves the server
	SpectatorDummyFunction,		//pfnSpectatorThink        Called when spectator sends a command packet (usercmd_t)

	Sys_Error,					//pfnSys_Error				Called when engine has encountered an error

	PM_Move,					//pfnPM_Move
	PM_Init,					//pfnPM_Init				Server version of player movement initialization
	TEXTURETYPE_Find,			//pfnPM_FindTextureType

	SetupVisibility,			//pfnSetupVisibility        Set up PVS and PAS for networking for this client
	UpdateClientData,			//pfnUpdateClientData       Set up data sent only to specific client
	AddToFullPack,				//pfnAddToFullPack
	CreateBaseline,				//pfnCreateBaseline			Tweak entity baseline for network encoding, allows setup of player baselines, too.
	RegisterEncoders,			//pfnRegisterEncoders		Callbacks for network encoding
	GetWeaponData,				//pfnGetWeaponData
	CmdStart,					//pfnCmdStart
	CmdEnd,						//pfnCmdEnd
	ConnectionlessPacket,		//pfnConnectionlessPacket
	GetHullBounds,				//pfnGetHullBounds
	CreateInstancedBaselines,   //pfnCreateInstancedBaselines
	InconsistentFile,			//pfnInconsistentFile
	AllowLagCompensation,		//pfnAllowLagCompensation
};

NEW_DLL_FUNCTIONS gNewDLLFunctions =
{
	OnFreeEntPrivateData,		//pfnOnFreeEntPrivateData
	GameDLLShutdown,
};

int DispatchSpawn(edict_t* pent)
{
	CBaseEntity* pEntity = (CBaseEntity*)GET_PRIVATE(pent);

	if (pEntity)
	{
		// Initialize these or entities who don't link to the world won't have anything in here
		pEntity->pev->absmin = pEntity->pev->origin - Vector(1, 1, 1);
		pEntity->pev->absmax = pEntity->pev->origin + Vector(1, 1, 1);

		pEntity->Spawn();

		// Try to get the pointer again, in case the spawn function deleted the entity.
		// UNDONE: Spawn() should really return a code to ask that the entity be deleted, but
		// that would touch too much code for me to do that right now.
		pEntity = (CBaseEntity*)GET_PRIVATE(pent);

		if (pEntity)
		{
			if (g_pGameRules && !g_pGameRules->IsAllowedToSpawn(pEntity))
				return -1;	// return that this entity should be deleted
			if (pEntity->pev->flags & FL_KILLME)
				return -1;
		}


		// Handle global stuff here
		if (pEntity && !IsStringNull(pEntity->pev->globalname))
		{
			const globalentity_t* pGlobal = gGlobalState.EntityFromTable(pEntity->pev->globalname);
			if (pGlobal)
			{
				// Already dead? delete
				if (pGlobal->state == GlobalEntState::Dead)
					return -1;
				else if (!AreStringsEqual(STRING(gpGlobals->mapname), pGlobal->levelName))
					pEntity->MakeDormant();	// Hasn't been moved to this level yet, wait but stay alive
				// In this level & not dead, continue on as normal
			}
			else
			{
				// Spawned entities default to 'On'
				gGlobalState.EntityAdd(pEntity->pev->globalname, gpGlobals->mapname, GlobalEntState::On);
				//				ALERT( at_console, "Added global entity %s (%s)\n", STRING(pEntity->pev->classname), STRING(pEntity->pev->globalname) );
			}
		}

	}

	return 0;
}

void DispatchThink(edict_t* pent)
{
	CBaseEntity* pEntity = (CBaseEntity*)GET_PRIVATE(pent);
	if (pEntity)
	{
		if (IsBitSet(pEntity->pev->flags, FL_DORMANT))
			ALERT(at_error, "Dormant entity %s is thinking!!\n", STRING(pEntity->pev->classname));

		pEntity->Think();
	}
}

void DispatchUse(edict_t* pentUsed, edict_t* pentOther)
{
	CBaseEntity* pEntity = (CBaseEntity*)GET_PRIVATE(pentUsed);
	CBaseEntity* pOther = (CBaseEntity*)GET_PRIVATE(pentOther);

	if (pEntity && !(pEntity->pev->flags & FL_KILLME))
		pEntity->Use(pOther, pOther, USE_TOGGLE, 0);
}

// HACKHACK -- this is a hack to keep the node graph entity from "touching" things (like triggers)
// while it builds the graph
bool gTouchDisabled = false;

void DispatchTouch(edict_t* pentTouched, edict_t* pentOther)
{
	if (gTouchDisabled)
		return;

	CBaseEntity* pEntity = (CBaseEntity*)GET_PRIVATE(pentTouched);
	CBaseEntity* pOther = (CBaseEntity*)GET_PRIVATE(pentOther);

	if (pEntity && pOther && !((pEntity->pev->flags | pOther->pev->flags) & FL_KILLME))
		pEntity->Touch(pOther);
}

void DispatchBlocked(edict_t* pentBlocked, edict_t* pentOther)
{
	CBaseEntity* pEntity = (CBaseEntity*)GET_PRIVATE(pentBlocked);
	CBaseEntity* pOther = (CBaseEntity*)GET_PRIVATE(pentOther);

	if (pEntity)
		pEntity->Blocked(pOther);
}

void SV_NewMapStarted()
{
	ClearStringPool();
}

void DispatchKeyValue(edict_t* pentKeyvalue, KeyValueData* pkvd)
{
	if (!pkvd || !pentKeyvalue)
		return;

	if (g_IsStartingNewMap)
	{
		g_IsStartingNewMap = false;
		SV_NewMapStarted();
	}

	EntvarsKeyvalue(VARS(pentKeyvalue), pkvd);

	// If the key was an entity variable, or there's no class set yet, don't look for the object, it may
	// not exist yet.
	if (pkvd->fHandled || pkvd->szClassName == nullptr)
		return;

	// Get the actualy entity object
	CBaseEntity* pEntity = (CBaseEntity*)GET_PRIVATE(pentKeyvalue);

	if (!pEntity)
		return;

	pEntity->KeyValue(pkvd);
}

void DispatchSave(edict_t* pent, SAVERESTOREDATA* pSaveData)
{
	gpGlobals->time = pSaveData->time;

	CBaseEntity* pEntity = (CBaseEntity*)GET_PRIVATE(pent);

	if (pEntity && pSaveData)
	{
		ENTITYTABLE* pTable = &pSaveData->pTable[pSaveData->currentIndex];

		if (pTable->pent != pent)
			ALERT(at_error, "ENTITY TABLE OR INDEX IS WRONG!!!!\n");

		if (pEntity->ObjectCaps() & FCAP_DONT_SAVE)
			return;

		// These don't use ltime & nextthink as times really, but we'll fudge around it.
		if (pEntity->pev->movetype == MOVETYPE_PUSH)
		{
			float delta = pEntity->pev->nextthink - pEntity->pev->ltime;
			pEntity->pev->ltime = gpGlobals->time;
			pEntity->pev->nextthink = pEntity->pev->ltime + delta;
		}

		pTable->location = pSaveData->size;		// Remember entity position for file I/O
		pTable->classname = pEntity->pev->classname;	// Remember entity class for respawn

		CSave saveHelper(pSaveData);
		pEntity->Save(saveHelper);

		pTable->size = pSaveData->size - pTable->location;	// Size of entity block is data size written to block
	}
}

int DispatchRestore(edict_t* pent, SAVERESTOREDATA* pSaveData, int globalEntity)
{
	gpGlobals->time = pSaveData->time;

	CBaseEntity* pEntity = (CBaseEntity*)GET_PRIVATE(pent);

	if (pEntity && pSaveData)
	{
		entvars_t tmpVars;
		Vector oldOffset;

		CRestore restoreHelper(pSaveData);
		if (globalEntity)
		{
			CRestore tmpRestore(pSaveData);
			tmpRestore.PrecacheMode(false);
			tmpRestore.ReadEntVars("ENTVARS", &tmpVars);

			// HACKHACK - reset the save pointers, we're going to restore for real this time
			pSaveData->size = pSaveData->pTable[pSaveData->currentIndex].location;
			pSaveData->pCurrentData = pSaveData->pBaseData + pSaveData->size;
			// -------------------


			const globalentity_t* pGlobal = gGlobalState.EntityFromTable(tmpVars.globalname);

			// Don't overlay any instance of the global that isn't the latest
			// pSaveData->szCurrentMapName is the level this entity is coming from
			// pGlobla->levelName is the last level the global entity was active in.
			// If they aren't the same, then this global update is out of date.
			if (!AreStringsEqual(pSaveData->szCurrentMapName, pGlobal->levelName))
				return 0;

			// Compute the new global offset
			oldOffset = pSaveData->vecLandmarkOffset;
			CBaseEntity* pNewEntity = FindGlobalEntity(tmpVars.classname, tmpVars.globalname);
			if (pNewEntity)
			{
				//				ALERT( at_console, "Overlay %s with %s\n", STRING(pNewEntity->pev->classname), STRING(tmpVars.classname) );
								// Tell the restore code we're overlaying a global entity from another level
				restoreHelper.SetGlobalMode(1);	// Don't overwrite global fields
				pSaveData->vecLandmarkOffset = (pSaveData->vecLandmarkOffset - pNewEntity->pev->mins) + tmpVars.mins;
				pEntity = pNewEntity;// we're going to restore this data OVER the old entity
				pent = ENT(pEntity->pev);
				// Update the global table to say that the global definition of this entity should come from this level
				gGlobalState.EntityUpdate(pEntity->pev->globalname, gpGlobals->mapname);
			}
			else
			{
				// This entity will be freed automatically by the engine.  If we don't do a restore on a matching entity (below)
				// or call EntityUpdate() to move it to this level, we haven't changed global state at all.
				return 0;
			}

		}

		if (pEntity->ObjectCaps() & FCAP_MUST_SPAWN)
		{
			pEntity->Restore(restoreHelper);
			pEntity->Spawn();
		}
		else
		{
			pEntity->Restore(restoreHelper);
			pEntity->Precache();
		}

		// Again, could be deleted, get the pointer again.
		pEntity = (CBaseEntity*)GET_PRIVATE(pent);

#if 0
		if (pEntity && pEntity->pev->globalname && globalEntity)
		{
			ALERT(at_console, "Global %s is %s\n", STRING(pEntity->pev->globalname), STRING(pEntity->pev->model));
		}
#endif

		// Is this an overriding global entity (coming over the transition), or one restoring in a level
		if (globalEntity)
		{
			//			ALERT( at_console, "After: %f %f %f %s\n", pEntity->pev->origin.x, pEntity->pev->origin.y, pEntity->pev->origin.z, STRING(pEntity->pev->model) );
			pSaveData->vecLandmarkOffset = oldOffset;
			if (pEntity)
			{
				UTIL_SetOrigin(pEntity->pev, pEntity->pev->origin);
				pEntity->OverrideReset();
			}
		}
		else if (pEntity && !IsStringNull(pEntity->pev->globalname))
		{
			const globalentity_t* pGlobal = gGlobalState.EntityFromTable(pEntity->pev->globalname);
			if (pGlobal)
			{
				// Already dead? delete
				if (pGlobal->state == GlobalEntState::Dead)
					return -1;
				else if (!AreStringsEqual(STRING(gpGlobals->mapname), pGlobal->levelName))
				{
					pEntity->MakeDormant();	// Hasn't been moved to this level yet, wait but stay alive
				}
				// In this level & not dead, continue on as normal
			}
			else
			{
				ALERT(at_error, "Global Entity %s (%s) not in table!!!\n", STRING(pEntity->pev->globalname), STRING(pEntity->pev->classname));
				// Spawned entities default to 'On'
				gGlobalState.EntityAdd(pEntity->pev->globalname, gpGlobals->mapname, GlobalEntState::On);
			}
		}
	}
	return 0;
}

void DispatchObjectCollsionBox(edict_t* pent)
{
	CBaseEntity* pEntity = (CBaseEntity*)GET_PRIVATE(pent);
	if (pEntity)
	{
		pEntity->SetObjectCollisionBox();
	}
	else
		SetObjectCollisionBox(&pent->v);
}

void SaveWriteFields(SAVERESTOREDATA* pSaveData, const char* pname, void* pBaseData, TYPEDESCRIPTION* pFields, int fieldCount)
{
	CSave saveHelper(pSaveData);
	saveHelper.WriteFields(pname, pBaseData, pFields, fieldCount);
}

void SaveReadFields(SAVERESTOREDATA* pSaveData, const char* pname, void* pBaseData, TYPEDESCRIPTION* pFields, int fieldCount)
{
	CRestore restoreHelper(pSaveData);
	restoreHelper.ReadFields(pname, pBaseData, pFields, fieldCount);
}

void OnFreeEntPrivateData(edict_t* pEdict)
{
	if (pEdict && pEdict->pvPrivateData)
	{
		auto entity = reinterpret_cast<CBaseEntity*>(pEdict->pvPrivateData);

		delete entity;

		pEdict->pvPrivateData = nullptr;
	}
}
