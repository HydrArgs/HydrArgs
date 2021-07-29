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
	set(BACKEND_LIB_NAME "HydrArgs_${COMPONENT_NAME}")
	message(STATUS "BACKEND_LIB_NAME ${BACKEND_LIB_NAME}")

	generateCapsFile("${name}" ${ARGN})

	buildAndPackageLib("${BACKEND_LIB_NAME}"
		COMPONENT "${COMPONENT_NAME}"
		DESCRIPTION "A ${PROJECT_NAME} backend using ${name} CLI parsing library"
		PRIVATE_INCLUDES ${Include_dir}
		PRIVATE_LIBS ${lib2link}
		PUBLIC_LIBS "${PROJECT_NAME}"
		SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${name}.cpp" "${CAPS_FILE}"
	)

	target_compile_definitions(${BACKEND_LIB_NAME} PRIVATE "-DHYDRARGS_BACKEND_EXPORTS")
endfunction()

function(add_backend_pkgconfig name package2import)
	add_backend("${name}" "${package2import}" ${ARGN})
	pass_through_cpack_vars()
endfunction()

function(add_backend_cmake_import name package2import lib2link)
	foreach(packageSpec ${package2import})
		string(REPLACE "@" ";" "packageSpec" "${packageSpec}")
		list(GET packageSpec 0 packageName)
		list(LENGTH packageSpec packageSpecLen)
		if(packageSpecLen GREATER 1)
			list(SUBLIST packageSpec 1 "${packageSpecLen}" packageComponents)
			message(STATUS "packageComponents ${packageComponents}")
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
		find_package(${packageName})
	endforeach()
	add_backend("${name}" "${lib2link}" ${ARGN})
	pass_through_cpack_vars()
endfunction()

# ToDO: POSITIONAL_BOOLEANS

