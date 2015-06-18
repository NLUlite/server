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



#include"Rules.hpp"

const bool debug = false;

template<class T>
static void print_vector(std::vector<T> &vs)
{
	if (vs.size()) {
		typename vector<T>::iterator tags_iter = vs.begin();
		while (tags_iter != vs.end()) {
			std::cout << (*tags_iter) << " ";
			++tags_iter;
		}
		std::cout << std::endl;
	}
}

static bool shortfind(const boost::unordered_map<string, bool> &mapp, const string &element)
{
	boost::unordered_map<string, bool>::const_iterator miter = mapp.find(element);
	if (miter != mapp.end())
		return true;
	return false;
}


bool ConditionalAction::operator ==(const ConditionalAction &rhs)
{
	if (link_ == rhs.link_ && text_ == rhs.text_ && ref_ == rhs.link_ && verb_ == rhs.verb_ && object_ == rhs.object_
			&& complements_ == rhs.complements_)
		return true;

	return false;
}

void ConditionalAction::addComplement(const DrtVect &dvect)
{
	string ctype = "";
	for (int n = 0; n < dvect.size(); ++n) {
		if (dvect.at(n).is_complement()) {
			ctype = extract_header(dvect.at(n));
			break;
		}
	}
	if (ctype == "")
		return;

	complements_[ctype].push_back(dvect);
}

vector<DrtVect> ConditionalAction::getSpecificComplement(const string &name) const
{
	vector<DrtVect> to_return;
	map<string, vector<DrtVect> >::const_iterator miter = complements_.find(name);
	if (miter != complements_.end()) {
		to_return.insert(to_return.end(), miter->second.begin(), miter->second.end());
	}
	return to_return;
}

vector<vector<DrtPred> > ConditionalAction::getComplements() const
{
	vector<DrtVect> to_return;
	map<string, vector<DrtVect> >::const_iterator miter = complements_.begin();
	map<string, vector<DrtVect> >::const_iterator mend = complements_.end();
	for (; miter != mend; ++miter) {
		vector<DrtVect> complement_lines = miter->second;
		for (int n = 0; n < complement_lines.size(); ++n)
			to_return.push_back(complement_lines.at(n));
	}
	return to_return;
}
static vector<DrtPred> create_dummy_preds_from(const vector<string> &heads, const string &ref)
{
	vector<string>::const_iterator hiter = heads.begin();
	vector<string>::const_iterator hend = heads.end();
	vector<DrtPred> to_return;

	for (; hiter != hend; ++hiter) {
		to_return.push_back(*hiter + "(" + ref + ")");
	}
	return to_return;
}

clause_vector ConditionalAction::getClause() const
{
	return cv_;
}

vector<DrtPred> ConditionalAction::getCons() const
{
	return cv_.getConsequence();
}

bool ConditionalPersona::hasSpecification()
{
	if (specifications_.size())
		return true;
	return false;
}

vector<vector<DrtPred> > ConditionalPersona::getSpecifications()
{
	return specifications_;
}

void ConditionalPersona::addConditionalPersona(const ConditionalPersona &rhs)
{
	if (reference_ != "" && reference_ != rhs.reference_)
		throw(std::runtime_error("Trying to join two personas with different references!"));

	if (reference_ == "")
		reference_ = rhs.reference_;

	names_.insert(names_.end(), rhs.names_.begin(), rhs.names_.end());
	pred_names_.insert(pred_names_.end(), rhs.pred_names_.begin(), rhs.pred_names_.end());
	actions_.insert(actions_.end(), rhs.actions_.begin(), rhs.actions_.end());
	specifications_.insert(specifications_.end(), rhs.specifications_.begin(), rhs.specifications_.end());
	/// Add the specifications!!!!
}

ConditionalAction ConditionalPersona::getActionFromVerbName(const string &head)
// Searches for an action that finds the verb with name "head"
{
	MapStCondActionPtr::iterator miter = verb_to_actions_.find(head);
	if (miter != verb_to_actions_.end())
		return ConditionalAction(*miter->second);
	// If the verb name is not found
	throw(std::runtime_error("No ConditionalAction found with verb " + head));

}

