#ifndef MEH_VULKAN_OBJECTS
#define MEH_VULKAN_OBJECTS

#include "meh_vulkan_functions.hh"
#include "stb/stb_image.h"
#include "vulkan/vulkan_core.h"
#include <iostream>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <set>

typedef uint32_t U32;

enum MEH_DEBUG_MODE { NONE = 0, VALIDATION_ONLY = 1, FULL = 2 };

namespace meh {

struct push_constants {
  U32 screen_width;
  U32 screen_height;
  U32 x_pos;
  U32 y_pos;
  U32 quad_id;
};

struct meh_descriptors {
  VkDescriptorPool descriptor_pool;
  std::vector<VkDescriptorSet> descriptor_sets;

  void init(VkDevice device, VkDescriptorSetLayout layout, U32 render_ahead) {
    this->render_ahead = render_ahead;
    this->device = device;

    pool_sizes.resize(this->render_ahead);
    pool_sizes[0] = {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, this->render_ahead};
    pool_sizes[1] = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, this->render_ahead};

    VkDescriptorPoolCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .maxSets = render_ahead,
        .poolSizeCount = static_cast<U32>(this->pool_sizes.size()),
        .pPoolSizes = pool_sizes.data()};
 
    vkCreateDescriptorPool(device, &create_info, nullptr,
                           &this->descriptor_pool);

    std::vector<VkDescriptorSetLayout> layouts(render_ahead, layout);

    VkDescriptorSetAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = this->descriptor_pool,
        .descriptorSetCount = static_cast<U32>(layouts.size()),
        .pSetLayouts = layouts.data()};
    this->descriptor_sets.resize(this->render_ahead);
    vkAllocateDescriptorSets(this->device,&alloc_info, this->descriptor_sets.data());
  }

  void write(const VkBuffer storage_buffer, const VkSampler sampler, const VkImageView image_view) {
    descriptor_sets.resize(this->render_ahead);
    for (U32 i = 0; i < this->render_ahead; i++) {
      VkDescriptorBufferInfo storage_buffer_info = {
          .buffer = storage_buffer, .offset = 0, .range = VK_WHOLE_SIZE};

      VkDescriptorImageInfo image_info = {.sampler = sampler,
                                          .imageView = image_view,
                                          .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

      std::vector<VkWriteDescriptorSet> descriptor_writes(2);

      descriptor_writes[0] = {.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                              .pNext = nullptr,
                              .dstSet = descriptor_sets[i],
                              .dstBinding = 0,
                              .dstArrayElement = 0,
                              .descriptorCount = 1,
                             .descriptorType =
                                  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                              .pImageInfo = nullptr,
                              .pBufferInfo = &storage_buffer_info,
                              .pTexelBufferView = nullptr};

      descriptor_writes[1] = {.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                              .pNext = nullptr,
                              .dstSet = descriptor_sets[i],
                              .dstBinding = 1,
                              .dstArrayElement = 0,
                              .descriptorCount = 1,
                              .descriptorType =
                                  VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                              .pImageInfo = &image_info,
                              .pBufferInfo = nullptr,
                              .pTexelBufferView = nullptr};

      vkUpdateDescriptorSets(this->device,
                             static_cast<U32>(descriptor_writes.size()),
                             descriptor_writes.data(), 0, nullptr);
    }
  }

private:
  U32 render_ahead = 0;
  VkDevice device{nullptr};
  std::vector<VkDescriptorPoolSize> pool_sizes;
};

struct meh_pipeline {
  VkPipeline handle;
  VkPipelineLayout pipeline_layout;
  VkDescriptorSetLayout descriptor_set_layout;

  void init(std::string_view vertex_shader_path,
            std::string_view fragment_shader_path, VkExtent2D image_extent,
            VkDevice device, VkRenderPass render_pass) {
    this->image_extent = image_extent;
    this->device = device;
    create_graphics_pipelien(vertex_shader_path, fragment_shader_path,
                             render_pass);
  }

private:
  VkDevice device;
  VkExtent2D image_extent;
  bool create_graphics_pipelien(std::string_view vertex_shader_path,
                                std::string_view fragment_shader_path,
                                VkRenderPass render_pass) {
    VkShaderModule vertex_shader_module =
        create_shader_module(vertex_shader_path.data());
    VkShaderModule fragment_shader_module =
        create_shader_module(fragment_shader_path.data());

    VkPipelineShaderStageCreateInfo vertex_shader_stage = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertex_shader_module,
        .pName = "main"};

    VkPipelineShaderStageCreateInfo fragment_shader_stage = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragment_shader_module,
        .pName = "main"};

    std::vector<VkPipelineShaderStageCreateInfo> shader_stages = {
        vertex_shader_stage, fragment_shader_stage};

    VkPipelineVertexInputStateCreateInfo vertex_input_state = {
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

    VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE};

    VkViewport viewport = {
        0.0f, 0.0f, (float)image_extent.width, (float)image_extent.height,
        0.0f, 1.0f};

    VkRect2D scissor = {{0, 0}, image_extent};

    VkPipelineViewportStateCreateInfo viewport_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor};

    VkPipelineRasterizationStateCreateInfo rasterization_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_FRONT_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f};

    VkPipelineMultisampleStateCreateInfo multi_sample_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE};

    VkPipelineColorBlendAttachmentState color_blend_attachment = {
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};

    VkPipelineColorBlendStateCreateInfo color_blend_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &color_blend_attachment,
        .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}};

    std::vector<VkDynamicState> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT,
                                                  VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamic_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
        .pDynamicStates = dynamic_states.data()};

    std::vector<VkDescriptorSetLayoutBinding> descriptor_bindings(2);
    descriptor_bindings[0] = {.binding = 0,
                              .descriptorType =
                                  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                              .descriptorCount = 1,
                              .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                              .pImmutableSamplers = nullptr};

    descriptor_bindings[1] = {.binding = 1,
                              .descriptorType =
                                  VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                              .descriptorCount = 1,
                              .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                              .pImmutableSamplers = nullptr};

    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .bindingCount = static_cast<uint32_t>(descriptor_bindings.size()),
        .pBindings = descriptor_bindings.data()};

    // VkDescriptorSetLayout descriptor_set_layout;
    vkCreateDescriptorSetLayout(device, &descriptor_set_layout_info, nullptr,
                                &this->descriptor_set_layout);

    VkPushConstantRange push_constant_range = {.stageFlags =
                                                   VK_SHADER_STAGE_VERTEX_BIT,
                                               .offset = 0,
                                               .size = sizeof(push_constants)};
    VkPipelineLayoutCreateInfo pipeline_layout_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount =
            1, // static_cast<u32>(meh.descriptor_set_layout.size()),
        .pSetLayouts = &this->descriptor_set_layout,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &push_constant_range};

    vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr,
                           &this->pipeline_layout);

    VkGraphicsPipelineCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .flags = {},
        .stageCount = static_cast<uint32_t>(shader_stages.size()),
        .pStages = shader_stages.data(),
        .pVertexInputState = &vertex_input_state,
        .pInputAssemblyState = &input_assembly_state,
        .pTessellationState = nullptr,
        .pViewportState = &viewport_state,
        .pRasterizationState = &rasterization_state,
        .pMultisampleState = &multi_sample_state,
        .pDepthStencilState = nullptr,
        .pColorBlendState = &color_blend_state,
        .pDynamicState = &dynamic_state,
        .layout = pipeline_layout,
        .renderPass = render_pass,
        .subpass = 0,
        .basePipelineHandle = nullptr};

    if (vkCreateGraphicsPipelines(device, nullptr, 1, &create_info, nullptr,
                                  &this->handle) != VK_SUCCESS) {
      std::cerr << "Failed to create meh graphics pipeline!" << std::endl;
      return false;
    }

    return true;
  }

  VkShaderModule create_shader_module(const std::string &path) {
    std::ifstream shader_file(path, std::ios::ate | std::ios::binary);

    if (!shader_file.is_open()) {
      std::cerr << "Failed to open the shader file!" << std::endl;
    }

    size_t file_size = (size_t)shader_file.tellg();
    std::vector<char> buffer(file_size);

    shader_file.seekg(0);
    shader_file.read(buffer.data(), file_size);

    shader_file.close();

    VkShaderModuleCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = file_size,
        .pCode = reinterpret_cast<const uint32_t *>(buffer.data())};

    VkShaderModule shader_module;
    if (vkCreateShaderModule(this->device, &create_info, nullptr,
                             &shader_module)) {
      std::cerr << "Failed to create shader module!" << std::endl;
    }

    return shader_module;
  }
};

