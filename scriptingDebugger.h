
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

#include "quickStack.h"

class Debugger : public constVisitor<Debugger>
{
	string					myPrefix;
	staticStack<string>		myStack;

	//	The main function call from every node visitor
	void debug( const Node& node, const string& nodeId)
	{
		//	One more tab
		myPrefix += '\t';

		//	Visit arguments, right to left
		for( auto it = node.arguments.rbegin(); it != node.arguments.rend(); ++it)
			(*it)->accept( *this);
	
		//	One less tab
		myPrefix.pop_back();

		string str( myPrefix + nodeId);
		if( ! node.arguments.empty())
		{ 			
			str += "(\n";

			//	First argument, pushed last
			str += myStack.top();
			myStack.pop();
			if( node.arguments.size() > 1) str += myPrefix + ",\n";

			//	Args 2 to n-1
			for(size_t i=1; i<node.arguments.size()-1; ++i)
			{
				str += myStack.top() + myPrefix + ",\n";
				myStack.pop();
			}

			if( node.arguments.size() > 1)
			{
				//	Last argument, pushed first
				str += myStack.top();
				myStack.pop();
			}

			//	Close ')'
			str += myPrefix + ')';
		}

		str += '\n';
		myStack.push( move( str));
	}

public:

    using constVisitor<Debugger>::visit;

	//	Access the top of the stack, contains the functional form after the tree is traversed
	string getString() const
	{
		return myStack.top();
	}

	//	All concrete node visitors, visit arguments by default unless overridden

	void visit(const NodeCollect& node)  { debug( node, "COLLECT"); }

	void visit(const NodeUplus& node)  { debug( node, "UPLUS"); }
	void visit(const  NodeUminus& node)  { debug( node, "UMINUS"); }
	void visit(const  NodeAdd& node)  { debug( node, "ADD"); }
	void visit(const NodeSub& node)  { debug( node, "SUBTRACT"); }
	void visit(const NodeMult& node)  { debug( node, "MULT"); }
	void visit(const NodeDiv& node)  { debug( node, "DIV"); }
	void visit(const NodePow& node)  { debug( node, "POW"); }
	void visit(const NodeLog& node)  { debug( node, "LOG"); }
	void visit(const NodeSqrt& node)  { debug( node, "SQRT"); }
	void visit(const NodeMax& node)  { debug( node, "MAX"); }
	void visit(const NodeMin& node)  { debug( node, "MIN"); }
	void visit(const NodeSmooth& node)  { debug( node, "SMOOTH"); }
	
	void visit(const NodeEqual& node)
	{
		string s="EQUALZERO";

		if( !node.discrete)
		{
			s+= "[CONT,EPS=" + to_string( node.eps) + "]";
		}
		else 
		{
			s+= "[DISCRETE,";
			s+= "BOUNDS=" + to_string( node.lb) + "," + to_string( node.rb) + "]";
		}
		debug( node, s); 
	}
	
	void visit(const NodeNot& node)
	{ 
		string s = "NOT";
		debug(node, s);
	}
	
	void visit(const NodeSup& node)
	{
		string s = "GTZERO";
		if( !node.discrete)
		{
			s+= "[CONT,EPS=" + to_string( node.eps) + "]";
		}
		else 
		{
			s+= "[DISCRETE,";
			s+= "BOUNDS=" + to_string( node.lb) + "," + to_string( node.rb) + "]";
		}
		debug( node, s); 
	}
	
	void visit(const NodeSupEqual& node)
	{
		string s = "GTEQUALZERO";
		if( !node.discrete)
		{
			s+= "[CONT,EPS=" + to_string( node.eps) + "]";
		}
		else 
		{
			s+= "[DISCRETE,";
			s+= "BOUNDS=" + to_string( node.lb) + "," + to_string( node.rb) + "]";
		}
		debug( node, s); 
	}

	void visit(const NodeAnd& node)
	{ 
		string s = "AND";
		debug(node, s);
	}

	void visit(const NodeOr& node)
	{ 
		string s = "OR";
		debug(node, s);
	}

	void visit(const NodeAssign& node)  { debug( node, "ASSIGN"); }
	void visit(const NodePays& node)  { debug( node, "PAYS"); }
	void visit(const NodeSpot& node)  { debug( node, "SPOT"); }
	
	void visit(const NodeIf& node)
	{
		string s = "IF";
		s += "[FIRSTELSE=" + to_string( node.firstElse)+"]";

		debug(node, s);
	}
	
	void visit(const NodeTrue& node)  { debug( node, "TRUE"); }
	void visit(const NodeFalse& node)  { debug( node, "FALSE"); }

	void visit(const NodeConst& node)
	{
		debug(node, string( "CONST[")+to_string( node.constVal)+']');
	}
	void visit(const NodeVar& node)
	{
		debug( node, string( "VAR[")+node.name+','+to_string( node.index)+']');
	}
};

