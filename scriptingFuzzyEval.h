
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

#include "scriptingEvaluator.h"

#define EPS 1.0e-12
#define ONEMINUSEPS 0.999999999999

//	The fuzzy evaluator

template <class T>
class FuzzyEvaluator : public EvaluatorBase<T, FuzzyEvaluator>
{
	//	Default smoothing factor for conditions that don't override it
	double						myDefEps;

	//	Stack for the fuzzy evaluation of conditions
	staticStack<T>			    myFuzzyStack;

	//	Temp storage for variables, preallocated for performance
	//	[i][j] = nested if level i variable j
	vector<vector<T>>		    myVarStore0;
	vector<vector<T>>		    myVarStore1;

	//	Nested if level, 0: not in an if, 1: in the outermost if, 2: if nested in another if, etc.
	size_t                      myNestedIfLvl;

	//	Pop the top 2 numbers of the fuzzy condition stack
	pair<T,T> pop2f()
	{
		pair<T,T> res;
		res.first = myFuzzyStack.top();
		myFuzzyStack.pop();
		res.second = myFuzzyStack.top();
		myFuzzyStack.pop();
		return res;
	}

	//	Call Spread (-eps/2,+eps/2)
	static T cSpr( const T x, const double eps)
	{
		const double halfEps = 0.5 * eps;

		if (x < - halfEps) return 0.0;
		else if (x > halfEps) return 1.0;
		else return (x + halfEps) / eps;
	}

	//	Call Spread (lb,rb)
	static T cSpr( const T x, const double lb, const double rb)
	{
		if (x < lb) return 0.0;
		else if (x > rb) return 1.0;
		else return (x - lb) / (rb - lb);
	}

	//	Butterfly (-eps/2,+eps/2)
	static T bFly( const T x, const double eps)
	{
		const double halfEps = 0.5 * eps;

		if (x < - halfEps || x > halfEps) return 0.0;
		else return ( halfEps - fabs( x)) / halfEps;
	}

	//	Butterfly (lb,0,rb)
	static T bFly( const T x, const double lb, const double rb)
	{
		if( x < lb || x > rb) return 0.0;
		else if( x < 0.0) return 1.0 - x / lb;
		else return 1.0 - x / rb;
	}

public:

    using Base = EvaluatorBase<T, ::FuzzyEvaluator>;

    using Base::visit;
    using Base::visitNode;
    using Base::myDstack; 
    using Base::myVariables;

	FuzzyEvaluator( const size_t nVar, const size_t maxNestedIfs, const double defEps = 0)
		: Base( nVar), myDefEps( defEps), myVarStore0( maxNestedIfs), myVarStore1( maxNestedIfs), myNestedIfLvl( 0)
	{
		for( auto& varStore : myVarStore0) varStore.resize( nVar);
		for( auto& varStore : myVarStore1) varStore.resize( nVar);
	}

	//	Copy/Move

	FuzzyEvaluator( const FuzzyEvaluator& rhs) 
		: Base( rhs), myDefEps( rhs.myDefEps), myVarStore0( rhs.myVarStore0.size()), myVarStore1( rhs.myVarStore1.size()), myNestedIfLvl( 0)
	{
		for( auto& varStore : myVarStore0) varStore.resize( myVariables.size());
		for( auto& varStore : myVarStore1) varStore.resize( myVariables.size());	
	}
	FuzzyEvaluator& operator=( const FuzzyEvaluator& rhs) 
	{
		if( this == &rhs) return *this;
		Base::operator=( rhs);
		myDefEps = rhs.myDefEps;
		myVarStore0.resize( rhs.myVarStore0.size());
		myVarStore1.resize( rhs.myVarStore1.size());
		for( auto& varStore : myVarStore0) varStore.resize( myVariables.size());
		for( auto& varStore : myVarStore1) varStore.resize( myVariables.size());	
		myNestedIfLvl = 0;
		return *this;
	}

	FuzzyEvaluator( FuzzyEvaluator&& rhs) 
		: Base( move( rhs)), myDefEps( rhs.myDefEps), myVarStore0( move( rhs.myVarStore0)), myVarStore1( move( rhs.myVarStore1)), myNestedIfLvl( 0) {}
	FuzzyEvaluator& operator=( FuzzyEvaluator&& rhs) 
	{
		Base::operator=( move( rhs));
		myDefEps = rhs.myDefEps;
		myVarStore0 = move( rhs.myVarStore0);
		myVarStore1 = move( rhs.myVarStore1);
		myNestedIfLvl = 0;
		return *this;
	}

	//	(Re)set default smoothing factor
	void setDefEps( const double defEps)
	{
		myDefEps = defEps;
	}

	//	Overriden visitors

