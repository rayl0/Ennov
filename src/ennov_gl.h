#pragma once

// ----- TODO --------
// Add viewport and projection matrix updating with window size
// Add a setviewproj function.

#include "glm/glm.hpp"

struct texture
{
    u32 Id;
    u32 Width, Height;
};

#define MAX_TEXQUADS 300

#define DATA_PER_VERTEX_TEXQUAD 9
#define VERTEX_BUFFER_SIZE (MAX_TEXQUADS * 4 * DATA_PER_VERTEX_TEXQUAD)

#define INDEX_BUFFER_SIZE (MAX_TEXQUADS * 6)

struct render_cmd
{
    u32 TextureMap[32];
    u32 NumBindTextureSlots;
    u32 NumVertices;
};

struct render_context
{
    u32 VertexArray;
    u32 VertexBuffer;
    u32 IndexBuffer;
    u32 TexQuadShader;

    glm::mat4 ViewProj;

    f32 VertexBufferData[VERTEX_BUFFER_SIZE];
    u32 VertexBufferCurrentPos;

    u32 IndexBufferData[INDEX_BUFFER_SIZE];
    u32 IndexBufferCurrentPos;
    u32 BufferIndex;

    u32 TextureMap[32];
    s32 NumTextureSlots;
    u32 NumBindTextureSlots;

    // TODO(rajat): This could be removed and decoupled after out of debug memory assertions
    render_cmd RenderCommands[5]; // TODO(rajat): It's not a good idea to have a constant length command queue
    u32 NumCommands;
};

extern void
CreateRenderContext(u32* Id, const char* VertexShader, const char* FragmentShader);

// TODO(rajat): Not optimal for multithreading
extern void
BindRenderContext(u32 Id);

// TODO(rajat): Not optimal for multithreading
extern void
FlushRenderCommands(u32 Id);

extern void
RenderContextUpdateViewProj(u32 Id, f32 Width, f32 Height);

/* extern void */
/* PositionCameraAt(); */

// BASE API

extern void
FillQuad(f32 x, f32 y, f32 w, f32 h, u32 Color);

extern void
FillTexQuad(f32 x, f32 y, f32 w, f32 h, u32 Color, texture *Texture);

extern void
FillTexQuad(f32 x, f32 y, f32 w, f32 h, texture *Texture);

extern void
FillTexQuadClipped(f32 x, f32 y, f32 w, f32 h, u32 Color, texture *Texture, vec4 Clip);

extern void
FillTexQuadClipped(f32 x, f32 y, f32 w, f32 h, texture *Texture, vec4 Clip);

// NOTE(rajat): Provide your own texture coordinates to these functions
extern void
FillTexQuadClipped(f32 x, f32 y, f32 w, f32 h, u32 Color, texture *Texture, f32 s0, f32 t0, f32 s1, f32 t1);

extern void
FillTexQuadClipped(f32 x, f32 y, f32 w, f32 h, texture *Texture, f32 s0, f32 t0, f32 s1, f32 t1);

extern void
RenderCommit();

// EXTENDED API

/* Order x, y, w, h for DestRect array*/
extern void
FillQuad(f32 *DestRect, u32 Color);

extern void
FillTexQuad(f32 *DestRect, u32 Color, texture *Texture);

extern void
FillTexQuad(f32 *DestRect, texture *Texture);

// TODO(rajat): Remove the vec4 type in the base api and i
// implement FillTexQuadClipped in extended api and implement
// bluk functions like FillQuads

internal_ texture
CreateTexture(loaded_bitmap* Bitmap);

internal_ texture
CreateTextureEx(u8* Pixels, u32 PixelFormat, u32 Width, u32 Height, u32 TextureFormat);

internal_ u32
CreateShaderProgram(const char* VertexShaderSource, const char* FragmentShaderSource);
