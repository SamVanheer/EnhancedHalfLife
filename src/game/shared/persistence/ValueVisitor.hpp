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

#include <cassert>

#include <rttr/property.h>
#include <rttr/variant.h>

namespace persistence
{
/**
*	@brief Visits a value and invokes methods for specific types of values
*	@details Uses CRTP for compile-time dispatch
*	You must make this class a friend of the derived class
*	@tparam TDerived Type of the class that derives from this class. Re-implement any visit methods you want to handle yourself
*/
template<typename TDerived>
struct ValueVisitor
{
	void Visit(rttr::variant& value)
	{
		//Only check for valid variants here
		//Everywhere else variants are retrieved from object instances, so they will be valid
		if (!value.is_valid())
		{
			return;
		}

		VisitValid(value);
	}

protected:
	void DefaultVisit(rttr::variant& value)
	{
	}

	void VisitSequentialContainer(rttr::variant& value)
	{
		GetDerived()->DefaultVisit(value);
	}

	void VisitAssociativeContainer(rttr::variant& value)
	{
		GetDerived()->DefaultVisit(value);
	}

	void VisitArithmetic(rttr::variant& value)
	{
		GetDerived()->DefaultVisit(value);
	}

	void VisitEnumeration(rttr::variant& value)
	{
		GetDerived()->DefaultVisit(value);
	}

	void VisitClass(rttr::variant& value)
	{
		GetDerived()->DefaultVisit(value);
	}

	void VisitPointer(rttr::variant& value)
	{
		GetDerived()->DefaultVisit(value);
	}

	void VisitFunctionPointer(rttr::variant& value)
	{
		GetDerived()->DefaultVisit(value);
	}

	void VisitMemberFunctionPointer(rttr::variant& value)
	{
		GetDerived()->DefaultVisit(value);
	}

	void VisitMemberObjectPointer(rttr::variant& value)
	{
		GetDerived()->DefaultVisit(value);
	}

	void VisitUnknown(rttr::variant& value)
	{
		GetDerived()->DefaultVisit(value);
	}

	TDerived* GetDerived()
	{
		return static_cast<TDerived*>(this);
	}

	void VisitValid(rttr::variant& value)
	{
		const auto type = value.get_type();

		if (value.is_sequential_container())
		{
			GetDerived()->VisitSequentialContainer(value);
		}
		else if (value.is_associative_container())
		{
			GetDerived()->VisitAssociativeContainer(value);
		}
		else if (type.is_arithmetic())
		{
			GetDerived()->VisitArithmetic(value);
		}
		else if (type.is_enumeration())
		{
			GetDerived()->VisitEnumeration(value);
		}
		else if (type.is_class())
		{
			GetDerived()->VisitClass(value);
		}
		else if (type.is_pointer())
		{
			GetDerived()->VisitPointer(value);
		}
		else if (type.is_function_pointer())
		{
			GetDerived()->VisitFunctionPointer(value);
		}
		else if (type.is_member_function_pointer())
		{
			GetDerived()->VisitMemberFunctionPointer(value);
		}
		else if (type.is_member_object_pointer())
		{
			GetDerived()->VisitMemberObjectPointer(value);
		}
		else
		{
			GetDerived()->VisitUnknown(value);
		}
	}
};
}
