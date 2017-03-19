#include "html5_api_bindings.h"

#include <engine_plugin_api/plugin_api.h>
#include <engine_plugin_api/c_api/c_api_input_controller.h>
#include <plugin_foundation/quaternion.h>
#include <plugin_foundation/const_config.h>

namespace PLUGIN_NAMESPACE {

// Input binding helpers

template<typename F>
void bind_input_controller_api(CefRefPtr<CefV8Value> ns, CApiInputControllerPtr input, const char* name, F func)
{
	bind_api(ns, name, [func, input](const CefV8ValueList& args)
	{
		CefRefPtr<CefV8Value> retval;
		wrap_result(func(input), retval);
		return retval;
	});
}

template<typename P0, typename F>
void bind_input_controller_api(CefRefPtr<CefV8Value> ns, CApiInputControllerPtr input, const char* name, F func)
{
	bind_api(ns, name, [func, input](const CefV8ValueList& args)
	{
		CefRefPtr<CefV8Value> retval;
		wrap_result(func(input, get_arg<P0>(args, 0)), retval);
		return retval;
	});
}

template<typename P0, typename P1, typename F>
void bind_input_controller_api(CefRefPtr<CefV8Value> ns, CApiInputControllerPtr input, const char* name, F func)
{
	bind_api(ns, name, [func, input](const CefV8ValueList& args)
	{
		CefRefPtr<CefV8Value> retval;
		wrap_result(func(input, get_arg<P0>(args, 0), get_arg<P1>(args, 1)), retval);
		return retval;
	});
}

template<typename F>
void bind_input_controller_api_void(CefRefPtr<CefV8Value> ns, CApiInputControllerPtr input, const char* name, F func)
{
	bind_api(ns, name, [func, input](const CefV8ValueList& args)
	{
		func(input);
		return CefV8Value::CreateUndefined();
	});
}

template<typename P0, typename F>
void bind_input_controller_api_void(CefRefPtr<CefV8Value> ns, CApiInputControllerPtr input, const char* name, F func)
{
	bind_api(ns, name, [func, input](const CefV8ValueList& args)
	{
		func(input, get_arg<P0>(args, 0));
		return CefV8Value::CreateUndefined();
	});
}

template<typename P0, typename P1, typename F>
void bind_input_controller_api_void(CefRefPtr<CefV8Value> ns, CApiInputControllerPtr input, const char* name, F func)
{
	bind_api(ns, name, [func, input](const CefV8ValueList& args)
	{
		func(input, get_arg<P0>(args, 0), get_arg<P1>(args, 1));
		return CefV8Value::CreateUndefined();
	});
}

template<typename F>
void bind_api_input_controller_array(CefRefPtr<CefV8Value> stingray_ns, const char* name, F func)
{
	struct InputInterceptor : CefV8Interceptor
	{
		F input_controller_api_indexer;
		explicit InputInterceptor(F f) : input_controller_api_indexer(f) {}

		bool Get(const CefString&, const CefRefPtr<CefV8Value>, CefRefPtr<CefV8Value>&, CefString&) override { return false; }
		bool Set(const CefString&, const CefRefPtr<CefV8Value>, const CefRefPtr<CefV8Value>, CefString&) override { return false; }
		bool Set(int, const CefRefPtr<CefV8Value>, const CefRefPtr<CefV8Value>, CefString&) override { return false; }

		bool Get(int index, const CefRefPtr<CefV8Value>, CefRefPtr<CefV8Value>& retval, CefString&) override
		{
			retval = CefV8Value::CreateObject(nullptr, nullptr);
			bind_api_input_controller(retval, nullptr, input_controller_api_indexer(index));
			return true;
		}

		IMPLEMENT_REFCOUNTING(InputInterceptor)
	};

	CefRefPtr<CefV8Value> ns = CefV8Value::CreateObject(nullptr, new InputInterceptor(func));
	stingray_ns->SetValue(name, ns, V8_PROPERTY_ATTRIBUTE_READONLY);
}

void bind_api_input_controller(CefRefPtr<CefV8Value> stingray_ns, const char* name, CApiInputControllerPtr input)
{
	CefRefPtr<CefV8Value> ns = name == nullptr ? stingray_ns : CefV8Value::CreateObject(nullptr, nullptr);
	
	bind_input_controller_api(ns, input, "category", stingray::api::script->Input->InputController->category);
	bind_input_controller_api(ns, input, "type", stingray::api::script->Input->InputController->type);
	bind_input_controller_api(ns, input, "name", stingray::api::script->Input->InputController->name);
	
	bind_input_controller_api(ns, input, "num_buttons", stingray::api::script->Input->InputController->num_buttons);
	bind_input_controller_api<unsigned>(ns, input, "button", stingray::api::script->Input->InputController->button);
	bind_input_controller_api<unsigned>(ns, input, "pressed", stingray::api::script->Input->InputController->pressed);
	bind_input_controller_api<unsigned>(ns, input, "released", stingray::api::script->Input->InputController->released);
	bind_input_controller_api(ns, input, "any_pressed", stingray::api::script->Input->InputController->any_pressed);
	bind_input_controller_api(ns, input, "any_released", stingray::api::script->Input->InputController->any_released);
	bind_input_controller_api_void<float>(ns, input, "set_down_threshold", stingray::api::script->Input->InputController->set_down_threshold);
	bind_input_controller_api(ns, input, "down_threshold", stingray::api::script->Input->InputController->down_threshold);
	bind_input_controller_api(ns, input, "num_axes", stingray::api::script->Input->InputController->num_axes);
	
	bind_input_controller_api<unsigned, DeadZoneSetting*>(ns, input, "axis", stingray::api::script->Input->InputController->axis);
	bind_api(ns, "dead_zone", [input](const CefV8ValueList& args)
	{
		DeadZoneSetting out_dzs;
		stingray::api::script->Input->InputController->dead_zone(input, get_arg<float>(args, 0), &out_dzs);
		CefRefPtr<CefV8Value> retval;
		wrap_result(out_dzs, retval);
		return retval;
	});
	bind_input_controller_api_void<float, DeadZoneSetting*>(ns, input, "set_dead_zone", stingray::api::script->Input->InputController->set_dead_zone);

	bind_input_controller_api_void<unsigned>(ns, input, "set_rumble_enabled", stingray::api::script->Input->InputController->set_rumble_enabled);
	bind_input_controller_api(ns, input, "num_rumble_motors", stingray::api::script->Input->InputController->num_rumble_motors);
	bind_input_controller_api_void<unsigned, float>(ns, input, "set_rumble", stingray::api::script->Input->InputController->set_rumble);
	bind_input_controller_api<unsigned, RumbleParameters*>(ns, input, "rumble_effect", stingray::api::script->Input->InputController->rumble_effect);
	bind_input_controller_api_void<unsigned, unsigned>(ns, input, "stop_rumble_effect", stingray::api::script->Input->InputController->stop_rumble_effect);
	bind_input_controller_api<unsigned, unsigned>(ns, input, "is_rumble_effect_playing", stingray::api::script->Input->InputController->is_rumble_effect_playing);
	bind_input_controller_api_void<unsigned>(ns, input, "stop_all_rumble_effects", stingray::api::script->Input->InputController->stop_all_rumble_effects);

	bind_input_controller_api<unsigned>(ns, input, "button_name", stingray::api::script->Input->InputController->button_name);
	bind_input_controller_api<unsigned>(ns, input, "button_locale_name", stingray::api::script->Input->InputController->button_locale_name);
	bind_input_controller_api<unsigned>(ns, input, "button_id", stingray::api::script->Input->InputController->button_id);
	bind_input_controller_api<unsigned>(ns, input, "axis_name", stingray::api::script->Input->InputController->axis_name);
	bind_input_controller_api<unsigned>(ns, input, "axis_id", stingray::api::script->Input->InputController->axis_id);

	bind_input_controller_api<unsigned>(ns, input, "rumble_motor_name", stingray::api::script->Input->InputController->rumble_motor_name);
	bind_input_controller_api<unsigned>(ns, input, "rumble_motor_id", stingray::api::script->Input->InputController->rumble_motor_id);

	bind_input_controller_api(ns, input, "active", stingray::api::script->Input->InputController->active);
	bind_input_controller_api(ns, input, "connected", stingray::api::script->Input->InputController->connected);
	bind_input_controller_api(ns, input, "disconnected", stingray::api::script->Input->InputController->disconnected);

	if (name != nullptr)
		stingray_ns->SetValue(name, ns, V8_PROPERTY_ATTRIBUTE_READONLY);
}

void bind_api_input(CefRefPtr<CefV8Value> stingray_ns, const InputCApi* api)
{
	CefRefPtr<CefV8Value> ns = CefV8Value::CreateObject(nullptr, nullptr);

	BIND_API(num_pads);
	BIND_API(num_touch_panels);
	BIND_API(num_windows_ps4_pads);
	BIND_API(flush_controllers_state);
	BIND_API(synergy_clipboard);
	BIND_API(scan_for_windows_ps4_pads);
	BIND_API(set_tablet_pen_service_properties);

	bind_api_input_controller(stingray_ns, "Keyboard", stingray::api::script->Input->keyboard());
	bind_api_input_controller(stingray_ns, "Mouse", stingray::api::script->Input->mouse());
	bind_api_input_controller(stingray_ns, "Tablet", stingray::api::script->Input->tablet());
	bind_api_input_controller(stingray_ns, "SimulatedTouchPanel", stingray::api::script->Input->simulated_touch_panel());
	bind_api_input_controller(stingray_ns, "SynergyMouse", stingray::api::script->Input->synergy_mouse());
	bind_api_input_controller(stingray_ns, "SynergyKeyboard", stingray::api::script->Input->synergy_keyboard());
	bind_api_input_controller(stingray_ns, "TouchPanel", stingray::api::script->Input->touch_panel(0));
	bind_api_input_controller(stingray_ns, "Pad", stingray::api::script->Input->pad(0));
	bind_api_input_controller(stingray_ns, "Ps4Pad", stingray::api::script->Input->windows_ps4_pad(0));
	bind_api_input_controller_array(stingray_ns, "TouchPanels", stingray::api::script->Input->touch_panel);
	bind_api_input_controller_array(stingray_ns, "Pads", stingray::api::script->Input->pad);
	bind_api_input_controller_array(stingray_ns, "Ps4Pads", stingray::api::script->Input->windows_ps4_pad);

	stingray_ns->SetValue("Input", ns, V8_PROPERTY_ATTRIBUTE_READONLY);
}

} // end namespace
