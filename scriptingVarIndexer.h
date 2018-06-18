
/*
Written by Antoine Savine in 2018

This code is the strict IP of Antoine Savine

License to use and alter this code for personal and commercial applications
is freely granted to any person or company who purchased a copy of the book

Modern Computational Finance: Scripting for Derivatives and XVA
Jesper Andreasen & Antoine Savine
Wiley, 2018

As long as this comment is preserved at the top of the file
*/

#pragma once

#include "scriptingNodes.h"

#include <map>

class VarIndexer : public Visitor<VarIndexer>
{    
    //	State
	map<string,size_t>	myVarMap;

public:

    using Visitor<VarIndexer>::visit;

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
	void visit( NodeVar& node) 
	{
		auto varIt = myVarMap.find( node.name);
		if( varIt == myVarMap.end()) 
			node.index = myVarMap[node.name] = myVarMap.size();
		else node.index = varIt->second;
	}
};