#include <cstdlib>

#include <functional>
#include <algorithm>
#include <filesystem>
#include <unordered_map>
#include <utility>
#include <iterator>
#include <span>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

#include <HydrArgs/HydrArgs.hpp>

#include "num_utils.hpp"

#include <whereami.h>

using namespace HydrArgs;

#ifndef USE_DYLIB
	#include <function_loader/function_loader.hpp>

	using DynLibT = burda::function_loader::function_loader;
#else
	#include <dylib.hpp>

	using DynLibT = dylib;
#endif

#include "searchPaths.hpp"

using namespace HydrArgs;

namespace HydrArgs::Backend{

	const ParserCapabilities capabilities{
		.help = {
			.error_message = false,
			.help = false,
			.usage = false,
			.value_name = false,
			.value_unit = false
		},
		.native_positional = {
			.mandatory = false,
			.optional = false,
			.optional_before_mandatory = false
		},
		.dashed = {
			.mandatory = false,
			.optional = false
		},
		.bellsAndWhistles = {
			.path_validation = false,
			.auto_complete = false,
			.wide_strings = false
		},
		.important = {
			.stable = false,
			.bug_memory_safety = false,
			.bug_terminates_itself = false,
			.bug_prints_itself = false,
			.other_grave_bugs = false,
			.permissive_license = false
		}
	};

	const char ctorFuncName[] = "argsParserFactory";

	const std::string backendFileNameStemPrefix{"HydrArgs_backend_"};
	const std::string libBackendFileNameStemPrefix{"lib" + backendFileNameStemPrefix};

	const std::filesystem::path soExt{".so"};

	DynLibT *loadedLib = nullptr;
	std::function<ArgsParserFactoryT> chosenFactory;

	struct DiscoveredBackend{
		const std::string backendPath;
		ParserCapabilities caps;

		DiscoveredBackend(std::string backendPath, ParserCapabilities caps): backendPath(std::move(backendPath)), caps(caps){}
	};

	std::filesystem::path backendsPath;

	struct HydrArgsDiscovererErrorCategory: public std::error_category{
		[[nodiscard]] const char * name() const noexcept override {
			return "HydrArgs discoverer has errored";
		}
		[[nodiscard]] std::string message(int ev) const override {
			switch(static_cast<std::errc>(ev)) {
				case std::errc::no_such_file_or_directory:
					return "Backends haven't been found";
				default:
					return "Unknown HydrArgs discoverer error";
			}
		}
	};

	static HydrArgsDiscovererErrorCategory HydrArgs_discoverer_error;

	struct BackendNotDiscoveredError: public std::filesystem::filesystem_error{
		explicit BackendNotDiscoveredError(const std::filesystem::path &dir): std::filesystem::filesystem_error("No backend was discovered.", dir, std::error_code(static_cast<int>(std::errc::no_such_file_or_directory), HydrArgs_discoverer_error)){}
	};

	void initializeBackendsPath(){
		std::string myPathStr;
		auto myPathSizeS = wai_getModulePath(nullptr, 0, nullptr);
		auto myPathSizeU = toUnsigned(myPathSizeS);
		myPathStr.reserve(myPathSizeU);
		myPathStr.resize(myPathSizeU);
		wai_getModulePath(myPathStr.data(), myPathSizeS, nullptr);

		std::filesystem::path const myPath(myPathStr);
		//std::cout << "My dll path: " << myPathStr << std::endl;
		backendsPath = myPath.parent_path();
		//std::cout << "My dll parent dir: " << backendsPath << std::endl;
	}

	void loadBackend(DiscoveredBackend &backend){
		#ifdef USE_DYLIB
		loadedLib = new DynLibT(backend.backendPath, false);
		#else
		loadedLib = new DynLibT(backend.backendPath);
		#endif
		chosenFactory = loadedLib->get_function<ArgsParserFactoryT>(ctorFuncName);
	}

	using BackendsCollectionT = std::unordered_map<std::string, DiscoveredBackend>;
	static BackendsCollectionT discoveredBackends;

