#version 460 core
out vec4 o_FragColor;

uniform sampler2D u_FrameTexture;
in vec2 TexCoord;

void main() {
    vec4 frame_color = texture(u_FrameTexture, TexCoord);
    if (frame_color.a < 0.001) discard;
    o_FragColor = frame_color;
} 