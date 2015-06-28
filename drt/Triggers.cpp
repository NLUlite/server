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



#include"Triggers.hpp"

const bool debug = false;

template<class T>
static void print_vector(std::vector<T> &vs)
{
	typename vector<T>::iterator tags_iter = vs.begin();
	while (tags_iter < vs.end()) {
		std::cout << (*tags_iter) << " ";
		++tags_iter;
	}
	std::cout << std::endl;
}

static vector<vector<FuzzyPred> > get_new_tags(const DrtVect &drtvect, const vector<FuzzyPred> &taglist,
		const FuzzyPred &parsed_tree, const DrtPred &error)
{

	vector<vector<FuzzyPred> > to_return;

	PredTree enhanced_tree = parsed_tree.pred();
	PredTree::height_iterator hi(enhanced_tree, 0);
	vector<PredTree::iterator> iters;
	for (; hi != enhanced_tree.end(); ++hi) {
		iters.push_back(hi);
	}
	for (int n = 0; n < drtvect.size() && n < iters.size(); ++n) {
		stringstream ss;
		drtvect.at(n).print(ss);
		PredTree::iterator pi = iters.at(n);
		enhanced_tree.replace(Predicate(ss.str()).pred(), pi);
	}

	clock_t start;


	CodePred get_candidate_verbs("join(set(taglist,_B),"
			"     set(taglist-VBZ,_B),"
			"     set(taglist-VBZ2,_B),"
			"     set(like-taglist,_B),"
			"     set(taglist-VBD,_B),"
			"     set(taglist-VB,_B),"
			"     set(taglist-VB2,_B),"
			"     set(taglist-VBP,_B),"
			"     set(taglist-VBP2,_B),"
			"     set(taglist-combo,_B),"
			"     set(taglist-VBP3,_B),"
			"     set(taglist-VBG,_B),"
			"     set(taglist-IN,_B),"
			"     set(taglist-NN,_B),"
			"     set(taglist-VBN,_B),"
			"     set(taglist-VBZ,_B),"
			"     set(taglist-WDT,_B),"
			"     set(taglist-RBS,_B),"
			"     set(taglist-RP,_B),"
			"     set(taglist-RB,_B),"
			"     set(taglist-MD,_B),"
			"     set(taglist-NNS,_B),"
			"     set(taglist-THAT,_B),"
			"     set(taglist-PRN-END,_B),"
			"     set(taglist-comma,_B),"
			"     set(taglist-CD-date,_B),"
			"     set(taglist-JJ,_B),"
			"     set(taglist-switch,_B),"
			"     set(taglist-switch2,_B),"
			"     set(taglist-ALLOCUTION,_B),"
			"     set(taglist-error,_B),"
			"     set(error,_D),"
			"     set(is-question,find-str-in-tree(_C,?)),"
			"     set(is-conditional,find-str-in-tree(_C,if/IN)),"
			"     join(print(here-is-question),print(is-question)),"
			"     if(greater-than(num-children(not-connected-to-verbs(_A)),0),"
			"        join(for-each(_b,_A,"
			"                  join(if(and(or(find-str(_b,/NNS),find-str(_b,/NNPS)),"
			"                              find(not-connected-to-verbs(_A),_b),"
			"                              is-candidate-verb(_b,NNS)"
			"                             ),"
			"                           set(positions,add-list(positions,_counter))"
			"                          ),"
			"                       if(and(find-str(_b,like/IN),"
			"                               find(not-connected-to-verbs(_A),_b)"
			"                              ),"
			"                           set(like-positions,add-list(like_positions,_counter))"
			"                          )"
			"                       )"
			"                 ),"
			"            for-each(_c,positions,"
			"                     set(taglist,set-at(taglist,_c,VBZ))"
			"                    ),"
			"            for-each(_c,like-positions,"
			"                     set(like-taglist,set-at(like-taglist,_c,VBP))"
			"                    )"
			"            )"
			"        ),"
			"     if(find-str(error,complement_with_no_prior)," // for cases like "for ( S ( NP example, david and maria) went to the cinema)."
			"        join(print(here-is-error),print(error),set(fref-error,first-child(error)),"
			"             for-each(_b,_A,"
			"                      if(find-str(_b,-comma-),"
			"                         join(set(taglist-error,set-at(taglist-error,_counter,-comma-SBAR)),break)"
			"                        )"
			"                     )"
			"            )"
			"       ),"
			"     if(find-str(error,dummy_subordinate),"// for cases like "for ( S ( NP example, david and maria) went to the cinema)."
			"        join(print(here-is-error),print(error),set(fref-error,first-child(error)),"
			"             for-each(_b,_A,"
			"                      if(and(print(_counter),print(fref-error),equal(_counter,fref-error),"
			"                             print(here2),find-str(_b,/VB),print(here3)"
			"                            ),"
			"                         if-else(is-candidate-noun(_b,NNS),"
			"                                 join(set(taglist-error,set-at(taglist-error,_counter,NNS)),break),"
			"                                 if(is-candidate-noun(_b,NN),"
			"                                    join(set(taglist-error,set-at(taglist-error,_counter,NN)),break)"
			"                                   )"
			"                                )"
			"                        )"
			"                     )"
			"            )"
			"       ),"
			"     for-each(_b,_A,"
			"              if(and(find-str(_b,/VBZ),not(has-subject(_b)),not(has-object(_b)),is-candidate-noun(_b,NNS)),"
			"                 set(taglist-VBZ,set-at(taglist-VBZ,_counter,NNS))"
			"                )"
			"             ),"
			"     for-each(_b,_A,"
			"              if(and(find-str(_b,/VBZ),find-str(_prior[_b],/CC),not(has-next(_prior[_b])),is-candidate-noun(_b,NNS)),"
			"                 set(taglist-VBZ,set-at(taglist-VBZ,_counter,NNS))"
			"                )"
			"             ),"
			"     for-each(_b,_A,"
			"              if("
			"                 or("
			"                    find-str(_b,/RP),"
			"                    find-str(_b,up/RB),"
			"                    find-str(_b,down/RB),"
			"                    and(find-str(_b,outside/JJ),not(find-str(_prior[_b],/JJ)),find-str(_next[_b],/NN)),"
			"                    and(find-str(_b,outside/JJ),find-str(_prior[_b],/VB),find-str(_next[_b],/PRP)),"
			"                    and(find-str(_b,before/RB),find-str(_next[_b],/PRP)),"
			"                    and(find-str(_b,because/RB),find-str(_next[_b],/PRP)),"
			"                    and(find-str(_b,like/VB),find-str(_next[_b],/-period-)),"
			"                    and(find-str(_b,like/VB),find-str(_next[_b],/-comma-)),"
			"                    and(find-str(_b,like/VB),find-str(_prior[_b],/NN),find-str(_next[_b],/NN)),"
			"                    and(find-str(_b,like/VB),find-str(_next[_b],/DT)),"
			"                    and(find-str(_b,beyond/NN),find-str(_next[_b],/DT)),"
			"                    and(find-str(_b,beyond/NN),find-str(_next[_b],/NN)),"
			"                    and(find-str(_b,that/WDT),find-str(_next[_b],/NN)),"
			"                    and(find-str(_b,that/WDT),find-str(_next[_b],/DT)),"
			"                    and(find-str(_b,that/WDT),find-str(_next[_b],/PRP$)),"
			"                    and(find-str(_b,that/WDT),find-str(_next[_b],/DT)),"
			"                    and(find-str(_b,that/WDT),find-str(_next[_b],/PRP)),"
			"                    and(find-str(_b,that/WDT),find-str(_next[_b],/MD))"
			"                   ),"
			"                 join(set(taglist-IN,set-at(taglist-IN,_counter,IN)),break)"// one at a time
			"                )"
			"             ),"
			"     for-each(_b,_A,"
			"              join(if(or(find-str(_b,that/IN),"
			"                         find-str(_b,that/NN)"
			"                         ),"
			"                      set(taglist-WDT,set-at(taglist-WDT,_counter,WDT))"
			"                     ),"
			"                   if(or(false,"
			"                         is-verb-levin(_b,verb.communication)"
			"                         ),"
			"                      break"
			"                     )"
			"                  )"
			"             ),"
			"     for-each(_b,_A,"
			"              if(and(is-verb-levin(_b,verb.communication),"
			"                          not(find-str(_b,tell)),"
			"                          find-str(_next[_b],PRP)"
			"                         ),"
			"                      set(taglist-THAT,insert-at(taglist-THAT,plus(_counter,1),IN(that)))"
			"               )"
			"             ),"
			"     for-each(_b,_A,"
			"              join(if(and(find-str(_b,that/IN),"
			"                          find-str(_next[_b],/IN)"
			"                         ),"
			"                      set(taglist-WDT,set-at(taglist-WDT,_counter,WDT))"
			"                     ),true"
			"                  )"
			"             ),"
			"     for-each(_b,_A,"
			"                   if(or("
//			"                         and(find-str(_b,/JJ),"  // This does more harm than good: "why is the sky blue/JJ->NN?"
//			"                             not(find-str(_b,/JJS)),"
//			"                             not(find-str(_b,/JJR)),"
//			"                             find-str(_next[_b],?/-period-)"
//			"                            ),"
			"                         and(find-str(_b,/VBP),"
			"                             is-candidate-noun(_b,VB),"
			"                             find-str(_next[_b],?/-period-)"
			"                            ),"
			"                         and(find-str(_b,/RB),"
			"                             find-str(_next[_b],of/IN)"
			"                            ),"
			"                         and(find-str(_b,number/VB),"
			"                             is-candidate-noun(_b,VB),"
			"                             find-str(_next[_b],/CD)"
			"                            )"
			"                        ),"
			"                        set(taglist-NN,set-at(taglist-NN,_counter,NN))"
			"                     )"
			"             ),"
			"     for-each(_b,_A,"
			"                   if(and(find-str(_b,/JJ)," // Philip Kindred/JJ->NNP Dick
			"                          not(find-str(_b,/JJS)),"
			"                          not(find-str(_b,/JJR)),"
			"                          find-str(_next[_b],/NNP),"
			"                          find-str(_prior[_b],/NNP)"
			"                         ),"
			"                        set(taglist-NN,set-at(taglist-NN,_counter,NNP))"
			"                     )"
			"             ),"
			"     for-each(_b,_A,"
			"              if(or(and(find-str(_b,/VBP),"
			"                        find-str(_next[_b],?/-period-)"
			"                        ),"
			"                    and(find-str(_b,/NN),not(find-str(_b,/NNS)),not(find-str(_b,/NNPS)),"
			"                        not(find-str(_prior[_b],what/WP)),"
			"                        not(find-str(_prior[_b],PRP$)),"
			"                        not(find-str(_prior[_b],the/DT)),"
			"                        not(find-str(_prior[_b],a/DT)),"
			"                        not(find-str(_prior[_b],an/DT)),"
			"                        not(find-str(_prior[_b],/IN)),"
			"                        not(find-str(_prior[_b],/VBZ)),"
			"                        not(find-str(_prior[_b],/VBP)),"
			"                        not(find-str(_prior[_b],/VBD)),"
			"                        is-candidate-verb(_b,VB),"
			"                        find-str(_next[_b],?/-period-)"
			"                       ),"
			"                    and(find-str(_b,like/IN),"
			"                        find-str(_prior[_b],/CC),"
			"                        find-str(_next[_b],/VBG)"
			"                       )"
			"                   ),"
			"                 set(taglist-VB,set-at(taglist-VB,_counter,VB))"
			"                )"
			"             ),"
			"if(is-question,"
			"     for-each(_b,_A,"
			"              if(or(false,"
			"                    and(find-str(_b,/NN),not(find-str(_b,/NNS)),not(find-str(_b,/NNPS)),"
			"                        not(find-str(_prior[_b],what/WP)),"
			"                        not(find-str(_prior[_b],PRP$)),"
			"                        not(find-str(_prior[_b],the/DT)),"
			"                        not(find-str(_prior[_b],a/DT)),"
			"                        not(find-str(_prior[_b],an/DT)),"
			"                        not(find-str(_prior[_b],/IN)),"
			"                        not(find-str(_prior[_b],/VBZ)),"
			"                        not(find-str(_prior[_b],/VBP)),"
			"                        not(find-str(_prior[_b],/VBD)),"
			"                        is-candidate-verb(_b,VBN),"
			"                        find-str(_next[_b],?/-period-)"
			"                       )"
			"                   ),"
			"                 set(taglist-VBN,set-at(taglist-VBN,_counter,VBN))"
			"                )"
			"             )"
			"  )"
			"     for-each(_b,_A,"
			"              if(or(false,"
			"                    and(find-str(_b,/CC),find-str(_prior[_b],/NN),find-str(_next[_b],/WRB),"
			"                        is-candidate-verb(_prior[_b],VB)"
			"                       )"
			"                   ),"
			"                 set(taglist-VB,set-at(taglist-VB,minus(_counter,1),VB))"
			"                )"
			"             ),"
			"     set(no-VBD,list),"
			"     for-each-tree(_c,_C,"
			"                   if(and(find-str(_c,VP),"
			"                          find-str(_parent[_c],VP),"
			"                          find-str(first-child(_c),VBD)"
			"                         ),"
			"                      set(no-VBD,add-list(no-VBD,first-child(_c)))"
			"                     )"
			"                  ),"
			"     set(is-after-aux,false),"
			"     for-each(_b,_A,"
			"              join(if(find-str(_b,AUX/VB),"
			"                      set(is-after-aux,true)"
			"                     ),"
			"                   if(or(and(find-str(_b,/VB),"
			"                             not(find-str(_b,/VBP)),"
			"                             not(find-str(_b,/VBD)),"
			"                             not(find-str(_b,/VBZ)),"
			"                             not(find-str(_b,/VBN)),"
			"                             not(find-str(_b,/VBG)),"
			"                             find-str(_prior[_b],/VB),"
			"                             not(find-str(_prior[_b],@TIME_AT)),"
			"                             is-candidate-noun(_b,VB)"
			"                            ),"
			"                         and(find-str(_b,/VB),"
			"                             not(find-str(_b,/VBP)),"
			"                             not(find-str(_b,/VBD)),"
			"                             not(find-str(_b,/VBZ)),"
			"                             not(find-str(_b,/VBN)),"
			"                             find-str(_prior[_b],/NN),"
			"                             find-str(_next[_b],/NN),"
			"                             not(find-str(_prior[_b],@TIME_AT)),"
			"                             is-candidate-noun(_b,VB)"
			"                            ),"
			"                         and(find-str(_b,/VB),"
			"                             not(find-str(_b,/VBP)),"
			"                             not(find-str(_b,/VBD)),"
			"                             not(find-str(_b,/VBZ)),"
			"                             not(find-str(_b,/VBN)),"
			"                             not(find-str(_prior[_b],@TIME_AT)),"
			"                             find-str(_prior[_b],/JJS),"
			"                             is-candidate-noun(_b,VB)"
			"                            ),"
			"                         and(find-str(_b,/VB),"
			"                             not(find-str(_b,/VBP)),"
			"                             not(find-str(_b,/VBD)),"
			"                             not(find-str(_b,/VBZ)),"
			"                             not(find-str(_b,/VBN)),"
			"                             not(find-str(_prior[_b],@TIME_AT)),"
			"                             find-str(_prior[_b],/EX),"
			"                             is-candidate-noun(_b,VB)"
			"                            ),"
			"                         and(find-str(_b,/VB),"
			"                             not(find-str(_b,/VBD)),"
			"                             not(find-str(_b,/VBZ)),"
			"                             not(find-str(_b,/VBN)),"
			"                             find-str(_prior[_b],/CD),"
			"                             not(find-str(_prior[_b],@TIME_AT)),"
			"                             is-candidate-noun(_b,VB)"
			"                            ),"
			"                         and(find-str(_b,/VB),"
			"                             not(is-after-aux),"
			"                             not(find-str(_b,/VBD)),"
			"                             not(find-str(_b,/VBZ)),"
			"                             not(find-str(_b,/VBN)),"
			"                             not(find-str(_prior[_b],/TO)),"
			"                             not(find-str(_prior[_b],@TIME_AT)),"
			"                             find-str(_next[_b],?/-period-),"
			"                             is-candidate-noun(_b,VB)"
			"                            )"
			"                        ),"
			"                      join(set(taglist-VB,set-at(taglist-VB,_counter,NN)),"
			"                           if(find-str(_prior[_b],[prior_WP]),set(taglist-VB,remove-at(taglist-VB,minus(_counter,1))))"
			"                          )"
			"                     ),"
			"                   if(and(not(find-str(_b,AUX/VB)),find-str(_b,/VB)),"
			"                      set(is-after-aux,true)"
			"                     ),"
			"                 )"
			"              ),"
			"     for-each(_b,_A,"
			"                   if(or(and(find-str(_b,/VBP),"
			"                             or(find-str(_next[_b],/-comma-),find-str(_next[_b],/-period-)),"
			"                             is-candidate-noun(_b,VBP)"
			"                            ),"
			"                         and(find-str(_b,call/VBP),"
			"                             true"
			"                            )"
			"                        ),"
			"                      set(taglist-VBP,set-at(taglist-VBP,_counter,NN))"
			"                     )"
			"              ),"
			"     for-each(_b,_A,"
			"                   if(and(find-str(_b,/VBP),"
			"                          find-str(_next[_b],it/PRP),"
			"                          is-candidate-noun(_b,VBP)"
			"                         ),"
			"                      set(taglist-VBP2,set-at(taglist-VBP2,_counter,NN))"
			"                     )"
			"              ),"
			"     for-each(_b,_A,"
			"                   if(and(find-str(_b,/NN),"
			"                          find-str(_next[_b],/RP),"
			"                          is-candidate-verb(_b,VB)"
			"                         ),"
			"                      set(taglist-VB2,set-at(taglist-VB2,_counter,VB))"
			"                     )"
			"              ),"
			"     for-each(_b,_A,"
			"                   if(and(find-str(_b,/VBZ),"
			"                          find-str(_next[_b],/NN),"
			"                          not(find-str(_next[_b],/NNS)),"
			"                          not(find-str(_next[_b],/NNPS)),"
			"                          not(find-str(_prior[_b],that/WDT)),"
			"                          not(find-str(_prior[_b],that/IN)),"
			"                          is-candidate-noun(_b,NNS),"
			"                          is-candidate-verb(_next[_b],VBP)"
			"                         ),"
			"                      join(set(taglist-switch,set-at(taglist-switch,_counter,NNS)),"
			"                           set(taglist-switch,set-at(taglist-switch,plus(_counter,1),VBP)))"
			"                     )"
			"              ),"
			"     for-each(_b,_A,"
			"                   if(and(find-str(_b,/VBZ),"
			"                          find-str(_next[_b],/VBZ),"
			"                          is-candidate-noun(_next[_b],NNS)"
			"                         ),"
			"                         set(taglist-switch2,set-at(taglist-switch2,plus(_counter,1),NNS))"
			"                     )"
			"              ),"
			"     for-each(_b,_A,"
			"                   if(or(find-str(_b,hit/NN),"
			"                         and(find-str(_b,/NN),"
			"                             not(find-str(_b,/NNS)),"
			"                             not(find-str(_b,/NNPS)),"
			"                             not(find-str(_prior[_b],/PRP$)),"
			"                             find-str(_next[_b],/JJ),"
			"                             is-candidate-verb(_b,VBP)"
			"                            ),"
			"                         and(find-str(_b,/NN),"
			"                             not(find-str(_b,/NNS)),"
			"                             not(find-str(_b,/NNPS)),"
			"                             not(find-str(_prior[_b],/PRP$)),"
			"                             find-str(_prior[_b],/NNP),"
			"                             find-str(_next[_b],/NNP),"
			"                             is-candidate-verb(_b,VBP)"
			"                            ),"
			"                         and(find-str(_b,/NN),"
			"                             not(find-str(_b,/NNS)),"
			"                             not(find-str(_b,/NNPS)),"
			"                             or(find-str(_next[_b],/IN),find-str(_next[_b],/TO)),"
			"                             not(find-str(_prior[_b],/JJR)),"
			"                             not(find-str(_prior[_b],the/DT)),"
			"                             not(find-str(_prior[_b],a/DT)),"
			"                             not(find-str(_prior[_b],/PRP$)),"
			"                             is-candidate-verb(_b,VBP)"
			"                            ),"
			"                         and(find-str(_b,/NN),"
			"                             not(find-str(_b,/NNS)),"
			"                             not(find-str(_b,/NNPS)),"
			"                             not(find-str(_prior[_b],/PRP$)),"
			"                             or(find-str(_next[_b],/-period-),"
			"                                find-str(_next[_b],/-comma-)"
			"                               ),"
			"                             find-str(_prior[_b],/MD),"
			"                             is-candidate-verb(_b,VBP)"
			"                            ),"
			"                         and(find-str(_b,/NN),"
			"                             not(find-str(_b,/NNS)),"
			"                             not(find-str(_b,/NNPS)),"
			"                             not(find-str(_prior[_b],/PRP$)),"
			"                             find-str(_prior[_b],/WDT),"
			"                             not(find-str(_next[_b],/VB)),"
			"                             is-candidate-verb(_b,VBP)"
			"                            ),"
			"                         and(find-str(_b,/RB),"
			"                             not(find-str(_b,/RBS)),"
			"                             not(find-str(_b,/RBR)),"
			"                             not(find-str(_prior[_b],/VB)),"
			"                             not(find-str(_next[_b],/VB)),"
			"                             is-candidate-verb(_b,NN)"
			"                            ),"
			"                         and(find-str(_b,like/IN),"
			"                             find-str(_prior[_b],/NNS),"
			"                             find-str(_next[_b],/VB)"
			"                            )"
			"                        ),"
			"                      join(set(taglist-VBP3,set-at(taglist-VBP3,_counter,VBP)),"
			"                           if(and(find-str(_prior[_b],/JJ),"
			"                                  not(find-str(_prior[_b],/JJS)),"
			"                                  not(find-str(_prior[_b],/JJR)),"
			"                                  is-candidate-adverb(_prior[_b])"
			"                                 ),"
			"                              set(taglist-VBP3,set-at(taglist-VBP3,minus(_counter,1),RB))"
			//"                              join(print(here-is-JJ),print(minus(_counter,1)))"
			"                             )"
			"                          )"
			"                     )"
			"              ),"
			"     for-each(_b,_A,"
			"                   if(and(or(find-str(_b,/NN),find-str(_b,/JJ),find-str(_b,when/WRB),find-str(_b,where/WRB)),"
			"                          is-candidate-verb(_b,VBG),"
			"                          not(find-str(_prior[_b],a/DT)),"
			"                          not(find-str(_prior[_b],an/DT)),"
			"                          not(find-str(_prior[_b],the/DT))"
			"                          not(find-str(_prior[_b],PRP$))"
			"                         ),"
			"                      set(taglist-VBG,set-at(taglist-VBG,_counter,VBG))"
			"                     )"
			"              ),"
			"     for-each(_b,_A,"
			"                   if(or(and(find-str(_b,/JJ),"
			"                             find-str(_prior[_b],/RB),"
			// "                             not(find-str(_prior[_b],ly/RB))," // "the first democratically elected/JJ president"
			"                             is-candidate-verb(_b,VBD)"
			"                            ),"
			"                         and(find-str(_b,/NN),"
			"                             not(find-str(_b,/NNS)),"
			"                             or(find-str(_prior[_b],/NNS),find-str(_prior[_b],/NNPS)),"
			"                             find-str(_next[_b],/TO),"
			"                             is-candidate-verb(_b,VBD),"
			"                            ),"
			"                         and(find-str(_b,/NN),"
			"                             not(find-str(_b,/NNS)),"
			"                             find-str(_next[_b],/DT),"
			"                             is-candidate-verb(_b,VBD)"
			"                            ),"
			"                         and(find-str(_b,/VBN),"
			"                             not(has_subject(_b)),"
			"                             is-candidate-verb(_b,VBD),"
			"                             not(find-str(_prior[_b],/VB)),"
			"                             not(find-str(_prior[_b],/RB))"
			"                            )"
			"                        ),"
			"                      set(taglist-VBD,set-at(taglist-VBD,_counter,VBD))"
			"                     )"
			"              ),"
			"     for-each(_b,_A,"
			"                   if(and(find-str(_b,most/RBS),"
			"                          find-str(_next[_b],/VB)"
			"                         ),"
			"                      set(taglist-RBS,set-at(taglist-RBS,_counter,NN))"
			"                     )"
			"              ),"
			"     for-each(_b,_A,"
			"                   if(and(find-str(_b,off/VB),"
			"                             not(find-str(_b,/VBP)),"
			"                             not(find-str(_b,/VBD)),"
			"                             not(find-str(_b,/VBZ))"
			"                         ),"
			"                      set(taglist-RP,set-at(taglist-RP,_counter,RP))"
			"                     )"
			"              ),"
			"     for-each(_b,_A,"
			"                   if(or(and(find-str(_b,life/NNS),"
			"                             or(find-str(_next[_b],/IN),"
			"                                find-str(_next[_b],/DT)"
			"                               )"
			"                            ),"
			"                         and(or(find-str(_b,/NNS),find-str(_b,/NNPS)),"
			"                             find-str(_prior[_b],/CC),"
			"                             find-str(_next[_b],/NN),"
			"                             not(find-str(_next[_b],/NNS)),"
			"                             not(find-str(_next[_b],/NNPS)),"
			"                             is-candidate-verb(_b,VB)"
			"                            ),"
			"                         and(or(find-str(_b,/NNS),find-str(_b,/NNPS)),"
			"                             or(find-str(_prior[_b],/JJ),find-str(_prior[_b],/NN),find-str(_prior[_b],/WP)),"
			"                             or(find-str(_next[_b],/PRP),find-str(_next[_b],/TO)),"
			"                             not(find-str(_prior[_b],/NNS)),"
			"                             not(find-str(_prior[_b],/NNPS)),"
			"                             is-candidate-verb(_b,VB)"
			"                            ),"
			"                         and(or(find-str(_b,/NNS),find-str(_b,/NNPS)),"
			"                             find-str(_prior[_b],/CC),"
			"                             or(find-str(_next[_b],/PRP),find-str(_next[_b],/PRP$),find-str(_next[_b],/DT)),"
			"                             is-candidate-verb(_b,VB)"
			"                            ),"
			"                         and(or(find-str(_b,/NNS),find-str(_b,/NNPS)),"
			"                             find-str(_prior[_b],/IN),"
			"                             or(find-str(_next[_b],/JJ),find-str(_next[_b],/PRP),find-str(_next[_b],/PRP$),find-str(_next[_b],/DT)),"
			"                             is-candidate-verb(_b,VB)"
			"                            ),"
			"                         and(or(find-str(_b,/NNS),find-str(_b,/NNPS)),"
			"                             or(find-str(_prior[_b],/NN),find-str(_prior[_b],/WP) ),"
			"                             not(find-str(_prior[_b],/NNS)),"
			"                             not(find-str(_prior[_b],/NNPS)),"
			"                             or( find-str(_next[_b],/NNP), find-str(_next[_b],/DT) ),"
			"                             not(find-str(_next[_b],/NNPS)),"
			"                             is-candidate-verb(_b,VB)"
			"                            )"
			"                        ),"
			"                      set(taglist-VBZ2,set-at(taglist-VBZ2,_counter,VBZ))"
			"                     )"
			"              ),"
			"     for-each(_b,_A,"
			"                   if(and(find-str(_b,/JJ),"
			"                          find-str(_next[_b],/VB),"
			"                          is-candidate-adverb(_b)"
			"                         ),"
			"                      set(taglist-RB,set-at(taglist-RB,_counter,RB))"
			"                     )"
			"              ),"
			"     for-each(_b,_A,"
			"                   if(or(equal(find-str-pos(_b,can/NN),0),"
			"                         equal(find-str-pos(_b,may/NN),0)"
			"                        ),"
			"                      set(taglist-MD,set-at(taglist-MD,_counter,MD))"
			"                     )"
			"              ),"
			"     for-each(_b,_A,"
			"                   if(or(and(find-str(_b,/VBZ),"
			"                             find-str(_next[_b],/JJR),"
			"                             is-candidate-noun(_b,NNS)"
			"                            ),"
			"                         and(find-str(_b,/VBZ),"
			"                             find-str(_prior[_b],that/IN),"
			"                             find-str(_next[_b],that/IN),"
			"                             is-candidate-noun(_b,NNS)"
			"                            ),"
			"                         and(find-str(_b,/VBZ),"
			"                             find-str(_prior[_b],that/IN),"
			"                             find-str(_next[_b],that/WDT),"
			"                             is-candidate-noun(_b,NNS)"
			"                            ),"
			"                         and(find-str(_b,/VBZ),"
			"                             find-str(_prior[_b],that/IN),"
			"                             find-str(_next[_b],which/WDT),"
			"                             is-candidate-noun(_b,NNS)"
			"                            ),"
			"                         and(find-str(_b,/VBZ),"
			"                             find-str(_prior[_b],/WP),"
			"                             find-str(_next[_b],/VB),"
			"                             is-candidate-noun(_b,NNS)"
			"                            )"
			"                        ),"
			"                      set(taglist-NNS,set-at(taglist-NNS,_counter,NNS))"
			"                     )"
			"              ),"
			"     for-each(_b,_A,"
			"                   if(or(and(find-str(_b,/VBZ),"
			"                             find-str(_prior[_b],that/IN),"
			"                             find-str(_next[_b],/JJR),"
			"                             is-candidate-noun(_b,NNS)"
			"                            )"
			"                         and(find-str(_b,/VBZ),"
			"                             equal(_counter,1),"// start position
			"                             find-str(_next[_b],/JJR),"
			"                             is-candidate-noun(_b,NNS)"
			"                            )"
			"                        ),"
			"                      taglist-NNS,set-at(taglist-NNS,_counter,NNS)"
			"                     )"
			"              ),"
			"     for-each(_b,_A,"
			"                   if(and(find-str(_b,that/DT),"
			"                          find-str(_next[_b],/VB),"
			"                          not(find-str(_prior[_b],/IN))"
			"                         ),"
			"                      join(set(taglist-WDT,set-at(taglist-WDT,_counter,WDT)),"
			"                           if(find-str(_next[_b],/VBN),set(taglist-WDT,set-at(taglist-WDT,plus(_counter,1),VBD)) )"
			"                          )"
			"                     )"
			"              ),"
			"     set(can-add-to-taglist-THAT,true),"
			"     for-each(_b,_A,"
			"                   if(and(can-add-to-taglist-THAT,"
			"                          find-str(_b,/NNP),"
			"                          find-str(_prior[_b],/NN),"
			"                          find-str(_next[_b],/VB),"
			"                          not(has-subject(_next[_b])),"
			"                        ),"
			"                      join(set(taglist-THAT,insert-at(taglist-THAT,_counter,WDT(that))),"
			"                           set(can-add-to-taglist-THAT,false)"
			"                          )"
			"                     )"
			"              ),"
			"     for-each(_b,_A,"
			"                   if(and(not(is-conditional),"
			"                          can-add-to-taglist-THAT,"
			"                          find-str(_b,/PRP),"
			"                          find-str(_prior[_b],/NN)"
			"                         ),"
			"                      join(set(taglist-THAT,insert-at(taglist-THAT,_counter,IN(that))),set(can-add-to-taglist-THAT,false))"
			"                     )"
			"              ),"
			"     set(can-add-comma,false),"
			"     set(indirect-verb-list,list(give,wish,make,buy,pay,show,call,regard_as,set,promise,named)),"
			"     for-each(_b,_A,"
			"                   join(if(find-str-in-tree(indirect-verb-list,ff-name),set(can-add-comma,false)),"
			"                   if(or(and(not(is-question),"
			"                             find-str(_b,/VBG),"
			"                             find-str(_prior[_b],/NN)"
			"                            ),"
			"                         and(find-str(_b,/DT),"
			"                             find-str(_prior[_b],/NNP),"
			"                             not(after-indirect-verb)"
			"                            ),"
			"                         and(find-str(_b,/NN),"
			"                             find-str(_prior[_b],PASSIVE_POST_AUX),"
			"                             not(find-str-in-tree(indirect-verb-list,get-lemma(_prior[_b]))),"
			"                             join(print(here-is-NN-lemma),print(get-lemma(_prior[_b]))),"
			"                             not(is-question)"
			"                            ),"
			"                         and(not(is-question),"
			"                             find-str(_b,/NN),"
			"                             find-str(_next[_b],/VB),"
			"                             not(has-subject(_next[_b])),"
			"                             find-str(_prior[_b],/NN),"
			"                            )"
			"                        ),"
			"                      join(if(can-add-comma,set(taglist-comma,insert-at(taglist-comma,_counter,-comma-(-comma-))))"
			"                              ,set(can-add-comma,true))"
			"                     )"
			"              )),"
			"     for-each(_b,_A,"
			"                   if(or(and(can-add-to-taglist-THAT,"
			"                             find-str(_b,/VBG),"
			"                             find-str(_prior[_b],/NN),"
			"                             not(has-subject(_b))"
			"                            ),"
			"                         false"
			"                        ),"
			"                      join(set(taglist-THAT,insert-at(taglist-THAT,_counter,VB(be))),"
			"                           set(taglist-THAT,insert-at(taglist-THAT,_counter,WDT(that))),"
			"                           set(can-add-to-taglist-THAT,false)"
			"                          )"
			"                     )"
			"              ),"
			"     for-each(_b,_A,"
			"                   if(or(and(can-add-to-taglist-THAT,"
			"                             find-str(_b,/VBD),"
			"                             not(equal(get-lemma(_b,VBD),be)),"
			"                             not(equal(get-lemma(_b,VBD),have)),"
			"                             not(equal(get-lemma(_b,VBD),do)),"
			"                             not(find-str(get-lemma(_b,VBD),AUX)),"
			"                             not(is-question)," /// You don't want: what dictator [that be] died in an island?
			"                             find-str(_next[_b],/IN),"
			"                             find-str(_prior[_b],/-comma-)"
			"                            ),"
			"                         and(can-add-to-taglist-THAT,"
			"                             find-str(_b,/VBD),"
			"                             not(is-question)," /// You don't want: what dictator [that be] died in an island?
			"                             not(equal(get-lemma(_b,VBD),be)),"
			"                             not(equal(get-lemma(_b,VBD),have)),"
			"                             not(equal(get-lemma(_b,VBD),do)),"
			"                             not(find-str(get-lemma(_b,VBD),AUX)),"
			"                             not(and(is-verb-levin(_b,verb.communication),not(find-str(_next[_b],/TO)))),"
			"                             or(find-str(_next[_b],/IN),find-str(_next[_b],/-comma-)),"
			"                             not(find-str(_next[_b],that/IN)),"
			"                             find-str(_prior[_b],/NN)"
			"                            ),"
			"                         and(can-add-to-taglist-THAT,"
			"                             find-str(_b,call/VBD),"
			"                             not(find-str(_next[_b],that/IN)),"
			"                             find-str(_prior[_b],/CD)"
			"                            ),"
			"                         and(can-add-to-taglist-THAT,"
			"                             find-str(_b,/VBD),"
			"                             not(equal(get-lemma(_b,VBD),be)),"
			"                             not(equal(get-lemma(_b,VBD),have)),"
			"                             not(equal(get-lemma(_b,VBD),do)),"
			"                             not(find-str(get-lemma(_b,VBD),AUX)),"
			"                             not(is-question)," /// You don't want: what dictator [that be] died in an island?
			"                             not(find-str(_next[_b],that/IN)),"
			"                             not(equal(extract-subject(_b),extract-first-tag(_prior[_b])))"
			"                            )"
			"                        ),"
			"                      join(print(here-is-lemma),print(get-lemma(_b,VBD)),set(taglist-THAT,set-at(taglist-THAT,_counter,VBN)),"
			"                           set(taglist-THAT,insert-at(taglist-THAT,_counter,VB(be))),"
			"                           set(taglist-THAT,insert-at(taglist-THAT,_counter,WDT(that))),"
			"                           set(can-add-to-taglist-THAT,false),"
			"                           print(here-is-THAT)"
			"                          )"
			"                     )"
			"              ),"
			"     for-each(_b,_A,"
			"                   if(and(can-add-to-taglist-THAT,find-str(_b,/VBD),"
			"                             not(equal(get-lemma(_b,VBD),be)),"
			"                             not(equal(get-lemma(_b,VBD),have)),"
			"                             not(equal(get-lemma(_b,VBD),do)),"
			"                             not(find-str(get-lemma(_b,VBD),AUX)),"
			"                          find-str(_next[_b],/RB),"
			"                          find-str(_prior[_b],/NNS)"
			"                         ),"
			"                      join(set(taglist-THAT,insert-at(taglist-THAT,plus(_counter,1),-comma-(-END-PRN-))),"
			"                           set(taglist-THAT,set-at(taglist-THAT,_counter,VBN)),"
			"                           set(taglist-THAT,insert-at(taglist-THAT,_counter,VB(be))),"
			"                           set(taglist-THAT,insert-at(taglist-THAT,_counter,WDT(that)))"
			"                          )"
			"                     )"
			"              ),"
			"     if(is-question,"// is the boy who watches the tv smiling?
			"        for-each(_b,_A,"
			"                 if(and(find-str(_prior[_b],/WRB),"
			"                        find-str(_next[_b],/NN),"
			"                        find-str(_b,/VB),"
			"                        is-candidate-adjective(_b,VB)"
			"                       ),"
			"                    set(taglist-JJ,set-at(taglist-JJ,_counter,JJ))"
			"                   )"
			"                )"
			"       ),"
			"     if(is-question,"// found/VBD ? -> found/VBN ?
			"        for-each(_b,_A,"
			"                 if(and(find-str(_b,find/VBD),"
			"                        find-str(_next[_b],?/)"
			"                       ),"
			"                    set(taglist-VBN,set-at(taglist-VBN,_counter,VBN))"
			"                   )"
			"                )"
			"       ),"
			"     if(is-question,"// dog/VB do/VB -> dog/NN do/VB
			"        for-each(_b,_A,"
			"                 if(and(find-str(_b,/VB),"
			"                        find-str(_next[_b],do/VB),"
			"                        is-candidate-noun(_b,VB)"
			"                       ),"
			"                    set(taglist-NN,set-at(taglist-NN,_counter,NN))"
			"                   )"
			"                )"
			"       ),"
			"     set(is-after-be,false),"
			"     if(is-question,"
			"        for-each(_b,_A,"
			"                 join(if(find-str(_b,be/VB),"
			"                         set(is-after-be,true)"
			"                        ),"
			"                      if(and(find-str(_b,/JJ),"
			"                             is-candidate-verb(_b,VBN),"
			"                             not(find-str(_prior[_b],/DT))"
			"                            ),"
			"                         set(taglist-VBN,set-at(taglist-VBN,_counter,VBN))"
			"                        ),"
			"                      if(find-str(_b,/VB),"
			"                         set(is-after-be,false)"
			"                        )"
			"                    )"
			"              )"
			"       ),"
			"     if(is-question,"// is the boy who watches the tv smiling?
			"        join(set(has-AUX,false),set(has-PRN-candidate-verb,false),"
			"             for-each(_b,_A,"
			"                      join(if(and(has-AUX,has-PRN-candidate-verb,"// not(find-str(_prior[_b],/-comma-)),"
			"                                  or(find-str(_prior[_b],/NN),find-str(_prior[_b],/JJ)),"
			"                                  or(find-str(_b,/VBG),find-str(_b,/VBN),find-str(_b,/VBD)),"
			"                                  true"// find-str(_next[_b],?)"
			"                                 ),"
			"                              join(print(here-is-QM),print(_b),print(_prior[_b]),"
			"                                   if(find-str(_b,/VBD),set(taglist-PRN-END,set-at(taglist-PRN-END,_counter,VBN))),"
			"                                   set(taglist-PRN-END,insert-at(taglist-PRN-END,_counter,-comma-(-END-PRN-))),print(taglist-PRN-END),"
			"                                   set(has-AUX,false),"
			"                                   set(has-PRN-candidate-verb,false)"
			"                                  )"
			"                             ),"
			"                           if(and(has-AUX,not(has-PRN-candidate-verb),or(and(find-str(_prior[_b],/TO),find-str(_b,/VB)),find-str(_b,/VBZ),find-str(_b,/VBP),find-str(_b,/VBD))),join(set(has-PRN-candidate-verb,true),print(HERE3))),"
			"                           if(and(not(has-AUX),find-str(_b,be/VB)),join(set(has-AUX,true)))"
			//"                           if(and(print(HERE),not(has-AUX),find-str(_b,/VB),equal(get-lemma(_b),be)),join(set(has-AUX,true),print(HERE2)))"
			"                          )"
			"                     )"
			"           )"
			"       ),"
			"     if(is-question,"// is the boy who watches the tv in the living room?
			"        join(set(has-AUX,false),set(has-PRN-candidate-verb,false),"
			"             for-each(_b,_A,"
			"                      join(print(_b),if(and(has-AUX,has-PRN-candidate-verb,"// not(find-str(_prior[_b],/-comma-)),"
			"                                  or(find-str(_prior[_b],/NN),find-str(_prior[_b],/JJ)),"
			"                                  or(find-str(_b,/IN),find-str(_b,/TO)),"
			"                                  true"// find-str(_next[_b],?)"
			"                                 ),"
			"                              join(print(here-is-QM),print(_b),print(_prior[_b]),"
			"                                   set(taglist-PRN-END,insert-at(taglist-PRN-END,_counter,-comma-(-END-PRN-))),print(taglist-PRN-END),"
			"                                   set(has-AUX,false),"
			"                                   set(has-PRN-candidate-verb,false)"
			"                                  )"
			"                             ),"
			"                           if(and(has-AUX,not(has-PRN-candidate-verb),or(find-str(_b,/VBZ),find-str(_b,/VBP),find-str(_b,/VBD))),join(set(has-PRN-candidate-verb,true),print(HERE3))),"
			"                           if(and(print(HERE),not(has-AUX),find-str(_b,be/VB)),join(set(has-AUX,true),print(HERE2)))"
			//"                           if(and(print(HERE),not(has-AUX),find-str(_b,/VB),equal(get-lemma(_b),be)),join(set(has-AUX,true),print(HERE2)))"
			"                          )"
			"                     )"
			"           )"
			"       ),"
			"     if(is-question,"// the/DT only/RB->JJ line/NN
			"        join(for-each(_b,_A,"
			"                      if(and(find-str(_b,/RB),"
			"                             find-str(_prior[_b],/DT),"
			"                             find-str(_next[_b],/NN),"
			"                             is-candidate-adjective(_b),"
			"                             print(here-is-only),print(_b)"
			"                            ),"
			"                         set(taglist-JJ,set-at(taglist-JJ,_counter,JJ))"
			"                        )"
			"                     )"
			"            )"
			"       ),"
			"       join(set(has-communication-verb,false),"
			"            for-each(_b,_A,"
			"                     join(print(here-is-comma-levin2),print(_b),if(and(has-communication-verb,"
			"                                 find-str(_b,/-comma-),"
			"                                 find-str(_next[_b],[verbatim]_)"
			"                                ),"
			"                             set(taglist-ALLOCUTION,replace-at(taglist-ALLOCUTION,_counter,IN(that)))"
			"                            ),"
			"                           if(is-verb-levin(_b,verb.communication),set(has-communication-verb,true)),"
			"                           if(and(find-str(_b,/-comma-),not(find-str(_next[_b],[verbatim]_))),has-communication-verb)"
			"                         )"
			"                    )"
			"           ),"
			"     for-each(_b,_A,"
			"                   if(and(false,can-add-to-taglist-THAT,find-str(_b,/VBN),"
			"                          not(equal(get-lemma(_b,VBN),be)),"
			"                          or(find-str(_prior[_b],/NN),find-str(_prior[_b],/CD),find-str(_prior[_b],-comma-)),"
			"                          not(is-question)"
			"                         ),"
			"                      join(set(taglist-THAT,insert-at(taglist-THAT,_counter,VB(be))),"
			"                           set(taglist-THAT,insert-at(taglist-THAT,_counter,WDT(that))),"
			"                           set(can-add-to-taglist-THAT,false))"
			"                     )"
			"              ),"
			"     for-each(_b,_A,"
			"                   if(and(find-str(_b,/CD),"
			"                          find-str(_prior[_b],/IN),"
			"                          not(find-str(_prior[_b],of/IN)),"
			"                          not(find-str(_prior[_b],by/IN)),"
			"                          not(find-str(_prior[_b],that/IN)),"
			"                          not(find-str(_next[_b],/JJ)),"
			"                          not(and(find-str(_next[_b],/NN),not(or(find-str(_next[_b],/NNS),find-str(_next[_b],/NNPS)))))"
			"                         ),"
			"                      set(taglist-CD-date,set-at(taglist-CD-date,_counter,CD-DATE))"
			"                     )"
			"              ),"
			"     for-each(_b,_A,"
			"                   if(or(and(find-str(_b,/VBD),"
			"                             is-candidate-adjective(_b,VBN),"
			"                             or(find-str(_prior[_b],/DT),find-str(_prior[_b],/JJ)),"
			"                             find-str(_next[_b],/NN)"
			"                            ),"
			"                         and(is-question,"
			"                             find-str(_b,/VBN),"
			"                             is-candidate-adjective(_b,VBN),"
			"                             find-str(_prior[_b],/JJ),"
			"                             find-str(_next[_b],/NN)"
			"                            ),"
			"                         and(find-str(_b,/VBN),"
			"                             is-candidate-adjective(_b,VBN),"
			"                             find-str(_prior[_b],/VBD),"
			"						not(equal(get-lemma(_prior[_b],VBD),be)),"
			"						not(equal(get-lemma(_prior[_b],VBD),have)),"
			"                             find-str(_next[_b],/NN)"
			"                            )"
			"                        ),"
			"                      set(taglist-JJ,set-at(taglist-JJ,_counter,JJ))"
			"                     )"
			"              ),"
			"     for-each-tree-height(_c,1,_C,"
			"                          if(and(find(no-VBD,_c),is-candidate-adjective(_c)),"
			"                             set(taglist-VBD,set-at(taglist-VBD,_counter,JJ)))"
			"                         )"
			"    )");

	get_candidate_verbs.insert("_A", drtvect);
	get_candidate_verbs.insert("_B", taglist);
	get_candidate_verbs.insert("_C", enhanced_tree);
	get_candidate_verbs.insert("_D", error);

	Knowledge k;
	Engine engine(&k);
	CodePred result = engine.run(get_candidate_verbs);


	vector<FuzzyPred> predlist;
	predlist = engine.getList<FuzzyPred>("taglist");
	if (!(predlist.size() && predlist.at(0) == Predicate("false")))
		to_return.push_back(predlist);
	predlist = engine.getList<FuzzyPred>("like-taglist");
	if (!(predlist.size() && predlist.at(0) == Predicate("false")))
		to_return.push_back(predlist);
	predlist = engine.getList<FuzzyPred>("taglist-VBZ");
	if (!(predlist.size() && predlist.at(0) == Predicate("false")))
		to_return.push_back(predlist);
	predlist = engine.getList<FuzzyPred>("taglist-VBD");
	if (!(predlist.size() && predlist.at(0) == Predicate("false")))
		to_return.push_back(predlist);
	predlist = engine.getList<FuzzyPred>("taglist-IN");
	if (!(predlist.size() && predlist.at(0) == Predicate("false")))
		to_return.push_back(predlist);
	predlist = engine.getList<FuzzyPred>("taglist-WDT");
	if (!(predlist.size() && predlist.at(0) == Predicate("false")))
		to_return.push_back(predlist);
	predlist = engine.getList<FuzzyPred>("taglist-VB");
	if (!(predlist.size() && predlist.at(0) == Predicate("false")))
		to_return.push_back(predlist);
	predlist = engine.getList<FuzzyPred>("taglist-VBG");
	if (!(predlist.size() && predlist.at(0) == Predicate("false")))
		to_return.push_back(predlist);
	predlist = engine.getList<FuzzyPred>("taglist-VBP");
	if (!(predlist.size() && predlist.at(0) == Predicate("false")))
		to_return.push_back(predlist);
	predlist = engine.getList<FuzzyPred>("taglist-VBP2");
	if (!(predlist.size() && predlist.at(0) == Predicate("false")))
		to_return.push_back(predlist);
	predlist = engine.getList<FuzzyPred>("taglist-VBP3");
	if (!(predlist.size() && predlist.at(0) == Predicate("false")))
		to_return.push_back(predlist);
	predlist = engine.getList<FuzzyPred>("taglist-RBS");
	if (!(predlist.size() && predlist.at(0) == Predicate("false")))
		to_return.push_back(predlist);
	predlist = engine.getList<FuzzyPred>("taglist-NN");
	if (!(predlist.size() && predlist.at(0) == Predicate("false")))
		to_return.push_back(predlist);
	predlist = engine.getList<FuzzyPred>("taglist-VBN");
	if (!(predlist.size() && predlist.at(0) == Predicate("false")))
		to_return.push_back(predlist);
	predlist = engine.getList<FuzzyPred>("taglist-VBZ");
	if (!(predlist.size() && predlist.at(0) == Predicate("false")))
		to_return.push_back(predlist);
	predlist = engine.getList<FuzzyPred>("taglist-RP");
	if (!(predlist.size() && predlist.at(0) == Predicate("false")))
		to_return.push_back(predlist);
	predlist = engine.getList<FuzzyPred>("taglist-RB");
	if (!(predlist.size() && predlist.at(0) == Predicate("false")))
		to_return.push_back(predlist);
	predlist = engine.getList<FuzzyPred>("taglist-MD");
	if (!(predlist.size() && predlist.at(0) == Predicate("false")))
		to_return.push_back(predlist);
	predlist = engine.getList<FuzzyPred>("taglist-THAT");
	if (!(predlist.size() && predlist.at(0) == Predicate("false")))
		to_return.push_back(predlist);
	predlist = engine.getList<FuzzyPred>("taglist-NNS");
	if (!(predlist.size() && predlist.at(0) == Predicate("false")))
		to_return.push_back(predlist);
	predlist = engine.getList<FuzzyPred>("taglist-comma");
	if (!(predlist.size() && predlist.at(0) == Predicate("false")))
		to_return.push_back(predlist);
	predlist = engine.getList<FuzzyPred>("taglist-VB2");
	if (!(predlist.size() && predlist.at(0) == Predicate("false")))
		to_return.push_back(predlist);
	predlist = engine.getList<FuzzyPred>("taglist-VBZ2");
	if (!(predlist.size() && predlist.at(0) == Predicate("false")))
		to_return.push_back(predlist);
	predlist = engine.getList<FuzzyPred>("taglist-CD-date");
	if (!(predlist.size() && predlist.at(0) == Predicate("false")))
		to_return.push_back(predlist);
	predlist = engine.getList<FuzzyPred>("taglist-combo");
	if (!(predlist.size() && predlist.at(0) == Predicate("false")))
		to_return.push_back(predlist);
	predlist = engine.getList<FuzzyPred>("taglist-JJ");
	if (!(predlist.size() && predlist.at(0) == Predicate("false")))
		to_return.push_back(predlist);
	predlist = engine.getList<FuzzyPred>("taglist-switch");
	if (!(predlist.size() && predlist.at(0) == Predicate("false")))
		to_return.push_back(predlist);
	predlist = engine.getList<FuzzyPred>("taglist-switch2");
	if (!(predlist.size() && predlist.at(0) == Predicate("false")))
		to_return.push_back(predlist);
	predlist = engine.getList<FuzzyPred>("taglist-PRN-END");
	if (!(predlist.size() && predlist.at(0) == Predicate("false")))
		to_return.push_back(predlist);
	predlist = engine.getList<FuzzyPred>("taglist-ALLOCUTION");
	if (!(predlist.size() && predlist.at(0) == Predicate("false")))
		to_return.push_back(predlist);
	predlist = engine.getList<FuzzyPred>("taglist-error");
	if (!(predlist.size() && predlist.at(0) == Predicate("false")))
		to_return.push_back(predlist);


	return to_return;
}

