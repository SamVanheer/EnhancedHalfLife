/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/

#pragma once

#include "CGrenade.hpp"
#include "CApacheHVR.generated.hpp"

class EHL_CLASS("EntityName": "hvr_rocket") CApacheHVR : public CGrenade
{
	EHL_GENERATED_BODY()

	void Spawn() override;
	void Precache() override;
	void EXPORT IgniteThink();
	void EXPORT AccelerateThink();

	//Don't save, precached
	int m_iTrail = 0;

	EHL_FIELD("Persisted": true)
	Vector m_vecForward;
};
