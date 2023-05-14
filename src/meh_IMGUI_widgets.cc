#include <algorithm>
#include <bitset>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <stdint.h>
#include <stdio.h>

#include "../meh_IMGUI_widgets.hh"
#include "meh_default_settings.hh"
#include "meh_memory.hh"

#include "ft_to_atlas/ft_to_atlas.hh"

namespace meh {

M_QUAD quads;
MEH_GLYPH glyph;
M_WIDGET widgets;

U32 button_padding = 10;
U32 button_height = 22;

  MEH_QUAD* quad{NULL};
  MEH_QUAD* left_quad{NULL};
  MEH_QUAD* right_quad{NULL};
  MEH_QUAD* top_quad{NULL};
  MEH_QUAD* bottom_quad{NULL};

void draw_background() {
  quads.init();
  glyph.init();
  widgets.init();
  // ll.init();
  MEH_QUAD q = {
      QUAD_FLAG_NONE, QUAD_TYPE_BACKGROUND, 0, 0, 0, 0, 0, 0.0, 0.0, 1.0, 1.0};
  quads.push(q);

  MEH_WIDGET_DATA w = {};
  w.quad = &quads.data[quads.index - 1];
  widgets.push(w);
}

// TODO: add font index and scaling
MEH_WIDGET *button(std::string_view text, uint32_t widget_flag,
                   uint32_t weight) {
  MEH_QUAD q = {QUAD_FLAG_CLICKABLE,
                QUAD_TYPE_TEXT_BUTTON,
                0,
                0,
                0,
                get_text_length(text, 0, 1.0) + 2 * button_padding,
                button_height,
                0.0,
                0.0,
                1.0,
                1.0};
  quads.push(q);
  MEH_WIDGET_DATA w = {};
  w.quad = &quads.data[quads.index - 1];
  w.flag = widget_flag;
  w.p_data = glyph.push(text, QUAD_TYPE_TEXT_LAYOUT, &quads);
  w.weight = weight;
  w.custom = false;

  if (widgets.recording) {
    return widgets.add(w);
  }

  return widgets.push(w);
}

MEH_WIDGET *custom_icon_button(uint32_t width, uint32_t height,
                               MEH_ICON icon_uv, uint32_t widget_flag,
			       uint32_t gravity_flag, uint32_t weight){
  MEH_QUAD q = {QUAD_FLAG_CLICKABLE,
                QUAD_TYPE_ICON_BUTTON,
                0,
                0,
                0,
                width,
                height,
                icon_uv.u,
		icon_uv.v,
                icon_uv.width,
                icon_uv.height};
  quads.push(q);
  MEH_WIDGET_DATA w = {};
  w.quad = &quads.data[quads.index - 1];
  w.flag = widget_flag;
  w.p_data = NULL;
  w.weight = weight;
  w.custom = false;

  if (widgets.recording) {
    return widgets.add(w);
  }
  return widgets.push(w);
}

