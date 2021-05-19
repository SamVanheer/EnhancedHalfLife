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

#include "CXenHull.hpp"

LINK_ENTITY_TO_CLASS(xen_hull, CXenHull);

CXenHull* CXenHull::CreateHull(CBaseEntity* source, const Vector& mins, const Vector& maxs, const Vector& offset)
{
	CXenHull* pHull = GetClassPtr((CXenHull*)nullptr);

	pHull->SetAbsOrigin(source->GetAbsOrigin() + offset);
	pHull->SetModel(STRING(source->pev->model));
	pHull->SetSolidType(Solid::BBox);
	pHull->SetClassname("xen_hull");
	pHull->SetMovetype(Movetype::None);
	pHull->SetOwner(source);
	pHull->SetSize(mins, maxs);
	pHull->SetRenderAmount(0);
	pHull->SetRenderMode(RenderMode::TransTexture);
	//	pHull->pev->effects = EF_NODRAW;

	return pHull;
}
