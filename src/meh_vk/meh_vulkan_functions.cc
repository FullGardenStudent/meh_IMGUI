// Author : FullGardenStudent
// Description:
//////////////////////////////////////////////////////////////////////

#include "meh_vulkan_functions.hh"

namespace meh {

#define EXPORTED_VULKAN_FUNCTION(name) PFN_##name name;
#define GLOBAL_LEVEL_VULKAN_FUNCTION(name) PFN_##name name;
#define INSTANCE_LEVEL_VULKAN_FUNCTION(name) PFN_##name name;
#define INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(name, extension)         \
  PFN_##name name;
#define DEVICE_LEVEL_VULKAN_FUNCTION(name) PFN_##name name;
#define DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(name, extension)           \
  PFN_##name name;

#include "meh_vulkan_functions.inl"

bool LoadFunctionExportedFromVulkanLoaderLibrary(
    LIBRARY_TYPE const &vulkan_library) {

#define EXPORTED_VULKAN_FUNCTION(name)                                         \
  name = (PFN_##name)LoadFunction(vulkan_library, #name);                      \
  if (name == nullptr) {                                                       \
    CFATAL("Could not load exported Vulkan function named: " #name);           \
    return false;                                                              \
  } else {                                                                     \
    CINFO("Loaded vulkan function : " #name);                                  \
  }

#include "meh_vulkan_functions.inl"

  return true;
}

bool LoadGlobalLevelFunctions() {
#define GLOBAL_LEVEL_VULKAN_FUNCTION(name)                                     \
  name = (PFN_##name)vkGetInstanceProcAddr(nullptr, #name);                    \
  if (name == nullptr) {                                                       \
    CERROR("Could not load global level Vulkan function named: " #name);       \
    return false;                                                              \
  } else {                                                                     \
    CINFO("Loaded vulkan function : " #name);                                  \
  }

#include "meh_vulkan_functions.inl"

  return true;
}

bool LoadInstanceLevelFunctions(
    VkInstance instance, std::vector<char const *> const &enabled_extensions) {
  // Load core Vulkan API instance-level functions
#define INSTANCE_LEVEL_VULKAN_FUNCTION(name)                                   \
  name = (PFN_##name)vkGetInstanceProcAddr(instance, #name);                   \
  if (name == nullptr) {                                                       \
    CERROR("Could not load instance-level Vulkan function named: " #name);     \
    return false;                                                              \
  } else {                                                                     \
    CINFO("Loaded vulkan function : " #name);                                  \
  }

  // Load instance-level functions from enabled extensions
#define INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(name, extension)         \
  for (auto &enabled_extension : enabled_extensions) {                         \
    if (std::string(enabled_extension) == std::string(extension)) {            \
      name = (PFN_##name)vkGetInstanceProcAddr(instance, #name);               \
      if (name == nullptr) {                                                   \
        CERROR("Could not load instance-level Vulkan function named: " #name); \
                                                                               \
      } else {                                                                 \
        CINFO("Loaded vulkan function : " #name);                              \
      }                                                                        \
    }                                                                          \
  }

#include "meh_vulkan_functions.inl"

  return true;
}

bool LoadDeviceLevelFunctions(
    VkDevice logical_device,
    std::vector<char const *> const &enabled_extensions) {
  // Load core Vulkan API device-level functions
#define DEVICE_LEVEL_VULKAN_FUNCTION(name)                                     \
  name = (PFN_##name)vkGetDeviceProcAddr(logical_device, #name);               \
  if (name == nullptr) {                                                       \
    CERROR("Could not load device-level Vulkan function named: " #name);       \
    return false;                                                              \
  } else {                                                                     \
    CINFO("Loaded vulkan function : " #name);                                   \
  }

  // Load device-level functions from enabled extensions
#define DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(name, extension)           \
  for (auto &enabled_extension : enabled_extensions) {                         \
    if (std::string(enabled_extension) == std::string(extension)) {            \
      name = (PFN_##name)vkGetDeviceProcAddr(logical_device, #name);           \
      if (name == nullptr) {                                                   \
        std::cout                                                              \
            << "Could not load device-level Vulkan function named: " #name     \
            << std::endl;                                                      \
        return false;                                                          \
      } else {                                                                 \
        CINFO("Loaded vulkan function : " #name);                               \
      }                                                                        \
    }                                                                          \
  }

#include "meh_vulkan_functions.inl"

  return true;
}

bool load_vulkan_runtime(LIBRARY_TYPE vulkan_library) {
  CLOG_RF(LoadFunctionExportedFromVulkanLoaderLibrary(vulkan_library),
          "Loading Vulkan Loader Library Exported Function!");
  CLOG_RF(LoadGlobalLevelFunctions(), "Loading Global Level Vulkan Functions!");
  return true;
}

} // namespace meh
