#pragma once
#ifndef ENNOV_DEFS_H

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

#define ENNOV_DEFS_H
#endif
