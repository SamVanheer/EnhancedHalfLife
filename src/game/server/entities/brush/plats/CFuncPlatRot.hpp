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

class EHL_CLASS() CFuncPlatRot : public CFuncPlat
{
public:
	void Spawn() override;
	void SetupRotation();

	void	GoUp() override;
	void	GoDown() override;
	void	HitTop() override;
	void	HitBottom() override;

	void			RotMove(const Vector& destAngle, float time);
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	Vector	m_end, m_start;
};
