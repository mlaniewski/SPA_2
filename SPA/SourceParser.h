#pragma once
#include "Builder.h"
#include<memory>
#include<vector>

typedef std::vector<std::string> Tokens;

class SourceParser
{
public:
	SourceParser(Builder &builder);
	~SourceParser();
	void parse(std::string filename);
private:
	Builder & builder;
	Tokens::iterator token;
	string procName;
	static std::shared_ptr<Tokens> createTokenVector(std::string filename);
	static bool match(Tokens::iterator token, string value);
	static bool matchName(Tokens::iterator token);
	static bool matchInteger(Tokens::iterator token);
	static void validate(bool match, string errorMsg);
	NODE parseProcedure();
	NODE parseStmtLst(NODETYPE stmtLstType = StmtLst);
	NODE parseStmt();
	NODE parseCall();
	NODE parseWhile();
	NODE parseIf();
	NODE parseAssignment();
	NODE parseExpression();
	NODE parseTerm();
	NODE parseFactor();
};

struct ParserException : public exception {
	string msg;
	ParserException(string msg) : msg(msg)
	{
	}
	const char * what() const throw () {
		return msg.c_str();
	}
};
