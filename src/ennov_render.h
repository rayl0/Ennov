#pragma once

struct vb_dynamic_create_command
{
    int Idx_;
    memory_index Size;
};

struct vb_dynamic_update_command
{
    void* Data;
    memory_index Size;
};

struct draw_command
{
    int VertexArray;
    int VertexBuffer;
    int Shader;
    int NumVertexs;
    int NumTextures;
    int TextureArray[32];
};

struct render_output
{
    vb_dynamic_create_command VB_Commands[1000];
    vb_dynamic_update_command VB_UpdateCommands[1000];
    draw_command DrawCommands[1000];
};

void ProcessDrawData()
