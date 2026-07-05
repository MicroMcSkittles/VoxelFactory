#version 460 core
out vec4 o_FragColor;

uniform sampler2D u_Texture;
in vec2 TexCoord;
uniform uint u_TextureID;
uniform uint u_AtlasWidth;
uniform uint u_AtlasHeight;

//const int c_AtlasSize = 16;

void main() {
    // Find texture in the atlas
    vec2 atlas_coord = vec2(TexCoord.x, TexCoord.y);
    atlas_coord.x += float(u_TextureID % u_AtlasWidth);
    atlas_coord.y += u_AtlasHeight - 1.0 - float(u_TextureID / u_AtlasWidth);
    atlas_coord.x /= float(u_AtlasWidth);
    atlas_coord.y /= float(u_AtlasHeight);
    vec4 texture_color = texture(u_Texture, atlas_coord);
    if (texture_color.a < 0.0001) discard;
    o_FragColor = texture_color;
} 