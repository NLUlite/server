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



#ifndef __FUZZYPRED__
#define __FUZZYPRED__

#include<iostream>
#include<fstream>
#include<boost/utility.hpp>
#include<vector>
#include<string>
#include"../fopl/fopl.hpp"

using std::vector;
using std::pair;

class FuzzyPred : public Predicate
{
     double weigth;
     std::string attribute_;
     friend void operator / (std::vector<FuzzyPred> &predVect, const genUpg &upg);
     friend void operator / (std::vector<FuzzyPred> &predVect, const Upg &upg);
public:
     FuzzyPred() {}
     FuzzyPred(const std::string &s, const double w=1) : Predicate(s), weigth(w) {}
     FuzzyPred(const PredTree &t, const double w=1):  Predicate(t), weigth(w) {}
     FuzzyPred(const Predicate &p, const double w=1):  Predicate(p), weigth(w) {}
     ~FuzzyPred() {}
     
     double getWeigth() const { return weigth; }
     void setWeigth(double w) { weigth= w; }
     std::string getAttribute() { return attribute_; }
     void setAttribute(const std::string &s) { attribute_= s; }
     
     void print(std::ostream &out) const;
};

void operator / (std::vector<FuzzyPred> &predVect, const genUpg &upg);
void operator / (std::vector<FuzzyPred> &predVect, const Upg &upg);

#endif // __FUZZYPRED__
