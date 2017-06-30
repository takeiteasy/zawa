#version 330

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;
layout (location = 2) in vec2 tex_coord;
                                       
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

void main() {
  FragPos = vec3(model * vec4(pos, 1.0));
  Normal = mat3(transpose(inverse(model))) * norm;
  gl_Position = projection * view * vec4(FragPos, 1.0);
  TexCoords = vec2(tex_coord.x, -tex_coord.y);
}