  MEH_WIDGET *icon_button(uint32_t size, MEH_ICON icon, uint32_t flag, uint32_t weight, uint32_t gravity) {
    return custom_icon_button(size, size, icon, flag,gravity, weight);
}

MEH_WIDGET *linear_layout_start(uint32_t spacing,
                                MEH_LINEAR_LAYOUT_ORIENTATION orientation,
                                uint32_t gravity) {
  MEH_QUAD q = {};
  q.type = QUAD_TYPE_LINEAR_LAYOUT;
  q.flag = QUAD_FLAG_NONE;
  q.uv_x = 0.0, q.uv_y = 0.0, q.uv_width = 1.0, q.uv_height = 1.0;
  quads.push(q);

  MEH_WIDGET_DATA w = {};
  w.quad = &quads.data[quads.index - 1];
  w.constraint_data = NULL;
  
  MEH_LINEAR_LAYOUT_DATA ll{};
  ll.gravity = gravity;
  ll.orientation = orientation;
  ll.spacing = spacing;
  ll.p_next = NULL;
  ll.ll_flag = WRAP_CONTENT;

  w.p_data = widgets.add_p_data(&ll, sizeof(ll));

  // w.p_data = NULL;
  return widgets.start_linear_layout(w);
}

void linear_layout_end() { widgets.end_linear_layout(); }

// calculate linear layout parent dimensions
void ll_parent_dim(MEH_WIDGET_DATA *data) {
  // U32 w=0,h=0;
  MEH_WIDGET_DATA *d = data->first;
  U32 num_nodes = 0;
  U32 wx = 0;
  U32 wy = 0;
  U32 spacing = 0;
  U32 orientation = 0;
  MEH_LINEAR_LAYOUT_DATA *ld = NULL;
  data->weight = 0;

  // if the parent is a linear layout, get its layout properties

  if (data->p_data && data->quad->type == QUAD_TYPE_LINEAR_LAYOUT) {
    ld = (MEH_LINEAR_LAYOUT_DATA *)data->p_data;
    spacing = ld->spacing;
    orientation = ld->orientation;
    ld->max_height = 0;
    ld->max_width = 0;
    // std::cout << "orientation : " << orientation << std::endl;
    // ld->weight = 0;
  }
  // iterate through each widget and calcualte parent's overall width and height
  while (d) {
    if (d->first) {
      ll_parent_dim(d);
    }
    if (orientation) { // vertical orientation
      if (d->flag) {
        // do not do anything about MATCH_PARENT_HEIGHT when orientation is
        // vertical
        if (d->flag & MATCH_PARENT_WIDTH) {
          d->quad->width = data->quad->width;
        }
      }
      wy += d->quad->height + d->margin.TOP + d->margin.BOTTOM;
      wy += spacing;
      wx = MEH_MAX(wx, d->quad->width + d->margin.LEFT + d->margin.RIGHT);
      ld->max_height = MEH_MAX(ld->max_height, d->quad->height + d->margin.TOP +
                                                   d->margin.BOTTOM);
    } else { // horizontal orientation
      if (d->flag) {
        // totally ignore MATCH_PARENT_WIDTH when orientation is horizontal
        if (d->flag & MATCH_PARENT_HEIGHT) {
          d->quad->height = data->quad->height;
        }
      }
      wx += d->quad->width + d->margin.LEFT + d->margin.RIGHT;
      wx += spacing;
      wy = MEH_MAX(wy, d->quad->height + d->margin.TOP + d->margin.BOTTOM);
      ld->max_width = MEH_MAX(ld->max_width, d->quad->width + +d->margin.LEFT +
                                                 d->margin.RIGHT);
    }
    data->weight += d->weight;
    num_nodes++;
    d = d->next;
  }
  (orientation) ? ld->max_width = wx : ld->max_height = wy;

  // check if any width or height are specified beforehand
  // TODO: tested only when both width and height are specified.
  // need to test for when only width or height are specified
  //if (!data->custom) {
    // check if any of the child have weights and adjust dimensions accordingly
    // if (data->weight) {
    //   if (orientation) {
    //     data->quad->width = wx;
    //     data->quad->height =
    //         ld->max_height * num_nodes + spacing * (num_nodes - 1);

    //   } else {
    //     data->quad->width =
    //         ld->max_width * num_nodes + spacing * (num_nodes - 1);
    //     data->quad->height = wy;
    //     std::cout << "orientation!" << std::endl;
    //   }
    //} else {
      data->quad->width = (orientation) ? wx : wx - spacing;
      data->quad->height = (orientation) ? wy - spacing : wy;
      //}
      //}
  //}
  // remove additional spacing incurred from above iterations as per the
  // orientation
}

void calculate_ll_parent() {
  MEH_WIDGET_DATA *temp_data = widgets.root;
  while (temp_data) {
    if (temp_data->first) {
      ll_parent_dim(temp_data);
    }
    temp_data = temp_data->next;
  }
}

// set the position of child elements recursizely
void set_ll_children_pos(MEH_WIDGET_DATA *data) {
  // start from the first widget
  MEH_WIDGET_DATA *d = data->first;

  // get parent's x and y position
  U32 wx = data->quad->x;
  U32 wy = data->quad->y;

  U32 spacing = 0;
  U32 orientation = 0;
  MEH_LINEAR_LAYOUT_DATA *ld;
  // if the parent is a linear layout, get its layout properties
  if (data->p_data && data->quad->type == QUAD_TYPE_LINEAR_LAYOUT) {
    ld = (MEH_LINEAR_LAYOUT_DATA *)data->p_data;
    spacing = ld->spacing;
    orientation = ld->orientation;
  }

  // iterate until last widget and recurse if
  // another parent node is found
  while (d) {
    if (d->first) {
      set_ll_children_pos(d);
    }
    // if (d->weight) {
    //   if (orientation) {
    //     d->quad->height = (uint32_t)(((float)data->quad->height -
    //                                   ((data->weight - 1) * spacing)) *
    //                                  ((float)d->weight / data->weight));
    //   } else {
    //     d->quad->width = (uint32_t)(((float)data->quad->width -
    //                                  ((data->weight - 1) * spacing)) *
    //                                 ((float)d->weight / data->weight));
    //   }
    // }
    
    //  set widget's x and y position as per the orientation
    if (orientation) { // vertical
      d->quad->y = wy + d->margin.TOP;
      wy = d->quad->y + d->quad->height + d->margin.BOTTOM;
      wy += spacing;
      d->quad->x = wx + d->margin.LEFT;
    } else { // horizontal
      d->quad->x = wx + d->margin.LEFT;
      wx = d->quad->width + d->margin.RIGHT + d->quad->x;
      wx += spacing;
      d->quad->y = wy + d->margin.TOP;
    }
    d = d->next;
  }
}

// iterate through root nodes and travers from first
// to last widget if they exist.
void calcualte_ll_child() {
  MEH_WIDGET_DATA *data = widgets.root;
  while (data) {
    if (data->first) {
      set_ll_children_pos(data);
    }
    data = data->next;
  }
}

// traverse through widgets and update any widget specific stuff
void set_widget_properties(MEH_WIDGET_DATA *data) {
  MEH_WIDGET_DATA *d = data;
  while (d) {
    if (d->first) {
      set_widget_properties(d);
    }
    if (d->quad->type == QUAD_TYPE_TEXT_BUTTON) {
        glyph.update_glyph_pos(
          (d->quad->x + d->quad->width / 2) -
              glyph.get_width((MEH_GLYPH::MEH_CHARACTER *)d->p_data) / 2,
          (d->quad->y + d->quad->height / 2) +
              glyph.get_height((MEH_GLYPH::MEH_CHARACTER *)d->p_data) / 2,
          (MEH_GLYPH::MEH_CHARACTER *)d->p_data);
    }
    d = d->next;
  }
}

// iterate through root nodes and traverse through it if it has childern
void update_widget_properties() {
  MEH_WIDGET_DATA *root_node = widgets.root;
  while (root_node) {
    set_widget_properties(root_node);
    root_node = root_node->next;
  }
}

