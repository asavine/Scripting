#pragma once

//  Declaration
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
#define VISITORS VarIndexer, ConstProcessor, ConstCondProcessor, IfProcessor, DomainProcessor, Debugger, Evaluator<double>, Compiler, FuzzyEvaluator<double>
