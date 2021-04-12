#include "Solver.h"
#include "AST.h"
#include <iostream>
#include <string> 
#include <initializer_list>

using namespace std;

bool Solver::timeout()
{
	return time(NULL) >= beginTime + 15;
}

Solver::Solver(
	std::list<std::string>& results,
	Selector selector,
	vector<Closure> closureTable,
	vector<Pattern> patternTable,
	vector<Predicate> predTable,
	vector<With> withTable,
	std::shared_ptr<AST> astw) :
	results(results),
	selector(selector),
	closureTable(closureTable),
	patternTable(patternTable),
	predTable(predTable),
	withTable(withTable),
	ast(astw)
{
	beginTime = time(NULL);
	boolResult = false;
	tmpResult = new string[predTable.size()];
	for (size_t i = 0; i < predTable.size(); i++)
	{
		valueOfPred.push_back(predTable[i].getValue());
		indexOfPred[predTable[i].getValue()] = i;
	}
	for (size_t i = 0; i < closureTable.size(); i++)
	{
		auto result = getClosureResult(closureTable[i]);
		if (!result.resultType.compare("MAP") && result.pq.empty())
		{
			continue;
		}
		closureResults.push_back(result);
	}
	for (size_t i = 0; i < patternTable.size(); i++)
	{
		closureResults.push_back(getPatternResult(patternTable[i]));
	}
	for (rsize_t i = 0; i < withTable.size(); i++)
	{
		closureResults.push_back(getWithResult(withTable[i]));
	}
	for (auto cr = closureResults.begin(); cr != closureResults.end(); ++cr) {
		if (!cr->resultType.compare("MAP"))
		{
			dependentClosures[max(indexOfPred[cr->p], indexOfPred[cr->q])].push_back(*cr);
		}
	}
}

Solver::~Solver()
{
}

string Solver::nodeToString(NODE node)
{
	switch ((*node)->nodeType)
	{
	case Procedure:
	case Variable:
	case Constant:
		return (*node)->getParam(Name);
	default:
		return to_string(static_cast<long long>(ast->getLineNumber(node)));
	}
}

vector<string> Solver::getAllVariables()
{
	vector<string> result;
	for (auto n = ast->variables->begin(); n != ast->variables->end(); ++n)
	{
		result.push_back(*n);
	}
	return result;
}

vector<NODE> Solver::getAllValues(string predType)
{
	vector<NODE> result;
	vector<shared_ptr<vector<NODE>>> nodeTypes;
	if (!predType.compare("procedure"))
	{
		nodeTypes.push_back(ast->procedures);
	}
	else if (!predType.compare("statement"))
	{
		nodeTypes.push_back(ast->whiles);
		nodeTypes.push_back(ast->ifs);
		nodeTypes.push_back(ast->assigments);
		nodeTypes.push_back(ast->callNodes);
	}
	else if (!predType.compare("while"))
	{
		nodeTypes.push_back(ast->whiles);
	}
	else if (!predType.compare("if"))
	{
		nodeTypes.push_back(ast->ifs);
	}
	else if (!predType.compare("call"))
	{
		nodeTypes.push_back(ast->callNodes);
	}
	else if (!predType.compare("assign"))
	{
		nodeTypes.push_back(ast->assigments);
	}
	for (auto nt = nodeTypes.begin(); nt != nodeTypes.end(); ++nt)
	{
		for (auto n = (*nt)->begin(); n != (*nt)->end(); ++n)
		{
			result.push_back(*n);
		}
	}
	return result;
}

bool Solver::matchType(NODETYPE nodeType, string predType)
{
	if (!predType.compare("procedure"))
	{
		return nodeType == Procedure;
	}
	else if (!predType.compare("statement"))
	{
		return nodeType == Call
			|| nodeType == While
			|| nodeType == If
			|| nodeType == Assign;
	}
	else if (!predType.compare("assign"))
	{
		return nodeType == Assign;
	}
	else if (!predType.compare("while"))
	{
		return nodeType == While;
	}
	else if (!predType.compare("if"))
	{
		return nodeType == If;
	}
	else if (!predType.compare("var"))
	{
		return nodeType == Variable;
	}
	else if (!predType.compare("call"))
	{
		return nodeType == Call;
	}
	else if (!predType.compare("const"))
	{
		return nodeType == Constant;
	}
	return false;
}

nodeCollection Solver::filterNodesByType(nodeCollection nodes, string type, string type2)
{
	nodeCollection filteredNodes;
	for (auto n = nodes.begin(); n != nodes.end(); ++n)
	{
		bool m1 = matchType((**n)->nodeType, type);
		bool m2 = matchType((**n)->nodeType, type2);
		bool e = type2.empty();
		if (e ? m1 : (m1 || m2))
		{
			filteredNodes.push_back(*n);
		}
	}
	return filteredNodes;
}

