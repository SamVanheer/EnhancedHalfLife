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

#include "CBaseDelay.hpp"

#include "CTriggerChangeTarget.generated.hpp"

/**
*	@brief this is a really bad idea.
*/
class EHL_CLASS() CTriggerChangeTarget : public CBaseDelay
{
	EHL_GENERATED_BODY()

public:
	void KeyValue(KeyValueData * pkvd) override;
	void Spawn() override;
	void Use(const UseInfo & info) override;

	int ObjectCaps() override { return CBaseDelay::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

private:
	EHL_FIELD(Persisted)
		string_t m_iszNewTarget = iStringNull;
};
