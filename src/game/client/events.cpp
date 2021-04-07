//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "hud.h"
#include "cl_util.h"

void Game_HookEvents();

/**
*	@brief See if game specific code wants to hook any events.
*/
void EV_HookEvents()
{
	Game_HookEvents();
}