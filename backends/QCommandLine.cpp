#include <cassert>
#include <cstdint>

#include <utility>
#include <algorithm>
#include <iterator>
#include <ostream>
#include <span>
#include <string>
#include <vector>

#include <HydrArgs/HydrArgs.hpp>
#include <HydrArgs/toolbox.hpp>
#include <HydrArgs/fallback/errors.hpp>
#include <HydrArgs/fallback/parsing.hpp>
#include <HydrArgs/fallback/positional/iterative.hpp>

#include <QCommandLineParser>

using namespace HydrArgs;
using namespace HydrArgs::fallback::positional;

namespace HydrArgs::Backend{
	struct QCommandLineBackend: public IBackendOwnStoredSpec, public PositionalParserIterative{
		std::string name;
		std::string usage;
		std::vector<std::string> remaining;
		std::vector<char *> remainingPtrs;
		QCommandLineParser parser;
		QCommandLineOption helpOption, versionOption;

		std::vector<QCommandLineOption> nativeArgs;

		uint32_t dashedCounter = 0;

		QCommandLineBackend(std::string name, const std::string &descr, std::string usage, const std::vector<Arg*> &dashedSpec, const std::vector<Arg*> &positionalSpec, Streams streams): IBackendOwnStoredSpec(dashedSpec, positionalSpec, streams), PositionalParserIterative(this), name(std::move(name)), usage(std::move(usage)), helpOption(QCommandLineOption(QStringList() << DEFAULT_HELP_ARG.long_name.undashed << DEFAULT_HELP_ARG.letter_str.undashed, DEFAULT_HELP_ARG.doc)), versionOption(parser.addVersionOption()){
			parser.setApplicationDescription(descr.data());
			parser.addOption(helpOption);
			parser.setOptionsAfterPositionalArgumentsMode(QCommandLineParser::ParseAsPositionalArguments);

			for(auto argPtr: dashedSpec) {
				_addArg(argPtr, false);
			}
			for(auto argPtr: positionalSpec) {
				_addArg(argPtr, true);
			}
		}

		void _addArg(Arg *argPtr, bool isPositional) override {
			auto qName = QString::fromStdString(argPtr->name);
			auto qDescr = QString::fromStdString(argPtr->description);

			if(isPositional){
				addPosArg(argPtr);
				parser.addPositionalArgument(qName, qDescr);
			} else {
				QStringList optNamesList;
				optNamesList << qName;
				if(argPtr->letter){
					optNamesList << argPtr->letterStr;
				}
				parser.addOption(
					nativeArgs.emplace_back(
						QCommandLineOption{
							optNamesList,
							qDescr,
							(
								argPtr->type == ArgType::flag
							?
								nullptr
							:
								(argPtr->value_name ? argPtr->value_name : VALUE_PLACEHOLDER)
							)
						}
					)
				);
			}
		}

		~QCommandLineBackend() override = default;
		ParseResult _parseArgs(CLIRawArgs rawArgs) override {

			// make sure QCoreApplication is created. If it is not, create it and recurse
			{
				auto appPtr = QCoreApplication::instance();
				if(appPtr == nullptr){
					//QCoreApplication app(rawArgs.argc, const_cast<char **>(rawArgs.argv));
					int shitArgc = 1;
					QCoreApplication const app(shitArgc, const_cast<char **>(&rawArgs.argv0));
					QCoreApplication::setApplicationName(QString::fromStdString(name));
					//QCoreApplication::setApplicationVersion("1.0");
					return _parseArgsQCoreApplicationExists(rawArgs);
				}
			}

			return _parseArgsQCoreApplicationExists(rawArgs);
		}

		[[nodiscard]] bool isNotEnoughArgs(uint32_t positionalArgCountLowerBound) const override {
			return static_cast<decltype(positionalArgCountLowerBound)>(positionalArguments.size()) < positionalArgCountLowerBound;
		}

		void printHelp(std::ostream &stream, const char *const argv0 [[maybe_unused]]) override {
			stream << parser.helpText().toStdString() << std::endl;
		}

		QStringList positionalArguments;

		ParseResult _parseArgsQCoreApplicationExists(CLIRawArgs rawArgs) {
			// Here QCoreApplication must be already created
			QStringList argsStringList;

			argsStringList << rawArgs.argv0;
			for(const auto a: rawArgs.argvRest) {
				argsStringList << a;
			}

			auto parseStatus = parser.parse(argsStringList);

			if(!parseStatus){
				PROCESS_CASE_OF_SYNTAX_ERROR_MESSAGE(parser.errorText().toStdString());
			}

			PROCESS_CASE_OF_HELP_CALLED(parser.isSet(helpOption));

			positionalArguments = parser.positionalArguments();

			HYDRARGS_CHECK_IF_ENOUGH_MAND_POS_ARGS_AND_RETURN_ERROR_OTHERWISE();

			resetPositionalParsing();
			dashedCounter = 0;

			for(auto argPtr: dashedSpec) {
				std::string argString;
				auto &nativeArg = nativeArgs[dashedCounter++];

				if(argPtr->type == ArgType::flag){
					auto specOptPtr = static_cast<SArg<ArgType::flag> *>(argPtr);
					specOptPtr->value = parser.isSet(nativeArg);

					if(!specOptPtr->value){
						PROCESS_ARGUMENT_MISSING_CASE();
					}
				} else {
					if(parser.isSet(nativeArg)){
						argString = parser.value(nativeArg).toStdString();
						PARSE_ARG_FROM_STRING(argString, rawArgs.argv0);
					} else {
						PROCESS_ARGUMENT_MISSING_CASE();
					}
				}
			}

			for(auto argPtr: positionalSpec) {
				std::string argString;
				if(this->positionalCounter < positionalArguments.size()){
					argString = positionalArguments[this->positionalCounter].toStdString();
					PARSE_ARG_FROM_STRING(argString, rawArgs.argv0);
					this->positionalCounter++;
				} else {
					PROCESS_ARGUMENT_MISSING_CASE();
					break;
				}
			}

			auto remainingCountS = positionalArguments.size() - static_cast<int64_t>(this->positionalCounter);
			assert(remainingCountS >= 0);
			auto remainingCount = static_cast<uint32_t>(remainingCountS);

			remaining.reserve(remainingCount);
			remaining.resize(remainingCount);
			for(uint32_t i = 0; i<remainingCount;++i){
				remaining[i] = positionalArguments[i + this->positionalCounter].toStdString();
			}

			auto remainingPtrsSize = remainingCount;
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
		return new QCommandLineBackend(name, descr, usage, dashedSpec, positionalSpec, streams);
	}
};
