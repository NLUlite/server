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



#include"Presupposition.hpp"

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

static DrtVect create_gen_adj_drtvect(DrtPred name, DrtPred adj, int pnum)
{
	DrtVect to_return;
	string nameref = extract_first_tag(name);
	string adjref = extract_first_tag(adj);

	string impladj = string("[presupp") + boost::lexical_cast<string>(pnum) + "]v" + adjref;
	implant_first(adj, impladj);

	string implverb = nameref;
	if (nameref.find("name") != string::npos) {
		implverb.erase(0, 4);
	} else if (nameref.find("ref") != string::npos) {
		implverb.erase(0, 3);
	}
	implverb = string("[presupp") + boost::lexical_cast<string>(pnum) + "]verb" + implverb;

	name.set_pivot();
	to_return.push_back(name);
	DrtPred verb_pred(string("be(") + implverb + "," + nameref + "," + impladj + ")");
	verb_pred.setTag("VBP");
	to_return.push_back(verb_pred);
	adj.set_pivot();
	to_return.push_back(adj);

	return to_return;
}

static vector<DrtVect> process_generic_adj(const DrtVect &drtvect)
// "the white(name1) cat(name1) is on the sofa" -> cat(name1) is white
{
	vector<DrtVect> to_return;
	metric *d = metric_singleton::get_metric_instance();

	// extracts the adjectives
	vector<string> refs;
	map<string, vector<DrtPred> > map_ref;
	for (int n = 0; n < drtvect.size(); ++n) {
		if (drtvect.at(n).is_adjective()) {
			string head = extract_header(drtvect.at(n));
			if (d->pertains_to_name(head, "color", 6) > 0.5 || d->pertains_to_name(head, "newness", 6) > 0.5
					|| d->pertains_to_name(head, "political_orientation", 6) > 0.5
					|| d->hypernym_dist(head, "country", 8) > 0.5 || d->hypernym_dist(head, "color", 8) > 0.5) {
				string ref = extract_first_tag(drtvect.at(n));
				map_ref[ref].push_back(drtvect.at(n));
				refs.push_back(ref);
			}
		}
	}

	// create DrtVect of the type "cat is white"
	vector<string> already_done;
	int pnum = 0;
	for (int n = 0; n < drtvect.size(); ++n) {
		string ref = extract_first_tag(drtvect.at(n));
		if (drtvect.at(n).is_name() && drtvect.at(n).is_pivot() && !drtvect.at(n).is_adjective()
				&& find(already_done.begin(), already_done.end(), ref) == already_done.end()) {
			if (find(refs.begin(), refs.end(), ref) != refs.end()) {
				already_done.push_back(ref);
				vector<DrtPred> adjs = map_ref[ref];
				for (int m = 0; m < adjs.size(); ++m) {
					DrtVect tmpdrt = create_gen_adj_drtvect(drtvect.at(n), adjs.at(m), pnum++);
					to_return.push_back(tmpdrt);
				}
			}
		}
	}

	return to_return;
}

static DrtVect create_material_drtvect(DrtPred name, DrtPred adj, int pnum)
{
	DrtVect to_return;
	string nameref = extract_first_tag(name);
	string adjref = extract_first_tag(adj);

	metric *d = metric_singleton::get_metric_instance();

	string impladj = string("[material][presupp") + boost::lexical_cast<string>(pnum) + "]v" + adjref;
	implant_first(adj, impladj);
	string adj_head = extract_header(adj);
	string new_head = extract_header(adj_head);

	// Make sure this is the case where the adjective is pertaining to a name
	if (d->pertains_to_name(adj_head, "material", 6) > 0.5) {
		new_head = adj_head + "|";
		vector<string> pertainyms = d->get_pertainyms(adj_head);
		for (int n = 0; n < pertainyms.size(); ++n) {
			new_head += pertainyms.at(n);
			if (n != pertainyms.size() - 1)
				new_head += "|";
		}
	}
	implant_header(adj, new_head);
	adj.setTag("NN");

	string implverb = nameref;
	if (nameref.find("name") != string::npos) {
		implverb.erase(0, 4);
	} else if (nameref.find("ref") != string::npos) {
		implverb.erase(0, 3);
	}
	implverb = string("[material][presupp") + boost::lexical_cast<string>(pnum) + "]verb" + implverb;
	string none = string("[material][presupp") + boost::lexical_cast<string>(pnum) + "]none";

	name.set_pivot();
	to_return.push_back(name);
	DrtPred verb_pred(string("make(") + implverb + "," + none + "," + nameref + ")");
	verb_pred.setTag("VBP");
	to_return.push_back(verb_pred);
	DrtPred genitive("@GENITIVE(" + implverb + "," + impladj + ")");
	to_return.push_back(genitive);
	adj.set_pivot();
	to_return.push_back(adj);

	return to_return;
}

static DrtVect create_be_material_drtvect(DrtPred name, DrtPred adj, int pnum)
{
	DrtVect to_return;
	string nameref = extract_first_tag(name);
	string adjref = extract_first_tag(adj);

	metric *d = metric_singleton::get_metric_instance();

	string impladj = string("[material][presupp") + boost::lexical_cast<string>(pnum) + "]v" + adjref;
	implant_first(adj, impladj);
	adj.setTag("JJ");

	string implverb = nameref;
	if (nameref.find("name") != string::npos) {
		implverb.erase(0, 4);
	} else if (nameref.find("ref") != string::npos) {
		implverb.erase(0, 3);
	}
	implverb = string("[material][presupp") + boost::lexical_cast<string>(pnum) + "]verb" + implverb;
	string none = string("[material][presupp") + boost::lexical_cast<string>(pnum) + "]none";

	name.set_pivot();
	to_return.push_back(name);
	DrtPred verb_pred(string("be(") + implverb + "," + nameref + "," + impladj + ")");
	verb_pred.setTag("VBP");
	to_return.push_back(verb_pred);
	adj.set_pivot();
	to_return.push_back(adj);

	return to_return;
}

static vector<DrtVect> process_material_adj(const DrtVect &drtvect)
// "the wooden(name1) table(name1) is in the kitchen" -> table(name1) is made of wood
{
	vector<DrtVect> to_return;
	metric *d = metric_singleton::get_metric_instance();

	// extracts the adjectives
	vector<string> refs;
	map<string, vector<DrtPred> > map_ref;
	for (int n = 0; n < drtvect.size(); ++n) {
		if (drtvect.at(n).is_adjective()) {
			string ref = extract_first_tag(drtvect.at(n));
			string head = extract_header(drtvect.at(n));
			if (d->pertains_to_name(head, "material", 6) > 0.5 || d->hypernym_dist(head, "material", 8) > 0.5
					|| d->hypernym_dist(head, "metal", 8) > 0.5) { // the adjective is a material
					//cout << d->hypernym_dist(head, "material", 8) << endl;
					//cout << d->hypernym_dist(head, "metal", 8) << endl;
				if(debug) {
					cout << "MATERIAL::: " << head << " " << endl;
				}

				map_ref[ref].push_back(drtvect.at(n));
				refs.push_back(ref);
			}
		}
	}

	// create DrtVect of the type "table is made of wood"
	vector<string> already_done;
	int pnum = 0;
	for (int n = 0; n < drtvect.size(); ++n) {
		string ref = extract_first_tag(drtvect.at(n));
		if (drtvect.at(n).is_name() && !drtvect.at(n).is_adjective()
				&& find(already_done.begin(), already_done.end(), ref) == already_done.end()) {
			if (find(refs.begin(), refs.end(), ref) != refs.end()) {
				already_done.push_back(ref);
				vector<DrtPred> adjs = map_ref[ref];
				for (int m = 0; m < adjs.size(); ++m) {
					DrtVect tmpdrt = create_material_drtvect(drtvect.at(n), adjs.at(m), pnum++);
					to_return.push_back(tmpdrt);
					tmpdrt = create_be_material_drtvect(drtvect.at(n), adjs.at(m), pnum++);
					to_return.push_back(tmpdrt);
				}
			}
		}
	}

	return to_return;
}

static DrtVect create_size(const DrtPred &subj, DrtPred unit, int pnum)
// Create DrtVect in the following way
// ->  table(A) be(C,A,B) cm(B) wide/J(B) @QUANTITY(B,D) 50(D)  ->table(A) be(C,A,none) @SIZE(C,B) cm(B)
// the quantity is already present as a reference to B
{
	DrtVect to_return;
	string subj_ref = extract_first_tag(subj);
	string unit_ref = extract_first_tag(unit);

	string implverb = subj_ref;
	if (subj_ref.find("name") != string::npos) {
		implverb.erase(0, 4);
	} else if (subj_ref.find("ref") != string::npos) {
		implverb.erase(0, 3);
	}
	string verb_ref = string("[size][presupp") + boost::lexical_cast<string>(pnum) + "]verb" + implverb;
	string none = "none";

	to_return.push_back(subj);
	DrtPred verb_pred(string("be(") + verb_ref + "," + subj_ref + "," + none + ")");
	verb_pred.setTag("VBP");
	to_return.push_back(verb_pred);

	DrtPred size_pred(string("@SIZE(") + verb_ref + "," + unit_ref + ")");

	to_return.push_back(size_pred);
	to_return.push_back(unit);

	return to_return;
}

