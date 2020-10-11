
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

//	Base node
struct Node
{
	vector<ExprTree>	arguments;

	virtual ~Node() {}

	virtual void acceptVisitor( Visitor& visitor) = 0;	
	virtual void acceptVisitor( constVisitor& visitor) const = 0;	
};

//	Factories

//  Make concrete node
template <typename ConcreteNode, typename... Args>
unique_ptr<ConcreteNode> make_node(Args&&... args)
{
    return unique_ptr<ConcreteNode>(new ConcreteNode(forward<Args>(args)...));
}

//  Same but return as pointer on base
template <typename ConcreteNode, typename... Args>
unique_ptr<Node> make_base_node(Args&&... args)
{
    return unique_ptr<Node>(new ConcreteNode(forward<Args>(args)...));
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
struct NodeCollect : public Node
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

//	True
struct NodeTrue: public Node
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

//	False
struct NodeFalse : public Node
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

//	Unary +/-
struct NodeUplus : public Node 
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

struct NodeUminus : public Node 
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

//	Math operators
struct NodeAdd : public Node 
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

struct NodeSubtract : public Node 
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

struct NodeMult : public Node 
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

struct NodeDiv : public Node 
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

struct NodePow : public Node 
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

//	Math functions
struct NodeLog : public Node
	 
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

struct NodeSqrt : public Node	 
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

struct NodeMax : public Node 
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

struct NodeMin : public Node 
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

//	Functional if
struct NodeSmooth : public Node 
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

//	Comparators
struct NodeEqual : public Node 
{
	bool				myAlwaysTrue;
	bool				myAlwaysFalse;
	//	Fuzzying stuff
	bool				myDiscrete;	//	Continuous or discrete
	//	Continuous eps
	double				myEps;		
	//	Discrete butterfly bounds
	double				myLb;
	double				myRb;
	//	End of fuzzying stuff

	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

struct NodeNot : public Node 
{
	bool				myAlwaysTrue;
	bool				myAlwaysFalse;

	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

struct NodeSuperior : public Node 
{
	bool				myAlwaysTrue;
	bool				myAlwaysFalse;

	//	Fuzzying stuff
	bool				myDiscrete;	//	Continuous or discrete
	//	Continuous eps
	double				myEps;		
	//	Discrete call spread bounds
	double				myLb;
	double				myRb;
	//	End of fuzzying stuff

	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

struct NodeSupEqual : public Node 
{
	bool				myAlwaysTrue;
	bool				myAlwaysFalse;

	//	Fuzzying stuff
	bool				myDiscrete;	//	Continuous or discrete
	//	Continuous eps
	double				myEps;		
	//	Discrete call spread bounds
	double				myLb;
	double				myRb;
	//	End of fuzzying stuff

	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

//	And/or

struct NodeAnd : public Node 
{
	bool				myAlwaysTrue;
	bool				myAlwaysFalse;

	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

struct NodeOr : public Node 
{
	bool				myAlwaysTrue;
	bool				myAlwaysFalse;

	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

//	Assign, Pays
struct NodeAssign : public Node 
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

struct NodePays : public Node 
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

//	Market access
struct NodeSpot : public Node 
{
	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

//	If
struct NodeIf : public Node
{
	int					firstElse;
	//	For fuzzy eval: indices of variables affected in statements, including nested
	vector<unsigned>	myAffectedVars;		
	//	Always true/false as per domain processor
	bool				myAlwaysTrue;
	bool				myAlwaysFalse;

	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

struct NodeConst : public Node
{
	double				val;

    NodeConst(const double val_) : val(val_) {}

	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};

struct NodeVar : public Node
{
	string				name;
	unsigned			index;

    NodeVar(const string name_) : name(name_) {}

	void acceptVisitor( Visitor& visitor) override;
	void acceptVisitor( constVisitor& visitor) const override;
};