#include "html5_web_browser.h"
#include "html5_web_view.h"

#include "stingray_api.h"

#include <plugin_foundation/vector.h>
#include <plugin_foundation/matrix4x4.h>
#include <plugin_foundation/math.h>

#include <include/internal/cef_ptr.h>

namespace PLUGIN_NAMESPACE { namespace browser {

struct UnitWebView
{
	UnitWebView()
		: unit(nullptr)
		, web_view(nullptr)
		, mesh_index(UINT_MAX) { }

	UnitWebView(const UnitWebView& uwv)
		: unit(uwv.unit)
		, web_view(uwv.web_view)
		, mesh_index(UINT_MAX) { }

	CApiUnit* unit;
	CefRefPtr<WebView> web_view;
	unsigned mesh_index;
};

Vector<UnitWebView> web_views(allocator);

void init()
{
	web_views.set_allocator(allocator);
}

void shutdown()
{
	web_views.reset();
}

Vector3 get_browser_extents(MeshPtr mesh)
{
	const auto& scale = stingray::api::script->Mesh->local_scale(mesh);
	const BoundingVolumeWrapper& bouding_volume = stingray::api::script->Mesh->bounding_volume(mesh);
	auto extent_x = bouding_volume.max.x - bouding_volume.min.x;
	auto extent_y = bouding_volume.max.y - bouding_volume.min.y;
	auto extent_z = bouding_volume.max.z - bouding_volume.min.z;
	return vector3(extent_x * scale->x, extent_y * scale->y, extent_z * scale->z);
}

Vector3 get_browser_extents_2d(MeshPtr mesh)
{
	auto extents = get_browser_extents(mesh);
	if (extents.z > extents.y)
		std::swap(extents.y, extents.z);
	if (extents.z > extents.x)
		std::swap(extents.x, extents.z);
	return extents;
}

bool try_load(CApiUnit* unit)
{
	auto unit_ref = stingray::api::unit->reference(unit);

	// Define script data field name to get.
	const auto browser_url_key = "browser_url";
	const auto browser_mesh_index_key = "browser_mesh_index";
	const auto browser_material_slot_name_key = "browser_material_slot_name";

	// Do not continue if this unit does not have any Giphy resource.
	if (!stingray::api::script->DynamicScriptData->Unit->has_data(unit_ref, 1, browser_url_key))
		return false;

	// Make sure the unit has all the data we need to display a Giphy on it.
	if (stingray::api::unit->num_meshes(unit) == 0 ||
		!stingray::api::script->DynamicScriptData->Unit->has_data(unit_ref, 1, browser_mesh_index_key) ||
		!stingray::api::script->DynamicScriptData->Unit->has_data(unit_ref, 1, browser_material_slot_name_key))
		return false;

	// Get script data values
	auto browser_url = (const char*)stingray::api::script->DynamicScriptData->Unit->get_data(unit_ref, 1, browser_url_key).pointer;
	auto browser_mesh_index = (unsigned)*(float*)stingray::api::script->DynamicScriptData->Unit->get_data(unit_ref, 1, browser_mesh_index_key).pointer;
	auto browser_material_slot_name = (const char*)stingray::api::script->DynamicScriptData->Unit->get_data(unit_ref, 1, browser_material_slot_name_key).pointer;

	// We need a valid material slot name
	if (strlen(browser_material_slot_name) == 0)
		return false;

	auto browser_mesh = stingray::api::script->Unit->mesh(unit_ref, browser_mesh_index, nullptr);
	auto browser_material = stingray::api::script->Mesh->material(browser_mesh, 0);
	CefRefPtr<WebView> browser_web_view = new WebView(nullptr, browser_material);
	browser_web_view->load_page(browser_url);

	// Get mesh bounding rect
	const float pixel_scale = 300.0f;
	const Vector3& extents = get_browser_extents_2d(browser_mesh);
	const auto aspect_ratio = extents.x / extents.y / 2.0;
	browser_web_view->set_resolution(ceil(pixel_scale * extents.x), ceil(pixel_scale * extents.y));

	UnitWebView uwv;
	uwv.unit = unit;
	uwv.web_view = browser_web_view;
	uwv.mesh_index = browser_mesh_index;
	web_views.push_back(uwv);

	return true;
}

bool try_unload(CApiUnit* unit)
{
	for (auto it = web_views.begin(), end = web_views.end(); it != end; ++it) {
		UnitWebView& uwv = *it;
		if (uwv.unit == unit) {
			uwv.web_view->close_browser();
			uwv.web_view = nullptr;
			web_views.erase(it);
			return true;
		}
	}
	return false;
}

float ray_box_intersection(const Vector3& from, const Vector3& dir, const Matrix4x4& pose, const Vector3& extent)
{
	const Matrix4x4 tminv = inverse(pose);
	const Vector3 p = transform(tminv, from);
	const Vector3 y = transform_without_translation(tminv, dir);

	float tmin = -FLT_MAX;
	float tmax = FLT_MAX;

	// Epsilon to determine if direction is perpendicular to box axis to avoid overflow
	// in later computation.
	const float perpendicular_eps = 1e-6f;

	for (int e = 0; e < 3; ++e) {
		const float ye = element(y, e);
		const float pe = element(p, e);
		const float re = element(extent, e);

		if (ye > -perpendicular_eps && ye < perpendicular_eps) {
			if (pe < -re || pe > re) {
				return -1.0f;
			}
		} else {
			const float t1 = (-re - pe) / ye;
			const float t2 = (re - pe) / ye;
			if (t1 < t2) {
				tmin = math::max(tmin, t1);
				tmax = math::min(tmax, t2);
			} else {
				tmin = math::max(tmin, t2);
				tmax = math::min(tmax, t1);
			}
		}
	}

	if (tmin > tmax || tmax < 0)
		return -1.0f;
	if (tmin < 0)
		return tmax;
	return tmin;
}

void pick(const Vector3& mouse_pos, const Vector3& from, const Vector3& ray)
{
	for (auto it = web_views.begin(), end = web_views.end(); it != end; ++it) {
		UnitWebView& uwv = *it;
		auto unit_ref = stingray::api::unit->reference(uwv.unit);

		// Get pos of browser
		auto browser_pose = stingray::api::script->Unit->world_pose(unit_ref, uwv.mesh_index);
		auto browser_mesh = stingray::api::script->Unit->mesh(unit_ref, uwv.mesh_index, nullptr);
		const Vector3& half_extents = get_browser_extents(browser_mesh) / 2.0f;
		float d = ray_box_intersection(from, ray, *browser_pose, half_extents);
		if (d > 0.0f) {
			const auto& resolution = uwv.web_view->resolution();
			Vector3 hit = from + ray * d;

			Vector3 proj_hit = transform(inverse(*browser_pose), hit);
			Vector2 half_res = vector2(resolution.x / 2.0f, resolution.y / 2.0f);
			float sx = (proj_hit.x / half_extents.x) * half_res.x + half_res.x;
			float sy = resolution.y - ((proj_hit.y / half_extents.y) * half_res.y + half_res.y);

			uwv.web_view->send_mouse_event(sx, sy, WebView::LEFT, false);
			uwv.web_view->send_mouse_event(sx, sy, WebView::LEFT, true);

			stingray::api::log->info("pick", stingray::api::error->eprintf("(%f, %f) -> %s", sx, sy, uwv.web_view->browser()->GetMainFrame()->GetURL().ToString().c_str()));
		}
	}
}

}} // end namespace