struct meh_renderer_cxt {

  meh_pipeline pipeline;
  meh_descriptors descriptor;

  bool render_frame = false;
  
#ifdef _WIN32
  HMODULE h_module;
#endif

#ifdef __linux
  void *vulkan_library;
#endif

  meh_renderer_cxt(meh_renderer_cxt &) = delete;

  meh_renderer_cxt &operator=(const meh_renderer_cxt &) = delete;

  meh_renderer_cxt &operator=(meh_renderer_cxt &&) = delete;

  struct meh_instance {

    VkInstance handle{VK_NULL_HANDLE};
    std::vector<const char *> instance_layers;

    bool initialize(std::vector<const char *> &instance_extensions,
                    MEH_DEBUG_MODE debug_mode) {

      VkApplicationInfo app_info = {
          .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
          .pNext = nullptr,
          .pApplicationName = "meh_IMGUI",
          .applicationVersion = VK_API_VERSION_1_0,
          .pEngineName = "?????????????",
          .engineVersion = VK_API_VERSION_1_0,
          .apiVersion = VK_API_VERSION_1_3,
      };

      VkInstanceCreateInfo create_info = {};
      create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
      create_info.pNext = nullptr;
      create_info.flags = 0;
      create_info.pApplicationInfo = &app_info;
      create_info.enabledExtensionCount =
          static_cast<U32>(instance_extensions.size());
      create_info.ppEnabledExtensionNames = instance_extensions.data();

      switch (debug_mode) {
      case NONE:
        break;
      case FULL: {
        instance_layers.emplace_back("VK_LAYER_KHRONOS_validation");
        instance_layers.emplace_back("VK_LAYER_LUNARG_api_dump");
      } break;
      case VALIDATION_ONLY: {
        instance_layers.emplace_back("VK_LAYER_KHRONOS_validation");
      } break;
      }

      create_info.enabledLayerCount = static_cast<U32>(instance_layers.size());
      create_info.ppEnabledLayerNames = instance_layers.data();

      VkInstanceCreateInfo create_info2 = {
          .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
          .pNext = nullptr,
          .flags = 0,
          .pApplicationInfo = &app_info,
          .enabledLayerCount = static_cast<U32>(instance_layers.size()),
          .ppEnabledLayerNames = instance_layers.data(),
          .enabledExtensionCount = static_cast<U32>(instance_extensions.size()),
          .ppEnabledExtensionNames = instance_extensions.data()};

      CLOG_VK(vkCreateInstance(&create_info2, nullptr, &this->handle),
              "Creating Vulkan Instance!");
      return true;
    }

    bool load_functions(std::vector<const char *> enabled_extensions) {
      return LoadInstanceLevelFunctions(this->handle, enabled_extensions);
    }

  } instance;

  struct meh_surface {
    VkSurfaceKHR handle;

#ifdef __linux
    bool create(VkInstance instance, wl_display *display, wl_surface *surface) {
      VkWaylandSurfaceCreateInfoKHR create_info = {
          .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
          .pNext = NULL,
          .flags = 0,
          .display = display,
          .surface = surface};
      vkCreateWaylandSurfaceKHR(instance, &create_info, nullptr, &this->handle);

      return true;
    }

#endif
#ifdef _WIN32
    bool create(VkInstance instance, HINSTANCE h_instance, HWND hwnd) {
      VkWin32SurfaceCreateInfoKHR create_info = {
          .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
          .pNext = nullptr,
          .flags = 0,
          .hinstance = h_instance,
          .hwnd = hwnd};

      CLOG_VK(vkCreateWin32SurfaceKHR(instance, &create_info, nullptr, &handle),
              "Surface creation");
      return true;
    }
#endif

  } surface;

  struct meh_physical_device {
    VkPhysicalDevice handle{VK_NULL_HANDLE};

    // gpu properties
    VkPhysicalDeviceProperties2 properties;
    VkPhysicalDeviceVulkan13Properties properties_13;

    // gpu features
    VkPhysicalDeviceFeatures2 features;
    VkPhysicalDeviceVulkan13Features features_13;

    // gpu memory properties
    VkPhysicalDeviceMemoryProperties2 memory_properties;

    // gpu format properties
    VkFormatProperties2 format_properties;

    // surface info for swapchain surface format
    VkPhysicalDeviceSurfaceInfo2KHR surface_info;

    std::vector<VkQueueFamilyProperties2> queue_family_properties;

    // first element of queue_infos is for graphics queue.
    // second is for present and third is for compute respectively
    std::vector<VkDeviceQueueInfo2> queue_infos;

#ifdef __linux
    bool wayland_support(wl_display *display) {
      return vkGetPhysicalDeviceWaylandPresentationSupportKHR(this->handle, 0,
                                                              display);
    }
#endif

    // Ignoring preformance counter for now.
    // TODO: a decent performance counter
    void populate_gpu_properties(VkSurfaceKHR surface) {
      // get queue family properties
      {
        U32 queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties2(this->handle,
                                                  &queue_family_count, nullptr);
        this->queue_family_properties.resize(queue_family_count);
        for (auto &i : queue_family_properties) {
          i.sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
        }
        vkGetPhysicalDeviceQueueFamilyProperties2(
            this->handle, &queue_family_count, queue_family_properties.data());
      }

      // get gpu features
      {
        this->features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        this->features_13.sType =
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        this->features.pNext = &this->features_13;
        vkGetPhysicalDeviceFeatures2(this->handle, &features);
      }

      // fill up surface info for swapchain
      {
        surface_info = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR,
            .pNext = NULL,
            .surface = surface};
      }
      // get gpu memory properties
      {
        this->memory_properties.sType =
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
        this->memory_properties.pNext = NULL;
        vkGetPhysicalDeviceMemoryProperties2(this->handle,
                                             &this->memory_properties);
      }

