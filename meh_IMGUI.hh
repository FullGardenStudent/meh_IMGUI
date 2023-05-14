#ifndef MEH_IMGUI_H
#define MEH_IMGUI_H

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>


#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#ifdef MEH_USE_RENDERER
// include windows sdk if custom proc is to be suppliled
#define _AMD64_
#define UNICODE
#define NO_MIN_MAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#endif // MEH_USE_CUSTOM_PROC


enum MEH_WINDOW_TYPE
{
  MEH_NORMAL_WINDOW = 0,
  MEH_BORDERLESS_WINDOW = 1
};

#endif // _WIN32

struct MEH_TEXTURE_FONT
{
  std::string_view font_name;
  uint32_t size;
};

typedef bool (*hit_fn_ptr)(uint32_t x, uint32_t y, long top, long left, long right,long bottom);

namespace meh
{
// windows functions
#ifdef _WIN32
bool create_window (std::string_view window_title, uint32_t x_position,
                    uint32_t y_position, uint32_t width, uint32_t height,
                    enum MEH_WINDOW_TYPE window_type);

//bool hit(bool (*top_hit)(uint32_t mousex, uint32_t mousey, 
//    long top, long left, long right));

void set_top_hit(hit_fn_ptr hit_fn);
void set_left_hit(hit_fn_ptr hit_fn);
void set_right_hit(hit_fn_ptr hit_fn);
void set_bottom_hit(hit_fn_ptr hit_fn);

#endif // _WIN32


// linux functions
#ifdef __linux
bool create_window (std::string_view title);
#endif

#ifdef MEH_USE_RENDERER

bool init_renderer ();
bool create_font_texture (std::vector<MEH_TEXTURE_FONT> fonts, uint32_t width,
                          uint32_t height, bool output_texture,
                          uint32_t offset = 0);
bool init_ui ();
void render ();
bool load_engine_icons (std::string_view path);
double get_delta_time ();
void calculate_delta_time ();
double get_total_time ();
#endif // MEH_USE_RENDERER

// returns window width and height
uint32_t get_screen_width ();
uint32_t get_screen_height ();

// called to handle input events the window needs to be updated
bool update_window ();
// called initially once for creating the rexture atlas
//  no point in exposing this
// bool create_UI_atlas();

/*
  Clean up while exitting
 */
bool meh_cleanup ();

} // namespace meh
#ifdef __cplusplus
}
#endif
#endif
