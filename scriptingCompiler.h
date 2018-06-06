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

#include <functional>
#include <algorithm>

template <class T>
struct EvalState
{
    //	State
    vector<T>				    variables;

    //	Stacks
    staticStack<T>			    dstack;
    staticStack<char>		    bstack;

    //  Constructor
    EvalState(const size_t nVar) : variables (nVar) {}

    //  Initializer
    void init()
    {
        for (auto& var : variables) var = 0.0;
        //	Stacks should be empty, if this is not the case the empty them
        //		without affecting capacity for added performance
        if (!dstack.empty()) dstack.reset();
        if (!bstack.empty()) bstack.reset();
    }

    //	Copy/Move

    EvalState(const EvalState& rhs) : variables(rhs.variables) {}
    EvalState& operator=(const EvalState& rhs)
    {
        if (this == &rhs) return *this;
        variables = rhs.variables;
        return *this;
    }

    EvalState(EvalState&& rhs) : variables(move(rhs.variables)) {}
    EvalState& operator=(EvalState&& rhs)
    {
        variables = move(rhs.variables);
        return *this;
    }
};

enum NodeType
{
    Add,
    AddConst,
    Sub,
    SubConst,
    ConstSub,
    Mult,
    MultConst,
    Div,
    DivConst,
    ConstDiv,
    Pow,
    PowConst,
    ConstPow,
    Max,
    Max2,
    MaxConst,
    Max2Const,
    Min,
    Min2,
    MinConst,
    Min2Const,
    Spot,
    Var,
    Const,
    Assign,
    AssignConst,
    Pays,
    PaysConst,
    If,
    IfElse,
    Equal,
    Sup,
    SupEqual,
    And,
    Or,
    Smooth,
    Sqrt,
    Log,
    Not,
    Uminus,
    True,
    False
};

#define EPS 1.0e-12

class Compiler : public constVisitor
{

protected:

    //	State
    vector<int> myNodeStream;
    vector<double> myConstStream;
    vector<const void*> myDataStream;

public:

    //	Accessors

    //	Access the streams after traversal
    const tuple<vector<int>, vector<double>, vector<const void*>> streams() 
    {
        return make_tuple(myNodeStream, myConstStream, myDataStream);
    }

    //	Visitors

    //	Expressions

    //  Binaries

    template<NodeType IfBin, NodeType IfConstLeft, NodeType IfConstRight>
    void visitBinary(const dNode& node) 
    {
        if (node.isConst)
        {
            myNodeStream.push_back(Const);
            myNodeStream.push_back(myConstStream.size());
            myConstStream.push_back(node.constVal);
        }
        else
        {
            const dNode* lhs = downcast<dNode>(node.arguments[0]);
            const dNode* rhs = downcast<dNode>(node.arguments[1]);

            if (lhs->isConst)
            {
                node.arguments[1]->acceptVisitor(*this);
                myNodeStream.push_back(IfConstLeft);
                myNodeStream.push_back(myConstStream.size());
                myConstStream.push_back(lhs->constVal);
            }
            else if (rhs->isConst)
            {
                node.arguments[0]->acceptVisitor(*this);
                myNodeStream.push_back(IfConstRight);
                myNodeStream.push_back(myConstStream.size());
                myConstStream.push_back(rhs->constVal);
            }
            else
            {
                node.arguments[0]->acceptVisitor(*this);
                node.arguments[1]->acceptVisitor(*this);
                myNodeStream.push_back(IfBin);
            }
        }
    }

    void visitAdd(const NodeAdd& node) override
    {
        visitBinary<Add, AddConst, AddConst>(node);
    }
    void visitSubtract(const NodeSubtract& node) override
    {
        visitBinary<Sub, ConstSub, SubConst>(node);
    }
    void visitMult(const NodeMult& node) override
    {
        visitBinary<Mult, MultConst, MultConst>(node);
    }
    void visitDiv(const NodeDiv& node) override
    {
        visitBinary<Div, ConstDiv, DivConst>(node);
    }
    void visitPow(const NodePow& node) override
    {
        visitBinary<Pow, ConstPow, PowConst>(node);
    }

