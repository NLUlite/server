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



#include<typeinfo>
#include"mytree.hpp"
#include<string>
#include<iostream>
#include<vector>

template<class T, class A, class M>
M FTree<T, A, M>::allocMan;

// FTree Class:
template<class T, class A, class M>
FTree<T, A, M>::FTree()
{
	head = allocMan.construct();
	//head= new Node<T>;
}
template<class T, class A, class M>
FTree<T, A, M>::FTree(const T &headValue)
{
	head = allocMan.construct(headValue);
	//head= new Node<T>(headValue);
}
template<class T, class A, class M>
void FTree<T, A, M>::buildTree(const FTree<T, A, M>::iterator &i)
{
	Node<T> *run = i.node;
	head = allocMan.construct(run->data);
	iterator inTree(*head);

	if (run == NULL)
		throw TreeError((std::string) "Parsing a void " + typeid(T).name() + " node");
	if (run->firstChild) {
		run = run->firstChild;
		inTree = appendChild(inTree, run->data);
		while (true) {
			if (run->parent == i.node && !run->firstChild && !run->nextSibling)
				break;
			else if (run->firstChild) {
				run = run->firstChild;
				inTree = appendChild(inTree, run->data);
			} else if (run->nextSibling) {
				run = run->nextSibling;
				inTree = appendChild(iterator(*inTree.node->parent), run->data);
			} else {
				while (run->parent && !run->nextSibling && run->parent != i.node) {
					run = run->parent;
					inTree = iterator(*inTree.node->parent);
				}
				if (run->nextSibling) {
					run = run->nextSibling;
					inTree = appendChild(iterator(*inTree.node->parent), run->data);
				} else
					break;
			}
		}
	}
}

template<class T, class A, class M>
void FTree<T, A, M>::buildTree(const FTree<T, A, M> &tree_to_copy, const FTree<T, A, M>::iterator &i)
// Build a new subtree "tree_to_copy" a the position "i"
// This function requires the node at "i" to be already allocated and inserted into the tree
{
	Node<T> *run = tree_to_copy.head;
	iterator inTree(*i.node);

	if (run == NULL)
		throw TreeError((std::string) "Parsing a void " + typeid(T).name() + " node");
	if (run->firstChild) {
		run = run->firstChild;
		inTree = appendChild(inTree, run->data);
		while (true) {
			if (run->parent == i.node && !run->firstChild && !run->nextSibling)
				break;
			else if (run->firstChild) {
				run = run->firstChild;
				inTree = appendChild(inTree, run->data);
			} else if (run->nextSibling) {
				run = run->nextSibling;
				inTree = appendChild(iterator(*inTree.node->parent), run->data);
			} else {
				while (run->parent && !run->nextSibling && run->parent != i.node) {
					run = run->parent;
					inTree = iterator(*inTree.node->parent);
				}
				if (run->nextSibling) {
					run = run->nextSibling;
					inTree = appendChild(iterator(*inTree.node->parent), run->data);
				} else
					break;
			}
		}
	}
}

template<class T, class A, class M>
FTree<T, A, M>::FTree(const FTree<T, A, M> &tree)
{
	buildTree(*tree.head);
}

template<class T, class A, class M>
FTree<T, A, M>::FTree(const FTree<T, A, M>::iterator &i)
{
	buildTree(i);
}
template<class T, class A, class M>
FTree<T, A, M>::~FTree()
{
	erase(iterator(*head));
	allocMan.destroy(head);
	//delete head;
}
template<class T, class A, class M>
FTree<T, A, M>& FTree<T, A, M>::operator =(const FTree<T, A, M> &tree)
{
	if (tree.head == NULL)
		throw TreeError((std::string) "Parsing a void " + typeid(T).name() + " node");
	if (this->head == tree.head)
		return *this;
	erase(iterator(*head));
	allocMan.destroy(head);
	buildTree(iterator(*tree.head));
	return *this;
}
template<class T, class A, class M>
bool FTree<T, A, M>::operator ==(const FTree<T, A, M> &rhs) const
{
	FTree<T, A, M>::iterator sx = begin(), dx = rhs.begin();
	for (; sx != end() && dx != rhs.end(); ++sx, ++dx)
		if (*sx != *dx)
			return false;
	if (sx == end() && dx == rhs.end())
		return true;
	return false;
}
template<class T, class A, class M>
bool FTree<T, A, M>::isMine(const iterator &i) const
{
	if (i.node == NULL)
		throw TreeError((std::string) "Parsing a void " + typeid(T).name() + " node");
	Node<T> *toTheHead = i.node;
	while (toTheHead->parent)
		toTheHead = toTheHead->parent;
	if (toTheHead == head)
		return true;
	return false;
}
template<class T, class A, class M>
typename FTree<T, A, M>::iterator FTree<T, A, M>::appendChild(const iterator &i, const T &t) const
{
	if (!isMine(i))
		return i;

	Node<T> *tmpNode = allocMan.construct(t);
	tmpNode->parent = i.node;
	if (i.node->firstChild) {
		tmpNode->dummy = i.node->lastChild;
		i.node->lastChild->nextSibling = tmpNode;
		i.node->lastChild = tmpNode;
	} else {
		i.node->firstChild = tmpNode;
		i.node->lastChild = tmpNode;
	}

	FTree<T, A, M>::iterator retIter(*tmpNode);
	return retIter;
}