static vector<DrtVect> process_size(DrtVect drtvect)
// "the table is 50 cm wide." 
// ->  table(A) be(C,A,B) cm(B) wide/J(B) @QUANTITY(B,D) 50(D)  ->table(A) be(C,A,B) @SIZE(C,D) cm(D) @QUANTITY(D,E) 50(E)
{
	vector<DrtVect> to_return;
	metric *d = metric_singleton::get_metric_instance();

	// extracts the adjectives
	vector<string> refs;
	map<string, DrtPred> map_ref; // <ref,subj>
	for (int n = 0; n < drtvect.size(); ++n) {
		if (drtvect.at(n).is_adjective()
		    || drtvect.at(n).is_name()
				) {
			string ref = extract_first_tag(drtvect.at(n));
			string head = extract_header(drtvect.at(n));
			if (head == "long" || head == "wide" || head == "tall" || head == "high" || head == "big") { // the adjective is a material
				int m = find_verb_with_object(drtvect, ref);
				if (m == -1)
					continue;
				DrtPred verb_pred = drtvect.at(m);
				string verb_head = extract_header(verb_pred);
				string verb_levin = d->get_levin_verb(verb_head);
				if (verb_levin != "verb.stative")
					continue;
				string subj_ref = extract_subject(verb_pred);
				m = find_element_with_string(drtvect, subj_ref);
				if (m == -1)
					continue;
				map_ref[ref] = drtvect.at(m);
				refs.push_back(ref);
			}
		}
	}

	// create DrtVect of the type "table be @SIZE cm @QUANTITY 50"
	vector<string> already_done;
	int pnum = 0;
	for (int n = 0; n < drtvect.size(); ++n) {
		string ref = extract_first_tag(drtvect.at(n));
		if (drtvect.at(n).is_name() && !drtvect.at(n).is_adjective()
				&& find(already_done.begin(), already_done.end(), ref) == already_done.end()) {
			if (find(refs.begin(), refs.end(), ref) != refs.end()) {
				string head = extract_header(drtvect.at(n));
				if (d->hypernym_dist(head, "unit", 8) > 0.1) {
					already_done.push_back(ref);
					DrtPred subj_pred = map_ref[ref];
					DrtPred unit_pred = drtvect.at(n);
					DrtVect tmpdrt = create_size(subj_pred, unit_pred, pnum++);
					to_return.push_back(tmpdrt);
				}
			}
		}
	}

	return to_return;
}

static bool contains_unit(const string &str)
{
	vector<string> units;
	units.push_back("meter");
	units.push_back("kilometer");
	units.push_back("gram");
	units.push_back("kilo");
	units.push_back("ton");
	units.push_back("m");
	units.push_back("g");
	units.push_back("feet");
	units.push_back("inch");
	units.push_back("ft");
	units.push_back("in");

	for (int n = 0; n < units.size(); ++n) {
		int pos = str.find(units.at(n));
		if (pos > 1) {
			char prior = str.at(pos - 1);
			char prior_prior = str.at(pos - 2);
			if (prior == '-' && isdigit(prior_prior)) {
				return true;
			}
		}
		if (pos > 0) {
			char prior = str.at(pos - 1);
			if (isdigit(prior)) {
				return true;
			}
		}
	}

	return false;
}

static DrtVect create_size_from_quantities(const DrtPred &subj, DrtPred unit, int pnum)
//  table(A) be(C2,A,none) @SIZE(C,D) 50cm(D)
{
	DrtVect to_return;
	string subj_ref = extract_first_tag(subj);
	string unit_ref = extract_first_tag(unit);

	string implverb = subj_ref;
	if (subj_ref.find("name") != string::npos) {
		implverb.erase(0, 4);
	} else if (subj_ref.find("ref") != string::npos) {
		implverb.erase(0, 3);
	}
	string verb_ref = string("[size2][presupp") + boost::lexical_cast<string>(pnum) + "]verb" + implverb;
	string none = "none";

	to_return.push_back(subj);
	DrtPred verb_pred(string("be(") + verb_ref + "," + subj_ref + "," + none + ")");
	verb_pred.setTag("VBP");
	to_return.push_back(verb_pred);

	DrtPred size_pred(string("@SIZE(") + verb_ref + "," + unit_ref + ")");

	to_return.push_back(size_pred);
	to_return.push_back(unit);

	return to_return;
}

static vector<DrtVect> process_size_from_quantities(DrtVect drtvect)
// "the 50cm table is blue." 
//  -> table(A) be(B,A,C) blue(C) @QUANTITY(A,D) 50cm(D)   ->table(A) be(C2,A,none) @SIZE(C,D) 50cm(D)
{
	vector<DrtVect> to_return;
	metric *d = metric_singleton::get_metric_instance();

	// extracts the adjectives
	vector<pair<DrtPred, DrtPred> > unit_pairs;
	for (int n = 0; n < drtvect.size(); ++n) {
		string ref = extract_first_tag(drtvect.at(n));
		string head = extract_header(drtvect.at(n));
		if (drtvect.at(n).is_complement() && head == "@QUANTITY") {
			string name_ref = extract_first_tag(drtvect.at(n));
			string unit_ref = extract_second_tag(drtvect.at(n));
			int m;
			m = find_element_with_string(drtvect, name_ref);
			if (m == -1)
				continue;
			DrtPred name_pred(drtvect.at(m));
			m = find_element_with_string(drtvect, unit_ref);
			if (m == -1)
				continue;
			DrtPred unit_pred(drtvect.at(m));
			string head_unit = extract_header(unit_pred);
			if (!contains_unit(head_unit)) // this CD is something like "50-meters" or "3cm"
				continue;
			unit_pairs.push_back(make_pair(name_pred, unit_pred));
		}
	}

	// create the DrtVect
	int pnum = 0;
	for (int n = 0; n < unit_pairs.size(); ++n) {
		DrtVect tmpdrt = create_size_from_quantities(unit_pairs.at(n).first, unit_pairs.at(n).second, pnum++);
		to_return.push_back(tmpdrt);
	}

	return to_return;
}

static bool has_object(const DrtPred &pred)
{
	string obj = extract_object(pred);
	if (debug)
		cout << "OBJ::: " << obj << endl;
	if (obj.find("obj") == string::npos && obj.find("none") == string::npos && obj.find("subj") == string::npos // The passive verbs invert the tags
	    )
		return true;
	return false;
}

static vector<DrtVect> process_is_when(DrtVect to_return)
// something is when ... -> something happens when ...
{
	DrtVect orig_drt(to_return);

	vector<pair<DrtPred, DrtPred> > unit_pairs;
	for (int n = 0; n < to_return.size(); ++n) {
		string fref = extract_first_tag(to_return.at(n));
		string sref = extract_second_tag(to_return.at(n));
		string head = extract_header(to_return.at(n));
		string name = to_return.at(n).name();

		if (to_return.at(n).tag() == "WRB" && name == "when" && head == "@TIME_AT"
			&& ref_is_verb(fref)
			&& ref_is_verb(sref)
		) {
			int m = find_verb_with_string(to_return, fref);
			if (m == -1)
				continue;
			if(has_object(to_return.at(m)))
				continue;
			string vheader = extract_header(to_return.at(m));
			if(vheader == "be")  {
				implant_header(to_return.at(m),"happen");
				to_return.at(m).setName("happen");
			}
		}
	}

	vector<DrtVect> to_return2;
	if(to_return != orig_drt)
		to_return2.push_back(to_return);

	return to_return2;
}

static DrtVect create_have_good_sentence(DrtPred subj_pred, const string &noun_str, bool has_negation, int serial)
{
	DrtVect to_return;
	string subj_ref = extract_first_tag(subj_pred);
	string impl_verb= subj_ref;
	if (subj_ref.find("name") != string::npos) {
		impl_verb.erase(0, 4);
	} else if (subj_ref.find("ref") != string::npos) {
		impl_verb.erase(0, 3);
	}
	string serial_str = boost::lexical_cast<string>(serial);

	string verb_ref = "[presupp_noun"+serial_str+"]_verb_" + impl_verb;
	string obj_ref  = "[presupp_noun"+serial_str+"]_name_" + impl_verb;

	DrtPred verb_pred(string("have/VB(") + verb_ref + "," + subj_ref + "," + obj_ref + ")");
	DrtPred not_pred (string("not/RB(")  + verb_ref + ")");
	DrtPred good_pred(string("good/JJ(") + obj_ref  + ")");
	DrtPred name_pred(noun_str + "/NN("  + obj_ref  + ")");

	name_pred.set_pivot();

	to_return.push_back(subj_pred);
	to_return.push_back(verb_pred);
	if(has_negation)
		to_return.push_back(not_pred);
	to_return.push_back(good_pred);
	to_return.push_back(name_pred);

	return to_return;
}


