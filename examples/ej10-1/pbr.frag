#version 430 core

$GLMatrices
$PBRLights

out vec4 FragColor;

in vec2 TexCoords;
in vec3 ecPosition;
in vec3 ecNormal;

// Parámetros del material
uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;


// Basado en https://learnopengl.com/PBR/Lighting

const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
float DistributionGGX(float NdotH, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, 0.000001); // Para evitar divisiones por cero
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / max(denom, 0.000001);
}
// ----------------------------------------------------------------------------
float GeometrySmith(float NdotV, float NdotL, float roughness)
{
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}
// ----------------------------------------------------------------------------
void main()
{		
    vec3 N = normalize(ecNormal);
    vec3 V = normalize(-ecPosition);

    // Calcular el color especular. Para materiales dieléctricos, usar 0.04 (monocromo),
    // para los metales, usar su albedo
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);
    for(int i = 0; i < lights.length(); ++i) 
    {
        if (lights[i].enabled == 0) continue;
        float distance = length(lights[i].positionEye.xyz - ecPosition);
        vec3 L = (lights[i].positionEye.xyz - ecPosition)/distance;
        vec3 H = normalize(V + L);

        float NdotH = max(dot(N, H), 0.0);
        float NdotV = max(dot(N, V), 0.0);
        float NdotL = max(dot(N, L), 0.0);
        float HdotV = max(dot(H, V), 0.0);

        // De manera realista, deberíamos usar exclusivamente una atenuación con el cuadrado de la distancia,
        // pero así damos más libertad al diseñador
        float attenuation = min(1.0, 1.0 / (distance * distance * lights[i].attenuation.x + distance * lights[i].attenuation.y + lights[i].attenuation.z));
        vec3 radiance = lights[i].scaledColor * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(NdotH, roughness);   
        float G   = GeometrySmith(NdotV, NdotL, roughness);      
        vec3 F    = fresnelSchlick(HdotV, F0);
           
        vec3 nominator    = NDF * G * F; 
        float denominator = 4 * NdotV * NdotL;
        vec3 specular = nominator / max(denominator, 0.001); 
        
        // kS es igual a Fresnel
        vec3 kS = F;
        // La suma de las fracciones de luz reflejadas especular y difusamente debe ser uno
        vec3 kD = vec3(1.0) - kS;
        // Multiplicar kD por la el complemento a uno del valor de metallic, ya que los 
        // metales no tienen reflejo difuso. Interpolar si no es puramente metálico
        kD *= 1.0 - metallic;	  

        // Acumular la radiancia de salida
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;  // Antes ya hemos multiplicado la BRDF por Fresnel (kS)
    }   
    
    // Componente ambiental constante (se debería usar IBL)
    vec3 ambient = vec3(0.03) * albedo * ao;

    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // Corrección gamma 
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color, 1.0);
}
