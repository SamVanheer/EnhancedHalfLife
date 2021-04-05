# Set up vgui library
add_library(vgui SHARED IMPORTED)

if (WIN32)
	set_target_properties(vgui PROPERTIES IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/../../utils/vgui/lib/win32_vc6/vgui.lib")
else()
	set_target_properties(vgui PROPERTIES IMPORTED_LOCATION "${HALFLIFE_DIRECTORY}/vgui.so")
endif()

target_include_directories(vgui INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../../utils/vgui/include)

# Set up SDL2 library. No include path since it's used directly by the client project
add_library(SDL2 SHARED IMPORTED)

if (WIN32)
	set_target_properties(SDL2 PROPERTIES IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/../../../lib/public/SDL2.lib")
else()
	set_target_properties(SDL2 PROPERTIES IMPORTED_LOCATION "${HALFLIFE_DIRECTORY}/libSDL2.so")
endif()

# Add client library
add_library(client SHARED)

set_common_properties(client)

target_include_directories(client PRIVATE
	.
	../server
	../../public
	../common
	../pm_shared
	../engine
	../game_shared
	../../../external)
	
target_compile_definitions(client PRIVATE
	CLIENT_DLL
	HL_DLL)
	
target_link_libraries(client PRIVATE
	vgui
	$<$<PLATFORM_ID:Windows>:wsock32>
	SDL2)

target_link_options(client PRIVATE
	$<$<PLATFORM_ID:Darwin>:
		-framework Carbon>)

target_sources(client PRIVATE
	ammo.cpp
	ammo.h
	ammohistory.cpp
	ammohistory.h
	ammo_secondary.cpp
	battery.cpp
	bench.h
	camera.h
	cdll_int.cpp
	cl_dll.h
	cl_util.h
	com_weapons.cpp
	com_weapons.h
	death.cpp
	demo.cpp
	demo.h
	entity.cpp
	events.cpp
	eventscripts.h
	ev_common.cpp
	ev_hldm.cpp
	ev_hldm.h
	Exports.h
	flashlight.cpp
	GameStudioModelRenderer.cpp
	GameStudioModelRenderer.h
	geiger.cpp
	global_consts.h
	health.cpp
	health.h
	hud.cpp
	hud.h
	hud_bench.cpp
	hud_benchtrace.cpp
	hud_benchtrace.h
	hud_msg.cpp
	hud_redraw.cpp
	hud_spectator.cpp
	hud_spectator.h
	hud_update.cpp
	input.cpp
	inputw32.cpp
	interpolation.cpp
	interpolation.h
	in_camera.cpp
	in_defs.h
	kbutton.h
	menu.cpp
	message.cpp
	player_info.h
	saytext.cpp
	statusbar.cpp
	status_icons.cpp
	StudioModelRenderer.cpp
	StudioModelRenderer.h
	studio_util.cpp
	studio_util.h
	text_message.cpp
	tf_defs.h
	train.cpp
	tri.cpp
	tri.h
	util.cpp
	vgui_ClassMenu.cpp
	vgui_CustomObjects.cpp
	vgui_int.cpp
	vgui_int.h
	vgui_MOTDWindow.cpp
	vgui_SchemeManager.cpp
	vgui_SchemeManager.h
	vgui_ScorePanel.cpp
	vgui_ScorePanel.h
	vgui_SpectatorPanel.cpp
	vgui_SpectatorPanel.h
	vgui_TeamFortressViewport.cpp
	vgui_TeamFortressViewport.h
	vgui_teammenu.cpp
	view.cpp
	view.h
	voice_status.cpp
	voice_status.h
	wrect.h
	hl/hl_baseentity.cpp
	hl/hl_events.cpp
	hl/hl_objects.cpp
	hl/hl_weapons.cpp
	../common/beamdef.h
	../common/cl_entity.h
	../common/com_model.h
	../common/const.h
	../common/con_nprint.h
	../common/crc.h
	../common/cvardef.h
	../common/demo_api.h
	../common/director_cmds.h
	../common/dlight.h
	../common/dll_state.h
	../common/entity_state.h
	../common/entity_types.h
	../common/enums.h
	../common/event_api.h
	../common/event_args.h
	../common/event_flags.h
	../common/hltv.h
	# ../common/interface.cpp # Do not include, version from /public is used
	../common/interface.h
	../common/in_buttons.h
	../common/ivoicetweak.h
	../common/mathlib.cpp
	../common/mathlib.h
	../common/netadr.h
	../common/net_api.h
	../common/nowin.h
	../common/parsemsg.cpp
	../common/parsemsg.h
	../common/particledef.h
	../common/Platform.h
	../common/pmtrace.h
	../common/port.h
	../common/qfont.h
	../common/ref_params.h
	../common/r_efx.h
	../common/r_studioint.h
	../common/screenfade.h
	../common/Sequence.h
	../common/studio_event.h
	../common/triangleapi.h
	../common/usercmd.h
	../common/weaponinfo.h
	../common/winsani_in.h
	../common/winsani_out.h
	../game_shared/bitvec.h
	../game_shared/vgui_checkbutton2.cpp
	../game_shared/vgui_checkbutton2.h
	../game_shared/vgui_defaultinputsignal.h
	../game_shared/vgui_grid.cpp
	../game_shared/vgui_grid.h
	../game_shared/vgui_helpers.cpp
	../game_shared/vgui_helpers.h
	../game_shared/vgui_listbox.cpp
	../game_shared/vgui_listbox.h
	../game_shared/vgui_loadtga.cpp
	../game_shared/vgui_loadtga.h
	../game_shared/vgui_scrollbar2.cpp
	../game_shared/vgui_scrollbar2.h
	../game_shared/vgui_slider2.cpp
	../game_shared/vgui_slider2.h
	../game_shared/voice_banmgr.cpp
	../game_shared/voice_banmgr.h
	../game_shared/voice_common.h
	../pm_shared/pm_debug.cpp
	../pm_shared/pm_debug.h
	../pm_shared/pm_defs.h
	../pm_shared/pm_info.h
	../pm_shared/pm_materials.h
	../pm_shared/pm_math.cpp
	../pm_shared/pm_movevars.h
	../pm_shared/pm_shared.cpp
	../pm_shared/pm_shared.h
	../../public/archtypes.h
	../../public/FileSystem.h
	../../public/interface.cpp
	../../public/interface.h
	../../public/keydefs.h
	../../public/particleman.h
	../../public/pman_particlemem.h
	../../public/pman_triangleffect.h
	../../public/cl_dll/IGameClientExports.h
	../../public/steam/steamtypes.h
	../server/crossbow.cpp
	../server/crowbar.cpp
	../server/egon.cpp
	../server/gauss.cpp
	../server/handgrenade.cpp
	../server/hornetgun.cpp
	../server/mp5.cpp
	../server/python.cpp
	../server/rpg.cpp
	../server/satchel.cpp
	../server/shotgun.cpp
	../server/squeakgrenade.cpp
	../server/tripmine.cpp
	../server/weapons_shared.cpp
	../server/wpn_shared/hl_wpn_glock.cpp
)

# Create filters
get_target_property(CLIENT_SOURCES client SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/../.. FILES ${CLIENT_SOURCES})

if (MSVC)
	set(HL_EXECUTABLE "hl.exe")
	
	# Set debugging settings to work on the mod directory
	# It is assumed that the install path points to the mod directory
	get_filename_component(MOD_NAME ${CMAKE_INSTALL_PREFIX} NAME_WE)

	set_target_properties(client PROPERTIES
		VS_DEBUGGER_COMMAND "${HALFLIFE_DIRECTORY}/${HL_EXECUTABLE}"
		VS_DEBUGGER_COMMAND_ARGUMENTS "-game ${MOD_NAME}"
		VS_DEBUGGER_WORKING_DIRECTORY "${HALFLIFE_DIRECTORY}")
endif()
	
# Set install paths to the user-defined mod directory
install(TARGETS client
	RUNTIME DESTINATION cl_dlls
	LIBRARY DESTINATION cl_dlls
)

if (WIN32)
	install(FILES $<TARGET_PDB_FILE:client> DESTINATION cl_dlls OPTIONAL)
endif()