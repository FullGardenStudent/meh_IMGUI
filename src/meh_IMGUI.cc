#include "../meh_IMGUI.hh"
#include "../meh_IMGUI_widgets.hh"

#include "./meh_vk/meh_vulkan_objects.hh"

//#include "meh_widgets/meh_widgets.hh"

#ifdef _WIN32
#include "win32/meh_win32.hh"
#endif

#include "meh_vk/meh_vulkan.hh"
#include "meh_allocator.hh"

#ifdef __linux
#include "wl/meh_wayland.hh"
#include <poll.h>
#endif

namespace meh {

meh_mem_pool p(1024 * 1024);
// g_block g(1024 * 1024);


struct meh_window* window = MEH_MEM_POOL_ADD(p, struct meh_window);
    //meh_alloc(g, struct meh_window);
struct push_constants* pc = MEH_MEM_POOL_ADD(p, push_constants);// meh_alloc(g, push_constants);
U32 t_width = 0, t_height = 0;
uint8_t *ui_texture = nullptr;
static bool render_flag;
#ifdef __linux
VkResult res = VK_SUCCESS;
pollfd fds[1];
U32 frame_index = 0;
#endif

void update_push_constants() {
#ifdef _WIN32
  POINT pos = get_mouse_pos();
  *pc = {get_width(window->hwnd), get_height(window->hwnd), (U32)pos.x,
         (U32)pos.y, 0};
#endif
#ifdef __linux
  *pc = {window->width, window->height, (U32)window->x_pos, (U32)window->y_pos};
#endif
}

uint32_t get_screen_width() {
#ifdef _WIN32
  return get_width(window->hwnd);
#endif
#ifdef __linux
  return window->width;
#endif
}
uint32_t get_screen_height() {
#ifdef _WIN32
  return get_height(window->hwnd);
#endif
#ifdef __linux
  return window->height;
#endif
}

bool init_renderer() {
#ifdef _WIN32
  renderer_enabled(); 
  init_vulkan(get_width(window->hwnd), get_height(window->hwnd),
              window->h_instance, window->hwnd,
              MEH_DEBUG_MODE::NONE);
  window->total_time = 0.0;
  window->delta_time = 0.0;
  if (!QueryPerformanceFrequency(&window->frequency)) { // get the frequency of the high-resolution performance counter
      std::cerr << "Failed to get performance counter frequency." << std::endl;
      return false;
  }
  
#endif

#ifdef __linux
  // res = (VkResult)0;
  fds[0].fd = wl_display_get_fd(window->display);
  fds[0].events = POLLIN;
  // struct poolfd fds[]={
  //   {wl_display_get_fd(window->display),POLLIN}
  // };
  // initialize vulkan for linux
  // renderer->swapchain.image_extent = {window->width, window->height};
  init_vulkan(window->display, window->surface, window->width, window->height,
              MEH_DEBUG_MODE::VALIDATION_ONLY);
  // return true;
#endif
  //init_ui();
  // end_resizing();
  //update_push_constants();
  //update_ui(pc->screen_width, pc->screen_height, pc->x_pos, pc->y_pos,pc->quad_id, 0);
  return true;
}

bool init_ui() {
  update_push_constants();
  renderer_init_ui(ui_texture, t_width, t_height, pc->screen_width, pc->screen_height);
#ifdef _WIN32
  render_frame(*pc);
#endif
  return true;
}

bool create_font_texture(std::vector<MEH_TEXTURE_FONT> fonts, uint32_t width,
                         uint32_t height, bool output_texture, U32 offset) {
  std::vector<std::string_view> font_names;
  std::vector<uint32_t> font_sizes;
  for (const auto &f : fonts) {
    font_names.emplace_back(f.font_name);
    font_sizes.emplace_back(f.size);
  }
  ui_texture = create_font_atlas(width, height, font_names, font_sizes, true, offset);
  if (ui_texture == nullptr) {
    //CINFO("found null pixels data!");
  }
  t_width = width;
  t_height = height;
  return true;
}

void resize_window() {
  update_push_constants();
  update_ui(pc->screen_width, pc->screen_height, pc->x_pos, pc->y_pos,pc->quad_id, 0);
  update_quads();
  meh::resize(*pc);
}

