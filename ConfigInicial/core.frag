#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 ourColor;

out vec4 color;

uniform sampler2D texture1;
uniform bool      useTexture;   // true = usa textura, false = usa color solido
uniform vec3      lightPos;     // posicion de la luz
uniform vec3      lightColor;   // color de la luz
uniform vec3      viewPos;      // posicion de la camara

void main()
{
    // --- Componente base: textura o color ---
    vec3 baseColor;
    if (useTexture)
        baseColor = texture(texture1, TexCoord).rgb;
    else
        baseColor = ourColor;

    // --- Iluminacion Phong ---
    // Ambiente (luz minima para que nada quede totalmente negro)
    float ambientStrength = 0.35;
    vec3 ambient = ambientStrength * lightColor * baseColor;

    // Difusa
    vec3 norm     = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff    = max(dot(norm, lightDir), 0.0);
    vec3 diffuse  = diff * lightColor * baseColor;

    // Especular (brillo leve)
    float specularStrength = 0.25;
    vec3 viewDir    = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec      = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular   = specularStrength * spec * lightColor;

    vec3 result = ambient + diffuse + specular;
    color = vec4(result, 1.0);
}
