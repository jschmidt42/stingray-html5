#include "html5_api_bindings.h"

#include <engine_plugin_api/c_api/c_api_window.h>

namespace PLUGIN_NAMESPACE {

void bind_api_window(CefRefPtr<CefV8Value> stingray_ns, const WindowCApi* api)
{
	DEFINE_API("Window");

	BIND_API(has_mouse_focus);
	BIND_API(has_focus);
	BIND_API(set_mouse_focus);
	BIND_API(set_focus);

	BIND_API(show_cursor);
	BIND_API(clip_cursor);
	BIND_API(set_cursor);
	BIND_API(set_show_cursor);
	BIND_API(set_clip_cursor);

	BIND_API(is_resizable);
	BIND_API(set_resizable);
	BIND_API(set_resolution);
	BIND_API(get_dpi_scale);
	
	BIND_API(set_title);
	BIND_API(has_window);
	BIND_API(get_main_window);

	BIND_API(minimize);
	BIND_API(maximize);
	BIND_API(restore);
	BIND_API(is_closing);
	BIND_API(close);
	BIND_API(get_main_window);
	BIND_API(trigger_resize);
	BIND_API(set_ime_enabled);
	BIND_API(set_foreground);
	BIND_API(set_keystroke_enabled);
	BIND_API(id);
	BIND_API(rect);
	BIND_API(set_rect);
	BIND_API(open);

	bind_api(ns, "fill_default_open_parameter", [api](const CefV8ValueList& args)
	{
		CefRefPtr<CefV8Value> retval;
		WindowOpenParameter open_params;
		api->fill_default_open_parameter(&open_params);
		wrap_result(open_params, retval);
		return retval;
	});
}

} // end namespace
