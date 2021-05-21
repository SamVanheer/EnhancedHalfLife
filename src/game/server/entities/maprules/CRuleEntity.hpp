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

#include "CBaseEntity.hpp"
#include "CRuleEntity.generated.hpp"

/**
*	@file
*
*	This module contains entities for implementing/changing game rules dynamically within each map (.BSP)
*/

class EHL_CLASS() CRuleEntity : public CBaseEntity
{
	EHL_GENERATED_BODY()

public:
	void	Spawn() override;
	void	KeyValue(KeyValueData* pkvd) override;

	void	SetMaster(string_t iszMaster) { m_iszMaster = iszMaster; }

protected:
	bool	CanFireForActivator(CBaseEntity* pActivator);

private:
	EHL_FIELD(Persisted)
	string_t m_iszMaster = iStringNull;
};
