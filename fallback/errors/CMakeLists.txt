buildAndPackageLib("${PROJECT_NAME}_fallback_errors"
	COMPONENT "fallback_parsing_lib"
	DESCRIPTION "A library implementing error messages generation for ${PROJECT_NAME} backends implementations"
	PRIVATE_INCLUDES ${Include_dir} ${Private_include_dir}
	PRIVATE_LIBS "lib${PROJECT_NAME}"
)
