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



#include"LogicPath.hpp"
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

void LogicPath::clear()
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
     predicatesQuestions.back().at(0) = question;
     lastData.clear();

     isIdOnly.resize(L);
     clausesNum.resize(L);
     numNotId=0;
     for(incr=0; incr < L; incr++) {
	  isIdOnly.at(incr)= true;
	  clausesNum.at(incr)= 0;
     }
     clauses.clear();
     clauses.resize(L);
}


bool compare_clauses( const Clause &lhs, const Clause &rhs) 
// true if the left clause has higher priority
{
     double lhs_w= lhs.getWeigth()* exp(lhs.getHypothesis().size());
     double rhs_w= rhs.getWeigth()* exp(rhs.getHypothesis().size());

     return lhs_w > rhs_w;
}



void LogicPath::initClauses()
{  
     clauses.resize(L);     
}

LogicPath::LogicPath(const FuzzyPred &q,  // The question
		     const vector<FuzzyPred> &d, // The data
		     const int &l, // The max length of the logical chain
		     const vector<Clause> &fc) // The feet available clauses
     : question(q), data(d), L(l), feetClauses(fc)
{
     noise=1; // Default noise
     
     predicatesData.resize(L);
     predicatesQuestions.resize(L);
     predicatesData.front() = data;
     predicatesQuestions.back().resize(1);
     predicatesQuestions.back().at(0) = question;

     isIdOnly.resize(L);
     clausesNum.resize(L);
     numNotId=0;
     int incr;
     for(incr=0; incr < L; incr++) {
	  isIdOnly.at(incr)= true;
	  clausesNum.at(incr)= 0;
     }

     initClauses();
     sort(feetClauses.begin(), feetClauses.end(), compare_clauses);
     //updatePath();

     //std::cout << "Logic Path initialization complete." << std::endl;
}

void LogicPath::printAll()
// Prints the path on screen
{
     vector<vector<FuzzyPred> >::const_iterator dataIter;
     vector<vector<FuzzyPred> >::const_iterator endData= predicatesData.end();
     vector<vector<FuzzyPred> >::const_iterator questionIter;
     vector<vector<FuzzyPred> >::const_iterator endQuestion= predicatesQuestions.end();
     vector<Clause>::const_iterator clausesIter;
     vector<Clause>::const_iterator endClauses= clauses.end(); 
     vector< FuzzyPred >::const_iterator dIter;
     vector< FuzzyPred >::const_iterator endD;
     vector< FuzzyPred >::const_iterator qIter;
     vector< FuzzyPred >::const_iterator endQ;
     
     std::cout  <<  std::endl;
     std::cout  << "Printing situation" << std::endl;
     int m;

     for( dataIter= predicatesData.begin(), 
	       questionIter= predicatesQuestions.begin(),
	       clausesIter= clauses.begin(),
	       m=0; 
	  dataIter != endData; 
	  ++dataIter, ++questionIter, ++clausesIter , ++m) 
	  {
	       std::cout  << "Printing Data" << std::endl;
	       endD= dataIter->end();
	       for(dIter=dataIter->begin(); dIter!=endD; ++dIter) 
		    std::cout  << (*dIter) << std::endl;
	       std::cout  <<  std::endl;
	       std::cout  << "Printing clauses" << std::endl;
	       if( isIdOnly.at(m)) 
		    std::cout << "Id"  << std::endl;
	       else {
		    clausesIter->print();
		    std::cout << clausesIter->getUpg() << std::endl;
		    std::cout << clausesIter->pos() << std::endl;
	       }
	       std::cout  <<  std::endl;
	       std::cout  << "Printing questions" << std::endl;
	       endQ= questionIter->end();      
	       for(qIter=questionIter->begin(); qIter!=endQ; ++qIter) 
		    std::cout  << (*qIter) << std::endl;
	       std::cout  <<  std::endl;
	  }
     std::cout  <<  std::endl;
     std::cout  << "lastdata: "<< std::endl;
     print_vector(lastData);
     std::cout  <<  std::endl;
}

static vector<FuzzyPred> get_next_data(const vector< FuzzyPred > &data,
				const vector< FuzzyPred > &hyp,
				const FuzzyPred & cons,
				const int &pos)
{
     vector< FuzzyPred > ret_vect;
     vector< FuzzyPred >::const_iterator d2Iter= data.begin();
     vector< FuzzyPred >::const_iterator endD2=  data.end();  
     vector< FuzzyPred >::const_iterator hypIter= hyp.begin();
     vector< FuzzyPred >::const_iterator endHyp=  hyp.end();  

     for (;d2Iter!=data.begin()+pos; ++d2Iter) 
	  ret_vect.push_back(*d2Iter);
     ret_vect.push_back(cons);
     for (;d2Iter!=endD2; ++d2Iter) {
	  if( hypIter != endHyp && d2Iter->canGenUnify(*hypIter) ) 
	       ++hypIter;
	  else
	       ret_vect.push_back(*d2Iter);
     }	           
     return ret_vect;
}
static vector<FuzzyPred> remove_connected(const vector< FuzzyPred > &data,
				   const vector< FuzzyPred > &hyp,
				   const FuzzyPred & cons,
				   const int &pos)
{
     vector< FuzzyPred > ret_vect;
     vector< FuzzyPred >::const_iterator d2Iter= data.begin()+pos;
     vector< FuzzyPred >::const_iterator endD2=  data.end();  
     vector< FuzzyPred >::const_iterator hypIter= hyp.begin();
     vector< FuzzyPred >::const_iterator endHyp=  hyp.end();  

     for (;hypIter != endHyp && d2Iter != endD2; ++d2Iter, ++hypIter) {	  
	  ret_vect.push_back(*hypIter);
     }	           
     return ret_vect;
}

