#include <HydrArgs/fallback/positional/iterative.hpp>

#include <cstring>
#include <span>

using namespace HydrArgs;

namespace HydrArgs::fallback::positional{

	void PositionalParserIterative::addPosArg(Arg *argPtr){
		//static_assert(isPositional);
		positionalArgsCount++;
		mandatoryPositionalArgsCount += argPtr->minCount;
	}

	PositionalParserIterative::PositionalParserIterative(IArgsParser *parent): IPositionalParser(parent), positionalArgsCount(0u), mandatoryPositionalArgsCount(0u) {
		resetPositionalParsing();
	}

	uint32_t PositionalParserIterative::getMandatoryPositionalArgsCount() const {
		return mandatoryPositionalArgsCount;
	};

	PositionalParserIterative::~PositionalParserIterative() = default;

};
