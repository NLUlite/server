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




     // Corrects VBN that should be "VBD" if followed by a "IN(by)"
     // NO! it can be "sold by the 1990s"
     // words_iter  = words.begin();
     // tagged_iter = tagged.begin()+1;          
     // while( boost::next(tagged_iter) != tagged.end() ) {
     // 	  word = *words_iter;
     // 	  tag  = *tagged_iter;
     //      base = info->get_conj(word, "VBN");
     //      cout << "CORRECT::: " << word << " " << tag << " " << base << endl;
     //      if (base != "" && tag == "VBD" 
     //          && *boost::next(tagged_iter) == "IN"
     //          && *boost::next(words_iter) == "by"
     //              )
     //           *tagged_iter = "VBN";    
     // 	  ++tagged_iter;
     // 	  ++words_iter;
     // }
     
