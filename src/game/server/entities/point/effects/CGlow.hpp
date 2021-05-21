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
#include "CGlow.generated.hpp"

class EHL_CLASS() CGlow : public CPointEntity
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	void Think() override;
	void Animate(float frames);

	EHL_FIELD(Persisted, Type=Time)
	float m_lastTime = 0;

	EHL_FIELD(Persisted)
	float m_maxFrame = 0;
};
