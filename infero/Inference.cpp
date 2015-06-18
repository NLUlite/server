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



#include"Inference.hpp"
#include<boost/thread.hpp>

extern "C" {
#include"ran.h"
}

int Inference::instance_=0;

void Inference::init()
{
     //if(instance_ == 0) {
      //	  //init_ran(time(0)+(instance_++) );	// Initialize random
      //	  init_ran(time(0) );	// Initialize random
      //	  ++instance_;
     //}
     
}

static vector< std::pair< genUpg, double > > makePair( const vector<genUpg> &upgs, const vector<double> &weigths )
{
     vector< std::pair< genUpg, double > > tmpPair;
     vector<genUpg>::const_iterator upgIter = upgs.begin();
     vector<genUpg>::const_iterator upgEnd = upgs.end();
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
static void printLastData(vector<vector<FuzzyPred > > &lastData)
{
     vector<vector<FuzzyPred > >::iterator diter= lastData.begin();

     std::cout << "-----" << std::endl;
     for(; diter != lastData.end(); ++diter) {	  
	  print_vector(*diter);
	  std::cout << "-----" << std::endl;
     }
}

void Inference::makeSingleStep(const Clause &clause)
{
     path.clear();
     path.addFeetClause(0,clause); 
     path.updatePath();
     vector<FuzzyPred > tmp= path.getLastData();
     double wtot=clause.getWeigth();
     if( tmp.size() > 0) {
	  lastData.push_back( make_pair(tmp, wtot) );
	  path.clear();
	  ++numSteps;
     }	
}

void Inference::thermalizeFeet(const int &num_cycles)
{
     int n,m, c;
     for (n=0; n < num_cycles; ++n) {
	  for (m=0; m < L; ++m) {
	       if( path.isId(m)) //  Adds only one random available clause;
	   	    path.addFeetClause(m);   // addClause changes finalUpg and finalWeigth in path
	       else  // If it is not Id at m then remove random clause
	   	    path.removeClause(m);
	       //	       path.addFeetClause(m);	       
	  }
	  path.updatePath();
     }
}

void Inference::makeFeetStep(const int &num_cycles, const int &skip)
{
     int n,m, c;
     path.clear();
     path.initRandom();
     //thermalizeFeet(3*skip);
     for (n=0; n < num_cycles; ++n) {
	  for (m=0; m < L; ++m) {
	       if( path.isId(m)) //  Adds only one random available clause;
	   	    path.addFeetClause(m);   // addClause changes finalUpg and finalWeigth in path
	       else  // If it is not Id at m then remove random clause
	   	    path.removeClause(m);
	  }
	  path.updatePath();
	  if (n%skip == 0) {
	       vector<FuzzyPred > tmp= path.getLastData();
	       if( tmp.size() > 0) {
		    double wtot= path.getTotalProbability();
		    lastData.push_back( make_pair(tmp,wtot) );
		    ++numSteps;
	       }
	       // path.clear();
	       // path.initRandom();		       
	  }
     }
}


void Inference::computeFinalWeigths()
{
     std::map<pair<genUpg,double>,int> weigthMap;
     std::map<pair<genUpg,double>,int>::iterator iterMap;
     std::map<pair<genUpg,double>,int>::iterator endMap;
     vector< pair<genUpg,int> >::iterator upgIter;
     vector< pair<genUpg,int> >::iterator endUpg = finalUpg.end();
     vector< pair< genUpg, int > > newFinalUpg;  

     for(upgIter= finalUpg.begin(); 
	 upgIter!=endUpg;
	 ++upgIter) {
	  ++weigthMap[ *upgIter ];
	  //std::cout << (upgIter->first) << " , " << (upgIter->second) << std::endl;      
     }

     iterMap= weigthMap.begin();
     endMap= weigthMap.end();

     while(iterMap != endMap) { 
	  newFinalUpg.push_back( std::make_pair(iterMap->first.first, 
						iterMap->first.second * iterMap->second) );
	  ++iterMap;
     }

     finalUpg= newFinalUpg;
}


static bool compare(const pair<vector<FuzzyPred>,Memory> &lhs, const pair<vector<FuzzyPred>,Memory> &rhs) 
{
     return lhs.second.getEntropy() < rhs.second.getEntropy();
}

class compare_pred {
public:
     bool operator () (const pair<vector<FuzzyPred>,double> lhs, const pair<vector<FuzzyPred>,double> rhs) const
     {
	  std::stringstream sl, sr;
	  print_vector(sl, lhs.first);
	  print_vector(sr, rhs.first);
	  return sl.str() < sr.str();
	  //	  return lhs.second > lhs.second;
     }
};

static void set_all_weigths(vector<FuzzyPred> *data, const double &w)
{
     vector<FuzzyPred>::iterator dataIter= data->begin();
     vector<FuzzyPred>::iterator dataEnd= data->end();

     for(; dataIter != dataEnd; ++dataIter) {
	  dataIter->setWeigth(w);
     }
}

vector<pair<vector<FuzzyPred>,Memory> > Inference::getLastData()
{
     std::map<pair<vector<FuzzyPred>,double>,int, compare_pred> lastDataMap;
     std::map<pair<vector<FuzzyPred>,double>,int, compare_pred>::iterator iterMap;
     std::map<pair<vector<FuzzyPred>,double>,int, compare_pred>::iterator endMap;
     vector<pair<vector<FuzzyPred>,double> >::iterator dataIter;
     vector<pair<vector<FuzzyPred>,double> >::iterator endData= lastData.end();
     vector<pair<vector<FuzzyPred>,Memory> > returnLastData;

     for(dataIter= lastData.begin(); dataIter!=endData; ++dataIter) {
	  ++lastDataMap[*dataIter];
     }

     iterMap= lastDataMap.begin();
     endMap= lastDataMap.end();

     while(iterMap != endMap) {
	  pair<vector<FuzzyPred>,double> tmp = iterMap->first;	  	  
	  vector<FuzzyPred> data= tmp.first;
	  set_all_weigths(&data,1);	  
	  double wtmp= tmp.second;
	  Memory m( mem_ );
	  m.add(data.size(), tmp.second);
	  returnLastData.push_back( make_pair(data,m) );
	  ++iterMap;
     }
     sort(returnLastData.begin(),returnLastData.end(),compare);
     return returnLastData;
}