static bool has_negation(DrtPred pred, vector<DrtPred> speech)
{
	if (pred.is_verb()) {
		string vref = extract_first_tag(pred);
		for (int n = 0; n < speech.size(); ++n) {
			if (speech.at(n).is_adverb()) {
				string fref = extract_first_tag(speech.at(n));
				string head = extract_header(speech.at(n));
				if (fref == vref && head == "not")
					return true;
			}
		}
	}
	return false;
}


static vector<DrtVect> process_nouns_from_verbs(DrtVect to_return)
// Dogs hear well -> dogs have good hearing.
{
	DrtVect orig_drt(to_return);
	metric *d = metric_singleton::get_metric_instance();
	vector<DrtVect> to_return2;

	for (int n = 0; n < to_return.size(); ++n) {
		string fref = extract_first_tag(to_return.at(n));
		string head = extract_header(to_return.at(n));
		string tag = to_return.at(n).tag();
		if (tag == "RB" && head == "well"
		) {
			if(debug) {
				cout << "IS_WELL::: " << to_return.at(n) << endl;
			}
			int m = find_verb_with_string(to_return, fref);
			if (m == -1)
				continue;
			if(debug) {
				cout << "IS_VERB1::: " << to_return.at(m) << endl;
			}
			string subj_ref = extract_subject(to_return.at(m) );
			int m2= find_element_with_string(to_return,subj_ref);
			if (m2 == -1)
				continue;
			if(debug) {
				cout << "IS_VERB2::: " << to_return.at(m2) << endl;
			}
			string verb_str= extract_header(to_return.at(m));
			string noun_str= d->getNounFromVerb(verb_str);
			if(debug) {
				cout << "IS_VERB3::: " << verb_str << " " << noun_str << endl;
			}
			if(noun_str == "")
				continue;
			to_return2.push_back( create_have_good_sentence(to_return.at(m2), noun_str, has_negation(to_return.at(m),to_return), n ) );
		}
	}

	return to_return2;
}

static int find_complement_with_first_tag(const vector<DrtPred> &pre_drt, string ref, const string &head = "")
{
	for (int n = 0; n < pre_drt.size(); ++n) {
		string head_tmp = extract_header(pre_drt.at(n));
		if (head != "" && head != head_tmp)
			continue;
		string sref = extract_first_tag(pre_drt.at(n));
		if (pre_drt.at(n).is_complement() && sref == ref) {
			return n;
		}
	}
	return -1;
}


static vector<DrtPred> substitute_ref(vector<DrtPred> &pre_drt, const string &from_str, const string &to_str)
{
	vector<DrtPred> predicates(pre_drt);

	for (int n = 0; n < predicates.size(); ++n) {
		vector<string> children = predicates.at(n).extract_children();
		for (int m = 0; m < children.size(); ++m) {
			if (children.at(m) == from_str) {
				children.at(m) = to_str;
			}
		}
		predicates.at(n).implant_children(children);
	}
	return predicates;
}

static int find_complement_with_second_tag(const DrtVect &drtvect, const string &ref)
{
	int m = -1;

	for (int n = 0; n < drtvect.size(); ++n) {
		if (drtvect.at(n).is_complement()) {
			string sref = extract_second_tag(drtvect.at(n));
			if (ref == sref) {
				m = n;
				break;
			}
		}
	}

	return m;
}


static pair<vector<DrtVect>, vector<DrtPred> > process_question_kind_of(DrtVect to_return)
// what kind of jobs ... ? -> what jobs ... ?
{
	DrtVect orig_drt(to_return);
	vector<DrtPred> conjunctions;

	for (int n = 0; n < to_return.size(); ++n) {
		string fref = extract_first_tag(to_return.at(n));
		string head = extract_header(to_return.at(n));
		string name = to_return.at(n).name();

		if (to_return.at(n).is_question()
			&& head == "kind"
		) {
			int m = find_complement_with_first_tag(to_return, fref, "@GENITIVE");
			if (m == -1)
				continue;
			string sref = extract_second_tag(to_return.at(m));
			int m2 = find_element_with_string(to_return, sref);
			if (m2 == -1)
				continue;
			DrtPred tmppred = to_return.at(m2);
			tmppred.set_question();
			tmppred.set_question_word(extract_header(to_return.at(m2)));
			to_return.at(m2) = tmppred;
			to_return.at(m2) = implant_first(to_return.at(m2),extract_first_tag(to_return.at(n)));
			add_header( to_return.at(n), ":DELETE");
			add_header( to_return.at(m), ":DELETE");
		}
	}

	vector<DrtVect> to_return2;
	DrtVect drtvect;
	for (int n = 0; n < to_return.size(); ++n) {
		if(!to_return.at(n).is_delete()) {
			drtvect.push_back(to_return.at(n));
		}
	}
	to_return2.push_back(drtvect);

	return make_pair(to_return2, conjunctions);
}

static void switch_children(DrtPred &pred)
{
	string fstr = extract_first_tag(pred);
	string sstr = extract_second_tag(pred);
	string str_tmp;
	str_tmp = fstr;
	fstr = sstr;
	sstr = str_tmp;
	implant_first(pred, fstr);
	implant_second(pred, sstr);
}


static pair<vector<DrtVect>, vector<DrtPred> > process_question_age_subord(DrtVect to_return)
// what kind of jobs ... ? -> what jobs ... ?
{
	DrtVect orig_drt(to_return);
	vector<DrtPred> conjunctions;

	for (int n = 0; n < to_return.size(); ++n) {
		string fref = extract_first_tag(to_return.at(n));
		string head = extract_header(to_return.at(n));
		string name = to_return.at(n).name();

		if (to_return.at(n).is_complement()
			&& head == "@AGE"
		) {
			int m = find_complement_with_first_tag(to_return, fref, "@SUBORD");
			if (m == -1)
				continue;
			string sref = extract_second_tag(to_return.at(m));
			switch_children(to_return.at(m)); // invert subordinate/main sentence
			implant_header(to_return.at(m),"@BEFORE|@AFTER|@TIME_TO|@TIME_FROM|@TIME_AT");
			m = find_verb_with_string(to_return,sref); // the verb that was in the subordinate is now the main sentence
			if(m != -1) {
				DrtPred first_pred= to_return.at(m);
				implant_header(to_return.at(m),":DELETE");
				to_return.insert(to_return.begin(),first_pred);
			}
		}
	}

	vector<DrtVect> to_return2;
	DrtVect drtvect;
	for (int n = 0; n < to_return.size(); ++n) {
		if(!to_return.at(n).is_delete()) {
			drtvect.push_back(to_return.at(n));
		}
	}
	to_return2.push_back(drtvect);

	return make_pair(to_return2, conjunctions);
}



static pair<vector<DrtVect>, vector<DrtPred> > process_question_about(DrtVect drtvect)
// happen(_B,_A,none) @TOPIC(_B,_C) ...noun/N(_C)
// -> happen(_B,_C,none)  ...noun/N(_C)
// -> happen(_B,_A,_C)  ...noun/N(_C)
{
	DrtVect orig_drtvect(drtvect);
	vector<DrtVect> to_return;
	metric *d = metric_singleton::get_metric_instance();

	vector<pair<string, string> > ref_pair;
	vector<DrtPred> conjunctions;

	// check that it is a "happen about" case
	// and find the topic
	DrtPred about_verb, about_noun;
	bool question_is_happen = false;
	for (int n = 0; n < drtvect.size(); ++n) {
		string header = extract_header(drtvect.at(n));
		if (drtvect.at(n).is_complement() && header.find("@TOPIC") != string::npos
				&& header.find("@ALLOCUTION") == string::npos) {
			string fref = extract_first_tag(drtvect.at(n));
			string sref = extract_second_tag(drtvect.at(n));
			if (!ref_is_verb(fref))
				continue;
			int vpos = find_verb_with_string(drtvect, fref);
			int npos = find_name_with_string(drtvect, sref);
			if (vpos == -1 || npos == -1)
				continue;
			about_verb = drtvect.at(vpos);
			string about_verb_str = extract_header(drtvect.at(vpos));
			if(about_verb_str != "happen")
				continue;
			about_noun = drtvect.at(npos);
			question_is_happen = true;
			break;
		}
	}

	// create the DrtVect with subj and obj
	if (question_is_happen) {
		DrtPred new_verb;
		DrtVect subj_about, obj_about;
		subj_about.push_back(about_noun);
		new_verb = about_verb;
		implant_subject(new_verb, extract_first_tag(about_noun));
		subj_about.push_back(new_verb);

		obj_about.push_back(about_noun);
		new_verb = about_verb;
		implant_object(new_verb, extract_first_tag(about_noun));
		obj_about.push_back(new_verb);

		to_return.push_back(subj_about);
		to_return.push_back(obj_about);
	}
	return make_pair(to_return, conjunctions);
}


