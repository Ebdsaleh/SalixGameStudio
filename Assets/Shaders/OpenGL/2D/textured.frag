#version 450 core
out vec4 fragment_color;

in vec2 TexCoord;

uniform sampler2D texture_sampler;
uniform vec4 tint_color;

void main()
{
    vec4 sampled_color = texture(texture_sampler, vec2(TexCoord.x, 1.0 - TexCoord.y));
    fragment_color = sampled_color * tint_color; 
}