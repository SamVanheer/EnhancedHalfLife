//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

/**
*	@file
*
*	Triangle rendering, if any
*	Triangle rendering apis are in gEngfuncs.pTriAPI
*/

#include "entity_state.h"
#include "cl_entity.h"
#include "triangleapi.h"
#include "Exports.h"

#include "particleman.h"
#include "tri.h"

void DLLEXPORT HUD_DrawNormalTriangles()
{
	gHUD.m_Spectator.DrawOverview();
}

void DLLEXPORT HUD_DrawTransparentTriangles()
{
	if (g_pParticleMan)
		g_pParticleMan->Update();
}
