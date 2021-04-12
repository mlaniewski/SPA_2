#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "Predicate.h"
#include "Selector.h"
#include "Closure.h"
#include "Pattern.h"
#include "With.h"
#include <list>
#include "ClosureResult.h"
#include "AST.h"
#include <ctime>

using namespace std;

class Solver
{
private:
	ClosureResult getClosureResult(Closure closure);
	ClosureResult getPatternResult(Pattern pattern);
	ClosureResult getWithResult(With with);

	std::shared_ptr<AST> ast;
	vector<string> valueOfPred;
	map<string, int> indexOfPred;
	vector<ClosureResult> closureResults;
	vector<vector<string>> resultTable;
	string * tmpResult;
	map<int, vector<ClosureResult>> dependentClosures;
	map<int, set<string>> possibleValues;
	void findResult(int pred);
	void findPossibleValues();
	void updatePossibleValues(int pred, set<string> & vals, bool updated);

	string nodeToString(NODE node);
	vector<string> getAllVariables();
	vector<NODE> getAllValues(string predType);
	bool matchType(NODETYPE nodeType, string predType);
	nodeCollection filterNodesByType(nodeCollection nodes, string type, string type2 = "");
	set<string> getAllPropertyValues(string propName);

	bool boolResult;
	time_t beginTime;

	bool timeout();
public:
	Solver(
		std::list<std::string>& results,
		Selector selector,
		vector<Closure> closureTable,
		vector<Pattern> patternTable,
		vector<Predicate> predTable,
		vector<With> withTable,
		std::shared_ptr<AST> astw);
	~Solver();
	void evaluate();

	list<std::string> &results;
	Selector selector;
	vector<Closure> closureTable;
	vector<Pattern> patternTable;
	vector<Predicate> predTable;
	vector<With> withTable;
};

struct SolverException : public std::exception {
	string msg;
	SolverException(string msg) : msg(msg)
	{
	}
	const char * what() const throw () {
		return msg.c_str();
	}
};