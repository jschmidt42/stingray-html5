#include "html5_api_bindings.h"

#include <plugin_foundation/matrix4x4.h>
#include <plugin_foundation/quaternion.h>
#include <plugin_foundation/vector2.h>

namespace PLUGIN_NAMESPACE {

void bind_api_matrix4x4(CefRefPtr<CefV8Value> stingray_ns)
{
	CefRefPtr<CefV8Value> ns = CefV8Value::CreateObject(nullptr, nullptr);

	create_handler(ns, "zero", matrix4x4_zero);
	create_handler(ns, "identity", matrix4x4_identity);
	create_handler(ns, "identity", matrix4x4_identity);
	bind_api(ns, "forward_axis", [](const CefV8ValueList& args)
	{
		const Matrix4x4& m = get_arg<Matrix4x4>(args, 0);
		CefRefPtr<CefV8Value> retval;
		const Vector3& axis = forward_axis(m);
		wrap_result(axis, retval);
		return retval;
	});

	bind_api(ns, "from_quaternion", [](const CefV8ValueList& args)
	{
		const Quaternion& q = get_arg<Quaternion>(args, 0);
		CefRefPtr<CefV8Value> retval;
		const Matrix4x4& m = matrix4x4(q);
		wrap_result(m, retval);
		return retval;
	});

	bind_api(ns, "x", [](const CefV8ValueList& args)
	{
		CefRefPtr<CefV8Value> retval;
		wrap_result(x_axis(get_arg<Matrix4x4>(args, 0)), retval);
		return retval;
	});

	bind_api(ns, "y", [](const CefV8ValueList& args)
	{
		CefRefPtr<CefV8Value> retval;
		wrap_result(y_axis(get_arg<Matrix4x4>(args, 0)), retval);
		return retval;
	});

	bind_api(ns, "z", [](const CefV8ValueList& args)
	{
		CefRefPtr<CefV8Value> retval;
		wrap_result(z_axis(get_arg<Matrix4x4>(args, 0)), retval);
		return retval;
	});

	bind_api(ns, "transform", [](const CefV8ValueList& args)
	{
		CefRefPtr<CefV8Value> retval;
		wrap_result(transform(get_arg<Matrix4x4>(args, 0), get_arg<Vector3>(args, 1)), retval);
		return retval;
	});

	stingray_ns->SetValue("Matrix4x4", ns, V8_PROPERTY_ATTRIBUTE_READONLY);
}

void bind_api_vector2(CefRefPtr<CefV8Value> stingray_ns)
{
	CefRefPtr<CefV8Value> ns = CefV8Value::CreateObject(nullptr, nullptr);

	bind_api(ns, "zero", [](const CefV8ValueList& args)
	{
		CefRefPtr<CefV8Value> retval;
		wrap_result(vector2(0,0), retval);
		return retval;
	});

	bind_api(ns, "create", [](const CefV8ValueList& args)
	{
		CefRefPtr<CefV8Value> retval;
		float v1 = get_arg<float>(args, 0);
		float v2 = get_arg<float>(args, 1);
		wrap_result(vector2(v1,v2), retval);
		return retval;
	});

	bind_api(ns, "equal", [](const CefV8ValueList& args)
	{
		return CefV8Value::CreateBool(get_arg<Vector2>(args, 0) == get_arg<Vector2>(args, 1));
	});

	bind_api(ns, "normalize", [](const CefV8ValueList& args)
	{
		const Vector2& nv = normalize(get_arg<Vector2>(args, 0));
		CefRefPtr<CefV8Value> retval;
		wrap_result(nv, retval);
		return retval;
	});

	bind_api(ns, "add", [](const CefV8ValueList& args)
	{
		CefRefPtr<CefV8Value> retval;
		wrap_result(get_arg<Vector2>(args, 0) + get_arg<Vector2>(args, 1), retval);
		return retval;
	});

	bind_api(ns, "subtract", [](const CefV8ValueList& args)
	{
		CefRefPtr<CefV8Value> retval;
		wrap_result(get_arg<Vector2>(args, 0) - get_arg<Vector2>(args, 1), retval);
		return retval;
	});

	bind_api(ns, "multiply", [](const CefV8ValueList& args)
	{
		CefRefPtr<CefV8Value> retval;
		wrap_result(get_arg<Vector2>(args, 0) * get_arg<float>(args, 1), retval);
		return retval;
	});

	bind_api(ns, "multiply_elements", [](const CefV8ValueList& args)
	{
		CefRefPtr<CefV8Value> retval;
		wrap_result(get_arg<Vector2>(args, 0) * get_arg<Vector2>(args, 1), retval);
		return retval;
	});
	bind_api(ns, "dot", [](const CefV8ValueList& args)
	{
		CefRefPtr<CefV8Value> retval;
		wrap_result(dot(get_arg<Vector3>(args, 0), get_arg<Vector3>(args, 1)), retval);
		return retval;
	});
	stingray_ns->SetValue("Vector2", ns, V8_PROPERTY_ATTRIBUTE_READONLY);
}

void bind_api_vector3(CefRefPtr<CefV8Value> stingray_ns)
{
	CefRefPtr<CefV8Value> ns = CefV8Value::CreateObject(nullptr, nullptr);

	bind_api(ns, "zero", [](const CefV8ValueList& args)
	{
		CefRefPtr<CefV8Value> retval;
		wrap_result(vector3(0,0,0), retval);
		return retval;
	});

	bind_api(ns, "equal", [](const CefV8ValueList& args)
	{
		return CefV8Value::CreateBool(get_arg<Vector3>(args, 0) == get_arg<Vector3>(args, 1));
	});

	bind_api(ns, "normalize", [](const CefV8ValueList& args)
	{
		const Vector3& nv = normalize(get_arg<Vector3>(args, 0));
		CefRefPtr<CefV8Value> retval;
		wrap_result(nv, retval);
		return retval;
	});

	bind_api(ns, "add", [](const CefV8ValueList& args)
	{
		CefRefPtr<CefV8Value> retval;
		wrap_result(get_arg<Vector3>(args, 0) + get_arg<Vector3>(args, 1), retval);
		return retval;
	});

	bind_api(ns, "subtract", [](const CefV8ValueList& args)
	{
		CefRefPtr<CefV8Value> retval;
		wrap_result(get_arg<Vector3>(args, 0) - get_arg<Vector3>(args, 1), retval);
		return retval;
	});

	bind_api(ns, "multiply", [](const CefV8ValueList& args)
	{
		CefRefPtr<CefV8Value> retval;
		wrap_result(get_arg<Vector3>(args, 0) * get_arg<float>(args, 1), retval);
		return retval;
	});

	bind_api(ns, "multiply_elements", [](const CefV8ValueList& args)
	{
		CefRefPtr<CefV8Value> retval;
		wrap_result(get_arg<Vector3>(args, 0) * get_arg<Vector3>(args, 1), retval);
		return retval;
	});

	bind_api(ns, "dot", [](const CefV8ValueList& args)
	{
		CefRefPtr<CefV8Value> retval;
		wrap_result(dot(get_arg<Vector3>(args, 0), get_arg<Vector3>(args, 1)), retval);
		return retval;
	});

	stingray_ns->SetValue("Vector3", ns, V8_PROPERTY_ATTRIBUTE_READONLY);
}

void bind_api_quaternion(CefRefPtr<CefV8Value> stingray_ns)
{
	CefRefPtr<CefV8Value> ns = CefV8Value::CreateObject(nullptr, nullptr);

	bind_api(ns, "identity", [](const CefV8ValueList& args)
	{
		CefRefPtr<CefV8Value> retval;
		wrap_result(quaternion_identity(), retval);
		return retval;
	});

	bind_api(ns, "multiply", [](const CefV8ValueList& args)
	{
		CefRefPtr<CefV8Value> retval;
		wrap_result(get_arg<Quaternion>(args, 0) * get_arg<Quaternion>(args, 1), retval);
		return retval;
	});

	bind_api(ns, "forward", [](const CefV8ValueList& args)
	{
		CefRefPtr<CefV8Value> retval;
		wrap_result(forward_axis(matrix4x4(get_arg<Quaternion>(args, 0))), retval);
		return retval;
	});

	bind_api(ns, "up", [](const CefV8ValueList& args)
	{
		CefRefPtr<CefV8Value> retval;
		wrap_result(up_axis(matrix4x4(get_arg<Quaternion>(args, 0))), retval);
		return retval;
	});

	bind_api(ns, "right", [](const CefV8ValueList& args)
	{
		CefRefPtr<CefV8Value> retval;
		wrap_result(right_axis(matrix4x4(get_arg<Quaternion>(args, 0))), retval);
		return retval;
	});

	bind_api(ns, "axis_angle", [](const CefV8ValueList& args)
	{
		CefRefPtr<CefV8Value> retval;
		wrap_result(quaternion(get_arg<Vector3>(args, 0), get_arg<float>(args, 1)), retval);
		return retval;
	});

	stingray_ns->SetValue("Quaternion", ns, V8_PROPERTY_ATTRIBUTE_READONLY);
}

void bind_api_math(CefRefPtr<CefV8Value> stingray_ns)
{
	bind_api_vector2(stingray_ns);
	bind_api_vector3(stingray_ns);
	bind_api_quaternion(stingray_ns);
	bind_api_matrix4x4(stingray_ns);
}

} // end namespace
