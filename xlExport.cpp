
#include <windows.h>
#include "xlcall.h"
#include "framework.h"

//	Additional xl utilities, useful for ranges of strings
#include "xlOper12.h"
static XLOper12 error = ErrorNotAvailable12;

//	Wrappers

extern "C" __declspec(dllexport)
double xMultiply2Numbers(double x, double y)
{
	return x * y;
}

#include "scriptingParser.h"
#include "scriptingDebugger.h"
#include "scriptingEvaluator.h"
#include "scriptingEvents.h"
#include "scriptingModel.h"

extern "C" __declspec(dllexport) XLOper12* TestScript(
	XLOper12 *xToday,
	XLOper12 *xSpot,
	XLOper12 *xVol,
	XLOper12 *xRate,
	XLOper12 *xEvtDates,
	XLOper12 *xEvts,
	XLOper12 *xNumSim,
	XLOper12 *xSeed,
	XLOper12 *xFuzzy,
	XLOper12 *xEps,
	XLOper12 *xSkipDoms,
    XLOper12 *xNormal){
	
	try{

		Date today = int( *xToday);
		double spot = double( *xSpot);
		double vol = double( *xVol);
		double rate = double( *xRate);
		unsigned numSim = (unsigned) int( *xNumSim);
		unsigned seed = (unsigned) int( *xSeed);

		unsigned nEvt = xEvts->Size();
		if( nEvt != xEvtDates->Size()) throw "Event dates and event have different dimensions";

		if( today == 0 || spot == 0 || vol == 0 || numSim == 0 || nEvt == 0) throw exception();

		map<Date,string> events;
		for( unsigned i=0; i<nEvt; ++i)
		{
			if( int( (*xEvtDates)(i)) > 0) events[int( (*xEvtDates)(i))] += string( (*xEvts)(i)) + " ";
		}

		if( !events.size()) throw "No events";

		bool fuzzy = bool( *xFuzzy);
		double eps = 0.0001;
		if( fuzzy) eps = double( *xEps);
		bool skipDoms = bool( *xSkipDoms);

        bool normal = bool( *xNormal);

		vector<string>			varNames;
		vector<double>			varVals;

		simpleBsScriptVal( today, spot, vol, rate, normal, events, numSim, seed, fuzzy, eps, skipDoms, varNames, varVals);

		XLOper12 res( varNames.size(), 2);

		for( unsigned i=0; i<varNames.size(); ++i)
		{
			res(i,0) = XLOper12( varNames[i]);
			res(i,1) = XLOper12( varVals[i]);
		}

		return return_xloper_raw_ptr (res);

	} 
	catch (const exception& e){
		
		XLOper12 res( e.what());
		return return_xloper_raw_ptr (res);
	}
	catch (...){
				
		return &error;
	}

}

//	Registers

extern "C" __declspec(dllexport) int xlAutoOpen(void)
{
	XLOPER12 xDLL;

	Excel12f(xlGetName, &xDLL, 0);

	Excel12f(xlfRegister, 0, 11, (LPXLOPER12)&xDLL,
		(LPXLOPER12)TempStr12(L"xMultiply2Numbers"),
		(LPXLOPER12)TempStr12(L"BBB"),
		(LPXLOPER12)TempStr12(L"xMultiply2Numbers"),
		(LPXLOPER12)TempStr12(L"x, y"),
		(LPXLOPER12)TempStr12(L"1"),
		(LPXLOPER12)TempStr12(L"myOwnCppFunctions"),
		(LPXLOPER12)TempStr12(L""),
		(LPXLOPER12)TempStr12(L""),
		(LPXLOPER12)TempStr12(L"Multiplies 2 numbers"),
		(LPXLOPER12)TempStr12(L"number 1, number 2"));

	Excel12f(xlfRegister, 0, 11, (LPXLOPER12)&xDLL,
		(LPXLOPER12)TempStr12(L"TestScript"),
		(LPXLOPER12)TempStr12(L"QQQQQQQQQQQQQ"),
		(LPXLOPER12)TempStr12(L"TestScript"),
		(LPXLOPER12)TempStr12(L"today,spot,vol,rate,{evtDates},{events},numSim,[Seed],[FuzzyEval],[FuzzyEps],[SkipDomains],[Normal]"),
		(LPXLOPER12)TempStr12(L"1"),
		(LPXLOPER12)TempStr12(L"myOwnCppFunctions"),
		(LPXLOPER12)TempStr12(L""),
		(LPXLOPER12)TempStr12(L""),
		(LPXLOPER12)TempStr12(L""),
		(LPXLOPER12)TempStr12(L""));

	/* Free the XLL filename */
	Excel12f(xlFree, 0, 1, (LPXLOPER12)&xDLL);

	return 1;
}
