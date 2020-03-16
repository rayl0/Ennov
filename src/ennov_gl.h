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
    u32 DefShaderProgram;
    u32 VertexArray;
    batch_data Batch;
    game_areana *Areana;
    glm::mat4 Projection;
};

#define MAX_TEXQUADS 100

#define DATA_PER_VERTEX_TEXQUAD 9
#define VERTEX_BUFFER_SIZE (MAX_TEXQUADS * 6 * DATA_PER_VERTEX_TEXQUAD)

struct render_context
{
    u32 VertexArray;
    u32 VertexBuffer;

    glm::mat4 ViewProj;

    f32 VertexBufferData[VERTEX_BUFFER_SIZE];
    u32 VertexBufferCurrentPos;

    u32 TextureMap[32];
    u8 StartIndex;
    s32 NumTextureSlots;
    u32 NumBindTextureSlots;

    u32 TexQuadShader;
};

/* void BeginRender(); */
/* void FillQuad(60, 60, 60, 60, 0xFF0305FA); */
/* void FillTexQuad(60, 60, 60, 60, 0xFF0305FA, Texture); */
/* void FillTexQuad(60, 60, 60, 60, Texture); */
/* void FillTexQuadClipped(60, 60, 60, 60, 0xFF0305FA, Texture, ClipRect);
/* void EndRender(); */

extern void
CreateRenderContext();

/* extern void */
/* PositionCameraAt(); */

extern void
FillQuad(f32 x, f32 y, f32 w, f32 h, u32 Color);

extern void
FillTexQuad(f32 x, f32 y, f32 w, f32 h, u32 Color, texture *Texture);

extern void
FillTexQuad(f32 x, f32 y, f32 w, f32 h, texture *Texture);

extern void
FillTexQuadClipped(f32 x, f32 y, f32 w, f32 h, u32 Color, texture *Texture, vec4 *Clip);

extern void
RenderCommit();

internal_ texture
CreateTextureEx(u8* Pixels, u32 PixelFormat, u32 Width, u32 Height, u32 TextureFormat);

internal_ u32
CreateShaderProgram(const char* VertexShaderSource, const char* FragmentShaderSource);

internal_ batch_data
CreateBatch(game_areana* Areana);

void
DrawBatchRectangleEx(renderer_data *RenderData, texture *Texture, vec4 Color,
                     rect *SrcClip, vec2 Position, vec2 Dimension, u32 ShaderProgram);

void
DrawBatchRectangleExA(renderer_data *RenderData, texture *Texture, vec4 Color,
                      rect *SrcClip, f32 x, f32 y, f32 w, f32 h, f32 s0, f32 t0, f32 s1, f32 t1, u32 ShaderProgram);

void
DrawBatchRectangle(renderer_data *RenderData, texture *Texture, vec4 Color,
                     rect *SrcClip, vec2 Position, vec2 Dimension);

void
FlushBatch(batch_data* Batch, u32 ShaderProgram, u32 VertexArray, u32 VertexBuffer);

void
BatchRenderRectangle(batch_data* Batch, texture *Texture, vec4 Color,
                     rect *SrcClip, vec2 Position, vec2 Dimension);

void
BatchRenderRectangleDx(batch_data* Batch, texture *Texture, vec4 Color,
                       f32 x, f32 y, f32 w, f32 h, f32 s0, f32 t0, f32 s1, f32 t1);

void
DrawBatchRectangleDx(renderer_data* Data, texture* Texture, vec4 Color,
                     f32 x, f32 y, f32 w, f32 h, f32 s0, f32 t0, f32 s1, f32 t1);

void
DrawString(const char *String, f32 x, f32 y, f32 Scale, vec4 Color);
