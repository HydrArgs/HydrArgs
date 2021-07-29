#pragma once

#include "../HydrArgs.hpp"

#ifdef HYDRARGS_FALLBACK_ERRORS_EXPORTS
	#define HYDRARGS_FALLBACK_ERRORS_API HYDRARGS_EXPORT_API
#else
	#define HYDRARGS_FALLBACK_ERRORS_API HYDRARGS_IMPORT_API
#endif

namespace HydrArgs::fallback::errors{

	HYDRARGS_FALLBACK_ERRORS_API void mandatoryArgumentMissingErrorMessage(IArgsParser *parser, Arg *argPtr, const char *argv0);  ///< Output a message when a mandatory argument is missing
	HYDRARGS_FALLBACK_ERRORS_API void mandatoryArgumentMissingErrorMessage(IArgsParser *parser, const char *argName, const char *argv0);  ///< Output a message when a mandatory argument is missing
	HYDRARGS_FALLBACK_ERRORS_API std::string mandatoryArgumentMissingFormatErrorMessage(const char *argName);  ///< Formats a message when a mandatory argument is missing

	HYDRARGS_FALLBACK_ERRORS_API std::string notEnoughMandatoryArgsFormatErrorMessage(uint32_t mandatoryArgsCount);
	HYDRARGS_FALLBACK_ERRORS_API void notEnoughMandatoryArgsErrorMessage(IArgsParser *parser, uint32_t mandatoryArgsCount, const char *argv0);  ///< Output a message when there is not enough positional mandatory arguments
};

#define PROCESS_ARGUMENT_MISSING_CASE() \
{\
	static_assert(std::is_base_of<Arg, std::remove_pointer<std::remove_reference<decltype(argPtr)>::type>::type>::value); \
	static_assert(std::is_base_of<CLIRawArgs, std::remove_reference<decltype(rawArgs)>::type>::value);\
	if(argPtr->minCount){\
		HydrArgs::fallback::errors::mandatoryArgumentMissingErrorMessage(this, argPtr, rawArgs.argv0);\
		return RESULT_SYNTAX_ERROR;\
	}\
	continue;\
}
