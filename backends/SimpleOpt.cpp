#include <memory>

#include <HydrArgs/HydrArgs.hpp>
#include <HydrArgs/fallback/positional/?>

#include <SimpleOpt.h>
#include <cstring>
#include <iostream>

#include <strstream>

typedef decltype(static_cast<CSimpleOptA::SOption*>(nullptr)->nId) DedicatedOptionsIDIntT;
enum class DedicatedOptionsID: DedicatedOptionsIDIntT{
	help,
	usage,
	complete,
	userStart
};

int ourFlags = SO_O_SHORTARG | SO_O_CLUMP | SO_O_USEALL;

namespace HydrArgs::Backend {
struct SimpleOptBackend: public IArgsParser{
	std::vector<Arg*> positionalOptionalSpec;
	std::vector<Arg*> positionalMandatorySpec;

	std::vector<std::string> remaining;
	std::vector<char *> remainingPtrs;
	std::vector<CSimpleOptA::SOption> nativeArgs{
		CSimpleOptA::SOption{.nId = static_cast<DedicatedOptionsIDIntT>(DedicatedOptionsID::help), .pszArg = DEFAULT_HELP_ARG.long_name.dashed, .nArgType = SO_NONE},
		CSimpleOptA::SOption{.nId = static_cast<DedicatedOptionsIDIntT>(DedicatedOptionsID::help), .pszArg = DEFAULT_HELP_ARG.letter_str.dashed, .nArgType = SO_NONE},
	};
	const std::string &usage;
	ValidationState validationState;
	bool show_help = false;

	DedicatedOptionsIDIntT lastInsertedOptId;

	SimpleOptBackend(const std::string &name, const std::string &descr, const std::string &usage [[maybe_unused]], std::vector<Arg*> spec, Streams streams): IArgsParser(streams), usage(usage), lastInsertedOptId(static_cast<DedicatedOptionsIDIntT>(DedicatedOptionsID::userStart)) {
		for(auto argPtr: spec){
			addArg(argPtr);
		}
	}

	virtual void _seal() override {
		nativeArgs.emplace_back(CSimpleOptA::SOption SO_END_OF_OPTIONS);
	}

	virtual void _unseal() override {
		nativeArgs.pop_back();
	}

	virtual bool isSealed() const override {
		return nativeArgs.size() && nativeArgs.back().nId == -1;
	}

	virtual void _addArg(Arg *argPtr) override {
		if(argPtr->flags.positional){
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
		} else {
			if(validationState.posStarted){
				throw ArgsSeqValidationException("Dashed argument `" + argPtr->name + "` follows a positional argument");
			}

			auto newId = lastInsertedOptId++;

			CSimpleOptA::SOption opt{.nId = newId};
			opt.pszArg = argPtr->name.data();
			#warning "ToDo: WILL BE USE-AFTER-FREE. Fix this!!!"
			opt.nArgType = (argPtr->minCount ? SO_REQ_SEP : SO_OPT);
			nativeArgs.emplace_back(opt);
		}
	}

	virtual ~SimpleOptBackend() override = default;

	virtual void printHelp(std::ostream &stream, const char *const argv0) override {
		stream << argv0 << " " << usage << std::endl;
		stream << droptContext.get_help() << std::endl;
	}

