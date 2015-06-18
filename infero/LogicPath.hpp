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



#ifndef __LOGICPATH__
#define __LOGICPATH__

#include<iostream>
#include<fstream>
#include<boost/utility.hpp>
#include<vector>
#include<map>
#include<string>
#include<algorithm>
#include<stdexcept>
#include"FuzzyPred.hpp"
#include"Clause.hpp"
#include"Policy.hpp"

using std::map;
using std::vector;
using std::pair;
using std::string;
using std::cout;
using std::endl;

class LogicPath {  

     int L;
     //     int numSteps;
     double noise; 
     int numNotId;
     vector<bool>  isIdOnly;
     policy_contiguous policy;

     vector<vector<FuzzyPred> > predicatesData;
     vector<vector<FuzzyPred> > predicatesQuestions;  
     vector<FuzzyPred> lastData;  
     vector<Clause> clauses;
     vector<int> clausesNum;
     vector<Clause> feetClauses;
     vector<FuzzyPred> data;
     FuzzyPred question;
     vector<genUpg> finalUpgs;
     vector<double> finalWeigths;

     void updateAll();
     bool finalValues();
     void initClauses();
     void printAll();

public:

     LogicPath(const FuzzyPred &q,  // The question
	       const vector<FuzzyPred> &d, // The data
	       const int &l, // The max length of the logical chain
	       const vector<Clause> &fc); // The available clauses
     ~LogicPath() {}

     void setNoise(const double &n)
     {
	  noise= n;
     }
     void setData(const vector<FuzzyPred> &d)
     {
	  data= d;
     }
     void setQuestion(const FuzzyPred &q)
     {
	  question= q;
     }
     void setFeetClauses(const vector<Clause> &c)
     {
	  feetClauses= c;
     }


     void clear(); 
     void initRandom();
     bool updatePath();
     void addFeetClause(int m);
     void addFeetClause(int m, Clause clause);
     void removeClause(int m);
     void changeFeetClause(int m);
     int getLength();
     double getTotalProbability();
     vector<genUpg> getFinalUpgs() {return finalUpgs;}
     vector<double> getFinalWeigths() {return finalWeigths;}
     vector<FuzzyPred> getLastData() {return lastData;}
     pair<Clause,double> getFeetClauseToAdd(const int &);
     int getClauseToRemove();
     bool isId(const int &);
     void apply_single_clause(const Clause &clause);
};

#endif // __LOGICPATH__
