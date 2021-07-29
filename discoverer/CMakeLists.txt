find_package(whereami REQUIRED)

buildAndPackageLib("${PROJECT_NAME}_discoverer"
	COMPONENT "discoverer"
	DESCRIPTION "A ${PROJECT_NAME} pseudo-backend dynamically loading and selecting a backend"
	PRIVATE_INCLUDES ${Include_dir}
	PUBLIC_LIBS "${PROJECT_NAME}"
	PRIVATE_LIBS "whereami::whereami" ${CMAKE_DL_LIBS}
)

target_compile_definitions(HydrArgs_discoverer PRIVATE "-DHYDRARGS_BACKEND_EXPORTS" "$<$<CONFIG:Debug>:HYDRARGS_SCAN_BACKENDS_DIR=1>")