  void set_constraints(MEH_WIDGET_DATA* data){
    
    if(data->constraint_data){
      left_quad = (data->constraint_data->left.quad)? data->constraint_data->left.quad:NULL;
      right_quad = (data->constraint_data->right.quad)? data->constraint_data->right.quad:NULL;
      // left_quad = (data->constraint_data->left.quad)? data->constraint_data->left.quad:NULL;
      // left_quad = (data->constraint_data->left.quad)? data->constraint_data->left.quad:NULL;
    }else{
      left_quad = NULL;
      right_quad = NULL;
    }
    MEH_WIDGET_DATA* d = data->first;
    //left_quad = (data->constraint_data)? data->constraint_data->left.quad:NULL;
    // right_quad = data->constraint_data->right.quad;
    // top_quad = data->constraint_data->top.quad;
    // bottom_quad = data->constraint_data->bottom.quad;
    
    while(data){
      //if(data->first){set_constraints(data);}
      if(data->constraints_flag & MEH_CONSTRAINT_RIGHT_TO_RIGHT_OF){
	quad->x = 10;//right_quad->x + right_quad->width;
	if(data->constraints_flag & MEH_CONSTRAINT_LEFT_TO_RIGHT_OF){
	  quad->x = 20//((left_quad->x + left_quad->width) + quad->x)/2
	    - quad->width/2;
	}
	else if(data->constraints_flag & MEH_CONSTRAINT_LEFT_TO_LEFT_OF){
	  quad->x = 30;//(left_quad->x + quad->x)/2 - quad->width/2;
	}	
      }
      
    }
  }

