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
	cl_dll.hpp
	cl_util.hpp
	client_precompiled.hpp
	com_weapons.cpp
	com_weapons.hpp
	demo.cpp
	demo.hpp
	entity.cpp
	events.cpp
	eventscripts.hpp
	ev_common.cpp
	ev_hldm.cpp
	ev_hldm.hpp
	global_consts.hpp
	interpolation.cpp
	interpolation.hpp
	player_info.hpp
	util.cpp
	view.cpp
	view.hpp
	wrect.hpp
	client_interface/cdll_int.cpp
	client_interface/Exports.hpp
	hl/hl_baseentity.cpp
	hl/hl_events.cpp
	hl/hl_objects.cpp
	hl/hl_weapons.cpp
	input/camera.hpp
	input/input.cpp
	input/inputw32.cpp
	input/in_camera.cpp
	input/kbutton.hpp
	rendering/GameStudioModelRenderer.cpp
	rendering/GameStudioModelRenderer.hpp
	rendering/StudioModelRenderer.cpp
	rendering/StudioModelRenderer.hpp
	rendering/tri.cpp
	rendering/tri.hpp
	ui/voice_status.cpp
	ui/voice_status.hpp
	ui/hud/ammo.cpp
	ui/hud/ammo.hpp
	ui/hud/ammohistory.cpp
	ui/hud/ammohistory.hpp
	ui/hud/battery.cpp
	ui/hud/death.cpp
	ui/hud/flashlight.cpp
	ui/hud/geiger.cpp
	ui/hud/health.cpp
	ui/hud/health.hpp
	ui/hud/hud.cpp
	ui/hud/hud.hpp
	ui/hud/hud_msg.cpp
	ui/hud/hud_redraw.cpp
	ui/hud/hud_spectator.cpp
	ui/hud/hud_spectator.hpp
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
	ui/vgui/vgui_int.hpp
	ui/vgui/vgui_MOTDWindow.cpp
	ui/vgui/vgui_SchemeManager.cpp
	ui/vgui/vgui_SchemeManager.hpp
	ui/vgui/vgui_ScorePanel.cpp
	ui/vgui/vgui_ScorePanel.hpp
	ui/vgui/vgui_SpectatorPanel.cpp
	ui/vgui/vgui_SpectatorPanel.hpp
	ui/vgui/vgui_TeamFortressViewport.cpp
	ui/vgui/vgui_TeamFortressViewport.hpp
	ui/vgui/utils/vgui_checkbutton2.cpp
	ui/vgui/utils/vgui_checkbutton2.hpp
	ui/vgui/utils/vgui_defaultinputsignal.hpp
	ui/vgui/utils/vgui_grid.cpp
	ui/vgui/utils/vgui_grid.hpp
	ui/vgui/utils/vgui_helpers.cpp
	ui/vgui/utils/vgui_helpers.hpp
	ui/vgui/utils/vgui_listbox.cpp
	ui/vgui/utils/vgui_listbox.hpp
	ui/vgui/utils/vgui_loadtga.cpp
	ui/vgui/utils/vgui_loadtga.hpp
	ui/vgui/utils/vgui_scrollbar2.cpp
	ui/vgui/utils/vgui_scrollbar2.hpp
	ui/vgui/utils/vgui_slider2.cpp
	ui/vgui/utils/vgui_slider2.hpp
	ui/vgui/utils/voice_banmgr.cpp
	ui/vgui/utils/voice_banmgr.hpp
	../server/entities/cbase.hpp
	../server/entities/CBaseAnimating.hpp
	../server/entities/CBaseDelay.hpp
	../server/entities/CBaseEntity.hpp
	../server/entities/CBaseToggle.hpp
	../server/entities/CWorld.hpp
	../server/entities/brush/CBreakable.hpp
	../server/entities/brush/buttons/CBaseButton.hpp
	../server/entities/characters/CBasePlayer.hpp
	../server/entities/characters/NPCs/CBaseMonster.hpp
	../server/entities/characters/NPCs/aliens/CHornet.hpp
	../server/entities/items/weapons/CGrenade.hpp
	../server/entities/point/CPointEntity.hpp
	../server/entities/point/CSoundEnt.hpp
	../server/entities/point/effects/CBeam.hpp
	../server/entities/point/effects/CGib.hpp
	../server/entities/point/effects/CSprite.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/con_nprint.hpp
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
	${INCLUDE_DIRECTORY}/GoldSrc/parsemsg.hpp
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
	${INCLUDE_DIRECTORY}/GoldSrc/engine/client/beamdef.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/engine/client/cl_engine_int.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/engine/client/cl_entity.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/engine/client/demo_api.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/engine/client/dlight.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/engine/client/entity_types.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/engine/client/event_api.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/engine/client/ivoicetweak.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/engine/client/net_api.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/engine/client/particledef.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/engine/client/particleman.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/engine/client/pman_particlemem.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/engine/client/pman_triangleffect.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/engine/client/ref_params.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/engine/client/r_efx.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/engine/client/triangleapi.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/engine/server/sv_engine_int.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/game/voice_common.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/game/client/cdll_int.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/game/client/client_int.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/game/client/IGameClientExports.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/game/server/server_int.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/math/mathlib.cpp
	${INCLUDE_DIRECTORY}/GoldSrc/math/mathlib.hpp
	${INCLUDE_DIRECTORY}/GoldSrc/math/vector.hpp)

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
