#include "Pattern.h"
#include <stdlib.h>
#include <iostream>
#include <sstream>

Pattern::Pattern(std::string varName, std::string lhs, std::string rhs)
{
	this->varName = varName;
	this->lhs = lhs;
	this->rhs = rhs;
}


Pattern::~Pattern()
{
}

string Pattern::toString() const
{
	stringstream predText;
	predText << "Var name: " << this->varName << ", lhs: " << this->lhs << ", rhs: " << this->rhs << endl;
	return predText.str();
}