ConditionalAction ConditionalPersona::getActionFromVerbRef(const string &ref)
// Searches for an action that finds the verb with reference "ref"
{
	MapStCondActionPtr::iterator miter = ref_to_actions_.find(ref);
	if (miter != ref_to_actions_.end())
		return ConditionalAction(*miter->second);
	// If the verb name is not found
	throw(std::runtime_error("No ConditionalAction found with reference " + ref));
}

void ConditionalPersona::addAction(const ConditionalAction &a)
{
	boost::shared_ptr<ConditionalAction> pa(new ConditionalAction(a));

	actions_.push_back(pa);

	string verb_name = a.getVerb();
	string verb_ref = a.getRef();

	verb_to_actions_[verb_name] = pa;
	ref_to_actions_[verb_ref] = pa;
}

static bool has_specification(const DrtPred &pred)
{
	string head = extract_header(pred);

	if (head.size() && head.at(0) == '@') {
		string ftag = extract_first_tag(pred);
		if (ftag.find("name") != string::npos)
			return true;
	}
	return false;
}

static vector<DrtPred> get_specification_from_pred(const vector<DrtPred> &preds, const DrtPred &pred)
{
	vector<DrtPred> to_return = get_elements_next_of(preds, pred);

	return to_return;
}

static bool is_subordinate_of_verb(const vector<DrtPred> &pre_drt, const string &subord_ref, DrtPred *candidate_subord_pred)
{
	vector<DrtPred>::const_iterator piter = pre_drt.begin();
	vector<DrtPred>::const_iterator pend = pre_drt.end();
	for (; piter != pend; ++piter) {
		if (extract_header(*piter) == "@SUBORD") {
			string second_tag = extract_second_tag(*piter);
			if (second_tag == subord_ref) {
				*candidate_subord_pred = *piter;
				return true;
			}
		}
	}
	return false;
}

static int find_prep_with_target(vector<DrtPred> &pre_drt, string from_str)
{
	for (int n = 0; n < pre_drt.size(); ++n) {
		PredTree tmp_verb = extract_header(pre_drt.at(n));
		string sref = extract_second_tag(pre_drt.at(n));
		if (pre_drt.at(n).is_complement() && !pre_drt.at(n).is_parenthesis() // the percolation MUST be inside a PRN
				&& sref == from_str) {
			return n;
		}
	}
	return -1;
}

