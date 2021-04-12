#pragma once
#include "tree.hh"

enum NODETYPE {
	Program,
	Procedure,
	StmtLst,
	Call,
	While,
	If,
	Then,
	Else,
	Assign,
	Expression,
	Variable,
	Constant,
	Operator
};

enum NODEPARAMTYPE {
	Name,
	Caller,
	Callee,
	Cond
};

#include <memory>
class Node;
typedef tree<std::shared_ptr<Node>>::iterator NODE;