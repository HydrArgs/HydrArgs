#pragma once

#include <type_traits>

#include "../HydrArgs.hpp"
#include "./errors.hpp"

#ifdef HYDRARGS_FALLBACK_PARSING_EXPORTS
	#define HYDRARGS_FALLBACK_PARSING_API HYDRARGS_EXPORT_API
#else
	#define HYDRARGS_FALLBACK_PARSING_API HYDRARGS_IMPORT_API
#endif

#include <unordered_set>

namespace HydrArgs::fallback::parsing{

	HYDRARGS_FALLBACK_PARSING_API bool _parseBoolValue(const std::string &argStr, bool &value);  ///< Used to parse a boolen arg value into `value` arg from a string supplied in `argStr` arg. Returns `true` if has succeeded and `false` if the value contains incorrect boolean.

	HYDRARGS_FALLBACK_PARSING_API std::string parseBoolValue(const std::string &argStr, Arg *argPtr);  ///< Used to parse a boolen arg value from a string supplied in `argStr` arg. Returns a non-empty error message on failure.

	HYDRARGS_FALLBACK_PARSING_API std::string parseIntValue(const std::string &argStr, Arg *argPtr);  ///< Used to parse an int arg value from a string supplied in `argStr` arg. Returns a non-empty error message on failure.

	HYDRARGS_FALLBACK_PARSING_API std::string parseFloatValue(const std::string &argStr, Arg *argPtr);  ///< Used to parse a float arg value from a string supplied in `argStr` arg. Returns a non-empty error message on failure.

	HYDRARGS_FALLBACK_PARSING_API std::string parseDoubleValue(const std::string &argStr, Arg *argPtr);  ///< Used to parse a double arg value from a string supplied in `argStr` arg. Returns a non-empty error message on failure.

	HYDRARGS_FALLBACK_PARSING_API std::string parseArgFromString(const std::string &argStr, Arg *argPtr);  ///< Used to parse an arg value from a string supplied in `argStr` arg. Returns a non-empty error message on failure.


	extern const std::unordered_set<std::string> trueStrings HYDRARGS_FALLBACK_PARSING_API;  ///< List of strings considered to correspond to `true` bool values for the purpose of creation of a fallback implementation
	extern const std::unordered_set<std::string> falseStrings HYDRARGS_FALLBACK_PARSING_API;  ///< List of strings considered to correspond to `false` bool values for the purpose of creation of a fallback implementation

};

#define PARSE_TYPED_ARG_FROM_STRING(parserFunction, resStr, argv0)\
{\
	static_assert(\
		std::is_constructible<std::string, decltype(resStr)>::value\
		||\
		std::is_base_of<std::string, std::remove_const<std::remove_reference<decltype(resStr)>::type>::type>::value\
	);\
	static_assert(std::is_base_of<Arg, std::remove_pointer<std::remove_reference<decltype(argPtr)>::type>::type>::value);\
	static_assert(std::is_base_of<std::string, decltype(parserFunction(resStr, argPtr))>::value);\
	std::string errorMessage = parserFunction(resStr, argPtr);\
	if(errorMessage.size()){\
		streams.cerr << errorMessage << std::endl;\
		printHelp(streams.cerr, argv0);\
		return RESULT_SYNTAX_ERROR;\
	}\
}
#define PARSE_ARG_FROM_STRING(resStr, argv0) PARSE_TYPED_ARG_FROM_STRING(HydrArgs::fallback::parsing::parseArgFromString, resStr, argv0)

