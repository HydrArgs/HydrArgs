#include <cstddef>

#include <iostream>
#include <utility>
#include <algorithm>
#include <exception>
#include <iterator>
#include <string>
#include <vector>

#include <HydrArgs/HydrArgs.hpp>
#include <HydrArgs/toolbox.hpp>

#include "num_utils.hpp"

#include <octargs/octargs.hpp>


using namespace HydrArgs;

namespace HydrArgs::Backend{

	struct OctargsBackend: public IBackendOwnStoredSpec{
		oct::args::parser app;
		std::vector<std::string> remaining;
		std::vector<char *> remainingPtrs;

		OctargsBackend(const std::string &name [[maybe_unused]], const std::string &descr, const std::string &usage [[maybe_unused]], const std::vector<Arg*> &dashedSpec, const std::vector<Arg*> &positionalSpec, Streams streams): IBackendOwnStoredSpec(dashedSpec, positionalSpec, streams){
			//app.set_usage_oneliner(descr);
			app.set_usage_header(descr);
			//app.set_usage_footer(descr);
			app.set_allow_unconsummed(true);
			app.add_exclusive(DEFAULT_HELP_ARG.long_name.dashed).set_description(DEFAULT_HELP_ARG.doc);
			for(auto argPtr: dashedSpec){
				_addArg(argPtr, false);
			}
			for(auto argPtr: positionalSpec){
				_addArg(argPtr, true);
			}
		}

		void _addArg(Arg *argPtr, bool isPositional) override {
			auto switchCaseBodyLambda = [&] <ArgType t>() {
				if(isPositional){
					auto arg = app.add_positional(argPtr->name).set_min_count(argPtr->minCount).set_max_count(argPtr->maxCount).set_description(argPtr->description);
					arg.set_type<typename  GetUnderlyingTypeByEnumValue<t>::type>();
				} else {
					if constexpr (t == ArgType::flag){
						auto arg = app.add_switch("--" + argPtr->name).set_min_count(argPtr->minCount).set_max_count(argPtr->maxCount).set_description(argPtr->description);
					} else {
						auto arg = app.add_valued("--" + argPtr->name).set_min_count(argPtr->minCount).set_max_count(argPtr->maxCount).set_description(argPtr->description);
						arg.set_type<typename GetUnderlyingTypeByEnumValue<t>::type>();
					}
				}
			};
			switch(argPtr->type){
				case ArgType::flag:
				{
					switchCaseBodyLambda.operator()<ArgType::flag>();
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
		~OctargsBackend() override = default;

		void printHelp(std::ostream &stream, const char *const argv0 [[maybe_unused]]) override {
			stream << app.get_usage() << std::endl;
		}

		ParseResult _parseArgs(CLIRawArgs rawArgs) override {
			decltype(app.parse(0, static_cast<char **>(nullptr))) res{nullptr, nullptr};

			try{
				auto tempArgvVec = rawArgs.getArgvVector();
				res = app.parse(checked_cast<int>(tempArgvVec.size() - 1), tempArgvVec.data());
			} catch(const oct::args::parser_error_ex<char> &exc) {
				PROCESS_CASE_OF_SYNTAX_ERROR_MESSAGE_WITH_KNOWN_LOCATION(exc.get_name(), exc.get_value());
			}

			PROCESS_CASE_OF_HELP_CALLED(res.has_value(DEFAULT_HELP_ARG.long_name.dashed));

			std::pair<decltype(dashedSpec)&, bool> const specs[]{
				{dashedSpec, false},
				{positionalSpec, true},
			};
			for(auto p: specs){
				auto spec = p.first;
				auto isPositional = p.second;

				for(auto argPtr: spec){
					auto argName = isPositional ? argPtr->name : ("--" + argPtr->name);
					auto switchCaseBodyLambda = [&] <ArgType t>() {
						auto specOptPtr = static_cast<SArg<t>*>(argPtr);
						if constexpr (t == ArgType::flag){
							if(isPositional){
								if(res.has_value(argName)){
									specOptPtr->value = res.get_first_value_as<decltype(specOptPtr->value)>(argName);
								}
							} else {
								specOptPtr->value = res.has_value(argName);
							}
						} else {
							if(res.has_value(argName)){
								specOptPtr->value = res.get_first_value_as<decltype(specOptPtr->value)>(argName);
							}
						}
					};
					switch(argPtr->type){
						case ArgType::flag:
						{
							switchCaseBodyLambda.operator()<ArgType::flag>();
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

			remaining = res.get_unconsummed_args();
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

	IArgsParser * argsParserFactory(const std::string &name, const std::string &descr, const std::string &usage [[maybe_unused]], const std::vector<Arg *> &dashedSpec, const std::vector<Arg *> &positionalSpec, Streams streams){
		return new OctargsBackend(name, descr, usage, std::move(dashedSpec), std::move(positionalSpec), streams);
	}
};
