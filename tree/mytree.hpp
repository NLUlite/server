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



#ifndef _TREE_
#define _TREE_

#include<iostream>
#include<string>
#include<exception>
//#include"../fixed_allocator/fixed_allocator.hpp"
#include<boost/pool/pool_alloc.hpp>

template<class T>
class Node {
public:
     Node *parent,
	  *firstChild,
	  *lastChild,
	  *dummy,
	  *nextSibling;
     T data;
     
     Node():
	  //	 data(),
	  parent(0),
	  firstChild(0),
	  lastChild(0),
	  dummy(0),
	  nextSibling(0) {}
     Node(const T &t):
	  data(t),
	  parent(0),
	  firstChild(0),
	  lastChild(0),
	  dummy(0),
	  nextSibling(0) {}
};

// Caution: Every method can throw a TreeError exception when parsing a NULL node.

template<class T, class A>
class AllocMan
{
     A allocator;

public:
     AllocMan() {}
     ~AllocMan() {}
     
     T* construct(const T &value= T())
     {
	  T* points= allocator.allocate(1);
	  allocator.construct(points, value);
	  return points;
     }
     void destroy(T *points)
     {
	  allocator.destroy(points);
	  allocator.deallocate(points,1);
     }
};



template<class T,
	 //class A= fixed_allocator<Node<T> >,
	 class A= std::allocator<Node<T> >,
	 //class A= boost::pool_allocator<Node<T> >,
	 class M= AllocMan<Node<T>,A> >
class FTree {
public:
     class iterator;

protected:
     Node<T> *head;
     static M allocMan;

     void buildTree(const iterator &);
     void buildTree(const FTree &tree_to_copy, const iterator &i);

public:  
     FTree();
     FTree(const T&);
     FTree(const iterator &);
     FTree(const FTree &tree);
     ~FTree();
     FTree& operator = (const FTree &);
     bool operator == (const FTree &) const;
     bool operator != (const FTree &rhs) {return !(*this == rhs);}

     void erase(const iterator &);  // Erase a node and all his children.
     // CAUTION! Does not delete head, just empty his data!

     void cut(const iterator&);	  // Erase all node's children
     int height() const;
     Node<T>* header() const {return head;}
     bool isMine(const iterator&) const;
     iterator rename(const iterator &, const T&);
     iterator appendChild(const iterator&, const T&) const;
     iterator appendChildFront(const iterator&, const T&) const;
     iterator appendTree(const iterator&, const FTree &);
     iterator appendTreeFront(const iterator&, const FTree &);
     iterator begin() const {return iterator(*head);}
     iterator end() const {return iterator();}
     iterator findSubtree(const FTree &, const iterator &iter=iterator()) const;
     iterator findData(const T &, const iterator &iter= iterator()) const;
     iterator insert(const FTree &, const iterator &);
     iterator replace(const FTree &, const iterator &);
     void replaceEachData(const T &,
			  const FTree &,
			  const iterator &iter= iterator());
     FTree subTree(const iterator &) const;
     void trim(const int &heigth);

     class iterator
     {
     public:
	  iterator(): node(NULL) {}
	  //	 iterator(const iterator &);
	  iterator(Node<T> &n): node(&n) {}
	  iterator(const FTree &tree) {node= tree.head;}
	  T& operator*() const {return node->data;}
	  T* operator->() const {return &node->data;}
	  iterator operator ++ ();
	  iterator up ();
	  iterator down ();
	  bool leftLeaf() const;
	  bool rightLeaf() const ;
	  iterator nextSibling() {return *node->nextSibling;}
	  iterator firstChild() {return *node->firstChild;}
	  iterator lastChild() {return *node->lastChild;}
	  iterator parent() {return *node->parent;}
	  bool hasChild() const {return node->firstChild != NULL;}
	  bool hasParent() const {return node->parent != NULL;}
	  virtual int depth() const;
	  int height() const;
	  int num_children() const;
	  bool operator == (const iterator &rhs) const {return node == rhs.node;}
	  bool operator != (const iterator &rhs) const {return !(node == rhs.node);}
	  Node<T>* firstLeafLeft() const;
	  Node<T>* firstLeafRight() const;
   
	  Node<T> *node;
     };
     class children_iterator: public iterator
     {
     public:
	  children_iterator(): iterator() {}
	  children_iterator(Node<T> &n);
	  children_iterator(const FTree<T,A,M>::iterator &);
	  children_iterator(const FTree<T,A,M> &tree) {*this= children_iterator(*tree.head);}
	  children_iterator operator ++ ();
     };
     class leaf_iterator: public iterator
     {
     public:
	  leaf_iterator(): iterator() {}
	  leaf_iterator(Node<T> &n);
	  leaf_iterator(const FTree<T,A,M> &tree) {*this= leaf_iterator(*tree.head);}
	  leaf_iterator(const FTree<T,A,M>::iterator &i) {*this= leaf_iterator(*i.node);}
	  leaf_iterator operator ++ ();
     };
     class depth_iterator: public iterator
     {
	  int mydepth;
     public:
	  depth_iterator(): iterator(), mydepth(0) {}
	  depth_iterator(Node<T> &n) { *this= depth_iterator(iterator(n));}
	  depth_iterator(const FTree<T,A,M> &, const int&);
	  depth_iterator(const FTree<T,A,M>::iterator &i) {this->node= i.node; mydepth= i.depth();}
	  depth_iterator operator ++ ();
	  depth_iterator beginAt(const int&);
	  int depth() {return mydepth;}
     };
     class height_iterator: public iterator
     {
	  int myheight;
     public:
	  height_iterator(): iterator(), myheight(0) {}
	  height_iterator(Node<T> &n) { *this= height_iterator(iterator(n));}
	  height_iterator(const FTree<T,A,M> &, const int&);
	  height_iterator(const FTree<T,A,M>::iterator &i) {this->node= i.node; myheight= i.height();}
	  height_iterator operator ++ ();
	  int get_height() {return myheight;}
     };
  
     //Exception::
     class TreeError: public std::exception
     {
	  std::string str;
     public:	 
	  TreeError(const std::string &s) :str(s) {}
	  ~TreeError() throw() {}
	  const char * what() const throw() {return str.c_str();}
     };
};

#endif // _TREE_
