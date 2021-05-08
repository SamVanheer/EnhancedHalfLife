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

#include "extdll.h"
#include "util.h"
#include "saverestore.hpp"
#include "globalstate.hpp"
#include "schedule.h"
#include "animationevent.hpp"
#include "ehandle.hpp"

/*

Class Hierachy

CBaseEntity
	CBaseDelay
		CBaseToggle
			CBaseItem
			CBaseMonster
				CBaseCycler
				CBasePlayer
				CBaseGroup
*/

constexpr int MAX_PATH_SIZE = 10; //!< max number of nodes available for a path.

// These are caps bits to indicate what an object's capabilities (currently used for save/restore and level transitions)
constexpr int FCAP_CUSTOMSAVE = 0x00000001;
constexpr int FCAP_ACROSS_TRANSITION = 0x00000002;	//!< should transfer between transitions
constexpr int FCAP_MUST_SPAWN = 0x00000004;			//!< Spawn after restore
constexpr int FCAP_DONT_SAVE = 0x80000000;			//!< Don't save this
constexpr int FCAP_IMPULSE_USE = 0x00000008;		//!< can be used by the player
constexpr int FCAP_CONTINUOUS_USE = 0x00000010;		//!< can be used by the player
constexpr int FCAP_ONOFF_USE = 0x00000020;			//!< can be used by the player
constexpr int FCAP_DIRECTIONAL_USE = 0x00000040;	//!< Player sends +/- 1 when using (currently only tracktrains)
constexpr int FCAP_MASTER = 0x00000080;				//!< Can be used to "master" other entities (like multisource)

// UNDONE: This will ignore transition volumes (trigger_transition), but not the PVS!!!
constexpr int FCAP_FORCE_TRANSITION = 0x00000080;	//!< ALWAYS goes across transitions

struct studiohdr_t;

#define EXPORT DLLEXPORT

enum class UseType
{
	Off = 0,
	On = 1,
	Set = 2,
	Toggle = 3
};

enum class TriggerState
{
	Off,
	On,
	Toggle
};

constexpr UseType UTIL_TriggerStateToTriggerType(TriggerState state)
{
	switch (state)
	{
	case TriggerState::Off:		return UseType::Off;
	case TriggerState::Toggle:	return UseType::Toggle;
	default:					return UseType::On;
	}
}

void FireTargets(const char* targetName, CBaseEntity* pActivator, CBaseEntity* pCaller, UseType useType, float value);

// For CLASSIFY
constexpr int CLASS_NONE = 0;
constexpr int CLASS_MACHINE = 1;
constexpr int CLASS_PLAYER = 2;
constexpr int CLASS_HUMAN_PASSIVE = 3;
constexpr int CLASS_HUMAN_MILITARY = 4;
constexpr int CLASS_ALIEN_MILITARY = 5;
constexpr int CLASS_ALIEN_PASSIVE = 6;
constexpr int CLASS_ALIEN_MONSTER = 7;
constexpr int CLASS_ALIEN_PREY = 8;
constexpr int CLASS_ALIEN_PREDATOR = 9;
constexpr int CLASS_INSECT = 10;
constexpr int CLASS_PLAYER_ALLY = 11;
constexpr int CLASS_PLAYER_BIOWEAPON = 12;	//!< hornets and snarks.launched by players
constexpr int CLASS_ALIEN_BIOWEAPON = 13;	//!< hornets and snarks.launched by the alien menace
constexpr int CLASS_BARNACLE = 99;			//!< special because no one pays attention to it, and it eats a wide cross-section of creatures.

constexpr int CLASS_COUNT = CLASS_ALIEN_BIOWEAPON + 1;

class CBaseEntity;
class CBaseMonster;
class CBasePlayerItem;
class CSquadMonster;

constexpr int SF_NORESPAWN = 1 << 30; // !!!set this bit on guns and stuff that should never respawn.

/**
*	@brief Contains information about an entity damage event
*/
class TakeDamageInfo
{
public:
	TakeDamageInfo(CBaseEntity* inflictor, CBaseEntity* attacker, float damage, int damageTypes)
		: _inflictor(inflictor)
		, _attacker(attacker)
		, _damage(damage)
		, _damageTypes(damageTypes)
	{
	}

	TakeDamageInfo(const TakeDamageInfo&) = default;
	TakeDamageInfo& operator=(const TakeDamageInfo&) = default;

	~TakeDamageInfo() = default;

	CBaseEntity* GetInflictor() const { return _inflictor; }

	void SetInflictor(CBaseEntity* inflictor)
	{
		_inflictor = inflictor;
	}

	CBaseEntity* GetAttacker() const { return _attacker; }