      // get format properties
      {
        this->format_properties = {VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2, NULL};
        // vkGetPhysicalDeviceFormatProperties2(this->handle,  );
      }
    }

    bool acquire_queue_family_indices(VkSurfaceKHR surface) {
      // graphics queue
      queue_infos.resize(2);
      for (auto &i : queue_infos) {
        i.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2;
        i.flags = 0;
        i.pNext = nullptr;
        i.queueIndex = 0;
      }
      VkQueueFlags required_queue =
          VK_QUEUE_GRAPHICS_BIT; // | VK_QUEUE_COMPUTE_BIT;
      VkBool32 present_support;
      {
        for (U32 i = 0; i < this->queue_family_properties.size(); i++) {
          if ((this->queue_family_properties[i]
                   .queueFamilyProperties.queueCount > 0) &&
              ((this->queue_family_properties[i]
                    .queueFamilyProperties.queueFlags &
                required_queue) == required_queue)) {
            CINFO("Graphics Queue family index is : %d", i);
            this->queue_infos[0].queueFamilyIndex = i;
            // queue_family_indices.insert(i);
            //  check for present support
            vkGetPhysicalDeviceSurfaceSupportKHR(this->handle, i, surface,
                                                 &present_support);
            // check if presentation is alose supported by this queue family
            if (present_support) {
              this->queue_infos[1].queueFamilyIndex = i;
              // present_queue_family_index = i;
              CINFO("Present Queue family index : %d", i);
            }
            break;
          }
        }

        // if present support was not find in the same queue family as
        // the graphics queue, iterate through available queue families and find
        // a valid queue family with present support
        if (!present_support) {
          for (U32 i = 0; i < queue_family_properties.size(); i++) {
            vkGetPhysicalDeviceSurfaceSupportKHR(this->handle, i, surface,
                                                 &present_support);
            if (present_support == VK_TRUE) {
              queue_infos[1].queueFamilyIndex = i;
              // present_queue_family_index = i;
              CINFO("Present Queue family index : %d", i);
              break;
            }
          }
        }
      }

      return true;
    }

