
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

//  Base
#include "scriptingNodes.h"

//  All the nodes and visitors
#include "visitorHeaders.h"

//  Parser
#include "scriptingParser.h"

//  Scenarios
#include "scriptingScenarios.h"

using namespace std;
#include <vector>

//	Date class from your date library
//	class Date;
using Date = int;

//  The Product class is the top level API for scripted instruments
//  Client code addresses scripting from here only

class Product
{
	vector<Date>		        myEventDates;
	vector<Event>		        myEvents;
    vector<string>		        myVariables;

    //  Compiled form
    vector<vector<int>>         myNodeStreams;
    vector<vector<double>>      myConstStreams;
    vector<vector<const void*>> myDataStreams;

public:

	//	Accessors

	//	Access event dates
	const vector<Date>& eventDates()
	{
		return myEventDates;
	}
	//	Events are not accessed, remain encapsulated in the product

	//	Access number of variables (vector size) and names
	const vector<string>& varNames() const
	{
		return myVariables;
	}

	//	Factories

	//	Evaluator factory
	template <class T>
    Evaluator<T> buildEvaluator()
	{
		//	Move
		return Evaluator<T>( myVariables.size());
	}
    template <class T>
	FuzzyEvaluator<T> buildFuzzyEvaluator( const size_t maxNestedIfs, const double defEps)
	{
		return FuzzyEvaluator<T>( myVariables.size(), maxNestedIfs, defEps);
	}

	//	Scenario factory
	template <class T>
    unique_ptr<Scenario<T>> buildScenario()
	{
		//	Move
		return unique_ptr<Scenario<T>>( new Scenario<T>( myEventDates.size()));
	}

	//	Parser : builds a scripted product out of text scripts

	//	Build events out of event strings
	template<class EvtIt>
	//	Takes begin and end iterators on pairs of dates and corresponding event strings
	//		as from a map<Date,string>
	void parseEvents( EvtIt begin, EvtIt end)
	{
		//	Copy event dates and parses event strings sequentially
		for( EvtIt evtIt = begin; evtIt != end; ++evtIt)
		{
			//	Copy event date
			myEventDates.push_back( evtIt->first);
			//	Parse event string
			myEvents.push_back( parse( evtIt->second)); 
		}
	}

	//	Visitors

	//	Sequentially visit all statements in all events
	template<class V>
    void visit(Visitor<V>& v)
	{
		//	Loop over events
		for( auto& evt : myEvents)
		{
			//	Loop over statements in event
			for( auto& stat : evt)
			{
				//	Visit statement
                stat->accept(static_cast<V&>(v));
			}
		}
	}

    //  Same for const visitors
    template<class V>
    void visit(constVisitor<V>& v) const
    {
        //	Loop over events
        for (const auto& evt : myEvents)
        {
            //	Loop over statements in event
            for (const auto& stat : evt)
            {
                //	Visit statement
                stat->accept(static_cast<V&>(v));
            }
        }
    }
    
    //	Evaluate the product in a given scenario with the given evaluator
    //  The product must be pre-processed first
    template <class T, class Eval>
	void evaluate( const Scenario<T>& scen, Eval& eval) const
	{
		//	Set scenario
		eval.setScenario( &scen);

		//	Initialize all variables
		eval.init();

		//	Loop over events
		for(size_t i=0; i<myEvents.size(); ++i)
		{
			//	Set current event
			eval.setCurEvt( i);
			
			//	Loop over statements in event
			for( const auto& stat : myEvents[i])
			{
				//	Visit statement
				stat->accept(eval);
			}
		}
	}

    //	Evaluate all compiled statements in all events
    //  The product must be pre-processed and compiled first
    template <class T>
    void evaluateCompiled(
        const Scenario<T>& scen, 
        EvalState<T>& state) const
    {
        //	Initialize state
        state.init();

        //	Loop over events
        for (size_t i = 0; i<myEvents.size(); ++i)
        {
            //	Evaluate the compiled events
            evalCompiled(myNodeStreams[i], myConstStreams[i], myDataStreams[i], scen[i], state);
        }
    }
    
    //  Processors

    //	Index all variables
	void indexVariables()
	{
		//	Our indexer
		VarIndexer indexer;
		
		//	Visit all trees, iterate on events and statements
		visit( indexer);

		//	Get result moved in myVariables
		myVariables = indexer.getVarNames();
	}

	//	If processing, returns max number of nested ifs
	size_t ifProcess()
	{
		//	The fuzzy var processor
		IfProcessor ifProc;

		//	Visit
		visit( ifProc);

		//	Return
		return ifProc.maxNestedIfs();
	}

	//	Domain processing
	void domainProcess( const bool fuzzy)
	{
		//	The domain processor
		DomainProcessor domProc( myVariables.size(), fuzzy);

		//	Visit
		visit( domProc);
	}

    //  Const process, identify (but not remove) all constant nodes
    void constProcess()
    {
        ConstProcessor cProc( myVariables.size());

        //	Visit
        visit(cProc);
    }

	//	Const condition process, remove all conditions that are always true or always false
	void constCondProcess()
	{
		//	The const cond processor
		ConstCondProcessor ccProc;

		//	Visit
		//	Note that changes the structure of the tree, hence a special function must be called 
		//		from the top of each tree
		//	Loop over events
		for( auto& evt : myEvents)
		{
			//	Loop over statements in event
			for( auto& stat : evt)
			{
				//	Visit statement
				ccProc.processFromTop( stat);
			}
		}
	}

    //	Compile into streams of instructions, constants and data, one per event date
    void compile()
    {
        //  First, identify constants
        constProcess();

        //  Clear
        myNodeStreams.clear();
        myConstStreams.clear();
        myDataStreams.clear();
        
        //  One per event date
        myNodeStreams.reserve(myEvents.size());
        myConstStreams.reserve(myEvents.size());
        myDataStreams.reserve(myEvents.size());

        //	Visit
        for (auto& evt : myEvents)
        {
            //	The compiler
            Compiler comp;

            //	Loop over statements in event
            for (auto& stat : evt)
            {
                //	Visit statement
                stat->accept(comp);
            }

            //  Get compiled 
            myNodeStreams.push_back(comp.nodeStream());
            myConstStreams.push_back(comp.constStream());
            myDataStreams.push_back(comp.dataStream());
        }
    }

	//	All preprocessing
	size_t preProcess( const bool fuzzy, const bool skipDoms)
	{
		indexVariables();

        size_t maxNestedIfs = 0;
		
		if( fuzzy || !skipDoms)
		{
			maxNestedIfs = ifProcess();
			domainProcess( fuzzy);
			constCondProcess();
		}

		return maxNestedIfs;
	}

	//	Debug whole product
	void debug( ostream& ost)
	{
        size_t v = 0;
		for( auto it=myVariables.begin(); it!=myVariables.end(); ++it)
		{
			ost << "Var[" << v++ << "] = " << *it << endl;
		}

		Debugger d;
        size_t e=0;
		for( auto& evtIt : myEvents)
		{
			ost << "Event: " << ++e << endl;
			unsigned s=0;
			for( auto& stat : evtIt)
			{
				stat->accept(d);
				ost << "Statement: " << ++s << endl;
				ost << d.getString() << endl;
			}
		}
	}
};