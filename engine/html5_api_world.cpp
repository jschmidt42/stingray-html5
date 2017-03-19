#include "html5_api_bindings.h"
#include "html5_api_dynamic_data.inl"

namespace PLUGIN_NAMESPACE {

void bind_api_world(CefRefPtr<CefV8Value> stingray_ns, const WorldCApi* api)
{
	DEFINE_API("World");

	BIND_API(spawn_unit);
	BIND_API(destroy_unit);
	BIND_API(num_units);

	BIND_API(unit_by_name);
	BIND_API(unit_by_id);
	BIND_API(unit_by_index);
	BIND_API(num_units_by_resource);
	BIND_API(units_by_resource);

	BIND_API(link_unit);
	BIND_API(unlink_unit);
	BIND_API(update_unit);
	BIND_API(create_particles);
	BIND_API(destroy_particles);
	BIND_API(stop_spawning_particles);
	BIND_API(are_particles_playing);
	BIND_API(set_particles_collision_filter);

	BIND_API(move_particles);
	BIND_API(link_particles);

	BIND_API(find_particles_variable);
	BIND_API(set_particles_variable);

	BIND_API(load_level);
	BIND_API(destroy_level);
	BIND_API(num_levels);
	BIND_API(level);

	BIND_API(update);
	BIND_API(update_animations);
	BIND_API(update_scene);

	BIND_API(delta_time);
	BIND_API(time);

	BIND_API(storyteller);
	BIND_API(vector_field);
	BIND_API(scatter_system);

	BIND_API(set_flow_enabled);
	BIND_API(set_editor_flow_enabled);

	BIND_API(create_shading_environment);
	BIND_API(create_default_shading_environment);
	BIND_API(destroy_shading_environment);
	BIND_API(set_shading_environment);

	BIND_API(create_screen_gui);
	BIND_API(create_world_gui);
	BIND_API(destroy_gui);
	
	BIND_API(physics_world);

	BIND_API(create_video_player);
	BIND_API(destroy_video_player);

	BIND_API(debug_camera_pose);

	#if defined(DEVELOPMENT)
		BIND_API(set_flow_enabled);
		BIND_API(set_editor_flow_enabled);

		BIND_API(num_particles);
		BIND_API(advance_particles_time);

		BIND_API(set_frustum_inspector_camera);

		BIND_API(replay);
		BIND_API(start_playback);
		BIND_API(stop_playback);
		BIND_API(is_playing_back);
		BIND_API(num_frames);
		BIND_API(frame);
		BIND_API(set_frame);
		BIND_API(record_debug_line);
		BIND_API(record_screen_debug_text);
		BIND_API(record_world_debug_text);
		BIND_API(set_unit_record_mode);
	#endif

	bind_dynamic_data_api<WorldPtr>(ns, stingray::api::script->DynamicScriptData->World);
}

} // end namespace
