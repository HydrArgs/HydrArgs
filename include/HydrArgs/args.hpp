#pragma once

#include "api.hpp"

#include <vector>
#include <initializer_list>

#if __cplusplus >= 202002
	#include <span>
#endif

namespace HydrArgs{

struct HYDRARGS_API CLIRawArgs{
	using element_type = const char *;
	using SpanT = std::span<element_type>;
	using VectorT = std::vector<element_type>;

	element_type argv0 = nullptr;  ///< argv0, program name
	SpanT argvRest;  ///< array of pointers to CLI args

	VectorT getArgvVector() const;  ///< Returns an std::vector of pointers to argv. WARNING, vectorSize() - 1 == argc beacause the last element of the vector must be NULL
};

extern const CLIRawArgs::SpanT NO_REST_ARGS HYDRARGS_API;  ///< Convenience constant to be used in the cases args haven't been pased or make no sense at all.
extern const struct CLIRawArgs NO_ARGS HYDRARGS_API;  ///< Convenience constant to be used in the cases args haven't been pased or make no sense at all.

};
