#include <cstdint>
#include <cassert>
#include <cstring>

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <utility>
#include <iterator>
#include <limits>
#include <string>
#include <vector>

#include <HydrArgs/HydrArgs.hpp>
#include <HydrArgs/fallback/parsing.hpp>
#include <HydrArgs/fallback/positional/iterative.hpp>
#include <HydrArgs/toolbox.hpp>

#include "num_utils.hpp"

#include <getopt.h>


using namespace HydrArgs::fallback::positional;

namespace HydrArgs::Backend{

	const auto emptyOpt = option{nullptr, 0, nullptr, 0};

	struct GetoptBackend: public IBackendOwnStoredSpec, PositionalParserIterative{
		std::vector<const char *> remainingPtrs;
		std::vector<struct option> nativeLongArgs;
		std::string nativeShortArgs;

		std::unordered_map<char, Arg *> dashedCharToArgMap;

		const std::string descr;

		static constexpr const uint8_t LONG_ARGS_OFFSET = std::numeric_limits<char>::max() + 1;  ///< To exclude intersection between short args and long-only args we move long-only ones to the range not available to short-only ones (chars)

		std::vector<Arg *> dashedMandatoryArgs;

		GetoptBackend(const std::string &name, const std::string &descr, const std::string &usage [[maybe_unused]], std::vector<Arg*> dashedSpec, std::vector<Arg*> positionalSpec, Streams streams): IBackendOwnStoredSpec(dashedSpec, positionalSpec, streams), PositionalParserIterative(this), descr(descr){
			//arg_set_module_version(int major, int minor, int patch, const char* tag);

			nativeLongArgs.emplace_back(option {
				.name = "help",
				.has_arg = no_argument,
				.flag = nullptr,
				.val = 'h',  // MUST MATCH!
			});
			nativeShortArgs += "-h:";  // leading `-` means positional are returned as 1! It is a GNU extension!

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
				struct option newOpt{
					.name = argPtr->name.data(),
					.flag = nullptr,
				};
				if(argPtr->letter){
					nativeShortArgs += argPtr->letterStr;
					if(argPtr->type != ArgType::flag){
						nativeShortArgs += ':';
					}
					dashedCharToArgMap[argPtr->letter] = argPtr;
					newOpt.val = argPtr->letter;
				} else {
					newOpt.val = static_cast<int>(nativeLongArgs.size() + LONG_ARGS_OFFSET);
				}

				if(argPtr->type == ArgType::flag){
					newOpt.has_arg = no_argument;
				} else {
					newOpt.has_arg = required_argument;
				}
				nativeLongArgs.emplace_back(newOpt);
				if(argPtr->minCount){
					dashedMandatoryArgs.emplace_back(argPtr);
				}
			}
		}

		virtual void _seal() override {
			nativeLongArgs.emplace_back(emptyOpt);
		}

		virtual void _unseal() override {
			nativeLongArgs.pop_back();
		}

		virtual bool isSealed() const override {
			return nativeLongArgs.size() && nativeLongArgs.back().name == nullptr;
		}

		virtual ~GetoptBackend() override = default;

		virtual void printHelp(std::ostream &stream, const char *const argv0) override {
			auto argtable = reinterpret_cast<void **>(nativeLongArgs.data());
			//printHelpMessage(argv0, argtable, stream);
			stream << std::endl;
		}

		virtual bool isNotEnoughArgs(uint32_t positionalArgCountLowerBound) const override {
			return positionalCounter < mandatoryPositionalArgsCount;
		}

		void resetGetoptInternalState(){
			optind = 1;
			opterr = 0;
			optopt = 0;
			optarg = nullptr;
		}

		virtual ParseResult _parseArgs(CLIRawArgs rawArgs) override {
			int long_option_index;
			int short_option_char_SignedInt;

			std::unordered_set<Arg*> setDashedArgs;

			resetGetoptInternalState();

			auto tempArgvVec = rawArgs.getArgvVector();

			for(;;){
				long_option_index = 0;

				short_option_char_SignedInt = getopt_long(checked_cast<int>(tempArgvVec.size() - 1), const_cast<char **>(tempArgvVec.data()), nativeShortArgs.data(), nativeLongArgs.data(), &long_option_index);

				std::cout << "optind: " << optind << std::endl;
				std::cout << "argc: " << tempArgvVec.size() - 1 << std::endl;
				switch(short_option_char_SignedInt){
					case 1:
					{
						std::cout << "positional" << std::endl;
						// It is a positional arg;
						const char *argCStr = optarg;
						std::string argString = argCStr;
						if(positionalCounter < positionalSpec.size()){
							auto curPosIdx = getNextPosArgIdx();
							auto argPtr = positionalSpec[curPosIdx];
							PARSE_ARG_FROM_STRING(argString, rawArgs.argv0);
						} else {
							remainingPtrs.emplace_back(const_cast<const char *>(argCStr));
						}
					}
					break;
					case -1:
					{
						goto exitLoop;
					}
					break;
					case '?':
					{
						// either a request for help, or syntax error
						auto short_option_char = static_cast<char>(short_option_char_SignedInt);
						PROCESS_CASE_OF_HELP_CALLED(optopt == short_option_char);

						// It is not help ...
						std::cerr << "Unknown short option: " << optopt << std::endl;
						printHelp(std::cerr, rawArgs.argv0);
						return RESULT_SYNTAX_ERROR;
					}
					break;
					default:
					{
						Arg *argPtr = nullptr;

						if(long_option_index > 0){
							auto nla = &nativeLongArgs[long_option_index];
							if(long_option_index >= LONG_ARGS_OFFSET){
								auto dashedArgIdx = static_cast<size_t>(long_option_index - LONG_ARGS_OFFSET);
								argPtr = dashedSpec[dashedArgIdx];
							} else {
								PROCESS_CASE_OF_HELP_CALLED(long_option_index == 'h');
							}
						}


						if(!argPtr && short_option_char_SignedInt > 0 && short_option_char_SignedInt <= std::numeric_limits<char>::max()){
							const auto short_option_char = static_cast<char>(short_option_char_SignedInt);
							PROCESS_CASE_OF_HELP_CALLED(short_option_char == 'h');
							const auto it = dashedCharToArgMap.find(short_option_char);
							if(it != end(dashedCharToArgMap)){
								// It is a short option!
								argPtr = it->second;
							}
						}

						if(argPtr){
							setDashedArgs.emplace(argPtr);
							switch(argPtr->type){
								case ArgType::flag:
								{
									auto specOptPtr = static_cast<SArg<ArgType::flag>*>(argPtr);
									specOptPtr->value = true;
								}
								break;
								default:
								{
									std::string valStr = optarg;
									PARSE_ARG_FROM_STRING(valStr, rawArgs.argv0);
								}
								break;
							}
						} else {
							assert(false);  // we must never reach here. It means an option was parsed that was not registered
						}
					}
					break;
				}
			}

		exitLoop:;
			HYDRARGS_CHECK_IF_ENOUGH_MAND_POS_ARGS_AND_RETURN_ERROR_OTHERWISE();

			// checking mandatory dashed args
			for(auto argPtr: dashedMandatoryArgs){
				if(!setDashedArgs.contains(argPtr)){
					HydrArgs::fallback::errors::mandatoryArgumentMissingErrorMessage(this, argPtr, rawArgs.argv0);
					return RESULT_SYNTAX_ERROR;
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
		return new GetoptBackend(name, descr, usage, dashedSpec, positionalSpec, streams);
	}

};
