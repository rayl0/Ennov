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
    u32 Level[32];
    s32 Lives;
    b32 IsPaused;
};

#define Velocity 15
#define PaddleVelocity 150

#define WasPressed(Input, x) Input->Button.x.EndedDown

void GameUpdateAndRender(game_memory* Memory, game_state *State, game_input *Input, u32 *ConfigBits)
{
    // NOTE(Rajat): Never do assertions within a loop increment
    // Assert(Count < 1);

    //  OpenGLInitContext({State->ContextAttribs.Width, State->ContextAttribs.Height});
    // DrawRectangle(draw_attribs);

    #if ENNOV_PLATFORM_ANDROID
    #endif

    breakout_game_state* CurrentState = (breakout_game_state*)Memory->PermanentStorage;
    if(!Memory->IsInitialized) {
        InitializeAreana(&State->GameStorage, (char*)Memory->PermanentStorage + sizeof(CurrentState), Memory->PermanentStorageSize - sizeof(CurrentState));
        InitializeAreana(&State->ScratchStorage, Memory->TransientStorage, Memory->TransientStorageSize);
        InitializeAreana(&State->AssestStorage, Memory->AssetMemory, Memory->AssetMemorySize);

        // TODO(Rajat): Move OpenGL code to platform layer and introduce command buffer
        if(!CurrentState->HaveLoadState) {
            CurrentState->Paddle = {0.0f, 550.0f, 100.0f, 50.0f};
            CurrentState->Ball = {CurrentState->Paddle.Dimensions.x / 2.0f, 530.0f, 20.0f, 20.0f};
            CurrentState->Direction = {1.0f, 2.0f};

            CurrentState->Fired = false;
            CurrentState->Lives = 3;

            CurrentState->IsInitialized = true;
            CurrentState->IsPaused = false;

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
        CreateTextFonts((u8*)CurrentState->TextFontFile->Data, 32);

        GameState = State;
        LoadFont("assets/fonts/default.fnt");
        CreateFontRenderObjects((const char*)FontVertexShader->Data, (const char*)FontFragmentShader->Data);

        Memory->IsInitialized = true;
    }

    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0, 0, 0, 0);

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
            Ball->Pos.y -= Velocity * Direction->y * State->dt;
            Ball->Pos.x += Velocity * Direction->x * State->dt;
        }
        if(Ball->Pos.x > 790.0f) Direction->x = -(Direction->x);
        if(Ball->Pos.y < 0.0f) Direction->y = -(Direction->y);
        if(Ball->Pos.x < 0.0f) Direction->x = -(Direction->x);
        if(Ball->Pos.y > 600.0f) {
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
            Paddle->Pos.x += State->dt * PaddleVelocity;
            if(Paddle->Pos.x >= 700.0f)
                Paddle->Pos.x = 700.0f;
        }
        if(Input->Button.MoveLeft.EndedDown)
        {
            Paddle->Pos.x -= State->dt * PaddleVelocity;
            if(Paddle->Pos.x <= 0.0f)
                Paddle->Pos.x = 0.0f;
        }
    }

    // TODO(rajat): Introduce new buttons in game_input struct for this
    if(WasPressed(Input, Select))
    {
        if(*ConfigBits == PlatformFullScreenToggle_BIT)
        {
            *ConfigBits = 0;
        }
        else
        {
            *ConfigBits = PlatformFullScreenToggle_BIT;
        }
    }

    u32 NumActieTiles = 0;

    // TODO(rajat): Add src clipping to the renderer
    FillTexQuad(0, 0, 800, 600, &CurrentState->Textures[0]);

    u32* Level = CurrentState->Level;

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

    if(Input->Button.Terminate.EndedDown)
    {
        if(NumActieTiles == 0)
            CurrentState->HaveLoadState = false;
        else
            CurrentState->HaveLoadState = true;
        Ball->Pos = {Paddle->Pos.x + Paddle->Dimensions.x / 2, 550.0f - Ball->Dimensions.y};
        CurrentState->Fired = false;
    }

    FillTexQuad(Paddle->data, &CurrentState->Textures[2]);
    FillTexQuad(Ball->data, &CurrentState->Textures[0]);

    const char* LiveString = "Lives: %i";
    char Buffer[50];

    // NOTE(rajat): It will be a good idea to replace this text renderer pointing
    // thing with an actual global backend renderer
    sprintf(Buffer, LiveString, CurrentState->Lives);

    DrawString(Buffer, 0.0f, 20.0f,
               1.0f, 0xFFFFFFFF);

    const char* FpsString = "FPS: %u";
    if((1000/(State->dt * 1.0e2f)) >= 55.0f)
        sprintf(Buffer, FpsString, 60);
    else
    {
        sprintf(Buffer, FpsString, (u32)(1000/(State->dt * 1.0e2f)));
    }
    // TODO(rajat): Might not render stuff like this
    DrawString(Buffer, 700, 570,
               1.0f, 0xFFFFFFFF);

    if(NumActieTiles == 0)
    {
        DrawString("You Win!", 300.0f, 300.0f, 1.0f, 0xFFFFFFFF);
        DrawString("Press Terminate to close", 190.0f, 348.0f, 0.5f, 0xFFFFFFFF);
        CurrentState->IsPaused = true;
        CurrentState->Fired = false;
    }
    else
    {
        if(CurrentState->IsPaused)
        {
            DrawString("Paused!", 150, 150, 2.0f, 0xFFFFFFFF);
        }
    }

    RenderCommit();

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
//
//    // if(UIButton("Pressmeplease!"))
//    // {
//    //     UIBeginWindow("Hello Window 2");
//    //     UIEnd();
//    // }
//
    UIEndWindow();
    UIEnd();
//
//    FlushRenderer(Batch);

    // FillQuad(100, 100, 100, 100, 0x0000FFFF);
    // FillQuad(100, 200, 100, 100, 0xF06FFFFF);
    // FillQuad(100, 300, 100, 100, 0x005FFFFF);
    // FillQuad(100, 400, 100, 100, 0x00FFFFFF);
    // FillTexQuad(100, 500, 100, 100, 0x00FFFFFF, &CurrentState->Textures[1]);
    // FillTexQuad(200, 500, 100, 100, 0x00FFFFFF, &CurrentState->Textures[2]);
    // FillTexQuad(300, 500, 100, 100, &CurrentState->Textures[0]);
    // FillTexQuad(500, 500, 100, 100, &CurrentState->Textures[2]);

    // FillTexQuadClipped(600, 500, 100, 100, &CurrentState->Textures[2], {0, 0, 50, 50});


    FillText("Some text with outline!", Ball->Pos.x, Ball->Pos.y, 1.0f, 0xFFF00FFF);
}
