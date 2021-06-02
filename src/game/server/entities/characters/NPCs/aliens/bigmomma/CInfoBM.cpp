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

#include "CInfoBM.hpp"

void CInfoBM::Spawn()
{
}

void CInfoBM::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "radius"))
	{
		pev->scale = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "reachdelay"))
	{
		pev->speed = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "reachtarget"))
	{
		pev->message = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "reachsequence"))
	{
		pev->netname = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "presequence"))
	{
		m_preSequence = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CPointEntity::KeyValue(pkvd);
}
