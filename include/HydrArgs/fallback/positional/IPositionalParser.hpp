#pragma once

#include <type_traits>

#include "../../HydrArgs.hpp"

#ifdef HYDRARGS_FALLBACK_POSITIONAL_EXPORTS
	#define HYDRARGS_FALLBACK_POSITIONAL_API HYDRARGS_EXPORT_API
	#define HYDRARGS_FALLBACK_POSITIONAL_NOT_USER_FACING
#else
	#define HYDRARGS_FALLBACK_POSITIONAL_API HYDRARGS_IMPORT_API
	#define HYDRARGS_FALLBACK_POSITIONAL_NOT_USER_FACING [[deprecated("This method is probably not for you! It is for our purposes and should be overridden and called in HydrArgs itself")]]
#endif

namespace HydrArgs::fallback::positional{

	struct HYDRARGS_FALLBACK_POSITIONAL_API ValidationState{
		bool posOptStarted: 1 = false,
		posStarted: 1 = false;
	};

	struct IPositionalParserClient;

	/// Our implementation of positional arguments state tracker. Inherit this class in your backend library, if it is needed. The easiest way to use it is multiple inheritance along with `IArgsParser`  Implement the pure methods. Then call the methods defined in it from `_addArg` and `_parseArgs`.
	struct HYDRARGS_FALLBACK_POSITIONAL_API IPositionalParser: public IArgsParserClient{
		IPositionalParser(IArgsParser *parent);
		virtual ~IPositionalParser();

		virtual void addPosArg(Arg *argPtr) = 0;  ///< Call this method before/after registering a positional argument in your `IArgsParser::_addArg`
		inline virtual bool isNotEnoughArgs(uint32_t positionalArgCountLowerBound) const = 0;  ///< Implement this method in your inherited class. It must return `true` if amount of positional arguments is less than the supplied `positionalArgCountLowerBound`, and otherwise return `false`.

		HYDRARGS_FALLBACK_POSITIONAL_NOT_USER_FACING inline virtual uint32_t getMandatoryPositionalArgsCount() const = 0;  ///< Returns the count of mandatory positional args. This method is not for users.

		struct ParseEvents checkIfEnoughMandPosArgs(const char *argv0);  ///< Call this method before/after registering a positional argument in your `IArgsParser::_parseArgs`. If it returns an error, return from `_parseArgs` filling the struct with the supplied error code.

		uint32_t positionalCounter;  ///<Count of positional arguments already processed one by one and verified
		uint32_t getNextPosArgIdx();  ///< Call this method while processing each positional argument one-by-one in your `IArgsParser::_parseArgs` to get an index of the current positional argument and update the counter. You can use the returned index to get the actual value of the positional argument.
		void resetPositionalParsing();  ///< Call this method before processing positional arguments one-by-one in your `IArgsParser::_parseArgs`
	};
};

#define HYDRARGS_CHECK_IF_ENOUGH_MAND_POS_ARGS_AND_RETURN_ERROR_OTHERWISE() \
{\
	static_assert(std::is_base_of<IPositionalParser, std::remove_reference<decltype(*this)>::type>::value);\
	static_assert(std::is_base_of<CLIRawArgs, std::remove_reference<decltype(rawArgs)>::type>::value);\
	static_assert(std::is_same<struct ParseEvents, decltype(checkIfEnoughMandPosArgs(nullptr))>::value);\
	if(struct ParseEvents isNotEnoughPos = checkIfEnoughMandPosArgs(rawArgs.argv0)){\
		return {\
			.parsingStatus = {\
				.returnCode = EXIT_FAILURE,\
				.events = isNotEnoughPos\
			}\
		};\
	}\
}
