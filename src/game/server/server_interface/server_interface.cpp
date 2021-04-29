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

#include "server_interface.hpp"
#include "util.h"

// Holds engine functionality callbacks
enginefuncs_t g_engfuncs;
globalvars_t* gpGlobals;

#ifdef _WIN32
#define GIVEFNPTRSTODLL_EXPORT __stdcall
#else
#define GIVEFNPTRSTODLL_EXPORT DLLEXPORT
#endif

extern "C"
{
	void GIVEFNPTRSTODLL_EXPORT GiveFnptrsToDll(enginefuncs_t* pengfuncsFromEngine, globalvars_t* pGlobals)
	{
		memcpy(&g_engfuncs, pengfuncsFromEngine, sizeof(enginefuncs_t));
		gpGlobals = pGlobals;
	}

	int GetEntityAPI(DLL_FUNCTIONS* pFunctionTable, int interfaceVersion)
	{
		if (!pFunctionTable || interfaceVersion != INTERFACE_VERSION)
		{
			return false;
		}

		memcpy(pFunctionTable, &gEntityInterface, sizeof(DLL_FUNCTIONS));
		return true;
	}

	int GetEntityAPI2(DLL_FUNCTIONS* pFunctionTable, int* interfaceVersion)
	{
		if (!pFunctionTable || *interfaceVersion != INTERFACE_VERSION)
		{
			// Tell engine what version we had, so it can figure out who is out of date.
			*interfaceVersion = INTERFACE_VERSION;
			return false;
		}

		memcpy(pFunctionTable, &gEntityInterface, sizeof(DLL_FUNCTIONS));
		return true;
	}

	int GetNewDLLFunctions(NEW_DLL_FUNCTIONS* pFunctionTable, int* interfaceVersion)
	{
		if (!pFunctionTable || *interfaceVersion != NEW_DLL_FUNCTIONS_VERSION)
		{
			*interfaceVersion = NEW_DLL_FUNCTIONS_VERSION;
			return false;
		}

		memcpy(pFunctionTable, &gNewDLLFunctions, sizeof(gNewDLLFunctions));
		return true;
	}

	struct TITLECOMMENT
	{
		const char* pBSPName;
		const char* pTitleName;
	};

	static constexpr TITLECOMMENT gTitleComments[66] =
	{
		{ "t0a0", "#T0A0TITLE"},
		{ "c0a0", "#C0A0TITLE"},
		{ "c1a0", "#C0A1TITLE"},
		{ "c1a1", "#C1A1TITLE"},
		{ "c1a2", "#C1A2TITLE"},
		{ "c1a3", "#C1A3TITLE"},
		{ "c1a4", "#C1A4TITLE"},
		{ "c2a1", "#C2A1TITLE"},
		{ "c2a2", "#C2A2TITLE"},
		{ "c2a3", "#C2A3TITLE"},
		{ "c2a4d", "#C2A4TITLE2"},
		{ "c2a4e", "#C2A4TITLE2"},
		{ "c2a4f", "#C2A4TITLE2"},
		{ "c2a4g", "#C2A4TITLE2"},
		{ "c2a4", "#C2A4TITLE1"},
		{ "c2a5", "#C2A5TITLE"},
		{ "c3a1", "#C3A1TITLE"},
		{ "c3a2", "#C3A2TITLE"},
		{ "c4a1a", "#C4A1ATITLE"},
		{ "c4a1b", "#C4A1ATITLE"},
		{ "c4a1c", "#C4A1ATITLE"},
		{ "c4a1d", "#C4A1ATITLE"},
		{ "c4a1e", "#C4A1ATITLE"},
		{ "c4a1", "#C4A1TITLE"},
		{ "c4a2", "#C4A2TITLE"},
		{ "c4a3", "#C4A3TITLE"},
		{ "c5a1", "#C5TITLE"},
		{ "ofboot", "#OF_BOOT0TITLE"},
		{ "of0a", "#OF1A1TITLE"},
		{ "of1a1", "#OF1A3TITLE"},
		{ "of1a2", "#OF1A3TITLE"},
		{ "of1a3", "#OF1A3TITLE"},
		{ "of1a4", "#OF1A3TITLE"},
		{ "of1a", "#OF1A5TITLE"},
		{ "of2a1", "#OF2A1TITLE"},
		{ "of2a2", "#OF2A1TITLE"},
		{ "of2a3", "#OF2A1TITLE"},
		{ "of2a", "#OF2A4TITLE"},
		{ "of3a1", "#OF3A1TITLE"},
		{ "of3a2", "#OF3A1TITLE"},
		{ "of3a", "#OF3A3TITLE"},
		{ "of4a1", "#OF4A1TITLE"},
		{ "of4a2", "#OF4A1TITLE"},
		{ "of4a3", "#OF4A1TITLE"},
		{ "of4a", "#OF4A4TITLE"},
		{ "of5a", "#OF5A1TITLE"},
		{ "of6a1", "#OF6A1TITLE"},
		{ "of6a2", "#OF6A1TITLE"},
		{ "of6a3", "#OF6A1TITLE"},
		{ "of6a4b", "#OF6A4TITLE"},
		{ "of6a4", "#OF6A1TITLE"},
		{ "of6a5", "#OF6A4TITLE"},
		{ "of6a", "#OF6A4TITLE"},
		{ "of7a", "#OF7A0TITLE"},
		{ "ba_tram", "#BA_TRAMTITLE"},
		{ "ba_security", "#BA_SECURITYTITLE"},
		{ "ba_main", "#BA_SECURITYTITLE"},
		{ "ba_elevator", "#BA_SECURITYTITLE"},
		{ "ba_canal", "#BA_CANALSTITLE"},
		{ "ba_yard", "#BA_YARDTITLE"},
		{ "ba_xen", "#BA_XENTITLE"},
		{ "ba_hazard", "#BA_HAZARD"},
		{ "ba_power", "#BA_POWERTITLE"},
		{ "ba_teleport1", "#BA_YARDTITLE"},
		{ "ba_teleport", "#BA_TELEPORTTITLE"},
		{ "ba_outro", "#BA_OUTRO"}
	};

	void SV_SaveGameComment(char* pszBuffer, int iSizeBuffer)
	{
		const char* mapName = STRING(gpGlobals->mapname);

		const char* titleName = nullptr;

		//See if the current map is in the hardcoded list of titles
		if (mapName)
		{
			for (const auto& comment : gTitleComments)
			{
				//See if the bsp name starts with this prefix
				if (!strnicmp(mapName, comment.pBSPName, strlen(comment.pBSPName)))
				{
					if (comment.pTitleName)
					{
						titleName = comment.pTitleName;
						break;
					}
				}
			}
		}

		if (!titleName)
		{
			//Slightly different behavior here compared to the engine
			//Instead of using maps/mapname.bsp we just use an empty string
			if (mapName)
			{
				titleName = mapName;
			}
			else
			{
				titleName = "";
			}
		}

		safe_strcpy(pszBuffer, titleName, iSizeBuffer);
	}
}
