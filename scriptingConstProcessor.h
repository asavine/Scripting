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
#include "scriptingVisitor.h"
#include "scriptingScenarios.h"

#include <vector>
#include "quickStack.h"

class ConstProcessor : public Visitor
{

protected:

    //	State

    //  Const status of variables
    vector<char>		        myVarConst;
    vector<double>		        myVarConstVal;

    //	Inside an if?
    bool						myInConditional;

    //  Is this node a constant?
    //  Note the argument must be of dNode type
    static bool constArg(const ExprTree& node)
    {
        auto dnode = downcast<const dNode>(node);
        return dnode->isConst;
    }

    //  Are all the arguments to this node constant?
    //  Note the arguments must be of dNode type
    static bool constArgs(const Node& node, const size_t first = 0)
    {
        for (size_t i = first; i < node.arguments.size(); ++i)
        {
            if (!constArg(node.arguments[i])) return false;
        }
        return true;
    }

public:

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
    void visitBinary(dNode& node, const OP op)
    {
        visitArguments(node);
        if (constArgs(node))
        {
            node.isConst = true;

            const double lhs = downcast<dNode>(node.arguments[0])->constVal;
            const double rhs = downcast<dNode>(node.arguments[1])->constVal;
            node.constVal = op(lhs, rhs);
        }
    }

    void visitAdd(NodeAdd& node) override
    {
        visitBinary(node, [](const double& x, const double& y) {return x + y; });
    }

    void visitSubtract(NodeSubtract& node) override
    {
        visitBinary(node, [](const double& x, const double& y) {return x - y; });
    }
    void visitMult(NodeMult& node) override
    {
        visitBinary(node, [](const double& x, const double& y) {return x * y; });
    }
    void visitDiv(NodeDiv& node) override
    {
        visitBinary(node, [](const double& x, const double& y) {return x / y; });
    }
    void visitPow(NodePow& node) override
    {
        visitBinary(node, [](const double& x, const double& y) {return pow(x, y); });
    }

    //	Unaries
    template <class OP>
    void visitUnary(dNode& node, const OP op)
    {
        visitArguments(node);
        if (constArgs(node))
        {
            node.isConst = true;

            const double arg = downcast<dNode>(node.arguments[0])->constVal;
            node.constVal = op(arg);
        }
    }

    void visitUplus(NodeUplus& node) override 
    { 
        visitUnary(node, [](const double& x) {return x; });
    }
    void visitUminus(NodeUminus& node) override 
    {
        visitUnary(node, [](const double& x) {return -x; });
    }

    //	Functions
    void visitLog(NodeLog& node) override
    {
        visitUnary(node, [](const double& x) {return log(x); });
    }
    void visitSqrt(NodeSqrt& node) override
    {
        visitUnary(node, [](const double& x) {return sqrt(x); });
    }

    //  Multies

    void visitMax(NodeMax& node) override
    {
        visitArguments(node);
        if (constArgs(node))
        {
            node.isConst = true;

            const size_t n = node.arguments.size();
            double m = reinterpret_cast<dNode*>(node.arguments[0].get())->constVal;

            for (size_t i = 1; i < n; ++i)
            {
                m = max(m, reinterpret_cast<dNode*>(node.arguments[i].get())->constVal);
            }

            node.constVal = m;
        }
    }
    void visitMin(NodeMin& node) override
    {
        visitArguments(node);
        if (constArgs(node))
        {
            node.isConst = true;

            const size_t n = node.arguments.size();
            double m = reinterpret_cast<dNode*>(node.arguments[0].get())->constVal;

            for (size_t i = 1; i < n; ++i)
            {
                m = min(m, reinterpret_cast<dNode*>(node.arguments[i].get())->constVal);
            }

            node.constVal = m;
        }
    }
    void visitSmooth(NodeSmooth& node) override
    {
        visitArguments(node);
        if (constArgs(node))
        {
            node.isConst = true;

            const double x = reinterpret_cast<dNode*>(node.arguments[0].get())->constVal;
            const double vPos = reinterpret_cast<dNode*>(node.arguments[1].get())->constVal;
            const double vNeg = reinterpret_cast<dNode*>(node.arguments[2].get())->constVal;
            const double halfEps = 0.5 * reinterpret_cast<dNode*>(node.arguments[3].get())->constVal;

            if (x < -halfEps) node.constVal = vNeg;
            else if (x > halfEps) node.constVal = vPos;
            else
            {
                node.constVal = vNeg + 0.5 * (vPos - vNeg) / halfEps * (x + halfEps);
            }
        }
    }

    //	If
    void visitIf(NodeIf& node) override
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

    void visitAssign(NodeAssign& node) override
    {
        //  Get index from LHS
        const size_t varIndex = downcast<const NodeVar>(node.arguments[0])->index;

        //  Visit RHS
        node.arguments[1]->acceptVisitor(*this);

        //  All conditional assignments result in non const vars
        if (!myInConditional)
        {
            //  RHS constant?
            if (constArg(node.arguments[1]))
            {
                myVarConst[varIndex] = true;
                myVarConstVal[varIndex] = downcast<const dNode>(node.arguments[1])->constVal;
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

    void visitPays(NodePays& node) override
    {
        //  A payment is always non constant because it is normalized by a possibly stochastic numeraire
        const size_t varIndex = downcast<const NodeVar>(node.arguments[0])->index;
        myVarConst[varIndex] = false;

        //  Visit RHS
        node.arguments[1]->acceptVisitor(*this);
    }

    //	Variables, RHS only, we don't visit LHS vars
    void visitVar(NodeVar& node) override
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

