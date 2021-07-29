#include <cstdint>
#include <ostream>
#include <string>
#include <vector>

#include <HydrArgs/HydrArgs.hpp>
#include <HydrArgs/toolbox.hpp>
#include <HydrArgs/fallback/parsing.hpp>
#include <HydrArgs/fallback/positional/iterative.hpp>

#include "num_utils.hpp"

#include <dlib/cmd_line_parser.h>

using namespace HydrArgs;

struct DLibBackend: public IBackendOwnStoredSpec, public HydrArgs::fallback::positional::PositionalParserIterative{
	std::string name;
	std::string usage;
	std::vector<std::string> remaining;
	std::vector<char *> remainingPtrs;
	dlib::command_line_parser app;

	uint32_t dashedCounter = 0;

	DLibBackend(const std::string &name, const std::string &descr, const std::string &usage, std::vector<Arg*> dashedSpec, std::vector<Arg*> positionalSpec, Streams streams): IBackendOwnStoredSpec(dashedSpec, positionalSpec, streams), HydrArgs::fallback::positional::PositionalParserIterative(this), name(name), usage(usage){
		app.add_option(DEFAULT_HELP_ARG.long_name.undashed, DEFAULT_HELP_ARG.doc);
		app.add_option(DEFAULT_HELP_ARG.letter_str.undashed, DEFAULT_HELP_ARG.doc);

		for(auto argPtr: dashedSpec){
			_addArg(argPtr, false);
		}
		for(auto argPtr: positionalSpec){
			_addArg(argPtr, true);
		}
	}

	virtual void _addArg(Arg *argPtr, bool isPositional) override {
		if(isPositional){
			addPosArg(argPtr);
		} else {
			app.add_option(argPtr->name, argPtr->description, argPtr->type != ArgType::flag);
			if(argPtr->letter){
				app.add_option(argPtr->letterStr, argPtr->description, argPtr->type != ArgType::flag);
			}
		}
	}

	virtual ~DLibBackend() override = default;
	virtual ParseResult _parseArgs(CLIRawArgs rawArgs) override {
		// Here QCoreApplication must be already created

		{
			auto tempArgvVec = rawArgs.getArgvVector();
			app.parse(checked_cast<int>(tempArgvVec.size()), tempArgvVec.data());
		}
		//parser.check_one_time_options(nullptr);
		//parser.check_incompatible_options("c", "d");
		//app.

		/*if(!parseStatus){
			PROCESS_CASE_OF_SYNTAX_ERROR_MESSAGE(parser.errorText().toStdString());
		}*/

		PROCESS_CASE_OF_HELP_CALLED(app.option("h") || app.option("help"));
		HYDRARGS_CHECK_IF_ENOUGH_MAND_POS_ARGS_AND_RETURN_ERROR_OTHERWISE();

		resetPositionalParsing();
		dashedCounter = 0;

		for(auto argPtr: dashedSpec){
			//auto opt = app.option(argPtr->name);

			auto switchCaseBodyLambda = [&] <ArgType t>() {
				auto specOptPtr = static_cast<SArg<t>*>(argPtr);
				specOptPtr->value = app.option(argPtr->name);
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

		uint8_t i = 0;
		for(auto argPtr: positionalSpec){
			PARSE_ARG_FROM_STRING(app[i], rawArgs.argv0);
		}

		//std::transform(begin(remaining), end(remaining), begin(remainingPtrs), [](std::string &el) -> char * {return el.data();});
		return {
			.parsingStatus = STATUS_OK,
			.rest = {
				.argv0=rawArgs.argv0,
				.argvRest = {static_cast<const char **>(nullptr), 0}
			}
		};
	}

	virtual bool isNotEnoughArgs(uint32_t positionalArgCountLowerBound) const override {
		return app.number_of_arguments() < positionalArgCountLowerBound;
	}

	virtual void printHelp(std::ostream &stream, const char *const argv0 [[maybe_unused]]) override {
		app.print_options(stream);
		stream << std::endl;
	}
};

IArgsParser * argsParserFactory(const std::string &name, const std::string &descr, const std::string &usage [[maybe_unused]], const std::vector<Arg *> &dashedSpec, const std::vector<Arg *> &positionalSpec, Streams streams){
	return new DLibBackend(name, descr, usage, dashedSpec, positionalSpec, streams);
}
