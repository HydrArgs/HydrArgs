#include <iostream>
#include <utility>
#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

#include <HydrArgs/HydrArgs.hpp>
#include <HydrArgs/fallback/parsing.hpp>
#include <HydrArgs/toolbox.hpp>

#include "num_utils.hpp"

#include <clipp.h>

using namespace HydrArgs;

union our_bool{
	bool value;
};

template<> struct clipp::detail::make<our_bool> {
	static inline our_bool from(const char *s) {
		our_bool res;
		HydrArgs::fallback::parsing::_parseBoolValue(s, res.value);
		return res;
	}
};

namespace HydrArgs::Backend{

	struct CLIPPBackend: public IArgsParser{
		clipp::group app;
		std::vector<std::string> remaining;
		std::vector<char *> remainingPtrs;
		std::string name;
		std::string descr;

		bool show_help = false;

		CLIPPBackend(std::string name, std::string descr, const std::string &usage [[maybe_unused]], const std::vector<Arg*> &dashedSpec, const std::vector<Arg*> &positionalSpec, Streams streams): IArgsParser(streams), name(std::move(name)), descr(std::move(descr)){
			app = (app, clipp::option(DEFAULT_HELP_ARG.long_name.dashed).set(show_help).doc(DEFAULT_HELP_ARG.doc).required(false));// Other ways to write this (anything combined to `app` using | ) cause sigsegvs. IDK why. Maybe a bug in the lib. Working around by adding --help as a sequential optional flag and then checking for it before checking for validity of CLI args.

			for(auto argPtr: dashedSpec){
				_addArg(argPtr, false);
			}
			for(auto argPtr: positionalSpec){
				_addArg(argPtr, true);
			}
		}

		void _addArg(Arg *argPtr, bool isPositional) override {
			if(isPositional){
				auto switchCaseBodyLambda = [&] <ArgType t>() {
					auto specOptPtr = static_cast<SArg<t>*>(argPtr);
					if constexpr(t == ArgType::flag){
						auto v = clipp::value(argPtr->name, reinterpret_cast<our_bool&>(specOptPtr->getValueRef())).doc(argPtr->description).required(argPtr->minCount);
						app = (app, v);
					} else {
						app = (app, clipp::value(argPtr->name, specOptPtr->getValueRef()).doc(argPtr->description).required(argPtr->minCount));
					}
				};
				switch(argPtr->type){
					case ArgType::flag:
					{
						switchCaseBodyLambda.operator()<ArgType::flag>();
					}
					break;
					case ArgType::u1:
					{
						switchCaseBodyLambda.operator()<ArgType::u1>();
					}
					break;
					case ArgType::s2:
					{
						switchCaseBodyLambda.operator()<ArgType::s2>();
					}
					break;
					case ArgType::u2:
					{
						switchCaseBodyLambda.operator()<ArgType::u2>();
					}
					break;
					case ArgType::s4:
					{
						switchCaseBodyLambda.operator()<ArgType::s4>();
					}
					break;
					case ArgType::u4:
					{
						switchCaseBodyLambda.operator()<ArgType::u4>();
					}
					break;
					case ArgType::s8:
					{
						switchCaseBodyLambda.operator()<ArgType::s8>();
					}
					break;
					case ArgType::u8:
					{
						switchCaseBodyLambda.operator()<ArgType::u8>();
					}
					break;
					case ArgType::f4:
					{
						switchCaseBodyLambda.operator()<ArgType::f4>();
					}
					break;
					case ArgType::f8:
					{
						switchCaseBodyLambda.operator()<ArgType::f8>();
					}
					break;
					case ArgType::string:
					case ArgType::path:
					{
						switchCaseBodyLambda.operator()<ArgType::string>();
					}
					break;
					default:
						throw UnsupportedArgumentType{argPtr->type};
				}
			} else {
				std::string name = "--" + argPtr->name;
				std::string valueName = (argPtr->value_name ? argPtr->value_name : argPtr->name);

				auto switchCaseBodyLambda = [&] <ArgType t>() {
					auto specOptPtr = static_cast<SArg<t>*>(argPtr);
					if constexpr(t == ArgType::flag){
						app = (app, clipp::option(name).doc(argPtr->description).set(specOptPtr->getValueRef(), true).required(argPtr->minCount));
					} else {
						app = (app, clipp::option(name).doc(argPtr->description).required(argPtr->minCount) & clipp::value(valueName, specOptPtr->getValueRef()));
					}
				};

				switch(argPtr->type){
					case ArgType::flag:
					{
						switchCaseBodyLambda.operator()<ArgType::flag>();
					}
					break;
					case ArgType::u1:
					{
						switchCaseBodyLambda.operator()<ArgType::u1>();
					}
					break;
					case ArgType::s2:
					{
						switchCaseBodyLambda.operator()<ArgType::s2>();
					}
					break;
					case ArgType::u2:
					{
						switchCaseBodyLambda.operator()<ArgType::u2>();
					}
					break;
					case ArgType::s4:
					{
						switchCaseBodyLambda.operator()<ArgType::s4>();
					}
					break;
					case ArgType::u4:
					{
						switchCaseBodyLambda.operator()<ArgType::u4>();
					}
					break;
					case ArgType::s8:
					{
						switchCaseBodyLambda.operator()<ArgType::s8>();
					}
					break;
					case ArgType::u8:
					{
						switchCaseBodyLambda.operator()<ArgType::u8>();
					}
					break;
					case ArgType::f4:
					{
						switchCaseBodyLambda.operator()<ArgType::f4>();
					}
					break;
					case ArgType::f8:
					{
						switchCaseBodyLambda.operator()<ArgType::f8>();
					}
					break;
					case ArgType::string:
					case ArgType::path:
					{
						switchCaseBodyLambda.operator()<ArgType::string>();
					}
					break;
					default:
						throw UnsupportedArgumentType{argPtr->type};
				}
			}
		}
		~CLIPPBackend() override = default;