	void SetAttacker(CBaseEntity* attacker)
	{
		_attacker = attacker;
	}

	float GetDamage() const { return _damage; }

	void SetDamage(float damage)
	{
		_damage = damage;
	}

	int GetDamageTypes() const { return _damageTypes; }

	void SetDamageTypes(int damageTypes)
	{
		_damageTypes = damageTypes;
	}

private:
	CBaseEntity* _inflictor;
	CBaseEntity* _attacker;
	float _damage;
	int _damageTypes;
};

class TraceAttackInfo
{
public:
	TraceAttackInfo(CBaseEntity* attacker, float damage, const Vector& direction, const TraceResult& trace, int damageTypes)
		: _attacker(attacker)
		, _damage(damage)
		, _direction(direction)
		, _trace(trace)
		, _damageTypes(damageTypes)
	{
	}

	TraceAttackInfo(const TraceAttackInfo&) = default;
	TraceAttackInfo& operator=(const TraceAttackInfo&) = default;

	~TraceAttackInfo() = default;

	CBaseEntity* GetAttacker() const { return _attacker; }

	void SetAttacker(CBaseEntity* attacker)
	{
		_attacker = attacker;
	}

	float GetDamage() const { return _damage; }

	void SetDamage(float damage)
	{
		_damage = damage;
	}

	const Vector& GetDirection() const { return _direction; }

	void SetDirection(const Vector& direction)
	{
		_direction = direction;
	}

	const TraceResult& GetTraceResult() const { return _trace; }

	TraceResult& GetTraceResult() { return _trace; }

	void SetTraceResult(const TraceResult& trace)
	{
		_trace = trace;
	}

	int GetDamageTypes() const { return _damageTypes; }

	void SetDamageTypes(int damageTypes)
	{
		_damageTypes = damageTypes;
	}

private:
	CBaseEntity* _attacker;
	float _damage;
	Vector _direction;
	TraceResult _trace;
	int _damageTypes;
};

/**
*	@brief when calling Killed(), a value that governs gib behavior is expected to be one of these three values
*/
enum class GibType
{
	Normal = 0,	//!< gib if entity was overkilled
	Never,		//!< never gib, no matter how much death damage is done ( freezing, etc )
	Always		//!< always gib ( Houndeye Shock, Barnacle Bite )
};

/**
*	@brief Contains information about an entity's death
*/
class KilledInfo
{
public:
	KilledInfo(CBaseEntity* inflictor, CBaseEntity* attacker, GibType gibType)
		: _inflictor(inflictor)
		, _attacker(attacker)
		, _gibType(gibType)
	{
	}

	~KilledInfo() = default;

	CBaseEntity* GetInflictor() const { return _inflictor; }

	CBaseEntity* GetAttacker() const { return _attacker; }

	GibType GetGibType() const { return _gibType; }

private:
	CBaseEntity* const _inflictor;
	CBaseEntity* const _attacker;
	const GibType _gibType;
};

/**
*	@brief Contains information about a use event
*/
class UseInfo
{
public:
	UseInfo(CBaseEntity* activator, CBaseEntity* caller, UseType useType, float value)
		: _activator(activator)
		, _caller(caller)
		, _useType(useType)
		, _value(value)
	{
	}

	UseInfo(CBaseEntity* activator, CBaseEntity* caller, UseType useType)
		: UseInfo(activator, caller, useType, 0.0f)
	{
	}

	CBaseEntity* GetActivator() const { return _activator; }

	CBaseEntity* GetCaller() const { return _caller; }

	UseType GetUseType() const { return _useType; }

	float GetValue() const { return _value; }

private:
	CBaseEntity* _activator;
	CBaseEntity* _caller;
	UseType _useType;
	float _value = 0.0f;
};

using BASEPTR = void (CBaseEntity::*)();
using ENTITYFUNCPTR = void (CBaseEntity::*)(CBaseEntity* pOther);
using USEPTR = void (CBaseEntity::*)(const UseInfo& info);

/**
*	@brief Base Entity.  All entity types derive from this
*/
class CBaseEntity
{
public:
	/**
	*	@brief Constructor.  Set engine to use C/C++ callback functions pointers to engine data
	*
	*	Don't need to save/restore this pointer, the engine resets it
	*/
	entvars_t* pev = nullptr;

	// path corners
	/**
	*	@brief path corner we are heading towards
	*/
	EHANDLE m_hGoalEnt;

	/**
	*	@brief used for temporary link-list operations.
	*/
	CBaseEntity* m_pLink = nullptr;

	CBaseEntity() = default;
	virtual ~CBaseEntity() {}

	void* operator new(std::size_t count)
	{
		auto memory = new byte[count];
		std::memset(memory, 0, count);
		return memory;
	}

