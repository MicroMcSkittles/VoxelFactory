#version 460 core
out vec4 o_FragColor;

uniform vec3 u_Tint;
in vec2 TexCoord;

void main() {
    o_FragColor = vec4(u_Tint, 1.0);
} 