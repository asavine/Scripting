
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

using namespace std;

#include <vector>
#include <memory>

#include "scriptingVisitor.h"

struct Node;
using ExprTree = unique_ptr<Node>;

//	Base nodes

struct Node :
    VisitableBase<VISITORS>
{
    using VisitableBase<VISITORS>::accept;

	vector<ExprTree>	arguments;
	virtual ~Node() {}
};

//  Nodes that return a number
struct exprNode : Node
{
    bool                isConst = false;
    double              constVal;
};

//  Action
struct actNode : Node {};

//  Return a bool
struct boolNode : Node 
{
    bool				alwaysTrue;
    bool				alwaysFalse;
};

//	Factories

template <typename NodeType, typename... Args>
unique_ptr<NodeType> make_node(Args&&... args)
{
	return unique_ptr<NodeType>( new NodeType(forward<Args>(args)...));
}

template <typename NodeType, typename... Args>
unique_ptr<Node> make_base_node(Args&&... args)
{
	return unique_ptr<Node>( new NodeType(forward<Args>(args)...));
}

//  Downcast Node to concrete type, crashes if used on another Node type
template<class NODE>
const NODE* downcast(const unique_ptr<Node>& node)
{
    return reinterpret_cast<const NODE*>(node.get());
}

//	Build binary of the kind in the template parameter, and set its arguments to lhs and rhs trees
template <class NodeType>
ExprTree buildBinary( ExprTree& lhs, ExprTree& rhs)
{
	auto top = make_base_node<NodeType>();
	top->arguments.resize( 2);
	//	Take ownership of lhs and rhs
	top->arguments[0] = move( lhs);
	top->arguments[1] = move( rhs);
	//	Return
	return top;
}

//	Overload that returns concrete node
template <class NodeType>
unique_ptr<NodeType> buildConcreteBinary( ExprTree& lhs, ExprTree& rhs)
{
	auto top = make_node<NodeType>();
	top->arguments.resize( 2);
	//	Take ownership of lhs and rhs
	top->arguments[0] = move( lhs);
	top->arguments[1] = move( rhs);
	//	Return
	return top;
}

//  All the concrete nodes

//  Binary expressions

struct NodeAdd :
    Visitable<exprNode, NodeAdd, VISITORS>
{};

struct NodeSub :
    Visitable<exprNode, NodeSub, VISITORS>
{};

struct NodeMult :
    Visitable<exprNode, NodeMult, VISITORS>
{};

struct NodeDiv :
    Visitable<exprNode, NodeDiv, VISITORS>
{};

struct NodePow :
    Visitable<exprNode, NodePow, VISITORS>
{};

struct NodeMax :
    Visitable<exprNode, NodeMax, VISITORS>
{};

struct NodeMin :
    Visitable<exprNode, NodeMin, VISITORS>
{};

//  Unary expressions

struct NodeUplus :
    Visitable<exprNode, NodeUplus, VISITORS>
{};

struct NodeUminus :
    Visitable<exprNode, NodeUminus, VISITORS>
{};

//	Math operators

struct NodeLog :
    Visitable<exprNode, NodeLog, VISITORS>
{};

struct NodeSqrt :
    Visitable<exprNode, NodeSqrt, VISITORS>
{};

//  Multi expressions

//	Functional if
struct NodeSmooth :
    Visitable<exprNode, NodeSmooth, VISITORS>
{};

//  Comparisons

struct compNode : boolNode
{
    //	Fuzzying stuff
    bool				discrete;	//	Continuous or discrete
                                    //	Continuous eps
    double				eps;
    //	Discrete butterfly bounds
    double				lb;
    double				rb;
    //	End of fuzzying stuff
};

struct NodeEqual :
    Visitable<compNode, NodeEqual, VISITORS>
{};

struct NodeSup :
    Visitable<compNode, NodeSup, VISITORS>
{};

struct NodeSupEqual :
    Visitable<compNode, NodeSupEqual, VISITORS>
{};

//	And/or/not

struct NodeAnd :
    Visitable<boolNode, NodeAnd, VISITORS>
{};

struct NodeOr :
    Visitable<boolNode, NodeOr, VISITORS>
{};

struct NodeNot :
    Visitable<boolNode, NodeNot, VISITORS>
{};

//  Leaves

//	Market access
struct NodeSpot :    
    Visitable<exprNode, NodeSpot, VISITORS>
{};

//  Const
struct NodeConst :
    Visitable<exprNode, NodeConst, VISITORS>
{
    NodeConst(const double val)
    {
        isConst = true;
        constVal = val;
    }
};

struct NodeTrue :
    Visitable<boolNode, NodeTrue, VISITORS>
{
    NodeTrue()
    {
        alwaysTrue = true;
    }
};

struct NodeFalse :
    Visitable<boolNode, NodeFalse, VISITORS>
{
    NodeFalse()
    {
        alwaysFalse = true;
    }
};

//  Variable
struct NodeVar :
    Visitable<exprNode, NodeVar, VISITORS>
{
    NodeVar(const string n) : name(n)
    {
        isConst = true;
        constVal = 0.0;
    }

    const string		name;
    unsigned			index;
};

//	Assign, Pays

struct NodeAssign :
    Visitable<actNode, NodeAssign, VISITORS>
{};

struct NodePays :
    Visitable<actNode, NodePays, VISITORS>
{};

//	If
struct NodeIf :
    Visitable<actNode, NodeIf, VISITORS>
{
    int					firstElse;
    //	For fuzzy eval: indices of variables affected in statements, including nested
    vector<unsigned>	affectedVars;
    //	Always true/false as per domain processor
    bool				alwaysTrue;
    bool				alwaysFalse;
};

//	Collection of statements
struct NodeCollect : 
    Visitable<actNode, NodeCollect, VISITORS>
{};






