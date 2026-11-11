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

// Mallas SIN textura: Mesh::Draw pone useColor = 1 y meshColor = Kd del .mtl.
// Para todo lo demas (superficies de las escenas, modelos con textura) useColor vale 0.
uniform int  useColor;
uniform vec3 meshColor;

// Niebla: la densidad la fija main.cpp segun el tramo (0 = sin niebla)
uniform float nieblaDensidad;
uniform vec3  nieblaColor;


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

    // Color base del material. IMPORTANTE: se aplica UNA sola vez (antes la textura se multiplicaba dos veces
    // y todo se veia oscuro y sucio).
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

    // 3. Fresnel (aproximacion de Schlick) + environment mapping:
    // mezcla el color del material con el reflejo del cielo; refleja mas cuanto mas de lado se mira.
        float k = max(reflectivity, materialReflect);
    if (k > 0.0) {
        vec3 R = reflect(-viewDir, norm);
        vec3 entorno = texture(envMap, R).rgb;
        float cosT = clamp(dot(norm, viewDir), 0.0, 1.0);
        float F = k + (1.0 - k) * pow(1.0 - cosT, 5.0);
        result = mix(result, entorno, F);
    }

    float distCam = length(viewPos - FragPos);
    float niebla = 1.0 - exp(-pow(distCam * nieblaDensidad, 2.0));
    result = mix(result, nieblaColor, clamp(niebla, 0.0, 1.0));
    color = vec4(clamp(result, 0.0, 1.0), 1.0);
}

// Luz Direccional (Sol)
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

// Luz Puntual
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

// Luz Focal (faro)
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
