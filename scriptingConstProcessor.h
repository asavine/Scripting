#pragma once


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

#include <iostream>

#include "scriptingNodes.h"

#include "scriptingScenarios.h"

#include <vector>
#include "quickStack.h"

class ConstProcessor : public Visitor<ConstProcessor>
{
protected:

    //	State

    //  Const status of variables
    vector<char>		        myVarConst;
    vector<double>		        myVarConstVal;

    //	Inside an if?
    bool						myInConditional;

    //  Is this node a constant?
    //  Note the argument must be of exprNode type
    static bool constArg(const ExprTree& node)
    {
        return downcast<const exprNode>(node)->isConst;
    }

    //  Are all the arguments to this node constant?
    //  Note the arguments must be of exprNode type
    static bool constArgs(const Node& node, const size_t first = 0)
    {
        for (size_t i = first; i < node.arguments.size(); ++i)
        {
            if (!constArg(node.arguments[i])) return false;
        }
        return true;
    }

public:

    using Visitor<ConstProcessor>::visit;

    //	Constructor, nVar = number of variables, from Product after parsing and variable indexation
    //  All variables start as constants with value 0
    ConstProcessor(const size_t nVar) : 
        myVarConst(nVar, true), 
        myVarConstVal(nVar, 0.0), 
        myInConditional(false) 
    {}

    //	Visitors

    //	Expressions

    //	Binaries

    template <class OP>
    void visitBinary(exprNode& node, const OP op)
    {
        visitArguments(node);
        if (constArgs(node))
        {
            node.isConst = true;

            const double lhs = downcast<exprNode>(node.arguments[0])->constVal;
            const double rhs = downcast<exprNode>(node.arguments[1])->constVal;
            node.constVal = op(lhs, rhs);
        }
    }

    void visit(NodeAdd& node) 
    {
        visitBinary(node, [](const double& x, const double& y) {return x + y; });
    }
    void visit(NodeSub& node) 
    {
        visitBinary(node, [](const double& x, const double& y) {return x - y; });
    }
    void visit(NodeMult& node) 
    {
        visitBinary(node, [](const double& x, const double& y) {return x * y; });
    }
    void visit(NodeDiv& node) 
    {
        visitBinary(node, [](const double& x, const double& y) {return x / y; });
    }
    void visit(NodePow& node) 
    {
        visitBinary(node, [](const double& x, const double& y) {return pow(x, y); });
    }
    void visit(NodeMax& node) 
    {
        visitBinary(node, [](const double& x, const double& y) {return max(x, y); });
    }
    void visit(NodeMin& node) 
    {
        visitBinary(node, [](const double& x, const double& y) {return min(x, y); });
    }

    //	Unaries
    template <class OP>
    void visitUnary(exprNode& node, const OP op)
    {
        visitArguments(node);
        if (constArgs(node))
        {
            node.isConst = true;

            const double arg = downcast<exprNode>(node.arguments[0])->constVal;
            node.constVal = op(arg);
        }
    }

    void visit(NodeUplus& node)  
    { 
        visitUnary(node, [](const double& x) {return x; });
    }
    void visit(NodeUminus& node) 
    {
        visitUnary(node, [](const double& x) {return -x; });
    }

    //	Functions
    void visit(NodeLog& node) 
    {
        visitUnary(node, [](const double& x) {return log(x); });
    }
    void visit(NodeSqrt& node) 
    {
        visitUnary(node, [](const double& x) {return sqrt(x); });
    }

    //  Multies

    void visit(NodeSmooth& node) 
    {
        visitArguments(node);
        if (constArgs(node))
        {
            node.isConst = true;

            const double x = reinterpret_cast<exprNode*>(node.arguments[0].get())->constVal;
            const double vPos = reinterpret_cast<exprNode*>(node.arguments[1].get())->constVal;
            const double vNeg = reinterpret_cast<exprNode*>(node.arguments[2].get())->constVal;
            const double halfEps = 0.5 * reinterpret_cast<exprNode*>(node.arguments[3].get())->constVal;

            if (x < -halfEps) node.constVal = vNeg;
            else if (x > halfEps) node.constVal = vPos;
            else
            {
                node.constVal = vNeg + 0.5 * (vPos - vNeg) / halfEps * (x + halfEps);
            }
        }
    }

    //	If
    void visit(NodeIf& node) 
    {
        //  Mark conditional

        //  Identify nested
        bool nested = myInConditional;

        //  Mark
        if (!nested) myInConditional = true;

        //  Visit arguments
        visitArguments(node);

        //  Reset (unless nested)
        if (!nested) myInConditional = false;
    }

    void visit(NodeAssign& node) 
    {
        //  Get index from LHS
        const size_t varIndex = downcast<const NodeVar>(node.arguments[0])->index;

        //  Visit RHS
        node.arguments[1]->accept(*this);

        //  All conditional assignments result in non const vars
        if (!myInConditional)
        {
            //  RHS constant?
            if (constArg(node.arguments[1]))
            {
                myVarConst[varIndex] = true;
                myVarConstVal[varIndex] = downcast<const exprNode>(node.arguments[1])->constVal;
            }
            else
            {
                myVarConst[varIndex] = false;
            }
        }
        else
        {
            myVarConst[varIndex] = false;
        }
    }

    void visit(NodePays& node) 
    {
        //  A payment is always non constant because it is normalized by a possibly stochastic numeraire
        const size_t varIndex = downcast<const NodeVar>(node.arguments[0])->index;
        myVarConst[varIndex] = false;

        //  Visit RHS
        node.arguments[1]->accept(*this);
    }

    //	Variables, RHS only, we don't visit LHS vars
    void visit(NodeVar& node) 
    {
        if (myVarConst[node.index])
        {
            node.isConst = true;
            node.constVal = myVarConstVal[node.index];
        }
        else
        {
            node.isConst = false;
        }
    }

    //  We don't visit boolean nodes, that is best left to fuzzy logic

    //  We don't visit constants (which are always const) or spots (which are never const)
};

