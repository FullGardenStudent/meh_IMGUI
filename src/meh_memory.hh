#include "../meh_IMGUI_widgets.hh"
#include "ft_to_atlas/ft_to_atlas.hh"
#include "meh_widgets.hh"
#include <cstdlib>
#include <stdio.h>

#include "meh_allocator.hh"

namespace meh {

struct M_QUAD {
  // M_QUAD(){this->quads=MEH_ALLOCATOR(int, this->quads, 0, 1000);}
  //  allocate a size of 1000 quads initially and increment it by 1.5 times
  //  when its full
  void init() {
    data = nullptr;
    index = 0;
    size = 1000;
    data = MEH_ALLOCATE(MEH_QUAD, data, 0, size);
  }

  U32 get_id() {
    index++;
    return (index - 1);
  }

  void push(MEH_QUAD &q) {
    if (index + 1 > size) {
      std::size_t i = size;
      size = INCREASE_SIZE(i);
      data = MEH_ALLOCATE(MEH_QUAD, data, i, size);
    }
    q.id = index;
    data[index] = q;
    index++;
  }

  void disassemble() {
    printf("quads size : %d", (U32)index);
    for (int i = 0; i < index; i++) {
      printf("index : %d\tflag : %d\tx : %d\ty : %d\twidth : %d\theight : "
             "%d\tid : %d\n",
             i, data[i].flag, data[i].x, data[i].y, data[i].width,
             data[i].height, data[i].id);
    }
  }

  void clear() { FREE_ALLOCATION(MEH_QUAD, data, size); }

  std::size_t size;
  std::size_t index;
  MEH_QUAD *data;
};

struct MEH_GLYPH {

  struct MEH_CHARACTER {
    // char* c;
    char c;
    MEH_QUAD *quad;
    MEH_CHARACTER *next;
  };

  void init() {
    data = nullptr;
    size = 1024;
    index_count = 0;
    // stack_index = 0;
    data = MEH_ALLOCATE(MEH_CHARACTER, data, 0, size);
    *data = {0, NULL, NULL};
    index = data + 1;
    *index = {0, NULL, NULL};
  }

  void disassemble() {
    MEH_CHARACTER *t = data + 1;
    while (t) {
      printf("curr : %p\tnext : %p\tquad_id : %d\tchar : %c\n", &*t, t->next,
             t->quad->id, t->c);
      t = t->next;
    }
  }

  MEH_CHARACTER *push(std::string_view text, MEH_QUAD_TYPE quad_type,
                      M_QUAD *quads) {
    MEH_CHARACTER *ret_ptr = index;
    int i = 0;
    for (auto &c : text) {
      Character C = get_char_info(c, 0);
      MEH_QUAD q = {
          QUAD_FLAG_FONT, quad_type,     0,        0,        0,
          C.char_width,   C.char_height, C.char_u, C.char_v, C.uv_width,
          C.uv_height};
      quads->push(q);
      *index = {c, &quads->data[(quads->index) - 1], NULL};
      if (i == 0) {
        i = 1;
      } else {
        (index - 1)->next = index;
      }
      index++;
    }
    //(index-1)->next = NULL;
    return ret_ptr;
  }

  uint32_t get_height(MEH_CHARACTER *text_ptr) {
    uint32_t h = 0;
    MEH_CHARACTER *p = text_ptr;
    while (p) {
      Character C = get_char_info(p->c, 0);
      h = MEH_MAX(C.bearing_y, h);
      p = p->next;
    }
    return h;
  }

  uint32_t get_width(MEH_CHARACTER *text_ptr) {
    uint32_t width = 0;
    MEH_CHARACTER *p = text_ptr;
    while (p) {
      Character C = get_char_info(p->c, 0);
      width += (C.advance >> 6);
      width += C.bearing_x;
      p = p->next;
    }
    return width;
  }

  void update_glyph_pos(U32 x, U32 y, MEH_CHARACTER *text_ptr) {
    U32 wx = x, wy = y;
    MEH_CHARACTER *p = text_ptr;
    while (p) {
      Character C = get_char_info(p->c, 0);
      p->quad->x = wx + C.bearing_x;
      p->quad->y = wy - C.bearing_y;
      wx += (C.advance >> 6);
      p = p->next;
    }
  }

