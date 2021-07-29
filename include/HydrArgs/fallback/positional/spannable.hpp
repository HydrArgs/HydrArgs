#pragma once

#include "./IPositionalParser.hpp"
#include <type_traits>

namespace HydrArgs::fallback::positional{

	/// This class is intended to be used by backends that store the spec for DASHED arguments IN THE UNDERLYING LIBRARY, NOT IN THE BACKEND ITSELF
	struct HYDRARGS_FALLBACK_POSITIONAL_API PositionalParserSpannable: public IPositionalParser{
		std::vector<Arg*> positionalOptionalSpec;
		std::vector<Arg*> positionalMandatorySpec;
		ValidationState validationState;

		PositionalParserSpannable(IArgsParser *parent);
		virtual ~PositionalParserSpannable() override;

		virtual uint32_t getMandatoryPositionalArgsCount() const override;

		virtual void addPosArg(Arg *argPtr) override;  ///< Call this method before/after registering a positional argument in your `IArgsParser::_addArg`
		void validateDashed(Arg *argPtr) const;  ///< Call this method before/after registering a dashed argument in your `IArgsParser::_addArg`
		struct ParseResult parsePositionals(const char * const argv0, std::span<const char *> rest);

		struct ParseResult printErrorMessage(const char *const argv0, const std::string& errMsg, std::span<const char *> rest);
	};
};
