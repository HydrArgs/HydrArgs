#pragma once

#include "ArgType.hpp"
#include <cstdint>
#include <type_traits>
#include <string>
#include <complex>

namespace HydrArgs{

	template <ArgType typeValue> struct GetUnderlyingTypeByEnumValue{};
	template<> struct GetUnderlyingTypeByEnumValue<ArgType::flag> {using type = bool;};
	template<> struct GetUnderlyingTypeByEnumValue<ArgType::string> {using type = std::string;};
	template<> struct GetUnderlyingTypeByEnumValue<ArgType::wstring> {using type = std::wstring;};
	template<> struct GetUnderlyingTypeByEnumValue<ArgType::path> {using type = std::string;};

	template<> struct GetUnderlyingTypeByEnumValue<ArgType::u1> {using type = uint8_t;};
	template<> struct GetUnderlyingTypeByEnumValue<ArgType::u2> {using type = uint16_t;};
	template<> struct GetUnderlyingTypeByEnumValue<ArgType::u4> {using type = uint32_t;};
	template<> struct GetUnderlyingTypeByEnumValue<ArgType::u8> {using type = uint64_t;};
	//template<> struct GetUnderlyingTypeByEnumValue<ArgType::u16> {using type = uint128_t;};

	template<> struct GetUnderlyingTypeByEnumValue<ArgType::s1> {using type = std::make_signed<GetUnderlyingTypeByEnumValue<ArgType::u1>::type>::type;};
	template<> struct GetUnderlyingTypeByEnumValue<ArgType::s2> {using type = std::make_signed<GetUnderlyingTypeByEnumValue<ArgType::u2>::type>::type;};
	template<> struct GetUnderlyingTypeByEnumValue<ArgType::s4> {using type = std::make_signed<GetUnderlyingTypeByEnumValue<ArgType::u4>::type>::type;};
	template<> struct GetUnderlyingTypeByEnumValue<ArgType::s8> {using type = std::make_signed<GetUnderlyingTypeByEnumValue<ArgType::u8>::type>::type;};
	//template<> struct GetUnderlyingTypeByEnumValue<ArgType::s16> {using type = std::make_signed<GetUnderlyingTypeByEnumValue<ArgType::u16>::type>::type;};


	//template<> struct GetUnderlyingTypeByEnumValue<ArgType::f1> {using type = short half;};
	//template<> struct GetUnderlyingTypeByEnumValue<ArgType::f2> {using type = half;};
	template<> struct GetUnderlyingTypeByEnumValue<ArgType::f4> {using type = float;};
	template<> struct GetUnderlyingTypeByEnumValue<ArgType::f8> {using type = double;};
	//template<> struct GetUnderlyingTypeByEnumValue<ArgType::f10> {using type = long double;};
	template<> struct GetUnderlyingTypeByEnumValue<ArgType::f16> {using type = long double;};


	//template<> struct GetUnderlyingTypeByEnumValue<ArgType::complex_f1> {using type = std::complex<GetUnderlyingTypeByEnumValue<ArgType::f1>::type>;};
	//template<> struct GetUnderlyingTypeByEnumValue<ArgType::complex_f2> {using type = std::complex<GetUnderlyingTypeByEnumValue<ArgType::f2>::type>;};
	template<> struct GetUnderlyingTypeByEnumValue<ArgType::complex_f4> {using type = std::complex<GetUnderlyingTypeByEnumValue<ArgType::f4>::type>;};
	template<> struct GetUnderlyingTypeByEnumValue<ArgType::complex_f8> {using type = std::complex<GetUnderlyingTypeByEnumValue<ArgType::f8>::type>;};
	//template<> struct GetUnderlyingTypeByEnumValue<ArgType::complex_f10> {using type = std::complex<GetUnderlyingTypeByEnumValue<ArgType::f10>::type>;};
	template<> struct GetUnderlyingTypeByEnumValue<ArgType::complex_f16> {using type = std::complex<GetUnderlyingTypeByEnumValue<ArgType::f16>::type>;};

};
