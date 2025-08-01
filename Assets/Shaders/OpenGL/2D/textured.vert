// Assets/Shaders/OpenGL/2D/textured.vert
#version 450 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection; 

out vec2 TexCoord;

void main()
{
    gl_Position = projection * view * model * vec4(aPos.x, aPos.y, 0.0, 1.0);
    TexCoord = aTexCoord;
}