void LogicPath::apply_single_clause(const Clause &clause)
{
     clauses.at(0)= clause;
     updateAll();
}


void LogicPath::updateAll()
// Update the Id clauses
{
     vector<vector<FuzzyPred> >::iterator dataIter;
     vector<vector<FuzzyPred> >::iterator endData= predicatesData.end();
     vector<vector<FuzzyPred> >::iterator qIter;
     vector<vector<FuzzyPred> >::iterator beginQ= predicatesQuestions.begin();
     vector< FuzzyPred >::iterator fIter;
     vector< FuzzyPred >::iterator hIter;
     vector< FuzzyPred >::iterator endH;
     vector< FuzzyPred >::iterator beginD2;
     vector< FuzzyPred >::iterator d2Iter;
     vector< FuzzyPred >::iterator endD2;
     vector< FuzzyPred >::iterator q2Iter;
     vector< FuzzyPred >::iterator endQ2;

     int m,pos;
     vector<FuzzyPred> hyp;
     genUpg upg;
     FuzzyPred cons;
     Clause tmpClause;
     double weigth;
     bool cons_inserted;
     for(dataIter= predicatesData.begin(), m=0; dataIter != endData; ++dataIter, ++m) {
	  if(isIdOnly.at(m)) {
	       if(boost::next(dataIter) != predicatesData.end()) {
		    *boost::next(dataIter)= *(dataIter);
	       }
	       else { // If this is the last data
		    lastData= *dataIter;
	       }
	       // if(boost::next(dataIter) != predicatesData.end())
	       // 	    *boost::next(dataIter)= *(dataIter);
	  }
	  // If it is not Id, propagate the data
	  ////// Should be in the policies !!!
	  else {
	       upg.clear();
	       weigth= policy.canInsert(predicatesData.at(m),clauses.at(m),&upg,&pos);
	       clauses.at(m).pos()= pos;
	       //clauses.at(m).setUpg(upg);
	       vector<FuzzyPred> add_next;
	       if(weigth > 0) {
		    tmpClause= clauses.at(m);
		    hyp= tmpClause.getHypothesis();
		    hyp/upg;
		    cons= tmpClause.getConsequence();
		    //cons.setWeigth(tmpClause.getWeigth()); // the clause propagates forward the weigth		    
		    cons.setWeigth(-1); // Avoids recursion
		    cons/upg;
		    add_next = get_next_data(*dataIter,hyp,cons,pos);
	       }
	       else {
		    isIdOnly.at(m)=true;
		    add_next= *dataIter;
	       }
	       vector<FuzzyPred>::iterator nextIter= add_next.begin();
	       vector<FuzzyPred>::iterator nextEnd= add_next.end();
	       if(boost::next(dataIter) != predicatesData.end()) {
		    for (; nextIter != nextEnd; ++nextIter) 
			 boost::next(dataIter)->push_back(*nextIter);
	       }
	       else { // If this is the last data
		    for (; nextIter != nextEnd; ++nextIter) 
			 lastData.push_back(*nextIter);
	       }
	  }
     }
}

bool LogicPath::finalValues()
// Compute final Upg, finding the clause that fits
{
     vector<FuzzyPred>::iterator dIter= lastData.begin();
     vector<FuzzyPred>::iterator endD= lastData.end();


     /// Unify consequence before testing to unify for questions
     FuzzyPred tmpPred;
     FuzzyPred questionTmp;
     genUpg firstUpg, finalUpg;
     double finalWeigth;
     finalUpgs.clear();
     finalWeigths.clear();
     for(;dIter != endD ; ++ dIter) {
	  questionTmp = question;
	  tmpPred = *dIter;
	  //	  questionTmp.unique(tmpPred.unique(0));
	  firstUpg.clear();
	  if( questionTmp.canGenUnify(tmpPred) ) {
	       questionTmp.genUnify(tmpPred, &firstUpg);
	       questionTmp/firstUpg;
	       if( !questionTmp.hasEtiquette() ) {
		    finalWeigth= dIter->getWeigth();
		    finalUpg.clear();
		    //		    question.unique(tmpPred.unique(0));
		    question.genUnify(tmpPred, &finalUpg);
		    finalUpgs.push_back(finalUpg);
		    finalWeigths.push_back(finalWeigth);
		    //		    ++numSteps;
	       }
	  }
     }
     if(finalUpgs.size() == 0)
	  return false;
     return true;
}

