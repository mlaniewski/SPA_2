#include "Predicate.h"
#include <stdlib.h>
#include <iostream>
#include <sstream>

Predicate::Predicate()
{
}

Predicate::Predicate(string type, string value) {
	this->type = type;
	this->value = value;
}


Predicate::~Predicate()
{
}

string Predicate::getType() {
	return type;
}

void Predicate::setType(string type) {
	this->type = type;
}

string Predicate::getValue() {
	return value;
}

void Predicate::setValue(string value) {
	this->value = value;
}

string Predicate::toString() const
{
	stringstream predText;
	predText << "Predicate: " << this->type << " " << this->value << endl;
	return predText.str();
}