static int find_previous_verb(const DrtVect &drtvect, int pos)
{
	for(int n= pos; n >= 0; --n) {
		if(drtvect.at(n).is_verb())
			return n;
	}
	return -1;
}


static pair<vector<DrtVect>, vector<DrtPred> > process_question_expand_genitive(DrtVect drtvect)
// @GENITIVE -> @GENITIVE|@WITH
// dogs of rank -> dogs with rank
{
	DrtVect orig_drtvect(drtvect);
	vector<DrtVect> to_return;
	metric *d = metric_singleton::get_metric_instance();

	vector<pair<string, string> > ref_pair;
	vector<DrtPred> conjunctions;

	for (int n = 0; n < drtvect.size(); ++n) {
		string header = extract_header(drtvect.at(n));
		if (drtvect.at(n).is_complement() && header == "@GENITIVE") {
			implant_header(drtvect.at(n),"@GENITIVE|@WITH");
		}
	}
	to_return.push_back(drtvect);
	drtvect=orig_drtvect;
	for (int n = 0; n < drtvect.size(); ++n) {
		string header = extract_header(drtvect.at(n));
		if (drtvect.at(n).is_complement() && header == "@GENITIVE") {
			implant_header(drtvect.at(n),"@GENITIVE|@WITH");
			string fref = extract_first_tag(drtvect.at(n));
			int m = find_verb_with_subject(drtvect,fref);
			if(m != -1) {
				continue;
			}
			m = find_previous_verb(drtvect,n);
			if(m != -1) {
				string vref = extract_first_tag(drtvect.at(m));
				implant_first(drtvect.at(n),vref); // detach the complement from any noun
			}
		}
	}
	if(debug) {
		cout << "PRESUPP_WITH::: ";
		print_vector(drtvect);
	}
	to_return.push_back(drtvect);
	sort(to_return.begin(), to_return.end());
	to_return.erase(std::unique(to_return.begin(), to_return.end()), to_return.end());

	return make_pair(to_return, conjunctions);
}

static pair<vector<DrtVect>, vector<DrtPred> > process_question_verbless(DrtVect drtvect)
{
	DrtVect orig_drtvect(drtvect);
	vector<DrtPred> conjunctions;
	vector<DrtVect> to_return;
	metric *d = metric_singleton::get_metric_instance();

	// check if there are verbs
	bool has_verb= false;
	for (int n = 0; n < drtvect.size(); ++n) {
		if(drtvect.at(n).is_verb()) {
			has_verb = true;
			break;
		}
	}
	if(!has_verb && drtvect.size() >0) {
		DrtPred what_pred("[what]/NN#[pivot](_name_vless_-1)");
		DrtPred be_pred  ("be/VB(_verb_vless_-1,_name_vless_-1,dummy)");
		string fref = extract_first_tag(drtvect.at(0));
		implant_object(be_pred,fref);
		what_pred.set_question();
		what_pred.set_question_word(extract_header(drtvect.at(0)));
		drtvect.at(0).set_question(false);
		DrtVect tmp_drtvect;

		if(debug) {
			cout << "PHEADER::: " << extract_header(what_pred) << endl;
		}

		tmp_drtvect.push_back(what_pred);
		tmp_drtvect.push_back(be_pred);
		tmp_drtvect.insert(tmp_drtvect.end(),drtvect.begin(),drtvect.end() );
		drtvect = tmp_drtvect;
	}
	if(debug) {
		cout << "PRESUPP_VERBLESSs::: ";
		print_vector(drtvect);
	}
	to_return.push_back(drtvect);
	return make_pair(to_return, conjunctions);
}



static bool verb_supports_indirect_obj(const string &name)
{
	vector<string> ind_verbs;
	ind_verbs.push_back("give");
	ind_verbs.push_back("wish");
	ind_verbs.push_back("make");
	ind_verbs.push_back("make_it");
	ind_verbs.push_back("find");
	ind_verbs.push_back("buy");
	ind_verbs.push_back("pay");
	ind_verbs.push_back("purchase");
	ind_verbs.push_back("show");
	ind_verbs.push_back("call");
	ind_verbs.push_back("tell");
	ind_verbs.push_back("sell");
	ind_verbs.push_back("regard_as");
	ind_verbs.push_back("set");
	ind_verbs.push_back("name");
	ind_verbs.push_back("promise");
	ind_verbs.push_back("consider");
	ind_verbs.push_back("build");
	ind_verbs.push_back("bring");
	ind_verbs.push_back("pass");
	ind_verbs.push_back("cost");

	if (find(ind_verbs.begin(), ind_verbs.end(), name) != ind_verbs.end())
		return true;

	return false;
}


static vector<DrtPred> switch_dative(const vector<DrtPred> &pre_drt, int pos_verb)
// switch an adjective tagged as an object with an @DATIVE
{
	if(pos_verb > pre_drt.size() || pos_verb < 0 )
		return pre_drt;

	vector<DrtPred> to_return(pre_drt);

	metric *d = metric_singleton::get_metric_instance();

	int n = find_complement_with_first_tag(pre_drt, extract_first_tag(pre_drt.at(pos_verb)), "@DATIVE" );

	if(debug) {
		cout << "FIND_COMPLEMENT::: " << n << endl;
	}

	if (n == -1)
		return pre_drt;
	string header = extract_header(to_return.at(n));
	if (to_return.at(n).is_complement() ) {
		string fref = extract_first_tag(to_return.at(n));
		string sref = extract_second_tag(to_return.at(n));
		int m = find_verb_with_string(to_return, fref);
		if (m == -1)
			return pre_drt;
		string header = extract_header(to_return.at(m));
		header = header.substr(0, header.find(":"));
		string levin = d->get_levin_verb(header);
		if ( header == "get" || header == "be" || (levin == "verb.stative" && header != "find") )
			return pre_drt;
		string sobj = extract_object(to_return.at(m));

		// find out if the object is an adjective
		vector<int> obj_poz = find_all_names_with_string_no_delete(to_return, sobj);
		vector<int> end_poz = find_all_names_with_string_no_delete(to_return, sref);

		if (obj_poz.size() == 0 || end_poz.size() == 0)
			return pre_drt;
		// switch @DATIVE and the object
		implant_second(to_return.at(n), sobj);
		implant_object(to_return.at(m), sref);
	}
	return to_return;
}

static pair<vector<DrtVect>, vector<DrtPred> > process_question_indirect_object(DrtVect drtvect)
// give(_A,_B,_C) noun1/N(_C) @DATIVE(_A,_D) ...noun/N(_D)
// give(_A,_B,_D) noun1/N(_C) @DATIVE(_A,_C) ...noun/N(_D)
// Sometimes the @DATIVE is not correct for questions
{
	DrtVect orig_drtvect(drtvect);
	vector<DrtVect> to_return;
	metric *d = metric_singleton::get_metric_instance();

	vector<pair<string, string> > ref_pair;
	vector<DrtPred> conjunctions;

	// find the verb with indirect object
	DrtPred about_verb, about_noun;
	bool question_is_happen = false;
	for (int n = 0; n < drtvect.size(); ++n) {
		string header = extract_header(drtvect.at(n));
		if (drtvect.at(n).is_verb()
		    && verb_supports_indirect_obj(header)
		) {
			DrtVect tmp_drt= switch_dative(drtvect,n);
			if(tmp_drt != orig_drtvect)
				to_return.push_back(tmp_drt);
		}
	}
	return make_pair(to_return, conjunctions);
}


static pair<vector<DrtVect>, vector<DrtPred> > process_question_subordinates(DrtVect drtvect)
// David(_A) say(_B,_A,none) @ALLOCUTION(_B,_C) ...|[S]/N(_C)
// -> David(_A) say(_B,_A,none) @ALLOCUTION(_B,_C) ...|[*]/V(_C,_D,_E)
// It returns the DrtVect of the presuppositions, and the dummy DrtPred of the conjunctions
{
	DrtVect orig_drtvect(drtvect);
	vector<DrtVect> to_return;
	metric *d = metric_singleton::get_metric_instance();

	vector<pair<string, string> > ref_pair;
	vector<DrtPred> conjunctions;

	// Check if there is a noun with [S]
	// and create a DrtVect with a dummy verb instead of the noun
	vector<string> refs;
	map<string, vector<DrtPred> > map_ref;
	for (int n = 0; n < drtvect.size(); ++n) {
		if (drtvect.at(n).is_sentence()) {
			string fref = extract_first_tag(drtvect.at(n));
			string implverb = fref;
			if (fref.find("name") != string::npos) {
				implverb.erase(0, 4);
			} else if (fref.find("ref") != string::npos) {
				implverb.erase(0, 3);
			}
			string vref, sref, oref;
			vref = string("_[presupp]verb") + implverb;
			sref = string("_[presupp]namesubj") + implverb;
			oref = string("_[presupp]nameobj") + implverb;
			DrtPred tmpverb(string("[*]/V(") + vref + "," + sref + "," + oref + ")");

			if (drtvect.at(n).is_question()) {
				tmpverb.set_question();
				tmpverb.set_question_word(drtvect.at(n).get_question_word());
			}

			ref_pair.push_back(make_pair(fref, vref));
			drtvect.at(n) = tmpverb;

			int m = find_complement_with_second_tag(drtvect, fref);
			if (m != -1) {
				string orig_verb_ref = extract_first_tag(drtvect.at(m));
				DrtPred tmp_conj(string("@COORDINATION(" + orig_verb_ref + "," + vref + ")"));
				conjunctions.push_back(tmp_conj);
			}
		}
	}

	for (int n = 0; n < ref_pair.size(); ++n) {
		string ref = ref_pair.at(n).first;
		string name = ref_pair.at(n).second;
		drtvect = substitute_ref(drtvect, ref, name);
	}

	if (drtvect != orig_drtvect)
		to_return.push_back(drtvect);

	return make_pair(to_return, conjunctions);
}

