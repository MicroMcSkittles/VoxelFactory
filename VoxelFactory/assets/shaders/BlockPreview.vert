#version 460 core
layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoord;
layout (location = 3) in float a_AmbientOcclution;
layout (location = 4) in uint a_TextureID;

uniform mat4 u_ViewProjection;
uniform mat4 u_Model;

out vec2 TexCoord;
flat out uint Face;

out vec3 Normal;

void main() {
    gl_Position = u_ViewProjection * u_Model * vec4(a_Pos, 1.0);
    Normal = a_Normal;//normalize((u_Model * vec4(a_Normal, 1.0)).xyz);
    TexCoord = a_TexCoord;
    Face = a_TextureID;
}