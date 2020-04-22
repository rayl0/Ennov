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
        CreateRenderContext((const char*)VertexShaderFile->Data, (const char*)FragmentShaderFile->Data);
        GameState = State;
        LoadFont("assets/fonts/default.fnt");
        CreateFontRenderObjects((const char*)FontVertexShader->Data, (const char*)FontFragmentShader->Data);

        Memory->IsInitialized = true;
    }

    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(1, 0, 1, 1);

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
            f32 dv = normf(0, 0.16, State->dt);
            dv = SMOOTHSTEP(dv);

            Ball->Pos.y -= 3 * Direction->y * dv;
            Ball->Pos.x += 3 * Direction->x * dv;

            Ball->Pos.x = clampf(0, 800 - Ball->Dimensions.x, Ball->Pos.x);
            Ball->Pos.y = clampf(0, 800 - Ball->Dimensions.y, Ball->Pos.y);
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
            f32 dv = normf(0, 0.16, State->dt);
            dv = SMOOTHSTEP(dv);
            Paddle->Pos.x += dv * PaddleVelocity;
            Paddle->Pos.x = clampf(0, 800 - Paddle->Dimensions.x, Paddle->Pos.x);
        }
        if(Input->Button.MoveLeft.EndedDown)
        {
            f32 dv = normf(0, 0.16, State->dt);
            dv = SMOOTHSTEP(dv);
            Paddle->Pos.x -= dv * PaddleVelocity;
            Paddle->Pos.x = clampf(0, 800 - Paddle->Dimensions.x, Paddle->Pos.x);
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

    // TODO(rajat): Add src clipping to the renderer
    FillTexQuad(00, 00, 800, 600, &CurrentState->Textures[0]);

    u8* Level = CurrentState->Level;

    static f32 t = 0;

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

            f32 n = normf(0, 2, t);
            n = SMOOTHSTEP(n);

            // NOTE(rajat): Always remember the range of u8 0...255, not 256
            u8 S = (u8)lerpf(0, 255, n);

            printf("Alpha: %u\n", S);

            u8 r, g, b, a;
            DecodeRGBA(Color, &r, &g, &b, &a);

            a = S;
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

    t += (State->dt / 10);
    t = clampf(0, 2, t);

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

    FillTexQuad(Paddle->data, &CurrentState->Textures[2]);
    FillTexQuad(Ball->data, &CurrentState->Textures[0]);

    static u32 WasHit = 0;

    if(!WasHit)
    if(RectangleContainsPoint({200, 200, 100, 50}, {Input->Cursor.X, Input->Cursor.Y}))
        FillQuad(200, 200, 100, 50, 0x000000FF);
    else
        FillQuad(200, 200, 100, 50, 0xFFFFFFFF);
    else
        FillQuad(200, 200, 100, 50, 0x000000FF);

    RenderCommit();

    if(!WasHit)
    if(RectangleContainsPoint({200, 200, 100, 50}, {Input->Cursor.X, Input->Cursor.Y}))
        FillText("Button", 200, 200, 32, 0xFFFFFFFF);
    else
        FillText("Button", 200, 200, 32, 0x0000000FF);
    else
        FillText("Button", 200, 200, 32, 0x0000FFFF);


    if(RectangleContainsPoint({200, 200, 100, 50}, {Input->Cursor.X, Input->Cursor.Y}) && Input->Cursor.Hit)
        WasHit = WasHit ^ 1;


    const char* LiveString = "Lives: %i";
    char Buffer[50];

    // NOTE(rajat): It will be a good idea to replace this text renderer pointing
    // thing with an actual global backend renderer
    sprintf(Buffer, LiveString, CurrentState->Lives);

    FillText(Buffer, 10.0f, 10.0f,
             32, 0xFFFFFFFF, 0x007A7AFF);

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
        FillText("You Win!\n Press Terminate to close", lerpf(0.0f, 800.0f, 0.42), lerpf(0.0f, 600.0f, 0.42), 32, 0xFFFFFFFF);
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
