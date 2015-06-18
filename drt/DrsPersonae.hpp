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



#ifndef __PERSONAE__
#define __PERSONAE__

#include<iostream>
#include<fstream>
#include<cstdlib> // for posix_memalign()
#include<boost/functional/hash.hpp>
#include<boost/utility.hpp>
#include<boost/lexical_cast.hpp>
#include<boost/utility.hpp>
#include<boost/shared_ptr.hpp>
#include<functional>
#include<algorithm>
#include<stdexcept>
#include<vector>
//#include<forward_list>
#include<map>
#include"DrtPred.hpp"
#include"drt.hpp"
#include"metric_singleton.hpp"
#include"../fopl/fopl.hpp"
#include"complements.hpp"
#include"../engine/Engine.hpp"
#include<boost/unordered_map.hpp>
//#include<google/sparse_hash_map>
//#include<google/dense_hash_map>
#include "../google-libs/cpp-btree-1.0.1/btree_map.h"


using std::string;
using std::vector;
using std::map;
using std::pair;
using std::make_pair;
using std::cout;
using std::endl;
//using boost::tuple;
//using boost::make_tuple;
//using boost::unordered_map;
//using google::sparse_hash_map;
//using google::dense_hash_map;

//typedef boost::unordered_map<string,vector<string> > CMap;
typedef btree::btree_map<string,vector<string> > CMap;
//typedef map<string,vector<string> > CMap;

const int max_action_size = 50000; // the maximum amount of actions in each Persona

class Action {
     string link_;
     string text_;

     string ref_;
     string verb_;

     CMap complements_;
     
     string drs_form_;
     CodePred code_;
     bool is_subordinate_;

public:
     Action() : is_subordinate_(false) {}
     Action(const string &str, const string &link, const string &text, const vector<DrtPred> &drs);
     void addVerb(const string &str) { verb_= str; }
     
     void addComplement(const DrtVect &dvect );
     
     vector< DrtVect > getSpecificComplement(const string &name) const;
     vector< DrtVect > getComplements() const;
     string getVerb() const { return verb_; }
     string getRef() const { return ref_; }
     vector<DrtPred> getDrs() const;
     string getLink() const { return link_; }
     string getText() const { return text_; }

     void setCode(const CodePred &cp) {code_= cp;}
     CodePred getCode() {return code_;}

     bool isSubordinate() {return is_subordinate_;}
     void setSubordinate(bool value) {is_subordinate_ = value;}

     bool operator == (const Action &rhs);
};

struct RefHasher
{
	std::size_t operator()(const string& str) const;
};

//typedef dense_hash_map<string, boost::shared_ptr<Action>, std::tr1::hash<string> > MapStActionPtr;
//typedef sparse_hash_map<string, boost::shared_ptr<Action>, std::tr1::hash<string> > MapStActionPtr;
typedef btree::btree_map<string, boost::shared_ptr<Action> > MapStActionPtr;
//typedef boost::unordered_map<string, boost::shared_ptr<Action>, RefHasher  > MapStActionPtr;
//typedef map<string, boost::shared_ptr<Action> > MapStActionPtr;

//typedef btree::btree_map<string, Action* > MapStActionPtr;

class Persona {
	int num_actions_;
     string reference_;
     vector<string> names_;
     vector<DrtPred> pred_names_;
     //std::forward_list<Action *> actions_;
     //std::forward_list<boost::shared_ptr<Action> > actions_;
     std::vector<boost::shared_ptr<Action> > actions_;
     //boost::shared_ptr<Action> actions_[max_action_size];

     MapStActionPtr verb_to_actions_;
     MapStActionPtr ref_to_actions_;

     vector<DrtVect> specifications_;
     // It contains specifications like the man 'in the kitchen'.

public:

     Persona();
     ~Persona();

     bool hasSpecification();

     void setReference(const string &s) {reference_=s;}
     void setNames(const vector<string> &vs) {names_= vs;}
     //void addName(const string &s);
     void addPred(const DrtPred &p) {pred_names_.push_back(p);}
     boost::shared_ptr<Action> addAction(const Action &a);
     void addSpecification(const vector<DrtPred> &pred) { specifications_.push_back(pred); }

     vector<string> getNames() const;// {return names_;}
     vector<DrtPred> getPreds() const;// {return pred_names_;}
     std::vector<boost::shared_ptr<Action> > getActions() const {return actions_;}
     //std::forward_list<boost::shared_ptr<Action> > getActions() const {return actions_;}
     //std::forward_list<Action*> getActions() const {return actions_;}
     Action getActionFromVerbName(const string &ref);
     Action getActionFromVerbRef(const string &ref);
     vector<DrtVect> getSpecifications();
     string getReference() const {return reference_;}

     void addPersona(const Persona &persona);
     void sort();

