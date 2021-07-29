#pragma once

#include <cstdint>
#include <type_traits>
#include <stdexcept>
#include <string>
#include <vector>
#include <ostream>

#include "api.hpp"
#include "ArgType.hpp"
#include "ArgTypeEnumMapping.hpp"
#include "exceptions.hpp"
#include "args.hpp"
#include "result.hpp"

namespace HydrArgs{

	/// Our structure allowing a user to redefine all the streams to which messages must be output.
	struct HYDRARGS_API Streams{
		std::ostream &cout, &cerr;
	};

	struct HYDRARGS_API ArgsSeqValidationException: public std::logic_error{
		ArgsSeqValidationException(const std::string &what);
		ArgsSeqValidationException();
	};

	struct HYDRARGS_API ArgFlags{
		uint8_t reserved: 8 = false;///< All flags are currently reserved!
	};

	/// A unified class to store an arg
	struct HYDRARGS_API Arg{
		const ArgType type;  ///< Identifies type of destination.
		const ArgFlags flags;///< Defines properties of this flag.

		union {
			const char letter;  ///< 1-letter short flag without any leading dashes, i.e for -h you need 'h'
			const char letterStr[2]{0u, 0u};  ///< to allow usage of `letter` field as a null-terminated string;
		};

		const std::string name;  ///< multi-letter long flag without any leading dashes, i. e. for --help you need "help"
		const std::string description;  ///< the purpose of the option to be printed in a help message.
		uint32_t minCount = 0;  ///< Minimal count of times an argument can appear
		uint32_t maxCount = 1;  ///< Maximal count of times an argument can appear. 0 means Unlimited.
		const char *const value_name = nullptr;  ///< name of a value to be printed in a usage message
		const char *const value_unit = nullptr;  ///< unit of the value to be printed in a help message

		Arg(ArgType type, char letter, std::string name, std::string description, uint32_t minCount, const char *value_name, const char *value_unit);
		Arg(ArgType type, std::string name, std::string description, uint32_t minCount, const char *value_name, const char *value_unit);

		Arg(ArgType type, char letter, std::string name, std::string description, uint32_t minCount, const char *value_name);
		Arg(ArgType type, std::string name, std::string description, uint32_t minCount, const char *value_name);

		Arg(ArgType type, char letter, std::string name, std::string description, uint32_t minCount);
		Arg(ArgType type, std::string name, std::string description, uint32_t minCount);
	};


