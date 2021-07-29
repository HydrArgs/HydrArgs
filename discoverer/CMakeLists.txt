find_package(whereami REQUIRED)
#find_package(function-loader REQUIRED)

# https://github.com/karel-burda/function-loader/tree/develop libfunction-loader-dev  - lacks the needed features
# https://github.com/martin-olivier/dylib the upstream has removed the packaging code, so we have to inline it. https://github.com/martin-olivier/dylib/issues/69
set(USE_DYLIB ON)

buildAndPackageLib("${PROJECT_NAME}_discoverer"
	COMPONENT "discoverer"
	DESCRIPTION "A ${PROJECT_NAME} pseudo-backend dynamically loading and selecting a backend"
	PRIVATE_INCLUDES ${Include_dir} ${Private_include_dir}
	PUBLIC_LIBS "lib${PROJECT_NAME}"
	PRIVATE_LIBS "whereami::whereami" ${CMAKE_DL_LIBS}
)

target_compile_definitions(HydrArgs_discoverer PRIVATE "-DHYDRARGS_BACKEND_EXPORTS" "$<$<CONFIG:Debug>:HYDRARGS_SCAN_BACKENDS_DIR=1>")
if(USE_DYLIB)
	target_include_directories(HydrArgs_discoverer PRIVATE "${CMAKE_CURRENT_LIST_DIR}/dylib")
	target_compile_definitions(HydrArgs_discoverer PRIVATE "-DUSE_DYLIB")
else()
	target_link_libraries(HydrArgs_discoverer PRIVATE "burda::function-loader")
endif()

