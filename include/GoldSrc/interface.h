#pragma once

/**
*	@file
*
*	This header defines the interface convention used in the valve engine.
*	To make an interface and expose it:
*	1. Derive from IBaseInterface.
*	2. The interface must be ALL pure virtuals, and have no data members.
*	3. Define a name for it.
*	4. In its implementation file, use EXPOSE_INTERFACE or EXPOSE_SINGLE_INTERFACE.
*
*	Versioning
*	There are two versioning cases that are handled by this:
*	1. You add functions to the end of an interface, so it is binary compatible with the previous interface. In this case,
*	   you need two EXPOSE_INTERFACEs: one to expose your class as the old interface and one to expose it as the new interface.
*	2. You update an interface so it's not compatible anymore (but you still want to be able to expose the old interface
*	   for legacy code). In this case, you need to make a new version name for your new interface, and make a wrapper interface and
*	   expose it for the old interface.
*/

#include <string_view>

/**
*	@brief All interfaces derive from this.
*/
class IBaseInterface
{
public:
	virtual ~IBaseInterface() {}
};

using InstantiateInterfaceFn = IBaseInterface * (*)();

// Used internally to register classes.
class InterfaceReg
{
public:
	InterfaceReg(InstantiateInterfaceFn fn, const char* pName);

public:
	InstantiateInterfaceFn m_CreateFn;
	const char* m_pName;

	InterfaceReg* m_pNext; // For the global list.
	static inline InterfaceReg* s_pInterfaceRegs = nullptr;
};

/**
*	@brief Use this if you want to write the factory function.
*/
#define EXPOSE_INTERFACE_FN(functionName, interfaceName, versionName)			\
	static InterfaceReg __g_Create##className##_reg(functionName, versionName)

/**
*	@brief Use this to expose an interface that can have multiple instances.
*
*	e.g.:
*	EXPOSE_INTERFACE( CInterfaceImp, IInterface, "MyInterface001" )
*	This will expose a class called CInterfaceImp that implements IInterface (a pure class)
*	clients can receive a pointer to this class by calling CreateInterface( "MyInterface001" )
*
*	In practice, the shared header file defines the interface (IInterface) and version name ("MyInterface001")
*	so that each component can use these names/vtables to communicate
*
*	A single class can support multiple interfaces through multiple inheritance
*/
#define EXPOSE_INTERFACE(className, interfaceName, versionName)										\
	EXPOSE_INTERFACE_FN([] { return static_cast<interfaceName*>(new className); }, versionName )

/**
*	@brief Use this to expose a singleton interface with a global variable you've created.
*/
#define EXPOSE_SINGLE_INTERFACE_GLOBALVAR(className, interfaceName, versionName, globalVarName)													\
	static InterfaceReg __g_Create##className##interfaceName##_reg([] { return static_cast<IBaseInterface*>(&globalVarName); }, versionName)

/**
*	@brief Use this to expose a singleton interface. This creates the global variable for you automatically.
*/
#define EXPOSE_SINGLE_INTERFACE(className, interfaceName, versionName)										\
	static className __g_##className##_singleton;															\
	EXPOSE_SINGLE_INTERFACE_GLOBALVAR(className, interfaceName, versionName, __g_##className##_singleton)

#ifdef WIN32
#define EXPORT_FUNCTION __declspec(dllexport)
#else
#define EXPORT_FUNCTION __attribute__ ((visibility("default")))
#endif

// This function is automatically exported and allows you to access any interfaces exposed with the above macros.
// if pReturnCode is set, it will return one of the following values
// extend this for other error conditions/code
enum class InterfaceResult
{
	Ok = 0,
	Failed
};

constexpr std::string_view CREATEINTERFACE_PROCNAME{"CreateInterface"};

extern "C" EXPORT_FUNCTION IBaseInterface * CreateInterface(const char* pName, InterfaceResult * pReturnCode);

using CreateInterfaceFn = decltype(CreateInterface)*;

/**
*	@brief returns the instance of this module
*/
CreateInterfaceFn Sys_GetFactoryThis();

/**
*	Load & Unload should be called in exactly one place for each module.
*	The factory for that module should be passed on to dependent components for proper versioning.
*/

class CSysModule;

/**
*	@brief Loads a DLL/component from disk and returns a handle to it
*/
CSysModule* Sys_LoadModule(const char* pModuleName);

/**
*	@brief Unloads a DLL/component
*/
void Sys_UnloadModule(CSysModule* module);

/**
*	@brief returns a pointer to a function, given a module
*/
void* Sys_GetProcAddress(CSysModule* module, const char* pName);

/**
*	@brief returns a pointer to the CreateInterface function, given a module
*/
CreateInterfaceFn Sys_GetFactory(CSysModule* module);
