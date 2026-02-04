#version 460 core
out vec4 o_FragColor;

uniform sampler2D u_Texture;
in vec2 TexCoord;
flat in uint TextureID;

const int c_AtlasSize = 16;

void main() {
    //o_FragColor = mix(texture(u_Texture, TexCoord), vec4(TexCoord, 0.0f, 1.0f), 0.2);
    //o_FragColor = mix(texture(u_Texture, TexCoord), vec4(BlockID / 2, BlockID / 2, BlockID / 2, 1.0f), 0.2);
    vec2 atlas_coord = vec2(TexCoord.x, 1.0 - TexCoord.y);
    atlas_coord.x += float(TextureID % c_AtlasSize);
    atlas_coord.y += float(TextureID / c_AtlasSize);
    o_FragColor = texture(u_Texture, atlas_coord / float(c_AtlasSize));
} 