#pragma once

#include "./HydrArgs.hpp"

namespace HydrArgs{
	HYDRARGS_API void syntaxErrorAroundArgErrorMessage(const std::string &argName, const std::string &error, std::ostream &s);///< Output a message when a mandatory argument is missing
	HYDRARGS_API void syntaxErrorAroundArgErrorMessage(const char *argName, const std::string &error, std::ostream &s);  ///< Output a message when a mandatory argument is missing
	HYDRARGS_API void syntaxErrorAroundArgErrorMessage(const char *argName, const char *error, std::ostream &s);  ///< Output a message when a mandatory argument is missing
	HYDRARGS_API void syntaxErrorAroundArgErrorMessage(const std::string &argName, const char *error, std::ostream &s);  ///< Output a message when a mandatory argument is missing
};

#define PROCESS_CASE_OF_SYNTAX_ERROR_MESSAGE_WITH_KNOWN_LOCATION(argName, error) \
{\
	auto argNameVar = (argName);\
	auto errorVar = (error);\
	syntaxErrorAroundArgErrorMessage(argNameVar, errorVar, streams.cerr);\
	printHelp(streams.cerr, rawArgs.argv0);\
	return RESULT_SYNTAX_ERROR;\
}

#define PROCESS_CASE_OF_SYNTAX_ERROR_MESSAGE(msg) \
{\
	auto msgVar = (msg);\
	streams.cerr << msgVar << std::endl;\
	printHelp(streams.cerr, rawArgs.argv0);\
	return RESULT_SYNTAX_ERROR;\
}

#define PROCESS_CASE_OF_SYNTAX_ERROR_WHATABLE_EXCEPTION(ex) PROCESS_CASE_OF_SYNTAX_ERROR_MESSAGE(ex.what())

#define PROCESS_CASE_OF_HELP_CALLED(help_called) \
{\
	bool wasHelpCalled = (help_called);\
	if(wasHelpCalled) {\
		printHelp(streams.cout, rawArgs.argv0);\
		return RESULT_HELP_CALLED;\
	}\
}
