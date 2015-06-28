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



#include"tools.hpp"
#include"drt_builder-corrector.hpp"

static const bool debug = false;

template<class T>
bool shortfind(const vector<T> &vect, const T &element)
{
	if (find(vect.begin(), vect.end(), element) == vect.end())
		return false;
	return true;
}

template<class T>
void print_vector(std::vector<T> &vs)
{
	typename vector<T>::iterator tags_iter = vs.begin();
	while (tags_iter < vs.end()) {
		std::cout << (*tags_iter) << " ";
		++tags_iter;
	}
	std::cout << std::endl;
}

static bool has_only_adjectives(const DrtVect &drtvect, const string &ref)
{
	for(int n = 0; n < drtvect.size(); ++n) {
		string fref = extract_first_tag(drtvect.at(n));
		if( fref == ref
				&& !drtvect.at(n).is_adjective()
				&& !drtvect.at(n).is_article()
				&& !drtvect.at(n).is_preposition()
				&& !drtvect.at(n).is_complement())
			return false;
	}
	return true;
}

static bool has_adjective(const DrtVect &drtvect, const string &ref)
{
	for(int n = 0; n < drtvect.size(); ++n) {
		string fref = extract_first_tag(drtvect.at(n));
		if( fref == ref && drtvect.at(n).is_adjective() )
			return true;
	}
	return false;
}

static bool has_verb(const DrtVect &drtvect, const string &ref)
{
	for(int n = 0; n < drtvect.size(); ++n) {
		string fref = extract_first_tag(drtvect.at(n));
		if( fref == ref && drtvect.at(n).is_verb() )
			return true;
	}
	return false;
}




