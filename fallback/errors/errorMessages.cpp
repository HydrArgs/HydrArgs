#include <cstdint>
#include <sstream>
#include <string>

#include <HydrArgs/fallback/errors.hpp>

using namespace HydrArgs;

namespace HydrArgs::fallback::errors{

	std::string mandatoryArgumentMissingFormatErrorMessage(const char *argName){
		return std::string {"Mandatory argument `"} + argName + "` is missing";
	}

	void mandatoryArgumentMissingErrorMessage(IArgsParser *parser, const char *argName, const char *argv0){
		parser->streams.cerr << mandatoryArgumentMissingFormatErrorMessage(argName) << std::endl;
		parser->printHelp(parser->streams.cerr, argv0);
	}

	void mandatoryArgumentMissingErrorMessage(IArgsParser *parser, Arg *argPtr, const char *argv0){
		mandatoryArgumentMissingErrorMessage(parser, argPtr->name.data(), argv0);
	}

	std::string notEnoughMandatoryArgsFormatErrorMessage(uint32_t mandatoryArgsCount){
		std::stringstream serr;
		serr << "Not enough positional arguments (at least " << mandatoryArgsCount << " required)";
		return serr.str();
	}

	void notEnoughMandatoryArgsErrorMessage(IArgsParser *parser, uint32_t mandatoryArgsCount, const char *argv0){
		parser->streams.cerr << notEnoughMandatoryArgsFormatErrorMessage(mandatoryArgsCount) << std::endl;
		parser->printHelp(parser->streams.cerr, argv0);
	}
};
