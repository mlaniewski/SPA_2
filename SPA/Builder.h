#pragma once
#include "pkb.h"
#include "Node.h"
#include "AST.h"
#include "tree.hh"
#include <string>
#include <tuple>
#include <vector>
#include <map>
#include <set>
#include <iostream>
#include <fstream>

using namespace std;

class Builder
{

public:
	Builder();
	~Builder();

	/// <summary>Tworzenie nowego wêz³a</summary>
	NODE createNode(NODETYPE nodeType);

	/// <summary>Dodanie parametrów dla wêz³a</summary>
	void addNodeParameter(NODE node, NODEPARAMTYPE nodeParamType, string val);

	/// <summary>Dodanie dziecka dla danego wêz³a</summary>
	void addChild(NODE parent, NODE child);

	/// <summary>Zwrócenie budowanego drzewa AST</summary>
	shared_ptr<AST> getAST();

private:
	shared_ptr<AST> ast;
	shared_ptr<tree<shared_ptr<Node>>> astTree;
	shared_ptr<vector<NODE>> procedures;
	shared_ptr<vector<NODE>> whiles;
	shared_ptr<vector<NODE>> ifs;
	shared_ptr<vector<NODE>> assigments;
	shared_ptr<vector<NODE>> callNodes;
	shared_ptr<set<string>> variables;
	shared_ptr<set<string>> constants;
	shared_ptr<vector<NODE>> programLines;
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
	shared_ptr<map<unsigned int, set<unsigned int>>> affecting;
	shared_ptr<map<unsigned int, set<unsigned int>>> affectingT;
	shared_ptr<map<unsigned int, set<unsigned int>>> affected;
	shared_ptr<map<unsigned int, set<unsigned int>>> affectedT;
	shared_ptr<map<unsigned int, set<string>>> pattern;
	shared_ptr<map<unsigned int, string>> fullPattern;


	vector<NODE> varNodes;
	vector<unsigned int> tempUiVector;
	vector<NODE> constantNodes;

	void initializeCallMaps();
	void initializeModifiesAndUsesMaps();
	void initializeUsesAssignment(NODE assignment, NODE child);
	void initializeModifiesAndUsesContainers(NODE container);
	void initializeModifiesAndUsesCalls(NODE call, set<unsigned int> &callsToInit);
	void initializeParentMap(NODE node);
	void initializeNextStmtLst(NODE stmtLst, NODE next = NULL);
	void initializeNext(NODE prev, NODE next);
	void initializeTransientRelation(shared_ptr<map<unsigned int, set<unsigned int>>> rel);
	void initializeInvertedVariableRelation(shared_ptr<map<unsigned int, set<string>>> rel, shared_ptr<map<string, set<unsigned int>>> inv);
	void initializePattern();
	string initializePatternNode(NODE exp, unsigned int id);
	void addLineNumbers();

	bool checkAffects(NODE a, NODE b);
};