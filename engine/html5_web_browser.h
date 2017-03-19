#pragma once
#include <plugin_foundation/vector3.h>

struct CApiUnit;

namespace PLUGIN_NAMESPACE { namespace browser {

using namespace stingray_plugin_foundation;

// Initialize the browser module.
void init();

// Release browser module resources.
void shutdown();

// Check if unit has an browser url, if yes load web view, otherwise continue.
bool try_load(CApiUnit* unit);

// Check if unit has an browser url, if yes unload web view, otherwise continue.
bool try_unload(CApiUnit* unit);

// Try to pick any web browser.
void pick(const Vector3& mouse_pos, const Vector3& from, const Vector3& ray);

}} // end namespace
