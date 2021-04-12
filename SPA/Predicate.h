#pragma once
#include <string>

using namespace std;

class Predicate {
private:
	string type;  // assign, string, constant, variable, prog_line, procedure, stmt#, any
	string value;
public:
	Predicate();
	Predicate(string type, string value);
	virtual ~Predicate();
	string getType();
	void setType(string type);
	string getValue();
	void setValue(string value);
	string toString() const;
};