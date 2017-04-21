#include "html5_web_browser.h"
#include "html5_web_view.h"
#include "html5_api_bindings.h"

#include <include/cef_app.h>

namespace PLUGIN_NAMESPACE {

volatile int sync_point = 0;
void set_signal() { sync_point = 1; }
void sync_signal() { sync_point = 0; }
void wait_for_sync() { while (sync_point == 1) CefDoMessageLoopWork(); }

void bind_api_web_app(CefRefPtr<CefV8Value> stingray_ns)
{
	DEFINE_API("WebApp");

	bind_api(ns, "sync", [](const CefV8ValueList&)
	{
		sync_signal();
		return CefV8Value::CreateUndefined();
	});
}

void bind_api_web_view(CefRefPtr<CefV8Value> stingray_ns)
{
	DEFINE_API("WebView");
	bind_api(ns, "load_browser", [](const CefV8ValueList& args)
	{
		UnitRef unit_ref = get_arg<UnitRef>(args, 0);
		CApiUnit* unit = stingray::api::unit_reference->dereference(unit_ref);
		return CefV8Value::CreateBool(browser::try_load(unit));
	});

	bind_api(ns, "pick", [](const CefV8ValueList& args)
	{
		const Vector3& mouse_pos = get_arg<Vector3>(args, 0);
		const Vector3& from = get_arg<Vector3>(args, 1);
		const Vector3& ray = get_arg<Vector3>(args, 2);

		browser::pick(mouse_pos, from, ray);
		return CefV8Value::CreateBool(true);
	});

	bind_api(ns, "create", [](const CefV8ValueList& args)
    {
		CefRefPtr<CefV8Value> retval = CefV8Value::CreateObject(nullptr, nullptr);
		MaterialPtr material = get_arg<MaterialPtr>(args,2);
		WindowPtr   window   = get_arg<WindowPtr>(args,1);
		const char *url      = get_arg<const char *>(args,0);
		if ( url && material ) {
			if (!window) {
				window = stingray::api::script->Window->get_main_window();
			}
			CefRefPtr<WebView> view = new WebView(window, material);
			view->load_page(url);
			retval->SetUserData( view );
		}
		return retval;
    });
	bind_api(ns, "destroy", [](const CefV8ValueList& args)
    {
		if ( args.size() != 1 || !args[0]->IsValid() ||
			 !args[0]->IsObject() || !args[0]->IsUserCreated() ) {
			throw std::exception("Argument must be a WebView");
		}
		CefRefPtr<CefBase> base = args[0]->GetUserData();
		if ( base ) {
			CefRefPtr<WebView> view = dynamic_cast<WebView *>(base.get());

			view->close_browser();
			args[0]->SetUserData(NULL);
			// Free it
			view = NULL;
		}

		return CefV8Value::CreateUndefined();
	});
}

} // end namespace
