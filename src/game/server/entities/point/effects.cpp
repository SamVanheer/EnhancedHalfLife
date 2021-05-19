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

#include "effects/CLightning.hpp"

// UNDONE: Jay -- This is only a test
//TODO: remove
#if _DEBUG
class CTripBeam : public CLightning
{
	void Spawn() override;
};
LINK_ENTITY_TO_CLASS(trip_beam, CTripBeam);

void CTripBeam::Spawn()
{
	CLightning::Spawn();
	SetTouch(&CTripBeam::TriggerTouch);
	SetSolidType(Solid::Trigger);
	RelinkBeam();
}
#endif
