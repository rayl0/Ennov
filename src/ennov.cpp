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
    rect Ball;
    rect Paddle;
    u8 Level[32];
    s32 Lives;
    b32 IsPaused;
};

static ui_render_ctx UI_Ctx;
static u32 rctx;

struct breakout_save_data
{
    u32 NumLives;
    b32 IsPaused;
    u8 LevelData[32];
};

#define PaddleVelocity 25

#define WasPressed(Input, x) Input->Button.x.EndedDown

void GameInit(game_state* State, breakout_game_state* CurrentState, b32 LoadFromSaveData)
{
        CurrentState->Paddle = {0.0f, 550.0f, 100.0f, 50.0f};
        CurrentState->Ball = {CurrentState->Paddle.Dimensions.x / 2.0f, 530.0f, 20.0f, 20.0f};
        CurrentState->Direction = {1.0f, 2.0f};
        CurrentState->Fired = false;
        CurrentState->Lives = 3;
        CurrentState->IsPaused = false;

        // TODO(Rajat): Move OpenGL code to platform layer and introduce command buffer
        if(LoadFromSaveData) {

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
        CurrentState->TileBitmap = LoadPixelsFrom("assets/container.png", &State->AssestStorage);

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

    // TODO(rajat): Replace constants with viewport dimensions for different display sizes.

    UI_FillQuad(&UI_Ctx, 0, 0, 800, 600, 0xFF);

    UI_FillQuadRounded(&UI_Ctx, 800 / 2 - 300 / 2, 600 / 2 - 500 / 2, 300, 500, 0x007A7AFF, 0.2 * 100);
    UI_FillQuadRounded(&UI_Ctx, 800 / 2 - 500 / 2, 600 / 2 - 500 / 2, 500, 100, 0xFF761FFF, 0.2 * 100);
    FillText("BreakOUT", 800 / 2 - 500 / 2, 600 / 2 - 500 / 2 + 100 / 2 - (94 * normf(72, 0, 58)) / 2, 72, 0xFFFFFFFF, 0.56, 0.06, 0x00FFFFFF, 0.0, 0.1, true, 500);

    FillText("Play", 800 / 2 - 300 / 2, 600 / 2 - 500 / 2 + 100 + 10, 68, 0xFF761FFF, true, 300);

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

    if(CurrentState->Lives == 0)
    {
        fprintf(stderr, "You Lose!\n");
        FillText("You Lose!", 400, 300, 72, 0xFFFFFFFF);

        CurrentState->IsPaused = true;
        CurrentState->Fired = false;

        if(WasPressed(Input, Start))
        {
            GameInit(State, CurrentState, false);

            CurrentState->IsPaused = false;
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

    FillTexQuad(00, 00, 800, 600, Color2, &CurrentState->Textures[0]);

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

    FillTexQuad(Paddle->Pos.x, Paddle->Pos.y,
                Paddle->Dimensions.x, Paddle->Dimensions.y,
                Color2, &CurrentState->Textures[2]);
    FillTexQuad(Ball->Pos.x, Ball->Pos.y,
                Ball->Dimensions.x, Ball->Dimensions.y,
                Color2, &CurrentState->Textures[0]);

    FillQuad(0, 0, 800, 600, Color);

    char Buffer[50];
    const char* LiveString = "Lives: %i";

    sprintf(Buffer, LiveString, CurrentState->Lives);

    FillText(Buffer, 10.0f, 10.0f,
             50, 0xFFFFFFFF, 0x007A7AFF);

    if(NumActieTiles == 0)
    {
        FillText("You Win!",
                 800 * 0.3, 600 * 0.3, 32, 0xFFFFFFFF);

        CurrentState->IsPaused = true;
        CurrentState->Fired = false;

        if(WasPressed(Input, Start))
        {
            GameInit(State, CurrentState, false);

            CurrentState->IsPaused = false;
            CurrentState->Fired = false;
        }
    }
    else
    {
        if(CurrentState->IsPaused && CurrentState->Lives != 0)
        {
            FillText("Paused!", 0.35 * State->ContextAttribs.Width, .35 * State->ContextAttribs.Height, 64.0f, 0xFFFFFFFF);
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
            SaveData->IsPaused = CurrentState->IsPaused;

            for(int i = 0; i < 32; i++)
                SaveData->LevelData[i] = CurrentState->Level[i];

            State->SaveDataSize = sizeof(breakout_save_data);
        }

        Ball->Pos = {Paddle->Pos.x + Paddle->Dimensions.x / 2, 550.0f - Ball->Dimensions.y};
        CurrentState->Fired = false;
    }

    RenderCommit();

    FlushRenderCommands(rctx);
    UI_FlushCommands(&UI_Ctx);
    FontFlushRenderCommands();
}
