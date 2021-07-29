#include <HydrArgs/exceptions.hpp>

namespace HydrArgs{

UnsupportedArgumentType::UnsupportedArgumentType(ArgType type) : std::logic_error("This argument type is unsupported for this backend"), type(type) {}


};
