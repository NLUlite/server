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



#include"drs_anaphora_levels.hpp"


template <class T>
static void print_vector(std::vector<T> &vs)
{
     if(vs.size()) {
	  typename vector<T>::iterator tags_iter= vs.begin();
	  while ( tags_iter != vs.end() ) {
	       std::cout << (*tags_iter) << " ";
	       ++ tags_iter;
	  }
	  std::cout << std::endl;
     }
}


static vector<DrtPred> assign_levels_right_of(vector<DrtPred> drt_vect)
{
     vector<string> triggers; // everything at the right of a trigger is raised of one level     
     triggers.push_back("not");

     vector<int> to_the_right;
     vector<int>::iterator riter;

     vector<DrtPred>::iterator diter= drt_vect.begin();
     vector<DrtPred>::iterator dend= drt_vect.end();

     int n=0;
     for(; diter != dend; ++diter, ++n) {
     	vector<string>::iterator triter= find(triggers.begin(), triggers.end(), diter->name() );

     	if( triter != triggers.end() ) {
     		to_the_right= get_elements_next_of(drt_vect, n);
     		for(riter= to_the_right.begin(); riter != to_the_right.end(); ++riter) {
     			string anaphora_str= drt_vect.at( *riter ).anaphoraLevel() + "_" +  *triter;
     			drt_vect.at( *riter ).setAnaphoraLevel( anaphora_str );
     		}
     	}
     }

     return drt_vect;
}

static vector<DrtPred> assign_levels_conditionals(vector<DrtPred> drt_vect)
{
     vector<string> triggers; // everything at the right of a trigger is raised of one level     
     triggers.push_back("@CONDITION");

     vector<int> to_the_right;
     vector<int>::iterator riter;

     vector<DrtPred>::iterator diter= drt_vect.begin();
     vector<DrtPred>::iterator dend= drt_vect.end();

     int n=0;
     for(; diter != dend; ++diter, ++n) {
	  vector<string>::iterator triter= find(triggers.begin(), triggers.end(), diter->name() );	  
	  if( triter != triggers.end() ) {
	       string verb_cons= extract_second_tag(*diter);
	       int m =  find_verb_with_string(drt_vect, verb_cons);
	       to_the_right= find_int_attached_to_verb(drt_vect, m);
	       for(riter= to_the_right.begin(); riter != to_the_right.end(); ++riter) {
		    string anaphora_str= drt_vect.at( *riter ).anaphoraLevel() + "_" +  *triter;
		    drt_vect.at( *riter ).setAnaphoraLevel( anaphora_str );
	       }
	  }	       
     }

     return drt_vect;
}


void drs_anaphora_levels::visit(drt_collection *collection)
{
     vector<drt> drss= collection->get_collection();
     vector<drt>::iterator drtiter= drss.begin();
     vector<drt>::iterator drtend= drss.end();
     for(; drtiter != drtend; ++drtiter) {
	  vector<DrtPred> tmp_drt= drtiter->predicates();
	  tmp_drt= assign_levels_right_of(tmp_drt);
	  tmp_drt= assign_levels_conditionals(tmp_drt);
	  /// Re-assign back here
	  drtiter->setPredicates( tmp_drt );
     }
     collection->set_collection( drss );
}