    //	Unaries
    template<NodeType Instr>
    void visitUnary(const dNode& node)
    {
        if (node.isConst)
        {
            myNodeStream.push_back(Const);
            myNodeStream.push_back(myConstStream.size());
            myConstStream.push_back(node.constVal);
        }
        else
        {
            node.arguments[0]->acceptVisitor(*this);
            myNodeStream.push_back(Instr);
        }
    }

    void visitUplus(const NodeUplus& node) override
    {
        visitArguments(node);
    }

    void visitUminus(const NodeUminus& node) override
    {
        visitUnary<Uminus>(node);
    }
    void visitLog(const NodeLog& node) override
    {
        visitUnary<Log>(node);
    }
    void visitSqrt(const NodeSqrt& node) override
    {
        visitUnary<Sqrt>(node);
    }

    //  Multies
    void visitMax(const NodeMax& node) override
    {
        //  Const?
        if (node.isConst)
        {
            myNodeStream.push_back(Const);
            myNodeStream.push_back(myConstStream.size());
            myConstStream.push_back(node.constVal);
        }

        //  Special case for 2 args since it is so common
        else if (node.arguments.size() == 2)
        {
            visitBinary<Max2, Max2Const, Max2Const>(node);
        }

        //  More than 2 args
        else
        {
            //  Find all const and non const args
            vector<size_t> constArgs, nonConstArgs;
            for (size_t i = 0; i < node.arguments.size(); ++i)
            {
                const dNode* arg = downcast<dNode>(node.arguments[i]);
                if (arg->isConst)
                {
                    constArgs.push_back(i);
                }
                else
                {
                    nonConstArgs.push_back(i);
                }

                //  Visit non const args
                for (size_t i : nonConstArgs)
                {
                    node.arguments[i]->acceptVisitor(*this);
                }

                //  Nothing is const
                if (constArgs.empty())
                {
                    myNodeStream.push_back(Max); 
                    myNodeStream.push_back(nonConstArgs.size()); 
                }
                else
                {
                    myNodeStream.push_back(MaxConst);
                    myNodeStream.push_back(myConstStream.size());
                    myNodeStream.push_back(nonConstArgs.size());
                    double m = downcast<dNode>(node.arguments[constArgs[0]])->constVal;
                    for (size_t i = 1; i < constArgs.size(); ++i)
                    {
                        m = max(m, downcast<dNode>(node.arguments[constArgs[i]])->constVal);
                    }
                    myConstStream.push_back(m);
                }
            }
        }
    }

    void visitMin(const NodeMin& node) override
    {
        //  Const?
        if (node.isConst)
        {
            myNodeStream.push_back(Const);
            myNodeStream.push_back(myConstStream.size());
            myConstStream.push_back(node.constVal);
        }

        //  Special case for 2 args since it is so common
        else if (node.arguments.size() == 2)
        {
            visitBinary<Min2, Min2Const, Min2Const>(node);
        }

        //  More than 2 args
        else
        {
            //  Find all const and non const args
            vector<size_t> constArgs, nonConstArgs;
            for (size_t i = 0; i < node.arguments.size(); ++i)
            {
                const dNode* arg = downcast<dNode>(node.arguments[i]);
                if (arg->isConst)
                {
                    constArgs.push_back(i);
                }
                else
                {
                    nonConstArgs.push_back(i);
                }

                //  Visit non const args
                for (size_t i : nonConstArgs)
                {
                    node.arguments[i]->acceptVisitor(*this);
                }

                //  Nothing is const
                if (constArgs.empty())
                {
                    myNodeStream.push_back(Min);
                    myNodeStream.push_back(nonConstArgs.size());
                }
                else
                {
                    myNodeStream.push_back(MinConst);
                    myNodeStream.push_back(myConstStream.size());
                    myNodeStream.push_back(nonConstArgs.size());
                    double m = downcast<dNode>(node.arguments[constArgs[0]])->constVal;
                    for (size_t i = 1; i < constArgs.size(); ++i)
                    {
                        m = min(m, downcast<dNode>(node.arguments[constArgs[i]])->constVal);
                    }
                    myConstStream.push_back(m);
                }
            }
        }
    }

