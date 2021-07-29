#include <cstdint>
#include <compare>

#include <HydrArgs/scoring.hpp>

using namespace HydrArgs;

uint32_t ICapabilitiesStruct::getScore() const {
	return 0;
}

bool ICapabilitiesStruct::operator>(const ICapabilitiesStruct &other) const {
	return getScore() > other.getScore();
}

std::weak_ordering ICapabilitiesStruct::operator<=>(const ICapabilitiesStruct &other) const {
	return getScore() <=> other.getScore();
}

uint32_t PositionalSupport::getScore() const {
	return static_cast<uint32_t>(
		(mandatory << 3U) |
		(optional << 2U) |
		(optional_before_mandatory << 1U) |
		(booleans << 0U)
	);
}

bool PositionalSupport::operator>(const PositionalSupport &other) const {
	return getScore() > other.getScore();
}

std::weak_ordering PositionalSupport::operator<=>(const PositionalSupport &other) const {
	return getScore() <=> other.getScore();
}

uint32_t DashedSupport::getScore() const {
	return static_cast<uint32_t>(
		(optional << 1U) |
		(mandatory << 0U)
	);
}

bool DashedSupport::operator>(DashedSupport &other) const {
	return getScore() > other.getScore();
}

std::weak_ordering DashedSupport::operator<=>(const DashedSupport &other) const {
	return getScore() <=> other.getScore();
}

uint32_t HelpCapabilities::getScore() const {
	return static_cast<uint32_t>(
		(error_message << 5U) |
		(help << 4U) |
		(usage << 3U) |
		(descr << 2U) |
		(value_name << 1U) |
		(value_unit << 0U)
	);
}

std::weak_ordering HelpCapabilities::operator<=>(const HelpCapabilities &other) const {
	return getScore() <=> other.getScore();
}

bool HelpCapabilities::operator>(HelpCapabilities &other) const {
	return getScore() > other.getScore();
}

uint32_t BellsAndWhistlesSupport::getScore() const {
	return static_cast<uint32_t>(
		(auto_complete << 2U) |
		(wide_strings << 1U) |
		(path_validation << 0U)
	);
}

std::weak_ordering BellsAndWhistlesSupport::operator<=>(const BellsAndWhistlesSupport &other) const {
	return getScore() <=> other.getScore();
};

bool BellsAndWhistlesSupport::operator>(BellsAndWhistlesSupport &other) const {
	return getScore() > other.getScore();
}

uint32_t ImportantShit::getScore() const {
	return static_cast<uint32_t>(
		(permissive_license << 7U) |
		((!bug_memory_safety) << 6U) |
		((!other_grave_bugs) << 5U) |
		((!bug_terminates_itself) << 4U) |
		((!bug_prints_itself) << 3U) |
		((!thread_unsafe) << 2U) |
		(stable << 1U) |
		((!unmaintained) << 0U)
	);
}

std::weak_ordering ImportantShit::operator<=>(const ImportantShit &other) const {
	return getScore() <=> other.getScore();
};

bool ImportantShit::operator>(ImportantShit &other) const {
	return getScore() > other.getScore();
}

uint32_t ParserCapabilities::getScore() const {
	return (
		(important.getScore() << important_offset) |
		(dashed.getScore() << dashed_offset) |
		(help.getScore() << help_offset) |
		(native_positional.getScore() << native_positional_offset) |
		bellsAndWhistles.getScore()
	);
}

std::weak_ordering ParserCapabilities::operator<=>(const ParserCapabilities &other) const {
	return getScore() <=> other.getScore();
};

bool ParserCapabilities::operator>(ParserCapabilities &other) const {
	return getScore() > other.getScore();
}
