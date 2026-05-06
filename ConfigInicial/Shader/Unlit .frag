#v#version 330 core
in vec2 TexCoord;
out vec4 color;

uniform sampler2D texture1;

void main()
{
    // Color exacto de la textura, SIN ningun calculo de luz
    color = vec4(texture(texture1, TexCoord).rgb, 1.0);
}

