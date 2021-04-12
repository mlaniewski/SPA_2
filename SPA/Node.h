#pragma once
#include "pkb.h"
#include <string>
#include <tuple>
#include <map>

using namespace std;
class Node
{
	typedef map<NODEPARAMTYPE, string> paramMap;
public:
	static NODE getNodeById(int id);
	Node(NODETYPE nodeType);
	~Node();
	NODETYPE nodeType;
	void setParam(NODEPARAMTYPE type, string val);
	string getParam(NODEPARAMTYPE type);
	int lineNumber;
	unsigned int id;
	NODE getTreeIterator();
	void setTreeIterator(NODE node);
private:
	static unsigned int nextId;
	static map<unsigned int, NODE> nodes;
	NODE nodeIter;
	paramMap params;
};

