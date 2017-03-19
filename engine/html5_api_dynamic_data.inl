#pragma once

#include "html5_api_bindings.h"

#include <include/cef_app.h>

namespace PLUGIN_NAMESPACE {

// Variadic helpers

union DynamicVariant
{
	void* p;
	int b;
	int i;
	unsigned u;
	float n;
	char s[1024];
};

inline Array<DynamicVariant> get_variadic_args(const CefV8ValueList& args, unsigned start_arg)
{
	Array<DynamicVariant> ret(allocator);
	for (unsigned i = start_arg; i < args.size(); ++i) {
		DynamicVariant av;
		if (args[i]->IsBool())
			av.b = (int)args[i]->GetBoolValue();
		else if (args[i]->IsDouble())
			av.n = (float)args[i]->GetDoubleValue();
		else if (args[i]->IsInt())
			av.i = args[i]->GetIntValue();
		else if (args[i]->IsUInt())
			av.u = args[i]->GetUIntValue();
		else if (args[i]->IsString()) {
			strncpy(av.s, args[i]->GetStringValue().ToString().c_str(), sizeof av.s);
		} else av.p = nullptr;

		ret.push_back(av);
	}

	return ret;
}

#define INVOKE_FUNC_VARIADIC(result, vargs, FUNC, ...) \
	if (vargs.size() == 0) { \
		result = FUNC( ##__VA_ARGS__ ); \
	} else if (vargs.size() == 1) { \
		result = FUNC( ##__VA_ARGS__ , vargs[0]); \
	} else if (vargs.size() == 2) { \
		result = FUNC( ##__VA_ARGS__ , vargs[0], vargs[1]); \
	} else if (vargs.size() == 3) { \
		result = FUNC( ##__VA_ARGS__ , vargs[0], vargs[1], vargs[2]); \
	} else if (vargs.size() == 4) { \
		result = FUNC( ##__VA_ARGS__ , vargs[0], vargs[1], vargs[2], vargs[3]); \
	} else if (vargs.size() == 5) { \
		result = FUNC( ##__VA_ARGS__ , vargs[0], vargs[1], vargs[2], vargs[3], vargs[4]); \
	} else if (vargs.size() == 6) { \
		result = FUNC( ##__VA_ARGS__ , vargs[0], vargs[1], vargs[2], vargs[3], vargs[4], vargs[5]); \
	} else if (vargs.size() == 7) { \
		result = FUNC( ##__VA_ARGS__ , vargs[0], vargs[1], vargs[2], vargs[3], vargs[4], vargs[5], vargs[6]); \
	} else if (vargs.size() == 8) { \
		result = FUNC( ##__VA_ARGS__ , vargs[0], vargs[1], vargs[2], vargs[3], vargs[4], vargs[5], vargs[6], vargs[7]); \
	} else if (vargs.size() == 9) { \
		result = FUNC( ##__VA_ARGS__ , vargs[0], vargs[1], vargs[2], vargs[3], vargs[4], vargs[5], vargs[6], vargs[7], vargs[8]); \
	} else if (vargs.size() == 10) { \
		result = FUNC( ##__VA_ARGS__ , vargs[0], vargs[1], vargs[2], vargs[3], vargs[4], vargs[5], vargs[6], vargs[7], vargs[8], vargs[9]); \
	} else \
		throw std::exception("Variadic call not supported");

template<typename T, typename API>
void bind_dynamic_data_api(CefRefPtr<CefV8Value> ns, API api)
{
	bind_api(ns, "has_data", [api](const CefV8ValueList& args)
	{
		int result = 0;
		auto vargs = get_variadic_args(args, 1);
		INVOKE_FUNC_VARIADIC(result, vargs, api->has_data, get_arg<T>(args, 0), vargs.size());
		return CefV8Value::CreateBool(result != 0);
	});

	bind_api(ns, "get_data", [api](const CefV8ValueList& args)
	{
		DynamicScriptDataItem dsdi;
		auto vargs = get_variadic_args(args, 1);
		INVOKE_FUNC_VARIADIC(dsdi, vargs, api->get_data, get_arg<T>(args, 0), vargs.size());
		CefRefPtr<CefV8Value> retvalue;
		return wrap_result(dsdi, retvalue);
	});

	bind_api(ns, "set_data", [api](const CefV8ValueList& args)
	{
		DynamicScriptDataItem dsdi = get_arg<DynamicScriptDataItem>(args, 1);
		Array<DynamicVariant> vargs = get_variadic_args(args, 2);
		if (vargs.size() == 0) {
			api->set_data(get_arg<T>(args, 0), &dsdi, vargs.size() );
		} else if (vargs.size() == 1) {
			api->set_data(get_arg<T>(args, 0), &dsdi, vargs.size() , vargs[0]);
		} else if (vargs.size() == 2) {
			api->set_data(get_arg<T>(args, 0), &dsdi, vargs.size() , vargs[0], vargs[1]);
		} else if (vargs.size() == 3) {
			api->set_data(get_arg<T>(args, 0), &dsdi, vargs.size() , vargs[0], vargs[1], vargs[2]);
		} else if (vargs.size() == 4) {
			api->set_data(get_arg<T>(args, 0), &dsdi, vargs.size() , vargs[0], vargs[1], vargs[2], vargs[3]);
		} else if (vargs.size() == 5) {
			api->set_data(get_arg<T>(args, 0), &dsdi, vargs.size() , vargs[0], vargs[1], vargs[2], vargs[3], vargs[4]);
		} else if (vargs.size() == 6) {
			api->set_data(get_arg<T>(args, 0), &dsdi, vargs.size() , vargs[0], vargs[1], vargs[2], vargs[3], vargs[4], vargs[5]);
		} else if (vargs.size() == 7) {
			api->set_data(get_arg<T>(args, 0), &dsdi, vargs.size() , vargs[0], vargs[1], vargs[2], vargs[3], vargs[4], vargs[5], vargs[6]);
		} else if (vargs.size() == 8) {
			api->set_data(get_arg<T>(args, 0), &dsdi, vargs.size() , vargs[0], vargs[1], vargs[2], vargs[3], vargs[4], vargs[5], vargs[6], vargs[7]);
		} else if (vargs.size() == 9) {
			api->set_data(get_arg<T>(args, 0), &dsdi, vargs.size() , vargs[0], vargs[1], vargs[2], vargs[3], vargs[4], vargs[5], vargs[6], vargs[7], vargs[8]);
		} else if (vargs.size() == 10) {
			api->set_data(get_arg<T>(args, 0), &dsdi, vargs.size() , vargs[0], vargs[1], vargs[2], vargs[3], vargs[4], vargs[5], vargs[6], vargs[7], vargs[8], vargs[9]);
		} else
			throw std::exception("Variadic call not supported");;
		return CefV8Value::CreateUndefined();
	});
}

} // end namespace
