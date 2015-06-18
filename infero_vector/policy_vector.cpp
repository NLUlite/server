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



#include"policy_vector.hpp"

#include<cmath>
#include<string>
#include<sstream>

using std::cout;
using std::endl;

static inline double min(const int &a, const int &b)
{
	if(a<b)
		return a;
	else
		return b;
}


static inline vector<int> get_predicates_positions(int pos_first_feet, const vector<int> &space_between_feet)
		{
	int size= space_between_feet.size()+1;
	vector<int> ret_vect(size);
	ret_vect.at(0)= pos_first_feet;
	for(int n=1; n < size; ++n)
		ret_vect.at(n)= ret_vect.at(n-1) + space_between_feet.at(n-1);
	return ret_vect;
		}

static inline vector<DrtPred> get_predicates_on_positions(const vector<DrtPred> &data, vector<int> positions)
		{
	vector<DrtPred> to_ret;
	int size= positions.size();
	for(int n=0; n < size; ++n)
		to_ret.push_back( data.at( positions.at(n) ) );
	return to_ret;
		}

double policy_non_contiguous_vect::canInsert(vector<DrtPred> &answers, 
		clause_vector &clause,
		Upg *retUpg,
		vector<int> *heigth
)
{
	vector<DrtPred> hyp= clause.getConsequence();
	int hyp_hooks= hyp.size();
	int hooks=0, hooks_tmp;
	double ret_weigth= 0;
	vector<DrtPred>::iterator answerIter;
	vector<DrtPred>::iterator endAnswer= answers.end();
	vector<DrtPred>::iterator hypIter;
	vector<DrtPred>::iterator endHyp= hyp.end();
	Upg upgTmp;
	DrtPred hypoTmp;
	Match match(k_);
	answerIter= answers.begin();
	hypIter= hyp.begin();
	heigth->clear();
	int pos=0;
	for(; hypIter != endHyp; ++hypIter, ++pos) {
		for(; answerIter != endAnswer; ++answerIter) {
			hypoTmp= (*hypIter);
			hypoTmp/upgTmp;
			Upg tmp_u;
			MatchType dummy_mtype;
			double tmp_w= match.singleMatch(hypoTmp,*answerIter,&tmp_u, &dummy_mtype);
			if( tmp_w > 0 ) {
				upgTmp.insert(upgTmp.end(), tmp_u.begin(), tmp_u.end());
				ret_weigth += tmp_w;
				heigth->push_back( pos );
				++answerIter;
				++hooks;
				break;
			}
		}
	}
	if(hooks != hyp_hooks)
		return 0;
	*retUpg = upgTmp;
	return ret_weigth/hyp_hooks;
}

double policy_non_contiguous_vect::canInsert_fast(vector<DrtPred> &answers, 
		clause_vector &clause)
{
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

double policy_contiguous_vect::canInsert(vector<DrtPred> &answers, 
		clause_vector &clause,
		Upg *retUpg,
		int *pos)
{
	int cons_hooks= 1;// clause.getConsequence().....//// num consequences;
	int hyp_hooks= clause.getHypothesis().size();
	int total_hooks= cons_hooks + hyp_hooks;
	int num_connected, num_connected_tmp;
	int hooks=0, hooks_tmp;
	vector<DrtPred>::iterator answerIter;
	vector<DrtPred>::iterator answerBegin;
	vector<DrtPred>::iterator endAnswerBegin;
	vector<DrtPred>::iterator endAnswer= answers.end();
	vector<DrtPred>::iterator hypIter;
	vector<DrtPred>::iterator endHyp;
	vector<DrtPred> hyp;
	Upg upgTmp;
	DrtPred hypoTmp;
	hyp= clause.getHypothesis();
	endHyp= hyp.end();
	vector<DrtPred> hypot= clause.getHypothesis();
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
				if( answerIter->getWeigth() > 0 &&  hypIter->unify(*answerIter, &upgTmp) ) {
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
	return clause.getWeigth()*(double)num_connected; ///
}
double policy_contiguous_vect::canInsert_fast(vector<DrtPred> &answers, 
		clause_vector &clause)
{
	int num_connected, num_connected_tmp;
	vector<DrtPred>::iterator answerIter;
	vector<DrtPred>::iterator answerBegin;
	vector<DrtPred>::iterator endAnswerBegin;
	vector<DrtPred>::iterator endAnswer= answers.end();
	vector<DrtPred>::iterator hypIter;
	vector<DrtPred>::iterator endHyp;
	vector<DrtPred> hyp;

	// Check unification hypothesis and return weigth and Upg
	hyp= clause.getHypothesis();
	endHyp= hyp.end();
	vector<DrtPred> hypot= clause.getHypothesis();
	if(answers.size() >= hyp.size())
		endAnswerBegin = endAnswer - clause.getHypothesis().size()+1;
	else
		endAnswerBegin = endAnswer;
	int m=0;
	num_connected= 0;
	for(answerBegin= answers.begin(); answerBegin != endAnswerBegin; ++answerBegin, ++m) {
		num_connected_tmp=0;
		answerIter= answerBegin;
		hypIter= hyp.begin();
		for(; hypIter != endHyp && answerIter != endAnswer; ++hypIter, ++answerIter ) {
			if( answerIter->getWeigth() > 0
					&&  hypIter->pred().begin()->str == answerIter->pred().begin()->str) {
				++num_connected_tmp;
			}
			else { 	    break;
			}
		}
		if(num_connected_tmp > num_connected)
			num_connected= num_connected_tmp;
		if(hypIter == endHyp)
			break;
	}
	if(num_connected < hyp.size())
		return 0;
	return clause.getWeigth()*(double)(num_connected);
}

double policy_contiguous_vect::canInsert(vector< vector<DrtPred> > &data,
		vector< clause_vector > &clauses,
		Upg *retUpg,
		vector<int> *pos
)
{       
	double w=1;

	Upg upgtmp= *retUpg;
	int posTmp;
	for(int n=0; n < data.size(); ++n) {
		double wtmp= this->canInsert(data.at(n), clauses.at(n), &upgtmp, &posTmp);
		if(wtmp == 0) return 0;
		w *= wtmp;
		pos->at(n) = posTmp;
	}
	*retUpg = upgtmp;
	return w;
}