	void operator delete(void* ptr)
	{
		auto memory = reinterpret_cast<byte*>(ptr);
		delete[] memory;
	}

	// initialization functions
	void Construct()
	{
		OnConstruct();
	}

	void Destroy()
	{
		OnDestroy();
	}

	/**
	*	@brief This updates global tables that need to know about entities being removed
	*	Do not call this in the destructor/Destroy/OnDestroy as it's not meant to be called there, only on explicit removal
	*/
	void UpdateOnRemove();

protected:
	virtual void OnConstruct() {}
	virtual void OnDestroy() {}
	virtual void OnRemove() {}

public:
	virtual void Spawn() {}

	/**
	*	@brief precaches all resources this entity needs
	*/
	virtual void Precache() {}
	virtual void KeyValue(KeyValueData* pkvd) { pkvd->fHandled = false; }
	virtual bool Save(CSave& save);
	virtual bool Restore(CRestore& restore);
	virtual int ObjectCaps() { return FCAP_ACROSS_TRANSITION; }
	virtual void Activate() {}

	/**
	*	@brief Setup the object->object collision box (pev->mins / pev->maxs is the object->world collision box)
	*/
	virtual void SetObjectCollisionBox();

	/**
	*	@brief returns the type of group (i.e, "houndeye", or "human military")
	*	so that monsters with different classnames still realize that they are teammates.
	*	(overridden for monsters that form groups)
	*/
	virtual int Classify() { return CLASS_NONE; }

	/**
	*	@brief monster maker children use this to tell the monster maker that they have died.
	*/
	virtual void DeathNotice(CBaseEntity* pChild) {}


	static	TYPEDESCRIPTION m_SaveData[];

	virtual void TraceAttack(const TraceAttackInfo& info);

	/**
	*	@brief inflict damage on this entity. bitsDamageType indicates type of damage inflicted, ie: DMG_CRUSH
	*/
	virtual bool TakeDamage(const TakeDamageInfo& info);
	virtual bool GiveHealth(float flHealth, int bitsDamageType);
	virtual void Killed(const KilledInfo& info);
	virtual int BloodColor() { return DONT_BLEED; }
	virtual void TraceBleed(const TraceAttackInfo& info);
	virtual bool IsTriggered(CBaseEntity* pActivator) { return true; }
	virtual CBaseMonster* MyMonsterPointer() { return nullptr; }
	virtual CSquadMonster* MySquadMonsterPointer() { return nullptr; }
	virtual	ToggleState GetToggleState() { return ToggleState::AtTop; }
	virtual float GetDelay() { return 0; }
	virtual bool IsMoving() { return GetAbsVelocity() != vec3_origin; }
	virtual void OverrideReset() {}
	virtual int DamageDecal(int bitsDamageType);
	virtual bool OnControls(CBaseEntity* pTest) { return false; }
	virtual bool IsAlive() { return (pev->deadflag == DeadFlag::No) && pev->health > 0; }
	virtual bool IsBSPModel() { return GetSolidType() == Solid::BSP || GetMovetype() == Movetype::PushStep; }
	virtual bool ReflectGauss() { return (IsBSPModel() && !pev->takedamage); }
	virtual bool HasTarget(string_t targetname) { return AreStringsEqual(STRING(targetname), STRING(pev->targetname)); }
	virtual bool IsInWorld();
	virtual	bool IsPlayer() { return false; }
	virtual bool IsNetClient() { return false; }
	virtual const char* TeamID() { return ""; }

	//	virtual void	SetActivator( CBaseEntity *pActivator ) {}
	virtual CBaseEntity* GetNextTarget();

	// fundamental callbacks
	BASEPTR m_pfnThink = nullptr;
	ENTITYFUNCPTR m_pfnTouch = nullptr;
	USEPTR m_pfnUse = nullptr;
	ENTITYFUNCPTR m_pfnBlocked = nullptr;

	virtual void Think() { if (m_pfnThink) (this->*m_pfnThink)(); }
	virtual void Touch(CBaseEntity* pOther) { if (m_pfnTouch) (this->*m_pfnTouch)(pOther); }
	virtual void Use(const UseInfo& info)
	{
		if (m_pfnUse)
			(this->*m_pfnUse)(info);
	}
	virtual void Blocked(CBaseEntity* pOther) { if (m_pfnBlocked) (this->*m_pfnBlocked)(pOther); }

	// common member functions
	/**
	*	@brief Convenient way to delay removing oneself
	*/
	void EXPORT SUB_Remove();

	/**
	*	@brief Convenient way to explicitly do nothing (passed to functions that require a method)
	*/
	void EXPORT SUB_DoNothing();

