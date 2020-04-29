// ------- TODO --------
// Implement alpha input instead of color input for FillTexQuad function

#pragma once
#ifndef ENNOV_UI_H

#include "glm/glm.hpp"

#include "ennov_math.h"
#include "ennov_defs.h"
#include "ennov_gl.h"

struct ui_draw_cmd
{
    f32 UseTexture;
    f32 UseColor;
    u8 r, g, b, a;
    u32 TextureId;
    f32 UI_Width;
    f32 UI_Height;
    f32 Radius;
    vec4 ClipRect;
    b32 HaveClipTest;
    glm::mat4 Model;
};

struct ui_render_ctx
{
    u32 VertexArray;
    u32 VertexBuffer;
    u32 IndexBuffer;

    u32 Shader;

    vec4 ClipRect;
    b32 HaveClipTest;

    // TODO(rajat): It's not a good idea to have a const length command queue
    ui_draw_cmd CommandQueue[1000];
    u32 NumCommands;
};

extern void
UI_UpdateViewProj(f32 ViewportWidth, f32 ViewportHeight);

extern void
UI_CreateContext(ui_render_ctx* ctx, const char* VertexShader, const char* FragmentShader);

extern void
UI_StartClip(ui_render_ctx* ctx, vec4 ClipRect);

extern void
UI_EndClip(ui_render_ctx* ctx);

extern void
UI_FillQuad(ui_render_ctx* ctx, f32 x, f32 y, f32 w, f32 h, u32 Color);

extern void
UI_FillTexQuad(ui_render_ctx* ctx, f32 x, f32 y, f32 w, f32 h, u32 Color, texture* Texture);

extern void
UI_FillQuadRounded(ui_render_ctx* ctx, f32 x, f32 y, f32 w, f32 h, u32 Color, f32 Radius);

extern void
UI_FillTexQuadRounded(ui_render_ctx* ctx, f32 x, f32 y, f32 w, f32 h, texture* Texture, f32 Radius);

extern void
UI_FillTexQuadRounded(ui_render_ctx* ctx, f32 x, f32 y, f32 w, f32 h, u32 Color, texture* Texture, f32 Radius);

// TODO(rajat): May have support for blurred textures drop shadows
extern void
UI_FillQuad(ui_render_ctx* ctx, f32 x, f32 y, f32 w, f32 h, u32 Color, f32 UseColor, texture* Texture, f32 UseTexture, f32 Radius);

extern void
UI_FlushCommands(ui_render_ctx* ctx);

#ifdef UI_SRC_ID
#define GEN_ID ((UI_SRC_ID) + (__LINE__))
#else
#define GEN_ID (__LINE__)
#endif

struct ui_io
{
    struct
    {
        vec2 at;
        b32 Hit;
        /* b32 HitMask; */

        b32 Drag;
        /* b32 DragMask; */
    }Pointer;
};

struct ui_ctx
{
    f32 CanvasX;
    f32 CanvasY;
    f32 CanvasW;
    f32 CanvasH;

    ui_render_ctx* rctx;
    ui_io io;

    s32 ActiveIdx;
    s32 HotIdx;
};

extern void
UI_CreateContext(f32 CanvasX, f32 CanvasY, f32 CanvasW, f32 CanvasH);

extern void
UI_NewFrame(ui_io io);

extern b32
UI_Button(u32 Id, const char* Title);

extern void
UI_EndFrame();

extern void
UI_StartPanel(const char* Title, f32 winw, f32 winh);

extern void
UI_EndPanel();

#define ENNOV_UI_H
#endif
