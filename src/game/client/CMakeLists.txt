include(../../../cmake/EHLCodeGenerator.cmake)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set up vgui library
add_library(vgui SHARED IMPORTED)

if (WIN32)
	set_target_properties(vgui PROPERTIES IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/../../utils/vgui/lib/win32_vc6/vgui.lib")
else()
	set_target_properties(vgui PROPERTIES IMPORTED_LOCATION "${GAME_DIRECTORY}/vgui.so")
endif()

target_include_directories(vgui INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../../utils/vgui/include)

# Set up SDL2 library
add_library(SDL2 SHARED IMPORTED)

if (WIN32)
	set_target_properties(SDL2 PROPERTIES IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/../../../external/SDL2/lib/SDL2.lib")
else()
	set_target_properties(SDL2 PROPERTIES IMPORTED_LOCATION "${GAME_DIRECTORY}/libSDL2.so")
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
	../server/entities/items/weapons
	../server/entities/point
	../server/gamerules
	../server/persistence
	../server/server_interface
	${INCLUDE_DIRECTORY}/GoldSrc
	${INCLUDE_DIRECTORY}/GoldSrc/engine
	${INCLUDE_DIRECTORY}/GoldSrc/engine/client
	${INCLUDE_DIRECTORY}/GoldSrc/engine/server
	${INCLUDE_DIRECTORY}/GoldSrc/game
	${INCLUDE_DIRECTORY}/GoldSrc/game/client
	${INCLUDE_DIRECTORY}/GoldSrc/game/server
	${INCLUDE_DIRECTORY}/GoldSrc/math
	../shared
	../shared/entities
	../shared/entities/items
	../shared/entities/items/weapons
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
	client_precompiled.hpp
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
	input/kbutton.h
	rendering/GameStudioModelRenderer.cpp
	rendering/GameStudioModelRenderer.h
	rendering/StudioModelRenderer.cpp
	rendering/StudioModelRenderer.h
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
	../server/entities/cbase.h
	../server/entities/CBaseAnimating.hpp
	../server/entities/CBaseDelay.hpp
	../server/entities/CBaseEntity.hpp
	../server/entities/CBaseToggle.hpp
	../server/entities/brush/CBreakable.hpp
	../server/entities/characters/CBasePlayer.hpp
	../server/entities/characters/NPCs/CBaseMonster.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/con_nprint.h
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
	${INCLUDE_DIRECTORY}/GoldSrc/parsemsg.h
	${INCLUDE_DIRECTORY}/GoldSrc/Platform.h
	${INCLUDE_DIRECTORY}/GoldSrc/PlatformHeaders.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/Sequence.h
	${INCLUDE_DIRECTORY}/GoldSrc/studio.h
	${INCLUDE_DIRECTORY}/GoldSrc/winsani_in.h
	${INCLUDE_DIRECTORY}/GoldSrc/winsani_out.h
	${INCLUDE_DIRECTORY}/GoldSrc/codegen/codegen_api.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/engine/com_model.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/custom.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/customentity.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/edict.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/entity_state.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/event_args.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/event_flags.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/pm_defs.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/pm_info.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/pm_movevars.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/pmtrace.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/progdefs.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/progs.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/r_studioint.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/screenfade.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/shake.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/usercmd.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/weaponinfo.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/client/beamdef.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/client/cl_engine_int.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/engine/client/cl_entity.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/client/demo_api.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/client/dlight.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/client/entity_types.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/client/event_api.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/client/ivoicetweak.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/client/net_api.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/client/particledef.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/client/particleman.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/client/pman_particlemem.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/client/pman_triangleffect.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/client/ref_params.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/client/r_efx.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/client/triangleapi.h
	${INCLUDE_DIRECTORY}/GoldSrc/engine/server/sv_engine_int.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/game/voice_common.h
	${INCLUDE_DIRECTORY}/GoldSrc/game/client/cdll_int.h
	${INCLUDE_DIRECTORY}/GoldSrc/game/client/client_int.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/game/client/IGameClientExports.h
	${INCLUDE_DIRECTORY}/GoldSrc/game/server/server_int.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/math/mathlib.cpp
	${INCLUDE_DIRECTORY}/GoldSrc/math/mathlib.h
	${INCLUDE_DIRECTORY}/GoldSrc/math/vector.h)

add_subdirectory(../shared shared)

add_shared_sources(client)

if (MSVC)
	target_sources(client PRIVATE ../cpp.hint)
endif()

target_precompile_headers(client
	PRIVATE
		client_precompiled.hpp)

# Create filters
get_target_property(CLIENT_SOURCES client SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/../../.. FILES ${CLIENT_SOURCES})

ehl_codegen_enable(client)

if (MSVC)
	set(HL_EXECUTABLE "hl.exe")

	set_target_properties(client PROPERTIES
		VS_DEBUGGER_COMMAND "${GAME_DIRECTORY}/${HL_EXECUTABLE}"
		VS_DEBUGGER_COMMAND_ARGUMENTS ${COMMAND_ARGUMENTS}
		VS_DEBUGGER_WORKING_DIRECTORY "${GAME_DIRECTORY}")
endif()
	
# Set install paths to the user-defined mod directory
install(TARGETS client
	RUNTIME DESTINATION cl_dlls
	LIBRARY DESTINATION cl_dlls
)

if (WIN32)
	install(FILES $<TARGET_PDB_FILE:client> DESTINATION cl_dlls OPTIONAL)
endif()
