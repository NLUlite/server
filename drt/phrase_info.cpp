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



#include"phrase_info.hpp"

void PhraseInfo::computeCCG()
{
     // The first term is the parent, the second is the candidate head under the head
     head_tags_.push_back( make_tuple("ADJP","SO", left) );
     head_tags_.push_back( make_tuple("ADJP","ADJP", left) );
     head_tags_.push_back( make_tuple("ADJP","$", left) );
     head_tags_.push_back( make_tuple("ADJP","VP", right) );
     head_tags_.push_back( make_tuple("ADJP","VBN", right) );
     head_tags_.push_back( make_tuple("ADJP","VBG", right) );
     head_tags_.push_back( make_tuple("ADJP","JJ", right) );
     head_tags_.push_back( make_tuple("ADJP","JJR", right) );
     head_tags_.push_back( make_tuple("ADJP","JJS", right) );
     head_tags_.push_back( make_tuple("ADJP","VBJ", right) ); // my addition
     head_tags_.push_back( make_tuple("ADJP","NN", right) );
     head_tags_.push_back( make_tuple("ADJP","NNS", right) );
     head_tags_.push_back( make_tuple("ADJP","NNP", right) );
     head_tags_.push_back( make_tuple("ADJP","QP", left) );
     head_tags_.push_back( make_tuple("ADJP","DT", left) );
     head_tags_.push_back( make_tuple("ADJP","POSSDT", left) );
     head_tags_.push_back( make_tuple("ADJP","CD", right) );
     head_tags_.push_back( make_tuple("ADJP","RB", right) );
     head_tags_.push_back( make_tuple("ADJP","ADVP", right) );
     head_tags_.push_back( make_tuple("ADJP","RBR", right) );
     head_tags_.push_back( make_tuple("ADJP","IN", left) );
     head_tags_.push_back( make_tuple("ADJP","VBD", left) );
     head_tags_.push_back( make_tuple("ADJP","VBP", left) );
     head_tags_.push_back( make_tuple("ADJP","VB", left) );
     head_tags_.push_back( make_tuple("ADJP","NP", left) );
     head_tags_.push_back( make_tuple("ADJP","WHNP", left) );
     head_tags_.push_back( make_tuple("ADJP","FW", left) );
     head_tags_.push_back( make_tuple("ADJP","RBS", left) );
     head_tags_.push_back( make_tuple("ADJP","SBAR", left) );
     head_tags_.push_back( make_tuple("ADJP","PDT", left) );

     head_tags_.push_back( make_tuple("ADVP","RBR", left) );
     head_tags_.push_back( make_tuple("ADVP","JJ", left) );
     head_tags_.push_back( make_tuple("ADVP","JJR", left) );
     //head_tags_.push_back( make_tuple("ADVP","VBJ", left) ); // my addition
     head_tags_.push_back( make_tuple("ADVP","RB", right) );
     head_tags_.push_back( make_tuple("ADVP","ADVP", right) );
     head_tags_.push_back( make_tuple("ADVP","RBR", right) );
     head_tags_.push_back( make_tuple("ADVP","RBS", right) );
     head_tags_.push_back( make_tuple("ADVP","IN", right) );
     head_tags_.push_back( make_tuple("ADVP","JJR", right) );
     head_tags_.push_back( make_tuple("ADVP","JJS", right) );
     head_tags_.push_back( make_tuple("ADVP","FW", right) );
     head_tags_.push_back( make_tuple("ADVP","TO", right) );
     head_tags_.push_back( make_tuple("ADVP","CD", right) );
     head_tags_.push_back( make_tuple("ADVP","JJR", right) );
     head_tags_.push_back( make_tuple("ADVP","JJ", right) );
     //head_tags_.push_back( make_tuple("ADVP","VBJ", right) ); // my addition
     head_tags_.push_back( make_tuple("ADVP","IN", right) );
     head_tags_.push_back( make_tuple("ADVP","WHADJP", right) );     
     head_tags_.push_back( make_tuple("ADVP","NP", right) );
     head_tags_.push_back( make_tuple("ADVP","WHNP", right) );
     head_tags_.push_back( make_tuple("ADVP","JJS", right) );
     head_tags_.push_back( make_tuple("ADVP","NN", right) );
     head_tags_.push_back( make_tuple("ADVP","RP", right) );

     head_tags_.push_back( make_tuple("S","MD", left) );
     head_tags_.push_back( make_tuple("S","VBP", left) );
     head_tags_.push_back( make_tuple("S","VBD", left) );
     head_tags_.push_back( make_tuple("S","VBZ", left) );
     head_tags_.push_back( make_tuple("S","TO", left) );
     //head_tags_.push_back( make_tuple("S","RB", left) ); // my addition
     head_tags_.push_back( make_tuple("S","VP", left) );
     //head_tags_.push_back( make_tuple("S","ADJP", left) );
     head_tags_.push_back( make_tuple("S","S", left) );
     head_tags_.push_back( make_tuple("S","SBAR", left) );
     head_tags_.push_back( make_tuple("S","SINV", left) );
     head_tags_.push_back( make_tuple("S","UCP", left) );
     head_tags_.push_back( make_tuple("S","INTJ", left) );
     head_tags_.push_back( make_tuple("S","NP", left) );
     head_tags_.push_back( make_tuple("S","WHNP", left) );
     head_tags_.push_back( make_tuple("S","ADJP", left) ); // My change: it was after VP
     head_tags_.push_back( make_tuple("S","PP", left) ); /// My addition (still some unresolved distinction between S and SBAR)
     head_tags_.push_back( make_tuple("S","WHPP", left) ); /// My addition (still some unresolved distinction between S and SBAR)     

     head_tags_.push_back( make_tuple("ROOT","ROOT", left) ); // my addition (a root can be under a root after the binarization)
     head_tags_.push_back( make_tuple("ROOT","NP", left) ); /// This was the last element
     head_tags_.push_back( make_tuple("ROOT","WHNP", left) ); /// my addition
     head_tags_.push_back( make_tuple("ROOT","MD", left) );
     head_tags_.push_back( make_tuple("ROOT","VBP", left) );
     head_tags_.push_back( make_tuple("ROOT","VBD", left) );
     head_tags_.push_back( make_tuple("ROOT","VBZ", left) );
     head_tags_.push_back( make_tuple("ROOT","TO", left) );
     head_tags_.push_back( make_tuple("ROOT","VP", left) );
     head_tags_.push_back( make_tuple("ROOT","ADJP", left) );
     head_tags_.push_back( make_tuple("ROOT","S", left) );
     head_tags_.push_back( make_tuple("ROOT","SQ", left) );    // my addition
     head_tags_.push_back( make_tuple("ROOT","SBARQ", left) ); // my addition
     head_tags_.push_back( make_tuple("ROOT","SBAR", left) );
     head_tags_.push_back( make_tuple("ROOT","SINV", left) );
     head_tags_.push_back( make_tuple("ROOT","UCP", left) );
     head_tags_.push_back( make_tuple("ROOT","INTJ", left) );
     head_tags_.push_back( make_tuple("ROOT","X", left) ); // my addition
     head_tags_.push_back( make_tuple("ROOT","PP", left) ); /// My addition (still some unresolved distinction between S and SBAR and ROOT)
     head_tags_.push_back( make_tuple("ROOT","WHPP", left) ); /// My addition (still some unresolved distinction between S and SBAR and ROOT)
     

     //head_tags_.push_back( make_tuple("SINV","ADVP(so)", right) );
     head_tags_.push_back( make_tuple("SINV","ADVP", right) );
     head_tags_.push_back( make_tuple("SINV","VBZ", left) );
     head_tags_.push_back( make_tuple("SINV","VBD", left) );
     head_tags_.push_back( make_tuple("SINV","VBP", left) );
     head_tags_.push_back( make_tuple("SINV","VB", left) );
     head_tags_.push_back( make_tuple("SINV","MD", left) );
     head_tags_.push_back( make_tuple("SINV","VP", left) );
     head_tags_.push_back( make_tuple("SINV","S", left) );
     head_tags_.push_back( make_tuple("SINV","SINV", left) );
     head_tags_.push_back( make_tuple("SINV","ADJP", left) );
     head_tags_.push_back( make_tuple("SINV","NP", left) );
     head_tags_.push_back( make_tuple("SINV","WHNP", left) );

     head_tags_.push_back( make_tuple("SQ","VBZ", left) );
     head_tags_.push_back( make_tuple("SQ","VBD", left) );
     head_tags_.push_back( make_tuple("SQ","VBP", left) );
     head_tags_.push_back( make_tuple("SQ","VB", left) );
     head_tags_.push_back( make_tuple("SQ","MD", left) );
     head_tags_.push_back( make_tuple("SQ","VP", left) );
     head_tags_.push_back( make_tuple("SQ","SQ", left) );
     head_tags_.push_back( make_tuple("SQ","SBARQ", left) );

     head_tags_.push_back( make_tuple("SBAR","S", right) );
     head_tags_.push_back( make_tuple("SBAR","IN", left) );
     head_tags_.push_back( make_tuple("SBAR","WHNP", left) );
     head_tags_.push_back( make_tuple("SBAR","WHPP", left) );
     head_tags_.push_back( make_tuple("SBAR","WHADVP", left) );
     head_tags_.push_back( make_tuple("SBAR","WHADJP", left) );
     head_tags_.push_back( make_tuple("SBAR","WDT", left) );
     head_tags_.push_back( make_tuple("SBAR","RB", left) );
     head_tags_.push_back( make_tuple("SBAR","MD", left) );
     head_tags_.push_back( make_tuple("SBAR","DT", left) );
     head_tags_.push_back( make_tuple("SBAR","POSSDT", left) );
     head_tags_.push_back( make_tuple("SBAR","X", left) );
     head_tags_.push_back( make_tuple("SBAR","PP", left) );
     head_tags_.push_back( make_tuple("SBAR","WHPP", left) );
     head_tags_.push_back( make_tuple("SBAR","PRN", left) );
     head_tags_.push_back( make_tuple("SBAR","UCP", left) );
     head_tags_.push_back( make_tuple("SBAR","VBD", left) );
     head_tags_.push_back( make_tuple("SBAR","VB", left) );
     head_tags_.push_back( make_tuple("SBAR","S", left) );
     head_tags_.push_back( make_tuple("SBAR","SQ", left) );
     head_tags_.push_back( make_tuple("SBAR","NP", left) );
     head_tags_.push_back( make_tuple("SBAR","WHNP", left) );
     head_tags_.push_back( make_tuple("SBAR","SINV", left) );
     head_tags_.push_back( make_tuple("SBAR","SBAR", left) );
     head_tags_.push_back( make_tuple("SBAR","FRAG", left) );

     head_tags_.push_back( make_tuple("SBARQ","WHNP", left) );
     head_tags_.push_back( make_tuple("SBARQ","WHADVP", left) );
     head_tags_.push_back( make_tuple("SBARQ","WHADJP", left) );
     head_tags_.push_back( make_tuple("SBARQ","WHPP", left) );
     head_tags_.push_back( make_tuple("SBARQ","WP", left) );
     head_tags_.push_back( make_tuple("SBARQ","SBARQ", left) );

     head_tags_.push_back( make_tuple("PP","PRN", left) );
     head_tags_.push_back( make_tuple("PP","IN", right) );
     head_tags_.push_back( make_tuple("PP","TO", right) );
     head_tags_.push_back( make_tuple("PP","VBG", right) );
     head_tags_.push_back( make_tuple("PP","VBN", right) );
     head_tags_.push_back( make_tuple("PP","VB", right) );
     head_tags_.push_back( make_tuple("PP","VP", right) ); // My addition
     head_tags_.push_back( make_tuple("PP","RP", right) );
     head_tags_.push_back( make_tuple("PP","PP", right) );
     head_tags_.push_back( make_tuple("PP","WHPP", right) ); // My addition
     head_tags_.push_back( make_tuple("PP","FW", right) );
     head_tags_.push_back( make_tuple("PP","*", right) );

     head_tags_.push_back( make_tuple("VP","TO", left) );
     head_tags_.push_back( make_tuple("VP","VBD", left) );
     head_tags_.push_back( make_tuple("VP","VBN", left) );
     head_tags_.push_back( make_tuple("VP","MD", left) );
     head_tags_.push_back( make_tuple("VP","VBZ", left) );
     head_tags_.push_back( make_tuple("VP","VB", left) );
     head_tags_.push_back( make_tuple("VP","VBG", left) );
     head_tags_.push_back( make_tuple("VP","VBP", left) );
     head_tags_.push_back( make_tuple("VP","V", left) );
     head_tags_.push_back( make_tuple("VP","AUX", left) );
     head_tags_.push_back( make_tuple("VP","VP", left) );  // original in CCG
     //head_tags_.push_back( make_tuple("VP","VP", right) ); // works only for questions
     head_tags_.push_back( make_tuple("VP","JJ", left) );
     head_tags_.push_back( make_tuple("VP","VBJ", left) ); // my addition
     head_tags_.push_back( make_tuple("VP","S", left) );
     head_tags_.push_back( make_tuple("VP","SYM", left) );
     head_tags_.push_back( make_tuple("VP","NN", left) );
     head_tags_.push_back( make_tuple("VP","NNS", left) );
     head_tags_.push_back( make_tuple("VP","NP", left) );
     head_tags_.push_back( make_tuple("VP","WHNP", left) );

     head_tags_.push_back( make_tuple("QP","QP", left) );
     head_tags_.push_back( make_tuple("QP","ASTAG", left) );
     head_tags_.push_back( make_tuple("QP","NP", left) );
     head_tags_.push_back( make_tuple("QP","WHNP", left) );

     head_tags_.push_back( make_tuple("QP","IN", left) );

     head_tags_.push_back( make_tuple("QP","N", left) );
     head_tags_.push_back( make_tuple("QP","NN", left) );
     head_tags_.push_back( make_tuple("QP","NNS", left) );
     head_tags_.push_back( make_tuple("QP","NNP", left) );
     head_tags_.push_back( make_tuple("QP","NNPS", left) );
     head_tags_.push_back( make_tuple("QP","NNS", left) );
     head_tags_.push_back( make_tuple("QP","RBR", left) );
     head_tags_.push_back( make_tuple("QP","UP", left) );
     head_tags_.push_back( make_tuple("QP","NP", left) );
     head_tags_.push_back( make_tuple("QP","WHNP", left) );
     head_tags_.push_back( make_tuple("QP","$", left) );
     head_tags_.push_back( make_tuple("QP","SYM", left) );
     head_tags_.push_back( make_tuple("QP","CD", left) );

     head_tags_.push_back( make_tuple("WHNP","WDT", left) );
     head_tags_.push_back( make_tuple("WHNP","WP", left) );
     head_tags_.push_back( make_tuple("WHNP","WPS", left) );
     head_tags_.push_back( make_tuple("WHNP","WHADJP", left) );
     head_tags_.push_back( make_tuple("WHNP","WHNP", left) );
     head_tags_.push_back( make_tuple("WHNP","WHNP", left) );
     head_tags_.push_back( make_tuple("WHNP","IN", left) );
     head_tags_.push_back( make_tuple("WHNP","WBR", left) );
     head_tags_.push_back( make_tuple("WHNP","S", left) );
     head_tags_.push_back( make_tuple("WHNP","NN", right) );
     head_tags_.push_back( make_tuple("WHNP","NNS", right) );
     head_tags_.push_back( make_tuple("WHNP","NNP", right) );
     head_tags_.push_back( make_tuple("WHNP","NNPS", right) );

     head_tags_.push_back( make_tuple("WHADJP","WRB", left) );
     head_tags_.push_back( make_tuple("WHADJP","WHADVP", left) );
     head_tags_.push_back( make_tuple("WHADJP","WP", left) );
     head_tags_.push_back( make_tuple("WHADJP","RB", left) );

     head_tags_.push_back(make_tuple("WHADVP", "VP", left));
     head_tags_.push_back(make_tuple("WHADVP", "WRB", right));
     
     head_tags_.push_back( make_tuple("WHADJP","IN", right) );

     head_tags_.push_back( make_tuple("WHPP","PRN", left) );
     head_tags_.push_back( make_tuple("WHPP","IN", right) );
     head_tags_.push_back( make_tuple("WHPP","TO", right) );
     head_tags_.push_back( make_tuple("WHPP","VBG", right) );
     head_tags_.push_back( make_tuple("WHPP","VBN", right) );
     head_tags_.push_back( make_tuple("WHPP","VB", right) );
     head_tags_.push_back( make_tuple("WHPP","VP", right) ); // My addition
     head_tags_.push_back( make_tuple("WHPP","RP", right) );
     head_tags_.push_back( make_tuple("WHPP","PP", right) );
     head_tags_.push_back( make_tuple("WHPP","WHPP", right) );
     head_tags_.push_back( make_tuple("WHPP","FW", right) );

     head_tags_.push_back( make_tuple("CONJP","CC", right) );
     head_tags_.push_back( make_tuple("CONJP","JJ", right) );
     head_tags_.push_back( make_tuple("CONJP","VBJ", right) ); // my addition
     head_tags_.push_back( make_tuple("CONJP","IN", right) );
     head_tags_.push_back( make_tuple("CONJP","RB", right) );
     head_tags_.push_back( make_tuple("CONJP","CONJP", right) );

     head_tags_.push_back( make_tuple("FRAG","CC", right) );
     head_tags_.push_back( make_tuple("FRAG","CONJP", right) );
     head_tags_.push_back( make_tuple("FRAG","WP", left) );
     head_tags_.push_back( make_tuple("FRAG","WHADVP", left) );
     head_tags_.push_back( make_tuple("FRAG","RB", left) );
     head_tags_.push_back( make_tuple("WHADJP","SBAR", left) );
     head_tags_.push_back( make_tuple("WHADJP","IN", left) );
     head_tags_.push_back( make_tuple("WHADJP","S", left) );
     head_tags_.push_back( make_tuple("WHADJP","NP", left) );
     head_tags_.push_back( make_tuple("WHADJP","WHNP", left) );
     head_tags_.push_back( make_tuple("WHADJP","ADVP", left) );
     head_tags_.push_back( make_tuple("WHADJP","PP", left) );
     head_tags_.push_back( make_tuple("WHADJP","WHPP", left) );

     head_tags_.push_back( make_tuple("PRN",":", right) );
     head_tags_.push_back( make_tuple("PRN","LRB", right) );
     head_tags_.push_back( make_tuple("PRN","PRN", right) );
     head_tags_.push_back( make_tuple("PRN","PRNS", left) );
     head_tags_.push_back( make_tuple("PRN","S", left) );
     head_tags_.push_back( make_tuple("PRN","NP", left) );
     head_tags_.push_back( make_tuple("PRN","WHNP", left) );
     head_tags_.push_back( make_tuple("PRN","VP", left) );
     head_tags_.push_back( make_tuple("PRN","PP", left) );
     head_tags_.push_back( make_tuple("PRN","WHPP", left) );
     head_tags_.push_back( make_tuple("PRN","SBAR", left) );
     //head_tags_.push_back( make_tuple("PRN","SINV", left) ); // my addition
     head_tags_.push_back( make_tuple("PRN","UCP", left) );
     head_tags_.push_back( make_tuple("PRN","ADJP", left) );
     head_tags_.push_back( make_tuple("PRN","ADJP", left) );
     head_tags_.push_back( make_tuple("PRN","ADVP", left) );
     head_tags_.push_back( make_tuple("PRN","RB", left) );

     head_tags_.push_back( make_tuple("INTJ","*", right) );

     head_tags_.push_back( make_tuple("X","DT", left) );
     head_tags_.push_back( make_tuple("X","POSSDT", left) );

     head_tags_.push_back( make_tuple("RRC","VP", right) );
     head_tags_.push_back( make_tuple("RRC","NP", right) );
     head_tags_.push_back( make_tuple("RRC","WHNP", right) );
     head_tags_.push_back( make_tuple("RRC","ADVP", right) );
     head_tags_.push_back( make_tuple("RRC","ADJP", right) );
     head_tags_.push_back( make_tuple("RRC","PP", right) );
     head_tags_.push_back( make_tuple("RRC","WHPP", right) );

     head_tags_.push_back( make_tuple("PRT","RP", right) );

     head_tags_.push_back( make_tuple("LST","LS", right) );
     head_tags_.push_back( make_tuple("LST","-colon-", right) );
     
     head_tags_.push_back( make_tuple("NP","CC", right) ); // my addition
     head_tags_.push_back( make_tuple("NP","VBP", right) ); // my addition
     head_tags_.push_back( make_tuple("NP","VBZ", right) ); // my addition
     head_tags_.push_back( make_tuple("NP","VBD", right) ); // my addition
     head_tags_.push_back( make_tuple("NP","VBN", right) ); // my addition
     head_tags_.push_back( make_tuple("NP","VBG", left) ); // my addition
     head_tags_.push_back( make_tuple("NP","AUX", right) ); // my addition
     head_tags_.push_back( make_tuple("NP","VP", right) ); // my addition
     head_tags_.push_back( make_tuple("NP","SINV", right) ); // my addition
     head_tags_.push_back( make_tuple("NP","PRN", right) ); // my addition     
     head_tags_.push_back( make_tuple("NP","NP", left) ); // my addition
     head_tags_.push_back( make_tuple("NP","WHNP", left) ); // my addition
     head_tags_.push_back( make_tuple("NP","NN", right) ); // my addition
     head_tags_.push_back( make_tuple("NP","NNS", right) ); // my addition
     head_tags_.push_back( make_tuple("NP","NNP", left) ); // my addition
     head_tags_.push_back( make_tuple("NP","NNPS", left) ); // my addition
     head_tags_.push_back( make_tuple("NP","QP", left) ); // my addition
     head_tags_.push_back( make_tuple("NP","S", left) ); // my addition
     head_tags_.push_back( make_tuple("NP","SBAR", left) ); // my addition

     head_tags_.push_back( make_tuple("POSSDT","POS", right) ); // my addition

     head_tags_.push_back( make_tuple("X","S", left) ); /// My addition

     // The first term is the parent, the second is the candidate complement under the head
     compl_tags_.push_back( make_pair("ADJP","PP") );
     compl_tags_.push_back( make_pair("ADJP","WHPP") );
     compl_tags_.push_back( make_pair("ADJP","S") );
     compl_tags_.push_back( make_pair("ADJP","SBAR") );
     compl_tags_.push_back( make_pair("ADJP","NP") );
     compl_tags_.push_back( make_pair("ADJP","WHNP") );

     compl_tags_.push_back( make_pair("ADJP","PP") );
     compl_tags_.push_back( make_pair("ADJP","WHPP") );
     compl_tags_.push_back( make_pair("ADVP","SBAR") );
     compl_tags_.push_back( make_pair("ADVP","NP") );
     compl_tags_.push_back( make_pair("ADVP","WHNP") );
     compl_tags_.push_back( make_pair("ADVP","SOADJ") );

     compl_tags_.push_back( make_pair("NP","NOUN") );
     compl_tags_.push_back( make_pair("NP","NN") );
     compl_tags_.push_back( make_pair("NP","NNS") );
     compl_tags_.push_back( make_pair("NP","NNP") );
     compl_tags_.push_back( make_pair("NP","NNPS") );
     compl_tags_.push_back( make_pair("NP","CD") ); // my addition
     compl_tags_.push_back( make_pair("NP","JJ") ); // my addition
     compl_tags_.push_back( make_pair("NP","JJR") ); // my addition
     compl_tags_.push_back( make_pair("NP","JJS") ); // my addition
     compl_tags_.push_back( make_pair("NP","VBJ") ); // my addition
     compl_tags_.push_back( make_pair("NP","PRP") ); // my addition
     //compl_tags_.push_back( make_pair("NP","DT") ); // my addition (not correct!! if DT is before NP, assign ":c" )
     compl_tags_.push_back( make_pair("NP","NP") );
     compl_tags_.push_back( make_pair("NP","WHNP") );
     compl_tags_.push_back( make_pair("NP","EX") ); // my addition
     compl_tags_.push_back( make_pair("NX","NOUN") );
     compl_tags_.push_back( make_pair("NAC","NOUN") );
     compl_tags_.push_back( make_pair("NP","SBAR") );
     compl_tags_.push_back( make_pair("NP","PRN") ); // my addition
     compl_tags_.push_back( make_pair("NP","SINV") ); // my addition

     compl_tags_.push_back( make_pair("S","S") );
     compl_tags_.push_back( make_pair("S","SINV") );  // my addition
     compl_tags_.push_back( make_pair("S","NP") );
     compl_tags_.push_back( make_pair("S","WHNP") ); //// my addition  (it is because not all the questions are SQ) Change it!!!
     compl_tags_.push_back( make_pair("S","VP") );
     compl_tags_.push_back( make_pair("S","NN") ); // in some cases the complement can be just a bare noun
     compl_tags_.push_back( make_pair("S","DT") ); // in some cases the complement can be just a bare DT (this)
     compl_tags_.push_back( make_pair("S","NNS") ); 
     compl_tags_.push_back( make_pair("S","NNP") ); 
     compl_tags_.push_back( make_pair("S","NNPS") ); 
     compl_tags_.push_back( make_pair("S","PRP") ); // in some cases the complement can be just a bare pronoun
     compl_tags_.push_back( make_pair("S","PRN") );
     compl_tags_.push_back( make_pair("S","EX") ); // my addition
     compl_tags_.push_back( make_pair("S","ADJP") ); // my addition

     compl_tags_.push_back( make_pair("ROOT","S") );
     compl_tags_.push_back( make_pair("ROOT","SINV") );
     compl_tags_.push_back( make_pair("ROOT","SBAR") );
     compl_tags_.push_back( make_pair("ROOT","NP") );
     compl_tags_.push_back( make_pair("ROOT","WHNP") );
     compl_tags_.push_back( make_pair("ROOT","VP") );
     compl_tags_.push_back( make_pair("ROOT","PRN") ); 

     compl_tags_.push_back( make_pair("SINV","NP") );
     compl_tags_.push_back( make_pair("SINV","WHNP") );
     compl_tags_.push_back( make_pair("SINV","S") );
     compl_tags_.push_back( make_pair("SINV","SBARQ") );
     compl_tags_.push_back( make_pair("SINV","VP") );

     compl_tags_.push_back( make_pair("SQ","VP") );
     compl_tags_.push_back( make_pair("SQ","NP") );
     compl_tags_.push_back( make_pair("SQ","WHNP") );
     compl_tags_.push_back( make_pair("SQ","WP") );
     compl_tags_.push_back( make_pair("SQ","S") );

     compl_tags_.push_back( make_pair("SBAR","NN") );
     compl_tags_.push_back( make_pair("SBAR","WHNP") );
     compl_tags_.push_back( make_pair("SBAR","NP") );  // My addition
     //compl_tags_.push_back( make_pair("SBAR","WDT") );  // My addition
     compl_tags_.push_back( make_pair("SBAR","S") );
     compl_tags_.push_back( make_pair("SBAR","SBAR") ); // My addition
     compl_tags_.push_back( make_pair("SBAR","SQ") );
     compl_tags_.push_back( make_pair("SBAR","VP") );
     compl_tags_.push_back( make_pair("SBAR","SINV") );
     compl_tags_.push_back( make_pair("SBAR","SBARQ") );

     compl_tags_.push_back( make_pair("SBARQ","SQ") );
     compl_tags_.push_back( make_pair("SBARQ","WHNP") );
     compl_tags_.push_back( make_pair("SBARQ","SINV") );
     compl_tags_.push_back( make_pair("SBARQ","S") );
     compl_tags_.push_back( make_pair("SBARQ","SBARQ") );
     compl_tags_.push_back( make_pair("SBARQ","NP") );
     compl_tags_.push_back( make_pair("SBARQ","VP") );

     compl_tags_.push_back( make_pair("PP","NP") );
     compl_tags_.push_back( make_pair("PP","WHNP") );
     compl_tags_.push_back( make_pair("PP","NN") );
     compl_tags_.push_back( make_pair("PP","NNS") );
     compl_tags_.push_back( make_pair("PP","NNP") );
     compl_tags_.push_back( make_pair("PP","NNPS") );
     compl_tags_.push_back( make_pair("PP","S") ); // my addition, for cases like (PP (if...) (S ...)
     compl_tags_.push_back( make_pair("PP","VP") ); // my addition, for cases like (PP (that...) (VP ...)

     compl_tags_.push_back( make_pair("VP","NP") );
     compl_tags_.push_back( make_pair("VP","WHNP") );
     compl_tags_.push_back( make_pair("VP","PRP") );
     compl_tags_.push_back( make_pair("VP","NN") );
     compl_tags_.push_back( make_pair("VP","NNP") );
     compl_tags_.push_back( make_pair("VP","NNS") );
     compl_tags_.push_back( make_pair("VP","NNPS") );
     compl_tags_.push_back( make_pair("VP","VP") );
     compl_tags_.push_back( make_pair("VP","SBARQ") );
     compl_tags_.push_back( make_pair("VP","S") );
     compl_tags_.push_back( make_pair("VP","SINV") );
     compl_tags_.push_back( make_pair("VP","SQ") );
     compl_tags_.push_back( make_pair("VP","SBAR") );
     compl_tags_.push_back( make_pair("VP","ADVP") ); // My addition, for cases like (VP (VBN ADVP))
     compl_tags_.push_back( make_pair("VP","ADJP") ); // My addition
     compl_tags_.push_back( make_pair("VP","UCP") ); // My addition

     compl_tags_.push_back( make_pair("WHNP","NP") );     
     compl_tags_.push_back( make_pair("WHNP","CD") );     
     compl_tags_.push_back( make_pair("WHNP","S") ); // for phrases like "what we need .. are open markets"

     compl_tags_.push_back( make_pair("WHADJP","JJ") );
     compl_tags_.push_back( make_pair("WHADJP","VBJ") ); // my addition
     compl_tags_.push_back( make_pair("WHADJP","ADVP") );
     compl_tags_.push_back( make_pair("WHADJP","ADJP") );

     compl_tags_.push_back( make_pair("WHADVP","NP") ); // my addition
     compl_tags_.push_back( make_pair("WHADVP","WHNP") ); // my addition
     compl_tags_.push_back( make_pair("WHADVP","JJ") );
     compl_tags_.push_back( make_pair("WHADVP","VBJ") ); // my addition
     compl_tags_.push_back( make_pair("WHADVP","WHADVP") ); // my addition
     compl_tags_.push_back( make_pair("WHADVP","ADVP") );     
     compl_tags_.push_back( make_pair("WHADVP","ADJP") );

     compl_tags_.push_back( make_pair("WHPP","NP") );
     compl_tags_.push_back( make_pair("WHPP","WHNP") );
     compl_tags_.push_back( make_pair("WHPP","NN") );
     compl_tags_.push_back( make_pair("WHPP","NNS") );
     compl_tags_.push_back( make_pair("WHPP","NNP") );
     compl_tags_.push_back( make_pair("WHPP","NNPS") );
     compl_tags_.push_back( make_pair("WHPP","S") ); // my addition, for cases like (PP (if...) (S ...)
     compl_tags_.push_back( make_pair("WHPP","VP") ); // my addition

     // compl_tags_.push_back( make_pair("WHPP","IN") ); 
     // compl_tags_.push_back( make_pair("WHPP","TO") );

     compl_tags_.push_back( make_pair("UCP","NN") ); // my addition
     compl_tags_.push_back( make_pair("UCP","NNP") ); // my addition
     compl_tags_.push_back( make_pair("UCP","NNS") ); // my addition
     compl_tags_.push_back( make_pair("UCP","NNPS") ); // my addition
     compl_tags_.push_back( make_pair("UCP","*") );

     compl_tags_.push_back( make_pair("FRAG","NP") );
     compl_tags_.push_back( make_pair("FRAG","WHNP") );
     compl_tags_.push_back( make_pair("FRAG","PP") );
     compl_tags_.push_back( make_pair("FRAG","WHPP") );

     compl_tags_.push_back( make_pair("PRN","RRB") );
     compl_tags_.push_back( make_pair("PRN","-colon-") );
     compl_tags_.push_back( make_pair("PRN","PRN") );
     compl_tags_.push_back( make_pair("PRN","SINV") );
     compl_tags_.push_back( make_pair("PRN","PP") );
     compl_tags_.push_back( make_pair("PRN","WHPP") );

     compl_tags_.push_back( make_pair("QP","NN") ); // my addition (for cases like "be (QP (JJR better) (IN than) that)"
     compl_tags_.push_back( make_pair("QP","NNS") ); // my addition (for cases like "be (QP (JJR better) (IN than) that)"
     compl_tags_.push_back( make_pair("QP","NNP") ); // my addition (for cases like "be (QP (JJR better) (IN than) that)"
     compl_tags_.push_back( make_pair("QP","NNPS") ); // my addition (for cases like "be (QP (JJR better) (IN than) that)"

     compl_tags_.push_back( make_pair("X","ADJP") );
     compl_tags_.push_back( make_pair("X","JJR") );
     //     compl_tags.push_back( make_pair("POSSDT","POS") ); // my addition
}

