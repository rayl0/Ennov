#include <stdio.h>
#include <unistd.h>
#include <memory.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "ennov_font_parser.cpp"

#include "ennov_text.cpp"
#include "ennov_math.h"
#include "ennov_gl.cpp"
#include "ennov_ui.cpp"

#include "ennov_platform.h"
#include "ennov.h"

// TODO(rajat): Fix bug, Position of you win text depends upon numactive tiles, >= or ==
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define SMOOTHSTEP(x) ((x) * (x) * (3 - 2 * (x)))
static bool Toggle = false;

loaded_bitmap*
LoadPixelsFrom(const char* FileName, game_areana* Areana)
{
    game_file* File = (game_file*)PlatformLoadFile(FileName, PushStruct_, Areana);
    if(File)
    {
        loaded_bitmap* NewBitmap = PushStruct(Areana, loaded_bitmap);
        NewBitmap->Pixels = stbi_load_from_memory((stbi_uc*)File->Data, File->Size, &NewBitmap->Width, &NewBitmap->Height, &NewBitmap->Channels, 0);

        return NewBitmap;
    }
    return NULL;
}

// TODO(Rajat): Implement Rand number generator
// TODO(Rajat): Implement Matrix and transformation methods
// TODO(Rajat): Don't start a new project after coming back on
// this after march exams
// NOTE(Rajat): Don't ever change your editor from spacemacs

enum breakout_batch_id
{
    BackgroundBatch,
    PaddleBatch,
    BallBatch,
    TileBatch
};

struct breakout_game_state
{
    b32 HaveLoadState;
    b32 IsInitialized;
    loaded_bitmap *BackgroundBitmap;
    loaded_bitmap *PaddleBitmap;
    loaded_bitmap *BallBitmap;
    loaded_bitmap *TileBitmap;
    texture Textures[4];
    game_file *TextFontFile;
    b32 Fired;
    vec2 Direction;
    rect Ball;
    rect Paddle;
    u8 Level[32];
    s32 Lives;
    b32 IsPaused;
};

static ui_render_ctx UI_Ctx;

struct breakout_save_data
{
    u32 NumLives;
    b32 IsPaused;
    u8 LevelData[32];
};

#define PaddleVelocity 25

#define WasPressed(Input, x) Input->Button.x.EndedDown

