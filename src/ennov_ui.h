#pragma once
#ifndef ENNOV_UI_H

#include "ennov_math.h"
#include "ennov_defs.h"
#include "ennov_gl.h"

struct ui_render_ctx
{
    u32 VertexArray;
    u32 VertexBuffer;
    u32 IndexBuffer;

    u32 Shader;
};

extern void
UI_CreateContext(ui_render_ctx* ctx, const char* VertexShader, const char* FragmentShader);

extern void
UI_FillQuad(ui_render_ctx* ctx, f32 x, f32 y, f32 w, f32 h, u32 Color);

extern void
UI_FillTexQuad(ui_render_ctx* ctx, f32 x, f32 y, f32 w, f32 h, u32 Color, texture* Texture);

// TODO(Rajat): Implement Clipping stuff
/* extern void */
/* UI_FillTexQuad(ui_render_ctx* ctx, f32 x, f32 y, f32 w, f32 h, u32 Color, texture* Texture, vec4 Clip); */

#define ENNOV_UI_H
#endif
