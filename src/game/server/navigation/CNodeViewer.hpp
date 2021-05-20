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

#pragma once

#include "CBaseEntity.hpp"

/**
*	@brief Draws a graph of the shorted path from all nodes to current location (typically the player).
*	It then draws as many connects as it can per frame, trying not to overflow the buffer
*/
class CNodeViewer : public CBaseEntity
{
public:
	void Spawn() override;

	int m_iBaseNode = 0;
	int m_iDraw = 0;
	int	m_nVisited = 0;
	int m_aFrom[128]{};
	int m_aTo[128]{};
	int m_iHull = 0;
	int m_afNodeType = 0;
	Vector m_vecColor;

	void FindNodeConnections(int iNode);
	void AddNode(int iFrom, int iTo);
	void EXPORT DrawThink();
};
