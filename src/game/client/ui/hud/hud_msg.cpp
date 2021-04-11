/***
*
*	Copyright (c) 1999, Valve LLC. All rights reserved.
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

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "r_efx.h"

#include "extdll.h"
#include "util.h"

#include "particleman.h"

extern BEAM *pBeam;
extern BEAM *pBeam2;
extern TEMPENTITY* pFlare;	// Vit_amiN

/// USER-DEFINED SERVER MESSAGE HANDLERS

bool CHud :: MsgFunc_ResetHUD(const char *pszName, int iSize, void *pbuf )
{
	ASSERT( iSize == 0 );

	// clear all hud data
	HUDLIST *pList = m_pHudList;

	while ( pList )
	{
		if ( pList->p )
			pList->p->Reset();
		pList = pList->pNext;
	}

	// reset sensitivity
	m_flMouseSensitivity = 0;

	return true;
}

void CHud :: MsgFunc_InitHUD( const char *pszName, int iSize, void *pbuf )
{
	// prepare all hud data
	HUDLIST *pList = m_pHudList;

	while (pList)
	{
		if ( pList->p )
			pList->p->InitHUDData();
		pList = pList->pNext;
	}

	if ( g_pParticleMan )
		 g_pParticleMan->ResetParticles();

	//Probably not a good place to put this.
	pBeam = pBeam2 = nullptr;
	pFlare = nullptr;	// Vit_amiN: clear egon's beam flare
}


bool CHud :: MsgFunc_GameMode(const char *pszName, int iSize, void *pbuf )
{
	BufferReader reader{pbuf, iSize};
	m_Teamplay = reader.ReadByte();

	return true;
}


bool CHud :: MsgFunc_Damage(const char *pszName, int iSize, void *pbuf )
{
	int		armor, blood;
	Vector	from;
	int		i;
	float	count;
	
	BufferReader reader{pbuf, iSize};
	armor = reader.ReadByte();
	blood = reader.ReadByte();

	for (i=0 ; i<3 ; i++)
		from[i] = reader.ReadCoord();

	count = (blood * 0.5) + (armor * 0.5);

	if (count < 10)
		count = 10;

	// TODO: kick viewangles,  show damage visually

	return true;
}
