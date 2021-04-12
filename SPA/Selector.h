#pragma once
#include <string>
#include <vector>
#include <map>
#include <set>

class Selector
{
public:
	Selector();
	~Selector();

	void addVariable(std::string variableName);

	std::string type;
	std::vector<std::string> variables;
	std::map < std::string, std::set<std::string> > variableProperties;

private:

};