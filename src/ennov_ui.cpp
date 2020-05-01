#include "ennov_platform.h"
#include "ennov_math.h"

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

// NOTE(rajat): Platform independence defines for rendering apis
struct texture;

typedef u32 vertex_array;
typedef u32 vertex_buffer;
typedef u32 shader;

#include "ennov_gl.h"
#include "ennov_ui.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

void
UI_CreateContext(ui_render_ctx* ctx, const char* VertexShader, const char* FragmentShader)
{
    Assert(ctx != NULL);

    glGenVertexArrays(1, &ctx->VertexArray);
    glBindVertexArray(ctx->VertexArray);

    f32 Verticies[16] = {
        0.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 1.0,
        1.0, 1.0, 1.0, 1.0,
        1.0, 0.0, 1.0, 0.0
    };

    glGenBuffers(1, &ctx->VertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, ctx->VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Verticies), Verticies, GL_STATIC_DRAW);

    u32 Indicies[6] = {
        0, 1, 2, 2, 3, 0
    };

    glGenBuffers(1, &ctx->IndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->IndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indicies), Indicies, GL_STATIC_DRAW);

    ctx->Shader = CreateShaderProgram(VertexShader, FragmentShader);
    glUseProgram(ctx->Shader);

    glm::mat4 ViewProj = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f);

    u32 ViewProjLocation = glGetUniformLocation(ctx->Shader, "ViewProj");
    glUniformMatrix4fv(ViewProjLocation, 1, GL_FALSE, glm::value_ptr(ViewProj));

    ctx->HaveClipTest = false;

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
}

inline static void
PushCommand(ui_render_ctx* ctx, ui_draw_cmd* cmd)
{
    Assert(ctx->NumCommands != 1000);

    ctx->CommandQueue[ctx->NumCommands] = *cmd;
    ctx->NumCommands++;
}

extern void
UI_StartClip(ui_render_ctx* ctx, vec4 ClipRect)
{
    ctx->HaveClipTest = true;
    ctx->ClipRect = ClipRect;
}

extern void
UI_EndClip(ui_render_ctx* ctx)
{
    ctx->HaveClipTest = false;
}


void
UI_FillQuad(ui_render_ctx* ctx, f32 x, f32 y, f32 w, f32 h, u32 Color)
{
    UI_FillQuad(ctx, x, y, w, h, Color, 1, NULL, 0, 0);
}

void
UI_FillTexQuad(ui_render_ctx* ctx, f32 x, f32 y, f32 w, f32 h, u32 Color, texture* Texture)
{
    UI_FillQuad(ctx, x, y, w, h, Color, 1, Texture, 1, 0);
}

void
UI_FillQuadRounded(ui_render_ctx* ctx, f32 x, f32 y, f32 w, f32 h, u32 Color,
                   f32 Radius)
{
    UI_FillQuad(ctx, x, y, w, h, Color, 1, NULL, 0, Radius);
}

void
UI_FillTexQuadRounded(ui_render_ctx* ctx, f32 x, f32 y, f32 w, f32 h, texture* Texture,
                   f32 Radius)
{
    UI_FillQuad(ctx, x, y, w, h, 0xFFFFFFFF, 0, Texture, 1, Radius);
}

void
UI_FillTexQuadRounded(ui_render_ctx* ctx, f32 x, f32 y, f32 w, f32 h, u32 Color, texture* Texture,
                      f32 Radius)
{
    UI_FillQuad(ctx, x, y, w, h, Color, 1, Texture, 1, Radius);
}

void
UI_FillQuad(ui_render_ctx* ctx, f32 x, f32 y, f32 w, f32 h, vec3 Color,
            u8 Alpha, f32 UseColor, texture* Texture,
            f32 UseTexture, f32 Radius)
{
    Assert(ctx != NULL);

    ui_draw_cmd cmd = {};

    glm::mat4 Model = glm::mat4(1.0f);
    Model = glm::translate(Model, glm::vec3(x, y, 0));
    Model = glm::scale(Model, glm::vec3(w, h, 1));

    cmd.Model = Model;

    cmd.r = Color.r;
    cmd.g = Color.g;
    cmd.b = Color.b;
    cmd.a = Alpha / 255.0f;

    cmd.UI_Width = w;
    cmd.UI_Height = h;

    cmd.UseColor = UseColor;
    cmd.UseTexture = UseTexture;
    cmd.Radius = Radius;

    if(ctx->HaveClipTest)
    {
        cmd.HaveClipTest = true;
        cmd.ClipRect = ctx->ClipRect;
    }
    else
    {
        cmd.HaveClipTest = false;
    }

    if(Texture)
        cmd.TextureId = Texture->Id;

    PushCommand(ctx, &cmd);
}


void
UI_FillQuad(ui_render_ctx* ctx, f32 x, f32 y, f32 w, f32 h, vec3 Color, u8 Alpha)
{
    UI_FillQuad(ctx, x, y, w, h, Color, Alpha, 1.0f, NULL, 0.0f, 0.0f);
}

void
UI_FillTexQuad(ui_render_ctx* ctx, f32 x, f32 y, f32 w, f32 h, vec3 Color, u8 Alpha, texture* Texture)
{
    UI_FillQuad(ctx, x, y, w, h, Color, Alpha, 1, Texture, 1, 0);
}

void
UI_FillQuadRounded(ui_render_ctx* ctx, f32 x, f32 y, f32 w, f32 h, vec3 Color, u8 Alpha, f32 Radius)
{
    UI_FillQuad(ctx, x, y, w, h, Color, Alpha, 1, NULL, 0, Radius);
}

