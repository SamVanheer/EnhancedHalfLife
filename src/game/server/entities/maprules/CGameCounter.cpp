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

#include "CGameCounter.hpp"

LINK_ENTITY_TO_CLASS(game_counter, CGameCounter);

void CGameCounter::Spawn()
{
	// Save off the initial count
	SetInitialValue(CountValue());
	CRulePointEntity::Spawn();
}

void CGameCounter::Use(const UseInfo& info)
{
	if (!CanFireForActivator(info.GetActivator()))
		return;

	switch (info.GetUseType())
	{
	case UseType::On:
	case UseType::Toggle:
		CountUp();
		break;

	case UseType::Off:
		CountDown();
		break;

	case UseType::Set:
		SetCountValue((int)info.GetValue());
		break;
	}

	if (HitLimit())
	{
		SUB_UseTargets(info.GetActivator(), UseType::Toggle, 0);
		if (RemoveOnFire())
		{
			UTIL_Remove(this);
		}

		if (ResetOnFire())
		{
			ResetCount();
		}
	}
}
