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
    u32 Textureid;
    f32 UI_Width;
    f32 UI_Height;
    glm::mat4 Model;
};

struct ui_render_ctx
{
    u32 VertexArray;
    u32 VertexBuffer;
    u32 IndexBuffer;

    u32 Shader;

    // TODO(rajat): It's not a good idea to have a variable length command queue
    ui_draw_cmd CommandQueue[1000];
    u32 NumCommands;
};

extern void
UI_UpdateViewProj(f32 ViewportWidth, f32 ViewportHeight);

extern void
UI_CreateContext(ui_render_ctx* ctx, const char* VertexShader, const char* FragmentShader);

extern void
UI_FillQuad(ui_render_ctx* ctx, f32 x, f32 y, f32 w, f32 h, u32 Color);

extern void
UI_FillTexQuad(ui_render_ctx* ctx, f32 x, f32 y, f32 w, f32 h, u32 Color, texture* Texture);

extern void
UI_FlushCommands(ui_render_ctx* ctx);

// TODO(Rajat): Implement Clipping stuff
/* extern void */
/* UI_FillTexQuad(ui_render_ctx* ctx, f32 x, f32 y, f32 w, f32 h, u32 Color, texture* Texture, vec4 Clip); */

#define ENNOV_UI_H
#endif