    bool pick(VkInstance instance) {
      {
        U32 gpu_count = 0;
        vkEnumeratePhysicalDevices(instance, &gpu_count, nullptr);
        std::vector<VkPhysicalDevice> gpus(gpu_count);
        vkEnumeratePhysicalDevices(instance, &gpu_count, gpus.data());

        // just pick the first available GPU for now.
        //  TODO : a better way of picking GPU and queue families
        // VkPhysicalDeviceProperties2 gpu_properties{};
        properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        properties_13.sType =
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES;
        properties.pNext = &properties_13;
        vkGetPhysicalDeviceProperties2(gpus[0], &properties);
        this->handle = gpus[0];

// pirnt selected GPU!
#ifdef _WIN32
        const int tl = MultiByteToWideChar(
            CP_UTF8, 0, properties.properties.deviceName, -1, nullptr, 0);
        wchar_t *gpu_name = new wchar_t[tl];
        MultiByteToWideChar(CP_UTF8, 0, properties.properties.deviceName, -1,
                            gpu_name, tl);
        CSUCCESS("Picked %ls GPU!", gpu_name);
#endif
        CINFO("Driver version: %u.%u.%u\n",
              VK_VERSION_MAJOR(properties.properties.driverVersion),
              VK_VERSION_MINOR(properties.properties.driverVersion),
              VK_VERSION_PATCH(properties.properties.driverVersion));
      }
      return true;
    }
  } gpu;

  struct meh_device {
    VkDevice handle{VK_NULL_HANDLE};

    VkQueue graphics_queue;
    VkQueue present_queue;
    VkQueue compute_queue;

    inline void wait() { vkDeviceWaitIdle(this->handle); }

    // totally ignoring device groups for now.
    // TODO: become certain about inclusion of device groups
    bool create(VkPhysicalDevice gpu,
                std::vector<VkDeviceQueueInfo2> &queue_infos,
                //VkPhysicalDeviceVulkan13Features &features,
		VkPhysicalDeviceFeatures2 &features,
                const std::vector<const char *> &device_extensions) {
      {
        U32 queue_count = 1;
        float queue_priority =
            VkQueueGlobalPriorityKHR::VK_QUEUE_GLOBAL_PRIORITY_REALTIME_KHR /
            1024;
        std::vector<VkDeviceQueueCreateInfo> device_queue_create_infos;
        std::set<U32> queue_family_indices;
        for (VkDeviceQueueInfo2 &i : queue_infos) {
          queue_family_indices.insert(i.queueFamilyIndex);
        }
        for (const auto i : queue_family_indices) {
          VkDeviceQueueCreateInfo device_queue_create_info = {
              .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
              .pNext = nullptr,
              .flags = 0,
              .queueFamilyIndex = i,
              .queueCount = queue_count++,
              .pQueuePriorities = &queue_priority};
          device_queue_create_infos.emplace_back(device_queue_create_info);
          queue_count++;
        }

	features.features.samplerAnisotropy = VK_TRUE;
	
	// VkPhysicalDeviceFeatures2 gpu_features{};
        VkDeviceCreateInfo create_info = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &features,
            .flags = 0,
            .queueCreateInfoCount =
                static_cast<U32>(device_queue_create_infos.size()),
            .pQueueCreateInfos = device_queue_create_infos.data(),
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = static_cast<U32>(device_extensions.size()),
            .ppEnabledExtensionNames = device_extensions.data(),
            .pEnabledFeatures = nullptr};
        CLOG_VK(vkCreateDevice(gpu, &create_info, nullptr, &this->handle),
                "Device creation!");
      }
      return true;
    }

    void load_functions(const std::vector<const char *> &device_extensions) {
      LoadDeviceLevelFunctions(this->handle, device_extensions);
    }

    void get_queues(const std::vector<VkDeviceQueueInfo2> queue_infos) {
      vkGetDeviceQueue2(this->handle, &queue_infos[0], &this->graphics_queue);
      vkGetDeviceQueue2(this->handle, &queue_infos[1], &this->present_queue);
    }
  } device;

  struct meh_swapchain {
    VkSwapchainKHR handle{VK_NULL_HANDLE};
    std::vector<VkImage> images;
    std::vector<VkImageView> image_views;

    VkFormat image_format = VK_FORMAT_B8G8R8A8_SRGB;
    VkColorSpaceKHR color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

    VkPresentModeKHR present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    VkExtent2D image_extent{0};

    // since all surface stuff is associated with WSI, they are
    // not a part of gpu object
    VkSurfaceCapabilities2KHR surface_capabilities;
    std::vector<VkSurfaceFormat2KHR> surface_formats;
    std::vector<VkPresentModeKHR> present_modes;

    bool exclusive = true;
    U32 queue_family_indices = 0;
    std::vector<U32> concurrent_queue_families;

    void
    get_surface_capabalities(VkPhysicalDevice gpu,
                             VkPhysicalDeviceSurfaceInfo2KHR &surface_info) {
      surface_capabilities.sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR;
      surface_capabilities.pNext = nullptr;
      vkGetPhysicalDeviceSurfaceCapabilities2KHR(gpu, &surface_info,
                                                 &this->surface_capabilities);
      this->image_extent =
          this->surface_capabilities.surfaceCapabilities.currentExtent;
    }

    void get_surface_properties(VkPhysicalDevice gpu,
                                VkPhysicalDeviceSurfaceInfo2KHR &surface_info) {
      // get surface capabilities
      surface_capabilities.sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR;
      surface_capabilities.pNext = nullptr;
      vkGetPhysicalDeviceSurfaceCapabilities2KHR(gpu, &surface_info,
                                                 &this->surface_capabilities);

      // get surface formats
      U32 surface_format_count = 0;
      vkGetPhysicalDeviceSurfaceFormats2KHR(gpu, &surface_info,
                                            &surface_format_count, NULL);
      CINFO("Surface formats : %d", surface_format_count);
      surface_formats.resize(surface_format_count);
      for (auto &i : surface_formats) {
        i.sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR;
        i.pNext = nullptr;
      }
      vkGetPhysicalDeviceSurfaceFormats2KHR(
          gpu, &surface_info, &surface_format_count, surface_formats.data());

      // get present modes
      U32 present_mode_count = 0;
      vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface_info.surface,
                                                &present_mode_count, NULL);
      present_modes.resize(present_mode_count);
      vkGetPhysicalDeviceSurfacePresentModesKHR(
          gpu, surface_info.surface, &present_mode_count, present_modes.data());
    }

    bool choose_surface_properties(U32 window_width, U32 window_height) {
      // choose a format and colorspace
      {
        bool found_format = false;
        for (const auto &i : this->surface_formats) {
          if (i.surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
              i.surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            this->image_format = VK_FORMAT_B8G8R8A8_SRGB;
            this->color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
            found_format = true;
          }
        }
        if (!found_format) {
          this->image_format = surface_formats[0].surfaceFormat.format;
          this->color_space = surface_formats[0].surfaceFormat.colorSpace;
        }
      }

      // choose present mode
      {
        bool found_present_mode = false;
        for (const auto &i : this->present_modes) {
          if (i == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            found_present_mode = true;
          }
        }
        if (!found_present_mode) {
          this->present_mode = VK_PRESENT_MODE_FIFO_KHR;
        }
      }
#ifdef _WIN32
      // check swapchain extent
      {
        if (this->surface_capabilities.surfaceCapabilities.currentExtent
                .width != 0xFFFFFFFF) {
          this->image_extent =
              surface_capabilities.surfaceCapabilities.currentExtent;
        } else {
          int width=0, height=0;
          VkExtent2D actual_extent = {(U32)width, (U32)height};
          this->image_extent.width = std::clamp(
              actual_extent.width,
              surface_capabilities.surfaceCapabilities.minImageExtent.width,
              surface_capabilities.surfaceCapabilities.maxImageExtent.width);

          this->image_extent.height = std::clamp(
              actual_extent.width,
              surface_capabilities.surfaceCapabilities.minImageExtent.height,
              surface_capabilities.surfaceCapabilities.maxImageExtent.height);
        }
      }
#endif
#ifdef __linux
      // if(this->image_extent.width == 0xFFFFFFFF || this->image_extent.height == 0xFFFFFFFF){
      // 	this->image_extent = {1020,720};
      // }
      // only set this for the first time. 
      if(this->image_extent.width == 0 || image_extent.height == 0){
	this->image_extent = {1020,720};
      }
#endif
      return true;
    }

    void is_exclusive(std::vector<VkDeviceQueueInfo2> &queue_infos) {
      // check if all queue family indices are same
      U32 queue_family_index = queue_infos[0].queueFamilyIndex;
      for (const auto &i : queue_infos) {
        if (queue_family_index != i.queueFamilyIndex) {
          exclusive = false;
        }
      }

      // if sharing mode is concurrent, calculate how many queue family indices
      // are involved
      if (!exclusive) {
        std::set<U32> queue_families;
        for (const auto &i : queue_infos) {
          queue_families.insert(i.queueFamilyIndex);
        }
        queue_family_indices = (uint32_t)queue_families.size();

        for (const auto &i : queue_families) {
          concurrent_queue_families.push_back(i);
        }
      }
    }

    bool create(VkSurfaceKHR surface, VkDevice device) {
      if(this->image_extent.width == 0xFFFFFFFF || this->image_extent.height == 0xFFFFFFFF || this->image_extent.width == 0 || this->image_extent.height == 0){
	this->image_extent = {1020,720};
      }
      
      VkSwapchainKHR old_swapchain = this->handle;

      VkSwapchainCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .surface = surface,
        .minImageCount =
            surface_capabilities.surfaceCapabilities.minImageCount + 1,
        .imageFormat = this->image_format,
        .imageColorSpace = color_space,
#ifdef _WIN32
        .imageExtent = surface_capabilities.surfaceCapabilities.currentExtent,
#endif
        #ifdef __linux
          .imageExtent = image_extent,
	  #endif
          .imageArrayLayers = 1,
          .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
          .imageSharingMode = !this->exclusive ? VK_SHARING_MODE_EXCLUSIVE
                                               : VK_SHARING_MODE_CONCURRENT,
          .queueFamilyIndexCount = queue_family_indices,
          .pQueueFamilyIndices =
              this->exclusive ? nullptr : concurrent_queue_families.data(),
          .preTransform =
              surface_capabilities.surfaceCapabilities.currentTransform,
#ifdef __linux
          .compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
#endif
#ifdef _WIN32
          .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
#endif
          .presentMode = present_mode,
          .clipped = VK_TRUE,
          .oldSwapchain = old_swapchain};

      if (!old_swapchain) {
        vkDestroySwapchainKHR(device, old_swapchain, nullptr);
      }

      vkCreateSwapchainKHR(device, &create_info, nullptr, &this->handle);
      

      U32 image_count = 0;
      vkGetSwapchainImagesKHR(device, this->handle, &image_count, NULL);
      this->images.resize(image_count);
      vkGetSwapchainImagesKHR(device, this->handle, &image_count,
                              images.data());

      // create swapchain image views
      this->image_views.resize(this->images.size());
      for (U32 i = 0; i < this->images.size(); i++) {
        VkImageViewCreateInfo create_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .image = this->images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = this->image_format,
            .components = {VK_COMPONENT_SWIZZLE_IDENTITY,
                           VK_COMPONENT_SWIZZLE_IDENTITY,
                           VK_COMPONENT_SWIZZLE_IDENTITY,
                           VK_COMPONENT_SWIZZLE_IDENTITY},
            .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};
        CLOG_VK(vkCreateImageView(device, &create_info, nullptr,
                                  &this->image_views[i]),
                "Image view creation");
      }
      return true;
    }
  } swapchain;

  struct meh_renderpass {
    VkRenderPass handle{VK_NULL_HANDLE};

    bool destroy(VkDevice device) {
      vkDestroyRenderPass(device, this->handle, nullptr);
      return true;
    }

    void begin(VkFramebuffer frame_buffer, VkExtent2D extent,
               VkCommandBuffer &command_buffer) {
      VkClearValue clear_value = {{{0.2, 0.2, 0.2, 0.0}}};
      VkRenderPassBeginInfo render_pass_begin_info = {
          .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
          .pNext = nullptr,
          .renderPass = this->handle,
          .framebuffer = frame_buffer,
          .renderArea = VkRect2D{{0, 0}, extent},
          .clearValueCount = 1,
          .pClearValues = &clear_value};

      vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info,
                           VK_SUBPASS_CONTENTS_INLINE);
    }

    void end(VkCommandBuffer command_buffer) {
      vkCmdEndRenderPass(command_buffer);
    }

    bool create(VkDevice device, VkFormat format) {

      VkAttachmentDescription2 attachment_description = {
          .sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
          .pNext = NULL,
          .flags = 0,
          .format = format,
          .samples = VK_SAMPLE_COUNT_1_BIT,
          .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
          .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
          .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
          .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
          .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
          .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};

      VkAttachmentReference2 attachment_ref = {
          .sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
          .pNext = NULL,
          .attachment = 0,
          .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
          .aspectMask = 0};

      VkSubpassDescription2 subpass_descirption = {
          .sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2,
          .pNext = NULL,
          .flags = 0,
          .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
          .viewMask = 0,
          .inputAttachmentCount = 0,
          .pInputAttachments = NULL,
          .colorAttachmentCount = 1,
          .pColorAttachments = &attachment_ref,
          .pResolveAttachments = nullptr,
          .pDepthStencilAttachment = nullptr,
          .preserveAttachmentCount = 0,
          .pPreserveAttachments = nullptr};

      VkSubpassDependency2 subpass_dependency = {
          .sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
          .pNext = NULL,
          .srcSubpass = VK_SUBPASS_EXTERNAL,
          .dstSubpass = 0,
          .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
          .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
          .srcAccessMask = 0,
          .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
          .dependencyFlags = {},
          .viewOffset = {}};

      VkRenderPassCreateInfo2 create_info = {
          .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2,
          .pNext = NULL,
          .flags = 0,
          .attachmentCount = 1,
          .pAttachments = &attachment_description,
          .subpassCount = 1,
          .pSubpasses = &subpass_descirption,
          .dependencyCount = 1,
          .pDependencies = &subpass_dependency,
          .correlatedViewMaskCount = 0,
          .pCorrelatedViewMasks = NULL};

      CLOG_VK(vkCreateRenderPass2(device, &create_info, nullptr, &this->handle),
              "Render Pass creation!");
      return true;
    }
  };

  struct meh_framebuffer {

    std::vector<VkFramebuffer> buffers;

    bool destroy(VkDevice device) {
      for (auto f : this->buffers) {
        vkDestroyFramebuffer(device, f, nullptr);
      }
      return true;
    }

    bool create(const std::vector<VkImageView> &swapchain_image_views,
                VkExtent2D current_extent, VkRenderPass render_pass,
                VkDevice device) {
      this->buffers.resize(swapchain_image_views.size());
      for (U32 i = 0; i < swapchain_image_views.size(); i++) {
        VkFramebufferCreateInfo create_info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .renderPass = render_pass,
            .attachmentCount = 1,
            .pAttachments = &swapchain_image_views[i],
            .width = current_extent.width,
            .height = current_extent.height,
            .layers = 1};
        CLOG_VK(vkCreateFramebuffer(device, &create_info, nullptr,
                                    &this->buffers[i]),
                "Frame buffer creation!");
      }
      return true;
    }
  };

  struct meh_command_pool {

    VkCommandPool handle;

    bool create(VkDevice device, U32 queue_index) {
      VkCommandPoolCreateInfo create_info = {
          .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
          .pNext = nullptr,
          .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
          .queueFamilyIndex = queue_index};
      CLOG_VK(vkCreateCommandPool(device, &create_info, nullptr, &this->handle),
              "Command pool creation!");
      return true;
    }
  } command_pool;

  struct meh_command_buffers {
    std::vector<VkCommandBuffer> buffers;
    // VkCommandBuffer handle;

    void begin(U32 frame_index) {
      vkResetCommandBuffer(this->buffers[frame_index], 0);
      VkCommandBufferBeginInfo begin_info = {
          VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
      vkBeginCommandBuffer(buffers[frame_index], &begin_info);
    }

    void end(U32 frame_index) { vkEndCommandBuffer(buffers[frame_index]); }

    bool allocate(VkDevice device, VkCommandPool command_pool, U32 count) {
      VkCommandBufferAllocateInfo alloc_info = {
          .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
          .pNext = nullptr,
          .commandPool = command_pool,
          .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
          .commandBufferCount = count};
      buffers.resize(count);
      CLOG_VK(
          vkAllocateCommandBuffers(device, &alloc_info, this->buffers.data()),
          "Command buffer creation!");
      return true;
    }
  };

  struct meh_frame {
    U32 render_ahead = 2;
    U32 frame_index = 0;
    U32 image_index = 0;
    U32 quads_count = 0;
    void* quads_mapped_memory;

    std::vector<VkSemaphore> present_semaphore;
    std::vector<VkSemaphore> render_semaphore;
    std::vector<VkFence> wait_fence;

    VkBuffer index_buffer;

    meh_command_buffers command_buffers;
    meh_renderpass render_pass;
    meh_framebuffer frame_buffers;

    void set_render_ahead(U32 count) {
      this->render_ahead = count;
      this->frame_index = 0;
    }

    void create_sync_objects(VkDevice device) {
      VkSemaphoreCreateInfo semaphore_info = {
          VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0};

      VkFenceCreateInfo fence_info = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                                      nullptr, VK_FENCE_CREATE_SIGNALED_BIT};

      present_semaphore.resize(render_ahead);
      render_semaphore.resize(render_ahead);
      wait_fence.resize(render_ahead);
      command_buffers.buffers.resize(render_ahead);
      for (size_t i = 0; i < this->render_ahead; i++) {
        CLOG_VK(vkCreateSemaphore(device, &semaphore_info, nullptr,
                                  &this->render_semaphore[i]),
                "Image available semaphore");
        CLOG_VK(vkCreateSemaphore(device, &semaphore_info, nullptr,
                                  &this->present_semaphore[i]),
                "Render Finished semaphore");
        CLOG_VK(
            vkCreateFence(device, &fence_info, nullptr, &this->wait_fence[i]),
            "Fence");
      }
    }

    VkResult acquire_next_image(VkDevice device, VkSwapchainKHR swapchain) {

      vkWaitForFences(device, 1, &this->wait_fence[frame_index], VK_TRUE,
                      UINT64_MAX);
      // vkResetFences(device, 1, &this->wait_fence[frame_index]);

      // VkAcquireNextImageInfoKHR acquire_info = {
      //     .sType = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR,
      //     .pNext = NULL,
      //     .swapchain = swapchain,
      //     .timeout = UINT64_MAX,
      //     .semaphore = this->render_semaphore[frame_index],
      //     .fence = VK_NULL_HANDLE,
      //     .deviceMask = 1};

      // return vkAcquireNextImage2KHR(device, &acquire_info, &image_index);
      return vkAcquireNextImageKHR(device, swapchain, UINT64_MAX,
                                   this->render_semaphore[frame_index],
                                   VK_NULL_HANDLE, &image_index);
    }

    VkResult submit_and_present(VkQueue graphics_queue, VkQueue present_queue,
                                VkSwapchainKHR swapchain) {

      VkSemaphore wait_semaphore[] = {this->render_semaphore[frame_index]};

      // VkSemaphoreSubmitInfo wait_semaphore_info = {
      //     .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
      //     .pNext = nullptr,
      //     .semaphore = this->render_semaphore[frame_index],
      //     .value = 0,
      //     .stageMask = 0,
      //     .deviceIndex = 0,
      // };

      // VkCommandBufferSubmitInfo cb_submit_info = {
      //     .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
      //     .pNext = NULL,
      //     .commandBuffer = this->command_buffers.buffers[frame_index],
      //     .deviceMask = 0};

      // VkSemaphoreSubmitInfo semaphore_submit_info = {
      //     .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
      //     .pNext = nullptr,
      //     .semaphore = this->present_semaphore[frame_index],
      //     .value = 0,
      //     .stageMask = 0,
      //     .deviceIndex = 0,
      // };

      // VkSubmitInfo2 submit_info = {.sType =
      // VK_STRUCTURE_TYPE_SUBMIT_INFO_2_KHR,
      //                              .pNext = NULL,
      //                              .flags = 0,
      //                              .waitSemaphoreInfoCount = 1,
      //                              .pWaitSemaphoreInfos =
      //                              &wait_semaphore_info,
      //                              .commandBufferInfoCount = 1,
      //                              .pCommandBufferInfos = &cb_submit_info,
      //                              .signalSemaphoreInfoCount = 1,
      //                              .pSignalSemaphoreInfos =
      //                                  &semaphore_submit_info};

      // CLOG_VK(vkQueueSubmit2(graphics_queue, 1, &submit_info,
      //                        this->wait_fence[frame_index]),
      //         "queue submit");

      VkPipelineStageFlags wait_stages = {
          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
      VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO,
                                  nullptr,
                                  1,
                                  &render_semaphore[frame_index],
                                  &wait_stages,
                                  1,
                                  &this->command_buffers.buffers[frame_index],
                                  1,
                                  &present_semaphore[frame_index]};

      if (vkQueueSubmit(graphics_queue, 1, &submit_info,
                        wait_fence[frame_index]) != VK_SUCCESS) {
        //throw std::runtime_error("Failed to submit draw command buffer!");
      }

      VkPresentInfoKHR present_info = {
          .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
          .pNext = nullptr,
          .waitSemaphoreCount = 1,
          .pWaitSemaphores = &this->present_semaphore[frame_index],
          .swapchainCount = 1,
          .pSwapchains = &swapchain,
          .pImageIndices = &this->image_index,
          .pResults = nullptr};

      frame_index = (frame_index + 1) % render_ahead;
      return vkQueuePresentKHR(present_queue, &present_info);
    }

    void record(VkDevice device, VkExtent2D extent, VkPipeline pipeline,
                VkPipelineLayout layout,
                std::vector<VkDescriptorSet> &descriptor_sets,
		const push_constants &pc) {
      vkResetFences(device, 1, &wait_fence[frame_index]);
      command_buffers.begin(this->frame_index);
      this->render_pass.begin(this->frame_buffers.buffers[image_index], extent,
                              command_buffers.buffers[frame_index]);
      vkCmdBindPipeline(this->command_buffers.buffers[this->frame_index],
                        VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
      VkViewport viewport = {
          0.0f, 0.0f, (float)extent.width, (float)extent.height, 0.0f, 1.0f};

      VkRect2D scissor = {{0, 0}, extent};
      vkCmdSetViewport(this->command_buffers.buffers[frame_index], 0, 1,
                       &viewport);
      vkCmdSetScissor(this->command_buffers.buffers[frame_index], 0, 1,
                      &scissor);

      vkCmdPushConstants(this->command_buffers.buffers[frame_index],layout,
			 VK_SHADER_STAGE_VERTEX_BIT,0, sizeof(push_constants), &pc);

      vkCmdBindDescriptorSets(this->command_buffers.buffers[this->frame_index],
                              VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1,
                              &descriptor_sets[this->frame_index], 0, nullptr);

      vkCmdBindIndexBuffer(this->command_buffers.buffers[frame_index],
                           this->index_buffer, 0, VK_INDEX_TYPE_UINT16);
      vkCmdDrawIndexed(this->command_buffers.buffers[frame_index], 6, quads_count, 0,
                       0, 0);

      this->render_pass.end(command_buffers.buffers[frame_index]);
      command_buffers.end(this->frame_index);
    }

  } frame;
};

