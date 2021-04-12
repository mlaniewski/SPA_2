#pragma once
#include<string>

using namespace std;

class With
{
public:
	With();
	~With();

	std::string operand;
	std::string lhsVarName;
	std::string lhsPropertyName;
	std::string rhsVarName;
	std::string rhsPropertyName;
	bool rhsIsProperty;
	bool lhsIsProperty;

	string toString() const;
};