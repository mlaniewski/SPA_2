#include "AST.h"

using namespace std;

AST::AST(
	shared_ptr<tree<shared_ptr<Node>>> astTree,
	shared_ptr<vector<NODE>> procedures,
	shared_ptr<vector<NODE>> whiles,
	shared_ptr<vector<NODE>> ifs,
	shared_ptr<vector<NODE>> assigments,
	shared_ptr<set<string>> variables,
	shared_ptr<set<string>> constants,
	shared_ptr<vector<NODE>> programLines,
	shared_ptr<map<unsigned int, set<unsigned int>>> callers,
	shared_ptr<map<unsigned int, set<unsigned int>>> callersT,
	shared_ptr<map<unsigned int, set<unsigned int>>> callees,
	shared_ptr<map<unsigned int, set<unsigned int>>> calleesT,
	shared_ptr<map<unsigned int, set<string>>> modifies,
	shared_ptr<map<string, set<unsigned int>>> modified,
	shared_ptr<map<unsigned int, set<string>>> uses,
	shared_ptr<map<string, set<unsigned int>>> used,
	shared_ptr<map<unsigned int, unsigned int>> parent,
	shared_ptr<map<unsigned int, set<unsigned int>>> parentT,
	shared_ptr<map<unsigned int, set<unsigned int>>> children,
	shared_ptr<map<unsigned int, set<unsigned int>>> childrenT,
	shared_ptr<map<unsigned int, set<unsigned int>>> nextN,
	shared_ptr<map<unsigned int, set<unsigned int>>> nextT,
	shared_ptr<map<unsigned int, set<unsigned int>>> prevN,
	shared_ptr<map<unsigned int, set<unsigned int>>> prevT,
	shared_ptr<map<unsigned int, set<string>>> pattern,
	shared_ptr<map<unsigned int, string>> fullPattern,
	shared_ptr<vector<NODE>> callNodes,
	shared_ptr<map<unsigned int, set<unsigned int>>> affecting,
	shared_ptr<map<unsigned int, set<unsigned int>>> affectingT,
	shared_ptr<map<unsigned int, set<unsigned int>>> affected,
	shared_ptr<map<unsigned int, set<unsigned int>>> affectedT) :
astTree(astTree),
procedures(procedures),
whiles(whiles),
ifs(ifs),
assigments(assigments),
constants(constants),
variables(variables),
programLines(programLines),
callers(callers),
callersT(callersT),
callees(callees),
calleesT(calleesT),
modifies(modifies),
modified(modified),
uses(uses),
used(used),
parent(parent),
parentT(parentT),
children(children),
childrenT(childrenT),
nextN(nextN),
nextT(nextT),
prevN(prevN),
prevT(prevT),
pattern(pattern),
fullPattern(fullPattern),
callNodes(callNodes),
affecting(affecting),
affectingT(affectingT),
affected(affected),
affectedT(affectedT)
{
}

AST::~AST()
{
}

NODE AST::getProcedureByName(string procName)
{
	auto result = find_if(procedures->begin(), procedures->end(), [procName](NODE n) { return !procName.compare((*n)->getParam(Name)); });
	if (result == procedures->end())
	{
		throw exception();
	}
	return *result;
}

NODE AST::getStmtByLineNumber(int lineNumber)
{
	int idx = lineNumber - 1;
	if (programLines->size() <= idx)
	{
		throw exception();
	}
	return (*programLines)[idx];
}

int AST::getLineNumber(NODE n)
{
	return (*n)->lineNumber;
}

NODETYPE AST::getNodeType(NODE n)
{
	return (*n)->nodeType;
}

bool AST::checkFollows(NODE s1, NODE s2, bool transient)
{
	tree<shared_ptr<Node>>::sibling_iterator s = s1;
	if (transient)
	{
		auto end = astTree->end(astTree->parent(s1));
		while (s != end)
		{
			if (++s == s2)
			{
				return true;
			}
		}
		return false;
	}
	else
	{
		return ++s == s2;
	}
}

nodeCollection AST::getFollowing(NODE s1, bool transient)
{
	nodeCollection following;
	tree<shared_ptr<Node>>::sibling_iterator s = s1;
	auto end = astTree->end(astTree->parent(s1));
	if (transient)
	{
		while (++s != end)
		{
			following.push_back(s);
		}
	}
	else
	{
		if (++s != end)
		{
			following.push_back(s);
		}
	}
	return following;
}

nodeCollection AST::getFollowed(NODE s2, bool transient)
{
	nodeCollection followed;
	tree<shared_ptr<Node>>::sibling_iterator end = s2;
	auto it = astTree->begin(astTree->parent(s2));
	while (it != end)
	{
		if (transient)
		{
			followed.push_back(it++);
		}
		else
		{
			auto prev = it;
			if (++it == end)
			{
				followed.push_back(prev);
			}
		}
	}
	return followed;
}

bool AST::checkParent(NODE s1, NODE s2, bool transient)
{
	return transient ? (*parentT)[(*s2)->id].find((*s1)->id) != (*parentT)[(*s2)->id].end() : (*parent)[(*s2)->id] == (*s1)->id;
}

nodeCollection AST::getParent(NODE s1, bool transient)
{
	nodeCollection parents;
	if (!transient)
	{
		auto p = Node::getNodeById((*parent)[(*s1)->id]);
		if (p != NULL)
		{
			parents.push_back(p);
		}
	}
	else
	{
		for (auto it = (*parentT)[(*s1)->id].begin(); it != (*parentT)[(*s1)->id].end(); ++it)
		{
			auto p = Node::getNodeById(*it);
			if (p != NULL)
			{
				parents.push_back(p);
			}
		}
	}
	return parents;
}

