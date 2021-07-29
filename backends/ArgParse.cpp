#include <type_traits>
#include <algorithm>
#include <any>
#include <iosfwd>
#include <iterator>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include <HydrArgs/HydrArgs.hpp>
#include <HydrArgs/fallback/errors.hpp>
#include <HydrArgs/fallback/parsing.hpp>
#include <HydrArgs/toolbox.hpp>

#include "num_utils.hpp"

#include <argparse/argparse.hpp>


using namespace HydrArgs;

namespace HydrArgs::Backend{
	struct ArgparseBackend: public IBackendOwnStoredSpec{
		argparse::ArgumentParser app;
		std::vector<std::string> remaining;
		std::vector<char*> remainingPtrs;
		std::vector<argparse::Argument*> nativeArgs;

		argparse::Argument *helpArg = nullptr, *versionArg = nullptr; ///< as recommended in https://github.com/p-ranav/argparse/issues/113 , we redefine them
		argparse::Argument *ourVersionArg = nullptr;
		argparse::Argument *remainingArg = nullptr;

		ArgparseBackend(const std::string& name, const std::string& descr, const std::string& usage [[maybe_unused]], std::vector<Arg*> dashedSpec, std::vector<Arg*> positionalSpec, Streams streams): IBackendOwnStoredSpec(dashedSpec, positionalSpec, streams), app(name){
			app.add_description(descr);

			helpArg = &app.add_argument("--help", "-h").default_value(false).implicit_value(true).help(DEFAULT_HELP_ARG.doc);  // Don't redefine the flags names, they are to overcome Argparse own default flags and must match them exactly
			versionArg = &app.add_argument("--version", "-v").default_value(false).implicit_value(true);
			ourVersionArg = &app.add_argument(DEFAULT_HELP_ARG.long_name.dashed, DEFAULT_HELP_ARG.letter_str.dashed).default_value(false).implicit_value(true).help(DEFAULT_HELP_ARG.doc);  // Our version arg, it is not necessarily the same as `helpArg`, so we process them both identically
			remainingArg = &app.add_argument("remaining").remaining();

			for(auto &argPtr: dashedSpec){
				_addArg(argPtr, false);
			}
			for(auto &argPtr: positionalSpec){
				_addArg(argPtr, true);
			}
		}

		argparse::Argument& constructArgument(Arg* argPtr, bool isPositional){
			if(isPositional){
				if(!argPtr->letter){
					return app.add_argument(argPtr->name);
				} else {
					//return app.add_argument(argPtr->name, std::string{argPtr->letter});
					return app.add_argument(argPtr->name);
				}
			} else {
				std::string argName{"--" + argPtr->name};
				if(!argPtr->letter){
					return app.add_argument(argName);
				} else {
					return app.add_argument(argName, std::string{'-', argPtr->letter});
				}
			}
		}

		virtual void _addArg(Arg* argPtr, bool isPositional) override {
			auto &arg = constructArgument(argPtr, isPositional).help(argPtr->description);
			auto switchCaseBodyLambda = [&] <ArgType t>() {
				auto specOptPtr = static_cast<SArg<t>*>(argPtr);
				if constexpr (t == ArgType::flag){
					auto na = arg.default_value(specOptPtr->value);

					if(!isPositional){  // yet another bug
						na.implicit_value(!specOptPtr->value);
					}
				} else {
					if(!argPtr->minCount){
						arg.default_value(specOptPtr->value);
					}
					if constexpr (std::is_integral<typename GetUnderlyingTypeByEnumValue<t>::type>::value){
						arg.scan<'d', decltype(specOptPtr->value)>();
					}
				}
			};
			switch(argPtr->type){
				case ArgType::flag:
				{
					switchCaseBodyLambda.operator()<ArgType::flag>();
				}
				break;
				case ArgType::s1:
				{
					switchCaseBodyLambda.operator()<ArgType::s1>();
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
			if(argPtr->type != ArgType::flag && argPtr->minCount){
				arg.required();
			}
		}
		virtual ~ArgparseBackend() override = default;

		virtual void printHelp(std::ostream &stream, const char * const argv0 [[maybe_unused]]) override {
			stream << app;
		}

		/* IMPORTANT!!!! This library DOESN'T CONVERT INT TO OTHER TYPES. ALL VALUES MUST BE STRINGS */

		bool hasHelpBeenRequested(){
			return app.get<bool>("--help") || app.get<bool>(DEFAULT_HELP_ARG.letter_str.dashed);
		}

		virtual ParseResult _parseArgs(CLIRawArgs rawArgs) override {
			try {
				auto tempArgvVec = rawArgs.getArgvVector();
				app.parse_args(checked_cast<int>(tempArgvVec.size() - 1), tempArgvVec.data());
			} catch (const std::runtime_error& err) {
				PROCESS_CASE_OF_HELP_CALLED(hasHelpBeenRequested());
				PROCESS_CASE_OF_SYNTAX_ERROR_WHATABLE_EXCEPTION(err);
			}

			PROCESS_CASE_OF_HELP_CALLED(hasHelpBeenRequested());

			std::pair<decltype(dashedSpec)&, bool> specs[]{
				{dashedSpec, false},
				{positionalSpec, true},
			};
			for(auto p: specs){
				auto spec = p.first;
				auto isPositional = p.second;
				for(auto argPtr: spec){
					std::string argName;
					if(isPositional){
						argName = argPtr->name;
					} else {
						argName = "--" + argPtr->name;
					}
					try {
						auto switchCaseBodyLambda = [&] <ArgType t>() {
							auto specOptPtr = static_cast<SArg<t>*>(argPtr);
							specOptPtr->value = app.get<decltype(specOptPtr->value)>(argName);
						};
						switch(argPtr->type){
							case ArgType::s1:
							{
								switchCaseBodyLambda.operator()<ArgType::s1>();
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
							{
								if(argPtr->type == ArgType::flag && !isPositional){
									auto specOptPtr = static_cast<SArg<ArgType::flag>*>(argPtr);
									specOptPtr->value = app.is_used(argName);
									if(!specOptPtr->value){
										PROCESS_ARGUMENT_MISSING_CASE();
									}
								} else {
									auto resStr = app.get<std::string>(argName);
									PARSE_ARG_FROM_STRING(resStr, rawArgs.argv0);
								}
							}
							break;
						}
					} catch(std::bad_any_cast &e) {
						PROCESS_ARGUMENT_MISSING_CASE();
					}
				}
			}

			try {
				remaining = app.get<std::vector<std::string>>("remaining");
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
			} catch (std::logic_error& e) {
				return {
					.parsingStatus = STATUS_OK,
					.rest = {
						.argv0 = rawArgs.argv0,
						.argvRest = NO_REST_ARGS
					}
				};
			}
		}
	};

	IArgsParser* argsParserFactory(const std::string &name, const std::string &descr, const std::string &usage [[maybe_unused]], const std::vector<Arg *> &dashedSpec, const std::vector<Arg *> &positionalSpec, Streams streams){
		return new ArgparseBackend(name, descr, usage, dashedSpec, positionalSpec, streams);
	}
};
