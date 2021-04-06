#pragma once

#include "archtypes.h"     // DAL
#include "netadr.h"

#ifndef _WIN32
#include "enums.h"
#endif

/**
*	@brief Functions exported by the client .dll
*/
typedef struct
{
	int (*pInitFunc)(struct cl_enginefuncs_s*, int);
	void (*pHudInitFunc)();
	int (*pHudVidInitFunc)();
	int (*pHudRedrawFunc)(float, int);
	int (*pHudUpdateClientDataFunc)(struct client_data_s*, float);
	void (*pHudResetFunc)();
	void (*pClientMove)(struct playermove_s* ppmove, qboolean server);
	void (*pClientMoveInit)(struct playermove_s* ppmove);
	char (*pClientTextureType)(char* name);
	void (*pIN_ActivateMouse)();
	void (*pIN_DeactivateMouse)();
	void (*pIN_MouseEvent)(int mstate);
	void (*pIN_ClearStates)();
	void (*pIN_Accumulate)();
	void (*pCL_CreateMove)(float frametime, struct usercmd_s* cmd, int active);
	int (*pCL_IsThirdPerson)();
	void (*pCL_GetCameraOffsets)(float* ofs);
	struct kbutton_s* (*pFindKey)(const char* name);
	void (*pCamThink)();
	void (*pCalcRefdef)(struct ref_params_s* pparams);
	int	(*pAddEntity)(int type, struct cl_entity_s* ent, const char* modelname);
	void (*pCreateEntities)();
	void (*pDrawNormalTriangles)();
	void (*pDrawTransparentTriangles)();
	void (*pStudioEvent)(const struct mstudioevent_s* event, const struct cl_entity_s* entity);
	void (*pPostRunCmd) (struct local_state_s* from, struct local_state_s* to, struct usercmd_s* cmd, int runfuncs, double time, unsigned int random_seed);
	void (*pShutdown)();
	void (*pTxferLocalOverrides)(struct entity_state_s* state, const struct clientdata_s* client);
	void (*pProcessPlayerState)(struct entity_state_s* dst, const struct entity_state_s* src);
	void (*pTxferPredictionData)(struct entity_state_s* ps, const struct entity_state_s* pps, struct clientdata_s* pcd, const struct clientdata_s* ppcd,
		struct weapon_data_s* wd, const struct weapon_data_s* pwd);
	void (*pReadDemoBuffer)(int size, unsigned char* buffer);
	int (*pConnectionlessPacket)(const struct netadr_s* net_from, const char* args, char* response_buffer, int* response_buffer_size);
	int	(*pGetHullBounds)(int hullnumber, float* mins, float* maxs);
	void (*pHudFrame)(double);
	int (*pKeyEvent)(int eventcode, int keynum, const char* pszCurrentBinding);
	void (*pTempEntUpdate)(double frametime, double client_time, double cl_gravity, struct tempent_s** ppTempEntFree, struct tempent_s** ppTempEntActive,
		int (*Callback_AddVisibleEntity)(struct cl_entity_s* pEntity), void (*Callback_TempEntPlaySound)(struct tempent_s* pTemp, float damp));
	struct cl_entity_s* (*pGetUserEntity)(int index);
	void (*pVoiceStatus)(int entindex, qboolean bTalking);		// Possibly null on old client dlls.
	void (*pDirectorMessage)(int iSize, void* pbuf);	// Possibly null on old client dlls.
	int (*pStudioInterface)(int version, struct r_studio_interface_s** ppinterface, struct engine_studio_api_s* pstudio);	// Not used by all clients
	void (*pChatInputPosition)(int* x, int* y);	// Not used by all clients
	int (*pGetPlayerTeam)(int iplayer); // Not used by all clients
	void* (*pClientFactory)();	// this should be CreateInterfaceFn but that means including interface.h
								// which is a C++ file and some of the client files a C only... 
								// so we return a void * which we then do a typecast on later.
} cldll_func_t;
