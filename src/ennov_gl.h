#pragma once

#include "glm/glm.hpp"

struct texture
{
    u32 Id;
    u32 Width, Height;
};

struct batch_data
{
    u32 Id;
    u32 NumBindTextureSlots;
    s32 TextureSlots[32]; // NOTE(rajat): There is no point in dynamically allocating here
    f32 *VertexBufferData;
    u32 VertexBufferSize;
    u32 ShaderProgram;
    u32 VertexBufferCurrentPos;
};

struct renderer_data
{
    s32 NumTextureSlots;
    u32 DynamicVertexBuffer;
    u32 ShaderProgram;
    u32 VertexArray;
    batch_data Batch;
    game_areana *Areana;
    glm::mat4 Projection;
};

internal_ texture
CreateTextureEx(u8* Pixels, u32 PixelFormat, u32 Width, u32 Height, u32 TextureFormat);

internal_ u32
CreateShaderProgram(char* VertexShaderSource, char* FragmentShaderSource);

internal_ batch_data
CreateBatch(game_areana* Areana);

void
DrawBatchRectangleEx(renderer_data *RenderData, texture *Texture, vec4 Color,
                     rect *SrcClip, vec2 Position, vec2 Dimension, u32 ShaderProgram);

void
FlushBatch(batch_data* Batch, u32 ShaderProgram, u32 VertexArray, u32 VertexBuffer);

void
BatchRenderRectangle(batch_data* Batch, texture *Texture, vec4 Color,
                     rect *SrcClip, vec2 Position, vec2 Dimension);
