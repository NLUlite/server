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



// $Id: main.cpp,v 1.7 2004/12/20 07:22:04 alce Exp $
// $Log: main.cpp,v $
// Revision 1.7  2004/12/20 07:22:04  alce
// Added [untested] depth iterator.
//
// Revision 1.6  2004/12/19 07:56:55  alce
// Fast Resolver Of Goals.
//
// Revision 1.5  2004/12/16 10:52:03  alce
// added throw TreeError on null nodes.
//
// Revision 1.4  2004/11/30 08:56:07  alce
// Manages also head replacing and deleting
//
// Revision 1.3  2004/11/30 05:04:53  alce
// replace, insert and find methods added.
//
// Revision 1.2  2004/11/29 08:13:53  alce
// Compy constructor and copy method. You can extract subtrees.
//
// Revision 1.1.1.2  2004/11/28 22:40:45  alce
// Simple Tree Class
//

#include"mytree.hpp"
#include"mytree.cpp"
#include<string>

using namespace std;

inline void printTree(const Tree<string> &tree)
{
  Tree<string>::iterator i= tree.begin();
  for(i= tree.begin(); i != tree.end(); ++i)
	 cerr << string(i.depth(),' ')  << "::" << *i << endl;
}

int main(int argc, char **argv)
{
  Tree<string> tree("Head");
  Tree<string>::iterator i= tree.begin(), i1, i2, i3, i4, i5, i6;

  i= tree.appendChild(i,"theNeck");
  i= tree.appendChild(i,"Shoulders");
  i3= i;
  i5= tree.appendChild(i,"RightArm");
  i2=tree.appendChild(i,"Spine");
  tree.appendChild(i2,"Legs");
  tree.appendChild(i,"LeftArm");
  printTree(tree);

  Tree<string> subTree(i3), tree2;
  printTree(subTree);

  tree2= tree.subTree(i2);
  printTree(tree2);

  cerr << "SubTree\n";
  i4= subTree.findSubtree(tree2);
  Tree<string> treep(i4);
  printTree(treep);
  
  tree.erase(i2);
  printTree(tree);

  Tree<string> anTree(tree);
  printTree(anTree);

  if(anTree == tree) cerr << "Equal!\n";
  if(anTree != subTree) cerr << "nonononoEqual!\n";

  i5= tree.insert(anTree,i5);
  printTree(tree);

  tree.erase(tree.begin());
  tree.rename(tree.begin(),"Head");
  i1= tree.appendChild(tree.begin(),"theNeck");
  i2= tree.appendChild(i1,"Shoulders");
  i3= tree.appendChild(i2,"RightArm");
  i4= tree.appendChild(i2,"Spine");
  i5= tree.appendChild(i4,"Legs");
  i5= tree.appendChild(i4,"Legs");
  i6= tree.appendChild(i2,"LeftArm");
  printTree(tree);
  cerr << tree.height() << endl;
  tree.cut(i4);
  printTree(tree);
  
  Tree<string>::children_iterator incr;
  for(incr= i2; incr != tree.end(); ++incr)
	 cerr << "---" <<*incr;
  cerr << endl;

  incr= i1;
  cerr << *incr << endl;
  cerr << incr.node->firstChild->data << endl;
  Tree<string> t(incr);
  printTree(t);

  try {
	 Tree<string>::depth_iterator justATry;
  for(int d=0; Tree<string>::depth_iterator(tree,d) != tree.end(); ++d)
	 {
		Tree<string>::depth_iterator dep(tree,d);
		while(dep != tree.end())
		  {
			 cerr << *dep << ':';
			 ++dep;
		  }
		cerr << endl;
	 }
  } catch (const Tree<string>::TreeError &te) {
	 cerr << te.what() << endl;
  }

//   for(Tree<string>::leaf_iterator iter(tree); iter != tree.end(); ++iter)
// 	 cerr << "-leafing-" <<*iter;
//   cerr << endl;

//   tree.replace(subTree, i6);//tree.begin());
//   printTree(tree);

  tree.erase(tree.begin());

  // Freeing memory test:

//   for(int incr=0; incr < 100*100; ++incr)
// 	 {
// 		cerr << incr <<":";
// 		for(int incr2= 0; incr2 < 100; ++incr2)
// 		  {
// 			 i= tree.appendChild(tree.begin(), "This is just a memory test.");
// 			 for(int incr3= 0; incr3 < 100; ++incr3)
// 				tree.appendChild(i, "This is just a childish memory test.");
// 		  }
// 		tree.erase(tree.begin());
// 	 }

  return 1;
}