template<class T, class A, class M>
typename FTree<T, A, M>::iterator FTree<T, A, M>::appendChildFront(const iterator &i, const T &t) const
{
	if (!isMine(i))
		return i;

	Node<T> *tmpNode = allocMan.construct(t);
	tmpNode->parent = i.node;
	if (i.node->firstChild) {
		tmpNode->nextSibling = i.node->firstChild;
		i.node->firstChild->dummy = tmpNode;
		i.node->firstChild = tmpNode;
	} else {
		i.node->firstChild = tmpNode;
		i.node->lastChild = tmpNode;
	}

	FTree<T, A, M>::iterator retIter(*tmpNode);
	return retIter;
}

template<class T, class A, class M>
typename FTree<T, A, M>::iterator FTree<T, A, M>::appendTree(const FTree<T, A, M>::iterator &at, const FTree<T, A, M> &t)
{
	FTree<T, A, M>::iterator where = appendChild(at, t.head->data);
	replace(t, where);
	return where;
}

template<class T, class A, class M>
typename FTree<T, A, M>::iterator FTree<T, A, M>::appendTreeFront(const FTree<T, A, M>::iterator &at, const FTree<T, A, M> &t)
{
	FTree<T, A, M>::iterator where = appendChildFront(at, t.head->data);
	replace(t, where);
	return where;
}

