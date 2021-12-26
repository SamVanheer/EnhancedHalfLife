#pragma once

#include <type_traits>

#include <rttr/registration.h>

#include "persistence/PersistenceDefs.hpp"

/**
*	@file
*
*	Defines the API used for code generation
*/

#ifdef EHL_CODEGEN_PROCESSING
#define EHL_META(annotation) __attribute__((annotate(annotation)))
#else
/**
*	@brief The core macro used to add annotations to elements
*	Should never be used directly, always use one of the EHL_* macros provided below
*/
#define EHL_META(annotation)
#endif

/**
*	@brief Marks a class for processing, and adds any annotations provided as parameters
*	@details Must be added between the class/struct keyword and the class/struct name
*	Example:
*	class EHL_CLASS() CBaseEntity
*/
#define EHL_CLASS(...) EHL_META("Class=" #__VA_ARGS__)

/**
*	@brief Marks a field for processing, and adds any annotations provided as parameters
*	The enclosing class must be marked with ::EHL_CLASS for this to be considered
*	@details Must be added before the field declaration
*	Example:
*	EHL_FIELD()
*	float m_flHealth;
*/
#define EHL_FIELD(...) EHL_META("Field=" #__VA_ARGS__)

//Define an empty version of this macro now so users don't get errors on first compilation
//Generated headers will undefine and redefine this to contain the generated body of the class to which the header belongs
#define EHL_GENERATED_BODY()

/**
*	@brief Specialized for each type that needs reflection initialized
*/
template<typename T>
class TypeReflectionInitializer;
