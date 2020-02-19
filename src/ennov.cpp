#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ennov_text.cpp"
#include "ennov_math.h"
#include "ennov_gl.cpp"
#include "ennov.h"

// TODO(rajat): Fix bug, Position of you win text depends upon numactive tiles, >= or ==

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

game_file*(*PlatformLoadFile)(char*, void*(*)(game_areana*, memory_index), game_areana*);

loaded_bitmap*
LoadPixelsFrom(char* FileName, game_areana* Areana)
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
    loaded_bitmap* BackgroundBitmap;
    loaded_bitmap* PaddleBitmap;
    loaded_bitmap* BallBitmap;
    loaded_bitmap* TileBitmap;
    texture Textures[4];
    glm::mat4 Projection; // TODO(Rajat): Replace glm fast!
    renderer_data RendererData;
    character_glyph* Glyphs;
    game_file *TextFontFile;
    text_rendering_data TextData;
    b32 Fired;
    vec2 Direction;
    rect Ball;
    rect Paddle;
    u32 Level[32];
    s32 Lives;
    b32 IsPaused;
};

#define Velocity 20;
#define PaddleVelocity 150;

void GameUpdateAndRender(game_memory* Memory, game_state *State, game_input *Input, u32 *ConfigBits)
{
    // NOTE(Rajat): Never do assertions within a loop increment
    // Assert(Count < 1);

    //  OpenGLInitContext({State->ContextAttribs.Width, State->ContextAttribs.Height});
    // DrawRectangle(draw_attribs);

    breakout_game_state* CurrentState = (breakout_game_state*)Memory->PermanentStorage;
    PlatformLoadFile = State->Interface.PlatformLoadFile;
    if(!Memory->IsInitialized) {
        gladLoadGL();

        InitializeFreeType();

        InitializeAreana(&State->GameStorage, Memory->PermanentStorage + sizeof(CurrentState), Memory->PermanentStorageSize - sizeof(CurrentState));
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

        CurrentState->RendererData = {};

        CurrentState->RendererData.Projection = glm::ortho(0.0f, 800.0f, 600.0f, 00.0f, -1.0f, 1.0f); // TODO(Rajat): Update projection with window size change!
        CurrentState->Projection = glm::ortho(0.0f, 800.0f, 600.0f, 00.0f); // TODO(Rajat): Update projection with window size change!

        InitRenderer(&CurrentState->RendererData, &State->ScratchStorage);
        CurrentState->BackgroundBitmap = LoadPixelsFrom("./background.jpg", &State->AssestStorage);
        CurrentState->BallBitmap = LoadPixelsFrom("./background.jpg", &State->AssestStorage);
        CurrentState->PaddleBitmap = LoadPixelsFrom("./paddle.png", &State->AssestStorage);
        CurrentState->TileBitmap = LoadPixelsFrom("./container.png", &State->AssestStorage);

        // asset_work_queue WorkQueue;

        CurrentState->Textures[0] = CreateTexture(CurrentState->BackgroundBitmap);
        CurrentState->Textures[1] = CreateTexture(CurrentState->TileBitmap);
        CurrentState->Textures[2] = CreateTexture(CurrentState->PaddleBitmap);

        CurrentState->Glyphs = LoadTTF("./s.ttf", 74, &State->AssestStorage);

        CurrentState->TextData = {};
        CurrentState->TextFontFile = (game_file*)PlatformLoadFile("./s.ttf", PushStruct_, &State->AssestStorage);

        Memory->IsInitialized = true;
    }

    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0, 0, 0, 0);

    rect* Ball = &CurrentState->Ball;
    rect* Paddle = &CurrentState->Paddle;
    vec2* Direction = &CurrentState->Direction;
    renderer_data* Batch = &CurrentState->RendererData;

    if(Input->Button.S.EndedDown) {
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
        if(Input->Button.Start.EndedDown) {
            CurrentState->Fired = true;
        }
        if(CurrentState->Fired) {
            Ball->Pos.y -= State->Delta * Direction->y * Velocity;
            Ball->Pos.x += State->Delta * Direction->x * Velocity;
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
            Paddle->Pos.x += State->Delta * PaddleVelocity;
            if(Paddle->Pos.x >= 700.0f)
                Paddle->Pos.x = 700.0f;
        }
        if(Input->Button.MoveLeft.EndedDown)
        {
            Paddle->Pos.x -= State->Delta * PaddleVelocity;
            if(Paddle->Pos.x <= 0.0f)
                Paddle->Pos.x = 0.0f;
        }
    }

    // TODO(rajat): Introduce new buttons in game_input struct for this
    if(Input->Button.Select.EndedDown)
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
    DrawBatchRectangle(Batch, &CurrentState->Textures[0], {1, 1, 1, 1.0f}, NULL, {0, 0}, {800, 600});

    u32* Level = CurrentState->Level;

    for(int i = 0; i < 4; ++i)
    {
        for(int j = 0; j < 8; ++j) {
            vec2 Position = {j * 100.0f, i * 50.0f};
            vec2 Dimensions = {100.0f, 50.0f};
            vec4 Color = {};
            if(Level[i * 8 + j] == 0)
                continue;
            else if(Level[i * 8 + j] == 1)
                Color = {0.0f, 0.5f, 0.5f, 1.0f};
            else if(Level[i * 8 + j] == 2)
                Color = {1.0f, 0.3f, 0.2f, 1.0f};
            else if(Level[i * 8 + j] == 3)
                Color = {0.0f, 0.7f, 0.9f, 1.0f};
            else if(Level[i * 8 + j] == 4)
                Color = {0.0f, 0.3f, 0.9f, 1.0f};
            else if(Level[i * 8 + j] == 5)
                Color = {1.0f, 0.7f, 0.3f, 1.0f};
            DrawBatchRectangle(Batch, &CurrentState->Textures[1], Color, NULL, Position, Dimensions);
            if(RectangleColloide({Position, Dimensions}, CurrentState->Ball))
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

    rect SrcClip = {100, 100, 100, 100};
    DrawBatchRectangle(Batch, &CurrentState->Textures[2], {1, 1, 1, 1}, &SrcClip, Paddle->Pos, Paddle->Dimensions);
    DrawBatchRectangle(Batch, &CurrentState->Textures[0], {1, 1, 1, 1}, NULL, Ball->Pos, Ball->Dimensions);

    BeginText(Batch, (u8*)CurrentState->TextFontFile->Data, 32);
    DrawString("Hello World", 200, 30, 1, {1, 1, 1, 1});
    DrawString("My Name is Rajat", 100, 300, 1, {1, 0.5, 1, 1});

    FlushRenderer(Batch);

    BeginText(&CurrentState->TextData, CurrentState->Glyphs, &CurrentState->Projection, &State->ScratchStorage);
    const char* LiveString = "Lives: %i";
    char Buffer[50];

    sprintf(Buffer, LiveString, CurrentState->Lives);

    RenderText(&CurrentState->TextData, Buffer, {0.0f, 10.0f},
               0.5f, {1.f, 1.f, 1.f, 1.f});

    const char* FpsString = "FPS: %u";
    if((1000/(State->Delta * 1.0e2f)) >= 55.0f)
        sprintf(Buffer, FpsString, 60);
    else
    {
        sprintf(Buffer, FpsString, (u32)(1000/(State->Delta * 1.0e2f)));
    }
    // TODO(rajat): Might not render stuff like this
    RenderText(&CurrentState->TextData, Buffer, {700, 570},
               0.3f, {1.f, 1.f, 1.f, 1.f});

    if(NumActieTiles == 0)
    {
        RenderText(&CurrentState->TextData, "You Win!", {250.0f, 300.0f}, 1.0f, {1.f, 1.f, 1.f, 1.f});
        RenderText(&CurrentState->TextData, "Press Terminate to close", {190.0f, 384.0f}, 0.5f, {1.f, 1.f, 1.f, 1.f});
        CurrentState->IsPaused = true;
        CurrentState->Fired = false;
    }
    else
    {
        if(CurrentState->IsPaused)
        {
            RenderText(&CurrentState->TextData, "Paused!", {250, 300}, 1.0f, {1.f, 1.f, 1.f, 1.f});
        }
    }

    EndText(&CurrentState->TextData);
    // sprite_batch Batch;
    // StartBatch(&Batch);
    // DrawRectangle(&Batch, Texture, Color, Pos, Dim, TextureClip);
    // EndBatch(&Batch);
}
