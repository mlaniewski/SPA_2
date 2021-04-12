#include "With.h"
#include <string>
#include <stdlib.h>
#include <iostream>
#include <sstream>

using namespace std;

With::With()
{
}


With::~With()
{
}


string With::toString() const
{
	stringstream predText;
	predText << "With: operand:" << this->operand << " , lhs var: " << this->lhsVarName << " , lhs prop: " << this->lhsPropertyName << " , rhs var: " << this->rhsVarName << " , rhs prop: " << this->rhsPropertyName << endl;
	return predText.str();
}