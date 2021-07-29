#include <cstdint>

#include <HydrArgs/fallback/positional/iterative.hpp>

using namespace HydrArgs;

namespace HydrArgs::fallback::positional{

	void PositionalParserIterative::addPosArg(Arg *argPtr){
		//static_assert(isPositional);
		positionalArgsCount++;
		mandatoryPositionalArgsCount += argPtr->minCount;
	}

	PositionalParserIterative::PositionalParserIterative(IArgsParser *parent): IPositionalParser(parent), positionalArgsCount(0U), mandatoryPositionalArgsCount(0U) {
		resetPositionalParsing();
	}

	uint32_t PositionalParserIterative::getMandatoryPositionalArgsCount() const {
		return mandatoryPositionalArgsCount;
	};

	PositionalParserIterative::~PositionalParserIterative() = default;

};
