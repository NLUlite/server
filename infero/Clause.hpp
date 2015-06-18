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



#ifndef __CLAUSE__
#define __CLAUSE__


#include<iostream>
#include<fstream>
#include<boost/utility.hpp>
#include<boost/algorithm/string/classification.hpp>
#include<boost/algorithm/string/erase.hpp>
#include<boost/algorithm/string/split.hpp>
#include<vector>
#include<stdexcept>
#include<string>
#include<iostream>
#include"FuzzyPred.hpp"


class Clause {  
     vector<FuzzyPred> hypothesis;
     FuzzyPred consequence;
     double weigth;
     int heigth;
     int position;
     double consequenceInfo_, hypothesisInfo_;

     // This total weigth is the combined weigth under this clause
     //  double totalWeigth;

     genUpg upg;
     void print_stream(std::ostream &out) const;  
     friend std::ostream & operator << (std::ostream &out, const Clause& c);
public:
     Clause() {}
     Clause(const FuzzyPred &out, const vector<FuzzyPred> &in, const double &w=1)
	  : consequence(out), hypothesis(in), weigth(w) 
     {}
     Clause(const std::string &s);
     Clause(const Clause &c)
	  : consequence(c.consequence), hypothesis(c.hypothesis), weigth(c.weigth) 
     {}

     ~Clause() {}

     bool operator < (const Clause &rhs) const
     {
	  return consequence < rhs.consequence && hypothesis < rhs.hypothesis;
     }

     bool operator == (const Clause &rhs) const
     {
	  return consequence == rhs.consequence && hypothesis == rhs.hypothesis;
     }

     double gradientInfo() const {return -(consequenceInfo() - hypothesisInfo() );}
     double consequenceInfo() const {return consequenceInfo_;}
     double hypothesisInfo() const {return hypothesisInfo_;}
     void setConsequenceInfo(double w) { consequenceInfo_= w;}
     void setHypothesisInfo(double w) { hypothesisInfo_= w;}

     int& pos() { return position; }
     int pos() const { return position; }
     int getHeigth() const { return heigth; }
     void setHeigth(double h) { heigth= h; }
     void setWeigth(const double &w) {weigth = w;}
     double getWeigth() const {return weigth;}
     //  double getTotalWeigth() {return totalWeigth;}
     FuzzyPred getConsequence() const {return consequence;}
     FuzzyPred &getConsequence() {return consequence;}
     vector<FuzzyPred> getHypothesis() const {return hypothesis;}
     genUpg  getUpg() const {return upg;}
     void setUpg(const genUpg &rhs);
     void uniVal(const int &value);

     //  void setTotalWeigth(const double &w) {totalWeigth= w;}
     void print() const;

     void operator / (const genUpg &upg);
};

#endif // __CLAUSE__
