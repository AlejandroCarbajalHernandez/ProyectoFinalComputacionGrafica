#version 330 core
out vec4 color;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define NUMBER_OF_POINT_LIGHTS 1

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight pointLights[NUMBER_OF_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform Material material;
uniform bool useTexture;   // true si el modelo tiene textura
uniform vec3 baseColor;    // color Kd del .mtl cuando no hay textura

vec3 Albedo(vec2 uv)
{
    if (useTexture) return texture(material.diffuse, uv).rgb;
    return baseColor;
}

// Prototipos de funciones
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // 1. Luz direccional (Sol)
    vec3 result = CalcDirLight(dirLight, norm, viewDir);

    // 2. Luz de reflector (Faro de la bicicleta)
    result += CalcSpotLight(spotLight, norm, FragPos, viewDir);

    // El color (textura o Kd) ya se aplico dentro de cada luz
    if (useTexture && texture(material.diffuse, TexCoords).a < 0.1) discard;
    color = vec4(result, 1.0);
}

// Cálculo de Luz Direccional
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    
    // Difuso
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Especular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    
    vec3 ambient = light.ambient * Albedo(TexCoords);
    vec3 diffuse = light.diffuse * diff * Albedo(TexCoords);
    vec3 specular = light.specular * spec * vec3(0.25);
    
    // Respaldo de intensidad si no hay mapa de texturas
    if (length(ambient + diffuse) < 0.02) {
        ambient = light.ambient;
        diffuse = light.diffuse * diff;
        specular = light.specular * spec;
    }
    
    return (ambient + diffuse + specular);
}

// Cálculo de Luz Puntual
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    vec3 ambient = light.ambient * Albedo(TexCoords);
    vec3 diffuse = light.diffuse * diff * Albedo(TexCoords);
    vec3 specular = light.specular * spec * vec3(0.25);
    
    if (length(ambient + diffuse) < 0.02) {
        ambient = light.ambient;
        diffuse = light.diffuse * diff;
        specular = light.specular * spec;
    }
    
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    
    return (ambient + diffuse + specular);
}

// Cálculo de Luz Focal (Spotlight / Faro)
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    
    vec3 ambient = light.ambient * Albedo(TexCoords);
    vec3 diffuse = light.diffuse * diff * Albedo(TexCoords);
    vec3 specular = light.specular * spec * vec3(0.25);
    
    if (length(ambient + diffuse) < 0.02) {
        ambient = light.ambient;
        diffuse = light.diffuse * diff;
        specular = light.specular * spec;
    }
    
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    
    return (ambient + diffuse + specular);
}