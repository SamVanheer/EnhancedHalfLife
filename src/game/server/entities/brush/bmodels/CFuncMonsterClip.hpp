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

#include "CFuncWall.hpp"
#include "CFuncMonsterClip.generated.hpp"

/**
*	@brief Monster only clip brush
*
*	@details This brush will be solid for any entity who has the FL_MONSTERCLIP flag set in pev->flags
*	otherwise it will be invisible and not solid. This can be used to keep  specific monsters out of certain areas
*/
class EHL_CLASS() CFuncMonsterClip : public CFuncWall
{
	EHL_GENERATED_BODY()

public:
	void	Spawn() override;
	void	Use(const UseInfo& info) override {}		//!< Clear out func_wall's use function
};
