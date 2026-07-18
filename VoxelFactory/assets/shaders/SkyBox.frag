#version 460 core
out vec4 o_FragColor;

in vec3 WorldPos;

uniform vec3 u_SkyColor;
uniform vec3 u_SkyHorizonColor;

void main() { 
    float theta = max(dot(vec3(0,1,0), normalize(WorldPos)), 0.0);
    o_FragColor = vec4(mix(u_SkyHorizonColor, u_SkyColor, theta), 1.0);
} 