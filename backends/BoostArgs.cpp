#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <exception>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include <HydrArgs/HydrArgs.hpp>
#include <HydrArgs/fallback/errors.hpp>
#include <HydrArgs/toolbox.hpp>

#include "num_utils.hpp"

#include <boost/program_options.hpp>


using namespace HydrArgs;

namespace HydrArgs::Backend{

	static std::unordered_map<std::string, struct ParseEvents> specialOptsMap{
		{DEFAULT_HELP_ARG.long_name.undashed, EVENTS_HELP_CALLED}
	};

	boost::program_options::options_description initCommonOptsDesc(){
		boost::program_options::options_description res;
		auto optsAdder = res.add_options();
		optsAdder(DEFAULT_HELP_ARG.long_name.undashed, DEFAULT_HELP_ARG.doc);
		return res;
	}

	const boost::program_options::options_description oCommonOptsDesc = initCommonOptsDesc();  ///< the options description that st be present in every program. Used as a workaround to inability to parse options if the rest of options are invalid.

	struct ParseEvents parseCommonOptions(CLIRawArgs::VectorT & argvVector){
		auto parsed = boost::program_options::command_line_parser(checked_cast<int>(argvVector.size() - 1), argvVector.data()).options(oCommonOptsDesc).allow_unregistered().run();

		for(auto &o: parsed.options){
			auto resIter = specialOptsMap.find(o.string_key);
			if(resIter != end(specialOptsMap)){
				return (*resIter).second;
			}
		}
		return EVENTS_NONE;
	}

	struct BoostArgsBackend: public IBackendOwnStoredSpec{
		std::string usage;
		std::string descr;

		boost::program_options::options_description oDesc;  ///< main options descriptiom
		boost::program_options::positional_options_description pd;
		boost::program_options::options_description_easy_init optsAdder;

		std::vector<std::string> remaining;
		std::vector<char *> remainingPtrs;

		BoostArgsBackend(const std::string &name, const std::string &descr, const std::string &usage, std::vector<Arg*> dashedSpec, std::vector<Arg*> positionalSpec, Streams streams): IBackendOwnStoredSpec(dashedSpec, positionalSpec, streams), usage(usage), descr(descr), oDesc(name), optsAdder(oDesc.add_options()){
			for(auto &argPtr: dashedSpec){
				_addArg(argPtr, false);
			}
			for(auto &argPtr: positionalSpec){
				_addArg(argPtr, true);
			}
		}

		virtual void _addArg(Arg *argPtr, bool isPositional) override {
			if(isPositional){
				pd.add(argPtr->name.data(), 1);
			}

			auto switchCaseBodyLambda = [&] <ArgType t>() {
				auto specOptPtr = static_cast<SArg<t>*>(argPtr);

				if constexpr (t == ArgType::flag){
					if(isPositional){
						auto nativeOpt = boost::program_options::value<decltype(specOptPtr->value)>();
						if(argPtr->value_name){
							nativeOpt->value_name(argPtr->value_name);
						}
						if(argPtr->minCount){
							nativeOpt->required();
						}
						optsAdder(argPtr->name.data(), nativeOpt, argPtr->description.data());
					} else {
						//Should likely be the same as the positional branch, but with ->zero_tokens();, but it doesn't work
						optsAdder(argPtr->name.data(), argPtr->description.data());
					}
				} else {
					auto nativeOpt = boost::program_options::value<decltype(specOptPtr->value)>();
					if(argPtr->value_name){
						nativeOpt->value_name(argPtr->value_name);
					}
					if(argPtr->minCount){
						nativeOpt->required();
					}
					optsAdder(argPtr->name.data(), nativeOpt, argPtr->description.data());
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

		virtual ~BoostArgsBackend() override = default;

		virtual void printHelp(std::ostream &stream, const char * const argv0) override {
			stream << argv0 << " " << usage << std::endl;
			stream << descr << std::endl;
			stream << oDesc;
		}

		virtual ParseResult _parseArgs(CLIRawArgs rawArgs) override {
			auto tempArgvVec = rawArgs.getArgvVector();
			{
				//Processing common options. Working around that the lib doesn't support --help natively and that it throws when incorrect arg occurs.
				auto specialEvents = parseCommonOptions(tempArgvVec);
				if(specialEvents){
					if(specialEvents.helpCalled){
						printHelp(streams.cout, rawArgs.argv0);
					}
					return {
						.parsingStatus = {
							.returnCode = 0,
							.events = specialEvents,
						},
						.rest = {
							.argv0 = rawArgs.argv0,
							.argvRest = NO_REST_ARGS
						}
					};
				}
			}

			boost::program_options::variables_map vm;
			try{
				auto parsed = boost::program_options::command_line_parser(checked_cast<int>(tempArgvVec.size() - 1), tempArgvVec.data()).options(oDesc).positional(pd).allow_unregistered().run();
				boost::program_options::store(parsed, vm);
				boost::program_options::notify(vm);
				remaining = boost::program_options::collect_unrecognized(parsed.options, boost::program_options::collect_unrecognized_mode::include_positional);
			} catch(std::exception &ex) {
				PROCESS_CASE_OF_SYNTAX_ERROR_WHATABLE_EXCEPTION(ex);
			}

			std::pair<decltype(dashedSpec)&, bool> specs[]{
				{dashedSpec, false},
				{positionalSpec, true},
			};
			for(auto p: specs){
				auto spec = p.first;
				auto isPositional = p.second;
				for(auto argPtr: spec){
					auto argVariant = vm[argPtr->name];
					if(argVariant.empty()){
						// working round inability to set ->required() for a flag
						if(argPtr->type == ArgType::flag){
							auto specOptPtr = static_cast<SArg<ArgType::flag>*>(argPtr);
							specOptPtr->value = false;
							PROCESS_ARGUMENT_MISSING_CASE();
						}
						continue;
					}
					switch(argPtr->type){
						case ArgType::flag:
						{
							auto specOptPtr = static_cast<SArg<ArgType::flag>*>(argPtr);
							if(isPositional){
								specOptPtr->value = argVariant.as<bool>();
							} else {
								specOptPtr->value = true;
							}
						}
						break;
						case ArgType::s4:
						{
							auto specOptPtr = static_cast<SArg<ArgType::s4>*>(argPtr);
							specOptPtr->value = argVariant.as<int>();
						}
						break;
						case ArgType::string:
						case ArgType::path:
						{
							auto specOptPtr = static_cast<SArg<ArgType::string>*>(argPtr);
							specOptPtr->value = argVariant.as<std::string>();
						}
						break;
						default:
							throw UnsupportedArgumentType{argPtr->type};
					}
				}
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

	IArgsParser * argsParserFactory(const std::string &name, const std::string &descr, const std::string &usage [[maybe_unused]], const std::vector<Arg *> &dashedSpec, const std::vector<Arg *> &positionalSpec, Streams streams){
		return new BoostArgsBackend(name, descr, usage, dashedSpec, positionalSpec, streams);
	}
};
