#version 460 core
out vec4 o_FragColor;

uniform vec3 u_LightDirection; // -0.6, -0.8, -0.55
uniform sampler2D u_Texture;
in vec2 TexCoord;
in vec3 Normal;
flat in uint TextureID;

const int c_AtlasSize = 16;

void main() {
    // Find texture in the atlas
    vec2 atlas_coord = vec2(TexCoord.x, 1.0 - TexCoord.y);
    atlas_coord.x += float(TextureID % c_AtlasSize);
    atlas_coord.y += float(TextureID / c_AtlasSize);
    vec4 texture_color = texture(u_Texture, atlas_coord / float(c_AtlasSize));
    if (texture_color.a < 0.0001) discard;
    
    // Apply basic lighting
    const vec3 light_direction = normalize(-vec3(-0.6, -0.8, -0.55));
    const vec3 ambient = vec3(0.4);
    const vec3 diffuse = vec3(max(dot(Normal, light_direction), 0.0));

    o_FragColor = vec4(ambient + diffuse, 1.0) * texture_color;
} 