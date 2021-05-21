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
#include "CMessage.generated.hpp"

constexpr int SF_MESSAGE_ONCE = 0x0001;	//!< Fade in, not out
constexpr int SF_MESSAGE_ALL = 0x0002;	//!< Send to all clients

class EHL_CLASS() CMessage : public CPointEntity
{
	EHL_GENERATED_BODY()

public:
	void	Spawn() override;
	void	Precache() override;
	void	Use(const UseInfo& info) override;
	void	KeyValue(KeyValueData* pkvd) override;
private:
};
