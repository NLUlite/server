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



#ifndef __PATH_MEMORY__
#define __PATH_MEMORY__

#include<iostream>
#include<fstream>
#include<boost/utility.hpp>
#include<boost/tuple/tuple.hpp>
//#include<tuple>
#include<vector>
#include<map>
#include<string>
#include<algorithm>
#include<stdexcept>
#include"../drt/DrtPred.hpp"
#include"../drt/drt.hpp"
#include"../engine/Engine.hpp"
#include"clause_vector.hpp"

using std::cout;
using std::endl;
using std::map;
using std::vector;
using std::pair;
using std::make_pair;
using std::string;

//using boost::tuple;
//using boost::make_tuple;


class path_memory {
     vector<drt> matching_drt_;
     vector<boost::tuple<vector<clause_vector>,DrtMgu,double> > memory_;
     DrtMgu last_upg_;
     double last_weigth_;
     bool is_closed_;
     int n;
     double weigth_;
     DrtMgu first_upg_;
     vector< vector<clause_vector> > clause_history_;

public:     
     path_memory()
     {
	  n=0;
	  last_weigth_=1;
	  is_closed_= false;
     }
     void clear()
     {
	  matching_drt_.clear();
          memory_.clear();
          last_upg_.clear();
          n=0;
          is_closed_=false;
     }
     void setDrt(const vector<drt> &d) {matching_drt_= d;}
     vector<drt> getDrt() {return matching_drt_;}
     void push(const vector<clause_vector> &cv, const DrtMgu &upg, double d) 
     {
	  if(!is_closed_) {
	       ++n;
	       memory_.push_back(boost::make_tuple(cv,upg,d));
	       weigth_=1;
	       for(int m=memory_.size()-1; m >= 0 ; --m) {
		    double w_tmp= memory_.at(m).get<2>();
		    weigth_ *= w_tmp;
	       }
	  }
     }
     void last_upg(const DrtMgu &upg)
     {
	  if(!is_closed_)
	       last_upg_= upg;
     }
     void last_weigth(double w)
     {
	  if(!is_closed_)
	       last_weigth_= w;
     }
     void close();
     bool is_closed() const
     {
	  return is_closed_;
     }
     void set_to_last()
     {
	  n= memory_.size()-1;
     }
     void set_to_first()
     {
	  n= 0;
     }
     boost::tuple<vector<clause_vector>,DrtMgu,double> pop()
     {
	  if(is_closed_) {
	       --n;
	       return memory_.at(n);
	  }
	  return boost::tuple<vector<clause_vector>,DrtMgu,double>();
     }
     vector< vector<clause_vector> > get_clause_history() const
     {
	  return clause_history_;
     }
     DrtMgu get_first_upg() const
     {
	  return first_upg_;
     }
     DrtMgu get_last_upg() const
     {
	  return last_upg_;
     }
     double get_total_weigth() const
     {
	  return weigth_;
     }     
     
     vector<boost::tuple<vector<clause_vector>,DrtMgu,double> > get_memory() {return memory_;}

     int getDepth() const {return n;}
};


#endif // __PATH_MEMORY__
