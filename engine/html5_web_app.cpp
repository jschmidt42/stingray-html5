#include "html5_web_app.h"
#include "html5_web_page.h"
#include "html5_api.h"

#include "stingray_api.h"

#include <plugin_foundation/exception_handling.h>

#include <include/internal/cef_types.h>

#include <tlhelp32.h>

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

namespace PLUGIN_NAMESPACE {

volatile int _closing = 0;

std::string remove_file_name(const std::string& path)
{
	auto result = path.substr(0, path.find_last_of("\\/"));
	result = result.substr(0, path.find_last_of("\\/"));
	return result;
}

class MessageAPIHandler : public CefV8Handler
{
public:
	MessageAPIHandler(CefRefPtr< CefBrowser > browser) : _browser(browser) {}

	 bool Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments,
					CefRefPtr<CefV8Value>&, CefString&) OVERRIDE
	{
		auto msg = CefProcessMessage::Create(name);
		return _browser->SendProcessMessage(PID_BROWSER, msg);
	}

private:

	CefRefPtr<CefBrowser> _browser;
	IMPLEMENT_REFCOUNTING(MessageAPIHandler);
};

CefRefPtr<WebApp> WebApp::init()
{
	// ReSharper disable once CppLocalVariableWithNonTrivialDtorIsNeverUsed
	FpuUnsafeScope fus;

	auto command_line = CefCommandLine::CreateCommandLine();
	command_line->InitFromString(GetCommandLineW());

	char buf[MAX_PATH] = { '\0' };
	GetModuleFileNameA((HINSTANCE)&__ImageBase, buf, MAX_PATH);
	std::string app_path = buf;
	std::string app_dir = remove_file_name(app_path);
	std::string resources_dir = app_dir + "\\resources";
	std::string locale_path = resources_dir + "\\locales";

	CefSettings settings;
	CefString(&settings.resources_dir_path).FromASCII(resources_dir.c_str());
	CefString(&settings.locales_dir_path).FromASCII(locale_path.c_str());
	CefString(&settings.cache_path) = command_line->GetSwitchValue("cache-path");
	settings.background_color = CefColorSetARGB(0, 0xFF, 0x0, 0x0);
	#ifdef NDEBUG
		settings.log_severity = LOGSEVERITY_DISABLE;
	#endif
	settings.multi_threaded_message_loop = false;
	settings.windowless_rendering_enabled = true;
	settings.single_process = true;
	settings.no_sandbox = true;
	#if defined(DEVELOPMENT)
		settings.remote_debugging_port = 9089;
	#endif

	CefRefPtr<WebApp> cef_app = new WebApp();
	CefMainArgs main_args;
	if (!CefInitialize(main_args, settings, cef_app.get(), nullptr))
		return nullptr;

	CefRegisterSchemeHandlerFactory("stingray", "", new WebPageSchemeHandlerFactory());

	return cef_app;
}

bool WebApp::closing()
{
	return _closing != 0;
}

void WebApp::shutdown()
{
	_closing = 1;

	// ReSharper disable once CppLocalVariableWithNonTrivialDtorIsNeverUsed
	FpuUnsafeScope fus;

	CefClearSchemeHandlerFactories();
	CefShutdown();
}

void WebApp::update()
{
	// ReSharper disable once CppLocalVariableWithNonTrivialDtorIsNeverUsed
	FpuUnsafeScope fus;

	CefDoMessageLoopWork();
}

WebApp::WebApp(): _context_created_ref_count(0)
{
	CefMessageRouterConfig config;
	config.js_query_function = "cefQuery";
	config.js_cancel_function = "cefCancel";
	_message_router = CefMessageRouterRendererSide::Create(config);
}

WebApp::~WebApp()
{
}

void WebApp::OnRenderThreadCreated(CefRefPtr<CefListValue> extra_info)
{
}

void WebApp::OnBrowserCreated(CefRefPtr<CefBrowser> browser)
{
}

void WebApp::OnBrowserDestroyed(CefRefPtr<CefBrowser> browser)
{
}

void WebApp::OnContextCreated(CefRefPtr< CefBrowser > browser, CefRefPtr< CefFrame > frame, CefRefPtr< CefV8Context > context)
{
	CefRefPtr<CefV8Value> object = context->GetGlobal();
	CefRefPtr<CefV8Handler> handler = new MessageAPIHandler(browser);

	CefRefPtr<CefV8Value> stingray_ns = CefV8Value::CreateObject(nullptr, nullptr);
	object->SetValue("stingray", stingray_ns, V8_PROPERTY_ATTRIBUTE_NONE);

	// Create global functions.
	stingray_ns->SetValue("reload", CefV8Value::CreateFunction("reload", handler), V8_PROPERTY_ATTRIBUTE_READONLY);
	stingray_ns->SetValue("openDevTools", CefV8Value::CreateFunction("openDevTools", handler), V8_PROPERTY_ATTRIBUTE_READONLY);

	bind_api(stingray_ns);

	_context_created_ref_count++;
	if (!stingray::api::error_context->has_thread_error_context_stack()) {
		static unsigned wti = UINT32_MAX;
		if (wti == UINT32_MAX) {
			stingray::api::thread->assign_worker_thread_index();
			wti = stingray::api::thread->worker_thread_index();
		}

		stingray::api::error_context->make_thread_error_context_stack(stingray::api::allocator_object);
		stingray::api::profiler->make_thread_profiler(stingray::api::allocator_object);
	}

	_message_router->OnContextCreated(browser, frame, context);
}

void WebApp::OnContextReleased(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context)
{
	_context_created_ref_count--;
	if (stingray::api::error_context->has_thread_error_context_stack() && _context_created_ref_count == 0) {
		stingray::api::profiler->delete_thread_profiler(stingray::api::allocator_object);
		stingray::api::error_context->delete_thread_error_context_stack(stingray::api::allocator_object);
	}

	_message_router->OnContextReleased(browser, frame, context);
}

bool WebApp::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message)
{
	return _message_router->OnProcessMessageReceived(browser, source_process, message);
}

void WebApp::OnUncaughtException(CefRefPtr<CefBrowser>, CefRefPtr<CefFrame>, CefRefPtr<CefV8Context>, CefRefPtr<CefV8Exception> exception, CefRefPtr<CefV8StackTrace> stack_trace)
{
	stingray::api::log->error("HTML5", stingray::api::error->eprintf("%s", exception->GetMessageA().ToString().c_str()));
	for (int i = 0; i < stack_trace->GetFrameCount(); ++i) {
		auto stack_frame = stack_trace->GetFrame(i);
		stingray::api::log->error("HTML5", stingray::api::error->eprintf("%s at `%s`%d:",
			stack_frame->GetFunctionName().ToString().c_str(),
			stack_frame->GetScriptNameOrSourceURL().ToString().c_str(),
			stack_frame->GetLineNumber()));
	}
}

void WebApp::OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line)
{
	command_line->AppendSwitch("disable-spell-checking");
	command_line->AppendSwitch("no-proxy-server");
}

void WebApp::OnRegisterCustomSchemes(CefRefPtr<CefSchemeRegistrar> registrar)
{
	// Register "client" as a standard scheme.
	registrar->AddCustomScheme("stingray", true, false, false);
}

void WebApp::OnContextInitialized()
{
}

void WebApp::OnScheduleMessagePumpWork(int64 delay_ms)
{
	
}

} // end namespace
