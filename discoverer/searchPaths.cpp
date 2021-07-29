#include "searchPaths.hpp"

const std::string searchPaths[] {
	"",
	#if defined(HYDRARGS_SCAN_BACKENDS_DIR)
		"../backends"
	#endif
};

const uint8_t searchPathsCount = sizeof(searchPaths) / sizeof(searchPaths[0]);
