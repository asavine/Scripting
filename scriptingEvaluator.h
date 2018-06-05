
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

#include <iostream>

#include "scriptingNodes.h"
#include "scriptingVisitor.h"
#include "scriptingScenarios.h"

#include <vector>
#include "quickStack.h"

template <class T>
class Evaluator : public constVisitor
{

protected:

	//	State
	vector<T>				    myVariables;

	//	Stacks
	staticStack<T>			    myDstack;
    staticStack<char>		    myBstack;

	//	LHS variable being visited?
	bool						myLhsVar;
	T*						    myLhsVarAdr;

	//	Reference to current scenario
	const Scenario<T>*			myScenario;

	//	Index of current event
	size_t					    myCurEvt;

	//	Visit arguments, right to left
	void evalArgsRL( const Node& node)
	{
        const auto end = node.arguments.rend();
		for( auto it = node.arguments.rbegin(); it != end; ++it) 
			(*it)->acceptVisitor( *this);
	}

	//	Pop the top 2 numbers of the number stack
	pair<T,T> pop2()
	{
		pair<T,T> res;
		res.first = myDstack.top();
		myDstack.pop();
		res.second = myDstack.top();
		myDstack.pop();
		return res;
	}

	//	Pop the top 2 bools of the bool stack
	pair<bool,bool> pop2b()
	{
		pair<bool,bool> res;
		res.first = myBstack.top();
		myBstack.pop();
		res.second = myBstack.top();
		myBstack.pop();
		return res;
	}

public:

	//	Constructor, nVar = number of variables, from Product after parsing and variable indexation
	Evaluator( const size_t nVar) : myVariables( nVar) {}

	virtual ~Evaluator() {}

	//	Copy/Move

	Evaluator( const Evaluator& rhs) : myVariables( rhs.myVariables) {}
	Evaluator& operator=( const Evaluator& rhs) 
	{
		if( this == &rhs) return *this;
		myVariables = rhs.myVariables;
		return *this;
	}

	Evaluator( Evaluator&& rhs) : myVariables( move( rhs.myVariables)) {}
	Evaluator& operator=( Evaluator&& rhs) 
	{
		myVariables = move( rhs.myVariables);
		return *this;
	}

	//	(Re-)initialize before evaluation in each scenario
	virtual void init()
	{
		for( auto& varIt : myVariables) varIt = 0.0;
		//	Stacks should be empty, if this is not the case the empty them
		//		without affecting capacity for added performance
		while( !myDstack.empty()) myDstack.pop();
		while( !myBstack.empty()) myBstack.pop();
		myLhsVar = false;
		myLhsVarAdr = nullptr;
	}

	//	Accessors

	//	Access to variable values after evaluation
	const vector<T>& varVals() const
	{
		return myVariables;
	}

	//	Set generated scenarios and current event

	//	Set reference to current scenario
	void setScenario( const Scenario<T>* scen)
	{
		myScenario = scen;
	}

	//	Set index of current event
	void setCurEvt( const size_t curEvt)
	{
		myCurEvt = curEvt;
	}

	//	Visitors

	//	Expressions

	//	Binaries
	
	void visitAdd( const NodeAdd& node) override
	{ 
		evalArgsRL( node); 
		const auto args=pop2(); 
		myDstack.push( args.first+args.second); 
	}
	void visitSubtract( const NodeSubtract& node) override
	{ 
		evalArgsRL( node); 
		const auto args=pop2(); 
		myDstack.push( args.first-args.second); 
	}
	void visitMult( const NodeMult& node) override
	{ 
		evalArgsRL( node); 
		const auto args=pop2(); 
		myDstack.push( args.first*args.second); 
	}
	void visitDiv( const NodeDiv& node) override
	{ 
		evalArgsRL( node); 
		const auto args=pop2(); 
		myDstack.push( args.first/args.second); 
	}
	void visitPow( const NodePow& node) override
	{ 
		evalArgsRL( node); 
		const auto args=pop2(); 
		myDstack.push( pow( args.first, args.second)); 
	}

	//	Unaries
	void visitUplus( const NodeUplus& node) override { evalArgsRL( node); }
	void visitUminus( const NodeUminus& node) override { evalArgsRL( node); myDstack.top() *= -1; }

