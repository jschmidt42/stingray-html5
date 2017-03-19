#pragma once

#include "shared_ptr.h"

#include <engine_plugin_api/plugin_api.h>

#include <plugin_foundation/allocator.h>
#include <plugin_foundation/string.h>
#include <plugin_foundation/vector.h>

#include <include/cef_request.h>
#include <include/cef_scheme.h>

namespace PLUGIN_NAMESPACE {

	using namespace stingray_plugin_foundation;

	struct WebResource
	{
		ALLOCATOR_AWARE;

		explicit WebResource(Allocator& a)
			: allocator(a), hash(0), url(a), path(a), mime_type(a), buffer(nullptr), size(0)
		{
		}

		WebResource(const WebResource& wpf)
			: allocator(wpf.allocator), hash(wpf.hash), url(wpf.url), path(wpf.path), mime_type(wpf.mime_type), buffer(wpf.buffer), size(wpf.size)
		{
		}

		WebResource& operator=(const WebResource& wpf)
		{
			if (this != &wpf) {
				this->~WebResource();
				new (this) WebResource(wpf);
			}
			return *this;
		}

		Allocator& allocator;
		unsigned hash;
		DynamicString url;
		DynamicString path;
		DynamicString mime_type;
		uint8_t* buffer;
		uint32_t size;
	};

	typedef shared_ptr<WebResource> WebResourcePtr;

	struct WebPage
	{
		ALLOCATOR_AWARE;

		explicit WebPage(Allocator& a)
			: allocator(a), name(a), resources(a), data(nullptr)
		{
		}

		Allocator& allocator;

		// Page resource name
		DynamicString name;

		// All files in package
		Vector<WebResourcePtr> resources;

		// The entire content of the web page folder will be returned in a buffer with the following layout :
		// 
		// [number_of_files]
		// [offset_to_path_1] [size_of_path_1] [offset_to_file_data_1] [size_of_file_data_1]
		// [offset_to_path_2] [size_of_path_2] [offset_to_file_data_2] [size_of_file_data_2]
		// ...
		// [offset_to_path_n][size_of_path_n][offset_to_file_data_n][size_of_file_data_n]
		// [buffer_data]
		uint8_t* data;
	};

	typedef shared_ptr<WebPage> WebPagePtr;

	void setup_web_page_database();

	void shutdown_web_page_database();

	void setup_web_page_resources(GetApiFunction get_engine_api);

	void setup_web_page_compiler(GetApiFunction get_engine_api);

	DynamicString get_mime_type(const char* extension);

	WebResourcePtr find_web_resource(const char* url);

	class WebPageSchemeHandlerFactory : public CefSchemeHandlerFactory {
	public:
		CefRefPtr<CefResourceHandler> Create(
			CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefFrame> frame,
			const CefString& scheme_name,
			CefRefPtr<CefRequest> request) OVERRIDE;

		IMPLEMENT_REFCOUNTING(WebPageSchemeHandlerFactory)
	};

} // end namespace
