//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#pragma once

void V_StartPitchDrift();
void V_StopPitchDrift();
void V_GetChasePos(int target, float* cl_angles, float* origin, float* angles);
void V_ResetChaseCam();
void V_GetInEyePos(int target, float* origin, float* angles);
void V_PunchAxis(int axis, float punch);
