#include <HydrArgs/scoring.hpp>

using namespace HydrArgs;

uint32_t ICapabilitiesStruct::getScore() const {
	return 0;
}

bool ICapabilitiesStruct::operator>(ICapabilitiesStruct &other){
	return getScore() > other.getScore();
}

uint32_t PositionalSupport::getScore() const {
	return static_cast<uint32_t>(
		(mandatory << 3u) |
		(optional << 2u) |
		(optional_before_mandatory << 1u) |
		(booleans << 0u)
	);
}

bool PositionalSupport::operator>(PositionalSupport &other){
	return getScore() > other.getScore();
}

uint32_t DashedSupport::getScore() const {
	return static_cast<uint32_t>(
		(optional << 1) |
		(mandatory << 0)
	);
}

bool DashedSupport::operator>(DashedSupport &other){
	return getScore() > other.getScore();
}

uint32_t HelpCapabilities::getScore() const {
	return static_cast<uint32_t>(
		(error_message << 5) |
		(help << 4) |
		(usage << 3) |
		(descr << 2) |
		(value_name << 1) |
		(value_unit << 0)
	);
}

bool HelpCapabilities::operator>(HelpCapabilities &other) const {
	return getScore() > other.getScore();
}

uint32_t BellsAndWhistlesSupport::getScore() const {
	return static_cast<uint32_t>(
		(auto_complete << 2) |
		(wide_strings << 1) |
		(path_validation << 0)
	);
}

bool BellsAndWhistlesSupport::operator>(BellsAndWhistlesSupport &other) const {
	return getScore() > other.getScore();
}

uint32_t ImportantShit::getScore() const {
	return static_cast<uint32_t>(
		(permissive_license << 7) |
		((!bug_memory_safety) << 6) |
		((!other_grave_bugs) << 5) |
		((!bug_terminates_itself) << 4) |
		((!bug_prints_itself) << 3) |
		((!thread_unsafe) << 2) |
		(stable << 1) |
		((!unmaintained) << 0)
	);
}

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

bool ParserCapabilities::operator>(ParserCapabilities &other) const {
	return getScore() > other.getScore();
}