set(backendsTable
	"dummy---PERMISSIVE"  # for testing purposes, license is Unlicense
	"CLI11-CLI11-CLI11::CLI11-USAGE HELP DESCR POSITIONAL_MANDATORY POSITIONAL_OPTIONAL POSITIONAL_OPTIONAL_BEFORE_MANDATORY POSITIONAL_BOOLEANS DASHED_MANDATORY DASHED_OPTIONAL STABLE PERMISSIVE" # BSD-3-Clause
	"clipp-clipp-${PROJECT_NAME}_fallback_parsing-USAGE HELP DESCR VALUE_NAME POSITIONAL_MANDATORY POSITIONAL_OPTIONAL POSITIONAL_OPTIONAL_BEFORE_MANDATORY DASHED_MANDATORY DASHED_OPTIONAL PERMISSIVE"  # MIT
	"Lyra-lyra-bfg::lyra-ERROR_MESSAGE USAGE HELP DESCR VALUE_NAME POSITIONAL_MANDATORY POSITIONAL_OPTIONAL POSITIONAL_BOOLEANS DASHED_MANDATORY DASHED_OPTIONAL PERMISSIVE" # BSL-1.0.
	"Dropt-dropt droptxx-dropt::dropt dropt::droptxx ${PROJECT_NAME}_fallback_errors ${PROJECT_NAME}_fallback_parsing ${PROJECT_NAME}_fallback_positional-ERROR_MESSAGE HELP VALUE_NAME DASHED_OPTIONAL PATH_VALIDATION PERMISSIVE" # BSD-3-Clause
	"QCommandLine-Qt5Core-Qt::Core ${PROJECT_NAME}_fallback_errors ${PROJECT_NAME}_fallback_parsing ${PROJECT_NAME}_fallback_positional-ERROR_MESSAGE USAGE HELP DESCR VALUE_NAME POSITIONAL_MANDATORY POSITIONAL_OPTIONAL DASHED_MANDATORY DASHED_OPTIONAL" # LGPL-2.0
	"LeanAndMean-optionparser-mbenkmann::optionparser ${PROJECT_NAME}_fallback_errors ${PROJECT_NAME}_fallback_parsing ${PROJECT_NAME}_fallback_positional-PERMISSIVE"  # MIT
	"ezOptionParser-ezOptionParser-ezOptionParser::ezOptionParser ${PROJECT_NAME}_fallback_errors ${PROJECT_NAME}_fallback_parsing ${PROJECT_NAME}_fallback_positional-PERMISSIVE"  # MIT
	"cxxopts-cxxopts-cxxopts::cxxopts ${PROJECT_NAME}_fallback_errors-ERROR_MESSAGE USAGE HELP DESCR POSITIONAL_MANDATORY POSITIONAL_OPTIONAL POSITIONAL_OPTIONAL_BEFORE_MANDATORY POSITIONAL_BOOLEANS DASHED_MANDATORY DASHED_OPTIONAL AUTO_COMPLETE PERMISSIVE"  # MIT
	"octargs-octargs-octargs::octargs-USAGE HELP DESCR POSITIONAL_MANDATORY POSITIONAL_OPTIONAL POSITIONAL_OPTIONAL_BEFORE_MANDATORY POSITIONAL_BOOLEANS DASHED_MANDATORY DASHED_OPTIONAL PERMISSIVE" # MIT.

	#"TCLAP-TCLAP--ERROR_MESSAGE USAGE HELP DESCR VALUE_NAME VALUE_UNIT POSITIONAL_MANDATORY POSITIONAL_OPTIONAL DASHED_MANDATORY DASHED_OPTIONAL BUG_GRAVE_OTHER PERMISSIVE"  # MIT license
	# OptionalUnlabeledTracker is the global state depending on the order of calling ctors rather than on order of inserting them int the class

	#"BoostArgs-Boost@program_options-Boost::program_options ${PROJECT_NAME}_fallback_errors-ERROR_MESSAGE USAGE HELP VALUE_NAME POSITIONAL_MANDATORY POSITIONAL_OPTIONAL POSITIONAL_BOOLEANS DASHED_MANDATORY DASHED_OPTIONAL PERMISSIVE" # BSL-1.0 . Seems to be incompatible to chaining - `allow_unregistered` doesn't work.

	#"Args-args-taywee::args-ERROR_MESSAGE USAGE HELP DESCR VALUE_NAME POSITIONAL_MANDATORY POSITIONAL_OPTIONAL POSITIONAL_OPTIONAL_BEFORE_MANDATORY DASHED_MANDATORY DASHED_OPTIONAL AUTO_COMPLETE"  # MIT, even compilation got broken. This backend is unfinished.
	#"ArgParse-argparse-argparse::argparse ${PROJECT_NAME}_fallback_errors ${PROJECT_NAME}_fallback_parsing-ERROR_MESSAGE USAGE HELP DESCR POSITIONAL_MANDATORY POSITIONAL_OPTIONAL_BEFORE_MANDATORY DASHED_MANDATORY DASHED_OPTIONAL TERMINATES_ITSELF PRINTS_ITSELF NON_REENTRABLE PERMISSIVE"  # MIT , LOT OF BUGS
	#"Argh-argh-${PROJECT_NAME}_fallback_errors ${PROJECT_NAME}_fallback_parsing ${PROJECT_NAME}_fallback_positional-POSITIONAL_MANDATORY POSITIONAL_OPTIONAL POSITIONAL_OPTIONAL_BEFORE_MANDATORY POSITIONAL_BOOLEANS DASHED_MANDATORY DASHED_OPTIONAL PERMISSIVE"  # BSD-3-Clause
	#"argtable3-Argtable3-argtable3::argtable3 ${PROJECT_NAME}_fallback_parsing-ERROR_MESSAGE USAGE HELP VALUE_NAME POSITIONAL_MANDATORY POSITIONAL_OPTIONAL POSITIONAL_OPTIONAL_BEFORE_MANDATORY DASHED_MANDATORY DASHED_OPTIONAL PERMISSIVE"  # BSD-3-Clause AND BSD-2-Clause AND ISC
	#"AnyOption-anyoption-anyoption::anyoption ${PROJECT_NAME}_fallback_errors ${PROJECT_NAME}_fallback_parsing ${PROJECT_NAME}_fallback_positional-DASHED_OPTIONAL BUG_MEMORY_SAFETY PERMISSIVE"  # MIT
	#"Sweet-sweet-sweet::sweet ${PROJECT_NAME}_fallback_errors ${PROJECT_NAME}_fallback_parsing ${PROJECT_NAME}_fallback_positional-"  # LGPL3, also doesn't support even splitting dashed/positional

	#"GetPot-getpot-getpot::getpot ${PROJECT_NAME}_fallback_errors ${PROJECT_NAME}_fallback_parsing ${PROJECT_NAME}_fallback_positional-PERMISSIVE"  # MIT. Seems to be broken, I cannot find a string assigned to a nonmandatory arg using it.
	#"ketopt-klib-klib::klib-PERMISSIVE"  # MIT
	#"getopt--${PROJECT_NAME}_fallback_errors ${PROJECT_NAME}_fallback_parsing ${PROJECT_NAME}_fallback_positional-POSITIONAL_OPTIONAL DASHED_OPTIONAL THREAD_UNSAFE"  # LGPL-2.0

	#"SimpleOpt-SimpleOpt-SimpleOpt::SimpleOpt-PERMISSIVE"  # MIT - broken. I have forgotten to refactor it. Though it was not vwery good.
	#"dlib-dlib-dlib::dlib ${PROJECT_NAME}_fallback_errors ${PROJECT_NAME}_fallback_parsing ${PROJECT_NAME}_fallback_positional-NON_REENTRABLE PERMISSIVE"  # BSL-1.0 . Also not sure that we need the fallbacks. Currently unfinished and broken. The impl is not reentrable
)
find_package(optionparser REQUIRED)

set(backendsTable "${backendsTable}" PARENT_SCOPE)

foreach(el ${backendsTable})
	string(REPLACE "-" ";" "el" "${el}")
	message(STATUS "el ${el}")
	list(GET el 0 BACKEND_NAME)
	list(GET el 1 BACKEND_LIB_EXPORT_NAME)
	list(GET el 2 BACKEND_LIB_LINK_NAME)
	list(GET el 3 CAPS)

	string(REPLACE " " ";" "BACKEND_LIB_EXPORT_NAME" "${BACKEND_LIB_EXPORT_NAME}")
	string(REPLACE " " ";" "BACKEND_LIB_LINK_NAME" "${BACKEND_LIB_LINK_NAME}")
	string(REPLACE " " ";" "CAPS" "${CAPS}")

	message(STATUS "BACKEND_NAME ${BACKEND_NAME} BACKEND_LIB_EXPORT_NAME ${BACKEND_LIB_EXPORT_NAME} BACKEND_LIB_LINK_NAME ${BACKEND_LIB_LINK_NAME}")
	add_backend_cmake_import("${BACKEND_NAME}" "${BACKEND_LIB_EXPORT_NAME}" "${BACKEND_LIB_LINK_NAME}" ${CAPS})
endforeach()

pass_through_cpack_vars()