     //void clearActions() { actions_.clear(); }
     void clearActions() { vector<boost::shared_ptr<Action> >().swap(actions_); }
};

typedef boost::shared_ptr<Action> SPAction;

//typedef map<string,Persona > MapStPers;
//typedef map<string,vector<DrtPred> > MapStVDPred;
//typedef map<string,vector<string> > PMapStVSt;
//typedef map<string, vector<tuple<DrtPred, string, string> > > MapStTuple;
//typedef map< string, vector<SPAction> > MapStBool; // a specific ref can be subj or obj in a specific Action

typedef boost::unordered_map<string,Persona, RefHasher> MapStPers;
//typedef boost::unordered_map<string,vector<DrtPred>, RefHasher > MapStVDPred;
//typedef boost::unordered_map<string,vector<string>, RefHasher > PMapStVSt;
//typedef boost::unordered_map<string, vector<boost::tuple<DrtPred, string, string> >, RefHasher > MapStTuple;
//typedef boost::unordered_map< string, vector<SPAction>, RefHasher > MapStBool; // a specific ref can be subj or obj in a specific Action

//typedef btree::btree_map<string,Persona> MapStPers;
typedef btree::btree_map<string,vector<DrtPred> > MapStVDPred;
typedef btree::btree_map<string,vector<string> > PMapStVSt;
typedef btree::btree_map<string, vector<boost::tuple<DrtPred, string, string> > > MapStTuple;
typedef btree::btree_map< string, vector<SPAction> > MapStBool; // a specific ref can be subj or obj in a specific Action

//typedef sparse_hash_map<string,Persona> MapStPers;
//typedef sparse_hash_map<string,vector<DrtPred> > MapStVDPred;
//typedef sparse_hash_map<string,vector<string> > PMapStVSt;
//typedef sparse_hash_map<string, vector<boost::tuple<DrtPred, string, string> > > MapStTuple;

//typedef dense_hash_map<string,Persona> MapStPers;
//typedef sparse_hash_map<string,vector<DrtPred> > MapStVDPred;
//typedef dense_hash_map<string,vector<string> > PMapStVSt;
//typedef sparse_hash_map<string, vector<boost::tuple<DrtPred, string, string> > > MapStTuple;



class DrsPersonae {
     
     string link_;
     vector<string> texts_;
     vector<DrtVect> preds_;
     vector<CodePred> codes_;

     vector<string> references_;
     MapStPers personae_;  // maps a the reference of a name to the persona
     PMapStVSt persona_pointer_; // all the references are mapped to
				 // the reference of the verb or the
				 // subject (mapped to the action)
     MapStBool subj_refs_, obj_refs_, name_refs_;
     MapStVDPred adverbs_;
     
     PMapStVSt verb_names_;     // maps references to verb names;
     MapStVDPred verb_preds_;     // maps references to verb predicates;
     PMapStVSt names_to_refs_;  // maps names to references;

     MapStTuple subord_verbs_;
     // maps verb_references to subordinates. The tuple is < @SUB-TYPE, persona-ref, verb-ref >.
     // The map's key is the parent verb's ref.

     SPAction addAction(const DrtVect &drtvect, const DrtPred &pred, int m, const string &text, const CodePred &code, string ref, string oref, SPAction pa_main);
     // Auxiliary method for this->compute()

public:

     DrsPersonae();
     DrsPersonae(const vector<DrtVect> p, const vector<string> &all_texts, const string &link);
     DrsPersonae(const vector<drt> &drt_list, const string &link);
     void compute();
     
     MapStPers getPersonae() const { return personae_; }
     Persona getPersona(const string &ref) const;
     vector<string> getReferences() const { return references_; }
     vector<string> mapRefToActionRef(const string &ref) const;
     vector<DrtVect> getPredicates() const { return preds_; }
     vector<pair<DrtPred,Action> > getSubordinates(const string &verb_ref) const;

     void addPersonae(const DrsPersonae &dp);
     void addReferences(DrsPersonae dp);
     void setLink(const string &link) { link_=link; }
     void setText(const vector<string> &text) { texts_=text; }
     void print(std::ostream &out);

     vector<string>  getVerbNames  (const string &ref)  const;
     vector<DrtPred> getVerbPreds  (const string &ref)  const;
     vector<string>  getRefFromName(const string &name) const;     
     vector<DrtPred> getAdverbs    (const string &ref)  const;     

     bool refIsSubj(const string &str) const;
     bool refIsObj (const string &str) const;

     vector<SPAction> getSubjActions(const string &str) const;
     vector<SPAction> getObjActions(const string &str) const;
     vector<SPAction> getNameActions(const string &str) const;

     void clear();
     void clearUseless();
     void sort();

     MapStPers::iterator begin() {return personae_.begin();}
     MapStPers::iterator end() {return personae_.end();}
};

std::ostream &operator << (std::ostream &, const SPAction &);

#endif //__PERSONAE__
