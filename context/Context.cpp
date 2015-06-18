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



#include"Context.hpp"

const bool debug = false;

void Context::addPredicate(const DrtPred &p)
{     
//     if((p.is_verb() || p.is_name()) && !p.is_proper_name() && !p.is_article() && !p.is_WP() && !p.is_WDT() && !p.is_number() )
//	  map_preds_[p.tag()].push_back(p.pred().begin()->str);
     if(p.is_proper_name() && !p.is_verbatim()) {
          string head= extract_header(p);
          vector<string> strs;
          boost::split(strs,head,boost::is_any_of("_"));
          for(int n=0; n < strs.size(); ++n) {
	       if(debug) 
		    cout << "NNPCONTEXT::: " << strs.at(n) << endl;
               map_NNP_[strs.at(n)]= 1;
	  }
     }     
}


void Context::add(const DrtVect &d)
{
     for(int n=0; n < d.size(); ++n) {
          this->addPredicate(d.at(n));
     }
}

void Context::add(const Context &c)
{
     // add the verb names in dp
     if(map_preds_.size() ) {
	  map_preds_.insert(c.map_preds_.begin(), c.map_preds_.end());
     }
     else 
	  map_preds_= c.map_preds_;

     if(map_NNP_.size() ) {
	  map_NNP_.insert(c.map_NNP_.begin(), c.map_NNP_.end());
     }
     else 
	  map_NNP_= c.map_NNP_;
}

double Context::evaluateVerb(const DrtPred &p)
{
     double to_return=0;

     return to_return;
}

double Context::evaluateName(const DrtPred &p)
{
     double to_return=0;

     return to_return;
}

double Context::evaluateProperName(const DrtPred &p)
{
     double to_return=0;

     return to_return;
}

double Context::evaluateVerb(const string &p)
{
     double to_return=0;

     return to_return;
}

double Context::evaluateName(const string &p)
{
     double to_return=0;

     return to_return;
}

static bool contained_NNP(const string &lhs, const string &rhs)
{
     vector<string> lhs_strs;
     boost::split(lhs_strs, lhs, boost::is_any_of("_"));
     vector<string> rhs_strs;
     boost::split(rhs_strs, rhs, boost::is_any_of("_"));

     for (int n = 0; n < lhs_strs.size(); ++n) {
          if (find(rhs_strs.begin(), rhs_strs.end(), lhs_strs.at(n)) == rhs_strs.end())
               return false;
     }
     return true;
}

double Context::evaluateProperName(const string &str)
{
     double to_return=0;                    
     vector<string> strs;
     boost::split(strs, str, boost::is_any_of("_"));
     for (int n = 0; n < strs.size(); ++n) {
          unordered_map<string, int >::iterator miter = map_NNP_.find(strs.at(n));
	  if(debug) 
	       cout << "NNPCONTEXT3::: " << strs.at(n) << endl;
          if (miter != map_NNP_.end()) {
	       if(debug) 
		    cout << "NNPCONTEXT2::: " << strs.at(n) << endl;	       
               to_return= 1;
               break;
          }
     }
     
          
//     vector<string> name_preds= map_preds_["NNP"];
//     for(int n=0; n < name_preds.size(); ++n) {
//          string head = name_preds.at(n);
//          if (contained_NNP(str, head)) { //|| contained_NNP(head, str)) {
//               to_return = 1;
//               break;
//          }
//     }

     return to_return;
}



