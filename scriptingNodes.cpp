
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

#include "scriptingNodes.h"
#include "scriptingVisitor.h"

//	Collection of statements
void NodeCollect::acceptVisitor( Visitor& visitor)
{
	visitor.visitCollect( *this);
}

//	True
void NodeTrue::acceptVisitor( Visitor& visitor)
{
	visitor.visitTrue( *this);
}

//	False
void NodeFalse::acceptVisitor( Visitor& visitor)
{
	visitor.visitFalse( *this);
}

//	Unary +/-
void NodeUplus::acceptVisitor( Visitor& visitor)
{
	visitor.visitUplus( *this);
}

void NodeUminus::acceptVisitor( Visitor& visitor)
{
	visitor.visitUminus( *this);
}

//	Math operators
void NodeAdd::acceptVisitor( Visitor& visitor)
{
	visitor.visitAdd( *this);
}

void NodeSubtract::acceptVisitor( Visitor& visitor)
{
	visitor.visitSubtract( *this);
}

void NodeMult::acceptVisitor( Visitor& visitor)
{
	visitor.visitMult( *this);
}

void NodeDiv::acceptVisitor( Visitor& visitor)
{
	visitor.visitDiv( *this);
}

void NodePow::acceptVisitor( Visitor& visitor)
{
	visitor.visitPow( *this);
}

//	Math functions
void NodeLog::acceptVisitor( Visitor& visitor)
{
	visitor.visitLog( *this);
}

void NodeSqrt::acceptVisitor( Visitor& visitor)
{
	visitor.visitSqrt( *this);
}

void NodeMax::acceptVisitor( Visitor& visitor)
{
	visitor.visitMax( *this);
}

void NodeMin::acceptVisitor( Visitor& visitor)
{
	visitor.visitMin( *this);
}

//	Functional if
void NodeSmooth::acceptVisitor( Visitor& visitor)
{
	visitor.visitSmooth( *this);
}

//	Comparators
void NodeEqual::acceptVisitor( Visitor& visitor)
{
	visitor.visitEqual( *this);
}

void NodeNot::acceptVisitor( Visitor& visitor)
{
	visitor.visitNot( *this);
}

void NodeSuperior::acceptVisitor( Visitor& visitor)
{
	visitor.visitSuperior( *this);
}

void NodeSupEqual::acceptVisitor( Visitor& visitor)
{
	visitor.visitSupEqual( *this);
}

//	And/or

void NodeAnd::acceptVisitor( Visitor& visitor)
{
	visitor.visitAnd( *this);
}

void NodeOr::acceptVisitor( Visitor& visitor)
{
	visitor.visitOr( *this);
}

//	Assign, Pays
void NodeAssign::acceptVisitor( Visitor& visitor)
{
	visitor.visitAssign( *this);
}

void NodePays::acceptVisitor( Visitor& visitor)
{
	visitor.visitPays( *this);
}

//	Market access
void NodeSpot::acceptVisitor( Visitor& visitor)
{
	visitor.visitSpot( *this);
}

//	If
void NodeIf::acceptVisitor( Visitor& visitor)
{
	visitor.visitIf( *this);
}

void NodeConst::acceptVisitor( Visitor& visitor)
{
	visitor.visitConst( *this);
}

void NodeVar::acceptVisitor( Visitor& visitor)
{
	visitor.visitVar( *this);
}

//	const

//	Collection of statements
void NodeCollect::acceptVisitor( constVisitor& visitor) const
{
	visitor.visitCollect( *this);
}

//	True
void NodeTrue::acceptVisitor( constVisitor& visitor) const
{
	visitor.visitTrue( *this);
}

//	False
void NodeFalse::acceptVisitor( constVisitor& visitor) const
{
	visitor.visitFalse( *this);
}

//	Unary +/-
void NodeUplus::acceptVisitor( constVisitor& visitor) const
{
	visitor.visitUplus( *this);
}

void NodeUminus::acceptVisitor( constVisitor& visitor) const
{
	visitor.visitUminus( *this);
}

//	Math operators
void NodeAdd::acceptVisitor( constVisitor& visitor) const
{
	visitor.visitAdd( *this);
}

void NodeSubtract::acceptVisitor( constVisitor& visitor) const
{
	visitor.visitSubtract( *this);
}

void NodeMult::acceptVisitor( constVisitor& visitor) const
{
	visitor.visitMult( *this);
}

void NodeDiv::acceptVisitor( constVisitor& visitor) const
{
	visitor.visitDiv( *this);
}

void NodePow::acceptVisitor( constVisitor& visitor) const
{
	visitor.visitPow( *this);
}

//	Math functions
void NodeLog::acceptVisitor( constVisitor& visitor) const
{
	visitor.visitLog( *this);
}

void NodeSqrt::acceptVisitor( constVisitor& visitor) const
{
	visitor.visitSqrt( *this);
}

void NodeMax::acceptVisitor( constVisitor& visitor) const
{
	visitor.visitMax( *this);
}

void NodeMin::acceptVisitor( constVisitor& visitor) const
{
	visitor.visitMin( *this);
}

//	Functional if
void NodeSmooth::acceptVisitor( constVisitor& visitor) const
{
	visitor.visitSmooth( *this);
}

//	Comparators
void NodeEqual::acceptVisitor( constVisitor& visitor) const
{
	visitor.visitEqual( *this);
}

void NodeNot::acceptVisitor( constVisitor& visitor) const
{
	visitor.visitNot( *this);
}

void NodeSuperior::acceptVisitor( constVisitor& visitor) const
{
	visitor.visitSuperior( *this);
}

void NodeSupEqual::acceptVisitor( constVisitor& visitor) const
{
	visitor.visitSupEqual( *this);
}

//	And/or

void NodeAnd::acceptVisitor( constVisitor& visitor) const
{
	visitor.visitAnd( *this);
}

void NodeOr::acceptVisitor( constVisitor& visitor) const
{
	visitor.visitOr( *this);
}

//	Assign, Pays
void NodeAssign::acceptVisitor( constVisitor& visitor) const
{
	visitor.visitAssign( *this);
}

void NodePays::acceptVisitor( constVisitor& visitor) const
{
	visitor.visitPays( *this);
}

//	Market access
void NodeSpot::acceptVisitor( constVisitor& visitor) const
{
	visitor.visitSpot( *this);
}

//	If
void NodeIf::acceptVisitor( constVisitor& visitor) const
{
	visitor.visitIf( *this);
}

void NodeConst::acceptVisitor( constVisitor& visitor) const
{
	visitor.visitConst( *this);
}

void NodeVar::acceptVisitor( constVisitor& visitor) const
{
	visitor.visitVar( *this);
}