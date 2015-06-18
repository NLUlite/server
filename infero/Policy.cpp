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



#include"Policy.hpp"

//////
#include<string> ///////
#include<sstream>

static inline double min(const int &a, const int &b)
{
     if(a<b) 
	  return a;
     else 
	  return b;
}


double policy_non_contiguous::canInsert(vector<FuzzyPred> &answers, 
					Clause &clause, 
					//vector<FuzzyPred> &questions, 
					genUpg *retUpg,
					int *heigth
					)
{
     int cons_hooks= 1;// clause.getConsequence().....//// num consequences;
     int hyp_hooks= clause.getHypothesis().size();
     int total_hooks= cons_hooks + hyp_hooks;
     int hooks=0, hooks_tmp;
     // vector<FuzzyPred>::iterator questIter; 
     // vector<FuzzyPred>::iterator endQuest= questions.end(); 
     vector<FuzzyPred>::iterator answerIter; 
     vector<FuzzyPred>::iterator endAnswer= answers.end(); 
     vector<FuzzyPred>::iterator hypIter; 
     vector<FuzzyPred>::iterator endHyp;
     vector<FuzzyPred> hyp;
     // if(true ) { /// CHANGE THIS!!!
     // 	  if(questions.size() == 0)
     // 	       return 0;	  
	  
     // 	  //  Check unification consequence
     // 	  FuzzyPred cons = clause.getConsequence();
     // 	  questIter= questions.begin();
     // 	  for(; questIter != endQuest; ++questIter) {
     // 	       //	       questIter->unique(cons.unique(0));
     // 	       if( questIter->canGenUnify(cons) ) {
     // 		    //		    *heigth= questIter->getHeigth();
     // 		    ++hooks;
     // 		    //		    std::cout << "aakjklj\n ";
     // 		    break;
     // 	       }
     // 	  }
     // 	  if(questIter == endQuest) 
     // 	       return 0;
     // }
     // Check unification hypothesis and return weigth and Upg
     genUpg upgTmp, upgTmp2;
     FuzzyPred hypoTmp;
     hyp= clause.getHypothesis();
     endHyp= hyp.end();
     answerIter= answers.begin();
     upgTmp.clear();
     hypIter= hyp.begin();
     for(; hypIter != endHyp; ++hypIter) {
	  upgTmp2.clear();
	  hooks_tmp= hooks;
	  for(; answerIter != endAnswer; ++answerIter) {
	       hypoTmp= (*hypIter);
	       hypoTmp/upgTmp;
	       //	       hypoTmp.unique(answerIter->unique(0));
	       if(hypoTmp.canGenUnify(*answerIter) ) {
		    hypoTmp.genUnify(*answerIter, &upgTmp2);
		    upgTmp.insert(upgTmp.end(), upgTmp2.begin(), upgTmp2.end());
		    ++answerIter;

		    ++hooks;

		    break;
	       }
	  }
     }
     hooks= hooks_tmp;
     *retUpg = upgTmp;
     return clause.getWeigth()*(double)hooks/total_hooks; /// 
}
double policy_non_contiguous::canInsert_fast(vector<FuzzyPred> &answers, 
					     Clause &clause)
{
     return 0;
}

template <class T>
static void print_vector(std::vector<T> &vs)
{
     typename vector<T>::iterator tags_iter= vs.begin();
     while ( tags_iter < vs.end() ) {
	  std::cout << (*tags_iter) << " ";
	  ++ tags_iter;
     }
     std::cout << std::endl;
}

