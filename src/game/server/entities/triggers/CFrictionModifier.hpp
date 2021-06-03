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

#include "CBaseEntity.hpp"
#include "CFrictionModifier.generated.hpp"

/**
*	@brief Modify an entity's friction
*	@details Sets toucher's friction to m_frictionFraction (1.0 = normal friction)
*/
class EHL_CLASS("EntityName": "func_friction") CFrictionModifier : public CBaseEntity
{
	EHL_GENERATED_BODY()

public:
	void		Spawn() override;
	void		KeyValue(KeyValueData * pkvd) override;
	void EXPORT	ChangeFriction(CBaseEntity * pOther);

	int	ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	EHL_FIELD("Persisted": true)
	float m_frictionFraction = 0; // Sorry, couldn't resist this name :)
};
