#include <HydrArgs/HydrArgs.hpp>

namespace HydrArgs::Backend{

	const struct HydrArgs::ParserCapabilities capabilities{
		.help = {
			.error_message = @_ERROR_MESSAGE@,
			.help = @_HELP@,
			.usage = @_USAGE@,
			.descr = @_DESCR@,
			.value_name = @_VALUE_NAME@,
			.value_unit = @_VALUE_UNIT@
		},
		.native_positional = {
			.mandatory = @_POSITIONAL_MANDATORY@,
			.optional = @_POSITIONAL_OPTIONAL@,
			.optional_before_mandatory = @_POSITIONAL_OPTIONAL_BEFORE_MANDATORY@,
			.booleans = @_POSITIONAL_BOOLEANS@
		},
		.dashed = {
			.mandatory = @_DASHED_MANDATORY@,
			.optional = @_DASHED_OPTIONAL@
		},
		.bellsAndWhistles = {
			.path_validation = @_PATH_VALIDATION@,
			.auto_complete = @_AUTO_COMPLETE@,
			.wide_strings = @_WIDE_STRINGS@
		},
		.important = {
			.stable = @_STABLE@,
			.bug_memory_safety = @_BUG_MEMORY_SAFETY@,
			.bug_terminates_itself = @_TERMINATES_ITSELF@,
			.bug_prints_itself = @_PRINTS_ITSELF@,
			.thread_unsafe = @_THREAD_UNSAFE@,
			.non_reentrable = @_NON_REENTRABLE@,
			.other_grave_bugs = @_BUG_GRAVE_OTHER@,
			.permissive_license = @_PERMISSIVE@
		}
	};
};