static vector<int> deny_PRN_positions(const DrtVect &drtvect, const vector<FuzzyPred> &taglist, const FuzzyPred &parsed_tree)
{
	vector<int> to_return;

	PredTree enhanced_tree = parsed_tree.pred();
	PredTree::height_iterator hi(enhanced_tree, 0);
	vector<PredTree::iterator> iters;
	for (; hi != enhanced_tree.end(); ++hi) {
		iters.push_back(hi);
	}
	for (int n = 0; n < drtvect.size() && n < iters.size(); ++n) {
		stringstream ss;
		drtvect.at(n).print(ss);
		PredTree::iterator pi = iters.at(n);
		enhanced_tree.replace(Predicate(ss.str()).pred(), pi);
	}

	CodePred get_candidate_verbs("join(set(unconnected,not-connected-to-verbs(_A)),"
			"     set(taglist,_B),"
			"     set(prnlist,list),"
			"     set(prnlist2,list),"
			"     set(prnlist3,list),"
			"     for-each-tree(_a,_C,join(if(and(find-str(_a,PRN),"
			"                                     find(unconnected,_a)"
			"                                    )," // an element that is unconnected within a PRN triggers a suppression of the PRN
			"                                 set(prnlist,add-list(prnlist,_a))"
			"                                ),"
			"                              if(and(find-str(_a,PRN),"
			"                                     find-str-in-tree(_a,SBAR)"
			"                                    ),"// A PRN with a SBAR inside triggers a suppression of the PRN
			"                                 set(prnlist2,add-list(prnlist2,_a))"
			"                                ),"
			"                              if(and(find-str(_a,PRN),"
			"                                     find-str-in-tree(_a,because/IN)"
			"                                    ),"// A PRN with a "because" inside triggers a suppression of the PRN
			"                                 set(prnlist3,add-list(prnlist3,_a))"
			"                                )"
			"                             )"
			"                  ),"
			"     set(allprnlist,list),"
			"     for-each(_b,prnlist,"
			"              join(set(no-prn,true),"
			"                   for-each-tree-height(_a,1,_b,if(or(and(not(find-str(_a,-comma-)),"
			"                                                          find-str-height(_a,0,name)"
			"                                                         ),"
			"                                                      false"
			"                                                     ),"
			"                                                   join(set(no-prn,false),break)"
			"                                                  )"
			"                                       ),"
			"                   if(no-prn,"
			"                      set(allprnlist,add-list(allprnlist,_b))"
			"                     )"
			"                  )"
			"             ),"
			"     for-each(_b,prnlist2,"
			"              set(allprnlist,add-list(allprnlist,_b))"
			"             ),"
			"     for-each(_b,prnlist3,"
			"              set(allprnlist,add-list(allprnlist,_b))"
			"             ),"
			"     set(positions,list),"
			"     set(drtvect,_A),"
			"     for-each(_c,drtvect,"
			"              if(and(find(allprnlist,_c),"
			"                     not(and(find-str(_c,/VB),find-str(_next[_c],/VB),not(find-str(_next[_c],/VBG))))"
			"                    )"
			"                  set(positions,add-list(positions,_counter))"
			"                 )"
			"             ),"
			"    for-each(_b,drtvect,"
			"                   if(or(and(find-str(_b,/IN),"
			"                             find-str(_next[_b],/PRP)),"
			"                         and(find-str(_b,/PRP),"
			"                             find-str(_next[_b],/VBZ)),"
			"                         and(find-str(_b,/PRP),"
			"                             find-str(_next[_b],/VBD))"
			"                        ),"
			"                      set(positions,add-list(positions,_counter))"
			"                     )"
			"            ),"
			"    for-each(_b,drtvect,"
			"                   if(or(and(find-str(_b,/NNP),"
			"                             find-str(_next[_b],/-comma-),"
			"                             find-str(_prior[_b],/-comma-)"
			"                             )"
			"                        ),"
			"                      join(set(positions,add-list(positions,_counter)),"
			"                           set(positions,add-list(positions,plus(_counter,1))),"
			"                           set(positions,add-list(positions,minus(_counter,1))))"
			"                     )"
			"            ),"
			"    print(positions)"
			"   )");
	get_candidate_verbs.insert("_A", drtvect);
	get_candidate_verbs.insert("_B", taglist);
	get_candidate_verbs.insert("_C", enhanced_tree);
	Knowledge k;
	Engine engine(&k);

	clock_t start;

	CodePred result = engine.run(get_candidate_verbs);



	vector<int> noprn;
	noprn = engine.getList<int>("positions");
	if (!(noprn.size() && noprn.at(0) == -1))
		to_return = noprn;


	return to_return;
}

