#version 410 core

in vec4 Position;
out vec2 TextureCoords;

uniform mat4 ViewProj;
uniform mat4 Model;

void main()
{
    gl_Position = ViewProj * Model * vec4(Position.xy, 0.0, 1.0f);
    TextureCoords = Position.zw;
}
