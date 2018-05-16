
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

//	If processor
//	Identifies variables affected in if and else statements
//		including those affected in nested ifs
//	Puts on the if node the indices of affected variables
//		and keeps track of the maximum number of nested ifs
//	Note the var indexer must have been run first

#include <set>
#include <memory>
#include <iterator>

#include "scriptingVisitor.h"
#include "quickStack.h"

class IfProcessor : public Visitor
{
	//	Top of the stack: current (possibly nested) if being processed
	//	Each element in stack: set of indices of variables modified by the corresponding if and nested ifs
	quickStack<set<size_t>>	    myVarStack;

	//	Nested if level, 0: not in an if, 1: in the outermost if, 2: if nested in another if, etc.
    size_t					    myNestedIfLvl;

	//	Keep track of the maximum number of nested ifs
    size_t					    myMaxNestedIfs;

public:

	IfProcessor() : myNestedIfLvl( 0), myMaxNestedIfs( 0) {}

	//	Access to the max nested ifs after the prcessor is run
	const size_t maxNestedIfs() const
	{ 
		return myMaxNestedIfs;
	}

	//	Visitors

	void visitIf( NodeIf& node) override
	{
		//	Increase nested if level
		++myNestedIfLvl;
		if( myNestedIfLvl > myMaxNestedIfs) myMaxNestedIfs = myNestedIfLvl;

		//	Put new element on the stack
		myVarStack.push( set<size_t>());

		//	Visit arguments, excluding condition
		for(size_t i = 1; i < node.arguments.size(); ++i) node.arguments[i]->acceptVisitor( *this);

		//	Copy the top of the stack into the node
		node.myAffectedVars.clear();
		copy( myVarStack.top().begin(), myVarStack.top().end(), back_inserter( node.myAffectedVars));

		//	Pop
		myVarStack.pop();

		//	Decrease nested if level
		--myNestedIfLvl;

		//	If not outmost if, copy changed vars into the immediately outer if 
		//	Variables changed in a nested if are also changed in the englobing if
		if( myNestedIfLvl) copy( node.myAffectedVars.begin(), node.myAffectedVars.end(), inserter( myVarStack.top(), myVarStack.top().end()));
	}

	void visitAssign( NodeAssign& node) override
	{
		//	Visit the lhs var
		if( myNestedIfLvl) node.arguments[0]->acceptVisitor( *this);
	}

	void visitPays( NodePays& node) override
	{
		//	Visit the lhs var
		if( myNestedIfLvl) node.arguments[0]->acceptVisitor( *this);
	}

	void visitVar( NodeVar& node) override
	{
		//	Insert the var idx
		if( myNestedIfLvl) myVarStack.top().insert( node.index);
	}
};
