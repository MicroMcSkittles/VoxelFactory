#version 460 core
layout (location = 0) in vec2 a_Pos;
layout (location = 1) in vec2 a_TexCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Model;

out vec2 TexCoord;
out vec2 AtlasPosition;
out vec2 AtlasSize;

uniform vec2 u_Sizes[128];
uniform vec2 u_Positions[128];
uniform vec2 u_AtlasPositions[128];
uniform vec2 u_AtlasSizes[128];

void main() {
    vec2 position = a_Pos * u_Sizes[gl_InstanceID] + u_Positions[gl_InstanceID];
    gl_Position = u_ViewProjection * u_Model * vec4(position, -1.0, 1.0);
    TexCoord = a_TexCoord;
    AtlasPosition = u_AtlasPositions[gl_InstanceID];
    AtlasSize = u_AtlasSizes[gl_InstanceID];
}