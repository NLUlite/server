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



#include"Memory.hpp"


double Memory::getEntropy() const
{
     vector<double>::const_iterator witer= wvect_.begin();
     vector<double>::const_iterator wend= wvect_.end();

     vector<int>::const_iterator iiter= ivect_.begin();
     vector<int>::const_iterator iend= ivect_.end();

     double wlocal=1;

     for(; witer != wend; ++witer, ++iiter) {
      	  double w= *witer;
      	  int i= *iiter;
	  wlocal *= w*exp( -i );
     }
     return -log(wlocal);
}

void Memory::vanish()
{
     wvect_.clear();
     ivect_.clear();
}

// double Memory::getEntropy() const
// {
//      vector<double>::const_iterator witer= wvect_.begin();
//      vector<double>::const_iterator wend= wvect_.end();

//      if(witer == wend) return 0;

//      double wlocal=1;

//      for(; witer != wend; ++witer) {
//       	  double w= *witer;
// 	  wlocal *= w;
//      }
//      return wlocal;
// }

void Memory::add(int i, double w)
{
     addWeight(w);
     addNum(i);
}

void Memory::add(const Memory &m)
{
     vector<double> wvect2(m.wvect_);
     vector<int> ivect2(m.ivect_);

     wvect_.insert(wvect_.end(), wvect2.begin(), wvect2.end());
     ivect_.insert(ivect_.end(), ivect2.begin(), ivect2.end());
}
