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



#include"path_memory.hpp"

void path_memory::close()
{
	if(is_closed_== false) {
		weigth_= last_weigth_;
		first_upg_.clear();
		clause_history_.clear();
		DrtMgu tmp_upg, old_upg;
		old_upg= last_upg_;
		for(int m=memory_.size()-1; m >= 0 ; --m) {
			weigth_ *= memory_.at(m).get<2>();
			tmp_upg=memory_.at(m).get<1>();
			clause_history_.push_back(memory_.at(m).get<0>());
			tmp_upg.add(old_upg);
			old_upg= tmp_upg;
		}
		first_upg_ = tmp_upg;
		is_closed_=true;
	}
}

