#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 ourColor;

out vec4 color;

uniform sampler2D texture1;
uniform int       useTexture;
uniform int       unlit;
uniform vec3      lightPos;
uniform vec3      lightColor;
uniform vec3      viewPos;
uniform float     ambientStr;  // controlado por tecla +/-
uniform float     diffuseStr;  // controlado por tecla R/T

void main()
{
    // Color base
    vec3 baseColor;
    if (useTexture == 1)
        baseColor = texture(texture1, TexCoord).rgb;
    else
        baseColor = ourColor;

    // Sin iluminacion: color exacto (mural, pantallas)
    if (unlit == 1) {
        color = vec4(baseColor, 1.0);
        return;
    }

    // Ambiente — nunca menos de 0.3 para que nada quede negro
    float amb = max(ambientStr, 0.30);
    vec3 ambient = amb * baseColor;

    // Difusa
    vec3 norm     = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff    = max(dot(norm, lightDir), 0.0) * diffuseStr;
    vec3 diffuse  = diff * lightColor * baseColor;

    color = vec4(ambient + diffuse, 1.0);
}