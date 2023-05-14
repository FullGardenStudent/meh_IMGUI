#ifndef MEH_WIDGETS
#define MEH_WIDGETS

#include <iostream>
#include <stdint.h>
#include <vector>

/*
  NOTE : Initially there was no intention to expose these below function but
  after concluding to have a webgpu build as well, having the UI widgets that
  works across both webgpu and native(win32api/wayland) builds became a
  necessity. Initialization of the UI renderer is different across each
  platforms and once the initialization is done, all the different builds call
  upon the below functions to draw and present the UI widgets/elements.
*/
#include <cstdint>

#include <sys/types.h>
#ifdef __cplusplus
extern "C" {
#endif
namespace meh {

struct MEH_ICON {
  float u, v, width, height;
};

// TODO: change the below comment
/*
  After the UI renderer is initialized, it calls the init_layout() function to
  to set up an initial UI layout of the engine, which includes things like
  close, minimize, maximize, menu..etc., buttons
*/
void *get_init_layout(uint32_t &num_quads, uint32_t screen_width,
                      uint32_t screen_height);

/*
**Function: reutrns a pointer to the buffer that the UI renderer should use to
draw the entire UI on the screen by taking the current screen dimensions and
cursor position as input. This buffer is expected to be used as a storage buffer
and update/draw the Quads accordingly onto the screen.
**@param width : width of the current window
**@param height : height of the window
**@param x : mouse cursor x-position
**@param y : mouse cursor y-position
*/
void *UI_buffer(int32_t width, int32_t height, int32_t x, int32_t y);

void draw_background();

uint32_t get_quads_size();
void *ui_data();

void update_ui(uint32_t screen_width, uint32_t screen_height, uint32_t mouse_x,
               uint32_t mouse_y, uint32_t &quad_id, double delta_time);

void print_widget(uint32_t index);

// void constraint_left(void* widget, void* parent);
// void constraint_top(uint32_t index,uint32_t *t, void* parent);

void update_main_window(uint32_t screen_width, uint32_t screen_height);

struct MEH_POS {
  uint32_t *x;
  uint32_t *y;
};

// widget constraint functions
void constraint_widget_right_to_right_of(uint32_t id, uint32_t p_id,
                                         uint32_t margin);
void constraint_widget_right_to_left_of(uint32_t id, uint32_t p_id,
                                        uint32_t margin);
void constraint_widget_left_to_right_of(uint32_t id, uint32_t p_id,
                                        uint32_t margin);
void constraint_widget_left_to_left_of(uint32_t id, uint32_t p_id,
                                       uint32_t margin);
void constraint_widget_top_to_top_of(uint32_t id, uint32_t p_id,
                                     uint32_t margin);
void constraint_widget_top_to_bottom_of(uint32_t id, uint32_t p_id,
                                        uint32_t margin);
void constraint_widget_bottom_to_top_of(uint32_t id, uint32_t p_id,
                                        uint32_t margin);
void constraint_widget_bottom_to_bottom_of(uint32_t id, uint32_t p_id,
                                           uint32_t margin);
// widget position and size functions
void set_X_position(uint32_t id, uint32_t X);
void set_Y_position(uint32_t id, uint32_t Y);
uint32_t get_X_position(uint32_t id);
uint32_t get_Y_position(uint32_t id);
void set_width(uint32_t id, uint32_t width);
void set_height(uint32_t id, uint32_t height);

// widget padding functions
void add_padding(uint32_t id, uint32_t padding);
void add_left_padding(uint32_t id, uint32_t padding);
void add_right_padding(uint32_t id, uint32_t padding);
void add_top_padding(uint32_t id, uint32_t padding);
void add_bottom_padding(uint32_t id, uint32_t padding);
void add_horizontal_padding(uint32_t id, uint32_t padding);
void add_vertical_padding(uint32_t id, uint32_t padding);

// widget margin functions
void add_margin(uint32_t id, uint32_t margin);
void add_left_margin(uint32_t id, uint32_t margin);
void add_right_margin(uint32_t id, uint32_t margin);
void add_top_margin(uint32_t id, uint32_t margin);
void add_bottom_margin(uint32_t id, uint32_t margin);
void add_horizontal_margin(uint32_t id, uint32_t margin);
void add_vertical_margin(uint32_t id, uint32_t margin);

void add_flag(uint32_t id, uint32_t flag);

/*
  NOTE: The reason why the default of the below thing is calling a reference is
  because i want to change the posiiton of quad without using an astrisk.
  Now, i can do "window->pos.x = 10;" to set the position instead of
  "*window->pos.x = 10;", avoiding to having to manually dereference
  a pointer every time I want to set the posiiton of a widget.
 */
struct MEH_WIDGET {
  // set x and y positions
  void X(uint32_t x_position) { set_X_position(id, x_position); }
  void Y(uint32_t y_position) { set_Y_position(id, y_position); }

  // get x and y positions
  uint32_t x() { return get_X_position(id); }
  uint32_t y() { return get_Y_position(id); }

  void horizontal_padding(uint32_t padding) {
    add_horizontal_padding(id, padding);
  }

  void vertical_padding(uint32_t padding) { add_vertical_padding(id, padding); }

  void set_flag(uint32_t flag) { add_flag(id, flag); }

  void width(uint32_t width) { set_width(id, width); }
  void height(uint32_t height) { set_height(id, height); }
  // MEH_WIDGET(uint32_t id):id(id){};
  void constraintRight_toLeftOf(MEH_WIDGET *widget, uint32_t margin = 0) {
    constraint_widget_right_to_left_of(id, (widget) ? widget->id : 0x1, margin);
  }
  void constraintRight_toRightOf(MEH_WIDGET *widget, uint32_t margin = 0) {
    constraint_widget_right_to_right_of(id, (widget)? widget->id : 0x1, margin);
  }
  void constraintTop_toTopOf(MEH_WIDGET *widget, uint32_t margin = 0) {
    constraint_widget_top_to_top_of(id, (widget) ? widget->id : 0x1, margin);
  }
  void constraintTop_toBottomOf(MEH_WIDGET *widget, uint32_t margin = 0) {
    constraint_widget_top_to_bottom_of(id, (widget) ? widget->id : 0x1, margin);
  }
  void constraintLeft_toLeftOf(MEH_WIDGET *widget, uint32_t margin = 0) {
    constraint_widget_left_to_left_of(id, (widget) ? widget->id : 0x1, margin);
  }
  void constraintLeft_toRightOf(MEH_WIDGET *widget, uint32_t margin = 0) {
    constraint_widget_left_to_right_of(id, (widget) ? widget->id : 0x1, margin);
  }
  void constraintBottom_toBottomOf(MEH_WIDGET *widget, uint32_t margin = 0) {
    constraint_widget_bottom_to_bottom_of(id, (widget)? widget->id : 0x1, margin);
  }
  void constraintBottom_toTopOf(MEH_WIDGET *widget, uint32_t margin = 0) {
    constraint_widget_bottom_to_top_of(id, (widget)? widget->id:0x1, margin);
  }

  void padding(uint32_t padding = 0) { add_padding(id,padding);}
  void padding_left(uint32_t padding = 0) {add_left_padding(id,padding);}
  void padding_right(uint32_t padding = 0) {add_right_padding(id,padding);}
  void padding_top(uint32_t padding = 0) {add_top_padding(id,padding);}
  void padding_bottom(uint32_t padding = 0) {add_bottom_padding(id,padding);}
  void padding_horizontal(uint32_t padding = 0) {add_horizontal_padding(id,padding);}
  void padding_vertical(uint32_t padding = 0) {add_vertical_padding(id,padding);}

  void margin(uint32_t margin = 0) { add_margin(id, margin); }
  void margin_left(uint32_t margin = 0) { add_left_margin(id, margin); }
  void margin_right(uint32_t margin = 0) { add_right_margin(id, margin); }
  void margin_top(uint32_t margin = 0) { add_top_margin(id, margin); }
  void margin_bottom(uint32_t margin = 0) { add_bottom_margin(id, margin); }
  void margin_horizontal(uint32_t margin = 0) {
    add_horizontal_margin(id, margin);
  }
  void margin_vertical(uint32_t margin = 0) { add_vertical_margin(id, margin); }
  uint32_t id;
};

struct MEH_MENU_ITEM {
  MEH_ICON icon_uv;
  const char *item_name;
  const char *item_shortcut;
};

struct MEH_MENU {
  const char *menu_name;
  std::vector<MEH_MENU_ITEM> items;
};

enum MEH_LINEAR_LAYOUT_ORIENTATION { HORIZONTAL, VERTICAL };

enum {
  WRAP_CONTENT,
  MATCH_PARENT,
};

enum {
  NONE = 0,
  MATCH_PARENT_WIDTH = (1 << 0),
  MATCH_PARENT_HEIGHT = (1 << 1),
};

// linear layout functions
MEH_WIDGET *linear_layout_start(uint32_t spacing,
                                MEH_LINEAR_LAYOUT_ORIENTATION type,
                                uint32_t gravity);
void linear_layout_end();

MEH_WIDGET *create_menu_item(std::vector<MEH_MENU> menu, uint32_t spacing);

struct LinearLayout {
  MEH_WIDGET *start(uint32_t spacing = 0,
                    MEH_LINEAR_LAYOUT_ORIENTATION type = HORIZONTAL,
                    uint32_t layout_gravity = 1) {
    return linear_layout_start(spacing, type, layout_gravity);
  }

  void end() { linear_layout_end(); }
};

bool interpolate(MEH_WIDGET *widget_1, MEH_WIDGET *widget_2);

// MEH_WIDGET* text(std::string_view text);

MEH_WIDGET *button(std::string_view text, uint32_t widget_flag = WRAP_CONTENT,
                   uint32_t weight = 0);
MEH_WIDGET *text_layout(std::string_view text);
  MEH_WIDGET *icon_button(uint32_t size, MEH_ICON icon,uint32_t flag = 0, uint32_t gravity=0, uint32_t weight=0);
MEH_WIDGET *custom_icon_button(uint32_t width, uint32_t height,
                               MEH_ICON icon_uv, uint32_t widget_flag = WRAP_CONTENT,
			       uint32_t gravity_flag =0, uint32_t weight =0);

typedef MEH_WIDGET *widget;
#define MAIN_WINDOW 0x1

} // namespace meh

#ifdef __cplusplus
}
#endif

#endif // MEH_WIDGETS