  void update_glyph_pos(U32 x, U32 y, MEH_CHARACTER *text_ptr,
                        MEH_QUAD *parent_quad) {
    U32 wx = x, wy = y, py = 0;
    MEH_CHARACTER *p = text_ptr;
    while (p) {
      Character C = get_char_info(p->c, 0);
      p->quad->x = wx + C.bearing_x;
      ;
      p->quad->y = wy - C.bearing_y;
      py = (py < C.bearing_y) ? C.bearing_y : py;
      // printf("quad_id : %d\tx : %d\ty :
      // %d\n",p->quad->id,p->quad->x,p->quad->y);
      //  wx +=
      //  wy -= C.bearing_y;
      wx += (C.advance >> 6);
      p = p->next;
    }
    p = text_ptr;
    while (p) {
      p->quad->y += py;
      p = p->next;
    }
    parent_quad->width = wx - x;
    parent_quad->x = x;
    parent_quad->y = y;
    parent_quad->height = py;
  }

  MEH_CHARACTER *index;
  std::size_t size, index_count;
  MEH_CHARACTER *data;
};

struct MEH_CONSTRAINT {
  uint32_t offset;
  MEH_QUAD *quad;
};

struct MEH_CONSTRAINT_DATA {
  MEH_CONSTRAINT left, right, top, bottom;
};

struct MEH_BORDER {
  void set(uint32_t value) {
    LEFT = value;
    RIGHT = value;
    TOP = value;
    BOTTOM = value;
  }
  //uint32_t value; // overall margin for top, left, bottom and right
  // uint32_t HORIZONTAL;
  // uint32_t VERTICAL;
  uint32_t LEFT;
  uint32_t RIGHT;
  uint32_t TOP;
  uint32_t BOTTOM;
};

enum { WIDGET_TYPE_TEXT, WIDGET_TYPE_LINEAR_LAYOUT, WIDGET_TYPE_TEXT_BUTTON };

// weight, gravity, type(match parent, wrap content)
struct MEH_LINEAR_LAYOUT_DATA {
  uint32_t ll_flag;
  uint32_t gravity;
  uint32_t spacing;
  MEH_LINEAR_LAYOUT_ORIENTATION orientation;
  void *p_next;
  uint32_t max_width;
  uint32_t max_height;
};

// -> widget data of constraints flag and a pointer to constraints data
// -> margin and padding of the widget
// -> stores the widget id of the widget
// -> weight and gravity properties of the widget while in a linear layout
// -> parent, first, last, next and prev pointers to traverse through widgets
// -> a p_data pointer to store any widget specific data.
// -> a widget_flag to let the widget match parent's width or height 
struct MEH_WIDGET_DATA {
  MEH_CONSTRAINT_FLAGS constraints_flag;
  MEH_BORDER margin;
  MEH_BORDER padding;
  MEH_WIDGET widget;
  uint32_t weight;
  uint32_t custom; // boolean
  uint32_t gravity;
  uint32_t flag;
  MEH_QUAD *quad;
  MEH_CONSTRAINT_DATA *constraint_data;
  void *p_data; // additional widgets specific data
  MEH_WIDGET_DATA *parent;
  MEH_WIDGET_DATA *first;
  MEH_WIDGET_DATA *last;
  MEH_WIDGET_DATA *next;
  MEH_WIDGET_DATA *prev;
};

struct M_WIDGET {

  void init() {
    // allocate data for sotring widget nodes
    data_size = 256, data_count = 0;
    data = MEH_ALLOCATE(MEH_WIDGET_DATA, data, 0, data_size);
    data_index = data;

    // allocate data to store root widget nodes
    root_size = 256, root_count = 0;
    root = MEH_ALLOCATE(MEH_WIDGET_DATA, root, 0, root_size);
    root_index = root;

    // allocate data for stroing constraing properties of widgets.
    constraint_size = 256, constraint_count = 0;
    constraint_data =
        MEH_ALLOCATE(MEH_CONSTRAINT_DATA, constraint_data, 0, constraint_size);
    constraint_index = constraint_data;

    // TODO: consider changing p_data name here as this name is same
    // in both MEH_WIDGET_DATA and M_WIDGET.
    // allocate data for storing widget specific data in p_data
    p_data_size = 1024 * 100, p_data_count = 0;
    p_data = MEH_ALLOCATE(char, p_data, 0, p_data_size);
    p_data_index = p_data;

    // linear layout stuff
    recording = false;
    layer_stack = 0;
  }

