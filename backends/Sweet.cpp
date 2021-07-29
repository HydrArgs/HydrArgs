#include <memory>

#include <HydrArgs/HydrArgs.hpp>
#include <HydrArgs/fallback/positional.hpp>

#include <iostream>
#include <sweet/options.hpp>

struct SweetBackend: public IBackendOwnStoredSpec, public PositionalParserBackendStored{
	const std::string &name;
	const std::string &descr;
	const std::string &usage;

	SweetBackend(const std::string &name, const std::string &descr, const std::string &usage [[maybe_unused]], std::vector<Arg*> spec, Streams streams): IBackendOwnStoredSpec(spec, streams), PositionalParserBackendStored(this), name(name), descr(descr), usage(usage){
		for(auto &arg: spec){
			_addArg(arg);
		}
	}

	virtual void _addArg(Arg *argPtr) override {
		if(argPtr->flags.positional){
			addPosArg(argPtr);
		} else {
			// preregister non-flag args
			if(argPtr->type != ArgType::flag){
				app->add_param(argPtr->name);
				if(argPtr->letter){
					app->add_param(std::string{argPtr->letter});
				}
			}
		}
	}

	virtual ~SweetBackend() override = default;

	virtual bool isNotEnoughArgs(uint32_t positionalArgCountLowerBound) const override {
		return
	}

	virtual void printHelp(std::ostream &stream, const char * const argv0) override {
		stream << argv0 << " " << usage << std::endl;
		stream << descr << std::endl;
	}

	sweet::Options *app = nullptr;

	virtual ParseResult _parseArgs(CLIRawArgs rawArgs) override {
		sweet::Options appLocal(rawArgs.argc, const_cast<char**>(rawArgs.argv));
		app = &appLocal;

		bool show_help = false;
		appLocal.get(DEFAULT_HELP_ARG.letter_str.dashed, DEFAULT_HELP_ARG.long_name.dashed, DEFAULT_HELP_ARG.doc, show_help);
		if(show_help) {
			printHelp(streams.cout, rawArgs.argv[0]);
			return RESULT_HELP_CALLED;
		}

		CHECK_IF_ENOUGH_MAND_POS_ARGS_AND_RETURN_ERROR_OTHERWISE();

		//resetPositionalParsing();
		//getNextPosArgIdx();  // indices start from 1

		for(auto &argPtr: spec){
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
				}
			}
		}

		//std::transform(begin(remaining), end(remaining), begin(remainingPtrs), [](std::string &el) -> char * {return el.data();});
		return ParseResult{
			.parsingStatus = STATUS_OK,
			/*.argc = 0,
			.argv = remainingPtrs.data()*/
			.rest = {
				.argc = 0,
				.argv = nullptr
			}
		};
	}
};

IArgsParser * argsParserFactory(const std::string &name, const std::string &descr, const std::string &usage [[maybe_unused]], std::vector<Arg*> spec, Streams streams){
	return new SweetBackend(name, descr, usage, spec, streams);
}
