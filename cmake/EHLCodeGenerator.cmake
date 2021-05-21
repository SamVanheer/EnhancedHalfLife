# Functions to help generate the codegen config file and adding the prebuild command
set(EHL_GENERATED_DIRECTORY_NAME generated)
set(EHL_GENERATED_SOURCE_NAME generated/ehl_generated.cpp)
set(EHL_CODEGEN_COMMAND dotnet "${CMAKE_CURRENT_SOURCE_DIR}/../../../external/EHLCodeGenerator/bin/CodeGenerator.dll")

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
	
	set(EHL_SOURCE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	set(EHL_BINARY_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

	configure_file(${CMAKE_CURRENT_FUNCTION_LIST_DIR}/EHLCodeGen.json.in ${CMAKE_CURRENT_BINARY_DIR}/EHLCodeGen.json @ONLY)
endfunction()

function(ehl_codegen_add_clean target)	
	# Create the target that will clean a specific target
	add_custom_target(
		CLEAN_CODEGEN_${target}
		COMMAND ${EHL_CODEGEN_COMMAND} clean --config-file "${CMAKE_CURRENT_BINARY_DIR}/EHLCodeGen.json")
	
	set_target_properties(CLEAN_CODEGEN_${target}
		PROPERTIES
			EXCLUDE_FROM_ALL TRUE
			EXCLUDE_FROM_DEFAULT_BUILD TRUE
			FOLDER "CLEAN_CODEGEN")
endfunction()

function(ehl_codegen_enable target)
	# Do this first so the include path is added to the generated file as well
	target_include_directories(${target} BEFORE PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/${EHL_GENERATED_DIRECTORY_NAME})
	
	ehl_codegen_generate_config_file(${target})

	ehl_codegen_add_clean(${target})
	
	# Process and generate code
	add_custom_command(
		TARGET ${target}
		PRE_BUILD
		COMMAND ${EHL_CODEGEN_COMMAND} generate --config-file "${CMAKE_CURRENT_BINARY_DIR}/EHLCodeGen.json"
		BYPRODUCTS ${EHL_GENERATED_SOURCE_NAME})
	
	# Update the cache file so subsequent compilations skip finished files
	add_custom_command(
		TARGET ${target}
		POST_BUILD
		COMMAND ${EHL_CODEGEN_COMMAND} update-cache-file --config-file "${CMAKE_CURRENT_BINARY_DIR}/EHLCodeGen.json")
	
	target_sources(${target} PRIVATE ${EHL_GENERATED_SOURCE_NAME})
endfunction()