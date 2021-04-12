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

#include "extdll.h"
#include "util.h"
#include "skill.h"

float GetSkillCvar(const char* pName)
{
	char	szBuffer[64];

	const int iCount = sprintf(szBuffer, "%s%d", pName, static_cast<int>(gSkillData.Level));

	const float flValue = CVAR_GET_FLOAT(szBuffer);

	if (flValue <= 0)
	{
		ALERT(at_console, "\n\n** GetSkillCVar Got a zero for %s **\n\n", szBuffer);
	}

	return flValue;
}
