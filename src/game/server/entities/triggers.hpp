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
#include "CPointEntity.hpp"
#include "CMultiSource.hpp"
#include "CBaseDelay.hpp"
#include "CBaseToggle.hpp"

class CBasePlayer;

#include "CFrictionModifier.generated.hpp"

constexpr int SF_TRIGGER_ALLOWMONSTERS = 1;	//!< monsters allowed to fire this trigger
constexpr int SF_TRIGGER_NOCLIENTS = 2;		//!< players not allowed to fire this trigger
constexpr int SF_TRIGGER_PUSHABLES = 4;		//!< only pushables can fire this trigger

/**
*	@brief Modify an entity's friction
*	@details Sets toucher's friction to m_frictionFraction (1.0 = normal friction)
*/
class EHL_CLASS() CFrictionModifier : public CBaseEntity
{
	EHL_GENERATED_BODY()

public:
	void		Spawn() override;
	void		KeyValue(KeyValueData* pkvd) override;
	void EXPORT	ChangeFriction(CBaseEntity* pOther);

	int	ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	EHL_FIELD(Persisted)
	float m_frictionFraction = 0; // Sorry, couldn't resist this name :)
};

#include "CAutoTrigger.generated.hpp"

constexpr int SF_AUTO_FIREONCE = 0x0001;

/**
*	@brief This trigger will fire when the level spawns (or respawns if not fire once)
*	It will check a global state before firing. It supports delay and killtargets
*/
class EHL_CLASS() CAutoTrigger : public CBaseDelay
{
	EHL_GENERATED_BODY()

public:
	void KeyValue(KeyValueData* pkvd) override;
	void Spawn() override;
	void Precache() override;
	void Think() override;

	int ObjectCaps() override { return CBaseDelay::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
private:
	EHL_FIELD(Persisted)
	string_t m_globalstate = iStringNull;

	EHL_FIELD(Persisted)
	UseType	triggerType = UseType::Off;
};

#include "CTriggerRelay.generated.hpp"

constexpr int SF_RELAY_FIREONCE = 0x0001;

class EHL_CLASS() CTriggerRelay : public CBaseDelay
{
	EHL_GENERATED_BODY()

public:
	void KeyValue(KeyValueData* pkvd) override;
	void Spawn() override;
	void Use(const UseInfo& info) override;

	int ObjectCaps() override { return CBaseDelay::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

private:
	EHL_FIELD(Persisted)
	UseType	triggerType = UseType::Off;
};

#include "CMultiManager.generated.hpp"

constexpr int SF_MULTIMAN_CLONE = 0x80000000;
constexpr int SF_MULTIMAN_THREAD = 0x00000001;

/**
*	@brief The Multimanager Entity - when fired, will fire up to MAX_MULTI_TARGETS targets at specified times.
*	@details FLAG: THREAD (create clones when triggered)
*	FLAG: CLONE (this is a clone for a threaded execution)
*	@see MAX_MULTI_TARGETS
*/
class EHL_CLASS() CMultiManager : public CBaseToggle
{
	EHL_GENERATED_BODY()

public:
	void KeyValue(KeyValueData* pkvd) override;
	void Spawn() override;
	void EXPORT ManagerThink();
	void EXPORT ManagerUse(const UseInfo& info);

#if _DEBUG
	void EXPORT ManagerReport();
#endif

	bool		HasTarget(string_t targetname) override;

