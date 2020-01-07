#if !defined(ENNOV_PALTFORM_H)

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define global_variable static
#define local_persist static
#define internal static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

typedef struct game_button_state
{
	int HalfTransitionCount;
	bool32 EndedDown;
}game_button_state;

typedef struct game_input_controller
{
	union {
		game_button_state Buttons[11];

		struct {
			game_button_state MoveUp;
			game_button_state MoveDown;
			game_button_state MoveLeft;
			game_button_state MoveRight;

			game_button_state Start;
			game_button_state Select;

			game_button_state ActionUp;
			game_button_state ActionDown;
			game_button_state ActionLeft;
			game_button_state ActionRight;

			game_button_state Terminate;
		};
	};
}game_input_controller;

typedef struct game_mouse_controller
{
	int X, Y;
	union {
		game_button_state Buttons[2];

		struct {
			game_button_state MouseLeft;
			game_button_state MouseRight;
		};
	};
}game_mouse_controller;

typedef struct game_input
{
	game_input_controller Controller;
	game_mouse_controller Mouse;
}game_input;

void GameUpdateAndRender(game_input* Input);

#ifdef __cplusplus
}
#endif

#define ENNOV_PALTFORM_H 
#endif