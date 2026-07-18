#version 460 core
layout (location = 0) in vec3 a_Pos;

uniform mat4 u_ViewProjection;
uniform vec3 u_CameraPosition;

out vec3 WorldPos;

void main() {
    WorldPos = a_Pos;
    gl_Position = u_ViewProjection * vec4(a_Pos + u_CameraPosition, 1.0);
}