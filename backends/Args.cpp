#include <stddef.h>
#include <memory>
#include <algorithm>
#include <ostream>
#include <string>
#include <vector>

#include <HydrArgs/HydrArgs.hpp>

#include "num_utils.hpp"

#include <args.hxx>

using namespace HydrArgs;

struct ArgsBackend: public IBackendOwnStoredSpec{
	args::ArgumentParser app;
	std::vector<std::string> remaining;
	std::vector<char *> remainingPtrs;
	std::vector<std::unique_ptr<args::NamedBase>> nativeArgs;

	ArgsBackend(const std::string &name [[maybe_unused]], const std::string &descr, const std::string &usage [[maybe_unused]], std::vector<Arg*> dashedSpec, std::vector<Arg*> positionalSpec, Streams streams): IBackendOwnStoredSpec(dashedSpec, positionalSpec, streams), app(descr){
		args::HelpFlag help(app, DEFAULT_HELP_ARG.long_name.undashed, DEFAULT_HELP_ARG.doc, {DEFAULT_HELP_ARG.letter_str.getLetter(), DEFAULT_HELP_ARG.long_name.undashed});
		args::CompletionFlag completion(app, {"complete"});

		for(auto argPtr: dashedSpec){
			_addArg(argPtr, false);
		}
		for(auto argPtr: positionalSpec){
			_addArg(argPtr, true);
		}
	}

	virtual void _addArg(Arg *argPtr, bool isPositional) override {
		if(isPositional){
			switch(argPtr->type){
				case ArgType::flag:
				{
					nativeArgs.emplace_back(std::make_unique<args::Positional<bool>>(app, argPtr->name, argPtr->description, args::Options::Required | args::Options::Single));
				}
				break;
				case ArgType::s4:
				{
					nativeArgs.emplace_back(std::make_unique<args::Positional<int>>(app, argPtr->name, argPtr->description, args::Options::Required | args::Options::Single));
				}
				break;
				case ArgType::string:
				case ArgType::path:
				{
					nativeArgs.emplace_back(std::make_unique<args::Positional<std::string>>(app, argPtr->name, argPtr->description, args::Options::Required | args::Options::Single));
				}
				break;
				default:
					throw UnsupportedArgumentType{argPtr->type};
			}
		} else {
			switch(argPtr->type){
				case ArgType::flag:
				{
					auto specOptPtr = static_cast<SArg<ArgType::flag>*>(argPtr);
					//std::make_unique doesn't work here
					// ToDo: it can be a sign of a worser issue
					auto ptr = new args::ValueFlag<decltype(specOptPtr->value)>(app, argPtr->name, argPtr->description, {argPtr->name, argPtr->letter}, specOptPtr->value, args::Options::Required | args::Options::Single);
					nativeArgs.emplace_back(ptr);
				}
				break;
				case ArgType::s4:
				{
					auto specOptPtr = static_cast<SArg<ArgType::s4>*>(argPtr);
					auto ptr = new args::ValueFlag<decltype(specOptPtr->value)>(app, argPtr->name, argPtr->description, {argPtr->name, argPtr->letter}, specOptPtr->value, args::Options::Required | args::Options::Single);
					nativeArgs.emplace_back(ptr);
				}
				break;
				case ArgType::string:
				case ArgType::path:
				{
					auto specOptPtr = static_cast<SArg<ArgType::string>*>(argPtr);
					auto ptr = new args::ValueFlag<decltype(specOptPtr->value)>(app, argPtr->name, argPtr->description, {argPtr->name, argPtr->letter}, specOptPtr->value, args::Options::Required | args::Options::Single);
					nativeArgs.emplace_back(ptr);
				}
				break;
				default:
					throw UnsupportedArgumentType{argPtr->type};
			}
		}
	}

	virtual void printHelp(std::ostream &stream, const char * const argv0 [[maybe_unused]]) override {
		stream << app;
	}

