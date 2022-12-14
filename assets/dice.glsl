@ctype vec3 hmm_vec3
@ctype mat4 hmm_mat4

@vs vs
in vec3 pos;
in vec3 norm;
in vec2 texcoord;
in vec3 inst_pos;
in vec3 inst_col;
in vec3 inst_vel;

uniform vs_params {
    mat4 model;
    mat4 view;
    mat4 projection;
};

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out vec3 Color;

void main() {
    FragPos = vec3(model * vec4(pos, 1.0));
    Normal = mat3(transpose(inverse(model))) * norm;
    gl_Position = projection * view * vec4(FragPos + inst_pos, 1.0);
    TexCoord = vec2(texcoord.x, -texcoord.y);
    Color = inst_col;
}
@end

@fs fs
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 Color;

out vec4 frag_color;

void main() {
    vec2 bl = step(vec2(0.7, 0.7), vec2(TexCoord.x, -TexCoord.y) + fract(gl_FragCoord.xy * vec2(64.f)));
    vec3 tex = vec3(bl.x * bl.y);
    if (tex.x == 0 && tex.y == 0 && tex.z == 0)
        tex = Color;
    frag_color = vec4(tex, 1.0);
}
@end

@program dice vs fs
