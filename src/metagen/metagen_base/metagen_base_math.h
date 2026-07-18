// File: metagen_base_math.h
// ------
// Copyright (c) 2026 Morgan Arrington
// Licensed under the MIT license (https://opensource.org/license/mit/)

#ifndef METAGEN_BASE_MATH_H
#define METAGEN_BASE_MATH_H

typedef union Rng1U64 Rng1U64;
union Rng1U64
{
  struct
  {
    U64 min;
    U64 max;
  };
  U64 v[2];
};

#endif // !METAGEN_BASE_MATH_H
