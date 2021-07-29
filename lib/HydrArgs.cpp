#include <HydrArgs/HydrArgs.hpp>
#include <cstdlib>
#include <iostream>
#include <utility>
#include <algorithm>
#include <iterator>
#include <span>

#include <HydrArgs/HydrArgs.hpp>

namespace HydrArgs{

	ArgsSeqValidationException::ArgsSeqValidationException(const std::string &what): std::logic_error(what) {}
	ArgsSeqValidationException::ArgsSeqValidationException(): ArgsSeqValidationException("Invalid args sequence!") {}

	Arg::Arg(ArgType type, char letter, std::string name, std::string description, uint32_t minCount, const char *value_name, const char *value_unit): type(type), flags({}), letterStr {letter, 0U}, name(std::move(name)), description(std::move(description)), minCount(minCount), value_name(value_name), value_unit(value_unit){}

	Arg::Arg(ArgType type, std::string name, std::string description, uint32_t minCount, const char *value_name, const char *value_unit): Arg(type, '\0', std::move(name), std::move(description), minCount, value_name, value_unit){}

	Arg::Arg(ArgType type, char letter, std::string name, std::string description, uint32_t minCount, const char *value_name): Arg(type, letter, std::move(name), std::move(description), minCount, value_name, nullptr){}

	Arg::Arg(ArgType type, std::string name, std::string description, uint32_t minCount, const char *value_name): Arg(type, std::move(name), std::move(description), minCount, value_name, nullptr){}

	Arg::Arg(ArgType type, char letter, std::string name, std::string description, uint32_t minCount): Arg(type, letter, std::move(name), std::move(description), minCount, nullptr){}

	Arg::Arg(ArgType type, std::string name, std::string description, uint32_t minCount): Arg(type, std::move(name), std::move(description), minCount, nullptr){}

	template struct SArg<ArgType::flag>;
	template struct SArg<ArgType::s4>;
	template struct SArg<ArgType::string>;

	PathArg::PathArg(char letter, std::string name, std::string description, uint32_t minCount, const char *value_name, const char *value_unit, StorageType defaultValue): SArg<ArgType::string>(ArgType::path, letter, std::move(name), std::move(description), minCount, value_name, value_unit, std::move(defaultValue)){}
	PathArg::PathArg(std::string name, std::string description, uint32_t minCount, const char *value_name, const char *value_unit, StorageType defaultValue): SArg<ArgType::string>(ArgType::path, std::move(name), std::move(description), minCount, value_name, value_unit, std::move(defaultValue)){}

	PathArg::PathArg(char letter, std::string name, std::string description, uint32_t minCount, const char *value_name, StorageType defaultValue): SArg<ArgType::string>(ArgType::path, letter, std::move(name), std::move(description), minCount, value_name, std::move(defaultValue)){}
	PathArg::PathArg(std::string name, std::string description, uint32_t minCount, const char *value_name, StorageType defaultValue): SArg<ArgType::string>(ArgType::path, std::move(name), std::move(description), minCount, value_name, std::move(defaultValue)){}

	PathArg::PathArg(char letter, std::string name, std::string description, uint32_t minCount, StorageType defaultValue): SArg<ArgType::string>(ArgType::path, letter, std::move(name), std::move(description), minCount, std::move(defaultValue)){}
	PathArg::PathArg(std::string name, std::string description, uint32_t minCount, StorageType defaultValue): SArg<ArgType::string>(ArgType::path, std::move(name), std::move(description), minCount, std::move(defaultValue)){}

	IArgsParser::IArgsParser(Streams streams): streams(streams){};
	IArgsParser::IArgsParser(): IArgsParser(DEFAULT_STREAMS){};
	IArgsParser::~IArgsParser() = default;

	void IArgsParser::seal(){
		if(!isSealed()){
			_seal();
		}
	}
	void IArgsParser::unseal(){
		if(isSealed()){
			_unseal();
		}
	}
	bool IArgsParser::isSealed() const {
		return false;
	}

	void IArgsParser::_seal(){}
	void IArgsParser::_unseal(){}

	void IArgsParser::addArg(Arg *argPtr, bool isPositional){
		unseal();
		_addArg(argPtr, isPositional);
	}

	IArgsParserClient::IArgsParserClient(IArgsParser *parent): parent(parent){}

	IBackendOwnStoredSpec::IBackendOwnStoredSpec(std::vector<Arg *> dashedSpec, std::vector<Arg *> positionalSpec):  dashedSpec(std::move(dashedSpec)), positionalSpec(std::move(positionalSpec)){}
	IBackendOwnStoredSpec::IBackendOwnStoredSpec(std::vector<Arg *> dashedSpec, std::vector<Arg *> positionalSpec, Streams streams): IArgsParser(streams), dashedSpec(std::move(dashedSpec)), positionalSpec(std::move(positionalSpec)){}
	IBackendOwnStoredSpec::~IBackendOwnStoredSpec() = default;

