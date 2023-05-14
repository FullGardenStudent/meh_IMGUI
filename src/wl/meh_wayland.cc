// Author : FullGardenStudent
// Description:
//////////////////////////////////////////////////////////////////////
#include "meh_IMGUI/src/meh_vk/meh_vulkan.hh"
#include "meh_IMGUI/src/wl/xdg-shell-client-protocol.h"
#include <cstdint>
#include <wayland-client-protocol.h>
#include <wayland-util.h>
#ifdef __linux

#include "linux/input-event-codes.h"

#include "meh_wayland.hh"

#include <iostream>
#include <string.h>

namespace meh {

  void kbd_keymap(void *data,
		       struct wl_keyboard *wl_keyboard,
		       uint32_t format,
		       int32_t fd,
		  uint32_t size){
    
  }

  void kbd_enter(void *data,
		      struct wl_keyboard *wl_keyboard,
		      uint32_t serial,
		      struct wl_surface *surface,
		 struct wl_array *keys){}

  void kbd_leave(void *data,
		      struct wl_keyboard *wl_keyboard,
		      uint32_t serial,
		 struct wl_surface *surface){}
	
  void kbd_key(void *data,
		    struct wl_keyboard *wl_keyboard,
		    uint32_t serial,
		    uint32_t time,
		    uint32_t key,
	       uint32_t state){}

  void kbd_modifiers(void *data,
			  struct wl_keyboard *wl_keyboard,
			  uint32_t serial,
			  uint32_t mods_depressed,
			  uint32_t mods_latched,
			  uint32_t mods_locked,
		     uint32_t group){}

  void kbd_repeat_info(void *data,
			    struct wl_keyboard *wl_keyboard,
			    int32_t rate,
		       int32_t delay){}
	
	
  wl_keyboard_listener kbd_lsnr = {
    .keymap = kbd_keymap,
    .enter = kbd_enter,
    .leave = kbd_leave,
    .key = kbd_key,
    .modifiers = kbd_modifiers,
    .repeat_info = kbd_repeat_info
  };

  void pointer_enter(void *data,
		      struct wl_pointer *wl_pointer,
		      uint32_t serial,
		      struct wl_surface *surface,
		      wl_fixed_t surface_x,
		     wl_fixed_t surface_y){
    std::cout << "pointer enter!" << std::endl;
  }

  void pointer_leave(void *data,
		      struct wl_pointer *wl_pointer,
		      uint32_t serial,
		     struct wl_surface *surface){
    std::cout << "pointer left!" << std::endl;
  }

  void pointer_motion(void *data,
		       struct wl_pointer *wl_pointer,
		       uint32_t time,
		      wl_fixed_t surface_x,
		      wl_fixed_t surface_y){
    meh_window *window = (meh_window*)data;
    window->x_pos = wl_fixed_to_double(surface_x);
    window->y_pos = wl_fixed_to_double(surface_y);
  }

  void pointer_button(void *data,
		       struct wl_pointer *wl_pointer,
		       uint32_t serial,
		       uint32_t time,
		       uint32_t button,
		      uint32_t state){
    if(button == BTN_LEFT){std::cout << "left mouse button pressed!, state: " << state << std::endl;}
    
    else if(button == BTN_RIGHT){std::cout << "left mouse right pressed!" << std::endl;}
    else if(button == BTN_FORWARD){std::cout << "left mouse forward pressed!, state : " << state << std::endl;} 
  }

  void pointer_axis(void *data,
		     struct wl_pointer *wl_pointer,
		     uint32_t time,
		     uint32_t axis,
		    wl_fixed_t value){
  }
	
  void pointer_frame(void *data,
		     struct wl_pointer *wl_pointer){}


  void pointer_axis_source(void *data,
			    struct wl_pointer *wl_pointer,
			   uint32_t axis_source){
  }
	

  void pointer_axis_stop(void *data,
			  struct wl_pointer *wl_pointer,
			  uint32_t time,
			 uint32_t axis){}
	
  void pointer_axis_discrete(void *data,
			      struct wl_pointer *wl_pointer,
			      uint32_t axis,
			     int32_t discrete){}
   