	virtual ~ArgsBackend() override = default;
	virtual ParseResult _parseArgs(CLIRawArgs rawArgs) override {
		try {
			auto tempArgvVec = rawArgs.getArgvVector();
			app.ParseCLI(checked_cast<int>(tempArgvVec.size() - 1), tempArgvVec.data());
		} catch(const args::Completion &e) {
			streams.cout << e.what();
			return RESULT_AUTOCOMPLETE_CALLED;
		} catch(const args::Help &e) {
			printHelp(streams.cout, rawArgs.argv0);
			return RESULT_HELP_CALLED;
		} catch(const args::ParseError &e) {
			streams.cerr << e.what() << std::endl;
			printHelp(streams.cerr, rawArgs.argv0);
			return RESULT_SYNTAX_ERROR;
		}

		size_t i = 0;
		for(auto &nativeArg: nativeArgs){
			auto argPtr = dashedSpec[i];
			switch(argPtr->type){
				case ArgType::flag:
				{
					auto specOptPtr = static_cast<SArg<ArgType::flag> *>(argPtr);
					args::ValueFlag<decltype(specOptPtr->value)> &tOptPtr = static_cast<args::ValueFlag<decltype(specOptPtr->value)>&>(*nativeArg);
					specOptPtr->value = tOptPtr.Get();
				}
				break;
				case ArgType::s4:
				{
					auto specOptPtr = static_cast<SArg<ArgType::s4> *>(argPtr);
					args::ValueFlag<decltype(specOptPtr->value)> &tOptPtr = static_cast<args::ValueFlag<decltype(specOptPtr->value)>&>(*nativeArg);
					specOptPtr->value = tOptPtr.Get();
				}
				break;
				case ArgType::string:
				case ArgType::path:
				{
					auto specOptPtr = static_cast<SArg<ArgType::string> *>(argPtr);
					args::ValueFlag<decltype(specOptPtr->value)> &tOptPtr = static_cast<args::ValueFlag<decltype(specOptPtr->value)>&>(*nativeArg);
					specOptPtr->value = tOptPtr.Get();
				}
				break;
				default:
					throw UnsupportedArgumentType{argPtr->type};
			}
			++i;
		}

		i = 0;
		for(auto &nativeArg: nativeArgs){
			auto argPtr = positionalSpec[i];
			switch(argPtr->type){
				case ArgType::flag:
				{
					auto specOptPtr = static_cast<SArg<ArgType::flag> *>(argPtr);
					args::Positional<decltype(specOptPtr->value)> &tOptPtr = static_cast<args::Positional<decltype(specOptPtr->value)>&>(*nativeArg);
					specOptPtr->value = tOptPtr.Get();
				}
				break;
				case ArgType::s4:
				{
					auto specOptPtr = static_cast<SArg<ArgType::s4> *>(argPtr);
					args::Positional<decltype(specOptPtr->value)> &tOptPtr = static_cast<args::Positional<decltype(specOptPtr->value)>&>(*nativeArg);
					specOptPtr->value = tOptPtr.Get();
				}
				break;
				case ArgType::string:
				case ArgType::path:
				{
					auto specOptPtr = static_cast<SArg<ArgType::string> *>(argPtr);
					args::Positional<decltype(specOptPtr->value)> &tOptPtr = static_cast<args::Positional<decltype(specOptPtr->value)>&>(*nativeArg);
					specOptPtr->value = tOptPtr.Get();
				}
				break;
				default:
					throw UnsupportedArgumentType{argPtr->type};
			}
			++i;
		}

		//std::transform(begin(remaining), end(remaining), begin(remainingPtrs), [](std::string &el) -> char * {return el.data();});
		return {
			.parsingStatus = STATUS_OK,
			.rest = {
				.argv0=rawArgs.argv0,
				.argvRest = {static_cast<const char **>(nullptr), 0}
			}
		};
	}
};

// for autocompletion call `./bin/testabs-runner --complete bash 1 ./bin/testabs-runner --fuck=`

IArgsParser * argsParserFactory(const std::string &name, const std::string &descr, const std::string &usage [[maybe_unused]], const std::vector<Arg *> &dashedSpec, const std::vector<Arg *> &positionalSpec, Streams streams){
	return new ArgsBackend(name, descr, usage, dashedSpec, positionalSpec, streams);
}
