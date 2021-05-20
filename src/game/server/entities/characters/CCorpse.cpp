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

#include "CCorpse.hpp"

LINK_ENTITY_TO_CLASS(bodyque, CCorpse);

void InitBodyQue()
{
	string_t istrClassname = MAKE_STRING("bodyque");

	g_pBodyQueueHead = UTIL_CreateNamedEntity(istrClassname);
	auto pEntity = g_pBodyQueueHead;

	// Reserve 3 more slots for dead bodies
	for (int i = 0; i < 3; i++)
	{
		pEntity->SetOwner(UTIL_CreateNamedEntity(istrClassname));
		pEntity = pEntity->GetOwner();
	}

	pEntity->SetOwner(g_pBodyQueueHead);
}

void CopyToBodyQue(CBaseEntity* pEntity)
{
	if (pEntity->pev->effects & EF_NODRAW)
		return;

	g_pBodyQueueHead->SetAbsAngles(pEntity->GetAbsAngles());
	g_pBodyQueueHead->pev->model = pEntity->pev->model;
	g_pBodyQueueHead->pev->modelindex = pEntity->pev->modelindex;
	g_pBodyQueueHead->pev->frame = pEntity->pev->frame;
	g_pBodyQueueHead->pev->colormap = pEntity->pev->colormap;
	g_pBodyQueueHead->SetMovetype(Movetype::Toss);
	g_pBodyQueueHead->SetAbsVelocity(pEntity->GetAbsVelocity());
	g_pBodyQueueHead->pev->flags = 0;
	g_pBodyQueueHead->pev->deadflag = pEntity->pev->deadflag;
	g_pBodyQueueHead->SetRenderFX(RenderFX::DeadPlayer);
	g_pBodyQueueHead->SetRenderAmount(pEntity->entindex());

	g_pBodyQueueHead->pev->effects = pEntity->pev->effects | EF_NOINTERP;
	//g_pBodyQueueHead->pev->goalstarttime = pEntity->pev->goalstarttime;
	//g_pBodyQueueHead->pev->goalframe	= pEntity->pev->goalframe;
	//g_pBodyQueueHead->pev->goalendtime = pEntity->pev->goalendtime;

	g_pBodyQueueHead->pev->sequence = pEntity->pev->sequence;
	g_pBodyQueueHead->pev->animtime = pEntity->pev->animtime;

	g_pBodyQueueHead->SetAbsOrigin(pEntity->GetAbsOrigin());
	g_pBodyQueueHead->SetSize(pEntity->pev->mins, pEntity->pev->maxs);
	g_pBodyQueueHead = g_pBodyQueueHead->GetOwner();
}
