#include <HydrArgs/HydrArgs.hpp>
#include <HydrArgs/toolbox.hpp>

#include <tclap/CmdLine.h>

#include <cassert>
#include <memory>
#include <span>

using namespace HydrArgs;

namespace HydrArgs::Backend{

	struct TCLAPBackend: public IBackendOwnStoredSpec{
		CLIRawArgs::VectorT tempArgvVec;  /// DON'T MOVE IT LOWER THAN TCLAP::CmdLine, otherwise use-after-free will happen!

		TCLAP::CmdLine app;
		std::vector<std::string> remaining;
		std::vector<char *> remainingPtrs;

		TCLAPBackend(const std::string &name [[maybe_unused]], const std::string &descr, const std::string &usage [[maybe_unused]], std::vector<Arg*> dashedSpec, std::vector<Arg*> positionalSpec, Streams streams): IBackendOwnStoredSpec(dashedSpec, positionalSpec, streams), app(descr){
			app.setExceptionHandling(false);
			app.ignoreUnmatched(true);

			for(auto argPtr: dashedSpec){
				_addArg(argPtr, false);
			}
			for(auto argPtr: positionalSpec){
				_addArg(argPtr, true);
			}
		}

		virtual void _addArg(Arg *argPtr, bool isPositional) override {
			TCLAP::Arg *opt = nullptr;
			std::string valueName;
			if(argPtr->value_name){
				valueName = argPtr->value_name;
			}
			auto switchCaseBodyLambda = [&] <ArgType t>() {
				auto specOptPtr = static_cast<SArg<t>*>(argPtr);
				if(isPositional){
					opt = new TCLAP::UnlabeledValueArg<typename GetUnderlyingTypeByEnumValue<t>::type>(argPtr->name, argPtr->description, argPtr->minCount, specOptPtr->getValueRef(), valueName);
				} else {
					opt = new TCLAP::ValueArg<typename GetUnderlyingTypeByEnumValue<t>::type>("", argPtr->name, argPtr->description, argPtr->minCount, specOptPtr->getValueRef(), valueName);
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
			}
			app.add(opt);
		}
		virtual ~TCLAPBackend() override {
			for(auto nativeArgPtr: app.getArgList()){
				delete nativeArgPtr;
			}
		}

		virtual void printHelp(std::ostream &stream, const char *const argv0 [[maybe_unused]]) override {
			//stream << app.getMessage() << std::endl;
		}

		virtual ParseResult _parseArgs(CLIRawArgs rawArgs) override {
			tempArgvVec = rawArgs.getArgvVector();
			try{
				app.parse(tempArgvVec.size() - 1, tempArgvVec.data());
				//app.parse(tempArgvVec);  // not suitable, requires a vector of std::string s, not pointers to C-strings
			} catch(TCLAP::ArgException &e) {
				PROCESS_CASE_OF_SYNTAX_ERROR_MESSAGE_WITH_KNOWN_LOCATION(e.argId(), e.error());
			} catch(TCLAP::ExitException &ee) {
				auto s = ee.getExitStatus();
				if(!s){
					// Assumme it is help. Need a dedicated exception for this case. This exception with error code of 0 is throw for version and help.
					return RESULT_HELP_CALLED;
				} else {
					return {
						.parsingStatus = {s, EVENTS_SYNTAX_ERROR},
						.rest = NO_ARGS
					};
				}
			}

			auto aList = app.getArgList();
			assert(aList.size() > dashedSpec.size());
			assert(aList.size() - 3 == dashedSpec.size());

			auto listIter = rbegin(aList);// Impl detail: args are added into the beginning, so the order is reversed. The pre-added args are in the end. There are 3 pre-added args (here are in reversed order!):
			++listIter;  // --
			++listIter;  // --version
			++listIter;  // --help

			decltype(dashedSpec)* specs[]{
				&dashedSpec,
				&positionalSpec
			};
			for(auto specP: specs){
				auto & spec = *specP;
				for(auto i = 0u; i < spec.size(); ++i){
					auto &nativeArg = *listIter;
					auto argPtr = spec[i];
					auto switchCaseBodyLambda = [&] <ArgType t>() {
						auto specOptPtr = static_cast<SArg<t>*>(argPtr);
						auto tOptPtr = static_cast<TCLAP::ValueArg<typename GetUnderlyingTypeByEnumValue<t>::type> *>(nativeArg);
						specOptPtr->value = tOptPtr->getValue();
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
					}
					++listIter;
				}
			}

			//std::transform(begin(remaining), end(remaining), begin(remainingPtrs), [](std::string &el) -> char * {return el.data();});
			return {
				.parsingStatus = STATUS_OK,
				.rest = {
					.argv0 = rawArgs.argv0,
					.argvRest = NO_REST_ARGS
				}
			};
		}
	};

	IArgsParser * argsParserFactory(const std::string &name, const std::string &descr, const std::string &usage [[maybe_unused]], std::vector<Arg*> dashedSpec, std::vector<Arg*> positionalSpec, Streams streams){
		return new TCLAPBackend(name, descr, usage, dashedSpec, positionalSpec, streams);
	}
};
