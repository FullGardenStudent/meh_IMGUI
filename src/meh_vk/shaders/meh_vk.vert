#version 450

layout (location = 0) out vec2 fragCoord;
layout (location = 1) out int quad_flag;
layout (location = 2) out int quad_type;
layout (location = 3) out int quad_id;
layout (location = 4) out int quad_width;
layout (location = 5) out int quad_height;
layout (location = 6) out int screen_width;
layout (location = 7) out int screen_height;
layout (location = 8) out int mouse_x;
layout (location = 9) out int mouse_y;
layout (location = 10) out int m_quad_id;

layout(push_constant) uniform constants{
  int screen_width;
  int screen_height;
  int mouse_x;
  int mouse_y;
  int quad_id;
} pc;

struct Quad{
  int flag,type,id;
  int x, y, width, height;
  float uv_x, uv_y, uv_width, uv_height;
};

layout (set =0, binding = 0)  buffer Buttons{
  Quad quads[];
};

Quad quad = quads[gl_InstanceIndex];

vec3 vertices[4] = vec3[](vec3(quad.x, quad.y, 0.0),
			  vec3(quad.x + quad.width, quad.y, 0.0),
			  vec3(quad.x + quad.width, quad.y + quad.height, 0.0),
			  vec3(quad.x, quad.y + quad.height, 0.0));

vec2 uv[4] = vec2[](vec2(quad.uv_x, quad.uv_y  ),
		    vec2(quad.uv_x + quad.uv_width , quad.uv_y),
		    vec2(quad.uv_x + quad.uv_width, quad.uv_y + quad.uv_height ),
		    vec2(quad.uv_x, quad.uv_y + quad.uv_height ));

void main(){
  vec2 normalizePos = 2.0 * vec2(vertices[gl_VertexIndex].x / pc.screen_width, vertices[gl_VertexIndex].y / pc.screen_height) - 1.0;
  
  gl_Position = vec4(normalizePos, 0.0, 1.0);
  fragCoord = uv[gl_VertexIndex];
  quad_flag = quad.flag;
  quad_type = quad.type;
  quad_id = quad.id;
  quad_width = quad.width;
  quad_height = quad.height;
  screen_width = pc.screen_width;
  screen_height = pc.screen_height;
  mouse_x = pc.mouse_x;
  mouse_y = pc.mouse_y;
  m_quad_id = pc.quad_id;
}
