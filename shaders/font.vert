#version 420 core

in vec4 Pos;

out vec2 TexCoords;

uniform mat4 ViewProj;

void main()
{ 
    gl_Position = ViewProj * vec4(Pos.xy, 0.0, 1.0);
    TexCoords = Pos.zw;
}

