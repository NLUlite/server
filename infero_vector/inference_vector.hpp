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



#ifndef __INFERENCE_VECT__
#define __INFERENCE_VECT__

#include<iostream>
#include<fstream>
#include<boost/utility.hpp>
#include<vector>
#include"../drt/DrtPred.hpp"
#include"clause_vector.hpp"
#include"policy_vector.hpp"
#include"path_memory.hpp"
#include"logic_path_vector.hpp"
#include"Level.hpp"

class inference_vector {
     static int instance_;
     int L;
     int numSteps;
     logic_path_vector path;
     vector< std::pair< DrtMgu, int > > finalDrtMgu;
     vector< Level > lastData;
     path_memory memory_;
     vector< vector<clause_vector> > clauses_;
     policy_non_contiguous_vect policy;
     vector< vector<DrtPred> > data_;

public:

     inference_vector(const DrtPred &q,
             const vector< vector<DrtPred> > &d,
             const int &l,
             int num_level,
             const vector< vector<clause_vector> > &fc,
             const path_memory &mem,
             Knowledge *k
             )
     : path(q, d, l, num_level, fc, k), L(l), memory_(mem), clauses_(fc), data_(d), policy(k) 
     {
          numSteps = 0;
     }
     
     void setNoise(const double &n) 
     {
          path.setNoise(n);
     }
     void setData(const vector<DrtVect> &d) 
     {
          path.setData(d);
     }
     void setQuestion(const DrtPred &q)
     {
	  path.setQuestion(q);
     }
     void setFeetClauses(const vector< vector<clause_vector> > &c)
     {
	  path.setFeetClauses(c);
     }
     void clear()
     {
	  numSteps= 0;
	  path.clear();
     }
     vector< std::pair< DrtMgu, int > > getFinalDrtMgu() 
     {
          return finalDrtMgu;
     }
     int getNumSteps() 
     {
          return numSteps;
     }
     logic_path_vector* logic_path() 
     {
          return &path;
     }
     
     void init();
     void makeFeetStep(const int &, const int &);
     void computeFinalWeigths();
     vector<Level> getLastData();     
};

#endif // __INFERENCE_VECT__
