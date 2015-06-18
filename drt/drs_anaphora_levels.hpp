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



#ifndef __DRT_LEVELS__
#define __DRT_LEVELS__

#include<iostream>
#include<algorithm>
#include<vector>
#include<cmath>
#include<map>
#include<string>
#include<fstream>
#include<boost/tuple/tuple.hpp>
#include<boost/utility.hpp>
#include<boost/algorithm/string.hpp>
#include<boost/lexical_cast.hpp>
#include"../infero/infero.hpp"
#include"../infero_vector/infero_vector.hpp"
#include"metric.hpp"
#include"metric_singleton.hpp"
#include"constituents.hpp"
#include"phrase.hpp"
#include"phrase_versions.hpp"
#include"drt.hpp"
#include"drt_collection.hpp"
#include"DrtPred.hpp"


using std::string;
using std::vector;
using std::map;
using boost::tuple;
using boost::make_tuple;
using std::pair;
using std::make_pair;

class drt_collection;

class drs_anaphora_levels {
 
public:
     void visit(drt_collection *);
};

#endif // __DRT_LEVELS__
