#version 460 core
layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec2 a_TexCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Model;

out vec2 TexCoord;

void main() {
    gl_Position = u_ViewProjection * u_Model * vec4(a_Pos, 1.0);
    TexCoord = a_TexCoord;
}