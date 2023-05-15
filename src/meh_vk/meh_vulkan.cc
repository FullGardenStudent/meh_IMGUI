#include "meh_vulkan.hh"
#include "../../meh_IMGUI_widgets.hh"
#include "../meh_memory.hh"
#include "../meh_allocator.hh"
#include "meh_vulkan_objects.hh"
#include <cstdint>

namespace meh {
meh_mem_pool vk_g(1024 * 1024);
struct meh_renderer_cxt *renderer = MEH_MEM_POOL_ADD(vk_g, meh_renderer_cxt);

std::vector<const char *> device_extensions = {
    "VK_KHR_swapchain", "VK_KHR_synchronization2", "VK_KHR_create_renderpass2"};

std::vector<const char *> instance_extensions = {
    VK_KHR_SURFACE_EXTENSION_NAME,
    VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME};

bool init_meh_vulkan(U32 width, U32 height);

#ifdef _WIN32
bool init_vulkan(U32 width, U32 height, HINSTANCE h_instance, HWND hwnd,
                 MEH_DEBUG_MODE debug_mode) {

  instance_extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
  renderer->h_module = LoadLibraryW(L"vulkan-1.dll");
  load_vulkan_runtime(renderer->h_module);

  // instance
  renderer->instance.initialize(instance_extensions, debug_mode);
  renderer->instance.load_functions(instance_extensions);

  // surface
  renderer->surface.create(renderer->instance.handle, h_instance, hwnd);

  // TODO: call init_meh_vulkan here
  renderer->gpu.pick(renderer->instance.handle);
  renderer->gpu.populate_gpu_properties(renderer->surface.handle);
  renderer->gpu.acquire_queue_family_indices(renderer->surface.handle);

  // if (renderer->gpu.wayland_support(display)) {
  //   // TODO: log surface not suupported msg
  // }

  init_meh_vulkan(1024, 720);
  return true;
}
#endif

#ifdef __linux

bool init_vulkan(wl_display *display, wl_surface *surface, U32 width,
                 U32 height, MEH_DEBUG_MODE debug_mode) {
  renderer->vulkan_library = dlopen("libvulkan.so.1", RTLD_NOW);
  instance_extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
  if (!renderer->vulkan_library) {
    CFATAL("Failed to load vulkan runtime!");
    return 0;
  } else {
    std::cout << "loaded vulkan runtime!" << std::endl;
  }
  load_vulkan_runtime(renderer->vulkan_library);

  renderer->instance.initialize(instance_extensions, debug_mode);
  renderer->instance.load_functions(instance_extensions);

  renderer->surface.create(renderer->instance.handle, display, surface);

  // gpu
  renderer->gpu.pick(renderer->instance.handle);
  renderer->gpu.populate_gpu_properties(renderer->surface.handle);
  renderer->gpu.acquire_queue_family_indices(renderer->surface.handle);

  if (!renderer->gpu.wayland_support(display)) {
    // TODO: log surface not suupported msg
    std::cout << "Surface is not supported!" << std::endl;
  }

  renderer->swapchain.image_extent = {width, height};

  init_meh_vulkan(1024, 720);
  return true;
}
#endif

bool init_meh_vulkan(U32 width, U32 height) {

  // device
  renderer->device.create(renderer->gpu.handle, renderer->gpu.queue_infos,
                          renderer->gpu.features, device_extensions);

  renderer->device.load_functions(device_extensions);
  renderer->device.get_queues(renderer->gpu.queue_infos);

  // swapchain properties
  // renderer->swapchain.get_surface_capabalities(renderer->gpu.handle,
  //                                             renderer->gpu.surface_info);
  renderer->swapchain.get_surface_properties(renderer->gpu.handle,
                                             renderer->gpu.surface_info);
  renderer->swapchain.choose_surface_properties(width, height);
  renderer->swapchain.is_exclusive(renderer->gpu.queue_infos);

  // swapchain
  renderer->swapchain.create(renderer->surface.handle, renderer->device.handle);

  // render pass
  renderer->frame.render_pass.create(renderer->device.handle,
                                     renderer->swapchain.image_format);

  // frame buffers
  renderer->frame.frame_buffers.create(
      renderer->swapchain.image_views, renderer->swapchain.image_extent,
      renderer->frame.render_pass.handle, renderer->device.handle);

  // command pool
  renderer->command_pool.create(renderer->device.handle,
                                renderer->gpu.queue_infos[0].queueIndex);

  // command buffer
  renderer->frame.set_render_ahead(2);

  renderer->frame.command_buffers.allocate(renderer->device.handle,
                                           renderer->command_pool.handle,
                                           renderer->frame.render_ahead);

  // sync objects
  renderer->frame.create_sync_objects(renderer->device.handle);

  return true;
}

bool vk_font_texture(U32 width, U32 height, uint8_t *p) { return true; }

bool renderer_init_ui(uint8_t *p, U32 texture_width, U32 texture_height,
                      U32 screen_width, U32 screen_height) {
  // uint32_t width, height;
  // uint8_t *p = create_UI_atlas(width, height);
  // if (p == nullptr) {
  //   CINFO("found null pixels data!");
  // }

  meh_texture t(texture_width, texture_height, p, *renderer);

  renderer->pipeline.init(
      "..\\meh_vk_vert", "..\\meh_vk_frag", renderer->swapchain.image_extent,
      renderer->device.handle, renderer->frame.render_pass.handle);

  std::vector<uint16_t> ib = {0, 1, 2, 2, 3, 0}; // index buffer

  meh_buffer index_buffer(ib.data(), sizeof(ib[0]) * ib.size(), INDEX_BUFFER,
                          *renderer);

  MEH_QUAD *q = (MEH_QUAD *)get_init_layout(renderer->frame.quads_count, screen_width,
                                    screen_height);
 
  meh_buffer storage_buffer(q, sizeof(MEH_QUAD) * renderer->frame.quads_count,
                            STORAGE_BUFFER, *renderer);

  // storage_buffer.map(renderer->frame.quads_mapped_memory);
  vkMapMemory(renderer->device.handle, storage_buffer.memory, 0,
              sizeof(MEH_QUAD) * renderer->frame.quads_count, 0,
              &renderer->frame.quads_mapped_memory);

  // std::memcpy(renderer->frame.quads_mapped_memory, q, sizeof(Quad) *
  // renderer->frame.quads_count);

  renderer->descriptor.init(renderer->device.handle,
                            renderer->pipeline.descriptor_set_layout,
                            renderer->frame.render_ahead);

  renderer->descriptor.write(storage_buffer.handle, t.sampler, t.image_view);

  renderer->frame.index_buffer = index_buffer.handle;
  return true;
}