double policy_contiguous::canInsert(vector<FuzzyPred> &answers, 
				    Clause &clause, 
				    //vector<FuzzyPred> &questions, 
				    genUpg *retUpg,
				    int *pos)
{
     int cons_hooks= 1;// clause.getConsequence().....//// num consequences;
     int hyp_hooks= clause.getHypothesis().size();
     int total_hooks= cons_hooks + hyp_hooks;
     int num_connected, num_connected_tmp;
     int hooks=0, hooks_tmp;
     // vector<FuzzyPred>::iterator questIter; 
     // vector<FuzzyPred>::iterator endQuest= questions.end(); 
     vector<FuzzyPred>::iterator answerIter; 
     vector<FuzzyPred>::iterator answerBegin; 
     vector<FuzzyPred>::iterator endAnswerBegin; 
     vector<FuzzyPred>::iterator endAnswer= answers.end(); 
     vector<FuzzyPred>::iterator hypIter; 
     vector<FuzzyPred>::iterator endHyp;
     vector<FuzzyPred> hyp;
     // // Check unification consequence
     // FuzzyPred cons = clause.getConsequence();
     // questIter= questions.begin();    
     // for(; questIter != endQuest; ++questIter) {
     // 	  if( questIter->canGenUnify(cons) ) {
     // 	       ++hooks;
     // 	       break;
     // 	  }
     // }
     // Check unification hypothesis and return weigth and Upg
     genUpg upgTmp;
     FuzzyPred hypoTmp;
     hyp= clause.getHypothesis();
     endHyp= hyp.end();
     vector<FuzzyPred> hypot= clause.getHypothesis();
     if(answers.size() >= hyp_hooks) 
	  endAnswerBegin = endAnswer - clause.getHypothesis().size()+1;
     else 
      	  endAnswerBegin = endAnswer;
     int hooks_old= -1;
     int m=0;
     *pos=0;
     num_connected= 0;
     for(answerBegin= answers.begin(); answerBegin != endAnswerBegin; ++answerBegin, ++m) {
	  upgTmp.clear();
	  hooks_tmp= hooks;
	  num_connected_tmp=0;
	  answerIter= answerBegin;
	  hypIter= hyp.begin();
	  for(; hypIter != endHyp && answerIter != endAnswer; ++hypIter, ++answerIter ) {
	       //if(hypIter->canUnify(*answerIter)) {
	       if( answerIter->getWeigth() > 0 &&  hypIter->genUnify(*answerIter, &upgTmp) ) {
		    //	    hypIter->unify(*answerIter, &upgTmp);
		    ++num_connected_tmp;
		    ++hooks_tmp;
		    //		    }
	       }
	       else break;
	  }
	  if(hooks_tmp > hooks_old) {
	       *pos= m;
	       *retUpg= upgTmp;
	       hooks_old= hooks_tmp;
	       num_connected= num_connected_tmp;
	  }
	  if(hypIter == endHyp)
	       break;
     }
     hooks= hooks_tmp;
     if(num_connected != hyp_hooks) // this should be used only for feet clauses
	  return 0;
     // std::cout << "clause= " << clause << std::endl;
     // std::cout << "hooks= " << hooks << std::endl;
     //return clause.getWeigth()*(double)hooks/total_hooks; /// 
     //return clause.getWeigth()*(double)num_connected/hyp_hooks; /// 
     return clause.getWeigth()*(double)num_connected; /// 
}
double policy_contiguous::canInsert_fast(vector<FuzzyPred> &answers, 
					 Clause &clause)
{
     int num_connected, num_connected_tmp;
     vector<FuzzyPred>::iterator answerIter; 
     vector<FuzzyPred>::iterator answerBegin; 
     vector<FuzzyPred>::iterator endAnswerBegin; 
     vector<FuzzyPred>::iterator endAnswer= answers.end(); 
     vector<FuzzyPred>::iterator hypIter; 
     vector<FuzzyPred>::iterator endHyp;
     vector<FuzzyPred> hyp;

     // Check unification hypothesis and return weigth and Upg
     hyp= clause.getHypothesis();
     endHyp= hyp.end();
     vector<FuzzyPred> hypot= clause.getHypothesis();
     if(answers.size() >= hyp.size()) 
	  endAnswerBegin = endAnswer - clause.getHypothesis().size()+1;
     else 
      	  endAnswerBegin = endAnswer;
     int m=0;
     num_connected= 0;
     //int unified;
     for(answerBegin= answers.begin(); answerBegin != endAnswerBegin; ++answerBegin, ++m) {
	  //unified = 0;
	  num_connected_tmp=0;
	  answerIter= answerBegin;
	  hypIter= hyp.begin();
	  for(; hypIter != endHyp && answerIter != endAnswer; ++hypIter, ++answerIter ) {
	       if( answerIter->getWeigth() > 0 
		   &&  hypIter->pred().begin()->str == answerIter->pred().begin()->str) {
		    // if( hypIter->pred().begin().node->firstChild->data.str == answerIter->pred().begin().node->firstChild->data.str )
		    //  	 unified = 1;
		    ++num_connected_tmp;
	       }
	       else { 
		    // num_connected_tmp= 0;
		    // num_connected= 0;
		    break;
	       }
	  }
	  if(num_connected_tmp > num_connected)
	       num_connected= num_connected_tmp;
	  if(hypIter == endHyp)
	       break;
     }
     if(num_connected < hyp.size())
	  return 0;

     int size= hyp.size();     
     return clause.getWeigth()*exp( (double) size );
}
