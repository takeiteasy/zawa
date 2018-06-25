#version 330

out vec4 FragColor;

struct Material {
  sampler2D diffuse;
  vec3 specular;
  float shininess;
};

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
uniform Material material;
uniform Light light;
uniform vec3 die_color;

void main() {
  vec2 bl = step(vec2(0.7, 0.7), vec2(TexCoords.x, -TexCoords.y) + fract(gl_FragCoord.xy * vec2(64.f)));
  vec3 tex = vec3(bl.x * bl.y);
  if (tex.x == 0 && tex.y == 0 && tex.z == 0)
    tex = die_color;
  
  vec3 lightDir = normalize(light.position - FragPos);
  float theta = dot(lightDir, normalize(-light.direction));
  
  if (theta > light.cutOff) {
    vec3 ambient = light.ambient * tex;
    
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * tex;
    
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.f);
    vec3 specular = light.specular * spec * material.specular;
    
    float distance    = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    diffuse  *= attenuation;
    specular *= attenuation;
    
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
  }
  else
    FragColor = vec4(vec3(.1f) * tex, 1.0);
}