	/**
	*	@brief fade out - slowly fades a entity out, then removes it.
	*	@details DON'T USE ME FOR GIBS AND STUFF IN MULTIPLAYER!
	*	SET A FUTURE THINK AND A RENDERMODE!!
	*/
	void EXPORT SUB_StartFadeOut();
	void EXPORT SUB_FadeOut();
	void EXPORT SUB_CallUseToggle() { this->Use({this, this, UseType::Toggle}); }
	bool ShouldToggle(UseType useType, bool currentState);

	const char* GetClassname() const { return STRING(pev->classname); }

	bool ClassnameIs(const char* classname) const
	{
		return AreStringsEqual(GetClassname(), classname);
	}

	void SetClassname(const char* classname)
	{
		pev->classname = ALLOC_STRING(classname);
	}

	const Vector& GetAbsOrigin() { return pev->origin; }
	void SetAbsOrigin(const Vector& origin);

	const Vector& GetAbsVelocity() const { return pev->velocity; }

	void SetAbsVelocity(const Vector& velocity)
	{
		pev->velocity = velocity;
	}

	const Vector& GetAbsAngles() const { return pev->angles; }

	void SetAbsAngles(const Vector& angles)
	{
		pev->angles = angles;
	}

	void SetSize(const Vector& mins, const Vector& maxs);

	const char* GetModelName() const { return STRING(pev->model); }

	void SetModel(const char* modelName)
	{
		g_engfuncs.pfnSetModel(edict(), modelName);
	}

	/**
	*	@brief Sets the model name without setting the model
	*	Mostly useful during setup to provide an initial/custom model before precache and spawn
	*/
	void SetModelName(const char* modelName)
	{
		pev->model = ALLOC_STRING(modelName);
	}

	Solid GetSolidType() const { return pev->solid; }

	void SetSolidType(Solid solid)
	{
		pev->solid = solid;
	}

	Movetype GetMovetype() const { return pev->movetype; }

	void SetMovetype(Movetype movetype)
	{
		pev->movetype = movetype;
	}

	RenderMode GetRenderMode() const { return pev->rendermode; }

	void SetRenderMode(RenderMode rendermode)
	{
		pev->rendermode = rendermode;
	}

	float GetRenderAmount() const { return pev->renderamt; }

	void SetRenderAmount(float renderamount)
	{
		pev->renderamt = renderamount;
	}

	const Vector& GetRenderColor() const { return pev->rendercolor; }

	void SetRenderColor(const Vector& rendercolor)
	{
		pev->rendercolor = rendercolor;
	}

	RenderFX GetRenderFX() const { return pev->renderfx; }

	void SetRenderFX(RenderFX renderfx)
	{
		pev->renderfx = renderfx;
	}

	DamageMode GetDamageMode() const { return static_cast<DamageMode>(pev->takedamage); }

	void SetDamageMode(DamageMode mode)
	{
		pev->takedamage = static_cast<int>(mode);
	}

	CBaseEntity* GetAimEntity()
	{
		return InstanceOrNull(pev->aiment);
	}

	void SetAimEntity(CBaseEntity* aiment)
	{
		pev->aiment = EdictOrNull(aiment);
	}

	CBaseEntity* GetOwner()
	{
		return InstanceOrNull(pev->owner);
	}

	void SetOwner(CBaseEntity* owner)
	{
		pev->owner = EdictOrNull(owner);
	}

	/**
	*	@brief Go to the trouble of combining multiple pellets into a single damage call.
	*	This version is used by Monsters.
	*/
	void FireBullets(std::uint32_t cShots, const Vector& vecSrc, const Vector& vecDirShooting, const Vector& vecSpread, float flDistance, int iBulletType, int iTracerFreq = 4, int iDamage = 0, CBaseEntity* pAttacker = nullptr);

	virtual CBaseEntity* Respawn() { return nullptr; }

	/**
	*	@brief Trigger targets specified in pev->target
	*
	*	@details Search for (string)pev->targetname in all entities that match (string)pev->target and call their Use function (if they have one)
	*
	*	CBaseDelay classes only:
	*	If m_flDelay is set, a DelayedUse entity will be created that will actually do the SUB_UseTargets after that many seconds have passed.
	*	Removes all entities with a targetname that match m_iszKillTarget, and removes them, so some events can remove other triggers.
	*/
	void SUB_UseTargets(CBaseEntity* pActivator, UseType useType, float value);
	/**
	*	@brief Do the bounding boxes of these two intersect?
	*/
	bool Intersects(CBaseEntity* pOther);
	void MakeDormant();
	bool IsDormant();
	bool IsLockedByMaster() { return false; }

