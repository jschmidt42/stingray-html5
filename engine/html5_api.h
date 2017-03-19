#pragma once

#include <engine_plugin_api/plugin_api.h>

#include <include/cef_v8.h>

namespace PLUGIN_NAMESPACE {

// WebApp/Engine sync APIs
void set_signal();
void sync_signal();
void wait_for_sync();
struct SyncEngine { SyncEngine() { set_signal(); } ~SyncEngine() { wait_for_sync(); } };

// Stingray Web API Lua bindings
void load_lua_api(LuaApi* env);
void unload_lua_api(LuaApi* env);

// Stingray API JavaScript bindings
void bind_api(CefRefPtr<CefV8Value> stingray_ns);
void bind_api_web_app(CefRefPtr<CefV8Value> stingray_ns);
void bind_api_web_view(CefRefPtr<CefV8Value> stingray_ns);
void bind_api_host(CefRefPtr<CefV8Value> stingray_ns);
void bind_api_math(CefRefPtr<CefV8Value> stingray_ns);
void bind_api_application(CefRefPtr<CefV8Value> stingray_ns, const ApplicationCApi* api);
void bind_api_world(CefRefPtr<CefV8Value> stingray_ns, const WorldCApi* api);
void bind_api_input(CefRefPtr<CefV8Value> stingray_ns, const InputCApi* api);
void bind_api_unit(CefRefPtr<CefV8Value> stingray_ns, const UnitCApi* api);
void bind_api_camera(CefRefPtr<CefV8Value> stingray_ns, const CameraCApi* api);
void bind_api_window(CefRefPtr<CefV8Value> stingray_ns, const WindowCApi* api);
void bind_api_level(CefRefPtr<CefV8Value> stingray_ns, const LevelCApi* api);
void bind_api_fs(CefRefPtr<CefV8Value> stingray_ns);

}
