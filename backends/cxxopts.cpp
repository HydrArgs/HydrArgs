#include <algorithm>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <iterator>
#include <vector>

#include <HydrArgs/HydrArgs.hpp>
#include <HydrArgs/fallback/errors.hpp>
#include <HydrArgs/toolbox.hpp>

#include "num_utils.hpp"

#include <cxxopts.hpp>


namespace HydrArgs::Backend{
	struct CXXOptsBackend: public IBackendOwnStoredSpec{
		cxxopts::Options app;
		cxxopts::OptionAdder optAdder;
		std::vector<std::string> remaining;
		std::vector<char *> remainingPtrs;
		std::vector<cxxopts::Option> nativeArgs;

		std::vector<std::string> positionalArgs;

		bool show_help = false;

		CXXOptsBackend(const std::string &name, const std::string &descr, const std::string &usage [[maybe_unused]], const std::vector<Arg*>& dashedSpec, const std::vector<Arg*>& positionalSpec, Streams streams): IBackendOwnStoredSpec(dashedSpec, positionalSpec, streams), app(name, descr), optAdder(app.add_options()){
			std::vector<std::string> const positionals;

			for(auto argPtr: dashedSpec){
				_addArg(argPtr, false);
			}

			for(auto argPtr: positionalSpec){
				_addArg(argPtr, true);
			}

			app.parse_positional(positionals);
			app.allow_unrecognised_options();
			//app.set_tab_expansion();
			optAdder(DEFAULT_HELP_ARG.long_name.undashed, DEFAULT_HELP_ARG.doc);
		}

		void _addArg(Arg *argPtr, bool isPositional) override {
			auto switchCaseBodyLambda = [&] <ArgType t>() {
				auto specOptPtr = static_cast<SArg<t>*>(argPtr);
				if constexpr (t == ArgType::flag){
					optAdder(specOptPtr->name, specOptPtr->description);
				} else {
					if constexpr (t == ArgType::string){
						optAdder(specOptPtr->name, specOptPtr->description, cxxopts::value<decltype(specOptPtr->value)>()->default_value(specOptPtr->value));
					} else {
						std::stringstream s;
						s << specOptPtr->value;
						optAdder(specOptPtr->name, specOptPtr->description, cxxopts::value<decltype(specOptPtr->value)>()->default_value(s.str()));
					}
				}
			};
			switch(argPtr->type){
				case ArgType::flag:
				{
					switchCaseBodyLambda.operator()<ArgType::flag>();
				}
				break;
				case ArgType::u1:
				{
					switchCaseBodyLambda.operator()<ArgType::u1>();
				}
				break;
				case ArgType::s1:
				{
					switchCaseBodyLambda.operator()<ArgType::s1>();
				}
				break;
				case ArgType::u2:
				{
					switchCaseBodyLambda.operator()<ArgType::u2>();
				}
				break;
				case ArgType::s2:
				{
					switchCaseBodyLambda.operator()<ArgType::s2>();
				}
				break;
				case ArgType::u4:
				{
					switchCaseBodyLambda.operator()<ArgType::u4>();
				}
				break;
				case ArgType::s4:
				{
					switchCaseBodyLambda.operator()<ArgType::s4>();
				}
				break;
				case ArgType::u8:
				{
					switchCaseBodyLambda.operator()<ArgType::u8>();
				}
				break;
				case ArgType::s8:
				{
					switchCaseBodyLambda.operator()<ArgType::s8>();
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
			if(isPositional){
				//  `m_positional` is overridden (not appended) on every `app.parse_positional` (this function adds positional arguments). There is no way to only append element there. So we have to populate an own array and then call `app.parse_positional` in `_seal`
				positionalArgs.emplace_back(argPtr->name);
				/*
				quote from the docs:
				options.parse_positional({"first", "second", "last"})
				where "last" should be the name of an option with a container type, and the others should have a single value.
				*/
			}
		}
		~CXXOptsBackend() override = default;

		void _seal() override {
			app.parse_positional(positionalArgs);
		}

		void _unseal() override {
		}

		bool isSealed() const override {
			return false;
		}

		void printHelp(std::ostream &stream, const char *const argv0 [[maybe_unused]]) override {
			stream << app.help({""});
		}

		ParseResult _parseArgs(CLIRawArgs rawArgs) override {
			cxxopts::ParseResult result;

			{
				auto tempArgvVec = rawArgs.getArgvVector();
				result = app.parse(checked_cast<int>(tempArgvVec.size() - 1), tempArgvVec.data());
			}

			PROCESS_CASE_OF_HELP_CALLED(result[DEFAULT_HELP_ARG.long_name.undashed].as<bool>());

			decltype(dashedSpec) * const specs[]{
				&dashedSpec,
				&positionalSpec,
			};
			for(auto spec: specs){
				for(auto argPtr: *spec){
					if(result.count(argPtr->name)){
						auto switchCaseBodyLambda = [&] <ArgType t>() {
							auto specOptPtr = static_cast<SArg<t>*>(argPtr);
							specOptPtr->value = result[argPtr->name].as<typename GetUnderlyingTypeByEnumValue<t>::type>();
						};
						switch(argPtr->type){
							case ArgType::flag:
							{
								switchCaseBodyLambda.operator()<ArgType::flag>();
							}
							break;
							case ArgType::u1:
							{
								switchCaseBodyLambda.operator()<ArgType::u1>();
							}
							break;
							case ArgType::s1:
							{
								switchCaseBodyLambda.operator()<ArgType::s1>();
							}
							break;
							case ArgType::u2:
							{
								switchCaseBodyLambda.operator()<ArgType::u2>();
							}
							break;
							case ArgType::s2:
							{
								switchCaseBodyLambda.operator()<ArgType::s2>();
							}
							break;
							case ArgType::u4:
							{
								switchCaseBodyLambda.operator()<ArgType::u4>();
							}
							break;
							case ArgType::s4:
							{
								switchCaseBodyLambda.operator()<ArgType::s4>();
							}
							break;
							case ArgType::u8:
							{
								switchCaseBodyLambda.operator()<ArgType::u8>();
							}
							break;
							case ArgType::s8:
							{
								switchCaseBodyLambda.operator()<ArgType::s8>();
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
					} else {
						PROCESS_ARGUMENT_MISSING_CASE();
					}
				}
			}

			remaining = result.unmatched();
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
		return new CXXOptsBackend(name, descr, usage, std::move(dashedSpec), std::move(positionalSpec), streams);
	}
};
