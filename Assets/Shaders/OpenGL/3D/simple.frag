// Assets/Shaders/OpenGL/3D/simple.frag
#version 450 core

// Input from the vertex shader (will be interpolated across the triangle).
in vec4 vertex_color;

// The final output color of the pixel.
out vec4 frag_color;

void main() {
    // Set the output color to the color received from the vertex shader.
    frag_color = vertex_color;
}