	int ObjectCaps() override { return CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	EHL_FIELD(Persisted)
	int m_cTargets = 0;								//!< the total number of targets in this manager's fire list.

	EHL_FIELD(Persisted)
	int m_index = 0;								//!< Current target

	EHL_FIELD(Persisted, Type=Time)
	float m_startTime = 0;							//!< Time we started firing

	EHL_FIELD(Persisted)
	string_t m_iTargetName[MAX_MULTI_TARGETS]{};	//!< list if indexes into global string array

	EHL_FIELD(Persisted)
	float m_flTargetDelay[MAX_MULTI_TARGETS]{};		//!< delay (in seconds) from time of manager fire to target fire
private:
	inline bool IsClone() { return (pev->spawnflags & SF_MULTIMAN_CLONE) != 0; }
	inline bool ShouldClone()
	{
		if (IsClone())
			return false;

		return (pev->spawnflags & SF_MULTIMAN_THREAD) != 0;
	}

	CMultiManager* Clone();
};

// Flags to indicate masking off various render parameters that are normally copied to the targets
constexpr int SF_RENDER_MASKFX = 1 << 0;
constexpr int SF_RENDER_MASKAMT = 1 << 1;
constexpr int SF_RENDER_MASKMODE = 1 << 2;
constexpr int SF_RENDER_MASKCOLOR = 1 << 3;

/**
*	@brief This entity will copy its render parameters (renderfx, rendermode, rendercolor, renderamt) to its targets when triggered.
*/
class EHL_CLASS() CRenderFxManager : public CBaseEntity
{
public:
	void Spawn() override;
	void Use(const UseInfo& info) override;
};

class EHL_CLASS() CBaseTrigger : public CBaseToggle
{
public:
	int	ObjectCaps() override { return CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	void KeyValue(KeyValueData* pkvd) override;

	void InitTrigger();
	void ActivateMultiTrigger(CBaseEntity* pActivator);

	/**
	*	@brief the wait time has passed, so set back up for another activation
	*/
	void EXPORT MultiWaitOver();

	/**
	*	@brief If this is the USE function for a trigger, its state will toggle every time it's fired
	*/
	void EXPORT ToggleUse(const UseInfo& info);
};

constexpr int SF_TRIGGER_HURT_TARGETONCE = 1;		//!< Only fire hurt target once
constexpr int SF_TRIGGER_HURT_START_OFF = 2;		//!< spawnflag that makes trigger_push spawn turned OFF
constexpr int SF_TRIGGER_HURT_NO_CLIENTS = 8;		//!< spawnflag that makes trigger_push spawn turned OFF
constexpr int SF_TRIGGER_HURT_CLIENTONLYFIRE = 16;	//!< trigger hurt will only fire its target if it is hurting a client
constexpr int SF_TRIGGER_HURT_CLIENTONLYTOUCH = 32;	//!< only clients may touch this trigger.

/**
*	@brief hurts anything that touches it. if the trigger has a targetname, firing it will toggle state
*	@details trigger hurt that causes radiation will do a radius check
*	and set the player's geiger counter level according to distance from center of trigger
*/
class EHL_CLASS() CTriggerHurt : public CBaseTrigger
{
public:
	void Spawn() override;
	/**
	*	@brief When touched, a hurt trigger does DMG points of damage each half-second
	*/
	void EXPORT HurtTouch(CBaseEntity* pOther);
	void EXPORT RadiationThink();
};

class EHL_CLASS() CTriggerMonsterJump : public CBaseTrigger
{
public:
	void Spawn() override;
	void Touch(CBaseEntity* pOther) override;
	void Think() override;
};

/**
*	@brief starts/stops cd audio tracks
*	Changes tracks or stops CD when player touches or when triggered
*/
class EHL_CLASS() CTriggerCDAudio : public CBaseTrigger
{
public:
	void Spawn() override;

	void Use(const UseInfo& info) override;
	void PlayTrack();
	void Touch(CBaseEntity* pOther) override;
};

/**
*	@brief This plays a CD track when fired or when the player enters it's radius
*/
class EHL_CLASS() CTargetCDAudio : public CPointEntity
{
public:
	void			Spawn() override;
	void			KeyValue(KeyValueData* pkvd) override;

	void	Use(const UseInfo& info) override;
	void			Think() override;
	void			Play();
};

/**
*	@brief Variable sized repeatable trigger. Must be targeted at one or more entities.
*	@details "wait" : Seconds between triggerings. (.2 default)
*/
class EHL_CLASS() CTriggerMultiple : public CBaseTrigger
{
public:
	void Spawn() override;

