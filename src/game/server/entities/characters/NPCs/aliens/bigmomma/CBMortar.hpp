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
#include "CBMortar.generated.hpp"

/**
*	@brief Mortar shot entity
*/
class EHL_CLASS(EntityName=bmortar) CBMortar : public CBaseEntity
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;

	static CBMortar* Shoot(CBaseEntity* pOwner, const Vector& vecStart, const Vector& vecVelocity);
	void Touch(CBaseEntity* pOther) override;
	void EXPORT Animate();

	EHL_FIELD(Persisted)
	int m_maxFrame = 0;
};
