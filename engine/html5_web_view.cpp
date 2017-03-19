#include "html5_web_view.h"
#include "html5_web_app.h"
#include "html5_api.h"

#include "stingray_api.h"

#include <plugin_foundation/id_string.h>
#include <plugin_foundation/assert.h>

#include <include/cef_browser.h>

namespace PLUGIN_NAMESPACE {

static const auto HTML5_TEXTURE_SLOT_NAME = IdString32("html5_texture").id();

cef_mouse_button_type_t stingray_mouse_event_to_cef(int event_type)
{
	if (event_type == 0)
		return MBT_LEFT;
	if (event_type == 1)
		return MBT_RIGHT;
	return MBT_MIDDLE;
}

WebView::WebView(WindowPtr window, MaterialPtr material)
	: _window(window)
	, _material(material)
	, _texture_buffer_handle(UINT_MAX)
	, _texture_buffer_size(0)
	, _current_url()
	, _function_handlers()
	, _modifiers(EVENTFLAG_NONE)
{
	CefMessageRouterConfig config;
	config.js_query_function = "cefQuery";
	config.js_cancel_function = "cefCancel";

	_resolution[0] = 0;
	_resolution[1] = 0;

	_message_router = CefMessageRouterBrowserSide::Create(config);

	_cursor_pos[0] = _cursor_pos[1] = -1;
	if ((GetKeyState(VK_CAPITAL) & 0x0001)!=0)
		_modifiers |= EVENTFLAG_CAPS_LOCK_ON;
	else
		_modifiers &= ~EVENTFLAG_CAPS_LOCK_ON;
}

WebView::~WebView()
{
	close_browser();
}

WindowPtr WebView::get_window_or_default() const
{
	return _window ? _window : stingray::api::script->Window->get_main_window();
}

void WebView::create_browser(const CefString& url)
{
	CefWindowInfo info;

	auto window_owner = get_window_or_default();
	HWND hwnd = (HWND)stingray::api::script->Window->id(window_owner);
	info.SetAsWindowless(hwnd, true);

	CefBrowserSettings brsettings;
	brsettings.file_access_from_file_urls  = STATE_ENABLED;
	brsettings.universal_access_from_file_urls = STATE_ENABLED;
	brsettings.javascript_access_clipboard = STATE_ENABLED;
	brsettings.local_storage = STATE_ENABLED;
	brsettings.web_security = STATE_DISABLED;
	brsettings.plugins = STATE_ENABLED;

	CefBrowserHost::CreateBrowser(info, this, url, brsettings, nullptr);

	if (_window) {
		stingray::api::script->Window->add_activate_callback(_window, this, on_activate);
		stingray::api::script->Window->add_char_down_callback(_window, this, on_char_down);
		stingray::api::script->Window->add_cursor_pos_callback(_window, this, on_cursor_pos);
		stingray::api::script->Window->add_key_down_callback(_window, this, on_key_down);
		stingray::api::script->Window->add_key_up_callback(_window, this, on_key_up);
		stingray::api::script->Window->add_mouse_down_callback(_window, this, on_mouse_down);
		stingray::api::script->Window->add_mouse_up_callback(_window, this, on_mouse_up);
		stingray::api::script->Window->add_mouse_move_callback(_window, this, on_mouse_move);
		stingray::api::script->Window->add_set_focus_callback(_window, this, on_set_focus);
		stingray::api::script->Window->add_wheel_move_callback(_window, this, on_mouse_wheel);
		stingray::api::script->Window->add_resize_callback(_window, this, on_resize);
		stingray::api::script->Window->set_resizable(_window, true);
	}
}

void WebView::close_browser()
{
	if (_window) {
		stingray::api::script->Window->remove_activate_callback(_window, this, on_activate);
		stingray::api::script->Window->remove_char_down_callback(_window, this, on_char_down);
		stingray::api::script->Window->remove_cursor_pos_callback(_window, this, on_cursor_pos);
		stingray::api::script->Window->remove_key_down_callback(_window, this, on_key_down);
		stingray::api::script->Window->remove_key_up_callback(_window, this, on_key_up);
		stingray::api::script->Window->remove_mouse_down_callback(_window, this, on_mouse_down);
		stingray::api::script->Window->remove_mouse_up_callback(_window, this, on_mouse_up);
		stingray::api::script->Window->remove_mouse_move_callback(_window, this, on_mouse_move);
		stingray::api::script->Window->remove_set_focus_callback(_window, this, on_set_focus);
		stingray::api::script->Window->remove_wheel_move_callback(_window, this, on_mouse_wheel);
		stingray::api::script->Window->remove_resize_callback(_window, this, on_resize);
	}

	if (_browser) {
		_browser->GetHost()->CloseBrowser(true);
		_browser = nullptr;
	}

	clean_texture_buffer();
}

void WebView::clean_texture_buffer()
{
	if (_texture_buffer_handle == UINT_MAX)
		return;
	stingray::api::render_buffer->destroy_buffer(_texture_buffer_handle);
	_texture_buffer_handle = UINT_MAX;
	_texture_buffer_size = 0;
}

void WebView::open_dev_tools(const CefPoint& pt) const
{
	struct CefDebugWindowHandler : CefClient { IMPLEMENT_REFCOUNTING(CefDebugWindowHandler); };

	CefWindowInfo info;
	info.SetAsPopup(_browser->GetHost()->GetOpenerWindowHandle(), "Stingray Developer Tools");
	_browser->GetHost()->ShowDevTools(info, new CefDebugWindowHandler(), CefBrowserSettings(), pt);
}

void WebView::load_page(const char* new_url)
{
	_current_url = new_url;
	if (!_browser) {
		create_browser(new_url);
	} else {
		_browser->GetMainFrame()->LoadURL(new_url);
	}
}

void WebView::reload() const
{
	if (!_browser)
		return;
	_browser->Reload();
}

void WebView::execute(const char* script) const
{
	if (!_browser)
		return;
	_browser->GetMainFrame()->ExecuteJavaScript(script, "", 0);
}

void WebView::focus() const
{
	if (!_browser)
		return;
	_browser->GetHost()->SendFocusEvent(true);
}

void WebView::add_execute_handler(const char* name, FunctionHandler handler)
{
	_function_handlers[name] = handler;
}

void WebView::set_resolution(int cx, int cy)
{
	_resolution[0] = cx;
	_resolution[1] = cy;

	invalidate();
}

void WebView::invalidate() const
{
	if (!_browser)
		return;
	auto host = _browser->GetHost();
	host->NotifyMoveOrResizeStarted();
	host->Invalidate(PET_VIEW);
	host->WasResized();
}

void WebView::on_activate(void* obj, int active)
{
	WebView* web_view = static_cast<WebView*>(obj);
	if (!web_view->_browser)
		return;
	auto host = web_view->_browser->GetHost();
	host->SendFocusEvent(active != 0);
}

void WebView::on_char_down(void* obj, int char_code)
{
	WebView* web_view = static_cast<WebView*>(obj);
	auto host = web_view->_browser->GetHost();
	CefKeyEvent ke;
	ke.type = KEYEVENT_CHAR;
	ke.windows_key_code  = char_code;
	host->SendKeyEvent(ke);
}

void WebView::on_key_down(void* obj, int virtual_key, int /*repeat_count*/, int scan_code, int /*extended*/, int /*previous_state*/)
{
	WebView* web_view = static_cast<WebView*>(obj);
	auto host = web_view->_browser->GetHost();

	CefKeyEvent ke;
	ke.type = KEYEVENT_KEYDOWN;
	ke.windows_key_code = virtual_key;
	ke.native_key_code = scan_code;

	if (virtual_key == VK_SHIFT)
		web_view->_modifiers |= EVENTFLAG_SHIFT_DOWN;
	if (virtual_key == VK_MENU)
		web_view->_modifiers |= EVENTFLAG_ALT_DOWN;
	if (virtual_key == VK_CONTROL)
		web_view->_modifiers |= EVENTFLAG_CONTROL_DOWN;

	ke.modifiers = web_view->_modifiers;

	host->SendKeyEvent(ke);
}

void WebView::on_key_up(void* obj, int virtual_key, int scan_code, int /*extended*/)
{
	WebView* web_view = static_cast<WebView*>(obj);
	auto host = web_view->_browser->GetHost();
	CefKeyEvent ke;
	ke.type = KEYEVENT_KEYUP;
	ke.windows_key_code = virtual_key;
	ke.native_key_code = scan_code;

	if ((GetKeyState(VK_CAPITAL) & 0x0001)!=0)
		web_view->_modifiers |= EVENTFLAG_CAPS_LOCK_ON;
	else
		web_view->_modifiers &= ~EVENTFLAG_CAPS_LOCK_ON;

	if (virtual_key == VK_SHIFT)
		web_view->_modifiers &= ~EVENTFLAG_SHIFT_DOWN;
	if (virtual_key == VK_MENU)
		web_view->_modifiers &= ~EVENTFLAG_ALT_DOWN;
	if (virtual_key == VK_CONTROL)
		web_view->_modifiers &= ~EVENTFLAG_CONTROL_DOWN;

	ke.modifiers = web_view->_modifiers;
	host->SendKeyEvent(ke);
}

void WebView::send_mouse_event(int sx, int sy, int button, bool up)
{
	if (!_browser)
		return;

	auto host = _browser->GetHost();

	if (button == LEFT && !up)
		_modifiers |= EVENTFLAG_LEFT_MOUSE_BUTTON;
	else if (button == LEFT && up)
		_modifiers &= ~EVENTFLAG_LEFT_MOUSE_BUTTON;
	else if (button == MIDDLE && !up)
		_modifiers |= EVENTFLAG_MIDDLE_MOUSE_BUTTON;
	else if (button == MIDDLE && up)
		_modifiers &= ~EVENTFLAG_MIDDLE_MOUSE_BUTTON;
	else if (button == RIGHT && up)
		_modifiers &= ~EVENTFLAG_RIGHT_MOUSE_BUTTON;
	else if (button == RIGHT && up)
		_modifiers &= ~EVENTFLAG_RIGHT_MOUSE_BUTTON;

	cef_mouse_event_t me = { sx, sy, _modifiers };
	host->SendMouseClickEvent(me, stingray_mouse_event_to_cef(button), up, 1);
}

void WebView::send_mouse_event(void* obj, int button, bool up)
{
	WebView* web_view = static_cast<WebView*>(obj);
	XENSURE(web_view->_window != nullptr);

	auto rect = stingray::api::script->Window->rect(web_view->_window);
	int h = rect.pos[3];

	web_view->send_mouse_event(web_view->_cursor_pos[0], h - web_view->_cursor_pos[1], button, up);
}

void WebView::on_mouse_down(void* obj, int button)
{
	send_mouse_event(obj, button, false);
}

void WebView::on_mouse_up(void* obj, int button)
{
	send_mouse_event(obj, button, true);
}

void WebView::on_mouse_move(void*, ConstVector3Ptr)
{
	// do nothing, see on_cursor_pos
}

void WebView::on_mouse_wheel(void* obj, ConstVector3Ptr delta)
{
	WebView* web_view = static_cast<WebView*>(obj);
	auto host = web_view->_browser->GetHost();

	XENSURE(web_view->_window != nullptr);

	auto rect = stingray::api::script->Window->rect(web_view->_window);
	int h = rect.pos[3];

	cef_mouse_event_t mouse_move_event = {web_view->_cursor_pos[0], h-web_view->_cursor_pos[1], web_view->_modifiers};
	host->SendMouseWheelEvent(mouse_move_event, 0, static_cast<int>(delta->y*60.0f));
}

void WebView::on_cursor_pos(void* obj, unsigned x, unsigned y)
{
	WebView* web_view = static_cast<WebView*>(obj);
	auto host = web_view->_browser->GetHost();

	XENSURE(web_view->_window != nullptr);

	auto rect = stingray::api::script->Window->rect(web_view->_window);
	int h = rect.pos[3];
	web_view->_cursor_pos[0] = x;
	web_view->_cursor_pos[1] = y;

	cef_mouse_event_t mouse_move_event = {x, h-y, web_view->_modifiers};
	host->SendMouseMoveEvent(mouse_move_event, false);
}

void WebView::on_set_focus(void*)
{
	// not used
}

void WebView::on_resize(void* obj, WindowPtr window, int x, int y, int width, int height)
{
	WebView* web_view = static_cast<WebView*>(obj);
	web_view->set_resolution(width, height);
}

void WebView::OnRenderViewReady(CefRefPtr<CefBrowser>)
{
	_message_router->AddHandler(this, false);
}

void WebView::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
	if (_browser && browser->GetIdentifier() == _browser->GetIdentifier()) {
		_browser = nullptr;
		_message_router->RemoveHandler(this);
	}
	_message_router->OnBeforeClose(browser);
}

