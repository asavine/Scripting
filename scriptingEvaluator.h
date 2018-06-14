
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

	//	Reference to current scenario
	const Scenario<T>*			myScenario;

	//	Index of current event
	size_t					    myCurEvt;

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
		myDstack.reset();
		myBstack.reset();
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

    template<class NODE, class OP> 
    inline void visitBinary(const NODE& node, OP op)
    {
        node.arguments[0]->acceptVisitor(*this);
        node.arguments[1]->acceptVisitor(*this);
        op(myDstack[1], myDstack.top());
        myDstack.pop();
    }
	
	void visitAdd( const NodeAdd& node) override
	{ 
        visitBinary(node, [](T& x, const T y) { x += y; });
	}
	void visitSubtract( const NodeSubtract& node) override
	{ 
        visitBinary(node, [](T& x, const T y) { x -= y; });
    }
	void visitMult( const NodeMult& node) override
	{ 
        visitBinary(node, [](T& x, const T y) { x *= y; });
    }
	void visitDiv( const NodeDiv& node) override
	{ 
        visitBinary(node, [](T& x, const T y) { x /= y; });
    }
	void visitPow( const NodePow& node) override
	{ 
        visitBinary(node, [](T& x, const T y) { x = pow(x, y); });
    }
    void visitMax(const NodeMax& node) override
    {
        visitBinary(node, [](T& x, const T y) { if (x < y) x = y; });
    }
    void visitMin(const NodeMin& node) override
    {
        visitBinary(node, [](T& x, const T y) { if (x > y) x = y; });
    }

	//	Unaries
    template<class NODE, class OP>
    inline void visitUnary(const NODE& node, OP op)
    {
        node.arguments[0]->acceptVisitor(*this);
        op(myDstack.top());
    }

	void visitUplus( const NodeUplus& node) override 
    { 
        visitUnary(node, [](T& x) { });
    }
	void visitUminus( const NodeUminus& node) override 
    { 
        visitUnary(node, [](T& x) { x = -x; });
    }

	//	Functions
	void visitLog( const NodeLog& node) override
	{
        visitUnary(node, [](T& x) { x = log(x); });
    }
	void visitSqrt( const NodeSqrt& node) override
	{
        visitUnary(node, [](T& x) { x = sqrt(x); });
    }

    //  Multies
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
    template<class NODE, class OP>
    inline void visitCondition(const NODE& node, OP op)
    {
        node.arguments[0]->acceptVisitor(*this);
        myBstack.push(op(myDstack.top()));
        myDstack.pop();
    }

	void visitEqual( const NodeEqual& node) override
	{
        visitCondition(node, [](const T x) { return x == 0; });
    }
	void visitSuperior( const NodeSuperior& node) override
	{ 
        visitCondition(node, [](const T x) { return x > 0; });
    }
	void visitSupEqual( const NodeSupEqual& node) override
	{ 
        visitCondition(node, [](const T x) { return x >= 0; });
    }

	void visitAnd( const NodeAnd& node) override
	{ 
        node.arguments[0]->acceptVisitor(*this);
        if (myBstack.top())
        {
            myBstack.pop();
            node.arguments[1]->acceptVisitor(*this);
        }
    }
	void visitOr( const NodeOr& node) override
	{ 
        node.arguments[0]->acceptVisitor(*this);
        if (!myBstack.top())
        {
            myBstack.pop();
            node.arguments[1]->acceptVisitor(*this);
        }
    }
    void visitNot(const NodeNot& node) override
    {
        node.arguments[0]->acceptVisitor(*this);
        auto& b = myBstack.top();
        b = !b;
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
        const auto varIdx = static_cast<NodeVar*>(node.arguments[0].get())->index;

		//	Visit the RHS expression
		node.arguments[1]->acceptVisitor( *this);
	
		//	Write result into variable
        myVariables[varIdx] = myDstack.top();
		myDstack.pop();
	}

	void visitPays( const NodePays& node) override
	{
        const auto varIdx = static_cast<NodeVar*>(node.arguments[0].get())->index;

        //	Visit the RHS expression
        node.arguments[1]->acceptVisitor(*this);

        //	Write result into variable
        myVariables[varIdx] = myDstack.top() / (*myScenario)[myCurEvt].numeraire;
        myDstack.pop();
	}

	//	Variables and constants
	void visitVar( const NodeVar& node) override
	{
		//	Push value onto the stack
		myDstack.push( myVariables[node.index]);
	}

	void visitConst( const NodeConst& node) override
	{
		myDstack.push( node.constVal);
	}

    void visitTrue(const NodeTrue& node) override
    {
        myBstack.push(true);
    }
    void visitFalse(const NodeFalse& node) override
    {
        myBstack.push(false);
    }

	//	Scenario related
	void visitSpot( const NodeSpot& node) override
	{
		myDstack.push( (*myScenario)[myCurEvt].spot);
	}
};

