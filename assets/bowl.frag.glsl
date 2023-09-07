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

void main() {
    vec3 lightDir = normalize(light.position - FragPos);
    float theta = dot(lightDir, normalize(-light.direction));
    
    vec3 color = vec3(1.f);
    if (theta > light.cutOff) {
        vec3 ambient = light.ambient * color;
        
        vec3 norm = normalize(Normal);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = light.diffuse * diff * color;
        
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
    else
        FragColor = vec4(vec3(.08f * theta * 1.5f) * color, 1.0);
}
