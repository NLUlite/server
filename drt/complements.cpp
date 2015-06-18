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



#include"complements.hpp"

vector<string> get_all_complement_strings()
{
     vector<string> all_complements;
     all_complements.push_back("@MOTION_TO");
     all_complements.push_back("@MOTION_FROM");
     all_complements.push_back("@PLACE_AT");
     all_complements.push_back("@TIME_TO");
     all_complements.push_back("@TIME_FROM");
     all_complements.push_back("@TIME_AT");
     all_complements.push_back("@TIME_DURATION");
     all_complements.push_back("@DATIVE");
     all_complements.push_back("@CAUSED_BY");
     all_complements.push_back("@FOR");
     all_complements.push_back("@GENITIVE");
     all_complements.push_back("@TOPIC");
     all_complements.push_back("@AND");
     all_complements.push_back("@OR");
     all_complements.push_back("@WITH");
     all_complements.push_back("@EXCLUDING");
     all_complements.push_back("@AFTER");
     all_complements.push_back("@BEFORE");
     all_complements.push_back("@MOTION_THROUGH");
     all_complements.push_back("@MOTION_AGAINST");
     all_complements.push_back("@TIME_THROUGH");
     all_complements.push_back("@QUANTITY");
     all_complements.push_back("@QUANTIFIER");
     all_complements.push_back("@ALLOCUTION");
     all_complements.push_back("@COMPARED");
     all_complements.push_back("@COMPARED_TO");
     all_complements.push_back("@OWNED_BY");
     all_complements.push_back("@CLOCK_AT");
     all_complements.push_back("@OWN");
     all_complements.push_back("@AGE");
     all_complements.push_back("@MORE");
     all_complements.push_back("@LESS");
     all_complements.push_back("@TIMES");
     all_complements.push_back("@MORE_THAN"); // this is like a @QUANTITY complement
     all_complements.push_back("@LESS_THAN"); // this is like a @QUANTITY complement
     all_complements.push_back("@SIZE"); // created only from presuppositions
     all_complements.push_back("@ACCORDING_TO");
     // all_complements.push_back("@TIME");

     return all_complements;
}

map<string,string> get_all_complement_conversion()
{
     map<string,string> all_complements;
     all_complements["@MOTION_TO"]= "to";
     all_complements["@MOTION_FROM"]= "from";
     all_complements["@PLACE_AT"] = "in";
     all_complements["@TIME_TO"] = "to";
     all_complements["@TIME_FROM"]= "from";
     all_complements["@TIME_AT"] = "at";
     all_complements["@DATIVE"] = "to";
     all_complements["@CAUSED_BY"]= "by";
     all_complements["@FOR"]= "for";
     all_complements["@GENITIVE"]= "of";
     all_complements["@TOPIC"]= "about";
     all_complements["@WITH"]= "with";
     //all_complements["@EXCLUDING"]= "though";
     //all_complements["@EXCLUDING"]= "without";
     all_complements["@EXCLUDING"]= "even_though";
     all_complements["@AFTER"]= "after";
     all_complements["@BEFORE"]= "before";
     all_complements["@MOTION_THROUGH"]= "across";
     all_complements["@MOTION_AGAINST"]= "against";
     all_complements["@TIME_THROUGH"]= "across";
     all_complements["@CLOCK_AT"]= "at";
     all_complements["@OWNED_BY"]= "of";
     all_complements["@QUANTITY"]= "[of-quantity]";
     all_complements["@QUANTIFIER"]= "[of-quantity]";
     all_complements["@ALLOCUTION"]= "[speech]";
     all_complements["@SIZE"]= "[of-size]";
     all_complements["@TIMES"]= "[of-times]";

     //all_complements["@COMPARED_TO"]= "than";
     all_complements["@COMPARED_TO"]= "compared_to";
     all_complements["@MORE"]= "more";
     all_complements["@LESS"]= "less";

     all_complements["@MORE_THAN"]= "more_than";
     all_complements["@LESS_THAN"]= "less_than";

     all_complements["@TIME_BETWEEN"]= "between";
     all_complements["@TIME_DURATION"]= "for";


     all_complements["@AND"]= "and";
     all_complements["@OR"]= "or";

     //all_complements["@SUB-OBJ"]= "to";
     //all_complements["@SUBORD"]= "to";

     all_complements["@ACCORDING_TO"]= "according_to";

     all_complements["@AGE"]= "of";

     return all_complements;
}
