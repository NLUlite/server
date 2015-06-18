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



#ifndef __WRITER__
#define __WRITER__

#include<iostream>
#include<algorithm>
#include<vector>
#include<map>
#include<string>
#include"../infero_vector/clause_vector.hpp"
#include"../drt/DrtPred.hpp"
#include"../drt/drt.hpp"
#include"../knowledge/Knowledge.hpp"
#include"../match/Match.hpp"
#include"../nl/tagger_info.hpp"


using std::string;
using std::vector;
using std::map;
using boost::tuple;
using boost::make_tuple;
using std::pair;
using std::make_pair;

typedef FTree< pair<DrtPred, vector<DrtPred> > > SentenceTree ; 
// the first DrtPred is the complement that introduces the subordinate, while the vector<DrtPred> is the subordinate

class ComparePredNames {

	Priors priors_;
	string question_str_;

public:
	ComparePredNames(Priors priors, const string &question_str): priors_(priors), question_str_(question_str) {}
	bool operator () (const DrtPred &plhs, const DrtPred &prhs);
};



class Writer {

	Priors priors_;
     Knowledge &k_;

     pair<string,DrtPred> getStringFromPred(const DrtPred pred, vector<DrtPred> speech, bool conjugate, Priors *priors, DrtPred *printed_pred);
     vector< vector<DrtPred> > get_subordinates( const vector<DrtPred> &drs);
     vector< vector<DrtPred> > get_missing_complement_preds( const vector<DrtPred> &drs);
     vector< vector<DrtPred> > get_missing_specification_preds( const vector<DrtPred> &drs);
     void writer_unique(vector<DrtPred> &preds);

     SentenceTree getSentenceTree(vector<DrtPred> speech);
     string writeSingleSentence(DrtPred complement, vector<DrtPred> speech);
     vector<DrtPred> eliminateRedundant(vector<DrtPred> speech);
     vector<DrtPred> eliminateCompeting(vector<DrtPred> speech);
     vector<DrtPred> addMissing(vector<DrtPred> speech);
     vector<DrtPred> writer_graph_sort(const SingleMatchGraph &graph, const MatchWrite &mgraph);          

     string getNumberFromRef(const string ref);
     vector<DrtVect> getJoinedNounsFromRef(const string nref);
     DrtVect insertJoinedNouns(DrtVect speech);
     DrtVect insertAdverbs(DrtVect speech);
     DrtVect insertSpecifications(DrtVect speech);
     DrtVect insertSpecificationsLocally(DrtVect speech, int pos);
     string trivialSpeech(const DrtPred &d);
     
public:
     Writer(Knowledge &k) : k_(k) {}
     
     string write(const DrtPred &drs, Priors priors= Priors());
     string write(const vector<DrtPred> &drs, Priors priors= Priors());
     string write(const clause_vector &clause);
     string write(const vector<KnowledgeAnswer> &kav, drt &dquest);
};


#endif // __WRITER__
 
