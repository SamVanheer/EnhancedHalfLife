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

#include "CPointEntity.hpp"
#include "CLight.generated.hpp"

constexpr int SF_LIGHT_START_OFF = 1;

/**
*	@brief Non-displayed light.
*	Default style is 0
*	If targeted, it will toggle between on or off.
*/
class EHL_CLASS("EntityName": "light") CLight : public CPointEntity
{
	EHL_GENERATED_BODY()

public:
	void	KeyValue(KeyValueData* pkvd) override;
	void	Spawn() override;
	void	Use(const UseInfo& info) override;

private:
	EHL_FIELD("Persisted": true)
	int m_iStyle = 0;

	EHL_FIELD("Persisted": true)
	string_t m_iszPattern = iStringNull;
};
