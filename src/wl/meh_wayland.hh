#ifndef MEH_WAYLAND_H_
#define MEH_WAYLAND_H_

#include <wayland-client-protocol.h>
#ifdef __linux

#include "wayland-client.h"
#include "xdg-shell-client-protocol.h"

typedef unsigned int U32;

/* Wayland code */
extern "C" struct meh_window {
  /* Globals */
  struct wl_display *display;
  struct wl_registry *registry;
  //struct wl_shm *shm;
  struct wl_compositor *compositor;
  struct xdg_wm_base *shell;
  struct wl_seat *seat;
  struct wl_pointer* pointer;
  struct wl_keyboard* keyboard;
  /* Objects */
  struct wl_surface *surface;
  struct xdg_surface *xdg_surface;
  struct xdg_toplevel *toplevel;
  /* window specs */
  bool render_frame = false;
  U32 width;
  U32 height;
  double x_pos;
  double y_pos;
  double delta_time=0.0,
    total_time = 0.0,
    previous_time,
    current_time;
};

namespace meh {
// init walyalnd client objects
bool init_wl(meh_window *window);

void get_window_state(meh_window &window);

// clean wayland objects
bool clean_wl(meh_window *window);
}; // namespace meh

#endif // __linux
#endif // MEH_WAYLAND_H_
