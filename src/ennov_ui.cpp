#include "ennov_platform.h"

// NOTE(rajat): Platform independence defines for rendering apis
struct texture;

typedef u32 vertex_array;
typedef u32 vertex_buffer;
typedef u32 shader;

#include "ennov_gl.h"

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

#define DATA_PER_VERTEX_TEXQUAD 9
#define VERTEX_BUFFER_SIZE (MAX_UI_TEXQUADS * 6 * DATA_PER_VERTEX_TEXQUAD)

static vertex_array VertexArray = {};
static vertex_buffer VertexBuffer = {};

static glm::mat4 Projection;
static glm::mat4 View;

static f32 VertexBufferData[VERTEX_BUFFER_SIZE];
static u32 VertexBufferCurrentPos = {};

// NOTE(rajat): Is this has to be a pointer
// static texture *FontTexture = {};

static s32 NumTextureSlots;
static u32 NumBindTextureSlots = 1;
static u32 TextureMap[32];

static shader TexQuadShader = {};
internal_ void
CreateRenderObjects()
{
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &NumTextureSlots);

    glGenVertexArrays(1, &VertexArray);
    glBindVertexArray(VertexArray);

    glGenBuffers(1, &VertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);

    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(float) * 6 * DATA_PER_VERTEX_TEXQUAD * MAX_UI_TEXQUADS,
                 NULL, GL_DYNAMIC_DRAW);

    char TempBuffer[2048];
    sprintf(TempBuffer, TexQuadFragmentShaderSource, NumTextureSlots);

    TexQuadShader = CreateShaderProgram(TexQuadVertexShaderSource, TempBuffer);

    glUseProgram(TexQuadShader);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    // NOTE(Rajat): Always set proper stride values otherwise go under a huge
    // Debugging sesssion.
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 9, 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 9, (void*)(sizeof(float) * 4));
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(float) * 9, (void*)(sizeof(float) * 8));

    Projection = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f);

    u32 ProjectionLocation = glGetUniformLocation(TexQuadShader, "Proj");
    glUniformMatrix4fv(ProjectionLocation, 1, GL_FALSE, glm::value_ptr(Projection));

    u32 SamplerLocation = glGetUniformLocation(TexQuadShader, "Textures");
    s32 Samplers[32];

    for(int i = 0; i < 32; ++i)
    {
        Samplers[i] = i;
    }

    glUniform1iv(SamplerLocation, NumTextureSlots, Samplers);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

internal_ void
FlushDrawData();

void
vFillQuad(f32 x, f32 y, f32 w, f32 h, u32 Color)
{
    if(VertexBufferCurrentPos + 54 >= VERTEX_BUFFER_SIZE)
        FlushDrawData();

    // TODO(rajat): Implement for big endian also
    u8 r = Color >> 24;
    u8 g = (Color << 8) >> 24;
    u8 b = (Color << 16) >> 24;
    u8 a = (Color << 24) >> 24;

    // This index will always be negative
    f32 Index = -1.0f;

    glm::mat4 Model = glm::mat4(1.0f);

    Model = glm::translate(Model, glm::vec3(x, y, 0));
    Model = glm::scale(Model, glm::vec3(w, h, 0));

    glm::vec4 tvd[6] = {
        {0.0f, 1.0f, 0.0f, 1.0f},
        {1.0f, 0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},

        {0.0f, 1.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, 0.0f, 1.0f},
        {1.0f, 0.0f, 0.0f, 1.0f}
    };

    for(int i = 0; i < 6; ++i) {
        tvd[i] = Model * tvd[i];
    }

    f32 VertexData[54] = {
        tvd[0].x, tvd[0].y, 0, 0, r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f, Index,
        tvd[1].x, tvd[1].y, 0, 0, r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f, Index,
        tvd[2].x, tvd[2].y, 0, 0, r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f, Index,
        tvd[3].x, tvd[3].y, 0, 0, r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f, Index,
        tvd[4].x, tvd[4].y, 0, 0, r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f, Index,
        tvd[5].x, tvd[5].y, 0, 0, r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f, Index
    };

    for(int i = 0; i < 54; ++i) {
        VertexBufferData[VertexBufferCurrentPos + i] = VertexData[i];
    }

    VertexBufferCurrentPos += 54;
}

internal_ void
FlushDrawData()
{
    glBindVertexArray(VertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);

    glUseProgram(TexQuadShader);

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * VertexBufferCurrentPos, VertexBufferData);
    glDrawArrays(GL_TRIANGLES, 0, VertexBufferCurrentPos/9);

    VertexBufferCurrentPos = 0;
    NumBindTextureSlots = 0;
}


// TODO(rajat): Use static text data directly
internal_ void
GenFontTexture(const char* FontFile)
{
}

struct ui_context
{
    ui_io io;
    f32 ViewPortW;
    f32 ViewPortH;
    u32 ActiveIdx;
    u32 HotIdx;

    renderer_data *Renderer;
};

ui_context UIContext = {};

void
UIBegin(renderer_data* Renderer, ui_io* Inputs, f32 ViewPortW, f32 ViewPortH)
{
    UIContext.io.Pointer = Inputs->Pointer;
    UIContext.io.Hit = Inputs->Hit;

    UIContext.ViewPortW = ViewPortW;
    UIContext.ViewPortH = ViewPortH;

    UIContext.HotIdx = 0;

    // NOTE(rajat): Should it be cleared to zero
    UIContext.ActiveIdx = 0;

    CreateRenderObjects();

    // TODO(rajat): Moving a separate renderer to UI system
    UIContext.Renderer = Renderer;
}

void
UIBeginWindow(const char* Title, u32 w, u32 h)
{
    FillQuad(100, 100, 100, 100, 0xFF0000FF);
    FillQuad(200, 200, 100, 100, 0x0F0F0FE0);
    FillQuad(300, 300, 100, 100, 0xD0fcbaFF);
    FillQuad(400, 400, 100, 100, 0xEAdfcbFF);
    FillQuad(500, 200, 100, 100, 0xFCaddcFF);
    FillQuad(600, 300, 100, 100, 0xADbbedFF);
    FlushDrawData();
}

void
UIEndWindow()
{
}

void
UIEnd()
{
    UIContext.ActiveIdx = 0;
}