static boost::tuple<string, string, string> get_reference_percolated_to_verb(vector<DrtPred> &pre_drt, int n)
/// Implement a recursive algorithm !!!
{
	if (debug) {
		cout << "PERCOLATING::: ";
		print_vector(pre_drt);
	}

	boost::tuple<string, string, string> to_return(boost::make_tuple(extract_first_tag(pre_drt.at(n)), extract_subject(pre_drt.at(n)), extract_object(pre_drt.at(n)) ));
	string from_str = extract_first_tag(pre_drt.at(n));
	int m = find_verb_with_subject(pre_drt, from_str);
	if (m == -1) {
		m = find_verb_with_object(pre_drt, from_str);
	}
	if (m == -1) { // if there is no verb, try to find a preposition
		m = find_prep_with_target(pre_drt, from_str);
	}
	if (debug) {
		cout << "PERCOLATIN2::: ";
		cout << m << endl;
	}

	if (m != -1) { // There might not be such a verb
		string verb_str = extract_first_tag(pre_drt.at(m));
		if (ref_is_verb(verb_str)) {
			int m2 = find_verb_with_string(pre_drt, verb_str);
			string subj_str = "none";
			string  obj_str = "none";
			if (m2 != -1) {
				subj_str = extract_subject(pre_drt.at(m2));
				obj_str  = extract_object(pre_drt.at(m2));
			}
			to_return = boost::make_tuple(verb_str, subj_str, obj_str);
		} else {
			m = find_verb_with_object(pre_drt, verb_str);
			if (m == -1) // if there is no verb, try to find a preposition
				m = find_prep_with_target(pre_drt, verb_str);
			if (m != -1) { // There might not be such a verb
				string verb_str = extract_first_tag(pre_drt.at(m));
				if (ref_is_verb(verb_str)) {
					int m2 = find_verb_with_string(pre_drt, verb_str);
					string subj_str = "none";
					string  obj_str = "none";
					if (m2 != -1) {
						subj_str = extract_subject(pre_drt.at(m2));
						obj_str  = extract_object(pre_drt.at(m2));
					}
					to_return = boost::make_tuple(verb_str, subj_str, obj_str);
				} else {
					m = find_verb_with_object(pre_drt, verb_str);
					if (m == -1) // if there is no verb, try to find a preposition
						m = find_prep_with_target(pre_drt, verb_str);
					if (m != -1) { // There might not be such a verb
						string verb_str = extract_first_tag(pre_drt.at(m));
						if (ref_is_verb(verb_str)) {
							int m2 = find_verb_with_string(pre_drt, verb_str);
							string subj_str = "none";
							string  obj_str = "none";
							if (m2 != -1) {
								subj_str = extract_subject(pre_drt.at(m2));
								obj_str  = extract_object(pre_drt.at(m2));
							}
							to_return = boost::make_tuple(verb_str, subj_str, obj_str);
						} else {
							m = find_verb_with_object(pre_drt, verb_str);
							if (m == -1) // if there is no verb, try to find a preposition
								m = find_prep_with_target(pre_drt, verb_str);
							if (m != -1) { // There might not be such a verb
								string verb_str = extract_first_tag(pre_drt.at(m));
								int m2 = find_verb_with_string(pre_drt, verb_str);
								string subj_str = "none";
								string  obj_str = "none";
								if (m2 != -1) {
									subj_str = extract_subject(pre_drt.at(m2));
									obj_str  = extract_object(pre_drt.at(m2));
								}
								to_return = boost::make_tuple(verb_str, subj_str, obj_str);
							}
						}
					}
				}
			}
		}
	}
	return to_return;
}

static bool is_valid_subject_ref(const string &str)
{
	if(str.find("none") != string::npos || str.find("subj") != string::npos || str.find("obj") != string::npos )
		return false;
	return true;
}