  bool load_engine_icons(std::string_view path){
    return load_icons(path);
  }

U32 temp_id = 0;
void render() {
#ifdef _WIN32
  update_push_constants();
  update_ui(pc->screen_width, pc->screen_height,pc->x_pos, pc->y_pos,pc->quad_id, window->delta_time);
  update_quads();
  render_frame( *pc);
#endif
#ifdef __linux
  if (window->render_frame) {
    update_push_constants();
    update_ui(meh::get_screen_width(),meh::get_screen_height(),pc->x_pos, pc->y_pos, pc->quad_id, delta_time);
    update_quads();
    resize(*pc);
    render_frame(*pc);
    window->render_frame = false;
  }
#endif
}

  double get_delta_time(){
    return window->delta_time;
  }

  void calculate_delta_time(){
    QueryPerformanceCounter(&window->previous_time); // get the current timestamp

    window->delta_time = static_cast<double>(window->previous_time.QuadPart - window->current_time.QuadPart) / window->frequency.QuadPart; // calculate the delta time in seconds
    window->total_time = static_cast<double>(window->previous_time.QuadPart) / window->frequency.QuadPart;// -static_cast<double>(window->current_time.QuadPart) / window->frequency.QuadPart;
    //window->total_time += window->delta_time;
  }

  double get_total_time(){
    return window->total_time;
  }

  void* _update_window(void *data){
    return (bool*)update_window();
  }

bool update_window() {
#ifdef _WIN32
  if (PeekMessageW(&window->msg, NULL, 0u, 0u, PM_REMOVE) > 0) {
    TranslateMessage(&window->msg);
    DispatchMessageW(&window->msg);
    if (window->msg.message == WM_QUIT) {
      return 0;
    }
  }//else{
      if (!QueryPerformanceCounter(&window->current_time)) { // get the timestamp for the start of the game loop
          std::cerr << "Failed to get start performance counter." << std::endl;
          return false;
	  // }
  }
  return 1;
#endif

#ifdef __linux
  // while(wl_display_prepare_read(window->display) !=0){
  //   wl_display_dispatch_pending(window->display);
  //   if(wl_display_flush(window->display) <0 && errno != EAGAIN){
  //     wl_display_cancel_read(window->display);
  //     return 1;
  //   }
  //   if(poll(fds,1,0)>0){
  //     wl_display_read_events(window->display);
  //     wl_display_dispatch_pending(window->display);
  //   }else{
  //     wl_display_cancel_read(window->display);
  //   }
  // }
  wl_display_dispatch(window->display);
  return 1;
#endif
}

bool meh_cleanup() {

#ifdef __linux
  return clean_wl(window);
#endif
  return true;
}

// linux stuff
#ifdef __linux
bool create_window(std::string_view title) {
  init_wl(window);
  return true;
}
#endif

// win32 shit
#ifdef _WIN32
// void use_custom_proc(LRESULT (*custom_proc)(HWND, UINT, WPARAM, LPARAM)) {
//   // custom_procw = custom_proc;
//   set_custom_proc(custom_proc);
//   window->using_custom_proc = true;
// }

//bool hit(bool (*top_hit)(uint32_t mousex, uint32_t mousey,
//    
//}

void set_top_hit(hit_fn_ptr hit_fn) {
    set_top_hit_fn(hit_fn);
}

void set_left_hit(hit_fn_ptr hit_fn) {
    set_left_hit_fn(hit_fn);
}
void set_right_hit(hit_fn_ptr hit_fn) {
    set_right_hit_fn(hit_fn);
}
void set_bottom_hit(hit_fn_ptr hit_fn) {
    set_bottom_hit_fn(hit_fn);
}

  void* _create_window (void* arg){
    MEH_WINDOW_INFO* i = (MEH_WINDOW_INFO*)arg;
    bool* b = (bool*)malloc(sizeof(bool));
    *b = create_window(*i);
    return b;
  }  

bool create_window(MEH_WINDOW_INFO &info) {
  init_lib(*window, (U32)info.window_type);
  return meh_window(*window, info.window_title, info.x_position, info.y_position, info.window_width,
                    info.window_height, info.window_type);
}
#endif

} // namespace meh
