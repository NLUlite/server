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



#include"Filter.hpp"

Filter::Filter(const string &str)
{
     input_= str;

     filter2_["ñ"]= 'n';

     filter2_["à"]= 'a';
     filter2_["è"]= 'e';
     filter2_["ò"]= 'o';

     filter2_["À"]= 'A';
     filter2_["È"]= 'E';
     filter2_["Ò"]= 'O';
    
     filter2_["á"]= 'a';
     filter2_["é"]= 'e';
     filter2_["ó"]= 'o';

     filter2_["Á"]= 'A';
     filter2_["É"]= 'E';
     filter2_["Ó"]= 'O';

     filter2_["å"]= 'a';
     filter2_["ä"]= 'a';
     filter2_["ö"]= 'o';
     filter2_["ë"]= 'e';
     filter2_["ï"]= 'i';

     filter2_["Ï"]= 'I';
     filter2_["Å"]= 'A';
     filter2_["Ä"]= 'A';
     filter2_["Ö"]= 'O';
     filter2_["Ë"] = 'E';     
     filter2_["--"] = " - ";
     
     filter3_["—"]= " - ";
     filter3_["..."]= ' ';
     filter3_["’"] = '\'';
     filter3_["“"] = '\"';
     filter3_["”"] = '\"';
     filter3_["&"]= "and";

     this->compute();
}

void Filter::compute()
{
     for(int n=0; n< input_.size(); ++n) {
	  string c2, c3;
	  c2= input_.substr(n,2);
	  c3= input_.substr(n,3);
	  map<string,string>::iterator fiter2= filter2_.find(c2);
	  map<string,string>::iterator fiter3= filter3_.find(c3);
	  if( fiter2 != filter2_.end() ) {
	       output_ += fiter2->second;
	       n += fiter2->first.size()-1;
	  }	  
	  else if( fiter3 != filter3_.end() ) {
	       output_ += fiter3->second;
	       n += fiter3->first.size()-1;
	  }
	  else
	       output_ += input_.at(n);

     }     
}
