#include <cstdlib>
#include <cstdio>

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <HydrArgs/HydrArgs.hpp>

using namespace HydrArgs;
using namespace HydrArgs::Backend;

constexpr const char description[] = "Demo";
constexpr const char usage[] = "[-?|-h|--help] [--odf|-a] [--odi <int>] [--ods|-c <string>] [<flag>] [<int>] [<string>]";  // from Lyra backend

constexpr const char programName[] = "HydrArgs demo";

int main(int argc, const char **argv){
	SArg<ArgType::flag> odfA{'a', "odf", "opt dashed flag", 0, "flag", "units", false};
	SArg<ArgType::s4> odiA{'b', "odi", "opt dashed int", 0, "int", "units", 0};
	SArg<ArgType::string> odsA{'c', "ods", "opt dashed str", 0, "string", "units", ""};

	/*SArg<ArgType::flag> mdfA{'d', "mdf", "mand dashed flag", 1, "flag", "units", false};
	SArg<ArgType::s4> mdiA{'e', "mdi", "mand dashed int", 1, "int", "units", 0};
	SArg<ArgType::string> mdsA{'f', "mds", "mand dashed str", 1, "string", "units", ""};

	SArg<ArgType::flag> mpfA{'g', "mpf", "mand pos flag", 1, "flag", "units", false};
	SArg<ArgType::s4> mpiA{'i', "mpi", "mand pos int", 1, "int", "units", 0};
	SArg<ArgType::string> mpsA{'j', "mps", "mand pos str", 1, "string", "units", ""};*/

	SArg<ArgType::flag> opfA{'k', "opf", "opt pos flag", 0, "flag", "units", false};
	SArg<ArgType::s4> opiA{'l', "opi", "opt pos int", 0, "int", "units", 0};
	SArg<ArgType::string> opsA{'m', "ops", "opt pos str", 0, "string", "units", ""};

	std::vector<Arg*> const dashedSpec{
		&odfA, &odiA, &odsA,
		//&mdfA, &mdiA, &mdsA
	};

	std::vector<Arg*> const positionalSpec{
		//&mpfA, &mpiA, &mpsA,
		&opfA, &opiA, &opsA,
	};

	std::unique_ptr<IArgsParser> ap{argsParserFactory(programName, description, usage, dashedSpec, positionalSpec)};

	if(!ap){
		std::cerr << "Cannot initialize CLI parsing library!" << std::endl;
		return EXIT_FAILURE;
	}

	auto status = (*ap)({argv[0], {&argv[1], static_cast<size_t>(argc - 1)}});

	if(status.parsingStatus){
		return status.parsingStatus.returnCode;
	}

	std::cout << "opi=" << opiA.value << ", ops=" << opsA.value << std::endl;

	return status.parsingStatus.returnCode;
}
