#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 ourColor;

out vec4 color;

uniform sampler2D texture1;
uniform int       useTexture; // 1 = usa textura, 0 = color solido
uniform int       unlit;      // 1 = sin iluminacion (mural/pantallas)
uniform vec3      lightPos;
uniform vec3      lightColor;
uniform vec3      viewPos;

void main()
{
    // Base: textura o color solido
    vec3 baseColor;
    if (useTexture == 1)
        baseColor = texture(texture1, TexCoord).rgb;
    else
        baseColor = ourColor;

    // Mural y objetos sin luz: color exacto de la textura
    if (unlit == 1) {
        color = vec4(baseColor, 1.0);
        return;
    }

    // Iluminacion Phong
    float ambientStrength = 0.45;
    vec3 ambient = ambientStrength * lightColor * baseColor;

    vec3 norm     = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff    = max(dot(norm, lightDir), 0.0);
    vec3 diffuse  = diff * lightColor * baseColor;

    float specularStrength = 0.18;
    vec3 viewDir    = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec      = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular   = specularStrength * spec * lightColor;

    color = vec4(ambient + diffuse + specular, 1.0);
}
