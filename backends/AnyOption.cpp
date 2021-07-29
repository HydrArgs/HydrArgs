#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include <HydrArgs/HydrArgs.hpp>
#include <HydrArgs/result.hpp>
#include <HydrArgs/toolbox.hpp>

#include <HydrArgs/fallback/errors.hpp>
#include <HydrArgs/fallback/parsing.hpp>
#include <HydrArgs/fallback/positional/IPositionalParser.hpp>
#include <HydrArgs/fallback/positional/iterative.hpp>

#include "num_utils.hpp"

#include <anyoption.h>

using namespace HydrArgs;
using namespace HydrArgs::fallback::positional;

namespace HydrArgs::Backend{
	struct AnyOptionBackend: public IBackendOwnStoredSpec, public PositionalParserIterative{
		AnyOption app;
		std::string name;
		std::string descr;
		std::string usage;

		std::vector<char *> remainingPtrs;

		AnyOptionBackend(const std::string &name, const std::string &descr, const std::string &usage [[maybe_unused]], std::vector<Arg*> dashedSpec, std::vector<Arg*> positionalSpec, Streams streams): IBackendOwnStoredSpec(dashedSpec, positionalSpec, streams), PositionalParserIterative(this), name(name), descr(descr), usage(usage){
			app.addUsage(this->name.data());
			app.addUsage(this->usage.data());
			app.addUsage(this->descr.data());
			//app.setVerbose();
			//app.autoUsagePrint(true);  // shit

			app.setFlag(DEFAULT_HELP_ARG.long_name.undashed, DEFAULT_HELP_ARG.letter_str.getLetter());

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
				// preregister args
				if(argPtr->type == ArgType::flag){
					if(argPtr->letter){
						app.setFlag(argPtr->name.data(), argPtr->letter);
					} else {
						app.setFlag(argPtr->name.data());
					}
				} else {
					if(argPtr->letter){
						app.setOption(argPtr->name.data(), argPtr->letter);
					} else {
						app.setOption(argPtr->name.data());
					}
				}
			}
		}

		virtual ~AnyOptionBackend() override = default;

		virtual bool isNotEnoughArgs(uint32_t positionalArgCountLowerBound) const override {
			return toUnsigned(app.getArgc()) < positionalArgCountLowerBound;
		}

		virtual void printHelp(std::ostream &stream, const char *const argv0 [[maybe_unused]]) override {
			app.printUsage();
			stream << descr << std::endl;
			//app.printHelp();  // Not defined!
			#warning "ToDO: add printing into the selected stream into the library"
		}

		inline char * getNativePosArgType(){
			auto idx = static_cast<int>(getNextPosArgIdx());
			if(idx < app.getArgc()){
				return app.getArgv(idx);
			}
			return nullptr;
		}

		inline bool getNativeDashedFlag(Arg *argPtr){
			return app.getFlag(argPtr->name.data()) || app.getFlag(argPtr->letter);
		}

		inline char * getNativeDashedArgType(Arg *argPtr){
			auto res = app.getValue(argPtr->name.data());
			if(res){
				return res;
			}
			if(argPtr->letter){
				res = app.getValue(argPtr->letter);
				if(res){
					return res;
				}
			}
			return nullptr;
		}

		std::vector<char *> remaining;

		virtual ParseResult _parseArgs(CLIRawArgs rawArgs) override {
			{
				auto tempArgvVec = rawArgs.getArgvVector();
				app.processCommandArgs(checked_cast<int>(tempArgvVec.size() - 1), const_cast<char **>(tempArgvVec.data()));
			}

			PROCESS_CASE_OF_HELP_CALLED(app.getFlag(DEFAULT_HELP_ARG.long_name.undashed) || app.getFlag(DEFAULT_HELP_ARG.letter_str.getLetter()));

			HYDRARGS_CHECK_IF_ENOUGH_MAND_POS_ARGS_AND_RETURN_ERROR_OTHERWISE();

			for(auto &argPtr: dashedSpec){
				if(argPtr->type == ArgType::flag){
					auto specOptPtr = static_cast<SArg<ArgType::flag> *>(argPtr);
					specOptPtr->value = getNativeDashedFlag(argPtr);

					if(!specOptPtr->value){
						PROCESS_ARGUMENT_MISSING_CASE();
					}
				} else {
					char *argString = getNativeDashedArgType(argPtr);
					if(!argString){
						PROCESS_ARGUMENT_MISSING_CASE();
					} else {
						PARSE_ARG_FROM_STRING(argString, rawArgs.argv0);
					}
				}
			}

			resetPositionalParsing();

			for(auto &argPtr: positionalSpec){
				char *argString = getNativePosArgType();
				if(!argString){
					PROCESS_ARGUMENT_MISSING_CASE();
				} else {
					PARSE_ARG_FROM_STRING(argString, rawArgs.argv0);
				}
			}

			auto restSize = toUnsigned(app.getArgc() - toSigned(this->positionalCounter));
			remainingPtrs.reserve(restSize);
			while(char * restArg = getNativePosArgType()){
				remainingPtrs.emplace_back(restArg);
			}
			remainingPtrs.shrink_to_fit();

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
		return new AnyOptionBackend(name, descr, usage, dashedSpec, positionalSpec, streams);
	}
};
