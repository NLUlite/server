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



#ifndef __LOGIC_PATH_VECT__
#define __LOGIC_PATH_VECT__


#include"logic_path_vector.hpp"
#include<boost/thread.hpp>
#include<stdexcept>

extern "C" {
#include"ran.h"
}
extern "C" {
double ran_gaussian (double sigma);
}


using std::map;
using std::pair;
using std::make_pair;
using std::cout;
using std::endl;


const bool debug = false;

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

void logic_path_vector::clear()
{
	int incr;
	for(incr=0; incr < L; incr++) {
		predicatesData.at(incr).clear();
		predicatesQuestions.at(incr).clear();
	}
	predicatesData.resize(L);
	predicatesQuestions.resize(L);
	predicatesData.front() = data;
	predicatesQuestions.back().resize(1);
	//predicatesQuestions.back().at(0) = question;
	lastData.clear();

	isIdOnly.resize(L);
	numNotId=0;
	for(incr=0; incr < L; incr++)
		isIdOnly.at(incr)= true;
	clauses.clear();
	clauses.resize(L);
}


void logic_path_vector::initClauses()
{  
	clauses.resize(L);
}

logic_path_vector::logic_path_vector(const DrtPred &q, // The question
		const vector<DrtVect> &d, // The data
		const int &l, // The max length of the logical chain
		int num_level,
		const vector< vector<clause_vector> > &fc, // The feet available clauses
		Knowledge *k
)
: question(q), data(d), L(l), feetClauses(fc), k_(k), num_level_(num_level)
{
	noise=1; // Default noise

	predicatesData.resize(L);
	predicatesQuestions.resize(L);
	predicatesData.front() = data;
	predicatesQuestions.back().resize(1);

	isIdOnly.resize(L);
	numNotId=0;
	int incr;
	for(incr=0; incr < L; incr++)
		isIdOnly.at(incr)= true;

	initClauses();
}


static vector<DrtPred> get_next_data(const vector< DrtPred > &data,
		const vector< DrtPred > &hyp,
		const vector<DrtPred> &cons,
		const vector<int> &pos)
{
	vector< DrtPred > ret_vect(data);

	for(int n= pos.size()-1 ; n >= 0; --n) {
		// it has to be done backwards, otherwise the pointers in
		// "pos" are not correct
		ret_vect.erase( ret_vect.begin()+pos.at(n) );
	}
	ret_vect.insert(ret_vect.end(), cons.begin(), cons.end());
	return ret_vect;
}

void logic_path_vector::updateAll()
// Update the clauses by filling lastData, finalWeigth, and lastDrtMgu
{
	vector<int> pos;
	DrtVect cons;
	vector<DrtVect> hyp;
	clause_vector tmpClause;
	double weigth;
	Match m(k_);
	MatchInfo mi;
	mi.setInvertedPerson(true);
	MatchSubstitutions msubs;
	clause_vector id_clause;
	lastData.clear();
	finalWeigth=1;
	for (int nn = 0; nn < data.size(); ++nn) {
		vector<DrtVect> add_next;
		tmpClause = clauses.at(0).at(nn);
		if(tmpClause == id_clause) {
			lastData.push_back(data.at(nn));
			continue;
		}
		weigth = m.singlePhraseMatch(tmpClause.getConsequence(), data.at(nn), &msubs, mi);
		if (weigth > 0) {
			hyp  = tmpClause.getAllHypothesis();
			for(int j = 0; j < hyp.size(); ++j) {
				DrtMgu upg= msubs.getDrtMgu();
				hyp.at(j) / upg;
			}
			add_next= hyp;
			finalWeigth *= weigth;
		} else
			add_next.push_back( data.at(nn) );
		lastData.insert(lastData.end(), add_next.begin(), add_next.end());
	}
	finalDrtMgu = msubs.getDrtMgu();
}

bool logic_path_vector::updatePath()
// Returns true if a new estimate for the final question is made.
{
	int incr;
	for(incr=0; incr < L; incr++) {
		predicatesData.at(incr).clear();
		predicatesQuestions.at(incr).clear();
	}
	predicatesData.front() = data;
	lastData.clear();
	updateAll();
	return true;
}