	static CBaseEntity* InstanceOrDefault(edict_t* pEntity, CBaseEntity* pDefault)
	{
		if (!pEntity)
		{
			return pDefault;
		}

		return reinterpret_cast<CBaseEntity*>(pEntity->pvPrivateData);
	}

	static CBaseEntity* InstanceOrNull(edict_t* pEntity)
	{
		return InstanceOrDefault(pEntity, nullptr);
	}

	static CBaseEntity* InstanceOrWorld(edict_t* pEntity)
	{
		return InstanceOrDefault(pEntity, UTIL_GetWorld());
	}

	static CBaseEntity* Instance(edict_t* pent)
	{
		return InstanceOrWorld(pent);
	}

	static edict_t* EdictOrNull(CBaseEntity* pEntity)
	{
		if (!pEntity)
		{
			return nullptr;
		}

		return pEntity->edict();
	}

	// Ugly code to lookup all functions to make sure they are exported when set.
#ifdef _DEBUG
	void FunctionCheck(void* pFunction, const char* name)
	{
		if (pFunction && !NAME_FOR_FUNCTION((std::uint32_t)pFunction))
			ALERT(at_error, "No EXPORT: %s:%s (%08lx)\n", GetClassname(), name, (std::uint32_t)pFunction);
	}

	BASEPTR	ThinkSet(BASEPTR func, const char* name)
	{
		m_pfnThink = func;
		FunctionCheck((void*)*((int*)((char*)this + (offsetof(CBaseEntity, m_pfnThink)))), name);
		return func;
	}
	ENTITYFUNCPTR TouchSet(ENTITYFUNCPTR func, const char* name)
	{
		m_pfnTouch = func;
		FunctionCheck((void*)*((int*)((char*)this + (offsetof(CBaseEntity, m_pfnTouch)))), name);
		return func;
	}
	USEPTR	UseSet(USEPTR func, const char* name)
	{
		m_pfnUse = func;
		FunctionCheck((void*)*((int*)((char*)this + (offsetof(CBaseEntity, m_pfnUse)))), name);
		return func;
	}
	ENTITYFUNCPTR	BlockedSet(ENTITYFUNCPTR func, const char* name)
	{
		m_pfnBlocked = func;
		FunctionCheck((void*)*((int*)((char*)this + (offsetof(CBaseEntity, m_pfnBlocked)))), name);
		return func;
	}

#endif

	// virtual functions used by a few classes
	/**
	*	@brief NOTE: szName must be a pointer to constant memory, e.g. "monster_class" because the entity will keep a pointer to it after this call.
	*/
	static CBaseEntity* Create(const char* szName, const Vector& vecOrigin, const Vector& vecAngles, CBaseEntity* pOwner = nullptr);

	virtual bool BecomeProne() { return false; }
	edict_t* edict() { return pev->pContainingEntity; }
	int	  entindex() { return ENTINDEX(edict()); }

	virtual Vector Center() { return (pev->absmax + pev->absmin) * 0.5; }	//!< center point of entity
	virtual Vector EyePosition() { return GetAbsOrigin() + pev->view_ofs; }	//!< position of eyes
	virtual Vector EarPosition() { return GetAbsOrigin() + pev->view_ofs; }	//!< position of ears
	virtual Vector BodyTarget(const Vector& posSrc) { return Center(); }	//!< position to shoot at

	virtual int Illumination() { return g_engfuncs.pfnGetEntityIllum(edict()); }

	/**
	*	@brief returns true if a line can be traced from the caller's eyes to the target
	*/
	virtual	bool IsVisible(CBaseEntity* pEntity);

	/**
	*	@brief returns true if a line can be traced from the caller's eyes to the target vector
	*/
	virtual	bool IsVisible(const Vector& vecOrigin);

	void EmitSound(SoundChannel channel, const char* fileName, float volume = VOL_NORM, float attenuation = ATTN_NORM, int pitch = PITCH_NORM, int flags = 0);

	void StopSound(SoundChannel channel, const char* fileName);

	template<std::size_t Size>
	void EMIT_SOUND_ARRAY_DYN(SoundChannel chan, const char* (&array)[Size])
	{
		EmitSound(chan, RANDOM_SOUND_ARRAY(array), VOL_NORM, ATTN_NORM, RANDOM_LONG(95, 105));
	}
};

inline bool IsNullEnt(CBaseEntity* ent) { return (ent == nullptr) || IsNullEnt(ent->edict()); }

// Ugly technique to override base member functions
// Normally it's illegal to cast a pointer to a member function of a derived class to a pointer to a 
// member function of a base class.  static_cast is a sleezy way around that problem.

#ifdef _DEBUG

