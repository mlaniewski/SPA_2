#include "Selector.h"
#include <iostream>

using namespace std;

Selector::Selector()
{
}

Selector::~Selector()
{

}

void Selector::addVariable(string variableName) {
	this->variables.push_back(variableName);
}