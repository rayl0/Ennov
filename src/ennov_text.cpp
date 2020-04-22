#include "ennov.h"
#include "ennov_gl.h"
#include "ennov_utils.h"
#include "glad/glad.h"

#include <string.h>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

void
InitializeAreana(game_areana* Areana, void* BaseAddress, u32 Size)
{
    Areana->BaseAddress = BaseAddress;
    Areana->Size = Size;
    Areana->Used = 0;
}

u32
CalculatePadding(u64 Address, u32 Align)
{
    u32 Padding = (Address % Align);
    return (Padding == 0) ? 0 : (Align - Padding);
}

void*
PushStruct_(game_areana* Areana, memory_index Size)
{
    Assert(Areana->Used + Size <= Areana->Size);
    void* NewStruct = ((char*)Areana->BaseAddress + Areana->Used);
    u64 Address = ((u64)(Areana->BaseAddress) + Areana->Used);
    u32 Padding = CalculatePadding(Address, 8);
    Areana->Used += Size + Padding;
    return NewStruct;
}

#define MAX_FONTQUADS 200
#define FONT_VERTEXBUFFER_SIZE MAX_FONTQUADS * 4 * 6

struct font_render_data
{
    fontinfo CurrentFont;
    texture Texture;
    glm::mat4 ViewProj;

    u32 VertexArray;
    u32 VertexBuffer;

    f32 VertexBufferData[FONT_VERTEXBUFFER_SIZE];
    u32 VertexBufferCurrentPos;

    u32 Shader;
};

global_variable u32 ViewProjLoc;
global_variable u32 WidthLoc;
global_variable u32 EdgeLoc;
global_variable u32 BorderWidthLoc;
global_variable u32 BorderEdgeLoc;
global_variable u32 BorderColorLoc;
global_variable u32 FontColorLoc;

global_variable font_render_data FontRenderData;

void
CreateFontRenderObjects(const char* VertexShader, const char* FragmentShader)
{
    glGenVertexArrays(1, &FontRenderData.VertexArray);
    glBindVertexArray(FontRenderData.VertexArray);

    glGenBuffers(1, &FontRenderData.VertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, FontRenderData.VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * 4 * 6 * MAX_FONTQUADS,
                 NULL, GL_DYNAMIC_DRAW);

    FontRenderData.Shader = CreateShaderProgram(VertexShader, FragmentShader);
    glUseProgram(FontRenderData.Shader);

    // TODO(rajat): Move all blends to draw call functions
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    FontRenderData.ViewProj = glm::ortho(0.0f, (f32)GameState->ContextAttribs.Width,
                                         (f32)GameState->ContextAttribs.Height,
                                         0.0f, -1.0f, 1.0f);

    ViewProjLoc = glGetUniformLocation(FontRenderData.Shader, "ViewProj");
    glUniformMatrix4fv(ViewProjLoc, 1, GL_FALSE, glm::value_ptr(FontRenderData.ViewProj));

    WidthLoc = glGetUniformLocation(FontRenderData.Shader, "Width");
    glUniform1f(WidthLoc, 0.5);

    EdgeLoc = glGetUniformLocation(FontRenderData.Shader, "Edge");
    glUniform1f(EdgeLoc, 0.1);

    BorderWidthLoc = glGetUniformLocation(FontRenderData.Shader, "BorderWidth");
    glUniform1f(BorderWidthLoc, 0.7);

    BorderEdgeLoc = glGetUniformLocation(FontRenderData.Shader, "BorderEdge");
    glUniform1f(BorderEdgeLoc, 0.1);

    BorderColorLoc = glGetUniformLocation(FontRenderData.Shader, "BorderColor");
    glUniform3f(BorderColorLoc, 1.0, 0.0, 0.0);

    FontColorLoc = glGetUniformLocation(FontRenderData.Shader, "FontColor");
    glUniform4f(FontColorLoc, 1.0, 1.0, 0.0, 1.0);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
}

static glm::vec4 fqd[6] = {
    {0.0f, 1.0f, 0.0f, 1.0f},
    {1.0f, 0.0f, 0.0f, 1.0f},
    {0.0f, 0.0f, 0.0f, 1.0f},

    {0.0f, 1.0f, 0.0f, 1.0f},
    {1.0f, 1.0f, 0.0f, 1.0f},
    {1.0f, 0.0f, 0.0f, 1.0f}
};

internal_ void
FontRenderCommit(u32 FontColor, f32 Width, f32 Edge,
                 u32 BorderColor, u32 BorderWidth, u32 BorderEdge);


internal_ void
FillSingleFontQuadClipped(f32 x, f32 y, f32 w, f32 h,
                   texture* Texture, f32 s0, f32 t0, f32 s1, f32 t1)
{
    Assert(FontRenderData.VertexBufferCurrentPos + 54 <= VERTEX_BUFFER_SIZE);

    glm::mat4 Model = glm::mat4(1.0f);

    Model = glm::translate(Model, glm::vec3(x, y, 0));
    Model = glm::scale(Model, glm::vec3(w, h, 0));

    glm::vec4 tvd[6];

    for(int i = 0; i < 6; ++i) {
        tvd[i] = Model * fqd[i];
    }

    f32 VertexData[24] = {
        tvd[0].x, tvd[0].y, s0, t1,
        tvd[1].x, tvd[1].y, s1, t0,
        tvd[2].x, tvd[2].y, s0, t0,
        tvd[3].x, tvd[3].y, s0, t1,
        tvd[4].x, tvd[4].y, s1, t1,
        tvd[5].x, tvd[5].y, s1, t0
    };

    for(int i = 0; i < 24; i++)
    {
        FontRenderData.VertexBufferData[FontRenderData.VertexBufferCurrentPos + i] = VertexData[i];
    }

    FontRenderData.VertexBufferCurrentPos += 24;
}

