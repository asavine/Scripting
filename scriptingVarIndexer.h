#pragma once

#include "scriptingNodes.h"
#include "scriptingVisitor.h"

#include <map>

class VarIndexer : public Visitor
{
	//	State
	map<string,size_t>	myVarMap;

public:

	//	Access vector of variable names v[index]=name after visit to all events
	vector<string> getVarNames() const
	{
		vector<string> v( myVarMap.size());
		for( auto varMapIt = myVarMap.begin(); varMapIt != myVarMap.end(); ++varMapIt)
		{
			v[varMapIt->second] = varMapIt->first;
		}

		//	C++11: move not copy
		return v;
	}

	//	Variable indexer: build map of names to indices and write indices on variable nodes
	void visitVar( NodeVar& node) override
	{
		auto varIt = myVarMap.find( node.name);
		if( varIt == myVarMap.end()) 
			node.index = myVarMap[node.name] = myVarMap.size();
		else node.index = varIt->second;
	}
};