void logic_path_vector::addFeetClause(int m, vector< clause_vector > cls)
{     
	isIdOnly.at(m)= false;
	numNotId++;
	clauses.at(m) = cls;
}

int logic_path_vector::getLength()
{
	return L;
}

vector< vector<clause_vector> > logic_path_vector::getClausesToAdd()
{     
	vector< vector<clause_vector> > to_return;
	if(feetClauses.size() == 0)
		return to_return;

	Match m(k_);
	MatchInfo mi;
	mi.setInvertedPerson(true);
	vector< tuple<vector<clause_vector>, int, double, MatchSubstitutions> > back_pairs;
	int hpos, hsize, fsize;
	hsize = data.size();
	fsize = feetClauses.size();
	back_pairs.resize(hsize);
	for (hpos = 0; hpos != hsize; ++hpos) {
		// Save the answers in the backtracking vector
		back_pairs.at(hpos).get<1>() = 0; // The backtracked position
		back_pairs.at(hpos).get<2>() = 0; // The backtracked weight
	}
	clause_vector id_clause;
	hpos = 0;
	while (hpos < hsize && hpos < fsize) {
		vector<DrtPred> question = data.at(hpos);
		double w;
		MatchSubstitutions msubs, prev_msubs;
		double prev_w = 1;
		vector<clause_vector> cls;
		clause_vector clause_tmp;
		int n = back_pairs.at(hpos).get<1>();
		int nmax = feetClauses.at(hpos).size();
		clause_tmp = feetClauses.at(hpos).at(n);
		// If there are answers at this level
		if (hpos > 0) {
			cls = back_pairs.at(hpos - 1).get<0>();
			prev_w = back_pairs.at(hpos - 1).get<2>();
			prev_msubs = back_pairs.at(hpos - 1).get<3>();
		}
		if(clause_tmp == id_clause) {
			cls.push_back(clause_tmp);
			if (hpos == hsize - 1) {
				to_return.push_back(cls);
				if(n != nmax -1)
					back_pairs.at(hpos).get<1>() = n + 1;
				else
					break;
				continue;
			}
			back_pairs.at(hpos).get<0>() = cls;
			back_pairs.at(hpos).get<1>() = n + 1;
			back_pairs.at(hpos).get<2>() = w * prev_w;
			back_pairs.at(hpos).get<3>() = msubs;
			++hpos;
			continue;
		}
		w = m.singlePhraseMatch(clause_tmp.getConsequence(), data.at(hpos), &msubs, mi);
		if (w < 0.4) {
			if (n == nmax - 1) {
				back_pairs.at(hpos).get<1>() = 0;
				back_pairs.at(hpos).get<2>() = 0;
				if(hpos==0)
					break; // nothing is found
					--hpos;
			} else
				back_pairs.at(hpos).get<1>()= n+1;
		} else {
			cls.push_back(clause_tmp);
			if (hpos == hsize - 1) {
				to_return.push_back(cls);
				if(n != nmax -1)
					back_pairs.at(hpos).get<1>() = n + 1;
				else
					break;
				continue;
			}
			// Save the data into the backtracking structure
			back_pairs.at(hpos).get<0>() = cls;
			back_pairs.at(hpos).get<1>() = n + 1;
			back_pairs.at(hpos).get<2>() = w * prev_w;
			back_pairs.at(hpos).get<3>() = msubs;
			++hpos;
		}
	}
	return to_return;
}

int logic_path_vector::getClauseToRemove()
// Return the clause number to change when removing
{
	if (numNotId == 0)
		return -1;
	int m;
	for(;;) {
		m = dran() *  L ;
		if(!isIdOnly.at(m))
			return m;
	}
}
bool logic_path_vector::isId(const int &m)
{
	if(m>=0 && m < L)
		return isIdOnly.at(m);
	return false;
}


#endif // __LOGIC_PATH_VECT__
