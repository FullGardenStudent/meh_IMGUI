#ifndef MEH_IMGUI_DEFAULT_SETTINGS
#define MEH_IMGUI_DEFAULT_SETTINGS

#include <array>
#include <string>
#include <stdio.h>

// struct MEH_COLOR{

//   MEH_COLOR(std::string_view hex){
//     sscanf_s(hex.data(),"%02hhx%02hhx%02hhx",&color[0],&color[1],&color[2]);
//   }

//   // get RGB value from hex code and store it in the color array 
//   void rgb(std::string_view hex){
//     sscanf_s(hex.data(),"%02hhx%02hhx%02hhx",&color[0],&color[1],&color[2]);
//   }

//   uint8_t r(){ return color[0];}
//   uint8_t g(){ return color[1];}
//   uint8_t b(){ return color[2];}
//   //uint8_t a(){ return color[3];}//0

//   void disassemble(){
//     printf("r : %d, g : %d, b : %d\n",color[0],color[1],color[2]);
//   }
  
//   // last element is added for performance purpose. Its not used but if needed,
//   // it could be used as an alpha channel
//   uint8_t color[4]={};
// };

struct MEH_DEFAULT_COLORS{

  // MEH_DEFAULT_EDITOR_SETTINGS(){
  //   primary.rgb("245fa6");
  //   on_primary.rgb("ffffff");
  //   primary_container.rgb("d5e3ff");
  //   on_primary_container.rgb("001b3c");
  //   secondary.rgb("555f71");
  //   on_secondary.rgb("ffffff");
  //   secondary_container.rgb("d9e3f8");
  //   on_secondary_container.rgb("121c2b");
  //   teritary.rgb("6e5675");
  //   on_teritary.rgb("ffffff");
  //   teritary_container.rgb("f8d8fe");
  //   on_teritary_container.rgb("28132f");
  // }

  // MEH_COLOR primary;
  // MEH_COLOR on_primary;
  // MEH_COLOR primary_container;
  // MEH_COLOR on_primary_container;
  // MEH_COLOR secondary;
  // MEH_COLOR on_secondary;
  // MEH_COLOR secondary_container;
  // MEH_COLOR on_secondary_container;
  // MEH_COLOR teritary;
  // MEH_COLOR on_teritary;
  // MEH_COLOR teritary_container;
  // MEH_COLOR on_teritary_container;
};

struct MEH_DEFAULT_EDITOR_SETTINGS
{
  //MEH_DEFAULT_EDITOR_SETTINGS:light{}{
    // light = {
    //   .primary.color{"245fa6"},
    //   .on_primary.color = "ffffff",
    //   .primary_container = "d5e3ff",
    //   .on_primary_container = "001b3c",
    //   .secondary = "555f71",
    //   .on_secondary = "ffffff",
    //   .secondary_container = "d9e3f8",
    //   .on_secondary_container = "131c2b",
    //   .teritary = "6e5675",
    //   .on_teritary = "ffffff",
    //   .teritary_container = "f8d8fe",
    //   .on_teritary_container = "28132f",
    // };
  //}
  
  MEH_DEFAULT_COLORS light;
  MEH_DEFAULT_COLORS dark;
};


#endif
