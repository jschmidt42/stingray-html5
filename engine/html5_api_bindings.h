#pragma once

#include "stingray_api.h"

#include <plugin_foundation/id_string.h>
#include <plugin_foundation/matrix4x4.h>
#include <engine_plugin_api/c_api/c_api_types.h>

#include <include/cef_v8.h>
#include <include/cef_base.h>

#include <functional>

namespace PLUGIN_NAMESPACE {

using namespace stingray_plugin_foundation;

struct UserObject : CefBase
{
	enum UserObjectType : unsigned char {
		ID = 1,
		OBJECT_PTR = 2,
		OBJECT_DATA = 3
	};

	UserObjectType type;
	union {
		uint64_t id64;
		void* o;
	};

	size_t data_size;

	uint64_t id() const
	{
		if (type != UserObjectType::ID)
			return 0;
		return id64;
	}

	void* ptr() const
	{
		if (type == UserObjectType::ID)
			return (void*)id64;

		if (type == UserObjectType::OBJECT_PTR || type == UserObjectType::OBJECT_DATA)
			return o;

		return nullptr;
	}

	size_t size() const
	{
		return data_size;
	}

	static CefRefPtr<CefV8Value> CreateId(uint64_t id)
	{
		CefRefPtr<CefV8Value> user_object = CefV8Value::CreateObject(nullptr, nullptr);
		user_object->SetUserData(new UserObject(id));
		user_object->SetValue("@id", CefV8Value::CreateDouble(id), V8_PROPERTY_ATTRIBUTE_READONLY);
		return user_object;
	}
	static CefRefPtr<CefV8Value> CreateObjectPtr(void* obj)
	{
		CefRefPtr<CefV8Value> user_object = CefV8Value::CreateObject(nullptr, nullptr);
		user_object->SetUserData(new UserObject(obj));
		return user_object;
	}
	static CefRefPtr<CefV8Value> CreateObjectData(const void* obj, size_t size)
	{
		CefRefPtr<CefV8Value> user_object = CefV8Value::CreateObject(nullptr, nullptr);
		user_object->SetUserData(new UserObject(obj, size));
		return user_object;
	}

private:
	explicit UserObject(uint64_t id) : type(OBJECT_PTR), id64(id), data_size(0) {}
	explicit UserObject(void* obj): type(OBJECT_PTR), o(obj), data_size(0) {}

	explicit UserObject(const void* obj, size_t size) : type(UserObjectType::OBJECT_DATA), o(nullptr), data_size(size)
	{
		o = allocator.allocate(data_size);
		memcpy(o, obj, data_size);
	}
	
	~UserObject()
	{
		if (type == UserObjectType::OBJECT_DATA) {
			allocator.deallocate(o);
		}
	}

	IMPLEMENT_REFCOUNTING(UserObject)
};

template<typename T, unsigned STACK_SIZE = 16>
struct CachedValue
{
	struct Scope
	{
		explicit Scope(CachedValue<T, STACK_SIZE>& ref_tv) : tv(ref_tv) {}
		~Scope() { ++tv.i; }
		CachedValue<T>& tv;
	};

	T& operator=(const T& lhs)
	{
		v[i % STACK_SIZE] = lhs;
		return **this;
	}

	T& operator*()
	{
		return v[i % STACK_SIZE];
	}

