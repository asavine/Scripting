
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

#include "visitorList.h"

struct Node;

//  Base visitors

//  V = concrete visitor (CRTP)
//  V overloads visits to particular node types
//  Other visits are caught in the base visitor
template <class V>
struct Visitor
{
    //  Visit a node with concrete visitor
    //      use this to hide the statc_cast
    void visitNode(Node& node)
    {
        //  static_cast : visit as concrete visitor 
        node.accept(static_cast<V&> (*this));
    }

    //  Visit all the arguments with concrete (type V) visitor
    void visitArguments(Node& node)
    {
        for (auto& arg : node.arguments)
        {
            //  static_cast : visit as concrete visitor 
            arg->accept(static_cast<V&> (*this));
        }
    }

    //  Default catch all = visit arguments
    void visit(Node& node)
    {
        //  V does not declare a visit to that node type,
        //      either const or non const - fall back to default visit arguments
        visitArguments(node);
    }
};

//  Const visitor

template <class V>
struct constVisitor
{
    void visitNode(const Node& node)
    {
        //  static_cast : visit as concrete visitor 
        node.accept(static_cast<V&> (*this));
    }

    void visitArguments(const Node& node)
    {
        for (const auto& arg : node.arguments)
        {
            //  static_cast : visit as visitor of type V
            arg->accept(static_cast<V&> (*this));
        }
    }

    template <class NODE>
    void visit(const NODE& node)
    {
        //  Const visitors cannot declare non const visits: we check that and produce a compilation error
        static_assert(!hasNonConstVisit<V>::forNodeType<NODE>(), "CONST VISITOR DECLARES A NON-CONST VISIT");

        //  V does not declare a visit to that node type,
        //      either const or non const - fall back to visiting arguments
        visitArguments(node);
    }
};

//  Visitable classes

//  Base Node must inherit visitableBase 
//      so it (automatically) declares pure virtual accept methods for all visitors on the list

//  Concrete Nodes must inherit Visitable
//      so they (automatically) declare overrides accepts for all visitors on the list

/*
Note that despite the complexity of that meta code, its use is trivial:

To declare struct Node : VisitableBase<Visitor1, Visitor2, ...> {};

    is only sugar for :
    
    struct Node
    {
        virtual void accept(Visitor1&) = 0;
        virtual void accept(Visitor2&) = 0;
        ...
    };

And to declare a concrete node (say, NodeAdd) as NodeAdd : Visitable<Node, AddNode, Visitor1, Visitor2, ...> {};

    is sugar for:

    struct NodeAdd : Node
    {
        void accept(Visitor1& v) override
        {
            v.visit(*this);
        }

        void accept(Visitor2& v) override
        {
            v.visit(*this);
        }

        ...
    };

*/

//  Visitable Base

template <typename V, bool CONST>
struct BaseImpl;

template <typename V>
struct BaseImpl<V, false>
{
    virtual void accept(V& visitor) = 0;
};

template <typename V>
struct BaseImpl<V, true>
{
    virtual void accept(V& visitor) const = 0;
};

template <typename... Vs>
struct VisitableBase;

template <typename V>
struct VisitableBase<V> : BaseImpl<V, isVisitorConst<V>()>
{
    using BaseImpl<V, isVisitorConst<V>()>::accept;
};

template <typename V, typename... Vs>
struct VisitableBase<V, Vs...> : VisitableBase<V>, VisitableBase<Vs...>
{
    using VisitableBase<V>::accept;
    using VisitableBase<Vs...>::accept;
};

//  Visitable

template <typename V, typename Base, typename Concrete, bool CONST, typename... Vs>
struct DerImpl;

template <typename V, typename Base, typename Concrete>
struct DerImpl<V, Base, Concrete, false> : Base
{
    void accept(V& visitor) override
    {
        visitor.visit(static_cast<Concrete&>(*this));
    }
};

template <typename V, typename Base, typename Concrete>
struct DerImpl<V, Base, Concrete, true> : Base
{
    void accept(V& visitor) const override
    {
        visitor.visit(static_cast<const Concrete&>(*this));
    }
};

template <typename Base, typename Concrete, typename... Vs>
struct Visitable;

template <typename Base, typename Concrete, typename V>
struct Visitable<Base, Concrete, V> : DerImpl<V, Base, Concrete, isVisitorConst<V>()>
{
    using DerImpl<V, Base, Concrete, isVisitorConst<V>()>::accept;
};

template <typename V, typename Base, typename Concrete, typename... Vs>
struct DerImpl<V, Base, Concrete, false, Vs...> : Visitable<Base, Concrete, Vs...>
{
    using Visitable<Base, Concrete, Vs...>::accept;

    void accept(V& visitor) override
    {
        visitor.visit(static_cast<Concrete&>(*this));
    }
};

template <typename V, typename Base, typename Concrete, typename... Vs>
struct DerImpl<V, Base, Concrete, true, Vs...> : Visitable<Base, Concrete, Vs...>
{
    using Visitable<Base, Concrete, Vs...>::accept;

    void accept(V& visitor) const override
    {
        visitor.visit(static_cast<const Concrete&>(*this));
    }
};

template <typename Base, typename Concrete, typename V, typename... Vs>
struct Visitable<Base, Concrete, V, Vs...> : DerImpl<V, Base, Concrete, isVisitorConst<V>(), Vs...>
{
    using  DerImpl<V, Base, Concrete, isVisitorConst<V>(), Vs...>::accept;
};

/*

Without const correctness : to be removed

//  Visitablebase classes
//  Base Node must inherit visitableBase
//  Concrete Nodes must inherit Visitable

template <typename... Vs>
struct VisitableBase;

template <typename V>
struct VisitableBase<V>
{
virtual void accept(V& visitor) = 0;
virtual void accept(V& visitor) const = 0;
};

template <typename V, typename... Vs>
struct VisitableBase<V, Vs...> : VisitableBase<Vs...>
{
using VisitableBase<Vs...>::accept;
virtual void accept(V& visitor) = 0;
virtual void accept(V& visitor) const = 0;
};

//  Visitable

template <typename Base, typename Concrete, typename... Vs>
struct Visitable;

template <typename Base, typename Concrete, typename V>
struct Visitable<Base, Concrete, V> : Base
{
void accept(V& visitor) override
{
visitor.visit(static_cast<Concrete&>(*this));
}

void accept(V& visitor) const override
{
visitor.visit(static_cast<const Concrete&>(*this));
}

};

template <typename Base, typename Concrete, typename V, typename... Vs>
struct Visitable<Base, Concrete, V, Vs...> : Visitable<Base, Concrete, Vs...>
{
using  Visitable<Base, Concrete, Vs...>::accept;

void accept(V& visitor) override
{
visitor.visit(static_cast<Concrete&>(*this));
}

void accept(V& visitor) const override
{
visitor.visit(static_cast<const Concrete&>(*this));
}

};


*/