void Rules::compute()
{
	metric *d = metric_singleton::get_metric_instance();
	vector<clause_vector>::iterator citer = cv_.begin();
	vector<clause_vector>::iterator cend = cv_.end();

	for (; citer != cend; ++citer) {
		vector<DrtPred> preds = citer->getConsequence();
		vector<DrtPred>::iterator diter = preds.begin();
		vector<DrtPred>::iterator dend = preds.end();
		int m=0;
		// extract the references from names
		for (; diter != dend; ++diter, ++m) {
			if (diter->is_name() || diter->is_PRP()) {
				string ref = extract_first_tag(*diter);
				if (ref.size() == 0)
					continue; // just a safety check
				CMapStPers::iterator pers_iter = personae_.find(ref);
				names_to_refs_[diter->name()].push_back(ref);
				if (pers_iter == personae_.end()) {
					references_.push_back(ref);
					personae_[ref].setReference(ref);
					personae_[ref].addName(diter->name());
					personae_[ref].addPred(*diter);
				} else {
					pers_iter->second.addName(diter->name());
					pers_iter->second.addPred(*diter);
				}

				// Keep in memory to which verb (and subject, for
				// the Persona ) the current reference percolate
				// to.
boost::tuple<string, string, string> percolated_reference_verb_subj_obj = get_reference_percolated_to_verb(preds, m);
				if (percolated_reference_verb_subj_obj.get<0>() != "none"
					&& !is_valid_subject_ref( percolated_reference_verb_subj_obj.get<1>() )
						) {
					if (debug) {
						puts("PERCOLATED_VERB:::");
						cout << percolated_reference_verb_subj_obj.get<0>()
							<< " "
							<< percolated_reference_verb_subj_obj.get<2>()
							<< endl;
					}
					if(!is_valid_subject_ref( percolated_reference_verb_subj_obj.get<2>()) )
						persona_pointer_[ref].push_back(percolated_reference_verb_subj_obj.get<0>());
					else
						persona_pointer_[ref].push_back(percolated_reference_verb_subj_obj.get<2>());
				}
				if (percolated_reference_verb_subj_obj.get<1>() != "none"
					&& is_valid_subject_ref( percolated_reference_verb_subj_obj.get<1>() )
						) {
					if (debug) {
						puts("PERCOLATED_SUBJ:::");
						cout << percolated_reference_verb_subj_obj.get<1>() << endl;
					}
					persona_pointer_[ref].push_back(percolated_reference_verb_subj_obj.get<1>());
				}
			}
			if (has_specification(*diter)) {
				string ref = extract_first_tag(*diter);
				int pos_name = find_element_with_string(preds, ref);
				string name = preds.at(pos_name).name();
				CMapStPers::iterator pers_iter = personae_.find(ref);
				if (pers_iter != personae_.end()) {
					vector<DrtPred> drtvect = get_specification_from_pred(preds, *diter);
					personae_[ref].addSpecification(drtvect);
				}
			}
		}
	}
	// extract the references from verbs

	vector<string>::iterator txiter = texts_.begin();
	vector<string>::iterator txend = texts_.end();
	citer = cv_.begin();
	vector<DrtPred> previous_elements;
	int m = 0;

	for (; citer != cend && txiter != texts_.end(); ++citer, ++txiter) {
		vector<DrtPred> preds = citer->getConsequence();
		vector<DrtPred>::iterator diter = preds.begin();
		vector<DrtPred>::iterator dend = preds.end();
		for (; diter != dend; ++diter, ++m) {
			if (diter->is_verb()
			) {
				string ref  = extract_subject(*diter);
				string oref = extract_object(*diter);
				if (!shortfind(subj_refs_, ref)) {
					if (debug) {
						cout << "RULES_SUBJ_REF:::" << ref << endl;
					}
					subj_refs_[ref] = true;
				}
				if (!shortfind(obj_refs_, oref)) {
					obj_refs_[oref] = true;
					if (debug) {
						cout << "Rules_OBJ_REF:::" << oref << endl;
					}
				}
				if (extract_header(*diter) == "be") {
					// invert subject and object for the verb "to be" (consistently with Match )
					if (!shortfind(obj_refs_, ref)) {
						obj_refs_[ref] = true;
					}
					if (!shortfind(subj_refs_, oref)) {
						if (debug) {
							cout << "Rules_INVERTED_SUBJ_REF:::" << oref << endl;
						}
						subj_refs_[oref] = true;
					}
				}
				string verb_ref = extract_first_tag(*diter);
				CMapStPers::iterator pers_iter = personae_.find(ref);

				DrtPred candidate_subord_prep;
				bool is_subord_trigger;
				is_subord_trigger = is_subordinate_of_verb(preds, verb_ref, &candidate_subord_prep);
				if (is_subord_trigger)
					continue;

				// add the verb name to the verb references
				verb_names_[verb_ref].push_back(extract_header(*diter));
				if (pers_iter == personae_.end()) {
					/// If the persona does not exist then the verb reference is used
					references_.push_back(verb_ref);
					personae_[verb_ref].setReference(verb_ref);
					pers_iter = personae_.find(verb_ref);
				}
				/// the type should be a reference to the type of phase
				/// but now it is just the reference to the verb "verb_ref"
				ConditionalAction tmp_action(verb_ref, link_, *txiter, *citer);

				string verb_name = extract_header(*diter);
				tmp_action.addVerb(verb_name);

				vector<string> object = find_string_object_of_verb(preds, m);
				tmp_action.addObject(object);

				// You must also find the complements of the subordinate (to be used in Knowledge's "processRules")
				for(int n2=0; n2 < preds.size(); ++n2) {
					if(!preds.at(n2).is_verb())
						continue;
					vector<DrtVect> complements_lines = find_complements_of_verb(preds, n2);
					for (int n = 0; n < complements_lines.size(); ++n) {
						tmp_action.addComplement(complements_lines.at(n));
					}
				}


				pers_iter->second.addAction(tmp_action);
			}
		}
	}
}

