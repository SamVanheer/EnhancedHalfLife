/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#include "extdll.h"
#include "plane.h"

CPlane::CPlane(const Vector& vecNormal, const Vector& vecPoint)
	: m_vecNormal(vecNormal)
	, m_flDist(DotProduct(vecNormal, vecPoint))
{
}

bool CPlane::PointInFront(const Vector& vecPoint) const
{
	const float flFace = DotProduct(m_vecNormal, vecPoint) - m_flDist;
	return flFace >= 0;
}