void WebView::OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser, TerminationStatus)
{
	_message_router->OnRenderProcessTerminated(browser);
}

bool WebView::OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest>, bool)
{
	_message_router->OnBeforeBrowse(browser, frame);
	return false;
}

bool WebView::OnConsoleMessage(CefRefPtr<CefBrowser>, const CefString& message, const CefString& source, int line)
{
	stingray::api::log->info("HTML5", message.ToString().c_str());
	return false;
}

bool WebView::OnQuery(CefRefPtr<CefBrowser>, CefRefPtr<CefFrame>, int64, const CefString& request, bool, FunctionCallback callback)
{
#if 0
	Buffer request_buffer(request.ToString().c_str(), static_cast<unsigned>(request.size()));
	DynamicConfigValue data(_allocator);
	sjson::parse(request_buffer, data);

	bool err = false;
	auto function_name = data["type"].to_string(err);
	auto func = _function_handlers.find(IdString64(function_name));
	if (func != _function_handlers.end()) {
		func->second(data, callback);
		return true;
	}
#endif
	// Function does not exist.
	return false;
}

bool WebView::GetViewRect(CefRefPtr<CefBrowser>, CefRect& out_rect)
{
	if (_resolution[0] == 0 || _resolution[1] == 0) {
		auto window = get_window_or_default();
		if (window == nullptr)
			return false;
		auto rect = stingray::api::script->Window->rect(window);
		int w = rect.pos[2], h = rect.pos[3];
		out_rect.Set(0, 0, w, h);
	} else {
		out_rect.Set(0, 0, _resolution[0], _resolution[1]);
	}

	return out_rect.width != 0 && out_rect.height != 0;
}

