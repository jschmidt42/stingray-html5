#include "html5_api_bindings.h"
#include "html5_api_dynamic_data.inl"

namespace PLUGIN_NAMESPACE {

void bind_api_unit(CefRefPtr<CefV8Value> stingray_ns, const UnitCApi* api)
{
	DEFINE_API("Unit");

	BIND_API(local_position);
	BIND_API(local_rotation);
	BIND_API(local_scale);
	BIND_API(local_pose);

	BIND_API(set_local_position);
	BIND_API(set_local_rotation);
	BIND_API(set_local_scale);
	BIND_API(set_local_pose);

	BIND_API(world_position);
	BIND_API(world_pose);
	BIND_API(world_rotation);

	BIND_API(teleport_local_position);
	BIND_API(teleport_local_rotation);
	BIND_API(teleport_local_scale);
	BIND_API(teleport_local_pose);

	BIND_API(delta_position);
	BIND_API(delta_rotation);
	BIND_API(delta_pose);

	BIND_API(create_actor);
	BIND_API(destroy_actor);
	BIND_API(num_actors);
	BIND_API(find_actor);
	BIND_API(actor);

	BIND_API(num_movers);
	BIND_API(find_mover);
	BIND_API(set_mover);
	BIND_API(set_mover_to_none);
	BIND_API(mover);
	BIND_API(mover_fits_at);

	BIND_API(trigger_flow_event);
	BIND_API(flow_variable);
	BIND_API(set_flow_variable);
	BIND_API(trigger_unit_spawned);

	BIND_API(set_material);
	BIND_API(set_material_to_none);
	BIND_API(query_material);
	BIND_API(save_instance_material_data);
	BIND_API(restore_instance_material_data);
	BIND_API(is_using_material_set);

	BIND_API(num_meshes);
	BIND_API(find_mesh);
	BIND_API(mesh);

	BIND_API(bones);
	BIND_API(animation_wanted_root_pose);
	BIND_API(animation_set_bones_lod);

	BIND_API(animation_root_mode);
	BIND_API(animation_bone_mode);

	BIND_API(set_animation_root_mode);
	BIND_API(set_animation_bone_mode);

	BIND_API(animation_find_constraint_target);
	BIND_API(animation_has_constraint_target);
	BIND_API(animation_get_constraint_target);
	BIND_API(animation_set_constraint_target_pose);
	BIND_API(animation_set_constraint_target_position);
	BIND_API(animation_set_constraint_target_rotation);

	BIND_API(crossfade_animation);
	BIND_API(is_crossfading_animation);
	BIND_API(crossfade_animation_set_time);
	BIND_API(crossfade_animation_set_speed);

	BIND_API(disable_animation_state_machine);
	BIND_API(enable_animation_state_machine);
	BIND_API(set_animation_state_machine);
	BIND_API(has_animation_state_machine);
	BIND_API(has_animation_event);

	BIND_API(animation_trigger_event);
	BIND_API(animation_trigger_event_with_parameters);

	BIND_API(animation_find_variable);
	BIND_API(animation_has_variable);
	BIND_API(animation_get_variable);
	BIND_API(animation_set_variable);

	BIND_API(animation_set_state);
	BIND_API(animation_get_state);

	BIND_API(animation_set_seeds);
	BIND_API(animation_get_seeds);

	BIND_API(animation_layer_info);
	BIND_API(set_animation_merge_options);

	BIND_API(animation_set_moving);
	BIND_API(set_animation_logging);
	bind_api(ns, "animation_get_curve_value", [api](const CefV8ValueList& args)
	{
		float out_curve_value;
		int result = api->animation_get_curve_value(
			get_arg<UnitRef>(args, 0),
			get_arg<unsigned>(args, 1),
			get_arg<unsigned>(args, 2),
			get_arg<unsigned int>(args, 3),
			&out_curve_value);
		auto retval = CefV8Value::CreateArray(2);
		retval->SetValue(0, CefV8Value::CreateInt(result));
		if (args.size() >= 5) {
			retval->SetValue(1, CefV8Value::CreateDouble(out_curve_value));
		}
		return retval;
	});

	BIND_API(play_simple_animation);
	BIND_API(stop_simple_animation);

	BIND_API(num_terrains);
	BIND_API(find_terrain);
	BIND_API(terrain);
	BIND_API(terrain_update_height_field);

	BIND_API(create_joint);
	BIND_API(destroy_joint);
	BIND_API(create_custom_joint);

	bind_api(ns, "set_property", [api](const CefV8ValueList& args)
	{
		Array<unsigned> key_id32_array(allocator);
		for (unsigned i = 2; i < args.size(); ++i) {
			if (args[i]->IsString()) {
				key_id32_array.push_back(IdString32(args[i]->GetStringValue().ToString().c_str()).id());
			} else {
				key_id32_array.push_back(args[i]->GetUIntValue());
			}
		}
		api->set_property(get_arg<UnitRef>(args, 0), get_arg<float>(args, 1), &key_id32_array[0]);
		return CefV8Value::CreateUndefined();
	});
	bind_api(ns, "get_property", [api](const CefV8ValueList& args)
	{
		Array<unsigned> key_id32_array(allocator);
		for (unsigned i = 1; i < args.size(); ++i) {
			if (args[i]->IsString()) {
				key_id32_array.push_back(IdString32(args[i]->GetStringValue().ToString().c_str()).id());
			} else {
				key_id32_array.push_back(args[i]->GetUIntValue());
			}
		}
		return CefV8Value::CreateDouble(api->get_property(get_arg<UnitRef>(args, 0), &key_id32_array[0]));
	});

	BIND_API(num_scene_graph_items);
	BIND_API(find_scene_graph_parent);
	BIND_API(scene_graph_link);
	BIND_API(scene_graph_link_to_none);
	BIND_API(copy_scene_graph_local_from);

	BIND_API(num_lod_objects);
	BIND_API(find_lod_object);
	BIND_API(lod_object);
	BIND_API(num_steps_lod);
	bind_api(ns, "animation_get_curve_value", [api](const CefV8ValueList& args)
	{
		float out_range[2] = {0.0f};
		api->lod_step_range(
			get_arg<LodObjectPtr>(args, 0),
			get_arg<unsigned>(args, 1),
			out_range);
		auto retval = CefV8Value::CreateArray(2);
		retval->SetValue(0, CefV8Value::CreateDouble(out_range[0]));
		retval->SetValue(1, CefV8Value::CreateDouble(out_range[1]));
		return retval;
	});
	BIND_API(num_mesh_lod_step);
	BIND_API(lod_step_meshes);

	BIND_API(num_lights);
	BIND_API(find_light);
	BIND_API(light);
	BIND_API(set_light_material);

	BIND_API(create_vehicle);
	BIND_API(destroy_vehicle);
	BIND_API(has_vehicle);
	BIND_API(vehicle);

	BIND_API(enable_physics);
	BIND_API(disable_physics);
	BIND_API(apply_initial_actor_velocities);

	BIND_API(set_unit_visibility);
	BIND_API(set_mesh_visibility);
	BIND_API(set_cloth_visibility);
	BIND_API(set_visibility_group);
	BIND_API(has_visibility_group);

	BIND_API(create_cloth);
	BIND_API(destroy_cloth);
	BIND_API(num_cloths);
	BIND_API(find_cloth);
	BIND_API(cloth);

	BIND_API(num_cameras);
	BIND_API(find_camera);
	BIND_API(camera);

	BIND_API(has_node);
	BIND_API(node);

	BIND_API(resource_has_node);
	BIND_API(resource_node);
	BIND_API(resource_local_pose);

	BIND_API(world);
	BIND_API(level);
	BIND_API(is_alive);
	BIND_API(id_in_level);
	BIND_API(is_of_resource_type);
	BIND_API(unit_name_s);

	BIND_API(box);
	BIND_API(debug_name);
	bind_api(ns, "name_hash", [api](const CefV8ValueList& args)
	{
		char out_name[8] = { '\0' };
		api->name_hash(get_arg<UnitRef>(args, 0), out_name);
		return CefV8Value::CreateString(out_name);
	});
	BIND_API(is_a);
	BIND_API(set_id_in_level);
	BIND_API(draw_tree);

	bind_api(ns, "mesh_raycast", [api](const CefV8ValueList& args)
	{
		float out_distance;
		CApiVector3 out_normal_world;
		unsigned success = api->mesh_raycast(
			get_arg<UnitRef>(args, 0),
			get_arg<ConstVector3Ptr>(args, 1),
			get_arg<ConstVector3Ptr>(args, 2),
			get_arg<float>(args, 3),
			get_arg<int>(args, 4),
			&out_distance, &out_normal_world);
		if (success == 0)
			return CefV8Value::CreateUndefined();

		CefRefPtr<CefV8Value> retval = CefV8Value::CreateObject(nullptr, nullptr);
		retval->SetValue("distance", CefV8Value::CreateDouble(out_distance), V8_PROPERTY_ATTRIBUTE_READONLY);
		CefRefPtr<CefV8Value> normal_world;
		wrap_result(out_normal_world, normal_world);
		retval->SetValue("normal_world", normal_world, V8_PROPERTY_ATTRIBUTE_READONLY);
		return retval;
	});
	bind_api(ns, "mesh_pick_raycast", [api](const CefV8ValueList& args)
	{
		float out_distance;
		CApiVector3 out_normal_world;
		unsigned out_best_mesh_index, out_best_triangle_index;
		unsigned success = api->mesh_pick_raycast(
			get_arg<UnitRef>(args, 0),
			get_arg<ConstVector3Ptr>(args, 1),
			get_arg<ConstVector3Ptr>(args, 2),
			get_arg<float>(args, 3),
			get_arg<int>(args, 4),
			&out_distance, &out_normal_world,
			&out_best_mesh_index, &out_best_triangle_index);
		if (success == 0)
			return CefV8Value::CreateUndefined();

		CefRefPtr<CefV8Value> retval = CefV8Value::CreateObject(nullptr, nullptr);
		retval->SetValue("distance", CefV8Value::CreateDouble(out_distance), V8_PROPERTY_ATTRIBUTE_READONLY);
		CefRefPtr<CefV8Value> normal_world;
		wrap_result(out_normal_world, normal_world);
		retval->SetValue("normal_world", normal_world, V8_PROPERTY_ATTRIBUTE_READONLY);
		retval->SetValue("best_mesh_index", CefV8Value::CreateUInt(out_best_mesh_index), V8_PROPERTY_ATTRIBUTE_READONLY);
		retval->SetValue("best_triangle_index", CefV8Value::CreateUInt(out_best_triangle_index), V8_PROPERTY_ATTRIBUTE_READONLY);
		return retval;
	});
	bind_api(ns, "mesh_closest_point_raycast", [api](const CefV8ValueList& args)
	{
		CApiVector3 out_best_point_world;
		float out_best_point_distance_along_ray;
		float out_best_point_distance_to_ray;
		unsigned success = api->mesh_closest_point_raycast(
			get_arg<UnitRef>(args, 0),
			get_arg<ConstVector3Ptr>(args, 1),
			get_arg<ConstVector3Ptr>(args, 2),
			get_arg<float>(args, 3),
			&out_best_point_world, &out_best_point_distance_along_ray,
			&out_best_point_distance_to_ray);
		if (success == 0)
			return CefV8Value::CreateUndefined();

		CefRefPtr<CefV8Value> retval = CefV8Value::CreateObject(nullptr, nullptr);
		CefRefPtr<CefV8Value> best_point_world;
		wrap_result(out_best_point_world, best_point_world);
		retval->SetValue("best_point_world", best_point_world, V8_PROPERTY_ATTRIBUTE_READONLY);
		retval->SetValue("best_point_distance_along_ray", CefV8Value::CreateDouble(out_best_point_distance_along_ray), V8_PROPERTY_ATTRIBUTE_READONLY);
		retval->SetValue("best_point_distance_to_ray", CefV8Value::CreateDouble(out_best_point_distance_to_ray), V8_PROPERTY_ATTRIBUTE_READONLY);
		return retval;
	});

	bind_dynamic_data_api<UnitRef>(ns, stingray::api::script->DynamicScriptData->Unit);
}

} // end namespace