#define SetThink(a) ThinkSet(static_cast<BASEPTR>(a), #a)
#define SetTouch(a) TouchSet(static_cast<ENTITYFUNCPTR>(a), #a)
#define SetUse(a) UseSet(static_cast<USEPTR>(a), #a)
#define SetBlocked(a) BlockedSet(static_cast<ENTITYFUNCPTR>(a), #a)

#else

#define SetThink(a) m_pfnThink = static_cast<BASEPTR>(a)
#define SetTouch(a) m_pfnTouch = static_cast<ENTITYFUNCPTR>(a)
#define SetUse(a) m_pfnUse = static_cast<USEPTR>(a)
#define SetBlocked(a) m_pfnBlocked = static_cast<ENTITYFUNCPTR>(a)

#endif

class CPointEntity : public CBaseEntity
{
public:
	void Spawn() override;
	int	ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
private:
};

/**
*	@brief sounds that doors and buttons make when locked/unlocked
*/
struct locksound_t
{
	string_t sLockedSound = iStringNull;		//!< sound a door makes when it's locked
	string_t sLockedSentence = iStringNull;		//!< sentence group played when door is locked
	string_t sUnlockedSound = iStringNull;		//!< sound a door makes when it's unlocked
	string_t sUnlockedSentence = iStringNull;	//!< sentence group played when door is unlocked

	int iLockedSentence = 0;		//!< which sentence in sentence group to play next
	int iUnlockedSentence = 0;		//!< which sentence in sentence group to play next

	float flwaitSound = 0;			//!< time delay between playing consecutive 'locked/unlocked' sounds
	float flwaitSentence = 0;		//!< time delay between playing consecutive sentences
	bool bEOFLocked = false;		//!< true if hit end of list of locked sentences
	bool bEOFUnlocked = false;		//!< true if hit end of list of unlocked sentences
};

/**
*	@brief play door or button locked or unlocked sounds.
*	@details pass in pointer to valid locksound struct.
*	NOTE: this routine is shared by doors and buttons
*	@param flocked if true, play 'door is locked' sound, otherwise play 'door is unlocked' sound
*/
void PlayLockSounds(CBaseEntity* entity, locksound_t* pls, int flocked, int fbutton);

constexpr int MAX_MULTI_TARGETS = 16; //!< maximum number of targets a single multi_manager entity may be assigned.
constexpr int MS_MAX_TARGETS = 32;

class CMultiSource : public CPointEntity
{
public:
	void Spawn() override;
	void KeyValue(KeyValueData* pkvd) override;
	void Use(const UseInfo& info) override;
	int	ObjectCaps() override { return (CPointEntity::ObjectCaps() | FCAP_MASTER); }
	bool IsTriggered(CBaseEntity* pActivator) override;
	void EXPORT Register();
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];

	EHANDLE m_rgEntities[MS_MAX_TARGETS]{};
	bool m_rgTriggered[MS_MAX_TARGETS]{};

	int m_iTotal = 0;
	string_t m_globalstate = iStringNull;
};

/**
*	@brief generic Delay entity.
*/
class CBaseDelay : public CBaseEntity
{
public:
	float m_flDelay = 0;
	string_t m_iszKillTarget = iStringNull;

	void KeyValue(KeyValueData* pkvd) override;
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];
	// common member functions
	//TODO: this is a non-virtual override of the same function in CBaseEntity. Should probably just merge this class into CBaseEntity
	void SUB_UseTargets(CBaseEntity* pActivator, UseType useType, float value);
	void EXPORT DelayThink();
};

class CBaseAnimating : public CBaseDelay
{
public:
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

	/**
	*	@brief Gets the studio model header
	*	Only valid for entities that use studio models
	*/
	studiohdr_t* GetModelPointer();

	// Basic Monster Animation functions
	/**
	*	@brief advance the animation frame up to the current time
	*
	*	accumulate animation frame time from last time called until now
	*	if an flInterval is passed in, only advance animation that number of seconds
	*/
	float StudioFrameAdvance(float flInterval = 0.0);
	int GetSequenceFlags();
	int LookupActivity(int activity);

	/**
	*	@brief Get activity with highest 'weight'
	*/
	int LookupActivityHeaviest(int activity);
	int LookupSequence(const char* label);
	void ResetSequenceInfo();

	/**
	*	@brief Handle events that have happend since last time called up until X seconds into the future
	*/
	void DispatchAnimEvents(float flFutureInterval = 0.1);

