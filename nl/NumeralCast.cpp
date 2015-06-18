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



#include"NumeralCast.hpp"

const bool debug = false;

NumeralCast::NumeralCast(const string &input)
{
     base_.push_back(make_pair("a dozen",    12));
     base_.push_back(make_pair("a hundred",  100));
     base_.push_back(make_pair("a thousand", 1000));
     base_.push_back(make_pair("a million",  1000000));
     base_.push_back(make_pair("a billion",  1000000000));
     base_.push_back(make_pair("millions",  1000000));
     base_.push_back(make_pair("Millions",  1000000));
     base_.push_back(make_pair("millions",  1000000));
     base_.push_back(make_pair("hundreds",  100));
     base_.push_back(make_pair("Hundreds",  100));
     base_.push_back(make_pair("thousands", 1000));
     base_.push_back(make_pair("Thousands", 1000));

     base_.push_back(make_pair("eleven",   11));
     base_.push_back(make_pair("twelve",   12));
     base_.push_back(make_pair("thirteen", 13));
     base_.push_back(make_pair("fourteen", 14));
     base_.push_back(make_pair("fifteen",  15));
     base_.push_back(make_pair("sixteen",  16));
     base_.push_back(make_pair("seventeen",17));
     base_.push_back(make_pair("eighteen", 18));
     base_.push_back(make_pair("nineteen", 19));
     base_.push_back(make_pair("twenty", 20));
     base_.push_back(make_pair("thirty", 30));
     base_.push_back(make_pair("forty",  40));
     base_.push_back(make_pair("fifty",  50));
     base_.push_back(make_pair("sixty",  60));
     base_.push_back(make_pair("seventy",70));
     base_.push_back(make_pair("eighty", 80));
     base_.push_back(make_pair("ninety", 90));
     base_.push_back(make_pair("zero",  0));
     base_.push_back(make_pair("one",   1));
     base_.push_back(make_pair("two",   2));
     base_.push_back(make_pair("three", 3));
     base_.push_back(make_pair("four",  4));
     base_.push_back(make_pair("five",  5));
     base_.push_back(make_pair("six",   6));
     base_.push_back(make_pair("seven", 7));
     base_.push_back(make_pair("eight", 8));
     base_.push_back(make_pair("nine",  9));
     base_.push_back(make_pair("ten",   10));
     
     base_.push_back(make_pair("Eleven",   11));
     base_.push_back(make_pair("Twelve",   12));
     base_.push_back(make_pair("Thirteen", 13));
     base_.push_back(make_pair("Fourteen", 14));
     base_.push_back(make_pair("Fifteen",  15));
     base_.push_back(make_pair("Sixteen",  16));
     base_.push_back(make_pair("Seventeen",17));
     base_.push_back(make_pair("Eighteen", 18));
     base_.push_back(make_pair("Nineteen", 19));
     base_.push_back(make_pair("Twenty", 20));
     base_.push_back(make_pair("Thirty", 30));
     base_.push_back(make_pair("Forty",  40));
     base_.push_back(make_pair("Fifty",  50));
     base_.push_back(make_pair("Sixty",  60));
     base_.push_back(make_pair("Seventy",70));
     base_.push_back(make_pair("Eighty", 80));
     base_.push_back(make_pair("Ninety", 90));
     base_.push_back(make_pair("Zero",  0));
     base_.push_back(make_pair("One",   1));
     base_.push_back(make_pair("Two",   2));
     base_.push_back(make_pair("Three", 3));
     base_.push_back(make_pair("Four",  4));
     base_.push_back(make_pair("Five",  5));
     base_.push_back(make_pair("Six",   6));
     base_.push_back(make_pair("Seven", 7));
     base_.push_back(make_pair("Eight", 8));
     base_.push_back(make_pair("Nine",  9));
     base_.push_back(make_pair("Ten",   10));

     multi_.push_back(make_pair("hundred",  100));
     multi_.push_back(make_pair("thousand", 1000));
     multi_.push_back(make_pair("million",  1000000));
     multi_.push_back(make_pair("billion",  1000000000));
     multi_.push_back(make_pair("trillion", 1000000000000));
     
     multi_.push_back(make_pair("Hundred",  100));
     multi_.push_back(make_pair("Thousand", 1000));
     multi_.push_back(make_pair("Million",  1000000));
     multi_.push_back(make_pair("Billion",  1000000000));
     multi_.push_back(make_pair("Trillion", 1000000000000));

     inb_.push_back(" ");
     //inb_.push_back("and"); /// Needs to be processed carefully: "between 1956 and 1967"

	// Compute the result
	this->compute(input);
}

static string all_lowercase(string str)
{
     for(int i=0; i < str.size(); ++i) {
		str.at(i)= std::tolower(str.at(i));
     }

     return str;
}

static int find_number_in_string(const string &str, int start = 0)
{
     int pos=-1;
     for(int i=start; i < str.size(); ++i) {
		if(isdigit(str.at(i))) {
			pos= i;
			break;
		}
     }
     for(int i=pos; i < str.size(); ++i) {
		if(i > 0 && !isdigit(str.at(i))) {
			if( str.at(i) == '.'
			    || str.at(i) == ':'
			    || str.at(i) == ','
			    ) {
				pos = -1;
			}
			break;
		}
     }

     return pos;
}

