// Assets/Shaders/OpenGL/3D/simple.vert
#version 450 core

// Input vertex attributes from the VBO
layout(location = 0) in vec3 aPos ;  // The position of the vertex in model space.
layout(location = 1) in vec3 aColor; // The color of the vertex.

// Uniforms (data sent from the CPU)
uniform mat4 model;         // Transforms the vertex from model space to world space
uniform mat4 view;          // Transforms the vertex from world space to view (camera) space
uniform mat4 projection;    // Transforms the vertex from view space to screen space

// Output to the fragment shader
out vec4 vertex_color;

void main() {
    // The core operation: transform the vertex position.
    // The order of multiplication is important (P * V * M).
    gl_Position = projection * view * model * vec4(aPos, 1.0);

    // Pass the vertex's color directly to the fragment shader.
    // Convert the vec3 input color to a vec4 for the output.
    vertex_color = vec4(aColor, 1.0);
}