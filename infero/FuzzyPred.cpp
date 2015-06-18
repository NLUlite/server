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



#include"FuzzyPred.hpp"

void operator / (std::vector<FuzzyPred> &predVect, const genUpg &upg)
{
  std::vector<FuzzyPred>::iterator predIter;
  std::vector<FuzzyPred>::iterator endPred= predVect.end();

  for(predIter= predVect.begin(); predIter != endPred; predIter++) {
    (*predIter) / upg;
  }
}

void operator / (std::vector<FuzzyPred> &predVect, const Upg &upg)
{
  std::vector<FuzzyPred>::iterator predIter;
  std::vector<FuzzyPred>::iterator endPred= predVect.end();

  for(predIter= predVect.begin(); predIter != endPred; predIter++) {
    (*predIter) / upg;
  }
}

void FuzzyPred::print(std::ostream &out) const
{
     PredTree pt= this->pred();
     PredTree::iterator i= pt.begin();

     int d=i.depth();
     int diff;
     out << *i;
     ++i;
     while(i != pt.end()) {
	  if(i.depth() > d) {
	       ++d;
	       out << '(';
	  }
	  if( (diff = d - i.depth()) > 0) {
	       d -= diff;
	       for(;diff > 0; --diff)
		    out << ')';
	  }
	  if(i.parent() != this->pred().end() && i.parent().firstChild() != i)
	       out << ",";	  
	  out << *i;
	  ++i;
     }
     while(( (--d) - pt.begin().depth()+1) > 0)  out << ')';
}