enum MEH_BUFFER_TYPE {
  VERTEX_BUFFER = 0,
  INDEX_BUFFER = 1,
  STORAGE_BUFFER = 2
};

struct meh_buffer {
  // VkDevice const &device;
  VkBuffer handle{VK_NULL_HANDLE};
  VkDeviceMemory memory{nullptr};
  VkDeviceSize size = 0;

  void *data{nullptr};

  meh_buffer(const meh_buffer &) = delete;

  meh_buffer(meh_buffer &&other);

  meh_buffer &operator=(const meh_buffer &) = delete;

  meh_buffer &operator=(meh_buffer &&) = delete;

  void map(void* &pointer_to_map){
    vkMapMemory(device, this->memory, 0, this->size, 0, &pointer_to_map);
  }

  void map(std::vector<void*> vector_to_map){
    for(auto& v : vector_to_map){
      vkMapMemory(device, this->memory, 0, this->size, 0, &v);
    }
  }
  
  meh_buffer(void* resource_data,U32 resource_size,
	     MEH_BUFFER_TYPE buffer_type, meh_renderer_cxt &renderer) {
    this->device = renderer.device.handle;
    this->queue = renderer.device.graphics_queue;
    this->queue_index = renderer.gpu.queue_infos[0].queueFamilyIndex;
    this->mem_properties = &renderer.gpu.memory_properties;
    this->command_pool = renderer.command_pool.handle;
    if(data != nullptr) {
      CFATAL("buffer already created!");
      //return false;
    }
    this->size = resource_size,
    create_staging_buffer();
    vkMapMemory(device, this->staging_buffer_memory, 0, size, 0, &this->data);
    std::memcpy(this->data, resource_data, (size_t)size);
    vkUnmapMemory(device, this->staging_buffer_memory);

    // NOTE: VK_BUFFER_USAGE_TRANSFER_SRC_BIT is already included.
    // just define either vertex or index or storeage buffer bit is enough
    switch (buffer_type) {
    case VERTEX_BUFFER: {
      create_buffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    } break;
    case INDEX_BUFFER: {
      create_buffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    } break;
    case STORAGE_BUFFER: {
      create_buffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    } break;
    }
    
    // copy staging buffer over to device local
    copy(this->staging_buffer, this->handle);

    clear_staging_buffer();
    
  }

  
  // bool create() {
  //   return true;
  // }

private:
  U32 queue_index;
  VkQueue queue;
  VkCommandBuffer command_buffer;
  VkCommandPool command_pool;
  VkPhysicalDeviceMemoryProperties2 *mem_properties;
  VkDevice device{VK_NULL_HANDLE};

