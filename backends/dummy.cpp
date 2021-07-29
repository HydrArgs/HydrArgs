#include <HydrArgs/HydrArgs.hpp>
#include <HydrArgs/toolbox.hpp>

using namespace HydrArgs;

namespace HydrArgs::Backend{

	/// A dummy backend doing nothing neeeded to debug issues in the framework itself
	struct HYDRARGS_BACKEND_API Dummy: public IArgsParser {
		Dummy(const std::string &name [[maybe_unused]], const std::string &descr [[maybe_unused]], const std::string &usage [[maybe_unused]], std::vector<Arg *> dashedSpec [[maybe_unused]], std::vector<Arg*> positionalSpec [[maybe_unused]], Streams streams): IArgsParser(streams){
		}

		virtual void _addArg(Arg *argPtr [[maybe_unused]], bool isPositional [[maybe_unused]]) override {
		}
		virtual ~Dummy() override = default;

		virtual void printHelp(std::ostream &stream [[maybe_unused]], const char *const argv0 [[maybe_unused]]) override {
		}

		virtual ParseResult _parseArgs(CLIRawArgs rawArgs) override {
			return {
				.parsingStatus = STATUS_OK,
				.rest = rawArgs
			};
		}
	};

	IArgsParser * argsParserFactory(const std::string &name, const std::string &descr, const std::string &usage [[maybe_unused]], std::vector<Arg*> dashedSpec, std::vector<Arg*> positionalSpec, Streams streams){
		return new Dummy(name, descr, usage, dashedSpec, positionalSpec, streams);
	}

};
