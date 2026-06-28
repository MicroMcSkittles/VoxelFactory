#version 460 core
out vec4 o_FragColor;

uniform sampler2D u_Texture;
in vec2 TexCoord;

void main() {
    vec4 texture_color = texture(u_Texture, TexCoord);
    o_FragColor = texture_color;
} 