#pragma once
#include <string>

class Closure
{
public:
	Closure(std::string type, std::string lhs, std::string rhs);
	~Closure();

	std::string type;
	std::string lhs;
	std::string rhs;

	std::string toString() const;
};