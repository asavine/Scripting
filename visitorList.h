#pragma once

//  Declaration of all visitors
class Debugger;
class VarIndexer;
class ConstProcessor;
template <class T> class Evaluator;
class Compiler;
class ConstCondProcessor;
class IfProcessor;
class DomainProcessor;
template <class T> class FuzzyEvaluator;

//  List

//  Modifying visitors
#define MVISITORS VarIndexer, ConstProcessor, ConstCondProcessor, IfProcessor, DomainProcessor

//  Const visitors
#define CVISITORS Debugger, Evaluator<double>, Compiler, FuzzyEvaluator<double>

//  All visitors
#define VISITORS MVISITORS , CVISITORS

//  Various meta-programming utilities

//  Is V a const visitor?

#include "packIncludes.h"

template <class V>
inline constexpr bool isVisitorConst()
{
    return Pack<CVISITORS>::includes<V>();
}

//  Use : isVisitorConst<V>() returns true if V is const, or false
//  isVisitorConst() resolves at compile time

//  Does V have a visit for a const N? A non-const N?

template <typename V>
struct hasNonConstVisit
{
    template <typename N, void (V::*) (N&) = &V::visit>
    static bool constexpr forNodeType()
    {
        return true;
    }

    template <typename N>
    static bool constexpr forNodeType(...)
    {
        return false;
    }
};

template <typename V>
struct hasConstVisit
{
    template <typename N, void (V::*) (const N&) = &V::visit>
    static bool constexpr forNodeType()
    {
        return true;
    }

    template <typename N>
    static bool constexpr forNodeType(...)
    {
        return false;
    }
};

//  Use: hasConstVisit<V>::forNodeType<N>() returns true 
//      if V declares a method void visit(const N&) 
//      false otherwise
//  Everything resolves at compile time
//  hasNonConstVisit is the same: hasNonConstVisit<V>::forNodeType<N>()
//      returns true if V declares void visit(N&)