static vector<int> deny_SBAR_positions(const DrtVect &drtvect, const vector<FuzzyPred> &taglist, const FuzzyPred &parsed_tree)
{
	vector<int> to_return;

	PredTree enhanced_tree = parsed_tree.pred();
	PredTree::height_iterator hi(enhanced_tree, 0);
	vector<PredTree::iterator> iters;
	for (; hi != enhanced_tree.end(); ++hi) {
		iters.push_back(hi);
	}
	for (int n = 0; n < drtvect.size() && n < iters.size(); ++n) {
		stringstream ss;
		drtvect.at(n).print(ss);
		PredTree::iterator pi = iters.at(n);
		enhanced_tree.replace(Predicate(ss.str()).pred(), pi);
	}

	if (debug)
		cout << "ETREE::: " << Predicate(enhanced_tree) << endl;

	CodePred get_candidate_verbs("join(set(positions,list),"
			"     set(drtvect,_A),"
			"     for-each(_c,drtvect,"
			"              if(and(find-str(_c,/IN),"
			"                     not(find-str(_c,that/IN)),"
			"                     find-str(extract-first-tag(_c),name),"
			"                     find-str(extract-second-tag(_c),verb)"
			"                    ),"
			"                  set(positions,add-list(positions,_counter))"
			"                )"
			"             ),"
			"     for-each-tree(_a,_C,if(and(find-str(_a,PP),"
			"                                find-str-sub(_a,PP)"
			"                               ),"
			"                            join(set(nosbar,add-list(nosbar,first-child(_a))),"
			"                                 break"
			"                                )"
			"                           )"
			"                  ),"
			"     for-each(_c,drtvect,"
			"                if(find(nosbar,_c),"
			"                   set(positions,add-list(positions,_counter))"
			"                  )"
			"             ),"
			"    for-each(_c,drtvect,"
			"             if(and(find-str(_c,/-comma-),"
			"                    find-str(_next[_c],/JJ),"
			"                    find-str(_prior[_c],/JJ)"
			"                   ),"
			"                set(positions,add-list(positions,_counter))"
			"               )"
			"            ),"
			"    for-each(_c,drtvect,"
			"             if(and(find-str(_c,/JJ),"
			"                    find-str(_next[_c],/IN),"
			"                    find-str(_prior[_c],/VB)"
			"                   ),"
			"                set(positions,add-list(positions,plus(_counter,1)))"
			"               )"
			"            ),"
			"     set(all-no-sbar-trees,list),"
			"     for-each-tree(_a,_C,join(if(and(find-str(_a,SBAR),"
			"                                     find-str-in-tree(_a,/TO),"
			"                                     find-str-in-tree(_a,/VB)"
			// "                                     not(find-str-in-tree(_a,/VBP)),"
			// "                                     not(find-str-in-tree(_a,/VBD)),"
			// "                                     not(find-str-in-tree(_a,/VBN))"
			"                                    ),"// (SBAR CC NN (TO VB)) should not be an sbar
			"                                 set(all-no-sbar-trees,add-list(all-no-sbar-trees,_a))"
			"                                ),"
			"                              print(_a)"
			"                             )"
			"                  ),"
			"     print(here-is-all-no-sbar-trees),"
			"     print(all-no-sbar-trees),"
			"     print(positions),"
			"     for-each(_b,all-no-sbar-trees,"
			"                 for-each(_a,drtvect,if(find-sub(_b,_a),"
			"                                        set(positions,add-list(positions,_counter))"
			"                                       )"
			"                         )"
			"             ),"
			"    for-each(_b,drtvect,"
			"                   if(or(and(find-str(_b,of/IN),"
			"                             find-str(_prior[_b],and/CC)"
			"                            ),"
			"                         and(find-str(_b,and/CC),"
			"                             find-str(_prior[_b],/JJ),"
			"                             find-str(_next[_b],/JJ)"
			"                            )"
			"                        ),"
			"                      set(positions,add-list(positions,_counter))"
			"                     )"
			"            ),"
			"    print(positions)"
			"   )");

	get_candidate_verbs.insert("_A", drtvect);
	get_candidate_verbs.insert("_B", taglist);
	get_candidate_verbs.insert("_C", enhanced_tree);
	Knowledge k;
	Engine engine(&k);

	clock_t start;

	CodePred result = engine.run(get_candidate_verbs);


	vector<int> nosbar;
	nosbar = engine.getList<int>("positions");
	if (!(nosbar.size() && nosbar.at(0) == -1))
		to_return = nosbar;


	return to_return;
}

