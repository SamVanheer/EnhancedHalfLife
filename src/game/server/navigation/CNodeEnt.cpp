/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/

#include "CNodeEnt.hpp"
#include "CTestHull.hpp"
#include "nodes.hpp"

void CNodeEnt::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "hinttype"))
	{
		m_sHintType = (short)atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}

	if (AreStringsEqual(pkvd->szKeyName, "activity"))
	{
		m_sHintActivity = (short)atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

void CNodeEnt::Spawn()
{
	SetMovetype(Movetype::None);
	SetSolidType(Solid::Not);// always solid_not 

	if (WorldGraph.m_fGraphPresent)
	{// graph loaded from disk, so discard all these node ents as soon as they spawn
		UTIL_RemoveNow(this);
		return;
	}

	if (WorldGraph.m_cNodes == 0)
	{// this is the first node to spawn, spawn the test hull entity that builds and walks the node tree
		CTestHull* pHull = static_cast<CTestHull*>(g_EntityList.Create("testhull"));
		pHull->Spawn(this);
	}

	if (WorldGraph.m_cNodes >= MAX_NODES)
	{
		ALERT(at_aiconsole, "cNodes > MAX_NODES\n");
		return;
	}

	WorldGraph.m_pNodes[WorldGraph.m_cNodes].m_vecOriginPeek =
		WorldGraph.m_pNodes[WorldGraph.m_cNodes].m_vecOrigin = GetAbsOrigin();
	WorldGraph.m_pNodes[WorldGraph.m_cNodes].m_flHintYaw = GetAbsAngles().y;
	WorldGraph.m_pNodes[WorldGraph.m_cNodes].m_sHintType = m_sHintType;
	WorldGraph.m_pNodes[WorldGraph.m_cNodes].m_sHintActivity = m_sHintActivity;

	if (ClassnameIs("info_node_air"))
		WorldGraph.m_pNodes[WorldGraph.m_cNodes].m_afNodeInfo = bits_NODE_AIR;
	else
		WorldGraph.m_pNodes[WorldGraph.m_cNodes].m_afNodeInfo = 0;

	WorldGraph.m_cNodes++;

	UTIL_RemoveNow(this);
}
