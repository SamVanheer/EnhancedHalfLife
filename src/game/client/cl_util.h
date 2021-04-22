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

#pragma once

#include "cvardef.h"

#include "Platform.h"

#include "filesystem_shared.hpp"
#include "string_utils.hpp"

// Macros to hook function calls into the HUD object
#define HOOK_MESSAGE(x) gEngfuncs.pfnHookUserMsg(#x, __MsgFunc_##x );

#define DECLARE_MESSAGE(y, x) int __MsgFunc_##x(const char *pszName, int iSize, void *pbuf) \
							{ \
							return gHUD.y.MsgFunc_##x(pszName, iSize, pbuf ); \
							}


#define HOOK_COMMAND(x, y) gEngfuncs.pfnAddCommand( x, __CmdFunc_##y );
#define DECLARE_COMMAND(y, x) void __CmdFunc_##x() \
							{ \
								gHUD.y.UserCmd_##x( ); \
							}

inline float CVAR_GET_FLOAT(const char* x) { return gEngfuncs.pfnGetCvarFloat(x); }
inline const char* CVAR_GET_STRING(const char* x) { return gEngfuncs.pfnGetCvarString(x); }
inline cvar_t* CVAR_CREATE(const char* cv, const char* val, const int flags) { return gEngfuncs.pfnRegisterVariable(cv, val, flags); }

#define SPR_Load (*gEngfuncs.pfnSPR_Load)
#define SPR_Set (*gEngfuncs.pfnSPR_Set)
#define SPR_Frames (*gEngfuncs.pfnSPR_Frames)
#define SPR_GetList (*gEngfuncs.pfnSPR_GetList)

/**
*	@brief draws a the current sprite as solid
*/
#define SPR_Draw (*gEngfuncs.pfnSPR_Draw)

/**
*	@brief draws the current sprites, with color index255 not drawn (transparent)
*/
#define SPR_DrawHoles (*gEngfuncs.pfnSPR_DrawHoles)

/**
*	@brief adds the sprites RGB values to the background (additive transulency)
*/
#define SPR_DrawAdditive (*gEngfuncs.pfnSPR_DrawAdditive)

/**
*	@brief sets a clipping rect for HUD sprites. (0,0) is the top-left hand corner of the screen.
*/
#define SPR_EnableScissor (*gEngfuncs.pfnSPR_EnableScissor)

/**
*	@brief disables the clipping rect
*/
#define SPR_DisableScissor (*gEngfuncs.pfnSPR_DisableScissor)

#define FillRGBA (*gEngfuncs.pfnFillRGBA)

/**
* @brief returns the height of the screen, in pixels
*/
#define ScreenHeight (gHUD.m_scrinfo.iHeight)
/**
*	@brief returns the width of the screen, in pixels
*/
#define ScreenWidth (gHUD.m_scrinfo.iWidth)

constexpr float BASE_XRES = 640.f;

/**
*	@brief use this to project world coordinates to screen coordinates
*/
inline float XPROJECT(float x)
{
	return (1.0f + x) * ScreenWidth * 0.5;
}

/**
*	@copydoc XPROJECT
*/
inline float YPROJECT(float y)
{
	return (1.0f - y) * ScreenHeight * 0.5f;
}

inline float XRES(float x)
{
	return x * (ScreenWidth / 640.0f);
}

inline float YRES(float y)
{
	return y * (ScreenHeight / 480.0f);
}

#define GetScreenInfo (*gEngfuncs.pfnGetScreenInfo)
#define ServerCmd (*gEngfuncs.pfnServerCmd)
#define EngineClientCmd (*gEngfuncs.pfnClientCmd)
#define SetCrosshair (*gEngfuncs.pfnSetCrosshair)

/**
*	@brief Gets the height of a sprite, at the specified frame
*/
inline int SPR_Height(HSPRITE x, int f) { return gEngfuncs.pfnSPR_Height(x, f); }

/**
*	@brief Gets the width of a sprite, at the specified frame
*/
inline int SPR_Width(HSPRITE x, int f) { return gEngfuncs.pfnSPR_Width(x, f); }

inline 	client_textmessage_t* TextMessageGet(const char* pName) { return gEngfuncs.pfnTextMessageGet(pName); }
inline 	int						TextMessageDrawChar(int x, int y, int number, int r, int g, int b)
{
	return gEngfuncs.pfnDrawCharacter(x, y, number, r, g, b);
}

inline int DrawConsoleString(int x, int y, const char* string)
{
	return gEngfuncs.pfnDrawConsoleString(x, y, string);
}

inline void GetConsoleStringSize(const char* string, int* width, int* height)
{
	gEngfuncs.pfnDrawConsoleStringLen(string, width, height);
}

inline int ConsoleStringLen(const char* string)
{
	int _width, _height;
	GetConsoleStringSize(string, &_width, &_height);
	return _width;
}

inline void ConsolePrint(const char* string)
{
	gEngfuncs.pfnConsolePrint(string);
}

inline void CenterPrint(const char* string)
{
	gEngfuncs.pfnCenterPrint(string);
}

// sound functions
inline void PlaySound(const char* szSound, float vol) { gEngfuncs.pfnPlaySoundByName(szSound, vol); }
inline void PlaySound(int iSound, float vol) { gEngfuncs.pfnPlaySoundByIndex(iSound, vol); }

void ScaleColors(int& r, int& g, int& b, int a);

// disable 'possible loss of data converting float to int' warning message
#pragma warning( disable: 4244 )
// disable 'truncation from 'const double' to 'float' warning message
#pragma warning( disable: 4305 )

inline void UnpackRGB(int& r, int& g, int& b, unsigned long ulRGB)
{
	r = (ulRGB & 0xFF0000) >> 16;
	g = (ulRGB & 0xFF00) >> 8;
	b = ulRGB & 0xFF;
}

HSPRITE LoadSprite(const char* pszName);
