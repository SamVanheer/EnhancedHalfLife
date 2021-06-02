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
#include "CRenderFxManager.generated.hpp"

// Flags to indicate masking off various render parameters that are normally copied to the targets
constexpr int SF_RENDER_MASKFX = 1 << 0;
constexpr int SF_RENDER_MASKAMT = 1 << 1;
constexpr int SF_RENDER_MASKMODE = 1 << 2;
constexpr int SF_RENDER_MASKCOLOR = 1 << 3;

/**
*	@brief This entity will copy its render parameters (renderfx, rendermode, rendercolor, renderamt) to its targets when triggered.
*/
class EHL_CLASS(EntityName=env_render) CRenderFxManager : public CBaseEntity
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	void Use(const UseInfo & info) override;
};
