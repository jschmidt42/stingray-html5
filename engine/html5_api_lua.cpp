#include "html5_api.h"
#include "html5_web_view.h"
#include "html5_api_bindings.h"

#include <engine_plugin_api/plugin_api.h>
#include <engine_plugin_api/c_api/c_api_window.h>
#include <plugin_foundation/exception_handling.h>
#include <plugin_foundation/quaternion.h>

#include <include/cef_app.h>

namespace PLUGIN_NAMESPACE {

#define LUA_ASSERT(test, L, msg, ...) do {if (!(test)) _lua->lib_error(L, msg,  ## __VA_ARGS__);} while (0)

class LuaWebApp : public CefClient,
	public CefLifeSpanHandler,
	public CefDisplayHandler
{
public:

	bool is_ready() const { return !!_browser; }

	bool emit_event(const char* event_name, const char* detail = "{}") const
	{
		if (!_browser)
			return false;
		char event_script[2048] = { '\0' };
		if (sprintf(event_script, "window.dispatchEvent(new CustomEvent('%s', { detail: %s }));", event_name, detail) <= 0)
			return false;
		return execute(event_script);
	}

	bool execute(const char* script) const
	{
		if (!_browser)
			return false;
		_browser->GetMainFrame()->ExecuteJavaScript(script, _browser->GetMainFrame()->GetURL(), 0);
		return true;
	}

	void close() const
	{
		if (!_browser)
			return;
		_browser->GetHost()->CloseBrowser(true);
	}

protected: // CefClient

	CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE { return this; }
	CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE { return this; }

protected: // CefLifeSpanHandler

	void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE { _browser = browser; }
	void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE { _browser = nullptr; }

protected: // CefDisplayHandler

	bool OnConsoleMessage(CefRefPtr<CefBrowser> browser, const CefString& message, const CefString& source, int line) OVERRIDE
	{
		stingray::api::log->info("WebApp", message.ToString().c_str());
		return false;
	}

private:

	CefRefPtr<CefBrowser> _browser;
	IMPLEMENT_REFCOUNTING(LuaWebApp)
};

typedef Vector<CefRefPtr<LuaWebApp>> LuaWebAppVector;
LuaWebAppVector* web_apps = nullptr;

template <typename T> T get_pointer(lua_State *L, int i)
{
	return (T)stingray::api::lua->topointer(L, i);
}

template <typename T> T get_object(lua_State *L, int i)
{
	void *ud = stingray::api::lua->touserdata(L, i);
	void *p = *(void **)ud;
	return (T)p;
}

/* @adoc lua
	@obj stingray.WebView : userdata
	@grp core
	@des Represents a web view to display HTML5 content.
*/
CefRefPtr<WebView> get_web_view(lua_State *L, unsigned index = 1)
{
	return get_pointer<WebView*>(L, index);
}

/* @adoc lua
   @obj stingray.WebApp : userdata
   @grp core
   @des Represents a web app to run HTML5 application.
*/
CefRefPtr<LuaWebApp> get_web_app(lua_State *L, unsigned index = 1)
{
	return get_pointer<LuaWebApp*>(L, index);
}

void load_lua_api(LuaApi* env)
{
	/* @adoc lua
	   @sig stingray.WebApp.create(url:string) : stingray.WebApp
	   @arg url				URL of the web app to be loaded.
	   @ret stingray.WebApp	Created web app.
	   @des Create a web app that runs in the background and can evaluate JavaScript.
	*/
	env->add_module_function("WebApp", "create", [](lua_State* L)
	{
		// ReSharper disable once CppLocalVariableWithNonTrivialDtorIsNeverUsed
		FpuUnsafeScope fus;

		if (web_apps == nullptr) {
			web_apps = MAKE_NEW(allocator, LuaWebAppVector, allocator);
		}

		const char* url = stingray::api::lua->tolstring(L, 1, nullptr);

		CefWindowInfo info;
		info.SetAsPopup(nullptr, "web app");
		info.style &= ~WS_VISIBLE;
		info.ex_style |= WS_EX_NOACTIVATE;

		CefBrowserSettings brsettings;
		brsettings.file_access_from_file_urls = STATE_ENABLED;
		brsettings.universal_access_from_file_urls = STATE_ENABLED;
		brsettings.javascript_access_clipboard = STATE_ENABLED;
		brsettings.local_storage = STATE_ENABLED;
		brsettings.web_security = STATE_DISABLED;
		brsettings.plugins = STATE_ENABLED;

		CefRefPtr<LuaWebApp> client = new LuaWebApp();
		CefBrowserHost::CreateBrowser(info, client, url, brsettings, nullptr);

		web_apps->push_back(client);

		stingray::api::lua->pushlightuserdata(L, client);
		return 1;
	});

	/* @adoc lua
	   @sig stingray.WebApp.update(dt: number) : nil
	   @des Update and synchronize the web app with the engine update loop.
	*/
	env->add_module_function("WebApp", "update", [](lua_State* L)
	{
		if (!web_apps)
			return 0;

		CefRefPtr<LuaWebApp> web_app = get_web_app(L, 1);
		if (!web_app->is_ready())
			return 0;

		auto dt = stingray::api::lua->tonumber(L, 2);
		char update_call_script[128];
		sprintf(update_call_script, "if (typeof window.update === 'function') window.update(%f); stingray.WebApp.sync();", dt);
		// ReSharper disable once CppLocalVariableWithNonTrivialDtorIsNeverUsed
		SyncEngine sync;
		web_app->execute(update_call_script);
		return 0;
	});

	/* @adoc lua
	   @sig stingray.WebApp.render() : nil
	   @des Render and synchronize the web app with the engine render frame.
	*/
	env->add_module_function("WebApp", "render", [](lua_State* L)
	{
		if (!web_apps)
			return 0;

		CefRefPtr<LuaWebApp> web_app = get_web_app(L, 1);
		if (!web_app->is_ready())
			return 0;

		// ReSharper disable once CppLocalVariableWithNonTrivialDtorIsNeverUsed
		SyncEngine sync;
		web_app->execute("if (typeof window.render === 'function') window.render(); stingray.WebApp.sync();");
		return 0;
	});

	/* @adoc lua
	   @sig stingray.WebApp.destroy(web_app:stingray.WebApp) : nil
	   @des Destroy and release the web app resources.
	*/
	env->add_module_function("WebApp", "destroy", [](lua_State* L)
	{
		if (!web_apps)
			return 0;

		CefRefPtr<LuaWebApp> web_app = get_web_app(L, 1);
		if (!web_app->is_ready())
			return 0;
		{
			// ReSharper disable once CppLocalVariableWithNonTrivialDtorIsNeverUsed
			SyncEngine sync;
			web_app->execute("if (typeof window.shutdown === 'function') window.shutdown(); stingray.WebApp.sync();");
		}
		web_app->close();
		web_apps->erase(web_app);
		if (web_apps->empty()) {
			MAKE_DELETE_TYPE(allocator, LuaWebAppVector, web_apps);
			web_apps = nullptr;
		}
		return 0;
	});

	/* @adoc lua
	   @sig stingray.WebView.create(url:string, material:Material) : stingray.WebView
	   @arg url				URL to load the  the web view at creation.
	   @arg material			Material in which to set the texture slot with the web view rendering.
	   @ret stingray.WebView	Returns the created web view.
	   @des Creates a web view to display HTML5 content.
	*/
	env->add_module_function("WebView", "create", [](lua_State *L)
	{
		// ReSharper disable CppLocalVariableWithNonTrivialDtorIsNeverUsed
		FpuUnsafeScope fus;

		const char* url = stingray::api::lua->tolstring(L, 1, nullptr);
		WindowPtr window = stingray::api::lua->isuserdata(L, 2) ? get_object<WindowPtr>(L, 2) : stingray::api::script->Window->get_main_window();
		MaterialPtr material = get_pointer<MaterialPtr>(L, 3);

		CefRefPtr<WebView> view = new WebView(window, material);
		view->load_page(url);

		stingray::api::lua->pushlightuserdata(L, view);
		return 1;
	});

	/* @adoc lua
	   @sig stingray.WebView.open_dev_tool(self:stingray.WebView) : nil
	   @arg stingray.WebView	Target web view
	   @des Opens the view dev tools.
	*/
	env->add_module_function("WebView", "open_dev_tools", [](lua_State *L) {
		CefRefPtr<WebView> web_view = get_web_view(L);
		web_view->open_dev_tools();
		return 0;
	});

	/* @adoc lua
	   @sig stingray.WebView.update(dt: number) : nil
	   @des Call update root function in the web view.
	*/
	env->add_module_function("WebView", "update", [](lua_State *L)
	{
		CefRefPtr<WebView> web_view = get_web_view(L, 1);
		auto dt = stingray::api::lua->tonumber(L, 2);
		char update_call_string[64];
		sprintf(update_call_string, "if (typeof update === 'function') update(%f);", dt);
		web_view->execute(update_call_string);
		return 0;
	});

	/* @adoc lua
	   @sig stingray.WebView.render() : nil
	   @des Called when it's time to render the frame..
	*/
	env->add_module_function("WebView", "render", [](lua_State *L) {
		CefRefPtr<WebView> web_view = get_web_view(L, 1);
		web_view->execute("if (typeof render === 'function') render();");
		return 0;
	});

	/* @adoc lua
	   @sig stingray.WebView.destroy(self:stingray.WebView) : nil
	   @arg stingray.WebView	Target web view
	   @des Destroy the web view and all corresponding resources.
	*/
	env->add_module_function("WebView", "destroy", [](lua_State *L) {
		FpuUnsafeScope fus;

		CefRefPtr<WebView> web_view = get_web_view(L, 1);
		web_view->close_browser();
		return 0;
	});

	/* @adoc lua
	   @sig stingray.WebView.char_down(self:stingray.WebView, char_code:number) : nil
	   @arg stingray.WebView	Target web view
	   @arg char_code			character key code
	   @des Trigger char key code event
	*/
	env->add_module_function("WebView", "char_down", [](lua_State *L) {
		CefRefPtr<WebView> web_view = get_web_view(L, 1);
		const int char_code = stingray::api::lua->tointeger(L, 2);
		WebView::on_char_down(web_view.get(), char_code);
		return 0;
	});

	/* @adoc lua
	   @sig stingray.WebView.key_down(self:stingray.WebView, key:number, repeat_count:number, scan_code:number, extended:boolean, previous_state:boolean) : nil
	   @arg stingray.WebView	Target web view
	   @arg key			key code
	   @arg repeat_count	repeat count
	   @arg scan_code		scan code
	   @arg extended		internal
	   @arg previous_state	internal
	   @des Trigger key down event
	*/
	env->add_module_function("WebView", "key_down", [](lua_State *L) {
		CefRefPtr<WebView> web_view = get_web_view(L, 1);

		if (stingray::api::lua->isstring(L, 2)) {
			WebView::on_char_down(web_view.get(), stingray::api::lua->tolstring(L, 2, nullptr)[0]);
		} else {
			const int key = stingray::api::lua->tointeger(L, 2);
			const int repeat_count = stingray::api::lua->tointeger(L, 3);
			const int scan_code = stingray::api::lua->tointeger(L, 4);
			const bool extended = stingray::api::lua->toboolean(L, 5) != 0;
			const bool previous_state = stingray::api::lua->toboolean(L, 6) != 0;
			WebView::on_key_down(web_view.get(), key, repeat_count, scan_code, extended, previous_state);
		}
		return 0;
	});

	/* @adoc lua
	   @sig stingray.WebView.key_up(self:stingray.WebView, key:number, scan_code:number, extended:boolean) : nil
	   @arg stingray.WebView	Target web view
	   @arg key			key code
	   @arg scan_code		scan code
	   @arg extended		internal
	   @des Trigger a key up event
	*/
	env->add_module_function("WebView", "key_up", [](lua_State *L) {
		CefRefPtr<WebView> web_view = get_web_view(L, 1);
		const int key = stingray::api::lua->tointeger(L, 2);
		const int scan_code = stingray::api::lua->tointeger(L, 3);
		const bool extended = stingray::api::lua->toboolean(L, 4) != 0;
		WebView::on_key_up(web_view.get(), key, scan_code, extended);
		return 0;
	});

	/* @adoc lua
	   @sig stingray.WebView.mouse_down(self:stingray.WebView, button:number) : nil
	   @arg stingray.WebView	Target web view
	   @arg button				mouse button being pressed down
	   @des Trigger mouse down event
	*/
	env->add_module_function("WebView", "mouse_down", [](lua_State *L) {
		CefRefPtr<WebView> web_view = get_web_view(L, 1);
		const int button = stingray::api::lua->tointeger(L, 2);
		WebView::on_mouse_down(web_view.get(), button);
		return 0;
	});

	/* @adoc lua
	   @sig stingray.WebView.mouse_up(self:stingray.WebView, button:number) : nil
	   @arg stingray.WebView	Target web view
	   @arg button				mouse button being released
	   @des Trigger mouse down event
	   @des
	*/
	env->add_module_function("WebView", "mouse_up", [](lua_State *L) {
		CefRefPtr<WebView> web_view = get_web_view(L, 1);
		const int button = stingray::api::lua->tointeger(L, 2);
		WebView::on_mouse_up(web_view.get(), button);
		return 0;
	});

	/* @adoc lua
	   @sig stingray.WebView.mouse_wheel(self:stingray.WebView, delta:Vector3) : nil
	   @arg stingray.WebView	Target web view
	   @arg delta				Mouse wheel delta
	   @des Trigger a mouse wheel event
	*/
	env->add_module_function("WebView", "mouse_wheel", [](lua_State *L) {
		CefRefPtr<WebView> web_view = get_web_view(L, 1);
		const float* delta = stingray::api::lua->getvector3(L, 3);
		WebView::on_mouse_wheel(web_view.get(), (ConstVector3Ptr)delta);
		return 0;
	});

	/* @adoc lua
	   @sig stingray.WebView.set_cursor_pos(self:stingray.WebView, x:number, y:number) : nil
	   @arg stingray.WebView	Target web view
	   @arg x					View x mouse position
	   @arg y					View y mouse position
	   @des Set the view mouse cursor position
	*/
	env->add_module_function("WebView", "set_cursor_pos", [](lua_State *L) {
		CefRefPtr<WebView> web_view = get_web_view(L, 1);
		const int x = stingray::api::lua->tointeger(L, 2);
		const int y = stingray::api::lua->tointeger(L, 3);
		WebView::on_cursor_pos(web_view.get(), x, y);
		return 0;
	});
}

void unload_lua_api(LuaApi* env)
{
	env->remove_all_module_entries("WebApp");
	env->remove_all_module_entries("WebView");

	// Release web app resources.
	if (!web_apps)
		return;

	for (unsigned i = 0; i < web_apps->size(); ++i) {
		(*web_apps)[i]->close();
	}
	web_apps->clear();
	MAKE_DELETE_TYPE(allocator, LuaWebAppVector, web_apps);
	web_apps = nullptr;
}

} // end namespace
