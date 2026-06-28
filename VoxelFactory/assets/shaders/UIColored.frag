#version 460 core
out vec4 o_FragColor;

uniform vec3 u_Color;
in vec2 TexCoord;

void main() {
    o_FragColor = vec4(u_Color, 1.0);
} 