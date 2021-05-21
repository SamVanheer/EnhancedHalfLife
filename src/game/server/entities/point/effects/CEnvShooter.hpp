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

#include "CGibShooter.hpp"
#include "CEnvShooter.generated.hpp"

class EHL_CLASS() CEnvShooter : public CGibShooter
{
	EHL_GENERATED_BODY()

	void		Precache() override;
	void		KeyValue(KeyValueData* pkvd) override;

	CGib* CreateGib() override;
};
