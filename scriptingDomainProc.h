
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

#include "functDomain.h"

#include "scriptingNodes.h"

//#define DUMP

#ifdef DUMP
#include <fstream>
#endif

using namespace std;

/*
	Domain processor

	Variable indexer and if processor must have been run prior

	Determines the domains of all variables and expressions. Note that the goal is to identify singletons from continuous intervals,
	not necessarily to compute all intervals accurately. For example, is x's domain is {0} and y's domain is (-inf, inf) then
	xy's domain is {0}, but if x and y have domain (-inf, inf), then xy's domain is (-inf, inf), even if it turns out that y=x.

	This processor sets the "always true" and "always false" flags 
	on the if, equal, superior, not, and and or nodes according to the condition's domain

	In addition, when fuzzy processing is requested, the processor sets the continuous/discrete flag on the equal and superior nodes
	and if discrete, the left and right interpolation bounds

*/

class DomainProcessor : public Visitor<DomainProcessor>
{
	//	Fuzzy?
	const bool				myFuzzy;

	//	Domains for all variables
	vector<Domain>			myVarDomains;

	//	Stack of domains for expressions
	staticStack<Domain>		myDomStack;

	//	Stack of always true/false properties for conditions
	enum CondProp
	{
		alwaysTrue,
		alwaysFalse,
		trueOrFalse
	};
	staticStack<CondProp>	myCondStack;

	//	LHS variable being visited?
	bool					myLhsVar;
    size_t				    myLhsVarIdx;

public:

    using Visitor<DomainProcessor>::visit;

	//	Domains start with the singleton 0
	DomainProcessor( const size_t nVar, const bool fuzzy) : 
        myFuzzy( fuzzy), myVarDomains( nVar, 0.0), myLhsVar( false) {}

	//	Visitors

	//	Expressions

	//	Binaries
	
	void visit( NodeAdd& node) 
	{ 
		visitArguments( node); 
		Domain res = myDomStack[1] + myDomStack[0];
		myDomStack.pop( 2);
		myDomStack.push( move( res)); 
	}
	void visit( NodeSub& node) 
	{ 
		visitArguments( node); 
		Domain res = myDomStack[1] - myDomStack[0];
		myDomStack.pop( 2);
		myDomStack.push( move( res)); 
	}
	void visit( NodeMult& node) 
	{ 
		visitArguments( node); 
		Domain res = myDomStack[1] * myDomStack[0];
		myDomStack.pop( 2);
		myDomStack.push( move( res)); 
	}
	void visit( NodeDiv& node) 
	{ 
		visitArguments( node); 
		Domain res = myDomStack[1] / myDomStack[0];
		myDomStack.pop( 2);
		myDomStack.push( move( res)); 
	}
	void visit( NodePow& node) 
	{ 
		visitArguments( node); 
		Domain res = myDomStack[1].applyFunc2<double(*)(const double, const double)>( 
            pow, 
            myDomStack[0], 
            Interval( Bound::minusInfinity, Bound::plusInfinity));
		myDomStack.pop( 2);
		myDomStack.push( move( res)); 
	}

	//	Unaries
	void visit( NodeUplus& node)  
    { 
        visitArguments( node); 
    }
	void visit( NodeUminus& node)  
    { 
        visitArguments( node); 
        myDomStack.top() = -myDomStack.top(); 
    }

