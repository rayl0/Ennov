#pragma once

#include "glm/glm.hpp"

struct texture
{
    u32 Id;
    u32 Width, Height;
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

extern void
CreateRenderContext(const char* VertexShader, const char* FragmentShader);

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

/* Always uses the reserved texture slots */
extern void
FillTexQuadClippedReserved(f32 x, f32 y, f32 w, f32 h, u32 Color, texture *Texture, f32 s0, f32 t0, f32 s1, f32 t1);

extern void
FillTexQuadClippedReservedPointed(f32 x, f32 y, f32 x0, f32 y0, u32 Color, texture *Texture, f32 s0, f32 t0, f32 s1, f32 t1);

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
CreateTextureEx(u8* Pixels, u32 PixelFormat, u32 Width, u32 Height, u32 TextureFormat);

internal_ u32
CreateShaderProgram(const char* VertexShaderSource, const char* FragmentShaderSource);

void
DrawString(const char *String, f32 x, f32 y, f32 Scale, vec4 Color);
