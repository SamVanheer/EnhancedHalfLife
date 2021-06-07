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

#include "server_int.hpp"

extern bool gTouchDisabled;

int DispatchSpawn(edict_t* pent);
void DispatchThink(edict_t* pent);
void DispatchUse(edict_t* pentUsed, edict_t* pentOther);
void DispatchTouch(edict_t* pentTouched, edict_t* pentOther);
void DispatchBlocked(edict_t* pentBlocked, edict_t* pentOther);
void DispatchKeyValue(edict_t* pentKeyvalue, KeyValueData* pkvd);
void DispatchSave(edict_t* pent, SAVERESTOREDATA* pSaveData);
int  DispatchRestore(edict_t* pent, SAVERESTOREDATA* pSaveData, int globalEntity);
void DispatchObjectCollisionBox(edict_t* pent);

void SaveWriteFields(SAVERESTOREDATA* pSaveData, const char* pname, void* pBaseData, TYPEDESCRIPTION* pFields, int fieldCount);
void SaveReadFields(SAVERESTOREDATA* pSaveData, const char* pname, void* pBaseData, TYPEDESCRIPTION* pFields, int fieldCount);

void SaveGlobalState(SAVERESTOREDATA* pSaveData);
void RestoreGlobalState(SAVERESTOREDATA* pSaveData);
void ResetGlobalState();

void OnFreeEntPrivateData(edict_t* pEdict);
int ShouldCollide(edict_t* pentTouched, edict_t* pentOther);
void CvarValue(const edict_t* pEnt, const char* value);
void CvarValue2(const edict_t* pEnt, int requestID, const char* cvarName, const char* value);