template<class T, class A, class M>
typename FTree<T, A, M>::iterator FTree<T, A, M>::findSubtree(const FTree<T, A, M> &rhs,
		const FTree<T, A, M>::iterator &beginAt) const
{
	if (rhs.head == NULL)
		throw TreeError((std::string) "Parsing a void " + typeid(T).name() + " node");
	for (FTree<T, A, M>::iterator here = (beginAt == iterator()) ? begin() : beginAt; here != end(); ++here)
		if (rhs.head->data == *here)
			if (rhs == here)
				return here;
	return end();
}
template<class T, class A, class M>
inline typename FTree<T, A, M>::iterator FTree<T, A, M>::findData(const T &rhs, const FTree<T, A, M>::iterator &beginAt) const
{
	const FTree<T, A, M>::iterator end = iterator();
	for (FTree<T, A, M>::iterator here = (beginAt == iterator()) ? begin() : beginAt; here != end; ++here)
		if (rhs == *here)
			return here;
	return iterator();
}
template<class T, class A, class M>
inline void FTree<T, A, M>::replaceEachData(const T &rhs, const FTree<T, A, M> &tree, const FTree<T, A, M>::iterator &beginAt)
{
	FTree<T, A, M>::iterator tend = FTree<T, A, M>::iterator();
	for (FTree<T, A, M>::iterator here = (beginAt == iterator()) ? begin() : beginAt; here != tend;) {
		if (rhs == *here)
			here = replace(tree, here);
		++here;
	}
}
template<class T, class A, class M>
typename FTree<T, A, M>::iterator FTree<T, A, M>::rename(const iterator &i, const T &t)
{
	if (i.node == NULL)
		throw TreeError((std::string) "Parsing a void " + typeid(T).name() + " node");
	if (!isMine(i))
		return i;
	i.node->data = t;
	return i;
}
template<class T, class A, class M>
typename FTree<T, A, M>::iterator FTree<T, A, M>::insert(const FTree<T, A, M> &tree, const iterator &at)
{
	if (at.node == NULL)
		throw TreeError((std::string) "Parsing a void " + typeid(T).name() + " node");
	if (!at.node->parent) {
		throw TreeError("Cannot modify tree head.");
	}

	if (at.node == head)
		return end();

	Node<T> *new_node = allocMan.construct();
	new_node->data = tree.head->data;

	if (at.node->nextSibling) {
		new_node->dummy = at.node;
		new_node->nextSibling = at.node->nextSibling;
		at.node->nextSibling->dummy = new_node;
		at.node->nextSibling = new_node;
		new_node->parent = at.node->parent;
	} else {
		new_node->parent = at.node->parent;
		at.node->parent->lastChild = new_node;
		at.node->nextSibling = new_node;
	}
	this->buildTree(tree, iterator(*new_node));
	return iterator(*new_node);
}
template<class T, class A, class M>
typename FTree<T, A, M>::iterator FTree<T, A, M>::replace(const FTree<T, A, M> &tree, const iterator &at)
{
	if (tree.head == NULL)
		throw TreeError((std::string) "Parsing a void " + typeid(T).name() + " node");

	if (at == *head) {
		erase(*head);
		allocMan.destroy(head);
		buildTree(iterator(*tree.head));
		return *head;
	}

	FTree<T, A, M>::iterator retIter = insert(tree, at);
	erase(at);
	return retIter;
}
template<class T, class A, class M>
int FTree<T, A, M>::height() const
{
	int ret = 0, d;
	iterator run;
	run.node = head;

	while (run.node != NULL) {
		d = run.depth();
		ret = (ret < d) ? d : ret;
		++run;
	}
	return ++ret;
}
template<class T, class A, class M>
void FTree<T, A, M>::cut(const iterator &i)
{
	if (i.node == NULL)
		throw TreeError((std::string) "Parsing a void " + typeid(T).name() + " node");
	while (i.node->firstChild)
		erase(*i.node->firstChild);
}
template<class T, class A, class M>
void FTree<T, A, M>::erase(const iterator &i)
{
	if (i.node == NULL)
		throw TreeError((std::string) "Parsing a void " + typeid(T).name() + " node");
	if (i.node == head) {
		while (i.node->firstChild)
			erase(*i.node->firstChild);
		allocMan.destroy(head);
		head = allocMan.construct();
		return;
	}
	while (i.node->firstChild) {
		erase(*i.node->firstChild);
	}
	if (i.node->nextSibling) {
		if (i.node->parent->firstChild == i.node)
			i.node->parent->firstChild = i.node->nextSibling;
		i.node->nextSibling->dummy = i.node->dummy;
	}
	if (i.node->dummy) {
		if (i.node->parent->lastChild == i.node)
			i.node->parent->lastChild = i.node->dummy;
		i.node->dummy->nextSibling = i.node->nextSibling;
	}
	if (!i.node->dummy && !i.node->nextSibling && i.node->parent) {
		i.node->parent->firstChild = NULL;
		i.node->parent->lastChild = NULL;
	}
	allocMan.destroy(i.node);
}
template<class T, class A, class M>
FTree<T, A, M> FTree<T, A, M>::subTree(const FTree<T, A, M>::iterator &i) const
{
	if (i.node == NULL)
		throw TreeError((std::string) "Parsing a void " + typeid(T).name() + " node");
	if (!isMine(i))
		return FTree(iterator(*head));
	return FTree(iterator(*i.node));
}
template<class T, class A, class M>
void FTree<T, A, M>::trim(const int &h)
{
	if (h > height())
		throw TreeError((std::string) "Cannot trim, the tree is not deep enough.");
	depth_iterator to_erase(*this, h);
	while (to_erase != end()) {
		cut(to_erase);
		++to_erase;
	}
}
template<class T, class A, class M>
Node<T>* FTree<T, A, M>::iterator::firstLeafLeft() const
{
	if (node == NULL)
		throw TreeError((std::string) "Parsing a void " + typeid(T).name() + " node");
	Node<T> *toTheHead = node;
	while (toTheHead->parent && !toTheHead->nextSibling)
		toTheHead = toTheHead->parent;
	if (toTheHead->parent)
		return toTheHead->nextSibling;
	return NULL;
}
template<class T, class A, class M>
Node<T>* FTree<T, A, M>::iterator::firstLeafRight() const
{
	if (node == NULL)
		throw TreeError((std::string) "Parsing a void " + typeid(T).name() + " node");
	Node<T> *toTheHead = node;
	while (toTheHead->parent && !toTheHead->dummy)
		toTheHead = toTheHead->parent;
	if (toTheHead->parent)
		return toTheHead->dummy->lastChild;
	return NULL;
}
template<class T, class A, class M>
bool FTree<T, A, M>::iterator::leftLeaf() const
{
	if (node == NULL)
		throw TreeError((std::string) "Parsing a void " + typeid(T).name() + " node");
	Node<T> *toTheHead = node;
	while (toTheHead->parent && !toTheHead->nextSibling)
		toTheHead = toTheHead->parent;
	if (!toTheHead->parent)
		return true;
	return false;
}
template<class T, class A, class M>
bool FTree<T, A, M>::iterator::rightLeaf() const
{
	if (node == NULL)
		throw TreeError((std::string) "Parsing a void " + typeid(T).name() + " node");
	Node<T> *toTheHead = node;
	while (toTheHead->parent && !toTheHead->dummy)
		toTheHead = toTheHead->parent;
	if (!toTheHead->parent)
		return true;
	return false;
}
template<class T, class A, class M>
int FTree<T, A, M>::iterator::depth() const
{
	if (this->node == NULL)
		throw TreeError((std::string) "Parsing a void " + typeid(T).name() + " node");
	int retDepth = 0;
	Node<T> *toTheHead = this->node;
	while (toTheHead->parent) {
		++retDepth;
		toTheHead = toTheHead->parent;
	}
	return retDepth;
}
template<class T, class A, class M>
int FTree<T, A, M>::iterator::height() const
{
	if (this->node == NULL)
		throw TreeError((std::string) "Parsing a void " + typeid(T).name() + " node");
	int retHeigth = 0;
	Node<T> *toTheBottom = this->node;
	while (toTheBottom->firstChild) {
		++retHeigth;
		toTheBottom = toTheBottom->firstChild;
	}
	return retHeigth;
}
template<class T, class A, class M>
int FTree<T, A, M>::iterator::num_children() const
{
	if (this->node == NULL)
		throw TreeError((std::string) "Parsing a void " + typeid(T).name() + " node");
	Node<T> *child;
	if (this->node->firstChild)
		child = this->node->firstChild;
	else
		return 0;
	int retChildren = 1;
	while (child->nextSibling) {
		++retChildren;
		child = child->nextSibling;
	}
	return retChildren;
}
template<class T, class A, class M>
typename FTree<T, A, M>::iterator FTree<T, A, M>::iterator::operator ++()
{
	if (node == NULL)
		throw TreeError((std::string) (std::string) "Parsing a void " + typeid(T).name() + " node");
	if (!node->parent && !node->firstChild)
		node = NULL;
	else if (node->firstChild)
		node = node->firstChild;
	else if (node->nextSibling)
		node = node->nextSibling;
	else
		node = firstLeafLeft();

	return *this;
}
template<class T, class A, class M>
typename FTree<T, A, M>::iterator FTree<T, A, M>::iterator::down()
{
	if (node == NULL)
		throw TreeError((std::string) "Parsing a void " + typeid(T).name() + " node");
	if (node->firstChild)
		node = node->firstChild;
	return *this;
}
template<class T, class A, class M>
typename FTree<T, A, M>::iterator FTree<T, A, M>::iterator::up()
{
	if (node == NULL)
		throw TreeError((std::string) "Parsing a void " + typeid(T).name() + " node");
	if (node->parent)
		node = node->parent;
	return *this;
}

