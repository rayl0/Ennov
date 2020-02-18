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
#define internal_ static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef bool bool32;

typedef size_t memory_index;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8 s8;
typedef int16 s16;
typedef int32 s32;
typedef int64 s64;
typedef bool32 b32;

typedef uint8 u8;
typedef uint16 u16;
typedef uint32 u32;
typedef uint64 u64;

typedef float real32;
typedef double real64;

typedef real32 f32;
typedef real64 f64;

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
		game_button_state Buttons[11];

		// TODO(Rajat): Update them for the Standard gaming
		struct
		{
			game_button_state MoveUp;
			game_button_state MoveDown;
			game_button_state MoveLeft;
			game_button_state MoveRight;

			game_button_state Start;
            game_button_state Terminate;
			game_button_state Select;

			game_button_state X;
			game_button_state Y;
			game_button_state A;
			game_button_state S;
		}Button;
	};
}game_input;

#define PlatformFullScreenToggle_BIT (1 << 1)
#define PlatformMaximizeToggle_BIT (1 << 2)

typedef struct game_file
{
    void* Data;
    u32 Size;
}game_file;

typedef struct loaded_bitmap
{
    s32 Width;
    s32 Height;
    s32 Channels;
    uint8* Pixels;
}loaded_bitmap;

typedef struct game_memory
{
    b32 IsInitialized;
    void* PermanentStorage;
    u32 PermanentStorageSize;

    void *AssetMemory;
    u32 AssetMemorySize;

    void* TransientStorage;
    u32 TransientStorageSize;
}game_memory;

typedef struct game_areana
{
    void* BaseAddress;
    u32 Used;
    memory_index Size;
}game_areana;

void InitializeAreana(game_areana* Areana, void* BaseAddress, u32 Size);
void* PushStruct_(game_areana* Areana, memory_index Size);

#define PushStruct(Areana, Type) (Type *) PushStruct_(Areana, sizeof(Type))

typedef struct game_interface
{
	game_file*(*PlatformLoadFile)(char* File, void*(*)(game_areana*, memory_index), game_areana*);
}game_interface;

typedef struct game_state
{
	game_interface Interface;
    game_areana GameStorage;
    game_areana ScratchStorage;
    game_areana AssestStorage;
    f32 Delta;
  struct window_context_attribs
  {
    int32 Width;
    int32 Height;
  }ContextAttribs;
}game_state;

void GameUpdateAndRender(game_memory* Memory, game_state* State, game_input* Input, u32 *ConfigBits);

#ifdef __cplusplus
}
#endif

#define ENNOV_PALTFORM_H
#endif