void
FillText(const char* s, f32 x, f32 y, f32 Size, u32 Color, f32 Width, f32 Edge, u32 BorderColor,
         f32 BorderWidth, f32 BorderEdge);

void
FillText(const char* s, f32 x, f32 y, f32 Size, u32 Color)
{
    f32 Width = 0.46, Edge = 0.19;
    f32 Scale = normf(0, FontRenderData.CurrentFont.Size, Size);
    if(Scale >= 0.3f)
    {
        Width = 0.5;
        Edge = 0.1;
    }

    FillText(s, x, y, Size, Color
             , Width, Edge, 0x0000000, 0.0, 0.1);
}

void
FontRenderCommit(u32 FontColor, f32 Width, f32 Edge,
                 u32 BorderColor, f32 BorderWidth, f32 BorderEdge)
{
    glBindVertexArray(FontRenderData.VertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, FontRenderData.VertexBuffer);

    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    sizeof(f32) * FontRenderData.VertexBufferCurrentPos,
                    FontRenderData.VertexBufferData);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, FontRenderData.Texture.Id);

    glUseProgram(FontRenderData.Shader);

    glUniform1f(BorderWidthLoc, BorderWidth);
    glUniform1f(BorderEdgeLoc, BorderEdge);
    glUniform1f(WidthLoc, Width);
    glUniform1f(EdgeLoc, Edge);

    u8 r, g, b, a;
    DecodeRGBA(FontColor, &r, &g, &b, &a);
    glUniform4f(FontColorLoc, r / 0xFF, g / 0xFF, b / 0xFF, a / 0xFF);

    DecodeRGBA(BorderColor, &r, &g, &b, &a);
    glUniform3f(BorderColorLoc, r / 0xFF, g / 0xFF, b / 0xFF);

    FontRenderData.ViewProj = glm::ortho(0.0f, (f32)GameState->ContextAttribs.Width,
                                         (f32)GameState->ContextAttribs.Height,
                                         0.0f, -1.0f, 1.0f);

    glUniformMatrix4fv(ViewProjLoc, 1, GL_FALSE, glm::value_ptr(FontRenderData.ViewProj));

    glDrawArrays(GL_TRIANGLES, 0, FontRenderData.VertexBufferCurrentPos / 4);

    FontRenderData.VertexBufferCurrentPos = 0;
}

void
FillText(const char* s, f32 x, f32 y, f32 Size, u32 Color, f32 Width, f32 Edge)
{
    FillText(s, x, y, Size, Color, Width, Edge, 0x0000000, 0.0, 0.1);
}

void
FillText(const char* s, f32 x, f32 y, f32 Size, u32 Color, u32 BorderColor)
{
    FillText(s, x, y, Size, Color, 0.5, 0.1, BorderColor, 0.7, 0.1);
}

void
FillText(const char* s, f32 x, f32 y, f32 Size, u32 Color, f32 Width, f32 Edge, u32 BorderColor)
{
    FillText(s, x, y, Size, Color, Width, Edge, BorderColor, Width + 0.2, Edge + 0.1);
}

internal_ f32
GetScale(f32 FontSize, f32 Size)
{
    return Size/FontSize;
}

void
FillText(const char* s, f32 x, f32 y, f32 Size, u32 Color, f32 Width, f32 Edge, u32 BorderColor,
         f32 BorderWidth, f32 BorderEdge)
{
    // TODO(rajat): Remove reference and use a pointer instead
    fontinfo& CurrentFont = FontRenderData.CurrentFont;
    f32 Scale = normf(0, FontRenderData.CurrentFont.Size, Size);

    f32 x1 = x;
    f32 y1 = y;

    while(*s)
    {
        if(*s >= 32 && (int)*s < 128)
        {
            fontface c = CurrentFont.Fonts[*s];

            f32 w = c.w - CurrentFont.PaddingWidth + (2 * DESIRED_PADDING);
            f32 h = c.h - CurrentFont.PaddingHeight + (2 * DESIRED_PADDING);
            f32 x0 = c.x + CurrentFont.Padding[PAD_LEFT] - DESIRED_PADDING;
            f32 y0 = c.y + CurrentFont.Padding[PAD_TOP] - DESIRED_PADDING;
            f32 xoff = c.xoffset + CurrentFont.Padding[PAD_LEFT] - DESIRED_PADDING;
            f32 yoff = c.yoffset - CurrentFont.Padding[PAD_TOP];

            FillSingleFontQuadClipped(x + xoff * Scale,
                                      y + yoff * Scale,
                                      w * Scale,
                                      h * Scale,
                                      &FontRenderData.Texture,
                                      x0 / CurrentFont.FontBitmapWidth,
                                      y0 / CurrentFont.FontBitmapHeight,
                                      (x0 + w) / CurrentFont.FontBitmapWidth,
                                      (y0 + h) / CurrentFont.FontBitmapHeight);

            x += (c.xadvance - CurrentFont.PaddingWidth) * Scale;

            if(((x + w) / GameState->ContextAttribs.Width) >= 1.0f && *s ==' ') {
                f32 y0 = y1;
                y = y0 + CurrentFont.LineHeight * Scale - CurrentFont.PaddingHeight * Scale;
                y1 = y;
                x = x1;
            }
        }
        ++s;
    }
    FontRenderCommit(Color, Width, Edge, BorderColor, BorderWidth, BorderEdge);
}
// STUDY(rajat): stb_truetype and stb_textedit
void
LoadFont(const char* File)
{
    GetFontInfo(File, &FontRenderData.CurrentFont);
    FontRenderData.Texture = CreateTexture(FontRenderData.CurrentFont.FontBitmap);
}
