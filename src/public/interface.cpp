#include <cstdlib>
#include <cstdio>
#include <cstring>

#include "interface.h"

#include "PlatformHeaders.hpp"

#ifdef _WIN32
constexpr std::string_view LibraryExtension{"dll"};
#elif defined(OSX)
constexpr std::string_view LibraryExtension{"dylib"};
#else
constexpr std::string_view LibraryExtension{"so"};
#endif

#ifndef _WIN32
#include <dlfcn.h> // dlopen,dlclose, et al
#include <unistd.h>
#endif

InterfaceReg::InterfaceReg(InstantiateInterfaceFn fn, const char* pName)
	: m_CreateFn(fn)
	, m_pName(pName)
	, m_pNext(s_pInterfaceRegs)
{
	s_pInterfaceRegs = this;
}

static IBaseInterface* CreateInterfaceLocal(const char* pName, InterfaceResult* pReturnCode)
{
	for (InterfaceReg* pCur = InterfaceReg::s_pInterfaceRegs; pCur; pCur = pCur->m_pNext)
	{
		if (strcmp(pCur->m_pName, pName) == 0)
		{
			if (pReturnCode)
			{
				*pReturnCode = InterfaceResult::Ok;
			}
			return pCur->m_CreateFn();
		}
	}

	if (pReturnCode)
	{
		*pReturnCode = InterfaceResult::Failed;
	}
	return nullptr;
}

EXPORT_FUNCTION IBaseInterface* CreateInterface(const char* pName, InterfaceResult* pReturnCode)
{
	return CreateInterfaceLocal(pName, pReturnCode);
}

CreateInterfaceFn Sys_GetFactoryThis()
{
	return &CreateInterfaceLocal;
}

CSysModule* Sys_LoadModule(const char* pModuleName)
{
	CSysModule* module = nullptr;

#ifdef _WIN32
	module = reinterpret_cast<CSysModule*>(LoadLibrary(pModuleName));
#else
	char szAbsoluteModuleName[1024];
	szAbsoluteModuleName[0] = 0;

	//If it's not an absolute path, convert it using the current working directory
	if (pModuleName[0] != '/')
	{
		char szCwd[1024];

		//Prevent loading from garbage paths if the path is too large for the buffer
		if (!getcwd(szCwd, sizeof(szCwd)))
		{
			exit(-1);
		}

		if (szCwd[strlen(szCwd) - 1] == '/')
			szCwd[strlen(szCwd) - 1] = 0;

		snprintf(szAbsoluteModuleName, sizeof(szAbsoluteModuleName), "%s/%s", szCwd, pModuleName);

		pModuleName = szAbsoluteModuleName;
	}

	module = dlopen(pModuleName, RTLD_NOW);
#endif

	if (!module)
	{
		char str[512];

		snprintf(str, sizeof(str), "%s.%s", pModuleName, LibraryExtension.data());

#if defined(_WIN32)
		module = reinterpret_cast<CSysModule*>(LoadLibrary(str));
#else
		printf("Error:%s\n", dlerror());
		module = reinterpret_cast<CSysModule*>(dlopen(str, RTLD_NOW));
#endif
	}

	return module;
}

void Sys_UnloadModule(CSysModule* module)
{
	if (!module)
	{
		return;
	}

#ifdef _WIN32
	FreeLibrary(reinterpret_cast<HMODULE>(module));
#else
	dlclose(reinterpret_cast<void*>(module));
#endif
}

void* Sys_GetProcAddress(CSysModule* module, const char* pName)
{
	if (!module)
	{
		return nullptr;
	}

#ifdef _WIN32
	return GetProcAddress(reinterpret_cast<HINSTANCE>(module), pName);
#else
	return dlsym(module, pName);
#endif
}

CreateInterfaceFn Sys_GetFactory(CSysModule* module)
{
	return reinterpret_cast<CreateInterfaceFn>(Sys_GetProcAddress(module, CREATEINTERFACE_PROCNAME.data()));
}