	unsigned i = 0;
	T v[STACK_SIZE];
};

#define CACHED_VALUE(T, VAR_NAME) static thread_local CachedValue<T> VAR_NAME; CachedValue<T>::Scope __tvs_ ##VAR_NAME (VAR_NAME)

// GET ARGS

typedef void(*VoidCallbackParamVoidPtr)(void const *);

template<typename T_PTR> T_PTR get_ptr(CefRefPtr<CefV8Value> value)
{
	if (!value->IsUserCreated())
		throw std::exception("Value is not user created");
	UserObject* user_object = static_cast<UserObject*>(value->GetUserData().get());
	if (!user_object)
		throw std::exception("Value is not a user object");
	return static_cast<T_PTR>(user_object->ptr());
}

// ReSharper disable once CppFunctionIsNotImplemented
template<typename T> T get_arg(const CefV8ValueList& args, unsigned i);

template<typename T> T get_arg_struct(const CefV8ValueList& args, unsigned i)
{
	if (i >= args.size() || !args[i]->IsValid() || !args[i]->IsUserCreated() || args[i]->IsNull() || args[i]->IsUndefined())
		return nullptr;
	return get_ptr<T>(args[i]);
}

template<typename E> E get_arg_enum(const CefV8ValueList& args, unsigned i)
{
	if (i >= args.size() || !args[i]->IsValid())
		return (E)0;
	return (E)args[i]->GetIntValue();
}

template<> inline VoidCallbackParamVoidPtr get_arg<VoidCallbackParamVoidPtr>(const CefV8ValueList& args, unsigned i)
{
	// TODO:
	return nullptr;
}

template<> inline char const * get_arg<char const *>(const CefV8ValueList& args, unsigned i)
{
	if (i >= args.size() || args[i]->IsNull() || args[i]->IsUndefined())
		return nullptr;
	if (!args[i]->IsString())
		throw std::exception("Argument must be a string");
	CACHED_VALUE(std::string, str);
	*str = args[i]->GetStringValue().ToString();
	return (*str).c_str();
}

template<> inline unsigned int get_arg<unsigned int>(const CefV8ValueList& args, unsigned i)
{
	if (i >= args.size() || !args[i]->IsValid())
		return 0;
	if (args[i]->IsString())
		return IdString32(args[i]->GetStringValue().ToString().c_str()).id();
	return args[i]->GetUIntValue();
}

template<> inline int* get_arg<int*>(const CefV8ValueList& args, unsigned i)
{
	if (i >= args.size() || !args[i]->IsValid())
		return nullptr;

	if (args[i]->IsUserCreated()) {
		return get_arg_struct<int*>(args, i);
	}

	if (args[i]->IsUInt()) {
		CACHED_VALUE(int, tv);
		tv = args[i]->GetUIntValue();
		return &*tv;
	}

	return nullptr;
}

template<> inline unsigned int* get_arg<unsigned int*>(const CefV8ValueList& args, unsigned i)
{
	if (i >= args.size() || !args[i]->IsValid())
		return nullptr;

	if (args[i]->IsUserCreated()) {
		return get_arg_struct<unsigned int*>(args, i);
	}

	if (args[i]->IsUInt()) {
		CACHED_VALUE(unsigned, tv);
		tv = args[i]->GetUIntValue();
		return &*tv;
	}

	return nullptr;
}

template<> inline int get_arg<int>(const CefV8ValueList& args, unsigned i)
{
	if (i >= args.size() || !args[i]->IsValid())
		return 0;
	if (args[i]->IsBool())
		return args[i]->GetBoolValue() ? 1 : 0;
	return args[i]->GetIntValue();
}

template<> inline float get_arg<float>(const CefV8ValueList& args, unsigned i)
{
	if (i >= args.size() || !args[i]->IsValid())
		return std::numeric_limits<float>::infinity();
	return (float)args[i]->GetDoubleValue();
}

template<> inline uint64_t get_arg<uint64_t>(const CefV8ValueList& args, unsigned i)
{
	if (i >= args.size() || !args[i]->IsValid())
		return 0;
	if (args[i]->IsString())
		return IdString64(args[i]->GetStringValue().ToString().c_str()).id();
	if (args[i]->IsDouble())
		return (uint64_t)args[i]->GetDoubleValue();
	if (args[i]->IsUserCreated())
		return static_cast<UserObject*>(args[i]->GetUserData().get())->id();
	return 0;
}

template<> inline WindowRectWrapper get_arg<WindowRectWrapper>(const CefV8ValueList& args, unsigned i)
{
	if (i >= args.size() || !args[i]->IsValid() || !args[i]->IsArray() || args[i]->GetArrayLength() != 4)
		throw std::exception("Argument is not a rect");
	WindowRectWrapper rect;
	for (int pi = 0; pi < 4; ++pi) {
		rect.pos[pi] = args[pi]->GetValue(0)->GetIntValue();
	}
	return rect;
}

template<> inline WindowOpenParameter* get_arg<WindowOpenParameter*>(const CefV8ValueList& args, unsigned i)
{
	if (i >= args.size() || !args[i]->IsValid() || !args[i]->IsObject())
		throw std::exception("Argument is a window open parameters object");

	auto arg = args[i];
	CACHED_VALUE(WindowOpenParameter, cv);
	WindowOpenParameter& open_params = *cv;
	if (arg->HasValue( "x" )) open_params.x = arg->GetValue( "x" )->GetIntValue();;
	if (arg->HasValue( "y" )) open_params.y = arg->GetValue( "y" )->GetIntValue();;
	if (arg->HasValue( "width" )) open_params.width = arg->GetValue( "width" )->GetIntValue();;
	if (arg->HasValue( "height" )) open_params.height = arg->GetValue( "height" )->GetIntValue();;

	if (arg->HasValue( "explicit_resize" )) open_params.explicit_resize = arg->GetValue( "explicit_resize" )->GetBoolValue();;
	if (arg->HasValue( "main_window" )) open_params.main_window = arg->GetValue( "main_window" )->GetBoolValue();;
	if (arg->HasValue( "visible" )) open_params.visible = arg->GetValue( "visible" )->GetBoolValue();;
	if (arg->HasValue( "pass_key_events_to_parent" )) open_params.pass_key_events_to_parent = arg->GetValue( "pass_key_events_to_parent" )->GetBoolValue();;
	if (arg->HasValue( "layered" )) open_params.layered = arg->GetValue( "layered" )->GetBoolValue();;

	CACHED_VALUE(std::string, optional_title);
	*optional_title = arg->GetValue("title")->GetStringValue().ToString();
	if (arg->HasValue("title")) open_params.optional_title = (*optional_title).c_str();
	if (arg->HasValue("parent")) open_params.optional_parent = get_ptr<WindowPtr>(arg->GetValue("parent"));
	return &open_params;
}

template<> inline const CApiVector2* get_arg<const CApiVector2*>(const CefV8ValueList& args, unsigned i)
{
	if (i >= args.size() || !args[i]->IsValid())
		return nullptr;
	if (args[i]->IsArray() && args[i]->GetArrayLength() == 2) {
		CACHED_VALUE(CApiVector2, tv);
		(*tv).x = (float)args[i]->GetValue(0)->GetDoubleValue();
		(*tv).y = (float)args[i]->GetValue(1)->GetDoubleValue();
		return &*tv;
	}
	return nullptr;
}

template<> inline CApiVector2 get_arg<CApiVector2>(const CefV8ValueList& args, unsigned i)
{
	return *get_arg<const CApiVector2*>(args, i);
}

template<> inline const CApiVector3* get_arg<const CApiVector3*>(const CefV8ValueList& args, unsigned i)
{
	if (i >= args.size() || !args[i]->IsValid())
		return nullptr;
	if (args[i]->IsArray() && args[i]->GetArrayLength() == 3) {
		CACHED_VALUE(CApiVector3, tv);
		(*tv).x = (float)args[i]->GetValue(0)->GetDoubleValue();
		(*tv).y = (float)args[i]->GetValue(1)->GetDoubleValue();
		(*tv).z = (float)args[i]->GetValue(2)->GetDoubleValue();
		return &*tv;
	}
	return nullptr;
}

template<> inline CApiVector3 get_arg<CApiVector3>(const CefV8ValueList& args, unsigned i)
{
	return *get_arg<const CApiVector3*>(args, i);
}

template<> inline const CApiVector4* get_arg<const CApiVector4*>(const CefV8ValueList& args, unsigned i)
{
	if (i >= args.size() || !args[i]->IsValid())
		return nullptr;
	if (args[i]->IsArray() && args[i]->GetArrayLength() == 4) {
		CACHED_VALUE(CApiVector4, tv);
		(*tv).x = (float)args[i]->GetValue(0)->GetDoubleValue();
		(*tv).y = (float)args[i]->GetValue(1)->GetDoubleValue();
		(*tv).z = (float)args[i]->GetValue(2)->GetDoubleValue();
		(*tv).w = (float)args[i]->GetValue(3)->GetDoubleValue();
		return &*tv;
	}
	return nullptr;
}

template<> inline CApiVector4 get_arg<CApiVector4>(const CefV8ValueList& args, unsigned i)
{
	return *get_arg<const CApiVector4*>(args, i);
}

template<> inline const CApiQuaternion* get_arg<const CApiQuaternion*>(const CefV8ValueList& args, unsigned i)
{
	if (i >= args.size() || !args[i]->IsValid())
		return nullptr;
	if (args[i]->IsArray() && args[i]->GetArrayLength() == 4) {
		CACHED_VALUE(CApiQuaternion, tv);
		(*tv).x = (float)args[i]->GetValue(0)->GetDoubleValue();
		(*tv).y = (float)args[i]->GetValue(1)->GetDoubleValue();
		(*tv).z = (float)args[i]->GetValue(2)->GetDoubleValue();
		(*tv).w = (float)args[i]->GetValue(3)->GetDoubleValue();
		return &*tv;
	}
	return nullptr;
}

template<> inline CApiQuaternion get_arg<CApiQuaternion>(const CefV8ValueList& args, unsigned i)
{
	return *get_arg<const CApiQuaternion*>(args, i);
}

template<> inline const Matrix4x4* get_arg<const Matrix4x4*>(const CefV8ValueList& args, unsigned arg_index)
{
	if (arg_index >= args.size() || !args[arg_index]->IsValid())
		throw std::exception("Argument must be a matrix");
	CACHED_VALUE(Matrix4x4, m);
	if (args[arg_index]->IsArray() && args[arg_index]->GetArrayLength() == 16) {
		for (unsigned i = 0; i < 16; ++i)
			(*m).v[i] = args[arg_index]->GetValue(i)->GetDoubleValue();
		return &*m;
	}
	throw std::exception("Cannot get matrix 4x4 from argument");
}

template<> inline Matrix4x4 get_arg<Matrix4x4>(const CefV8ValueList& args, unsigned arg_index)
{
	return *get_arg<const Matrix4x4*>(args, arg_index);
}

template<> inline DynamicScriptDataItem get_arg<DynamicScriptDataItem>(const CefV8ValueList& args, unsigned index)
{
	if (index >= args.size() || !args[index]->IsValid())
		throw std::exception("Argument not of type DynamicScriptDataItem");
	const auto& arg = args[index];
	DynamicScriptDataItem result = { nullptr };

	if (arg->IsNull() || arg->IsUndefined()) {
		result.type = D_DATA_NIL_TYPE;
	} else if (arg->IsBool()) {
		result.type = D_DATA_BOOLEAN_TYPE;
		result.pointer = (const void*)(int)arg->GetBoolValue();
		result.size = sizeof(int);
	} else if (arg->IsDouble() || arg->IsUInt() || arg->IsInt()) {
		// TODO: Handle D_DATA_CUSTOM_TUNITREFERENCE
		float f = 0.0f;
		if (arg->IsDouble())
			f = (float)arg->GetDoubleValue();
		else if (arg->IsUInt())
			f = (float)arg->GetUIntValue();
		else if (arg->IsInt())
			f = (float)arg->GetIntValue();
		result.type = D_DATA_NUMBER_TYPE;
		result.size = sizeof(float);
		memcpy((void*)&result.pointer, &f, result.size);
	} else if (arg->IsString()) {
		static thread_local auto cached_string = arg->GetStringValue().ToString();
		result.type = D_DATA_STRING_TYPE;
		result.pointer = cached_string.c_str();
		result.size = (unsigned)cached_string.size();
	} else if (arg->IsArray() && arg->GetArrayLength() == 2) {
		static thread_local Vector2 cached_v2 = { (float)arg->GetValue(0)->GetDoubleValue(), (float)arg->GetValue(1)->GetDoubleValue() };
		result.type = D_DATA_CUSTOM_TVECTOR2;
		result.pointer = &cached_v2;
		result.size = sizeof(Vector2);
	} else if (arg->IsArray() && arg->GetArrayLength() == 3) {
		static thread_local Vector3 cached_v3 = { (float)arg->GetValue(0)->GetDoubleValue(), (float)arg->GetValue(1)->GetDoubleValue(), (float)arg->GetValue(2)->GetDoubleValue() };
		result.type = D_DATA_CUSTOM_TVECTOR3;
		result.pointer = &cached_v3;
		result.size = sizeof(Vector3);
	} else if (arg->IsArray() && arg->GetArrayLength() == 4) {
		static thread_local Vector4 cached_v4 = {
			(float)arg->GetValue(0)->GetDoubleValue(), (float)arg->GetValue(1)->GetDoubleValue(),
			(float)arg->GetValue(2)->GetDoubleValue(), (float)arg->GetValue(3)->GetDoubleValue() };
		result.type = D_DATA_CUSTOM_TVECTOR4;
		result.pointer = &cached_v4;
		result.size = sizeof(Vector4);
	} else if (result.type == D_DATA_CUSTOM_TMATRIX4X4) {
		static thread_local Matrix4x4 cached_m;
		for (int i = 0; i < 16; ++i)
			cached_m.v[i] = (float)arg->GetValue(i)->GetDoubleValue();
		result.type = D_DATA_CUSTOM_TMATRIX4X4;
		result.pointer = &cached_m;
		result.size = sizeof(Matrix4x4);
	} else if (arg->IsObject() && arg->IsUserCreated()) {
		UserObject* user_object = static_cast<UserObject*>(arg->GetUserData().get());;
		if (user_object->type == UserObject::ID) {
			result.type = D_DATA_CUSTOM_ID64;
			result.pointer = (const void*)user_object->id();
			result.size = sizeof(uint64_t);
		} else if (user_object->type == UserObject::OBJECT_PTR) {
			result.type = D_DATA_CUSTOM_TPOINTER;
			result.pointer = user_object->ptr();
			result.size = (unsigned)user_object->size();
		}
	} else
		throw std::exception("Dynamic data type not supported");

	return result;
}

template<> inline DeadZoneSetting* get_arg<DeadZoneSetting*>(const CefV8ValueList& args, unsigned arg_index)
{
	if (arg_index >= args.size() || !args[arg_index]->IsObject() || args[arg_index]->IsUndefined() || args[arg_index]->IsNull())
		return nullptr;
	CACHED_VALUE(DeadZoneSetting, dzs);
	(*dzs).mode = (DeadZoneMode)args[arg_index]->GetValue("mode")->GetIntValue();
	(*dzs).size = args[arg_index]->GetValue("size")->GetDoubleValue();
	return &*dzs;
}

template<> inline RumbleParameters* get_arg<RumbleParameters*>(const CefV8ValueList& args, unsigned arg_index)
{
	if (arg_index >= args.size() || !args[arg_index]->IsObject() || args[arg_index]->IsUndefined() || args[arg_index]->IsNull())
		return nullptr;
	CACHED_VALUE(RumbleParameters, params);
	(*params).frequency = args[arg_index]->GetValue("frequency")->GetDoubleValue();
	(*params).offset = args[arg_index]->GetValue("offset")->GetDoubleValue();
	(*params).attack_level = args[arg_index]->GetValue("attack_level")->GetDoubleValue();
	(*params).sustain_level = args[arg_index]->GetValue("sustain_level")->GetDoubleValue();
	(*params).attack = args[arg_index]->GetValue("attack")->GetDoubleValue();
	(*params).release = args[arg_index]->GetValue("release")->GetDoubleValue();
	(*params).sustain = args[arg_index]->GetValue("sustain")->GetDoubleValue();
	(*params).decay = args[arg_index]->GetValue("decay")->GetDoubleValue();
	return &*params;
}

#define DEFINE_GET_ARG_STRUCT(__T) \
	template<> inline __T get_arg<__T>(const CefV8ValueList& args, unsigned i) { return get_arg_struct<__T>(args, i); } \
	template<> inline const __T get_arg<const __T>(const CefV8ValueList& args, unsigned i) { return get_arg_struct<const __T>(args, i); }

#define DEFINE_GET_ARG_ENUM(__E) \
	template<> inline __E get_arg<__E>(const CefV8ValueList& args, unsigned i) { return get_arg_enum<__E>(args, i); }

DEFINE_GET_ARG_STRUCT(void *)
DEFINE_GET_ARG_STRUCT(DynamicScriptDataItem *)
DEFINE_GET_ARG_STRUCT(CApiWorld *)
DEFINE_GET_ARG_STRUCT(CApiShadingEnvironment *)
DEFINE_GET_ARG_STRUCT(CApiCallbackData32 *)
DEFINE_GET_ARG_STRUCT(CApiLevel *)
DEFINE_GET_ARG_STRUCT(CApiViewport *)
DEFINE_GET_ARG_STRUCT(CApiCamera *)
DEFINE_GET_ARG_STRUCT(CApiWindow *)
DEFINE_GET_ARG_STRUCT(CApiWorldConfig *)
DEFINE_GET_ARG_STRUCT(CApiMover *)
DEFINE_GET_ARG_STRUCT(CApiMaterialData *)
DEFINE_GET_ARG_STRUCT(CApiMaterial *)
DEFINE_GET_ARG_STRUCT(AnimationEventParameters *)
DEFINE_GET_ARG_STRUCT(AnimationStates *)
DEFINE_GET_ARG_STRUCT(AnimationLayerSeeds *)
DEFINE_GET_ARG_STRUCT(CApiActor *)
DEFINE_GET_ARG_STRUCT(CApiLight *)
DEFINE_GET_ARG_STRUCT(CApiGui *)
DEFINE_GET_ARG_STRUCT(CApiVideoPlayer *)
DEFINE_GET_ARG_STRUCT(CApiReplay *)
DEFINE_GET_ARG_STRUCT(TimeStepPolicyWrapper *)
DEFINE_GET_ARG_STRUCT(MultipleStringsBuffer *)

DEFINE_GET_ARG_STRUCT(CApiLocalTransform *)

DEFINE_GET_ARG_ENUM(WorldCApi_OrphanedParticlePolicy)
DEFINE_GET_ARG_ENUM(AnimationBoneRootMode)
DEFINE_GET_ARG_ENUM(AnimationBlendType)
DEFINE_GET_ARG_ENUM(ReplayRecordMode)
DEFINE_GET_ARG_ENUM(TimeStepPolicyType)
DEFINE_GET_ARG_ENUM(CameraProjectionType)
DEFINE_GET_ARG_ENUM(CameraMode)
DEFINE_GET_ARG_ENUM(WindowKeystrokes)

// WRAP RESULT

inline void wrap_result(void* r, CefRefPtr<CefV8Value>& retval) { retval = UserObject::CreateObjectPtr(r); }
inline void wrap_result(const void* r, CefRefPtr<CefV8Value>& retval) { retval = UserObject::CreateObjectPtr((void*)r); }
inline void wrap_result(const unsigned* r, CefRefPtr<CefV8Value>& retval) { retval = UserObject::CreateObjectPtr((void*)r); }
inline void wrap_result(const char* r, CefRefPtr<CefV8Value>& retval)
{
	retval = CefV8Value::CreateString(r);
}
inline void wrap_result(unsigned int r, CefRefPtr<CefV8Value>& retval)
{
	retval = CefV8Value::CreateUInt(r);
}
inline void wrap_result(int r, CefRefPtr<CefV8Value>& retval)
{
	retval = CefV8Value::CreateInt(r);
}
inline void wrap_result(float r, CefRefPtr<CefV8Value>& retval) { retval = CefV8Value::CreateDouble((double)r); }
inline void wrap_result(double r, CefRefPtr<CefV8Value>& retval)
{
	retval = CefV8Value::CreateDouble(r);
}
inline void wrap_result(uint64_t r, CefRefPtr<CefV8Value>& retval) { retval = UserObject::CreateId(r); }
inline void wrap_result(const CApiVector3* v, CefRefPtr<CefV8Value>& retval)
{
	retval = CefV8Value::CreateArray(3);
	retval->SetValue(0, CefV8Value::CreateDouble(v->x));
	retval->SetValue(1, CefV8Value::CreateDouble(v->y));
	retval->SetValue(2, CefV8Value::CreateDouble(v->z));
}

inline void wrap_result(const CApiVector2* v, CefRefPtr<CefV8Value>& retval)
{
	retval = CefV8Value::CreateArray(2);
	retval->SetValue(0, CefV8Value::CreateDouble(v->x));
	retval->SetValue(1, CefV8Value::CreateDouble(v->y));
}

inline void wrap_result(const Matrix4x4& m, CefRefPtr<CefV8Value>& retval)
{
	retval = CefV8Value::CreateArray(16);
	for (unsigned i = 0; i < 16; ++i) {
		retval->SetValue(i, CefV8Value::CreateDouble(m.v[i]));
	}
}
inline void wrap_result(const Matrix4x4* m, CefRefPtr<CefV8Value>& retval)
{
	wrap_result(*m, retval);
}
inline void wrap_result(const CApiVector3& v, CefRefPtr<CefV8Value>& retval)
{
	retval = CefV8Value::CreateArray(3);
	retval->SetValue(0, CefV8Value::CreateDouble(v.x));
	retval->SetValue(1, CefV8Value::CreateDouble(v.y));
	retval->SetValue(2, CefV8Value::CreateDouble(v.z));
}
inline void wrap_result(const CApiVector2& v, CefRefPtr<CefV8Value>& retval)
{
	retval = CefV8Value::CreateArray(2);
	retval->SetValue(0, CefV8Value::CreateDouble(v.x));
	retval->SetValue(1, CefV8Value::CreateDouble(v.y));
}
inline void wrap_result(const CApiQuaternion& q, CefRefPtr<CefV8Value>& retval)
{
	retval = CefV8Value::CreateArray(4);
	retval->SetValue(0, CefV8Value::CreateDouble(q.x));
	retval->SetValue(1, CefV8Value::CreateDouble(q.y));
	retval->SetValue(2, CefV8Value::CreateDouble(q.z));
	retval->SetValue(3, CefV8Value::CreateDouble(q.w));
}
inline void wrap_result(const CApiLocalTransform* pose, CefRefPtr<CefV8Value>& retval)
{
	retval = CefV8Value::CreateObject(nullptr, nullptr);
	CefRefPtr<CefV8Value> rot = CefV8Value::CreateArray(9);
	CefRefPtr<CefV8Value> pos = CefV8Value::CreateArray(3);
	CefRefPtr<CefV8Value> scale = CefV8Value::CreateArray(3);

	pos->SetValue(0, CefV8Value::CreateDouble(pose->pos.x));
	pos->SetValue(1, CefV8Value::CreateDouble(pose->pos.y));
	pos->SetValue(2, CefV8Value::CreateDouble(pose->pos.z));

	scale->SetValue(0, CefV8Value::CreateDouble(pose->scale.x));
	scale->SetValue(1, CefV8Value::CreateDouble(pose->scale.y));
	scale->SetValue(2, CefV8Value::CreateDouble(pose->scale.z));

	rot->SetValue(0, CefV8Value::CreateDouble(pose->rot.x.x));
	rot->SetValue(1, CefV8Value::CreateDouble(pose->rot.x.y));
	rot->SetValue(2, CefV8Value::CreateDouble(pose->rot.x.z));
	rot->SetValue(3, CefV8Value::CreateDouble(pose->rot.y.x));
	rot->SetValue(4, CefV8Value::CreateDouble(pose->rot.y.y));
	rot->SetValue(5, CefV8Value::CreateDouble(pose->rot.y.z));
	rot->SetValue(6, CefV8Value::CreateDouble(pose->rot.z.x));
	rot->SetValue(7, CefV8Value::CreateDouble(pose->rot.z.y));
	rot->SetValue(8, CefV8Value::CreateDouble(pose->rot.z.z));

	retval->SetValue("pos", pos, V8_PROPERTY_ATTRIBUTE_READONLY);
	retval->SetValue("scale", scale, V8_PROPERTY_ATTRIBUTE_READONLY);
	retval->SetValue("rot", rot, V8_PROPERTY_ATTRIBUTE_READONLY);
}
inline void wrap_result(const MoverFitsAtResult& mfat, CefRefPtr<CefV8Value>& retval)
{
	retval = CefV8Value::CreateObject(nullptr, nullptr);
	CefRefPtr<CefV8Value> pos = CefV8Value::CreateArray(3);

	pos->SetValue(0, CefV8Value::CreateDouble(mfat.pos.x));
	pos->SetValue(1, CefV8Value::CreateDouble(mfat.pos.y));
	pos->SetValue(2, CefV8Value::CreateDouble(mfat.pos.z));

	retval->SetValue("fits", CefV8Value::CreateInt(mfat.fits), V8_PROPERTY_ATTRIBUTE_READONLY);
	retval->SetValue("pos", pos, V8_PROPERTY_ATTRIBUTE_READONLY);
}
inline void wrap_result(const BoneNamesWrapper& names, CefRefPtr<CefV8Value>& retval)
{
	retval = CefV8Value::CreateArray(names.num_bones);
	const char* bone_name = names.bone_names_list;
	for (unsigned i = 0; i < names.num_bones; ++i) {
		retval->SetValue(i, CefV8Value::CreateString(bone_name));
		bone_name = names.bone_names_list + strlen(bone_name) + 1;
	}
}
inline void wrap_result(const AnimationStates& states, CefRefPtr<CefV8Value>& retval)
{
	retval = CefV8Value::CreateArray(states.num_states);
	for (unsigned i = 0; i < states.num_states; ++i) {
		retval->SetValue(i, CefV8Value::CreateUInt(states.states[i]));
	}
}
inline void wrap_result(const AnimationLayerInfo& ali, CefRefPtr<CefV8Value>& retval)
{
	retval = CefV8Value::CreateObject(nullptr, nullptr);
	retval->SetValue("length", CefV8Value::CreateDouble(ali.length), V8_PROPERTY_ATTRIBUTE_READONLY);
	retval->SetValue("time", CefV8Value::CreateDouble(ali.t), V8_PROPERTY_ATTRIBUTE_READONLY);
}
inline void wrap_result(const AnimationLayerSeeds& als, CefRefPtr<CefV8Value>& retval)
{
	retval = CefV8Value::CreateArray(als.num_seeds);
	for (unsigned i = 0; i < als.num_seeds; ++i) {
		retval->SetValue(i, CefV8Value::CreateUInt(als.seeds[i]));
	}
}
inline void wrap_result(const OOBBWrapper& oobbw, CefRefPtr<CefV8Value>& retval)
{
	retval = CefV8Value::CreateObject(nullptr, nullptr);
	CefRefPtr<CefV8Value> tm = CefV8Value::CreateArray(16);
	CefRefPtr<CefV8Value> half_ext = CefV8Value::CreateArray(3);
	for (unsigned i = 0; i < 16; ++i) {
		tm->SetValue(i, CefV8Value::CreateDouble(oobbw.tm[i]));
	}
	for (unsigned i = 0; i < 3; ++i) {
		half_ext->SetValue(i, CefV8Value::CreateDouble(oobbw.half_ext[i]));
	}

	retval->SetValue("tm", tm, V8_PROPERTY_ATTRIBUTE_READONLY);
	retval->SetValue("half_ext", half_ext, V8_PROPERTY_ATTRIBUTE_READONLY);
}
inline void wrap_result(const TimeStepPolicyWrapper& tspw, CefRefPtr<CefV8Value>& retval)
{
	retval = CefV8Value::CreateObject(nullptr, nullptr);
	retval->SetValue("type", CefV8Value::CreateInt(tspw.type), V8_PROPERTY_ATTRIBUTE_READONLY);
	retval->SetValue("frames", CefV8Value::CreateInt(tspw.frames), V8_PROPERTY_ATTRIBUTE_READONLY);
	retval->SetValue("fps", CefV8Value::CreateInt(tspw.fps), V8_PROPERTY_ATTRIBUTE_READONLY);
	retval->SetValue("multiplier", CefV8Value::CreateDouble(tspw.multiplier), V8_PROPERTY_ATTRIBUTE_READONLY);
	retval->SetValue("time", CefV8Value::CreateDouble(tspw.time), V8_PROPERTY_ATTRIBUTE_READONLY);
	retval->SetValue("min", CefV8Value::CreateDouble(tspw.min), V8_PROPERTY_ATTRIBUTE_READONLY);
	retval->SetValue("outliers", CefV8Value::CreateInt(tspw.outliers), V8_PROPERTY_ATTRIBUTE_READONLY);
	retval->SetValue("max", CefV8Value::CreateDouble(tspw.max), V8_PROPERTY_ATTRIBUTE_READONLY);
	retval->SetValue("lerp", CefV8Value::CreateDouble(tspw.lerp), V8_PROPERTY_ATTRIBUTE_READONLY);
}
inline void wrap_result(const WindowRectWrapper& wrw, CefRefPtr<CefV8Value>& retval)
{
	retval = CefV8Value::CreateArray(4);
	retval->SetValue(0, CefV8Value::CreateInt(wrw.pos[0]));
	retval->SetValue(1, CefV8Value::CreateInt(wrw.pos[1]));
	retval->SetValue(2, CefV8Value::CreateInt(wrw.pos[2]));
	retval->SetValue(3, CefV8Value::CreateInt(wrw.pos[3]));
}
inline void wrap_result(const WindowOpenParameter& wop, CefRefPtr<CefV8Value>& retval)
{
	retval = CefV8Value::CreateObject(nullptr, nullptr);
	retval->SetValue("x", CefV8Value::CreateInt(wop.x), V8_PROPERTY_ATTRIBUTE_DONTDELETE);
	retval->SetValue("y", CefV8Value::CreateInt(wop.y), V8_PROPERTY_ATTRIBUTE_DONTDELETE);
	retval->SetValue("width", CefV8Value::CreateInt(wop.width), V8_PROPERTY_ATTRIBUTE_DONTDELETE);
	retval->SetValue("height", CefV8Value::CreateInt(wop.height), V8_PROPERTY_ATTRIBUTE_DONTDELETE);
	retval->SetValue("title", CefV8Value::CreateString(wop.optional_title ? wop.optional_title : ""), V8_PROPERTY_ATTRIBUTE_DONTDELETE);
	retval->SetValue("parent", UserObject::CreateObjectPtr(wop.optional_parent), V8_PROPERTY_ATTRIBUTE_DONTDELETE);
	retval->SetValue("explicit_resize", CefV8Value::CreateBool(wop.explicit_resize != 0), V8_PROPERTY_ATTRIBUTE_DONTDELETE);
	retval->SetValue("main_window", CefV8Value::CreateBool(wop.main_window != 0), V8_PROPERTY_ATTRIBUTE_DONTDELETE);
	retval->SetValue("visible", CefV8Value::CreateBool(wop.visible != 0), V8_PROPERTY_ATTRIBUTE_DONTDELETE);
	retval->SetValue("pass_key_events_to_parent", CefV8Value::CreateBool(wop.pass_key_events_to_parent != 0), V8_PROPERTY_ATTRIBUTE_DONTDELETE);
	retval->SetValue("layered", CefV8Value::CreateBool(wop.layered != 0), V8_PROPERTY_ATTRIBUTE_DONTDELETE);
}
inline CefRefPtr<CefV8Value> wrap_result(const DynamicScriptDataItem& result, CefRefPtr<CefV8Value>& retval)
{
	if (result.type == D_DATA_NIL_TYPE)
		retval = CefV8Value::CreateUndefined();
	else if (result.type == D_DATA_BOOLEAN_TYPE)
		retval = CefV8Value::CreateBool(*(int*)result.pointer != 0);
	else if (result.type == D_DATA_NUMBER_TYPE)
		retval = CefV8Value::CreateDouble(*(float*)result.pointer);
	else if (result.type == D_DATA_STRING_TYPE)
		retval = CefV8Value::CreateString((const char*)result.pointer);
	else if (result.type == D_DATA_CUSTOM_TVECTOR2) {
		Vector2 v = *(Vector2*)result.pointer;
		auto a2 = CefV8Value::CreateArray(2);
		a2->SetValue(0, CefV8Value::CreateDouble(v.x));
		a2->SetValue(1, CefV8Value::CreateDouble(v.y));
		retval = a2;
	} else if (result.type == D_DATA_CUSTOM_TVECTOR3) {
		Vector3 v = *(Vector3*)result.pointer;
		auto a3 = CefV8Value::CreateArray(3);
		a3->SetValue(0, CefV8Value::CreateDouble(v.x));
		a3->SetValue(1, CefV8Value::CreateDouble(v.y));
		a3->SetValue(2, CefV8Value::CreateDouble(v.z));
		retval = a3;
	} else if (result.type == D_DATA_CUSTOM_TVECTOR4) {
		Vector4 v = *(Vector4*)result.pointer;
		auto a4 = CefV8Value::CreateArray(4);
		a4->SetValue(0, CefV8Value::CreateDouble(v.x));
		a4->SetValue(1, CefV8Value::CreateDouble(v.y));
		a4->SetValue(2, CefV8Value::CreateDouble(v.z));
		a4->SetValue(3, CefV8Value::CreateDouble(v.w));
		retval = a4;
	} else if (result.type == D_DATA_CUSTOM_TMATRIX4X4) {
		Matrix4x4 m = *(Matrix4x4*)result.pointer;
		auto arr = CefV8Value::CreateArray(16);
		for (int i = 0; i < 16; ++i)
			arr->SetValue(i, CefV8Value::CreateDouble(m.v[i]));
		retval = arr;
	} else if (result.type == D_DATA_CUSTOM_TUNITREFERENCE)
		retval = CefV8Value::CreateUInt(*(unsigned*)result.pointer);
	else if (result.type == D_DATA_CUSTOM_TPOINTER) {
		retval = UserObject::CreateObjectData(result.pointer, result.size);
	} else if (result.type == D_DATA_CUSTOM_ID64) {
		retval = UserObject::CreateId(*(uint64_t*)result.pointer);
	} else
		throw std::exception("Dynamic data type not supported");

	return retval;
}
inline CefRefPtr<CefV8Value> wrap_result(const Vector3ArrayWrapper& v3aw, CefRefPtr<CefV8Value>& retval)
{
	retval = CefV8Value::CreateArray(v3aw.count);
	for (int i = 0; i < v3aw.count; ++i) {
		CefRefPtr<CefV8Value> v;
		wrap_result(v3aw.v[i], v);
		retval->SetValue(i, v);
	}
	return retval;
}
inline CefRefPtr<CefV8Value> wrap_result(const DeadZoneSetting& dzs, CefRefPtr<CefV8Value>& retval)
{
	retval = CefV8Value::CreateObject(nullptr, nullptr);
	retval->SetValue("mode", CefV8Value::CreateInt(dzs.mode), V8_PROPERTY_ATTRIBUTE_DONTDELETE);
	retval->SetValue("size", CefV8Value::CreateDouble(dzs.size), V8_PROPERTY_ATTRIBUTE_DONTDELETE);
	return retval;
}

// Return void

inline void call_f(void(*f)(), const CefV8ValueList&, CefRefPtr<CefV8Value>&) {
	f();
}
template<typename P0>
void call_f(void(*f)(P0), const CefV8ValueList& args, CefRefPtr<CefV8Value>&) {
	f(get_arg<P0>(args, 0));
}
template<typename P0, typename P1>
void call_f(void(*f)(P0, P1), const CefV8ValueList& args, CefRefPtr<CefV8Value>&) {
	f(get_arg<P0>(args, 0), get_arg<P1>(args, 1));
}
template<typename P0, typename P1, typename P2>
void call_f(void(*f)(P0, P1, P2), const CefV8ValueList& args, CefRefPtr<CefV8Value>&) {
	f(get_arg<P0>(args, 0), get_arg<P1>(args, 1), get_arg<P2>(args, 2));
}
template<typename P0, typename P1, typename P2, typename P3>
void call_f(void(*f)(P0,P1,P2,P3), const CefV8ValueList& args, CefRefPtr<CefV8Value>&) {
	f(get_arg<P0>(args, 0), get_arg<P1>(args, 1), get_arg<P2>(args, 2), get_arg<P3>(args, 3));
}
template<typename P0, typename P1, typename P2, typename P3, typename P4>
void call_f(void(*f)(P0,P1,P2,P3,P4), const CefV8ValueList& args, CefRefPtr<CefV8Value>&) {
	f(get_arg<P0>(args, 0), get_arg<P1>(args, 1), get_arg<P2>(args, 2), get_arg<P3>(args, 3), get_arg<P4>(args, 4));
}
template<typename P0, typename P1, typename P2, typename P3, typename P4, typename P5>
void call_f(void(*f)(P0,P1,P2,P3,P4,P5), const CefV8ValueList& args, CefRefPtr<CefV8Value>&) {
	f(get_arg<P0>(args, 0), get_arg<P1>(args, 1), get_arg<P2>(args, 2), get_arg<P3>(args, 3), get_arg<P4>(args, 4), get_arg<P5>(args, 5));
}
template<typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
void call_f(void(*f)(P0, P1, P2, P3, P4, P5, P6), const CefV8ValueList& args, CefRefPtr<CefV8Value>&)
{
	f(get_arg<P0>(args, 0), get_arg<P1>(args, 1), get_arg<P2>(args, 2), get_arg<P3>(args, 3), get_arg<P4>(args, 4), get_arg<P5>(args, 5), get_arg<P6>(args, 6));
}

// Return result

template<typename R>
void call_f(R(*f)(), const CefV8ValueList&, CefRefPtr<CefV8Value>& retval) {
	wrap_result(f(), retval);
}
template<typename R, typename P0>
void call_f(R(*f)(P0), const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval) {
	wrap_result(f(get_arg<P0>(args, 0)), retval);
}
template<typename R, typename P0, typename P1>
void call_f(R(*f)(P0,P1), const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval) {
	wrap_result(f(get_arg<P0>(args, 0), get_arg<P1>(args, 1)), retval);
}
template<typename R, typename P0, typename P1, typename P2>
void call_f(R(*f)(P0,P1,P2), const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval) {
	wrap_result(f(get_arg<P0>(args, 0), get_arg<P1>(args, 1), get_arg<P2>(args, 2)), retval);
}
template<typename R, typename P0, typename P1, typename P2, typename P3>
void call_f(R(*f)(P0,P1,P2,P3), const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval) {
	wrap_result(f(get_arg<P0>(args, 0), get_arg<P1>(args, 1), get_arg<P2>(args, 2), get_arg<P3>(args, 3)), retval);
}
template<typename R, typename P0, typename P1, typename P2, typename P3, typename P4>
void call_f(R(*f)(P0,P1,P2,P3,P4), const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval) {
	wrap_result(f(get_arg<P0>(args, 0), get_arg<P1>(args, 1), get_arg<P2>(args, 2), get_arg<P3>(args, 3), get_arg<P4>(args, 4)), retval);
}
template<typename R, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5>
void call_f(R(*f)(P0,P1,P2,P3,P4,P5), const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval) {
	wrap_result(f(
		get_arg<P0>(args, 0), get_arg<P1>(args, 1), get_arg<P2>(args, 2),
		get_arg<P3>(args, 3), get_arg<P4>(args, 4), get_arg<P5>(args, 5)), retval);
}
template<typename R, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
void call_f(R(*f)(P0, P1, P2, P3, P4, P5, P6), const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval)
{
	wrap_result(f(
		get_arg<P0>(args, 0), get_arg<P1>(args, 1), get_arg<P2>(args, 2),
		get_arg<P3>(args, 3), get_arg<P4>(args, 4), get_arg<P5>(args, 5),
		get_arg<P6>(args, 6)), retval);
}
template<typename R, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
void call_f(R(*f)(P0, P1, P2, P3, P4, P5, P6, P7), const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval)
{
	wrap_result(f(
		get_arg<P0>(args, 0), get_arg<P1>(args, 1), get_arg<P2>(args, 2),
		get_arg<P3>(args, 3), get_arg<P4>(args, 4), get_arg<P5>(args, 5),
		get_arg<P6>(args, 6), get_arg<P7>(args, 7)), retval);
}
template<typename R, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
void call_f(R(*f)(P0, P1, P2, P3, P4, P5, P6, P7, P8), const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval)
{
	wrap_result(f(
		get_arg<P0>(args, 0), get_arg<P1>(args, 1), get_arg<P2>(args, 2),
		get_arg<P3>(args, 3), get_arg<P4>(args, 4), get_arg<P5>(args, 5),
		get_arg<P6>(args, 6), get_arg<P7>(args, 7), get_arg<P8>(args, 8)), retval);
}

// Used to process C-API that returns a list of items.
template<typename R> void call_f(R(*f)(unsigned*), const CefV8ValueList&, CefRefPtr<CefV8Value>& retval)
{
	unsigned n = 0;
	R items = f(&n);
	retval = CefV8Value::CreateArray(n);
	for (unsigned i = 0; i < n; ++i) 	{
		CefRefPtr<CefV8Value> wrapped_item;
		wrap_result(items[i], wrapped_item);
		retval->SetValue(i, wrapped_item);
	}
}

// API CEF HANDLER
//

template<typename F> void create_handler(CefRefPtr<CefV8Value>& ns, const CefString& name, F func)
{
	class StingrayAPIHandler : public CefV8Handler
	{
	public:
		StingrayAPIHandler(F func) : _func(func) {}

		typedef F callback_handler_type;

		bool Execute(
			const CefString& name,
			CefRefPtr<CefV8Value>,
			const CefV8ValueList& arguments,
			CefRefPtr<CefV8Value>& retval,
			CefString& exception) OVERRIDE
		{
			//if (name == "spawn_unit") DebugBreak();
			try {
				call_f(_func, arguments, retval);
			} catch (std::exception& ex) {
				exception = stingray::api::error->eprintf("Failed to execute %s.\r\n%s", name.ToString().c_str(), ex.what());
			}
			return true;
		}

	private:

		F _func;
		IMPLEMENT_REFCOUNTING(StingrayAPIHandler);
	};

	auto handler = new StingrayAPIHandler(func);
	ns->SetValue(name, CefV8Value::CreateFunction(name, handler), V8_PROPERTY_ATTRIBUTE_READONLY);
}

/**
 * Bind a custom function to namespace.
 */
typedef std::function<CefRefPtr<CefV8Value>(const CefV8ValueList&)> ExecuteHandlerFunction;
inline void bind_api(CefRefPtr<CefV8Value>& ns, const CefString& name, ExecuteHandlerFunction func)
{
	class ExecuteHandler : public CefV8Handler
	{
	public:
		ExecuteHandler(ExecuteHandlerFunction func) : _handler(func) {}

		bool Execute(
			const CefString& name,
			CefRefPtr<CefV8Value> self,
			const CefV8ValueList& arguments,
			CefRefPtr<CefV8Value>& retval,
			CefString& exception) OVERRIDE
		{
			try {
				retval = _handler(arguments);
			} catch (std::exception& ex) {
				exception = stingray::api::error->eprintf("Failed to execute %s.\r\n%s", name.ToString().c_str(), ex.what());
			}
			return true;
		}

	private:

		ExecuteHandlerFunction _handler;
		IMPLEMENT_REFCOUNTING(ExecuteHandler);
	};

	auto handler = new ExecuteHandler(func);
	ns->SetValue(name, CefV8Value::CreateFunction(name, handler), V8_PROPERTY_ATTRIBUTE_READONLY);
}

/**
 * Bind C-API to namespace.
 */
#define DEFINE_API(name) \
	CefRefPtr<CefV8Value> ns = CefV8Value::CreateObject(nullptr, nullptr); \
	stingray_ns->SetValue(name, ns, V8_PROPERTY_ATTRIBUTE_READONLY)
#define BIND_API(NAME) create_handler(ns, #NAME, api->NAME);

} // end namespace
