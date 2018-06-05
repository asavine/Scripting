
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

#include "scriptingVisitor.h"

//	ConstCond processor
//	Processes all constant (always true/false) conditions and conditional statements
//	Remove all the if and condition nodes that are always true or always false
//	The domain proc must have been run first, so always true/false flags are properly set inside the nodes
//	The always true/false if nodes are replaced by collections of statements to be evaluated
//	The always true/false conditions are replaced by true/false nodes

class ConstCondProcessor : public Visitor
{
	//	The (unique) pointer on the node currently being visited 
	ExprTree*			myCurrent;

	//	Overriden default visitor sets myCurrent when visiting arguments
	void visitArguments( Node& node) override
	{
		for( auto& arg : node.arguments) 
		{
			myCurrent = &arg;
			arg->acceptVisitor( *this);
		}
	}

public:

	//	This patricular visitor modifies the structure of the tree, hence it must be called only
	//		with this method from the top of every tree, passing a ref on the unique_ptr holding 
	//		the top node of the tree
	void processFromTop( unique_ptr<Node>& top) 
	{
		myCurrent = &top;
		top->acceptVisitor( *this);
	}

	//	Conditions

	//	One visitor for all conditions
	template <class NodeCond>
	inline void visitCondT( NodeCond& node)
	{
		//	Always true ==> replace the tree by a True node
		if( node.alwaysTrue) myCurrent->reset( new NodeTrue());
		
		//	Always false ==> replace the tree by a False node
		else if( node.alwaysFalse) myCurrent->reset( new NodeFalse());

		//	Nothing to do here ==> visit the arguments
		else visitArguments( node);
	}

	//	Visitors
	void visitEqual( NodeEqual& node) override
	{
		visitCondT( node);
	}
	void visitNot( NodeNot& node) override
	{
		visitCondT( node);
	}
	void visitSuperior( NodeSuperior& node) override
	{
		visitCondT( node);
	}
	void visitSupEqual( NodeSupEqual& node) override
	{
		visitCondT( node);
	}
	void visitAnd( NodeAnd& node) override
	{
		visitCondT( node);
	}
	void visitOr( NodeOr& node) override
	{
		visitCondT( node);
	}

	//	If
	void visitIf( NodeIf& node) override
	{
		//	Always true ==> replace the tree by the collection of "if true" statements
		if( node.alwaysTrue) 
		{
            size_t lastTrueStat = node.firstElse == -1? node.arguments.size()-1: node.firstElse-1;
			
			//	Move arguments, destroy node
			vector<ExprTree> args = move( node.arguments);
			myCurrent->reset( new NodeCollect());
			
			for(size_t i=1; i<=lastTrueStat; ++i)
			{
				(*myCurrent)->arguments.push_back( move( args[i]));
			}

			visitArguments( **myCurrent);
		}
		
		//	Always false ==> replace the tree by the collection of "else" statements
		else if( node.alwaysFalse)
		{
			int firstElseStatement = node.firstElse;

			//	Move arguments, destroy node
			vector<ExprTree> args = move( node.arguments);
			myCurrent->reset( new NodeCollect());

			if( firstElseStatement != -1)
			{
				for(size_t i=firstElseStatement; i<args.size(); ++i)
				{
					(*myCurrent)->arguments.push_back( move( args[i]));
				}
			}

			visitArguments( **myCurrent);
		}

		//	Nothing to do here ==> visit the arguments
		else visitArguments( node);
	}
};
