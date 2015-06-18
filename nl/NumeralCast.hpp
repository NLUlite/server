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



#ifndef __NUMCAST__
#define __NUMCAST__

#include<iostream>
#include<vector>
#include<string>
#include<exception>
#include<boost/tuple/tuple.hpp>
#include<boost/lexical_cast.hpp>

using boost::tuple;
using boost::make_tuple;
using std::pair;
using std::string;
using std::make_pair;
using std::cout;
using std::endl;
using std::vector;

class NumeralCast {
     vector<pair<string,int> > base_;
     vector<pair<string,int> > multi_;
     vector<string> inb_;

     vector<tuple<int,int,int> >  substitutions_; // tuple< start_pos, end_pos, output > 
     string input_;
     string output_;

     void applySubstitutions();
     void compute(const string &input);

public:
     NumeralCast(const string &input);
     string result() { return output_; }
};


#endif // __NUMCAST__
