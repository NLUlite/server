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



#ifndef _FOPL_
#define _FOPL_

#include<iostream>
#include<list>
#include<string>
#include<vector>
#include<utility>
#include<stdexcept>
#include<boost/lexical_cast.hpp>
#include"../tree/mytree.hpp"

#define MIN(a,b) ((a<b)?a:b)

class Upg;
class PTEl;
struct Link;

struct PTEl
{
     std::string str;
     int uniVal; // Unification value, it has different values
                 // for different predicates (to avoid recursion
                 // in unification)
     ~PTEl();
     PTEl(): uniVal(0), str("") {}
     PTEl(const std::string &s);
     explicit PTEl(const char *s);
     bool operator == (const PTEl &rhs) const {return (str == rhs.str && uniVal == rhs.uniVal);}
     bool operator != (const PTEl &rhs) const {return (str != rhs.str || uniVal != rhs.uniVal);}
     bool operator < (const PTEl &rhs) const {return str < rhs.str;}
     //{return (uniVal < rhs.uniVal)}
     unsigned size() const{return str.size();}
     bool uniQuant() const {return uniVal>0;}
     bool genUniQuant() const {return uniVal<0;}
     // bool isEtiquette() const 
     // {
     // 	  if(str.at(0) == '_')
     // 	       return true;
     // 	  return false;
     // 	  /// Not done yet
     // }
};

class genUpg;

class PredTree: public FTree<PTEl>
{
public:
     PredTree(): FTree<PTEl>(PTEl("")) {}
     PredTree(const PredTree::iterator &i): FTree<PTEl>(i) {}
     PredTree(const PredTree &pt): FTree<PTEl>(pt) {}
     PredTree(const std::string &str): FTree<PTEl>(PTEl(str)) {}
     PredTree(const PTEl &ptel): FTree<PTEl>(ptel) {}
     void operator / (const Upg &upg);
     void operator / (const genUpg &genupg);
     void operator / (const Link &);
     bool operator < (const PredTree &) const;
     void substFrom(const Link &, const PredTree::iterator &);
     void uniVal(const int &);
};

void printLikeTree(const PredTree &, const std::string& = "");
void printLikePred(const PredTree &,
		   const std::string& = "",
		   std::ostream & = std::cerr,
		   const bool & = true);

struct Link
{
     PTEl first;
     PredTree second;
     Link() {}
     Link(const PTEl &i, const PredTree &o): first(i), second(o) {}
     bool operator < (const Link &rhs) const 
     {
	  return // (first < rhs.first)  ||
	         (second < rhs.second);
     }
};

class Upg : public std::list<Link>
{
public:
     Upg() {}
     void add(const PTEl &in, const PredTree &out) {push_back(Link(in, out));}
     void add(const Upg &u);
     void addWithoutUnification(const Upg &u);
     void uniVal(int num);
     void addReverse(const Upg &u);
     void operator /(const Link &);
     void operator /(const Upg &);
     bool operator <(const Upg &) const;
};

struct genLink
{
     PTEl first;
     PTEl header;
     PredTree second;
     genLink(){}
     genLink(const PTEl &i, const PTEl &h, const PredTree &o): first(i), header(h), second(o) {}
     bool operator < (const genLink &rhs) const 
     {
	  return // (first < rhs.first)  ||
	         (header < rhs.header);
     }
};

class genUpg : public std::list<genLink>
{
public:
     genUpg() {}
     void add(const PTEl &in, const PTEl &header, const PredTree &out) {push_back(genLink(in, header, out));}
     //void operator /(const genLink &);
     bool operator <(const genUpg &) const;
};


class Predicate
{
protected:
     PredTree predicate;
     PredTree readLiteral(const std::string &);

public:
     Predicate();
     Predicate(const std::string &);
     Predicate(const PredTree &);
     virtual ~Predicate();

     void readLikeLisp(std::string);
     
     void operator / (const Upg &upg);

     void operator / (const genUpg &genupg)
     {
	  predicate / genupg;
     }
  
     friend void operator / (std::vector<Predicate> &predVect, const Upg &upg);
  
     int unique(const int&);
     bool hasEtiquette() const;
     bool unify(const Predicate&, Upg*) const;
     bool unify(const Predicate&);
     bool genUnify(const Predicate&, genUpg*) const;
     bool genUnify(const Predicate&);
     bool uniQuant() const {return predicate.begin()->uniQuant();};
     //void operator = (const Predicate & rhs) {predicate = rhs.predicate;}
     bool operator == (const Predicate &rhs) const {return predicate == rhs.predicate;}
     bool operator != (const Predicate &rhs) const {return !(predicate == rhs.predicate);}
     virtual bool canUnify(const Predicate&) const;
     virtual bool canGenUnify(const Predicate&) const;
     void print() const;
     PredTree pred() const {return predicate;}
     PredTree& pred() {return predicate;}
     Predicate deUnique() const;
     void uniVal(const int &value) {predicate.uniVal(value);}
     bool operator < (const Predicate &rhs) const
     {
	  return predicate < rhs.predicate;
     }
     std::pair<std::string,int> getOrder() const
     {
	  return std::make_pair(predicate.begin()->str, predicate.begin().num_children() );
     }

     virtual bool equalAtoms(const PTEl &lhs, const PTEl &rhs) const;
};

std::ostream &operator << (std::ostream &, const Predicate&);
std::ostream &operator << (std::ostream &, const PTEl&);
std::ostream &operator << (std::ostream &out, const Upg& upg);
std::ostream &operator << (std::ostream &out, const genUpg& genupg);

#endif // _FOPL_
