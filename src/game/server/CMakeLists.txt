# Add server library
# Add the def file as well
add_library(hl SHARED hl.def)

set_common_properties(hl)

target_include_directories(hl PRIVATE
	.
	entities/characters
	entities/characters/NPCs
	../engine
	../common
	../pm_shared
	../game_shared
	../../public
	../shared/entities/weapons)
	
target_compile_definitions(hl PRIVATE
	VALVE_DLL)

target_sources(hl PRIVATE
	activity.h
	activitymap.h
	airtank.cpp
	animating.cpp
	animation.cpp
	animation.h
	bmodels.cpp
	buttons.cpp
	cbase.cpp
	cbase.h
	cdll_dll.h
	client.cpp
	client.h
	decals.h
	doors.cpp
	doors.h
	effects.cpp
	effects.h
	enginecallback.h
	explode.cpp
	explode.h
	extdll.h
	func_break.cpp
	func_break.h
	func_tank.cpp
	game.cpp
	game.h
	gamerules.cpp
	gamerules.h
	ggrenade.cpp
	globals.cpp
	healthkit.cpp
	h_ai.cpp
	h_battery.cpp
	h_cine.cpp
	h_cycler.cpp
	h_export.cpp
	items.cpp
	items.h
	lights.cpp
	maprules.cpp
	maprules.h
	monsterevent.h
	monstermaker.cpp
	monsterstate.cpp
	mortar.cpp
	multiplay_gamerules.cpp
	nodes.cpp
	nodes.h
	pathcorner.cpp
	plane.cpp
	plane.h
	plats.cpp
	saverestore.h
	singleplay_gamerules.cpp
	skill.cpp
	skill.h
	sound.cpp
	soundent.cpp
	soundent.h
	subs.cpp
	teamplay_gamerules.cpp
	teamplay_gamerules.h
	trains.h
	triggers.cpp
	UserMessages.cpp
	UserMessages.h
	util.cpp
	util.h
	vector.h
	weapons.cpp
	world.cpp
	xen.cpp
	entities/characters/observer.cpp
	entities/characters/player.cpp
	entities/characters/player.h
	entities/characters/spectator.cpp
	entities/characters/spectator.h
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
	../common/mathlib.cpp
	../game_shared/voice_gamemgr.cpp
	../game_shared/voice_gamemgr.h
	../pm_shared/pm_debug.cpp
	../pm_shared/pm_debug.h
	../pm_shared/pm_defs.h
	../pm_shared/pm_info.h
	../pm_shared/pm_materials.h
	../pm_shared/pm_math.cpp
	../pm_shared/pm_movevars.h
	../pm_shared/pm_shared.cpp
	../pm_shared/pm_shared.h)

add_subdirectory(../shared shared)

add_shared_sources(hl)

# Create filters
get_target_property(SERVER_SOURCES hl SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/../.. FILES ${SERVER_SOURCES})

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