nodeCollection AST::getChildren(NODE s2, bool transient)
{
	nodeCollection chlidren;
	auto rel = transient ? childrenT : children;
	for (auto it = (*rel)[(*s2)->id].begin(); it != (*rel)[(*s2)->id].end(); ++it)
	{
		chlidren.push_back(Node::getNodeById(*it));
	}
	return chlidren;
}

bool AST::checkUses(NODE n, string var)
{
	auto vars = (*uses)[(*n)->id];
	return vars.find(var) != vars.end();
}

variableCollection AST::getUsed(NODE n)
{
	auto vars = (*uses)[(*n)->id];
	return variableCollection(vars.begin(), vars.end());
}

nodeCollection AST::getUsing(string var)
{
	auto nodes = (*used)[var];
	return createNodeCollection(nodes.begin(), nodes.end());
}

bool AST::checkModifies(NODE n, string var)
{
	auto vars = (*modifies)[(*n)->id];
	return vars.find(var) != vars.end();
}

variableCollection AST::getModified(NODE n)
{
	auto vars = (*modifies)[(*n)->id];
	return variableCollection(vars.begin(), vars.end());
}

nodeCollection AST::getModifying(string var)
{
	auto nodes = (*modified)[var];
	return createNodeCollection(nodes.begin(), nodes.end());
}

bool AST::checkCalls(NODE p1, NODE p2, bool transient)
{
	auto rel = transient ? callersT : callers;
	return (*rel)[(*p1)->id].find((*p2)->id) != (*rel)[(*p1)->id].end();
}

nodeCollection AST::getCallees(NODE p1, bool transient)
{
	auto rel = transient ? callersT : callers;
	return createNodeCollection((*rel)[(*p1)->id].begin(), (*rel)[(*p1)->id].end());
}

nodeCollection AST::getCallers(NODE p2, bool transient)
{
	auto rel = transient ? calleesT : callees;
	return createNodeCollection((*rel)[(*p2)->id].begin(), (*rel)[(*p2)->id].end());
}

bool AST::checkNext(NODE s1, NODE s2, bool transient)
{
	auto rel = transient ? nextT : nextN;
	return (*rel)[(*s1)->id].find((*s2)->id) != (*rel)[(*s1)->id].end();
}

nodeCollection AST::getNext(NODE s1, bool transient)
{
	auto rel = transient ? nextT : nextN;
	return createNodeCollection((*rel)[(*s1)->id].begin(), (*rel)[(*s1)->id].end());
}

nodeCollection AST::getPrev(NODE s2, bool transient)
{
	auto rel = transient ? prevT : prevN;
	return createNodeCollection((*rel)[(*s2)->id].begin(), (*rel)[(*s2)->id].end());
}

bool AST::checkAffects(NODE s1, NODE s2, bool transient)
{
	auto rel = transient ? affectingT : affecting;
	return (*rel)[(*s1)->id].find((*s2)->id) != (*rel)[(*s1)->id].end();
}

nodeCollection AST::getAffected(NODE s1, bool transient)
{
	auto rel = transient ? affectingT : affecting;
	return createNodeCollection((*rel)[(*s1)->id].begin(), (*rel)[(*s1)->id].end());
}

nodeCollection AST::getAffecting(NODE s2, bool transient)
{
	auto rel = transient ? affectedT : affected;
	return createNodeCollection((*rel)[(*s2)->id].begin(), (*rel)[(*s2)->id].end());
}

nodeCollection AST::getPattern(string var, string expr)
{
	nodeCollection result;
	if (var.compare("_"))
	{
		var = var.substr(1, var.length() - 2);
	}
	if (!expr.compare("_"))
	{
		if (!var.compare("_"))
		{
			return nodeCollection(*assigments);
		}
		for (auto it = assigments->begin(); it != assigments->end(); ++it)
		{
			if (!(*astTree->begin(*it))->getParam(Name).compare(var))
			{
				result.push_back(*it);
			}
		}
		return result;
	}
	string p = "";
	if (expr[0] == '_')
	{
		p = expr.substr(2, expr.length() - 4);
		for (auto patternIt = pattern->begin(); patternIt != pattern->end(); ++patternIt)
		{
			NODE node = Node::getNodeById(patternIt->first);
			if (!(*astTree->begin(node))->getParam(Name).compare(var) || !var.compare("_"))
			{
				for (auto it = patternIt->second.begin(); it != patternIt->second.end(); ++it)
				{
					if (!it->compare(p))
					{
						result.push_back(node);
						break;
					}
				}
			}
		}
	}
	else
	{
		p = expr.substr(1, expr.length() - 2);
		for (auto patternIt = fullPattern->begin(); patternIt != fullPattern->end(); ++patternIt)
		{
			NODE node = Node::getNodeById(patternIt->first);
			if (!patternIt->second.compare(p) && (!(*astTree->begin(node))->getParam(Name).compare(var) || !var.compare("_")))
			{
				result.push_back(node);
			}
		}
	}
	return result;
}

nodeCollection AST::createNodeCollection(set<unsigned int>::iterator begin, set<unsigned int>::iterator end)
{
	nodeCollection nodes;
	for (auto it = begin; it != end; ++it)
	{
		nodes.push_back(Node::getNodeById(*it));
	}
	return nodes;
}
