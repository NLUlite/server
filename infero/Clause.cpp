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



#include"Clause.hpp"

using std::string;

std::ostream & operator << (std::ostream &out, const Clause& c)
{
     c.print_stream(out);
     return out;
}

Clause::Clause(const string &s)
{
     string str= s;
     boost::erase_all(str, string(" ") ); // strip all the spaces from
     // the string
     vector<string> strs;
     boost::split(strs,str, boost::is_any_of(":") );
     if(strs.size() != 2)
	  throw(std::invalid_argument("Badly formed clause."));
     strs.at(1).erase(0,1); // erase the '-'

     consequence= FuzzyPred(strs.at(0));



     str= strs.at(1);
     strs.clear();
     int p, size= str.size();
     int depth = 0;
     int p1= 0, p2;
     for(p=0; p < size; ++p) {
	  if(str.at(p) == '(' )
	       ++depth;
	  if(str.at(p) == ')')
	       --depth;
	  if(depth < 0)
	       throw(std::invalid_argument("Badly formed clause."));
	  if(str.at(p) == ',' && depth == 0) {
	       p2= p;
	       strs.push_back( str.substr(p1,p2-p1) );
	       p1=p2+1;
	  }
     }
     strs.push_back( str.substr(p1,p-p1) );

     vector<string>::iterator si=strs.begin();
     vector<string>::iterator se=strs.end();
     hypothesis.resize( strs.size() );
     vector<FuzzyPred>::iterator hi=hypothesis.begin();
     for(;si != se; ++si, ++hi) {
	  //      std::cout << (*si) << std::endl;
	  *hi= FuzzyPred(*si);      
     }

     setWeigth(1);
}

void Clause::operator/(const genUpg &rhs)
{
     upg.clear();
     upg= rhs;
     consequence/upg;
     hypothesis/upg;
}

void Clause::setUpg(const genUpg &rhs)
{
     upg.clear();
     upg= rhs;
}
void Clause::uniVal(const int &value)
{
     consequence.uniVal(value);
     vector<FuzzyPred>::iterator si=hypothesis.begin();
     vector<FuzzyPred>::iterator se=hypothesis.end();
     for(;si != se; ++si)
	  si->uniVal(value);
}

void Clause::print() const
{
     vector<FuzzyPred>::const_iterator hypIter;
     vector<FuzzyPred>::const_iterator endHyp= hypothesis.end();

     std::cout << consequence << " :- " ;
     for(hypIter= hypothesis.begin(); hypIter != endHyp; ++hypIter ) {
	  std::cout << *hypIter;
	  if(boost::next(hypIter) != endHyp )
	       std::cout << ", ";
     }
     std::cout << "." << std::endl;
}

void Clause::print_stream(std::ostream &out) const
{
     vector<FuzzyPred>::const_iterator hypIter;
     vector<FuzzyPred>::const_iterator endHyp= hypothesis.end();

     out << consequence << " :- " ;
     for(hypIter= hypothesis.begin(); hypIter != endHyp; ++hypIter ) {
	  out << *hypIter;
	  if(boost::next(hypIter) != endHyp )
	       out << ", ";
     }
     //  out << ".";
}