static pair<int,string> cast_number_from_position_in_string(const string &str, int start)
{     
     int number=-1, end = -1;
     for(int i=start; i < str.size(); ++i) {
		if(!isdigit(str.at(i))) {
			end= i;
			break;
		}
     }
     string to_cast= str.substr(start,end-start);
     try {
		number= boost::lexical_cast<int>(to_cast);
     }
     catch(std::exception &e) {
     }
     return make_pair(number,to_cast);
}

void NumeralCast::compute(const string &input)
{
     //input_= all_lowercase(input);
     input_ = input;

     int output= 0;
     
     vector<pair<string,int> >::iterator base_iter;
     vector<pair<string,int> >::iterator multi_iter;
     vector<string>::iterator inb_iter;

     int old_pos= input_.size()+1;
     int pos= -1;
     while(true) {
		int start=-1, end=-1;
		// find the first character for the number
		for(base_iter= base_.begin(); base_iter != base_.end(); ++base_iter) {
			int tmp_pos;
			if(pos != -1)
				tmp_pos= input_.find(base_iter->first, pos);
			else
				tmp_pos= input_.find(base_iter->first);

			if( tmp_pos != string::npos
			    && tmp_pos < old_pos
			    && (input_.at(tmp_pos + base_iter->first.size()) == ' '
				   || input_.at(tmp_pos + base_iter->first.size()) == ')'
				   || input_.at(tmp_pos + base_iter->first.size()) == ','
				   || input_.at(tmp_pos + base_iter->first.size()) == '.'
				   || input_.at(tmp_pos + base_iter->first.size()) == '!'
				   || input_.at(tmp_pos + base_iter->first.size()) == '?'
				   || input_.at(tmp_pos + base_iter->first.size()) == ':'
				   || input_.at(tmp_pos + base_iter->first.size()) == ';'
				   )
			    && (tmp_pos == 0
				   || (tmp_pos > 0 && ( input_.at(tmp_pos -1) == ' '
								    || input_.at(tmp_pos -1) == '('
								    || input_.at(tmp_pos -1) == '\n'
								    || input_.at(tmp_pos -1) == '\r'
								    )
					  )
				   )
			    ) {
				old_pos = tmp_pos;
			}
			else if( tmp_pos == string::npos ) {
				if(pos != -1)
					tmp_pos= find_number_in_string(input_, pos);
				else
					tmp_pos= find_number_in_string(input_);
				if(tmp_pos != string::npos
				   && tmp_pos < old_pos			
				   ) {
					old_pos = tmp_pos;
				}
			}
		}

		if(old_pos < input_.size())
			pos= old_pos;
		// Convert the string into an integer
		bool go_on_trigger= true, inb_trigger= false;
		int alt_pos= pos;
		while(pos != -1
			 && go_on_trigger) {
			go_on_trigger = false;	       
			for(base_iter= base_.begin(); base_iter != base_.end(); ++base_iter) {
				int tmp_pos= input_.find(base_iter->first, pos);
				if(tmp_pos == pos) {
					if(start < 0) 
						start= pos;
					output += base_iter->second;
					pos += base_iter->first.size()+1;
					go_on_trigger = true;
					inb_trigger= false;
					break;
				}
			}	       
			// Search for a number instead of a string
			if( !go_on_trigger) {
				int tmp_pos = find_number_in_string(input_, pos);
				if(tmp_pos == pos) {
					if(start < 0) 
						start= pos;
					pair<int,string> cast_number= cast_number_from_position_in_string(input_, tmp_pos);
					output += cast_number.first;
					pos += cast_number.second.size()+1;
					go_on_trigger = true;
					inb_trigger= false;
				}
			}
			for(inb_iter= inb_.begin(); inb_iter != inb_.end(); ++inb_iter) {
				int tmp_pos= input_.find(*inb_iter, pos);
				if(tmp_pos == pos) {
					alt_pos= pos;
					pos += inb_iter->size()+1;
					go_on_trigger = true;
					inb_trigger= true; // the last term was an "in betweener"
					break;
				}
			}
			for(multi_iter= multi_.begin(); multi_iter != multi_.end(); ++multi_iter) {
				int tmp_pos= input_.find(multi_iter->first, pos);
				if(tmp_pos == pos) {
					output *= multi_iter->second;
					pos += multi_iter->first.size()+1;
					go_on_trigger = true;
					inb_trigger= false;
					break;
				}
			}
		}
		if(inb_trigger)  
			end= alt_pos; // "one and" is not a number
		else
			end= pos;
		if(start != -1) {
			old_pos= input_.size()+1;
			pos= end;
			substitutions_.push_back( make_tuple(start, end-1, output) );
			output=0;
		}
		else break;
     }
     this->applySubstitutions();
}

void NumeralCast::applySubstitutions()
{
     vector<tuple<int,int,int> >::iterator siter= substitutions_.end();
     vector<tuple<int,int,int> >::iterator sbegin= substitutions_.begin();

     output_= input_;
     
     while(siter != sbegin) {
		--siter;
		int start = siter->get<0>();
		int end   = siter->get<1>();
		int output= siter->get<2>();
		string out_str= boost::lexical_cast<string>(output);
		output_.replace(start, end-start, out_str);
     }
}
