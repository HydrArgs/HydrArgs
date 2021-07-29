#include <memory>

#include <utility>
#include <iostream>
#include <cstring>
#include <cassert>


#include <HydrArgs/HydrArgs.hpp>
#include <HydrArgs/toolbox.hpp>
#include <HydrArgs/fallback/parsing.hpp>

#include <klib/ketopt.h>


struct KetoptBackend: public IBackendOwnStoredSpec{
	std::vector<std::string> remaining;
	std::vector<char *> remainingPtrs;
	std::vector<ko_longopt_t> nativeArgs;
	ko_longopt_t show_help;

	enum {
		DEFAULT_NATIVE_ARGS_COUNT = 1 // --help
	};

	const std::string descr;

	KetoptBackend(const std::string& name, const std::string& descr, const std::string& usage [[maybe_unused]], std::vector<Arg*> dashedSpec, std::vector<Arg*> positionalSpec, Streams streams): IBackendOwnStoredSpec(dashedSpec, positionalSpec, streams), descr(descr){
		//arg_set_module_version(int major, int minor, int patch, const char* tag);

		show_help = ko_longopt_t{
			.name="help",
			.val=0,

		};
		nativeArgs.emplace_back(reinterpret_cast<ko_longopt_t>(show_help));

		for(auto argPtr: dashedSpec){
			_addArg(argPtr, false);
		}
		for(auto argPtr: positionalSpec){
			_addArg(argPtr, true);
		}
	}

	virtual void _addArg(Arg* argPtr, bool isPositional) override {
		ko_longopt_t newOpt = nullptr;
		const char * shortOpt = nullptr;
		const char * longOpt = nullptr;

		if(!isPositional){
			if(argPtr->letter){
				shortOpt = argPtr->letterStr;
			}
			longOpt = argPtr->name.data();
		}
		if(argPtr->type == ArgType::flag && !isPositional){
			newOpt = reinterpret_cast<ko_longopt_t>(arg_lit0(shortOpt, longOpt, argPtr->description.data()));
		} else{
			const char * valueName = nullptr;
			if(argPtr->value_name){
				valueName = argPtr->value_name;
			} else {
				valueName = argPtr->name.data();
			}

			switch(argPtr->type){
				case ArgType::flag:
				{
					assert(isPositional);  // only positional flags are processed here! Non-positional are processed in the amother branch of `if`
					newOpt = reinterpret_cast<ko_longopt_t>(arg_str0(shortOpt, longOpt, valueName, argPtr->description.data()));
				}
				break;
				case ArgType::s4:
				{
					newOpt = reinterpret_cast<ko_longopt_t>(arg_int0(shortOpt, longOpt, valueName, argPtr->description.data()));
				}
				break;
				case ArgType::string:
				{
					newOpt = reinterpret_cast<ko_longopt_t>(arg_str0(shortOpt, longOpt, valueName, argPtr->description.data()));
				}
				break;
				case ArgType::path:
				{
					newOpt = reinterpret_cast<ko_longopt_t>(arg_file0(shortOpt, longOpt, valueName, argPtr->description.data()));
				}
				break;
			}
		}
		if(argPtr->minCount){
			newOpt->mincount = 1;
		}
		nativeArgs.emplace_back(newOpt);
	}

	inline struct arg_end ** _getEndArgPtrPtr(){
		return reinterpret_cast<struct arg_end **>(&nativeArgs.back());
	}

	inline struct arg_end * _getEndArgPtr(){
		return *_getEndArgPtrPtr();
	}

	virtual void _seal() override {
		nativeArgs.emplace_back(reinterpret_cast<ko_longopt_t>(arg_end(nativeArgs.size())));
	}

	virtual void _unseal() override {
		auto resPtr = _getEndArgPtrPtr();
		arg_freetable(reinterpret_cast<void**>(resPtr), 1);
		nativeArgs.pop_back();
	}

	virtual bool isSealed() const override {
		return nativeArgs.size() && nativeArgs.back()->flag == ARG_TERMINATOR;
	}

	virtual ~KetoptBackend() override{
		arg_freetable(reinterpret_cast<void **>(nativeArgs.data()), nativeArgs.size());
	}