static pair<vector<DrtVect>, vector<DrtPred> > process_what_subordinates_with_not(DrtVect drtvect)
// David(_A) say(_B,_A,none) @ALLOCUTION(_B,_C) ...|[S]/N(_C)
// -> David(_A) say(_B,_A,none) @ALLOCUTION(_B,_C) ...|[*]/V(_C,_D,_E)
// It returns the DrtVect of the presuppositions, and the dummy DrtPred of the conjunctions
{
	DrtVect orig_drtvect(drtvect);
	vector<DrtVect> to_return;
	metric *d = metric_singleton::get_metric_instance();

	vector<pair<string, string> > ref_pair;
	vector<DrtPred> conjunctions;

	// Create a DrtVect with a dummy verb instead of the noun
	vector<string> refs;
	map<string, vector<DrtPred> > map_ref;
	for (int n = 0; n < drtvect.size(); ++n) {
		string fref = extract_first_tag(drtvect.at(n));
		string sref = extract_second_tag(drtvect.at(n));
		if (drtvect.at(n).is_complement() && ref_is_verb(fref) && ref_is_name(sref)) {
			int wpos = find_element_with_string(drtvect, sref);
			if (wpos == -1)
				continue;
			if (drtvect.at(wpos).is_what()) {
				// cout << "drtvect::: " << drtvect.at(n) << endl;
				// cout << "drtvect2:: " << drtvect.at(wpos) << endl;
				int vpos = find_verb_with_string(drtvect, fref);
				if (vpos == -1)
					continue;
				string orig_verb = extract_first_tag(drtvect.at(vpos));
				string implverb = fref;
				if (fref.find("name") != string::npos) {
					implverb.erase(0, 4);
				} else if (fref.find("ref") != string::npos) {
					implverb.erase(0, 3);
				}
				string vref, sref, oref;
				vref = string("_[presupp]verb") + implverb;
				sref = string("_[presupp]namesubj") + implverb;
				oref = string("_[presupp]nameobj") + implverb;
				DrtPred tmpverb(string("[*]/V(") + vref + "," + sref + "," + oref + ")");
				DrtPred not_pred(string("not/RB(") + vref + ")");
				drtvect.push_back(not_pred);

				//if (drtvect.at(n).is_question()) {
				tmpverb.set_question();
				tmpverb.set_question_word(drtvect.at(wpos).get_question_word());
				//}

				implant_second(drtvect.at(n), vref);
				drtvect.at(wpos) = tmpverb;

				DrtPred tmp_conj(string("@COORDINATION(" + orig_verb + "," + vref + ")"));
				conjunctions.push_back(tmp_conj);
			}
		}
	}

	if (drtvect != orig_drtvect)
		to_return.push_back(drtvect);

	return make_pair(to_return, conjunctions);
}


static bool is_broken(const DrtPred &pred)
{
	string head = extract_header(pred);

	if (head.size() && head.at(0) == '@') {
		string ftag = extract_first_tag(pred);
		if (ftag.find("broken") != string::npos)
			return true;
	}
	return false;
}

static string substitute_string(string str, const string& orig, const string& replace)
{
	int pos = 0;
	while ((pos = str.find(orig, pos)) != std::string::npos) {
		str.replace(pos, orig.size(), replace);
		pos += replace.size();
	}
	return str;
}

static pair<vector<DrtVect>, vector<DrtPred> > process_question_broken_subordinates(DrtVect drtvect)
// David(_A) say(_B,_A,none) @ALLOCUTION(_B,_C) ...|[S]/N(_C)
// -> David(_A) say(_B,_A,none) @ALLOCUTION(_B,_C) ...|[*]/V(_C,_D,_E)
// It returns the DrtVect of the presuppositions, and the dummy DrtPred of the conjunctions
{
	DrtVect orig_drtvect(drtvect);
	vector<DrtVect> to_return;
	metric *d = metric_singleton::get_metric_instance();

	vector<pair<string, string> > ref_pair;
	vector<DrtPred> conjunctions;

	// Check if there is a noun with [S]
	// and create a DrtVect with a dummy verb instead of the noun
	vector<string> refs;
	map<string, vector<DrtPred> > map_ref;
	for (int n = 0; n < drtvect.size(); ++n) {
		if (drtvect.at(n).is_complement()) {
			string fref = extract_first_tag(drtvect.at(n));
			if (!is_broken(fref))
				continue;
			string implverb = fref;
			implverb = substitute_string(implverb, "verb", "name");
			implant_first(drtvect.at(n), implverb);
		}
	}

	if (drtvect != orig_drtvect)
		to_return.push_back(drtvect);

	return make_pair(to_return, conjunctions);
}

static pair<vector<DrtVect>, vector<DrtPred> > process_what_subordinates(DrtVect drtvect)
// David(_A) verb(_B,_A,none) @PREP(_B,_C) what(_C)
// -> David(_A) verb(_B,_A,none) @PREP(_B,_D) what_verb(_D,...)
// It returns the DrtVect of the presuppositions, and the dummy DrtPred of the conjunctions
{
	DrtVect orig_drtvect(drtvect);
	vector<DrtVect> to_return;
	metric *d = metric_singleton::get_metric_instance();

	vector<pair<string, string> > ref_pair;
	vector<DrtPred> conjunctions;

	// Create a DrtVect with a dummy verb instead of the noun
	vector<string> refs;
	map<string, vector<DrtPred> > map_ref;
	for (int n = 0; n < drtvect.size(); ++n) {
		string fref = extract_first_tag(drtvect.at(n));
		string sref = extract_second_tag(drtvect.at(n));
		if (drtvect.at(n).is_complement() && ref_is_verb(fref) && ref_is_name(sref)) {
			int wpos = find_element_with_string(drtvect, sref);
			if (wpos == -1)
				continue;
			if (drtvect.at(wpos).is_what()) {
				int vpos = find_verb_with_string(drtvect, fref);
				if (vpos == -1)
					continue;
				string orig_verb = extract_first_tag(drtvect.at(vpos));
				string implverb = fref;
				if (fref.find("name") != string::npos) {
					implverb.erase(0, 4);
				} else if (fref.find("ref") != string::npos) {
					implverb.erase(0, 3);
				}
				string vref, sref, oref;
				vref = string("_[presupp]verb") + implverb;
				sref = string("_[presupp]namesubj") + implverb;
				oref = string("_[presupp]nameobj") + implverb;
				DrtPred tmpverb(string("[*]/V(") + vref + "," + sref + "," + oref + ")");

				tmpverb.set_question();
				tmpverb.set_question_word(drtvect.at(wpos).get_question_word());

				implant_second(drtvect.at(n), vref);
				drtvect.at(wpos) = tmpverb;

				DrtPred tmp_conj(string("@COORDINATION(" + orig_verb + "," + vref + ")"));
				conjunctions.push_back(tmp_conj);
			}
		}
	}

	if (drtvect != orig_drtvect)
		to_return.push_back(drtvect);

	return make_pair(to_return, conjunctions);
}

