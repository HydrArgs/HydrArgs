#include <cstdint>
#include <cstring>
#include <span>
#include <algorithm>
#include <iterator>
#include <ostream>
#include <string>
#include <vector>

#include <HydrArgs/fallback/parsing.hpp>
#include <HydrArgs/fallback/positional/spannable.hpp>

namespace HydrArgs::fallback::positional{

	PositionalParserSpannable::PositionalParserSpannable(IArgsParser *parent): IPositionalParser(parent){};
	PositionalParserSpannable::~PositionalParserSpannable() = default;

	void PositionalParserSpannable::addPosArg(Arg *argPtr){
		validationState.posStarted = true;
		if(argPtr->minCount) {
			if(validationState.posOptStarted){
				throw ArgsSeqValidationException("Positional mandatory argument `" + argPtr->name + "` follows positional optional");
			}
			positionalMandatorySpec.emplace_back(argPtr);
		} else {
			validationState.posOptStarted = true;
			positionalOptionalSpec.emplace_back(argPtr);
		}
	}

	void PositionalParserSpannable::validateDashed(Arg *argPtr) const {
		if(validationState.posStarted) {
			throw ArgsSeqValidationException("Dashed argument `" + argPtr->name + "` follows a positional argument");
		}
	}

	struct ParseResult PositionalParserSpannable::parsePositionals(const char *const argv0, std::span<const char *> rest){
		resetPositionalParsing();
		decltype(getNextPosArgIdx()) posArgNo = getNextPosArgIdx();
		for(auto &posMandArg: positionalMandatorySpec){
			std::string const errorMessage = HydrArgs::fallback::parsing::parseArgFromString(std::string {rest[posArgNo]}, posMandArg);
			if(!errorMessage.empty()){
				return printErrorMessage(argv0, std::string{"Mandatory"} + errorMessage, rest);
			}
			posArgNo = getNextPosArgIdx();
		}

		auto maxOptPosArgNo = std::min(static_cast<decltype(rest.size())>(posArgNo + positionalOptionalSpec.size()), rest.size());
		for(auto posOptArgIter = begin(positionalOptionalSpec); posArgNo < maxOptPosArgNo; posArgNo = getNextPosArgIdx(), ++posOptArgIter){
			std::string const errorMessage = HydrArgs::fallback::parsing::parseArgFromString(std::string {rest[posArgNo]}, *posOptArgIter);
			if(!errorMessage.empty()){
				return printErrorMessage(argv0, std::string{"Optional"} + errorMessage, rest);
			}
		}
		return {
			.parsingStatus = STATUS_OK,
			.rest = CLIRawArgs{
				.argv0 = argv0,
				.argvRest = (
					rest.size() > posArgNo
					?
					CLIRawArgs::SpanT{const_cast<const char **>(&rest[posArgNo]), static_cast<size_t>(rest.size() - posArgNo)}
					:
					CLIRawArgs::SpanT{static_cast<const char **>(nullptr), 0}
				)
			}
		};
	}

	struct ParseResult PositionalParserSpannable::printErrorMessage(const char *const argv0, const std::string &errMsg, std::span<const char *> rest) {
		parent->streams.cerr << errMsg << std::endl;
		parent->streams.cerr << "Non-consumed rest args:";
		for(const auto restArg: rest) {
			parent->streams.cerr << " " << restArg;
		}
		parent->streams.cerr << std::endl;
		parent->printHelp(parent->streams.cerr, argv0);
		return RESULT_SYNTAX_ERROR;
	};

	uint32_t PositionalParserSpannable::getMandatoryPositionalArgsCount() const {
		return static_cast<uint32_t>(this->positionalMandatorySpec.size());
	};

};
