# Finds either Qt5 or Qt6
# licensed under Unlicense

set(_majorVersionsToProbe "6;5")

set(args "${${CMAKE_FIND_PACKAGE_NAME}_FIND_VERSION}")

if(${CMAKE_FIND_PACKAGE_NAME}_FIND_VERSION_EXACT)
	list(APPEND args "EXACT")
endif()

if(${CMAKE_FIND_PACKAGE_NAME}_FIND_QUIETLY)
	list(APPEND args "QUIET")
endif()

#if(${CMAKE_FIND_PACKAGE_NAME}_FIND_REQUIRED)
#	list(APPEND args "REQUIRED")
#endif()

# OPTIONAL_COMPONENTS

set(reqComps "")
set(optComps "")

if(${CMAKE_FIND_PACKAGE_NAME}_FIND_COMPONENTS STREQUAL "")
else()
	foreach(comp ${${CMAKE_FIND_PACKAGE_NAME}_FIND_COMPONENTS})
		if(${CMAKE_FIND_PACKAGE_NAME}_FIND_REQUIRED_${comp})
			list(APPEND reqComps "${comp}")
		else()
			list(APPEND optComps "${comp}")
		endif()
	endforeach()
endif()

if(reqComps STREQUAL "")
else()
	list(APPEND args "COMPONENTS;${reqComps}")
endif()

if(optComps STREQUAL "")
else()
	list(APPEND args "OPTIONAL_COMPONENTS;${reqComps}")
endif()

#[[
message(STATUS "${CMAKE_FIND_PACKAGE_NAME}_FIND_REGISTRY_VIEW ${${CMAKE_FIND_PACKAGE_NAME}_FIND_REGISTRY_VIEW}")
if(${CMAKE_FIND_PACKAGE_NAME}_FIND_REGISTRY_VIEW STREQUAL "")
else()
	list(APPEND args "REGISTRY_VIEW;${${CMAKE_FIND_PACKAGE_NAME}_FIND_REGISTRY_VIEW}")
endif()
#]]

message(STATUS "args ${args}")

set(_versionsWithNamesList "")
foreach(v ${_majorVersionsToProbe})
	list(APPEND _versionsWithNamesList "Qt${v}")
endforeach()

#message(STATUS "_versionsWithNamesList ${_versionsWithNamesList}")

find_package(Qt NAMES ${_versionsWithNamesList} ${args})
if(Qt_FOUND)
	find_package(Qt${Qt_VERSION_MAJOR} REQUIRED ${args})

	get_cmake_property(cpackVarsToPassthrough VARIABLES)
	foreach(varName ${cpackVarsToPassthrough})
		if(varName MATCHES "^Qt${Qt_VERSION_MAJOR}(.+)$")
			set(newVarName "Qt${CMAKE_MATCH_1}")
			#message(TRACE "${varName} -> ${newVarName} ${${varName}}")
			set("${newVarName}" "${${varName}}")
			set("${newVarName}" "${${newVarName}}" PARENT_SCOPE)
		endif()
	endforeach()
endif()
