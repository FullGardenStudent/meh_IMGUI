#version 450

layout (location = 0) in vec2 fragCoord;
layout (location = 1) in flat int quad_flag;
layout (location = 2) in flat int quad_type;
layout (location = 3) in flat int quad_id;
layout (location = 4) in flat int quad_width;
layout (location = 5) in flat int quad_height;
layout (location = 6) in flat int screen_width;
layout (location = 7) in flat int screen_height;
layout (location = 8) in flat int mouse_x;
layout (location = 9) in flat int mouse_y;
layout (location = 10) in flat int m_quad_id;

layout(location = 0) out vec4 out_color;

layout(binding = 1) uniform sampler2D texSampler;

float roundedBoxSDF(vec2 CenterPosition, vec2 Size, float Radius) {
  return length(max(abs(CenterPosition)-Size+Radius,0.0))-Radius;
}

void main(){
  vec2 uv = vec2(fragCoord);
  vec2 size = vec2(quad_width, quad_height);
  vec2 s = fragCoord * size;
  vec2 l = vec2(0.0);
  float radius = 8.0;// range is 0 to 9
  float softness = 1.0f;

  // check if quad is a font.
  if(quad_flag == (1<<1)){
    if(quad_type == 2){
      if(uv.x > 0.98 || uv.x < 0.02 || uv.y < 0.1 || uv.y > 0.9){
        out_color = vec4(1.0,0.1,0.1,1.0);
      }else{
	out_color = vec4(0.0);
      }
    }
    if(quad_type == 3){
      out_color =texture(texSampler, uv) * vec4(0.8,0.8,0.8,1.0);
    }
    else{
      out_color =texture(texSampler, uv) * vec4(1.0,1.0,1.0,1.0);
    }
  }
  // if the quad is not a font,draw it accordingly
  else{
    // background quad
    if(quad_type == 0){
      //uv.x = screen_width/ screen_height;
      vec2 pos = vec2(mouse_x / quad_width, mouse_y / quad_height) - uv;
      //vec2 pos = vec2(mouse_x * screen_width, mouse_y * screen_height);
      pos.y /= screen_width/screen_height;
      float d = 0.05/length(pos);
      d *= 0.01;
      //d = pow(d,0.9);
      //vec3 col = d * vec3(1.0, 0.5, 0.25);
      //col = 1.0 - exp(-col);
      //out_color = vec4(vec3(d), 1.0);
      out_color = vec4(vec3(0.0),1.0);
    }

    // text layout
    else if(quad_type == 2){
      size *= uv;
      if(uv.x > 0.98 || uv.x < 0.02 || uv.y < 0.1 || uv.y > 0.9){
        out_color = vec4(1.0,0.1,0.1,1.0);
      }else{
	out_color = vec4(0.0);
      }
    }
    
    else if(quad_type == 1){ // linear layout 
      if(uv.x > 0.99 || uv.x < 0.99 || uv.y < 0.1 || uv.y > 0.1){
      //if(step(uv.x, 0.1)){
      if(m_quad_id == quad_id){
	out_color = vec4(0.9,0.2,0.9,1.0);
      }
      else{
	out_color = vec4(0.1,1.0,0.1,1.0);
      }
      }else{
	out_color = vec4(0.0);
    }
    }
    
    else if(quad_type == 6){ // MENU_ITEMS
      size -= vec2(2.0,2.0);
      float d = roundedBoxSDF(s - l - (size/2.0) - vec2(1.0,1.0), size/2.0f, radius);
      float s = 1.0f - smoothstep(0.0f,softness , d);
      if(quad_id == m_quad_id){
	out_color = mix(vec4(0.2,0.2,0.2,s), vec4(0.0,0.0,0.0,s),s);
      }else{
	out_color = mix(vec4(0.2,0.2,0.2,s), vec4(0.1,0.1,0.1,s),s);
      }
    }
    else if(quad_type == 7){ // MENU_ITEM
      out_color = vec4(0.0);
    }
    else if(quad_type == 3){ // TEXT_BUTTON
      size -= vec2(2.0,2.0);
      float d = roundedBoxSDF(s - l - (size/2.0) - vec2(1.0), size/2.0f, radius);
      float s = 1.0f - smoothstep(0.0f,softness * 1.8f, d);
      if(quad_id == m_quad_id){
	out_color = mix(vec4(0.2,0.2,0.2,s), vec4(0.0,0.0,0.0,s),s);
      }else{
	out_color = mix(vec4(0.2,0.2,0.2,s), vec4(0.1,0.1,0.1,s),s);
      }
    }
    else if(quad_type == 4){ // ICON_BUTTON
      out_color = texture(texSampler, uv);
    }
  }  
}
