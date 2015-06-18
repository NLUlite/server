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



#ifndef __COMPLEMENTS__
#define __COMPLEMENTS__


#include<string>
#include<iostream>

using std::string;


class complements_type {
     string type_;
public:
     string get_type() {return type_;}

     // space-like complements
     void set_motion_to() {type_="MOTION_TO";}
     bool is_motion_to() {return type_ == "MOTION_TO";}
     void set_motion_from() {type_="MOTION_FROM";}
     bool is_motion_from() {return type_ == "MOTION_FROM";}
     void set_place_at() {type_="PLACE_AT";}
     bool is_place_at() {return type_ == "PLACE_AT";}
     bool is_place() {return ( is_motion_to() | is_motion_from() | is_place_at() );}

     // time-like complements
     void set_time_to() {type_="TIME_TO";}
     bool is_time_to() {return type_ == "TIME_TO";}
     void set_time_from() {type_="TIME_FROM";}
     bool is_time_from() {return type_ == "TIME_FROM";}
     void set_time_at() {type_="TIME_AT";}
     bool is_time_at() {return type_ == "TIME_AT";}
     bool is_time() {return ( is_time_to() | is_time_from() | is_time_at() );}

     // end complement /// (italian "complemento di termine", english "indirect object" )
     void set_end() {type_="DATIVE";}
     bool is_end() {return type_ == "DATIVE";}

     // end complement for subordinates (like in "he likes *to walk* ")
     void set_subord_end() {type_="SUBORD";}
     bool is_subord_end() {return type_ == "SUBORD";}


     // cause complement
     void set_cause() {type_="CAUSED_BY";}
     bool is_cause() {return type_ == "CAUSED_BY";}

     // genitive complement
     void set_genitive() {type_="GENITIVE";}
     bool is_genitive() {return type_ == "GENITIVE";}

     // saxon genitive complement
     void set_sax_genitive() {type_="SAX_GENITIVE";}
     bool is_sax_genitive() {return type_ == "SAX_GENITIVE";}

     // complarative complement
     void set_comparative() {type_="COMPARED_TO";}
     bool is_comparative() {return type_ == "COMPARED_TO";}
};




#endif // __COMPLEMENTS__
