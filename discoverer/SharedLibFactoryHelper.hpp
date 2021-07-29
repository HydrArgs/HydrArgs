#pragma once

#include <string>

#ifdef USE_DYLIB
	#include <dylib.hpp>
#else
	#include <function_loader/function_loader.hpp>
#endif

template <typename FuncT> struct SharedLibFactory{
	#ifdef USE_DYLIB
	using DynLibT = dylib;
	#else
	using DynLibT = burda::function_loader::function_loader;
	#endif


	DynLibT l;
	std::function<FuncT> f;

	SharedLibFactory(const char * libraryName, const char * funcName): l(
		#ifdef USE_DYLIB
		DynLibT(libraryName, false)
		#else
		DynLibT(libraryName)
		#endif
	){
		f = l.get_function<FuncT>(funcName);
	}

	template<typename... Args>
	typename std::invoke_result<FuncT>::type operator()(Args...){
		return f(...Args);
	}
};
