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



#include"inference_vector.hpp"
#include<boost/thread.hpp>

const bool debug = false;

extern "C" {
#include"ran.h"
}

int inference_vector::instance_=0;

void inference_vector::init()
{
	if(instance_ == 0)
		init_ran(time(0) );	// Initialize random
}

static vector< std::pair< DrtMgu, double > > makePair( const vector<DrtMgu> &upgs, const vector<double> &weigths )
		{
	vector< std::pair< DrtMgu, double > > tmpPair;
	vector<DrtMgu>::const_iterator upgIter = upgs.begin();
	vector<DrtMgu>::const_iterator upgEnd = upgs.end();
	vector<double>::const_iterator wIter = weigths.begin();
	vector<double>::const_iterator wEnd = weigths.end();
	while (upgIter != upgEnd) {
		tmpPair.push_back(std::make_pair( *upgIter ,*wIter ) );
		++ upgIter;
		++ wIter;
	}
	return tmpPair;
		}

template <class T>
static void print_vector(std::stringstream &ss, const std::vector<T> &vs) 
{
	typename vector<T>::const_iterator tags_iter= vs.begin();
	while ( tags_iter < vs.end() ) {
		ss << (*tags_iter) << " ";
		++ tags_iter;
	}
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
static void printLastData(vector<vector<DrtPred > > &lastData)
{
	vector<vector<DrtPred > >::iterator diter= lastData.begin();

	std::cout << "-----" << std::endl;
	for(; diter != lastData.end(); ++diter) {
		print_vector(*diter);
		std::cout << "-----" << std::endl;
	}
}

void inference_vector::makeFeetStep(const int &num_cycles, const int &skip)
{
	int n;
	vector<int> dummy_int;
	vector< vector<clause_vector> > cvs = path.getClausesToAdd();

	for (n = 0; n < cvs.size(); ++n) {
		path.addFeetClause(0, cvs.at(n));
		for(int i=0; i < cvs.at(n).size(); ++i) {
			clause_vector ctmp= cvs.at(n).at(i);
		}
		path.updatePath();
		vector< vector<DrtPred> > tmp = path.getLastData();
		DrtMgu upg = path.getFinalDrtMgu();
		double w= path.getFinalWeigth();
		if (tmp.size()) {
			path_memory mem_tmp = memory_;
			vector<clause_vector> ctmp = cvs.at(n); /// This is only temporary, you must save all the clauses
			mem_tmp.push(ctmp, upg, w);
			Level level;
			level.setData(tmp);
			level.setMemory(mem_tmp);
			lastData.push_back(level);
			++numSteps;
		}
	}
}

void inference_vector::computeFinalWeigths()
{
	std::map<pair<DrtMgu,double>,int> weigthMap;
	std::map<pair<DrtMgu,double>,int>::iterator iterMap;
	std::map<pair<DrtMgu,double>,int>::iterator endMap;
	vector< pair<DrtMgu,int> >::iterator upgIter;
	vector< pair<DrtMgu,int> >::iterator endDrtMgu = finalDrtMgu.end();
	vector< pair< DrtMgu, int > > newFinalDrtMgu;

	for(upgIter= finalDrtMgu.begin();
			upgIter!=endDrtMgu;
			++upgIter) {
		++weigthMap[ *upgIter ];
	}

	iterMap= weigthMap.begin();
	endMap= weigthMap.end();

	while(iterMap != endMap) {
		newFinalDrtMgu.push_back( std::make_pair(iterMap->first.first,
				iterMap->first.second * iterMap->second) );
		++iterMap;
	}

	finalDrtMgu= newFinalDrtMgu;
}


static bool compare(const pair<vector<DrtPred>,int> &lhs, const pair<vector<DrtPred>,int> &rhs) 
{
	return lhs.second > rhs.second;
}

class compare_pred {
public:
	bool operator () (const vector<DrtPred> lhs, const vector<DrtPred> rhs) const
	{
		std::stringstream sl, sr;
		print_vector(sl, lhs);
		print_vector(sr, rhs);
		return sl.str() < sr.str();
	}
};

static void set_all_weigths(vector<DrtPred> *data, const double &w)
{
	vector<DrtPred>::iterator dataIter= data->begin();

	for(; dataIter != data->end(); ++dataIter) {
		dataIter->setWeigth(w);
	}
}

vector< Level > inference_vector::getLastData()
{
	return lastData;
}
