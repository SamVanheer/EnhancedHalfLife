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

#include "CPointEntity.hpp"
#include "CEnvSound.generated.hpp"

/**
*	@brief Room sound FX
*	@details spawn a sound entity that will set player roomtype when player moves in range and sight.
*	A client that is visible and in range of a sound entity will have its room_type set by that sound entity.
*	If two or more sound entities are contending for a client, then the nearest sound entity to the client will set the client's room_type.
*	A client's room_type will remain set to its prior value until a new in-range, visible sound entity resets a new room_type.
*/
class EHL_CLASS("EntityName": "env_sound") CEnvSound : public CPointEntity
{
	EHL_GENERATED_BODY()

public:
	void KeyValue(KeyValueData* pkvd) override;
	void Spawn() override;

	void Think() override;

	EHL_FIELD("Persisted": true)
	float m_flRadius = 0;

	EHL_FIELD("Persisted": true)
	float m_flRoomtype = 0;
};
