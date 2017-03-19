#include "html5_api_bindings.h"

namespace PLUGIN_NAMESPACE {

void bind_api_host(CefRefPtr<CefV8Value> stingray_ns)
{
	DEFINE_API("host");
}

} // end namespace