	//	Functions
	void visit( NodeLog& node) 
	{
		visitArguments( node); 
		Domain res = myDomStack.top().applyFunc<double(*)(const double)>( 
            log, 
            Interval( Bound::minusInfinity, Bound::plusInfinity));
		myDomStack.pop();
		myDomStack.push( move( res)); 
	}
	void visit( NodeSqrt& node)
    {
		visitArguments( node); 
		Domain res = myDomStack.top().applyFunc<double(*)(const double)>( 
            sqrt, 
            Interval( 0.0, Bound::plusInfinity));
		myDomStack.pop();
		myDomStack.push( move( res)); 
	}
	void visit( NodeMax& node)
	{
		visitArguments( node); 
		Domain res = myDomStack.top();
		myDomStack.pop();
		for(size_t i=1; i<node.arguments.size(); ++i)
		{
			res = res.dmax( myDomStack.top());
			myDomStack.pop();
		}
		myDomStack.push( move( res)); 
	}
	void visit( NodeMin& node)
	{
		visitArguments( node); 
		Domain res = myDomStack.top();
		myDomStack.pop();
		for(size_t i=1; i<node.arguments.size(); ++i)
		{
			res = res.dmin( myDomStack.top());
			myDomStack.pop();
		}
		myDomStack.push( move( res)); 
	}
	void visit( NodeSmooth& node) 
	{
		visitArguments( node); 
		
		//	Pop eps
		myDomStack.pop();

		//	Makes no sense with non-continuous x
        if (myDomStack[2].discrete())
        {
            throw runtime_error("Smooth called with discrete x");
        }
				
		//	Get min and max val if neg and if pos
		Bound minIfNeg = myDomStack[0].minBound();
		Bound maxIfNeg = myDomStack[0].maxBound();
		Bound minIfPos = myDomStack[1].minBound();
		Bound maxIfPos = myDomStack[1].maxBound();
		Bound minB = min( minIfNeg, minIfPos), maxB = max( maxIfNeg, maxIfPos);

		//	Pop
		myDomStack.pop( 3);

		//	Result
		myDomStack.push( Interval( minB, maxB));
	}

	//	Conditions
	
	void visit( NodeEqual& node) 
	{ 
		visitArguments( node); 
	
		Domain& dom = myDomStack.top();

		//	Always true / false?
		if( !dom.canBeZero())
		{
			node.alwaysTrue = false;
			node.alwaysFalse = true;

			myCondStack.push( alwaysFalse);
		}
		else if( !dom.canBeNonZero())
		{
			node.alwaysTrue = true;
			node.alwaysFalse = false;
			myCondStack.push( alwaysTrue);
		}
		else
		{
			node.alwaysTrue = node.alwaysFalse = false;
			myCondStack.push( trueOrFalse);

			if( myFuzzy)
			{
				//	Continuous or discrete?
				node.discrete = dom.zeroIsDiscrete();

				//	Discrete
				if( node.discrete)
				{
					bool subDomRightOfZero = dom.smallestPosLb( node.rb, true);
					if( !subDomRightOfZero) node.rb = 0.5;

					bool subDomLeftOfZero = dom.biggestNegRb( node.lb, true);
					if( !subDomLeftOfZero) node.lb = -0.5;
				}
			}
		}

		//	Dump domain info to file, comment when not using
#ifdef DUMP
		static int iii = 0;
		++iii;
		{
			ofstream ofs( string( "c:\\temp\\equal") + to_string( iii) + ".txt");
			ofs << "Equality " << iii << endl;
			ofs << "Domain = " << dom << endl;
			ofs << "Node discrete = " << node.discrete << endl;
			if( node.discrete) ofs << "Node lB, rB = " << node.lb << "," << node.rb << endl;
		}
#endif		
		//	End of dump

		myDomStack.pop();
	}

	void visit( NodeNot& node) 
	{ 
		visitArguments( node); 
		CondProp cp = myCondStack.top();
		myCondStack.pop();
		
		if( cp == alwaysTrue)
		{
			node.alwaysTrue = false;
			node.alwaysFalse = true;
			myCondStack.push( alwaysFalse);
		}
		else if( cp == alwaysFalse)
		{
			node.alwaysTrue = true;
			node.alwaysFalse = false;
			myCondStack.push( alwaysTrue);
		}
		else
		{
			node.alwaysTrue = node.alwaysFalse = false;
			myCondStack.push( trueOrFalse);
		}
	}

