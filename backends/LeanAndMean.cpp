#include <memory>

#include <cassert>
#include <cstring>
#include <iostream>
#include <optional>
#include <utility>

#include <HydrArgs/HydrArgs.hpp>
#include <HydrArgs/fallback/parsing.hpp>
#include <HydrArgs/fallback/positional/spannable.hpp>
#include <HydrArgs/toolbox.hpp>

#include <optionparser.h>

using namespace HydrArgs;
using namespace HydrArgs::fallback::positional;

namespace HydrArgs::Backend{
	using OptionTypeIntT = decltype(static_cast<option::Descriptor *>(nullptr)->type);

	enum class SpecialOptionIndex: OptionTypeIntT {
		doc = 0,
		help = 1,
		COUNT
	};

	const option::Descriptor helpOpt{
		.index = static_cast<OptionTypeIntT>(SpecialOptionIndex::help),
		.type = 0,
		.shortopt = DEFAULT_HELP_ARG.letter_str.undashed,
		.longopt = DEFAULT_HELP_ARG.long_name.undashed,
		.check_arg = option::Arg::None,
		.help = DEFAULT_HELP_ARG.doc
	};

	const option::Descriptor emptyOpt{
		.index = 0,
		.type = 0,
		.shortopt = nullptr,
		.longopt = nullptr,
		.check_arg = nullptr,
		.help = nullptr
	};

	struct LeanAndMeanBackend: public IBackendOwnStoredSpec, public PositionalParserSpannable{
		std::vector<std::string> remaining;
		std::vector<char *> remainingPtrs;
		std::vector<option::Descriptor> nativeArgs;

		static constexpr const uint8_t DEFAULT_NATIVE_ARGS_COUNT = 1;  // --help

		const std::string descr;

		LeanAndMeanBackend(const std::string &name, const std::string &descr, const std::string &usage, std::vector<Arg*> dashedSpec, std::vector<Arg*> positionalSpec, Streams streams): IBackendOwnStoredSpec(dashedSpec, positionalSpec, streams), PositionalParserSpannable(this), descr(descr){
			//arg_set_module_version(int major, int minor, int patch, const char* tag);
			nativeArgs.emplace_back(option::Descriptor{
				.index = 0,
				.type = 0,
				.shortopt = "",
				.longopt = "",
				.check_arg = option::Arg::None,
				.help = descr.data()
			});  // 0, doc
			nativeArgs.emplace_back(helpOpt);  // 1, help
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
				nativeArgs.emplace_back(option::Descriptor{
					.index = static_cast<unsigned int>(nativeArgs.size()),
					.type = 0,
					.shortopt = (argPtr->letter ? argPtr->letterStr : ""),
					.longopt = argPtr->name.data(),
					.check_arg = (argPtr->type == ArgType::flag ? option::Arg::None : option::Arg::OptionalAny),
					.help = argPtr->description.data()
				});
			}
		}

		inline struct arg_end ** _getEndArgPtrPtr(){
			return reinterpret_cast<struct arg_end **>(&nativeArgs.back());
		}

		inline struct arg_end * _getEndArgPtr(){
			return *_getEndArgPtrPtr();
		}

		virtual ~LeanAndMeanBackend() = default;

		virtual void _seal() override {
			nativeArgs.emplace_back(emptyOpt);
		}

		virtual void _unseal() override {
			nativeArgs.pop_back();
		}

		virtual bool isSealed() const override {
			return nativeArgs.size() && nativeArgs.back().index == 0 && nativeArgs.back().check_arg == nullptr && nativeArgs.back().help == nullptr && nativeArgs.back().longopt == nullptr && nativeArgs.back().shortopt == nullptr;
		}

		uint32_t restc = 0;
		virtual bool isNotEnoughArgs(uint32_t positionalArgCountLowerBound) const override {
			return restc < positionalArgCountLowerBound;
		}

		option::Parser *parser = nullptr;
		virtual void printHelp(std::ostream &stream, const char * const argv0) override {
			seal();
			option::printUsage(stream, nativeArgs.data());
		}

		virtual ParseResult _parseArgs(CLIRawArgs rawArgs) override {
			option::Stats stats(nativeArgs.data(), static_cast<int>(rawArgs.argvRest.size()), const_cast<const char **>(&rawArgs.argvRest[0]));
			std::vector<option::Option> options(stats.options_max);
			std::optional<option::Parser> parser;
			{
				std::vector<option::Option> buffer(stats.buffer_max);
				parser.emplace(nativeArgs.data(), static_cast<int>(rawArgs.argvRest.size()), const_cast<const char **>(&rawArgs.argvRest[0]), &options[0], &buffer[0]);
			}
			this->parser = &*parser;

			PROCESS_CASE_OF_HELP_CALLED(options[static_cast<OptionTypeIntT>(SpecialOptionIndex::help)].count());

			if(parser->error()){
				PROCESS_CASE_OF_SYNTAX_ERROR_MESSAGE("An error has occured, but LeanAndMean backend cannot explain it");
			}
			auto positionalArguments = parser->nonOptions();

			restc = parser->nonOptionsCount();
			HYDRARGS_CHECK_IF_ENOUGH_MAND_POS_ARGS_AND_RETURN_ERROR_OTHERWISE();

			auto dashedIdx = static_cast<OptionTypeIntT>(SpecialOptionIndex::COUNT);
			for(auto argPtr: dashedSpec){
				auto nativeArg = options[dashedIdx++];

				if(argPtr->type == ArgType::flag){
					auto specOptPtr = static_cast<SArg<ArgType::flag>*>(argPtr);
					specOptPtr->value = nativeArg.count();

					if(!specOptPtr->value){
						PROCESS_ARGUMENT_MISSING_CASE();
					}
				} else {
					if(nativeArg.count()){
						auto argString = nativeArg.arg;
						PARSE_ARG_FROM_STRING(argString, rawArgs.argv0);
					} else {
						PROCESS_ARGUMENT_MISSING_CASE();
					}
				}
			}

			resetPositionalParsing();
			auto restSpan = std::span<const char *>(positionalArguments, restc);
			return parsePositionals(rawArgs.argv0, restSpan);
		}
	};

	IArgsParser * argsParserFactory(const std::string &name, const std::string &descr, const std::string &usage [[maybe_unused]], std::vector<Arg*> dashedSpec, std::vector<Arg*> positionalSpec, Streams streams){
		return new LeanAndMeanBackend(name, descr, usage, dashedSpec, positionalSpec, streams);
	}
};
