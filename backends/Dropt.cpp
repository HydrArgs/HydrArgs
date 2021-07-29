#include <stdint.h>
#include <cstring>
#include <memory>
#include <iostream>
#include <span>
#include <string>
#include <vector>

#include <HydrArgs/HydrArgs.hpp>
#include <HydrArgs/fallback/positional/spannable.hpp>

#include "num_utils.hpp"

#include <droptxx.hpp>

using namespace HydrArgs;
using namespace HydrArgs::fallback::positional;

namespace HydrArgs::Backend{
	template <ArgType typeValue> struct GetDroptHandler{};
	template<> struct GetDroptHandler<ArgType::flag> {constexpr static const dropt_option_handler_func handler = dropt::handle_verbose_bool;};
	template<> struct GetDroptHandler<ArgType::string> {constexpr static const dropt_option_handler_func handler = dropt::handle_string;};
	template<> struct GetDroptHandler<ArgType::s4> {constexpr static const dropt_option_handler_func handler = dropt::handle_int;};
	template<> struct GetDroptHandler<ArgType::u4> {constexpr static const dropt_option_handler_func handler = dropt::handle_uint;};
	template<> struct GetDroptHandler<ArgType::f8> {constexpr static const dropt_option_handler_func handler = dropt::handle_double;};

	struct dropt_option emptyOption(){
		dropt_option opt{};
		std::memset(&opt, 0, sizeof(opt));
		return opt;
	}

	// FUCK, they define an unnamed enum and define dropt_error as an int.
	enum class dropt_error_ec: dropt_error{
		none = dropt_error_none,
		unknown = dropt_error_unknown,
		bad_configuration = dropt_error_bad_configuration,
		insufficient_memory = dropt_error_insufficient_memory,
		invalid_option = dropt_error_invalid_option,
		insufficient_arguments = dropt_error_insufficient_arguments,
		mismatch = dropt_error_mismatch,
		overflow = dropt_error_overflow,
		underflow = dropt_error_underflow,

		custom_start = dropt_error_custom_start,
		custom_last = dropt_error_custom_last
	};

	using DroptAttrIntT = decltype(static_cast<dropt_option*>(nullptr)->attr);
	enum class dropt_attr: DroptAttrIntT{
		halt = dropt_attr_halt,
		hidden = dropt_attr_hidden,
		optional_val = dropt_attr_optional_val,

		our_mandatory_was_present = (1U << (sizeof(DroptAttrIntT) * 8U - 2U)),
		our_mandatory = (1U << (sizeof(DroptAttrIntT) * 8U - 1U))
	};

	template <dropt_option_handler_decl f> dropt_error dropt_handle_mandatory(dropt_context *context, const dropt_option *option, const dropt_char *optionArgument, void *dest) {
		auto const res = static_cast<dropt_error_ec>(f(context, option, optionArgument, dest));
		if(res == dropt_error_ec::none){
			const_cast<dropt_option*>(option)->attr |= static_cast<DroptAttrIntT>(dropt_attr::our_mandatory_was_present);
		}
		return static_cast<dropt_error>(res);
	}

	struct DroptBackend: public IArgsParser, public PositionalParserSpannable{
		std::vector<Arg*> positionalOptionalSpec;
		std::vector<Arg*> positionalMandatorySpec;

		std::vector<std::string> remaining;
		std::vector<char *> remainingPtrs;
		std::vector<dropt_option> nativeArgs{
			dropt_option{
				.short_name = DEFAULT_HELP_ARG.letter_str.getLetter(),
				.long_name = DEFAULT_HELP_ARG.long_name.undashed,
				.description = DEFAULT_HELP_ARG.doc,
				.handler = dropt::handle_bool,
				.dest = &show_help,
				.attr = dropt_attr_halt,
			}
		};
		dropt_option opt = emptyOption();
		const std::string name;
		const std::string usage;
		const std::string descr;

		ValidationState validationState;
		bool show_help = false;

		DroptBackend(const std::string &name, const std::string &descr, const std::string &usage [[maybe_unused]], const std::vector<Arg*>& dashedSpec, const std::vector<Arg*>& positionalSpec, Streams streams): IArgsParser(streams), PositionalParserSpannable(this), name(name), usage(usage), descr(descr){

			for(const auto &argPtr: dashedSpec) {
				_addArg(argPtr, false);
			}
			for(const auto &argPtr: positionalSpec) {
				_addArg(argPtr, true);
			}
		}

		void _seal() override {
			nativeArgs.emplace_back(emptyOption());
		}

		void _unseal() override {
			nativeArgs.pop_back();
		}

		[[nodiscard]] bool isSealed() const override {
			return !nativeArgs.empty() && nativeArgs.back().handler == nullptr;
		}

