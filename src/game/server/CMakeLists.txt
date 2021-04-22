# Add server library
# Add the def file as well
add_library(hl SHARED hl.def)

set_common_properties(hl)

target_include_directories(hl PRIVATE
	.
	entities
	entities/brush
	entities/characters
	entities/characters/NPCs
	entities/items
	entities/point
	gamerules
	persistence
	server_interface
	../engine
	../common
	../pm_shared
	${INCLUDE_DIRECTORY}/GoldSrc
	${INCLUDE_DIRECTORY}/GoldSrc/engine
	${INCLUDE_DIRECTORY}/GoldSrc/engine/server
	${INCLUDE_DIRECTORY}/GoldSrc/game
	${INCLUDE_DIRECTORY}/GoldSrc/game/server
	${INCLUDE_DIRECTORY}/GoldSrc/math
	../shared
	../shared/entities/weapons
	../shared/player_movement
	../shared/utils)
	
target_compile_definitions(hl PRIVATE
	VALVE_DLL)

target_sources(hl PRIVATE
	activity.h
	decals.cpp
	decals.hpp
	extdll.h
	globals.cpp
	nodes.cpp
	nodes.h
	plane.cpp
	plane.h
	skill.cpp
	skill.h
	UserMessages.cpp
	UserMessages.h
	util.cpp
	util.h
	voice_gamemgr.cpp
	voice_gamemgr.h
	entities/animating.cpp
	entities/animation.cpp
	entities/animation.h
	entities/animationevent.hpp
	entities/cbase.cpp
	entities/cbase.h
	entities/changelevel.cpp
	entities/changelevel.hpp
	entities/ehandle.cpp
	entities/ehandle.hpp
	entities/maprules.cpp
	entities/maprules.h
	entities/subs.cpp
	entities/triggers.cpp
	entities/triggers.hpp
	entities/world.cpp
	entities/brush/bmodels.cpp
	entities/brush/buttons.cpp
	entities/brush/doors.cpp
	entities/brush/doors.h
	entities/brush/func_break.cpp
	entities/brush/func_break.h
	entities/brush/func_tank.cpp
	entities/brush/h_battery.cpp
	entities/brush/mortar.cpp
	entities/brush/pathcorner.cpp
	entities/brush/plats.cpp
	entities/brush/trains.h
	entities/characters/corpse.cpp
	entities/characters/corpse.hpp
	entities/characters/observer.cpp
	entities/characters/player.cpp
	entities/characters/player.h
	entities/characters/NPCs/aflock.cpp
	entities/characters/NPCs/agrunt.cpp
	entities/characters/NPCs/apache.cpp
	entities/characters/NPCs/barnacle.cpp
	entities/characters/NPCs/barney.cpp
	entities/characters/NPCs/basemonster.h
	entities/characters/NPCs/bigmomma.cpp
	entities/characters/NPCs/bloater.cpp
	entities/characters/NPCs/bullsquid.cpp
	entities/characters/NPCs/combat.cpp
	entities/characters/NPCs/controller.cpp
	entities/characters/NPCs/defaultai.cpp
	entities/characters/NPCs/defaultai.h
	entities/characters/NPCs/flyingmonster.cpp
	entities/characters/NPCs/flyingmonster.h
	entities/characters/NPCs/gargantua.cpp
	entities/characters/NPCs/genericmonster.cpp
	entities/characters/NPCs/gman.cpp
	entities/characters/NPCs/h_ai.cpp
	entities/characters/NPCs/hassassin.cpp
	entities/characters/NPCs/headcrab.cpp
	entities/characters/NPCs/hgrunt.cpp
	entities/characters/NPCs/hornet.cpp
	entities/characters/NPCs/hornet.h
	entities/characters/NPCs/houndeye.cpp
	entities/characters/NPCs/ichthyosaur.cpp
	entities/characters/NPCs/islave.cpp
	entities/characters/NPCs/leech.cpp
	entities/characters/NPCs/monsters.cpp
	entities/characters/NPCs/monsters.h
	entities/characters/NPCs/monsterstate.cpp
	entities/characters/NPCs/nihilanth.cpp
	entities/characters/NPCs/osprey.cpp
	entities/characters/NPCs/rat.cpp
	entities/characters/NPCs/roach.cpp
	entities/characters/NPCs/schedule.cpp
	entities/characters/NPCs/schedule.h
	entities/characters/NPCs/scientist.cpp
	entities/characters/NPCs/scripted.cpp
	entities/characters/NPCs/scripted.h
	entities/characters/NPCs/scriptevent.h
	entities/characters/NPCs/squad.h
	entities/characters/NPCs/squadmonster.cpp
	entities/characters/NPCs/squadmonster.h
	entities/characters/NPCs/talkmonster.cpp
	entities/characters/NPCs/talkmonster.h
	entities/characters/NPCs/tentacle.cpp
	entities/characters/NPCs/turret.cpp
	entities/characters/NPCs/zombie.cpp
	entities/items/healthkit.cpp
	entities/items/items.cpp
	entities/items/items.h
	entities/point/airtank.cpp
	entities/point/effects.cpp
	entities/point/effects.h
	entities/point/explode.cpp
	entities/point/explode.h
	entities/point/ggrenade.cpp
	entities/point/h_cycler.cpp
	entities/point/lights.cpp
	entities/point/monstermaker.cpp
	entities/point/sound.cpp
	entities/point/soundent.cpp
	entities/point/soundent.h
	entities/point/spawnpoints.cpp
	entities/point/spawnpoints.hpp
	entities/point/xen.cpp
	entities/weapons/weapons.cpp
	gamerules/gamerules.cpp
	gamerules/gamerules.h
	gamerules/mapcycle.cpp
	gamerules/mapcycle.hpp
	gamerules/multiplay_gamerules.cpp
	gamerules/singleplay_gamerules.cpp
	gamerules/teamplay_gamerules.cpp
	gamerules/teamplay_gamerules.h
	persistence/globalstate.cpp
	persistence/globalstate.hpp
	persistence/saverestore.cpp
	persistence/saverestore.hpp
	server_interface/client.cpp
	server_interface/client.h
	server_interface/dll_functions.cpp
	server_interface/dll_functions.hpp
	server_interface/enginecallback.h
	server_interface/game.cpp
	server_interface/game.h
	server_interface/server_interface.cpp
	server_interface/server_interface.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/const.h
	${INCLUDE_DIRECTORY}/GoldSrc/crc.h
	${INCLUDE_DIRECTORY}/GoldSrc/cvardef.h
	${INCLUDE_DIRECTORY}/GoldSrc/director_cmds.h
	${INCLUDE_DIRECTORY}/GoldSrc/enums.h
	${INCLUDE_DIRECTORY}/GoldSrc/FileSystem.h
	${INCLUDE_DIRECTORY}/GoldSrc/hltv.h
	${INCLUDE_DIRECTORY}/GoldSrc/in_buttons.h
	${INCLUDE_DIRECTORY}/GoldSrc/interface.cpp
	${INCLUDE_DIRECTORY}/GoldSrc/interface.h
	${INCLUDE_DIRECTORY}/GoldSrc/keydefs.h
	${INCLUDE_DIRECTORY}/GoldSrc/netadr.h
	${INCLUDE_DIRECTORY}/GoldSrc/Platform.h
	${INCLUDE_DIRECTORY}/GoldSrc/PlatformHeaders.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/Sequence.h
	${INCLUDE_DIRECTORY}/GoldSrc/studio.h
	${INCLUDE_DIRECTORY}/GoldSrc/winsani_in.h
	${INCLUDE_DIRECTORY}/GoldSrc/winsani_out.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/com_model.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/custom.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/customentity.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/edict.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/entity_state.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/event_args.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/event_flags.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/pmtrace.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/progdefs.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/progs.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/r_studioint.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/screenfade.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/shake.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/usercmd.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/weaponinfo.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/server/sv_engine_int.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/game/voice_common.h
	${INCLUDE_DIRECTORY}/GoldSrc/game/server/server_int.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/math/mathlib.cpp
	${INCLUDE_DIRECTORY}/GoldSrc/math/mathlib.h
	${INCLUDE_DIRECTORY}/GoldSrc/math/vector.h)

add_subdirectory(../shared shared)

add_shared_sources(hl)

# Create filters
get_target_property(SERVER_SOURCES hl SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/../../.. FILES ${SERVER_SOURCES})

if (MSVC)
	set(HLDS_EXECUTABLE "hlds.exe")

	# Set debugging settings to work on the mod directory
	# It is assumed that the install path points to the mod directory
	get_filename_component(MOD_NAME ${CMAKE_INSTALL_PREFIX} NAME_WE)

	# If HLDS_DIRECTORY is not specified it will leave a non-functional "default" setting
	set_target_properties(hl PROPERTIES
		VS_DEBUGGER_COMMAND "${HLDS_DIRECTORY}/${HLDS_EXECUTABLE}"
		VS_DEBUGGER_COMMAND_ARGUMENTS "-game ${MOD_NAME}"
		VS_DEBUGGER_WORKING_DIRECTORY "${HLDS_DIRECTORY}")
endif()

# Set install paths to the user-defined mod directory
install(TARGETS hl
	RUNTIME DESTINATION dlls
	LIBRARY DESTINATION dlls
)

if (WIN32)
	install(FILES $<TARGET_PDB_FILE:hl> DESTINATION dlls OPTIONAL)
endif()