	/**
	*	@brief catches the entity-specific messages that occur when tagged animation frames are played.
	*/
	virtual void HandleAnimEvent(AnimationEvent& event) {}
	float SetBoneController(int iController, float flValue);
	void InitBoneControllers();
	float SetBlending(int iBlender, float flValue);
	void GetBonePosition(int iBone, Vector& origin, Vector& angles);
	int FindTransition(int iEndingSequence, int iGoalSequence, int& iDir);
	int FindTransition(int iEndingSequence, int iGoalSequence);
	void GetAttachment(int iAttachment, Vector& origin, Vector& angles);
	void SetBodygroup(int iGroup, int iValue);
	int GetBodygroup(int iGroup);
	bool ExtractBbox(int sequence, Vector& mins, Vector& maxs);
	void SetSequenceBox();

	// animation needs
	float m_flFrameRate = 0;			//!< computed FPS for current sequence
	float m_flGroundSpeed = 0;			//!< computed linear movement rate for current sequence
	float m_flLastEventCheck = 0;		//!< last time the event list was checked
	bool m_fSequenceFinished = false;	//!< flag set when StudioAdvanceFrame moves across a frame boundry
	bool m_fSequenceLoops = false;		//!< true if the sequence loops
};

constexpr int SF_ITEM_USE_ONLY = 256; //  ITEM_USE_ONLY = BUTTON_USE_ONLY = DOOR_USE_ONLY!!! 

/**
*	@brief generic Toggle entity.
*/
class CBaseToggle : public CBaseAnimating
{
public:
	void KeyValue(KeyValueData* pkvd) override;

	ToggleState m_toggle_state = ToggleState::AtTop;
	float m_flActivateFinished = 0;	//!< like attack_finished, but for doors
	float m_flMoveDistance = 0;		//!< how far a door should slide or rotate
	float m_flWait = 0;
	float m_flLip = 0;
	float m_flTWidth = 0;		//!< for plats
	float m_flTLength = 0;	//!< for plats

	Vector m_vecPosition1;
	Vector m_vecPosition2;
	Vector m_vecAngle1;
	Vector m_vecAngle2;

	float m_flHeight = 0;
	EHANDLE m_hActivator;
	void (CBaseToggle::* m_pfnCallWhenMoveDone)() = nullptr;
	Vector m_vecFinalDest;
	Vector m_vecFinalAngle;

	int m_bitsDamageInflict = 0;	//!< DMG_ damage type that the door or tigger does

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

	ToggleState GetToggleState() override { return m_toggle_state; }
	float GetDelay() override { return m_flWait; }

	// common member functions
	/**
	*	@brief calculate pev->velocity and pev->nextthink to reach vecDest from GetAbsOrigin() traveling at flSpeed
	*/
	void LinearMove(const Vector& vecDest, float flSpeed);

	/**
	*	@brief After moving, set origin to exact final destination, call "move done" function
	*/
	void EXPORT LinearMoveDone();

	/**
	*	@brief calculate pev->velocity and pev->nextthink to reach vecDest from GetAbsOrigin() traveling at flSpeed
	*
	*	Just like LinearMove, but rotational.
	*/
	void AngularMove(const Vector& vecDestAngle, float flSpeed);

	/**
	*	@brief After rotating, set angle to exact final angle, call "move done" function
	*/
	void EXPORT AngularMoveDone();
	bool IsLockedByMaster(); //TODO: non-virtual override

	static float AxisValue(int flags, const Vector& angles);
	static void AxisDir(CBaseEntity* pEntity);
	static float AxisDelta(int flags, const Vector& angle1, const Vector& angle2);

	/**
	*	@brief If this button has a master switch, this is the targetname.
	*
	*	@details A master switch must be of the multisource type.
	*	If all of the switches in the multisource have been triggered, then the button will be allowed to operate.
	*	Otherwise, it will be deactivated.
	*/
	string_t m_sMaster = iStringNull;
};

#define SetMoveDone(a) m_pfnCallWhenMoveDone = static_cast<void (CBaseToggle::*)()>(a)

// people gib if their health is <= this at the time of death
constexpr int GIB_HEALTH_VALUE = -30;

constexpr int ROUTE_SIZE = 8;				//!< how many waypoints a monster can store at one time
constexpr int MAX_OLD_ENEMIES = 4;			//!< how many old enemies to remember

constexpr int bits_CAP_DUCK = 1 << 0;		//!< crouch
constexpr int bits_CAP_JUMP = 1 << 1;		//!< jump/leap
constexpr int bits_CAP_STRAFE = 1 << 2;		//!< strafe ( walk/run sideways)
constexpr int bits_CAP_SQUAD = 1 << 3;		//!< can form squads
constexpr int bits_CAP_SWIM = 1 << 4;		//!< proficiently navigate in water
constexpr int bits_CAP_CLIMB = 1 << 5;		//!< climb ladders/ropes
constexpr int bits_CAP_USE = 1 << 6;		//!< open doors/push buttons/pull levers
constexpr int bits_CAP_HEAR = 1 << 7;		//!< can hear forced sounds
constexpr int bits_CAP_AUTO_DOORS = 1 << 8; //!< can trigger auto doors
constexpr int bits_CAP_OPEN_DOORS = 1 << 9; //!< can open manual doors
constexpr int bits_CAP_TURN_HEAD = 1 << 10;	//!< can turn head, always bone controller 0

