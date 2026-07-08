// File: base_strings.c
// ------
// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)
// Copyright (c) 2026 Morgan Arrington. All Rights Reserved.

////////////////////////////////
// Character Classification & Conversion Functions

internal B32 char_is_space(U8 c) {
  return (c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\f' ||
          c == '\v');
}

internal B32 char_is_upper(U8 c) { return ('A' <= c && c <= 'Z'); }

internal B32 char_is_lower(U8 c) { return ('a' <= c && c <= 'z'); }

internal B32 char_is_alpha(U8 c) {
  return (char_is_upper(c) || char_is_lower(c));
}

internal B32 char_is_slash(U8 c) { return (c == '/' || c == '\\'); }

internal B32 char_is_digit(U8 c, U32 base) {
  B32 result = 0;
  if (0 < base && base <= 16) {
    U8 val = integer_symbol_reverse[c];
    if (val < base) {
      result = 1;
    }
  }
  return result;
}

internal U8 lower_from_char(U8 c) {
  if (char_is_upper(c)) {
    c += ('a' - 'A');
  }
  return c;
}

internal U8 upper_from_char(U8 c) {
  if (char_is_lower(c)) {
    c += ('A' - 'a');
  }
  return c;
}

////////////////////////////////
// C-String Measurement

internal U64 cstring8_length(U8 *c) {
  U64 length = 0;
  if (c) {
    U8 *p = c;
    for (; *p != 0; p += 1)
      ;
    length = (U64)(p - c);
  }
  return length;
}

internal U64 cstring16_length(U16 *c) {
  U64 length = 0;
  if (c) {
    U16 *p = c;
    for (; *p != 0; p += 1)
      ;
    length = (U64)(p - c);
  }
  return length;
}

internal U64 cstring32_length(U32 *c) {
  U64 length = 0;
  if (c) {
    U32 *p = c;
    for (; *p != 0; p += 1)
      ;
    length = (U64)(p - c);
  }
  return length;
}

////////////////////////////////
// String Constructors

internal String8 str8(U8 *str, U64 size) {
  String8 result = {str, size};
  return result;
}

internal String8 str8_range(U8 *first, U8 *one_past_last) {
  String8 result = {first, (U64)(one_past_last - first)};
  return result;
}

internal String8 str8_zero(void) {
  String8 result = {0};
  return result;
}

internal String16 str16(U16 *str, U64 size) {
  String16 result = {str, size};
  return result;
}

internal String16 str16_range(U16 *first, U16 *one_past_last) {
  String16 result = {first, (U64)(one_past_last - first)};
  return result;
}

internal String16 str16_zero(void) {
  String16 result = {0};
  return result;
}

internal String32 str32(U32 *str, U64 size) {
  String32 result = {str, size};
  return result;
}

internal String32 str32_range(U32 *first, U32 *one_past_last) {
  String32 result = {first, (U64)(one_past_last - first)};
  return result;
}

internal String32 str32_zero(void) {
  String32 result = {0};
  return result;
}

internal String8 str8_cstring(char *c) {
  String8 result = {(U8 *)c, cstring8_length((U8 *)c)};
  return result;
}

internal String16 str16_cstring(U16 *c) {
  String16 result = {(U16 *)c, cstring16_length((U16 *)c)};
  return result;
}

internal String32 str32_cstring(U32 *c) {
  String32 result = {(U32 *)c, cstring32_length((U32 *)c)};
  return result;
}

internal String8 str8_cstring_capped(void *cstr, void *cap) {
  U64 cap_size = (U64)((U8 *)cap - (U8 *)cstr);
  U64 size = MemFind(cstr, cap_size, 0);
  return str8(cstr, size);
}

internal String16 str16_cstring_capped(void *cstr, void *cap) {
  U16 *ptr = (U16 *)cstr;
  U16 *opl = (U16 *)cap;
  for (; ptr < opl && *ptr != 0; ptr += 1)
    ;
  U64 size = (U64)(ptr - (U16 *)cstr);
  String16 result = str16(cstr, size);
  return result;
}

////////////////////////////////
// String Matching

internal B32 str8_match(String8 a, String8 b, StringMatchFlags flags) {
  B32 result = 0;
  if (a.size == b.size && flags == 0) {
    result = MemIsEqual(a.str, b.str, b.size);
  } else if (a.size == b.size && flags == StringMatchFlag_CaseInsensitive) {
    result = MemCompareI(a.str, b.str, a.size) == 0;
  }
  return result;
}
