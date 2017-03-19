#include "html5_api.h"
#include "stingray_api.h"

#include <include/cef_app.h>

namespace PLUGIN_NAMESPACE {

void bind_api(CefRefPtr<CefV8Value> stingray_ns)
{
	bind_api_fs(stingray_ns);
	bind_api_host(stingray_ns);
	bind_api_web_app(stingray_ns);
	bind_api_web_view(stingray_ns);
	bind_api_math(stingray_ns);
	bind_api_application(stingray_ns, stingray::api::script->Application);
	bind_api_world(stingray_ns, stingray::api::script->World);
	bind_api_input(stingray_ns, stingray::api::script->Input);
	bind_api_unit(stingray_ns, stingray::api::script->Unit);
	bind_api_camera(stingray_ns, stingray::api::script->Camera);
	bind_api_window(stingray_ns, stingray::api::script->Window);
	bind_api_level(stingray_ns, stingray::api::script->Level);

	/* TODO
	 struct DynamicScriptDataCApi* DynamicScriptData;

	// C- API
	struct MaterialCApi* Material;
	struct LanCApi* Lan;
	struct NetworkCApi* Network;
	struct GameSessionCApi* GameSession;
	struct UnitSynchronizerCApi* UnitSynchronizer;
	struct UtilitiesCApi* Utilities;
	struct EntityCApi* Entity;
	struct LineObjectCApi* LineObject;
	struct GuiCApi* Gui;
	struct PhysicsWorldCApi* PhysicsWorld;
	struct ActorCApi* Actor;
	struct MoverCApi* Mover;
	struct SaveSystemCApi* SaveSystem;
	struct ViewportCApi* Viewport;
	struct MeshCApi* Mesh;

	// Engine API
	RESOURCE_MANAGER_API_ID =			5,  // ResourceManagerApi
	LOGGING_API_ID =					6,  // LoggingApi
	UNIT_API_ID =						8,  // UnitApi
	SCENE_GRAPH_API_ID =				9,  // SceneGraphApi
	APPLICATION_API_ID =				12, // ApplicationApi
	APPLICATION_OPTIONS_API_ID =		13, // ApplicationOptionsApi
	UNIT_REFERENCE_API_ID =				14, // UnitReferenceApi
	RAYCAST_API_ID =					17, // RaycastApi
	RENDER_INTERFACE_API =              18, //  RenderInterfaceApi
	WORLD_API_ID =						22, // Line drawer?
	LINE_OBJECT_DRAWER_API_ID =			23, // LineObjectDrawerApi
	PROFILER_API_ID =					24, // ProfilerApi
	ERROR_API_ID =						25, // ErrorApi
	RENDER_BUFFER_API_ID =				26, // RenderBufferApi
	MESH_API_ID =						27, // MeshObjectApi
	RENDER_SCENE_GRAPH_API_ID =			29, // RenderSceneGraphApi
	TIMER_API_ID =						33, // TimerApi
	STREAM_CAPTURE_API_ID =				36, // StreamCaptureApi
	FLOW_NODES_API_ID =					37, // FlowNodesApi
	CAMERA_API_ID =						38, // CameraApi

	*/
}

} // end namespace
