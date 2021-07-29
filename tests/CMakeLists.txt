find_package(GTest)

set(backendsTestsTable
	"CLI11"  # ok
	"clipp"  # ok
	"Lyra"  # ok
	"Dropt"  # ok
	"QCommandLine"  # ok
	"LeanAndMean"  # ok
	"ezOptionParser"  # ok
	"cxxopts"  # ok
	"octargs"  # ok

	#"getopt"  # chaining return code is wrong; the rest of seems to be OK, but who knows, needs better testing.

	#"argtable3"  # chaining return code. almost ok, no positionals; bug in the lib itself: uses stdout and stderr directly

	#"BoostArgs"  # ok, but doesn't work: https://github.com/boostorg/program_options/issues/113
	#"Argh"  # GRAVE MEMORY SAFETY BUGS IN OUR BACKEND. should be ok, when https://github.com/adishavit/argh/issues/65 is fixed, it should be ok

	#"AnyOption"  # ok, except chaining: it eats dashed args. The lib seems to be abandoned, even PRs are not accepted. Tests pass, but the library quality is bad. It has grave bugs with memory safety and code quality. It is disrecommended to use it.

	#"TCLAP"  # COMPLETELY BROKEN USE AFTER FREE when our new approach to arg object has been introduced (argv0, argvRest and generation of a temporary vector ) has been introduced. Need to mo; also grave bug in the lib itself: global state has been introduced that interferes with certain legit patterns of arguments.
	#"ArgParse"  # ok, except double and chaining ("pattern not found"). THIS LIB IS NOT REENTRABLE, if a test with multiple checks breaks, then you should try to comment its part and check if it is a reentrability issue. lot of bugs in the lib itself: 1. calls exit and uses stdout directly 2. doesn't support positional and/or mandatory flags properly 3. doesn't auto-convert

	#"Args"  # completely broken. Since the time HydrRargs had been improved, even compilation got broken.
	#"SimpleOpt"  # unfinished
	#"GetPot"  # Broken: Nonmandatory dashed string. and nonmandatory positional. Cannot exclude a bug in the lib itself.
	#"dlib"  # THE IMPL IS NOT REENTRABLE. ok: nonmand dashed, mand pos not enough, special; not ok: mandatory dashed, nonmand pos, mand pos not enough
)

# These lists allow you to disable certain tests quickly
set(special
	help
	chaining
)
set(by_optionality
	mandatory
	optional
)
set(by_dashedness
	positional
	dashed
)
set(by_data_type
	string
	int
	bool
	double
)

foreach(backendName ${backendsTestsTable})
	set(BACKEND_TARGET_NAME "${PROJECT_NAME}_backend_${backendName}")
	if(TARGET "${BACKEND_TARGET_NAME}")
	else()
		continue()
	endif()

	set(TEST_TARGET_NAME "tests_${backendName}")
	add_executable("${TEST_TARGET_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/tests.cpp")
	target_include_directories("${TEST_TARGET_NAME}" PRIVATE "${Include_dir}" "${Private_include_dir}")
	target_link_libraries("${TEST_TARGET_NAME}" GTest::GTest GTest::Main "${BACKEND_TARGET_NAME}")

	set(TEST_FLAGS "-DBACKEND_NAME=${backendName}")
	foreach(special_test ${special})
		string(TOUPPER "${special_test}" special_test)
		list(APPEND TEST_FLAGS "-DTEST_SPECIAL_${special_test}=1")
	endforeach()
	foreach(optionality ${by_optionality})
		string(TOUPPER "${optionality}" optionality)
		list(APPEND TEST_FLAGS "-DTEST_OPTIONALITY_${optionality}=1")
	endforeach()
	foreach(dashedness ${by_dashedness})
		string(TOUPPER "${dashedness}" dashedness)
		list(APPEND TEST_FLAGS "-DTEST_DASHEDNESS_${dashedness}=1")
	endforeach()
	foreach(data_type ${by_data_type})
		string(TOUPPER "${data_type}" data_type)
		list(APPEND TEST_FLAGS "-DTEST_DATA_TYPE_${data_type}=1")
	endforeach()

	target_compile_definitions("${TEST_TARGET_NAME}" PRIVATE ${TEST_FLAGS})

	set_target_properties("${TEST_TARGET_NAME}" PROPERTIES
		CXX_STANDARD 20
	)
	harden("${TEST_TARGET_NAME}")
	add_sanitizers("${TEST_TARGET_NAME}")
	gtest_discover_tests(
		"${TEST_TARGET_NAME}"
		DISCOVERY_MODE PRE_TEST
	)
endforeach()