static FuzzyPred correct_parsed_tree(const DrtVect &drtvect, const vector<FuzzyPred> &taglist, const FuzzyPred &parsed_tree)
{
	FuzzyPred to_return;

	PredTree enhanced_tree = parsed_tree.pred();
	PredTree::height_iterator hi(enhanced_tree, 0);
	vector<PredTree::iterator> iters;
	for (; hi != enhanced_tree.end(); ++hi) {
		iters.push_back(hi);
	}
	for (int n = 0; n < drtvect.size() && n < iters.size(); ++n) {
		stringstream ss;
		drtvect.at(n).print(ss);
		PredTree::iterator pi = iters.at(n);
		enhanced_tree.replace(Predicate(ss.str()).pred(), pi);
	}

	if (debug)
		cout << "ETREE_CORRECT_PARSED::: " << Predicate(enhanced_tree) << endl;

	CodePred get_candidate_verbs(
			"join(set(etree,_C),"
			"     defun(first-foot(_a),'join(set(iter,_a),while(first-child(first-child(iter)),set(iter,first-child(iter))),iter)),"
			"     set(indirect-verb-list,list(give,wish,make,buy,pay,show,call,regard_as,set,promise,name)),"
			"     set(tlist,list),"
			"     set(tlist,add-list(tlist,pair(VP(_a,NP(_b1),NP(_c)),VP(V(_a),NP(_b1,_c)) ) ) ),"
			"     set(tlist,add-list(tlist,pair(VP(_a,NP(_b1,_b2),NP(_c)),VP(V(_a),NP(_b1,_b2,_c)) ) ) ),"
			"     set(tlist,add-list(tlist,pair(VP(_a,NP(_b1,_b2,_b3),NP(_c)),VP(V(_a),NP(_b1,_b2,_b3,_c)) ) ) ),"
			"     set(tlist,add-list(tlist,pair(VP(_a,NP(_b1),NP(_c),_d),VP(V(_a),NP(_b1,_c),_d) ) ) ),"
			"     set(tlist,add-list(tlist,pair(VP(_a,NP(_b1,_b2),NP(_c),_d),VP(V(_a),NP(_b1,_b2,_c),_d) ) ) ),"
			"     set(tlist,add-list(tlist,pair(VP(_a,NP(_b1,_b2,_b3),NP(_c),_d),VP(V(_a),NP(_b1,_b2,_b3,_c),_d) ) ) ),"
			"     set(tlist-NNS,list),"
			"     set(tlist-NNS,add-list(tlist-NNS,pair(VP(_a,NP(NNS(_b),NNS(_c))),VP(_a,NP(NNS(_b)),NP(NNS(_c))) ) ) ),"
			"     set(tlist-NNS,add-list(tlist-NNS,pair(VP(_a,NP(NNS(_b),NNS(_c)),_d),VP(_a,NP(NNS(_b)),NP(NNS(_c)),_d) ) ) ),"
			"     set(tlist-NNS,add-list(tlist-NNS,pair(VP(VBD(_a),NP(DT(_b1),NN(_b2),PP(IN(_b3),NP(DT(_b4),JJ(_b5),NNS(_b6),NNP(_c))))),"
			"                                           VP(VBD(_a),NP(DT(_b1),NN(_b2),PP(IN(_b3),NP(DT(_b4),JJ(_b5),NNS(_b6)))),NP(NNP(_c))) ))),"
			"     set(tlist-NNS,add-list(tlist-NNS,pair(VP(VBD(_a),NP(DT(_b1),NN(_b2),NNP(_c))),"
			"                                           VP(VBD(_a),NP(DT(_b1),NN(_b2)),NP(NNP(_c))) ))),"
			"     set(tlist-NNS,add-list(tlist-NNS,pair(VP(VBP(_a),NP(NNS(_b),NN(_c))),"
			"                                           VP(VBP(_a),NP(NNS(_b)),NP(NN(_c))) ))),"
			"     set(tlist-NNS,add-list(tlist-NNS,pair(VP(VB(_a),NP(NN(_b1),NN(part),PP(IN(of),_c2))),"
			"                                           VP(VB(_a),NP(NN(_b1)),NP(NN(part),PP(IN(of),_c2))) ))),"
			"     set(tlist-NNS,add-list(tlist-NNS,pair(VP(VB(_a),NP(NN(_b1),NN(_c1),PP(IN(_c2),NP(_c3,_c4))),_d),"
			"                                           VP(VB(_a),NP(NN(_b1)),NP(NN(_c1),PP(IN(_c2),NP(_c3,_c4))),_d) ))),"
			"     set(tlist-SBAR,list),"
			//					"     set(tlist-SBAR,add-list(tlist-SBAR,pair(NP(_A1,_A2,SBAR(CC(_A3),S(NP(_A4),_A5))),"
			//					"                                             SBAR(NP(_A1,_A2,CC(_A3),_A4),_A5) ) )),"
			"     for-each(item,tlist,join(print(first-child(item)),print(last-child(item)))),"
			"     set(is-question,find-str-in-tree(etree,?)),"
			"     set(new-tree,etree),"
			"     for-each-tree(_a,etree,"
			"                   join(set(ff,first-foot(_a)),"
			"                        set(ff-name,sub-string(ff,0,find-str-pos(ff,/))),"
			"                        if(and(not(is-question),find-str(_a,VP),find-str(ff,/VB),not(find-str-in-tree(indirect-verb-list,ff-name))),"
			"                           for-each(item,tlist,"
			"                                    join(set(new-tree-tmp,new-tree),"
			"                                         set(new-tree-tmp,match-and-substitute(_a,new-tree-tmp,first-child(item),last-child(item))),"
			"                                         print(new-tree-tmp),"
			"                                         if(not(equal(new-tree-tmp,false)),join(set(new-tree,new-tree-tmp),break))"
			"                                        )"
			"                                   )"
			"                          ),"
			"                        if(and(find-str(_a,VP),find-str(ff,/VB)),"
			"                           for-each(item,tlist-NNS,"
			"                                    join(set(new-tree-tmp,new-tree),"
			"                                         set(new-tree-tmp,match-and-substitute(_a,new-tree-tmp,first-child(item),last-child(item))),"
			"                                         print(new-tree-tmp),"
			"                                         if(not(equal(new-tree-tmp,false)),join(set(new-tree,new-tree-tmp),break))"
			"                                        )"
			"                                   )"
			"                          ),"
			"                        if(find-str-in-tree(_a,SBAR),"
			"                           for-each(item,tlist-SBAR,"
			"                                    join(set(new-tree-tmp,new-tree),"
			"                                         set(new-tree-tmp,match-and-substitute(_a,new-tree-tmp,first-child(item),last-child(item))),"
			"                                         print(new-tree-tmp),"
			"                                         if(not(equal(new-tree-tmp,false)),join(set(new-tree,new-tree-tmp),break))"
			"                                        )"
			"                                   )"
			"                          )"
			"                       )"
			"                  ),"
			"     print(here-is-the-new-tree),"
			"     print(new-tree)"
			"    )");

	get_candidate_verbs.insert("_A", drtvect);
	get_candidate_verbs.insert("_B", taglist);
	get_candidate_verbs.insert("_C", enhanced_tree);

	Knowledge k;
	Engine engine(&k);
	clock_t start;


	CodePred result = engine.run(get_candidate_verbs);


	FuzzyPred fp;
	fp = engine.getElement<FuzzyPred>("new-tree");
	if (!(fp == FuzzyPred())) {
		PredTree fpt = fp.pred();
		PredTree parsed_pt = parsed_tree.pred();
		PredTree::height_iterator hi1(fpt, 1);
		PredTree::height_iterator hi2(parsed_pt, 0);
		for (; hi1 != fpt.end() && hi2 != parsed_pt.end(); ++hi1, ++hi2) {
			// hi1= fpt.replace(PredTree(hi2), hi1);
			// 		  string tmp_str= hi1->str;
			// 		  tmp_str = tmp_str.substr(0,tmp_str.find('/') );
			// 		  hi1->str= tmp_str;
			hi1->str = hi2->str;
			fpt.cut(hi1);
		}
		fp.pred() = fpt;
		to_return = fp;
	}


	return to_return;
}

void Triggers::process(const DrtVect &drtvect, const vector<FuzzyPred> &taglist, const FuzzyPred &parsed_tree,
		const DrtPred &error)
{
	taglist_ = get_new_tags(drtvect, taglist, parsed_tree, error);
	noprn_ = deny_PRN_positions(drtvect, taglist, parsed_tree);
	nosbar_ = deny_SBAR_positions(drtvect, taglist, parsed_tree);
	new_parsed_tree_ = correct_parsed_tree(drtvect, taglist, parsed_tree);
}

bool Triggers::hasUnprocessed()
{
	if (taglist_.size())
		return true;
	if (noprn_.size())
		return true;
	if (nosbar_.size())
		return true;
	if (!(new_parsed_tree_ == FuzzyPred()))
		return true;

	return false;
}

vector<vector<FuzzyPred> > Triggers::getCorrectedTags()
{
	return taglist_;
}

vector<int> Triggers::getForbiddenPRN()
{
	return noprn_;
}

vector<int> Triggers::getForbiddenSBAR()
{
	return nosbar_;
}

FuzzyPred Triggers::getParsedTree()
{
	return new_parsed_tree_;
}