static pair<vector<DrtVect>, vector<DrtPred> > add_what_subordinates(DrtVect drtvect)
// David(_A) verb(_B,_A,_C) what(_C)
// -> David(_A) verb(_B,_A,none) @SUBORD(_B,_D) what_verb(_D,...)
// It returns the DrtVect of the presuppositions, and the dummy DrtPred of the conjunctions
{
	DrtVect orig_drtvect(drtvect);
	vector<DrtVect> to_return;
	metric *d = metric_singleton::get_metric_instance();

	vector<pair<string, string> > ref_pair;
	vector<DrtPred> conjunctions;

	// Create a DrtVect with a dummy verb instead of the noun
	vector<string> refs;
	map<string, vector<DrtPred> > map_ref;
	for (int n = 0; n < drtvect.size(); ++n) {
		if (drtvect.at(n).is_what()) {
			string fref = extract_first_tag(drtvect.at(n));
			int vpos = find_verb_with_object(drtvect, fref);
			if (vpos == -1)
				continue;
			string orig_verb = extract_first_tag(drtvect.at(vpos));

			string implverb = fref;
			if (fref.find("name") != string::npos) {
				implverb.erase(0, 4);
			} else if (fref.find("ref") != string::npos) {
				implverb.erase(0, 3);
			}
			string vref, sref, oref;
			vref = string("_[presupp]verb") + implverb;
			sref = string("_[presupp]namesubj") + implverb;
			oref = string("_[presupp]nameobj") + implverb;
			DrtPred tmpverb(string("[*]/V(") + vref + "," + sref + "," + oref + ")");

			//if(drtvect.at(n).is_question()) {
			tmpverb.set_question();
			tmpverb.set_question_word(drtvect.at(n).get_question_word());
			//}

			ref_pair.push_back(make_pair(fref, vref));
			drtvect.at(n) = tmpverb;

			// adds the new verb as a subordinate
			DrtPred tmp_subord("@SUBORD|@SUB-OBJ|@TOPIC(" + orig_verb + "," + vref + ")");
			drtvect.push_back(tmp_subord);
			implant_object(drtvect.at(vpos), "_none");

			int m = find_complement_with_second_tag(drtvect, fref);
			if (m != -1) {
				string orig_verb_ref = extract_first_tag(drtvect.at(m));
				DrtPred tmp_conj(string("@COORDINATION(" + orig_verb_ref + "," + vref + ")"));
				conjunctions.push_back(tmp_conj);
			}
		}
	}

	for (int n = 0; n < ref_pair.size(); ++n) {
		string ref = ref_pair.at(n).first;
		string name = ref_pair.at(n).second;
		drtvect = substitute_ref(drtvect, ref, name);
	}
	if (drtvect != orig_drtvect)
		to_return.push_back(drtvect);

	return make_pair(to_return, conjunctions);
}

static pair<vector<DrtVect>, vector<DrtPred> > process_question_WRB(DrtVect drtvect)
// @TIME_AT(_verb,_name) -> @TIME_AT(_verb,_verb)
{
	DrtVect orig_drtvect(drtvect);
	vector<DrtVect> to_return;
	metric *d = metric_singleton::get_metric_instance();

	vector<pair<string, string> > ref_pair;
	vector<DrtPred> conjunctions;

	vector<string> refs;
	map<string, vector<DrtPred> > map_ref;
	for (int n = 0; n < drtvect.size(); ++n) {
		if (drtvect.at(n).is_question() && drtvect.at(n).is_complement()) {
			string sref = extract_second_tag(drtvect.at(n));
			string head = extract_header(drtvect.at(n));
			if (head.find("@TIME") == string::npos && head.find("@PLACE") == string::npos
					&& head.find("@CAUSED_BY") == string::npos)
				continue;
			vector<int> m = find_all_element_with_string(drtvect, sref);
			if (m.size() == 0 && !ref_is_verb(sref)) {
				implant_second(drtvect.at(n), "_[presupp]verb");
			}
		}
	}

	DrtPred tmp_conj(string("@COORDINATION(dummy1,dummy2)"));
	conjunctions.push_back(tmp_conj);
	if (drtvect != orig_drtvect)
		to_return.push_back(drtvect);

	return make_pair(to_return, conjunctions);
}

static pair<vector<DrtVect>, vector<DrtPred> > process_question_HOW(DrtVect drtvect)
// @CAUSED_BY#[name:how](C,B) [*]/NN(A) be/V(C,A,D) 
// -> [*]/NN(A) be/V(C,A,B) [*]/JJ(B)
{
	DrtVect orig_drtvect(drtvect);
	vector<DrtVect> to_return;
	metric *d = metric_singleton::get_metric_instance();

	vector<pair<string, string> > ref_pair;
	vector<DrtPred> conjunctions;

	vector<string> refs;
	map<string, vector<DrtPred> > map_ref;
	for (int n = 0; n < drtvect.size(); ++n) {
		if (drtvect.at(n).is_question() && drtvect.at(n).is_complement() && drtvect.at(n).name() == "how") {
			string fref = extract_first_tag(drtvect.at(n));
			string sref = extract_second_tag(drtvect.at(n));
			string head = extract_header(drtvect.at(n));
			int m = find_verb_with_string(drtvect, fref);
			if (m != -1 && !ref_is_verb(sref)) {
				implant_object(drtvect.at(m), sref);
				DrtPred adj_pred(string("[JJ]/JJ") + "(" + sref + ")");
				adj_pred.set_question();
				adj_pred.set_question_word(drtvect.at(n).get_question_word());
				drtvect.at(n) = adj_pred;
			}
		}
	}

	DrtPred tmp_conj(string("@COORDINATION(dummy1,dummy2)"));
	conjunctions.push_back(tmp_conj);
	if (drtvect != orig_drtvect)
		to_return.push_back(drtvect);

	return make_pair(to_return, conjunctions);
}

static pair<vector<DrtVect>, vector<DrtPred> > process_question_complements(DrtVect drtvect)
// David(_A) go(_B,_A,none) @PLACE_AT(_B,_name) ...|[S]/N(_C)
// -> David(_A) go(_B,_A,none) @PLACE_AT(_B,_verb) 
// It returns the DrtVect of the presuppositions, and the dummy DrtPred of the conjunctions
{
	vector<DrtVect> to_return;
	metric *d = metric_singleton::get_metric_instance();

	vector<pair<string, string> > ref_pair;
	vector<DrtPred> conjunctions;

	// Check if there is a noun with [S]
	// and create a DrtVect with a dummy verb instead of the noun
	vector<string> refs;
	map<string, vector<DrtPred> > map_ref;
	for (int n = 0; n < drtvect.size(); ++n) {
		if (drtvect.at(n).is_complement()) {
			// @PLACE_AT(verb,name) -> @PLACE_AT(verb,[presupp]verb) @[*]/V([presupp]verb)
			string fref = extract_first_tag(drtvect.at(n));
			string implverb = extract_second_tag(drtvect.at(n));
			if (ref_is_verb(implverb))
				continue;
			if (implverb.find("name") != string::npos) {
				implverb.erase(0, 4);
			} else if (implverb.find("ref") != string::npos) {
				implverb.erase(0, 3);
			}
			string vref = fref;

			implant_second(drtvect.at(n), string("_[presupp]verb") + implverb);

			int m = find_complement_with_second_tag(drtvect, fref);
			if (m != -1) {
				string orig_verb_ref = extract_first_tag(drtvect.at(m));
				DrtPred tmp_conj(string("@COORDINATION(" + orig_verb_ref + "," + vref + ")"));
				conjunctions.push_back(tmp_conj);
			}
		}
	}

	for (int n = 0; n < ref_pair.size(); ++n) {
		string ref = ref_pair.at(n).first;
		string name = ref_pair.at(n).second;
		drtvect = substitute_ref(drtvect, ref, name);
	}

	to_return.push_back(drtvect);

	return make_pair(to_return, conjunctions);
}

static string get_first_verb_ref(const DrtVect &drtvect)
{
	string to_return = "";
	DrtVect::const_iterator diter = drtvect.begin(), dend = drtvect.end();
	for (; diter != dend; ++diter) {
		if (diter->is_verb()) {
			to_return = extract_first_tag(*diter);
			break;
		}
	}
	return to_return;
}

static pair<vector<DrtVect>, vector<DrtPred> > process_question_correlated_subs(DrtVect drtvect)
// David(_A) win(_B,_A,_C) medal(_C) be(_D,_C,_E) bronze(_E)
// -> David(_A) win(_B,_A,_C) medal(_C) @CONJUNCTION(_B,_D) medal(_C) be(_D,_C,_E) bronze(_E)
// It returns the DrtVect of the presuppositions, and the DrtPred of the conjunctions
/// Now included in process_questions_WRB
{
	vector<DrtVect> to_return;
	metric *d = metric_singleton::get_metric_instance();

	vector<pair<string, string> > ref_pair;
	vector<DrtPred> conjunctions;

	vector<DrtVect> drtvects = get_linked_drtvect_from_single_drtvect(drtvect);
	vector<string> verb_ref_list;

	if (drtvects.size() > 1) {
		for (int n = 0; n < drtvects.size(); ++n) {
			to_return.push_back(drtvects.at(n));
			string verb_ref = get_first_verb_ref(drtvects.at(n));
			verb_ref_list.push_back(verb_ref);
		}
	}

	for (int n = 0; n < verb_ref_list.size() - 1; ++n) {
		string fref = verb_ref_list.at(n);
		string sref = verb_ref_list.at(n + 1);
		if (fref != "" && sref != "") {
			DrtPred conj("@CONJUNCTION(" + fref + "," + sref + ")");
			conjunctions.push_back(conj);
		}
	}

	to_return.push_back(drtvect);

	return make_pair(to_return, conjunctions);
}

