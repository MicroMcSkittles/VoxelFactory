#version 460 core
layout (location = 0) in uint a_Pos;
layout (location = 1) in uint a_Data;

uniform mat4 u_ViewProjection;
uniform mat4 u_Model;

out vec2 TexCoord;
flat out uint Face;

out vec3 Normal;

void main() {
    vec3 position = vec3(0.0);
    position.x = float(a_Pos & 0x000000FF) - 0.5;
    position.y = float((a_Pos & 0x00FFFF00) >> 8) - 0.5;
    position.z = float((a_Pos & 0xFF000000) >> 24) - 0.5;

    gl_Position = u_ViewProjection * u_Model * vec4(position, 1.0);
    Face = a_Data & 0x00FFFFFF;

    uint data = a_Data >> 24;
    uint normal_bits            = (data & 7); // bits 0,1,2
    if      (normal_bits == 0) Normal = vec3( 0.0, 0.0, 1.0 );
    else if (normal_bits == 1) Normal = vec3( 0.0, 0.0,-1.0 );
    else if (normal_bits == 2) Normal = vec3( 1.0, 0.0, 0.0 );
    else if (normal_bits == 3) Normal = vec3(-1.0, 0.0, 0.0 );
    else if (normal_bits == 4) Normal = vec3( 0.0, 1.0, 0.0 );
    else if (normal_bits == 5) Normal = vec3( 0.0,-1.0, 0.0 );
    
    uint tex_coord_bits         = (data & 24) >> 3; // bits 3,4
    if      (tex_coord_bits == 0) TexCoord = vec2(0.0, 0.0);
    else if (tex_coord_bits == 1) TexCoord = vec2(1.0, 0.0);
    else if (tex_coord_bits == 2) TexCoord = vec2(0.0, 1.0);
    else if (tex_coord_bits == 3) TexCoord = vec2(1.0, 1.0);
}