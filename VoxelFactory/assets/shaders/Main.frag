#version 460 core
out vec4 o_FragColor;

uniform vec3 u_CameraPos;
uniform vec3 u_SkyHorizonColor;
uniform float u_Brightness;

uniform sampler2D u_Texture;
in vec2 TexCoord;
flat in uint TextureID;

in float AmbientOcclusion;
in vec3 Normal;
in vec3 WorldPos;

const int c_AtlasSize = 16;
const vec3 c_LightDirection = -vec3(-0.6, -0.8, -0.55);
const float c_FogMin = 75.0;
const float c_FogMax = 125.0;

void main() {
    // Find texture in the atlas
    vec2 atlas_coord = vec2(TexCoord.x, TexCoord.y);
    atlas_coord.x += float(TextureID % c_AtlasSize);
    atlas_coord.y += 15.0 - float(TextureID / c_AtlasSize);
    vec4 texture_color = texture(u_Texture, atlas_coord / float(c_AtlasSize));
    if (texture_color.a < 0.0001) discard;
    
    // Apply basic lighting
    vec3 diffuse = vec3(max(dot(Normal, normalize(c_LightDirection)), 0.0));
    vec3 ambient = vec3(0.4);
    float brightness = 1.0;
    vec4 lighting = vec4(((ambient + diffuse * 0.4) * AmbientOcclusion + 0.2) * u_Brightness, 1.0);

    // Calculate fog
    float depth = length(WorldPos - u_CameraPos);
    float fog = 0.0;
    if (depth >= c_FogMin && depth <= c_FogMax) fog = (depth - c_FogMin) / (c_FogMax - c_FogMin);
    else if (depth > c_FogMax) fog = 1.0;

    o_FragColor = mix(lighting * texture_color, vec4(u_SkyHorizonColor,1.0), fog);
} 