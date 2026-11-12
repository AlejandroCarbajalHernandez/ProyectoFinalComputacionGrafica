#version 330 core
// Fragment shader principal: modelo de iluminación de Phong, reflejos Fresnel (cubemap) y niebla
out vec4 color;

// Propiedades de superficie del objeto (mapas de textura y brillo especular)
struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

// Parámetros de luz direccional global (Sol)
struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// Parámetros de luz puntual con atenuación por distancia
struct PointLight {
    vec3 position;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// Parámetros de luz focal/reflector con cono interno y externo (Faro de bicicleta)
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

// Datos interpolados desde el vertex shader en espacio mundial
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

// Variables uniformes de cámara, fuentes de luz y material
uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight pointLights[NUMBER_OF_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform Material material;

// Mallas SIN textura: Mesh::Draw pone useColor = 1 y meshColor = Kd del .mtl.
// Para todo lo demas (superficies de las escenas, modelos con textura) useColor vale 0.
uniform int  useColor;
uniform vec3 meshColor;

// Brillo propio (para las luces del semaforo)
uniform vec3 emissive;

// Niebla del bosque
uniform float nieblaDensidad;
uniform vec3 nieblaColor;

// Fresnel + reflejo del ambiente (cube map).
// reflectivity = cuanto refleja al mirarlo de frente (0 = sin reflejo, 0.5 = metal plateado).
uniform samplerCube envMap;
uniform float reflectivity;
uniform float materialReflect;   // reflejo propio de cada pieza (por ejemplo, solo el vidrio de un edificio)

// Prototipos. albedo = color base del material (textura o Kd), specMask = brillo especular del material
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 albedo, vec3 specMask);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, vec3 specMask);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, vec3 specMask);

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // Color base del material (textura difusa o color sólido .mtl). Se aplica UNA sola vez.
    vec3 albedo;
    vec3 specMask;
    if (useColor == 1) {
        albedo = meshColor;
        specMask = vec3(0.10);
    } else {
        vec4 texColor = texture(material.diffuse, TexCoords);
        albedo = texColor.rgb;
        if (texColor.a < 0.1 || length(albedo) < 0.02) albedo = vec3(0.85);   // respaldo si no hay imagen
        specMask = texture(material.specular, TexCoords).rgb;
    }

    // 1. Luz direccional (Sol)
    vec3 result = CalcDirLight(dirLight, norm, viewDir, albedo, specMask);

    // 2. Luz de reflector (faro de la bicicleta)
    result += CalcSpotLight(spotLight, norm, FragPos, viewDir, albedo, specMask);

    // 3. Emision de luz (semaforos)
    result += emissive;

    // 4. Fresnel (aproximacion de Schlick) + environment mapping:
    float k = max(reflectivity, materialReflect);
    if (k > 0.0) {
        vec3 R = reflect(-viewDir, norm);
        vec3 entorno = texture(envMap, R).rgb;
        float cosT = clamp(dot(norm, viewDir), 0.0, 1.0);
        float F = k + (1.0 - k) * pow(1.0 - cosT, 5.0);
        result = mix(result, entorno, F);
    }

    // 5. Niebla atmosferica exponencial
    if (nieblaDensidad > 0.0001) {
        float distCam = length(viewPos - FragPos);
        float niebla = 1.0 - exp(-pow(distCam * nieblaDensidad, 2.0));
        result = mix(result, nieblaColor, clamp(niebla, 0.0, 1.0));
    }

    color = vec4(clamp(result, 0.0, 1.0), 1.0);
}

// Calcula las componentes ambiental, difusa y especular (Phong) de la luz direccional
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 albedo, vec3 specMask)
{
    vec3 lightDir = normalize(-light.direction);

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    vec3 ambient  = light.ambient * albedo;
    vec3 diffuse  = light.diffuse * diff * albedo;
    vec3 specular = light.specular * spec * specMask;

    return (ambient + diffuse + specular);
}

// Calcula la contribución de una luz puntual aplicando atenuación cuadrática por distancia
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, vec3 specMask)
{
    vec3 lightDir = normalize(light.position - fragPos);

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 ambient  = light.ambient * albedo;
    vec3 diffuse  = light.diffuse * diff * albedo;
    vec3 specular = light.specular * spec * specMask;

    return (ambient + diffuse + specular) * attenuation;
}

// Calcula la contribución del reflector suavizando los bordes entre cutOff y outerCutOff
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, vec3 specMask)
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

    vec3 ambient  = light.ambient * albedo;
    vec3 diffuse  = light.diffuse * diff * albedo;
    vec3 specular = light.specular * spec * specMask;

    return (ambient + diffuse + specular) * attenuation * intensity;
}