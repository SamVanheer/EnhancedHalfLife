//========= Copyright � 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "com_model.h"
#include "studio.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "dlight.h"
#include "triangleapi.h"

#include "r_studioint.h"

#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"
#include "Exports.h"

/**
*	@file
* 
*	Override the StudioModelRender virtual member functions here to implement custom bone setup, blending, etc.
*/

/**
*	@brief Global engine <-> studio model rendering code interface
*/
extern engine_studio_api_t IEngineStudio;

//TODO: not created on the stack
/**
*	@brief The renderer object, created on the stack.
*/
CGameStudioModelRenderer g_StudioRenderer;

int R_StudioDrawPlayer(int flags, entity_state_t* pplayer)
{
	return g_StudioRenderer.StudioDrawPlayer(flags, pplayer);
}

int R_StudioDrawModel(int flags)
{
	return g_StudioRenderer.StudioDrawModel(flags);
}

void R_StudioInit()
{
	g_StudioRenderer.Init();
}

/**
*	@brief The simple drawing interface we'll pass back to the engine
*/
r_studio_interface_t studio =
{
	STUDIO_INTERFACE_VERSION,
	R_StudioDrawModel,
	R_StudioDrawPlayer,
};

int DLLEXPORT HUD_GetStudioModelInterface(int version, r_studio_interface_t** ppinterface, engine_studio_api_t* pstudio)
{
	if (version != STUDIO_INTERFACE_VERSION)
		return false;

	// Point the engine to our callbacks
	*ppinterface = &studio;

	// Copy in engine helper functions
	memcpy(&IEngineStudio, pstudio, sizeof(IEngineStudio));

	// Initialize local variables, etc.
	R_StudioInit();

	// Success
	return true;
}
