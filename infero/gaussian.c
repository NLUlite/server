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



#include<stdio.h>
#include<math.h>
#include"ran.h"

/* Of the two methods provided below, I think the Polar method is more
 * efficient, but only when you are actually producing two random
 * deviates.  We don't produce two, because then we'd have to save one
 * in a static variable for the next call, and that would screws up
 * re-entrant or threaded code, so we only produce one.  This makes
 * the Ratio method suddenly more appealing.  There are further tests
 * one can make if the log() is slow.  See Knuth for details */

/* Both methods pass the statistical tests; but the polar method
 * seems to be a touch faster on my home Pentium, EVEN though we
 * are only using half of the available random deviates!
 */

/* Polar (Box-Mueller) method; See Knuth v2, 3rd ed, p122 */

/* Code taken from
 * http://www.bearcave.com/misl/misl_tech/wavelets/hurst/random.html */

double ran_gaussian (double sigma)
{
  double x, y, r2, gauss;

  do
    {
      /* choose x,y in uniform square (-1,-1) to (+1,+1) */

      x = -1 + 2 * dran_sign();
      y = -1 + 2 * dran_sign();

      /* see if it is in the unit circle */
      r2 = x * x + y * y;
    }
  while (r2 > 1.0 || r2 == 0);

  gauss= sigma * y * sqrt (-2.0 * log (r2) / r2);

  /* Box-Muller transform */
  return gauss;
}
