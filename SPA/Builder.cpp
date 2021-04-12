#include "Builder.h"
#include <math.h>

using namespace std;

Builder::Builder()
{
	shared_ptr<Node> n = make_shared<Node>(Program);
	astTree = make_shared<tree<shared_ptr<Node>>>(n);
	procedures = make_shared<vector<NODE>>();
	whiles = make_shared<vector<NODE>>();
	ifs = make_shared<vector<NODE>>();
	assigments = make_shared<vector<NODE>>();
	variables = make_shared<set<string>>();
	constants = make_shared<set<string>>();
	programLines = make_shared<vector<NODE>>();
	callers = make_shared<map<unsigned int, set<unsigned int>>>();
	callersT = make_shared<map<unsigned int, set<unsigned int>>>();
	callees = make_shared<map<unsigned int, set<unsigned int>>>();
	calleesT = make_shared<map<unsigned int, set<unsigned int>>>();
	modifies = make_shared<map<unsigned int, set<string>>>();
	modified = make_shared<map<string, set<unsigned int>>>();
	uses = make_shared<map<unsigned int, set<string>>>();
	used = make_shared<map<string, set<unsigned int>>>();
	parent = make_shared<map<unsigned int, unsigned int>>();
	parentT = make_shared<map<unsigned int, set<unsigned int>>>();
	children = make_shared<map<unsigned int, set<unsigned int>>>();
	childrenT = make_shared<map<unsigned int, set<unsigned int>>>();
	nextN = make_shared<map<unsigned int, set<unsigned int>>>();
	nextT = make_shared<map<unsigned int, set<unsigned int>>>();
	prevN = make_shared<map<unsigned int, set<unsigned int>>>();
	prevT = make_shared<map<unsigned int, set<unsigned int>>>();
	pattern = make_shared<map<unsigned int, set<string>>>();
	fullPattern = make_shared<map<unsigned int, string>>();
	callNodes = make_shared<vector<NODE>>();
	affecting = make_shared<map<unsigned int, set<unsigned int>>>();
	affectingT = make_shared<map<unsigned int, set<unsigned int>>>();
	affected = make_shared<map<unsigned int, set<unsigned int>>>();
	affectedT = make_shared<map<unsigned int, set<unsigned int>>>();
	ast = shared_ptr<AST>(new AST(
		astTree,
		procedures,
		whiles,
		ifs,
		assigments,
		variables,
		constants,
		programLines,
		callers,
		callersT,
		callees,
		calleesT,
		modifies,
		modified,
		uses,
		used,
		parent,
		parentT,
		children,
		childrenT,
		nextN,
		nextT,
		prevN,
		prevT,
		pattern,
		fullPattern,
		callNodes,
		affecting,
		affectingT,
		affected,
		affectedT));
}

Builder::~Builder()
{
}

NODE Builder::createNode(NODETYPE nodeType)
{
	if (nodeType == Program)
	{
		return astTree->begin();
	}
	shared_ptr<Node> n = make_shared<Node>(nodeType);
	NODE node = astTree->append_child(astTree->begin(), n);
	n->setTreeIterator(node);
	switch (nodeType)
	{
	case Procedure:
		procedures->push_back(node);
		break;
	case Variable:
		varNodes.push_back(node);
		break;
	case Call:
		callNodes->push_back(node);
		break;
	case While:
		whiles->push_back(node);
		break;
	case If:
		ifs->push_back(node);
		break;
	case Assign:
		assigments->push_back(node);
		break;
	case Constant:
		constantNodes.push_back(node);
	default:
		break;
	}
	return node;
}

void Builder::addNodeParameter(NODE node, NODEPARAMTYPE nodeParamType, string val)
{
	(*node)->setParam(nodeParamType, val);
}

void Builder::addChild(NODE parent, NODE child)
{
	if ((*parent)->nodeType != Program)
	{
		astTree->move_ontop(astTree->append_child(parent), child);
	}
}


