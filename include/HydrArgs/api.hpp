#pragma once

#ifdef _MSC_VER
	#define HYDRARGS_EXPORT_API __declspec(dllexport)
	#define HYDRARGS_IMPORT_API __declspec(dllimport)
	#define HYDRARGS_PACKED __declspec(packed)
#else
	#define HYDRARGS_PACKED [[gnu::packed]]
	#ifdef _WIN32
		#define HYDRARGS_EXPORT_API [[gnu::dllexport]]
		#define HYDRARGS_IMPORT_API [[gnu::dllimport]]
	#else
		#define HYDRARGS_EXPORT_API [[gnu::visibility("default")]]
		#define HYDRARGS_IMPORT_API
	#endif
#endif

#ifdef HYDRARGS_EXPORTS
	#define HYDRARGS_API HYDRARGS_EXPORT_API
#else
	#define HYDRARGS_API HYDRARGS_IMPORT_API
#endif

#ifdef HYDRARGS_BACKEND_EXPORTS
	#define HYDRARGS_BACKEND_API HYDRARGS_EXPORT_API
#else
	#define HYDRARGS_BACKEND_API HYDRARGS_IMPORT_API
#endif
