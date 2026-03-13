#version 460 core
out vec4 o_FragColor;

uniform sampler2D u_Texture;
in vec2 TexCoord;
flat in uint TextureID;

const int c_AtlasSize = 16;

void main() {
    vec2 atlas_coord = vec2(TexCoord.x, 1.0 - TexCoord.y);
    atlas_coord.x += float(TextureID % c_AtlasSize);
    atlas_coord.y += float(TextureID / c_AtlasSize);
    o_FragColor = texture(u_Texture, atlas_coord / float(c_AtlasSize));
    if (o_FragColor.a < 0.0001) discard;
    //o_FragColor = mix(o_FragColor, vec4(TexCoord, 0.0f, 1.0f), 0.45);
    //o_FragColor = vec4(TexCoord, 1.0f, 1.0f);
} 