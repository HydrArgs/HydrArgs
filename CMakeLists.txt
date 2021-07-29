cmake_minimum_required(VERSION 3.19)

set(CMAKE_USE_RELATIVE_PATHS TRUE)
project("HydrArgs")
set("PROJECT_DESCRIPTION" "An abstraction layer around C++ CLI args parsing libraries allowing switching them easily, and even in runtime.")
set("PROJECT_HOMEPAGE_URL" "https://github.com/fileTestSuite/${PROJECT_NAME}")

set(CPACK_PACKAGE_VENDOR "KOLANICH")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.ico")
set(CPACK_THREADS 0)

set(OUR_CMAKE_MODULES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(OUR_CMAKE_3PARTY_MODULES_DIR "${OUR_CMAKE_MODULES_DIR}/thirdParty")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${OUR_CMAKE_MODULES_DIR}" "${OUR_CMAKE_MODULES_DIR}/Boilerplate" "${OUR_CMAKE_MODULES_DIR}/DoxygenUtils" "${OUR_CMAKE_3PARTY_MODULES_DIR}")


include(Boilerplate)
include(DoxygenUtils)


set(Include_dir "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(Private_include_dir "${CMAKE_CURRENT_SOURCE_DIR}/private_include")
set(Lib_dir "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set(Fallback_lib_dir "${CMAKE_CURRENT_SOURCE_DIR}/fallback")
set(Backends_dir "${CMAKE_CURRENT_SOURCE_DIR}/backends")
set(Demo_dir "${CMAKE_CURRENT_SOURCE_DIR}/demo")
set(Discoverer_dir "${CMAKE_CURRENT_SOURCE_DIR}/discoverer")
set(PackagingTemplatesDir "${CMAKE_CURRENT_SOURCE_DIR}/packaging")
set(TestsDir "${CMAKE_CURRENT_SOURCE_DIR}/tests")

file(GLOB_RECURSE HDRFILES "${Include_dir}/*.h" "${Include_dir}/*.hpp")

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory("${Lib_dir}")
add_subdirectory("${Fallback_lib_dir}")
add_subdirectory("${Backends_dir}")
add_subdirectory("${Discoverer_dir}")
add_subdirectory("${Demo_dir}")

option(WITH_TESTS "Build and run tests" ON)
if(WITH_TESTS)
	enable_testing()  # This command should be in the source directory root because ctest expects to find a test file in the build directory root.
	add_subdirectory("${TestsDir}")
endif()

option(WITH_DOCS "Build docs" OFF)
if(WITH_DOCS)
	find_package(Doxygen REQUIRED dot)

	load_doxyfile("${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile")
	set(DOXYGEN_PROJECT_BRIEF "${CPACK_PACKAGE_DESCRIPTION}")
	set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${CPACK_RESOURCE_FILE_README}")
	set(DOXYGEN_EXAMPLE_PATH "${Examples_Dir}")

	doxygen_document_target("docs" "lib${PROJECT_NAME}" "html;man")
endif()

#pass_through_cpack_vars()

include(CPack)
