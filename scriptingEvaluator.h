
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
#include "scriptingScenarios.h"

#include <vector>
#include "quickStack.h"

template <class T, template <typename> class EVAL>
class EvaluatorBase : public constVisitor<EVAL<T>>
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

    using constVisitor<EVAL<T>>::visit;
    using constVisitor<EVAL<T>>::visitNode;

	//	Constructor, nVar = number of variables, from Product after parsing and variable indexation
	EvaluatorBase( const size_t nVar) : myVariables( nVar) {}

	//	Copy/Move

    EvaluatorBase( const EvaluatorBase& rhs) : myVariables( rhs.myVariables) {}
    EvaluatorBase& operator=( const EvaluatorBase& rhs)
	{
		if( this == &rhs) return *this;
		myVariables = rhs.myVariables;
		return *this;
	}

    EvaluatorBase(EvaluatorBase&& rhs) : myVariables( move( rhs.myVariables)) {}
    EvaluatorBase& operator=(EvaluatorBase&& rhs)
	{
		myVariables = move( rhs.myVariables);
		return *this;
	}

	//	(Re-)initialize before evaluation in each scenario
	void init()
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

    template<class OP> 
    void visitBinary(const exprNode& node, OP op)
    {
        visitNode(*node.arguments[0]);
        visitNode(*node.arguments[1]);
        op(myDstack[1], myDstack.top());
        myDstack.pop();
    }
	
	void visit(const NodeAdd& node)
	{ 
        visitBinary(node, [](T& x, const T y) { x += y; });
	}
	void visit(const NodeSub& node)
	{ 
        visitBinary(node, [](T& x, const T y) { x -= y; });
    }
	void visit(const NodeMult& node)
	{ 
        visitBinary(node, [](T& x, const T y) { x *= y; });
    }
	void visit(const NodeDiv& node)
	{ 
        visitBinary(node, [](T& x, const T y) { x /= y; });
    }
	void visit(const NodePow& node)
	{ 
        visitBinary(node, [](T& x, const T y) { x = pow(x, y); });
    }
    void visit(const NodeMax& node)
    {
        visitBinary(node, [](T& x, const T y) { if (x < y) x = y; });
    }
    void visit(const NodeMin& node)
    {
        visitBinary(node, [](T& x, const T y) { if (x > y) x = y; });
    }

	//	Unaries
    template<class OP>
    inline void visitUnary(const exprNode& node, OP op)
    {
        visitNode(*node.arguments[0]);
        op(myDstack.top());
    }

	void visit(const NodeUplus& node)
    { 
        visitUnary(node, [](T& x) { });
    }
	void visit(const NodeUminus& node)
    { 
        visitUnary(node, [](T& x) { x = -x; });
    }

	//	Functions
	void visit(const NodeLog& node)
	{
        visitUnary(node, [](T& x) { x = log(x); });
    }
	void visit(const NodeSqrt& node)
	{
        visitUnary(node, [](T& x) { x = sqrt(x); });
    }

    //  Multies
    void visit(const NodeSmooth& node)
	{
		//	Eval the condition
        visitNode(*node.arguments[0]);
        const T x = myDstack.top();
		myDstack.pop();

		//	Eval the epsilon
        visitNode(*node.arguments[3]);
        const T halfEps = 0.5*myDstack.top();
		myDstack.pop();

		//	Left
		if( x < -halfEps)         visitNode(*node.arguments[2]);


		//	Right
		else if( x > halfEps)       visitNode(*node.arguments[1]);


		//	Fuzzy
		else
		{
            visitNode(*node.arguments[1]);
            const T vPos = myDstack.top();
			myDstack.pop();

            visitNode(*node.arguments[2]);
            const T vNeg = myDstack.top();
			myDstack.pop();

			myDstack.push( vNeg + 0.5 * (vPos - vNeg) / halfEps * (x + halfEps));
		}
	}

	//	Conditions
    template<class OP>
    inline void visitCondition(const boolNode& node, OP op)
    {
        visitNode(*node.arguments[0]);
        myBstack.push(op(myDstack.top()));
        myDstack.pop();
    }

	void visit(const NodeEqual& node)
	{
        visitCondition(node, [](const T x) { return x == 0; });
    }
	void visit(const NodeSup& node)
	{ 
        visitCondition(node, [](const T x) { return x > 0; });
    }
	void visit(const NodeSupEqual& node)
	{ 
        visitCondition(node, [](const T x) { return x >= 0; });
    }

	void visit(const NodeAnd& node)
	{ 
        visitNode(*node.arguments[0]);
        if (myBstack.top())
        {
            myBstack.pop();
            visitNode(*node.arguments[1]);
        }
    }
	void visit(const NodeOr& node)
	{ 
        visitNode(*node.arguments[0]);
        if (!myBstack.top())
        {
            myBstack.pop();
            visitNode(*node.arguments[1]);
        }
    }
    void visit(const NodeNot& node)
    {
        visitNode(*node.arguments[0]);
        auto& b = myBstack.top();
        b = !b;
    }

	
	//	Instructions
	void visit(const NodeIf& node)
	{
		//	Eval the condition
        visitNode(*node.arguments[0]);

		//	Pick the result
		const auto isTrue = myBstack.top();
		myBstack.pop();

		//	Evaluate the relevant statements
		if( isTrue)
		{
			const auto lastTrue = node.firstElse == -1? node.arguments.size()-1: node.firstElse-1;
			for(unsigned i=1; i<=lastTrue; ++i)
			{
                visitNode(*node.arguments[i]);
            }
		}
		else if( node.firstElse != -1)
		{
            const size_t n = node.arguments.size();
			for(unsigned i=node.firstElse; i<n; ++i)
			{
                visitNode(*node.arguments[i]);
            }
		}
	}

	void visit(const NodeAssign& node)
	{
        const auto varIdx = downcast<NodeVar>(node.arguments[0])->index;

		//	Visit the RHS expression
        visitNode(*node.arguments[1]);

		//	Write result into variable
        myVariables[varIdx] = myDstack.top();
		myDstack.pop();
	}

	void visit(const NodePays& node)
	{
        const auto varIdx = downcast<NodeVar>(node.arguments[0])->index;

        //	Visit the RHS expression
        visitNode(*node.arguments[1]);

        //	Write result into variable
        myVariables[varIdx] += myDstack.top() / (*myScenario)[myCurEvt].numeraire;
        myDstack.pop();
	}

	//	Variables and constants
	void visit(const NodeVar& node)
	{
		//	Push value onto the stack
		myDstack.push( myVariables[node.index]);
	}

	void visit(const NodeConst& node)
	{
		myDstack.push( node.constVal);
	}

    void visit(const NodeTrue& node)
    {
        myBstack.push(true);
    }
    void visit(const NodeFalse& node)
    {
        myBstack.push(false);
    }

	//	Scenario related
	void visit(const NodeSpot& node)
	{
		myDstack.push( (*myScenario)[myCurEvt].spot);
	}
};

//  Concrete Evaluator
template <class T>
class Evaluator : public EvaluatorBase<T, Evaluator>
{

public:

    using Base = EvaluatorBase<T, ::Evaluator>;

    Evaluator(const size_t nVar) : Base(nVar) {}
    Evaluator(const Evaluator& rhs) : Base(rhs) {}
    Evaluator(Evaluator&& rhs) : Base(move(rhs)) {}
    Evaluator& operator=(const Evaluator& rhs)
    {
        Base::operator=(rhs);
    }
    Evaluator& operator=(const Evaluator&& rhs)
    {
        Base::operator=(move(rhs));
    }
};