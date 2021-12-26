cmake_minimum_required(VERSION 3.18)

project(HalfLifeMod
	VERSION 1.0.0
	DESCRIPTION "Half-Life CMake Mod Template"
	LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

get_filename_component(GAME_DIRECTORY ${CMAKE_INSTALL_PREFIX} DIRECTORY)

if (MSVC)
	# These options are prefixed with HalfLife so the CMake GUI can group them
	set(HalfLife_HLDS_DIRECTORY "" CACHE PATH "Path to the Half-Life Dedicated Server directory. Must be specified to automatically set up dedicated server debugging settings")
	
	set(HalfLife_DEVELOPER_LEVEL "1" CACHE STRING "When launching from Visual Studio, set the developer level to this")
	set_property(CACHE HalfLife_DEVELOPER_LEVEL PROPERTY STRINGS 0 1 2 3)
	
	option(HalfLife_ENABLE_CHEATS "When launching from Visual Studio, enable cheats as well" OFF)
	option(HalfLife_ENABLE_CONSOLE "When launching from Visual Studio, enable the console on startup" OFF)
	
	# Set debugging settings to work on the mod directory
	# It is assumed that the install path points to the mod directory
	get_filename_component(MOD_NAME ${CMAKE_INSTALL_PREFIX} NAME_WE)
	
	set(COMMAND_ARGUMENTS "-game ${MOD_NAME} +developer ${HalfLife_DEVELOPER_LEVEL}")
	
	if (HalfLife_ENABLE_CHEATS)
		set(COMMAND_ARGUMENTS "${COMMAND_ARGUMENTS} +sv_cheats 1")
	endif()
	
	if (HalfLife_ENABLE_CONSOLE)
		set(COMMAND_ARGUMENTS "${COMMAND_ARGUMENTS} -console")
	endif()
endif()

if ("${GAME_DIRECTORY}" STREQUAL "")
	message(FATAL_ERROR "You must specify the path to the mod directory in CMAKE_INSTALL_PREFIX")
elseif (NOT EXISTS ${GAME_DIRECTORY})
	message(FATAL_ERROR "The given Half-Life directory path ${CMAKE_INSTALL_PREFIX} is not valid")
endif()

set(INCLUDE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../include)

# Find dependencies
find_package(Threads REQUIRED)

# Set up SDL2 library
add_library(RTTR STATIC IMPORTED)

if (WIN32)
	set_target_properties(RTTR PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/../../external/RTTR/lib/rttr_core_lib_s.lib")
	set_target_properties(RTTR PROPERTIES IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/../../external/RTTR/lib/rttr_core_lib_s_d.lib")
else()
	#TODO
	#set_target_properties(RTTR PROPERTIES IMPORTED_LOCATION "${GAME_DIRECTORY}/libSDL2.so")
endif()

target_include_directories(RTTR INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../../external/RTTR/include)

# Sets properties common to all projects
function(set_common_properties TARGET_NAME)
	set_target_properties(${TARGET_NAME} PROPERTIES PREFIX "")

	# Link statically with the runtime
	set_target_properties(${TARGET_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
		
	target_compile_definitions(${TARGET_NAME} PRIVATE
		_CRT_SECURE_NO_WARNINGS
		$<$<CONFIG:DEBUG>:_DEBUG>
		CLIENT_WEAPONS
		# These are defined for OSX as well
		"$<$<PLATFORM_ID:Linux, Darwin>:POSIX _POSIX LINUX _LINUX GNUC>"
		"$<$<PLATFORM_ID:Darwin>: OSX _OSX>")
		
	target_link_libraries(${TARGET_NAME} PRIVATE
		RTTR
		Threads::Threads
		$<$<PLATFORM_ID:Linux,Darwin>:dl>)

	# TODO: review Linux compiler flags
	target_compile_options(${TARGET_NAME} PRIVATE
		# force 387 for FP math so the precision between win32 and linux and osx match
		# Note: the pentium-m arch setting is not used for AMD systems in the original makefile
		# Since the arch settings used are i686 this means including the setting ensures original settings are used,
		# but it could cause problems for AMD targets
		$<$<CXX_COMPILER_ID:Clang,AppleClang,GNU>:
			-m32 -fpermissive -fno-strict-aliasing -Wno-invalid-offsetof -gdwarf-2 -g2 -march=pentium-m -mfpmath=387>
			
		$<$<CXX_COMPILER_ID:MSVC>:
			/W3 /Zc:threadSafeInit->
			
		# These are all from the original Makefile
		# They have not been tested with CMake since there is no Mac system available to test on, so this might not work
		$<$<PLATFORM_ID:Darwin>:
			-fvisibility=hidden -Qunused-arguments -mmacosx-version-min=10.5 -fasm-blocks -arch i386 -march=prescott -momit-leaf-frame-pointer -mtune=core2>)

	target_link_options(${TARGET_NAME} PRIVATE
		$<$<CXX_COMPILER_ID:Clang,AppleClang,GNU>:
			-m32 -gdwarf-2 -g2>
		$<$<PLATFORM_ID:Linux>:
			-Wl,-Map,${TARGET_NAME}_map.txt>)
	
	if (MSVC)
		target_sources(${TARGET_NAME}
			PRIVATE
				${CMAKE_CURRENT_FUNCTION_LIST_DIR}/cpp.hint
				${CMAKE_CURRENT_FUNCTION_LIST_DIR}/EHLGame.natvis)
		
		set_property(SOURCE ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/EHLGame.natvis PROPERTY VS_SETTINGS "ExcludedFromBuild=true")
	endif()
endfunction()

add_subdirectory(client)
add_subdirectory(server)

# Set Visual Studio starting project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT client)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../../config/liblist.gam DESTINATION .)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../../config/network/delta.lst DESTINATION .)