set<string> Solver::getAllPropertyValues(string propName)
{
	set<string> result;
	if (!propName.compare("stmt"))
	{
		size_t lines = ast->programLines->size();
		for (size_t i = 1; i <= lines; ++i)
		{
			result.insert(to_string(static_cast<long long>(i)));
		}
	}
	else if (!propName.compare("value"))
	{
		result.insert(ast->constants->begin(), ast->constants->end());
	}
	else if (!propName.compare("procName"))
	{
		auto procedures = ast->procedures;
		for (auto p = procedures->begin(); p != procedures->end(); ++p)
		{
			result.insert((**p)->getParam(Name));
		}
	}
	else if (!propName.compare("varName"))
	{
		result.insert(ast->variables->begin(), ast->variables->end());
	}
	return result;
}

ClosureResult Solver::getClosureResult(Closure closure)
{
	ClosureResult closureResult;
	bool transient = closure.type[closure.type.length() - 1] == '*';
	string relation = transient ? closure.type.substr(0, closure.type.length() - 1) : closure.type;
	Predicate p1, p2;
	for (size_t i = 0; i < predTable.size(); i++)
	{
		if (closure.lhs == predTable[i].getValue())
		{
			p1 = predTable[i];
		}
		if (closure.rhs == predTable[i].getValue())
		{
			p2 = predTable[i];
		}
	}
	// sprawdzam czy lhs i rhs sa numerami linii
	int lhsLineNum = 0, rhsLineNum = 0;
	try
	{
		lhsLineNum = stoi(closure.lhs);
	}
	catch (...) {}
	try
	{
		rhsLineNum = stoi(closure.rhs);
	}
	catch (...) {}
	// sprawdzam czy lhs i rhs sa nazwami
	string lhsName = "", rhsName = "";
	if (closure.lhs[0] == '"')
	{
		lhsName = closure.lhs.substr(1, closure.lhs.length() - 2);
	}
	if (closure.rhs[0] == '"')
	{
		rhsName = closure.rhs.substr(1, closure.rhs.length() - 2);
	}
	if (p1.getType().empty() && p2.getType().empty()) // 0 predykatow
	{
		closureResult.resultType = "BOOL";
		if (!relation.compare("next"))
		{
			if (lhsLineNum && rhsLineNum)
			{
				closureResult.boolResult = ast->checkNext(ast->getStmtByLineNumber(lhsLineNum), ast->getStmtByLineNumber(rhsLineNum), transient);
			}
			else if (lhsLineNum)
			{
				closureResult.boolResult = !ast->getNext(ast->getStmtByLineNumber(lhsLineNum), transient).empty();
			}
			else if (rhsLineNum)
			{
				closureResult.boolResult = !ast->getPrev(ast->getStmtByLineNumber(rhsLineNum), transient).empty();
			}
			else
			{
				auto nodes = getAllValues("statement");
				closureResult.boolResult = false;
				for (auto n = nodes.begin(); n != nodes.end(); ++n)
				{
					if (!ast->getNext(*n, transient).empty())
					{
						closureResult.boolResult = true;
						break;
					}
				}
			}
		}
		if (!relation.compare("follows"))
		{
			if (lhsLineNum && rhsLineNum)
			{
				closureResult.boolResult = ast->checkFollows(ast->getStmtByLineNumber(lhsLineNum), ast->getStmtByLineNumber(rhsLineNum), transient);
			}
			else if (lhsLineNum)
			{
				closureResult.boolResult = !ast->getFollowing(ast->getStmtByLineNumber(lhsLineNum), transient).empty();
			}
			else if (rhsLineNum)
			{
				closureResult.boolResult = !ast->getFollowed(ast->getStmtByLineNumber(rhsLineNum), transient).empty();
			}
			else
			{
				auto nodes = getAllValues("statement");
				closureResult.boolResult = false;
				for (auto n = nodes.begin(); n != nodes.end(); ++n)
				{
					if (!ast->getFollowing(*n, transient).empty())
					{
						closureResult.boolResult = true;
						break;
					}
				}
			}
		}
		if (!relation.compare("parent"))
		{
			if (lhsLineNum && rhsLineNum)
			{
				closureResult.boolResult = ast->checkParent(ast->getStmtByLineNumber(lhsLineNum), ast->getStmtByLineNumber(rhsLineNum), transient);
			}
			else if (lhsLineNum)
			{
				closureResult.boolResult = !ast->getChildren(ast->getStmtByLineNumber(lhsLineNum), transient).empty();
			}
			else if (rhsLineNum)
			{
				closureResult.boolResult = !ast->getParent(ast->getStmtByLineNumber(rhsLineNum), transient).empty();
			}
			else
			{
				auto nodes = getAllValues("statement");
				closureResult.boolResult = false;
				for (auto n = nodes.begin(); n != nodes.end(); ++n)
				{
					if (!ast->getParent(*n, transient).empty())
					{
						closureResult.boolResult = true;
						break;
					}
				}
			}
		}
		if (!relation.compare("calls"))
		{
			if (!lhsName.empty() && !rhsName.empty())
			{
				closureResult.boolResult = ast->checkCalls(ast->getProcedureByName(lhsName), ast->getProcedureByName(rhsName), transient);
			}
			else if (!lhsName.empty())
			{
				closureResult.boolResult = !ast->getCallees(ast->getProcedureByName(lhsName), transient).empty();
			}
			else if (!rhsName.empty())
			{
				closureResult.boolResult = !ast->getCallers(ast->getProcedureByName(rhsName), transient).empty();
			}
			else
			{
				auto nodes = getAllValues("procedure");
				closureResult.boolResult = false;
				for (auto n = nodes.begin(); n != nodes.end(); ++n)
				{
					if (!ast->getCallees(*n, transient).empty())
					{
						closureResult.boolResult = true;
						break;
					}
				}
			}
		}
		if (!relation.compare("uses"))
		{
			if ((lhsLineNum || !lhsName.empty()) && !rhsName.empty())
			{
				NODE n = lhsLineNum ? ast->getStmtByLineNumber(lhsLineNum) : ast->getProcedureByName(lhsName);
				closureResult.boolResult = ast->checkUses(n, rhsName);
			}
			else if (lhsLineNum || !lhsName.empty())
			{
				NODE n = lhsLineNum ? ast->getStmtByLineNumber(lhsLineNum) : ast->getProcedureByName(lhsName);
				closureResult.boolResult = !ast->getUsed(n).empty();
			}
			else if (!rhsName.empty())
			{
				closureResult.boolResult = !ast->getUsing(rhsName).empty();
			}
			else
			{
				auto vars = getAllVariables();
				closureResult.boolResult = false;
				for (auto n = vars.begin(); n != vars.end(); ++n)
				{
					if (!ast->getUsing(*n).empty())
					{
						closureResult.boolResult = true;
						break;
					}
				}
			}
		}
		if (!relation.compare("modifies"))
		{
			if ((lhsLineNum || !lhsName.empty()) && !rhsName.empty())
			{
				NODE n = lhsLineNum ? ast->getStmtByLineNumber(lhsLineNum) : ast->getProcedureByName(lhsName);
				closureResult.boolResult = ast->checkModifies(n, rhsName);
			}
			else if (lhsLineNum || !lhsName.empty())
			{
				NODE n = lhsLineNum ? ast->getStmtByLineNumber(lhsLineNum) : ast->getProcedureByName(lhsName);
				closureResult.boolResult = !ast->getModified(n).empty();
			}
			else if (!rhsName.empty())
			{
				closureResult.boolResult = !ast->getModifying(rhsName).empty();
			}
			else
			{
				auto vars = getAllVariables();
				closureResult.boolResult = false;
				for (auto n = vars.begin(); n != vars.end(); ++n)
				{
					if (!ast->getModifying(*n).empty())
					{
						closureResult.boolResult = true;
						break;
					}
				}
			}
		}
		if (!relation.compare("affects"))
		{
			if (lhsLineNum && rhsLineNum)
			{
				closureResult.boolResult = ast->checkAffects(ast->getStmtByLineNumber(lhsLineNum), ast->getStmtByLineNumber(rhsLineNum), transient);
			}
			else if (lhsLineNum)
			{
				closureResult.boolResult = !ast->getAffected(ast->getStmtByLineNumber(lhsLineNum), transient).empty();
			}
			else if (rhsLineNum)
			{
				closureResult.boolResult = !ast->getAffecting(ast->getStmtByLineNumber(rhsLineNum), transient).empty();
			}
			else
			{
				auto nodes = getAllValues("assign");
				closureResult.boolResult = false;
				for (auto n = nodes.begin(); n != nodes.end(); ++n)
				{
					if (!ast->getAffected(*n, transient).empty())
					{
						closureResult.boolResult = true;
						break;
					}
				}
			}
		}
		// .....
	}
	else if (p2.getType().empty()) // predykat z lewej
	{
		closureResult.resultType = "SET";
		closureResult.p = p1.getValue();
		if (!relation.compare("next"))
		{
			if (rhsLineNum) // numer linii
			{
				auto results = filterNodesByType(ast->getPrev(ast->getStmtByLineNumber(rhsLineNum), transient), p1.getType());
				for (auto r = results.begin(); r != results.end(); ++r)
				{
					closureResult.vals.insert(nodeToString(*r));
				}
			}
			else // _
			{
				auto allVals = getAllValues("statement");
				for (auto val = allVals.begin(); val != allVals.end(); ++val)
				{
					auto results = filterNodesByType(ast->getPrev(*val, transient), p1.getType());
					for (auto r = results.begin(); r != results.end(); ++r)
					{
						closureResult.vals.insert(nodeToString(*r));
					}
				}
			}
		}
		if (!relation.compare("follows"))
		{
			if (rhsLineNum) // numer linii
			{
				auto results = filterNodesByType(ast->getFollowed(ast->getStmtByLineNumber(rhsLineNum), transient), p1.getType());
				for (auto r = results.begin(); r != results.end(); ++r)
				{
					closureResult.vals.insert(nodeToString(*r));
				}
			}
			else // _
			{
				auto allVals = getAllValues("statement");
				for (auto val = allVals.begin(); val != allVals.end(); ++val)
				{
					auto results = filterNodesByType(ast->getFollowed(*val, transient), p1.getType());
					for (auto r = results.begin(); r != results.end(); ++r)
					{
						closureResult.vals.insert(nodeToString(*r));
					}
				}
			}
		}
		if (!relation.compare("parent"))
		{
			if (rhsLineNum) // numer linii
			{
				auto results = filterNodesByType(ast->getParent(ast->getStmtByLineNumber(rhsLineNum), transient), p1.getType());
				for (auto r = results.begin(); r != results.end(); ++r)
				{
					closureResult.vals.insert(nodeToString(*r));
				}
			}
			else // _
			{
				auto allVals = getAllValues("statement");
				for (auto val = allVals.begin(); val != allVals.end(); ++val)
				{
					auto results = filterNodesByType(ast->getParent(*val, transient), p1.getType());
					for (auto r = results.begin(); r != results.end(); ++r)
					{
						closureResult.vals.insert(nodeToString(*r));
					}
				}
			}
		}
		if (!relation.compare("calls"))
		{
			if (!rhsName.empty()) // nazwa procedury
			{
				auto results = filterNodesByType(ast->getCallers(ast->getProcedureByName(rhsName), transient), p1.getType());
				for (auto r = results.begin(); r != results.end(); ++r)
				{
					closureResult.vals.insert(nodeToString(*r));
				}
			}
			else // _
			{
				auto allVals = getAllValues("procedure");
				for (auto val = allVals.begin(); val != allVals.end(); ++val)
				{
					auto results = filterNodesByType(ast->getCallers(*val, transient), p1.getType());
					for (auto r = results.begin(); r != results.end(); ++r)
					{
						closureResult.vals.insert(nodeToString(*r));
					}
				}
			}
		}
		if (!relation.compare("uses"))
		{
			if (!rhsName.empty()) // zmienna
			{
				auto results = filterNodesByType(ast->getUsing(rhsName), p1.getType());
				for (auto r = results.begin(); r != results.end(); ++r)
				{
					closureResult.vals.insert(nodeToString(*r));
				}
			}
			else // _
			{
				auto allVars = getAllVariables();
				for (auto val = allVars.begin(); val != allVars.end(); ++val)
				{
					auto results = filterNodesByType(ast->getUsing(*val), p1.getType());
					for (auto r = results.begin(); r != results.end(); ++r)
					{
						closureResult.vals.insert(nodeToString(*r));
					}
				}
			}
		}
		if (!relation.compare("modifies"))
		{
			if (!rhsName.empty()) // zmienna
			{
				auto results = filterNodesByType(ast->getModifying(rhsName), p1.getType());
				for (auto r = results.begin(); r != results.end(); ++r)
				{
					closureResult.vals.insert(nodeToString(*r));
				}
			}
			else // _
			{
				auto allVars = getAllVariables();
				for (auto val = allVars.begin(); val != allVars.end(); ++val)
				{
					auto results = filterNodesByType(ast->getModifying(*val), p1.getType());
					for (auto r = results.begin(); r != results.end(); ++r)
					{
						closureResult.vals.insert(nodeToString(*r));
					}
				}
			}
		}
		if (!relation.compare("affects"))
		{
			if (rhsLineNum) // numer linii
			{
				auto test = ast->getAffecting(ast->getStmtByLineNumber(rhsLineNum));
				auto results = filterNodesByType(ast->getAffecting(ast->getStmtByLineNumber(rhsLineNum), transient), p1.getType());
				for (auto r = results.begin(); r != results.end(); ++r)
				{
					closureResult.vals.insert(nodeToString(*r));
				}
			}
			else // _
			{
				auto allVals = getAllValues("assign");
				for (auto val = allVals.begin(); val != allVals.end(); ++val)
				{
					auto results = filterNodesByType(ast->getAffecting(*val, transient), p1.getType());
					for (auto r = results.begin(); r != results.end(); ++r)
					{
						closureResult.vals.insert(nodeToString(*r));
					}
				}
			}
		}
		// .......
	}
	else if (p1.getType().empty()) // predykat z prawej
	{
		closureResult.resultType = "SET";
		closureResult.p = p2.getValue();
		if (!relation.compare("next"))
		{
			if (lhsLineNum) // numer linii
			{
				auto results = filterNodesByType(ast->getNext(ast->getStmtByLineNumber(lhsLineNum), transient), p2.getType());
				for (auto r = results.begin(); r != results.end(); ++r)
				{
					closureResult.vals.insert(nodeToString(*r));
				}
			}
			else // _
			{
				auto allVals = getAllValues("statement");
				for (auto val = allVals.begin(); val != allVals.end(); ++val)
				{
					auto results = filterNodesByType(ast->getNext(*val, transient), p2.getType());
					for (auto r = results.begin(); r != results.end(); ++r)
					{
						closureResult.vals.insert(nodeToString(*r));
					}
				}
			}
		}
		if (!relation.compare("follows"))
		{
			if (lhsLineNum) // numer linii
			{
				auto results = filterNodesByType(ast->getFollowing(ast->getStmtByLineNumber(lhsLineNum), transient), p2.getType());
				for (auto r = results.begin(); r != results.end(); ++r)
				{
					closureResult.vals.insert(nodeToString(*r));
				}
			}
			else // _
			{
				auto allVals = getAllValues("statement");
				for (auto val = allVals.begin(); val != allVals.end(); ++val)
				{
					auto results = filterNodesByType(ast->getFollowing(*val, transient), p2.getType());
					for (auto r = results.begin(); r != results.end(); ++r)
					{
						closureResult.vals.insert(nodeToString(*r));
					}
				}
			}
		}
		if (!relation.compare("parent"))
		{
			if (lhsLineNum) // numer linii
			{
				auto results = filterNodesByType(ast->getChildren(ast->getStmtByLineNumber(lhsLineNum), transient), p2.getType());
				for (auto r = results.begin(); r != results.end(); ++r)
				{
					closureResult.vals.insert(nodeToString(*r));
				}
			}
			else // _
			{
				auto allVals = filterNodesByType(getAllValues("statement"), "if", "while");
				for (auto val = allVals.begin(); val != allVals.end(); ++val)
				{
					auto results = filterNodesByType(ast->getChildren(*val, transient), p2.getType());
					for (auto r = results.begin(); r != results.end(); ++r)
					{
						closureResult.vals.insert(nodeToString(*r));
					}
				}
			}
		}
		if (!relation.compare("calls"))
		{
			if (!lhsName.empty()) // numer linii
			{
				auto results = filterNodesByType(ast->getCallees(ast->getProcedureByName(lhsName), transient), p2.getType());
				for (auto r = results.begin(); r != results.end(); ++r)
				{
					closureResult.vals.insert(nodeToString(*r));
				}
			}
			else // _
			{
				auto allVals = getAllValues("procedure");
				for (auto val = allVals.begin(); val != allVals.end(); ++val)
				{
					auto results = filterNodesByType(ast->getCallees(*val, transient), p2.getType());
					for (auto r = results.begin(); r != results.end(); ++r)
					{
						closureResult.vals.insert(nodeToString(*r));
					}
				}
			}
		}
		if (!relation.compare("uses"))
		{
			if (lhsLineNum || !lhsName.empty()) // stmt lub proc
			{
				NODE n = lhsLineNum ? ast->getStmtByLineNumber(lhsLineNum) : ast->getProcedureByName(lhsName);
				auto results = ast->getUsed(n);
				for (auto r = results.begin(); r != results.end(); ++r)
				{
					closureResult.vals.insert(*r);
				}
			}
			else // _
			{
				auto procNodes = getAllValues("procedure");
				auto allVals = getAllValues("statement");
				allVals.insert(allVals.end(), procNodes.begin(), procNodes.end());
				for (auto val = allVals.begin(); val != allVals.end(); ++val)
				{
					auto results = ast->getUsed(*val);
					for (auto r = results.begin(); r != results.end(); ++r)
					{
						closureResult.vals.insert(*r);
					}
				}
			}
		}
		if (!relation.compare("modifies"))
		{
			if (lhsLineNum || !lhsName.empty()) // stmt lub proc
			{
				NODE n = lhsLineNum ? ast->getStmtByLineNumber(lhsLineNum) : ast->getProcedureByName(lhsName);
				auto results = ast->getModified(n);
				for (auto r = results.begin(); r != results.end(); ++r)
				{
					closureResult.vals.insert(*r);
				}
			}
			else // _
			{
				auto procNodes = getAllValues("procedure");
				auto allVals = getAllValues("statement");
				allVals.insert(allVals.end(), procNodes.begin(), procNodes.end());
				for (auto val = allVals.begin(); val != allVals.end(); ++val)
				{
					auto results = ast->getModified(*val);
					for (auto r = results.begin(); r != results.end(); ++r)
					{
						closureResult.vals.insert(*r);
					}
				}
			}
		}
		if (!relation.compare("affects"))
		{
			if (lhsLineNum) // numer linii
			{
				auto results = filterNodesByType(ast->getAffected(ast->getStmtByLineNumber(lhsLineNum), transient), p2.getType());
				for (auto r = results.begin(); r != results.end(); ++r)
				{
					closureResult.vals.insert(nodeToString(*r));
				}
			}
			else // _
			{
				auto allVals = getAllValues("assign");
				for (auto val = allVals.begin(); val != allVals.end(); ++val)
				{
					auto results = filterNodesByType(ast->getAffected(*val, transient), p2.getType());
					for (auto r = results.begin(); r != results.end(); ++r)
					{
						closureResult.vals.insert(nodeToString(*r));
					}
				}
			}
		}
		// .....
	}
	else // 2 predykaty
	{
		closureResult.resultType = "MAP";
		closureResult.p = p1.getValue();
		closureResult.q = p2.getValue();
		if (!relation.compare("next"))
		{
			auto allPVals = filterNodesByType(getAllValues("statement"), p1.getType());
			for (auto val = allPVals.begin(); val != allPVals.end(); ++val)
			{
				auto pResults = filterNodesByType(ast->getNext(*val, transient), p2.getType());
				for (auto r = pResults.begin(); r != pResults.end(); ++r)
				{
					closureResult.pq[nodeToString(*val)].insert(nodeToString(*r));
				}
			}
			auto allQVals = filterNodesByType(getAllValues("statement"), p2.getType());
			for (auto val = allQVals.begin(); val != allQVals.end(); ++val)
			{
				auto qResults = filterNodesByType(ast->getPrev(*val, transient), p1.getType());
				for (auto r = qResults.begin(); r != qResults.end(); ++r)
				{
					closureResult.qp[nodeToString(*val)].insert(nodeToString(*r));
				}
			}
		}
		if (!relation.compare("follows"))
		{
			auto allPVals = filterNodesByType(getAllValues("statement"), p1.getType());
			for (auto val = allPVals.begin(); val != allPVals.end(); ++val)
			{
				auto pResults = filterNodesByType(ast->getFollowing(*val, transient), p2.getType());
				for (auto r = pResults.begin(); r != pResults.end(); ++r)
				{
					closureResult.pq[nodeToString(*val)].insert(nodeToString(*r));
				}
			}
			auto allQVals = filterNodesByType(getAllValues("statement"), p2.getType());
			for (auto val = allQVals.begin(); val != allQVals.end(); ++val)
			{
				auto qResults = filterNodesByType(ast->getFollowed(*val, transient), p1.getType());
				for (auto r = qResults.begin(); r != qResults.end(); ++r)
				{
					closureResult.qp[nodeToString(*val)].insert(nodeToString(*r));
				}
			}
		}
		if (!relation.compare("parent"))
		{
			auto allPVals = filterNodesByType(filterNodesByType(getAllValues("statement"), "if", "while"), p1.getType());
			for (auto val = allPVals.begin(); val != allPVals.end(); ++val)
			{
				auto pResults = filterNodesByType(ast->getChildren(*val, transient), p2.getType());
				for (auto r = pResults.begin(); r != pResults.end(); ++r)
				{
					closureResult.pq[nodeToString(*val)].insert(nodeToString(*r));
				}
			}
			auto allQVals = filterNodesByType(getAllValues("statement"), p2.getType());
			for (auto val = allQVals.begin(); val != allQVals.end(); ++val)
			{
				auto qResults = filterNodesByType(ast->getParent(*val, transient), p1.getType());
				for (auto r = qResults.begin(); r != qResults.end(); ++r)
				{
					closureResult.qp[nodeToString(*val)].insert(nodeToString(*r));
				}
			}
		}
		if (!relation.compare("calls"))
		{
			auto allPVals = filterNodesByType(getAllValues("procedure"), p1.getType());
			for (auto val = allPVals.begin(); val != allPVals.end(); ++val)
			{
				auto pResults = filterNodesByType(ast->getCallees(*val, transient), p2.getType());
				for (auto r = pResults.begin(); r != pResults.end(); ++r)
				{
					closureResult.pq[nodeToString(*val)].insert(nodeToString(*r));
				}
			}
			auto allQVals = filterNodesByType(getAllValues("procedure"), p2.getType());
			for (auto val = allQVals.begin(); val != allQVals.end(); ++val)
			{
				auto qResults = filterNodesByType(ast->getCallers(*val, transient), p1.getType());
				for (auto r = qResults.begin(); r != qResults.end(); ++r)
				{
					closureResult.qp[nodeToString(*val)].insert(nodeToString(*r));
				}
			}
		}
		if (!relation.compare("uses"))
		{
			auto procNodes = getAllValues("procedure");
			auto allVals = getAllValues("statement");
			allVals.insert(allVals.end(), procNodes.begin(), procNodes.end());

			auto allPVals = filterNodesByType(allVals, p1.getType());
			for (auto val = allPVals.begin(); val != allPVals.end(); ++val)
			{
				auto pResults = ast->getUsed(*val);
				for (auto r = pResults.begin(); r != pResults.end(); ++r)
				{
					closureResult.pq[nodeToString(*val)].insert(*r);
				}
			}
			auto allQVals = getAllVariables();
			for (auto val = allQVals.begin(); val != allQVals.end(); ++val)
			{
				auto qResults = filterNodesByType(ast->getUsing(*val), p1.getType());
				for (auto r = qResults.begin(); r != qResults.end(); ++r)
				{
					closureResult.qp[*val].insert(nodeToString(*r));
				}
			}
		}
		if (!relation.compare("modifies"))
		{
			auto procNodes = getAllValues("procedure");
			auto allVals = getAllValues("statement");
			allVals.insert(allVals.end(), procNodes.begin(), procNodes.end());

			auto allPVals = filterNodesByType(allVals, p1.getType());
			for (auto val = allPVals.begin(); val != allPVals.end(); ++val)
			{
				auto pResults = ast->getModified(*val);
				for (auto r = pResults.begin(); r != pResults.end(); ++r)
				{
					closureResult.pq[nodeToString(*val)].insert(*r);
				}
			}
			auto allQVals = getAllVariables();
			for (auto val = allQVals.begin(); val != allQVals.end(); ++val)
			{
				auto qResults = filterNodesByType(ast->getModifying(*val), p1.getType());
				for (auto r = qResults.begin(); r != qResults.end(); ++r)
				{
					closureResult.qp[*val].insert(nodeToString(*r));
				}
			}
		}
		if (!relation.compare("affects"))
		{
			auto allPVals = filterNodesByType(getAllValues("assign"), p1.getType());
			for (auto val = allPVals.begin(); val != allPVals.end(); ++val)
			{
				auto pResults = filterNodesByType(ast->getAffected(*val, transient), p2.getType());
				for (auto r = pResults.begin(); r != pResults.end(); ++r)
				{
					closureResult.pq[nodeToString(*val)].insert(nodeToString(*r));
				}
			}
			auto allQVals = filterNodesByType(getAllValues("assign"), p2.getType());
			for (auto val = allQVals.begin(); val != allQVals.end(); ++val)
			{
				auto qResults = filterNodesByType(ast->getAffecting(*val, transient), p1.getType());
				for (auto r = qResults.begin(); r != qResults.end(); ++r)
				{
					closureResult.qp[nodeToString(*val)].insert(nodeToString(*r));
				}
			}
		}
		// ......
	}
	return closureResult;
}

