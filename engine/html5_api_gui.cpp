#include "html5_api_bindings.h"
#include "html5_api_dynamic_data.inl"

namespace PLUGIN_NAMESPACE {

void bind_api_gui(CefRefPtr<CefV8Value> stingray_ns, const GuiCApi* api)
{
	DEFINE_API("Gui");

	BIND_API(material);
	BIND_API(create_material);

	// Rect support
	BIND_API(rect);
	BIND_API(update_rect);
	BIND_API(destroy_rect);

	// Bitmap support
	//
	BIND_API(bitmap);
	BIND_API(destroy_bitmap);
	bind_api(ns, "update_bitmap", [](const CefV8ValueList& args)
	{
		stingray::api::script->Gui->update_bitmap(
			get_arg<GuiPtr>(args,0),
			get_arg<unsigned>(args,1),
			get_arg<MaterialPtr>(args,2),
			get_arg<ConstVector2Ptr>(args,3),
			get_arg<unsigned>(args,4),
			get_arg<ConstVector2Ptr>(args,5),
			get_arg<ConstVector4Ptr>(args,6),
			get_arg<ConstVector2Ptr>(args,7),
			get_arg<ConstVector2Ptr>(args,8) );
		return CefV8Value::CreateUndefined();
	});
	bind_api(ns, "resolution", [](const CefV8ValueList &args)
	{
		unsigned int out_width;
		unsigned int out_height;
		stingray::api::script->Gui->resolution( get_arg<ViewportPtr>(args,0),
												get_arg<ConstWindowPtr>(args,1),
												&out_width, &out_height );
		CefRefPtr<CefV8Value> retval = CefV8Value::CreateArray(2);
		retval->SetValue(0, CefV8Value::CreateUInt(out_width));
		retval->SetValue(1, CefV8Value::CreateUInt(out_height));
		return retval;
	});

#if 0 // TODO: incrementally add more apis.
	BIND_API(triangle);
	BIND_API(update_triangle);
	BIND_API(destroy_triangle);

	BIND_API(rect_3d);
	BIND_API(update_rect_3d);
	BIND_API(destroy_rect_3d);

	BIND_API(bitmap_3d);
	BIND_API(update_bitmap_3d);
	BIND_API(destroy_bitmap_3d);

	BIND_API(text);
	BIND_API(update_text);
	BIND_API(destroy_text);

	BIND_API(text_3d);
	BIND_API(update_text_3d);
	BIND_API(destroy_text_3d);

	BIND_API(text_extents);

	BIND_API(word_wrap);

	BIND_API(video);
	BIND_API(update_video);
	BIND_API(destroy_video);

	BIND_API(video_3d);
	BIND_API(update_video_3d);
	BIND_API(destroy_video_3d);

	BIND_API(set_visible);
	BIND_API(is_visible);

	BIND_API(has_all_glyphs);
	BIND_API(move);
	BIND_API(move_3d);
	BIND_API(reset);

	BIND_API(resolution);

	BIND_API(color_rgb);
	BIND_API(color_argb);

	BIND_API(get_id);

	BIND_API(set_video_playback_speed);
	BIND_API(set_video_loop);
	BIND_API(video_has_audio);

	BIND_API(video_sound_stream_source);
	// TODO: BIND_API(set_video_sound_stream_enable);

	BIND_API(video_number_of_frames);
	BIND_API(video_current_frame);
	BIND_API(video_times_looped);

	#if defined(DEVELOPMENT)
		BIND_API(texture_size);
		BIND_API(thumbnail_load_texture);
		BIND_API(thumbnail_load_dds);
		BIND_API(thumbnail_unload);
	#endif

#endif
}

} // end namespace