  bool copy(VkBuffer src, VkBuffer dst) {
    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = NULL,
        .commandPool = command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1};

    vkAllocateCommandBuffers(device, &alloc_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = NULL};

    vkBeginCommandBuffer(command_buffer, &begin_info);

    VkBufferCopy copy_region{
        .srcOffset = 0, .dstOffset = 0, .size = this->size};

    vkCmdCopyBuffer(command_buffer, src, dst, 1, &copy_region);

    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo sumbit_info = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                                .pNext = NULL,
                                .waitSemaphoreCount = {},
                                .pWaitSemaphores = {},
                                .pWaitDstStageMask = {},
                                .commandBufferCount = 1,
                                .pCommandBuffers = &command_buffer,
                                .signalSemaphoreCount = {},
                                .pSignalSemaphores = {}};

    vkQueueSubmit(this->queue, 1, &sumbit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(this->queue);

    vkFreeCommandBuffers(device, this->command_pool, 1, &command_buffer);
    this->command_buffer = VK_NULL_HANDLE;
    return true;
  }

  bool create_buffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags mem_properties) {
    VkBufferCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .size = this->size,
        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr};

    if (vkCreateBuffer(device, &create_info, nullptr, &this->handle) !=
        VK_SUCCESS) {
      return false;
    }
    this->mem_requirements.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
    this->mem_requirements.pNext = NULL;

    this->mem_info = {.sType =
                          VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2,
                      .pNext = NULL,
                      .buffer = this->handle};
    vkGetBufferMemoryRequirements2(this->device, &mem_info, &mem_requirements);

    VkMemoryAllocateInfo alloc_info{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = NULL,
        .allocationSize = mem_requirements.memoryRequirements.size,
        .memoryTypeIndex =
            find_memory_type(mem_requirements.memoryRequirements.memoryTypeBits,
                             mem_properties)};

    if (vkAllocateMemory(this->device, &alloc_info, nullptr, &this->memory) !=
        VK_SUCCESS) {
      return false;
    }

    vkBindBufferMemory(this->device, this->handle, this->memory, 0);
    return true;
  }

  bool create_staging_buffer() {

    VkBufferCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .size = this->size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr};

    if (vkCreateBuffer(device, &create_info, nullptr, &this->staging_buffer) !=
        VK_SUCCESS) {
      return false;
    }
    this->mem_requirements.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
    this->mem_requirements.pNext = NULL;

    this->mem_info = {.sType =
                          VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2,
                      .pNext = NULL,
                      .buffer = this->staging_buffer};
    vkGetBufferMemoryRequirements2(this->device, &mem_info, &mem_requirements);

    VkMemoryAllocateInfo alloc_info{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = NULL,
        .allocationSize = mem_requirements.memoryRequirements.size,
        .memoryTypeIndex =
            find_memory_type(mem_requirements.memoryRequirements.memoryTypeBits,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                 VK_MEMORY_PROPERTY_HOST_CACHED_BIT)};

    if (vkAllocateMemory(this->device, &alloc_info, nullptr,
                         &this->staging_buffer_memory) != VK_SUCCESS) {
      return false;
    }

    vkBindBufferMemory(this->device, this->staging_buffer,
                       this->staging_buffer_memory, 0);
    return true;
  }

  void clear_staging_buffer() {
    vkDestroyBuffer(this->device, staging_buffer, nullptr);
    vkFreeMemory(this->device, staging_buffer_memory, nullptr);
  }

  U32 find_memory_type(U32 type_filter, VkMemoryPropertyFlags properties) {
    for (U32 i = 0; i < this->mem_properties->memoryProperties.memoryTypeCount;
         i++) {
      if ((type_filter & i) &&
          (mem_properties->memoryProperties.memoryTypes[i].propertyFlags &
           properties) == properties) {
        return i;
      }
    }
    CFATAL("Failed to find suitable memory type!");
    return -1;
  }

  VkBufferMemoryRequirementsInfo2 mem_info;
  VkMemoryRequirements2 mem_requirements;
  VkBuffer staging_buffer;
  VkDeviceMemory staging_buffer_memory;

  // VkMemoryAllocateFlags memory_usage;
};

