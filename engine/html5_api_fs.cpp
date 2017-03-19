#include "html5_api_bindings.h"

namespace PLUGIN_NAMESPACE {

void bind_api_fs(CefRefPtr<CefV8Value> stingray_ns)
{
	DEFINE_API("fs");

	// stingray.fs.exists(path)
	bind_api(ns, "exists", [](const CefV8ValueList& args)
	{
		if (args.size() != 1) throw std::exception("function takes 1 argument");
		auto path = get_arg<const char*>(args, 0);
		if (FILE *file = fopen(path, "r")) {
			fclose(file);
			return CefV8Value::CreateBool(true);
		}
		return CefV8Value::CreateBool(false);
	});

	// stingray.fs.enumerate(path, pattern, recursive, filters)
	bind_api(ns, "enumerate", [](const CefV8ValueList& args)
	{
		throw std::exception("TODO");
		return CefV8Value::CreateUndefined();
	});

	bind_api(ns, "lock", [](const CefV8ValueList& args)
	{
		throw std::exception("TODO");
		return CefV8Value::CreateUndefined();
	});

	bind_api(ns, "unlock", [](const CefV8ValueList& args)
	{
		throw std::exception("TODO");
		return CefV8Value::CreateUndefined();
	});

	bind_api(ns, "mkdir", [](const CefV8ValueList& args)
	{
		throw std::exception("TODO");
		return CefV8Value::CreateUndefined();
	});

	bind_api(ns, "unlink", [](const CefV8ValueList& args)
	{
		throw std::exception("TODO");
		return CefV8Value::CreateUndefined();
	});

	bind_api(ns, "stats", [](const CefV8ValueList& args)
	{
		throw std::exception("TODO");
		return CefV8Value::CreateUndefined();
	});

	bind_api(ns, "read", [](const CefV8ValueList& args)
	{
		throw std::exception("TODO");
		return CefV8Value::CreateUndefined();
	});

	bind_api(ns, "write", [](const CefV8ValueList& args)
	{
		throw std::exception("TODO");
		return CefV8Value::CreateUndefined();
	});

	bind_api(ns, "watch", [](const CefV8ValueList& args)
	{
		throw std::exception("TODO");
		return CefV8Value::CreateUndefined();
	});

	// stingray.fs.copy(source, destination, [overwrite])
	bind_api(ns, "copy", [](const CefV8ValueList& args)
	{
		throw std::exception("TODO");
		return CefV8Value::CreateUndefined();
	});

	bind_api(ns, "move", [](const CefV8ValueList& args)
	{
		throw std::exception("TODO");
		return CefV8Value::CreateUndefined();
	});
}

} // end namespace
