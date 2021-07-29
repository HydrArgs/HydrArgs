#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include <HydrArgs/HydrArgs.hpp>
#include <HydrArgs/fallback/positional/spannable.hpp>

#include "num_utils.hpp"

#include <sweet/options.hpp>

namespace HydrArgs::Backend{
struct SweetBackend: public IBackendOwnStoredSpec, public fallback::positional::PositionalParserSpannable{
	const std::string &name;
	const std::string &descr;
	const std::string &usage;

	SweetBackend(const std::string &name, const std::string &descr, const std::string &usage [[maybe_unused]], std::vector<Arg*> dashedSpec, std::vector<Arg*> positionalSpec, Streams streams): IBackendOwnStoredSpec(dashedSpec, positionalSpec, streams), PositionalParserSpannable(this), name(name), descr(descr), usage(usage){
		for(auto &argPtr: dashedSpec){
			_addArg(argPtr, false);
		}
		for(auto &argPtr: positionalSpec){
			_addArg(argPtr, true);
		}
	}

	virtual void _addArg(Arg *argPtr, bool isPositional) override {
		if(isPositional){
			addPosArg(argPtr);
		} else {
		}
	}

	virtual ~SweetBackend() override = default;

	virtual bool isNotEnoughArgs(uint32_t positionalArgCountLowerBound) const override {
		//#error "TODO"
		return false;
	}

	virtual void printHelp(std::ostream &stream, const char * const argv0) override {
		stream << argv0 << " " << usage << std::endl;
		stream << descr << std::endl;
	}

	sweet::Options *app = nullptr;

	virtual ParseResult _parseArgs(CLIRawArgs rawArgs) override {
		auto tempArgvVec = rawArgs.getArgvVector();
		sweet::Options appLocal(checked_cast<int>(tempArgvVec.size() - 1), const_cast<char **>(tempArgvVec.data()));
		app = &appLocal;

		bool show_help = false;
		appLocal.get(DEFAULT_HELP_ARG.letter_str.dashed, DEFAULT_HELP_ARG.long_name.dashed, DEFAULT_HELP_ARG.doc, show_help);
		appLocal.finalize();

		if(show_help) {
			printHelp(streams.cout, rawArgs.argv0);
			return RESULT_HELP_CALLED;
		}

		HYDRARGS_CHECK_IF_ENOUGH_MAND_POS_ARGS_AND_RETURN_ERROR_OTHERWISE();

		//resetPositionalParsing();
		//getNextPosArgIdx();  // indices start from 1

		//ToDo: positionalSpec
		for(auto &argPtr: dashedSpec){
			{
				switch(argPtr->type){
					case ArgType::flag:
					{
						auto specOptPtr = static_cast<SArg<ArgType::flag>*>(argPtr);
						appLocal.get(argPtr->letterStr, "--" + argPtr->name, argPtr->description, specOptPtr->value);
					}
					break;
					case ArgType::s4:
					{
						auto specOptPtr = static_cast<SArg<ArgType::s4>*>(argPtr);
						appLocal.get(argPtr->letterStr, "--" + argPtr->name, argPtr->description, specOptPtr->value);
					}
					break;
					case ArgType::string:
					case ArgType::path:
					{
						auto specOptPtr = static_cast<SArg<ArgType::string>*>(argPtr);
						appLocal.get(argPtr->letterStr, "--" + argPtr->name, argPtr->description, specOptPtr->value);
					}
					break;
					default:
						throw UnsupportedArgumentType{argPtr->type};
				}
			}
		}

		appLocal.finalize();

		//std::transform(begin(remaining), end(remaining), begin(remainingPtrs), [](std::string &el) -> char * {return el.data();});
		return ParseResult{
			.parsingStatus = STATUS_OK,
			.rest = {
				.argv0 = rawArgs.argv0,
				//.argvRest = {const_cast<const char **>(remainingPtrs.data()), remainingPtrsSize}
				.argvRest = {static_cast<const char **>(nullptr), 0}
			}
		};
	}
};

IArgsParser * argsParserFactory(const std::string &name, const std::string &descr, const std::string &usage [[maybe_unused]], std::vector<Arg*> dashedSpec, std::vector<Arg*> positionalSpec, Streams streams){
	return new SweetBackend(name, descr, usage, dashedSpec, positionalSpec, streams);
}

};