static pair<vector<DrtVect>, vector<DrtPred> > process_question_verb_to_noun(DrtVect drtvect)
// cost/VB of living -> cost/NN of living
{
	vector<DrtVect> to_return;
	metric *d = metric_singleton::get_metric_instance();

	vector<pair<string, string> > ref_pair;
	vector<DrtPred> conjunctions;

	tagger_info *info = parser_singleton::get_tagger_info_instance();

	for (int n = 0; n < drtvect.size()-1; ++n) {
		if(drtvect.at(n).is_verb() ) {
			string header= extract_header(drtvect.at(n));
			string next_header= extract_header(drtvect.at(n+1));
			string next_tag= drtvect.at(n+1).tag();
			string fref= extract_first_tag(drtvect.at(n));
			string next_fref= extract_first_tag(drtvect.at(n+1));
			if( info->is_candidate_name(header) && next_header == "of" && next_tag == "IN" && fref == next_fref ) {
				string new_ref= "[presupp_verb_to_noun]name_"+boost::lexical_cast<string>(n);
				DrtPred new_noun_pred(header+"/NN(" + new_ref+ ")");
				drtvect.at(n) = new_noun_pred;
				implant_first(drtvect.at(n+1),new_ref);
			}
		}
	}

	to_return.push_back(drtvect);

	return make_pair(to_return, conjunctions);
}


string get_numbers(const string &ref)
{
	string implverb = ref;
	if (implverb.find("name") != string::npos) {
		implverb.erase(0, 4);
	} else if (implverb.find("ref") != string::npos) {
		implverb.erase(0, 3);
	} else if (implverb.find("verb") != string::npos) {
		implverb.erase(0, 4);
	}
	return implverb;
}

static vector<DrtVect> process_family(DrtVect drtvect)
// @OWNED_BY(A,B) brother(A) Daniel(A) -> Daniel(A) be(verb,A,C) brother(C) @GENITIVE(C,D) person(B)
// creates family relations
{
	vector<DrtVect> to_return;
	metric *d = metric_singleton::get_metric_instance();

	if (drtvect.size() == 0)
		return to_return;

	string numbers = get_numbers(extract_first_tag(drtvect.at(0)));
	string implverb = string("[presupp-family]verb") + numbers;
	string implobj = string("[presupp-family]name") + numbers;

	DrtVect to_match = create_drtvect("brother/NN(_A), person/NN(_A), @OWNED_BY(_A,_B)");
	vector<DrtVect> results;
	DrtVect result;
	result = create_drtvect(
			string("person/NN(_A), be/V(") + implverb + "_1,_A," + implobj + "_1)," + "=brother/NN(" + implobj
					+ "_1), @GENITIVE(" + implobj + "_1,_B)," + "person/NN(_B)");
	results.push_back(result);
	result = create_drtvect(
			string("person/NN(_B), be/V(") + implverb + "_2,_B," + implobj + "_2)," + "=brother/NN(" + implobj
					+ "_2), @GENITIVE(" + implobj + "_2,_A)," + "person/NN(_A)");
	results.push_back(result);
	result = create_drtvect(
			string("person/NN(_A), have/V(") + implverb + "_3,_A," + implobj + "_3)," + "=brother/NN(" + implobj + "_3)");
	results.push_back(result);
	result = create_drtvect(
			string("person/NN(_B), have/V(") + implverb + "_4,_B," + implobj + "_4)," + "=brother/NN(" + implobj + "_4)");
	results.push_back(result);

	Knowledge k; /// should be imported from above!
	Match match(&k);
	MatchSubstitutions msubs;
	double w = match.singlePhraseMatch(drtvect, to_match, &msubs);
	if (w > 0.5) {
		for (int n = 0; n < results.size(); ++n) {
			result = results.at(n);
			result / msubs.getDrtMgu();
			to_return.push_back(result);
		}
	}
	return to_return;
}

static vector<DrtVect> process_parentheticals(DrtVect drtvect)
// nelson(A) @PAR(A,B) rear-admiral(B) ... -> nelson(A) be(C,A,B) read-admiral(B)
// creates family relations
{
	vector<DrtVect> to_return;
	metric *d = metric_singleton::get_metric_instance();

	if (drtvect.size() == 0)
		return to_return;

	string numbers = get_numbers(extract_first_tag(drtvect.at(0)));
	// string implverb= string("[presupp-par]verb") + numbers;
	string implverb = string("[parenthetical]verb") + numbers;

	for (int n = 0; n < drtvect.size(); ++n) {
		string head = extract_header(drtvect.at(n));
		string fref = extract_first_tag(drtvect.at(n));
		string sref = extract_second_tag(drtvect.at(n));
		if (head == "@PAR" && ref_is_name(fref) && ref_is_name(sref)) {
			int msubj = find_name_with_string(drtvect, fref);
			int mobj = find_name_with_string(drtvect, sref);
			if (msubj == -1 || mobj == -1)
				continue;
			DrtVect new_drtvect;
			new_drtvect.push_back(drtvect.at(msubj));
			DrtPred new_verb(string("be/V(") + implverb + "," + fref + "," + sref + ")");
			new_drtvect.push_back(new_verb);
			new_drtvect.push_back(drtvect.at(mobj));

			to_return.push_back(new_drtvect);
		}
		if (head == "@PAR" && ref_is_verb(fref) && ref_is_verb(sref)) {
			// "this man, known as David, ...
			int fverb = find_verb_with_string(drtvect, fref);
			int sverb = find_verb_with_string(drtvect, sref);
			if (fverb == -1 || sverb == -1)
				continue;
			if (drtvect.at(sverb).tag() != "VBN")
				continue;
			string subj_ref = extract_subject(drtvect.at(fverb));
			implant_object(drtvect.at(sverb), subj_ref);
			DrtVect new_drtvect = find_all_attached_to_verb(drtvect, sverb);
			to_return.push_back(new_drtvect);
		}
		if (head == "@AND" && ref_is_name(fref) && ref_is_name(sref)) {
			// "... the president of Italy, Giorgio Napolitano ..."
			int msubj = find_name_with_string(drtvect, fref);
			int mobj = find_name_with_string(drtvect, sref);
			if (msubj == -1 || mobj == -1)
				continue;

			if ((drtvect.at(msubj).is_proper_name() && !drtvect.at(mobj).is_proper_name())
					|| (!drtvect.at(msubj).is_proper_name() && drtvect.at(mobj).is_proper_name())) {
				DrtVect new_drtvect;
				new_drtvect.push_back(drtvect.at(msubj));
				DrtPred new_verb(string("be/V(") + implverb + "," + fref + "," + sref + ")");
				new_drtvect.push_back(new_verb);
				new_drtvect.push_back(drtvect.at(mobj));
				to_return.push_back(new_drtvect);
			}
		}

	}

	return to_return;
}


static vector<DrtVect> process_genitive_to_have(DrtVect drtvect)
// the flag of a country is red -> the country has a red flag.
{
	vector<DrtVect> to_return;
	metric *d = metric_singleton::get_metric_instance();

	// extracts the adjectives
	vector<string> refs;
	vector<tuple<DrtPred,DrtPred,DrtPred> > item_genitive_adj_vect;
	for (int n = 0; n < drtvect.size(); ++n) {
		if (drtvect.at(n).is_adjective()) {
			string head = extract_header(drtvect.at(n));
			if (d->pertains_to_name(head, "color", 6) > 0.5
				|| d->hypernym_dist(head, "color", 8) > 0.5
			) {
				if(debug) {
					cout << "PHAVE1::: " << drtvect.at(n) << endl;
				}
				string oref = extract_first_tag(drtvect.at(n));
				int mverb = find_verb_with_object(drtvect,oref);
				if(mverb == -1)
					continue;
				if(debug) {
					cout << "PHAVE2::: " << drtvect.at(mverb) << endl;
				}
				int msubj = find_int_subject_of_verb(drtvect,mverb);
				if(msubj == -1)
					continue;
				string sref = extract_first_tag(drtvect.at(msubj) );
				int mcompl = find_complement_with_first_tag(drtvect,sref,"@GENITIVE");
				if(mcompl == -1)
					continue;
				if(debug) {
					cout << "PHAVE3::: " << drtvect.at(mcompl) << endl;
				}
				int mgen = find_element_with_string(drtvect,extract_second_tag(drtvect.at(mcompl)) );
				if(mgen == -1)
					continue;
				if(debug) {
					cout << "PHAVE4::: " << drtvect.at(mgen) << endl;
				}
				item_genitive_adj_vect.push_back(make_tuple(drtvect.at(msubj),drtvect.at(mgen),drtvect.at(n) ) );
			}
		}
	}

	// create DrtVect of the type "<genitive> has an <adj> <item>"
	for (int n = 0; n < item_genitive_adj_vect.size(); ++n) {
		DrtPred gen_pred  = item_genitive_adj_vect.at(n).get<1>();
		DrtPred adj_pred  = item_genitive_adj_vect.at(n).get<2>();
		DrtPred item_pred = item_genitive_adj_vect.at(n).get<0>();
		string numbers = get_numbers(extract_first_tag(gen_pred));
		string num_str = boost::lexical_cast<string>(n);
		DrtPred have_verb("have/VB([presupp_have_verb]verb_"+numbers+"_"+num_str+ ",2,3)");
		implant_subject(have_verb,extract_first_tag(gen_pred) );
		implant_object (have_verb,extract_first_tag(item_pred) );
		implant_first  (adj_pred,extract_first_tag(item_pred) );
		adj_pred.setTag("JJ");
		adj_pred.set_pivot(false);
		DrtVect tmp_drt;
		tmp_drt.push_back(gen_pred);
		tmp_drt.push_back(have_verb);
		tmp_drt.push_back(adj_pred);
		tmp_drt.push_back(item_pred);
		to_return.push_back(tmp_drt);
		if (debug) {
			puts("PRESUPP_HAVE:::");
			print_vector(tmp_drt);
		}
	}

	return to_return;
}