	void printHelpMessage(const char * const argv0, void** argtable, std::ostream &s){
		s << argv0;
		auto helpStr = arg_dstr_create();
		arg_print_syntax_ds(helpStr, argtable, "\n");
		arg_print_glossary_gnu_ds(helpStr, argtable);
		s << arg_dstr_cstr(helpStr);
		arg_dstr_free(helpStr);
		s << descr << std::endl;
	}

	int printErrorMessage(const char* argv0, void** argtable, int nerrors, struct arg_end* end, std::ostream &s){
		auto helpStr = arg_dstr_create();
		int exitCode = EXIT_FAILURE;
		arg_print_errors_ds(helpStr, end, argv0);
		s << arg_dstr_cstr(helpStr);
		arg_dstr_free(helpStr);
		return exitCode;
	}

	virtual void printHelp(std::ostream &stream, const char * const argv0) override {
		auto argtable = reinterpret_cast<void **>(nativeArgs.data());
		printHelpMessage(argv0, argtable, stream);
		stream << std::endl;
	}

	virtual ParseResult _parseArgs(CLIRawArgs rawArgs) override {
		auto argtable = reinterpret_cast<void **>(nativeArgs.data());
		{
			auto tempArgvVec = rawArgs.getArgvVector();
			int nerrors = arg_parse(tempArgvVec.size(), const_cast<char**>(tempArgvVec.data()), argtable);
		}

		PROCESS_CASE_OF_HELP_CALLED(show_help->count);

		if (nerrors > 0)
		{
			auto res = printErrorMessage(rawArgs.argv[0], argtable, nerrors, _getEndArgPtr(), streams.cerr);
			return {
				.parsingStatus = {
					.returnCode = res,
					.events = EVENTS_SYNTAX_ERROR
				},
				.rest = NO_ARGS
			};
		}

		uint32_t i = DEFAULT_NATIVE_ARGS_COUNT;
		std::pair<decltype(dashedSpec)&, bool> specs[]{
			{dashedSpec, false},
			{positionalSpec, true},
		};
		for(auto p: specs){
			auto spec = p.first;
			auto isPositional = p.second;

			for(auto argPtr: spec){
				auto &nativeArg = nativeArgs[i];
				switch(argPtr->type){
					case ArgType::flag:
					{
						if(isPositional){
							auto specOptPtr = static_cast<SArg<ArgType::string>*>(argPtr);
							auto nativeSpecArg = reinterpret_cast<arg_str*>(nativeArg);
							if(nativeSpecArg->count){
								PARSE_TYPED_ARG_FROM_STRING(parseBoolValue, nativeSpecArg->sval[0], rawArgs.argv[0]);
							}

						} else {
							auto specOptPtr = static_cast<SArg<ArgType::flag>*>(argPtr);
							specOptPtr->value = reinterpret_cast<arg_lit*>(nativeArg)->count > 0;
						}
					}
					break;
					case ArgType::s4:
					{
						auto specOptPtr = static_cast<SArg<ArgType::s4>*>(argPtr);
						auto nativeSpecArg = reinterpret_cast<arg_int*>(nativeArg);
						if(nativeSpecArg->count){
							specOptPtr->value = nativeSpecArg->ival[0];
						}
					}
					break;
					case ArgType::string:
					{
						auto specOptPtr = static_cast<SArg<ArgType::string>*>(argPtr);
						auto nativeSpecArg = reinterpret_cast<arg_str*>(nativeArg);
						if(nativeSpecArg->count){
							specOptPtr->value = nativeSpecArg->sval[0];
						}
					}
					break;
					case ArgType::path:
					{
						auto specOptPtr = static_cast<PathArg*>(argPtr);
						auto nativeSpecArg = reinterpret_cast<arg_file*>(nativeArg);
						if(nativeSpecArg->count){
							specOptPtr->value = nativeSpecArg->filename[0];
						}
					}
					break;
				}
				++i;
			}
		}

		//std::transform(begin(remaining), end(remaining), begin(remainingPtrs), [](std::string &el) -> char * {return el.data();});
		return {
			.parsingStatus = STATUS_OK,
			.rest={
				.argv0=rawArgs.argv0,
				.argvRest = {static_cast<const char **>(nullptr), 0}
			}
		};
	}
};

IArgsParser* argsParserFactory(const std::string& name, const std::string& descr, const std::string& usage [[maybe_unused]], std::vector<Arg*> dashedSpec, std::vector<Arg*> positionalSpec, Streams streams){
	return new KetoptBackend(name, descr, usage, dashedSpec, positionalSpec, streams);
}


