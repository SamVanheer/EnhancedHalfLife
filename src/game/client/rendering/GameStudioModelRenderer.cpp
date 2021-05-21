//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "com_model.hpp"
#include "studio.hpp"
#include "entity_state.hpp"
#include "cl_entity.hpp"
#include "dlight.hpp"
#include "triangleapi.hpp"

#include "r_studioint.hpp"

#include "StudioModelRenderer.hpp"
#include "GameStudioModelRenderer.hpp"
#include "Exports.hpp"

/**
*	@file
*
*	Override the StudioModelRender virtual member functions here to implement custom bone setup, blending, etc.
*/

/**
*	@brief Global engine <-> studio model rendering code interface
*/
extern engine_studio_api_t IEngineStudio;

/**
*	@brief The renderer object
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
