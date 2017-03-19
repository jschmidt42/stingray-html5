#include "html5_api_bindings.h"
#include "html5_api_dynamic_data.inl"

namespace PLUGIN_NAMESPACE {

void bind_api_camera(CefRefPtr<CefV8Value> stingray_ns, const CameraCApi* api)
{
	DEFINE_API("Camera");

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

	BIND_API(near_range);
	BIND_API(far_range);
	BIND_API(set_near_range);
	BIND_API(set_far_range);

	BIND_API(vertical_fov);
	BIND_API(set_vertical_fov);

	BIND_API(projection_type);
	BIND_API(set_projection_type);
	BIND_API(set_orthographic_view);
	BIND_API(set_post_projection_transform);

	BIND_API(set_frustum);
	BIND_API(set_frustum_half_angles);
	BIND_API(inside_frustum);

	BIND_API(node);

	BIND_API(mode);
	BIND_API(set_mode);

	BIND_API(set_local);

	BIND_API(screen_to_world);

	bind_dynamic_data_api<CameraPtr>(ns, stingray::api::script->DynamicScriptData->Camera);
}

} // end namespace
