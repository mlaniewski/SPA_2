#pragma once
#include <string>
#include <vector>
#include "Predicate.h"
#include "Selector.h"
#include "Closure.h"
#include "Pattern.h"
#include "With.h"
#include <boost/tokenizer.hpp>
#include <list>
#include "AST.h"


using namespace std;

typedef boost::tokenizer<boost::char_separator<char> > stringTokenizer;

class QueriesParser
{
private:
	void printDebugResults();
	void parsePredicate(string predicate);
	void parserResultPart(string resutPart);
	bool checkResultPartElement(string token);
	vector<string> split(string s, char delim);
	string query;
	std::shared_ptr<AST> ast;

	bool icompare(std::string const& a, std::string const& b);

public:
	list<std::string> &results;
	QueriesParser(string query, std::list<std::string> &results, std::shared_ptr<AST> astw) : results(results), ast(astw) {
		//transform(query.begin(), query.end(), query.begin(), ::tolower);
		this->query = query;
	};
	~QueriesParser();

	void parseQuery();


	Selector selector;
	vector<Closure> closureTable;
	vector<Pattern> patternTable;
	vector<Predicate> predTable;
	vector<With> withTable;
};

struct QueryException : public std::exception {
	string msg;
	QueryException(string msg) : msg(msg)
	{
	}
	const char * what() const throw () {
		return msg.c_str();
	}
};