	//	If
	void visit( const NodeIf& node) 
	{
		//	Last "if true" statement index
		const size_t lastTrueStat = node.firstElse == -1? node.arguments.size()-1: node.firstElse-1;

		//	Increase nested if level
		++myNestedIfLvl;

		//	Visit the condition and compute its degree of truth dt
		visitNode(*node.arguments[0]);
		const T dt = myFuzzyStack.top();
		myFuzzyStack.pop();

		//	Absolutely true
		if( dt > ONEMINUSEPS)
		{
			//	Eval "if true" statements
			for( size_t i=1; i<=lastTrueStat; ++i) 		visitNode(*node.arguments[i]);

		}
		//	Absolutely false
		else if( dt < EPS)
		{
			//	Eval "if false" statements if any
			if( node.firstElse != -1)
				for( size_t i=node.firstElse; i<node.arguments.size(); ++i) 		visitNode(*node.arguments[i]);

		}
		//	Fuzzy
		else
		{
			//	Record values of variables to be changed
			for( auto idx : node.affectedVars) myVarStore0[myNestedIfLvl-1][idx]=myVariables[idx];

			//	Eval "if true" statements
			for( size_t i=1; i<=lastTrueStat; ++i) 		visitNode(*node.arguments[i]);

			
			//	Record and reset values of variables to be changed
			for( auto idx : node.affectedVars) 
			{
				myVarStore1[myNestedIfLvl-1][idx] = myVariables[idx];
				myVariables[idx] = myVarStore0[myNestedIfLvl-1][idx];
			}

			//	Eval "if false" statements if any
			if( node.firstElse != -1)
				for( size_t i=node.firstElse; i<node.arguments.size(); ++i) 		visitNode(*node.arguments[i]);

			//	Set values of variables to fuzzy values
			for( auto idx : node.affectedVars) myVariables[idx] = dt * myVarStore1[myNestedIfLvl-1][idx] + (1.0-dt) * myVariables[idx];
		}
	
		//	Decrease nested if level
		--myNestedIfLvl;
	}

	//	Conditions
	
	void visit(const NodeTrue& node)
	{
		myFuzzyStack.push( 1.0);
	}
	void visit(const NodeFalse& node)
	{
		myFuzzyStack.push( 0.0);
	}

	//	Equality
	void visit(const NodeEqual& node)
	{
		//	Evaluate expression to be compared to 0
        visitNode(*node.arguments[0]);
        const T expr = myDstack.top();
		myDstack.pop();

		//	Discrete case: 0 is a singleton in expr's domain
		if( node.discrete)
		{
			myFuzzyStack.push( bFly( expr, node.lb, node.rb));
		}
		//	Continuous case: 0 is part of expr's continuous domain
		else 
		{
			//	Effective epsilon: take default unless overwritten on the node
			double eps = node.eps < 0 ? myDefEps : node.eps;

			//	Butterfly
			myFuzzyStack.push( bFly( expr, eps));
		}
	}

	//	Inequality

	//	For visiting superior and supEqual
	void visitComp(const compNode& node)
	{
		//	Evaluate expression to be compared to 0
        visitNode(*node.arguments[0]);
        const T expr = myDstack.top();
		myDstack.pop();

		//	Discrete case: 
		//	Either 0 is a singleton in expr's domain
		//	Or 0 is not part of expr's domain, but expr's domain has subdomains left and right of 0
		//		otherwise the condition would be always true/false
		if( node.discrete)
		{
			//	Call spread on the right
			myFuzzyStack.push( cSpr( expr, node.lb, node.rb));
		}
		//	Continuous case: 0 is part of expr's continuous domain
		else
		{
			//	Effective epsilon: take default unless overwritten on the node
			const double eps = node.eps < 0 ? myDefEps : node.eps;

			//	Call Spread
			myFuzzyStack.push( cSpr( expr, eps));
		}
	}

	void visit(const NodeSup& node)
	{
        visitComp( node);
	}
	
    void visit(const NodeSupEqual& node)
	{
        visitComp( node);
	}
	
	//	Negation
	void visitNot(const NodeNot& node)
	{
        visitNode(*node.arguments[0]);
        myFuzzyStack.top() = 1.0 - myFuzzyStack.top();
	}

	//	Combinators
	//	Hard coded proba stlye and->dt(lhs)*dt(rhs), or->dt(lhs)+dt(rhs)-dt(lhs)*dt(rhs)
	void visit(const NodeAnd& node)
	{ 
        visitNode(*node.arguments[0]);
        visitNode(*node.arguments[1]);
        const auto args=pop2f();
		myFuzzyStack.push( args.first * args.second); 
	}
	void visit(const NodeOr& node)
	{ 
        visitNode(*node.arguments[0]);
        visitNode(*node.arguments[1]);
        const auto args=pop2f();
		myFuzzyStack.push( args.first + args.second - args.first * args.second); 
	}
};