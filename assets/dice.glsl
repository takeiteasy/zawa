@ctype vec4 vec4
@ctype mat4 mat4

@vs vsDice
in vec3 pos;
in vec3 norm;
in vec2 texcoord;
in vec3 inst_pos;
in vec3 inst_col;

uniform vs_dice_params {
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

@fs fsDice
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 Color;

out vec4 frag_color;

uniform fs_dice_light {
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

void main() {
    vec2 bl = step(vec2(0.7, 0.7), vec2(TexCoord.x, -TexCoord.y) + fract(gl_FragCoord.xy * vec2(64.f)));
    vec3 tex = vec3(bl.x * bl.y);
    vec3 color = tex.x == 0 && tex.y == 0 && tex.z == 0 ? Color.xyz : tex;


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

@program dice vsDice fsDice