constexpr int bits_CAP_RANGE_ATTACK1 = 1 << 11;	//!< can do a range attack 1
constexpr int bits_CAP_RANGE_ATTACK2 = 1 << 12;	//!< can do a range attack 2
constexpr int bits_CAP_MELEE_ATTACK1 = 1 << 13;	//!< can do a melee attack 1
constexpr int bits_CAP_MELEE_ATTACK2 = 1 << 14;	//!< can do a melee attack 2

constexpr int bits_CAP_FLY = 1 << 15;			//!< can fly, move all around

constexpr int bits_CAP_DOORS_GROUP = bits_CAP_USE | bits_CAP_AUTO_DOORS | bits_CAP_OPEN_DOORS;

class CBaseMonster;
class CCineMonster;
class CSound;

#include "basemonster.h"

/**
*	@brief Button sound table. get string of button sound number
*	@details Also used by CBaseDoor to get 'touched' door lock/unlock sounds
*/
const char* ButtonSound(int sound);


/**
*	@brief Generic Button
*	@details When a button is touched, it moves some distance in the direction of its angle,
*	triggers all of its targets, waits some time, then returns to its original position where it can be triggered again.
*	"movedir"	determines the opening direction
*	"target"	all entities with a matching targetname will be used
*	"speed"		override the default 40 speed
*	"wait"		override the default 1 second wait (-1 = never return)
*	"lip"		override the default 4 unit lip remaining at end of move
*	"health"	if set, the button must be killed instead of touched
*/
class CBaseButton : public CBaseToggle
{
public:
	void Spawn() override;
	void Precache() override;
	void KeyValue(KeyValueData* pkvd) override;

	/**
	*	@brief Starts the button moving "in/up".
	*/
	void ButtonActivate();

	/**
	*	@brief Touching a button simply "activates" it.
	*/
	void EXPORT ButtonTouch(CBaseEntity* pOther);
	void EXPORT ButtonSpark();

	/**
	*	@brief Button has reached the "in/up" position.  Activate its "targets", and pause before "popping out".
	*/
	void EXPORT TriggerAndWait();

	/**
	*	@brief Starts the button moving "out/down".
	*/
	void EXPORT ButtonReturn();

	/**
	*	@brief Button has returned to start state. Quiesce it.
	*/
	void EXPORT ButtonBackHome();
	void EXPORT ButtonUse(const UseInfo& info);
	bool TakeDamage(const TakeDamageInfo& info) override;
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	enum class ButtonCode
	{
		Nothing,
		Activate,
		Return
	};

	ButtonCode ButtonResponseToTouch();

	static TYPEDESCRIPTION m_SaveData[];
	/**
	*	@brief Buttons that don't take damage can be IMPULSE used
	*/
	int	ObjectCaps() override { return (CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | (pev->takedamage ? 0 : FCAP_IMPULSE_USE); }

	bool m_fStayPushed = false;		//!< button stays pushed in until touched again?
	bool m_fRotating = false;		//!< a rotating button?  default is a sliding button.

	locksound_t m_ls;			//!< door lock sounds

	// ordinals from entity selection
	byte m_bLockedSound = 0;
	byte m_bLockedSentence = 0;
	byte m_bUnlockedSound = 0;
	byte m_bUnlockedSentence = 0;
	int m_sounds = 0;
};

/**
*	@brief Converts a entvars_t * to a class pointer. It will allocate the class and entity if necessary
*/
template <class T> T* GetClassPtr(T* a)
{
	entvars_t* pev = (entvars_t*)a;

	// allocate entity if necessary
	if (pev == nullptr)
		pev = &g_engfuncs.pfnCreateEntity()->v;

	// get the private data
	a = (T*)CBaseEntity::InstanceOrNull(pev->pContainingEntity);

	if (a == nullptr)
	{
		// allocate private data 
		a = new T();
		pev->pContainingEntity->pvPrivateData = a;
		a->pev = pev;
		a->Construct();
	}
	return a;
}

/**
*	@brief This spawns first when each level begins.
*
*	this moved here from world.cpp, to allow classes to be derived from it
*/
class CWorld : public CBaseEntity
{
public:
	~CWorld();

	void Spawn() override;
	void Precache() override;
	void KeyValue(KeyValueData* pkvd) override;

	int DamageDecal(int bitsDamageType) override;
};

