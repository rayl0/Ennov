#include <stdio.h>
#include "ennov.h"
#include "ennov_platform.h"

void GameUpdateAndRender(game_input* Input)
{
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
}
