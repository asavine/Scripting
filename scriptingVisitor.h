
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
#pragma warning(disable : 4250)

#include "visitorList.h"

//  Base visitors

//  V = concrete visitor 
//  V overloads visits to particular node types
//  The non-overloaded node types are caught in the base visitor
template <class V>
class Visitor
{

protected:

    //  Visit all the arguments with concrete type V visitor 
    template <class NODE>
    void visitArguments(NODE& node)
    {
        for (auto& arg : node.arguments)
        {
            //  static_cast : visit as concrete visitor 
            arg->accept(static_cast<V&> (*this));
        }
    }

public:

    //  Catch all = visit arguments
    template <class NODE>
    void visit(NODE& node)
    {
        visitArguments(node);
    }
};

//  Const visitor

template <class V>
class constVisitor
{

protected:

    template <class NODE>
    void visitArguments(const NODE& node)
    {
        for (const auto& arg : node.arguments)
        {
            //  static_cast : visit as visitor of type V
            arg->accept(static_cast<V&> (*this));
        }
    }


public:

    template <class NODE>
    void visit(const NODE& node)
    {
        visitArguments(node);
    }

};

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