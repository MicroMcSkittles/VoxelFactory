#version 460 core
out vec4 o_FragColor;

uniform vec4 u_Color;
in vec2 TexCoord;

void main() {
    o_FragColor = u_Color;
} 