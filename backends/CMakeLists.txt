set(CAPS_TEMPLATE_FILE_NAME "${CMAKE_CURRENT_SOURCE_DIR}/Caps_template.cpp.inc")

set(PARSER_CAPABILITIES_NAMES "ERROR_MESSAGE;HELP;USAGE;DESCR;VALUE_NAME;VALUE_UNIT;POSITIONAL_MANDATORY;POSITIONAL_OPTIONAL;POSITIONAL_OPTIONAL_BEFORE_MANDATORY;POSITIONAL_BOOLEANS;DASHED_MANDATORY;DASHED_OPTIONAL;PATH_VALIDATION;AUTO_COMPLETE;WIDE_STRINGS;STABLE;BUG_MEMORY_SAFETY;TERMINATES_ITSELF;PRINTS_ITSELF;THREAD_UNSAFE;NON_REENTRABLE;BUG_GRAVE_OTHER;PERMISSIVE")

set(PERMISSIVE_LICENSES "Unlicense;MIT;BSD-3-Clause;BSD-2-Clause;BSL-1.0;ISC")

function(parseBackendCaps)
	cmake_parse_arguments(""  # prefix
		"${PARSER_CAPABILITIES_NAMES}"  # options
		""  # single_values
		""  # lists
		${ARGN}
	)
	foreach(varName ${PARSER_CAPABILITIES_NAMES})
		if(_${varName})
			set("_${varName}" "true" PARENT_SCOPE)
		else()
			set("_${varName}" "false" PARENT_SCOPE)
		endif()
	endforeach()
endfunction()

function(generateCapsFile name)
	parseBackendCaps(${ARGN})
	set(CAPS_FILE "${CMAKE_CURRENT_BINARY_DIR}/${name}_Caps.cpp")
	configure_file("${CAPS_TEMPLATE_FILE_NAME}" ${CAPS_FILE} @ONLY)
	set(CAPS_FILE "${CAPS_FILE}" PARENT_SCOPE)
endfunction()

function(add_backend name lib2link)
	set(COMPONENT_NAME "backend_${name}")
	set(BACKEND_LIB_NAME "${PROJECT_NAME}_${COMPONENT_NAME}")

	generateCapsFile("${name}" ${ARGN})

	buildAndPackageLib("${BACKEND_LIB_NAME}"
		COMPONENT "${COMPONENT_NAME}"
		DESCRIPTION "A ${PROJECT_NAME} backend using ${name} CLI parsing library"
		PRIVATE_INCLUDES ${Include_dir} ${Private_include_dir}
		PRIVATE_LIBS ${lib2link}
		PUBLIC_LIBS "lib${PROJECT_NAME}"
		SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${name}.cpp" "${CAPS_FILE}"
	)

	target_compile_definitions(${BACKEND_LIB_NAME} PRIVATE "-DHYDRARGS_BACKEND_EXPORTS")
endfunction()

function(add_backend_pkgconfig name package2import)
	add_backend("${name}" "${package2import}" ${ARGN})
	pass_through_cpack_vars()
endfunction()

function(checkBackendDeps missingDepsVar package2import)
	set(missingDeps "")
	foreach(packageSpec ${package2import})
		string(REPLACE "@" ";" "packageSpec" "${packageSpec}")
		list(GET packageSpec 0 packageName)
		list(LENGTH packageSpec packageSpecLen)
		if(packageSpecLen GREATER 1)
			list(SUBLIST packageSpec 1 "${packageSpecLen}" packageComponents)
		else()
			set(packageComponents "")
		endif()
		if(packageComponents)
			find_package("${packageName}"
				COMPONENTS ${packageComponents}
			)
		else()
			find_package("${packageName}")
		endif()

		if(${packageName}_FOUND)
		else()
			list(APPEND missingDeps "${packageSpec}")
		endif()
	endforeach()
	set(${missingDepsVar} "${missingDeps}" PARENT_SCOPE)
endfunction()

function(add_backend_cmake_import name package2import lib2link disableReason)
	if(disableReason STREQUAL "")
		set(consideredReady ON)
	else()
		set(consideredReady OFF)
	endif()

	if(consideredReady OR BACKEND_${name})
		message(DEBUG "Trying to enable backend ${name}")
		checkBackendDeps(missingDeps "${package2import}")  # has to be done on every cmake run
		if(missingDeps STREQUAL "")
			set(allDepsPresent ON)
		else()
			set(allDepsPresent OFF)
			set(disableReason "Deps missing: ${missingDeps}")
		endif()
	endif()
	option(BACKEND_${name} "Enable backend ${name}" ${consideredReady})

	if(BACKEND_${name} AND allDepsPresent)
		if(consideredReady)
			message(STATUS "Enabling backend ${name}")
		else()
			message(NOTICE "Enabling backend ${name} considered broken: ${disableReason}")
		endif()
		add_backend("${name}" "${lib2link}" ${ARGN})
		pass_through_cpack_vars()
	else()
		message(VERBOSE "NOT enabling backend ${name} because of: ${disableReason}")
	endif()
endfunction()

# ToDo: POSITIONAL_BOOLEANS capability

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/backends.json" backendsJSONTable)
string(JSON backendsJSONTableLength LENGTH "${backendsJSONTable}")

function(jsonListToCmakeList res_var lst)
	string(JSON l LENGTH "${lst}")
	set(res "")
	if(l GREATER "0")
		math(EXPR lm1 "${l} - 1") # the loop is inclusive
		foreach(i RANGE "${lm1}")
			string(JSON el GET "${lst}" "${i}")
			string(REPLACE "$\{PROJECT_NAME\}" "${PROJECT_NAME}" el "${el}")
			list(APPEND res "${el}")
		endforeach()
	endif()
	set(${res_var} "${res}" PARENT_SCOPE)
endfunction()


math(EXPR backendsJSONTableLengthMinus1 "${backendsJSONTableLength} - 1") # the loop is inclusive
foreach(i RANGE "${backendsJSONTableLengthMinus1}")
	string(JSON el GET "${backendsJSONTable}" "${i}")

	string(JSON BACKEND_NAME GET "${el}" "name")
	string(JSON BACKEND_LICENSE GET "${el}" "license")
	string(JSON lst GET "${el}" "export_name")
	jsonListToCmakeList(BACKEND_LIB_EXPORT_NAME "${lst}")
	string(JSON lst GET "${el}" "lib_name")
	jsonListToCmakeList(BACKEND_LIB_LINK_NAME "${lst}")
	string(JSON CAPS GET "${el}" "caps")
	string(JSON DISABLE_REASON ERROR_VARIABLE noDisableReason GET "${el}" "disableReason")
	if(DISABLE_REASON STREQUAL "disableReason-NOTFOUND")
		set(DISABLE_REASON "")
	endif()

	if(BACKEND_LICENSE IN_LIST PERMISSIVE_LICENSES)
		list(APPEND CAPS "PERMISSIVE")
	endif()

	message(DEBUG "BACKEND_NAME ${BACKEND_NAME} BACKEND_LIB_EXPORT_NAME ${BACKEND_LIB_EXPORT_NAME} BACKEND_LIB_LINK_NAME ${BACKEND_LIB_LINK_NAME}")
	add_backend_cmake_import("${BACKEND_NAME}" "${BACKEND_LIB_EXPORT_NAME}" "${BACKEND_LIB_LINK_NAME}" "${DISABLE_REASON}" ${CAPS})
endforeach()

set(backendsTable "${backendsTable}" PARENT_SCOPE)

pass_through_cpack_vars()
