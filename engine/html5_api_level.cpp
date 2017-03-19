#include "html5_api_bindings.h"
#include "html5_api_dynamic_data.inl"

namespace PLUGIN_NAMESPACE {

void bind_api_level(CefRefPtr<CefV8Value> stingray_ns, const LevelCApi* api)
{
	DEFINE_API("Level");

	BIND_API(world);
	BIND_API(spawn_background);
	
	BIND_API(unit_by_index);
	BIND_API(unit_index);
	BIND_API(num_units);

	BIND_API(num_nested_levels);
	BIND_API(nested_level);

	BIND_API(num_entities);
	BIND_API(entity);

	BIND_API(random_point_inside_volume);
	BIND_API(next_random_point_inside_volume);
	BIND_API(is_point_inside_volume);
	BIND_API(has_volume);

	BIND_API(flow_event);
	BIND_API(flow_variable_type);
	BIND_API(flow_variable);
	BIND_API(set_flow_variable);

	BIND_API(trigger_event);
	BIND_API(trigger_level_loaded);
	BIND_API(trigger_level_shutdown);
	BIND_API(trigger_level_update);

	BIND_API(pose);

	BIND_API(navigation_mesh);

	BIND_API(spline);
	BIND_API(num_splines);
	BIND_API(spline_by_index);

	#if defined(DEVELOPMENT)
		BIND_API(set_pose);
		BIND_API(set_visibility);
	
		BIND_API(box);

		BIND_API(num_internal_units);
		BIND_API(internal_units);
	#endif

	bind_dynamic_data_api<LevelPtr>(ns, stingray::api::script->DynamicScriptData->Level);
}

} // end namespace
