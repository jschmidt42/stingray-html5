#include "html5_web_page.h"

#include "stingray_api.h"

#include <plugin_foundation/id_string.h>
#include <plugin_foundation/string.h>
#include <plugin_foundation/vector.h>
#include <plugin_foundation/hash_map.h>

#include <include/cef_parser.h>
#include <include/wrapper/cef_stream_resource_handler.h>

namespace PLUGIN_NAMESPACE {

// Data compiler resource properties
int WEB_PAGE_RESOURCE_VERSION = 14;
const char WEB_PAGE_RESOURCE_EXTENSION[] = "html5";
const IdString32 WEB_PAGE_RESOURCE_ID = IdString32(WEB_PAGE_RESOURCE_EXTENSION);

struct WebPageDatabase
{
	ALLOCATOR_AWARE;

	WebPageDatabase(Allocator& a)
		: allocator(a)
		, pages(a)
	{}

	WebPagePtr find_page(const char* page_name)
	{
		for (unsigned i = 0; i < pages.size(); ++i) {
			if (pages[i]->name == page_name) {
				return pages[i];
			}
		}
		return nullptr;
	}

	WebPagePtr load_page(const char* page_name)
	{
		WebPagePtr page = MAKE_NEW(allocator, WebPage, allocator);

		page->name = page_name;
		page->data = (uint8_t*)stingray::api::resource_manager->get(WEB_PAGE_RESOURCE_EXTENSION, page_name);
		const unsigned int* index = reinterpret_cast<const unsigned int*>(page->data);
		unsigned index_size = index[0];
		for (size_t i = 1; i < index_size; i += 4) {
			unsigned path_offset = index[i];
			unsigned path_size = index[i + 1];
			unsigned file_offset = index[i + 2];
			unsigned file_size = index[i + 3];

			WebResourcePtr res(MAKE_NEW(allocator, WebResource, allocator));
			res->path = DynamicString(allocator, (const char*)page->data + path_offset, path_size);

			DynamicString extension = path::get_extension(res->path.c_str(), allocator);
			res->mime_type = get_mime_type(extension.c_str());
			res->size = file_size;
			res->buffer = page->data + file_offset;

			page->resources.push_back(res);
		}

		pages.push_back(page);

		return page;
	}

	WebResourcePtr load_resource(const char* url)
	{
		CefURLParts url_parts;
		CefParseURL(url, url_parts);

		DynamicString page_name(allocator, CefString(&url_parts.host).ToString().c_str());

		unsigned current_path_index = 0;
		Vector<DynamicString> paths(allocator);
		string::split(CefString(&url_parts.path).ToString().c_str(), "/", paths);

		while (true) {
			if (stingray::api::resource_manager->can_get(WEB_PAGE_RESOURCE_EXTENSION, page_name.c_str())) {
				WebPagePtr page = find_page(page_name.c_str());
				if (!page) {
					page = load_page(page_name.c_str());
				}

				if (!page)
					return nullptr;

				unsigned url_hash = hash32(url);
				for (unsigned i = 0; i < page->resources.size(); ++i) {
					if (page->resources[i]->hash == url_hash) {
						return page->resources[i];
					}
				}

				DynamicString resource_path(allocator);
				string::join(&paths[current_path_index], paths.size() - current_path_index, "/", resource_path);

				for (unsigned i = 0; i < page->resources.size(); ++i) {
					if (page->resources[i]->path == resource_path) {
						page->resources[i]->hash = hash32(url);
						page->resources[i]->url = url;
						return page->resources[i];
					}
				}

				return nullptr;
			}

			if (current_path_index >= paths.size())
				break;

			append(page_name, "/");
			append(page_name, paths[current_path_index++].c_str());
		}

		return nullptr;
	}

	WebResourcePtr get(const char* url)
	{
		return load_resource(url);
	}

	Allocator& allocator;
	Vector<WebPagePtr> pages;

	static WebPageDatabase* instance;
};

WebPageDatabase* WebPageDatabase::instance = nullptr;

/**
 * Define plugin resource compiler.
 */
DataCompileResult web_page_compiler(DataCompileParameters *input)
{
	return stingray::api::data_compile_params->read_file_folder(input);
}

void setup_web_page_database()
{
	WebPageDatabase::instance = MAKE_NEW(allocator, WebPageDatabase, allocator);
}

void shutdown_web_page_database()
{
	MAKE_DELETE_TYPE(WebPageDatabase::instance->allocator, WebPageDatabase, WebPageDatabase::instance);
}

/**
 * Indicate to the resource manager that we'll be using our plugin resource type.
 */
void setup_web_page_resources(GetApiFunction get_engine_api)
{
	stingray::api::resource_manager->register_type(WEB_PAGE_RESOURCE_EXTENSION);
}

/**
 * Setup the HTML5 page folder compiler.
 */
void setup_web_page_compiler(GetApiFunction get_engine_api)
{
	stingray::api::data_compiler->add_compiler(WEB_PAGE_RESOURCE_EXTENSION, WEB_PAGE_RESOURCE_VERSION, web_page_compiler);
}

DynamicString get_mime_type(const char* extension)
{
	HKEY key = nullptr;
	DynamicString mime_type(allocator, "application/unknown");

	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, extension, 0, KEY_READ, &key) == ERROR_SUCCESS) {
		char content_type_buffer[256] = { 0 };
		DWORD buf_size = sizeof content_type_buffer;

		// Get content type
		if (RegQueryValueEx(key, "Content Type", nullptr, nullptr, (LPBYTE)content_type_buffer, &buf_size) == ERROR_SUCCESS)
			mime_type = content_type_buffer;

		RegCloseKey(key);
	}

	return mime_type;
}

WebResourcePtr find_web_resource(const char* url)
{
	return WebPageDatabase::instance->get(url);
}

CefRefPtr<CefResourceHandler> WebPageSchemeHandlerFactory::Create(
	CefRefPtr<CefBrowser>, CefRefPtr<CefFrame>, const CefString&, CefRefPtr<CefRequest> request)
{
	auto resource = find_web_resource(request->GetURL().ToString().c_str());
	if (!resource)
		return nullptr;

	// Create a stream reader for the web page resource.
	CefRefPtr<CefStreamReader> stream = CefStreamReader::CreateForData(resource->buffer, resource->size);
	return new CefStreamResourceHandler(resource->mime_type.c_str(), stream);
}

} // end namespace
