#if !defined(ENNOV_PALTFORM_H)

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define MEGABYTES_TO_BTYES(i)                   \
    (i * 1024 * 1024)

#ifdef ENNOV_DEBUG
#define Assert(Expression) {if(!(Expression)){ fprintf(stderr, "Assertion failed: %s\n", #Expression ); __builtin_trap(); }}
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

// Platform specific defines
// STUDY(Rajat): Various defines by platforms
#define ENNOV_PLATFORM_LINUX 0
#define ENNOV_PLATFORM_ANDROID 0

// TODO(rajat): Proper platforms defines should be used
#if defined(linux) || defined(__linux) || defined(__linux__)
    #undef ENNOV_PLATFORM_LINUX
    #define ENNOV_PLATFORM_LINUX 1
#endif
#if defined(ANDROID) || defined(__ANDROID__)
    #undef ENNOV_PLATFORM_ANDROID
    #define ENNOV_PLATFORM_ANDROID 1

    #if ENNOV_PLATFORM_LINUX
    #undef ENNOV_PLATFORM_LINUX
    #define ENNOV_PLATFORM_LINUX 0
    #endif
#endif

internal_
u32 EncodeRGBA(u8 r, u8 g, u8 b, u8 a)
{
    u32 t = r << 24 | g << 16 | b << 8 | a;
    return t;
}

internal_
void DecodeRGBA(u32 Color, u8 *r, u8 *g, u8 *b, u8 *a)
{
    *r = Color >> 24;
    *g = (Color << 8) >> 24;
    *b = (Color << 16) >> 24;
    *a = (Color << 24) >> 24;
}

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

extern void InitializeAreana(game_areana* Areana, void* BaseAddress, u32 Size);
extern void* PushStruct_(game_areana* Areana, memory_index Size);

#define PushStruct(Areana, Type) (Type *) PushStruct_(Areana, sizeof(Type))

extern void* PlatformLoadFile(const char* FileName, void*(*Push)(game_areana*, memory_index), game_areana* Areana);

typedef struct game_state
{
    game_areana GameStorage;
    game_areana ScratchStorage;
    game_areana AssestStorage;
    f32 dt;
    struct window_context_attribs
    {
        int32 Width;
        int32 Height;
    }ContextAttribs;
}game_state;

extern void GameUpdateAndRender(game_memory* Memory, game_state* State, game_input* Input, u32 *ConfigBits);

#ifdef __cplusplus
}
#endif

#define ENNOV_PALTFORM_H
#endif
