#pragma once
#include<string>

using namespace std;

class Pattern
{
public:
	Pattern(std::string varName, std::string lhs, std::string rhs);
	~Pattern();

	std::string varName;
	std::string lhs;
	std::string rhs;

	string toString() const;
};