  void append_constraint(MEH_WIDGET_DATA* w, MEH_WIDGET_DATA* parent, uint32_t margin){
    *constraint_index = {{},{},{},{}};
    w->constraint_data = constraint_index;
    constraint_index++;
    constraint_count++;
  }

  void add_constraint_data(MEH_WIDGET_DATA* w, MEH_WIDGET_DATA* pw, uint32_t margin){
    //w->constraints_flag |= flag;
    uint32_t flag = w->constraints_flag;
    if((flag & MEH_CONSTRAINT_RIGHT_TO_LEFT_OF) ||
       (flag & MEH_CONSTRAINT_RIGHT_TO_RIGHT_OF)){
      // add constraint data if it already exists
      // and create and assign new one it it does not exist
      if(w->constraint_data){
	w->constraint_data->right = {margin, pw->quad};
      }else{
	append_constraint(w,pw,margin);
	w->constraint_data->right = {margin, pw->quad};
      }
      std::cout << "qid : " << w->quad->id << "\tright id : " << w->constraint_data->right.quad->id << std::endl;
      //std::cout << "constraint d : " << w->constraint_data->right.quad->width << std::endl;
    }
    else if((flag & MEH_CONSTRAINT_LEFT_TO_LEFT_OF) ||
       (flag & MEH_CONSTRAINT_LEFT_TO_RIGHT_OF)){
      if(w->constraint_data){
	std::cout << "constraint data exists!" << std::endl;
	w->constraint_data->left = {margin, pw->quad};
      }else{
	std::cout << "constraint data does not exists!" << std::endl;
	append_constraint(w,pw,margin);
	w->constraint_data->left = {margin, pw->quad};
      }
      std::cout << "qid : " << w->quad->id << "\tleft id : " << w->constraint_data->left.quad->id << std::endl;
      //std::cout << "constraint d : " << w->constraint_data->right.quad->width << std::endl;
    }
    if((flag & MEH_CONSTRAINT_TOP_TO_TOP_OF) ||
       (flag & MEH_CONSTRAINT_TOP_TO_BOTTOM_OF)){
      if(w->constraint_data){
	w->constraint_data->top = {margin, pw->quad};
      }else{
	append_constraint(w,pw,margin);
	w->constraint_data->top = {margin, pw->quad};
      }
    }
    else if((flag & MEH_CONSTRAINT_BOTTOM_TO_BOTTOM_OF) ||
       (flag & MEH_CONSTRAINT_BOTTOM_TO_TOP_OF)){
      if(w->constraint_data){
	w->constraint_data->bottom = {margin, pw->quad};
      }else{
	append_constraint(w,pw,margin);
	w->constraint_data->bottom = {margin, pw->quad};
      }
    }
  }
  void add_constraint(uint32_t constraint_flag, uint32_t id, uint32_t p_id, uint32_t margin ){
    MEH_WIDGET_DATA* w{NULL};
    MEH_WIDGET_DATA* p_w{NULL};
    p_w = (p_id & 0x1)? &root[p_id >> 4] : &data[p_id >> 4];
    w = (id & 0x1)? &root[id >> 4] : &data[id >> 4];
    
    switch(constraint_flag){
    case MEH_CONSTRAINT_RIGHT_TO_RIGHT_OF:
      {
	// if another constraint to right already exist, remove it.
	(w->constraints_flag & MEH_CONSTRAINT_RIGHT_TO_LEFT_OF)
	  ? w->constraints_flag ^= MEH_CONSTRAINT_RIGHT_TO_LEFT_OF
	  : w->constraints_flag |= MEH_CONSTRAINT_RIGHT_TO_RIGHT_OF;
	//add_constraint_data(w,p_w,constraint_flag, margin);
      }break;
    case MEH_CONSTRAINT_RIGHT_TO_LEFT_OF:
      {
	(w->constraints_flag & MEH_CONSTRAINT_RIGHT_TO_RIGHT_OF)
	  ? w->constraints_flag ^= MEH_CONSTRAINT_RIGHT_TO_RIGHT_OF
	  : w->constraints_flag |= MEH_CONSTRAINT_RIGHT_TO_LEFT_OF;
      }break;
    case MEH_CONSTRAINT_LEFT_TO_LEFT_OF:
      {
	(w->constraints_flag & MEH_CONSTRAINT_LEFT_TO_RIGHT_OF)
	  ? w->constraints_flag ^= MEH_CONSTRAINT_LEFT_TO_RIGHT_OF
	  : w->constraints_flag |= MEH_CONSTRAINT_LEFT_TO_LEFT_OF;
      }break;
      case MEH_CONSTRAINT_LEFT_TO_RIGHT_OF:
      {
	(w->constraints_flag & MEH_CONSTRAINT_LEFT_TO_LEFT_OF)
	  ? w->constraints_flag ^= MEH_CONSTRAINT_LEFT_TO_LEFT_OF
	  : w->constraints_flag |= MEH_CONSTRAINT_LEFT_TO_RIGHT_OF;
      }break;
      case MEH_CONSTRAINT_TOP_TO_BOTTOM_OF:
      {
	(w->constraints_flag & MEH_CONSTRAINT_TOP_TO_TOP_OF)
	  ? w->constraints_flag ^= MEH_CONSTRAINT_TOP_TO_TOP_OF
	  : w->constraints_flag |= MEH_CONSTRAINT_TOP_TO_BOTTOM_OF;
      }break;
      case MEH_CONSTRAINT_TOP_TO_TOP_OF:
      {
	(w->constraints_flag & MEH_CONSTRAINT_TOP_TO_BOTTOM_OF)
	  ? w->constraints_flag ^= MEH_CONSTRAINT_TOP_TO_BOTTOM_OF
	  : w->constraints_flag |= MEH_CONSTRAINT_TOP_TO_TOP_OF;
      }break;
      case MEH_CONSTRAINT_BOTTOM_TO_BOTTOM_OF:
      {
	(w->constraints_flag & MEH_CONSTRAINT_BOTTOM_TO_TOP_OF)
	  ? w->constraints_flag ^= MEH_CONSTRAINT_BOTTOM_TO_TOP_OF
	  : w->constraints_flag |= MEH_CONSTRAINT_BOTTOM_TO_BOTTOM_OF;
      }break;
      case MEH_CONSTRAINT_BOTTOM_TO_TOP_OF:
      {
	(w->constraints_flag & MEH_CONSTRAINT_BOTTOM_TO_BOTTOM_OF)
	  ? w->constraints_flag ^= MEH_CONSTRAINT_BOTTOM_TO_BOTTOM_OF
	  : w->constraints_flag |= MEH_CONSTRAINT_BOTTOM_TO_TOP_OF;
      }break;
    }
    std::cout << "flag : " << w->constraints_flag << "\tpid : " << (p_id >> 4) << "\tid : " <<
      (id >> 4) << std::endl;
    add_constraint_data(w,p_w, margin);
  }

