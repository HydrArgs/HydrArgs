#include <iosfwd>
#include <string>
#include <vector>

#include <HydrArgs/HydrArgs.hpp>

using namespace HydrArgs;

namespace HydrArgs::Backend{

	/// A dummy backend doing nothing neeeded to debug issues in the framework itself
	struct HYDRARGS_BACKEND_API Dummy: public IArgsParser {
		Dummy(const std::string &name [[maybe_unused]], const std::string &descr [[maybe_unused]], const std::string &usage [[maybe_unused]], const std::vector<Arg *> &dashedSpec [[maybe_unused]], const std::vector<Arg*> &positionalSpec [[maybe_unused]], Streams streams): IArgsParser(streams){
		}

		void _addArg(Arg *argPtr [[maybe_unused]], bool isPositional [[maybe_unused]]) override {
		}
		~Dummy() override = default;

		void printHelp(std::ostream &stream [[maybe_unused]], const char *const argv0 [[maybe_unused]]) override {
		}

		ParseResult _parseArgs(CLIRawArgs rawArgs) override {
			return {
				.parsingStatus = STATUS_OK,
				.rest = rawArgs
			};
		}
	};

	IArgsParser * argsParserFactory(const std::string &name, const std::string &descr, const std::string &usage [[maybe_unused]], const std::vector<Arg *> &dashedSpec, const std::vector<Arg *> &positionalSpec, Streams streams){
		return new Dummy(name, descr, usage, dashedSpec, positionalSpec, streams);
	}

};