  void update_quads(){
    void *q = ui_data();
    std::memcpy(renderer->frame.quads_mapped_memory,q,get_quads_size());
  }

bool render_frame(push_constants &pc) {

  VkResult res;

  res = renderer->frame.acquire_next_image(renderer->device.handle,
                                           renderer->swapchain.handle);
  if (res == VK_ERROR_OUT_OF_DATE_KHR) {
    resize(pc);
    return false;
  } else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
    CFATAL("FAILED TO OBTAIN SWAPCHAIN IMAGE!!");
    // xthrow std::runtime_error("failed to acquire swap chain image!");
  }

  renderer->frame.record(
      renderer->device.handle, renderer->swapchain.image_extent,
      renderer->pipeline.handle, renderer->pipeline.pipeline_layout,
      renderer->descriptor.descriptor_sets, pc);

  res = renderer->frame.submit_and_present(renderer->device.graphics_queue,
                                           renderer->device.present_queue,
                                           renderer->swapchain.handle);
  if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {
    resize(pc);
    return false;
  } else if (res != VK_SUCCESS) {
    CFATAL("FAILED TO PRESENT SWAPCHAIN IMAGE!!");
    // throw std::runtime_error("failed to present swap chain image!");
  }
  return true;
}

void destroy_swapchain(){
    renderer->device.wait();
    renderer->frame.frame_buffers.destroy(renderer->device.handle);
    renderer->frame.render_pass.destroy(renderer->device.handle);
    vkDestroySwapchainKHR(renderer->device.handle, renderer->swapchain.handle, nullptr);
    for(U32 i=0;i<renderer->frame.render_ahead;i++){
      vkDestroySemaphore(renderer->device.handle,renderer->frame.render_semaphore[i],nullptr);
      vkDestroySemaphore(renderer->device.handle,renderer->frame.present_semaphore[i],nullptr);
      vkDestroyFence(renderer->device.handle,renderer->frame.wait_fence[i],nullptr);
    }
    //vkDestroyDevice(renderer->device.handle, nullptr);
    
  }

void resize(push_constants &pc) {
   renderer->device.wait();
    renderer->frame.frame_buffers.destroy(renderer->device.handle);
    renderer->frame.render_pass.destroy(renderer->device.handle);
#ifdef _WIN32
  renderer->swapchain.get_surface_capabalities(renderer->gpu.handle,
                                               renderer->gpu.surface_info);
#endif
#ifdef __linux
  renderer->swapchain.image_extent = {pc.screen_width, pc.screen_height};
#endif
  renderer->swapchain.create(renderer->surface.handle, renderer->device.handle);

  renderer->frame.render_pass.create(renderer->device.handle,
                                     renderer->swapchain.image_format);

  renderer->frame.frame_buffers.create(
      renderer->swapchain.image_views, renderer->swapchain.image_extent,
      renderer->frame.render_pass.handle, renderer->device.handle);

  
  //update_quads();
  render_frame(pc);
  
}

} // namespace meh
