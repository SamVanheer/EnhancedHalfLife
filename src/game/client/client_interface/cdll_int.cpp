/***
*
*	Copyright (c) 1999, Valve LLC. All rights reserved.
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

/**
*	@file
*
*	this implementation handles the linking of the engine to the DLL
*/

#include "hud.h"
#include "cl_util.h"
#include "client_int.hpp"
#include "netadr.h"
#include "interface.h"
#include "sound/materials.hpp"
//#include "vgui_schememanager.h"

#include "pm_shared.h"

#include "vgui_int.h"

#include "Platform.h"
#include "Exports.h"

#include "tri.h"
#include "vgui_TeamFortressViewport.h"
#include "shared_interface/shared_interface.hpp"

cl_enginefunc_t gEngfuncs;
TeamFortressViewport* gViewPort = nullptr;

#include "particleman.h"
CSysModule* g_hParticleManModule = nullptr;

void CL_LoadParticleMan();
void CL_UnloadParticleMan();

void InitInput();
void EV_HookEvents();
void IN_Commands();

int DLLEXPORT HUD_GetHullBounds(int hullnumber, Vector* mins, Vector* maxs)
{
	return Shared_GetHullBounds(hullnumber, *mins, *maxs);
}

int	DLLEXPORT HUD_ConnectionlessPacket(const netadr_t* net_from, const char* args, char* response_buffer, int* response_buffer_size)
{
	// Parse stuff from args
	int max_buffer_size = *response_buffer_size;

	// Zero it out since we aren't going to respond.
	// If we wanted to response, we'd write data into response_buffer
	*response_buffer_size = 0;

	// Since we don't listen for anything here, just respond that it's a bogus message
	// If we didn't reject the message, we'd return 1 for success instead.
	return false;
}

void DLLEXPORT HUD_PlayerMoveInit(playermove_t* ppmove)
{
	PM_Init(ppmove);
}

char DLLEXPORT HUD_PlayerMoveTexture(char* name)
{
	return TEXTURETYPE_Find(name);
}

void DLLEXPORT HUD_PlayerMove(playermove_t* ppmove, int server)
{
	PM_Move(ppmove, server);
}

int DLLEXPORT Initialize(cl_enginefunc_t* pEnginefuncs, int iVersion)
{
	gEngfuncs = *pEnginefuncs;

	if (iVersion != CLDLL_INTERFACE_VERSION)
		return false;

	memcpy(&gEngfuncs, pEnginefuncs, sizeof(cl_enginefunc_t));

	EV_HookEvents();
	CL_LoadParticleMan();

	if (!FileSystem_LoadFileSystem())
	{
		return false;
	}

	// get tracker interface, if any
	return true;
}

int DLLEXPORT HUD_VidInit()
{
	gHUD.VidInit();

	VGui_Startup();

	return true;
}

void DLLEXPORT HUD_Init()
{
	InitInput();
	gHUD.Init();
	Scheme_Init();
}

int DLLEXPORT HUD_Redraw(float time, int intermission)
{
	gHUD.Redraw(time, intermission);

	return true;
}

int DLLEXPORT HUD_UpdateClientData(client_data_t* pcldata, float flTime)
{
	IN_Commands();

	return gHUD.UpdateClientData(pcldata, flTime);
}

void DLLEXPORT HUD_Reset()
{
	gHUD.VidInit();
}

void DLLEXPORT HUD_Frame(double time)
{
	GetClientVoiceMgr()->Frame(time);
}

void DLLEXPORT HUD_VoiceStatus(int entindex, qboolean bTalking)
{
	GetClientVoiceMgr()->UpdateSpeakerStatus(entindex, bTalking);
}

void DLLEXPORT HUD_DirectorMessage(int iSize, void* pbuf)
{
	gHUD.m_Spectator.DirectorMessage(iSize, pbuf);
}

void CL_UnloadParticleMan()
{
	Sys_UnloadModule(g_hParticleManModule);

	g_pParticleMan = nullptr;
	g_hParticleManModule = nullptr;
}