  // TODO: implement default conditions for edge cases,
  // like when x or y position becomes less than 0
  void update_constraints(){
    MEH_WIDGET_DATA* data = widgets.root;
    while(data){
      quad = data->quad;
      if(data->constraints_flag & MEH_CONSTRAINT_RIGHT_TO_RIGHT_OF){
	right_quad = data->constraint_data->right.quad;
	left_quad = data->constraint_data->left.quad;
        quad->x = right_quad->x + right_quad->width - quad->width
	  - data->constraint_data->right.offset - data->margin.RIGHT;
	if(data->constraints_flag & MEH_CONSTRAINT_LEFT_TO_RIGHT_OF){
	  quad->x = ((left_quad->x + left_quad->width) + right_quad->x + right_quad->width)/2
	    - (quad->width + data->margin.LEFT + data->margin.RIGHT)/2;
	}
	else if(data->constraints_flag & MEH_CONSTRAINT_LEFT_TO_LEFT_OF){
	  quad->x = (left_quad->x + quad->x)/2 - quad->width/2 - data->margin.LEFT - data->margin.RIGHT;
	}
      }
      else if(data->constraints_flag & MEH_CONSTRAINT_RIGHT_TO_LEFT_OF){
	quad->x = right_quad->x + data->constraint_data->right.offset + data->margin.LEFT;
	right_quad = data->constraint_data->right.quad;
	left_quad = data->constraint_data->left.quad;
	if(data->constraints_flag & MEH_CONSTRAINT_LEFT_TO_RIGHT_OF){
	  quad->x = ((left_quad->x + left_quad->width) + quad->x)/2 -
	    quad->width/2 - data->margin.RIGHT;
	}
	if(data->constraints_flag & MEH_CONSTRAINT_LEFT_TO_LEFT_OF){
	  quad->x = (left_quad->x + quad->x)/2 - quad->width/2;
	}	
      }
      else if(data->constraints_flag & MEH_CONSTRAINT_LEFT_TO_RIGHT_OF){
	left_quad = data->constraint_data->left.quad;
	quad->x = left_quad->x + left_quad->width;
      }
      else if(data->constraints_flag & MEH_CONSTRAINT_LEFT_TO_LEFT_OF){
	quad->x = data->constraint_data->left.quad->x;
      }
      if(data->constraints_flag & MEH_CONSTRAINT_BOTTOM_TO_BOTTOM_OF){
	top_quad = data->constraint_data->top.quad;
	bottom_quad = data->constraint_data->bottom.quad; 
	quad->y = bottom_quad->y + bottom_quad->height - data->margin.BOTTOM;
	if(data->constraints_flag & MEH_CONSTRAINT_TOP_TO_BOTTOM_OF){
	  quad->y = ((top_quad->y + top_quad->height) + quad->y)/2 - quad->width/2 - data->margin.BOTTOM;
	}
	if(data->constraints_flag & MEH_CONSTRAINT_TOP_TO_TOP_OF){
	  quad->y = (top_quad->y + quad->y)/2 - quad->width/2 - data->margin.BOTTOM;
	}
      }
      else if(data->constraints_flag & MEH_CONSTRAINT_BOTTOM_TO_TOP_OF){
	top_quad = data->constraint_data->top.quad;
	bottom_quad = data->constraint_data->bottom.quad; 
	quad->y = bottom_quad->y + data->margin.BOTTOM;
	if(data->constraints_flag & MEH_CONSTRAINT_TOP_TO_BOTTOM_OF){
	  quad->y = ((top_quad->y + top_quad->height) + quad->y)/2 -
	    quad->width/2 + data->margin.BOTTOM;
	}
	if(data->constraints_flag & MEH_CONSTRAINT_TOP_TO_TOP_OF){
	  quad->y = (top_quad->y + quad->y)/2 - quad->width/2 + data->margin.BOTTOM;
	}
      }
      else if(data->constraints_flag & MEH_CONSTRAINT_TOP_TO_BOTTOM_OF){
	top_quad = data->constraint_data->top.quad;
	quad->y = top_quad->y + top_quad->height + data->margin.TOP;
      }
      else if(data->constraints_flag & MEH_CONSTRAINT_TOP_TO_TOP_OF){
	quad->y = data->constraint_data->top.quad->y + data->margin.TOP ;
      }
      //set_constraints(data);
      data = data->next;
    }
  }
  

// TODO: update constraints
void update_widgets() {
  calculate_ll_parent();
  update_constraints();
  calcualte_ll_child();
  update_widget_properties();
  // widgets.disassemble();
  // quads.disassemble();
}

void update_ui(uint32_t screen_width, uint32_t screen_height, uint32_t mouse_x,
               uint32_t mouse_y, uint32_t &quad_id, double delta_time) {
  // dt = delta_time;
  quads.data[0].width = screen_width;
  quads.data[0].height = screen_height;
  // quad_id = update_input(mouse_x, mouse_y);
  update_widgets();
}

void *get_init_layout(U32 &num_quads, uint32_t screen_width,
                      uint32_t screen_height) {
  num_quads = quads.index;
  return quads.data;
}

uint32_t get_quads_size() { return sizeof(MEH_QUAD) * quads.index; }

void *ui_data() { return quads.data; }

void set_width(uint32_t id, uint32_t width) {
  if (id & 0x1) {
    widgets.root[uint32_t(id >> 4)].quad->width = width;
    widgets.root[uint32_t(id >> 4)].custom = true;
  } else {
    widgets.data[uint32_t(id >> 4)].quad->width = width;
    widgets.data[uint32_t(id >> 4)].custom = true;
  }
}

void set_height(uint32_t id, uint32_t height) {
  if (id & 0x1) {
    widgets.root[uint32_t(id >> 4)].quad->height = height;
    widgets.root[uint32_t(id >> 4)].custom = true;
  } else {
    widgets.data[uint32_t(id >> 4)].quad->height = height;
    widgets.data[uint32_t(id >> 4)].custom = true;
  }
}

void set_X_position(uint32_t id, uint32_t X) {
  if (id & 0x1) {
    widgets.root[uint32_t(id >> 4)].quad->x = X;
  } else {
    widgets.data[uint32_t(id >> 4)].quad->x = X;
  }
}

void set_Y_position(uint32_t id, uint32_t Y) {
  if (id & 0x1) {
    widgets.root[uint32_t(id >> 4)].quad->y = Y;
  } else {
    widgets.data[uint32_t(id >> 4)].quad->y = Y;
  }
}

uint32_t get_X_position(uint32_t id) {
  if (id & 0x1) {
    return widgets.root[id >> 4].quad->x;
  } else {
    return widgets.data[id >> 4].quad->x;
  }
}

uint32_t get_Y_position(uint32_t id) {
  if (id & 0x1) {
    return widgets.root[id].quad->y;
  } else {
    return widgets.data[id].quad->y;
  }
}

void add_padding(uint32_t id, uint32_t padding) {
  if (id & 0x1) {
    widgets.root[id >> 4].padding.TOP = padding;
    widgets.root[id >> 4].padding.BOTTOM = padding;
    widgets.root[id >> 4].padding.LEFT = padding;
    widgets.root[id >> 4].padding.RIGHT = padding;
    widgets.root[id >> 4].quad->width =
      widgets.root[id >> 4].quad->width + padding * 2;
    widgets.root[id >> 4].quad->height =
      widgets.root[id >> 4].quad->height + padding * 2;
  } else {
    widgets.data[id >> 4].padding.TOP = padding;
    widgets.data[id >> 4].padding.BOTTOM = padding;
    widgets.data[id >> 4].padding.LEFT = padding;
    widgets.data[id >> 4].padding.RIGHT = padding;
    widgets.data[id >> 4].quad->width =
    widgets.data[id >> 4].quad->width + padding * 2;
    widgets.data[id >> 4].quad->height =
      widgets.data[id >> 4].quad->height + padding * 2;
  }
}
void add_left_padding(uint32_t id, uint32_t padding) {
  if (id & 0x1) {
    widgets.root[id >> 4].padding.LEFT = padding;
    widgets.root[id >> 4].quad->width =
      widgets.root[id >> 4].quad->width + padding;
  } else {
    widgets.data[id >> 4].padding.LEFT = padding;
    widgets.data[id >> 4].quad->width =
      widgets.data[id >> 4].quad->width + padding;
  }
}
void add_right_padding(uint32_t id, uint32_t padding) {
  if (id & 0x1) {
    widgets.root[id >> 4].padding.RIGHT = padding;
    widgets.root[id >> 4].quad->width =
      widgets.root[id >> 4].quad->width + padding;
  } else {
    widgets.data[id >> 4].padding.RIGHT = padding;
    widgets.data[id >> 4].quad->width =
      widgets.data[id >> 4].quad->width + padding;
  }
}
void add_top_padding(uint32_t id, uint32_t padding) {
  if (id & 0x1) {
    widgets.root[id >> 4].padding.TOP = padding;
    widgets.root[id >> 4].quad->height =
      widgets.root[id >> 4].quad->height + padding;
  } else {
    widgets.data[id >> 4].padding.TOP = padding;
    widgets.data[id >> 4].quad->height =
      widgets.data[id >> 4].quad->height + padding;
  }
}
void add_bottom_padding(uint32_t id, uint32_t padding) {
  if (id & 0x1) {
    widgets.root[id >> 4].padding.BOTTOM = padding;
    widgets.root[id >> 4].quad->height =
      widgets.root[id >> 4].quad->height + padding;
  } else {
    widgets.data[id >> 4].padding.BOTTOM = padding;
    widgets.data[id >> 4].quad->height =
      widgets.data[id >> 4].quad->height + padding;
  }
}

void add_horizontal_padding(uint32_t id, uint32_t padding) {
  if (id & 0x1) {
    widgets.root[id >> 4].padding.LEFT = padding;
    widgets.root[id >> 4].padding.RIGHT = padding;
    widgets.root[id >> 4].quad->width =
      widgets.root[id >> 4].quad->width + padding*2;
  } else {
    widgets.data[id >> 4].padding.LEFT = padding;
    widgets.data[id >> 4].padding.RIGHT = padding;
    widgets.root[id >> 4].quad->width =
      widgets.root[id >> 4].quad->width + padding*2;
  }
}

void add_vertical_padding(uint32_t id, uint32_t padding) {
  if (id & 0x1) {
    widgets.root[id >> 4].padding.TOP = padding;
    widgets.root[id >> 4].padding.BOTTOM = padding;
    widgets.root[id >> 4].quad->height =
      widgets.root[id >> 4].quad->height + padding * 2;
  } else {
    widgets.data[id >> 4].padding.TOP = padding;
    widgets.data[id >> 4].padding.BOTTOM = padding;
    widgets.root[id >> 4].quad->height =
      widgets.root[id >> 4].quad->height + padding * 2;
  }
}

void add_margin(uint32_t id, uint32_t margin) {
  if (id & 0x1) {
    widgets.root[id >> 4].margin.TOP = margin;
    widgets.root[id >> 4].margin.LEFT = margin;
    widgets.root[id >> 4].margin.BOTTOM = margin;
    widgets.root[id >> 4].margin.RIGHT = margin;
  } else {
    widgets.data[id >> 4].margin.TOP = margin;
    widgets.data[id >> 4].margin.LEFT = margin;
    widgets.data[id >> 4].margin.BOTTOM = margin;
    widgets.data[id >> 4].margin.RIGHT = margin;
  }
}
void add_left_margin(uint32_t id, uint32_t margin) {
  if (id & 0x1) {
    widgets.root[id >> 4].margin.LEFT = margin;
  } else {
    widgets.data[id >> 4].margin.LEFT = margin;
  }
}
void add_right_margin(uint32_t id, uint32_t margin) {
  if (id & 0x1) {
    widgets.root[id >> 4].margin.RIGHT = margin;
  } else {
    widgets.data[id >> 4].margin.RIGHT = margin;
  }
}
void add_top_margin(uint32_t id, uint32_t margin) {
  if (id & 0x1) {
    widgets.root[id >> 4].margin.TOP = margin;
  } else {
    widgets.data[id >> 4].margin.TOP = margin;
  }
}
void add_bottom_margin(uint32_t id, uint32_t margin) {
  if (id & 0x1) {
    widgets.root[id >> 4].margin.BOTTOM = margin;
  } else {
    widgets.data[id >> 4].margin.BOTTOM = margin;
  }
}

void add_horizontal_margin(uint32_t id, uint32_t margin) {
  if (id & 0x1) {
    widgets.root[id >> 4].margin.LEFT = margin;
    widgets.root[id >> 4].margin.RIGHT = margin;
  } else {
    widgets.data[id >> 4].margin.LEFT = margin;
    widgets.data[id >> 4].margin.RIGHT = margin;
  }
}

void add_vertical_margin(uint32_t id, uint32_t margin) {
  if (id & 0x1) {
    widgets.root[id >> 4].margin.TOP = margin;
    widgets.root[id >> 4].margin.BOTTOM = margin;
  } else {
    widgets.data[id >> 4].margin.TOP = margin;
    widgets.data[id >> 4].margin.BOTTOM = margin;
  }
}
void constraint_widget_right_to_right_of(uint32_t id, uint32_t p_id,
                                         uint32_t margin){
  widgets.add_constraint(MEH_CONSTRAINT_RIGHT_TO_RIGHT_OF, id,p_id,margin);
}
void constraint_widget_right_to_left_of(uint32_t id, uint32_t p_id,
                                        uint32_t margin){
  widgets.add_constraint(MEH_CONSTRAINT_RIGHT_TO_LEFT_OF,id, p_id,margin);
}
void constraint_widget_left_to_right_of(uint32_t id, uint32_t p_id,
                                        uint32_t margin){
  widgets.add_constraint(MEH_CONSTRAINT_LEFT_TO_RIGHT_OF,id, p_id,margin);
}
void constraint_widget_left_to_left_of(uint32_t id, uint32_t p_id,
                                       uint32_t margin){
  widgets.add_constraint(MEH_CONSTRAINT_LEFT_TO_LEFT_OF,id, p_id,margin);
}
void constraint_widget_top_to_top_of(uint32_t id, uint32_t p_id,
                                     uint32_t margin){
  widgets.add_constraint(MEH_CONSTRAINT_TOP_TO_TOP_OF,id, p_id,margin);
}
void constraint_widget_top_to_bottom_of(uint32_t id, uint32_t p_id,
                                        uint32_t margin){
  widgets.add_constraint(MEH_CONSTRAINT_TOP_TO_BOTTOM_OF,id, p_id,margin);
}
void constraint_widget_bottom_to_top_of(uint32_t id, uint32_t p_id,
                                        uint32_t margin){
  widgets.add_constraint(MEH_CONSTRAINT_BOTTOM_TO_TOP_OF,id, p_id,margin);
}
void constraint_widget_bottom_to_bottom_of(uint32_t id, uint32_t p_id,
                                           uint32_t margin){
  widgets.add_constraint(MEH_CONSTRAINT_BOTTOM_TO_BOTTOM_OF,id, p_id,margin);
}


} // namespace meh