	void IBackendOwnStoredSpec::addArg(Arg *argPtr, bool isPositional){
		IArgsParser::addArg(argPtr, isPositional);
		if(isPositional){
			positionalSpec.emplace_back(argPtr);
		} else {
			dashedSpec.emplace_back(argPtr);
		}
	}

	ParseResult IArgsParser::operator()(CLIRawArgs rawArgs) {
		seal();
		return _parseArgs(rawArgs);
	}

	/*
	ParseStatus::ParseStatus(){
		std::memset(this, 0, sizeof(decltype(*this)));
	}

	ParseStatus::ParseStatus(ParseStatus&) = default;
	ParseStatus::ParseStatus(const ParseStatus&) = default;
	ParseStatus::ParseStatus(ParseStatus&&) = default;
	ParseStatus::ParseStatus(int returnCode): returnCode(returnCode), events({.syntaxError = !!returnCode}){};
	ParseStatus::ParseStatus(struct ParseEvents events): returnCode(events.syntaxError), events(events){};
	ParseStatus::ParseStatus(int returnCode, struct ParseEvents events): returnCode(returnCode), events(events){};
	*/

	CLIRawArgs::VectorT CLIRawArgs::getArgvVector() const {
		VectorT res;
		auto resSize = argvRest.size() + 2;
		res.reserve(resSize);
		res.resize(resSize);
		res[0] = argv0;
		res[res.size() - 1] = nullptr;
		std::copy(begin(argvRest) , end(argvRest), begin(res) + 1);
		return res;
	}

	const CLIRawArgs::SpanT NO_REST_ARGS = {static_cast<const char **>(nullptr), 0};

	const struct CLIRawArgs NO_ARGS = {
		.argv0 = nullptr,
		.argvRest = NO_REST_ARGS
	};

	const struct ParseEvents EVENTS_NONE{};
	const struct ParseEvents EVENTS_HELP_CALLED{.helpCalled = true};
	const struct ParseEvents EVENTS_AUTOCOMPLETE_CALLED{.autoCompleteCalled = true};
	const struct ParseEvents EVENTS_SYNTAX_ERROR{.syntaxError = true};

	const struct ParseStatus STATUS_OK{EXIT_SUCCESS, EVENTS_NONE};
	const struct ParseStatus STATUS_HELP_CALLED{EXIT_SUCCESS, EVENTS_HELP_CALLED};
	const struct ParseStatus STATUS_AUTOCOMPLETE_CALLED{EXIT_SUCCESS, EVENTS_AUTOCOMPLETE_CALLED};
	const struct ParseStatus STATUS_SYNTAX_ERROR{EXIT_FAILURE, EVENTS_SYNTAX_ERROR};

	const struct ParseResult RESULT_HELP_CALLED{.parsingStatus = STATUS_HELP_CALLED, .rest = NO_ARGS};
	const struct ParseResult RESULT_AUTOCOMPLETE_CALLED{.parsingStatus = STATUS_AUTOCOMPLETE_CALLED, .rest = NO_ARGS};
	const struct ParseResult RESULT_SYNTAX_ERROR{.parsingStatus = STATUS_SYNTAX_ERROR, .rest = NO_ARGS};

	template struct DefaultArgName<2>;
	template struct DefaultArgName<1>;

	const struct DefaultArgInfo DEFAULT_HELP_ARG{
		.long_name = "--help",
		.letter_str = "-h",
		.doc = "Display this help"
	};

	const char *const VALUE_PLACEHOLDER = "value";

	const Streams DEFAULT_STREAMS{
		.cout = std::cout,
		.cerr = std::cerr
	};

	const char SYNTAX_ERRORR_AROUND[]{"Syntax error around `"};

	void syntaxErrorAroundArgErrorMessage(const char *argName, const std::string &error, std::ostream &s){
		s << SYNTAX_ERRORR_AROUND << argName << "`: " << error << std::endl;
	}
	void syntaxErrorAroundArgErrorMessage(const std::string &argName, const char *error, std::ostream &s){
		s << SYNTAX_ERRORR_AROUND << argName << "`: " << error << std::endl;
	}
	void syntaxErrorAroundArgErrorMessage(const std::string &argName, const std::string &error, std::ostream &s){
		s << SYNTAX_ERRORR_AROUND << argName << "`: " << error << std::endl;
	}
	void syntaxErrorAroundArgErrorMessage(const char *argName, const char *error, std::ostream &s){
		s << SYNTAX_ERRORR_AROUND << argName << "`: " << error << std::endl;
	}

};