// only one texture for texture atlas
struct meh_texture {
  int width=0, height=0, channels=0;
  uint8_t *pixels{nullptr};
  VkDeviceSize size = 0;
  VkImage image{nullptr};
  VkDeviceMemory image_memory{nullptr};
  VkImageView image_view{nullptr};
  VkSampler sampler{nullptr};
  void *data{nullptr};

  meh_texture(const meh_texture &) = delete;
  meh_texture(meh_texture &&other);
  //~meh_texture();
  meh_texture &operator=(const meh_texture &) = delete;
  meh_texture &operator=(meh_texture &&) = delete;

  // load texture from path
  meh_texture(std::string_view texture_path, meh_renderer_cxt &renderer) {
    load_image(texture_path, renderer);
    create();
    free(texture_path);
    create_image_view();
    create_sampler();
  }

  // load texture atlas data from ft_to_atlas lib
  meh_texture(U32 t_width, U32 t_height, uint8_t *pixels,
              meh_renderer_cxt &renderer) {
    this->width = t_width;
    this->height = t_height;
    this->pixels = pixels;
    this->size = t_width * t_height * 4;
    this->device = renderer.device.handle;
    this->command_pool = renderer.command_pool.handle;
    this->queue = renderer.device.graphics_queue;
    this->format = VK_FORMAT_R8G8B8A8_SRGB;
    load_image("", renderer);
    create();
    free("");
    create_image_view();
    create_sampler();
  }

private:
  // U32 queue_index;
  VkQueue queue;
  VkFormat format;
  VkCommandBuffer command_buffer;
  VkCommandPool command_pool;
  VkPhysicalDeviceMemoryProperties2 *mem_properties;
  VkPhysicalDeviceProperties2 *gpu_properties;
  VkDevice device{VK_NULL_HANDLE};
  VkPipelineStageFlags source_stage;
  VkPipelineStageFlags destination_stage;

  bool create_sampler() {
    VkSamplerCreateInfo sampler_info{
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
	.anisotropyEnable = VK_TRUE,
        .maxAnisotropy = gpu_properties->properties.limits.maxSamplerAnisotropy,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = {},
        .maxLod = {},
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
    };
    if (vkCreateSampler(device, &sampler_info, nullptr, &this->sampler) !=
        VK_SUCCESS) {
      // throw std::runtime_error("failed to create texture sampler!");
    }
    return true;
  }

  bool create_image_view(VkImage image, VkFormat format) {
    VkImageViewCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};

