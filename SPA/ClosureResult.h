#pragma once
#include <string>
#include <map>
#include <set>
using namespace std;


class ClosureResult
{
public:
	ClosureResult();
	~ClosureResult();

	string resultType; // BOOL, SET, MAP
	bool boolResult;
	string p;
	string q;
	set<string> vals;
	map<string, set<string>> pq;
	map<string, set<string>> qp;
};