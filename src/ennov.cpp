// ------------ TODO -------------
// -- Fix ennov_gl clip calls

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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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

enum game_mode
{
    PAUSED = 0xF06FFFFF,
    MENU = 0x005FFFFF,
    PLAY = 0x007A7AFF
};

// TODO(Rajat): Implement Rand number generator
// TODO(Rajat): Implement Matrix and transformation methods

struct breakout_game_state
{
    loaded_bitmap *BackgroundBitmap;
    loaded_bitmap *PaddleBitmap;
    loaded_bitmap *BallBitmap;
    loaded_bitmap *TileBitmap;
    texture Textures[4];
    b32 Fired;
    vec2 Direction;
    struct {
        rect BallRect;
        f32 Radius;
    }Ball;
    rect Paddle;
    u8 Level[16 * 9];
    s32 Lives;
    u32 Mode;
};

static ui_render_ctx UI_Ctx;
static u32 rctx;

struct breakout_save_data
{
    u32 NumLives;
    u32 Mode;
    u8 LevelData[16 * 9];
};

#define PaddleVelocity 25

#define WasPressed(Input, x) Input->Button.x.EndedDown

static f32 AbsSrcWidth;
static f32 AbsSrcHeight;

void GameInit(game_state* State, breakout_game_state* CurrentState, b32 LoadFromSaveData)
{
    f32 PaddleX = 0.0f;
    f32 PaddleWidth = mapf(250.0f, 0.0f, 1200, 0.0f, AbsSrcWidth);
    f32 PaddleHeight = mapf(35.0f, 0.0f, 675, 0.0f, AbsSrcHeight);
    f32 PaddleY = mapf(675 - PaddleHeight, 0.0f, 675.0f, 0.0f, AbsSrcHeight);

    f32 BallWidth = mapf(25.0f, 0.0f, 1200, 0.0f, AbsSrcWidth);
    f32 BallHeight = mapf(25.0f, 0.0f, 675, 0.0f, AbsSrcHeight);
    f32 BallX = PaddleX + PaddleWidth / 2.0f;
    f32 BallY = PaddleY - BallHeight;

    CurrentState->Paddle = {PaddleX, PaddleY, PaddleWidth, PaddleHeight};
    CurrentState->Ball.BallRect = {BallX, BallY, BallWidth, BallHeight};
    CurrentState->Ball.Radius = BallWidth / 2;
    CurrentState->Direction = {1.0f, 2.0f};
    CurrentState->Fired = false;
    CurrentState->Lives = 3;
    CurrentState->Mode = MENU;

    // TODO(Rajat): Move OpenGL code to platform layer and introduce command buffer
    if(LoadFromSaveData) {

        breakout_save_data* SaveData = (breakout_save_data*)State->GameSaveData;

        CurrentState->Lives = SaveData->NumLives;
        CurrentState->Mode = SaveData->Mode;

        if(CurrentState->Mode != MENU)
            CurrentState->Mode = MENU;

        for(int i = 0; i < 16 * 9; ++i) {
            CurrentState->Level[i] =  SaveData->LevelData[i];
        }

    }
    else
    {

        u32 TileMap[16 * 9] = {
            3, 2, 3, 1, 5, 1, 2, 3, 3, 5, 3, 4, 5, 2, 5, 1,
            5, 3, 5, 3, 3, 5, 4, 1, 4, 5, 3, 5, 5, 2, 0, 3,
            4, 1, 3, 5, 2, 1, 2, 5, 1, 2, 3, 4, 5, 5, 3, 3,
            0, 0, 3, 2, 4, 3, 0, 0, 4, 0, 0, 0, 1, 2, 0, 2,
            0, 1, 3, 4, 5, 2, 0, 1, 2, 3, 4, 5, 0, 1, 3, 0
        };

        for(int i = 0; i < 16 * 9; ++i) {
            CurrentState->Level[i] =  TileMap[i];
        }

    }
}

