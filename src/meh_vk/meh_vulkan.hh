#ifndef MEH_VULKAN_H
#define MEH_VULKAN_H

#include "meh_vulkan_objects.hh"
#include "ft_to_atlas/ft_to_atlas.hh"
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

namespace meh {

#ifdef _WIN32
bool init_vulkan(U32 width, U32 height, HINSTANCE h_instance, HWND hwnd,
                 MEH_DEBUG_MODE debug_mode);
#endif

#ifdef __linux
  bool init_vulkan(wl_display *display, wl_surface *surface,U32 width, U32 height,
                  MEH_DEBUG_MODE debug_mode);

#endif

  //bool vk_font_texture(U32 texture_width, U32 texture_height, U32 width, U32 height,uint8_t* p);

  bool renderer_init_ui(uint8_t *p, U32 texture_width, U32 texture_height, U32 screen_width, U32 screen_height);

  bool render_frame( push_constants &pc);

  void destroy_swapchain();
  void update_quads();

  void resize(push_constants &pc);

} // namespace meh


#ifdef __cplusplus
}
#endif


#endif