ConditionalPersona Rules::getConditionalPersona(const string &ref) const
{
	CMapStPers::const_iterator pers_iter = personae_.find(ref);
	if (pers_iter != personae_.end())
		return pers_iter->second;
	else
		throw std::runtime_error("No such refeference: " + ref + ".");
}

void Rules::print(std::ostream &out)
{
	CMapStPers::iterator pers_iter = personae_.begin();
	for (; pers_iter != personae_.end(); ++pers_iter) {
		vector<string> names = pers_iter->second.getNames();
		vector<string>::iterator niter = names.begin();
		vector<string>::iterator nend = names.end();
	}
}

void Rules::addRules(const Rules &dp)
{
	cv_.insert(cv_.end(), dp.cv_.begin(), dp.cv_.end());

	if (personae_.size()) {
		CMapStPers::const_iterator piter = dp.personae_.begin();
		CMapStPers::const_iterator pend = dp.personae_.end();
		for (; piter != pend; ++piter) {
			ConditionalPersona tmp_persona = piter->second;
			string ref = tmp_persona.getReference();

			CMapStPers::iterator this_piter = personae_.find(ref);
			if (this_piter != personae_.end())
				this_piter->second.addConditionalPersona(tmp_persona);
			else
				personae_[ref] = tmp_persona;
		}
		// personae_.insert(dp.personae_.begin(), dp.personae_.end() );
	} else
		personae_ = dp.personae_;

	// add the verb names in dp
	if (verb_names_.size()) {
		verb_names_.insert(dp.verb_names_.begin(), dp.verb_names_.end());
	} else
		verb_names_ = dp.verb_names_;

	// add the names_to_refs_ in dp
	if (names_to_refs_.size()) {
		names_to_refs_.insert(dp.names_to_refs_.begin(), dp.names_to_refs_.end());
	} else
		names_to_refs_ = dp.names_to_refs_;

	if (subj_refs_.size()) {
		subj_refs_.insert(dp.subj_refs_.begin(), dp.subj_refs_.end());
	} else
		subj_refs_ = dp.subj_refs_;
	if (obj_refs_.size()) {
		obj_refs_.insert(dp.obj_refs_.begin(), dp.obj_refs_.end());
	} else
		obj_refs_ = dp.obj_refs_;
	// add the persona_pointer_ in dp
	if (persona_pointer_.size()) {
		persona_pointer_.insert(dp.persona_pointer_.begin(), dp.persona_pointer_.end());
	} else
		persona_pointer_ = dp.persona_pointer_;

	// add the subordinates in dp
	if (subord_verbs_.size()) {
		subord_verbs_.insert(dp.subord_verbs_.begin(), dp.subord_verbs_.end());
	} else
		subord_verbs_ = dp.subord_verbs_;

	references_.insert(references_.end(), dp.references_.begin(), dp.references_.end());

}

void Rules::addReferences(Rules dp)
// Only add the references and the connected personae. It does not save the actions
{
	CMapStPers::iterator piter = dp.personae_.begin();
	CMapStPers::iterator pend = dp.personae_.end();
	for (; piter != pend; ++piter)
		piter->second.clearActions();

	if (personae_.size()) {
		CMapStPers::const_iterator piter = dp.personae_.begin();
		CMapStPers::const_iterator pend = dp.personae_.end();
		for (; piter != pend; ++piter) {
			ConditionalPersona tmp_persona = piter->second;
			string ref = tmp_persona.getReference();

			CMapStPers::iterator this_piter = personae_.find(ref);
			if (this_piter != personae_.end())
				this_piter->second.addConditionalPersona(tmp_persona);
			else
				personae_[ref] = tmp_persona;
		}
	} else
		personae_ = dp.personae_;

	// add the verb names in dp
	if (verb_names_.size()) {
		verb_names_.insert(dp.verb_names_.begin(), dp.verb_names_.end());
	} else
		verb_names_ = dp.verb_names_;

	// add the names_to_refs_ in dp
	if (names_to_refs_.size()) {
		names_to_refs_.insert(dp.names_to_refs_.begin(), dp.names_to_refs_.end());
	} else
		names_to_refs_ = dp.names_to_refs_;

	if (subj_refs_.size()) {
		subj_refs_.insert(dp.subj_refs_.begin(), dp.subj_refs_.end());
	} else
		subj_refs_ = dp.subj_refs_;
	if (obj_refs_.size()) {
		obj_refs_.insert(dp.obj_refs_.begin(), dp.obj_refs_.end());
	} else
		obj_refs_ = dp.obj_refs_;
	// add the persona_pointer_ in dp
	if (persona_pointer_.size()) {
		persona_pointer_.insert(dp.persona_pointer_.begin(), dp.persona_pointer_.end());
	} else
		persona_pointer_ = dp.persona_pointer_;

	// add the subordinates in dp
	if (subord_verbs_.size()) {
		subord_verbs_.insert(dp.subord_verbs_.begin(), dp.subord_verbs_.end());
	} else
		subord_verbs_ = dp.subord_verbs_;

	references_.insert(references_.end(), dp.references_.begin(), dp.references_.end());
}

