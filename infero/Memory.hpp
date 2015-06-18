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



#ifndef __MEMORY__
#define __MEMORY__

#include<iostream>
#include<fstream>
#include<stdexcept>
#include<vector>
#include<algorithm>
#include<string>
#include<cmath>
#include<boost/algorithm/string.hpp>
#include"FuzzyPred.hpp"

using std::vector;

class Memory {

     //vector<vector<FuzzyPred> > data_;
     vector<double> wvect_;
     vector<int> ivect_;

     //void addData(const vector<FuzzyPred> &d) {data_.push_back(d); }
     void addWeight(double w) {wvect_.push_back(w); }
     void addNum(int i) {ivect_.push_back(i); }

public:
     Memory() {}
     Memory(const Memory &m) : wvect_(m.wvect_), ivect_(m.ivect_) {}
     void add(const vector<FuzzyPred> &d, double w);
     void add(int i, double w);
     void add(const Memory &m);
     double getEntropy() const;
     void vanish();
};


#endif // __MEMORY__
