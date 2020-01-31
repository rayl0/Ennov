#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include "ennov.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ennov_math.h"
#include "ennov_gl.cpp"

// TODO(Rajat): Implement Rand number generator
// TODO(Rajat): Implement Matrix and transformation methods

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
    glm::mat4 Projection; // TODO(Rajat): Replace glm fast!
    batch DrawBatchs[10];
    b32 Fired;
    vec2 Direction;
    rect Ball;
    rect Paddle;
    u32 Level[32];
    s32 Lives;
    b32 IsPaused;
    f32 TransientBatchBuffer[2048];
    u32 TransientSectionPos[10];
    u32 TransientSectionSize;
};

void GameUpdateAndRender(game_memory* Memory, game_state *State, game_input *Input)
{
  // NOTE(Rajat): Never do assertions within a loop increment
  // Assert(Count < 1); 
  
  //  OpenGLInitContext({State->ContextAttribs.Width, State->ContextAttribs.Height});
  // DrawRectangle(draw_attribs);

    gladLoadGL(); 

  breakout_game_state* CurrentState = (breakout_game_state*)Memory->PermanentStorage;
  if(!Memory->IsInitialized) {

      // TODO(Rajat): Move OpenGL code to platform layer and introduce command buffer
    if(!CurrentState->HaveLoadState) {
      CurrentState->Paddle = {{0.0f, 550.0f}, {100.0f, 50.0f}};
      CurrentState->Ball = {{CurrentState->Paddle.Dimensions.x / 2.0f, 530.0f}, {20.0f, 20.0f}};
      CurrentState->Direction = {4.0f, 4.0f};

      CurrentState->Fired = false;
      CurrentState->Lives = 3;

      CurrentState->IsInitialized = true;
      CurrentState->IsPaused = false;

      u32 TileMap[32] = {
                         1, 2, 3, 1, 4, 1, 2, 3,
                         5, 3, 5, 1, 3, 5, 4, 1,
                         4, 1, 0, 0, 0, 1, 0, 0,
                         0, 0, 0, 0, 4, 3, 0, 0
      };

      for(int i = 0; i < 32; ++i) {
        CurrentState->Level[i] =  TileMap[i];
      }
    }
    CurrentState->TransientSectionSize = 256;

    for(int i = 0; i < 10; ++i) {
      CurrentState->DrawBatchs[i].Id = i;
      CurrentState->DrawBatchs[i].VertexBufferSize = CurrentState->TransientSectionSize;
      CurrentState->DrawBatchs[i].VertexBufferData = &CurrentState->TransientBatchBuffer[i * CurrentState->TransientSectionSize];
      CurrentState->DrawBatchs[i].IsInitialized = false;
    }

      CurrentState->BackgroundBitmap = State->Interface.PlatformLoadBitmapFrom("./background.jpg");
      CurrentState->BallBitmap = State->Interface.PlatformLoadBitmapFrom("./background.jpg");
      CurrentState->PaddleBitmap = State->Interface.PlatformLoadBitmapFrom("./paddle.png");
      CurrentState->TileBitmap = State->Interface.PlatformLoadBitmapFrom("./block-textures.png");

      CurrentState->Projection = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f); // TODO(Rajat): Update projection with window size change!

      Memory->IsInitialized = true;
  }

  rect* Ball = &CurrentState->Ball;
  rect* Paddle = &CurrentState->Paddle;
  vec2* Direction = &CurrentState->Direction;
  batch* Batch = CurrentState->DrawBatchs;
  
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
    Ball->Pos.y -= Direction->y;
    Ball->Pos.x += Direction->x;
  }
  if(Ball->Pos.x > 790.0f) Direction->x = -(Direction->x);
  if(Ball->Pos.y < 0.0f) Direction->y = -(Direction->y);
  if(Ball->Pos.x < 0.0f) Direction->x = -(Direction->x);
  if(Ball->Pos.y > 600.0f) {
    CurrentState->Fired = false;
    Ball->Pos = {100.0f, 550.0f - Ball->Dimensions.y};
    *Direction = { 4.0f, 4.0f};
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
      Paddle->Pos.x += 64.0f;
      if(Paddle->Pos.x >= 700.0f)
        Paddle->Pos.x = 700.0f;
    }
  if(Input->Button.MoveLeft.EndedDown)
    {
      Paddle->Pos.x -= 64.0f;
      if(Paddle->Pos.x <= 0.0f)
        Paddle->Pos.x = 0.0f;
    }
  }

  uint32 NumActieTiles = 0;

  StartBatch(&Batch[BackgroundBatch], CurrentState->BackgroundBitmap, CurrentState->Projection);
  DrawBatchRectangle(&Batch[BackgroundBatch], {0, 0}, {(f32)CurrentState->BackgroundBitmap->Width, (f32)CurrentState->BackgroundBitmap->Height}, {1.0f, 1.0f, 1.0f, 1.0f});
  EndBatch(&Batch[BackgroundBatch]);

  StartBatch(&Batch[TileBatch], CurrentState->TileBitmap, CurrentState->Projection);

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
          DrawBatchRectangle(&Batch[TileBatch], Position, Dimensions, Color);
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

  EndBatch(&Batch[TileBatch]);

  if(NumActieTiles == 0)
    {
      fprintf(stderr, "You Win!\n");
      exit(EXIT_SUCCESS);
    }

  StartBatch(&Batch[BallBatch], CurrentState->BallBitmap, CurrentState->Projection);
  DrawBatchRectangle(&Batch[BallBatch], Ball->Pos, Ball->Dimensions, {1.0f, 1.0f, 1.0f, 1.0f});
  EndBatch(&Batch[BallBatch]);

  StartBatch(&Batch[PaddleBatch], CurrentState->PaddleBitmap, CurrentState->Projection);
  DrawBatchRectangle(&Batch[PaddleBatch], Paddle->Pos, Paddle->Dimensions, {1.0f, 1.0f, 1.0f, 1.0f});
  EndBatch(&Batch[PaddleBatch]);

  if(Input->Button.Terminate.EndedDown)
    {
      CurrentState->HaveLoadState = true;
    }
}
