// File: metagen_base_core.h
// ------
// Copyright (c) 2026 Morgan Arrington
// Licensed under the MIT license (https://opensource.org/license/mit/)

#ifndef METAGEN_BASE_CORE_H
#define METAGEN_BASE_CORE_H

////////////////////////////////
//~ mja: Foreign Includes
#include <stdint.h>

////////////////////////////////
//~ mja: Codebase Keywords
#define internal      static
#define global        static
#define local_persist static

#if COMPILER_MSVC || (COMPILER_CLANG && OS_WINDOWS)
#pragma section(".rdata$", read)
#define read_only __declspec(allocate(".rdata$"))
#elif (COMPILER_CLANG && OS_LINUX)
#define read_only __attribute__((section(".rodata")))
#else
// NOTE(rjf): I don't know of a useful way to do this in GCC land.
// __attribute__((section(".rodata"))) looked promising, but it introduces a
// strange warning about malformed section attributes, and it doesn't look
// like writing to that section reliably produces access violations, strangely
// enough. (It does on Clang)
#define read_only
#endif

////////////////////////////////
//~ mja: Base Types
typedef uint8_t    U8;
typedef uint16_t   U16;
typedef uint32_t   U32;
typedef uint64_t   U64;
typedef int8_t     S8;
typedef int16_t    S16;
typedef int32_t    S32;
typedef int64_t    S64;
typedef S8         B8;
typedef S16        B16;
typedef S32        B32;
typedef S64        B64;
typedef float      F32;
typedef double     F64;
typedef void       VoidProc(void);
typedef union U128 U128;
union U128
{
  U8  u8[16];
  U16 u16[8];
  U32 u32[4];
  U64 u64[2];
  F32 f32[4];
  F64 f64[2];
};
typedef union U256 U256;
union U256
{
  U8   u8[32];
  U16  u16[16];
  U32  u32[8];
  U64  u64[4];
  U128 u128[2];
  F32  f32[8];
  F64  f64[4];
};
typedef union U512 U512;
union U512
{
  U8   u8[64];
  U16  u16[32];
  U32  u32[16];
  U64  u64[8];
  U128 u128[4];
  U256 u256[2];
  F32  f32[16];
  F64  f64[8];
};

////////////////////////////////
//~ mja: Asserts

#if COMPILER_MSVC
#define Trap() __debugbreak()
#elif COMPILER_CLANG || COMPILER_GCC
#define Trap() __builtin_trap()
#else
#error Unknown trap intrinsic for this compiler.
#endif

#define AssertAlways(x) \
  do                    \
  {                     \
    if (!(x))           \
    {                   \
      Trap();           \
    }                   \
  } while (0)
#if BUILD_DEBUG
#define Assert(x) AssertAlways(x)
#else
#define Assert(x) (void)(x)
#endif
#define InvalidPath         Assert(!"Invalid Path!")
#define NotImplemented      Assert(!"Not Implemented!")
#define NoOp                ((void)0)
#define StaticAssert(C, ID) global U8 Glue(ID, __LINE__)[(C) ? 1 : -1]

////////////////////////////////
//~ mja: Text 2D Coordinates & Ranges

typedef struct TxtPt TxtPt;
struct TxtPt
{
  S64 line;
  S64 column;
};

typedef struct TxtRng TxtRng;
struct TxtRng
{
  TxtPt min;
  TxtPt max;
};

#endif // !METAGEN_BASE_CORE_H
