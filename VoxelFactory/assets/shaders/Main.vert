#version 460 core
layout (location = 0) in uint a_Pos;
layout (location = 1) in uint a_Data;

uniform mat4 u_ViewProjection;
uniform mat4 u_Model;

out vec2 TexCoord;
flat out uint TextureID;

out float AmbientOcclusion;
out vec3 Normal;
out vec3 WorldPos;
out float LightLevel;

void main() {
    vec3 position = vec3(0.0);
    position.x = float(a_Pos & 0x000000FF) - 0.5;
    position.y = float((a_Pos & 0x00FFFF00) >> 8) - 0.5;
    position.z = float((a_Pos & 0xFF000000) >> 24) - 0.5;

    WorldPos = (u_Model * vec4(position, 1.0)).xyz;
    gl_Position = u_ViewProjection * vec4(WorldPos, 1.0);

    TextureID = a_Data & 0xFFFFF;

    uint data = a_Data >> 20;
    uint light_level_bits       = (data & 15); // bits 0,1,2,3
    LightLevel = float(light_level_bits + 1) / 16;

    uint normal_bits            = (data & 112) >> 4; // bits 4,5,6
    if      (normal_bits == 0) Normal = vec3( 0.0, 0.0, 1.0 );
    else if (normal_bits == 1) Normal = vec3( 0.0, 0.0,-1.0 );
    else if (normal_bits == 2) Normal = vec3( 1.0, 0.0, 0.0 );
    else if (normal_bits == 3) Normal = vec3(-1.0, 0.0, 0.0 );
    else if (normal_bits == 4) Normal = vec3( 0.0, 1.0, 0.0 );
    else if (normal_bits == 5) Normal = vec3( 0.0,-1.0, 0.0 );
    
    uint tex_coord_bits         = (data & 384) >> 7; // bits 7,8
    if      (tex_coord_bits == 0) TexCoord = vec2(0.0, 0.0);
    else if (tex_coord_bits == 1) TexCoord = vec2(1.0, 0.0);
    else if (tex_coord_bits == 2) TexCoord = vec2(0.0, 1.0);
    else if (tex_coord_bits == 3) TexCoord = vec2(1.0, 1.0);

    uint ambient_occlusion_bits = (data & 1536) >> 9; // bits 9,10
    if      (ambient_occlusion_bits == 0) AmbientOcclusion = 0.0;
    else if (ambient_occlusion_bits == 1) AmbientOcclusion = 0.5;
    else if (ambient_occlusion_bits == 2) AmbientOcclusion = 1.0;
    else if (ambient_occlusion_bits == 3) AmbientOcclusion = 1.5;
}