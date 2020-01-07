#include <stdio.h>
#include "ennov.h"
#include "ennov_platform.h"

void GameUpdateAndRender(game_input* Input)
{
	if(Input->Controller.MoveUp.EndedDown)
		fprintf(stderr, "%s\n", "Hello from the game");

	if(Input->Controller.MoveDown.EndedDown)
		fprintf(stderr, "%s\n", "Hello down from the game");

	if(Input->Controller.Terminate.EndedDown)
		fprintf(stderr, "%s\n", "Hello Game is ended now");
}
