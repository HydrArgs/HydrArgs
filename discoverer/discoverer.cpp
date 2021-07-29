#include <algorithm>
#include <filesystem>
#include <iostream>
#include <unordered_map>

#include <whereami.h>

#include <cassert>

#include <HydrArgs/HydrArgs.hpp>

using namespace HydrArgs;

#define USE_DYLIB
#ifndef USE_DYLIB
	#include <function_loader/function_loader.hpp>
	using DynLibT = burda::function_loader::function_loader;
#else
	#include <DyLib.hpp>
	using DynLibT = DyLib;
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

		DiscoveredBackend(const std::string &backendPath, ParserCapabilities caps): backendPath(backendPath), caps(caps){}
	};

	std::filesystem::path backendsPath;

	struct HydrArgsDiscovererErrorCategory: public std::error_category{
		virtual const char * name() const noexcept override {
			return "HydrArgs discoverer has errored";
		}
		virtual std::string message(int ev) const override {
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
		BackendNotDiscoveredError(const std::filesystem::path &dir): std::filesystem::filesystem_error("No backend was discovered.", dir, std::error_code(static_cast<int>(std::errc::no_such_file_or_directory), HydrArgs_discoverer_error)){}
	};

	void initializeBackendsPath(){
		std::string myPathStr;
		auto myPathSize = wai_getModulePath(nullptr, 0, nullptr);
		myPathStr.reserve(myPathSize);
		myPathStr.resize(myPathSize);
		wai_getModulePath(myPathStr.data(), myPathSize, nullptr);

		std::filesystem::path myPath(myPathStr);
		//std::cout << "My dll path: " << myPathStr << std::endl;
		backendsPath = myPath.parent_path();
		//std::cout << "My dll parent dir: " << backendsPath << std::endl;
	}

	void loadBackend(DiscoveredBackend &backend){
		loadedLib = new DynLibT(backend.backendPath);
		#ifndef USE_DYLIB
			chosenFactory = loadedLib->get_function<ArgsParserFactoryT>(ctorFuncName);
		#else
			chosenFactory = loadedLib->getFunction<ArgsParserFactoryT>(ctorFuncName);
		#endif
	}

	typedef std::unordered_map<std::string, DiscoveredBackend> BackendsCollectionT;
	static BackendsCollectionT discoveredBackends;

	void discoverBackendsFromPath(BackendsCollectionT &discoveredBackends, Streams streams, const std::filesystem::path &backendsPath, bool shouldPrintDebugOutput){
		//std::cout << "Searching for backends in dir: " << backendsPath << std::endl;
		if(!std::filesystem::is_directory(backendsPath)){
			if(shouldPrintDebugOutput){
				streams.cerr << "Backends dir path does not point to a dir: " << backendsPath << std::endl;
			}
			return;
		}
		std::filesystem::directory_iterator iter(backendsPath);
		for(auto libCand: iter){
			auto p = libCand.path();
			std::string stem{p.stem()};
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

					DynLibT candidateLibrary(libPathStr.data());
					auto caps = candidateLibrary.getVariable<ParserCapabilities>("capabilities");
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
		for(auto &parentDir: std::span(searchPaths, searchPathsCount)){
			discoverBackendsFromPath(discoveredBackends, streams, backendsPath / parentDir, shouldPrintDebugOutput);
		}

		/*#if defined(HYDRARGS_SCAN_BACKENDS_DIR)
		discoverBackendsFromPath(discoveredBackends, streams, backendsPath.parent_path() / "backends");
		#endif*/
		if(discoveredBackends.size() == 0){
			throw BackendNotDiscoveredError(backendsPath);
		}
	}

	constexpr const char backendEnvVar[] = "HYDRARGS_BACKEND";
	constexpr const char discovererDebugEnvVar[] = "HYDRARGS_DISCOVERER_DEBUG";

	//decltype(discoveredBackends)::node_type chooseBackend(){
	std::pair<const std::string, DiscoveredBackend&> chooseBackend(BackendsCollectionT &discoveredBackends, Streams streams [[maybe_unused]], bool shouldPrintDebugOutput [[maybe_unused]] =false){
		auto envVar = getenv(const_cast<char *>(backendEnvVar));
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

	IArgsParser * argsParserFactory(const std::string &name, const std::string &descr, const std::string &usage, std::vector<Arg*> dashedSpec, std::vector<Arg*> positionalSpec, Streams streams){
		bool shouldPrintDebugOutput = getenv(const_cast<char *>(discovererDebugEnvVar));
		if(!discoveredBackends.size()){
			discoverBackends(discoveredBackends, streams, shouldPrintDebugOutput);
		}
		auto b = chooseBackend(discoveredBackends, streams, shouldPrintDebugOutput);
		if(shouldPrintDebugOutput){
			streams.cerr << "Selected backend: " << b.first << std::endl;
		}
		loadBackend(b.second);
		return chosenFactory(name, descr, usage, dashedSpec, positionalSpec, streams);
	}
};