ClosureResult Solver::getPatternResult(Pattern pattern)
{
	ClosureResult result;
	nodeCollection nodes = ast->getPattern(pattern.lhs, pattern.rhs);
	if (nodes.empty())
	{
		result.resultType = "BOOL";
		result.boolResult = false;
	}
	else
	{
		result.resultType = "SET";
		result.p = pattern.varName;
		for (auto it = nodes.begin(); it != nodes.end(); ++it)
		{
			result.vals.insert(to_string(static_cast<long long>(ast->getLineNumber(*it))));
		}
	}
	return result;
}

ClosureResult Solver::getWithResult(With with)
{
	ClosureResult closureResult = ClosureResult();
	if (with.lhsPropertyName == with.rhsPropertyName)
	{
		closureResult.resultType = "MAP";
		closureResult.p = with.lhsVarName;
		closureResult.q = with.rhsVarName;
		set<string> allVals = getAllPropertyValues(with.lhsPropertyName);
		for (auto it = allVals.begin(); it != allVals.end(); ++it)
		{
			closureResult.pq[*it].insert(*it);
			closureResult.qp[*it].insert(*it);
		}
	}
	else if ((!with.lhsPropertyName.compare("value") && !with.rhsPropertyName.compare("stmt")) || (!with.lhsPropertyName.compare("stmt") && !with.rhsPropertyName.compare("value")))
	{
		closureResult.resultType = "MAP";
		closureResult.p = with.lhsVarName;
		closureResult.q = with.rhsVarName;
		set<string> allVals;
		auto lhsVals = getAllPropertyValues(with.lhsPropertyName);
		auto rhsVals = getAllPropertyValues(with.rhsPropertyName);
		for (auto el = lhsVals.begin(); el != lhsVals.end(); ++el)
		{
			if (rhsVals.find(*el) != rhsVals.end())
			{
				allVals.insert(*el);
			}
		}
		for (auto it = allVals.begin(); it != allVals.end(); ++it)
		{
			closureResult.pq[*it].insert(*it);
			closureResult.qp[*it].insert(*it);
		}
	}
	else if (!with.lhsPropertyName.compare("value"))
	{
		closureResult.resultType = "BOOLEAN";
		auto constants = ast->constants;
		closureResult.boolResult = false;
		for (auto it = constants->begin(); it != constants->end(); ++it)
		{
			if (!(*it).compare(with.rhsVarName))
			{
				closureResult.boolResult = true;
				break;
			}
		}
	}
	else
	{
		closureResult.resultType = "SET";
		closureResult.p = with.lhsVarName;
		if (with.rhsVarName[0] == '"')
		{
			closureResult.vals.insert(with.rhsVarName.substr(1, with.rhsVarName.length() - 2));
		}
		else
		{
			closureResult.vals.insert(with.rhsVarName);
		}
	}
	return closureResult;
}

