#pragma once

#include "scriptingNodes.h"
#include "scriptingParser.h"
#include "scriptingVisitor.h"
#include "scriptingVarIndexer.h"
#include "scriptingDebugger.h"
#include "scriptingEvaluator.h"
#include "scriptingFuzzyEval.h"
#include "scriptingScenarios.h"
#include "scriptingDomainProc.h"
#include "scriptingConstCondProc.h"

using namespace std;
#include <vector>

typedef ExprTree Statement;

typedef vector<Statement> Event;

//	Date class from your date library
//	class Date;
typedef int Date;

class Product
{
	vector<Date>		myEventDates;
	vector<Event>		myEvents;
	vector<string>		myVariables;

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
    unique_ptr<Evaluator<T>> buildEvaluator()
	{
		//	Move
		return unique_ptr<Evaluator<T>>( new Evaluator<T>( myVariables.size()));
	}
    template <class T>
	unique_ptr<Evaluator<T>> buildFuzzyEvaluator( const size_t maxNestedIfs, const double defEps)
	{
		return unique_ptr<Evaluator<T>>( new FuzzyEvaluator<T>( myVariables.size(), maxNestedIfs, defEps));
	}

	//	Scenario factory
	template <class T>
    unique_ptr<Scenario<T>> buildScenario()
	{
		//	Move
		return unique_ptr<Scenario<T>>( new Scenario<T>( myEventDates.size()));
	}

	//	Parser

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
	void visit( Visitor& v)
	{
		//	Loop over events
		for( auto& evt : myEvents)
		{
			//	Loop over statements in event
			for( auto& stat : evt)
			{
				//	Visit statement
				v.visit( stat);
			}
		}
	}

	//	Evaluate all statements in all events
    template <class T>
	void evaluate( const Scenario<T>& scen, Evaluator<T>& eval) const
	{
		//	Set scenario
		eval.setScenario( &scen);

		//	Initialize all variables
		eval.init();

		//	Loop over events
		for(auto i=0; i<myEvents.size(); ++i)
		{
			//	Set current event
			eval.setCurEvt( i);
			
			//	Loop over statements in event
			for( auto& statIt : myEvents[i])
			{
				//	Visit statement
				eval.visit( statIt);
			}
		}
	}

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
		for( auto& it=myVariables.begin(); it!=myVariables.end(); ++it)
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
				d.visit( stat);
				ost << "Statement: " << ++s << endl;
				ost << d.getString() << endl;
			}
		}
	}
};

/*
class Date
{
public:
	int					myDaysFromEpoch;
};
*/