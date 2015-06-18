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



#ifndef __CLAUSE_VECT__
#define __CLAUSE_VECT__


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
#include"../drt/DrtPred.hpp"
#include"../drt/metric_singleton.hpp"
#include"../engine/Engine.hpp"


class clause_vector {
     vector< vector<DrtPred> > hypothesis;
     vector<DrtPred> consequence;

     string link_;
     string text_;
     double weigth;
     int heigth;
     int position;

     CodePred code_;
     bool has_code_;

     CodePred match_code_;
     bool has_match_code_;


     vector<string> levin_descriptions_hyp_;
     vector<string> levin_descriptions_cons_;

     // This total weight is the combined weight under this clause
     //  double totalWeight;

     DrtMgu upg;
     void print_stream(std::ostream &out) const;  
     friend std::ostream & operator << (std::ostream &out, const clause_vector& c);

public:
     clause_vector() : has_code_(false), has_match_code_(false) {}
     clause_vector(const vector<DrtPred> &out, const vector< vector<DrtPred> > &in, const double &w=1)
	  : consequence(out), hypothesis(in), weigth(w), has_code_(false), has_match_code_(false)
     {}
     clause_vector(const std::string &s);

     ~clause_vector() {}

     bool operator < (const clause_vector &rhs) const
     {
	  return consequence < rhs.consequence && hypothesis < rhs.hypothesis;
     }

     bool operator == (const clause_vector &rhs) const
     {
	  return consequence == rhs.consequence && hypothesis == rhs.hypothesis;
     }

     int& pos() { return position; }
     int pos() const { return position; }
     int getHeigth() const { return heigth; }
     void setHeigth(double h) { heigth= h; }
     void  setWeigth(const double &w) {weigth = w;}
     double getWeigth() const {return weigth;}
     void setLink(const string &s) { link_= s;}
     string getLink() { return link_;}
     void setText(const string &s) { text_= s;}
     string getText() { return text_;}

     //  double getTotalWeigth() {return totalWeigth;}
     vector<DrtPred> getConsequence() const {return consequence;}
     vector<DrtPred> &getConsequence() {return consequence;}
     void setConsequence(const vector<DrtPred> &c) {consequence= c;}
     
     vector<DrtPred> getHypothesis() const {return hypothesis.at(0);}
     void setHypothesis(const vector<DrtPred> &h) {hypothesis.at(0)= h;}
     
     vector< DrtVect > getAllHypothesis() const {return hypothesis;}
     void setAllHypothesis(const vector< vector<DrtPred> > &h) {hypothesis= h;}
     
     
     DrtMgu  getDrtMgu() const {return upg;}
     void setDrtMgu(const DrtMgu &rhs);
     void uniVal(const int &value);     

     void find_levin();
     vector<string> getLevinCons() const {return levin_descriptions_cons_;}
     vector<string> getLevinHyp() const {return levin_descriptions_hyp_;}

     //  void setTotalWeigth(const double &w) {totalWeigth= w;}
     void print() const;

     void operator / (const DrtMgu &upg);

     void setCode(const CodePred &cp) {code_=cp; has_code_= true;}
     CodePred getCode() const {return code_;}
     bool hasCode() {return has_code_;}

     void setMatchCode(const CodePred &cp) {match_code_=cp; has_match_code_= true;}
     CodePred getMatchCode() const {return match_code_;}
     bool hasMatchCode() {return has_match_code_;}
};

void set_clauses_unival(vector<clause_vector> *cv, int n);
void set_clauses_unival(vector< vector<clause_vector> > *cv, int n);

void operator / (std::vector<clause_vector> &predVect, const DrtMgu &upg);
void operator / (std::vector<std::vector<clause_vector> > &predVect, const DrtMgu &upg);

// Sets all the clauses to a specific unival

#endif // __CLAUSE_VECT__
