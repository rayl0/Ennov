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

global_variable int Count = 0; // Counter, will be removed soon

void GameUpdateAndRender(game_state *State, game_input *Input)
{
  // NOTE(Rajat): Never do assertions within a loop increment
  // Assert(Count < 1); 
  
  OpenGLInitContext({State->ContextAttribs.Width, State->ContextAttribs.Height});
  // DrawRectangle(draw_attribs);

  local_persist rect_draw_attribs Paddle = {{0.0f, 590.0f}, {100.0f, 10.0f}, {1.0f, 1.0f, 0.0f, 1.0f}};
  local_persist rect_draw_attribs Ball = {{Paddle.Dimensions.x / 2.0f, 590.0f - Paddle.Dimensions.y}, {10.0f, 10.0f}, {0.5f, 0.3f, 0.7f, 1.0f}};
  local_persist bool32 Fired = false;
  local_persist vec2 Direction = {4.0f, 4.0f};
  local_persist rect BallRect;
  local_persist rect PaddleRect;
  local_persist rect_draw_attribs TileAttrib;
  local_persist uint32 Lives = 3;

  BallRect = {Ball.Position, Ball.Dimensions};
  PaddleRect = {Paddle.Position, Paddle.Dimensions};

  local_persist bool32 IsPaused = false;
  if(Input->Button.S.EndedDown) {
    if(IsPaused)
      {
        fprintf(stderr, "Resuming the game\n");
        IsPaused = false;
      }
    else {
       fprintf(stderr, "Pausing the game\n");
       IsPaused = true;
    }
  }
  if(!IsPaused) {
  if(Input->Button.Start.EndedDown) {
    Fired = true;
  }
  if(Fired) {
    Ball.Position.y -= Direction.y;
    Ball.Position.x += Direction.x;
  }
  if(Ball.Position.x > 790.0f) Direction.x = -(Direction.x);
  if(Ball.Position.y < 0.0f) Direction.y = -(Direction.y);
  if(Ball.Position.x < 0.0f) Direction.x = -(Direction.x);
  if(Ball.Position.y > 600.0f) {
    Fired = false;
    Ball.Position = {Paddle.Position.x + Paddle.Dimensions.x / 2.0f, 590.0f - Ball.Dimensions.y};
    Direction = { 4.0f, 4.0f};
    --(Lives);
    fprintf(stderr, "One Life deducted!\n");
    if(Lives == 0)
      {
        fprintf(stderr, "You Lose!\n");
        exit(EXIT_FAILURE);
      }
  }
  if(RectangleColloide(PaddleRect, BallRect))
    {
      if(Fired)
      Ball.Position = {Ball.Position.x, 580.0f - Ball.Dimensions.y};
      Direction.x = (Direction.x);
      Direction.y = -(Direction.y);
    }
  if(Input->Button.MoveRight.EndedDown)
    {
      Paddle.Position.x += 64.0f;
      if(!Fired)
        Ball.Position.x += 64.0f;
      if(Paddle.Position.x >= 700.0f)
        Paddle.Position.x = 700.0f;
    }
  if(Input->Button.MoveLeft.EndedDown)
    {
      Paddle.Position.x -= 64.0f;
      if(!Fired)
        Ball.Position.x -= 64.0f;
      if(Paddle.Position.x <= 0.0f)
        Paddle.Position.x = 0.0f;
    }
  }

  if(Count == 0)
    {
      TileAttrib.Texture = State->Interface.PlatformLoadBitmapFrom("./brick.jpg");
    }

  Count++;
  local_persist uint8 TileMap[4][8] = {
      {1, 2, 3, 1, 4, 1, 2, 3},
      {5, 3, 5, 1, 3, 5, 4, 1},
      {4, 1, 0, 0, 0, 1, 0, 0},
      {0, 0, 0, 0, 4, 3, 0, 0},
  };

  uint32 Id = 0;
  uint32 NumActieTiles = 0;
  for(int i = 0; i < 4; ++i)
  {
      for(int j = 0; j < 8; ++j)
      {
          TileAttrib.Id = Id;
          TileAttrib.Position = {j * 100.0f, i * 50.0f};
          TileAttrib.Dimensions = {100.0f, 50.0f};
          if(TileMap[i][j] == 0)
            continue;
          else if(TileMap[i][j] == 1)
              TileAttrib.Color = {0.0f, 0.5f, 0.5f, 1.0f};
          else if(TileMap[i][j] == 2)
              TileAttrib.Color = {1.0f, 0.3f, 0.2f, 1.0f};
          else if(TileMap[i][j] == 3)
              TileAttrib.Color = {0.0f, 0.7f, 0.9f, 1.0f};
          else if(TileMap[i][j] == 4)
              TileAttrib.Color = {0.0f, 0.3f, 0.9f, 1.0f};
          else if(TileMap[i][j] == 5)
              TileAttrib.Color = {1.0f, 0.7f, 0.3f, 1.0f};
          DrawRectangle(&TileAttrib, RECTANGLE_FILL_TEXCOLOR);
          if(RectangleColloide({TileAttrib.Position, TileAttrib.Dimensions}, BallRect))
            {
              fprintf(stderr, "Tile Colloide %i\n", TileMap[i][j]);
              TileMap[i][j] = 0;
              Direction.x = Direction.x;
              Direction.y = -Direction.y;
            }

          if(TileMap[i][j] != 0)
            {
              NumActieTiles++;
            }
      }
  }

  if(NumActieTiles == 0)
    {
      fprintf(stderr, "You Win!\n");
      exit(EXIT_SUCCESS);
    }


  DrawRectangle(&Paddle, RECTANGLE_FILL_COLOR);
  DrawRectangle(&Ball, RECTANGLE_FILL_COLOR);
}
