#include "SourceParser.h"
#include "pkb.h"
#include <fstream>
#include <iostream>
#include <string>
#include <regex>

SourceParser::SourceParser(Builder & builder) : builder(builder)
{
}


SourceParser::~SourceParser()
{
}

void SourceParser::parse(std::string filename)
{
	shared_ptr<Tokens> tokens = createTokenVector(filename);
	token = tokens->begin();
	NODE program = builder.createNode(Program);
	while (token != tokens->end())
	{
		builder.addChild(program, parseProcedure());
	}
}

NODE SourceParser::parseProcedure()
{
	validate(match(token, "procedure"), "Expected 'procedure'. Found '" + *token + "'.");
	++token;
	validate(matchName(token), "Expected procedure name. Found '" + *token + "'.");
	procName = *(token++);
	NODE procNode = builder.createNode(Procedure);
	builder.addNodeParameter(procNode, Name, procName);
	builder.addChild(procNode, parseStmtLst());
	return procNode;
}

NODE SourceParser::parseStmtLst(NODETYPE stmtLstType)
{
	validate(match(token, "{"), "Expected '{'. Found '" + *token + "'.");
	++token;
	validate(!match(token, "}"), "StmtLst must have at least one stmt");
	NODE stmtLstNode = builder.createNode(stmtLstType);
	while (!match(token, "}"))
	{
		builder.addChild(stmtLstNode, parseStmt());
	}
	++token;
	return stmtLstNode;
}

NODE SourceParser::parseStmt()
{
	if (match(token, "call"))
	{
		return parseCall();
	}
	if (match(token, "while"))
	{
		return parseWhile();
	}
	if (match(token, "if"))
	{
		return parseIf();
	}
	return parseAssignment();
}

NODE SourceParser::parseCall()
{
	++token;
	validate(matchName(token), "Expected calee procedure name. Found '" + *token + "'.");
	NODE callNode = builder.createNode(Call);
	builder.addNodeParameter(callNode, Caller, procName);
	builder.addNodeParameter(callNode, Callee, *token);
	++token;
	validate(match(token, ";"), "Expected ';'. Found '" + *token + "'.");
	++token;
	return callNode;
}

NODE SourceParser::parseWhile()
{
	++token;
	validate(matchName(token), "Expected variable name. Found '" + *token + "'.");
	NODE whileNode = builder.createNode(While);
	builder.addNodeParameter(whileNode, Cond, *token);
	++token;
	builder.addChild(whileNode, parseStmtLst());
	return whileNode;
}

NODE SourceParser::parseIf()
{
	++token;
	validate(matchName(token), "Expected variable name. Found '" + *token + "'.");
	NODE ifNode = builder.createNode(If);
	builder.addNodeParameter(ifNode, Cond, *token);
	++token;
	validate(match(token, "then"), "Expected 'then'. Found '" + *token + "'.");
	++token;
	builder.addChild(ifNode, parseStmtLst(Then));
	validate(match(token, "else"), "Expected 'else'. Found '" + *token + "'.");
	++token;
	builder.addChild(ifNode, parseStmtLst(Else));
	return ifNode;
}

NODE SourceParser::parseAssignment()
{
	validate(matchName(token), "Expected variable name. Found '" + *token + "'.");
	NODE assignment = builder.createNode(Assign);
	NODE variable = builder.createNode(Variable);
	builder.addNodeParameter(variable, Name, *token);
	builder.addChild(assignment, variable);
	++token;
	validate(match(token, "="), "Expected '='. Found '" + *token + "'.");
	++token;
	NODE expression = builder.createNode(Expression);
	builder.addChild(expression, parseExpression());
	builder.addChild(assignment, expression);
	validate(match(token, ";"), "Expected ';'. Found '" + *token + "'.");
	++token;
	return assignment;
}

NODE SourceParser::parseExpression()
{
	NODE expr = parseTerm();
	while (match(token, "+") || match(token, "-"))
	{
		string op = *(token++);
		NODE left = expr;
		NODE right = parseTerm();
		expr = builder.createNode(Operator);
		builder.addNodeParameter(expr, Name, op);
		builder.addChild(expr, left);
		builder.addChild(expr, right);
	}
	return expr;
}

NODE SourceParser::parseTerm()
{
	NODE term = parseFactor();
	while (match(token, "*"))
	{
		++token;
		NODE left = term;
		NODE right = parseFactor();
		term = builder.createNode(Operator);
		builder.addNodeParameter(term, Name, "*");
		builder.addChild(term, left);
		builder.addChild(term, right);
	}
	return term;
}

NODE SourceParser::parseFactor()
{
	NODE factor;
	if (match(token, "("))
	{
		++token;
		factor = parseExpression();
		validate(match(token, ")"), "Expected ')'. Found '" + *token + "'.");
		++token;
	}
	else
	{
		validate(matchName(token) || matchInteger(token), "Expected variable name or constant. Found '" + *token + "'.");
		factor = builder.createNode(matchInteger(token) ? Constant : Variable);
		builder.addNodeParameter(factor, Name, *(token++));
	}
	return factor;
}

std::shared_ptr<Tokens> SourceParser::createTokenVector(std::string filename)
{
	ifstream file;
	file.open(filename);
	shared_ptr<Tokens> tokens = make_shared<Tokens>();
	while (!file.eof()) {
		string s;
		file >> s;
		string::iterator i = s.begin();
		string::iterator j = s.begin();
		while (i != s.end())
		{
			if (isdigit(*i)) // integer
			{
				while (j != s.end() && isdigit(*j))
				{
					++j;
				}
				tokens->push_back(string(i, j));
			}
			else if (isalnum(*i)) // name
			{
				while (j != s.end() && isalnum(*j))
				{
					++j;
				}
				tokens->push_back(string(i, j));
			}
			else // special char
			{
				if (j != s.end())
				{
					++j;
				}
				tokens->push_back(string(i, j));
			}
			while (j != s.end() && iswspace(*j))
			{
				++j;
			}
			i = j;
		}
	}
	return tokens;
}

bool SourceParser::match(Tokens::iterator token, string value)
{
	return !value.compare(*token);
}

bool SourceParser::matchName(Tokens::iterator token)
{
	return regex_match(*token, regex("[a-z|A-Z]+[a-z|A-Z|\\d]*"));
}

bool SourceParser::matchInteger(Tokens::iterator token)
{
	return regex_match(*token, regex("\\d+"));
}

void SourceParser::validate(bool match, string errorMsg)
{
	if (!match)
	{
		throw ParserException(errorMsg);
	}
}