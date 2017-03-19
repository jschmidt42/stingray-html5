
#include "html5_web_app.h"
#include "html5_api.h"
#include "html5_web_browser.h"
#include "html5_web_page.h"

#include <engine_plugin_api/plugin_api.h>
#include <plugin_foundation/platform.h>
#include <plugin_foundation/allocator.h>
#include <plugin_foundation/assert.h>

#define DEFINE_STINGRAY_API 1
#include "stingray_api.h"

namespace PLUGIN_NAMESPACE {

/**
 * Main web app instance.
 */
CefRefPtr<WebApp> web_app = nullptr;

/**
 * Main plugin allocator.
 */
ApiAllocator allocator(nullptr, nullptr);

/**
 * Indicate the plugin name.
 */
const char *get_name() { return "html5_plugin"; }

/**
* Setup runtime and compiler common resources, such as allocators.
*/
void setup_plugin(GetApiFunction get_engine_api)
{
	stingray::api::allocator_api = (AllocatorApi*)get_engine_api(ALLOCATOR_API_ID);
	if (stingray::api::allocator_object == nullptr) {
		stingray::api::allocator_object = stingray::api::allocator_api->make_plugin_allocator(get_name());
		allocator = ApiAllocator(stingray::api::allocator_api, stingray::api::allocator_object);
	}

	stingray::api::log = (LoggingApi*)get_engine_api(LOGGING_API_ID);
	stingray::api::error = (ErrorApi*)get_engine_api(ERROR_API_ID);
	stingray::api::error_context = (ErrorContextApi*)get_engine_api(ERROR_CONTEXT_API_ID);
	stingray::api::resource_manager = (ResourceManagerApi*)get_engine_api(RESOURCE_MANAGER_API_ID);
	stingray::api::thread = (ThreadApi*)get_engine_api(THREAD_API_ID);
	stingray::api::profiler = (ProfilerApi*)get_engine_api(PROFILER_API_ID);
}

/**
 * Unload compiler and runtime plugin resources.
 */
void unload_common_plugin_resources()
{
	if (stingray::api::allocator_object != nullptr) {
		XENSURE(allocator.api());
		allocator = ApiAllocator(nullptr, nullptr);
		stingray::api::allocator_api->destroy_plugin_allocator(stingray::api::allocator_object);
		stingray::api::allocator_object = nullptr;
	}
}

/**
 * Initialize the plugin resources.
 */
void setup_runtime_plugin(GetApiFunction get_engine_api)
{
	setup_plugin(get_engine_api);

	// Load engine APIs
	stingray::api::script = (ScriptApi*)get_engine_api(C_API_ID);
	stingray::api::lua = (LuaApi*)get_engine_api(LUA_API_ID);
	stingray::api::render_buffer = (RenderBufferApi*)get_engine_api(RENDER_BUFFER_API_ID);
	stingray::api::unit = (UnitApi*)get_engine_api(UNIT_API_ID);
	stingray::api::renderer = (RenderInterfaceApi*)get_engine_api(RENDER_INTERFACE_API_ID);
	stingray::api::unit_reference = (UnitReferenceApi*)get_engine_api(UNIT_REFERENCE_API_ID);

	// Create main web app.
	web_app = WebApp::init();

	// Load lua Web API
	load_lua_api(stingray::api::lua);

	setup_web_page_database();

	// Initialize modules
	browser::init();
}

/**
 * Load browser URLs for units that defines one.
 */
void units_spawned(CApiUnit **units, unsigned count)
{
	for (unsigned i = 0; i < count; ++i) {
		auto unit = units[i];
		browser::try_load(unit);
	}
}

/**
 * Release browser resources for units that defined an browser URL.
 */
void units_unspawned(CApiUnit **units, unsigned count)
{
	for (unsigned i = 0; i < count; ++i) {
		auto unit = units[i];
		browser::try_unload(unit);
	}
}

/**
 * Update HTMl5 resources every update/frame.
 */
void update_plugin(float dt)
{
	WebApp::update();
}

/**
* Indicate to the resource manager that we'll be using our plugin resource type.
*/
void setup_resources(GetApiFunction get_engine_api)
{
	setup_plugin(get_engine_api);

	setup_web_page_resources(get_engine_api);
}

/**
 * Unload plugin resources.
 */
void unload_resources()
{
	unload_common_plugin_resources();
}

/**
 * Initialize the HTMl5 content compiler.
 */
void setup_data_compiler(GetApiFunction get_engine_api)
{
	setup_plugin(get_engine_api);

	stingray::api::data_compiler = (DataCompilerApi*)get_engine_api(DATA_COMPILER_API_ID);
	stingray::api::data_compile_params = (DataCompileParametersApi*)get_engine_api(DATA_COMPILE_PARAMETERS_API_ID);

	setup_web_page_compiler(get_engine_api);
}

/**
 * Release all compiler resources.
 */
void shutdown_data_compiler()
{
	unload_common_plugin_resources();
}

/**
 * Release all plugin resources.
 */
void unload_plugin()
{
	browser::shutdown();
	unload_lua_api(stingray::api::lua);
	shutdown_web_page_database();
	WebApp::shutdown();

	unload_common_plugin_resources();
}

} // end namespace

/**
 * `get_plugin_api` gets called by Stingray engine to define main API callbacks.
 */
extern "C" PLUGIN_DLLEXPORT void *get_plugin_api(unsigned api_id)
{
	using namespace PLUGIN_NAMESPACE;

	if (api_id == PLUGIN_API_ID) {
		static struct PluginApi plugin_api = { nullptr };
		plugin_api.get_name = get_name;
		plugin_api.setup_game = setup_runtime_plugin;
		plugin_api.update_game = update_plugin;
		plugin_api.units_spawned = units_spawned;
		plugin_api.units_unspawned = units_unspawned;
		plugin_api.setup_resources = setup_resources;
		plugin_api.shutdown_resources = unload_resources;
		plugin_api.setup_data_compiler = setup_data_compiler;
		plugin_api.shutdown_data_compiler = shutdown_data_compiler;
		plugin_api.shutdown_game = unload_plugin;
		return &plugin_api;
	}
	return nullptr;
}
