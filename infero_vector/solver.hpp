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



#ifndef __SOLVER__
#define __SOLVER__

#include<iostream>
#include<fstream>
#include<string>
#include<boost/utility.hpp>
#include<vector>
#include<map>
#include"infero_vector.hpp"
#include"path_memory.hpp"
#include"../knowledge/Knowledge.hpp" // This is needed for KnowledgeAnswer
#include"../drt/metric_singleton.hpp"
#include"../drt/drt.hpp"
#include"../match/Match.hpp"
#include"../match/Match.hpp"
#include"../wisdom/WisdomInfo.hpp"
#include"../pragmatics/Presupposition.hpp"


using std::string;
using std::vector;
using std::map;
using std::pair;
using std::make_pair;
using std::cout;
using std::endl;

class solver {

     Knowledge *k_;
     WisdomInfo *wi_;
     metric *dist_;
     drt drt_quest_;
     
     //vector<pair<vector<DrtPred>,path_memory > > solved_;     
     vector<KnowledgeAnswer> solved_;
     vector<DrtPred> question_;                         
     vector<pair<DrtMgu,double> > upg_vect_;

     vector< vector<clause_vector> > get_relevant_clauses(vector< DrtVect > &layer);
     vector<KnowledgeAnswer> arbiterStep(drt question);
     double is_match(const vector< vector<DrtPred> > &hyp, DrtMgu *retDrtMgu, vector<drt> *d, path_memory &mem);
     
     QuestionList qlist_;

     bool areValidRules(const path_memory &pm);
     vector<DrtVect> updateCurrentLayer(const Level &current_layer, DrtVect question);

public:
     solver(Knowledge *k, WisdomInfo *wi)
     {
     	dist_= metric_singleton::get_metric_instance();
          k_= k;
          wi_= wi;
     }
     
     void set_question(drt &q) 
     { 
          qlist_= q.getQuestionList();          
          drt_quest_= q;
          question_= q.predicates();           
     }
     
     void do_solve();
     vector<pair<DrtMgu,double> > get_DrtMgu_answer() 
     {
	  return upg_vect_;
     }
     //vector<pair<vector<DrtPred>,path_memory> > get_solved() 
     vector<KnowledgeAnswer> get_solved() 
     {
	  return solved_;
     }          
};

#endif // __SOLVER__