void CL_LoadParticleMan()
{
	char szPDir[512];

	if (gEngfuncs.COM_ExpandFilename(PARTICLEMAN_DLLNAME.data(), szPDir, sizeof(szPDir)) == false)
	{
		g_pParticleMan = nullptr;
		g_hParticleManModule = nullptr;
		return;
	}

	g_hParticleManModule = Sys_LoadModule(szPDir);
	CreateInterfaceFn particleManFactory = Sys_GetFactory(g_hParticleManModule);

	if (particleManFactory == nullptr)
	{
		g_pParticleMan = nullptr;
		g_hParticleManModule = nullptr;
		return;
	}

	g_pParticleMan = (IParticleMan*)particleManFactory(PARTICLEMAN_INTERFACE.data(), nullptr);

	if (g_pParticleMan)
	{
		g_pParticleMan->SetUp(&gEngfuncs);

		// Add custom particle classes here BEFORE calling anything else or you will die.
		g_pParticleMan->AddCustomParticleClassSize(sizeof(CBaseParticle));
	}
}

extern "C" void DLLEXPORT F(void* pv)
{
	cldll_func_t* pcldll_func = (cldll_func_t*)pv;

	cldll_func_t cldll_func =
	{
	Initialize,
	HUD_Init,
	HUD_VidInit,
	HUD_Redraw,
	HUD_UpdateClientData,
	HUD_Reset,
	HUD_PlayerMove,
	HUD_PlayerMoveInit,
	HUD_PlayerMoveTexture,
	IN_ActivateMouse,
	IN_DeactivateMouse,
	IN_MouseEvent,
	IN_ClearStates,
	IN_Accumulate,
	CL_CreateMove,
	CL_IsThirdPerson,
	CL_CameraOffset,
	KB_Find,
	CAM_Think,
	V_CalcRefdef,
	HUD_AddEntity,
	HUD_CreateEntities,
	HUD_DrawNormalTriangles,
	HUD_DrawTransparentTriangles,
	HUD_StudioEvent,
	HUD_PostRunCmd,
	HUD_Shutdown,
	HUD_TxferLocalOverrides,
	HUD_ProcessPlayerState,
	HUD_TxferPredictionData,
	Demo_ReadBuffer,
	HUD_ConnectionlessPacket,
	HUD_GetHullBounds,
	HUD_Frame,
	HUD_Key_Event,
	HUD_TempEntUpdate,
	HUD_GetUserEntity,
	HUD_VoiceStatus,
	HUD_DirectorMessage,
	HUD_GetStudioModelInterface,
	HUD_ChatInputPosition,
	};

	*pcldll_func = cldll_func;
}

#include "IGameClientExports.h"

/**
*	@brief Exports functions that are used by the gameUI for UI dialogs
*/
class CClientExports : public IGameClientExports
{
public:
	// returns the name of the server the user is connected to, if any
	const char* GetServerHostName() override
	{
		/*if (gViewPortInterface)
		{
			return gViewPortInterface->GetServerName();
		}*/
		return "";
	}

	// ingame voice manipulation
	bool IsPlayerGameVoiceMuted(int playerIndex) override
	{
		if (GetClientVoiceMgr())
			return GetClientVoiceMgr()->IsPlayerBlocked(playerIndex);
		return false;
	}

	void MutePlayerGameVoice(int playerIndex) override
	{
		if (GetClientVoiceMgr())
		{
			GetClientVoiceMgr()->SetPlayerBlockedState(playerIndex, true);
		}
	}

	void UnmutePlayerGameVoice(int playerIndex) override
	{
		if (GetClientVoiceMgr())
		{
			GetClientVoiceMgr()->SetPlayerBlockedState(playerIndex, false);
		}
	}
};

EXPOSE_SINGLE_INTERFACE(CClientExports, IGameClientExports, GAMECLIENTEXPORTS_INTERFACE_VERSION.data());
