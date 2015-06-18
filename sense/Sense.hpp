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



#ifndef __SENSE__
#define __SENSE__

#include<iostream>
#include<string>
#include<exception>
#include<vector>
#include<map>
#include<boost/unordered_map.hpp>
#include<boost/algorithm/string.hpp>
#include"../fopl/fopl.hpp"
#include"../match/Match.hpp"
#include"../infero/infero.hpp"
#include"../drt/metric_singleton.hpp"
#include"../drt/DrtPred.hpp"

using std::string;
using std::vector;
using std::map;
using std::cout;
using std::endl;
using std::stringstream;

typedef boost::unordered_map<int,vector<DrtVect> > MapIntDrt;
typedef boost::unordered_map<string,vector<DrtVect> > MapStrDrt;
typedef boost::unordered_map<string,pair<double,int> > MapDrtDouble;
//typedef boost::unordered_map<string,vector<pair<DrtVect,double> > > MapStrDrtDouble;

class Sense {

     MapIntDrt sense_actors_;
     MapIntDrt nouns_actors_;
     MapIntDrt sense_impossible_;
     MapIntDrt sense_complements_;
     MapIntDrt nouns_complements_;
     MapIntDrt sense_negative_;
     MapStrDrt sense_combined_;
     MapStrDrt nouns_combined_;
     MapDrtDouble sense_weight_;
     MapDrtDouble nouns_weight_;
     MapStrDrt sense_negative_combined_;
     MapDrtDouble sense_negative_weight_;

     double rateVerbs(DrtVect drtvect);
     double rateNouns(DrtVect drtvect);

public:
     Sense() {}
     Sense(const Sense &rhs);

     void loadVerbAgents(const string &s);
     void loadVerbComplements(const string &s);
     void loadNounsAgents(const string &s);
     void loadNounsComplements(const string &s);
     void loadImpossible(const string &s);
     void loadNegative(const string &s);
     void loadCombinedNegative(const string &s);
     void loadVerbCombined(const string &s);
     void loadNounsCombined(const string &s);

     vector<DrtVect> getCandidateCombined(const DrtVect &drtvect, const string &lemma);
     vector<DrtVect> getCandidateDrtVect (int lexname);
     vector<DrtVect> getCandidateNounsDrtVect (int lexname);
     vector<DrtVect> getCandidateImpossibleDrtVect (int lexname);
     vector<DrtVect> getCandidateNegative(int lexname);
     vector<DrtVect> getCandidateCombinedNegative(const DrtVect &drtvect, const string &lemma);
     
     bool increaseWeight(const vector<DrtVect> &all_drts, double, int);
     bool decreaseWeight(const vector<DrtVect> &all_drts, double, int);
     vector<tuple<string,string,double,int> > getCombinedLines();
     vector<tuple<string,string,double,int> > getCombinedNegativeLines();

     double rate(DrtVect d);     
};


#endif // __SENSE__
