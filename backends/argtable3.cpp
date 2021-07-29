#include <cstdint>
#include <cstdlib>
#include <cassert>

#include <iostream>
#include <utility>
#include <string>
#include <vector>

#include <HydrArgs/HydrArgs.hpp>
#include <HydrArgs/fallback/parsing.hpp>
#include <HydrArgs/toolbox.hpp>

#include "num_utils.hpp"

#include <argtable3.h>

using namespace HydrArgs;

namespace HydrArgs::Backend{

	template <ArgType typeValue> struct GetArgtableHandler{};
	//template<> struct GetArgtableHandler<ArgType::flag> {constexpr static const auto handler = arg_str0;};
	template<> struct GetArgtableHandler<ArgType::string> {constexpr static const auto handler = arg_str0; using type = arg_str;};
	template<> struct GetArgtableHandler<ArgType::path> {constexpr static const auto handler = arg_file0; using type = arg_file;};
	template<> struct GetArgtableHandler<ArgType::s4> {constexpr static const auto handler = arg_int0; using type = arg_int;};
	template<> struct GetArgtableHandler<ArgType::f8> {constexpr static const auto handler = arg_dbl0; using type = arg_dbl;};

	// ToDO: arg_rex0, arg_date0

	struct ArgTable3Backend: public IBackendOwnStoredSpec{
		std::vector<std::string> remaining;
		std::vector<char *> remainingPtrs;
		std::vector<struct arg_hdr *> nativeArgs;
		struct arg_lit *show_help = nullptr;

		static constexpr const uint8_t DEFAULT_NATIVE_ARGS_COUNT = 1;  // --help

		const std::string descr;

		ArgTable3Backend(const std::string &name, const std::string &descr, const std::string &usage [[maybe_unused]], std::vector<Arg*> dashedSpec, std::vector<Arg*> positionalSpec, Streams streams): IBackendOwnStoredSpec(dashedSpec, positionalSpec, streams), descr(descr){
			arg_set_module_name(name.data());
			//arg_set_module_version(int major, int minor, int patch, const char* tag);

			show_help = arg_lit0(DEFAULT_HELP_ARG.letter_str.undashed, DEFAULT_HELP_ARG.long_name.undashed, DEFAULT_HELP_ARG.doc);
			nativeArgs.emplace_back(reinterpret_cast<arg_hdr*>(show_help));

			for(auto argPtr: dashedSpec){
				_addArg(argPtr, false);
			}
			for(auto argPtr: positionalSpec){
				_addArg(argPtr, true);
			}
		}

		virtual void _addArg(Arg *argPtr, bool isPositional) override {
			struct arg_hdr *newOpt = nullptr;
			const char *shortOpt = nullptr;
			const char *longOpt = nullptr;

			if(!isPositional){
				if(argPtr->letter){
					shortOpt = argPtr->letterStr;
				}
				longOpt = argPtr->name.data();
			}
			if(argPtr->type == ArgType::flag && !isPositional){
				newOpt = reinterpret_cast<arg_hdr*>(arg_lit0(shortOpt, longOpt, argPtr->description.data()));
			} else {
				const char *valueName = nullptr;
				if(argPtr->value_name){
					valueName = argPtr->value_name;
				} else {
					valueName = argPtr->name.data();
				}

				auto switchCaseBodyLambda = [&] <ArgType t>() {
					auto specOptPtr = static_cast<SArg<t>*>(argPtr);
					if constexpr (t == ArgType::flag){
						assert(isPositional);  // only positional flags are processed here! Non-positional are processed in the amother branch of `if`
						newOpt = reinterpret_cast<arg_hdr*>(arg_str0(shortOpt, longOpt, valueName, argPtr->description.data()));
					} else {
						newOpt = reinterpret_cast<arg_hdr*>(GetArgtableHandler<t>::handler(shortOpt, longOpt, valueName, argPtr->description.data()));
					}
				};

				switch(argPtr->type){
					case ArgType::flag:
					{
						assert(isPositional);  // only positional flags are processed here! Non-positional are processed in the amother branch of `if`
						switchCaseBodyLambda.operator()<ArgType::string>();
					}
					break;
					case ArgType::s4:
					{
						switchCaseBodyLambda.operator()<ArgType::s4>();
					}
					break;
					case ArgType::f8:
					{
						switchCaseBodyLambda.operator()<ArgType::f8>();
					}
					break;
					case ArgType::string:
					{
						switchCaseBodyLambda.operator()<ArgType::string>();
					}
					break;
					case ArgType::path:
					{
						switchCaseBodyLambda.operator()<ArgType::path>();
					}
					break;
					default:
						throw UnsupportedArgumentType{argPtr->type};
				}
			}
			if(argPtr->minCount){
				newOpt->mincount = 1;
			}
			nativeArgs.emplace_back(newOpt);
		}

		inline struct arg_end **_getEndArgPtrPtr(){
			return reinterpret_cast<struct arg_end **>(&nativeArgs.back());
		}

