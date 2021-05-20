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

/**
*	@brief players in the zone fire my target when I'm fired
*	@details TODO: Needs master?
*/
class CGamePlayerZone : public CRuleBrushEntity
{
public:
	void		KeyValue(KeyValueData* pkvd) override;
	void		Use(const UseInfo& info) override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

private:
	string_t m_iszInTarget = iStringNull;
	string_t m_iszOutTarget = iStringNull;
	string_t m_iszInCount = iStringNull;
	string_t m_iszOutCount = iStringNull;
};
