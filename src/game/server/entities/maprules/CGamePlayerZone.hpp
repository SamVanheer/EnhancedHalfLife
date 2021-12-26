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

#pragma once

#include "CRuleBrushEntity.hpp"
#include "CGamePlayerZone.generated.hpp"

/**
*	@brief players in the zone fire my target when I'm fired
*	@details TODO: Needs master?
*/
class EHL_CLASS("EntityName": "game_zone_player") CGamePlayerZone : public CRuleBrushEntity
{
	EHL_GENERATED_BODY()

public:
	void		KeyValue(KeyValueData* pkvd) override;
	void		Use(const UseInfo& info) override;

private:
	EHL_FIELD("Persisted": true)
	string_t m_iszInTarget;

	EHL_FIELD("Persisted": true)
	string_t m_iszOutTarget;

	EHL_FIELD("Persisted": true)
	string_t m_iszInCount;

	EHL_FIELD("Persisted": true)
	string_t m_iszOutCount;
};