PhraseInfo::PhraseInfo()
{
     this->computeCCG();
     
     //load the sense     
     Parameters *ps= parameters_singleton::instance();
     string data_dir = ps->getDir();
     sense_= new Sense;
     sense_->loadVerbAgents(data_dir+"/countries.txt");
     sense_->loadVerbAgents(data_dir+"/sense_actors.txt");
     sense_->loadVerbComplements(data_dir+"/sense_complements.txt");
     sense_->loadVerbCombined(data_dir+"/sense_combined.txt");
     sense_->loadNounsAgents(data_dir+"/nouns_actors.txt");
     sense_->loadNounsComplements(data_dir+"/nouns_complements.txt");
     sense_->loadNounsCombined(data_dir+"/nouns_combined.txt");
     sense_->loadNegative(data_dir+"/sense_negative.txt");
     sense_->loadImpossible(data_dir+"/sense_impossible.txt");
     sense_->loadCombinedNegative(data_dir+"/sense_negative_combined.txt");
     loaded_sense_= true;

     
}

PhraseInfo::PhraseInfo(Sense *sense)
{
     this->computeCCG();
     
     // use the sense provided
     sense_= sense;
     loaded_sense_= false;
}


PhraseInfo::~PhraseInfo()
{
     if(loaded_sense_)
	  delete sense_;
}

Sense* PhraseInfo::getSense()
{
     return sense_;
}
