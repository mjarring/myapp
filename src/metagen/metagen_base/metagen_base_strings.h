// File: metagen_base_strings.h
// ------
// Copyright (c) 2026 Morgan Arrington
// Licensed under the MIT license (https://opensource.org/license/mit/)

#ifndef METAGEN_BASE_STRINGS_H
#define METAGEN_BASE_STRINGS_H

////////////////////////////////
//~ mja: String Types

typedef struct String8 String8;
struct String8
{
  U8 *str;
  U64 size;
};

////////////////////////////////
//~ mja: String List & Array Types

typedef struct String8Node String8Node;
struct String8Node
{
  String8Node *next;
  String8      string;
};

typedef struct String8List String8List;
struct String8List
{
  String8Node *first;
  String8Node *last;
  U64          node_count;
  U64          total_size;
};

////////////////////////////////
//~ mja: String Matching, Splitting, & Joining Types

typedef U32 StringMatchFlags;
enum
{
  StringMatchFlag_CaseInsensitive  = (1 << 0),
  StringMatchFlag_RightSideSloppy  = (1 << 1),
  StringMatchFlag_SlashInsensitive = (1 << 2),
};

#endif // !METAGEN_BASE_STRINGS_H
