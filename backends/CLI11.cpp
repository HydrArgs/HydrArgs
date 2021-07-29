#include <algorithm>
#include <iterator>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <HydrArgs/HydrArgs.hpp>

#include "num_utils.hpp"

#include <CLI/CLI.hpp>


using namespace HydrArgs;

namespace HydrArgs::Backend{

	struct HYDRARGS_BACKEND_API CLI11Backend: public IArgsParser{
		CLI::App app;
		std::vector<std::string> remaining;
		std::vector<char *> remainingPtrs;

		CLI11Backend(const std::string &name, const std::string &descr, const std::string &usage [[maybe_unused]], const std::vector<Arg*> &dashedSpec, const std::vector<Arg*> &positionalSpec, Streams streams): IArgsParser(streams), app(name, descr){
			app.allow_windows_style_options();
			app.positionals_at_end(false);
			for(const auto &argPtr: dashedSpec){
				_addArg(argPtr, false);
			}
			for(const auto &argPtr: positionalSpec){
				_addArg(argPtr, true);
			}
		}

		void _addArg(Arg *argPtr, bool isPositional) override {
			std::string argName = argPtr->name;
			if(!isPositional){
				argName = "--" + argName;
				if(argPtr->letter) {
					argName = std::string{'-'} + argPtr->letter + "," + argName;
				}
			}
			CLI::Option *nativeArg = nullptr;

			auto switchCaseBodyLambda = [&] <ArgType t>() {
				auto specOptPtr = static_cast<SArg<t>*>(argPtr);
				if constexpr (t == ArgType::flag){
					if(isPositional){
						nativeArg = app.add_option<typename GetUnderlyingTypeByEnumValue<t>::type>(argName, specOptPtr->getValueRef(), argPtr->description);//->default_val(specOptPtr->value);
					} else {
						nativeArg = app.add_flag(argName, specOptPtr->getValueRef(), specOptPtr->description);//->default_val(specOptPtr->value);
					}
				} else {
					nativeArg = app.add_option<typename GetUnderlyingTypeByEnumValue<t>::type>(argName, specOptPtr->getValueRef(), argPtr->description);//->default_val(specOptPtr->value);
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
			nativeArg->required(argPtr->minCount);
		}
		~CLI11Backend() override = default;

		void printHelp(std::ostream &stream, const char * const argv0 [[maybe_unused]]) override {
			_printHelp(stream, argv0, CLI::AppFormatMode::Normal);
		}

		void _printHelp(std::ostream &stream, const char * const argv0 [[maybe_unused]], CLI::AppFormatMode mode) const {
			stream << app.help("", mode);
		}

		ParseResult _parseArgs(CLIRawArgs rawArgs) override {
			app.allow_extras();
			try {
				auto tempArgvVec = rawArgs.getArgvVector();
				app.parse(checked_cast<int>(tempArgvVec.size() - 1), tempArgvVec.data());
			} catch(const CLI::CallForHelp &e) {
				printHelp(streams.cout, rawArgs.argv0);
				return {
					.parsingStatus = {
						e.get_exit_code(), EVENTS_HELP_CALLED
					},
					.rest = NO_ARGS
				};
			} catch(const CLI::CallForAllHelp &e) {
				_printHelp(streams.cout, rawArgs.argv0, CLI::AppFormatMode::All);
				return {
					.parsingStatus = {
						e.get_exit_code(), EVENTS_HELP_CALLED
					},
					.rest = NO_ARGS
				};
			} catch(const CLI::ParseError &e) {
				streams.cerr << e.what() << std::endl;
				printHelp(streams.cerr, rawArgs.argv0);
				return {
					.parsingStatus = {
						e.get_exit_code(), EVENTS_SYNTAX_ERROR
					},
					.rest = NO_ARGS
				};
			}

			remaining = app.remaining();
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
		return new CLI11Backend(name, descr, usage, dashedSpec, positionalSpec, streams);
	}
};
