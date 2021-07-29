#include <algorithm>
#include <iterator>
#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

#include <HydrArgs/HydrArgs.hpp>
#include <HydrArgs/toolbox.hpp>

#include <lyra/lyra.hpp>

using namespace HydrArgs;


namespace HydrArgs::Backend{
	struct LyraBackend: public IArgsParser{
		lyra::cli app;
		std::vector<std::string> remaining;
		std::vector<char *> remainingPtrs;
		bool show_help = false;

		LyraBackend(const std::string &name, const std::string &descr, const std::string &usage [[maybe_unused]], const std::vector<Arg*> &dashedSpec, const std::vector<Arg*> &positionalSpec, Streams streams): IArgsParser(streams){
			lyra::help helpArg(show_help);
			helpArg.description(descr);
			helpArg.name(name);
			app |= helpArg;

			for(auto argPtr: dashedSpec) {
				_addArg(argPtr, false);
			}
			for(auto argPtr: positionalSpec) {
				_addArg(argPtr, true);
			}
		}

		void _addArg(Arg *argPtr, bool isPositional) override {
			std::string valueName;
			if(argPtr->value_name) {
				valueName = argPtr->value_name;
			}

			auto switchCaseBodyLambda = [&] <ArgType t>() {
				auto specOptPtr = static_cast<SArg<t>*>(argPtr);
				if(isPositional){
					lyra::arg param(specOptPtr->getValueRef(), valueName);
					param = param(specOptPtr->description);
					if(specOptPtr->minCount){
						param = param.required();
					}
					app |= param;
				} else {
					auto getParam = [&]{
						if constexpr (t == ArgType::flag){
							return lyra::opt(specOptPtr->getValueRef());
						} else {
							return lyra::opt(specOptPtr->getValueRef(), valueName);
						}
					};
					auto param = getParam();

					param = param["--" + specOptPtr->name];
					if(specOptPtr->letter){
						param = param[std::string{"-"} + specOptPtr->letter];
					}
					param = param(specOptPtr->description);
					if(specOptPtr->minCount){
						param = param.required();
					}
					app |= param;
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
		}
		~LyraBackend() override = default;

		void printHelp(std::ostream &stream, const char *const argv0 [[maybe_unused]]) override {
			stream << app << std::endl;
		}

		ParseResult _parseArgs(CLIRawArgs rawArgs) override {
			lyra::arg const restArg(remaining, "");
			auto appWithRest = app | restArg;
			std::optional<lyra::parse_result> resultOpt;
			{
				auto tempArgvVec = rawArgs.getArgvVector();
				resultOpt = appWithRest.parse({static_cast<int>(tempArgvVec.size() - 1), tempArgvVec.data()});
			}
			lyra::parse_result const result = resultOpt.value();

			PROCESS_CASE_OF_HELP_CALLED(show_help);

			if(!result){
				PROCESS_CASE_OF_SYNTAX_ERROR_MESSAGE(result.message());
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
		return new LyraBackend(name, descr, usage, dashedSpec, positionalSpec, streams);
	}
};
