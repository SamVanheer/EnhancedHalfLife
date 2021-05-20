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
*	@brief Nodes start out as ents in the level. The node graph  is built, then these ents are discarded.
*/
class EHL_CLASS() CNodeEnt : public CBaseEntity
{
	void Spawn() override;

	/**
	*	@brief nodes start out as ents in the world. As they are spawned, the node info is recorded then the ents are discarded.
	*/
	void KeyValue(KeyValueData* pkvd) override;
	int	ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	short m_sHintType = 0;
	short m_sHintActivity = 0;
};
