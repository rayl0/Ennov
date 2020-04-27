#include "ennov_platform.h"
#include "ennov_math.h"

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

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
}

extern void
UI_FillQuad(ui_render_ctx* ctx, f32 x, f32 y, f32 w, f32 h, u32 Color)
{
    Assert(ctx != NULL);

    glm::mat4 Model = glm::mat4(1.0f);
    Model = glm::translate(Model, glm::vec3(x, y, 0));
    Model = glm::scale(Model, glm::vec3(w, h, 1));

    glBindVertexArray(ctx->VertexArray);
    glUseProgram(ctx->Shader);

    u32 ModelLoc, ColorLoc, ViewProjLocation;
    ModelLoc = glGetUniformLocation(ctx->Shader, "Model");
    glUniformMatrix4fv(ModelLoc, 1, GL_FALSE, glm::value_ptr(Model));

    glm::mat4 ViewProj = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f);

    ViewProjLocation = glGetUniformLocation(ctx->Shader, "ViewProj");
    glUniformMatrix4fv(ViewProjLocation, 1, GL_FALSE, glm::value_ptr(ViewProj));

    u8 r, g, b, a;
    DecodeRGBA(Color, &r, &g, &b, &a);

    ColorLoc = glGetUniformLocation(ctx->Shader, "Color");
    glUniform4f(ColorLoc, r / 255.0, g / 255.0, b / 255.0, a / 255.0);

    // u32 tl, bl, br, tr;
    // tl = glGetUniformLocation(ctx->Shader, "TopLeft");
    // bl = glGetUniformLocation(ctx->Shader, "BottomLeft");
    // br = glGetUniformLocation(ctx->Shader, "BottomRight");
    // tr = glGetUniformLocation(ctx->Shader, "TopRight");

    // glUniform2f(tl, x, y);
    // glUniform2f(bl, x, y + h);
    // glUniform2f(br, x + w, y + h);
    // glUniform2f(tr, x + w, y);

    // u32 TexturedLoc;
    // TexturedLoc = glGetUniformLocation(ctx->Shader, "TexturedLoc");
    // glUniform1f(TexturedLoc, 0.0);

    // u32 arl, rrl;
    // arl = glGetUniformLocation(ctx->Shader, "aspectRatio");
    // rrl = glGetUniformLocation(ctx->Shader, "radiusRatio");

    u32 WidthLoc, HeightLoc, RadiusLoc;
    WidthLoc = glGetUniformLocation(ctx->Shader, "uiWidth");
    HeightLoc = glGetUniformLocation(ctx->Shader, "uiHeight");
    RadiusLoc = glGetUniformLocation(ctx->Shader, "radius");

    glUniform1f(WidthLoc, w);
    glUniform1f(HeightLoc, h);
    glUniform1f(RadiusLoc, 40);

    glBindBuffer(GL_ARRAY_BUFFER, ctx->VertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->IndexBuffer);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

void
UI_FillTexQuad(ui_render_ctx* ctx, f32 x, f32 y, f32 w, f32 h, u32 Color, texture* Texture)
{
    Assert(ctx != NULL);

    glm::mat4 Model = glm::mat4(1.0f);
    Model = glm::translate(Model, glm::vec3(x, y, 0));
    Model = glm::scale(Model, glm::vec3(w, h, 1));

    glBindVertexArray(ctx->VertexArray);
    glUseProgram(ctx->Shader);

    u32 ModelLoc, ColorLoc, ViewProjLocation;
    ModelLoc = glGetUniformLocation(ctx->Shader, "Model");
    glUniformMatrix4fv(ModelLoc, 1, GL_FALSE, glm::value_ptr(Model));

    glm::mat4 ViewProj = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f);

    ViewProjLocation = glGetUniformLocation(ctx->Shader, "ViewProj");
    glUniformMatrix4fv(ViewProjLocation, 1, GL_FALSE, glm::value_ptr(ViewProj));

    u8 r, g, b, a;
    DecodeRGBA(Color, &r, &g, &b, &a);

    ColorLoc = glGetUniformLocation(ctx->Shader, "Color");
    glUniform4f(ColorLoc, r / 255.0, g / 255.0, b / 255.0, a / 255.0);

    glBindBuffer(GL_ARRAY_BUFFER, ctx->VertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->IndexBuffer);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Texture->Id);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

// It will be replaced in the shipping code with dynamic vectors
#define MAX_UI_ELEMENTS 3000
#define UI_ID (__LINE__)

struct ui_io
{
    struct
    {
        s32 x, y;
    }Pointer;
    struct
    {
        b32 Hit;
    };
};

// TODO(rajat): These must be tuned for performance
#define MAX_UI_TEXQUADS 100

struct ui_context
{
    ui_io io;
    f32 ViewPortW;
    f32 ViewPortH;
    u32 ActiveIdx;
    u32 HotIdx;
};

ui_context UIContext = {};

void
UIBegin(ui_io* Inputs, f32 ViewPortW, f32 ViewPortH)
{
    UIContext.io.Pointer = Inputs->Pointer;
    UIContext.io.Hit = Inputs->Hit;

    UIContext.ViewPortW = ViewPortW;
    UIContext.ViewPortH = ViewPortH;

    UIContext.HotIdx = 0;

    // NOTE(rajat): Should it be cleared to zero
    UIContext.ActiveIdx = 0;
}

b32 UIButton(const char* Title, f32 x, f32 y, f32 w, f32 h)
{
    if(RectangleContainsPoint({x, y, w, h}, {UIContext.io.Pointer.x, UIContext.io.Pointer.y}))
    {
        FillQuad(x, y, w, h, 0x000000FF);
        FillText(Title, x, y, 32, 0xFFFFFFFF);
        return true;
    }

    FillQuad(x, y, w, h, 0xFFFFFFFF);
    FillText(Title, x, y, 32, 0x000000FF);
}

void
UIBeginWindow(const char* Title, u32 w, u32 h)
{
}

void
UIEndWindow()
{
}

void
UIEnd()
{
    UIContext.HotIdx = 0;
    UIContext.ActiveIdx = 0;
}
