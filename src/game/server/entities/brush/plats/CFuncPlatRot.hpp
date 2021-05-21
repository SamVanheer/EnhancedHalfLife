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

#include "CFuncPlat.hpp"
#include "CFuncPlatRot.generated.hpp"

class EHL_CLASS() CFuncPlatRot : public CFuncPlat
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	void SetupRotation();

	void	GoUp() override;
	void	GoDown() override;
	void	HitTop() override;
	void	HitBottom() override;

	void			RotMove(const Vector& destAngle, float time);

	EHL_FIELD(Persisted)
	Vector	m_end;
		
	EHL_FIELD(Persisted)
	Vector m_start;
};
