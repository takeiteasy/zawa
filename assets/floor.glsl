@ctype vec4 vec4
@ctype mat4 mat4

@vs vsFloor
in vec3 pos;
in vec3 norm;
in vec2 texcoord;

uniform vs_floor_params {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec4 color;
};

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out vec4 Color;

void main() {
    FragPos = vec3(model * vec4(pos, 1.0));
    Normal = mat3(transpose(inverse(model))) * norm;
    gl_Position = projection * view * vec4(FragPos, 1.0);
    TexCoord = vec2(texcoord.x, -texcoord.y);
    Color = color;
}
@end

@fs fsFloor
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec4 Color;

out vec4 frag_color;

uniform fs_floor_light {
    vec4 LightPosition;
    vec4 LightDirection;
    float LightCutOff;
    float LightOuterCutOff;
  
    vec4 LightAmbient;
    vec4 LightDiffuse;
    vec4 LightSpecular;
  
    float LightConstant;
    float LightLinear;
    float LightQuadratic;
};

#define PI 3.14159265358979323846

vec2 rotate2D(vec2 _st, float _angle){
    return mat2(cos(_angle), -sin(_angle), sin(_angle), cos(_angle)) * (_st - 0.5) + 0.5;
}

vec2 tile(vec2 _st, float _zoom){
    return fract(_st * _zoom);
}

float box(vec2 _st, vec2 _size, float _smoothEdges){
    _size = vec2(0.5)-_size*0.5;
    vec2 aa = vec2(_smoothEdges*0.5);
    vec2 uv = smoothstep(_size,_size+aa,_st) * smoothstep(_size,_size+aa,vec2(1.0)-_st);
    return uv.x*uv.y;
}

void main() {
    vec3 tex = vec3(box(rotate2D(tile(TexCoord.st + fract(gl_FragCoord.xy * vec2(512.f)), 24.), PI * 0.25), vec2(0.7), 0.01));
    vec3 color = tex.x != 1 && tex.y != 1 && tex.z != 1 ? Color.xyz : tex;
    
    vec3 lightDir = normalize(LightPosition.xyz - FragPos);
    float theta = dot(lightDir, normalize(-LightDirection.xyz));
    if (theta > LightCutOff) {
        vec3 ambient = LightAmbient.xyz * color;
    
        vec3 norm = normalize(Normal);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = LightDiffuse.xyz * diff * color;
    
        vec3 viewDir = normalize(vec3(0.f, 2.f, 2.2f) - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 4.f);
        vec3 specular = LightSpecular.xyz * spec * vec3(.5f);
    
        float theta = dot(lightDir, normalize(-LightDirection.xyz));
        float epsilon = (LightCutOff - LightOuterCutOff);
        float intensity = clamp((theta - LightOuterCutOff) / epsilon, 0.0, 1.0);
        diffuse  *= intensity;
        specular *= intensity;
    
        float distance    = length(LightPosition.xyz - FragPos);
        float attenuation = 1.0 / (LightConstant + LightLinear * distance + LightQuadratic * (distance * distance));
        diffuse  *= attenuation;
        specular *= attenuation;
    
        frag_color = vec4(ambient + diffuse + specular, 1.0);
    } else {
        frag_color = vec4(vec3(.07f * theta * 1.5f) * color, 1.0);
    }
}
@end

@program floor vsFloor fsFloor
