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



#ifndef __CONSTITUENTS__
#define __CONSTITUENTS__

#include<iostream>
#include<algorithm>
#include<vector>
#include<map>
#include<string>
#include<fstream>
#include<boost/tuple/tuple.hpp>
#include<boost/utility.hpp>
#include<boost/algorithm/string.hpp>
#include<boost/lexical_cast.hpp>
#include"../infero/infero.hpp"
#include"metric.hpp"

using std::string;
using std::vector;
using std::map;
using boost::tuple;
using boost::make_tuple;
using std::pair;
using std::make_pair;


typedef pair<string,int> constit_element;
typedef pair<constit_element, FTree<constit_element> > clink;

class constituents_mgu : public std::list<pair<constit_element, FTree<constit_element> > > 
{
public:
     void add(const constit_element &in, const FTree<constit_element> &out) { push_back(make_pair(in, out)); }
     void operator /(const constituents_mgu &);
};


class constituents {
     FTree< constit_element > tree_;

public:
     constituents();
     constituents(const FTree< constit_element > &t) : tree_(t) {}
     constituents( string, int );
     constituents( const string &);
     constituents(const constituents &c, int);    
     
     void operator / (const constituents_mgu &);

     constituents add_left( string, int);
     constituents add_right( string, int );
     constituents add_left(const constituents &c, int);
     constituents add_right(const constituents &c, int);
     constituents add_left(const constituents &c);
     constituents add_right(const constituents &c);
     void erase() { tree_.erase( tree_.begin() ); }
     void print(std::ostream &out) const;
     void print_like_pred(std::ostream &out) const;
     constituents copy() {return *this;}
     FTree< constit_element > tree() const {return tree_;}
     FTree< constit_element >& tree() {return tree_;}
     //FTree< constit_element >& tree() {return tree_;}
     bool can_unify(const constituents &rhs) const; 
     bool unify(const constituents &rhs, constituents_mgu *retUpg) const; 
     bool can_co_unify(const constituents &rhs, int pos1, int pos2, constituents *constit, bool left_nish, bool right_nish);
          
     //void operator = (constituents rhs)  {tree_ = rhs.tree_; }

     bool operator == (const constituents &rhs) const {return rhs.tree_ == tree_;}
     bool operator != (const constituents &rhs) const {return ! (rhs.tree_ == tree_) ;}
     
};


class composition_tree : public FTree<constituents> {
public:
     composition_tree();
     ~composition_tree();
     composition_tree(const PredTree::iterator &);
};

class composition {
     composition_tree ctree_;

     FTree<constituents>::iterator find_head(FTree<constituents>::iterator&) const;
     int count_from_start(FTree<constituents>::iterator &unif_iter) const;

public:
     composition() {}
     composition(const composition_tree &ct) : ctree_(ct) {}

     composition_tree tree() const {return ctree_;}
     vector<pair<pair<int,int>,constituents> > get_connections(const PredTree &binary);

     int find_head(const constituents &) const;

     void print_like_tree(std::ostream &out) const;
     void print(std::ostream &out) const;
};



std::ostream & operator << (std::ostream &out, const constituents& pt);
std::ostream & operator << (std::ostream &out, const composition& pt);


#endif // __CONSTITUENTS__
