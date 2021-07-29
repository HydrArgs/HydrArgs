buildAndPackageLib("${PROJECT_NAME}_fallback_parsing"
	COMPONENT "fallback_parsing_lib"
	DESCRIPTION "A library implementing parsing (string to desired arg typr) functions useful for ${PROJECT_NAME} backends implementations"
	PRIVATE_INCLUDES ${Include_dir}
	PRIVATE_LIBS "${PROJECT_NAME}"
)
