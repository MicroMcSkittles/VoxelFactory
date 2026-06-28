#version 460 core
out vec4 o_FragColor;

uniform sampler2D u_Texture;
in vec2 TexCoord;
uniform uint u_TextureID;

const int c_AtlasSize = 16;

void main() {
    // Find texture in the atlas
    vec2 atlas_coord = vec2(TexCoord.x, 1.0 - TexCoord.y);
    atlas_coord.x += float(u_TextureID % c_AtlasSize);
    atlas_coord.y += float(u_TextureID / c_AtlasSize);
    vec4 texture_color = texture(u_Texture, atlas_coord / float(c_AtlasSize));
    if (texture_color.a < 0.0001) discard;
    o_FragColor = texture_color;
} 