	//	Functions
	void visitLog( const NodeLog& node) override
	{
		evalArgsRL( node);

		const T res = log( myDstack.top());
		myDstack.pop();
		
		myDstack.push( res);
	}
	void visitSqrt( const NodeSqrt& node) override
	{
		evalArgsRL( node);

		const T res = sqrt( myDstack.top());
		myDstack.pop();
		
		myDstack.push( res);
	}
	void visitMax( const NodeMax& node) override
	{
		evalArgsRL( node);
		
		T M = myDstack.top();
		myDstack.pop();
		
        const size_t n = node.arguments.size();
		for( size_t i=1; i<n; ++i)
		{
			M = max( M, myDstack.top());
			myDstack.pop();
		}
		
		myDstack.push( M);
	}
	void visitMin( const NodeMin& node) override
	{
		evalArgsRL( node);
		
		T m = myDstack.top();
		myDstack.pop();
		
        const size_t n = node.arguments.size();
		for( size_t i=1; i<n; ++i)
		{
			m = min( m, myDstack.top());
			myDstack.pop();
		}
		
		myDstack.push( m);
	}
	void visitSmooth( const NodeSmooth& node) override
	{
		//	Eval the condition
		node.arguments[0]->acceptVisitor( *this);
		const T x = myDstack.top();
		myDstack.pop();

		//	Eval the epsilon
		node.arguments[3]->acceptVisitor( *this);
		const T halfEps = 0.5*myDstack.top();
		myDstack.pop();

		//	Left
		if( x < -halfEps) node.arguments[2]->acceptVisitor( *this);

		//	Right
		else if( x > halfEps) node.arguments[1]->acceptVisitor( *this);

		//	Fuzzy
		else
		{
			node.arguments[1]->acceptVisitor( *this);
			const T vPos = myDstack.top();
			myDstack.pop();

			node.arguments[2]->acceptVisitor( *this);
			const T vNeg = myDstack.top();
			myDstack.pop();

			myDstack.push( vNeg + 0.5 * (vPos - vNeg) / halfEps * (x + halfEps));
		}
	}

	//	Conditions
	
	#define EPS 1.0e-12
	#define ONEMINUSEPS 0.999999999999

	void visitTrue( const NodeTrue& node) override
	{
		myBstack.push( true);
	}
	void visitFalse( const NodeFalse& node) override
	{
		myBstack.push( false);
	}

	void visitEqual( const NodeEqual& node) override
	{
		evalArgsRL( node); 
		const T res = myDstack.top();
		myDstack.pop();
		myBstack.push( fabs( res) < EPS);
	}
	void visitNot( const NodeNot& node) override
	{ 
		evalArgsRL( node); 
		const bool res = myBstack.top();
		myBstack.pop();
		myBstack.push( !res); 
	}
	void visitSuperior( const NodeSuperior& node) override
	{ 
		evalArgsRL( node); 
		const T res = myDstack.top();
		myDstack.pop();
		myBstack.push( res > EPS); 
	}
	void visitSupEqual( const NodeSupEqual& node) override
	{ 
		evalArgsRL( node); 
		const T res = myDstack.top();
		myDstack.pop();
		myBstack.push( res > -EPS); 
	}
	void visitAnd( const NodeAnd& node) override
	{ 
		evalArgsRL( node); 
		const auto args=pop2b(); 
		myBstack.push( args.first && args.second); 
	}
	void visitOr( const NodeOr& node) override
	{ 
		evalArgsRL( node); 
		const auto args=pop2b(); 
		myBstack.push( args.first || args.second); 
	}
	
	//	Instructions
	void visitIf( const NodeIf& node) override
	{
		//	Eval the condition
		node.arguments[0]->acceptVisitor( *this);
		
		//	Pick the result
		const bool isTrue = myBstack.top();
		myBstack.pop();

		//	Evaluate the relevant statements
		if( isTrue)
		{
			const auto lastTrue = node.firstElse == -1? node.arguments.size()-1: node.firstElse-1;
			for( auto i=1; i<=lastTrue; ++i)
			{
				node.arguments[i]->acceptVisitor( *this);
			}
		}
		else if( node.firstElse != -1)
		{
            const size_t n = node.arguments.size();
			for( auto i=node.firstElse; i<n; ++i)
			{
				node.arguments[i]->acceptVisitor( *this);
			}
		}
	}

	void visitAssign( const NodeAssign& node) override
	{
		//	Visit the LHS variable
		myLhsVar = true;
		node.arguments[0]->acceptVisitor( *this);
		myLhsVar = false;

		//	Visit the RHS expression
		node.arguments[1]->acceptVisitor( *this);
	
		//	Write result into variable
		*myLhsVarAdr = myDstack.top();
		myDstack.pop();
	}

	void visitPays( const NodePays& node) override
	{
		//	Visit the LHS variable
		myLhsVar = true;
		node.arguments[0]->acceptVisitor( *this);
		myLhsVar = false;

		//	Visit the RHS expression
		node.arguments[1]->acceptVisitor( *this);
	
		//	Write result into variable
		*myLhsVarAdr += myDstack.top() / (*myScenario)[myCurEvt].numeraire;
		myDstack.pop();
	}

	//	Variables and constants
	void visitVar( const NodeVar& node) override
	{
		//	LHS?
		if( myLhsVar)	//	Write
		{
			//	Record address in myLhsVarAdr
			myLhsVarAdr = &myVariables[node.index];
		}
		else			//	Read
		{
			//	Push value onto the stack
			myDstack.push( myVariables[node.index]);
		}
	}

	void visitConst( const NodeConst& node) override
	{
		myDstack.push( node.constVal);
	}

	//	Scenario related
	void visitSpot( const NodeSpot& node) override
	{
		myDstack.push( (*myScenario)[myCurEvt].spot);
	}
};

