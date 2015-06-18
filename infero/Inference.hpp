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



#ifndef __INFERENCE__
#define __INFERENCE__

#include<iostream>
#include<fstream>
#include<boost/utility.hpp>
#include<vector>
#include"FuzzyPred.hpp"
#include"Clause.hpp"
#include"Policy.hpp"
#include"LogicPath.hpp"
#include"Memory.hpp"

using std::cout;
using std::endl;

class Inference {

     static int instance_;
     int L;
     int numSteps;
     LogicPath path;
     vector< std::pair< genUpg, int > > finalUpg;
     vector<pair<vector<FuzzyPred >, double> >lastData;
     Memory mem_;

public:
     Inference(const FuzzyPred &q, 
	       const vector<FuzzyPred> &d, 
	       const int &l, 
	       const vector<Clause> fc,
	       const Memory &m)
	  : path(q,d,l,fc), L(l), mem_(m)
     { 
	  numSteps= 0;
     }
     ~Inference() {}


     void setNoise(const double &n)
     {
	  path.setNoise(n);
     }
     void setData(const vector<FuzzyPred> &d)
     {
	  path.setData(d);
     }
     void setQuestion(const FuzzyPred &q)
     {
	  path.setQuestion(q);
     }
     void setFeetClauses(const vector<Clause> &c)
     {
	  path.setFeetClauses(c);
     }
     void clear()
     {
	  numSteps= 0;
	  path.clear();
     }    

     void init();
     void makeSingleStep(const Clause &clause);
     void thermalizeFeet(const int &);
     void makeFeetStep(const int &, const int &);
     vector< std::pair< genUpg, int > > getFinalUpg() { return finalUpg; }
     int getNumSteps() { return numSteps;}
     void computeFinalWeigths();
     vector<pair<vector<FuzzyPred>,Memory> > getLastData();
     LogicPath* logic_path() {return &path;}
};

#endif // __INFERENCE__
