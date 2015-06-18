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



#ifndef __POLICY__
#define __POLICY__


#include<iostream>
#include<fstream>
#include<boost/utility.hpp>
#include<vector>
#include<cmath>
#include"FuzzyPred.hpp"
#include"Clause.hpp"

class Policy 
{

public:
     Policy() {}
     virtual ~Policy() {}
     virtual double canInsert(vector<FuzzyPred> &, 
			      Clause &, 
			      //vector<FuzzyPred> &, 
			      genUpg *retUpg,
			      int *pos
			      ) = 0;
     virtual double canInsert_fast(vector<FuzzyPred> &, 
				   Clause &
				   ) = 0;
};


class policy_non_contiguous : public Policy
{

public:
     policy_non_contiguous() {}
     ~policy_non_contiguous() {}
     double canInsert(vector<FuzzyPred> &, 
		      Clause &, 
		      //vector<FuzzyPred> &, 
		      genUpg *retUpg,
		      int *pos
		      );
     double canInsert_fast(vector<FuzzyPred> &, 
			   Clause &
			   );
};

class policy_contiguous : public Policy
{

public:
     policy_contiguous() {}
     ~policy_contiguous() {}
     double canInsert(vector<FuzzyPred> &, 
		      Clause &, 
		      //vector<FuzzyPred> &, 
		      genUpg *retUpg,
		      int *pos
		      );
     double canInsert_fast(vector<FuzzyPred> &, 
			   Clause &
			   );
};


#endif // __POLICY__