	/// SArg stands for a specialized arg
	template <ArgType SpecializedArgTypeEnumValue> struct SArg: public Arg{
		using StorageType = typename GetUnderlyingTypeByEnumValue<SpecializedArgTypeEnumValue>::type;
		using ThisType = SArg<SpecializedArgTypeEnumValue>;
		StorageType value;  ///< Stores result of parsing.

		operator StorageType & (){
			return getValueRef();
		}

		StorageType & getValueRef(){
			return value;
		}

		///@}
		/// @name Mirroring parent ctor, with arbitrary ArgType type
		///@{

		SArg(ArgType type, char letter, std::string name, std::string description, uint32_t minCount, const char *value_name, const char *value_unit, StorageType defaultValue): Arg(type, letter, name, description, minCount, value_name, value_unit), value(defaultValue){}
		SArg(ArgType type, std::string name, std::string description, uint32_t minCount, const char *value_name, const char *value_unit, StorageType defaultValue): Arg(type, name, description, minCount, value_name, value_unit), value(defaultValue){}

		SArg(ArgType type, char letter, std::string name, std::string description, uint32_t minCount, const char *value_name, StorageType defaultValue): Arg(type, letter, name, description, minCount, value_name), value(defaultValue){}
		SArg(ArgType type, std::string name, std::string description, uint32_t minCount, const char *value_name, StorageType defaultValue): Arg(type, name, description, minCount, value_name), value(defaultValue){}

		SArg(ArgType type, char letter, std::string name, std::string description, uint32_t minCount, StorageType defaultValue): Arg(type, letter, name, description, minCount), value(defaultValue){}
		SArg(ArgType type, std::string name, std::string description, uint32_t minCount, StorageType defaultValue): Arg(type, name, description, minCount), value(defaultValue){}

		///@}
		/// @name With arbitrary ArgType type = SpecializedArgTypeEnumValue
		///@{

		SArg(char letter, std::string name, std::string description, uint32_t minCount, const char *value_name, const char *value_unit, StorageType defaultValue): SArg(SpecializedArgTypeEnumValue, letter, name, description, minCount, value_name, value_unit, defaultValue){}
		SArg(std::string name, std::string description, uint32_t minCount, const char *value_name, const char *value_unit, StorageType defaultValue): SArg(SpecializedArgTypeEnumValue, name, description, minCount, value_name, value_unit, defaultValue){}

		SArg(char letter, std::string name, std::string description, uint32_t minCount, const char *value_name, StorageType defaultValue): SArg(SpecializedArgTypeEnumValue, letter, name, description, minCount, value_name, defaultValue){}
		SArg(std::string name, std::string description, uint32_t minCount, const char *value_name, StorageType defaultValue): SArg(SpecializedArgTypeEnumValue, name, description, minCount, value_name, defaultValue) {}

		SArg(char letter, std::string name, std::string description, uint32_t minCount, StorageType defaultValue): SArg(SpecializedArgTypeEnumValue, letter, name, description, minCount, defaultValue){}
		SArg(std::string name, std::string description, uint32_t minCount, StorageType defaultValue): SArg(SpecializedArgTypeEnumValue, name, description, minCount, defaultValue){}
	};


	/// An arg storing a path in a FS
	struct HYDRARGS_API PathArg: public SArg<ArgType::string>{
		PathArg(char letter, std::string name, std::string description, uint32_t minCount, const char *value_name, const char *value_unit, StorageType defaultValue = "");
		PathArg(std::string name, std::string description, uint32_t minCount, const char *value_name, const char *value_unit, StorageType defaultValue = "");
		PathArg(char letter, std::string description, uint32_t minCount, const char *value_name, const char *value_unit, StorageType defaultValue = "");

		PathArg(char letter, std::string name, std::string description, uint32_t minCount, const char *value_name, StorageType defaultValue = "");
		PathArg(std::string name, std::string description, uint32_t minCount, const char *value_name, StorageType defaultValue = "");
		PathArg(char letter, std::string description, uint32_t minCount, const char *value_name, StorageType defaultValue = "");

		PathArg(char letter, std::string name, std::string description, uint32_t minCount, StorageType defaultValue = "");
		PathArg(std::string name, std::string description, uint32_t minCount, StorageType defaultValue = "");
		PathArg(char letter, std::string description, uint32_t minCount, StorageType defaultValue = "");
	};

	/// Our unified interface for CLI args parsers.
	struct HYDRARGS_API IArgsParser{
		Streams streams;

		IArgsParser(Streams streams);
		IArgsParser();
		virtual ~IArgsParser();
		virtual void addArg(Arg *argPtr, bool isPositional);  ///< adds an arg. If the parser was sealed, unseals it.
		ParseResult operator()(CLIRawArgs rawArgs);  ///< A user must call this method to parse the CLI and get results.

		virtual bool isSealed() const;  ///< Some parsers require finalization actions be done before the parser can be used. This method returns `true` if the parser already has these actions applied and `false` if the actions (implemented in `_seal`) have to be applied.
		void seal();  ///< Some parsers require finalization actions be done before the parser can be used. A user of your library can call it to do this action, though it is likely he will never need it. It is here for internal purposes in fact.
		void unseal();  ///< Some parsers require finalization actions be done before the parser can be used. A user of your library can call it to cancell doing this action, though it is likely he will never need it. It is here for internal purposes in fact.