// children_iterator::
template<class T, class A, class M>
FTree<T, A, M>::children_iterator::children_iterator(Node<T> &n)
{
	if (n.firstChild)
		this->node = n.firstChild;
	else
		this->node = &n;
}
template<class T, class A, class M>
FTree<T, A, M>::children_iterator::children_iterator(const FTree<T, A, M>::iterator &i)
{
	if (i.node == NULL)
		throw TreeError((std::string) "Parsing a void " + typeid(T).name() + " node");
	if (i.node->firstChild)
		this->node = i.node->firstChild;
	else
		this->node = i.node;
}
template<class T, class A, class M>
typename FTree<T, A, M>::children_iterator FTree<T, A, M>::children_iterator::operator ++()
{
	if (this->node == NULL)
		throw TreeError((std::string) "Parsing a void " + typeid(T).name() + " node");
	this->node = this->node->nextSibling;
	return *this;
}

// leaf_iterator::
template<class T, class A, class M>
FTree<T, A, M>::leaf_iterator::leaf_iterator(Node<T> &n)
{
	if (n.firstChild == NULL)
		throw TreeError((std::string) "Parsing a void " + typeid(T).name() + " node");
	this->node = &n;
	while (this->node->firstChild)
		this->node = this->node->firstChild;
}
template<class T, class A, class M>
typename FTree<T, A, M>::leaf_iterator FTree<T, A, M>::leaf_iterator::operator ++()
{
	FTree<T, A, M>::iterator leafing = *this->node, control = *this->node;

	if (leafing.node == NULL)
		throw TreeError((std::string) "Parsing a void " + typeid(T).name() + " node");
	while ((++control).node)
		if (!(++leafing).node->firstChild)
			break;
	if (control.node)
		*this = leaf_iterator(*control.node);
	else
		this->node = NULL;
	return *this;
}

