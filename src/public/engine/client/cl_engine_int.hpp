#pragma once

#include "archtypes.h"     // DAL
#include "netadr.h"
#include "Sequence.h"

#ifndef _WIN32
#include "enums.h"
#endif

#define	MAX_ALIAS_NAME	32

typedef struct cmdalias_s
{
	struct cmdalias_s	*next;
	char	name[MAX_ALIAS_NAME];
	char	*value;
} cmdalias_t;

/**
*	@brief Functions exported by the engine
*/
typedef struct cl_enginefuncs_s
{
	HSPRITE (*pfnSPR_Load)(const char* szPicName);
	int (*pfnSPR_Frames)(HSPRITE hPic);
	int (*pfnSPR_Height)(HSPRITE hPic, int frame);
	int (*pfnSPR_Width)(HSPRITE hPic, int frame);
	void (*pfnSPR_Set)(HSPRITE hPic, int r, int g, int b);
	void (*pfnSPR_Draw)(int frame, int x, int y, const struct rect_s* prc);
	void (*pfnSPR_DrawHoles)(int frame, int x, int y, const struct rect_s* prc);
	void (*pfnSPR_DrawAdditive)(int frame, int x, int y, const struct rect_s* prc);
	void (*pfnSPR_EnableScissor)(int x, int y, int width, int height);
	void (*pfnSPR_DisableScissor)();
	struct client_sprite_s* (*pfnSPR_GetList)(const char* psz, int* piCount);
	void (*pfnFillRGBA)(int x, int y, int width, int height, int r, int g, int b, int a);
	int (*pfnGetScreenInfo)(struct SCREENINFO_s* pscrinfo);
	void (*pfnSetCrosshair)(HSPRITE hspr, wrect_t rc, int r, int g, int b);
	struct cvar_s* (*pfnRegisterVariable)	(const char* szName, const char* szValue, int flags);
	float (*pfnGetCvarFloat)(const char* szName);
	const char* (*pfnGetCvarString)(const char* szName);
	int (*pfnAddCommand)(const char* cmd_name, void (*pfnEngSrc_function)());
	int (*pfnHookUserMsg)(const char* szMsgName, pfnUserMsgHook pfn);
	int (*pfnServerCmd)(const char* szCmdString);
	int (*pfnClientCmd)(const char* szCmdString);
	void (*pfnGetPlayerInfo)(int ent_num, struct hud_player_info_s* pinfo);
	void (*pfnPlaySoundByName)(const char* szSound, float volume);
	void (*pfnPlaySoundByIndex)	(int iSound, float volume);
	void (*pfnAngleVectors)(const float* vecAngles, float* forward, float* right, float* up);
	struct client_textmessage_s* (*pfnTextMessageGet)(const char* pName);
	int (*pfnDrawCharacter)(int x, int y, int number, int r, int g, int b);
	int (*pfnDrawConsoleString)(int x, int y, char* string);
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
	void (*Con_NXPrintf)(struct con_nprint_s* info, const char* fmt, ...);
	const char* (*PhysInfo_ValueForKey)(const char* key);
	const char* (*ServerInfo_ValueForKey)(const char* key);
	float (*GetClientMaxspeed)();
	int (*CheckParm)(const char* parm, char** ppnext);
	void (*Key_Event)(int key, int down);
	void (*GetMousePosition)(int* mx, int* my);
	int (*IsNoClipping)();
	struct cl_entity_s* (*GetLocalPlayer)();
	struct cl_entity_s* (*GetViewModel)();
	struct cl_entity_s* (*GetEntityByIndex)(int idx);
	float (*GetClientTime)();
	void (*V_CalcShake)();
	void (*V_ApplyShake)(float* origin, float* angles, float factor);
	int (*PM_PointContents)(float* point, int* truecontents);
	int (*PM_WaterEntity)(float* p);
	struct pmtrace_s* (*PM_TraceLine)(float* start, float* end, int flags, int usehull, int ignore_pe);
	struct model_s* (*CL_LoadModel)(const char* modelname, int* index);
	int (*CL_CreateVisibleEntity)(int type, struct cl_entity_s* ent);
	const struct model_s* (*GetSpritePointer)(HSPRITE hSprite);
	void (*pfnPlaySoundByNameAtLocation)(const char* szSound, float volume, float* origin);
	unsigned short (*pfnPrecacheEvent)(int type, const char* psz);
	void (*pfnPlaybackEvent)(int flags, const struct edict_s* pInvoker, unsigned short eventindex, float delay, float* origin, float* angles,
		float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2);
	void (*pfnWeaponAnim)(int iAnim, int body);
	float (*pfnRandomFloat)(float flLow, float flHigh);
	int32(*pfnRandomLong)(int32 lLow, int32 lHigh);
	void (*pfnHookEvent)(const char* name, void (*pfnEvent)(struct event_args_s* args));
	int (*Con_IsVisible)();
	const char* (*pfnGetGameDirectory)();
	struct cvar_s* (*pfnGetCvarPointer)(const char* szName);
	const char* (*Key_LookupBinding)(const char* pBinding);
	const char* (*pfnGetLevelName)();
	void (*pfnGetScreenFade)(struct screenfade_s* fade);
	void (*pfnSetScreenFade)(struct screenfade_s* fade);
	void* (*VGui_GetPanel)();
	void (*VGui_ViewportPaintBackground)(int extents[4]);
	byte* (*COM_LoadFile)(const char* path, int usehunk, int* pLength);
	char* (*COM_ParseFile)(const char* data, char* token);
	void (*COM_FreeFile)(void* buffer);
	struct triangleapi_s* pTriAPI;
	struct efx_api_s* pEfxAPI;
	struct event_api_s* pEventAPI;
	struct demo_api_s* pDemoAPI;
	struct net_api_s* pNetAPI;
	struct IVoiceTweak_s* pVoiceTweak;
	int (*IsSpectateOnly)();
	struct model_s* (*LoadMapSprite)(const char* filename);
	void (*COM_AddAppDirectoryToSearchPath)(const char* pszBaseDir, const char* appName);
	int (*COM_ExpandFilename)(const char* fileName, char* nameOutBuffer, int nameOutBufferSize);
	const char* (*PlayerInfo_ValueForKey)(int playerNum, const char* key);
	void (*PlayerInfo_SetValueForKey)(const char* key, const char* value);
	qboolean(*GetPlayerUniqueID)(int iPlayer, char playerID[16]);
	int (*GetTrackerIDForPlayer)(int playerSlot);
	int (*GetPlayerForTrackerID)(int trackerID)	;
	int (*pfnServerCmdUnreliable)(char* szCmdString);
	void (*pfnGetMousePos)(struct tagPOINT* ppt);
	void (*pfnSetMousePos)(int x, int y);
	void (*pfnSetMouseEnable)(qboolean fEnable);
	struct cvar_s* (*GetFirstCvarPtr)();
	unsigned int (*GetFirstCmdFunctionHandle)();
	unsigned int (*GetNextCmdFunctionHandle)(unsigned int cmdhandle);
	const char* (*GetCmdFunctionName)(unsigned int cmdhandle);
	float (*hudGetClientOldTime)();
	float (*hudGetServerGravityValue)();
	struct model_s* (*hudGetModelByIndex)(int index);
	void (*pfnSetFilterMode)(int mode);
	void (*pfnSetFilterColor)(float r, float g, float b);
	void (*pfnSetFilterBrightness)(float brightness);
	sequenceEntry_s* (*pfnSequenceGet)(const char* fileName, const char* entryName);
	void (*pfnSPR_DrawGeneric)(int frame, int x, int y, const struct rect_s* prc, int src, int dest, int w, int h);
	sentenceEntry_s* (*pfnSequencePickSentence)(const char* sentenceName, int pickMethod, int* entryPicked);
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
} cl_enginefunc_t;
