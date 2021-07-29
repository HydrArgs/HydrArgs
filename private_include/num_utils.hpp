#pragma once

#include <stdexcept>

#include <type_traits>
#include <utility>

// 1. static_assert cannot be used for non-constexpr values
// 2. functions arguments are never constexpr values
// 3. even `if consteval` or `if constexpr (std::is_constant_evaluated())` are used

template <typename TargetT, typename SourceT>
constexpr std::enable_if_t<std::is_integral<SourceT>::value, TargetT> checked_cast(const SourceT v){
	if(!std::in_range<TargetT>(v)){
		throw std::logic_error("Out of bounds cast");
	}
	return static_cast<TargetT>(v);
}

template <typename ST>
constexpr std::enable_if_t<std::is_signed<ST>::value && std::is_integral<ST>::value, typename std::make_unsigned<ST>::type> toUnsigned(const ST v){
	using UnsT = typename std::make_unsigned<ST>::type;
	return checked_cast<UnsT, ST>(v);
}

template <typename ST>
constexpr std::enable_if_t<std::is_unsigned<ST>::value && std::is_integral<ST>::value, typename std::make_signed<ST>::type> toSigned(const ST v){
	using UnsT = typename std::make_signed<ST>::type;
	return checked_cast<UnsT, ST>(v);
}
