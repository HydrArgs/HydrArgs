#pragma once

#include <stdexcept>

#include "api.hpp"
#include "ArgType.hpp"

namespace HydrArgs{
	struct HYDRARGS_API UnsupportedArgumentType : public std::logic_error {
		ArgType type;
		UnsupportedArgumentType(ArgType type);
	};
};
