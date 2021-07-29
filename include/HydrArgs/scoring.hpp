#pragma once

#include "api.hpp"

#include <cstdint>

#include <type_traits>
#include <compare>

namespace HydrArgs{
	struct HYDRARGS_API HYDRARGS_PACKED ICapabilitiesStruct{
		uint32_t getScore() const;  ///< Score for sorting backends.
		bool operator>(const ICapabilitiesStruct &other) const;  ///< A comparator method for sorting backends
		std::weak_ordering operator<=>(const ICapabilitiesStruct &other) const;
	};

	static_assert(std::is_aggregate<ICapabilitiesStruct>::value);

	/// Describes if positional arguments are supported natively. If they are not, we have our fallback implementation, which can be inferior.
	struct HYDRARGS_API HYDRARGS_PACKED PositionalSupport: public ICapabilitiesStruct{
		bool mandatory: 1 = true,  ///< supports mandatory positional arguments
		optional: 1 = true,  ///< supports optional positional arguments
		optional_before_mandatory: 1 = false,  ///< optional positional arguments can be placed before mandatory positional ones
		booleans: 1 = false;  ///< supports positional booleans inside the lib

		constexpr static const uint8_t BIT_SIZE = 4;

		uint32_t getScore() const;
		bool operator>(const PositionalSupport &other) const;
		std::weak_ordering operator<=>(const PositionalSupport &other) const;
	};
	static_assert(std::is_aggregate<PositionalSupport>::value);
	static_assert(std::is_trivially_copyable<PositionalSupport>::value);

	/// Describes if positional arguments are supported natively. If they are not, we have our fallback implementation, which can be inferior.
	struct HYDRARGS_API HYDRARGS_PACKED DashedSupport: public ICapabilitiesStruct{
		bool mandatory: 1 = true,  ///< supports mandatory positional arguments
		optional: 1 = true;  ///< supports optional positional arguments

		constexpr static const uint8_t BIT_SIZE = 2;

		uint32_t getScore() const;
		bool operator>(DashedSupport &other) const ;
		std::weak_ordering operator<=>(const DashedSupport &other) const;
	};
	static_assert(std::is_aggregate<DashedSupport>::value);
	static_assert(std::is_trivially_copyable<DashedSupport>::value);

	/// Help info generation support
	struct HYDRARGS_API HYDRARGS_PACKED HelpCapabilities: public ICapabilitiesStruct{
		bool error_message: 1 = false,  ///< generates error message itself
		help: 1 = false,  ///< generates help message with verbose info and docstrings for each arg
		usage: 1 = false,  ///< shows "1-line" example of how to run a program in a quasi-BNF way
		descr: 1 = false,  ///< harmonically integrates user-provided description into help message
		value_name: 1 = false,  ///< allows to set a name of a value to show in help
		value_unit: 1 = false  ///< allows to set a unit of a value to show in help
		;

		constexpr static const uint8_t BIT_SIZE = 6;

		uint32_t getScore() const;
		bool operator>(HelpCapabilities &other) const;
		std::weak_ordering operator<=>(const HelpCapabilities &other) const;
	};
	static_assert(std::is_aggregate<HelpCapabilities>::value);
	static_assert(std::is_trivially_copyable<HelpCapabilities>::value);

	/// Help info generation support
	struct HYDRARGS_API HYDRARGS_PACKED BellsAndWhistlesSupport: public ICapabilitiesStruct{
		bool path_validation: 1 = false,  ///< arguments of type path are checked if the path exists. The lib generates error message itself.
		auto_complete: 1 = false,  ///< has built-in auto-completion capabilities
		wide_strings: 1 = false,  ///< supports `wstring`s. Nice for paths on Windows.
		builtin_version_flag: 1 = false,  ///< Has a predefined plag for printing a version
		builtin_usage_flag: 1 = false,  ///< Has a predefined plag for printing usage only
		builtin_license_flag: 1 = false  ///< Has a predefined plag for printing license
		;

		constexpr static const uint8_t BIT_SIZE = 6;

		uint32_t getScore() const;
		bool operator>(BellsAndWhistlesSupport &other) const;
		std::weak_ordering operator<=>(const BellsAndWhistlesSupport &other) const;
	};
	static_assert(std::is_aggregate<BellsAndWhistlesSupport>::value);
	static_assert(std::is_trivially_copyable<BellsAndWhistlesSupport>::value);

	/// Important shit
	struct HYDRARGS_API HYDRARGS_PACKED ImportantShit: public ICapabilitiesStruct{
		bool stable: 1 = false,  ///< the backend quality is much better than quality of other backends. Should be preferred over them.
		bug_memory_safety: 1 = false,  ///< The wrapped library likely has memory safety issues.
		bug_terminates_itself: 1 = false,  ///< The wrapped library terminates the process itself, disallowing proper testing and processing the case of help being called.
		bug_prints_itself: 1 = false,  ///< The wrapped library prints messages itself into hardcoded streams, disallowing end user to suppress its output
		thread_unsafe: 1 = false,  ///< The lib uses mutable global state and mutates it.
		non_reentrable: 1 = false,  ///< The object of the lib cannot be reused, a user has to create a brand new one.
		other_grave_bugs: 1 = false,  ///< The wrapped library has other grave bugs in its the most modern state.
		permissive_license: 1 = false,  ///< backend license is a permissive one.
		unmaintained: 1 = false  ///<  The wrapped library is not actively developed and maintained, bugs are not fixed, large lags in accepting PRs
		;

		constexpr static const uint8_t BIT_SIZE = 9;

		uint32_t getScore() const;
		bool operator>(ImportantShit &other) const;
		std::weak_ordering operator<=>(const ImportantShit &other) const;
	};
	static_assert(std::is_aggregate<ImportantShit>::value);
	static_assert(std::is_trivially_copyable<ImportantShit>::value);

	struct HYDRARGS_API HYDRARGS_PACKED ParserCapabilities: public ICapabilitiesStruct{

		struct HelpCapabilities help = {};  ///< Help info generation support

		struct PositionalSupport native_positional = {};  ///< Describes if positional arguments are supported natively.

		struct DashedSupport dashed = {};  ///< Describes if dashed arguments are supported natively. If they are not, they are not supported at all. It is the design decision.

		struct BellsAndWhistlesSupport bellsAndWhistles = {};  ///< Describes if some non-essential but very nice to have stuff is supported

		struct ImportantShit important = {};  ///< Important shit of non-technical origin

		constexpr static const uint8_t native_positional_offset = decltype(bellsAndWhistles)::BIT_SIZE;
		constexpr static const uint8_t help_offset = native_positional_offset + decltype(native_positional)::BIT_SIZE;
		constexpr static const uint8_t dashed_offset = help_offset + decltype(help)::BIT_SIZE;
		constexpr static const uint8_t important_offset = dashed_offset + decltype(dashed)::BIT_SIZE;

		uint32_t getScore() const;
		bool operator>(ParserCapabilities &other) const;
		std::weak_ordering operator<=>(const ParserCapabilities &other) const;
	};
	static_assert(std::is_aggregate<ParserCapabilities>::value);
	static_assert(std::is_trivially_copyable<ParserCapabilities>::value);
};
