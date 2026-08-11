#version 460 core
out vec4 o_FragColor;

uniform float u_Brightness;

uniform sampler2D u_Atlas;
uniform uint u_TextureID;
uniform vec2 u_Offset;
in vec2 TexCoord;

const int c_AtlasSize = 16;

void main() {
    // Find texture in the atlas
    vec2 atlas_coord = TexCoord * 0.25 + u_Offset;
    atlas_coord.x += float(u_TextureID % c_AtlasSize);
    atlas_coord.y += 15.0 - float(u_TextureID / c_AtlasSize);
    vec4 texture_color = texture(u_Atlas, atlas_coord / float(c_AtlasSize));
    if (texture_color.a < 0.0001) discard;

    o_FragColor = texture_color * vec4(vec3(u_Brightness), 1.0);
} 