#pragma once

#include <cstdint>

#include "Platform.h"
#include "const.h"
#include "netadr.h"
#include "Sequence.h"

struct cl_entity_t;
struct client_sprite_t;
struct client_textmessage_t;
struct con_nprint_t;
struct cvar_t;
struct demo_api_t;
struct edict_t;
struct efx_api_t;
struct event_api_t;
struct event_args_t;
struct hud_player_info_t;
struct IVoiceTweak;
struct model_t;
struct net_api_t;
struct pmtrace_t;
struct screenfade_t;
struct SCREENINFO;
struct sequenceEntry;
struct triangleapi_t;
struct wrect_t;

constexpr int MAX_ALIAS_NAME = 32;

struct cmdalias_t
{
	cmdalias_t* next;
	char	name[MAX_ALIAS_NAME];
	char* value;
};

struct Point
{
	long x;
	long y;
};

typedef int HSPRITE;	//!< handle to a graphic

typedef int (*pfnUserMsgHook)(const char* pszName, int iSize, void* pbuf);

/**
*	@brief Functions exported by the engine
*/
struct cl_enginefunc_t
{
	HSPRITE(*pfnSPR_Load)(const char* szPicName);
	int (*pfnSPR_Frames)(HSPRITE hPic);
	int (*pfnSPR_Height)(HSPRITE hPic, int frame);
	int (*pfnSPR_Width)(HSPRITE hPic, int frame);
	void (*pfnSPR_Set)(HSPRITE hPic, int r, int g, int b);
	void (*pfnSPR_Draw)(int frame, int x, int y, const wrect_t* prc);
	void (*pfnSPR_DrawHoles)(int frame, int x, int y, const wrect_t* prc);
	void (*pfnSPR_DrawAdditive)(int frame, int x, int y, const wrect_t* prc);
	void (*pfnSPR_EnableScissor)(int x, int y, int width, int height);
	void (*pfnSPR_DisableScissor)();
	client_sprite_t* (*pfnSPR_GetList)(const char* psz, int* piCount);
	void (*pfnFillRGBA)(int x, int y, int width, int height, int r, int g, int b, int a);
	int (*pfnGetScreenInfo)(SCREENINFO* pscrinfo);
	void (*pfnSetCrosshair)(HSPRITE hspr, wrect_t rc, int r, int g, int b);
	cvar_t* (*pfnRegisterVariable)	(const char* szName, const char* szValue, int flags);
	float (*pfnGetCvarFloat)(const char* szName);
	const char* (*pfnGetCvarString)(const char* szName);
	int (*pfnAddCommand)(const char* cmd_name, void (*pfnEngSrc_function)());
	int (*pfnHookUserMsg)(const char* szMsgName, pfnUserMsgHook pfn);
	int (*pfnServerCmd)(const char* szCmdString);
	int (*pfnClientCmd)(const char* szCmdString);
	void (*pfnGetPlayerInfo)(int ent_num, hud_player_info_t* pinfo);
	void (*pfnPlaySoundByName)(const char* szSound, float volume);
	void (*pfnPlaySoundByIndex)	(int iSound, float volume);
	void (*pfnAngleVectors)(const float* vecAngles, float* forward, float* right, float* up);
	client_textmessage_t* (*pfnTextMessageGet)(const char* pName);
	int (*pfnDrawCharacter)(int x, int y, int number, int r, int g, int b);
	int (*pfnDrawConsoleString)(int x, int y, const char* string);
	void (*pfnDrawSetTextColor)(float r, float g, float b);
	void (*pfnDrawConsoleStringLen)(const char* string, int* length, int* height);
	void (*pfnConsolePrint)(const char* string);
	void (*pfnCenterPrint)(const char* string);
	int (*GetWindowCenterX)();
	int (*GetWindowCenterY)();
	void (*GetViewAngles)(float*);
	void (*SetViewAngles)(float*);
	int (*GetMaxClients)();
	void (*Cvar_SetValue)(const char* cvar, float value);
	int (*Cmd_Argc)();
	char* (*Cmd_Argv)(int arg);
	void (*Con_Printf)(const char* fmt, ...);
	void (*Con_DPrintf)(const char* fmt, ...);
	void (*Con_NPrintf)(int pos, const char* fmt, ...);
	void (*Con_NXPrintf)(con_nprint_t* info, const char* fmt, ...);
	const char* (*PhysInfo_ValueForKey)(const char* key);
	const char* (*ServerInfo_ValueForKey)(const char* key);
	float (*GetClientMaxspeed)();
	int (*CheckParm)(const char* parm, char** ppnext);
	void (*Key_Event)(int key, int down);
	void (*GetMousePosition)(int* mx, int* my);
	int (*IsNoClipping)();
	cl_entity_t* (*GetLocalPlayer)();
	cl_entity_t* (*GetViewModel)();
	cl_entity_t* (*GetEntityByIndex)(int idx);
	float (*GetClientTime)();
	void (*V_CalcShake)();
	void (*V_ApplyShake)(float* origin, float* angles, float factor);
	Contents(*PM_PointContents)(float* point, Contents* truecontents);
	int (*PM_WaterEntity)(float* p);
	pmtrace_t* (*PM_TraceLine)(float* start, float* end, int flags, int usehull, int ignore_pe);
	model_t* (*CL_LoadModel)(const char* modelname, int* index);
	int (*CL_CreateVisibleEntity)(int type, cl_entity_t* ent);
	const model_t* (*GetSpritePointer)(HSPRITE hSprite);
	void (*pfnPlaySoundByNameAtLocation)(const char* szSound, float volume, const float* origin);
	unsigned short (*pfnPrecacheEvent)(int type, const char* psz);
	void (*pfnPlaybackEvent)(int flags, const edict_t* pInvoker, unsigned short eventindex, float delay, const float* origin, const float* angles,
		float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2);
	void (*pfnWeaponAnim)(int iAnim, int body);
	float (*pfnRandomFloat)(float flLow, float flHigh);
	std::int32_t(*pfnRandomLong)(std::int32_t lLow, std::int32_t lHigh);
	void (*pfnHookEvent)(const char* name, void (*pfnEvent)(event_args_t* args));
	int (*Con_IsVisible)();
	const char* (*pfnGetGameDirectory)();
	cvar_t* (*pfnGetCvarPointer)(const char* szName);
	const char* (*Key_LookupBinding)(const char* pBinding);
	const char* (*pfnGetLevelName)();
	void (*pfnGetScreenFade)(screenfade_t* fade);
	void (*pfnSetScreenFade)(screenfade_t* fade);
	void* (*VGui_GetPanel)();
	void (*VGui_ViewportPaintBackground)(int extents[4]);
	byte* (*COM_LoadFile)(const char* path, int usehunk, int* pLength);
	char* (*COM_ParseFile)(const char* data, char* token);
	void (*COM_FreeFile)(void* buffer);
	triangleapi_t* pTriAPI;
	efx_api_t* pEfxAPI;
	event_api_t* pEventAPI;
	demo_api_t* pDemoAPI;
	net_api_t* pNetAPI;
	IVoiceTweak* pVoiceTweak;
	int (*IsSpectateOnly)();
	model_t* (*LoadMapSprite)(const char* filename);
	void (*COM_AddAppDirectoryToSearchPath)(const char* pszBaseDir, const char* appName);
	int (*COM_ExpandFilename)(const char* fileName, char* nameOutBuffer, int nameOutBufferSize);
	const char* (*PlayerInfo_ValueForKey)(int playerNum, const char* key);
	void (*PlayerInfo_SetValueForKey)(const char* key, const char* value);
	qboolean(*GetPlayerUniqueID)(int iPlayer, char playerID[16]);
	int (*GetTrackerIDForPlayer)(int playerSlot);
	int (*GetPlayerForTrackerID)(int trackerID);
	int (*pfnServerCmdUnreliable)(char* szCmdString);
	void (*pfnGetMousePos)(Point* ppt);
	void (*pfnSetMousePos)(int x, int y);
	void (*pfnSetMouseEnable)(qboolean fEnable);
	cvar_t* (*GetFirstCvarPtr)();
	unsigned int (*GetFirstCmdFunctionHandle)();
	unsigned int (*GetNextCmdFunctionHandle)(unsigned int cmdhandle);
	const char* (*GetCmdFunctionName)(unsigned int cmdhandle);
	float (*hudGetClientOldTime)();
	float (*hudGetServerGravityValue)();
	model_t* (*hudGetModelByIndex)(int index);
	void (*pfnSetFilterMode)(int mode);
	void (*pfnSetFilterColor)(float r, float g, float b);
	void (*pfnSetFilterBrightness)(float brightness);
	sequenceEntry* (*pfnSequenceGet)(const char* fileName, const char* entryName);
	void (*pfnSPR_DrawGeneric)(int frame, int x, int y, const wrect_t* prc, int src, int dest, int w, int h);
	sentenceEntry* (*pfnSequencePickSentence)(const char* sentenceName, int pickMethod, int* entryPicked);
	// draw a complete string
	int (*pfnDrawString)(int x, int y, const char* str, int r, int g, int b);
	int (*pfnDrawStringReverse)(int x, int y, const char* str, int r, int g, int b);
	const char* (*LocalPlayerInfo_ValueForKey)(const char* key);
	int (*pfnVGUI2DrawCharacter)(int x, int y, int ch, unsigned int font);
	int (*pfnVGUI2DrawCharacterAdd)(int x, int y, int ch, int r, int g, int b, unsigned int font);
	unsigned int (*COM_GetApproxWavePlayLength) (const char* filename);
	void* (*pfnGetCareerUI)();
	void (*Cvar_Set)(char* cvar, char* value);
	int (*pfnIsCareerMatch)();
	void (*pfnPlaySoundVoiceByName)(const char* szSound, float volume, int pitch);
	void (*pfnPrimeMusicStream)(const char* szFilename, int looping);
	double (*GetAbsoluteTime)();
	void (*pfnProcessTutorMessageDecayBuffer)(int* buffer, int bufferLength);
	void (*pfnConstructTutorMessageDecayBuffer)(int* buffer, int bufferLength);
	void (*pfnResetTutorMessageDecayData)();
	void (*pfnPlaySoundByNameAtPitch)(const char* szSound, float volume, int pitch);
	void (*pfnFillRGBABlend)(int x, int y, int width, int height, int r, int g, int b, int a);
	int (*pfnGetAppID)();
	cmdalias_t* (*pfnGetAliasList)();
	void (*pfnVguiWrap2_GetMouseDelta)(int* x, int* y);
	int (*pfnFilteredClientCmd)(const char* szCmdString);
};
