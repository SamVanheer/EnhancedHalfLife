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

#include "event_flags.h"

// Must be provided by user of this code
extern enginefuncs_t g_engfuncs;

// The actual engine callbacks
#define GETPLAYERUSERID (*g_engfuncs.pfnGetPlayerUserId)
#define PRECACHE_MODEL	(*g_engfuncs.pfnPrecacheModel)
#define PRECACHE_SOUND	(*g_engfuncs.pfnPrecacheSound)
#define PRECACHE_GENERIC	(*g_engfuncs.pfnPrecacheGeneric)
#define MODEL_INDEX		(*g_engfuncs.pfnModelIndex)
#define MODEL_FRAMES	(*g_engfuncs.pfnModelFrames)
#define CHANGE_LEVEL	(*g_engfuncs.pfnChangeLevel)
#define MAKE_STATIC		(*g_engfuncs.pfnMakeStatic)
#define DROP_TO_FLOOR	(*g_engfuncs.pfnDropToFloor)
#define SERVER_COMMAND	(*g_engfuncs.pfnServerCommand)
#define SERVER_EXECUTE	(*g_engfuncs.pfnServerExecute)
#define CLIENT_COMMAND	(*g_engfuncs.pfnClientCommand)
#define LIGHT_STYLE		(*g_engfuncs.pfnLightStyle)
#define DECAL_INDEX		(*g_engfuncs.pfnDecalIndex)
#define CRC32_INIT           (*g_engfuncs.pfnCRC32_Init)
#define CRC32_PROCESS_BUFFER (*g_engfuncs.pfnCRC32_ProcessBuffer)
#define CRC32_PROCESS_BYTE   (*g_engfuncs.pfnCRC32_ProcessByte)
#define CRC32_FINAL          (*g_engfuncs.pfnCRC32_Final)
#define RANDOM_LONG		(*g_engfuncs.pfnRandomLong)
#define RANDOM_FLOAT	(*g_engfuncs.pfnRandomFloat)
#define GETPLAYERAUTHID	(*g_engfuncs.pfnGetPlayerAuthId)
#define MESSAGE_END		(*g_engfuncs.pfnMessageEnd)
#define WRITE_BYTE		(*g_engfuncs.pfnWriteByte)
#define WRITE_CHAR		(*g_engfuncs.pfnWriteChar)
#define WRITE_SHORT		(*g_engfuncs.pfnWriteShort)
#define WRITE_LONG		(*g_engfuncs.pfnWriteLong)
#define WRITE_ANGLE		(*g_engfuncs.pfnWriteAngle)
#define WRITE_COORD		(*g_engfuncs.pfnWriteCoord)
#define WRITE_STRING	(*g_engfuncs.pfnWriteString)
#define WRITE_ENTITY	(*g_engfuncs.pfnWriteEntity)
#define CVAR_REGISTER	(*g_engfuncs.pfnCVarRegister)
#define CVAR_GET_FLOAT	(*g_engfuncs.pfnCVarGetFloat)
#define CVAR_GET_STRING	(*g_engfuncs.pfnCVarGetString)
#define CVAR_SET_FLOAT	(*g_engfuncs.pfnCVarSetFloat)
#define CVAR_SET_STRING	(*g_engfuncs.pfnCVarSetString)
#define CVAR_GET_POINTER (*g_engfuncs.pfnCVarGetPointer)
#define ALERT			(*g_engfuncs.pfnAlertMessage)
#define REG_USER_MSG				(*g_engfuncs.pfnRegUserMsg)
#define FUNCTION_FROM_NAME			(*g_engfuncs.pfnFunctionFromName)
#define NAME_FOR_FUNCTION			(*g_engfuncs.pfnNameForFunction)
#define TRACE_TEXTURE				(*g_engfuncs.pfnTraceTexture)
#define CLIENT_PRINTF				(*g_engfuncs.pfnClientPrintf)
#define CMD_ARGS					(*g_engfuncs.pfnCmd_Args)
#define CMD_ARGC					(*g_engfuncs.pfnCmd_Argc)
#define CMD_ARGV					(*g_engfuncs.pfnCmd_Argv)
#define SET_VIEW				(*g_engfuncs.pfnSetView)
#define SET_CROSSHAIRANGLE		(*g_engfuncs.pfnCrosshairAngle)
#define COMPARE_FILE_TIME		(*g_engfuncs.pfnCompareFileTime)
#define IS_MAP_VALID			(*g_engfuncs.pfnIsMapValid)
#define NUMBER_OF_ENTITIES		(*g_engfuncs.pfnNumberOfEntities)
#define IS_DEDICATED_SERVER		(*g_engfuncs.pfnIsDedicatedServer)

#define PRECACHE_EVENT			(*g_engfuncs.pfnPrecacheEvent)

#define DELTA_SET				( *g_engfuncs.pfnDeltaSetField )
#define DELTA_UNSET				( *g_engfuncs.pfnDeltaUnsetField )
#define DELTA_ADDENCODER		( *g_engfuncs.pfnDeltaAddEncoder )
#define ENGINE_CURRENT_PLAYER   ( *g_engfuncs.pfnGetCurrentPlayer )

#define DELTA_FINDFIELD			( *g_engfuncs.pfnDeltaFindField )
#define DELTA_SETBYINDEX		( *g_engfuncs.pfnDeltaSetFieldByIndex )
#define DELTA_UNSETBYINDEX		( *g_engfuncs.pfnDeltaUnsetFieldByIndex )

#define ENGINE_FORCE_UNMODIFIED	( *g_engfuncs.pfnForceUnmodified )
