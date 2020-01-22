#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include "ennov.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ennov_math.h"

global_variable int Count = 0; // Counter, will be removed soon

struct draw_attribs
{
  vec2 Position;
  vec2 Dimensions;
  // vec4 Color;
  // loaded_bitmap* Texture;
};
void GameUpdateAndRender(game_state *State, game_input *Input)
{
  glm::mat4 Transform = glm::mat4(1.0f);
  local_persist rect SpriteRectangle = { {}, {100.0f, 100.0f}};
  local_persist vec2 MousePosition = {Input->Cursor.X, Input->Cursor.Y};
  // NOTE(Rajat): Never do assertions with a loop increment
  // Assert(Count < 1); 
  MousePosition = {Input->Cursor.X, Input->Cursor.Y};
  
  // DrawRectangle(draw_attribs);

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
      SpriteRectangle.Pos.Y += 10;
    }
  if(Input->Button.MoveRight.EndedDown)
    {
      SpriteRectangle.Pos.X += 10;
    }
  if(Input->Button.MoveLeft.EndedDown)
    {
      SpriteRectangle.Pos.X -= 10;
    }
  if(Input->Button.MoveUp.EndedDown)
    {
      SpriteRectangle.Pos.Y -= 10;
    }
  if(Input->Button.Select.EndedDown)
    {
      printf("This button has yet to be implemented");
    }
  if(RectangleContainsPoint(SpriteRectangle, MousePosition)) {
    fprintf(stderr, "In the rectangle!\n");
  } 
  Transform =  glm::translate(Transform, glm::vec3(SpriteRectangle.Pos.X, SpriteRectangle.Pos.Y, 0.0f));
  Transform = glm::scale(Transform, glm::vec3(SpriteRectangle.Dimensions.X, SpriteRectangle.Dimensions.Y, 1.0f));
  *(glm::mat4*)(State->Transform) = Transform;
}
