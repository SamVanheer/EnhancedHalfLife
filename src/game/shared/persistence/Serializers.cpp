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

#include "SerializerRegistry.hpp"
#include "Serializers.hpp"

namespace persistence
{
template <typename TArithmetic>
void AddArithmeticSerializer()
{
	SerializerRegistry::GetInstance()
		.AddArithmeticSerializer<TArithmetic>(std::make_unique<ScalarVariableSerializer<TArithmetic, ArithmeticBinarySerializer<TArithmetic>>>());
}

template <typename TClass, typename TBinarySerializer>
void AddClassSerializer(bool useForDerivedClasses = false)
{
	SerializerRegistry::GetInstance()
		.AddClassSerializer<TClass>(std::make_unique<ScalarVariableSerializer<TClass, TBinarySerializer>>(), useForDerivedClasses);
}

void RegisterSerializers()
{
	auto& registry = SerializerRegistry::GetInstance();

	AddArithmeticSerializer<bool>();

	AddArithmeticSerializer<std::uint8_t>();
	AddArithmeticSerializer<std::uint16_t>();
	AddArithmeticSerializer<std::uint32_t>();
	AddArithmeticSerializer<std::uint64_t>();

	AddArithmeticSerializer<std::int8_t>();
	AddArithmeticSerializer<std::int16_t>();
	AddArithmeticSerializer<std::int32_t>();
	AddArithmeticSerializer<std::int64_t>();

	AddArithmeticSerializer<float>();
	AddArithmeticSerializer<double>();

	AddArithmeticSerializer<char>();
	AddArithmeticSerializer<char8_t>();
	AddArithmeticSerializer<char16_t>();
	AddArithmeticSerializer<char32_t>();
	AddArithmeticSerializer<wchar_t>();

	AddClassSerializer<Vector, VectorBinarySerializer>();

	AddClassSerializer<string_t, StringOffsetBinarySerializer>();

	AddClassSerializer<edict_t*, EdictBinarySerializer>();

	AddClassSerializer<BaseHandle, EHandleBinarySerializer>(true);

	SerializerRegistry::GetInstance().AddClassSerializer<entvars_t*>(std::make_unique<EntvarsSerializer>());
}
}
