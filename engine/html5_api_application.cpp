#include "html5_api.h"
#include "html5_api_bindings.h"

#include <engine_plugin_api/plugin_api.h>
#include <plugin_foundation/const_config.h>

namespace PLUGIN_NAMESPACE {

CefRefPtr<CefV8Value> config_value_to_json(const ConstConfigItem& config_value)
{
	if (config_value.is_nil())
		return CefV8Value::CreateNull();
	if (config_value.is_bool())
		return CefV8Value::CreateBool(config_value.to_bool());
	if (config_value.is_integer())
		return CefV8Value::CreateInt(config_value.to_integer());
	if (config_value.is_float())
		return CefV8Value::CreateDouble(config_value.to_float());
	if (config_value.is_string())
		return CefV8Value::CreateString(config_value.to_string());
	if (config_value.is_object()) {
		CefRefPtr<CefV8Value> obj = CefV8Value::CreateObject(nullptr, nullptr);
		for (int i = 0, end = config_value.n_items(); i != end; ++i) {
			const char* key_name;
			const auto& item = config_value.item(i, &key_name);
			obj->SetValue(key_name, config_value_to_json(item), V8_PROPERTY_ATTRIBUTE_NONE);
		}
		return obj;
	}
	if (config_value.is_array()) {
		CefRefPtr<CefV8Value> obj = CefV8Value::CreateArray(config_value.size());
		for (int i = 0, end = config_value.size(); i != end; ++i) {
			const auto& item = config_value[i];
			obj->SetValue(i, config_value_to_json(item));
		}
		return obj;
	}

	throw std::exception("Type not supported");
}

void bind_api_application(CefRefPtr<CefV8Value> stingray_ns, const ApplicationCApi* api)
{
	DEFINE_API("Application");

	BIND_API(num_worlds);
	BIND_API(world);
	BIND_API(new_world);
	BIND_API(main_world);
	BIND_API(release_world);
	BIND_API(render_world);

	BIND_API(build);
	BIND_API(platform);
	BIND_API(build_identifier);
	BIND_API(sysinfo);

	BIND_API(create_viewport);
	BIND_API(destroy_viewport);

	BIND_API(time_since_launch);
	BIND_API(sleep);
	//BIND_API(ns, "set_time_step_policy", api->set_time_step_policy);
	BIND_API(get_time_step_policy);
	BIND_API(quit);

	bind_api(ns, "settings", [api](const CefV8ValueList& args)
	{
		auto config_value = ConstConfigItem(*(const ConstConfigRoot*)api->settings_root());
		return config_value_to_json(config_value);
	});

	bind_api(ns, "argv", [api](const CefV8ValueList& args)
	{
		char* args_buffer[2048] = { nullptr };
		MultipleStringsBuffer msb = { 0, args_buffer };
		api->argv(&msb, sizeof args_buffer / sizeof(char*));

		auto result_args = CefV8Value::CreateArray(msb.num_strings);
		for (unsigned i = 0; i < msb.num_strings; ++i)
			result_args->SetValue(i, CefV8Value::CreateString(msb.s[i]));
		return result_args;
	});

	bind_api(ns, "can_get", [](const CefV8ValueList& args)
	{
		return CefV8Value::CreateBool(stingray::api::resource_manager->can_get(
			get_arg<const char*>(args, 0),
			get_arg<const char*>(args, 1)) != 0);
	});
}

} // end namespace