		virtual void _addArg(Arg *argPtr, bool isPositional) = 0;  ///< override ths  method to add an arg to your parser
		virtual void printHelp(std::ostream &stream, const char *const argv0) = 0;
		virtual void _seal();  ///< Some parsers require finalization actions be done before the parser can be used. Override this method and implement the finalization action, if your backend needs it. It is allowed not to check
		virtual void _unseal();  ///< Some parsers require finalization actions be done before the parser can be used. Override this method and implement removal of finalization action. You MUST do it, if you have overridden `_seal`.
		virtual ParseResult _parseArgs(CLIRawArgs rawArgs) = 0;  ///< Implaments the logic needed for parsing the command lne using the parser.
	};

	struct HYDRARGS_API IArgsParserClient{
		IArgsParser *parent;  ///< Pointer to parent. Needed to overcome limitations of non-trueЪ inheritance in C++
		IArgsParserClient(IArgsParser *parent);
	};

	/// A base class for backends that would have to rely on the original spec, so it should be preserved.
	struct HYDRARGS_API IBackendOwnStoredSpec: virtual public IArgsParser{
		std::vector<Arg *> dashedSpec, positionalSpec;

		IBackendOwnStoredSpec(std::vector<Arg*> dashedSpec, std::vector<Arg*> positionalSpec);
		IBackendOwnStoredSpec(std::vector<Arg*> dashedSpec, std::vector<Arg*> positionalSpec, Streams streams);
		virtual ~IBackendOwnStoredSpec() override = 0;

		virtual void addArg(Arg *argPtr, bool isPositional) override;
	};

	/// Stores 2 pointers into the same string
	struct HYDRARGS_API DefaultArgName_{
		const char * const dashed;  ///< A name with needed amount of dashes (`"--long-name"`, `"-l"`)
		const char * const undashed;  ///< A name without any leading dashes (`"long-name"`, `"l"`)

		/// A single letter char (`'l'`)
		inline constexpr char getLetter() const {
			return *undashed;
		}
	};

	/// Tries to compute and populate DefaultArgName_ in compile time
	template <uint8_t countOfDashes>
	struct HYDRARGS_API DefaultArgName: public DefaultArgName_{
		inline constexpr DefaultArgName(const char * const dashed): DefaultArgName_({.dashed = dashed, .undashed = dashed + countOfDashes}){}
	};

	typedef struct DefaultArgName<2> LongDefaultArgName;  ///< A long name with 2 dashes (`"--long-name"`)
	typedef struct DefaultArgName<1> LetterDefaultArgName;///< A single letter name with a dash (`"-l"`)

	/// Stores an info about a predefined arg in a single place in a convenient form
	struct HYDRARGS_API DefaultArgInfo{
		const LongDefaultArgName long_name;  ///< A long name with 2 dashes (`"--long-name"`)
		const LetterDefaultArgName letter_str;///< A single letter name with a dash (`"-l"`)
		const char * const doc = nullptr;  ///< A docstring
	};

	extern const struct DefaultArgInfo DEFAULT_HELP_ARG HYDRARGS_API;

	extern const char * const VALUE_PLACEHOLDER HYDRARGS_API;///< Some CLI parsing libs require value name to be present. For these ones we replace nullptr with this value;

	extern const Streams DEFAULT_STREAMS HYDRARGS_API;///< `cout` and `cerr`
};

#include "scoring.hpp"

namespace HydrArgs::Backend{
	extern "C" {
	HYDRARGS_BACKEND_API IArgsParser * argsParserFactory(const std::string &name, const std::string &descr, const std::string &usage [[maybe_unused]], const std::vector<Arg *> &dashedSpec, const std::vector<Arg *> &positionalSpec, Streams streams = DEFAULT_STREAMS);  ///< A factory to create a backend.  Can be overridden. Must be exported by a backend shared lib.

	extern const struct HydrArgs::ParserCapabilities capabilities HYDRARGS_BACKEND_API;  ///< A struct describing the backend. Must be exported by a backend shared lib.
	};

	using ArgsParserFactoryT = decltype(argsParserFactory);

};
