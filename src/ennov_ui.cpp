#include "ennov_gl.h"
#include "ennov_platform.h"

// It will be replaced in the shipping code with dynamic vectors
#define MAX_UI_ELEMENTS 300
#define UI_ID (__LINE__)

rect UIBoundBoxes[MAX_UI_ELEMENTS];

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

struct ui_context
{
    ui_io io;
    f32 ViewPortW;
    f32 ViewPortH;
    u32 ActiveIdx;
    u32 HotIdx;

    renderer_data* Renderer;
};

ui_context UIContext;

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

    // TODO(rajat): Moving a separate renderer to UI system
    UIContext.Renderer = Renderer;
}

void
UIBeginWindow(const char* Title, u32 w, u32 h)
{
    // HEADER
    DrawBatchRectangle(UIContext.Renderer, NULL, {0,0,1,1.0f}, NULL,
                       {0, 0}, {UIContext.ViewPortW, 50});

    // Title
    DrawString(Title, 20, 20, 1, {1,1,1,1});
    // DrawBody
    DrawBatchRectangle(UIContext.Renderer, NULL, {0, 0, 0, 1.0f}, NULL,
                       {0, 50}, {UIContext.ViewPortW, 600});
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
