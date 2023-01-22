#Vertex shader
#version 410 core

in vec2 position;
in vec3 color;

out vec3 Color;

void main()
{
    Color = color;
    gl_Position = vec4(position, 0.0, 1.0);
}

#Fragment shader
#version 410 core
    
in vec3 Color;
uniform vec3 uniColor;
out vec4 outColor;

void main()
{
    outColor = vec4(uniColor.x, Color.y, Color.z, 1.0);
}
