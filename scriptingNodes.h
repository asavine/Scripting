
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

class Visitor;
class constVisitor;

struct Node;

using ExprTree = unique_ptr<Node>;

//	Base nodes

struct Node
{
	vector<ExprTree>	arguments;

	virtual ~Node() {}

	virtual void acceptVisitor( Visitor& visitor) = 0;	
	virtual void acceptVisitor( constVisitor& visitor) const = 0;	
};

//  Nodes that return a number
struct dNode : public Node
{
    bool                isConst = false;
    double              constVal;
};

//  Action
struct aNode : public Node {};

//  Return a bool
struct bNode : public Node {};

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

//	Collection of statements
struct NodeCollect : public aNode
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

//	True
struct NodeTrue: public bNode
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

//	False
struct NodeFalse : public bNode
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

//	Unary +/-
struct NodeUplus : public dNode 
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

struct NodeUminus : public dNode
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

//	Math operators
struct NodeAdd : public dNode
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

struct NodeSubtract : public dNode
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

struct NodeMult : public dNode
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

struct NodeDiv : public dNode
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

struct NodePow : public dNode
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

//	Math functions
struct NodeLog : public dNode
	 
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

struct NodeSqrt : public dNode
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

struct NodeMax : public dNode
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

struct NodeMin : public dNode
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

//	Functional if
struct NodeSmooth : public dNode
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

//	Comparators

struct condNode : public bNode
{
    bool				alwaysTrue;
    bool				alwaysFalse;
};

struct compNode : public condNode
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

struct NodeEqual : public compNode
{
    void acceptVisitor(Visitor& visitor) override;
    void acceptVisitor(constVisitor& visitor) const override;
};

struct NodeSuperior : public compNode
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

struct NodeSupEqual : public compNode
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

//	And/or/not

struct NodeAnd : public condNode
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

struct NodeOr : public condNode
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

struct NodeNot : public condNode
{
    void acceptVisitor(Visitor& visitor) override;
    void acceptVisitor(constVisitor& visitor) const override;
};

//	Assign, Pays
struct NodeAssign : public aNode 
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

struct NodePays : public aNode 
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

//	Market access
struct NodeSpot : public dNode 
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

//	If
struct NodeIf : public aNode
{
    int					firstElse;
    //	For fuzzy eval: indices of variables affected in statements, including nested
    vector<unsigned>	affectedVars;
    //	Always true/false as per domain processor
    bool				alwaysTrue;
    bool				alwaysFalse;

	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

struct NodeConst : public dNode
{
    NodeConst(const double val)
    {
        isConst = true;
        constVal = val;
    }

	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

struct NodeVar : public dNode
{
    NodeVar()
    {
        isConst = true;
        constVal = 0.0;
    }

    string				name;
    unsigned			index;

	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};