    if (vkCreateImageView(device, &create_info, nullptr, &image_view) !=
        VK_SUCCESS) {
      // throw std::runtime_error("failed to create texture image view!");
    }
    return true;
  }

  bool create_image_view() {
    VkImageViewCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = this->format,
        .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};

    if (vkCreateImageView(device, &create_info, nullptr, &this->image_view) !=
        VK_SUCCESS) {
      CFATAL("Failed to create texture image view!");
      return false;
    }

    return true;
  }

  bool load_image(std::string_view texture_path, meh_renderer_cxt &renderer) {
    if (texture_path.size() != 0) {
      this->pixels = stbi_load(texture_path.data(), &this->width, &this->height,
                               &this->channels, STBI_rgb_alpha);
      if (!pixels) {
        CFATAL("Error loading texture %s", texture_path.data());
        return false;
      }
      this->size = width * height * STBI_rgb_alpha;
    }

    this->gpu_properties = &renderer.gpu.properties;
    this->mem_properties = &renderer.gpu.memory_properties;
    return true;
  }

  bool free(std::string_view texture_path) {
    if (texture_path.size() != 0) {
      stbi_image_free(this->pixels);
    }
    return true;
  }

  bool create() {
    create_staging_buffer();

    vkMapMemory(device, staging_buffer_memory, 0, size, 0, &this->data);
    memcpy(this->data, pixels, static_cast<size_t>(size));
    vkUnmapMemory(device, staging_buffer_memory);

    create_image();

    transition_image_layout(VK_IMAGE_LAYOUT_UNDEFINED,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    copy_buffer_to_image();

    transition_image_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    return true;
  }
  bool copy_buffer_to_image() {
    VkCommandBufferAllocateInfo alloc_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = NULL,
        .commandPool = command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1};
    vkAllocateCommandBuffers(device, &alloc_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(command_buffer, &begin_info);

    VkBufferImageCopy region{
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
        .imageOffset = {0, 0, 0},
        .imageExtent = {(U32)this->width, (U32)this->height, 1}};

    vkCmdCopyBufferToImage(command_buffer, staging_buffer, image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo sumbit_info = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                                .pNext = NULL,
                                .waitSemaphoreCount = {},
                                .pWaitSemaphores = {},
                                .pWaitDstStageMask = {},
                                .commandBufferCount = 1,
                                .pCommandBuffers = &command_buffer,
                                .signalSemaphoreCount = {},
                                .pSignalSemaphores = {}};

    vkQueueSubmit(this->queue, 1, &sumbit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(this->queue);

    vkFreeCommandBuffers(device, this->command_pool, 1, &command_buffer);
    this->command_buffer = VK_NULL_HANDLE;

    return true;
  }

  bool transition_image_layout(VkImageLayout old_layout,
                               VkImageLayout new_layout) {
    VkCommandBufferAllocateInfo alloc_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = NULL,
        .commandPool = command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1};
    vkAllocateCommandBuffers(device, &alloc_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(command_buffer, &begin_info);

    VkImageMemoryBarrier barrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = NULL,
        .srcAccessMask = {},
        .dstAccessMask = {},
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};

    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
        new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
      barrier.srcAccessMask = 0;
      barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

      source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
      destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
               new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

      source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
      destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
      // throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(command_buffer, source_stage, destination_stage, 0, 0,
                         nullptr, 0, nullptr, 1, &barrier);

    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo sumbit_info = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                                .pNext = NULL,
                                .waitSemaphoreCount = {},
                                .pWaitSemaphores = {},
                                .pWaitDstStageMask = {},
                                .commandBufferCount = 1,
                                .pCommandBuffers = &command_buffer,
                                .signalSemaphoreCount = {},
                                .pSignalSemaphores = {}};

    vkQueueSubmit(this->queue, 1, &sumbit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(this->queue);

    vkFreeCommandBuffers(device, this->command_pool, 1, &command_buffer);
    this->command_buffer = VK_NULL_HANDLE;

    return true;
  }

  bool create_image() {
    VkImageCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = this->format,
        .extent = {(U32)this->width, (U32)height, 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = {},
        .pQueueFamilyIndices = {},
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED};

    if (vkCreateImage(device, &create_info, nullptr, &image) != VK_SUCCESS) {
      CFATAL("Failed to create texture image!");
      return false;
    }

    this->mem_requirements.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
    this->mem_requirements.pNext = NULL;

    this->mem_info = {.sType =
                          VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2,
                      .pNext = NULL,
                      .buffer = this->staging_buffer};
    vkGetBufferMemoryRequirements2(this->device, &mem_info, &mem_requirements);

    VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = NULL,
        .allocationSize = mem_requirements.memoryRequirements.size,
        .memoryTypeIndex =
            find_memory_type(mem_requirements.memoryRequirements.memoryTypeBits,
                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)};

    if (vkAllocateMemory(device, &alloc_info, nullptr, &image_memory) !=
        VK_SUCCESS) {
      CFATAL("Failed to allocate texture image memory!");
      return false;
    }

    vkBindImageMemory(device, image, image_memory, 0);

    return true;
  }

  bool create_staging_buffer() {

    VkBufferCreateInfo create_info = {.sType =
                                          VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                      .pNext = NULL,
                                      .flags = 0,
                                      .size = this->size,
                                      .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                                      .queueFamilyIndexCount = {},
                                      .pQueueFamilyIndices = {}};

    if (vkCreateBuffer(this->device, &create_info, nullptr,
                       &this->staging_buffer) != VK_SUCCESS) {
      return false;
    }

    this->mem_requirements.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
    this->mem_requirements.pNext = NULL;

    this->mem_info = {.sType =
                          VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2,
                      .pNext = NULL,
                      .buffer = this->staging_buffer};
    vkGetBufferMemoryRequirements2(this->device, &mem_info, &mem_requirements);

    VkMemoryAllocateInfo alloc_info{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = NULL,
        .allocationSize = mem_requirements.memoryRequirements.size,
        .memoryTypeIndex =
            find_memory_type(mem_requirements.memoryRequirements.memoryTypeBits,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                 VK_MEMORY_PROPERTY_HOST_CACHED_BIT)};

    if (vkAllocateMemory(this->device, &alloc_info, nullptr,
                         &this->staging_buffer_memory) != VK_SUCCESS) {
      return false;
    }

    vkBindBufferMemory(this->device, this->staging_buffer,
                       this->staging_buffer_memory, 0);
    return true;
  }

  void clear_staging_buffer() {
    vkDestroyBuffer(this->device, staging_buffer, nullptr);
    vkFreeMemory(this->device, staging_buffer_memory, nullptr);
  }

  U32 find_memory_type(U32 type_filter, VkMemoryPropertyFlags properties) {
    for (U32 i = 0; i < this->mem_properties->memoryProperties.memoryTypeCount;
         i++) {
      if ((type_filter & i) &&
          (mem_properties->memoryProperties.memoryTypes[i].propertyFlags &
           properties) == properties) {
        return i;
      }
    }
    CFATAL("Failed to find suitable memory type!");
    return -1;
  }
  VkBufferMemoryRequirementsInfo2 mem_info;
  VkMemoryRequirements2 mem_requirements;
  VkBuffer staging_buffer;
  VkDeviceMemory staging_buffer_memory;
}; // namespace meh

} // namespace meh

#endif
