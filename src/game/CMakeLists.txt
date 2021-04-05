cmake_minimum_required(VERSION 3.18)

project(HalfLifeMod
	VERSION 1.0.0
	DESCRIPTION "Half-Life CMake Mod Template"
	LANGUAGES CXX)

if (WIN32)
	#Try to get the directory path from registry if possible
	get_filename_component(STEAM_HALFLIFE_DIRECTORY "[HKEY_CURRENT_USER\\Software\\Valve\\Steam;ModInstallPath]" ABSOLUTE)
endif()

# Used to locate binaries so dependencies are fixed up correctly (e.g. vgui.so)
set(HALFLIFE_DIRECTORY "${STEAM_HALFLIFE_DIRECTORY}" CACHE PATH "Path to the Half-Life directory")

if (MSVC)
	set(HLDS_DIRECTORY "" CACHE PATH "Path to the Half-Life Dedicated Server directory. Must be specified to automatically set up dedicated server debugging settings")
endif()

if ("${HALFLIFE_DIRECTORY}" STREQUAL "")
	message(FATAL_ERROR "You must specify the path to the Half-Life directory")
elseif (NOT EXISTS ${HALFLIFE_DIRECTORY})
	message(FATAL_ERROR "The given Half-Life directory path ${HALFLIFE_DIRECTORY} is not valid")
endif()

# Find dependencies
find_package(Threads REQUIRED)

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
		$<$<PLATFORM_ID:Linux, Darwin>:
			POSIX
			_POSIX
			LINUX
			_LINUX
			GNUC
		>
		$<$<PLATFORM_ID:Darwin>:
			OSX
			_OSX
		>)
		
	target_link_libraries(${TARGET_NAME} PRIVATE
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
endfunction()

add_subdirectory(client)
add_subdirectory(server)

# Set Visual Studio starting project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT client)
