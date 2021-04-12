#include "Closure.h"
#include <iostream>
#include <sstream>
#include <stdlib.h>

using namespace std;

Closure::Closure(std::string type, std::string lhs, std::string rhs)
{
	this->type = type;
	this->lhs = lhs;
	this->rhs = rhs;
}


Closure::~Closure()
{
}

string Closure::toString() const
{
	stringstream predText;
	predText << "Type: " << this->type << " lhs: " << this->lhs << " rhs: " << this->rhs << endl;
	return predText.str();
}