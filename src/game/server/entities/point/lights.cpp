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

/**
*	@file
*
*	spawn and think functions for editor-placed lights
*/

constexpr int SF_LIGHT_START_OFF = 1;

/**
*	@brief Non-displayed light.
*	Default style is 0
*	If targeted, it will toggle between on or off.
*/
class CLight : public CPointEntity
{
public:
	void	KeyValue(KeyValueData* pkvd) override;
	void	Spawn() override;
	void	Use(const UseInfo& info) override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];

private:
	int m_iStyle = 0;
	string_t m_iszPattern = iStringNull;
};

LINK_ENTITY_TO_CLASS(light, CLight);

TYPEDESCRIPTION	CLight::m_SaveData[] =
{
	DEFINE_FIELD(CLight, m_iStyle, FIELD_INTEGER),
	DEFINE_FIELD(CLight, m_iszPattern, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CLight, CPointEntity);

void CLight::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "style"))
	{
		m_iStyle = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "pitch"))
	{
		SetAbsAngles({static_cast<float>(atof(pkvd->szValue)), GetAbsAngles().y, GetAbsAngles().z});
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "pattern"))
	{
		m_iszPattern = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
	{
		CPointEntity::KeyValue(pkvd);
	}
}

void CLight::Spawn()
{
	if (IsStringNull(pev->targetname))
	{       // inert light
		UTIL_RemoveNow(this);
		return;
	}

	if (m_iStyle >= 32)
	{
		//		CHANGE_METHOD(ENT(pev), em_use, light_use);
		if (IsBitSet(pev->spawnflags, SF_LIGHT_START_OFF))
			LIGHT_STYLE(m_iStyle, "a");
		else if (!IsStringNull(m_iszPattern))
			LIGHT_STYLE(m_iStyle, STRING(m_iszPattern));
		else
			LIGHT_STYLE(m_iStyle, "m");
	}
}

void CLight::Use(const UseInfo& info)
{
	if (m_iStyle >= 32)
	{
		if (!ShouldToggle(info.GetUseType(), !IsBitSet(pev->spawnflags, SF_LIGHT_START_OFF)))
			return;

		if (IsBitSet(pev->spawnflags, SF_LIGHT_START_OFF))
		{
			if (!IsStringNull(m_iszPattern))
				LIGHT_STYLE(m_iStyle, STRING(m_iszPattern));
			else
				LIGHT_STYLE(m_iStyle, "m");
			ClearBits(pev->spawnflags, SF_LIGHT_START_OFF);
		}
		else
		{
			LIGHT_STYLE(m_iStyle, "a");
			SetBits(pev->spawnflags, SF_LIGHT_START_OFF);
		}
	}
}

// shut up spawn functions for new spotlights
LINK_ENTITY_TO_CLASS(light_spot, CLight);

class CEnvLight : public CLight
{
public:
	void	KeyValue(KeyValueData* pkvd) override;
	void	Spawn() override;
};

LINK_ENTITY_TO_CLASS(light_environment, CEnvLight);

void CEnvLight::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "_light"))
	{
		int r, g, b, v;
		char szColor[64];
		const int j = sscanf(pkvd->szValue, "%d %d %d %d\n", &r, &g, &b, &v);
		if (j == 1)
		{
			g = b = r;
		}
		else if (j == 4)
		{
			r = r * (v / 255.0);
			g = g * (v / 255.0);
			b = b * (v / 255.0);
		}
		//TODO: if j is neither 1 nor 4 these values could be wrong

		// simulate qrad direct, ambient,and gamma adjustments, as well as engine scaling
		r = pow(r / 114.0, 0.6) * 264;
		g = pow(g / 114.0, 0.6) * 264;
		b = pow(b / 114.0, 0.6) * 264;

		pkvd->fHandled = true;
		snprintf(szColor, sizeof(szColor), "%d", r);
		CVAR_SET_STRING("sv_skycolor_r", szColor);
		snprintf(szColor, sizeof(szColor), "%d", g);
		CVAR_SET_STRING("sv_skycolor_g", szColor);
		snprintf(szColor, sizeof(szColor), "%d", b);
		CVAR_SET_STRING("sv_skycolor_b", szColor);
	}
	else
	{
		CLight::KeyValue(pkvd);
	}
}

void CEnvLight::Spawn()
{
	char szVector[64];
	UTIL_MakeAimVectors(GetAbsAngles());

	snprintf(szVector, sizeof(szVector), "%f", gpGlobals->v_forward.x);
	CVAR_SET_STRING("sv_skyvec_x", szVector);
	snprintf(szVector, sizeof(szVector), "%f", gpGlobals->v_forward.y);
	CVAR_SET_STRING("sv_skyvec_y", szVector);
	snprintf(szVector, sizeof(szVector), "%f", gpGlobals->v_forward.z);
	CVAR_SET_STRING("sv_skyvec_z", szVector);

	CLight::Spawn();
}
