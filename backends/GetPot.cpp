#include <cstdint>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <HydrArgs/HydrArgs.hpp>
#include <HydrArgs/fallback/errors.hpp>
#include <HydrArgs/fallback/positional/iterative.hpp>
#include <HydrArgs/toolbox.hpp>

#include "num_utils.hpp"

#include <GetPot.hpp>

using namespace HydrArgs;

namespace HydrArgs::Backend{
struct GetPotBackend: public IBackendOwnStoredSpec, public fallback::positional::PositionalParserIterative{
	const std::string &name;
	const std::string &descr;
	const std::string &usage;

	GetPotBackend(const std::string &name, const std::string &descr, const std::string &usage [[maybe_unused]], std::vector<Arg*> dashedSpec, std::vector<Arg*> positionalSpec, Streams streams): IBackendOwnStoredSpec(dashedSpec, positionalSpec, streams), PositionalParserIterative(this), name(name), descr(descr), usage(usage){
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
		}
	}

	virtual ~GetPotBackend() override = default;

	virtual bool isNotEnoughArgs(uint32_t positionalArgCountLowerBound) const override {
		//return !app(positionalArgCountLowerBound);
		return false;
	}

	virtual void printHelp(std::ostream &stream, const char *const argv0) override {
		stream << argv0 << " " << usage << std::endl;
		stream << descr << std::endl;
	}

	virtual ParseResult _parseArgs(CLIRawArgs rawArgs) override {

		auto argvVec = rawArgs.getArgvVector();
		auto app = GetPot(checked_cast<int>(argvVec.size() - 1), const_cast<char **>(argvVec.data()));

		PROCESS_CASE_OF_HELP_CALLED(app.search(2, "--help", "-h"));

		HYDRARGS_CHECK_IF_ENOUGH_MAND_POS_ARGS_AND_RETURN_ERROR_OTHERWISE();

		resetPositionalParsing();
		getNextPosArgIdx();  // indices start from 1

		auto searchLambda = [&](Arg *argPtr) -> bool {
			if(argPtr->letter){
				return app.search(2, argPtr->letterStr, ("--" + argPtr->name).data());
			} else {
				return app.search(("--" + argPtr->name).data());
			}
		};

		std::pair<decltype(dashedSpec)&, bool> specs[]{
			{dashedSpec, false},
			{positionalSpec, true},
		};
		for(auto p: specs){
			auto spec = p.first;
			auto isPositional = p.second;
			for(auto &argPtr: spec){
				if(argPtr->type == ArgType::flag && !isPositional){
					auto specOptPtr = static_cast<SArg<ArgType::flag>*>(argPtr);
					specOptPtr->value = searchLambda(argPtr);

					if(!specOptPtr->value){
						PROCESS_ARGUMENT_MISSING_CASE();
					}

					continue;
				}

				{
					if(!searchLambda(argPtr)){
						PROCESS_ARGUMENT_MISSING_CASE();
					} else {
						auto switchCaseBodyLambda = [&] <ArgType t>() {
							auto specOptPtr = static_cast<SArg<t>*>(argPtr);
							if constexpr (t == ArgType::string){
								const char *tres = app.next(const_cast<const char *>(specOptPtr->value.data()));
								if(tres != specOptPtr->value.data()){
									specOptPtr->value = tres;
								}
							} else {
								specOptPtr->value = app.next(specOptPtr->value);
							}
						};
						switch(argPtr->type){
							case ArgType::flag:
							{
								switchCaseBodyLambda.operator()<ArgType::flag>();
							}
							break;
							/*case ArgType::s1:
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
							break;*/
							case ArgType::s4:
							{
								switchCaseBodyLambda.operator()<ArgType::s4>();
							}
							break;
							/*case ArgType::u4:
							{
								switchCaseBodyLambda.operator()<ArgType::u4>();
							}
							break;
							case ArgType::s8:
							{
								switchCaseBodyLambda.operator()<ArgType::s8>();
							}
							break;*/
							/*case ArgType::u8:
							{
								switchCaseBodyLambda.operator()<ArgType::u8>();
							}
							break;
							case ArgType::f4:
							{
								switchCaseBodyLambda.operator()<ArgType::f4>();
							}
							break;*/
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

		//std::transform(begin(remaining), end(remaining), begin(remainingPtrs), [](std::string &el) -> char * {return el.data();});
		return ParseResult{
			.parsingStatus = STATUS_OK,
			.rest = {
				.argv0 = rawArgs.argv0,
				//.argvRest = {const_cast<const char **>(remainingPtrs.data()), remainingPtrsSize}
				.argvRest = {static_cast<const char **>(nullptr), 0}
			}
		};
	}
};

IArgsParser * argsParserFactory(const std::string &name, const std::string &descr, const std::string &usage [[maybe_unused]], const std::vector<Arg *> &dashedSpec, const std::vector<Arg *> &positionalSpec, Streams streams){
	return new GetPotBackend(name, descr, usage, dashedSpec, positionalSpec, streams);
}
};
