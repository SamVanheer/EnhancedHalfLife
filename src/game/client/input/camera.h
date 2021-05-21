//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#pragma once

#include "vector.h"

/**
*	@file
*
*	defines and such for a 3rd person camera
*/

/**
*	@brief pitch, yaw, dist
*/
extern Vector cam_ofs;

/**
*	@brief Using third person camera
*/
extern bool cam_thirdperson;

void CAM_Init();
void CAM_ClearStates();
void CAM_StartMouseMove();
void CAM_EndMouseMove();