void WebView::OnPaint(CefRefPtr<CefBrowser>, PaintElementType, const RectList&, const void* buffer, int width, int height)
{
	if (_browser == nullptr || WebApp::closing())
		return;

	// Allocate texture buffer if it does not exist or if the requested size has changed.
	auto texture_buffer_size = width * height * 4;
	if (_texture_buffer_handle == UINT_MAX || _texture_buffer_size != texture_buffer_size) {
		clean_texture_buffer();

		RB_TextureBufferView texture_buffer_view;
		memset(&texture_buffer_view, 0, sizeof(texture_buffer_view));
		texture_buffer_view.width = width;
		texture_buffer_view.height = height;
		texture_buffer_view.depth = 1;
		texture_buffer_view.mip_levels = 1;
		texture_buffer_view.slices = 1;
		texture_buffer_view.type = RB_TEXTURE_TYPE_2D;
		texture_buffer_view.format = stingray::api::render_buffer->format(RB_INTEGER_COMPONENT, false, true, 8, 8, 8, 8); // ImageFormat::PF_R8G8B8A8;

		_texture_buffer_handle = stingray::api::render_buffer->create_buffer(texture_buffer_size, RB_VALIDITY_UPDATABLE, RB_TEXTURE_BUFFER_VIEW, &texture_buffer_view, buffer);
		_texture_buffer_size = texture_buffer_size;

		auto texture_buffer = stingray::api::render_buffer->lookup_resource(_texture_buffer_handle);
		stingray::api::script->Material->set_resource(_material, HTML5_TEXTURE_SLOT_NAME, texture_buffer);
	} else {
		stingray::api::render_buffer->update_buffer(_texture_buffer_handle, texture_buffer_size, buffer);
	}
}

bool WebView::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message)
{
	auto msg_name = message->GetName();
	if (msg_name == "reload") {
		browser->ReloadIgnoreCache();
	} else if (msg_name == "openDevTools")	{
		open_dev_tools();
	}

	return _message_router->OnProcessMessageReceived(browser, source_process, message);
}

void WebView::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
	if (!_browser)
		_browser = browser;
	invalidate();
}

void WebView::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, TransitionType transition_type)
{
}

} // end namespace
