#pragma once
#include "pkb.h"
#include <vector>
#include <map>
#include <set>
#include "Node.h"
#include <iostream>

using namespace std;
typedef vector<NODE> nodeCollection;
typedef vector<string> variableCollection;

class AST
{
public:
	AST(
		shared_ptr<tree<shared_ptr<Node>>> astTree,
		shared_ptr<vector<NODE>> procedures,
		shared_ptr<vector<NODE>> whiles,
		shared_ptr<vector<NODE>> ifs,
		shared_ptr<vector<NODE>> assigments,
		shared_ptr<set<string>> constants,
		shared_ptr<set<string>> variables,
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
		shared_ptr<map<unsigned int, set<unsigned int>>> affectedT);
	~AST();

	NODE getProcedureByName(string procName);
	NODE getStmtByLineNumber(int lineNumber);
	int getLineNumber(NODE n);
	NODETYPE getNodeType(NODE n);

	bool checkFollows(NODE s1, NODE s2, bool transient = false);
	nodeCollection getFollowing(NODE s1, bool transient = false);
	nodeCollection getFollowed(NODE s2, bool transient = false);

	bool checkParent(NODE s1, NODE s2, bool transient = false);
	nodeCollection getParent(NODE s1, bool transient = false);
	nodeCollection getChildren(NODE s2, bool transient = false);

	bool checkUses(NODE n, string var);
	variableCollection getUsed(NODE n);
	nodeCollection getUsing(string var);

	bool checkModifies(NODE n, string var);
	variableCollection getModified(NODE n);
	nodeCollection getModifying(string var);

	bool checkCalls(NODE p1, NODE p2, bool transient = false);
	nodeCollection getCallees(NODE p1, bool transient = false);
	nodeCollection getCallers(NODE p2, bool transient = false);

	bool checkNext(NODE s1, NODE s2, bool transient = false);
	nodeCollection getNext(NODE s1, bool transient = false);
	nodeCollection getPrev(NODE s2, bool transient = false);

	bool checkAffects(NODE s1, NODE s2, bool transient = false);
	nodeCollection getAffected(NODE s1, bool transient = false);
	nodeCollection getAffecting(NODE s2, bool transient = false);

	nodeCollection getPattern(string var, string expr);

	shared_ptr<vector<NODE>> procedures;
	shared_ptr<vector<NODE>> whiles;
	shared_ptr<vector<NODE>> ifs;
	shared_ptr<vector<NODE>> assigments;
	shared_ptr<set<string>> variables;
	shared_ptr<set<string>> constants;
	shared_ptr<vector<NODE>> callNodes;
	shared_ptr<vector<NODE>> programLines;


	shared_ptr<tree<shared_ptr<Node>>> astTree;
private:
	shared_ptr<map<unsigned int, set<unsigned int>>> callers;
	shared_ptr<map<unsigned int, set<unsigned int>>> callersT;
	shared_ptr<map<unsigned int, set<unsigned int>>> callees;
	shared_ptr<map<unsigned int, set<unsigned int>>> calleesT;
	shared_ptr<map<unsigned int, set<string>>> modifies;
	shared_ptr<map<string, set<unsigned int>>> modified;
	shared_ptr<map<unsigned int, set<string>>> uses;
	shared_ptr<map<string, set<unsigned int>>> used;
	shared_ptr<map<unsigned int, unsigned int>> parent;
	shared_ptr<map<unsigned int, set<unsigned int>>> parentT;
	shared_ptr<map<unsigned int, set<unsigned int>>> children;
	shared_ptr<map<unsigned int, set<unsigned int>>> childrenT;
	shared_ptr<map<unsigned int, set<unsigned int>>> nextN;
	shared_ptr<map<unsigned int, set<unsigned int>>> nextT;
	shared_ptr<map<unsigned int, set<unsigned int>>> prevN;
	shared_ptr<map<unsigned int, set<unsigned int>>> prevT;
	shared_ptr<map<unsigned int, set<string>>> pattern;
	shared_ptr<map<unsigned int, string>> fullPattern;
	shared_ptr<map<unsigned int, set<unsigned int>>> affecting;
	shared_ptr<map<unsigned int, set<unsigned int>>> affectingT;
	shared_ptr<map<unsigned int, set<unsigned int>>> affected;
	shared_ptr<map<unsigned int, set<unsigned int>>> affectedT;
	nodeCollection createNodeCollection(set<unsigned int>::iterator begin, set<unsigned int>::iterator end);
};

