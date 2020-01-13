#include <stdio.h>
#include "ennov.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

void GameUpdateAndRender(game_input* Input, game_memory* Memory)
{
    glm::mat4 Transform = *(glm::mat4*)Memory->Transform;
   
	if(Input->Button.S.EndedDown) {
		printf("%s\n", "Hello World!");
		printf("%s:%i\n", "Repeat Count!", Input->Button.S.Repeat);
	}
	if(Input->Button.A.EndedDown) {
		printf("%s\n", "A button pressed");
	}
	if(Input->Button.Start.EndedDown) {
		printf("%s\n", "The Game has started everybody");
	}
    if(Input->Button.MoveDown.EndedDown) {
        Transform =  glm::translate(Transform, glm::vec3(1.0f, 0.0f, 0.0f));
    }
    *(glm::mat4*)(Memory->Transform) = Transform;
}