  void print_child(MEH_WIDGET_DATA *data) {
    printf("  parent %p\tself %p\tnext : %p\tprev : %p\tfirst : %p\tlast : "
           "%p\tquad_id : %d\twidget_id : %d\n",
           data->parent, &*data, data->next, data->prev, data->first,
           data->last, data->quad->id, (uint32_t)(data->widget.id >> 4));
  }

  void print_root(MEH_WIDGET_DATA *data) {
    printf("self %p\tparent %p\tnext : %p\tprev : %p\tfirst : %p\tlast : "
           "%p\tquad_id : %d\twidget_id : %d\n",
           &*data, data->parent, data->next, data->prev, data->first,
           data->last, data->quad->id, (uint32_t)(data->widget.id >> 4));
  }

  // print widgets info
  void disassemble() {
    MEH_WIDGET_DATA *t_root = root;
    while (t_root) {
      print_root(t_root);
      if (t_root->first) {
        MEH_WIDGET_DATA *d = t_root->first;
        while (d) {
          print_child(d);
          d = d->next;
        }
      }
      t_root = t_root->next;
    }
  }

  void *add_p_data(void *data, uint32_t size) {
    std::memcpy(p_data_index, *&data, size);
    p_data_index += size;
    return p_data_index - size;
  }

  // last bit of widget id is 1 if it is a root node.
  // last bit of widget id is 0 if it is not a root node.
  // all the widgets added via push() will be assigned as root nodes.
  // whilst widgets added via add() might be root or child nodes
  MEH_WIDGET *push(MEH_WIDGET_DATA &w) {
    // assign a widget id to the widget
    uint32_t id = root_count << 4;
    id ^= 0x1;
    w.widget.id = id;
    
    if (root_count > 0) {
      w.prev = &root[root_count - 1];
      w.prev->next = root_index;
      w.next = NULL;
    } else {
      w.prev = NULL;
      w.next = NULL;
    }
    *root_index = w;
    temp_parent = root_index;
    root_index++;
    root_count++;
    return &root[root_count - 1].widget;
  }

