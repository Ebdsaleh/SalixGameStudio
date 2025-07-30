// Assets/Shaders/3D/color_only.frag
#version 450
out vec4 FragColor;

// This is the uniform we are already sending from your C++ code
uniform vec4 tint_color;

void main()
{
    // Just output the color directly, ignoring any lighting.
    FragColor = tint_color;
}