void GameUpdateAndRender(game_memory* Memory, game_state *State, game_input *Input, u32 *ConfigBits)
{
    // NOTE(Rajat): Never do assertions within a loop increment
    // Assert(Count < 1);

    #if ENNOV_PLATFORM_ANDROID
    #endif

    breakout_game_state* CurrentState = (breakout_game_state*)Memory->PermanentStorage;

    if(!Memory->IsInitialized) {

        InitializeAreana(&State->GameStorage, (char*)Memory->PermanentStorage + sizeof(CurrentState),
                         Memory->PermanentStorageSize - sizeof(CurrentState));

        InitializeAreana(&State->ScratchStorage, Memory->TransientStorage, Memory->TransientStorageSize);
        InitializeAreana(&State->AssestStorage, Memory->AssetMemory, Memory->AssetMemorySize);
        CurrentState->BackgroundBitmap = LoadPixelsFrom("assets/background.jpg", &State->AssestStorage);
        CurrentState->BallBitmap = LoadPixelsFrom("assets/background.jpg", &State->AssestStorage);
        CurrentState->PaddleBitmap = LoadPixelsFrom("assets/paddle.png", &State->AssestStorage);
        CurrentState->TileBitmap = LoadPixelsFrom("assets/block-textures.png", &State->AssestStorage);

        AbsSrcWidth = State->ContextAttribs.Width;
        AbsSrcHeight = State->ContextAttribs.Height;

        GameInit(State, CurrentState, State->SaveDataSize);

        CurrentState->Textures[0] = CreateTexture(CurrentState->BackgroundBitmap);
        CurrentState->Textures[1] = CreateTexture(CurrentState->TileBitmap);
        CurrentState->Textures[2] = CreateTexture(CurrentState->PaddleBitmap);

        // TODO(rajat): Load precompiled shaders if possible
        game_file* VertexShaderFile = (game_file*)PlatformLoadFile("shaders/gquad.vert",  PushStruct_, &State->AssestStorage);
        game_file* FragmentShaderFile = (game_file*)PlatformLoadFile("shaders/gquad.frag", PushStruct_, &State->AssestStorage);

        game_file* FontVertexShader = (game_file*)PlatformLoadFile("shaders/font.vert", PushStruct_,
                                                                   &State->AssestStorage);
        game_file* FontFragmentShader = (game_file*)PlatformLoadFile("shaders/font.frag", PushStruct_,
                                                                     &State->AssestStorage);
        game_file* UI_VertexShader = (game_file*)PlatformLoadFile("shaders/ui.vert", PushStruct_, &State->AssestStorage);
        game_file* UI_FragmentShader = (game_file*)PlatformLoadFile("shaders/ui.frag", PushStruct_, &State->AssestStorage);

        CreateRenderContext(&rctx, (const char*)VertexShaderFile->Data, (const char*)FragmentShaderFile->Data);
        BindRenderContext(rctx);

        UI_CreateContext(&UI_Ctx, (const char*)UI_VertexShader->Data, (const char*)UI_FragmentShader->Data);

        GameState = State;
        LoadFont("assets/fonts/default.fnt");
        CreateFontRenderObjects((const char*)FontVertexShader->Data, (const char*)FontFragmentShader->Data);

        Memory->IsInitialized = true;
    }

    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0, 0, 0, 1);

    AbsSrcWidth = State->ContextAttribs.Width;
    AbsSrcHeight = State->ContextAttribs.Height;

    // TODO(rajat): Replace constants with viewport dimensions for different display sizes.

    // UI_FillQuad(&UI_Ctx, 0, 0, State->ContextAttribs.Width, State->ContextAttribs.Height, 0xFF);

    static u32 HotButton = 0;

    vec3 Button1Color = colors.SEA_GREEN;
    vec3 Button2Color = colors.SEA_GREEN;

    if(HotButton == 1)
        Button1Color = colors.ORANGE;
    else if(HotButton == 2)
        Button2Color = colors.ORANGE;

    HotButton = 0;

    static s32 InteractingButton = 0;

    if(CurrentState->Mode == MENU)
    {
        u32 UI_WindowWidth = AbsSrcWidth * 0.8;
        u32 UI_WindowHeight = AbsSrcHeight * 0.9;
        u32 UI_WindowX = AbsSrcWidth / 2 - UI_WindowWidth / 2;
        u32 UI_WindowY = AbsSrcHeight / 2 - UI_WindowHeight / 2;

        UI_FillQuadRounded(&UI_Ctx, UI_WindowX,
                           UI_WindowY,
                           UI_WindowWidth, UI_WindowHeight, colors.STEELBLUE, 100, 0.2 * 100);

        u32 UI_TitleBarHeight = UI_WindowHeight * 0.25;
        UI_FillQuadRounded(&UI_Ctx, UI_WindowX, UI_WindowY, UI_WindowWidth, UI_TitleBarHeight, colors.SEA_GREEN, 255, 0.2 * 100);

        FillText("BreakOUT", UI_WindowX, UI_WindowY + UI_TitleBarHeight / 2 - (94 * normf(72, 0, 58)) / 2,
                 72, 0xFFFFFFFF, 0.56, 0.06, 0x00FFFFFF, 0.0, 0.1, true, UI_WindowWidth);

        u32 UI_ButtonWidth = mapf(250, 0, 1100, 0, AbsSrcWidth);
        u32 UI_ButtonHeight = mapf(70, 0, 618, 0, AbsSrcHeight);

        u32 UI_ButtonX = UI_WindowX + UI_WindowWidth / 2 - UI_ButtonWidth / 2;
        u32 UI_ButtonY = UI_WindowY + UI_TitleBarHeight + 20;

        UI_FillQuadRounded(&UI_Ctx, UI_ButtonX, UI_ButtonY,
                           UI_ButtonWidth, UI_ButtonHeight, Button1Color, 255, 0.2 * 100);

        FillText("Play", UI_ButtonX, UI_ButtonY, 68, 0xFFFFFFFF, true, UI_ButtonWidth);

        if(RectangleContainsPoint({UI_ButtonX, UI_ButtonY, UI_ButtonWidth, UI_ButtonHeight}, Input->Cursor.at))
        {
            if(HotButton == 0)
            {
                HotButton = 1;
            }

            if(Input->Cursor.Hit)
            {
                if(InteractingButton == 0 && HotButton == 1)
                    InteractingButton = 1;
            }
        }

        u32 UI_ButtonWidth2 = mapf(250, 0, 1100, 0, AbsSrcWidth);
        u32 UI_ButtonHeight2 = mapf(70, 0, 618, 0, AbsSrcHeight);

        u32 UI_ButtonX2 = UI_WindowX + UI_WindowWidth / 2 - UI_ButtonWidth2 / 2;
        u32 UI_ButtonY2 = UI_ButtonY + UI_ButtonHeight + 20;

        UI_FillQuadRounded(&UI_Ctx, UI_ButtonX2, UI_ButtonY2,
                           UI_ButtonWidth2, UI_ButtonHeight2, Button2Color, 255, 0.2 * 100);

        FillText("Exit", UI_ButtonX2, UI_ButtonY2, 68, 0xFFFFFFFF, true, UI_ButtonWidth2);

        if(RectangleContainsPoint({UI_ButtonX2, UI_ButtonY2, UI_ButtonWidth2, UI_ButtonHeight2}, Input->Cursor.at))
        {
            if(HotButton == 0)
            {
                HotButton = 2;
            }

            if(Input->Cursor.Hit)
            {
                if(InteractingButton == 0 && HotButton == 2)
                    InteractingButton = 2;
            }
        }
    }

    if(Input->Cursor.Hit == 0)
    {
        InteractingButton = 0;
    }
    else
    {
        if(InteractingButton == 0)
            InteractingButton = -1;
    }

    if(InteractingButton == 1)
        CurrentState->Mode = PLAY;

    if(InteractingButton == 2)
        State->SignalTerminate = true;

    // InteractingButton = -1;

    UI_UpdateViewProj(&UI_Ctx, State->ContextAttribs.Width, State->ContextAttribs.Height);
    RenderContextUpdateViewProj(rctx, State->ContextAttribs.Width, State->ContextAttribs.Height);
    FontUpdateViewProj(State->ContextAttribs.Width, State->ContextAttribs.Height);

    if(!(CurrentState->Mode == MENU))
    {

    rect* Ball = &CurrentState->Ball.BallRect;
    rect* Paddle = &CurrentState->Paddle;
    vec2* Direction = &CurrentState->Direction;

    if(WasPressed(Input, S)) {
        if(CurrentState->Mode == PAUSED)
        {
            fprintf(stderr, "Resuming the game\n");
            CurrentState->Mode = PLAY;
        }
        else {
            fprintf(stderr, "Pausing the game\n");
            CurrentState->Mode = PAUSED;
        }
    }
    if(CurrentState->Mode == PLAY) {
        if(WasPressed(Input, Start)) {
            CurrentState->Fired = true;
        }
        if(CurrentState->Fired) {
            f32 dv = normf(State->dt, 0, 0.16);
            dv = SMOOTHSTEP(dv);

            Ball->Pos.y -= 4 * Direction->y * dv;
            Ball->Pos.x += 4 * Direction->x * dv;

            Ball->Pos.x = clampf(Ball->Pos.x, 0, AbsSrcWidth - Ball->Dimensions.x);
            Ball->Pos.y = clampf(Ball->Pos.y, 0, AbsSrcHeight + Ball->Dimensions.y);
        }

        if(Ball->Pos.x >= AbsSrcWidth - Ball->Dimensions.x) Direction->x = -(Direction->x);
        if(Ball->Pos.y <= 0.0f) Direction->y = -(Direction->y);
        if(Ball->Pos.x <= 0.0f) Direction->x = -(Direction->x);
        if(Ball->Pos.y >= AbsSrcHeight) {
            CurrentState->Fired = false;
            Ball->Pos = {Paddle->Pos.x + Paddle->Dimensions.x / 2, Paddle->Pos.y - Ball->Dimensions.y};
            *Direction = { 1.0f, 2.0f};
            --(CurrentState->Lives);
            fprintf(stderr, "One Life deducted!\n");
        }
        if(RectangleColloide(*Paddle, *Ball))
        {
            if(CurrentState->Fired)
                Ball->Pos = {Ball->Pos.x, Paddle->Pos.y - Ball->Dimensions.y};

            Direction->x = (Direction->x);
            Direction->y = -(Direction->y);
        }
        if(!CurrentState->Fired) {
            Ball->Pos.x = Paddle->Pos.x + Paddle->Dimensions.x / 2.0f;
            Ball->Pos.y = Paddle->Pos.y - Ball->Dimensions.y;
        }
        if(Input->Button.MoveRight.EndedDown)
        {
            f32 dv = normf(State->dt, 0, 0.16);
            dv = SMOOTHSTEP(dv);
            Paddle->Pos.x += dv * PaddleVelocity;
            Paddle->Pos.x = clampf(Paddle->Pos.x, 0, AbsSrcWidth - Paddle->Dimensions.x);
        }
        if(Input->Button.MoveLeft.EndedDown)
        {
            f32 dv = normf(State->dt, 0, 0.16);
            dv = SMOOTHSTEP(dv);
            Paddle->Pos.x -= dv * PaddleVelocity;
            Paddle->Pos.x = clampf(Paddle->Pos.x, 0, AbsSrcWidth - Paddle->Dimensions.x);
        }
    }

    if(CurrentState->Lives == 0)
    {
        fprintf(stderr, "You Lose!\n");
        FillText("You Lose!", 0, 300, 72, 0xFFFFFFFF, true, (u32)AbsSrcWidth);

        CurrentState->Mode = PAUSED;
        CurrentState->Fired = false;

        if(WasPressed(Input, Start))
        {
            GameInit(State, CurrentState, false);
        }
    }

    // TODO(rajat): Introduce new buttons in game_input struct for this
    if(WasPressed(Input, Select))
    {
        *ConfigBits = *ConfigBits ^ PlatformFullScreenToggle_BIT;
    }

    u32 NumActieTiles = 0;

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

    FillTexQuad(00, 00, AbsSrcWidth, AbsSrcHeight, Color2, &CurrentState->Textures[0]);

    u8* Level = CurrentState->Level;

    for(int i = 0; i < 9; ++i)
    {
        for(int j = 0; j < 16; ++j) {
            vec2 Pos = {j * (AbsSrcWidth / 16), i * (AbsSrcHeight / 16)};
            vec2 Dim = {AbsSrcWidth / 16, AbsSrcHeight / 16};
            u32 Color = {};

            u32 TileIndex = i * 16 + j;

            if(Level[TileIndex] == 0)
                continue;
            else if(Level[TileIndex] == 1)
                Color = 0xF06FFFFF;
            else if(Level[TileIndex] == 2)
                Color = 0xFF761FFF;
            else if(Level[TileIndex] == 3)
                Color = 0x005FFFFF;
            else if(Level[TileIndex] == 4)
                Color = 0xF06FFFFF;
            else if(Level[TileIndex] == 5)
                Color = 0x00FFFFFF;

            u8 r, g, b, a;
            DecodeRGBA(Color, &r, &g, &b, &a);

            a = 255 * n;
            Color = EncodeRGBA(r, g, b, a);

            f32 s0 = 0, t0 = 0, s1 = 0.5f, t1 = 0.5f;

            // if(Level[TileIndex] == 1)
            // {
                // s0 = 0.5f;
                // t0 = 0.5f;
                // s1 = 1.0f;
                // t1 = 1.0f;
            // }

            FillTexQuadClipped(Pos.x, Pos.y, Dim.x, Dim.y, Color, &CurrentState->Textures[1], s0, t0, s1, t1);

            if(RectangleColloide({Pos, Dim}, *Ball))
            {
                fprintf(stderr, "Tile Colloide %i\n", Level[TileIndex]);

                vec2 HalfExtents = Dim;
                vec2 Centre = {Pos.x + HalfExtents.x, Pos.x + HalfExtents.x};

                vec2 BallCentre = {Ball->Pos.x + Ball->Dimensions.x / 2, Ball->Pos.y + Ball->Dimensions.y / 2};

                vec2 DBar = {BallCentre.x - Centre.x, BallCentre.y - Centre.y};
                vec2 ClosestPoint = {clampf(DBar.x, 0, HalfExtents.x), clampf(DBar.y, 0, HalfExtents.y)};

                vec2 SBar = {ClosestPoint.x - BallCentre.x, ClosestPoint.y - BallCentre.y};

                if(length(SBar) <= CurrentState->Ball.Radius)
                    fprintf(stderr, "Hello World!");

                Level[TileIndex] = 0;

                Direction->x = Direction->x;
                Direction->y = -Direction->y;
            }

            if(Level[TileIndex] != 0)
            {
                NumActieTiles++;
            }
        }
    }

    FillTexQuad(Paddle->Pos.x, Paddle->Pos.y,
                Paddle->Dimensions.x, Paddle->Dimensions.y,
                Color2, &CurrentState->Textures[2]);
    FillTexQuad(Ball->Pos.x, Ball->Pos.y,
                Ball->Dimensions.x, Ball->Dimensions.y,
                Color2, &CurrentState->Textures[0]);

    FillQuad(0, 0, AbsSrcWidth, AbsSrcHeight, Color);

    char Buffer[50];
    const char* LiveString = "Lives: %i";

    sprintf(Buffer, LiveString, CurrentState->Lives);

    FillText(Buffer, 10.0f, 10.0f,
             50, Color2, false, 300);

    if(NumActieTiles == 0)
    {
        FillText("You Win!",
                 0, 300, 64, 0xFFFFFFFF, true, (u32)AbsSrcWidth);

        CurrentState->Mode = PAUSED;
        CurrentState->Fired = false;

        if(WasPressed(Input, Start))
        {
            GameInit(State, CurrentState, false);

            CurrentState->Fired = false;
        }
    }
    else
    {
        if(CurrentState->Mode == PAUSED && CurrentState->Lives != 0)
        {
            FillText("Paused!", 0, 300, 64.0f, 0xFFFFFFFF, true, (u32)AbsSrcWidth);
        }
    }


    if(Input->Button.Terminate.EndedDown)
    {
        if(NumActieTiles == 0 || CurrentState->Lives == 0)
            State->GameSaveData = NULL;
        else
        {
            breakout_save_data* SaveData = (breakout_save_data*)State->GameSaveData;

            SaveData->NumLives = CurrentState->Lives;
            SaveData->Mode = CurrentState->Mode;

            for(int i = 0; i < 16 * 9; i++)
                SaveData->LevelData[i] = CurrentState->Level[i];

            State->SaveDataSize = sizeof(breakout_save_data);
        }

        Ball->Pos = {Paddle->Pos.x + Paddle->Dimensions.x / 2, Paddle->Pos.y - Ball->Dimensions.y};
        CurrentState->Fired = false;
    }

    }

    RenderCommit();

    FlushRenderCommands(rctx);
    UI_FlushCommands(&UI_Ctx);
    FontFlushRenderCommands();
}
