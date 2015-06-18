/*
The MIT License (MIT)

Copyright (c) 2015 Alberto Cetoli (alberto.cetoli@nlulite.com)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/



#include <stdlib.h>
#include <time.h>
#include "ran.h"

// Adapted from Newman & Barkema, Monte Carlo Methods in Statistical Physics.

//#define SIZE 55
//#define OFFSET 24
#define SIZE (1279)
#define OFFSET (216)
//#define SIZE 22501
//#define OFFSET 1280

#define A 2416
#define C 374441
#define M 1771875
#define CONV 2423.9674
#define UCONV (1.0/(UINT_MAX + 1.0))
#define ICONV (1.0/(INT_MAX + 1.0))

static unsigned int vec[SIZE] = {0};
static int p, pp;

/*** Initialize the vector. ***/
void init_ran(int seed)
{
  int i;

  if (!seed)
    seed = time(NULL);
  for (i = 0; i < SIZE; i++) {
    seed = (A * seed + C) % M;
    vec[i] = CONV * seed;
  }
  p = 0;
  pp = OFFSET;
}

/*** This does the real job ***/
inline unsigned int uran()		// Integer 0 ... 2^32 - 1
{
  if (--pp < 0) pp = SIZE - 1;
  if (--p  < 0) p  = SIZE - 1;

  vec[p] += vec[pp];
  return vec[p];
}

double dran()				// Double [0, 1)
{
  return UCONV * uran();
}

double dran_sign()			// Double [-1, 1)
{
  return ICONV * (int) uran();
}

int iran()
{
  return uran() & INT_MAX;		// Integer 0 ... 2^31 - 1
}

int iran_sign()
{
  return (int) uran();			// Integer -2^31 ... 2^31 - 1
}