		void _addDashedArg(Arg *argPtr){
			//opt.short_name = argPtr->letter;
			opt.long_name = argPtr->name.data();
			opt.description = argPtr->description.data();
			opt.arg_description = argPtr->value_name ? argPtr->value_name : VALUE_PLACEHOLDER;// Must be set to non-NULL value in order to allow parsing without `=` separator
			opt.attr = static_cast<DroptAttrIntT>(dropt_attr::our_mandatory) * static_cast<DroptAttrIntT>(static_cast<bool>(argPtr->minCount));
				// | (static_cast<DroptAttrIntT>(dropt_attr::optional_val) * (!argPtr->minCount))  // it is not for optionality, it is for allowing calling `--aaa` as an alternative to `--aaa <value>`)
			auto switchCaseBodyLambda = [&] <ArgType t>() {
				auto specOptPtr = static_cast<SArg<t>*>(argPtr);
				if(t == ArgType::flag){
					opt.attr |= static_cast<DroptAttrIntT>(dropt_attr::optional_val);  // to allow boolean options without the next value (flags)
				}
				if(argPtr->minCount){
					opt.handler = dropt_handle_mandatory<GetDroptHandler<t>::handler>;
				} else {
					if constexpr (t == ArgType::flag){
						opt.arg_description = nullptr;  // to disallow verbose boolean options without = to avoid confusion with the next arg of string type
					}
					opt.handler = GetDroptHandler<t>::handler;
				}
				opt.dest = &specOptPtr->value;
			};

			switch(argPtr->type){
				case ArgType::flag:
				{
					switchCaseBodyLambda.operator()<ArgType::flag>();
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
				default:
					throw UnsupportedArgumentType{argPtr->type};
			}
			nativeArgs.emplace_back(opt);
		}

		void _addArg(Arg *argPtr, bool isPositional) override {
			if(isPositional){
				addPosArg(argPtr);
			} else {
				validateDashed(argPtr);
				_addDashedArg(argPtr);
			}
		}

		~DroptBackend() override = default;

		void printHelp(std::ostream &stream, const char *const argv0) override {
			seal();
			dropt::context droptContext(nativeArgs.data());
			_printHelp(stream, argv0, droptContext);
		}

		void _printHelp(std::ostream &stream, const char *const argv0, dropt::context &droptContext) {
			stream << argv0 << " " << usage << std::endl;
			stream << descr << std::endl;
			stream << droptContext.get_help() << std::endl;
		}

		uint32_t restc = 0;
		[[nodiscard]] bool isNotEnoughArgs(uint32_t positionalArgCountLowerBound) const override {
			return restc < positionalArgCountLowerBound;
		}

		ParseResult _parseArgs(CLIRawArgs rawArgs) override {
			dropt::context droptContext(nativeArgs.data());

			// This shitty impl assummes that argv contains no `argv0` and that argv ends with `nullptr` AFTER THE END OF THE ARRAY (`argv[argc] == nullptr`). So in the official examples argc passed to `parse` is `-1` (which is a special value that means "untill NULL occurs") and instead of `argv` `&rawArgs.argv[1]` is passed. So we have to do the following strange thing
			auto argsVector = rawArgs.getArgvVector(); // Have to do it, because must end with nullptr

			char **rest = droptContext.parse(checked_cast<int>(argsVector.size() - 1), const_cast<dropt_char **>(&argsVector[1]));
			restc = 0;
			for(; rest[restc]; ++restc);

			/*
			struct ParseResult printErrorMessage(const char * const argv0, std::string errMsg, std::span<char*> rest);*/

			auto printErrorMessage = [&](const std::string &errMsg, char **rest) -> struct ParseResult{
				streams.cerr << errMsg << std::endl;
				streams.cerr << "Non-consumed rest args:";
				for(; rest[restc]; ++restc){
					streams.cerr << " " << rest[restc];
				}
				streams.cerr << std::endl;
				_printHelp(streams.cerr, rawArgs.argv0, droptContext);
				return RESULT_SYNTAX_ERROR;
			};

			auto const droptNativeErr = static_cast<dropt_error_ec>(droptContext.get_error());

			if(show_help){
				_printHelp(streams.cout, rawArgs.argv0, droptContext);
				return RESULT_HELP_CALLED;
			}

			if(droptNativeErr != dropt_error_ec::none){
				return printErrorMessage(droptContext.get_error_message(), rest);
			}

			HYDRARGS_CHECK_IF_ENOUGH_MAND_POS_ARGS_AND_RETURN_ERROR_OTHERWISE();

			for(auto &nArg: nativeArgs){
				if(nArg.attr & static_cast<DroptAttrIntT>(dropt_attr::our_mandatory)){
					if(!(nArg.attr & static_cast<DroptAttrIntT>(dropt_attr::our_mandatory_was_present))){
						std::stringstream serr;
						serr << "Mandatory dashed arg `" << nArg.long_name << "` was not present";
						return printErrorMessage(serr.str(), rest);
					}
				}
			}

			auto restSpan = std::span<const char *>(const_cast<const char **>(rest), restc);
			return parsePositionals(rawArgs.argv0, restSpan);
		}
	};

	IArgsParser * argsParserFactory(const std::string &name, const std::string &descr, const std::string &usage [[maybe_unused]], const std::vector<Arg *> &dashedSpec, const std::vector<Arg *> &positionalSpec, Streams streams){
		return new DroptBackend(name, descr, usage, dashedSpec, positionalSpec, streams);
	}
}
