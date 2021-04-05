# Set up vgui library
add_library(vgui SHARED IMPORTED)

if (WIN32)
	set_target_properties(vgui PROPERTIES IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/../../utils/vgui/lib/win32_vc6/vgui.lib")
else()
	set_target_properties(vgui PROPERTIES IMPORTED_LOCATION "${HALFLIFE_DIRECTORY}/vgui.so")
endif()

target_include_directories(vgui INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../../utils/vgui/include)

# Set up SDL2 library
add_library(SDL2 SHARED IMPORTED)

if (WIN32)
	set_target_properties(SDL2 PROPERTIES IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/../../../external/SDL2/lib/SDL2.lib")
else()
	set_target_properties(SDL2 PROPERTIES IMPORTED_LOCATION "${HALFLIFE_DIRECTORY}/libSDL2.so")
endif()

target_include_directories(vgui INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../../../external/SDL2/include)

# Add client library
add_library(client SHARED)

set_common_properties(client)

target_include_directories(client PRIVATE
	.
	client_interface
	input
	rendering
	ui
	ui/hud
	ui/vgui
	ui/vgui/utils
	../server
	../server/entities
	../server/entities/characters
	../server/entities/characters/NPCs
	../server/entities/point
	../server/gamerules
	../server/server_interface
	../../public
	../../public/engine
	../../public/engine/client
	../../public/game
	../../public/game/client
	../../public/game/server
	../../public/math
	../common
	../pm_shared
	../engine
	../shared
	../shared/entities/weapons
	../shared/player_movement
	../shared/utils)
	
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
	cl_dll.h
	cl_util.h
	com_weapons.cpp
	com_weapons.h
	demo.cpp
	demo.h
	entity.cpp
	events.cpp
	eventscripts.h
	ev_common.cpp
	ev_hldm.cpp
	ev_hldm.h
	global_consts.h
	interpolation.cpp
	interpolation.h
	player_info.h
	tf_defs.h
	util.cpp
	view.cpp
	view.h
	wrect.h
	client_interface/cdll_int.cpp
	client_interface/Exports.h
	hl/hl_baseentity.cpp
	hl/hl_events.cpp
	hl/hl_objects.cpp
	hl/hl_weapons.cpp
	input/camera.h
	input/input.cpp
	input/inputw32.cpp
	input/in_camera.cpp
	input/in_defs.h
	input/kbutton.h
	rendering/GameStudioModelRenderer.cpp
	rendering/GameStudioModelRenderer.h
	rendering/StudioModelRenderer.cpp
	rendering/StudioModelRenderer.h
	rendering/studio_util.cpp
	rendering/studio_util.h
	rendering/tri.cpp
	rendering/tri.h
	ui/voice_status.cpp
	ui/voice_status.h
	ui/hud/ammo.cpp
	ui/hud/ammo.h
	ui/hud/ammohistory.cpp
	ui/hud/ammohistory.h
	ui/hud/battery.cpp
	ui/hud/death.cpp
	ui/hud/flashlight.cpp
	ui/hud/geiger.cpp
	ui/hud/health.cpp
	ui/hud/health.h
	ui/hud/hud.cpp
	ui/hud/hud.h
	ui/hud/hud_msg.cpp
	ui/hud/hud_redraw.cpp
	ui/hud/hud_spectator.cpp
	ui/hud/hud_spectator.h
	ui/hud/hud_update.cpp
	ui/hud/menu.cpp
	ui/hud/message.cpp
	ui/hud/saytext.cpp
	ui/hud/status_icons.cpp
	ui/hud/statusbar.cpp
	ui/hud/text_message.cpp
	ui/hud/train.cpp
	ui/vgui/vgui_ClassMenu.cpp
	ui/vgui/vgui_CustomObjects.cpp
	ui/vgui/vgui_int.cpp
	ui/vgui/vgui_int.h
	ui/vgui/vgui_MOTDWindow.cpp
	ui/vgui/vgui_SchemeManager.cpp
	ui/vgui/vgui_SchemeManager.h
	ui/vgui/vgui_ScorePanel.cpp
	ui/vgui/vgui_ScorePanel.h
	ui/vgui/vgui_SpectatorPanel.cpp
	ui/vgui/vgui_SpectatorPanel.h
	ui/vgui/vgui_TeamFortressViewport.cpp
	ui/vgui/vgui_TeamFortressViewport.h
	ui/vgui/vgui_teammenu.cpp
	ui/vgui/utils/vgui_checkbutton2.cpp
	ui/vgui/utils/vgui_checkbutton2.h
	ui/vgui/utils/vgui_defaultinputsignal.h
	ui/vgui/utils/vgui_grid.cpp
	ui/vgui/utils/vgui_grid.h
	ui/vgui/utils/vgui_helpers.cpp
	ui/vgui/utils/vgui_helpers.h
	ui/vgui/utils/vgui_listbox.cpp
	ui/vgui/utils/vgui_listbox.h
	ui/vgui/utils/vgui_loadtga.cpp
	ui/vgui/utils/vgui_loadtga.h
	ui/vgui/utils/vgui_scrollbar2.cpp
	ui/vgui/utils/vgui_scrollbar2.h
	ui/vgui/utils/vgui_slider2.cpp
	ui/vgui/utils/vgui_slider2.h
	ui/vgui/utils/voice_banmgr.cpp
	ui/vgui/utils/voice_banmgr.h
	../../public/archtypes.h
	../../public/const.h
	../../public/con_nprint.h
	../../public/crc.h
	../../public/cvardef.h
	../../public/director_cmds.h
	../../public/enums.h
	../../public/FileSystem.h
	../../public/hltv.h
	../../public/in_buttons.h
	../../public/interface.cpp
	../../public/interface.h
	../../public/keydefs.h
	../../public/netadr.h
	../../public/parsemsg.cpp
	../../public/parsemsg.h
	../../public/Platform.h
	../../public/port.h
	../../public/Sequence.h
	../../public/studio_event.h
	../../public/winsani_in.h
	../../public/winsani_out.h
	../../public/engine/com_model.h
	../../public/engine/entity_state.h
	../../public/engine/event_args.h
	../../public/engine/event_flags.h
	../../public/engine/pmtrace.h
	../../public/engine/r_studioint.h
	../../public/engine/screenfade.h
	../../public/engine/usercmd.h
	../../public/engine/weaponinfo.h
	../../public/engine/client/beamdef.h
	../../public/engine/client/cl_entity.h
	../../public/engine/client/demo_api.h
	../../public/engine/client/dlight.h
	../../public/engine/client/entity_types.h
	../../public/engine/client/event_api.h
	../../public/engine/client/ivoicetweak.h
	../../public/engine/client/net_api.h
	../../public/engine/client/particledef.h
	../../public/engine/client/particleman.h
	../../public/engine/client/pman_particlemem.h
	../../public/engine/client/pman_triangleffect.h
	../../public/engine/client/ref_params.h
	../../public/engine/client/r_efx.h
	../../public/engine/client/triangleapi.h
	../../public/game/voice_common.h
	../../public/game/client/IGameClientExports.h
	../../public/steam/steamtypes.h
	../../public/math/mathlib.cpp
	../../public/math/mathlib.h
	../../public/math/vector.h)

add_subdirectory(../shared shared)

add_shared_sources(client)

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