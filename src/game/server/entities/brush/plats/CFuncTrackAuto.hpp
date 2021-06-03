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

#include "CFuncTrackChange.hpp"
#include "CFuncTrackAuto.generated.hpp"

/**
*	@brief Auto track change
*/
class EHL_CLASS("EntityName": "func_trackautochange") CFuncTrackAuto : public CFuncTrackChange
{
	EHL_GENERATED_BODY()

public:
	void			Use(const UseInfo& info) override;
	void	UpdateAutoTargets(ToggleState toggleState) override;
};