		inline struct arg_end *_getEndArgPtr(){
			return *_getEndArgPtrPtr();
		}

		virtual void _seal() override {
			nativeArgs.emplace_back(reinterpret_cast<arg_hdr*>(arg_end(nativeArgs.size())));
		}

		virtual void _unseal() override {
			auto resPtr = _getEndArgPtrPtr();
			arg_freetable(reinterpret_cast<void**>(resPtr), 1);
			nativeArgs.pop_back();
		}

		virtual bool isSealed() const override {
			return nativeArgs.size() && nativeArgs.back()->flag == ARG_TERMINATOR;
		}

		virtual ~ArgTable3Backend() override {
			arg_freetable(reinterpret_cast<void **>(nativeArgs.data()), nativeArgs.size());
		}

		void printHelpMessage(const char *const argv0, void **argtable, std::ostream &s){
			s << argv0;
			auto helpStr = arg_dstr_create();
			arg_print_syntax_ds(helpStr, argtable, "\n");
			arg_print_glossary_gnu_ds(helpStr, argtable);
			s << arg_dstr_cstr(helpStr);
			arg_dstr_free(helpStr);
			s << descr << std::endl;
		}

		int printErrorMessage(const char *argv0, void **argtable, int nerrors, struct arg_end *end, std::ostream &s){
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

			int nerrors = -1;

			{
				auto tempArgvVec = rawArgs.getArgvVector();
				nerrors = arg_parse(checked_cast<int>(tempArgvVec.size() - 1), const_cast<char**>(tempArgvVec.data()), argtable);
			}

			PROCESS_CASE_OF_HELP_CALLED(show_help->count);

			if(nerrors > 0){
				auto end = _getEndArgPtr();
				for (int i = 0; i < end->count; i++) {
					auto error = end->error[i];
					if(error == ARG_ENOMATCH || error == ARG_ELONGOPT){
						remainingPtrs.emplace_back(const_cast<char *>(end->argval[i]));
					} else {
						auto res = printErrorMessage(rawArgs.argv0, argtable, nerrors, end, streams.cerr);
						return {
							.parsingStatus = {
								.returnCode = res,
								.events = EVENTS_SYNTAX_ERROR
							},
							.rest = NO_ARGS
						};
					}
				}
				remainingPtrs.shrink_to_fit();
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
							//switchCaseBodyLambda.operator()<ArgType::flag>();
							if(isPositional){
								auto specOptPtr = static_cast<SArg<ArgType::string>*>(argPtr);
								auto nativeSpecArg = reinterpret_cast<arg_str*>(nativeArg);
								if(nativeSpecArg->count){
									PARSE_TYPED_ARG_FROM_STRING(HydrArgs::fallback::parsing::parseBoolValue, nativeSpecArg->sval[0], rawArgs.argv0);
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
							auto nativeSpecArg = reinterpret_cast<GetArgtableHandler<ArgType::s4>::type*>(nativeArg);
							if(nativeSpecArg->count){
								specOptPtr->value = nativeSpecArg->ival[0];
							}
						}
						break;
						case ArgType::f8:
						{
							auto specOptPtr = static_cast<SArg<ArgType::f8>*>(argPtr);
							auto nativeSpecArg = reinterpret_cast<GetArgtableHandler<ArgType::f8>::type*>(nativeArg);
							if(nativeSpecArg->count){
								specOptPtr->value = nativeSpecArg->dval[0];
							}
						}
						break;
						case ArgType::string:
						{
							auto specOptPtr = static_cast<SArg<ArgType::string>*>(argPtr);
							auto nativeSpecArg = reinterpret_cast<GetArgtableHandler<ArgType::string>::type*>(nativeArg);
							if(nativeSpecArg->count){
								specOptPtr->value = nativeSpecArg->sval[0];
							}
						}
						break;
						case ArgType::path:
						{
							auto specOptPtr = static_cast<PathArg*>(argPtr);
							auto nativeSpecArg = reinterpret_cast<GetArgtableHandler<ArgType::path>::type*>(nativeArg);
							if(nativeSpecArg->count){
								specOptPtr->value = nativeSpecArg->filename[0];
							}
						}
						break;
						default:
							throw UnsupportedArgumentType{argPtr->type};
					}
					++i;
				}
			}

			return {
				.parsingStatus = STATUS_OK,
				.rest = {
					.argv0 = rawArgs.argv0,
					.argvRest = {const_cast<const char **>(remainingPtrs.data()), remainingPtrs.size()}
				}
			};
		}
	};

	IArgsParser * argsParserFactory(const std::string &name, const std::string &descr, const std::string &usage [[maybe_unused]], const std::vector<Arg *> &dashedSpec, const std::vector<Arg *> &positionalSpec, Streams streams){
		return new ArgTable3Backend(name, descr, usage, dashedSpec, positionalSpec, streams);
	}
};
