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



/* 
 * File:   Presupposition.hpp
 * Author: alce
 *
 * Created on 15 August 2013, 2:57 PM
 */

#ifndef PRESUPPOSITION_HPP
#define	PRESUPPOSITION_HPP

#include"../drt/drt.hpp"
#include"../knowledge/Knowledge.hpp"


class Presupposition {
     drt orig_drt_;
     vector<drt> answers_;
     vector<DrtPred> conjunctions_;
     
     void compute();
     
public:
     Presupposition(const drt &d);
     vector<drt> get() {return answers_;}   
     vector<DrtPred> getConjs() {return conjunctions_;}   
};



#endif	/* PRESUPPOSITION_HPP */

