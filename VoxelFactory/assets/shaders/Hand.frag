#version 460 core
out vec4 o_FragColor;

uniform float u_Brightness;

uniform sampler2D u_Texture;
in vec2 TexCoord;
in vec3 Normal;

const vec3 c_LightDirection = -vec3(-0.6, -0.8, -0.55);

void main() {
    // Find texture in the atlas
    vec4 texture_color = texture(u_Texture, TexCoord);
    
    // Apply basic lighting
    vec3 diffuse = vec3(max(dot(Normal, normalize(c_LightDirection)), 0.0));
    vec3 ambient = vec3(0.4);
    float brightness = 1.0;
    vec4 lighting = vec4((ambient + diffuse * 0.4 + 0.2) * u_Brightness, 1.0);

    //o_FragColor = lighting * texture_color;
    o_FragColor = vec4(TexCoord, 1.0, 1.0);
} 