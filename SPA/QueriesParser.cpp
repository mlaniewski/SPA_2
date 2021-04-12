#include "QueriesParser.h"
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <boost/tokenizer.hpp>
#include <list>
#include "Solver.h"

using namespace std;
using namespace boost;

QueriesParser::~QueriesParser()
{
}

vector<string> QueriesParser::split(string s, char delim) {
	stringstream ss(s);
	string item;
	vector<string> elems;
	while (getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

void QueriesParser::parseQuery()
{

	char_separator<char> sepQuery(";");
	stringTokenizer tokens(query, sepQuery);
	stringTokenizer::iterator tokenIterator;

	for (tokenIterator = tokens.begin(); tokenIterator != tokens.end(); ++tokenIterator) {
		vector<string> elements = split(*tokenIterator, ' ');
		if (icompare(elements[1], "select")) {
			parserResultPart(*tokenIterator);
			break;
		}
		else {
			parsePredicate(*tokenIterator);
		}
	}

	if (tokenIterator == tokens.end()) {
		throw QueryException("Expected 'select' keyword");
	}
}

void QueriesParser::parsePredicate(string predicate) {
	char_separator<char> sepQuery(" ", ";(),");
	stringTokenizer tokens(predicate, sepQuery);
	stringTokenizer::iterator token = tokens.begin();

	string currentPredicate = "";

	if (icompare(*token, "procedure")) {
		currentPredicate = "procedure";
	}
	else if (icompare(*token, "prog_line") || icompare(*token, "stmt") || icompare(*token, "stmtlist") || icompare(*token, "stmtlst") || icompare(*token, "statement")) {
		currentPredicate = "statement";
	}
	else if (icompare(*token, "assign")) {
		currentPredicate = "assign";
	}
	else if (icompare(*token, "while")) {
		currentPredicate = "while";
	}
	else if (icompare(*token, "if")) {
		currentPredicate = "if";
	}
	else if (icompare(*token, "var") || icompare(*token, "variable")) {
		currentPredicate = "var";
	}
	else if (icompare(*token, "call")) {
		currentPredicate = "call";
	}
	else if (icompare(*token, "const") || icompare(*token, "constant")) {
		currentPredicate = "const";
	}
	else {
		throw QueryException("Invalid predicate name: " + *token);
	}

	token++;
	while (token != tokens.end()) {

		if (*token == ",") {
			token++;
			continue;
		}
		predTable.push_back(Predicate(currentPredicate, *token));
		token++;
	}


}

void QueriesParser::parserResultPart(string resutPart) {
	size_t i = 1;

	char_separator<char> sepQuery(" #", "=<>;(),.");
	stringTokenizer tokens(resutPart, sepQuery);
	stringTokenizer::iterator token = tokens.begin();
	if (!icompare(*token, "select")) {
		throw QueryException("Expected 'select' keyword at the beginning");
	}
	token++;	//after select

				//====================
				//parse selector
				//====================
	if (*token == "<") {
		token++;	//after <
		selector.type = "tuple";
		while (true) {
			string tmpvar = *token;
			token++;	//after var name
			if (*token == ">") {
				if (checkResultPartElement(tmpvar)) {
					selector.addVariable(tmpvar);
				}
				token++;	//after >
				break;
			}
			else if (*token == ",") {
				if (checkResultPartElement(tmpvar)) {
					selector.addVariable(tmpvar);
				}
				token++;	//after ,
			}
			else if (*token == ".") {
				token++;	//after .
				selector.variableProperties[tmpvar].insert(*token);
				token++;	//after property name
			}
			else {
				throw QueryException("Unexpected tuple keyword: " + *token);
			}
		}
	}
	else if (icompare(*token, "boolean")) {
		token++; //after boolean
		selector.type = "boolean";
	}
	else {
		string tmpvar = *token;
		token++;	//after var name
		if (token != tokens.end() && *token == ".") {
			selector.type = "property";
			token++;		//after .
			if (icompare(*token, "stmt") || icompare(*token, "varname") || icompare(*token, "procname") || icompare(*token, "value")) {
				selector.variableProperties[tmpvar].insert(*token);
				token++;	//after property name
			}
			else {
				throw QueryException("Unsupported variable property: " + *token);
			}
		}
		else {

			if (checkResultPartElement(tmpvar)) {
				selector.addVariable(tmpvar);
				selector.type = "variable";
			}
		}
	}

	//==========================
	// parse after selector
	//==========================
	while (token != tokens.end()) {
		if (icompare(*token, "and")) {
			if (token != tokens.end()) {
				token++;	//after and
			}
			else {
				throw QueryException("Unexpected 'and' at the end.");
			}
		}

		//==========================
		// parse such that
		//==========================
		if (icompare(*token, "such")) {
			token++;	//after such
			if (!icompare(*token, "that")) {
				throw QueryException("Expected 'that' after 'such'");
			}
			token++;	//after that

			while (true) {
				string tmpvar = *token;
				transform(tmpvar.begin(), tmpvar.end(), tmpvar.begin(), ::tolower);
				token++;
				if (*token != "(") {
					throw QueryException("Expected '(' got: " + *token);
				}
				token++;	//after (
				string lhs = *token;
				token++;
				if (*token != ",") {
					throw QueryException("Expected ',' got: " + *token);
				}
				token++;	//after ,
				string rhs = *token;
				token++;
				if (*token != ")") {
					throw QueryException("Expected ')' got: " + *token);
				}
				closureTable.push_back(Closure(tmpvar, lhs, rhs));

				if (token != tokens.end()) {
					token++;	//after )
				}

				if (token != tokens.end()) {
					if (!icompare(*token, "and")) {
						break;
					}
					else {
						token++;	//after and
					}
				}
				else {
					break;
				}
			}
		}
		//==========================
		// parse with
		//==========================
		else if (icompare(*token, "with")) {
			token++;	//after with

			while (true) {

				With tmp = With();
				tmp.lhsVarName = *token;
				token++;	//after lhs var name

				if (*token == ".") {
					token++;	//after .
					tmp.lhsIsProperty = true;
					tmp.lhsPropertyName = *token;
					token++;	//after property name
				}

				tmp.operand = *token;
				token++;	//after operand = or other if exists
				tmp.rhsVarName = *token;
				token++;	//after rhs var name
				if (token != tokens.end()) {

					if (*token == ".") {
						token++;	//after .
						tmp.rhsIsProperty = true;
						tmp.rhsPropertyName = *token;

						if (token != tokens.end()) {
							token++;	//after rhs property name
						}
					}
				}
				withTable.push_back(tmp);
				if (token != tokens.end()) {
					if (!icompare(*token, "and")) {
						break;
					}
					else {
						token++;	//after and
					}
				}
				else {
					break;
				}

			}
		}
		//==========================
		// parse pattern
		//==========================
		else if (icompare(*token, "pattern")) {
			token++;	//after pattern
			string tmpvar = *token;
			token++;	//after var name
			if (*token != "(") {
				throw QueryException("Expected '(' got: " + *token);
			}
			token++;	//after (
			string lhs = *token;
			token++;
			if (*token != ",") {
				throw QueryException("Expected ',' got: " + *token);
			}
			token++;	//after ,
			string rhs = "";
			while (token != tokens.end() && *token != ")") {
				rhs.append(*token);
				token++;
			}
			if (*token != ")") {
				throw QueryException("Expected ')' at the end of pattern, got: " + *token);
			}
			patternTable.push_back(Pattern(tmpvar, lhs, rhs));
			if (token != tokens.end()) {
				token++;	//after )
			}
		}
		else {
			throw QueryException("Invalid query part: " + *token);
		}
	}
	Solver Solver(results, selector, closureTable, patternTable, predTable, withTable, ast);
	Solver.evaluate();
}

bool QueriesParser::checkResultPartElement(string token)
{
	if (icompare(token, "procedure") || icompare(token, "prog_line") ||
		icompare(token, "stmt") || icompare(token, "stmtlist") ||
		icompare(token, "stmtlst") || icompare(token, "statement") ||
		icompare(token, "assign") || icompare(token, "while") ||
		icompare(token, "if") || icompare(token, "var") ||
		icompare(token, "variable") || icompare(token, "call") ||
		icompare(token, "const") || icompare(token, "constant")) {
		return true;
	}
	else {
		size_t i = 0;
		for (i = 0; i < predTable.size(); i++) {
			if (icompare(predTable[i].getValue(), token)) {
				return true;
			}
		}
		if (i == predTable.size()) {
			throw QueryException("Invalid searching result element: " + token);
		}
	}

}

//DEBUG
void QueriesParser::printDebugResults() {
	cout << endl << "Predicates:" << endl;
	for (vector<Predicate>::const_iterator i = predTable.begin(); i != predTable.end(); ++i) {
		cout << i->toString();
	}

	cout << endl << "Selectors:" << endl;
	cout << "Type: " << selector.type << endl;

	for (vector<string>::const_iterator i = selector.variables.begin(); i != selector.variables.end(); ++i) {
		cout << "variable: " << *i << endl;

		for (set<string>::const_iterator it = selector.variableProperties[*i].begin(); it != selector.variableProperties[*i].end(); ++it) {
			cout << "property: " << *it << endl;
		}

	}

	cout << "Closures: " << endl;

	for (vector<Closure>::const_iterator i = closureTable.begin(); i != closureTable.end(); ++i) {
		cout << i->toString();
	}

	cout << "Withs: " << endl;

	for (vector<With>::const_iterator i = withTable.begin(); i != withTable.end(); ++i) {
		cout << i->toString();
	}

	cout << "Patterns: " << endl;

	for (vector<Pattern>::const_iterator i = patternTable.begin(); i != patternTable.end(); ++i) {
		cout << i->toString();
	}

	cout << endl;
}

bool QueriesParser::icompare(const std::string& str1, const std::string& str2) {
	std::string str1Cpy(str1);
	std::string str2Cpy(str2);
	std::transform(str1Cpy.begin(), str1Cpy.end(), str1Cpy.begin(), ::tolower);
	std::transform(str2Cpy.begin(), str2Cpy.end(), str2Cpy.begin(), ::tolower);
	return (str1Cpy == str2Cpy);
}