vector<DrtPred> corrector(const vector<DrtPred> &pre_drt, vector<string> &tags, vector<string> &names,
		vector<pair<pair<int, int>, constituents> > &connections, bool is_question)
{
	vector < DrtPred > to_return(pre_drt);

	vector < pair<string, string> > ref_pairs;
	vector < string > to_new_verb;
	map<string, int> new_verb_prep;

	for (int n = 0; n < to_return.size(); ++n) {
		if (extract_header(to_return.at(n)) == "@TIME_AT") {
			string fref = extract_first_tag(to_return.at(n));
			string sref = extract_second_tag(to_return.at(n));
			if (fref == sref && ref_is_verb(fref)) {
				to_new_verb.push_back(fref);
				new_verb_prep[fref] = n;
			}
		}

		// Undelete @CAUSED_BY if it introduces a subordinate
		if (extract_header(to_return.at(n)).find("@CAUSED_BY:DELETE") != string::npos) {
			string r1 = extract_first_tag(to_return.at(n));
			string r2 = extract_second_tag(to_return.at(n));
			if (ref_is_verb(r1) && ref_is_verb(r2)) {
				implant_header(to_return.at(n), "@CAUSED_BY");
				to_return.at(n).name() = "@CAUSED_BY";
			}
		}
	}

	int unreferenced_pos = -1;
	int unreferenced_compl_pos = -1;
	for (int n = 0; n < to_return.size(); ++n) {
		if(to_return.at(n).is_complement() ) {
			if( !has_first_tag(to_return.at(n)) ) {
				string fref= extract_first_tag(to_return.at(n) );
				unreferenced_pos = get_single_distance(fref);
				unreferenced_compl_pos = n;
				break;
			}
		}
	}

	int pos1, pos2;
	string str1, str2, tag1, tag2;
	for (int n = 0; n < connections.size(); ++n) { // resolve conjunctions that do not have a first tag
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		str1 = to_return.at(pos1).name();
		str2 = to_return.at(pos2).name();
		tag1 = to_return.at(pos1).tag();
		tag2 = to_return.at(pos2).tag();


		if( tag1 == "IN" && pos1 == unreferenced_pos
		    && tag2 == "-comma-") {
			if( has_first_tag(to_return.at(pos1)) )
				continue;
			string sref2 = extract_second_tag(to_return.at(pos2));
			if( ref_is_verb(sref2) && unreferenced_compl_pos != -1) {
				implant_first(to_return.at(unreferenced_compl_pos),sref2);
			}
		}
	}


	for (int n = 0; n < connections.size(); ++n) { // connects numerals and names with @QUANTITY(number,name)
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		str1 = to_return.at(pos1).name();
		str2 = to_return.at(pos2).name();
		tag1 = to_return.at(pos1).tag();
		tag2 = to_return.at(pos2).tag();

		string ref1 = extract_first_tag(to_return.at(pos1));
		vector<string>::iterator viter = find(to_new_verb.begin(), to_new_verb.end(), ref1);
		if (viter != to_new_verb.end()) {
			string vstr = extract_first_tag(to_return.at(pos2));
			if (vstr != ref1 && ref_is_verb(vstr)) {
				int m = new_verb_prep[ref1];
				implant_second(to_return.at(m), vstr);
				break;
			}
		}
		string ref2 = extract_first_tag(to_return.at(pos2));
		viter = find(to_new_verb.begin(), to_new_verb.end(), ref2);
		if (viter != to_new_verb.end()) {
			string vstr = extract_first_tag(to_return.at(pos1));
			if (vstr != ref2 && ref_is_verb(vstr)) {
				int m = new_verb_prep[ref2];
				implant_second(to_return.at(m), vstr);
				break;
			}
		}
	}

	for (int n = 0; n < to_return.size(); ++n) {
		if (to_return.at(n).is_complement()) {
			if (extract_header(to_return.at(n)) == "@CONJUNCTION") {
				string fref = extract_first_tag(to_return.at(n));
				string sref = extract_second_tag(to_return.at(n));
				ref_pairs.push_back(make_pair(fref, sref));
			}
			if (extract_header(to_return.at(n)).find("@SUB") != string::npos) {
				// A verb cannot be the object of another verb if they are in conjunction
				string fref = extract_first_tag(to_return.at(n));
				string sref = extract_second_tag(to_return.at(n));
				pair < string, string > tmp_pair = make_pair(fref, sref);
				if (find(ref_pairs.begin(), ref_pairs.end(), tmp_pair) != ref_pairs.end()) {
					add_header(to_return.at(n), ":DELETE");
				}
			}
		}
	}

	for (int n = 0; n < to_return.size(); ++n) {
		string head = extract_header(to_return.at(n));
		if (head == "@SUB-OBJ") {
			// Erase complements like @SUB-OBJ(name,verb)
			string fref = extract_first_tag(to_return.at(n));
			string sref = extract_second_tag(to_return.at(n));
			if (!ref_is_verb(fref) || !ref_is_verb(sref)) {
				add_header(to_return.at(n), ":DELETE");
			}
		}
	}
	for (int n = 0; n < to_return.size(); ++n) {
		if (to_return.at(n).is_complement()) {
			// Erase complements like @CONJUNCTION(A,A)
			string fref = extract_first_tag(to_return.at(n));
			string sref = extract_second_tag(to_return.at(n));
			if (fref == sref) {
				add_header(to_return.at(n), ":DELETE");
			}
		}
	}

	// sometimes the second name of a combination A @AND B is a subject. It is better if the first becomes the subject
	/// INCOMPLETE!! This does not take into account a name that is subject in two verbs: "if A is happy then B and A smile"
	for (int n = 0; n < to_return.size(); ++n) {
		string head = extract_header(to_return.at(n));
		if (head == "@AND" || head == "@OR") {
			string fref = extract_first_tag(to_return.at(n));
			string sref = extract_second_tag(to_return.at(n));
			int mf = find_verb_with_subject(to_return, tags, fref);
			int ms = find_verb_with_subject(to_return, tags, sref);
			if (mf == -1 && ms != -1) {
				switch_children(to_return.at(n));
				continue;
			}
			// same for objects
			mf = find_verb_with_object(to_return, tags, fref);
			ms = find_verb_with_object(to_return, tags, sref);
			if (mf == -1 && ms != -1) {
				switch_children(to_return.at(n));
			}
		}
	}

	for (int n = 0; n < to_return.size(); ++n) {
		string head = extract_header(to_return.at(n));
		if (head == "@AND" || head == "@OR") {
			string sref = extract_second_tag(to_return.at(n));
			vector<int> mm = find_all_compl_with_second_tag(to_return, tags, sref, "@AND");
			if (mm.size() == 0)
				mm = find_all_compl_with_second_tag(to_return, tags, sref, "@OR");
			if (mm.size() > 1) {
				switch_children(to_return.at(n));
			}
		}
	}

	// isolated elements (like "person" should be deleted)
	for (int n = 0; n < to_return.size(); ++n) {
		string head = extract_header(to_return.at(n));
		if (head == "person") {
			string fref = extract_first_tag(to_return.at(n));
			vector<int> m = find_all_element_with_string_everywhere(to_return, fref);
			if (m.size() == 1) { // there is only this element
				add_header(to_return.at(n), ":DELETE");
				to_return.at(n).name() += ":DELETE";
			}
		}
	}

	// !person|animal|.... associated with a name is deleted
	for (int n = 0; n < to_return.size(); ++n) {
		string head = extract_header(to_return.at(n));
		if (head == get_local_what()) {
			string fref = extract_first_tag(to_return.at(n));
			vector<int> m = find_all_names_with_string_no_delete(to_return, fref);
			if (m.size() > 1) { // there is only this element
				add_header(to_return.at(n), ":DELETE");
				to_return.at(n).name() += ":DELETE";
			}
		}
	}
	// verb(A,B,none) @DATIVE(A,C) name(C) -> verb(A,B,C) name(C)
	// it also corrects verb(A,B,C) @DATIVE(A,C) name(C) -> verb(A,B,C) name(C)
	for (int n = 0; n < to_return.size(); ++n) {
		string head = extract_header(to_return.at(n));
		if (head == "@DATIVE") {
			string fref = extract_first_tag(to_return.at(n));
			string sref = extract_second_tag(to_return.at(n));
			int m = find_verb_with_string(to_return, tags, fref);
			if (m != -1) { //
				string oref = extract_object(to_return.at(m));
				if (oref.find("none") != string::npos || oref == sref) {
					add_header(to_return.at(n), ":DELETE");
					to_return.at(n).name() += ":DELETE";
					implant_object(to_return.at(m), sref);
				}
			}
		}
	}
	// @GENITIVE(A,B) NAME(A) VERB(C,D,...) -> @GENITIVE(C,A) NAME(A) VERB(C,D,...)
	for (int n = 0; is_question && n < to_return.size(); ++n) {
		string head = extract_header(to_return.at(n));
		if (to_return.at(n).is_complement() && head.find("@QUANTITY") == string::npos
				&& head.find("@QUANTIFIER") == string::npos && !to_return.at(n).is_WP_pos()) {
			string fref = extract_first_tag(to_return.at(n));
			string sref = extract_second_tag(to_return.at(n));
			int opos = find_verb_with_object(to_return, tags, fref);
			int spos = find_verb_with_subject(to_return, tags, fref);
			int cpos = find_complement_with_target(to_return, fref);
			if (opos != -1 || spos != -1 || cpos != -1)
				continue;
			vector<int> m = find_all_element_with_string(to_return, sref);
			if (m.size() == 0 && ref_is_name(fref)) { // there is only this element
				string vref = get_first_verb_ref(to_return);
				implant_second(to_return.at(n), vref);
				switch_children(to_return.at(n));
			}
		}
	}

	// verb1(A,B,C) @SUB-OBJ(A,D) verb2(D,E,none) -> verb1(A,B,C) verb2(D,E,C)
	for (int n = 0; n < to_return.size(); ++n) {
		if (to_return.at(n).is_complement()) {
			string fref = extract_first_tag(to_return.at(n));
			string sref = extract_second_tag(to_return.at(n));
			string head = extract_header(to_return.at(n));
			if (head == "@SUB-OBJ" && ref_is_verb(fref) && ref_is_verb(sref)) {
				int fverb_pos = find_verb_with_string(to_return, fref);
				int sverb_pos = find_verb_with_string(to_return, sref);
				if (fverb_pos != -1 && sverb_pos != -1) {
					string sobj = extract_object(to_return.at(sverb_pos));
					string fobj = extract_object(to_return.at(fverb_pos));
					string fsubj = extract_subject(to_return.at(fverb_pos));
					if (!has_subject(to_return.at(sverb_pos))) {
						if (has_subject(to_return.at(fverb_pos))) {
							implant_subject(to_return.at(sverb_pos), fsubj);
							add_header(to_return.at(n), ":DELETE");
							to_return.at(n).name() += ":DELETE";
						}
						continue;
					}
					if (!has_object(to_return.at(sverb_pos))) {
						string s2 = extract_subject(to_return.at(sverb_pos));
						if (has_object(to_return.at(fverb_pos)) && fobj != s2) {
							implant_object(to_return.at(sverb_pos), fobj);
							add_header(to_return.at(n), ":DELETE");
							to_return.at(n).name() += ":DELETE";
						}
					}
				}
			}
		}
	}

	// @PAR that are not associated are deleted
	for (int n = 0; n < to_return.size(); ++n) {
		if (to_return.at(n).is_complement()) {
			string fref = extract_first_tag(to_return.at(n));
			string sref = extract_second_tag(to_return.at(n));
			string head = extract_header(to_return.at(n));
			if (head == "@PAR" && (fref.find("prev") != string::npos || sref.find("next") != string::npos))
				add_header(to_return.at(n), ":DELETE");
		}
	}
	// @CONJUNCTIONs that is associated to a noun should be deleted
	for (int n = 0; n < to_return.size(); ++n) {
		if (to_return.at(n).is_complement()) {
			string fref = extract_first_tag(to_return.at(n));
			string sref = extract_second_tag(to_return.at(n));
			string head = extract_header(to_return.at(n));
			if (head == "@CONJUNCTION" && ref_is_name(fref) && ref_is_verb(sref))
				add_header(to_return.at(n), ":DELETE");
			if (head == "@CONJUNCTION" && ref_is_name(sref) && ref_is_verb(fref))
				add_header(to_return.at(n), ":DELETE");
		}
	}


	// Subordinate verbs with no subjects borrow the subject from the parent verb
	for (int n = 0; n < to_return.size(); ++n) {
		if (to_return.at(n).is_complement()) {
			string fref = extract_first_tag(to_return.at(n));
			string sref = extract_second_tag(to_return.at(n));
			string head = extract_header(to_return.at(n));
			if (head == "@SUBORD" && ref_is_verb(fref) && ref_is_verb(sref)) {
				int fverb = find_verb_with_string(to_return, fref);
				int sverb = find_verb_with_string(to_return, sref);
				if (fverb == -1 || sverb == -1)
					continue;
				if (is_passive(to_return.at(fverb)) && !has_subject(to_return.at(sverb)) && has_object(to_return.at(fverb))) {
					implant_subject(to_return.at(sverb), extract_object(to_return.at(fverb)));
					continue;
				}
				if (!is_passive(to_return.at(sverb)) && !has_subject(to_return.at(sverb))
						&& has_subject(to_return.at(fverb))) {
					implant_subject(to_return.at(sverb), extract_subject(to_return.at(fverb)));
				}
			}
		}
	}

	if (debug)
		print_vector (to_return);

	// Percolate the target of a complement to the top of an @AND list
	for (int n = 0, safe = 0; n < to_return.size() && safe < 100; ++n) {
		string sref = extract_second_tag(to_return.at(n));
		string head = extract_header(to_return.at(n));
		if (to_return.at(n).is_complement() && head != "@AND" && head != "@OR") {
			vector<int> mm = find_all_compl_with_second_tag(to_return, tags, sref, "@AND");
			if (mm.size() == 0)
				mm = find_all_compl_with_second_tag(to_return, tags, sref, "@OR");
			if (mm.size() == 0)
				continue;
			string fref = extract_first_tag(to_return.at(mm.at(0)));
			implant_second(to_return.at(n), fref);
			n = 0;
			++safe;
		}
	}

	// @TIME_AT(A,B) @CONJUNCTION(B,A) -> @CONJUNCTION(B,A)
	for (int n = 0; n < to_return.size(); ++n) {
		string fref = extract_first_tag(to_return.at(n));
		string sref = extract_second_tag(to_return.at(n));
		string head = extract_header(to_return.at(n));
		if (to_return.at(n).is_complement() && head == "@CONJUNCTION") {
			vector<int> mm = find_all_compl_with_first_and_second_tag(to_return, tags, sref, fref);
			for (int j = 0; j < mm.size(); ++j) {
				string head2 = extract_header(to_return.at(mm.at(j)));
				if (head2 != "@CAUSED_BY" && head2 != "@TIME_FROM" && head2 != "@PLACE_AT")
					add_header(to_return.at(mm.at(j)), ":DELETE");
				else
					add_header(to_return.at(n), ":DELETE");
			}
		}
	}

	// verb(a,b,-) @prep(a,c) name(c) -comma-(a,d) name(d) -> verb(a,b,c) name(c) @prep(a,c) -comma-(c,d) name(d)
	for (int n = 0; n < to_return.size(); ++n) {
		string fref = extract_first_tag(to_return.at(n));
		string sref = extract_second_tag(to_return.at(n));
		string head = extract_header(to_return.at(n));
		if (head == "-comma-" && ref_is_verb(fref) && !ref_is_verb(sref)) {
			int m = find_verb_with_string(to_return, fref);
			if (m == -1)
				continue;
			if (n == 0)
				continue;
			string newref = extract_first_tag(to_return.at(n - 1));
			implant_first(to_return.at(n), newref);
			implant_header(to_return.at(n), "@AND");
		}
	}

	// A verb with a verb as an object should not be here (LAST TO DO!)
	for (int n = 0; n < to_return.size(); ++n) {
		string sref = extract_object(to_return.at(n));
		if (to_return.at(n).is_verb() && ref_is_verb(sref)) {
			implant_object(to_return.at(n), "none");
		}
	}

	// say(A,B,...) @SUBORD(A,C) verb(C,...,...) -> say(A,B,...) @PARENT-ALLOCUTION(C,A) verb(C,...,...)
	for (int n = 0; n < to_return.size(); ++n) {
		string fref = extract_first_tag(to_return.at(n));
		string sref = extract_second_tag(to_return.at(n));
		string head = extract_header(to_return.at(n));
		if (to_return.at(n).is_complement() && head == "@SUBORD") {
			int m = find_verb_with_string(to_return, tags, fref);
			if (m == -1)
				continue;
			string vhead = extract_header(to_return.at(m));
			vector < string > cverbs = get_communication_verbs();
			if (!shortfind(cverbs, vhead))
				continue;
			implant_header(to_return.at(n), "@PARENT-ALLOCUTION");
			switch_children(to_return.at(n));
			implant_object(to_return.at(m), "none"); // the object is reset
		}
	}

	// say(A,B,C) @PARENT-ALLOCUTION(D,A) ... -> say(A,B,none) @PARENT-ALLOCUTION(C,A) verb(C,...,...)
	for (int n = 0; n < to_return.size(); ++n) {
		string fref = extract_first_tag(to_return.at(n));
		string sref = extract_second_tag(to_return.at(n));
		string head = extract_header(to_return.at(n));
		if (to_return.at(n).is_complement() && head == "@PARENT-ALLOCUTION") {
			int m = find_verb_with_string(to_return, tags, sref);
			if (m == -1)
				continue;
			if (has_object(to_return.at(m)))
				implant_object(to_return.at(m), "none");
		}
	}

	// win(A,B,C) @DATIVE(A,C) -> win(A,B,C)
	for (int n = 0; n < to_return.size(); ++n) {
		string fref = extract_first_tag(to_return.at(n));
		string sref = extract_second_tag(to_return.at(n));
		string head = extract_header(to_return.at(n));
		if (to_return.at(n).is_complement() && head.find("@DATIVE") != string::npos) {
			int m = find_verb_with_string(to_return, tags, fref);
			if (m == -1)
				continue;
			string obj_ref = extract_object(to_return.at(m));
			if (obj_ref == sref) {
				add_header(to_return.at(n), ":DELETE");
			}
		}
	}

	if (debug)
		print_vector (to_return);
	// opt/V(verb1,subj,obj) @SUBORD(verb1,verb2) mown/V(verb1,name2,obj)
	// -> opt/V(verb1,name2,obj) @SUBORD(verb1,verb2) mown/V(verb1,name2,obj)
	for (int n = 0; n < to_return.size(); ++n) {
		string fref = extract_first_tag(to_return.at(n));
		string sref = extract_second_tag(to_return.at(n));
		string head = extract_header(to_return.at(n));
		if (to_return.at(n).is_complement()
				&& (head.find("@SUBORD") != string::npos || head.find("@CONJUNCTION") != string::npos)) {
			int m = find_verb_with_string(to_return, tags, fref);
			int m2 = find_verb_with_string(to_return, tags, sref);
			if (m == -1 || m2 == -1)
				continue;
			if (has_subject(to_return.at(m2)) && !has_subject(to_return.at(m))
					&& extract_header(to_return.at(m)).find("PASSIVE") == string::npos) {
				implant_subject_safe(to_return.at(m), extract_subject(to_return.at(m2)));
			}
		}
	}
	// passive verbs joined by a conjunction share agent and patient
	for (int n = 0; n < to_return.size(); ++n) {
		string fref = extract_first_tag(to_return.at(n));
		string sref = extract_second_tag(to_return.at(n));
		string head = extract_header(to_return.at(n));
		if (to_return.at(n).is_complement() && (head.find("@CONJUNCTION") != string::npos)) {
			int m = find_verb_with_string(to_return, tags, fref);
			int m2 = find_verb_with_string(to_return, tags, sref);
			if (m == -1 || m2 == -1)
				continue;
			if ((extract_header(to_return.at(m)).find("PASSIVE") == string::npos && to_return.at(m).tag() != "VBN")
					|| (extract_header(to_return.at(m2)).find("PASSIVE") == string::npos && to_return.at(m2).tag() != "VBN"))
				continue;
			if (has_subject(to_return.at(m2)) && !has_subject(to_return.at(m))) {
				implant_subject(to_return.at(m), extract_subject(to_return.at(m2)));
			}
			if (has_subject(to_return.at(m)) && !has_subject(to_return.at(m2))) {
				implant_subject(to_return.at(m2), extract_subject(to_return.at(m)));
			}
			if (has_object(to_return.at(m2)) && !has_object(to_return.at(m))) {
				implant_object(to_return.at(m), extract_object(to_return.at(m2)));
			}
			if (has_object(to_return.at(m)) && !has_object(to_return.at(m2))) {
				implant_object(to_return.at(m2), extract_object(to_return.at(m)));
			}
		}
	}

	// If a the verb "to be" has object but no subject switch subj and obj
	for (int n = 0; n < to_return.size(); ++n) {
		string fref = extract_first_tag(to_return.at(n));
		string sref = extract_second_tag(to_return.at(n));
		string head = extract_header(to_return.at(n));
		if (to_return.at(n).is_verb() && head == "be") {
			if (!has_subject(to_return.at(n)) && has_object(to_return.at(n))) {
				switch_subj_obj(to_return.at(n));
			}
		}
	}

	// the police and "[verbatim]" reported ...
	// -> a verbatim cannot be a subject
	for (int n = 0; n < to_return.size(); ++n) {
		string fref = extract_first_tag(to_return.at(n));
		string sref = extract_second_tag(to_return.at(n));
		string head = extract_header(to_return.at(n));
		if (to_return.at(n).is_verb() && head == "be") {
			if (!has_subject(to_return.at(n)) && has_object(to_return.at(n))) {
				switch_subj_obj(to_return.at(n));
			}
		}
	}

	// @AFTER(name,verb) -> @AFTER(verb,name)
	for (int n = 0; n < to_return.size(); ++n) {
		string fref = extract_first_tag(to_return.at(n));
		string sref = extract_second_tag(to_return.at(n));
		string head = extract_header(to_return.at(n));
		string tag = to_return.at(n).tag();
		if (to_return.at(n).is_complement()
			&& (head == "@AFTER" || head == "@BEFORE" || tag == "WRB")
				) {
			if (ref_is_name(fref) && ref_is_verb(sref)) {
				switch_children(to_return.at(n));
			}
		}
	}

	if (debug)
		print_vector (to_return);
	// A verb as subject is erased at this level
	for (int n = 0; n < to_return.size(); ++n) {
		string fref = extract_subject(to_return.at(n));
		string sref = extract_object(to_return.at(n));
		string head = extract_header(to_return.at(n));
		if (to_return.at(n).is_verb()) {
			if (ref_is_verb(fref)) {
				implant_subject(to_return.at(n), "none");
			}
		}
	}
	if (debug)
		print_vector (to_return);

	// more/JJR than 3 people smiled -> people more-than 3 people smiled
	for (int n = 0; n < to_return.size(); ++n) {
		string fref = extract_first_tag(to_return.at(n));
		string sref = extract_second_tag(to_return.at(n));
		string head = extract_header(to_return.at(n));
		if (to_return.at(n).tag() == "JJR" && head == "more") {
			int pos_verb = find_verb_with_subject(to_return, tags, fref);
			if (pos_verb != -1) {
				vector<int> all_pos_compared = find_complement_with_name(to_return, "than");
				for (int m = 0; m < all_pos_compared.size(); ++m) {
					string fref2 = extract_first_tag(to_return.at(all_pos_compared.at(m)));
					string sref2 = extract_second_tag(to_return.at(all_pos_compared.at(m)));
					int pos_new_name = find_element_with_string(to_return, sref2);
					if (pos_new_name == -1)
						continue;
					string new_name = extract_header(to_return.at(pos_new_name));
					if (fref2 == fref) {
						implant_header(to_return.at(n), new_name);
						to_return.at(n).setTag(to_return.at(pos_new_name).tag());
						for (int j = 0; j < to_return.size(); ++j) {
							string fref3 = extract_first_tag(to_return.at(j));
							string sref3 = extract_second_tag(to_return.at(j));
							string head3 = extract_header(to_return.at(j));
							if (head3 == "@MORE") {
								add_header(to_return.at(j), ":DELETE");
							}
							if (head3 == "@COMPARED_TO" && fref3 == fref) {
								add_header(to_return.at(j), "@MORE");
							}
						}
					}
				}
			}
		}
	}

	// If a complement points to nothing and it is a question, try to join a WP with the void pointers
	/// You should create a dummy name as a target for the preposition
	for (int n = 0; is_question && n < to_return.size(); ++n) {
		string fref = extract_first_tag(to_return.at(n));
		string sref = extract_second_tag(to_return.at(n));
		string head = extract_header(to_return.at(n));
		if (to_return.at(n).is_complement() && !to_return.at(n).is_question() && !ref_is_verb(sref) && !ref_is_name(sref)
				&& head != "@TIME"
				&& head != "@MODAL"
				&& head != "@AUXILIARY"
				&& head != "@QUANTIFIER"
				&& head != "@PARENT-ALLOCUTION"
		    ) {
			string new_ref = get_first_ref_with_tag(to_return, "WP");
			if (new_ref == "")
				new_ref = get_first_ref_with_tag(to_return, "WP$");
			if (new_ref != "")
				implant_second(to_return.at(n), new_ref);
			// "who is Nutt in love with?" (erase the subject of "to be")
			int m = find_verb_with_subject(to_return, tags, new_ref);
			if (m != -1 && extract_header(to_return.at(m)) == "be") {
				implant_subject(to_return.at(m), "none");
				switch_subj_obj(to_return.at(m));
			}
		}
	}

	// If a verb object points to nothing and it is a question, try to join a WP with the void pointers
	for (int n = 0; is_question && n < to_return.size(); ++n) {
		string fref = extract_first_tag(to_return.at(n));
		string sref = extract_second_tag(to_return.at(n));
		string head = extract_header(to_return.at(n));
		head = head.substr(0, head.find(":"));
		if (to_return.at(n).is_verb() && head == "do" && !to_return.at(n).is_question() && !has_object(to_return.at(n))) {
			string new_ref = get_first_ref_with_tag(to_return, "WP");
			if (new_ref == "")
				continue;
			// "what are the snakes able to do"
			if (has_object(to_return.at(n)))
				continue;
			//implant_object(to_return.at(n), new_ref);
			implant_object(to_return.at(n), "none");
			to_return.at(n).set_question();
			to_return.at(n).set_question_word("do-something");
			vector<int> positions = find_all_element_with_string(to_return, new_ref);
			for (int m = 0; m < positions.size(); ++m) {
				to_return.at(positions.at(m)).name() += ":DELETE";
				add_header(to_return.at(positions.at(m)), ":DELETE");
			}
		}
	}

	// A WP in a question cannot be alone
	for (int n = 0; is_question && n < to_return.size(); ++n) {
		string fref = extract_first_tag(to_return.at(n));
		string sref = extract_second_tag(to_return.at(n));
		string head = extract_header(to_return.at(n));
		head = head.substr(0, head.find(":"));
		if (head == get_local_what() && to_return.at(n).is_question()) {
			if (!is_lonely_name(to_return, n))
				continue;
			int cref = find_complement_with_target(to_return,fref);
			if( cref != -1 && !to_return.at(cref).is_delete())
				continue;
			// find the first verb with either no subject or no object
			for (int m = 0; m < to_return.size(); ++m) {
				if (to_return.at(m).is_verb()) {
					if (!has_subject(to_return.at(m)) && fref != extract_object(to_return.at(m))
						&& !is_passive(to_return.at(m) )
					)
						implant_subject(to_return.at(m), fref);
					else if (!has_object(to_return.at(m)) && fref != extract_subject(to_return.at(m)))
						implant_object(to_return.at(m), fref);
					else if(verb_supports_indirect_obj(names.at(m))
							|| is_passive(to_return.at(m) )
							) {
						string oref = extract_object(to_return.at(m));
						implant_object(to_return.at(m), fref);
						DrtPred dative_pred("@DATIVE(" + extract_first_tag(to_return.at(m)) + "," + oref + ")");
						to_return.push_back(dative_pred);
					}
				}
			}
		}
	}

	// A WP$ in a question cannot be alone
	/// You should create a dummy name as a subject or object
	for (int n = 0; is_question && n < to_return.size(); ++n) {
		string fref = extract_first_tag(to_return.at(n));
		string sref = extract_second_tag(to_return.at(n));
		string head = extract_header(to_return.at(n));
		string tag = to_return.at(n).tag();
		head = head.substr(0, head.find(":"));
		if (tag == "WP$" && to_return.at(n).is_question()) {
			int m = find_name_with_string(to_return, fref);
			if (m != -1)
				continue;
			// assing the WP$ to the subject of object of the first verb
			for (int m = 0; m < to_return.size(); ++m) {
				if (to_return.at(m).is_verb()) {
					string subj_ref = extract_subject(to_return.at(m));
					string obj_ref = extract_object(to_return.at(m));
					if (!has_subject(to_return.at(m)) && fref != obj_ref)
						implant_subject(to_return.at(n), fref);
					if (!has_object(to_return.at(m)) && fref != subj_ref)
						implant_object(to_return.at(m), fref);
				}
			}
		}
	}

	// if a sentence starts with a preposition, and the first tag of the preposition is unassigned,
	// then assign the preposition to the last verb. (not for questions!!)
	for (int n = 0; !is_question && n < to_return.size(); ++n) {
		string fref = extract_first_tag(to_return.at(n));
		string sref = extract_second_tag(to_return.at(n));
		string head = extract_header(to_return.at(n));
		if (to_return.at(n).is_complement() && !ref_is_name(fref) && !ref_is_verb(fref) && head != "@OWN" // @OWN complements can be unassigned
				) {
			string new_ref = get_last_verb_ref(to_return);
			if (new_ref == "" || new_ref == sref)
				continue;
			to_return = substitute_ref(to_return, fref, new_ref);
		}
	}

	// If a WDT is linked to an adverb which links to a verb, and the verb has no object,
	// then the WDT reference becomes object
	for (int n = 0; n < connections.size(); ++n) { // connects numerals and names with @QUANTITY(number,name)
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		str1 = to_return.at(pos1).name();
		str2 = to_return.at(pos2).name();
		tag1 = to_return.at(pos1).tag();
		tag2 = to_return.at(pos2).tag();

		if (tag1 == "WDT" && tag2 == "RB") {
			string fref = extract_first_tag(to_return.at(pos2));
			int m = find_verb_with_string(to_return, fref);
			if (m == -1)
				continue;
			if (has_object(to_return.at(m)))
				continue;
			string sref = extract_subject(to_return.at(m));
			string wref = extract_first_tag(to_return.at(pos1));
			if (sref != wref)
				implant_object(to_return.at(m), wref);
		}
	}

	// @PAR that point to a verb with no subject can take the subject from the parent
	for (int n = 0; n < to_return.size(); ++n) {
		if (to_return.at(n).is_complement()) {
			string fref = extract_first_tag(to_return.at(n));
			string sref = extract_second_tag(to_return.at(n));
			string head = extract_header(to_return.at(n));
			if (head == "@PAR") {
				int fverb = find_verb_with_string(to_return, fref);
				int sverb = find_verb_with_string(to_return, sref);
				if (fverb == -1 || sverb == -1)
					continue;
				if (has_subject(to_return.at(fverb)) && !has_subject(to_return.at(sverb))
//                       && !is_passive(to_return.at(fverb))
//                       && !is_passive(to_return.at(sverb))
						) {
					implant_subject(to_return.at(sverb), extract_subject(to_return.at(fverb)));
				}
			}
		}
	}


	// A verb connected to a verb should inherit a subject or an object
	// only if it is not already connected with a preposition
	for (int n = 0; n < connections.size(); ++n) {
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		str1 = to_return.at(pos1).name();
		str2 = to_return.at(pos2).name();
		tag1 = to_return.at(pos1).tag();
		tag2 = to_return.at(pos2).tag();

		if (is_verb(tag1) && is_verb(tag2)) {
			string fref1 = extract_first_tag(to_return.at(pos1));
			string fref2 = extract_first_tag(to_return.at(pos2));
			int m = find_verb_with_string(to_return, fref2);
			string h1 = extract_header(to_return.at(pos1));
			string h2 = extract_header(to_return.at(pos2));
			h1 = h1 + h2;
			if (h1.find("AUX") != string::npos)
				continue;
			if (h1.find("PASSIVE") != string::npos)
				continue;

			if (m == -1)
				continue;

			bool has_complement = false;
			vector<int> mconj = find_all_compl_with_second_tag(to_return, tags, fref2);
			for (int k = 0; k < mconj.size(); ++k) {
				string fref_tmp = extract_first_tag(to_return.at(mconj.at(k)));
				if (fref_tmp == fref1)
					has_complement = true;
			}

			if (has_complement)
				continue;

			// if it has a SUBORD it cannot have an object
			mconj = find_all_compl_with_first_tag(to_return, tags, fref2);
			bool has_dative = false;
			for (int k = 0; k < mconj.size(); ++k) {
				string fref_tmp = extract_first_tag(to_return.at(mconj.at(k)));
				string header = extract_header(to_return.at(mconj.at(k)));
				if (fref_tmp == fref2 && header == "@SUBORD")
					has_dative = true;
			}

			string s1 = extract_subject(to_return.at(pos1));
			string o1 = extract_object(to_return.at(pos1));
			string s2 = extract_subject(to_return.at(m));
			string o2 = extract_object(to_return.at(m));
			if (!has_object(to_return.at(m)) && !has_dative && s2 != s1 && s2 != o1) {
				if (has_object(to_return.at(pos1)))
					implant_object(to_return.at(m), extract_object(to_return.at(pos1)));
				else if (str2 == "be" && has_subject(to_return.at(pos1))) {
					implant_object(to_return.at(m), extract_subject(to_return.at(pos1)));
				}
			}
			if (!has_subject(to_return.at(m)) && o2 != o1 && o2 != s1) {
				if (has_subject(to_return.at(pos1)))
					implant_subject(to_return.at(m), extract_subject(to_return.at(pos1)));
				else if (str2 == "be" && has_object(to_return.at(pos1))) {
					implant_subject(to_return.at(m), extract_object(to_return.at(pos1)));
				}
			}
		}
	}


	// A verb gets the subject from the AUX if the verb does not have hone
	for (int n = 0; n < connections.size(); ++n) {
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		str1 = to_return.at(pos1).name();
		str2 = to_return.at(pos2).name();
		tag1 = to_return.at(pos1).tag();
		tag2 = to_return.at(pos2).tag();

		if (is_verb(tag1) && is_verb(tag2)) {
			string fref1 = extract_first_tag(to_return.at(pos1));
			string fref2 = extract_first_tag(to_return.at(pos2));
			int m = find_verb_with_string(to_return, fref2);
			string h1 = extract_header(to_return.at(pos1));
			string h2 = extract_header(to_return.at(pos2));
			if (h1.find("AUX") == string::npos)
				continue;
			h1 = h1 + h2;
			if (h1.find("PASSIVE") != string::npos)
				continue;

			if (m == -1)
				continue;

			if (!has_object(to_return.at(m))) {
				if (has_object(to_return.at(pos1)))
					implant_object(to_return.at(m), extract_object(to_return.at(pos1)));
			}
			if (!has_subject(to_return.at(m))) {
				if (has_subject(to_return.at(pos1)))
					implant_subject(to_return.at(m), extract_subject(to_return.at(pos1)));
			}
		}
	}


	// Some complements need to be corrected
	for (int n = 0; !is_question && n < to_return.size(); ++n) {
		string fref = extract_first_tag(to_return.at(n));
		string sref = extract_second_tag(to_return.at(n));
		string header = extract_header(to_return.at(n));
		if (to_return.at(n).is_complement() && header == "@TOPIC" && ref_is_verb(sref)) {
			implant_header(to_return.at(n), "@TIME_AT");
		}
		if (to_return.at(n).is_complement() && header == "@CAUSED_BY") {
			int m = find_element_with_string(to_return, sref);
			if (m == -1)
				continue;
			if (to_return.at(m).tag() == "CD") {
				string next_header = extract_header(to_return.at(m));
				if (!is_date(next_header)) {
					next_header = "[date]_" + next_header;
				}
				implant_header(to_return.at(n), "@TIME_AT");
			}
		}
	}

	// Some complements need to be corrected
	for (int n = 0; !is_question && n < to_return.size(); ++n) {
		string fref = extract_first_tag(to_return.at(n));
		string sref = extract_second_tag(to_return.at(n));
		string header = extract_header(to_return.at(n));
		if (to_return.at(n).is_complement() && header != "@QUANTIFIER" && header != "@MODAL" && header != "@TIME"
				&& !ref_is_verb(sref) && !ref_is_name(sref)) {
			add_header(to_return.at(n), ":DELETE");
		}
	}

	// Snakes have a long, slender body -> Snakes have a long slender body
	/// Refine this! it does'n always work for questions
	for (int n = 0; !is_question && n < to_return.size(); ++n) {
		string fref = extract_first_tag(to_return.at(n));
		string sref = extract_second_tag(to_return.at(n));
		string header = extract_header(to_return.at(n));
		if (to_return.at(n).is_complement()
		    && (to_return.at(n).tag() == "CC" || to_return.at(n).tag() == "-comma-") ) {
			if(has_only_adjectives(to_return,fref) && has_adjective(to_return,sref) && !has_verb(to_return,sref) ) {
				add_header(to_return.at(n), ":DELETE");
				// implant_first(to_return.at(n),"none");
				implant_second(to_return.at(n),"none");
				to_return= substitute_ref_unsafe(to_return,sref,fref);
			}
		}
	}


	// a noun cannot support an allocution
	for (int n = 0; n < to_return.size(); ++n) {
		string head = extract_header(to_return.at(n));
		string fref = extract_first_tag(to_return.at(n));
		string sref = extract_second_tag(to_return.at(n));
		if (head == "@PARENT-ALLOCUTION" && (!ref_is_verb(sref) || !ref_is_verb(fref))) {
			if(ref_is_name(fref) && ref_is_verb(sref)) {
				implant_header(to_return.at(n),"@ALLOCUTION");
				switch_children(to_return.at(n));
			} else
				add_header(to_return.at(n), ":DELETE");
		}
	}

	// Sometimes there can be @TIME_AT(A,B), @TIME_AT(B,A) -> @TIME_AT(A,B), @TIME_AT:DELETE(B,A)
	vector < pair<string, pair<string, string> > > prev_compl;
	for (int n = 0; n < to_return.size(); ++n) {
		if (!to_return.at(n).is_complement())
			continue;
		string header = extract_header(to_return.at(n));
		string fref = extract_first_tag(to_return.at(n));
		string sref = extract_second_tag(to_return.at(n));
		pair<string, pair<string, string> > new_tuple = make_pair(header, make_pair(sref, fref));
		if (shortfind(prev_compl, new_tuple)) {
			add_header(to_return.at(n), ":DELETE");
		} else
			prev_compl.push_back(make_pair(header, make_pair(fref, sref)));
	}



	// say that "he is happy": say(verb1,,) @SUBORD(verb1,name1) happy(name1) -> say(verb1,,) @ALLOCUTION(verb1,name1) happy(name1)
	vector < string > cverbs = get_communication_verbs();
	for (int n = 0; !is_question && n < to_return.size(); ++n) {
		string fref = extract_first_tag(to_return.at(n));
		string sref = extract_second_tag(to_return.at(n));
		string header = extract_header(to_return.at(n));
		header = header.substr(0,header.find(":"));
		if (to_return.at(n).tag() == "IN" && header == "that"
				&& ref_is_verb(fref) && ref_is_name(sref)) {
			int m= find_verb_with_string(to_return,fref);
			int m2= find_verb_with_string(to_return,fref);
			if (m == -1) 
				continue;
			string vheader= extract_header(to_return.at(m));
			vheader = vheader.substr(0,vheader.find(":"));
			if(!shortfind(cverbs,vheader))
				continue;
			vector<int> poz = find_all_names_with_string_no_delete(to_return,sref);
			if(poz.size() != 1) 
				continue;
			int pos = poz.at(0);
			if(!to_return.at(pos).is_verbatim() )
				continue;
			implant_header(to_return.at(n), "@ALLOCUTION");
		}
	}


	// check that there are no closed loops
	for (int n = 0; n < to_return.size(); ++n) {
		string header = extract_header(to_return.at(n));
		if (to_return.at(n).is_complement() && header != "@CONJUNCTION" && header != "@DISJUNCTION" && header != "@COORDINATION") {
			string fref = extract_first_tag(to_return.at(n));
			string sref = extract_second_tag(to_return.at(n));
			int m1= find_verb_with_object(to_return,fref);
			if(m1 != -1 ) {
				string vref = extract_first_tag(to_return.at(m1));
				int m2 = find_complement_with_first_and_second_tag(to_return,sref,vref,"@CONJUNCTION|@DISJUNCTION|@COORDINATION");
				int m3 = find_complement_with_first_and_second_tag(to_return,vref,sref,"@CONJUNCTION|@DISJUNCTION|@COORDINATION");
//				int m2 = find_complement_with_first_and_second_tag(to_return,sref,vref);
//				int m3 = find_complement_with_first_and_second_tag(to_return,vref,sref);
				if(m2 != -1 ) {
					add_header(to_return.at(m2),":DELETE");
				}
				if(m3 != -1 ) {
					add_header(to_return.at(m3),":DELETE");
				}
			}
		}
	}

	// @SUBORD(A,B), @CONJUNCTION(B,A) -> @SUBORD(A,B), @CONJUNCTION:DELETE(B,A)
	for (int n = 0; n < to_return.size(); ++n) {
		string header = extract_header(to_return.at(n));
		if (to_return.at(n).is_complement() && header != "@CONJUNCTION" && header != "@DISJUNCTION" && header != "@COORDINATION") {
			string fref = extract_first_tag(to_return.at(n));
			string sref = extract_second_tag(to_return.at(n));
			int m2 = find_complement_with_first_and_second_tag(to_return,sref,fref,"@CONJUNCTION|@DISJUNCTION|@COORDINATION");

			if(m2 != -1 ) {
				add_header(to_return.at(m2),":DELETE");
			}
		}
	}

	// @SUBORD(A,verb1), @...(B,A) -> @SUBORD:DELETE(A,verb1), @...(verb1,B)
	for (int n = 0; n < to_return.size(); ++n) {
		string header = extract_header(to_return.at(n));
		string sref   = extract_second_tag(to_return.at(n));
		if (header == "@SUBORD" && ref_is_verb(sref) ) {
			string fref = extract_first_tag(to_return.at(n));
			int m = find_complement_with_target(to_return,fref);
			int mverb = find_verb_with_string(to_return,sref);

			if(m != -1 && mverb == -1) {
				string header2 = extract_header(to_return.at(m));
				if(header2 == "@SUBORD")
					continue;
				add_header(to_return.at(n),":DELETE");
				switch_children(to_return.at(m) );
				implant_first(to_return.at(m),sref);
			}
		}
	}

	return to_return;
}

