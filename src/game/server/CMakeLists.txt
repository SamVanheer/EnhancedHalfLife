include(../../../cmake/EHLCodeGenerator.cmake)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Add server library
# Add the def file as well
# This file is necessary because GiveFnptrsToDll uses the __stdcall calling convention on Windows,
# and the def file exports the function with no decoration so that a GetProcAddress lookup will find it
add_library(hl SHARED hl.def)

set_common_properties(hl)

target_include_directories(hl PRIVATE
	.
	entities
	entities/brush
	entities/characters
	entities/characters/NPCs
	entities/items
	entities/items/weapons
	entities/point
	gamerules
	persistence
	server_interface
	${INCLUDE_DIRECTORY}/GoldSrc
	${INCLUDE_DIRECTORY}/GoldSrc/engine
	${INCLUDE_DIRECTORY}/GoldSrc/engine/server
	${INCLUDE_DIRECTORY}/GoldSrc/game
	${INCLUDE_DIRECTORY}/GoldSrc/game/server
	${INCLUDE_DIRECTORY}/GoldSrc/math
	../shared
	../shared/entities
	../shared/entities/items
	../shared/entities/items/weapons
	../shared/player_movement
	../shared/utils)
	
target_compile_definitions(hl PRIVATE
	VALVE_DLL)

target_sources(hl PRIVATE
	activity.hpp
	decals.cpp
	decals.hpp
	entity_iteration.cpp
	entity_iteration.hpp
	extdll.hpp
	globals.cpp
	plane.cpp
	plane.hpp
	server_precompiled.hpp
	skill.cpp
	skill.hpp
	UserMessages.cpp
	UserMessages.hpp
	util.cpp
	util.hpp
	voice_gamemgr.cpp
	voice_gamemgr.hpp
	gamerules/gamerules.cpp
	gamerules/gamerules.hpp
	gamerules/mapcycle.cpp
	gamerules/mapcycle.hpp
	gamerules/multiplay_gamerules.cpp
	gamerules/singleplay_gamerules.cpp
	gamerules/teamplay_gamerules.cpp
	gamerules/teamplay_gamerules.hpp
	navigation/CAirNodeEnt.cpp
	navigation/CAirNodeEnt.hpp
	navigation/CNodeEnt.cpp
	navigation/CNodeEnt.hpp
	navigation/CNodeViewer.cpp
	navigation/CNodeViewer.hpp
	navigation/CTestHull.cpp
	navigation/CTestHull.hpp
	navigation/nodes.cpp
	navigation/nodes.hpp
	navigation/nodes_io.cpp
	navigation/nodes_io.hpp
	persistence/globalstate.cpp
	persistence/globalstate.hpp
	persistence/saverestore.cpp
	persistence/saverestore.hpp
	server_interface/client.cpp
	server_interface/client.hpp
	server_interface/CServerLibrary.cpp
	server_interface/CServerLibrary.hpp
	server_interface/dll_functions.cpp
	server_interface/dll_functions.hpp
	server_interface/enginecallback.hpp
	server_interface/game.cpp
	server_interface/game.hpp
	server_interface/server_interface.cpp
	server_interface/server_interface.hpp
	sound/sound_playback.cpp
	sound/sound_playback.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/const.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/crc.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/cvardef.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/director_cmds.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/enums.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/FileSystem.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/hltv.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/in_buttons.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/interface.cpp
	${INCLUDE_DIRECTORY}/GoldSrc/interface.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/keydefs.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/netadr.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/Platform.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/PlatformHeaders.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/Sequence.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/studio.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/winsani_in.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/winsani_out.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/codegen/codegen_api.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/engine/com_model.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/engine/custom.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/engine/customentity.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/engine/edict.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/engine/entity_state.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/engine/event_args.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/engine/event_flags.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/engine/pm_defs.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/engine/pm_info.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/engine/pm_movevars.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/engine/pmtrace.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/engine/progdefs.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/engine/progs.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/engine/r_studioint.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/engine/screenfade.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/engine/shake.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/engine/usercmd.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/engine/weaponinfo.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/engine/server/sv_engine_int.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/game/voice_common.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/game/server/server_int.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/math/mathlib.cpp
	${INCLUDE_DIRECTORY}/GoldSrc/math/mathlib.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/math/vector.hpp)

add_subdirectory(entities)

add_subdirectory(../shared shared)

add_shared_sources(hl)

target_precompile_headers(hl
	PRIVATE
		server_precompiled.hpp)

# Create filters
get_target_property(SERVER_SOURCES hl SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/../../.. FILES ${SERVER_SOURCES})

ehl_codegen_enable(hl)

if (MSVC)
	set(HLDS_EXECUTABLE "hlds.exe")

	# If HalfLife_HLDS_DIRECTORY is not specified it will leave a non-functional "default" setting
	set_target_properties(hl PROPERTIES
		VS_DEBUGGER_COMMAND "${HalfLife_HLDS_DIRECTORY}/${HLDS_EXECUTABLE}"
		VS_DEBUGGER_COMMAND_ARGUMENTS ${COMMAND_ARGUMENTS}
		VS_DEBUGGER_WORKING_DIRECTORY "${HalfLife_HLDS_DIRECTORY}")
endif()

# Set install paths to the user-defined mod directory
install(TARGETS hl
	RUNTIME DESTINATION dlls
	LIBRARY DESTINATION dlls
)

if (WIN32)
	install(FILES $<TARGET_PDB_FILE:hl> DESTINATION dlls OPTIONAL)
endif()
