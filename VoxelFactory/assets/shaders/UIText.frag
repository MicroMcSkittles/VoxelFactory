#version 460 core
out vec4 o_FragColor;

uniform vec3 u_ForgroundColor;
uniform vec4 u_BackgroundColor;

uniform sampler2D u_FontAtlas;
in vec2 TexCoord;
in vec2 AtlasPosition;
in vec2 AtlasSize;

void main() {
    vec2 atlas_coord = TexCoord * AtlasSize + AtlasPosition;
    vec4 texture_color = texture(u_FontAtlas, atlas_coord);

    if (texture_color.a < 0.0001) discard;
    o_FragColor = vec4(u_ForgroundColor, 1.0);
} 