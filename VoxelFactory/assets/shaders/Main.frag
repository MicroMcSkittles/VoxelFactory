#version 460 core
out vec4 o_FragColor;

uniform sampler2D u_Texture;
in vec2 TexCoord;

void main() {
    //o_FragColor = mix(texture(u_Texture, TexCoord), vec4(TexCoord, 0.0f, 1.0f), 0.2);
    o_FragColor = texture(u_Texture, TexCoord);
} 