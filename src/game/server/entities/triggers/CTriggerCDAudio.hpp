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

#include "CBaseTrigger.hpp"

#include "CTriggerCDAudio.generated.hpp"

/**
*	@brief starts/stops cd audio tracks
*	Changes tracks or stops CD when player touches or when triggered
*/
class EHL_CLASS("EntityName": "trigger_cdaudio") CTriggerCDAudio : public CBaseTrigger
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;

	void Use(const UseInfo & info) override;
	void PlayTrack();
	void Touch(CBaseEntity * pOther) override;
};

void PlayCDTrack(int iTrack);
