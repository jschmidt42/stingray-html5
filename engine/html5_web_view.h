#pragma once

#include <engine_plugin_api/plugin_api.h>
#include <plugin_foundation/vector2.h>
#include <plugin_foundation/vector3.h>

#include <include/cef_browser.h>
#include <include/cef_client.h>
#include <include/wrapper/cef_message_router.h>

#include <functional>
#include <unordered_map>

namespace PLUGIN_NAMESPACE {

typedef CefRefPtr<CefMessageRouterBrowserSide::Callback> FunctionCallback;
typedef std::function<void(void*, FunctionCallback)> FunctionHandler;
typedef std::unordered_map<std::string, FunctionHandler> FunctionHandlerMap;

class WebView : public CefClient,
	public CefLifeSpanHandler,
	public CefLoadHandler,
	public CefRequestHandler,
	public CefDragHandler,
	public CefKeyboardHandler,
	public CefContextMenuHandler,
	public CefDisplayHandler,
	public CefRenderHandler,
	public CefMessageRouterBrowserSide::Handler
{
public:

	explicit WebView(WindowPtr window, MaterialPtr material);
	virtual ~WebView();

	void load_page(const char* new_url);
	void open_dev_tools(const CefPoint& pt = CefPoint()) const;
	void execute(const char* script_code) const;
	void focus() const;
	void reload() const;
	void close_browser();
	void add_execute_handler(const char* name, FunctionHandler handler);
	CefRefPtr<CefBrowser> browser() const { return _browser; }
	stingray_plugin_foundation::Vector2 resolution() const { return stingray_plugin_foundation::vector2(_resolution[0], _resolution[1]); };
	void set_resolution(int cx, int cy);
	void invalidate() const;

	enum {
		LEFT, RIGHT, MIDDLE, EXTRA_1, EXTRA_2,
		LEFT_DOUBLE, RIGHT_DOUBLE, MIDDLE_DOUBLE, EXTRA_1_DOUBLE, EXTRA_2_DOUBLE,
		WHEEL_UP, WHEEL_DOWN, WHEEL_LEFT, WHEEL_RIGHT
	};

	void send_mouse_event(int sx, int sy, int button, bool up);

	static void on_activate(void *obj, int active);
	static void on_char_down(void *obj, int char_code);
	static void on_key_down(void *obj, int virtual_key, int repeat_count, int scan_code, int extended, int previous_state);
	static void on_key_up(void *obj, int virtual_key, int scan_code, int extended);
	static void on_mouse_down(void *obj, int button);
	static void on_mouse_up(void *obj, int button);
	static void on_mouse_move(void *obj, ConstVector3Ptr delta);
	static void on_mouse_wheel(void *obj, ConstVector3Ptr delta);
	static void on_cursor_pos(void *obj, unsigned x, unsigned y);
	static void on_set_focus(void *obj);
	static void on_resize(void *obj, WindowPtr window, int x, int y, int width, int height);

protected: // CefClient

	CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() OVERRIDE { return this; }
	CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE { return this; }
	CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE { return this; }
	CefRefPtr<CefKeyboardHandler> GetKeyboardHandler() OVERRIDE { return this; }
	CefRefPtr<CefDragHandler> GetDragHandler() OVERRIDE { return this; }
	CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE { return this; }
	CefRefPtr<CefRequestHandler> GetRequestHandler() OVERRIDE { return this; }
	CefRefPtr<CefRenderHandler> GetRenderHandler() OVERRIDE { return this; }

	bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message) OVERRIDE;

protected: // CefLifeSpanHandler

	void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
	void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
	void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, TransitionType transition_type) OVERRIDE;

protected: // CefRequestHandler

	void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser, TerminationStatus status) OVERRIDE;
	bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool is_redirect) OVERRIDE;
	void OnRenderViewReady(CefRefPtr<CefBrowser> browser) OVERRIDE;

protected: // CefDragHandler

protected: // CefKeyboardHandler

protected: // CefContextMenuHandler

protected: // CefDisplayHandler

	bool OnConsoleMessage(CefRefPtr<CefBrowser> browser, const CefString& message, const CefString& source, int line) OVERRIDE;

protected: // CefMessageRouterBrowserSide::Handler

	bool OnQuery(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int64 query_id, const CefString& request,
		bool persistent, CefRefPtr<Callback> callback) OVERRIDE;

protected: // CefRenderHandler interface

	bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) OVERRIDE;
	void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const void* buffer, int width, int height) OVERRIDE;

private:

	void create_browser(CefString const& url);
	void clean_texture_buffer();
	WindowPtr get_window_or_default() const;

	static void send_mouse_event(void* obj, int button, bool up);

	WindowPtr _window;
	MaterialPtr _material;
	uint32_t _texture_buffer_handle;
	uint32_t _texture_buffer_size;
	std::string _current_url;
	CefRefPtr<CefBrowser> _browser;
	FunctionHandlerMap _function_handlers;
	CefRefPtr<CefMessageRouterBrowserSide> _message_router;
	uint32 _modifiers;
	int _cursor_pos[2];
	int _resolution[2];
	IMPLEMENT_REFCOUNTING(WebView)
};

} // end namespace