void Builder::initializeCallMaps()
{
	for (auto it = callNodes->begin(); it != callNodes->end(); ++it)
	{
		(*callers)[(*(ast->getProcedureByName((**it)->getParam(Caller))))->id].insert((*ast->getProcedureByName((**it)->getParam(Callee)))->id);
		(*callees)[(*(ast->getProcedureByName((**it)->getParam(Callee))))->id].insert((*ast->getProcedureByName((**it)->getParam(Caller)))->id);
		(*callersT)[(*(ast->getProcedureByName((**it)->getParam(Caller))))->id].insert((*ast->getProcedureByName((**it)->getParam(Callee)))->id);
		(*calleesT)[(*(ast->getProcedureByName((**it)->getParam(Callee))))->id].insert((*ast->getProcedureByName((**it)->getParam(Caller)))->id);
	}
}

void Builder::initializeModifiesAndUsesMaps()
{
	for (auto asgnIt = assigments->begin(); asgnIt != assigments->end(); ++asgnIt)
	{
		NODE asgnNodeIt = astTree->begin(*asgnIt);
		string variable = (*asgnNodeIt)->getParam(Name);
		(*modifies)[(**asgnIt)->id].insert(variable);
		initializeUsesAssignment(*asgnIt, *asgnIt);
	}
	set<unsigned int> callsToInit;
	for (auto procedureIt = procedures->begin(); procedureIt != procedures->end(); ++procedureIt)
	{
		initializeModifiesAndUsesContainers(*procedureIt);
	}
	for (auto callIt = callNodes->begin(); callIt != callNodes->end(); ++callIt)
	{
		callsToInit.insert((**callIt)->id);
	}
	while (!callsToInit.empty())
	{
		initializeModifiesAndUsesCalls(Node::getNodeById(*callsToInit.begin()), callsToInit);
	}
}

void Builder::initializeUsesAssignment(NODE assignment, NODE child)
{
	if ((*child)->nodeType == Assign)
	{
		child = astTree->begin(child);
		++child;
	}
	for (auto it = astTree->begin(child); it != astTree->end(child); ++it)
	{
		switch ((*it)->nodeType)
		{
		case Variable:
			(*uses)[(*assignment)->id].insert((*it)->getParam(Name));
			break;
		default:
			initializeUsesAssignment(assignment, it);
			break;
		}
	}
}

void Builder::initializeModifiesAndUsesContainers(NODE container)
{
	for (auto stmtLstIt = astTree->begin(container); stmtLstIt != astTree->end(container); ++stmtLstIt)
	{
		for (auto stmtIt = astTree->begin(stmtLstIt); stmtIt != astTree->end(stmtLstIt); ++stmtIt)
		{
			switch ((*stmtIt)->nodeType)
			{
			case While:
			case If:
				(*uses)[(*stmtIt)->id].insert((*stmtIt)->getParam(Cond));
			case Procedure:
				initializeModifiesAndUsesContainers(stmtIt);
			case Assign:
				(*modifies)[(*container)->id].insert((*modifies)[(*stmtIt)->id].begin(), (*modifies)[(*stmtIt)->id].end());
				(*uses)[(*container)->id].insert((*uses)[(*stmtIt)->id].begin(), (*uses)[(*stmtIt)->id].end());
			}
		}
	}
}

void Builder::initializeModifiesAndUsesCalls(NODE call, set<unsigned int> &callsToInit)
{
	callsToInit.erase((*call)->id);
	for (auto callIt = callNodes->begin(); callIt != callNodes->end(); ++callIt)
	{
		if (callsToInit.find((**callIt)->id) != callsToInit.end()
			&& !(**callIt)->getParam(Caller).compare((*call)->getParam(Callee)))
		{
			initializeModifiesAndUsesCalls(*callIt, callsToInit);
		}
		unsigned int caleeProcId = (*ast->getProcedureByName((*call)->getParam(Callee)))->id;
		(*modifies)[(*call)->id].insert((*modifies)[caleeProcId].begin(), (*modifies)[caleeProcId].end());
		(*uses)[(*call)->id].insert((*uses)[caleeProcId].begin(), (*uses)[caleeProcId].end());
		NODE node = call;
		while ((*node)->nodeType != Procedure)
		{
			do
			{
				node = astTree->parent(node);
			} while ((*node)->nodeType != If && (*node)->nodeType != While && (*node)->nodeType != Procedure);
			(*modifies)[(*node)->id].insert((*modifies)[(*call)->id].begin(), (*modifies)[(*call)->id].end());
			(*uses)[(*node)->id].insert((*uses)[(*call)->id].begin(), (*uses)[(*call)->id].end());
		}
	}
}

