#include <cstdint>
#include <iostream>
#include <utility>
#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

#include <HydrArgs/HydrArgs.hpp>
#include <HydrArgs/fallback/errors.hpp>
#include <HydrArgs/fallback/positional/iterative.hpp>
#include <HydrArgs/toolbox.hpp>

#include "num_utils.hpp"

#include <argh.h>

using namespace HydrArgs;
using namespace HydrArgs::fallback::positional;

namespace HydrArgs::Backend{
	struct ArghBackend: public IBackendOwnStoredSpec, public PositionalParserIterative{
		argh::parser app;
		const std::string &name;
		const std::string &descr;
		const std::string &usage;

		std::vector<std::string> remaining;
		std::vector<char *> remainingPtrs;

		ArghBackend(const std::string &name, const std::string &descr, const std::string &usage [[maybe_unused]], const std::vector<Arg*>& dashedSpec, const std::vector<Arg*>& positionalSpec, Streams streams): IBackendOwnStoredSpec(dashedSpec, positionalSpec, streams), PositionalParserIterative(this), name(name), descr(descr), usage(usage){
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
				// preregister non-flag args
				if(argPtr->type != ArgType::flag){
					app.add_param(argPtr->name);
					if(argPtr->letter){
						app.add_param(std::string{argPtr->letter});
					}
				}
			}
		}

		virtual ~ArghBackend() override = default;

		[[nodiscard]] virtual bool isNotEnoughArgs(uint32_t positionalArgCountLowerBound) const override {
			return !app(positionalArgCountLowerBound);
		}

		void printHelp(std::ostream &stream, const char * const argv0) override {
			stream << argv0 << " " << usage << std::endl;
			stream << descr << std::endl;
		}

		inline argh::string_stream getNativePosArgType(){
			return app.operator()(getNextPosArgIdx());
		}

		inline argh::string_stream getNativeDashedArgType(Arg *argPtr) const{
			return app(argPtr->name);
		}

		virtual ParseResult _parseArgs(CLIRawArgs rawArgs) override {

			{
				auto tempArgvVec = rawArgs.getArgvVector();
				app.parse(checked_cast<int>(tempArgvVec.size() - 1), tempArgvVec.data());
			}

			PROCESS_CASE_OF_HELP_CALLED(app[DEFAULT_HELP_ARG.long_name.undashed] || app(DEFAULT_HELP_ARG.long_name.undashed));

			HYDRARGS_CHECK_IF_ENOUGH_MAND_POS_ARGS_AND_RETURN_ERROR_OTHERWISE();

			resetPositionalParsing();
			getNextPosArgIdx();  // indices start from 1

			std::pair<decltype(dashedSpec)&, bool> const specs[]{
				{dashedSpec, false},
				{positionalSpec, true},
			};
			for(auto p: specs){
				auto spec = p.first;
				auto isPositional = p.second;
				for(auto &argPtr: spec){
					if(argPtr->type == ArgType::flag && !isPositional){
						auto specOptPtr = static_cast<SArg<ArgType::flag>*>(argPtr);
						specOptPtr->value = app[argPtr->name];

						if(!specOptPtr->value){
							PROCESS_ARGUMENT_MISSING_CASE();
						}

						continue;
					}

					{
						argh::string_stream curArgObj = (isPositional ? getNativePosArgType() : app(argPtr->name));
						if(!curArgObj){
							PROCESS_ARGUMENT_MISSING_CASE();
						} else {
							auto switchCaseBodyLambda = [&] <ArgType t>() {
								auto specOptPtr = static_cast<SArg<t>*>(argPtr);
								curArgObj >> specOptPtr->value;
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
					}
				}
			}

			while(auto curArgObj = app(getNextPosArgIdx())){
				std::string curArg;
				curArgObj >> curArg;
				remaining.emplace_back(curArg);
			}
			remaining.shrink_to_fit();

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

	IArgsParser *argsParserFactory(const std::string &name, const std::string &descr, const std::string &usage [[maybe_unused]], const std::vector<Arg *> &dashedSpec, const std::vector<Arg *> &positionalSpec, Streams streams){
		return new ArghBackend(name, descr, usage, std::move(dashedSpec), std::move(positionalSpec), streams);
	}
}  // namespace HydrArgs::Backend;
