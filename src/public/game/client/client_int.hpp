#pragma once

#include "archtypes.h"     // DAL
#include "netadr.h"

struct cl_enginefunc_t;
struct cl_entity_t;
struct client_data_t;
struct clientdata_t;
struct engine_studio_api_t;
struct entity_state_t;
struct kbutton_t;
struct local_state_t;
struct mstudioevent_t;
struct netadr_t;
struct playermove_t;
struct r_studio_interface_t;
struct ref_params_t;
struct TEMPENTITY;
struct usercmd_t;
struct weapon_data_t;

/**
*	@brief Functions exported by the client .dll
*/
struct cldll_func_t
{
	int (*pInitFunc)(cl_enginefunc_t*, int);
	void (*pHudInitFunc)();
	int (*pHudVidInitFunc)();
	int (*pHudRedrawFunc)(float, int);
	int (*pHudUpdateClientDataFunc)(client_data_t*, float);
	void (*pHudResetFunc)();
	void (*pClientMove)(playermove_t* ppmove, qboolean server);
	void (*pClientMoveInit)(playermove_t* ppmove);
	char (*pClientTextureType)(char* name);
	void (*pIN_ActivateMouse)();
	void (*pIN_DeactivateMouse)();
	void (*pIN_MouseEvent)(int mstate);
	void (*pIN_ClearStates)();
	void (*pIN_Accumulate)();
	void (*pCL_CreateMove)(float frametime, usercmd_t* cmd, int active);
	int (*pCL_IsThirdPerson)();
	void (*pCL_GetCameraOffsets)(Vector* ofs);
	kbutton_t* (*pFindKey)(const char* name);
	void (*pCamThink)();
	void (*pCalcRefdef)(ref_params_t* pparams);
	int	(*pAddEntity)(int type, cl_entity_t* ent, const char* modelname);
	void (*pCreateEntities)();
	void (*pDrawNormalTriangles)();
	void (*pDrawTransparentTriangles)();
	void (*pStudioEvent)(const mstudioevent_t* event, const cl_entity_t* entity);
	void (*pPostRunCmd) (local_state_t* from, local_state_t* to, usercmd_t* cmd, int runfuncs, double time, unsigned int random_seed);
	void (*pShutdown)();
	void (*pTxferLocalOverrides)(entity_state_t* state, const clientdata_t* client);
	void (*pProcessPlayerState)(entity_state_t* dst, const entity_state_t* src);
	void (*pTxferPredictionData)(entity_state_t* ps, const entity_state_t* pps, clientdata_t* pcd, const clientdata_t* ppcd,
		weapon_data_t* wd, const weapon_data_t* pwd);
	void (*pReadDemoBuffer)(int size, unsigned char* buffer);
	int (*pConnectionlessPacket)(const netadr_t* net_from, const char* args, char* response_buffer, int* response_buffer_size);
	int	(*pGetHullBounds)(int hullnumber, float* mins, float* maxs);
	void (*pHudFrame)(double);
	int (*pKeyEvent)(int eventcode, int keynum, const char* pszCurrentBinding);
	void (*pTempEntUpdate)(double frametime, double client_time, double cl_gravity, TEMPENTITY** ppTempEntFree, TEMPENTITY** ppTempEntActive,
		int (*Callback_AddVisibleEntity)(cl_entity_t* pEntity), void (*Callback_TempEntPlaySound)(TEMPENTITY* pTemp, float damp));
	cl_entity_t* (*pGetUserEntity)(int index);
	void (*pVoiceStatus)(int entindex, qboolean bTalking);		// Possibly null on old client dlls.
	void (*pDirectorMessage)(int iSize, void* pbuf);	// Possibly null on old client dlls.
	int (*pStudioInterface)(int version, r_studio_interface_t** ppinterface, engine_studio_api_t* pstudio);	// Not used by all clients
	void (*pChatInputPosition)(int* x, int* y);	// Not used by all clients
	int (*pGetPlayerTeam)(int iplayer); // Not used by all clients
	void* (*pClientFactory)();	// this should be CreateInterfaceFn but that means including interface.h
								// which is a C++ file and some of the client files a C only... 
								// so we return a void * which we then do a typecast on later.
};