    void visitSmooth(const NodeSmooth& node) override
    {
        //  Const?
        if (node.isConst)
        {
            myNodeStream.push_back(Const);
            myNodeStream.push_back(myConstStream.size());
            myConstStream.push_back(node.constVal);
        }
        else
        {
            //  Must come back to optimize that one
            visitArguments(node); myNodeStream.push_back(Smooth); 
        }
    }

    //	Conditions

    void visitTrue(const NodeTrue& node) override
    {
        myNodeStream.push_back(True); 
    }

    void visitFalse(const NodeFalse& node) override
    {
        myNodeStream.push_back(False); 
    }

    void visitNot(const NodeNot& node) override
    {
        visitArguments(node); myNodeStream.push_back(Not); myDataStream.push_back(nullptr);
    }

    void visitAnd(const NodeAnd& node) override
    {
        visitArguments(node); myNodeStream.push_back(And); myDataStream.push_back(nullptr);
    }

    void visitOr(const NodeOr& node) override
    {
        visitArguments(node); myNodeStream.push_back(Or); myDataStream.push_back(nullptr);
    }

    template<NodeType NT, typename OP>
    void visitCondition(const bNode& node, OP op)
    {
        const dNode* lhs = downcast<dNode>(node.arguments[0]);

        if (lhs->isConst)
        {
            myNodeStream.push_back(op(lhs->constVal) ? True: False);

        }
        else
        {
            node.arguments[0]->acceptVisitor(*this);
            myNodeStream.push_back(NT);
        }
    }

    void visitEqual(const NodeEqual& node) override
    {
        visitCondition<Equal>(node, [](const double x) {return fabs(x) < EPS; });
    }

    void visitSuperior(const NodeSuperior& node) override
    {
        visitCondition<Sup>(node, [](const double x) {return x > EPS; });
    }
    void visitSupEqual(const NodeSupEqual& node) override
    {
        visitCondition<SupEqual>(node, [](const double x) {return x > -EPS; });
    }

    //	Instructions
    void visitIf(const NodeIf& node) override
    {
        //  Visit condition
        node.arguments[0]->acceptVisitor(*this);

        //  Mark instruction
        myNodeStream.push_back(node.firstElse == -1? If : IfElse);
        //  Record space
        const size_t thisSpace = myNodeStream.size() - 1;
        //  Make 2 spaces for last if-true and last if-false
        myNodeStream.push_back(0); 
        if (node.firstElse != -1) myNodeStream.push_back(0);

        //  Visit if-true statements
        const auto lastTrue = node.firstElse == -1 ? node.arguments.size() - 1 : node.firstElse - 1;
        for (size_t i = 1; i <= lastTrue; ++i)
        {
            node.arguments[i]->acceptVisitor(*this);
        }
        //  Record last if-true space
        myNodeStream[thisSpace+1] = myNodeStream.size();

        //  Visit if-false statements
        const size_t n = node.arguments.size();
        if (node.firstElse != -1)
        {
            for (size_t i = node.firstElse; i < n; ++i)
            {
                {
                    node.arguments[i]->acceptVisitor(*this);
                }
            }
            //  Record last if-false space
            myNodeStream[thisSpace + 2] = myNodeStream.size();
        }
    }

    void visitAssign(const NodeAssign& node) override
    {
        const NodeVar* var = downcast<NodeVar>(node.arguments[0]);
        const dNode* rhs = downcast<dNode>(node.arguments[1]);

        if (rhs->isConst)
        {
            myNodeStream.push_back(AssignConst);
            myNodeStream.push_back(myConstStream.size());
            myConstStream.push_back(rhs->constVal);
        }
        else
        {
            node.arguments[1]->acceptVisitor(*this);
            myNodeStream.push_back(Assign);
        }
        myNodeStream.push_back(var->index);
    }

    void visitPays(const NodePays& node) override
    {
        const NodeVar* var = downcast<NodeVar>(node.arguments[0]);
        const dNode* rhs = downcast<dNode>(node.arguments[1]);

        if (rhs->isConst)
        {
            myNodeStream.push_back(PaysConst);
            myNodeStream.push_back(myConstStream.size());
            myConstStream.push_back(rhs->constVal);
        }
        else
        {
            node.arguments[1]->acceptVisitor(*this);
            myNodeStream.push_back(Pays);
        }
        myNodeStream.push_back(var->index);
    }