// depth_iterator::
template<class T, class A, class M>
FTree<T, A, M>::depth_iterator::depth_iterator(const FTree<T, A, M> &tree, const int &d)
{
	if (d >= tree.height())
		throw TreeError((std::string) "Requesting a depth iterator which is too deep.");
	mydepth = 0;
	this->node = tree.head;
	while (this->node->firstChild && mydepth < d) {
		++mydepth;
		this->node = this->node->firstChild;
	}
	if (mydepth < d) {
		FTree<T, A, M>::iterator run;
		run.node = this->node;
		while ((++run).node)
			if (run.depth() == d) {
				this->node = run.node;
				mydepth = d;
				break;
			}
	}
	if (mydepth < d)
		this->node = NULL;
}
template<class T, class A, class M>
typename FTree<T, A, M>::depth_iterator FTree<T, A, M>::depth_iterator::operator ++()
{
	FTree<T, A, M>::iterator depthing;
	depthing.node = this->node;

	while ((++depthing).node) {
		if (depthing.node == NULL)
			throw TreeError((std::string) "Parsing a void " + typeid(T).name() + " node");
		if (depthing.depth() == mydepth)
			break;
	}
	this->node = depthing.node;
	return *this;
}
template<class T, class A, class M>
typename FTree<T, A, M>::depth_iterator FTree<T, A, M>::depth_iterator::beginAt(const int &d)
{
	if (this->node == NULL)
		throw TreeError((std::string) "Parsing a void " + typeid(T).name() + " node");
	while (this->node->parent)
		this->node = this->node->parent;
	while (this->node->firstChild && mydepth < d) {
		++mydepth;
		this->node = this->node->firstChild;
	}
	if (mydepth < d) {
		iterator run;
		run.node = this->node;
		while ((++run).node)
			if (run.depth() == d) {
				this->node = run.node;
				mydepth = d;
				break;
			}
	}
	if (mydepth < d)
		this->node = NULL;
	return *this;
}

// height_iterator::
template<class T, class A, class M>
FTree<T, A, M>::height_iterator::height_iterator(const FTree<T, A, M> &tree, const int &h)
{
	if (h >= tree.height())
		throw TreeError((std::string) "Requesting a height iterator which is too high.");
	FTree<T, A, M>::iterator heighting;
	heighting.node = tree.head;

	if (heighting.height() == h)  // already there
		return;

	while ((++heighting).node) {
		if (heighting.node == NULL)
			throw TreeError((std::string) "Parsing a void " + typeid(T).name() + " node");
		if (heighting.height() == h)
			break;
	}
	if (heighting.height() != h)
		this->node = NULL;
	else {
		this->node = heighting.node;
		myheight = h;
	}
}
template<class T, class A, class M>
typename FTree<T, A, M>::height_iterator FTree<T, A, M>::height_iterator::operator ++()
{
	FTree<T, A, M>::iterator heighting;
	heighting.node = this->node;

	while ((++heighting).node) {
		if (heighting.node == NULL)
			throw TreeError((std::string) "Parsing a void " + typeid(T).name() + " node");
		if (heighting.height() == myheight)
			break;
	}
	this->node = heighting.node;
	return *this;
}
