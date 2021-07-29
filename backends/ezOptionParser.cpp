#include <HydrArgs/HydrArgs.hpp>
#include <HydrArgs/toolbox.hpp>

#include <HydrArgs/fallback/errors.hpp>
#include <HydrArgs/fallback/parsing.hpp>
#include <HydrArgs/fallback/positional/iterative.hpp>

#include <ezOptionParser.hpp>

using namespace HydrArgs;
using namespace HydrArgs::fallback::positional;

namespace HydrArgs::Backend{

	struct HYDRARGS_BACKEND_API EzOptionParserBackend: public IBackendOwnStoredSpec, public PositionalParserIterative{
		ez::ezOptionParser app;
		std::vector<std::string> remaining;
		std::vector<char *> remainingPtrs;

		EzOptionParserBackend(const std::string &name, const std::string &descr, const std::string &usage, std::vector<Arg*> dashedSpec, std::vector<Arg*> positionalSpec, Streams streams): IBackendOwnStoredSpec(dashedSpec, positionalSpec, streams), PositionalParserIterative(this), app(){
			app.syntax = usage;
			app.overview = descr;

			app.add("", false, 0, '\0', DEFAULT_HELP_ARG.doc, DEFAULT_HELP_ARG.letter_str.dashed, DEFAULT_HELP_ARG.long_name.dashed);

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
				auto longNameTmp = "--" + argPtr->name;

				if(!argPtr->letter){
					app.add("",
						argPtr->minCount,
						argPtr->type != ArgType::flag,
						'\0',
						argPtr->description.data(),
						longNameTmp.data()  // Implementation detail - internally it is converted into an std::string, so copied and the pointer can be freed, so no use-after-free
					);
				} else {
					auto shortNameTmp = std::string{"-"} + argPtr->letterStr;
					app.add("",
						argPtr->minCount,
						argPtr->type != ArgType::flag,
						'\0',
						argPtr->description.data(),
						longNameTmp.data(),  // Implementation detail - internally it is converted into an std::string, so copied and the pointer can be freed, so no use-after-free
						shortNameTmp.data()
					);
				}
			}
		}

		virtual ~EzOptionParserBackend() override = default;

		virtual bool isNotEnoughArgs(uint32_t positionalArgCountLowerBound) const override {
			return app.lastArgs.size() < positionalArgCountLowerBound;
		}

		virtual void printHelp(std::ostream &stream, const char *const argv0 [[maybe_unused]]) override {
			std::string usage;
			app.getUsage(usage, 170);
			stream << usage;
		}

		inline std::string *getNativePosArgType(){
			if(positionalCounter < app.lastArgs.size()){
				return app.lastArgs[positionalCounter++];
			} else {
				return nullptr;
			}
		}

		inline bool getNativeDashedFlag(Arg *argPtr){
			return app.isSet(argPtr->name.data());
		}

		inline ez::OptionGroup *getNativeDashedArgType(Arg *argPtr){
			return app.get(argPtr->name.data());
		}

		inline ParseStatus parseDashedArg(Arg *argPtr, ez::OptionGroup *nativeArg, const char *argv0){
			if(nativeArg){
				switch(argPtr->type){
					case ArgType::flag:
					{
						auto specOptPtr = static_cast<SArg<ArgType::flag>*>(argPtr);
						specOptPtr->value = nativeArg->isSet;
					}
					break;
					case ArgType::s4:
					{
						auto specOptPtr = static_cast<SArg<ArgType::s4>*>(argPtr);
						nativeArg->getInt(specOptPtr->value);
					}
					break;
					case ArgType::u8:
					{
						auto specOptPtr = static_cast<SArg<ArgType::u8>*>(argPtr);
						nativeArg->getULong(specOptPtr->value);
					}
					break;
					case ArgType::s8:
					{
						auto specOptPtr = static_cast<SArg<ArgType::s8>*>(argPtr);
						nativeArg->getLong(specOptPtr->value);
					}
					break;
					case ArgType::f4:
					{
						auto specOptPtr = static_cast<SArg<ArgType::f4>*>(argPtr);
						nativeArg->getFloat(specOptPtr->value);
					}
					break;
					case ArgType::f8:
					{
						auto specOptPtr = static_cast<SArg<ArgType::f8>*>(argPtr);
						nativeArg->getDouble(specOptPtr->value);
					}
					break;
					case ArgType::string:
					case ArgType::path:
					{
						auto specOptPtr = static_cast<SArg<ArgType::string>*>(argPtr);
						nativeArg->getString(specOptPtr->value);
					}
					break;
				}
			} else {
				if(argPtr->minCount){
					HydrArgs::fallback::errors::mandatoryArgumentMissingErrorMessage(this, argPtr, argv0);
					return STATUS_SYNTAX_ERROR;
				}
			}
			return STATUS_OK;
		}

		virtual ParseResult _parseArgs(CLIRawArgs rawArgs) override {
			app.resetArgs();
			{
				auto tempArgvVec = rawArgs.getArgvVector();
				app.parse(tempArgvVec.size() - 1, tempArgvVec.data());
			}

			PROCESS_CASE_OF_HELP_CALLED(app.isSet(DEFAULT_HELP_ARG.long_name.dashed));

			std::vector<std::string> badOptions;
			if(!app.gotRequired(badOptions)) {
				for(auto errorMessage: badOptions){
					std::cerr << HydrArgs::fallback::errors::mandatoryArgumentMissingFormatErrorMessage(errorMessage.data()) << std::endl;
				}
				printHelp(streams.cerr, rawArgs.argv0);
				return RESULT_SYNTAX_ERROR;
			}

			HYDRARGS_CHECK_IF_ENOUGH_MAND_POS_ARGS_AND_RETURN_ERROR_OTHERWISE()

			resetPositionalParsing();

			for(auto argPtr: dashedSpec){
				ParseStatus resStatus;
				auto argName = "--" + argPtr->name;
				auto nativeArg = app.get(argName.data());
				resStatus = parseDashedArg(argPtr, nativeArg, rawArgs.argv0);
				if(resStatus){
					return {
						.parsingStatus = resStatus,
						.rest = NO_ARGS
					};
				}
			}

			for(auto argPtr: positionalSpec){
				ParseStatus resStatus;
				auto argString = getNativePosArgType();
				if(argString){
					PARSE_ARG_FROM_STRING(*argString, rawArgs.argv0)
				} else {
					PROCESS_ARGUMENT_MISSING_CASE();
					break;
				}
			}


			auto idx = static_cast<size_t>(getNextPosArgIdx());

			auto remainingPtrsSize = app.lastArgs.size() - idx;
			remainingPtrs.reserve(remainingPtrsSize);
			remainingPtrs.resize(remainingPtrsSize);

			std::transform(begin(app.lastArgs) + idx, end(app.lastArgs), begin(remainingPtrs), [](std::string *el) -> char * {return el->data();});

			return {
				.parsingStatus = STATUS_OK,
				.rest = {
					.argv0 = rawArgs.argv0,
					.argvRest = {const_cast<const char **>(remainingPtrs.data()), remainingPtrsSize}
				}
			};
		}
	};

	IArgsParser * argsParserFactory(const std::string &name, const std::string &descr, const std::string &usage [[maybe_unused]], std::vector<Arg*> dashedSpec, std::vector<Arg*> positionalSpec, Streams streams){
		return new EzOptionParserBackend(name, descr, usage, dashedSpec, positionalSpec, streams);
	}
};