    //	Variables and constants
    void visitVar(const NodeVar& node) override
    {
        myNodeStream.push_back(Var); 
        myNodeStream.push_back(node.index); 
    }

    void visitConst(const NodeConst& node) override
    {
        myNodeStream.push_back(Const); 
        myConstStream.push_back(node.constVal);
    }

    //	Scenario related
    void visitSpot(const NodeSpot& node) override
    {
        myNodeStream.push_back(Spot); 
    }

    void visitCollect(const NodeCollect& node) override
    {
        visitArguments(node);
    }
};

template <class T>
inline void evalCompiled(
    //  Stream to eval
    const vector<int>&          nodeStream,
    const vector<double>&       constStream,
    const vector<const void*>&  dataStream,
    //  Scenario
    const SimulData<T>&         scen,
    //  State
    EvalState<T>&               state,
    //  First (included), last (excluded)
    const size_t                first = 0,
    const size_t                last = 0) 
{
    const size_t n = last? last: nodeStream.size();
    size_t i = first;

    T x, y, halfEps, vPos, vNeg;
    T* m;
    double val;
    int idx, narg;

    //  Loop on instructions
    while (i < n)
    {
        //  Big switch
        switch (nodeStream[i])
        {

        case Add:

            state.dstack[1] += state.dstack.top();
            state.dstack.pop();

            ++i;
            break;

        case AddConst:

            state.dstack.top() += constStream[nodeStream[++i]];

            ++i;
            break;

        case Sub:

            state.dstack[1] -= state.dstack.top();
            state.dstack.pop();

            ++i;
            break;

        case SubConst:

            state.dstack.top() -= constStream[nodeStream[++i]];

            ++i;
            break;

        case ConstSub:

            state.dstack.top() = constStream[nodeStream[++i]] - state.dstack.top();

            ++i;
            break;

        case Mult:

            state.dstack[1] *= state.dstack.top();
            state.dstack.pop();

            ++i;
            break;

        case MultConst:

            state.dstack.top() *= constStream[nodeStream[++i]];

            ++i;
            break;

        case Div:

            state.dstack[1] /= state.dstack.top();
            state.dstack.pop();

            ++i;
            break;

        case DivConst:

            state.dstack.top() /= constStream[nodeStream[++i]];

            ++i;
            break;

        case ConstDiv:

            state.dstack.top() = constStream[nodeStream[++i]] / state.dstack.top();

            ++i;
            break;

        case Pow:

            state.dstack[1] = pow(state.dstack[1], state.dstack.top());
            state.dstack.pop();

            ++i;
            break;

        case PowConst:

            state.dstack.top() = pow(state.dstack.top(), constStream[nodeStream[++i]]);

            ++i;
            break;

        case ConstPow:

            state.dstack.top() = pow(constStream[nodeStream[++i]], state.dstack.top());

            ++i;
            break;

        case Max:

            ++i;
            narg = nodeStream[i];
            m = &state.dstack[narg - 1];

            for (int j = narg - 2; j >= 0; --j)
            {
                x = state.dstack[j];
                if (x > *m)
                {
                    *m = x;
                }
            }

            state.dstack.pop(narg - 1);

            ++i;
            break;

        case Max2:

            m = &state.dstack[1];
            x = state.dstack.top();

            if (x > *m) *m = x;
            state.dstack.pop();

            ++i;
            break;

        case MaxConst:

            ++i;
            narg = nodeStream[i];
            y = constStream[nodeStream[++i]];

            for (int j = narg - 1; j >= 0; --j)
            {
                x = state.dstack[j];
                if (x > y)
                {
                    y = x;
                }
            }

            state.dstack.pop(narg - 1);
            state.dstack.top() = y;

            ++i;
            break;

        case Max2Const:

            m = &state.dstack.top();
            x = constStream[nodeStream[++i]];
            if (x > *m) *m = x;

            ++i;
            break;

        case Min:

            ++i;
            narg = nodeStream[i];
            m = &state.dstack[narg - 1];

            for (int j = narg - 2; j >= 0; --j)
            {
                x = state.dstack[j];
                if (x < *m)
                {
                    *m = x;
                }
            }

            state.dstack.pop(narg - 1);

            ++i;
            break;

        case Min2:

            m = &state.dstack[1];
            x = state.dstack.top();

            if (x < *m) *m = x;
            state.dstack.pop();

            ++i;
            break;

        case MinConst:

            ++i;
            narg = nodeStream[i];
            y = constStream[nodeStream[++i]];

            for (int j = narg - 1; j >= 0; --j)
            {
                x = state.dstack[j];
                if (x < y)
                {
                    y = x;
                }
            }

            state.dstack.pop(narg - 1);
            state.dstack.top() = y;

            ++i;
            break;

        case Min2Const:

            m = &state.dstack.top();
            x = constStream[nodeStream[++i]];
            if (x < *m) *m = x;

            ++i;
            break;
        
        case Spot:

            state.dstack.push(scen.spot);

            ++i;
            break;

        case Var:
        
            state.dstack.push(state.variables[nodeStream[++i]]);

            ++i;
            break;

        case Const:

            state.dstack.push(constStream[nodeStream[++i]]);

            ++i;
            break;

        case Assign:
        
            idx = nodeStream[++i];
            state.variables[idx] = state.dstack.top();
            state.dstack.pop();

            ++i;
            break;

        case AssignConst:

            val = constStream[nodeStream[++i]];
            idx = nodeStream[++i];
            state.variables[idx] = val;

            ++i;
            break;

        case Pays:

            ++i;
            idx = nodeStream[i];
            state.variables[idx] += state.dstack.top() / scen.numeraire;
            state.dstack.pop();

            ++i;
            break;

        case PaysConst:

            val = constStream[nodeStream[++i]];
            idx = nodeStream[++i];
            state.variables[idx] += val / scen.numeraire;

            ++i;
            break;

        case If:

            if (state.bstack.top())
            {
                i += 2;
            }
            else
            {
                i = nodeStream[++i];
            }

            state.bstack.pop();

        break;

        case IfElse:

            if (!state.bstack.top())
            {
                i = nodeStream[++i];
            }
            else
            {
                //  Cannot avoid nested call here
                evalCompiled(nodeStream, constStream, dataStream, scen, state, i+3, nodeStream[i+1]);
                i = nodeStream[i + 2];
            }

            state.bstack.pop();

            break;

        case Equal:

            state.bstack.push(state.dstack.top() == 0.0);
            state.dstack.pop();

            ++i;
            break;

        case Sup:

            state.bstack.push(state.dstack.top() > 0.0);
            state.dstack.pop();

            ++i;
            break;

        case SupEqual:

            state.bstack.push(state.dstack.top() >= 0.0);
            state.dstack.pop();

            ++i;
            break;

        case And:

            if (state.bstack[1])
            {
                state.bstack[1] = state.bstack.top();
            }
            state.bstack.pop();

            ++i;
            break;

        case Or:

            if (!state.bstack[1])
            {
                state.bstack[1] = state.bstack.top();
            }
            state.bstack.pop();

            ++i;
            break;

        case Smooth:

            //	Eval the condition
            x = state.dstack[3];
            halfEps = 0.5*state.dstack.top();
            vPos = state.dstack[2];
            vNeg = state.dstack[1];

            state.dstack.pop(3);

            //	Left
            if (x < -halfEps) state.dstack.top() = vNeg;

            //	Right
            if (x < -halfEps) state.dstack.top() = vPos;

            //	Fuzzy
            else
            {
                state.dstack.top() = vNeg + 0.5 * (vPos - vNeg) / halfEps * (x + halfEps);
            }

            ++i;
            break;

        case Sqrt:

            state.dstack.top() = sqrt(state.dstack.top());

            ++i;
            break;

        case Log:

            state.dstack.top() = log(state.dstack.top());

            ++i;
            break;

        case Not:

            state.bstack.top() = !state.bstack.top();

            ++i;
            break;

        case Uminus:

            state.dstack.top() = -state.dstack.top();

            ++i;
            break;

        case True:

            state.bstack.push(true);

            ++i;
            break;

        case False:

            state.bstack.push(false);

            ++i;
            break;
        }
    }
}