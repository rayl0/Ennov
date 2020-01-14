#include <stdio.h>
#include "ennov.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

global_variable int Count = 0; // Counter, will be removed soon

void GameUpdateAndRender(game_state *State, game_input *Input)
{
    glm::mat4 Transform = glm::mat4(1.0f);
    local_persist int32 X = 0, Y = 0;
    // NOTE(Rajat): Never do assertions with a loop increment
    // Assert(Count < 1);

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
        Y += 10;
    }
    if(Input->Button.MoveRight.EndedDown)
    {
        X += 10;
    }
    if(Input->Button.MoveLeft.EndedDown)
    {
        X -= 10;
    }
    if(Input->Button.MoveUp.EndedDown)
    {
        Y -= 10;
    }
    Transform =  glm::translate(Transform, glm::vec3((real32)X, (real32)Y, 0.0f));
    Transform = glm::scale(Transform, glm::vec3(100.0f, 100.0f, 1.0f));
    *(glm::mat4*)(State->Transform) = Transform;
}
