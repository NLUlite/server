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



#ifndef __RULES__
#define __RULES__

#include<iostream>
#include<fstream>
#include<boost/utility.hpp>
#include<boost/lexical_cast.hpp>
#include<boost/functional/hash.hpp>
#include<boost/utility.hpp>
#include<boost/lexical_cast.hpp>
#include<boost/utility.hpp>
#include<boost/tuple/tuple.hpp>
#include<boost/shared_ptr.hpp>
#include<algorithm>
#include<stdexcept>
#include<vector>
#include<map>
#include"DrtPred.hpp"
#include"metric_singleton.hpp"
#include"../fopl/fopl.hpp"
#include"../infero_vector/clause_vector.hpp"
#include"complements.hpp"
#include<boost/unordered_map.hpp>
//#include "../google-libs/cpp-btree-1.0.1/btree_map.h"

using std::string;
using std::vector;
using std::map;
using std::pair;
using std::make_pair;
using std::cout;
using std::endl;
using boost::tuple;
//using boost::make_tuple;
//using boost::unordered_map;


class ConditionalAction {
     string link_;
     string text_;

     string ref_;
     string verb_;
     vector<string> object_;

     //map<string,vector<pair<string,vector<string> > > > complements_; // these are only complements to the verb
     // "complements" is a map with @COMPLEMENT_NAME and with value a vector of <reference_objects_pointed_at, names_of_objects_pointed_at>
      
     map<string,vector<DrtVect> > complements_;
      
     clause_vector cv_;

public:
     ConditionalAction() {}
     ConditionalAction(const string &str, const string &link, const string &text, const clause_vector &cv) : ref_(str), text_(text), link_(link), cv_(cv) {}
     void addVerb(const string &str) { verb_= str; }
     void addObject(const vector<string> &cstr) { object_= cstr; }
     
     //void addComplement(const string &ctype, const string &ref, const vector<string> nouns) {complements_[ctype].push_back(make_pair(ref,nouns));}
     
     void addComplement(const DrtVect &dvect );
     //vector<pair<string,vector<string> > > getComplementPairs(const string &str) const;
     vector< DrtVect > getSpecificComplement(const string &name) const;
     vector< vector<DrtPred> > getComplements() const;
     string getVerb() const { return verb_; }
     string getRef() const { return ref_; }
     vector<string> getObject() const {return object_;}
     clause_vector getClause() const;
     vector<DrtPred> getCons() const;
     string getLink() const { return link_; }
     string getText() const { return text_; }

     bool operator == (const ConditionalAction &rhs);
};

typedef unordered_map<string, boost::shared_ptr<ConditionalAction> > MapStCondActionPtr;

class ConditionalPersona {
     string reference_;
     vector<string> names_;
     vector<DrtPred> pred_names_;
     vector<boost::shared_ptr<ConditionalAction> > actions_;

     MapStCondActionPtr verb_to_actions_;
     MapStCondActionPtr ref_to_actions_;

     vector<vector<DrtPred> > specifications_;
     // It contains specifications like the man 'in the kitchen'.

public:

     bool hasSpecification();

     void setReference(const string &s) {reference_=s;}
     void setNames(const vector<string> &vs) {names_= vs;}
     void addName(const string &s) {names_.push_back(s);}
     void addPred(const DrtPred &p) {pred_names_.push_back(p);}
     void addAction(const ConditionalAction &a);
     void addSpecification(const vector<DrtPred> &pred) { specifications_.push_back(pred); }

     vector<string> getNames() const {return names_;}
     vector<DrtPred> getPreds() const {return pred_names_;}
     vector<boost::shared_ptr<ConditionalAction> > getActions() const {return actions_;}
     ConditionalAction getActionFromVerbName(const string &ref);
     ConditionalAction getActionFromVerbRef(const string &ref);
     vector<vector<DrtPred> > getSpecifications();
     string getReference() const {return reference_;}

     void addConditionalPersona(const ConditionalPersona &persona);

     void clearActions() { actions_.clear(); }
     void sort();
};

typedef unordered_map<string,ConditionalPersona> CMapStPers;
typedef unordered_map<string,vector<DrtPred> > CMapStVDPred;
typedef unordered_map<string,vector<string> > CPMapStVSt;
typedef unordered_map<string, vector<boost::tuple<DrtPred, string, string> > > CMapStTuple;


typedef unordered_map<string, bool> CMapStBool;

//typedef btree::btree_map<string,ConditionalPersona> CMapStPers;
//typedef btree::btree_map<string,vector<DrtPred> > CMapStVDPred;
//typedef btree::btree_map<string,vector<string> > CPMapStVSt;
//typedef btree::btree_map<string, vector<boost::tuple<DrtPred, string, string> > > CMapStTuple;
//typedef btree::btree_map<string, bool> CMapStBool;


class Rules {
     
     string link_;
     vector<string> texts_;
     CMapStBool subj_refs_, obj_refs_;

     vector<string> references_;
     CMapStPers personae_;  // maps a the reference of a name to the persona
     vector<clause_vector> cv_;
     
     CPMapStVSt verb_names_;     // maps references to verb names;
     CPMapStVSt names_to_refs_;  // maps names to references;

     CMapStTuple subord_verbs_;
     // maps verb_references to subordinates. The tuple is < @SUB-TYPE, persona-ref, verb-ref >.
     // The map's key is the parent verb's ref.

     CPMapStVSt persona_pointer_;
     // all the references are mapped to
     // the reference of the verb or the
     // subject (mapped to the action)

public:

     Rules() {}
     Rules(const vector<clause_vector> &cv, const vector<string> &all_texts, const string &link) : cv_(cv), texts_(all_texts), link_(link) {}
     void compute();
     
     CMapStPers getRules() const { return personae_; }
     ConditionalPersona getConditionalPersona(const string &ref) const;
     vector<string> getReferences() const { return references_; }
     vector<clause_vector> getClauses() const { return cv_; }
     vector<pair<DrtPred,ConditionalAction> > getSubordinates(const string &verb_ref);

     void addRules(const Rules &dp);
     void addReferences(Rules dp);
     void setLink(const string &link) { link_=link; }
     void setText(const vector<string> &texts) { texts_=texts; }
     void print(std::ostream &out);

     vector<string> getVerbNames  (const string &ref)  const;
     vector<string> getRefFromName(const string &name) const;

     bool refIsObj(const string &str)  const;
     bool refIsSubj(const string &str) const;
     void clearUseless();
     void sort();
     vector<string> mapRefToActionRef(const string &ref) const;
};


#endif //__RULES__