	//	For visiting superior and supEqual
	template<bool strict, class NodeSup>
	inline void visitSupT( NodeSup& node)
	{
		visitArguments( node); 
	
		Domain& dom = myDomStack.top();

		//	Always true / false?
		if( !dom.canBePositive( strict))
		{
			node.alwaysTrue = false;
			node.alwaysFalse = true;
			myCondStack.push( alwaysFalse);
		}
		else if( !dom.canBeNegative( !strict))
		{
			node.alwaysTrue = true;
			node.alwaysFalse = false;
			myCondStack.push( alwaysTrue);
		}
		//	Can be true or false
		else
		{
			node.alwaysTrue = node.alwaysFalse = false;
			myCondStack.push( trueOrFalse);

			if( myFuzzy)
			{
				//	Continuous or discrete?
				node.discrete = !dom.canBeZero() || dom.zeroIsDiscrete();

				//	Fuzzy logic processing
				if( node.discrete)
				{
					//	Case 1: expr cannot be zero
					if( !dom.canBeZero())
					{
						//		we know we have subdomains on the left and on the right of 0
						dom.smallestPosLb( node.rb, true);
						dom.biggestNegRb( node.lb, true);
					}
					//	Case 2: {0} is a singleton
					else
					{
						if( strict)
						{
							node.lb = 0.0;
							dom.smallestPosLb( node.rb, true);
						}
						else
						{
							node.rb = 0.0;
							dom.biggestNegRb( node.lb, true);
						}
					}
				}
			}
		}

		//	Dump domain info to file, comment when not using
#ifdef DUMP
		static int iii = 0;
		++iii;
		{
			ofstream ofs( string( "c:\\temp\\sup") + (strict? "": "equal") + to_string( iii) + ".txt");
			ofs << "Inequality " << iii << endl;
			ofs << "Domain = " << dom << endl;
			ofs << "Node discrete = " << node.discrete << endl;
			if( node.discrete) ofs << "Node lB, rB = " << node.lb << "," << node.rb << endl;
		}
#endif	
		//	End of dump

		myDomStack.pop();
	}

	void visit( NodeSup& node) 
	{
		visitSupT<true>( node);
	}

	void visit( NodeSupEqual& node) 
	{
		visitSupT<false>( node);
	}

	void visit( NodeAnd& node) 
	{ 
		visitArguments( node); 
		CondProp cp1 = myCondStack.top();
		myCondStack.pop();
		CondProp cp2 = myCondStack.top();
		myCondStack.pop();
		
		if( cp1 == alwaysTrue && cp2 == alwaysTrue)
		{
			node.alwaysTrue = true;
			node.alwaysFalse = false;
			myCondStack.push( alwaysTrue);
		}
		else if( cp1 == alwaysFalse || cp2 == alwaysFalse)
		{
			node.alwaysTrue = false;
			node.alwaysFalse = true;
			myCondStack.push( alwaysFalse);
		}
		else
		{
			node.alwaysTrue = node.alwaysFalse = false;
			myCondStack.push( trueOrFalse);
		}
	}
	void visit( NodeOr& node) 
	{ 
		visitArguments( node); 
		CondProp cp1 = myCondStack.top();
		myCondStack.pop();
		CondProp cp2 = myCondStack.top();
		myCondStack.pop();
		
		if( cp1 == alwaysTrue || cp2 == alwaysTrue)
		{
			node.alwaysTrue = true;
			node.alwaysFalse = false;
			myCondStack.push( alwaysTrue);
		}
		else if( cp1 == alwaysFalse && cp2 == alwaysFalse)
		{
			node.alwaysTrue = false;
			node.alwaysFalse = true;
			myCondStack.push( alwaysFalse);
		}
		else
		{
			node.alwaysTrue = node.alwaysFalse = false;
			myCondStack.push( trueOrFalse);
		}
	}
	