void Solver::findResult(int pred)
{
	if (timeout())
	{
		return;
	}
	if (pred >= possibleValues.size()) // to jest do wywalenia jak beda juz ClosureResult zrobione
	{
		return;
	}

	for (auto val = possibleValues[pred].begin(); val != possibleValues[pred].end(); ++val)
	{
		bool foundValue = true;
		for (auto cr = dependentClosures[pred].begin(); cr != dependentClosures[pred].end(); ++cr)
		{
			string depPred = cr->p.compare(valueOfPred[pred]) ? cr->p : cr->q;
			auto & map = cr->p.compare(valueOfPred[pred]) ? cr->qp : cr->pq;
			if (map[*val].find(tmpResult[indexOfPred[depPred]]) == map[*val].end())
			{
				foundValue = false;
				break;
			}
		}
		if (foundValue)
		{
			tmpResult[pred] = *val;
			if (pred < predTable.size() - 1)
			{
				findResult(pred + 1);
			}
			else
			{
				vector<string> result;
				for (size_t i = 0; i < predTable.size(); ++i)
				{
					result.push_back(tmpResult[i]);
				}
				resultTable.push_back(result);
			}
		}
	}
}

void Solver::findPossibleValues()
{
	set<string> tmpValues;
	set<int> updatedPreds;

	if (closureResults.empty())
	{
		int i = 0;
		for (auto pred = predTable.begin(); pred != predTable.end(); ++pred)
		{
			auto allVals = getAllValues(pred->getType());
			for (auto val = allVals.begin(); val != allVals.end(); ++val)
			{
				possibleValues[i].insert(nodeToString(*val));
			}
			++i;
		}
	}

	for (auto cr = closureResults.begin(); cr != closureResults.end(); ++cr) {
		int pred;
		if (!cr->resultType.compare("SET"))
		{
			pred = indexOfPred[cr->p];
			updatePossibleValues(pred, cr->vals, updatedPreds.find(pred) != updatedPreds.end());
			updatedPreds.insert(pred);
		}
		else if (!cr->resultType.compare("MAP"))
		{
			for (size_t i = 0; i < 2; ++i)
			{
				tmpValues.clear();
				pred = indexOfPred[i == 0 ? cr->p : cr->q];
				auto & m = i == 0 ? cr->pq : cr->qp;
				for (auto mapIt = m.begin(); mapIt != m.end(); ++mapIt)
				{
					tmpValues.insert(mapIt->first);
				}
				updatePossibleValues(pred, tmpValues, updatedPreds.find(pred) != updatedPreds.end());
				updatedPreds.insert(pred);
			}
		}
	}
}

