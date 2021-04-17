//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#pragma once

/**
*	@file
*
*	defines and such for a 3rd person camera
*/

// pitch, yaw, dist
extern Vector cam_ofs;
// Using third person camera
extern bool cam_thirdperson;

void CAM_Init();
void CAM_ClearStates();
void CAM_StartMouseMove();
void CAM_EndMouseMove();