  void pointer_axis_value120(void *data,
			      struct wl_pointer *wl_pointer,
			      uint32_t axis,
			     int32_t value120){}
  
wl_pointer_listener pointer_lsnr = {
    .enter = pointer_enter,
    .leave = pointer_leave,
    .motion = pointer_motion,
    .button = pointer_button,
    .axis = pointer_axis,
    .frame = pointer_frame,
    .axis_source = pointer_axis_source,
    .axis_stop = pointer_axis_stop,
    .axis_discrete = pointer_axis_discrete,
    .axis_value120 = pointer_axis_value120
  };

  void seat_capabilities(void *data,
			 struct wl_seat *wl_seat,
			 uint32_t capabilities){
    meh_window *window = (meh_window *)data;

    bool have_pointer = capabilities & WL_SEAT_CAPABILITY_POINTER;

    if(have_pointer && window->pointer == NULL){
      window->pointer = wl_seat_get_pointer(window->seat);
      wl_pointer_add_listener(window->pointer, &pointer_lsnr, window);
    }else if(!have_pointer && window->pointer != NULL){
      wl_pointer_release(window->pointer);
      window->pointer = NULL;
    }
  }

  void seat_name(void *data,
		 struct wl_seat *wl_seat,
		 const char *name){
    std::cerr << "seat name : " << name << std::endl;
  }

  wl_seat_listener seat_lsnr = {
    .capabilities = seat_capabilities,
    .name = seat_name
  };
  

  void registry_global(void *data, struct wl_registry *wl_registry, uint32_t name,
		       const char *interface, uint32_t version) {
    meh_window *window = (meh_window *)data;
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
      window->compositor =
	(wl_compositor *)wl_registry_bind(window->registry, name, &wl_compositor_interface, 4);
    } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
      window->shell = (xdg_wm_base *)wl_registry_bind(window->registry, name,
						      &xdg_wm_base_interface, 1);
    } else if (strcmp(interface, wl_seat_interface.name) == 0) {
      window->seat =
        (wl_seat *)wl_registry_bind(wl_registry, name, &wl_seat_interface, 7);
      wl_seat_add_listener(window->seat, &seat_lsnr, window);
    } 
  }

void registry_global_remove(void *data, struct wl_registry *wl_registry,
                            uint32_t name) {}

wl_registry_listener registry_lsnr = {.global = registry_global,
                                      .global_remove = registry_global_remove};

void xdg_surface_lsnr_configure(void *data, struct xdg_surface *xdg_surface,
                                uint32_t serial) {
  meh_window *window = (meh_window *)data;
  xdg_surface_ack_configure(xdg_surface, serial);
}

  xdg_surface_listener xdg_surface_lsnr{.configure = xdg_surface_lsnr_configure};


void xdg_toplevel_configure(void *data, struct xdg_toplevel *xdg_toplevel,
                            int32_t width, int32_t height,
                            struct wl_array *states) {
  struct meh_window *window = (meh_window *)data;
  
  window->width = width;
  window->height = height;
  
  if(!window->render_frame){
    window->render_frame = true;
  }
}

  void xdg_toplevel_close(void *data, struct xdg_toplevel *xdg_toplevel) {std::cout << "exit" << std::endl; exit(0);}

void xdg_toplevel_configure_bonds(void *data,
				 struct xdg_toplevel *xdg_toplevel,
				 int32_t width,
				  int32_t height){}

  void xdg_topevel_wm_capabilities(void *data,
				   struct xdg_toplevel *xdg_toplevel,
				   struct wl_array *capabilities){
  }

  xdg_toplevel_listener xdg_toplevel_lsnr = {.configure = xdg_toplevel_configure,
					     .close = xdg_toplevel_close};

bool init_wl(meh_window *window) {
  window->display = wl_display_connect(NULL);
  window->registry = wl_display_get_registry(window->display);
  wl_registry_add_listener(window->registry, &registry_lsnr, window);
  wl_display_roundtrip(window->display);

  window->surface = wl_compositor_create_surface(window->compositor);
  window->xdg_surface =
      xdg_wm_base_get_xdg_surface(window->shell, window->surface);
  xdg_surface_add_listener(window->xdg_surface, &xdg_surface_lsnr, window);

  window->toplevel = xdg_surface_get_toplevel(window->xdg_surface);
  xdg_toplevel_add_listener(window->toplevel, &xdg_toplevel_lsnr, window);
  xdg_toplevel_set_title(window->toplevel, "some title");

  wl_surface_commit(window->surface);

  return true;
}

bool clean_wl(meh_window *window) {
  wl_surface_destroy(window->surface);
  wl_display_disconnect(window->display);
  return true;
}

} // namespace meh
#endif
