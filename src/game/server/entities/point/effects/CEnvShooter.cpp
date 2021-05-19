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

#include "CEnvShooter.hpp"

LINK_ENTITY_TO_CLASS(env_shooter, CEnvShooter);

void CEnvShooter::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "shootmodel"))
	{
		pev->model = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "shootsounds"))
	{
		const int iNoise = atoi(pkvd->szValue);
		pkvd->fHandled = true;
		switch (iNoise)
		{
		case 0:
			m_iGibMaterial = Materials::Glass;
			break;
		case 1:
			m_iGibMaterial = Materials::Wood;
			break;
		case 2:
			m_iGibMaterial = Materials::Metal;
			break;
		case 3:
			m_iGibMaterial = Materials::Flesh;
			break;
		case 4:
			m_iGibMaterial = Materials::Rocks;
			break;

		default:
		case -1:
			m_iGibMaterial = Materials::None;
			break;
		}
	}
	else
	{
		CGibShooter::KeyValue(pkvd);
	}
}

void CEnvShooter::Precache()
{
	m_iGibModelIndex = PRECACHE_MODEL(STRING(pev->model));
	CBreakable::MaterialSoundPrecache((Materials)m_iGibMaterial);
}

CGib* CEnvShooter::CreateGib()
{
	CGib* pGib = GetClassPtr((CGib*)nullptr);

	pGib->Spawn(STRING(pev->model));

	int bodyPart = 0;

	if (pev->body > 1)
		bodyPart = RANDOM_LONG(0, pev->body - 1);

	pGib->pev->body = bodyPart;
	pGib->m_bloodColor = DONT_BLEED;
	pGib->m_material = m_iGibMaterial;

	pGib->SetRenderMode(GetRenderMode());
	pGib->SetRenderAmount(GetRenderAmount());
	pGib->SetRenderColor(GetRenderColor());
	pGib->SetRenderFX(GetRenderFX());
	pGib->pev->scale = pev->scale;
	pGib->pev->skin = pev->skin;

	return pGib;
}
