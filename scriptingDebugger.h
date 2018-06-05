
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
#include "scriptingVisitor.h"

#include "quickStack.h"

class Debugger : public constVisitor
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
			(*it)->acceptVisitor( *this);
	
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

	//	Access the top of the stack, contains the functional form after the tree is traversed
	string getString() const
	{
		return myStack.top();
	}

	//	All concrete node visitors, visit arguments by default unless overridden

	void visitCollect( const NodeCollect& node) override { debug( node, "COLLECT"); }

	void visitUplus( const NodeUplus& node) override { debug( node, "UPLUS"); }
	void visitUminus( const NodeUminus& node) override { debug( node, "UMINUS"); }
	void visitAdd( const NodeAdd& node) override { debug( node, "ADD"); }
	void visitSubtract( const NodeSubtract& node) override { debug( node, "SUBTRACT"); }
	void visitMult( const NodeMult& node) override { debug( node, "MULT"); }
	void visitDiv( const NodeDiv& node) override { debug( node, "DIV"); }
	void visitPow( const NodePow& node) override { debug( node, "POW"); }
	void visitLog( const NodeLog& node) override { debug( node, "LOG"); }
	void visitSqrt( const NodeSqrt& node) override { debug( node, "SQRT"); }
	void visitMax( const NodeMax& node) override { debug( node, "MAX"); }
	void visitMin( const NodeMin& node) override { debug( node, "MIN"); }
	void visitSmooth( const NodeSmooth& node) override { debug( node, "SMOOTH"); }
	
	void visitEqual( const NodeEqual& node) override 
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
	
	void visitNot( const NodeNot& node) override 
	{ 
		string s = "NOT";
		debug(node, s);
	}
	
	void visitSuperior( const NodeSuperior& node) override 
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
	
	void visitSupEqual( const NodeSupEqual& node) override 
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

	void visitAnd( const NodeAnd& node) override 
	{ 
		string s = "AND";
		debug(node, s);
	}

	void visitOr( const NodeOr& node) override 
	{ 
		string s = "OR";
		debug(node, s);
	}

	void visitAssign( const NodeAssign& node) override { debug( node, "ASSIGN"); }
	void visitPays( const NodePays& node) override { debug( node, "PAYS"); }
	void visitSpot( const NodeSpot& node) override { debug( node, "SPOT"); }
	
	void visitIf( const NodeIf& node) override 
	{
		string s = "IF";
		s += "[FIRSTELSE=" + to_string( node.firstElse)+"]";

		debug(node, s);
	}
	
	void visitTrue( const NodeTrue& node) override { debug( node, "TRUE"); }
	void visitFalse( const NodeFalse& node) override { debug( node, "FALSE"); }

	void visitConst( const NodeConst& node) override 
	{
		debug(node, string( "CONST[")+to_string( node.constVal)+']');
	}
	void visitVar( const NodeVar& node) override 
	{
		debug( node, string( "VAR[")+node.name+','+to_string( node.index)+']');
	}
};

