#pragma once

#include "api.hpp"

#if __has_extension(enumerator_attributes)
#define HYDRARGS_UNUSED_ENUM_rNls9wcl [[maybe_unused]]
#else
#define HYDRARGS_UNUSED_ENUM_rNls9wcl
#endif

namespace HydrArgs{
	/// See ArgTypeEnumMapping.hpp for a compile-time mapping
	enum struct HYDRARGS_API ArgType: std::uint8_t{
		invalid HYDRARGS_UNUSED_ENUM_rNls9wcl = 0,  ///< Must never be used in apps. Marks uninitialized memory (we zero the memory).
		flag = 1,  ///< bool

		string = 2, ///< std::string
		wstring = 3, ///< std::wstring
		path = 4, ///< std::string or std::wstring used as a path

		u1 = 5,  ///< uint8_t
		s1 = 6,  ///< int8_t
		u2 = 7,  ///< uint16_t
		s2 = 8,  ///< int16_t
		u4 = 9,  ///< uint32_t
		s4 = 10, ///< int32_t
		u8 = 11, ///< uint64_t
		s8 = 12, ///< int64_t
		u16 = 13,///< uint128_t
		s16 = 14,///< int128_t

		f1 = 15, ///< short half
		f2 = 16, ///< half
		f4 = 17, ///< float
		f8 = 18, ///< double
		f10 = 19,///< long double
		f16 = 20,///< long double

		complex_f1 = 21,
		complex_f2 = 22,
		complex_f4 = 23,
		complex_f8 = 24,
		complex_f10 = 25,
		complex_f16 = 25,
	};

};

#undef HYDRARGS_UNUSED_ENUM_rNls9wcl