	//	Instructions
	void visit( NodeIf& node) 
	{
		//	Last "if true" statement index
        size_t lastTrueStat = node.firstElse == -1? node.arguments.size()-1: node.firstElse-1;

		//	Visit condition
		node.arguments[0]->accept( *this);

		//	Always true/false?
		CondProp cp = myCondStack.top();
		myCondStack.pop();
		
		if( cp == alwaysTrue)
		{
			node.alwaysTrue = true;
			node.alwaysFalse = false;
			//	Visit "if true" statements
			for(size_t i=1; i<=lastTrueStat; ++i) node.arguments[i]->accept( *this);
		}
		else if( cp == alwaysFalse)
		{
			node.alwaysTrue = false;
			node.alwaysFalse = true;
			//	Visit "if false" statements, if any
			if( node.firstElse != -1)
				for(size_t i=node.firstElse; i<node.arguments.size(); ++i) node.arguments[i]->accept( *this);
		}
		else
		{
			node.alwaysTrue = node.alwaysFalse = false;

			//	Record variable domain before if statements are executed
			vector<Domain> domStore0( node.affectedVars.size());
			for(size_t i=0; i<node.affectedVars.size(); ++i) domStore0[i] = myVarDomains[node.affectedVars[i]];

			//	Execute if statements
			for(size_t i=1; i<=lastTrueStat; ++i) node.arguments[i]->accept( *this);

			//	Record variable domain after if statements are executed
			vector<Domain> domStore1( node.affectedVars.size());
			for(size_t i=0; i<node.affectedVars.size(); ++i) domStore1[i] = move( myVarDomains[node.affectedVars[i]]);

			//	Reset variable domains
			for(size_t i=0; i<node.affectedVars.size(); ++i) myVarDomains[node.affectedVars[i]] = move( domStore0[i]);

			//	Execute else statements if any
			if( node.firstElse != -1)
				for(size_t i=node.firstElse; i<node.arguments.size(); ++i) node.arguments[i]->accept( *this);

			//	Merge domains
			for(size_t i=0; i<node.affectedVars.size(); ++i) myVarDomains[node.affectedVars[i]].addDomain( domStore1[i]);
		}
	}

	void visit( NodeAssign& node) 
	{
		//	Visit the LHS variable
		myLhsVar = true;
		node.arguments[0]->accept( *this);
		myLhsVar = false;

		//	Visit the RHS expression
		node.arguments[1]->accept( *this);

		//	Write RHS domain into variable
		myVarDomains[myLhsVarIdx] = myDomStack.top();
        
		//	Pop
		myDomStack.pop();
	}

	void visit( NodePays& node) 
	{
		//	Visit the LHS variable
		myLhsVar = true;
		node.arguments[0]->accept( *this);
		myLhsVar = false;

		//	Visit the RHS expression
		node.arguments[1]->accept( *this);

		//	Write RHS domain into variable
		
		//	Numeraire domain = (0,+inf)
		static const Domain numDomain( Interval( 0.0, Bound::plusInfinity));

		//	Payment domain
		Domain payDomain = myDomStack.top() / numDomain;

		//	Write
		myVarDomains[myLhsVarIdx] = myVarDomains[myLhsVarIdx] + payDomain;

		//	Pop
		myDomStack.pop();
	}

	//	Variables and constants
	void visit( NodeVar& node) 
	{
		//	LHS?
		if( myLhsVar)	//	Write
		{
			//	Record address in myLhsVarAdr
			myLhsVarIdx = node.index;
		}
		else			//	Read
		{
			//	Push domain onto the stack
			myDomStack.push( myVarDomains[node.index]);
		}
	}

	void visit( NodeConst& node) 
	{
		myDomStack.push( node.constVal);
	}

	//	Scenario related
	void visit( NodeSpot& node) 
	{
		static const Domain realDom( 
            Interval( Bound::minusInfinity, Bound::plusInfinity));
		myDomStack.push( realDom);
	}
};