  // create a linear layout out of recorded widgets
  MEH_WIDGET *start_linear_layout(MEH_WIDGET_DATA &w) {
    // start appending every widget from now on a child node.
    recording = true;
    w.next = NULL;
    
    // if layer_stack is 0, then the widget should be appended as a root node.
    // else it should be appended as a child node to a root node or another
    // child node
    if (!layer_stack) { // root node
      // assign widget id with last bit as 1 to represent this widget as a
      // parent node
      uint32_t id = root_count << 4;
      id ^= 0x1;
      w.widget.id = id;
      *root_index = w;
      root_index->prev = root_index - 1;
      temp_parent->next = root_index;
      temp_parent = root_index;
      temp_parent->next = NULL;
      root_index++;
      root_count++;
      layer_stack++;
      ;
      return &(root_index - 1)->widget;
    } else { // child node
      // assign widget id with last bit as 0 to represent this widget as a child
      // node
      uint32_t id = data_count << 4;
      id ^= 0x0;
      w.widget.id = id;
      *data_index = w;
      data_index->prev = temp_parent->last;
      data_index->parent = temp_parent;
      if (temp_parent->last) {
        temp_parent->last->next = data_index;
      } else {
        temp_parent->first = data_index;
      }
      temp_parent->last = data_index;
      temp_parent = data_index;
      data_index++;
      data_count++;
      return &(data_index - 1)->widget;
    }
  }

  // if recording is true, any widget created will
  // be added as a child node through this.
  MEH_WIDGET *add(MEH_WIDGET_DATA &w) {
    // fill up any needed widget properties
    U32 id = data_count << 4;
    id ^= 0x0;
    w.parent = temp_parent;
    w.next = NULL;
    w.widget.id = id;
    // check if the widget to add as a new node is not the first
    // node, then mark it as last node.
    if (temp_parent->first) {
      w.prev = temp_parent->last;
      *data_index = w;
      temp_parent->last->next = data_index;
      temp_parent->last = data_index;
    }
    // if the widget to add is the first node then
    // make it the first and last node.
    else {
      *data_index = w;
      temp_parent->first = data_index;
      temp_parent->last = data_index;
    }
    data_index++;
    data_count++;
    return &(data_index - 1)->widget;
  }

  // stop appending widgets as child nodes
  void end_linear_layout() {
    if (recording) {
      layer_stack--;
      //  check if layer_stack is 0 to avoid stopping of
      //  recording while current widgets no where near root node.
      if (layer_stack) {
        temp_parent = temp_parent->parent;
      } else {
        layer_stack = 0;
        recording = false;
      }
    }
  }

  // *_size defines the size and *_count keeps track of number of allocations
  // so that if count approaches the size, memory can be expanded.

  // root nodes
  MEH_WIDGET_DATA *root;
  MEH_WIDGET_DATA *root_index;
  uint32_t root_size, root_count;

  // widget nodes that are not root nodes
  MEH_WIDGET_DATA *data;
  MEH_WIDGET_DATA *data_index;
  uint32_t data_size, data_count;

  // widget constraints data
  MEH_CONSTRAINT_DATA *constraint_data;
  MEH_CONSTRAINT_DATA *constraint_index;
  uint32_t constraint_size, constraint_count;

  // for storing additional widget specific data, if needed.
  // (kind of like pNext in vulkan)
  char *p_data;
  char *p_data_index;
  uint32_t p_data_size, p_data_count;

  // if recording is true, widgets created will be added as child widget node.
  // if recording is false, widget created will be a root node.
  bool recording;
  // to avoid crashin or UB when linear_layout_start and end are misused.
  uint32_t layer_stack;
  // to get a hold of the current parent when needed.
  MEH_WIDGET_DATA *temp_parent;
};

}; // namespace meh