	void EXPORT MultiTouch(CBaseEntity* pOther);
};

/**
*	@brief Variable sized trigger. Triggers once, then removes itself.
*	@details You must set the key "target" to the name of another object in the level that has a matching "targetname".
*	if "killtarget" is set, any objects that have a matching "target" will be removed when the trigger is fired.
*	if "angle" is set, the trigger will only fire when someone is facing the direction of the angle.  Use "360" for an angle of 0.
*/
class EHL_CLASS() CTriggerOnce : public CTriggerMultiple
{
public:
	void Spawn() override;
};

#include "CTriggerCounter.generated.hpp"

constexpr int SF_COUNTER_NOMESSAGE = 1;

/**
*	@brief Acts as an intermediary for an action that takes multiple inputs.
*	@details If nomessage is not set, it will print "1 more.. " etc when triggered and "sequence complete" when finished.
*	After the counter has been triggered "cTriggersLeft" times (default 2), it will fire all of it's targets and remove itself.
*/
class EHL_CLASS() CTriggerCounter : public CBaseTrigger
{
	EHL_GENERATED_BODY()

public:
	void KeyValue(KeyValueData* pkvd) override;
	void Spawn() override;

	void EXPORT CounterUse(const UseInfo& info);

	EHL_FIELD(Persisted)
	int m_cTriggersLeft = 0; //!< # of activations remaining
};

/**
*	@brief makes an area vertically negotiable
*/
class EHL_CLASS() CLadder : public CBaseTrigger
{
public:
	void KeyValue(KeyValueData* pkvd) override;
	void Spawn() override;
	void Precache() override;
};

constexpr int SF_TRIGGER_PUSH_ONCE = 1;
constexpr int SF_TRIGGER_PUSH_START_OFF = 2;		//!< spawnflag that makes trigger_push spawn turned OFF

/**
*	@brief Pushes the player and other entities
*/
class EHL_CLASS() CTriggerPush : public CBaseTrigger
{
public:
	void Spawn() override;
	void KeyValue(KeyValueData* pkvd) override;
	void Touch(CBaseEntity* pOther) override;
};

class EHL_CLASS() CTriggerTeleport : public CBaseTrigger
{
public:
	void Spawn() override;

	void EXPORT TeleportTouch(CBaseEntity* pOther);
};

class EHL_CLASS() CTriggerGravity : public CBaseTrigger
{
public:
	void Spawn() override;
	void EXPORT GravityTouch(CBaseEntity* pOther);
};

#include "CTriggerChangeTarget.generated.hpp"

/**
*	@brief this is a really bad idea.
*/
class EHL_CLASS() CTriggerChangeTarget : public CBaseDelay
{
	EHL_GENERATED_BODY()

public:
	void KeyValue(KeyValueData* pkvd) override;
	void Spawn() override;
	void Use(const UseInfo& info) override;

	int ObjectCaps() override { return CBaseDelay::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

private:
	EHL_FIELD(Persisted)
	string_t m_iszNewTarget = iStringNull;
};

#include "CTriggerCamera.generated.hpp"

constexpr int SF_CAMERA_PLAYER_POSITION = 1;
constexpr int SF_CAMERA_PLAYER_TARGET = 2;
constexpr int SF_CAMERA_PLAYER_TAKECONTROL = 4;

class EHL_CLASS() CTriggerCamera : public CBaseDelay
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	void KeyValue(KeyValueData* pkvd) override;
	void Use(const UseInfo& info) override;
	void EXPORT FollowTarget();
	void Move();

	int	ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	EHL_FIELD(Persisted)
	EHandle<CBasePlayer> m_hPlayer;

	EHL_FIELD(Persisted)
	EHANDLE m_hTarget;

	EHL_FIELD(Persisted)
	EHandle<CBaseEntity> m_hEntPath;

	EHL_FIELD(Persisted)
	string_t m_sPath = iStringNull;

	EHL_FIELD(Persisted)
	float m_flWait = 0;

	EHL_FIELD(Persisted, Type=Time)
	float m_flReturnTime = 0;

	EHL_FIELD(Persisted, Type=Time)
	float m_flStopTime = 0;

	EHL_FIELD(Persisted)
	float m_moveDistance = 0;

	EHL_FIELD(Persisted)
	float m_targetSpeed = 0;

	EHL_FIELD(Persisted)
	float m_initialSpeed = 0;

	EHL_FIELD(Persisted)
	float m_acceleration = 0;

	EHL_FIELD(Persisted)
	float m_deceleration = 0;

	EHL_FIELD(Persisted)
	bool m_state = false;
};