void GameUpdateAndRender(game_memory* Memory, game_state *State, game_input *Input, u32 *ConfigBits)
{
    // NOTE(Rajat): Never do assertions within a loop increment
    // Assert(Count < 1);

    #if ENNOV_PLATFORM_ANDROID
    #endif

    breakout_game_state* CurrentState = (breakout_game_state*)Memory->PermanentStorage;
    if(!Memory->IsInitialized) {
        InitializeAreana(&State->GameStorage, (char*)Memory->PermanentStorage + sizeof(CurrentState), Memory->PermanentStorageSize - sizeof(CurrentState));
        InitializeAreana(&State->ScratchStorage, Memory->TransientStorage, Memory->TransientStorageSize);
        InitializeAreana(&State->AssestStorage, Memory->AssetMemory, Memory->AssetMemorySize);

        CurrentState->Paddle = {0.0f, 550.0f, 100.0f, 50.0f};
        CurrentState->Ball = {CurrentState->Paddle.Dimensions.x / 2.0f, 530.0f, 20.0f, 20.0f};
        CurrentState->Direction = {1.0f, 2.0f};
        CurrentState->Fired = false;
        CurrentState->Lives = 3;
        CurrentState->IsPaused = false;
        CurrentState->IsInitialized = true;

        // TODO(Rajat): Move OpenGL code to platform layer and introduce command buffer
        if(State->SaveDataSize) {
            breakout_save_data* SaveData = (breakout_save_data*)State->GameSaveData;

            CurrentState->Lives = SaveData->NumLives;
            CurrentState->IsPaused = SaveData->IsPaused;

            for(int i = 0; i < 32; ++i) {
                CurrentState->Level[i] =  SaveData->LevelData[i];
            }
        }
        else
        {
            u32 TileMap[32] = {
                1, 2, 3, 1, 4, 1, 2, 3,
                5, 3, 5, 1, 3, 5, 4, 1,
                4, 1, 3, 5, 2, 1, 2, 5,
                0, 0, 1, 2, 4, 3, 0, 0
            };

            for(int i = 0; i < 32; ++i) {
                CurrentState->Level[i] =  TileMap[i];
            }
        }

        CurrentState->BackgroundBitmap = LoadPixelsFrom("assets/background.jpg", &State->AssestStorage);
        CurrentState->BallBitmap = LoadPixelsFrom("assets/background.jpg", &State->AssestStorage);
        CurrentState->PaddleBitmap = LoadPixelsFrom("assets/paddle.png", &State->AssestStorage);
        CurrentState->TileBitmap = LoadPixelsFrom("assets/container.png", &State->AssestStorage);

        // asset_work_queue WorkQueue;

        CurrentState->Textures[0] = CreateTexture(CurrentState->BackgroundBitmap);
        CurrentState->Textures[1] = CreateTexture(CurrentState->TileBitmap);
        CurrentState->Textures[2] = CreateTexture(CurrentState->PaddleBitmap);

        CurrentState->TextFontFile = (game_file*)PlatformLoadFile("assets/s.ttf", PushStruct_, &State->AssestStorage);

        // TODO(rajat): Load precompiled shaders if possible
        game_file* VertexShaderFile = (game_file*)PlatformLoadFile("shaders/gquad.vert",  PushStruct_, &State->AssestStorage);
        game_file* FragmentShaderFile = (game_file*)PlatformLoadFile("shaders/gquad.frag", PushStruct_, &State->AssestStorage);

        game_file* FontVertexShader = (game_file*)PlatformLoadFile("shaders/font.vert", PushStruct_,
                                                                   &State->AssestStorage);
        game_file* FontFragmentShader = (game_file*)PlatformLoadFile("shaders/font.frag", PushStruct_,
                                                                     &State->AssestStorage);
        game_file* UI_VertexShader = (game_file*)PlatformLoadFile("shaders/ui.vert", PushStruct_, &State->AssestStorage);
        game_file* UI_FragmentShader = (game_file*)PlatformLoadFile("shaders/ui.frag", PushStruct_, &State->AssestStorage);

        CreateRenderContext((const char*)VertexShaderFile->Data, (const char*)FragmentShaderFile->Data);
        UI_CreateContext(&UI_Ctx, (const char*)UI_VertexShader->Data, (const char*)UI_FragmentShader->Data);
        GameState = State;
        LoadFont("assets/fonts/default.fnt");
        CreateFontRenderObjects((const char*)FontVertexShader->Data, (const char*)FontFragmentShader->Data);

        Memory->IsInitialized = true;
    }

    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0, 0, 0, 1);

    rect* Ball = &CurrentState->Ball;
    rect* Paddle = &CurrentState->Paddle;
    vec2* Direction = &CurrentState->Direction;

    if(WasPressed(Input, S)) {
        if(CurrentState->IsPaused)
        {
            fprintf(stderr, "Resuming the game\n");
            CurrentState->IsPaused = false;
        }
        else {
            fprintf(stderr, "Pausing the game\n");
            CurrentState->IsPaused = true;
        }
    }
    if(!CurrentState->IsPaused) {
        if(WasPressed(Input, Start)) {
            CurrentState->Fired = true;
        }
        if(CurrentState->Fired) {
            f32 dv = normf(State->dt, 0, 0.16);
            dv = SMOOTHSTEP(dv);

            Ball->Pos.y -= 3 * Direction->y * dv;
            Ball->Pos.x += 3 * Direction->x * dv;

            Ball->Pos.x = clampf(Ball->Pos.x, 0, 800 - Ball->Dimensions.x);
            Ball->Pos.y = clampf(Ball->Pos.y, 0, 800 - Ball->Dimensions.y);
        }
        if(Ball->Pos.x >= 780.0f) Direction->x = -(Direction->x);
        if(Ball->Pos.y <= 0.0f) Direction->y = -(Direction->y);
        if(Ball->Pos.x <= 0.0f) Direction->x = -(Direction->x);
        if(Ball->Pos.y >= 600.0f) {
            CurrentState->Fired = false;
            Ball->Pos = {100.0f, 550.0f - Ball->Dimensions.y};
            *Direction = { 1.0f, 2.0f};
            --(CurrentState->Lives);
            fprintf(stderr, "One Life deducted!\n");
            if(CurrentState->Lives == 0)
            {
                fprintf(stderr, "You Lose!\n");
                exit(EXIT_FAILURE);
            }
        }
        if(RectangleColloide(*Paddle, *Ball))
        {
            if(CurrentState->Fired)
                Ball->Pos = {Ball->Pos.x, 540.0f - Ball->Dimensions.y};

            Direction->x = (Direction->x);
            Direction->y = -(Direction->y);
        }
        if(!CurrentState->Fired) {
            Ball->Pos.x = Paddle->Pos.x + Paddle->Dimensions.x / 2.0f;
            Ball->Pos.y = 550.0f - Ball->Dimensions.y;
        }
        if(Input->Button.MoveRight.EndedDown)
        {
            f32 dv = normf(State->dt, 0, 0.16);
            dv = SMOOTHSTEP(dv);
            Paddle->Pos.x += dv * PaddleVelocity;
            Paddle->Pos.x = clampf(Paddle->Pos.x, 0, 800 - Paddle->Dimensions.x);
        }
        if(Input->Button.MoveLeft.EndedDown)
        {
            f32 dv = normf(State->dt, 0, 0.16);
            dv = SMOOTHSTEP(dv);
            Paddle->Pos.x -= dv * PaddleVelocity;
            Paddle->Pos.x = clampf(Paddle->Pos.x, 0, 800 - Paddle->Dimensions.x);
        }
    }

    // TODO(rajat): Introduce new buttons in game_input struct for this
    if(WasPressed(Input, Select))
    {
        *ConfigBits = *ConfigBits ^ PlatformFullScreenToggle_BIT;
    }

    u32 NumActieTiles = 0;

    vec2 StartPosition = {100, 300};
    vec2 EndPosition = {700, 300};

    static f32 i = 0;

    f32 x = 600 * i;

    i += .01;

    if(i > 1.0)
        i = 0;

    static f32 t = 0;

    f32 n = normf(t, 0, 1.5);
    n = SMOOTHSTEP(n);

    // NOTE(rajat): Always remember the range of u8 0...255, not 256
    u8 S = (u8)lerpf(1 - n, 0, 255);

    u8 r = 0 * n, g = 0 * n, b = 0 * n;
    u32 Color = EncodeRGBA(r, g, b, S);

    t += (State->dt / 10);
    t = clampf(t, 0, 1.5);


    u32 Color2 = EncodeRGBA(255 * n, 255 * n, 255 * n, 255 * n);

    // TODO(rajat): Add src clipping to the renderer
    FillTexQuad(00, 00, 800, 600, Color2, &CurrentState->Textures[0]);

    // printf("Alpha: %u\n", S);

    u8* Level = CurrentState->Level;

    for(int i = 0; i < 4; ++i)
    {
        for(int j = 0; j < 8; ++j) {
            vec2 Pos = {j * 100.0f, i * 50.0f};
            vec2 Dim = {100.0f, 50.0f};
            u32 Color = {};
            if(Level[i * 8 + j] == 0)
                continue;
            else if(Level[i * 8 + j] == 1)
                Color = 0x007A7AFF;
            else if(Level[i * 8 + j] == 2)
                Color = 0xFF761FFF;
            else if(Level[i * 8 + j] == 3)
                Color = 0x005FFFFF;
            else if(Level[i * 8 + j] == 4)
                Color = 0xF06FFFFF;
            else if(Level[i * 8 + j] == 5)
                Color = 0x00FFFFFF;

            u8 r, g, b, a;
            DecodeRGBA(Color, &r, &g, &b, &a);

            a = 255 * n;
            Color = EncodeRGBA(r, g, b, a);

            FillTexQuad(Pos.x, Pos.y, Dim.x, Dim.y, Color, &CurrentState->Textures[1]);
            if(RectangleColloide({Pos, Dim}, CurrentState->Ball))
            {
                fprintf(stderr, "Tile Colloide %i\n", Level[i * 8 + j]);
                CurrentState->Level[i * 8 + j] = 0;
                Direction->x = Direction->x;
                Direction->y = -Direction->y;
            }

            if(Level[i * 8 + j] != 0)
            {
                NumActieTiles++;
            }
        }
    }

    CurrentState->IsPaused = Toggle;


    if(Input->Button.Terminate.EndedDown)
    {
        if(NumActieTiles == 0)
            State->GameSaveData = NULL;
        else
        {
            breakout_save_data* SaveData = (breakout_save_data*)State->GameSaveData;

            SaveData->NumLives = CurrentState->Lives;
            SaveData->IsPaused = CurrentState->IsPaused;

            for(int i = 0; i < 32; i++)
                SaveData->LevelData[i] = CurrentState->Level[i];

            State->SaveDataSize = sizeof(breakout_save_data);
        }

        Ball->Pos = {Paddle->Pos.x + Paddle->Dimensions.x / 2, 550.0f - Ball->Dimensions.y};
        CurrentState->Fired = false;
    }

    FillTexQuad(Paddle->Pos.x, Paddle->Pos.y,
                Paddle->Dimensions.x, Paddle->Dimensions.y,
                Color2, &CurrentState->Textures[2]);
    FillTexQuad(Ball->Pos.x, Ball->Pos.y,
                Ball->Dimensions.x, Ball->Dimensions.y,
                Color2, &CurrentState->Textures[0]);

    FillQuad(0, 0, 800, 600, Color);

    static u32 WasHit = 0;

    if(!WasHit)
    if(RectangleContainsPoint({200, 200, 100, 50}, {Input->Cursor.X, Input->Cursor.Y}))
        FillQuad(200, 200, 100, 50, 0x000000FF);
    else
        FillQuad(200, 200, 100, 50, 0xFFFFFFFF);
    else
        FillQuad(200, 200, 100, 50, 0x000000FF);


    if(!WasHit)
    if(RectangleContainsPoint({200, 200, 100, 50}, {Input->Cursor.X, Input->Cursor.Y}))
        FillText("Button", 200, 200, 32, 0xFFFFFFFF);
    else
        FillText("Button", 200, 200, 32, 0x0000000FF);
    else
        FillText("Button", 200, 200, 32, 0x0000FFFF);

    f32 ystep = 0;
    f32 wspacing = 20;

    static f32 winx = 400, winy = 100;
    f32 winw = 200, winh = 400;
    static b32 WinToggle = 0;
    static u32 Active = 0;

    if(RectangleContainsPoint({winx + winw / 2 - 20 / 2, winy, 20, 20}, Input->Cursor.at))
    {
        if(Input->Cursor.Drag)
        {
            WinToggle = true;
        }
    }
    if(!Input->Cursor.Drag && WinToggle)
        WinToggle = false;

    if(WinToggle)
    {
        winx += clampf(Input->Cursor.X, 0, 800) - (winx + winw / 2 - 20 / 2);
        winy = clampf(Input->Cursor.Y, 0, 600 - winh);
    }

    FillQuad(winx, winy, winw, winh, 0x141414FF);
    FillQuad(winx + winw / 2 - 20 / 2, winy, 20, 20, 0x007A7A7F);

    if(RectangleContainsPoint({200, 200, 100, 50}, {Input->Cursor.X, Input->Cursor.Y}) && Input->Cursor.Hit)
        WasHit = WasHit ^ 1;

    f32 wslid = winw * 0.85, hslid = 20;
    f32 xslid = winx + (winw) / 2 - wslid / 2, yslid = winy + wspacing + 10;
    ystep += winy + hslid + wspacing * 2 + 10;

    static u32 BasicColor = 0x141414FF;
    static u32 HilightColor = 0x007A7A3F;
    static u32 Color3 = BasicColor;

    FillQuad(xslid, yslid, wslid, hslid, Color3);

    f32 wper = 0.85 * wslid;
    f32 hper = lerpf(0.25, 0, hslid);
    f32 xper = xslid + wslid / 2 - wper / 2;
    f32 yper = yslid + hslid / 2 - hper / 2;

    FillQuad(xper, yper, wper, hper, 0x007A7A5F);

    f32 wtogg = lerpf(0.05, 0, wper);
    f32 htogg = lerpf(0.7, 0, hslid);
    static f32 xtogg = xper;
    f32 ytogg = yslid + hslid / 2 - htogg / 2;

    static b32 Toggled = false;
    f32 Color5 = 0x00AFAFFF;

    if(Toggled)
    {
        xtogg = clampf(Input->Cursor.X, xper, xper + wper - wtogg);
        Color5 = 0x00FFFFFF;
    }

    static f32 xtogglehit = 0;

    if(RectangleContainsPoint({xslid, yslid, wslid, hslid}, {Input->Cursor.X, Input->Cursor.Y}))
    {
        if(Toggled)
        {
        }
        else if(Input->Cursor.Drag)
        {
            xtogg = clampf(Input->Cursor.X, xper, xper + wper - wtogg);
        }

        Color3 = HilightColor;
    }
    else
    {
        Color3 = BasicColor;
    }

    if(RectangleContainsPoint({xtogg, ytogg, wtogg, htogg}, {Input->Cursor.X, Input->Cursor.Y}))
    {
        if(Input->Cursor.Drag)
        {
            Toggled = true;
        }
    }

    if(!Input->Cursor.Drag && Toggled) {
        Toggled = false;
    }

    FillQuad(xtogg, ytogg, wtogg, htogg, Color5);
    FillQuad(xper, yper, (xtogg - xper), hper, 0x009F9FFF);

    f32 FontSize;

    char Buffer[50];

    FontSize = mapf(xtogg - xper + wtogg, wtogg, wper, 14, 72);
    sprintf(Buffer, "%f", FontSize);

    FillQuad(xslid + wslid + 10, yslid, 30, hslid, 0x0000004F);

    f32 Color4 = HilightColor + 20;
    f32 butwhw = winw * 0.85, butwhh = 20;
    f32 butwhx = winx + winw / 2 - butwhw / 2 - 10 / 2; f32 butwhy = ystep;

    ystep += butwhh + wspacing;

    f32 butw = butwhw * 0.075;
    f32 butx = butwhx;

    f32 ibutw = butwhw * (1 - 0.075);
    f32 ibutx = butwhx + butw + 10;

    FillQuad(butx, butwhy, butw, butwhh, Color4);
    FillQuad(ibutx, butwhy, ibutw, butwhh, Color4);

    if(RectangleContainsPoint({butx, butwhy, butw, butwhh}, Input->Cursor.at))
    {
        Color4 = HilightColor;

        if(Input->Cursor.Hit)
        {
            if(Input->Cursor.HitMask == LEFT_BUTTON_MASK)
                Toggle = Toggle ^ true;
        }
    }

    else if(WasPressed(Input, A))
    {
        Toggle = Toggle ^ true;
    }

    if(Toggle)
        FillQuad(butx + (butw / 2) - (0.50 * butw) / 2,
                 butwhy + (butwhh / 2) - (0.50 * butwhh) / 2,
                 0.50 * butw, 0.50 * butwhh, 0x00FFFFFF);

    f32 Spacing = 5;
    f32 bwhole = (winw * 0.85), bh = 50;
    f32 bw = bwhole / 5 - Spacing;
    f32 bx = winx + winw / 2 - bwhole / 2, by = ystep;
    f32 yofffac = 0;

    f32 Color6 = HilightColor;

    for(int i = 0; i < 5; i++)
    {
        if(RectangleContainsPoint({bx + i * bw + Spacing * i, by, bw, bh},
                                  Input->Cursor.at))
        {
            yofffac = -10;
            Color6 = 0x00FFFFFF;
        }
        else
        {
            yofffac = 0;
            Color6 = HilightColor;
        }
        UI_FillQuad(&UI_Ctx, bx + i * bw + (Spacing) * i,
                 by + yofffac,
                 bw, bh, Color6);
    }


    RenderCommit();

    FillText(Buffer, xslid + wslid + 10 + 2, yslid, 15, 0xFFFFFFFF);

    const char* LiveString = "Lives: %i";

    // NOTE(rajat): It will be a good idea to replace this text renderer pointing
    // thing with an actual global backend renderer
    sprintf(Buffer, LiveString, CurrentState->Lives);

    FillText(Buffer, 10.0f, 10.0f,
             FontSize, 0xFFFFFFFF, 0x007A7AFF);

    const char* FpsString = "FPS: %u";
    if((1000/(State->dt * 1.0e2f)) >= 55.0f)
        sprintf(Buffer, FpsString, 60);
    else
    {
        sprintf(Buffer, FpsString, (u32)(1000/(State->dt * 1.0e2f)));
    }
    // TODO(rajat): Might not render stuff like this
    FillText(Buffer, 700, 570,
               32.0f, 0xFFFFFFFF);

    if(NumActieTiles == 0)
    {
        FillText("You Win!\n Press Terminate to close", 800 * 0.42, 800 * 0.42, 32, 0xFFFFFFFF);
        CurrentState->IsPaused = true;
        CurrentState->Fired = false;
    }
    else
    {
        if(CurrentState->IsPaused)
        {
            FillText("Paused!", 0.35 * State->ContextAttribs.Width, .35 * State->ContextAttribs.Height, 64.0f, 0xFFFFFFFF);
        }
    }

    UI_FillQuad(&UI_Ctx, 0, 0, 400, 400, 0xFFFAAFFF);
    UI_FlushCommands(&UI_Ctx);

    // UIInit();

    // // User Inputs to the UI system
    // // Mouse Input and keyboard input
    // // Sets the current active ui element and hot ui

    ui_io Inputs;
    Inputs.Pointer.x = Input->Cursor.X;
    Inputs.Pointer.y = Input->Cursor.Y;
    Inputs.Hit = Input->Cursor.Hit;

    UIBegin(&Inputs, 800, 600);
    UIBeginWindow("Hello UI", 200, 200);

    UIButton("hello", 100, 100, 100, 50);
//    // if(UIButton("Pressmeplease!"))
//    // {
//    //     UIBeginWindow("Hello Window 2");
//    //     UIEnd();
//    // }
//
    UIEndWindow();
    UIEnd();

    static f32 PrevHeight = State->ContextAttribs.Height;

    // FillText("Hello, my name is Rajat and I am going to battle you the next morning we will meet, so are you ready for the epic battle that the world has never seen!", 0.5, 0.5 * State->ContextAttribs.Height, 44 * State->ContextAttribs.Height / 600.0, 0x00FF00FF, 0.3, 0.29);

    PrevHeight = State->ContextAttribs.Height;
}
