#include "ennov_platform.h"
#include "ennov_math.h"

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
