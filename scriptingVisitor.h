#pragma once

#include "scriptingNodes.h"

class Visitor
{
protected:
	//	Protected constructor so the base class cannot be instantiated
	Visitor() {}

public:
	virtual ~Visitor() {}

protected:
	//	Default visit just visits arguments so as to ensure that the whole sub-tree is visited
	virtual void visitArguments( Node& node) 
	{
		for( auto& arg : node.arguments) arg->acceptVisitor( *this);
	}

public:

	//	Entry point for visiting a (sub-) tree
	void visit( ExprTree& tree)
	{
		tree->acceptVisitor( *this);
	}

	//	All concrete node default visitors, visit arguments unless overridden
	virtual void visitCollect( NodeCollect& node) { visitArguments( node); }
	virtual void visitTrue( NodeTrue& node) { visitArguments( node); }
	virtual void visitFalse( NodeFalse& node) { visitArguments( node); }
	virtual void visitUplus( NodeUplus& node) { visitArguments( node); }
	virtual void visitUminus( NodeUminus& node) { visitArguments( node); }
	virtual void visitAdd( NodeAdd& node) { visitArguments( node); }
	virtual void visitSubtract( NodeSubtract& node) { visitArguments( node); }
	virtual void visitMult( NodeMult& node) { visitArguments( node); }
	virtual void visitDiv( NodeDiv& node) { visitArguments( node); }
	virtual void visitPow( NodePow& node) { visitArguments( node); }
	virtual void visitLog( NodeLog& node) { visitArguments( node); }
	virtual void visitSqrt( NodeSqrt& node) { visitArguments( node); }
	virtual void visitMax( NodeMax& node) { visitArguments( node); }
	virtual void visitMin( NodeMin& node) { visitArguments( node); }
	virtual void visitSmooth( NodeSmooth& node) { visitArguments( node); }
	virtual void visitEqual( NodeEqual& node) { visitArguments( node); }
	virtual void visitNot( NodeNot& node) { visitArguments( node); }
	virtual void visitSuperior( NodeSuperior& node) { visitArguments( node); }
	virtual void visitSupEqual( NodeSupEqual& node) { visitArguments( node); }
	virtual void visitAnd( NodeAnd& node) { visitArguments( node);}
	virtual void visitOr( NodeOr& node) { visitArguments( node); }
	virtual void visitAssign( NodeAssign& node) { visitArguments( node); }
	virtual void visitPays( NodePays& node) { visitArguments( node); }
	virtual void visitSpot( NodeSpot& node) { visitArguments( node); }
	virtual void visitIf( NodeIf& node) { visitArguments( node); }
	virtual void visitConst( NodeConst& node) { visitArguments( node); }
	virtual void visitVar( NodeVar& node) { visitArguments( node); }
};

class constVisitor
{
protected:
	//	Protected constructor so the base class cannot be instantiated
	constVisitor() {}

public:
	virtual ~constVisitor() {}

protected:
	//	Default visit just visits arguments so as to ensure that the whole sub-tree is visited
	virtual void visitArguments( const Node& node) 
	{
		for( auto& arg : node.arguments) arg->acceptVisitor( *this);
	}

public:

	//	Entry point for visiting a (sub-) tree
	void visit( const ExprTree& tree)
	{
		tree->acceptVisitor( *this);
	}

	//	All concrete node default visitors, visit arguments unless overridden

	virtual void visitCollect( const NodeCollect& node) { visitArguments( node); }
	virtual void visitTrue( const NodeTrue& node) { visitArguments( node); }
	virtual void visitFalse( const NodeFalse& node) { visitArguments( node); }
	virtual void visitUplus( const NodeUplus& node) { visitArguments( node); }
	virtual void visitUminus( const NodeUminus& node) { visitArguments( node); }
	virtual void visitAdd( const NodeAdd& node) { visitArguments( node); }
	virtual void visitSubtract( const NodeSubtract& node) { visitArguments( node); }
	virtual void visitMult( const NodeMult& node) { visitArguments( node); }
	virtual void visitDiv( const NodeDiv& node) { visitArguments( node); }
	virtual void visitPow( const NodePow& node) { visitArguments( node); }
	virtual void visitLog( const NodeLog& node) { visitArguments( node); }
	virtual void visitSqrt( const NodeSqrt& node) { visitArguments( node); }
	virtual void visitMax( const NodeMax& node) { visitArguments( node); }
	virtual void visitMin( const NodeMin& node) { visitArguments( node); }
	virtual void visitSmooth( const NodeSmooth& node) { visitArguments( node); }
	virtual void visitEqual( const NodeEqual& node) { visitArguments( node); }
	virtual void visitNot( const NodeNot& node) { visitArguments( node); }
	virtual void visitSuperior( const NodeSuperior& node) { visitArguments( node); }
	virtual void visitSupEqual( const NodeSupEqual& node) { visitArguments( node); }	
	virtual void visitAnd( const NodeAnd& node) { visitArguments( node);}
	virtual void visitOr( const NodeOr& node) { visitArguments( node); }
	virtual void visitAssign( const NodeAssign& node) { visitArguments( node); }
	virtual void visitPays( const NodePays& node) { visitArguments( node); }
	virtual void visitSpot( const NodeSpot& node) { visitArguments( node); }
	virtual void visitIf( const NodeIf& node) { visitArguments( node); }
	virtual void visitConst( const NodeConst& node) { visitArguments( node); }
	virtual void visitVar( const NodeVar& node) { visitArguments( node); }
};