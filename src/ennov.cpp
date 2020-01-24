#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include "ennov.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ennov_math.h"
#include "ennov_gl.cpp"

global_variable int Count = 0; // Counter, will be removed soon

void GameUpdateAndRender(game_state *State, game_input *Input)
{
  // NOTE(Rajat): Never do assertions with a loop increment
  // Assert(Count < 1); 
  
  OpenGLInitContext();
  // DrawRectangle(draw_attribs);

  local_persist rect_draw_attribs R1 = {{100.0f, 100.0f}, {100.0f, 100.0f}, {1.0f, 0.0f, 0.5f, 1.0f}};
  local_persist rect_draw_attribs R2 = {{600.0f, 100.0f}, {100.0f, 100.0f}, {1.0f, 0.5f, 0.5f, 1.0f}};

  if(Input->Button.S.EndedDown) {
    fprintf(stderr, "%s\n", "Hello World!");
    fprintf(stderr, "%s:%i\n", "Repeat Count!", Input->Button.S.Repeat);
  }
  if(Input->Button.A.EndedDown) {
    fprintf(stderr, "%s\n", "A button pressed");
  }
  if(Input->Button.Start.EndedDown) {
    fprintf(stderr, "%s\n", "The Game has started everybody");
  }
  if(Input->Button.MoveDown.EndedDown) 
    {
      R1.Position.y += 10;
    }
  if(Input->Button.MoveRight.EndedDown)
    {
      R1.Position.x += 10;
    }
  if(Input->Button.MoveLeft.EndedDown)
    {
      R1.Position.x -= 10;
    }
  if(Input->Button.MoveUp.EndedDown)
    {
      R1.Position.y -= 10;
    }
  if(Input->Button.Select.EndedDown)
    {
      printf("This button has yet to be implemented");
    }

  if(Count == 0)
    R1.Texture = State->Interface.PlatformLoadBitmapFrom("./stars.jpg");

  Count++;
  DrawRectangle(&R1, RECTANGLE_FILL_TEXTURE);
  DrawRectangle(&R2, RECTANGLE_FILL_COLOR);
}
