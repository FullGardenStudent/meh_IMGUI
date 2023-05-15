// Author : FullGardenStudent
// Description:
#ifndef MEH_VULKAN_FUNCTIONS_H
#define MEH_VULKAN_FUNCTIONS_H 1

#include <vector>

// #define VK_DEFINE_NON_DISPATCHABLE_HANDLE
// #if defined(_WIN32)

// no matter what platform, always use function pointers and load vulkan runtime
#ifndef VK_NO_PROTOTYPES
#define VK_NO_PROTOTYPES 1
#endif

// include vulkan shit for Windows
#ifdef _WIN32
#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR 1
#endif
#include "vulkan/vulkan.h"

#define LIBRARY_TYPE HMODULE
#define LoadFunction GetProcAddress

#endif // _WIN32

// vulkan includes for linux
#ifdef __linux
#ifndef VK_USE_PLATFORM_WAYLAND_KHR
#define VK_USE_PLATFORM_WAYLAND_KHR 1
#endif
#include "vulkan/vulkan.h"
#include <dlfcn.h>
#include <wayland-client.h>
#define LIBRARY_TYPE void *
#define LoadFunction dlsym
#endif

#define CINFO
#define CERROR
#define CLOG_VK
#define CSUCCESS
#define CFATAL
#define CLOG_RF

#include <iostream>

namespace meh {

#define EXPORTED_VULKAN_FUNCTION(name) extern PFN_##name name;
#define GLOBAL_LEVEL_VULKAN_FUNCTION(name) extern PFN_##name name;
#define INSTANCE_LEVEL_VULKAN_FUNCTION(name) extern PFN_##name name;
#define INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(name, extension)         \
  extern PFN_##name name;
#define DEVICE_LEVEL_VULKAN_FUNCTION(name) extern PFN_##name name;
#define DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(name, extension)           \
  extern PFN_##name name;

#include "meh_vulkan_functions.inl"

bool LoadFunctionExportedFromVulkanLoaderLibrary(
    LIBRARY_TYPE const &vulkan_library);
bool LoadGlobalLevelFunctions();
bool LoadInstanceLevelFunctions(
    VkInstance instance, std::vector<char const *> const &enabled_extensions);
bool LoadDeviceLevelFunctions(
    VkDevice logical_device,
    std::vector<char const *> const &enabled_extensions);

bool load_vulkan_runtime(LIBRARY_TYPE vulkan_library);

} // namespace meh

#endif // MEH_VULKAN_FUNCTIONS_H
