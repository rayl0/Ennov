#if !defined(ENNOV_PALTFORM_H)

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef ENNOV_DEBUG
#define Assert(Expression) {if(!(Expression)){ fprintf(stderr, "Assertion failed: %s\n", #Expression ); *(char*)0 = 1; }}
#else
#define Assert(Expression)
#endif 
    
#define global_variable static
#define local_persist static
#define internal static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef bool bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

// TODO(Rajat): Might want to replace it in future

typedef struct game_button_state
{
	int32  Repeat;
	bool32 EndedDown;
}game_button_state;

typedef struct game_cursor_state
{
    real32 X, Y;
}game_cursor_state;

typedef struct game_input
{
    game_cursor_state Cursor;    
	union {
		game_button_state Buttons[10];

		// TODO(Rajat): Update them for the Standard gaming
		struct 
		{
			game_button_state MoveUp;
			game_button_state MoveDown;
			game_button_state MoveLeft;
			game_button_state MoveRight;

			game_button_state Start;
			game_button_state Select;

			game_button_state X;
			game_button_state Y;
			game_button_state A;
			game_button_state S;
		}Button;
	};
}game_input;

typedef struct game_state 
{
    void* Transform;
}game_state;

void GameUpdateAndRender(game_state* State, game_input* Input);

#ifdef __cplusplus
}
#endif

#define ENNOV_PALTFORM_H 
#endif