void Builder::initializeParentMap(NODE node)
{
	NODETYPE nt = (*node)->nodeType;
	bool isLeafNode = nt == Call || nt == Assign;
	bool isContainerNode = nt == While || nt == If;
	bool isStmt = isLeafNode || isContainerNode;
	if (isStmt && !tempUiVector.empty())
	{
		(*parent)[(*node)->id] = tempUiVector.back();
		(*parentT)[(*node)->id].insert(tempUiVector.begin(), tempUiVector.end());
		(*children)[(*parent)[(*node)->id]].insert((*node)->id);
		(*childrenT)[(*parent)[(*node)->id]].insert((*node)->id);
	}
	if (!isLeafNode)
	{
		if (isContainerNode)
		{
			tempUiVector.push_back((*node)->id);
		}
		for (auto childIt = astTree->begin(node); childIt != astTree->end(node); ++childIt)
		{
			initializeParentMap(childIt);
		}
		if (isContainerNode)
		{
			tempUiVector.pop_back();
		}
	}
}

void Builder::initializeNextStmtLst(NODE stmtLst, NODE next)
{
	auto prev = astTree->begin(stmtLst);
	auto n = ++astTree->begin(stmtLst);
	while (n != astTree->end(stmtLst))
	{
		initializeNext(prev, n);
		++prev;
		++n;
	}
	initializeNext(prev, next);
}

void Builder::initializeNext(NODE prev, NODE next)
{
	tree<shared_ptr<Node>>::sibling_iterator childNode;
	switch ((*prev)->nodeType)
	{
	case If:
		childNode = astTree->begin(prev);
		(*nextN)[(*prev)->id].insert((*astTree->begin(childNode))->id);
		(*nextT)[(*prev)->id].insert((*astTree->begin(childNode))->id);
		(*prevN)[(*astTree->begin(childNode))->id].insert((*prev)->id);
		(*prevT)[(*astTree->begin(childNode))->id].insert((*prev)->id);
		(*nextN)[(*prev)->id].insert((*astTree->begin(++childNode))->id);
		(*nextT)[(*prev)->id].insert((*astTree->begin(childNode))->id);
		(*prevN)[(*astTree->begin(childNode))->id].insert((*prev)->id);
		(*prevT)[(*astTree->begin(childNode))->id].insert((*prev)->id);
		childNode = astTree->begin(prev);
		initializeNextStmtLst(childNode, next);
		initializeNextStmtLst(++childNode, next);
		break;
	case While:
		childNode = astTree->begin(prev);
		(*nextN)[(*prev)->id].insert((*astTree->begin(childNode))->id);
		(*nextT)[(*prev)->id].insert((*astTree->begin(childNode))->id);
		(*prevN)[(*astTree->begin(childNode))->id].insert((*prev)->id);
		(*prevT)[(*astTree->begin(childNode))->id].insert((*prev)->id);
		initializeNextStmtLst(astTree->begin(prev), prev);
	case Assign:
	case Call:
		if (next != NULL)
		{
			(*nextN)[(*prev)->id].insert((*next)->id);
			(*nextT)[(*prev)->id].insert((*next)->id);
			(*prevN)[(*next)->id].insert((*prev)->id);
			(*prevT)[(*next)->id].insert((*prev)->id);
		}
	}
}

void Builder::initializeTransientRelation(shared_ptr<map<unsigned int, set<unsigned int>>> rel)
{
	bool finished = false;
	while (!finished)
	{
		finished = true;
		for (auto aIt = rel->begin(); aIt != rel->end(); ++aIt)
		{
			unsigned int a = aIt->first;
			for (auto bIt = aIt->second.begin(); bIt != aIt->second.end(); ++bIt)
			{
				unsigned int b = *bIt;
				for (auto cIt = (*rel)[b].begin(); cIt != (*rel)[b].end(); ++cIt)
				{
					unsigned int c = *cIt;
					if ((*rel)[a].insert(c).second)
					{
						finished = false;
					}
				}
			}
		}
	}
}

void Builder::initializeInvertedVariableRelation(shared_ptr<map<unsigned int, set<string>>> rel, shared_ptr<map<string, set<unsigned int>>> inv)
{
	for (auto indexIt = rel->begin(); indexIt != rel->end(); ++indexIt)
	{
		for (auto valueIt = indexIt->second.begin(); valueIt != indexIt->second.end(); ++valueIt)
		{
			(*inv)[*valueIt].insert(indexIt->first);
		}
	}
}