bool LogicPath::updatePath()
// Returns true if a new estimate for the final question is made.
{
     //     printAll(); /// By using printAll in this place you can see the columns of spurious terms
     int incr;
     for(incr=0; incr < L; incr++) {
	  predicatesData.at(incr).clear();
	  predicatesQuestions.at(incr).clear();    
     }
     predicatesData.front() = data;
     predicatesQuestions.back().resize(1);
     predicatesQuestions.back().at(0) = question;
     lastData.clear();

     updateAll();
     //printAll(); 
     
     return finalValues();
}

void LogicPath::changeFeetClause(int m) 
// Returns true if it has done something
{
     pair<Clause,double> to_add;
     
     try { 
	  to_add= getFeetClauseToAdd(m); 
     }
     catch (std::runtime_error &) { 
	  return; 
     }
     double weigth_new= to_add.second, ran;
     double weigth_old= clauses.at(m).getWeigth()* clauses.at(m).getHypothesis().size();
     Clause clause= to_add.first;
     ran= dran(); 
     //if( ran < weigth_old/weigth_new ) { /// Check the correct weigth
     if( ran < exp(weigth_old)/exp(weigth_new) ) { /// Check the correct weigth
	  isIdOnly.at(m)= false;
	  numNotId++;	 
	  clause.uniVal( -(m+1) );
	  clauses.at(m)= clause;
     }
}

void LogicPath::addFeetClause(int m) 
// Returns true if it has done something
{
     pair<Clause,double> to_add;
     int n=0;
     while(n++ < 100) {
	  //while(true) {
	  try { 
	       to_add= getFeetClauseToAdd(m); 
	  }
	  catch (std::runtime_error &) { 
	       return; 
	  }
	  double weigth= to_add.second,
	       ran;
	  Clause clause= to_add.first;
     
	  //	  std::cout << "add: " << weigth << std::endl;
	  ran= dran(); 
	  if( ran < weigth ) { /// Check the correct weigth
	       isIdOnly.at(m)= false;
	       numNotId++;	 
	       clause.uniVal( -(m+1) );
	       clauses.at(m)= clause;
	       return;
	  }
     }     
}


double LogicPath::getTotalProbability()
{
     double wtot=1;
     for(int n=0; n< L; n++) {
	  if(!isIdOnly.at(n))
	       wtot*= clauses.at(n).getWeigth();
     }
     return wtot;
}

void LogicPath::addFeetClause(int m, Clause clause)
// Returns true if it has done something
{
     isIdOnly.at(m)= false;
     numNotId++;	 
     clause.uniVal( -(m+1) );
     clauses.at(m)= clause;
}


void LogicPath::removeClause(int m)
// This function removes the clause at (m,n) and substitutes it with
// identity clauses.
// Returns true if it has done something
{
     int n;
     double weigth, ran;
     genUpg upg;    
     int pos;
     //weigth= clauses.at(m).getWeigth()* log( 0.1 + clauses.at(m).getHypothesis().size() );
     //weigth= -log(clauses.at(m).getWeigth())* clauses.at(m).getHypothesis().size() ;
     int size= clauses.at(m).getHypothesis().size();     
     weigth= clauses.at(m).getWeigth()* exp( size );
     ran= dran();
     if( (weigth !=0 && ran < 1./weigth )
	 || weigth == 0 ) {
	  isIdOnly.at(m)= true;
	  --numNotId;
	  clausesNum.at(m)=0;
     }      
}

/// [not here] check if the substitution clause is Id

int LogicPath::getLength()
{
     return L;
}


pair<Clause,double> LogicPath::getFeetClauseToAdd(const int &m)
// Return the clause to add at the level m
{
     vector< Clause >::iterator citer;
     vector< Clause >::iterator cend;
     Clause to_select;          

     citer= feetClauses.begin()+clausesNum.at(m);
     cend= feetClauses.end();
     double weigth;
     double wvect=0;

     for (; citer != cend; ++citer, ++clausesNum.at(m)) {
     	  Clause cl= *citer;
     	  cl.uniVal( -(m+1) );
     	  weigth= policy.canInsert_fast(predicatesData.at(m), cl);	  
	  if(weigth != 0) {
	       to_select= *citer;
	       wvect= weigth;
	       break;
	  }
     }
     if(wvect == 0)
     	  throw std::runtime_error("no match");
     return std::make_pair(to_select,wvect);
}  

void LogicPath::initRandom()
{
     int c;
     if(feetClauses.size() > 0)
	  for(int m=0; m < L; ++m) {
	       c= dran()*feetClauses.size();
	       Clause clause= feetClauses.at(c);
	       clause.uniVal( -(m+1) );
	       clauses.at(m)= clause;
	  }
}

int LogicPath::getClauseToRemove()
// Return the clause number to change when removing
{
     if(numNotId == 0)
	  return -1;
     int m;
     for(;;) {
	  m = dran() *  L ;
	  if(!isIdOnly.at(m))
	       return m;
     }
}
bool LogicPath::isId(const int &m)
{
     if(m>=0 && m < L)
	  return isIdOnly.at(m);
     return false;
}