extern void
UI_FillTexQuadRounded(ui_render_ctx* ctx, f32 x, f32 y, f32 w, f32 h, vec3 Color, u8 Alpha, texture* Texture, f32 Radius)
{
    UI_FillQuad(ctx, x, y, w, h, Color, Alpha, 1, Texture, 1, Radius);
}

void
UI_FillQuad(ui_render_ctx* ctx, f32 x, f32 y, f32 w, f32 h, u32 Color, f32 UseColor, texture* Texture, f32 UseTexture, f32 Radius)
{
    Assert(ctx != NULL);

    ui_draw_cmd cmd = {};

    glm::mat4 Model = glm::mat4(1.0f);
    Model = glm::translate(Model, glm::vec3(x, y, 0));
    Model = glm::scale(Model, glm::vec3(w, h, 1));

    cmd.Model = Model;

    u8 r, g, b, a;
    DecodeRGBA(Color, &r, &g, &b, &a);

    cmd.r = r / 255.0f;
    cmd.g = g / 255.0f;
    cmd.b = b / 255.0f;
    cmd.a = a / 255.0f;

    cmd.UI_Width = w;
    cmd.UI_Height = h;

    // TODO(rajat): Move this to render command
    // glUniform1f(RadiusLoc, 20);

    cmd.UseColor = UseColor;
    cmd.UseTexture = UseTexture;
    cmd.Radius = Radius;

    if(ctx->HaveClipTest)
    {
        cmd.HaveClipTest = true;
        cmd.ClipRect = ctx->ClipRect;
    }
    else
    {
        cmd.HaveClipTest = false;
    }

    if(Texture)
        cmd.TextureId = Texture->Id;

    PushCommand(ctx, &cmd);
}


void
UI_UpdateViewProj(ui_render_ctx* ctx, f32 ViewportWidth, f32 ViewportHeight)
{
    glm::mat4 ViewProj = glm::ortho(0.0f, ViewportWidth, ViewportHeight, 0.0f, -1.0f, 1.0f);

    glUseProgram(ctx->Shader);

    u32 ViewProjLocation = glGetUniformLocation(ctx->Shader, "ViewProj");
    glUniformMatrix4fv(ViewProjLocation, 1, GL_FALSE, glm::value_ptr(ViewProj));

    glUseProgram(0);
}

void
UI_FlushCommands(ui_render_ctx* ctx)
{
    Assert(ctx != NULL);

    ui_draw_cmd* cmd;

    glBindVertexArray(ctx->VertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, ctx->VertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->IndexBuffer);
    glUseProgram(ctx->Shader);

    u32 ModelLoc, ColorLoc;
    ModelLoc = glGetUniformLocation(ctx->Shader, "Model");
    ColorLoc = glGetUniformLocation(ctx->Shader, "Color");

    u32 WidthLoc, HeightLoc, RadiusLoc;
    WidthLoc = glGetUniformLocation(ctx->Shader, "uiWidth");
    HeightLoc = glGetUniformLocation(ctx->Shader, "uiHeight");
    RadiusLoc = glGetUniformLocation(ctx->Shader, "radius");

    u32 UseColorLoc, UseTextureLoc;
    UseColorLoc = glGetUniformLocation(ctx->Shader, "useColor");
    UseTextureLoc = glGetUniformLocation(ctx->Shader, "useTexture");

    for(int i = 0; i < ctx->NumCommands; ++i)
    {
        cmd = &ctx->CommandQueue[i];

        glUniformMatrix4fv(ModelLoc, 1, GL_FALSE, glm::value_ptr(cmd->Model));

        glUniform4f(ColorLoc, cmd->r, cmd->g,
                    cmd->b, cmd->a);

        glUniform1f(WidthLoc, cmd->UI_Width);
        glUniform1f(HeightLoc, cmd->UI_Height);

        glUniform1f(UseColorLoc, cmd->UseColor);
        glUniform1f(UseTextureLoc, cmd->UseTexture);

        glUniform1f(RadiusLoc, cmd->Radius);

        if(cmd->HaveClipTest) {
            glEnable(GL_SCISSOR_TEST);
            glScissor(cmd->ClipRect.x, 600 - cmd->ClipRect.w - cmd->ClipRect.y,
                      cmd->ClipRect.z,
                      cmd->ClipRect.w);
        }

        if(cmd->UseTexture > 0.5)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, cmd->TextureId);
        }

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

        if(cmd->HaveClipTest)
            glDisable(GL_SCISSOR_TEST);
    }

    ctx->NumCommands = 0;
}

#define UI_ID (__LINE__)

static ui_ctx uctx = {};
static stbrp_context* context;

void
UI_CreateContext(f32 CanvasX, f32 CanvasY, f32 CanvasW, f32 CanvasH)
{
    uctx.CanvasY = CanvasX;
    uctx.CanvasY = CanvasY;
    uctx.CanvasW = CanvasW;
    uctx.CanvasH = CanvasH;
}


void
UI_NewFrame(ui_io io)
{
    uctx.io = io;
    uctx.HotIdx = 0;
}

b32
UI_Button(u32 Id, const char* Title)
{
}


void
UI_EndFrame()
{
    if(uctx.io.Pointer.Hit == 0)
    {
        uctx.ActiveIdx = 0;
    }
    else
    {
        if(uctx.ActiveIdx == 0)
            uctx.ActiveIdx = -1;
    }
}
