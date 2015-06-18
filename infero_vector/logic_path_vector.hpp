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



#ifndef __LOGICPATH_VECT__
#define __LOGICPATH_VECT__

#include<iostream>
#include<fstream>
#include<boost/utility.hpp>
#include<vector>
#include<map>
#include<string>
#include<algorithm>
#include<stdexcept>
#include"../drt/DrtPred.hpp"
#include"clause_vector.hpp"
#include"policy_vector.hpp"
#include"Level.hpp"


using std::map;
using std::vector;
using std::pair;
using std::make_pair;
using std::string;

class logic_path_vector {       
     int L;     
     int num_level_;
     double noise;
     int numNotId;
     vector<bool> isIdOnly;
     // policy_non_contiguous_vect policy;
     Knowledge *k_;

     vector< vector<DrtVect> > predicatesData;
     vector< vector<DrtVect> > predicatesQuestions;
     vector< DrtVect > lastData;
     vector< vector<clause_vector> > clauses;
     vector< vector<clause_vector> > feetClauses;
     vector< DrtVect > data;
     DrtPred question;
     DrtMgu finalDrtMgu;
     double finalWeigth;

     void updateAll();
     void initClauses();

public:

     logic_path_vector(const DrtPred &q, // The question
             const vector< vector<DrtPred> > &d, // The data
             const int &l, // The max length of the logical chain
             int num_level, // the level at which we are acting
             const vector< vector<clause_vector> > &fc, // The available clauses
             Knowledge *k
             );
    
     void setNoise(const double &n) 
     {
          noise = n;
     }

     void setData(const vector< DrtVect > &d) 
     {
          data = d;
     }

     void setQuestion(const DrtPred &q) 
     {
          question = q;
     }

     void setFeetClauses(const vector< vector<clause_vector> > &c) 
     {
          feetClauses = c;
     }

     void clear();
     bool updatePath();
     int getLength();
     void addFeetClause(int, vector<clause_vector>);

     DrtMgu getFinalDrtMgu() 
     {
          return finalDrtMgu;
     }

     double getFinalWeigth() 
     {
          return finalWeigth;
     }

     vector< DrtVect > getLastData() 
     {
          return lastData;
     }
     
     vector< vector<clause_vector> > getClausesToAdd();
     int getClauseToRemove();
     bool isId(const int &);
};

#endif // __LOGICPATH_VECT__
