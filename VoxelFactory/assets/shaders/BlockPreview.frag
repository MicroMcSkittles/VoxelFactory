#version 460 core
out vec4 o_FragColor;

uniform float u_Brightness;

uniform sampler2D u_Atlas;
in vec2 TexCoord;

uniform int u_TextureIDs[6]; // front, back, left, right, top, bottom
flat in uint Face;

in vec3 Normal;

const int c_AtlasSize = 16;
const vec3 c_LightDirection = -vec3(-0.6, -0.8, -0.55);

void main() {
    // Find texture in the atlas
    vec2 atlas_coord = vec2(TexCoord.x, TexCoord.y);
    atlas_coord.x += float(u_TextureIDs[Face] % c_AtlasSize);
    atlas_coord.y += 15.0 - float(u_TextureIDs[Face] / c_AtlasSize);
    vec4 texture_color = texture(u_Atlas, atlas_coord / float(c_AtlasSize));
    if (texture_color.a < 0.0001) discard;
    
    // Apply basic lighting
    vec3 diffuse = vec3(max(dot(Normal, normalize(c_LightDirection)), 0.0));
    vec3 ambient = vec3(0.4);
    float brightness = 1.0;
    vec4 lighting = vec4((ambient + diffuse * 0.4 + 0.2) * u_Brightness, 1.0);

    o_FragColor = lighting * texture_color;
} 