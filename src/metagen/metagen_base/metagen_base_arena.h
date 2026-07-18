// File: metagen_base_arena.h
// ------
// Copyright (c) 2026 Morgan Arrington
// Licensed under the MIT license (https://opensource.org/license/mit/)

#ifndef METAGEN_BASE_ARENA_H
#define METAGEN_BASE_ARENA_H

////////////////////////////////
//~ mja: Arena Types

#define ARENA_HEADER_SIZE 128

typedef U64 ArenaFlags;
enum
{
  ArenaFlag_NoChain    = (1 << 0),
  ArenaFlag_LargePages = (1 << 1),
};

typedef struct ArenaParams ArenaParams;
struct ArenaParams
{
  ArenaFlags flags;
  U64        reserve_size;
  U64        commit_size;
  void      *optional_backing_buffer;
  char      *allocation_site_file;
  int        allocation_site_line;
  char      *name;
};

typedef struct Arena Arena;
struct Arena
{
  Arena     *prev;    // previous arena in chain
  Arena     *current; // current arena in chain
  ArenaFlags flags;
  U64        cmt_size;
  U64        res_size;
  U64        base_pos;
  U64        pos;
  U64        cmt;
  U64        res;
  char      *allocation_site_file;
  int        allocation_site_line;
  char      *name;
#if ARENA_FREE_LIST
  Arena *free_last;
#endif
#if ARENA_TABLE_DEBUG
  struct ArenaTableNode *table_node;
#endif
};
StaticAssert(sizeof(Arena) <= ARENA_HEADER_SIZE, arena_header_size_check);

#endif // !METAGEN_BASE_ARENA_H
