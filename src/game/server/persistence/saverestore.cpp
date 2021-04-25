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

#include <bit>

#include "extdll.h"
#include "util.h"
#include "cbase.h"

TYPEDESCRIPTION	gEntvarsDescription[] =
{
	DEFINE_ENTITY_FIELD(classname, FIELD_STRING),
	DEFINE_ENTITY_GLOBAL_FIELD(globalname, FIELD_STRING),

	DEFINE_ENTITY_FIELD(origin, FIELD_POSITION_VECTOR),
	DEFINE_ENTITY_FIELD(oldorigin, FIELD_POSITION_VECTOR),
	DEFINE_ENTITY_FIELD(velocity, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD(basevelocity, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD(movedir, FIELD_VECTOR),

	DEFINE_ENTITY_FIELD(angles, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD(avelocity, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD(punchangle, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD(v_angle, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD(fixangle, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(idealpitch, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(pitch_speed, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(ideal_yaw, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(yaw_speed, FIELD_FLOAT),

	DEFINE_ENTITY_FIELD(modelindex, FIELD_INTEGER),
	DEFINE_ENTITY_GLOBAL_FIELD(model, FIELD_MODELNAME),

	DEFINE_ENTITY_FIELD(viewmodel, FIELD_MODELNAME),
	DEFINE_ENTITY_FIELD(weaponmodel, FIELD_MODELNAME),

	DEFINE_ENTITY_FIELD(absmin, FIELD_POSITION_VECTOR),
	DEFINE_ENTITY_FIELD(absmax, FIELD_POSITION_VECTOR),
	DEFINE_ENTITY_GLOBAL_FIELD(mins, FIELD_VECTOR),
	DEFINE_ENTITY_GLOBAL_FIELD(maxs, FIELD_VECTOR),
	DEFINE_ENTITY_GLOBAL_FIELD(size, FIELD_VECTOR),

	DEFINE_ENTITY_FIELD(ltime, FIELD_TIME),
	DEFINE_ENTITY_FIELD(nextthink, FIELD_TIME),

	DEFINE_ENTITY_FIELD(solid, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(movetype, FIELD_INTEGER),

	DEFINE_ENTITY_FIELD(skin, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(body, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(effects, FIELD_INTEGER),

	DEFINE_ENTITY_FIELD(gravity, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(friction, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(light_level, FIELD_FLOAT),

	DEFINE_ENTITY_FIELD(frame, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(scale, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(sequence, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(animtime, FIELD_TIME),
	DEFINE_ENTITY_FIELD(framerate, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(controller, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(blending, FIELD_INTEGER),

	DEFINE_ENTITY_FIELD(rendermode, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(renderamt, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(rendercolor, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD(renderfx, FIELD_INTEGER),

	DEFINE_ENTITY_FIELD(health, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(frags, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(weapons, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(takedamage, FIELD_FLOAT),

	DEFINE_ENTITY_FIELD(deadflag, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(view_ofs, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD(button, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(impulse, FIELD_INTEGER),

	DEFINE_ENTITY_FIELD(chain, FIELD_EDICT),
	DEFINE_ENTITY_FIELD(dmg_inflictor, FIELD_EDICT),
	DEFINE_ENTITY_FIELD(enemy, FIELD_EDICT),
	DEFINE_ENTITY_FIELD(aiment, FIELD_EDICT),
	DEFINE_ENTITY_FIELD(owner, FIELD_EDICT),
	DEFINE_ENTITY_FIELD(groundentity, FIELD_EDICT),

	DEFINE_ENTITY_FIELD(spawnflags, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(flags, FIELD_FLOAT),

	DEFINE_ENTITY_FIELD(colormap, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(team, FIELD_INTEGER),

	DEFINE_ENTITY_FIELD(max_health, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(teleport_time, FIELD_TIME),
	DEFINE_ENTITY_FIELD(armortype, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(armorvalue, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(waterlevel, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(watertype, FIELD_INTEGER),

	// Having these fields be local to the individual levels makes it easier to test those levels individually.
	DEFINE_ENTITY_GLOBAL_FIELD(target, FIELD_STRING),
	DEFINE_ENTITY_GLOBAL_FIELD(targetname, FIELD_STRING),
	DEFINE_ENTITY_FIELD(netname, FIELD_STRING),
	DEFINE_ENTITY_FIELD(message, FIELD_STRING),

	DEFINE_ENTITY_FIELD(dmg_take, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(dmg_save, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(dmg, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(dmgtime, FIELD_TIME),

	DEFINE_ENTITY_FIELD(noise, FIELD_SOUNDNAME),
	DEFINE_ENTITY_FIELD(noise1, FIELD_SOUNDNAME),
	DEFINE_ENTITY_FIELD(noise2, FIELD_SOUNDNAME),
	DEFINE_ENTITY_FIELD(noise3, FIELD_SOUNDNAME),
	DEFINE_ENTITY_FIELD(speed, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(air_finished, FIELD_TIME),
	DEFINE_ENTITY_FIELD(pain_finished, FIELD_TIME),
	DEFINE_ENTITY_FIELD(radsuit_finished, FIELD_TIME),
};

constexpr int ENTVARS_COUNT = (sizeof(gEntvarsDescription) / sizeof(gEntvarsDescription[0]));

static constexpr int gSizes[FIELD_TYPECOUNT] =
{
	sizeof(float),		// FIELD_FLOAT
	sizeof(int),		// FIELD_STRING
	sizeof(int),		// FIELD_ENTITY
	sizeof(int),		// FIELD_CLASSPTR
	sizeof(int),		// FIELD_EHANDLE
	sizeof(int),		// FIELD_entvars_t
	sizeof(int),		// FIELD_EDICT
	sizeof(float) * 3,	// FIELD_VECTOR
	sizeof(float) * 3,	// FIELD_POSITION_VECTOR
	sizeof(int*),		// FIELD_POINTER
	sizeof(int),		// FIELD_INTEGER
#ifdef GNUC
	sizeof(int*) * 2,		// FIELD_FUNCTION
#else
	sizeof(int*),		// FIELD_FUNCTION	
#endif
	sizeof(bool),		// FIELD_BOOLEAN
	sizeof(short),		// FIELD_SHORT
	sizeof(char),		// FIELD_CHARACTER
	sizeof(float),		// FIELD_TIME
	sizeof(int),		// FIELD_MODELNAME
	sizeof(int),		// FIELD_SOUNDNAME
};

CSaveRestoreBuffer::CSaveRestoreBuffer(SAVERESTOREDATA* pdata)
	: m_pdata(pdata)
{
}

int	CSaveRestoreBuffer::EntityIndex(CBaseEntity* pEntity)
{
	if (pEntity == nullptr)
		return -1;
	return EntityIndex(pEntity->pev);
}

int	CSaveRestoreBuffer::EntityIndex(entvars_t* pevLookup)
{
	if (pevLookup == nullptr)
		return -1;
	return EntityIndex(ENT(pevLookup));
}

int	CSaveRestoreBuffer::EntityIndex(edict_t* pentLookup)
{
	if (!m_pdata || pentLookup == nullptr)
		return -1;

	for (int i = 0; i < m_pdata->tableCount; i++)
	{
		ENTITYTABLE* pTable = m_pdata->pTable + i;
		if (pTable->pent == pentLookup)
			return i;
	}
	return -1;
}

edict_t* CSaveRestoreBuffer::EntityFromIndex(int entityIndex)
{
	if (!m_pdata || entityIndex < 0)
		return nullptr;

	for (int i = 0; i < m_pdata->tableCount; i++)
	{
		ENTITYTABLE* pTable = m_pdata->pTable + i;
		if (pTable->id == entityIndex)
			return pTable->pent;
	}
	return nullptr;
}

int	CSaveRestoreBuffer::EntityFlagsSet(int entityIndex, int flags)
{
	if (!m_pdata || entityIndex < 0)
		return 0;
	if (entityIndex > m_pdata->tableCount)
		return 0;

	m_pdata->pTable[entityIndex].flags |= flags;

	return m_pdata->pTable[entityIndex].flags;
}

void CSaveRestoreBuffer::BufferRewind(int size)
{
	if (!m_pdata)
		return;

	if (m_pdata->size < size)
		size = m_pdata->size;

	m_pdata->pCurrentData -= size;
	m_pdata->size -= size;
}

unsigned int CSaveRestoreBuffer::HashString(const char* pszToken)
{
	unsigned int hash = 0;

	while (*pszToken)
		hash = std::rotr(hash, 4) ^ *pszToken++;

	return hash;
}

unsigned short CSaveRestoreBuffer::TokenHash(const char* pszToken)
{
	const unsigned short hash = (unsigned short)(HashString(pszToken) % (unsigned)m_pdata->tokenCount);

#if _DEBUG
	static int tokensparsed = 0;
	tokensparsed++;
	if (!m_pdata->tokenCount || !m_pdata->pTokens)
		ALERT(at_error, "No token table array in TokenHash()!");
#endif

	for (int i = 0; i < m_pdata->tokenCount; i++)
	{
#if _DEBUG
		static bool beentheredonethat = false;
		if (i > 50 && !beentheredonethat)
		{
			beentheredonethat = true;
			ALERT(at_error, "CSaveRestoreBuffer :: TokenHash() is getting too full!");
		}
#endif

		int	index = hash + i;
		if (index >= m_pdata->tokenCount)
			index -= m_pdata->tokenCount;

		if (!m_pdata->pTokens[index] || strcmp(pszToken, m_pdata->pTokens[index]) == 0)
		{
			m_pdata->pTokens[index] = (char*)pszToken;
			return index;
		}
	}

	// Token hash table full!!! 
	// [Consider doing overflow table(s) after the main table & limiting linear hash table search]
	ALERT(at_error, "CSaveRestoreBuffer :: TokenHash() is COMPLETELY FULL!");
	return 0;
}

void CSave::WriteData(const char* pname, int size, const char* pdata)
{
	BufferField(pname, size, pdata);
}

void CSave::WriteShort(const char* pname, const short* data, int count)
{
	BufferField(pname, sizeof(short) * count, (const char*)data);
}

void CSave::WriteBoolean(const char* pname, const bool* value, int count)
{
	BufferHeader(pname, sizeof(byte) * count);
	for (int i = 0; i < count; ++i)
	{
		byte tmp = value[0];

		BufferData((const char*)&tmp, sizeof(byte));
		++value;
	}
}

void CSave::WriteInt(const char* pname, const int* data, int count)
{
	BufferField(pname, sizeof(int) * count, (const char*)data);
}

void CSave::WriteFloat(const char* pname, const float* data, int count)
{
	BufferField(pname, sizeof(float) * count, (const char*)data);
}

void CSave::WriteTime(const char* pname, const float* data, int count)
{
	BufferHeader(pname, sizeof(float) * count);
	for (int i = 0; i < count; i++)
	{
		float tmp = data[0];

		// Always encode time as a delta from the current time so it can be re-based if loaded in a new level
		// Times of 0 are never written to the file, so they will be restored as 0, not a relative time
		if (m_pdata)
			tmp -= m_pdata->time;

		BufferData((const char*)&tmp, sizeof(float));
		data++;
	}
}

void CSave::WriteString(const char* pname, const char* pdata)
{
#ifdef TOKENIZE
	const short token = (short)TokenHash(pdata);
	WriteShort(pname, &token, 1);
#else
	BufferField(pname, strlen(pdata) + 1, pdata);
#endif
}

void CSave::WriteString(const char* pname, const string_t* stringId, int count)
{
#ifdef TOKENIZE
	const short token = (short)TokenHash(STRING(*stringId));
	WriteShort(pname, &token, 1);
#else
#if 0
	if (count != 1)
		ALERT(at_error, "No string arrays!\n");
	WriteString(pname, STRING(*stringId));
#endif

	int size = 0;
	for (int i = 0; i < count; i++)
		size += strlen(STRING(stringId[i])) + 1;

	BufferHeader(pname, size);
	for (int i = 0; i < count; i++)
	{
		const char* pString = STRING(stringId[i]);
		BufferData(pString, strlen(pString) + 1);
	}
#endif
}

void CSave::WriteVector(const char* pname, const Vector& value)
{
	WriteVector(pname, &value.x, 1);
}

void CSave::WriteVector(const char* pname, const float* value, int count)
{
	BufferHeader(pname, sizeof(float) * 3 * count);
	BufferData((const char*)value, sizeof(float) * 3 * count);
}

void CSave::WritePositionVector(const char* pname, const Vector& value)
{

	if (m_pdata && m_pdata->fUseLandmark)
	{
		const Vector tmp = value - m_pdata->vecLandmarkOffset;
		WriteVector(pname, tmp);
	}

	WriteVector(pname, value);
}

void CSave::WritePositionVector(const char* pname, const float* value, int count)
{
	BufferHeader(pname, sizeof(float) * 3 * count);
	for (int i = 0; i < count; i++)
	{
		Vector tmp(value[0], value[1], value[2]);

		if (m_pdata && m_pdata->fUseLandmark)
			tmp = tmp - m_pdata->vecLandmarkOffset;

		BufferData((const char*)&tmp.x, sizeof(float) * 3);
		value += 3;
	}
}

void CSave::WriteFunction(const char* pname, void** data, int count)
{
	const char* functionName = NAME_FOR_FUNCTION((uint32)*data);
	if (functionName)
		BufferField(pname, strlen(functionName) + 1, functionName);
	else
		ALERT(at_error, "Invalid function pointer in entity!");
}

void EntvarsKeyvalue(entvars_t* pev, KeyValueData* pkvd)
{
	for (int i = 0; i < ENTVARS_COUNT; i++)
	{
		TYPEDESCRIPTION* pField = &gEntvarsDescription[i];

		if (!stricmp(pField->fieldName, pkvd->szKeyName))
		{
			switch (pField->fieldType)
			{
			case FIELD_MODELNAME:
			case FIELD_SOUNDNAME:
			case FIELD_STRING:
				(*(string_t*)((char*)pev + pField->fieldOffset)) = ALLOC_STRING(pkvd->szValue);
				break;

			case FIELD_TIME:
			case FIELD_FLOAT:
				(*(float*)((char*)pev + pField->fieldOffset)) = atof(pkvd->szValue);
				break;

			case FIELD_INTEGER:
				(*(int*)((char*)pev + pField->fieldOffset)) = atoi(pkvd->szValue);
				break;

			case FIELD_POSITION_VECTOR:
			case FIELD_VECTOR:
				*(Vector*)((char*)pev + pField->fieldOffset) = UTIL_StringToVector(pkvd->szValue);
				break;

			default:
			case FIELD_EVARS:
			case FIELD_CLASSPTR:
			case FIELD_EDICT:
			case FIELD_OBSOLETE1:
			case FIELD_POINTER:
				ALERT(at_error, "Bad field in entity!!\n");
				break;
			}
			pkvd->fHandled = true;
			return;
		}
	}
}

bool CSave::WriteEntVars(const char* pname, entvars_t* pev)
{
	return WriteFields(pname, pev, gEntvarsDescription, ENTVARS_COUNT);
}

bool CSave::WriteFields(const char* pname, void* pBaseData, TYPEDESCRIPTION* pFields, int fieldCount)
{
	// Precalculate the number of empty fields
	int emptyCount = 0;
	for (int i = 0; i < fieldCount; i++)
	{
		void* pOutputData;
		pOutputData = ((char*)pBaseData + pFields[i].fieldOffset);
		if (DataEmpty((const char*)pOutputData, pFields[i].fieldSize * gSizes[pFields[i].fieldType]))
			emptyCount++;
	}

	// Empty fields will not be written, write out the actual number of fields to be written
	const int actualCount = fieldCount - emptyCount;
	WriteInt(pname, &actualCount, 1);

	int entityArray[MAX_ENTITYARRAY]{};

	for (int i = 0; i < fieldCount; i++)
	{
		TYPEDESCRIPTION* pTest = &pFields[i];
		void* pOutputData = ((char*)pBaseData + pTest->fieldOffset);

		// UNDONE: Must we do this twice?
		if (DataEmpty((const char*)pOutputData, pTest->fieldSize * gSizes[pTest->fieldType]))
			continue;

		switch (pTest->fieldType)
		{
		case FIELD_FLOAT:
			WriteFloat(pTest->fieldName, (float*)pOutputData, pTest->fieldSize);
			break;
		case FIELD_TIME:
			WriteTime(pTest->fieldName, (float*)pOutputData, pTest->fieldSize);
			break;
		case FIELD_MODELNAME:
		case FIELD_SOUNDNAME:
		case FIELD_STRING:
			WriteString(pTest->fieldName, (string_t*)pOutputData, pTest->fieldSize);
			break;
		case FIELD_CLASSPTR:
		case FIELD_EVARS:
		case FIELD_EDICT:
		case FIELD_EHANDLE:
		{
			if (pTest->fieldSize > MAX_ENTITYARRAY)
				ALERT(at_error, "Can't save more than %d entities in an array!!!\n", MAX_ENTITYARRAY);

			for (int j = 0; j < pTest->fieldSize; j++)
			{
				switch (pTest->fieldType)
				{
				case FIELD_EVARS:
					entityArray[j] = EntityIndex(((entvars_t**)pOutputData)[j]);
					break;
				case FIELD_CLASSPTR:
					entityArray[j] = EntityIndex(((CBaseEntity**)pOutputData)[j]);
					break;
				case FIELD_EDICT:
					entityArray[j] = EntityIndex(((edict_t**)pOutputData)[j]);
					break;
				case FIELD_EHANDLE:
					entityArray[j] = EntityIndex((CBaseEntity*)(((EHANDLE*)pOutputData)[j]));
					break;
				}
			}
			WriteInt(pTest->fieldName, entityArray, pTest->fieldSize);
		}
			break;
		case FIELD_POSITION_VECTOR:
			WritePositionVector(pTest->fieldName, (float*)pOutputData, pTest->fieldSize);
			break;
		case FIELD_VECTOR:
			WriteVector(pTest->fieldName, (float*)pOutputData, pTest->fieldSize);
			break;

		case FIELD_BOOLEAN:
			WriteBoolean(pTest->fieldName, (bool*)pOutputData, pTest->fieldSize);
			break;

		case FIELD_INTEGER:
			WriteInt(pTest->fieldName, (int*)pOutputData, pTest->fieldSize);
			break;

		case FIELD_SHORT:
			WriteData(pTest->fieldName, 2 * pTest->fieldSize, ((char*)pOutputData));
			break;

		case FIELD_CHARACTER:
			WriteData(pTest->fieldName, pTest->fieldSize, ((char*)pOutputData));
			break;

			// For now, just write the address out, we're not going to change memory while doing this yet!
		case FIELD_POINTER:
			WriteInt(pTest->fieldName, (int*)(char*)pOutputData, pTest->fieldSize);
			break;

		case FIELD_FUNCTION:
			WriteFunction(pTest->fieldName, (void**)pOutputData, pTest->fieldSize);
			break;
		default:
			ALERT(at_error, "Bad field type\n");
		}
	}

	return true;
}

void CSave::BufferString(char* pdata, int len)
{
	const char c = 0;

	BufferData(pdata, len);		// Write the string
	BufferData(&c, 1);			// Write a null terminator
}

bool CSave::DataEmpty(const char* pdata, int size)
{
	for (int i = 0; i < size; i++)
	{
		if (pdata[i])
			return false;
	}
	return true;
}

void CSave::BufferField(const char* pname, int size, const char* pdata)
{
	BufferHeader(pname, size);
	BufferData(pdata, size);
}

void CSave::BufferHeader(const char* pname, int size)
{
	const short hashvalue = TokenHash(pname);
	if (size > 1 << (sizeof(short) * 8))
		ALERT(at_error, "CSave :: BufferHeader() size parameter exceeds 'short'!");
	BufferData((const char*)&size, sizeof(short));
	BufferData((const char*)&hashvalue, sizeof(short));
}

void CSave::BufferData(const char* pdata, int size)
{
	if (!m_pdata)
		return;

	if (m_pdata->size + size > m_pdata->bufferSize)
	{
		ALERT(at_error, "Save/Restore overflow!");
		m_pdata->size = m_pdata->bufferSize;
		return;
	}

	memcpy(m_pdata->pCurrentData, pdata, size);
	m_pdata->pCurrentData += size;
	m_pdata->size += size;
}

int CRestore::ReadField(void* pBaseData, TYPEDESCRIPTION* pFields, int fieldCount, int startField, int size, char* pName, void* pData)
{
	float time = 0;
	Vector position = vec3_origin;

	if (m_pdata)
	{
		time = m_pdata->time;
		if (m_pdata->fUseLandmark)
			position = m_pdata->vecLandmarkOffset;
	}

	byte* nextData = reinterpret_cast<byte*>(pData);

	for (int i = 0; i < fieldCount; i++)
	{
		const int fieldNumber = (i + startField) % fieldCount;
		TYPEDESCRIPTION* pTest = &pFields[fieldNumber];
		if (!stricmp(pTest->fieldName, pName))
		{
			if (!m_global || !(pTest->flags & FTYPEDESC_GLOBAL))
			{
				for (int j = 0; j < pTest->fieldSize; j++)
				{
					void* pOutputData = ((char*)pBaseData + pTest->fieldOffset + (j * gSizes[pTest->fieldType]));
					void* pInputData = nextData;

					//Default to advancing by size of type
					std::size_t advanceInputByBytes = gSizes[pTest->fieldType];

					switch (pTest->fieldType)
					{
					case FIELD_TIME:
					{
						float timeData = *(float*)pInputData;
						// Re-base time variables
						timeData += time;
						*((float*)pOutputData) = timeData;
						break;
					}
					case FIELD_FLOAT:
						*((float*)pOutputData) = *(float*)pInputData;
						break;
					case FIELD_MODELNAME:
					case FIELD_SOUNDNAME:
					case FIELD_STRING:
					{
						const char* inputString = reinterpret_cast<const char*>(pInputData);

						const std::size_t inputLength = strlen(inputString);

						//Skip over string and null terminator
						advanceInputByBytes = inputLength + 1;

						if (inputLength == 0)
							*((string_t*)pOutputData) = iStringNull;
						else
						{
							string_t string = ALLOC_STRING(inputString);

							*((string_t*)pOutputData) = string;

							if (!IsStringNull(string) && m_precache)
							{
								if (pTest->fieldType == FIELD_MODELNAME)
									PRECACHE_MODEL(STRING(string));
								else if (pTest->fieldType == FIELD_SOUNDNAME)
									PRECACHE_SOUND(STRING(string));
							}
						}
						break;
					}
					case FIELD_EVARS:
					{
						const int entityIndex = *(int*)pInputData;

						if (edict_t* pent = EntityFromIndex(entityIndex); pent)
							*((entvars_t**)pOutputData) = VARS(pent);
						else
							*((entvars_t**)pOutputData) = nullptr;
						break;
					}
					case FIELD_CLASSPTR:
					{
						const int entityIndex = *(int*)pInputData;

						if (edict_t* pent = EntityFromIndex(entityIndex); pent)
							*((CBaseEntity**)pOutputData) = CBaseEntity::Instance(pent);
						else
							*((CBaseEntity**)pOutputData) = nullptr;
						break;
					}
					case FIELD_EDICT:
					{
						const int entityIndex = *(int*)pInputData;
						edict_t* pent = EntityFromIndex(entityIndex);
						*((edict_t**)pOutputData) = pent;
						break;
					}
					case FIELD_EHANDLE:
					{
						// Input and Output sizes are different!
						pOutputData = (char*)pOutputData + j * (sizeof(EHANDLE) - gSizes[pTest->fieldType]);
						const int entityIndex = *(int*)pInputData;
						
						if (edict_t* pent = EntityFromIndex(entityIndex); pent)
							*((EHANDLE*)pOutputData) = CBaseEntity::Instance(pent);
						else
							*((EHANDLE*)pOutputData) = nullptr;
						break;
					}
					case FIELD_VECTOR:
						((float*)pOutputData)[0] = ((float*)pInputData)[0];
						((float*)pOutputData)[1] = ((float*)pInputData)[1];
						((float*)pOutputData)[2] = ((float*)pInputData)[2];
						break;
					case FIELD_POSITION_VECTOR:
						((float*)pOutputData)[0] = ((float*)pInputData)[0] + position.x;
						((float*)pOutputData)[1] = ((float*)pInputData)[1] + position.y;
						((float*)pOutputData)[2] = ((float*)pInputData)[2] + position.z;
						break;

					case FIELD_BOOLEAN:
						*((bool*)pOutputData) = *(byte*)pInputData;
						advanceInputByBytes = sizeof(byte);
						break;

					case FIELD_INTEGER:
						*((int*)pOutputData) = *(int*)pInputData;
						break;

					case FIELD_SHORT:
						*((short*)pOutputData) = *(short*)pInputData;
						break;

					case FIELD_CHARACTER:
						*((char*)pOutputData) = *(char*)pInputData;
						break;

					case FIELD_POINTER:
						*((int*)pOutputData) = *(int*)pInputData;
						break;
					case FIELD_FUNCTION:
					{
						const char* inputString = reinterpret_cast<const char*>(pInputData);

						const std::size_t inputLength = strlen(inputString);

						//Skip over string and null terminator
						advanceInputByBytes = inputLength + 1;

						//TODO: assumes function pointer is size of int
						if (inputLength == 0)
							*((int*)pOutputData) = 0;
						else
							*((int*)pOutputData) = FUNCTION_FROM_NAME(inputString);
						break;
					}

					default:
						ALERT(at_error, "Bad field type\n");
					}

					nextData += advanceInputByBytes;
				}
			}
#if 0
			else
			{
				ALERT(at_console, "Skipping global field %s\n", pName);
			}
#endif
			return fieldNumber;
		}
	}

	return -1;
}

bool CRestore::ReadEntVars(const char* pname, entvars_t* pev)
{
	return ReadFields(pname, pev, gEntvarsDescription, ENTVARS_COUNT);
}

bool CRestore::ReadFields(const char* pname, void* pBaseData, TYPEDESCRIPTION* pFields, int fieldCount)
{
	const unsigned short size = ReadShort();
	ASSERT(size == sizeof(int)); // First entry should be an int

	const unsigned short token = ReadShort();

	// Check the struct name
	if (token != TokenHash(pname)) // Field Set marker
	{
		//		ALERT( at_error, "Expected %s found %s!\n", pname, BufferPointer() );
		BufferRewind(2 * sizeof(short));
		return false;
	}

	// Skip over the struct name
	const int fileCount = ReadInt(); // Read field count

	int lastField = 0; // Make searches faster, most data is read/written in the same order

	// Clear out base data
	for (int i = 0; i < fieldCount; i++)
	{
		// Don't clear global fields
		if (!m_global || !(pFields[i].flags & FTYPEDESC_GLOBAL))
			memset(((char*)pBaseData + pFields[i].fieldOffset), 0, pFields[i].fieldSize * gSizes[pFields[i].fieldType]);
	}

	HEADER header;

	for (int i = 0; i < fileCount; i++)
	{
		BufferReadHeader(&header);
		lastField = ReadField(pBaseData, pFields, fieldCount, lastField, header.size, m_pdata->pTokens[header.token], header.pData);
		lastField++;
	}

	return true;
}

void CRestore::BufferReadHeader(HEADER* pheader)
{
	ASSERT(pheader != nullptr);
	pheader->size = ReadShort();				// Read field size
	pheader->token = ReadShort();				// Read field name token
	pheader->pData = BufferPointer();			// Field Data is next
	BufferSkipBytes(pheader->size);			// Advance to next field
}

short CRestore::ReadShort()
{
	short tmp = 0;

	BufferReadBytes((char*)&tmp, sizeof(short));

	return tmp;
}

int	CRestore::ReadInt()
{
	int tmp = 0;

	BufferReadBytes((char*)&tmp, sizeof(int));

	return tmp;
}

int CRestore::ReadNamedInt(const char* pName)
{
	HEADER header;

	BufferReadHeader(&header);
	return ((int*)header.pData)[0];
}

char* CRestore::ReadNamedString(const char* pName)
{
	HEADER header;

	BufferReadHeader(&header);
#ifdef TOKENIZE
	return (char*)(m_pdata->pTokens[*(short*)header.pData]);
#else
	return (char*)header.pData;
#endif
}

char* CRestore::BufferPointer()
{
	if (!m_pdata)
		return nullptr;

	return m_pdata->pCurrentData;
}

void CRestore::BufferReadBytes(char* pOutput, int size)
{
	ASSERT(m_pdata != nullptr);

	if (!m_pdata || Empty())
		return;

	if ((m_pdata->size + size) > m_pdata->bufferSize)
	{
		ALERT(at_error, "Restore overflow!");
		m_pdata->size = m_pdata->bufferSize;
		return;
	}

	if (pOutput)
		memcpy(pOutput, m_pdata->pCurrentData, size);
	m_pdata->pCurrentData += size;
	m_pdata->size += size;
}

void CRestore::BufferSkipBytes(int bytes)
{
	BufferReadBytes(nullptr, bytes);
}

int CRestore::BufferSkipZString()
{
	if (!m_pdata)
		return 0;

	const int maxLen = m_pdata->bufferSize - m_pdata->size;

	int len = 0;
	const char* pszSearch = m_pdata->pCurrentData;
	while (*pszSearch++ && len < maxLen)
		len++;

	len++;

	BufferSkipBytes(len);

	return len;
}

bool CRestore::BufferCheckZString(const char* string)
{
	if (!m_pdata)
		return false;

	const int maxLen = m_pdata->bufferSize - m_pdata->size;
	const int len = strlen(string);
	if (len <= maxLen)
	{
		if (!strncmp(string, m_pdata->pCurrentData, len))
			return true;
	}
	return false;
}
