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

constexpr int SF_BRUSH_ROTATE_Y_AXIS = 0;
constexpr int SF_BRUSH_ROTATE_INSTANT = 1;
constexpr int SF_BRUSH_ROTATE_BACKWARDS = 2;
constexpr int SF_BRUSH_ROTATE_Z_AXIS = 4;
constexpr int SF_BRUSH_ROTATE_X_AXIS = 8;
constexpr int SF_BRUSH_ACCDCC = 16;			// brush should accelerate and decelerate when toggled
constexpr int SF_BRUSH_HURT = 32;			// rotating brush that inflicts pain based on rotation speed
constexpr int SF_ROTATING_NOT_SOLID = 64;	// some special rotating objects are not solid.
constexpr int SF_BRUSH_ROTATE_SMALLRADIUS = 128;
constexpr int SF_BRUSH_ROTATE_MEDIUMRADIUS = 256;
constexpr int SF_BRUSH_ROTATE_LARGERADIUS = 512;

/**
*	@details You need to have an origin brush as part of this entity.
*	The center of that brush will be the point around which it is rotated.
*	It will rotate around the Z axis by default.
*	You can check either the X_AXIS or Y_AXIS box to change that.
*/
class EHL_CLASS() CFuncRotating : public CBaseEntity
{
public:
	// basic functions
	void Spawn() override;
	void Precache() override;

	/**
	*	@brief accelerates a non-moving func_rotating up to it's speed
	*/
	void EXPORT SpinUp();

	/**
	*	@brief decelerates a moving func_rotating to a standstill.
	*/
	void EXPORT SpinDown();
	void KeyValue(KeyValueData* pkvd) override;

	/**
	*	@brief will hurt others based on how fast the brush is spinning
	*/
	void EXPORT HurtTouch(CBaseEntity* pOther);

	/**
	*	@brief when a rotating brush is triggered
	*/
	void EXPORT RotatingUse(const UseInfo& info);
	void EXPORT Rotate();

	/**
	*	@brief ramp pitch and volume up to final values, based on difference between how fast we're going vs how fast we plan to go
	*/
	void RampPitchVol(int fUp);

	/**
	*	@brief An entity has blocked the brush
	*/
	void Blocked(CBaseEntity* pOther) override;
	int	ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];

	float m_flFanFriction = 0;
	float m_flAttenuation = 0;
	float m_flVolume = 0;
	float m_pitch = 0;
	int m_sounds = 0;
};
