# Functions to help generate the codegen config file and adding the prebuild command
set(EHL_GENERATED_DIRECTORY_NAME generated)
set(EHL_GENERATED_SOURCE_NAME ehl_generated.cpp)

function(ehl_codegen_list_to_array list array)
	# Prepend some spaces to align the result nicely
	list(TRANSFORM ${list} PREPEND "    \"")
	list(TRANSFORM ${list} APPEND "\"")
	list(JOIN ${list} ",\n" ${array})
	
	set(${array} ${${array}} PARENT_SCOPE)
endfunction()

function(ehl_codegen_generate_config_file target)
	get_target_property(TARGET_DEFINITIONS ${target} COMPILE_DEFINITIONS)
	# Strip out generator expressions and empty entries
	list(TRANSFORM TARGET_DEFINITIONS GENEX_STRIP)
	list(REMOVE_ITEM TARGET_DEFINITIONS "")
	ehl_codegen_list_to_array(TARGET_DEFINITIONS EHL_DEFINITIONS)
	#message(STATUS ${EHL_DEFINITIONS})
	
	get_target_property(TARGET_INCLUDES ${target} INCLUDE_DIRECTORIES)
	ehl_codegen_list_to_array(TARGET_INCLUDES EHL_INCLUDES)
	#message(STATUS ${EHL_INCLUDES})

	get_target_property(TARGET_HEADERS ${target} SOURCES)
	list(FILTER TARGET_HEADERS INCLUDE REGEX "^.*\.h(pp)?$")
	ehl_codegen_list_to_array(TARGET_HEADERS EHL_HEADERS)
	#message(STATUS ${EHL_HEADERS})
	
	get_target_property(TARGET_PRECOMILED_HEADERS ${target} PRECOMPILE_HEADERS)
	ehl_codegen_list_to_array(TARGET_PRECOMILED_HEADERS EHL_PRECOMPILED_HEADERS)
	#message(STATUS ${EHL_PRECOMPILED_HEADERS})
	
	set(EHL_SOURCE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	set(EHL_BINARY_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

	configure_file(${CMAKE_CURRENT_FUNCTION_LIST_DIR}/EHLCodeGen.json.in ${CMAKE_CURRENT_BINARY_DIR}/EHLCodeGen.json @ONLY)
endfunction()

function(ehl_codegen_enable target)
	ehl_codegen_generate_config_file(${target})

	# TODO fix this path
	add_custom_command(
		TARGET ${target}
		PRE_BUILD
		COMMAND dotnet "${CMAKE_CURRENT_SOURCE_DIR}/../../utils/CodeGenerator/CodeGenerator/bin/Debug/net5.0/win-x64/CodeGenerator.dll" generate --config-file "${CMAKE_CURRENT_BINARY_DIR}/EHLCodeGen.json"
		BYPRODUCTS ${EHL_GENERATED_SOURCE_NAME})
		
		target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/${EHL_GENERATED_DIRECTORY_NAME})
		target_sources(${target} PRIVATE ${EHL_GENERATED_SOURCE_NAME})
endfunction()