#version 330

out vec4 FragColor;

struct Light {
  vec3 position;
  vec3 direction;
  float cutOff;
  float outerCutOff;
  
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  
  float constant;
  float linear;
  float quadratic;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform Light light;
uniform vec3 plane_color;

#define PI 3.14159265358979323846

vec2 rotate2D(vec2 _st, float _angle){
  _st -= 0.5;
  _st =  mat2(cos(_angle), -sin(_angle),
              sin(_angle), cos(_angle)) * _st;
  _st += 0.5;
  return _st;
}

vec2 tile(vec2 _st, float _zoom){
  _st *= _zoom;
  return fract(_st);
}

float box(vec2 _st, vec2 _size, float _smoothEdges){
  _size = vec2(0.5)-_size*0.5;
  vec2 aa = vec2(_smoothEdges*0.5);
  vec2 uv = smoothstep(_size,_size+aa,_st);
  uv *= smoothstep(_size,_size+aa,vec2(1.0)-_st);
  return uv.x*uv.y;
}

void main() {
  vec3 tex = vec3(box(rotate2D(tile(TexCoords.st + fract(gl_FragCoord.xy * vec2(512.f)), 24.), PI * 0.25), vec2(0.7), 0.01));
  if (tex.x != 1 && tex.y != 1 && tex.z != 1)
    tex = plane_color;
  
  vec3 lightDir = normalize(light.position - FragPos);
  float theta = dot(lightDir, normalize(-light.direction));
  
  if (theta > light.cutOff) {
    vec3 ambient = light.ambient * tex;
    
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * tex;
    
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 4.f);
    vec3 specular = light.specular * spec * vec3(.5f);
    
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = (light.cutOff - light.outerCutOff);
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    diffuse  *= intensity;
    specular *= intensity;
    
    float distance    = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    diffuse   *= attenuation;
    specular *= attenuation;
    
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
  }
//  else if (theta > light.outerCutOff)
//    FragColor = vec4(vec3(.08f * theta * 4) * tex, 1.0);
  else
    FragColor = vec4(vec3(.08f * theta * 1.5f) * tex, 1.0);
}