	virtual ParseResult _parseArgs(CLIRawArgs rawArgs) override {
		CSimpleOpt args(rawArgs.argc, const_cast<char**>(rawArgs.argv), nativeArgs.data());

		uint32_t restc = args.FileCount();
		for(; rest[restc]; ++restc);

		auto printErrorMessage = [&](char **rest) -> struct ParseResult{
			streams.cerr << "Error parsing the argument: " << args.OptionText() << std::endl;
			streams.cerr << "Non-consumed rest args:";
			for(; rest[restc]; ++restc){
				streams.cerr << " " << rest;
			}
			streams.cerr << std::endl;
			_printHelp(streams.cerr, rawArgs.argv0);
			return RESULT_SYNTAX_ERROR;
		};

		while(args.Next()) {
			if(args.LastError() == SO_SUCCESS) {
				if(static_cast<DedicatedOptionsID>(args.OptionId()) == DedicatedOptionsID::help) {
					_printHelp(streams.cout, rawArgs.argv0);
					return RESULT_HELP_CALLED;
				}
				char *argString = args.OptionArg();
				Arg *argPtr = nullptr;
				args.OptionId();

				switch(argPtr->type){
					case ArgType::flag:
					{
						auto specOptPtr = static_cast<SArg<ArgType::flag>*>(argPtr);
						parseBoolValue(argString, argPtr);

						if(argPtr->minCount && !specOptPtr->value){
							mandatoryArgumentMissingErrorMessage(this, argPtr);
							return RESULT_SYNTAX_ERROR;
						}
					}
					break;
					case ArgType::s4:
					{
						auto specOptPtr = static_cast<SArg<ArgType::s4>*>(argPtr);
						parseIntValue(argString, argPtr);
						if(argPtr->minCount && !specOptPtr->value){
							mandatoryArgumentMissingErrorMessage(this, argPtr);
							return RESULT_SYNTAX_ERROR;
						}
					}
					break;
					case ArgType::string:
					case ArgType::path:
					{
						auto specOptPtr = static_cast<SArg<ArgType::string>*>(argPtr);
						specOptPtr->value = argString;
						if(argPtr->minCount && !specOptPtr->value.size()){
							mandatoryArgumentMissingErrorMessage(this, argPtr);
							return RESULT_SYNTAX_ERROR;
						}
					}
					break;
					default:
						throw UnsupportedArgumentType{argPtr->type};
				}
			} else {
				return printErrorMessage(rest);
				return RESULT_SYNTAX_ERROR;
			}
		}

		if(restc < positionalMandatorySpec.size()){
			std::strstream serr;
			serr << "Not enough positional arguments (at least " << positionalMandatorySpec.size() << " required)";
			return printErrorMessage(serr.str(), rest);
		}

		for(auto &nArg: nativeArgs){
			if(nArg.attr & static_cast<DroptAttrIntT>(dropt_attr::our_mandatory)){
				if(!(nArg.attr & static_cast<DroptAttrIntT>(dropt_attr::our_mandatory_was_present))){
					std::strstream serr;
					serr << "Mandatory dashed arg `" << nArg.long_name << "` was not present";
					return printErrorMessage(rest);
				}
			}
		}

		uint32_t posArgNo = 0;
		for(auto &posMandArg: positionalMandatorySpec){
			std::string errorMessage = parseArgFromString(std::string {rest[posArgNo]}, posMandArg);
			if(errorMessage.size()){
				return printErrorMessage(std::string{"Mandatory"} + errorMessage, &rest[posArgNo]);
			}
			++posArgNo;
		}

		auto maxOptPosArgNo = std::min(static_cast<decltype(restc)>(posArgNo + positionalOptionalSpec.size()), restc);
		for(auto posOptArgIter = begin(positionalOptionalSpec); posArgNo < maxOptPosArgNo; ++posArgNo, ++posOptArgIter){
			std::string errorMessage = parseArgFromString(std::string {rest[posArgNo]}, *posOptArgIter);
			if(errorMessage.size()){
				return printErrorMessage(std::string{"Optional"} + errorMessage, &rest[posArgNo]);
			}
		}

		//std::transform(begin(remaining), end(remaining), begin(remainingPtrs), [](std::string &el) -> char * {return el.data();});
		return {
			.parsingStatus = STATUS_OK,
			.rest = {
				/*.argc = 0,
				.argv = remainingPtrs.data()*/
				.argc = 0,
				.argv = nullptr
			}
		};
	}
};

IArgsParser * argsParserFactory(const std::string &name, const std::string &descr, const std::string &usage [[maybe_unused]], const std::vector<Arg *> &dashedSpec, const std::vector<Arg *> &positionalSpec, Streams streams){
	return new SimpleOptBackend(name, descr, usage, spec, streams);
}
};