void Builder::initializePattern()
{
	for (auto it = assigments->begin(); it != assigments->end(); ++it)
	{
		auto exp = astTree->begin(++astTree->begin(*it));
		(*fullPattern)[(**it)->id] = initializePatternNode(exp, (**it)->id);
	}
}

string Builder::initializePatternNode(NODE exp, unsigned int id)
{
	string val = (*exp)->getParam(Name);
	string result = "";
	if ((*exp)->nodeType == Variable || (*exp)->nodeType == Constant)
	{
		result = val;
	}
	else
	{
		auto itCh = astTree->begin(exp);
		for (int i = 0; i < 3; ++i)
		{
			if (i == 1)
			{
				result += val;
				continue;
			}
			string c = initializePatternNode(itCh, id);

			if (!val.compare("*") && !(*itCh)->getParam(Name).compare("+"))
			{
				c = "(" + c + ")";
			}
			result += c;
			++itCh;
		}
	}
	(*pattern)[id].insert(result);
	return result;
}

void Builder::addLineNumbers()
{
	int lineNumber = 0;
	for (NODE treeIt = astTree->begin(); treeIt != astTree->end(); ++treeIt)
	{
		switch ((*treeIt)->nodeType)
		{
		case Call:
		case While:
		case If:
		case Assign:
			(*treeIt)->lineNumber = ++lineNumber;
			programLines->push_back(treeIt);
		}
	}
}

bool Builder::checkAffects(NODE a, NODE b)
{
	bool cond = false;
	auto vs = ast->getModified(a);
	auto aNexts = ast->getNext(a, true);
	auto bPrevs = ast->getPrev(b, true);
	nodeCollection a1s;

	for (auto an = aNexts.begin(); an != aNexts.end(); ++an)
	{
		for (auto bp = bPrevs.begin(); bp != bPrevs.end(); ++bp)
		{
			if ((**an)->id == (**bp)->id && (**an)->nodeType == Assign)
			{
				a1s.push_back(*an);
				break;
			}
		}
	}

	for (auto v = vs.begin(); v != vs.end(); ++v)
	{
		if (ast->checkUses(b, *v))
		{
			cond = true;
			for (auto a1 = a1s.begin(); a1 != a1s.end(); ++a1)
			{
				if (ast->checkModifies(*a1, *v))
				{
					cond = false;
				}
			}
			if (cond)
			{
				return true;
			}
		}
	}
	return false;
}

shared_ptr<AST> Builder::getAST()
{
	addLineNumbers();
	initializeCallMaps();
	tempUiVector.clear();
	initializeParentMap(astTree->begin());
	initializeModifiesAndUsesMaps();
	tempUiVector.clear();
	for (auto procIt = procedures->begin(); procIt != procedures->end(); ++procIt)
	{
		initializeNextStmtLst(astTree->begin(*procIt));
	}
	initializeTransientRelation(nextT);
	initializeTransientRelation(callersT);
	initializeTransientRelation(calleesT);
	initializeTransientRelation(childrenT);
	initializeInvertedVariableRelation(modifies, modified);
	initializeInvertedVariableRelation(uses, used);
	initializeTransientRelation(prevT);
	initializePattern();
	for (auto it = varNodes.begin(); it != varNodes.end(); ++it)
	{
		(*variables).insert((**it)->getParam(Name));
	}
	for (auto it = constantNodes.begin(); it != constantNodes.end(); ++it)
	{
		(*constants).insert((**it)->getParam(Name));
	}

	for (auto a = assigments->begin(); a != assigments->end(); ++a)
	{
		auto bs = ast->getNext(*a, true);
		for (auto b = bs.begin(); b != bs.end(); ++b)
		{
			if ((**b)->nodeType == Assign && checkAffects(*a, *b))
			{
				(*affecting)[(**a)->id].insert((**b)->id);
				(*affectingT)[(**a)->id].insert((**b)->id);
				(*affected)[(**b)->id].insert((**a)->id);
				(*affectedT)[(**b)->id].insert((**a)->id);
			}
		}
	}
	initializeTransientRelation(affectingT);
	initializeTransientRelation(affectedT);

	return ast;
}