// File: base_strings.h
// ------
// Copyright (c) 2026 Morgan Arrington. All Rights Reserved.

#ifndef BASE_STRINGS_H
#define BASE_STRINGS_H

#ifndef MEM_STATIC
#define MEM_STATIC
#define MEM_API static inline
#include "third_party/martins_memfun/memfun.h"
#endif

////////////////////////////////
// String Types

typedef struct String8 String8;
struct String8 {
  U8 *str;
  U64 size;
};

typedef struct String16 String16;
struct String16 {
  U16 *str;
  U64 size;
};

typedef struct String32 String32;
struct String32 {
  U32 *str;
  U64 size;
};

////////////////////////////////
// String List & Array Types

typedef struct String8Node String8Node;
struct String8Node {
  String8Node *next;
  String8 string;
};

typedef struct String8List String8List;
struct String8List {
  String8Node *first;
  String8Node *last;
  U64 node_count;
  U64 total_size;
};

typedef struct String8Array String8Array;
struct String8Array {
  String8 *v;
  U64 count;
  U64 total_size;
};

////////////////////////////////
// String Matching, Splitting, & Joining Types

typedef U32 StringMatchFlags;
enum {
  StringMatchFlag_CaseInsensitive = (1 << 0),
};

typedef U32 StringSplitFlags;
enum {
  StringSplitFlag_KeepEmpties = (1 << 0),
};

typedef struct StringJoin StringJoin;
struct StringJoin {
  String8 pre;
  String8 sep;
  String8 post;
};

////////////////////////////////
// Character Classification & Conversion Functions

internal B32 char_is_space(U8 c);
internal B32 char_is_upper(U8 c);
internal B32 char_is_lower(U8 c);
internal B32 char_is_alpha(U8 c);
internal B32 char_is_slash(U8 c);
internal B32 char_is_digit(U8 c, U32 base);
internal U8 lower_from_char(U8 c);
internal U8 upper_from_char(U8 c);

////////////////////////////////
// C-String Measurement

internal U64 cstring8_length(U8 *c);
internal U64 cstring16_length(U16 *c);
internal U64 cstring32_length(U32 *c);

////////////////////////////////
// String Constructors

internal String8 str8(U8 *str, U64 size);
internal String8 str8_range(U8 *first, U8 *one_past_last);
internal String8 str8_zero(void);

internal String16 str16(U16 *str, U64 size);
internal String16 str16_range(U16 *first, U16 *one_past_last);
internal String16 str16_zero(void);

internal String32 str32(U32 *str, U64 size);
internal String32 str32_range(U32 *first, U32 *one_past_last);
internal String32 str32_zero(void);

internal String8 str8_cstring(char *c);
internal String16 str16_cstring(U16 *c);
internal String32 str32_cstring(U32 *c);

internal String8 str8_cstring_capped(void *cstr, void *cap);
internal String16 str16_cstring_capped(void *cstr, void *cap);

////////////////////////////////
// String Matching

internal B32 str8_match(String8 a, String8 b, StringMatchFlags flags);

#endif
