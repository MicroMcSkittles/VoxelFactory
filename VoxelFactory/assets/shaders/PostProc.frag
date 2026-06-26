#version 460 core
out vec4 o_FragColor;

uniform sampler2D u_FrameTexture;
in vec2 TexCoord;

void main() {
    vec4 frame_color = texture(u_FrameTexture, TexCoord);
    o_FragColor = frame_color;
} 