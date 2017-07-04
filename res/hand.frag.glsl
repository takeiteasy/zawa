#version 330

out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;

uniform sampler2D iChannel0;

void main() {
  FragColor = vec4(texture(iChannel0, TexCoords).rgb, 1.f);
}
