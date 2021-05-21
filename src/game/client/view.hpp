//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#pragma once

class Vector;

void V_StartPitchDrift();
void V_StopPitchDrift();
void V_GetChasePos(int target, Vector* cl_angles, Vector& origin, Vector& angles);
void V_ResetChaseCam();
void V_GetInEyePos(int target, Vector& origin, Vector& angles);

/**
*	@brief Client side punch effect
*/
void V_PunchAxis(int axis, float punch);
