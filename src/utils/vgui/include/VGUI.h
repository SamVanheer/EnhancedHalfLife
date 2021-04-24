//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#pragma once

#include <cstddef> // size_t define

//If you are going to add stuff to the vgui core...
//
//Keep it simple.
//
//Never put code in a header.
//
//The name of the class is the name of the the file
//
//Each class gets its own .cpp file for its definition and a .h for its header. Helper
//classes can be used but only within the .cpp and not referenceable from anywhere else.
//
//Don't add unneeded files. Keep the API clean.
//
//No platform specific code in vgui\lib-src\vgui dir. Code in vgui\lib-src\vgui should 
//only include from vgui\include or standard C includes. ie, if I see windows.h included
//anywhere but vgui\lib-src\win32 I will hunt you down and kill you. Don't give me any crap
//that mfc is platform inspecific.
//
//Always use <> and not "" for includes
//
//Use minimum dependencies in headers. Don't include another header if you can get away
//with forward declaring (which is usually the case)
//
//No macros in headers. They are tools of satan. This also means no use of DEFINEs, use enum
//
//Minimize global functions
//
//No global variables.
//
//Panel is getting pretty plump, try and avoid adding junk to it if you can

#ifdef _WIN32
# define VGUIAPI __declspec( dllexport )
#else
# define VGUIAPI  __attribute__ ((visibility("default")))
#endif

typedef unsigned char  uchar;

namespace vgui
{

VGUIAPI void  vgui_setMalloc(void *(*malloc)(size_t size) );
VGUIAPI void  vgui_setFree(void (*free)(void* memblock));
VGUIAPI void  vgui_strcpy(char* dst,int dstLen,const char* src);
VGUIAPI char* vgui_strdup(const char* src);
VGUIAPI int   vgui_printf(const char* format,...);
VGUIAPI int   vgui_dprintf(const char* format,...);
VGUIAPI int   vgui_dprintf2(const char* format,...);

}
