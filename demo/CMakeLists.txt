file(GLOB SRCFILES "${CMAKE_CURRENT_SOURCE_DIR}/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(demo "${SRCFILES}")
target_include_directories(demo PRIVATE "${Include_dir}" "${Private_include_dir}")
target_link_libraries(demo PRIVATE ${PROJECT_NAME}_discoverer)
#target_link_libraries(demo PRIVATE ${PROJECT_NAME}_backend_octargs)
#target_link_libraries(demo PRIVATE ${PROJECT_NAME}_backend_Lyra)
#target_link_libraries(demo PRIVATE ${PROJECT_NAME}_backend_CLI11)
#target_link_libraries(demo PRIVATE ${PROJECT_NAME}_backend_dummy)
#target_link_libraries(demo PRIVATE ${PROJECT_NAME}_backend_BoostArgs)
#target_link_libraries(demo PRIVATE ${PROJECT_NAME}_backend_Dropt)

set_target_properties(demo PROPERTIES
	CXX_STANDARD 20
)


harden(demo)
add_sanitizers(demo)

cpack_add_component(demo
	DISPLAY_NAME "demo"
	DESCRIPTION "The demo"
	REQUIRED
)

install(TARGETS "demo"
	RUNTIME
	COMPONENT "demo"
)
set("CPACK_DEBIAN_DEMO_PACKAGE_NAME" "HydrArgs-demo")
set("CPACK_RPM_DEMO_PACKAGE_NAME" "HydrArgs-demo")

#list(APPEND CPACK_COMPONENTS_ALL "demo")  # strangely, not populated automatically correctly


pass_through_cpack_vars()
