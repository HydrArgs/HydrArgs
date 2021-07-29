buildAndPackageLib("${PROJECT_NAME}_fallback_positional"
	COMPONENT "fallback_positional_lib"
	DESCRIPTION "A library implementing classes useful for creation of ${PROJECT_NAME} backends for the libs not having built-in positional args support"
	PRIVATE_INCLUDES ${Include_dir}
	PRIVATE_LIBS "${PROJECT_NAME}" "${PROJECT_NAME}_fallback_errors" "${PROJECT_NAME}_fallback_parsing"
)

