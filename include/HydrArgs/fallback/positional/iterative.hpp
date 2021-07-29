#pragma once

#include "./IPositionalParser.hpp"
#include <type_traits>

namespace HydrArgs::fallback::positional{

	struct HYDRARGS_FALLBACK_POSITIONAL_API PositionalParserIterative: public IPositionalParser{
		uint32_t positionalArgsCount;  ///< Total count of specified positional arguments.
		uint32_t mandatoryPositionalArgsCount;  ///< Count of specified poitional arguments that are mandatory.

		PositionalParserIterative(IArgsParser *parent);
		virtual ~PositionalParserIterative() override;

		virtual void addPosArg(Arg *argPtr) override;

		virtual uint32_t getMandatoryPositionalArgsCount() const override;
	};
};