void Presupposition::compute()
{
	if (orig_drt_.has_question()) {
		DrtVect drtvect = orig_drt_.predicates_with_references();
		pair<vector<DrtVect>, vector<DrtPred> > answer_tmp;

		vector<drt> tmp_drts;
		vector<DrtVect> tmp_drtpreds;

		answer_tmp = process_question_WRB(drtvect);
		tmp_drtpreds.insert(tmp_drtpreds.end(), answer_tmp.first.begin(), answer_tmp.first.end());
		conjunctions_.insert(conjunctions_.end(), answer_tmp.second.begin(), answer_tmp.second.end());

		answer_tmp = process_question_HOW(drtvect);
		tmp_drtpreds.insert(tmp_drtpreds.end(), answer_tmp.first.begin(), answer_tmp.first.end());
		conjunctions_.insert(conjunctions_.end(), answer_tmp.second.begin(), answer_tmp.second.end());

		answer_tmp = process_question_subordinates(drtvect);
		tmp_drtpreds.insert(tmp_drtpreds.end(), answer_tmp.first.begin(), answer_tmp.first.end());
		conjunctions_.insert(conjunctions_.end(), answer_tmp.second.begin(), answer_tmp.second.end());

		answer_tmp = process_what_subordinates(drtvect);
		tmp_drtpreds.insert(tmp_drtpreds.end(), answer_tmp.first.begin(), answer_tmp.first.end());
		conjunctions_.insert(conjunctions_.end(), answer_tmp.second.begin(), answer_tmp.second.end());

		answer_tmp = process_what_subordinates_with_not(drtvect);
		tmp_drtpreds.insert(tmp_drtpreds.end(), answer_tmp.first.begin(), answer_tmp.first.end());
		conjunctions_.insert(conjunctions_.end(), answer_tmp.second.begin(), answer_tmp.second.end());

		answer_tmp = add_what_subordinates(drtvect);
		tmp_drtpreds.insert(tmp_drtpreds.end(), answer_tmp.first.begin(), answer_tmp.first.end());
		conjunctions_.insert(conjunctions_.end(), answer_tmp.second.begin(), answer_tmp.second.end());

		answer_tmp = process_question_broken_subordinates(drtvect);
		tmp_drtpreds.insert(tmp_drtpreds.end(), answer_tmp.first.begin(), answer_tmp.first.end());
		conjunctions_.insert(conjunctions_.end(), answer_tmp.second.begin(), answer_tmp.second.end());

		answer_tmp = process_question_verb_to_noun(drtvect);
		tmp_drtpreds.insert(tmp_drtpreds.end(), answer_tmp.first.begin(), answer_tmp.first.end());
		conjunctions_.insert(conjunctions_.end(), answer_tmp.second.begin(), answer_tmp.second.end());

		answer_tmp = process_question_about(drtvect);
		tmp_drtpreds.insert(tmp_drtpreds.end(), answer_tmp.first.begin(), answer_tmp.first.end());
		conjunctions_.insert(conjunctions_.end(), answer_tmp.second.begin(), answer_tmp.second.end());

		answer_tmp = process_question_indirect_object(drtvect);
		tmp_drtpreds.insert(tmp_drtpreds.end(), answer_tmp.first.begin(), answer_tmp.first.end());
		conjunctions_.insert(conjunctions_.end(), answer_tmp.second.begin(), answer_tmp.second.end());

		answer_tmp = process_question_kind_of(drtvect);
		tmp_drtpreds.insert(tmp_drtpreds.end(), answer_tmp.first.begin(), answer_tmp.first.end());
		conjunctions_.insert(conjunctions_.end(), answer_tmp.second.begin(), answer_tmp.second.end());

		answer_tmp = process_question_age_subord(drtvect);
		tmp_drtpreds.insert(tmp_drtpreds.end(), answer_tmp.first.begin(), answer_tmp.first.end());
		conjunctions_.insert(conjunctions_.end(), answer_tmp.second.begin(), answer_tmp.second.end());

		answer_tmp = process_question_expand_genitive(drtvect);
		tmp_drtpreds.insert(tmp_drtpreds.end(), answer_tmp.first.begin(), answer_tmp.first.end());
		conjunctions_.insert(conjunctions_.end(), answer_tmp.second.begin(), answer_tmp.second.end());

		answer_tmp = process_question_verbless(drtvect);
		tmp_drtpreds.insert(tmp_drtpreds.end(), answer_tmp.first.begin(), answer_tmp.first.end());
		conjunctions_.insert(conjunctions_.end(), answer_tmp.second.begin(), answer_tmp.second.end());


		for (int n = 0; n < tmp_drtpreds.size(); ++n) {
			// new predicates are created, they need to be set as questions
			if(tmp_drtpreds.at(n) == drtvect)
				continue; // if the question is identical to the original question -> skip
			QuestionList ql;
			ql.add(tmp_drtpreds.at(n));
			drt tmpdrt(tmp_drtpreds.at(n));
			tmpdrt.setQuestionList(ql);
			tmp_drts.push_back(tmpdrt);
			if (debug) {
				puts("IMPLIED_QUESTIONS:::");
				print_vector(tmp_drtpreds.at(n));
			}
		}

		answers_.insert(answers_.end(), tmp_drts.begin(), tmp_drts.end());

	} else {
		DrtVect drtvect = orig_drt_.predicates_with_references();
		vector<DrtVect> answer_tmp;
		vector<drt> tmp_drts;
		vector<DrtVect> tmp_drtpreds;
		answer_tmp = process_generic_adj(drtvect);
		tmp_drtpreds.insert(tmp_drtpreds.end(), answer_tmp.begin(), answer_tmp.end());
		answer_tmp = process_material_adj(drtvect);
		tmp_drtpreds.insert(tmp_drtpreds.end(), answer_tmp.begin(), answer_tmp.end());
		answer_tmp = process_size(drtvect);
		tmp_drtpreds.insert(tmp_drtpreds.end(), answer_tmp.begin(), answer_tmp.end());
		answer_tmp = process_size_from_quantities(drtvect);
		tmp_drtpreds.insert(tmp_drtpreds.end(), answer_tmp.begin(), answer_tmp.end());
		answer_tmp = process_family(drtvect);
		tmp_drtpreds.insert(tmp_drtpreds.end(), answer_tmp.begin(), answer_tmp.end());
		answer_tmp = process_parentheticals(drtvect);
		tmp_drtpreds.insert(tmp_drtpreds.end(), answer_tmp.begin(), answer_tmp.end());
		answer_tmp = process_is_when(drtvect);
		tmp_drtpreds.insert(tmp_drtpreds.end(), answer_tmp.begin(), answer_tmp.end());
		answer_tmp = process_nouns_from_verbs(drtvect);
		tmp_drtpreds.insert(tmp_drtpreds.end(), answer_tmp.begin(), answer_tmp.end());
		answer_tmp = process_genitive_to_have(drtvect);
		tmp_drtpreds.insert(tmp_drtpreds.end(), answer_tmp.begin(), answer_tmp.end());


		for (int n = 0; n < tmp_drtpreds.size(); ++n) {
			// new predicates are created, they need to have the original text in the drt
			if (debug) {
				puts("PRESUPPOSITION_SENTENCE:::");
				print_vector(tmp_drtpreds.at(n));
			}
			drt tmpdrt(tmp_drtpreds.at(n));
			tmpdrt.setText(orig_drt_.getText());
			tmp_drts.push_back(tmpdrt);
		}
		answers_.insert(answers_.end(), tmp_drts.begin(), tmp_drts.end());
	}
}

Presupposition::Presupposition(const drt &d) :
		orig_drt_(d)
{
	this->compute();
}
