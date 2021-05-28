/************ (C) Copyright 2003 Valve, L.L.C. All rights reserved. ***********
**
** The copyright to the contents herein is the property of Valve, L.L.C.
** The contents may be used and/or copied only with the written permission of
** Valve, L.L.C., or in accordance with the terms and conditions stipulated in
** the agreement/contract under which the contents have been supplied.
**
******************************************************************************/

#pragma once

#include "vector.hpp"

/**
*	@brief Bezier inpolation class
*/
class CInterpolation
{
public:

	CInterpolation();
	virtual ~CInterpolation();

	void SetWaypoints(Vector* prev, const Vector& start, const Vector& end, Vector* next);
	void SetViewAngles(const Vector& start, const Vector& end);
	void SetFOVs(float start, float end);
	void SetSmoothing(bool start, bool end);

	/**
	*	@brief get interpolated point 0 =< t =< 1, 0 = start, 1 = end
	*/
	void Interpolate(float t, Vector& point, Vector& angle, float* fov);

protected:

	void BezierInterpolatePoint(float t, Vector& point);
	void InterpolateAngle(float t, Vector& angle);

	Vector	m_StartPoint;
	Vector	m_EndPoint;
	Vector	m_StartAngle;
	Vector	m_EndAngle;
	Vector	m_Center;
	float	m_StartFov = 0;
	float	m_EndFov = 0;

	bool	m_SmoothStart = false;
	bool	m_SmoothEnd = false;
};