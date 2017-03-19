#pragma once

#include <include/cef_app.h>
#include <include/cef_browser.h>
#include <include/cef_v8.h>
#include <include/cef_command_line.h>
#include <include/wrapper/cef_message_router.h>

struct ScriptApi;

namespace PLUGIN_NAMESPACE {

class WebApp : public CefApp
	, public CefRenderProcessHandler
	, public CefBrowserProcessHandler
{
public:

	static CefRefPtr<WebApp> init();
	static bool closing();
	static void shutdown();
	static void update();

	WebApp();
	~WebApp();

protected: // CefApp

	CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() OVERRIDE { return this; }
	CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() OVERRIDE { return this; }
	void OnBeforeCommandLineProcessing (const CefString& process_type, CefRefPtr<CefCommandLine> command_line) OVERRIDE;
	void OnRegisterCustomSchemes(CefRefPtr<CefSchemeRegistrar> registrar) OVERRIDE;

protected: // CefBrowserProcessHandler

	void OnContextInitialized() OVERRIDE;
	void OnScheduleMessagePumpWork(int64 delay_ms) OVERRIDE;

protected: // CefRenderProcessHandler

	void OnRenderThreadCreated(CefRefPtr<CefListValue> extra_info) OVERRIDE;
	void OnBrowserCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
	void OnBrowserDestroyed(CefRefPtr<CefBrowser> browser) OVERRIDE;
	void OnContextCreated( CefRefPtr< CefBrowser > browser, CefRefPtr< CefFrame > frame, CefRefPtr< CefV8Context > context ) OVERRIDE;
	void OnContextReleased( CefRefPtr< CefBrowser > browser, CefRefPtr< CefFrame > frame, CefRefPtr< CefV8Context > context ) OVERRIDE;
	bool OnProcessMessageReceived( CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message ) OVERRIDE;
	void OnUncaughtException(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context, CefRefPtr<CefV8Exception> exception, CefRefPtr<CefV8StackTrace> stackTrace) OVERRIDE;

private:

	unsigned _context_created_ref_count;
	CefRefPtr<CefMessageRouterRendererSide> _message_router;

	IMPLEMENT_REFCOUNTING(WebApp)
};

} // end namespace
