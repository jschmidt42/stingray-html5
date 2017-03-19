# Check for root directory existence. Can be overwritten with environment variable SR_<LIBRARY NAME>_ROOT.
find_package_root(CEF_ROOT cef)

# Set definitions
set(CEF_DEFINITIONS -DCEF_LIB)

# Set library directories
set(CEF_BASE_DIR "${CEF_ROOT}/x${ARCH_BITS}/$<$<CONFIG:DEBUG>:debug>$<$<NOT:$<CONFIG:DEBUG>>:release>")

# Set include directories
set(CEF_INCLUDE_DIRS "${CEF_ROOT}")

# Set resources directory
set(CEF_RESOURCES_DIR "${CEF_ROOT}/Resources")

# Set libraries
set(CEF_LIBRARIES
	"${CEF_BASE_DIR}/libcef${LIB_SUFFIX}"
	"${CEF_BASE_DIR}/libcef_dll_wrapper${LIB_SUFFIX}"
)

# Set binaries
# HACK: There are at least 2 assertions when using debug cef .dlls that need to be fixed
# ... using release CEF .dlls as a workaround until those assertions are fixed (same as done before using cmake)
# ... note that in debug we still link with the debug libs even though we are using release .dll (this is not ideal, but how it was setup before cmake as workaround)
set(CEF_BASE_DIR_RELEASE "${CEF_ROOT}/x${ARCH_BITS}/release")
set(CEF_BINARIES
	#"${CEF_BASE_DIR_RELEASE}/d3dcompiler_43.dll"
	"${CEF_BASE_DIR_RELEASE}/d3dcompiler_47.dll"
	"${CEF_BASE_DIR_RELEASE}/libcef.dll"
	"${CEF_BASE_DIR_RELEASE}/libEGL.dll"
	"${CEF_BASE_DIR_RELEASE}/chrome_elf.dll"
	"${CEF_BASE_DIR_RELEASE}/libGLESv2.dll"
	"${CEF_BASE_DIR_RELEASE}/icudtl.dat"
	"${CEF_BASE_DIR_RELEASE}/natives_blob.bin"
	"${CEF_BASE_DIR_RELEASE}/snapshot_blob.bin"
)

# Check if package setup is successful
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CEF DEFAULT_MSG CEF_ROOT CEF_INCLUDE_DIRS CEF_LIBRARIES)
mark_as_advanced(CEF_INCLUDE_DIRS CEF_LIBRARIES)
