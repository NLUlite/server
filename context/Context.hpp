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



#ifndef __CONTEXT__
#define __CONTEXT__

#include<iostream>
#include<algorithm>
#include<vector>
#include<cmath>
#include<map>
#include<string>
#include<fstream>
#include<stdexcept>
#include<boost/tuple/tuple.hpp>
#include<boost/utility.hpp>
#include<boost/algorithm/string.hpp>
#include<boost/lexical_cast.hpp>
#include<boost/unordered_map.hpp>
#include"../drt/DrtPred.hpp"
//#include"../knowledge/Knowledge.hpp"

using std::string;
using std::vector;
using std::map;
using boost::tuple;
using boost::make_tuple;
using std::pair;
using std::make_pair;
using boost::unordered_map;


class Knowledge;

class Context {
     
     Knowledge *k_;
     //map<string,vector<DrtPred> > map_preds_;
     map<string,vector<string> > map_preds_;
     unordered_map<string, int > map_NNP_;

public:
     void addKnowledge(Knowledge *k) {k_ = k;}
     void addPredicate(const DrtPred &p);
     void add(const DrtVect &p);
     void add(const Context &c);

     double evaluateVerb(const DrtPred &p);
     double evaluateName(const DrtPred &p);
     double evaluateProperName(const DrtPred &p);

     double evaluateVerb(const string &p);
     double evaluateName(const string &p);
     double evaluateProperName(const string &p);

};

#endif // __CONTEXT__
