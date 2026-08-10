#version 460 core
layout (location = 0) in vec3 a_Pos;
layout (location = 1) in uint a_Data;
layout (location = 2) in uint a_TextureID;

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
    TextureID = a_TextureID;

    uint normal_bits            = (a_Data & 7);
    if      (normal_bits == 0) Normal = vec3( 0.0, 0.0, 1.0 );
    else if (normal_bits == 1) Normal = vec3( 0.0, 0.0,-1.0 );
    else if (normal_bits == 2) Normal = vec3( 1.0, 0.0, 0.0 );
    else if (normal_bits == 3) Normal = vec3(-1.0, 0.0, 0.0 );
    else if (normal_bits == 4) Normal = vec3( 0.0, 1.0, 0.0 );
    else if (normal_bits == 5) Normal = vec3( 0.0,-1.0, 0.0 );
    
    uint tex_coord_bits         = (a_Data & 24) >> 3;
    if      (tex_coord_bits == 0) TexCoord = vec2(0.0, 0.0);
    else if (tex_coord_bits == 1) TexCoord = vec2(1.0, 0.0);
    else if (tex_coord_bits == 2) TexCoord = vec2(0.0, 1.0);
    else if (tex_coord_bits == 3) TexCoord = vec2(1.0, 1.0);

    uint ambient_occlusion_bits = (a_Data & 96) >> 5;
    if      (ambient_occlusion_bits == 0) AmbientOcclusion = 0.0;
    else if (ambient_occlusion_bits == 1) AmbientOcclusion = 0.5;
    else if (ambient_occlusion_bits == 2) AmbientOcclusion = 1.0;
    else if (ambient_occlusion_bits == 3) AmbientOcclusion = 1.5;
}