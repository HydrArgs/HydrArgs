#include <cstdint>

#include <HydrArgs/fallback/errors.hpp>
#include <HydrArgs/fallback/positional/IPositionalParser.hpp>

using namespace HydrArgs;
using namespace HydrArgs::fallback::positional;

IPositionalParser::IPositionalParser(IArgsParser *parent): IArgsParserClient(parent){};
IPositionalParser::~IPositionalParser() = default;

void IPositionalParser::resetPositionalParsing(){
	positionalCounter = 0;
}

uint32_t IPositionalParser::getNextPosArgIdx() {
	return positionalCounter++;
}

struct ParseEvents IPositionalParser::checkIfEnoughMandPosArgs(const char *argv0) {
	auto mandatoryArgsCount = getMandatoryPositionalArgsCount();
	if(isNotEnoughArgs(mandatoryArgsCount)) {
		HydrArgs::fallback::errors::notEnoughMandatoryArgsErrorMessage(parent, mandatoryArgsCount, argv0);
		return EVENTS_SYNTAX_ERROR;
	}
	return EVENTS_NONE;
}
