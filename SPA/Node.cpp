#include "Node.h"

using namespace std;

unsigned int Node::nextId = 0;
map<unsigned int, NODE> Node::nodes = map<unsigned int, NODE>();

Node::Node(NODETYPE nodeType) : nodeType(nodeType), id(nextId++)
{

}


Node::~Node()
{
}

void Node::setParam(NODEPARAMTYPE type, string val)
{
	params[type] = val;
}

string Node::getParam(NODEPARAMTYPE type)
{
	return params[type];
}

NODE Node::getTreeIterator()
{
	return nodeIter;
}

void Node::setTreeIterator(NODE node)
{
	nodeIter = node;
	nodes[id] = node;
}

NODE Node::getNodeById(int id)
{
	return nodes[id];
}