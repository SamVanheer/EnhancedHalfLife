# Add server library
# Add the def file as well
add_library(hl SHARED hl.def)

set_common_properties(hl)

target_include_directories(hl PRIVATE
	.
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
	aflock.cpp
	agrunt.cpp
	airtank.cpp
	animating.cpp
	animation.cpp
	animation.h
	apache.cpp
	barnacle.cpp
	barney.cpp
	basemonster.h
	bigmomma.cpp
	bloater.cpp
	bmodels.cpp
	bullsquid.cpp
	buttons.cpp
	cbase.cpp
	cbase.h
	cdll_dll.h
	client.cpp
	client.h
	combat.cpp
	controller.cpp
	decals.h
	defaultai.cpp
	defaultai.h
	doors.cpp
	doors.h
	effects.cpp
	effects.h
	enginecallback.h
	explode.cpp
	explode.h
	extdll.h
	flyingmonster.cpp
	flyingmonster.h
	func_break.cpp
	func_break.h
	func_tank.cpp
	game.cpp
	game.h
	gamerules.cpp
	gamerules.h
	gargantua.cpp
	genericmonster.cpp
	ggrenade.cpp
	globals.cpp
	gman.cpp
	hassassin.cpp
	headcrab.cpp
	healthkit.cpp
	hgrunt.cpp
	hornet.cpp
	hornet.h
	houndeye.cpp
	h_ai.cpp
	h_battery.cpp
	h_cine.cpp
	h_cycler.cpp
	h_export.cpp
	ichthyosaur.cpp
	islave.cpp
	items.cpp
	items.h
	leech.cpp
	lights.cpp
	maprules.cpp
	maprules.h
	monsterevent.h
	monstermaker.cpp
	monsters.cpp
	monsters.h
	monsterstate.cpp
	mortar.cpp
	multiplay_gamerules.cpp
	nihilanth.cpp
	nodes.cpp
	nodes.h
	observer.cpp
	osprey.cpp
	pathcorner.cpp
	plane.cpp
	plane.h
	plats.cpp
	player.cpp
	player.h
	rat.cpp
	roach.cpp
	saverestore.h
	schedule.cpp
	schedule.h
	scientist.cpp
	scripted.cpp
	scripted.h
	scriptevent.h
	singleplay_gamerules.cpp
	skill.cpp
	skill.h
	sound.cpp
	soundent.cpp
	soundent.h
	spectator.cpp
	spectator.h
	squad.h
	squadmonster.cpp
	squadmonster.h
	subs.cpp
	talkmonster.cpp
	talkmonster.h
	teamplay_gamerules.cpp
	teamplay_gamerules.h
	tempmonster.cpp
	tentacle.cpp
	trains.h
	triggers.cpp
	turret.cpp
	UserMessages.cpp
	UserMessages.h
	util.cpp
	util.h
	vector.h
	weapons.cpp
	world.cpp
	xen.cpp
	zombie.cpp
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