void Solver::updatePossibleValues(int pred, set<string>& vals, bool updated)
{
	if (updated)
	{
		set<string> intersection;
		for (auto el = vals.begin(); el != vals.end(); ++el)
		{
			if (possibleValues[pred].find(*el) != possibleValues[pred].end())
			{
				intersection.insert(*el);
			}
		}
		possibleValues[pred] = intersection;
	}
	else
	{
		possibleValues[pred].insert(vals.begin(), vals.end());
	}
}

void Solver::evaluate()
{
	bool foundFalseResult = false;
	bool foundSetOrMap = false;
	for (auto cr = closureResults.begin(); cr != closureResults.end(); ++cr) {
		if (!cr->resultType.compare("BOOL") && !cr->boolResult)
		{
			foundFalseResult = true;
			break;
		}
		if (cr->resultType.compare("BOOL"))
		{
			foundSetOrMap = true;
		}
	}
	if (!foundFalseResult)
	{
		findPossibleValues();
		if (foundSetOrMap)
		{
			findResult(0);
		}
		else
		{
			boolResult = true;
		}
	}

	if (selector.type == "boolean")
	{
		resultTable.empty() && !boolResult ? results.push_back("false") : results.push_back("true");
	}
	else if (selector.type == "variable")
	{
		set<string> res;
		for (auto it = resultTable.begin(); it != resultTable.end(); ++it)
		{
			res.insert((*it)[indexOfPred[(*selector.variables.begin())]]);
		}
		for (auto resultIt = res.begin(); resultIt != res.end(); ++resultIt)
		{
			results.push_back((*resultIt));
		}
	}
	else if (selector.type == "tuple")
	{
		set<string> res;
		for (auto it = resultTable.begin(); it != resultTable.end(); ++it)
		{
			string tuple = "";
			for (auto varIt = selector.variables.begin(); varIt != selector.variables.end(); ++varIt)
			{
				tuple = tuple.append(((*it)[indexOfPred[(*varIt)]]));
				tuple = tuple.append(" ");
			}
			tuple = tuple.substr(0, tuple.length() - 1);
			res.insert(tuple);
		}
		for (auto resultIt = res.begin(); resultIt != res.end(); ++resultIt)
		{
			results.push_back((*resultIt));
		}
	}
}