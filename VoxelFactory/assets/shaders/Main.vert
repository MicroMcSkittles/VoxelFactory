#version 460 core
layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoord;
layout (location = 3) in float a_AmbientOcclusion;
layout (location = 4) in uint a_TextureID;

uniform mat4 u_ViewProjection;
uniform mat4 u_Model;

out vec2 TexCoord;
flat out uint TextureID;

out float AmbientOcclusion;
out vec3 Normal;
out vec3 WorldPos;

void main() {
    WorldPos = (u_Model * vec4(a_Pos, 1.0)).xyz;
    gl_Position = u_ViewProjection * vec4(WorldPos, 1.0);
    TexCoord = a_TexCoord;
    TextureID = a_TextureID;

    AmbientOcclusion = a_AmbientOcclusion;
    Normal = a_Normal;
}