	void discoverBackendsFromPath(BackendsCollectionT &discoveredBackends, Streams streams, const std::filesystem::path &backendsPath, bool shouldPrintDebugOutput){
		//std::cout << "Searching for backends in dir: " << backendsPath << std::endl;
		if(!std::filesystem::is_directory(backendsPath)){
			if(shouldPrintDebugOutput){
				streams.cerr << "Backends dir path does not point to a dir: " << backendsPath << std::endl;
			}
			return;
		}
		std::filesystem::directory_iterator const iter(backendsPath);
		for(const auto &libCand: iter){
			const auto &p = libCand.path();
			std::string const stem{p.stem()};
			if(p.extension() == soExt){
				size_t prefixLen = 0;
				if(stem.starts_with(libBackendFileNameStemPrefix)){
					prefixLen = libBackendFileNameStemPrefix.size();
				} else if(stem.starts_with(backendFileNameStemPrefix)){
					prefixLen = backendFileNameStemPrefix.size();
				}

				if(prefixLen){
					auto name = stem.substr(prefixLen);
					if(shouldPrintDebugOutput){
						streams.cerr << "Probing candidate: " << stem << std::endl;
					}
					std::string libPathStr{p};

					#ifdef USE_DYLIB
					DynLibT const candidateLibrary(libPathStr.data(), false);
					auto caps = candidateLibrary.get_variable<ParserCapabilities>("capabilities");
					#else
					DynLibT candidateLibrary(libPathStr.data());
					auto &caps = *((ParserCapabilities *) candidateLibrary.get_function_address("capabilities"));
					#endif

					if(shouldPrintDebugOutput){
						streams.cerr << "Candidate score: " << std::hex << caps.getScore() << std::endl;
					}
					discoveredBackends.emplace(name, DiscoveredBackend{libPathStr, caps});
				}
			}
		}
	}

	void discoverBackends(BackendsCollectionT &discoveredBackends, Streams streams, bool shouldPrintDebugOutput){
		if(backendsPath.empty()){
			initializeBackendsPath();
		}
		for(const auto &parentDir: std::span(searchPaths, searchPathsCount)){
			discoverBackendsFromPath(discoveredBackends, streams, backendsPath / parentDir, shouldPrintDebugOutput);
		}

		/*#if defined(HYDRARGS_SCAN_BACKENDS_DIR)
		discoverBackendsFromPath(discoveredBackends, streams, backendsPath.parent_path() / "backends");
		#endif*/
		if(discoveredBackends.empty()){
			throw BackendNotDiscoveredError(backendsPath);
		}
	}

	constexpr const char backendEnvVar[] = "HYDRARGS_BACKEND";
	constexpr const char discovererDebugEnvVar[] = "HYDRARGS_DISCOVERER_DEBUG";

	//decltype(discoveredBackends)::node_type chooseBackend(){
	auto chooseBackend(BackendsCollectionT &discoveredBackends, Streams streams [[maybe_unused]], bool shouldPrintDebugOutput [[maybe_unused]] =false) -> std::pair<const std::string, DiscoveredBackend&>{
		auto *envVar = getenv(const_cast<char *>(backendEnvVar));
		if(envVar){
			auto res = discoveredBackends.find(envVar);
			if(res != end(discoveredBackends)){
				return {res->first, res->second};
			}
		}

		auto el = std::max_element(begin(discoveredBackends), end(discoveredBackends), [](std::pair<const std::string, DiscoveredBackend> &a, std::pair<const std::string, DiscoveredBackend> &b) -> bool{
			return b.second.caps > a.second.caps;
		});
		return {el->first, el->second};
	}

	IArgsParser * argsParserFactory(const std::string &name, const std::string &descr, const std::string &usage [[maybe_unused]], const std::vector<Arg *> &dashedSpec, const std::vector<Arg *> &positionalSpec, Streams streams){
		bool const shouldPrintDebugOutput = getenv(const_cast<char *>(discovererDebugEnvVar));
		if(discoveredBackends.empty()){
			discoverBackends(discoveredBackends, streams, shouldPrintDebugOutput);
		}
		auto b = chooseBackend(discoveredBackends, streams, shouldPrintDebugOutput);
		if(shouldPrintDebugOutput){
			streams.cerr << "Selected backend: " << b.first << std::endl;
		}
		loadBackend(b.second);
		return chosenFactory(name, descr, usage, std::move(dashedSpec), std::move(positionalSpec), streams);
	}
}  // namespace HydrArgs::Backend;
