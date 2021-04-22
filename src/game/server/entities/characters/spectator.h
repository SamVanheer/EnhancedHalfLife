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

class CBaseSpectator : public CBaseEntity
{
public:
	/**
	*	@brief Called when spectator is initialized:
	*	UNDONE: Is this actually being called because spectators are not allocated in normal fashion?
	*/
	void Spawn() override;

	/**
	*	@brief called when a spectator connects to a server
	*/
	void SpectatorConnect();

	/**
	*	@brief called when a spectator disconnects from a server
	*/
	void SpectatorDisconnect();

	/**
	*	@brief Called every frame after physics are run
	*/
	void SpectatorThink();

private:

	/**
	*	@brief Called by SpectatorThink if the spectator entered an impulse
	*/
	void SpectatorImpulseCommand();
};