vector<pair<DrtPred, ConditionalAction> > Rules::getSubordinates(const string &verb_ref)
{
	vector<pair<DrtPred, ConditionalAction> > to_return;

	CMapStTuple::iterator sub_iter = subord_verbs_.find(verb_ref);

	if (sub_iter != subord_verbs_.end()) {
		vector<boost::tuple<DrtPred, string, string> > sub_tuple = sub_iter->second;
		for (int m = 0; m < sub_tuple.size(); ++m) {
			DrtPred type_pred = sub_tuple.at(m).get<0>();
			string persona_ref = sub_tuple.at(m).get<1>();
			string verb_ref = sub_tuple.at(m).get<2>();
			ConditionalAction act = this->getConditionalPersona(persona_ref).getActionFromVerbRef(verb_ref);
			to_return.push_back(make_pair(type_pred, act));
		}
	}

	return to_return;
}

vector<string> Rules::getVerbNames(const string &ref) const
{
	CPMapStVSt::const_iterator verbs_iter = verb_names_.find(ref);
	if (verbs_iter != verb_names_.end()) {
		return verbs_iter->second;
	}
	return vector<string>();
}



vector<string> Rules::getRefFromName(const string &name) const
{
	CPMapStVSt::const_iterator name_iter = names_to_refs_.find(name);
	if (name_iter != names_to_refs_.end()) {
		return name_iter->second;
	}
	return vector<string>();
}

bool Rules::refIsSubj(const string &str) const
{
	if(debug) {
		cout << "REF_IS_SUBJ::: " << subj_refs_.size() << endl;
	}
	CMapStBool::const_iterator miter = subj_refs_.find(str);
	if (miter != subj_refs_.end()) {
		return true;
	}
	return false;
}

bool Rules::refIsObj(const string &str) const
{
	CMapStBool::const_iterator miter = obj_refs_.find(str);
	if (miter != obj_refs_.end()) {
		return true;
	}
	return false;
}

void Rules::clearUseless()
{
	subj_refs_.clear();
	obj_refs_.clear();
}

void ConditionalPersona::sort()
{
	std::sort(names_.begin(), names_.end());
	names_.erase(std::unique(names_.begin(), names_.end()), names_.end());
	std::sort(pred_names_.begin(), pred_names_.end());
	pred_names_.erase(std::unique(pred_names_.begin(), pred_names_.end()), pred_names_.end());
	std::sort(actions_.begin(), actions_.end());
	actions_.erase(std::unique(actions_.begin(), actions_.end()), actions_.end());
}

void Rules::sort()
{
	std::sort(references_.begin(), references_.end());
	references_.erase(std::unique(references_.begin(), references_.end()), references_.end());

	if (personae_.size()) {
		CMapStPers::iterator piter = personae_.begin();
		CMapStPers::iterator pend = personae_.end();
		for (; piter != pend; ++piter) {
			piter->second.sort();
		}
	}
}

vector<string> Rules::mapRefToActionRef(const string &ref) const
{
	vector<string> to_return;
	CPMapStVSt::const_iterator miter = persona_pointer_.find(ref);
	if (miter != persona_pointer_.end()) {
		to_return = miter->second;
	}
	return to_return;
}
