#pragma once

/**
 * Define Stingray Engine callbacks and API bindings.
 */

#include <engine_plugin_api/plugin_api.h>

#if !DEFINE_STINGRAY_API
	#include <plugin_foundation/allocator.h>
#endif

#include <plugin_foundation/string.h>
#include <plugin_foundation/vector.h>

namespace stingray_plugin_foundation {
	namespace string {
		inline void split(const char *s, const char *split_by, Vector<DynamicString> &result)
		{
			Allocator & allocator = result.allocator();
			DynamicString a(allocator), b(allocator);
			split(s, split_by, a, b);
			while (true) {
				if (!a.empty()) {
					result.resize(result.size() + 1);
					result.back() = a;
				}
				if (b.empty())
					break;
				DynamicString temp = b;
				split(temp.c_str(), split_by, a, b);
			}
		}

		#if defined(DEFINE_STINGRAY_API)
			void join(const DynamicString* strings, unsigned num_strings, const char* separator, DynamicString& result, SkipEmpty skip_empty)
			{
				for (unsigned i = 0; i < num_strings; ++i, strings++) {
					if (skip_empty != SKIP_EMPTY || strings->size() > 0) {
						append(result, strings->c_str());
						if (i+1 < num_strings)
							append(result, separator);
					}
				}
			}
		#endif
	}

	namespace path {
		inline DynamicString get_extension(const char *path, Allocator &a)
		{
			DynamicString extension(a);
			int n = (int)strlen(path);
			for (int i = n - 1; i >= 0; --i) {
				if (path[i] == '.') {
					append(extension, path + i);
					return extension;
				}
			}
			return extension;
		}
	}
}

namespace PLUGIN_NAMESPACE {

#if !DEFINE_STINGRAY_API
	extern stingray_plugin_foundation::ApiAllocator allocator;
#endif

using namespace stingray_plugin_foundation;

#if defined(DEFINE_STINGRAY_API)
	#define PLUGIN_NAMESPACE_API_EXTERN
	#define PLUGIN_NAMESPACE_INITIALIZE_API = nullptr
#else
	#define PLUGIN_NAMESPACE_API_EXTERN extern
	#define PLUGIN_NAMESPACE_INITIALIZE_API
#endif

namespace stingray { namespace api {

PLUGIN_NAMESPACE_API_EXTERN AllocatorApi* allocator_api PLUGIN_NAMESPACE_INITIALIZE_API;
PLUGIN_NAMESPACE_API_EXTERN AllocatorObject* allocator_object PLUGIN_NAMESPACE_INITIALIZE_API;
PLUGIN_NAMESPACE_API_EXTERN LuaApi* lua PLUGIN_NAMESPACE_INITIALIZE_API;
PLUGIN_NAMESPACE_API_EXTERN ScriptApi* script PLUGIN_NAMESPACE_INITIALIZE_API;
PLUGIN_NAMESPACE_API_EXTERN RenderInterfaceApi *renderer PLUGIN_NAMESPACE_INITIALIZE_API;
PLUGIN_NAMESPACE_API_EXTERN RenderBufferApi *render_buffer PLUGIN_NAMESPACE_INITIALIZE_API;
PLUGIN_NAMESPACE_API_EXTERN UnitApi* unit PLUGIN_NAMESPACE_INITIALIZE_API;
PLUGIN_NAMESPACE_API_EXTERN UnitReferenceApi* unit_reference PLUGIN_NAMESPACE_INITIALIZE_API;
PLUGIN_NAMESPACE_API_EXTERN LoggingApi* log PLUGIN_NAMESPACE_INITIALIZE_API;
PLUGIN_NAMESPACE_API_EXTERN ErrorApi *error PLUGIN_NAMESPACE_INITIALIZE_API;
PLUGIN_NAMESPACE_API_EXTERN ErrorContextApi *error_context PLUGIN_NAMESPACE_INITIALIZE_API;
PLUGIN_NAMESPACE_API_EXTERN ResourceManagerApi* resource_manager PLUGIN_NAMESPACE_INITIALIZE_API;
PLUGIN_NAMESPACE_API_EXTERN ThreadApi* thread PLUGIN_NAMESPACE_INITIALIZE_API;
PLUGIN_NAMESPACE_API_EXTERN ProfilerApi* profiler PLUGIN_NAMESPACE_INITIALIZE_API;
	
PLUGIN_NAMESPACE_API_EXTERN DataCompilerApi *data_compiler PLUGIN_NAMESPACE_INITIALIZE_API;
PLUGIN_NAMESPACE_API_EXTERN DataCompileParametersApi * data_compile_params PLUGIN_NAMESPACE_INITIALIZE_API;

}}

}
