#ifndef MEH_WIDGETS_HH
#define MEH_WIDGETS_HH

#include <stdint.h>

typedef uint32_t MEH_QUAD_FLAG;
enum {
  QUAD_FLAG_NONE = (0),
  QUAD_FLAG_FONT = (1<<1),
  QUAD_FLAG_CLICKABLE = (1<<2),
  QUAD_FLAG_ENLARGE_ON_HOVER = (1<<3)
};

typedef uint32_t MEH_QUAD_TYPE;
enum {
  QUAD_TYPE_BACKGROUND,
  QUAD_TYPE_LINEAR_LAYOUT,
  QUAD_TYPE_TEXT_LAYOUT,
  QUAD_TYPE_TEXT_BUTTON,
  QUAD_TYPE_ICON_BUTTON,
  QUAD_TYPE_BOOL_ICON_BUTTON,
  QUAD_TYPE_MENU_ITEMS,
  QUAD_TYPE_MENU_ITEM,  
};

struct QUAD_RECT{
  uint32_t x,y,width,height;
};

struct MEH_QUAD {

  // whether the quad is to be used for presenting font letter or a UI element 
  MEH_QUAD_FLAG flag;
  MEH_QUAD_TYPE type;
  uint32_t id; // ID  of the quad 
  
  uint32_t x, // x position 
    y, // y position 
    width, // width of the quad 
    height; // height of the quad 
  // uv positions on the UI texture Atlas
  float uv_x, uv_y, uv_width, uv_height;
};

typedef U32 MEH_CONSTRAINT_FLAGS;
enum {
    MEH_CONSTRAINTS_NONE = 0,
    MEH_CONSTRAINT_FLAG_BACKGROUND = (1 << 0),
    MEH_CONSTRAINT_TOP_TO_TOP_OF = (1 << 1),
    MEH_CONSTRAINT_TOP_TO_BOTTOM_OF = (1 << 2),
    MEH_CONSTRAINT_BOTTOM_TO_TOP_OF = (1 << 3),
    MEH_CONSTRAINT_BOTTOM_TO_BOTTOM_OF = (1 << 4),
    MEH_CONSTRAINT_RIGHT_TO_LEFT_OF = (1 << 5),
    MEH_CONSTRAINT_RIGHT_TO_RIGHT_OF = (1 << 6),
    MEH_CONSTRAINT_LEFT_TO_LEFT_OF = (1 << 7),
    MEH_CONSTRAINT_LEFT_TO_RIGHT_OF = (1 << 8),
};



#endif