		void printHelp(std::ostream &stream, const char * const argv0) override {
			stream << clipp::make_man_page(app, argv0).prepend_section(name, descr);
		}

		ParseResult _parseArgs(CLIRawArgs rawArgs) override {
			app = (app, clipp::any_other(remaining));
			decltype(clipp::parse(0, static_cast<char **>(nullptr), app)) parseRes;
			{
				auto tempArgvVec = rawArgs.getArgvVector();
				parseRes = clipp::parse(checked_cast<int>(tempArgvVec.size() - 1), const_cast<char **>(tempArgvVec.data()), app);
			}

			PROCESS_CASE_OF_HELP_CALLED(show_help);  // even if incorrect, if --help is mathed, then it is OK. Other ways to write this cause sigsegvs.

			if(!parseRes){
				PROCESS_CASE_OF_SYNTAX_ERROR_MESSAGE("Error in CLI, but clipp CLI parsing backend cannot explain it");
			}


			auto remainingPtrsSize = remaining.size();
			remainingPtrs.reserve(remainingPtrsSize);
			remainingPtrs.resize(remainingPtrsSize);

			std::transform(begin(remaining), end(remaining), begin(remainingPtrs), [](std::string &el) -> char * {return el.data();});
			return {
				.parsingStatus = STATUS_OK,
				.rest = {
					.argv0 = rawArgs.argv0,
					.argvRest = {const_cast<const char **>(remainingPtrs.data()), remainingPtrsSize}
				}
			};
		}
	};

	IArgsParser * argsParserFactory(const std::string &name, const std::string &descr, const std::string &usage [[maybe_unused]], const std::vector<Arg *> &dashedSpec, const std::vector<Arg *> &positionalSpec, Streams streams) {
		return new CLIPPBackend(name, descr, usage, dashedSpec, positionalSpec, streams);
	}
};
