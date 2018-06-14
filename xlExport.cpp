
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
#include "scriptingProduct.h"
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
    XLOper12 *xComp,
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

        bool comp = bool(*xComp);

        bool normal = bool( *xNormal);

		vector<string>			varNames;
		vector<double>			varVals;

		simpleBsScriptVal( today, spot, vol, rate, normal, events, numSim, seed, fuzzy, eps, skipDoms, comp, varNames, varVals);

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

extern "C" __declspec(dllexport) XLOper12* TestBar(
    XLOper12 *xToday,
    XLOper12 *xSpot,
    XLOper12 *xVol,
    XLOper12 *xRate,
    XLOper12 *xMat,
    XLOper12 *xStrike,
    XLOper12 *xBar,
    XLOper12 *xBarDates,
    XLOper12 *xNumSim,
    XLOper12 *xSeed,
    XLOper12 *xNormal) {

    try {

        Date today = int(*xToday);
        double spot = double(*xSpot);
        double vol = double(*xVol);
        double rate = double(*xRate);
        unsigned numSim = (unsigned) int(*xNumSim);
        unsigned seed = (unsigned) int(*xSeed);

        if (today == 0 || spot == 0 || vol == 0 || numSim == 0) throw exception();

        Date mat = int(*xMat);
        double strike = double(*xStrike), bar = double(*xBar);

        vector<Date> barDates;
        for (unsigned i = 0; i<xBarDates->Size(); ++i)
        {
            if (int((*xBarDates)(i)) > 0) barDates.push_back(int((*xBarDates)(i)));
        }

        bool normal = bool(*xNormal);

        double res;

        simpleBsBarVal(today, spot, vol, rate, normal, mat, barDates, strike, bar, numSim, seed, res);

        XLOper12 xRes(1, 1);
        xRes(0, 0) = XLOper12(res);

        return return_xloper_raw_ptr(xRes);

    }
    catch (const exception& e) {

        XLOper12 xRes(e.what());
        return return_xloper_raw_ptr(xRes);
    }
    catch (...) {

        return &error;
    }

}

extern "C" __declspec(dllexport) XLOper12* TestAsian(
    XLOper12 *xToday,
    XLOper12 *xSpot,
    XLOper12 *xVol,
    XLOper12 *xRate,
    XLOper12 *xBarDates,
    XLOper12 *xNumSim,
    XLOper12 *xSeed,
    XLOper12 *xNormal) {

    try {

        Date today = int(*xToday);
        double spot = double(*xSpot);
        double vol = double(*xVol);
        double rate = double(*xRate);
        unsigned numSim = (unsigned) int(*xNumSim);
        unsigned seed = (unsigned) int(*xSeed);

        if (today == 0 || spot == 0 || vol == 0 || numSim == 0) throw exception();

        vector<Date> barDates;
        for (unsigned i = 0; i<xBarDates->Size(); ++i)
        {
            if (int((*xBarDates)(i)) > 0) barDates.push_back(int((*xBarDates)(i)));
        }

        bool normal = bool(*xNormal);

        double res;

        simpleBsAsianVal(today, spot, vol, rate, normal, barDates, numSim, seed, res);

        XLOper12 xRes(1, 1);
        xRes(0, 0) = XLOper12(res);

        return return_xloper_raw_ptr(xRes);

    }
    catch (const exception& e) {

        XLOper12 xRes(e.what());
        return return_xloper_raw_ptr(xRes);
    }
    catch (...) {

        return &error;
    }

}

extern "C" __declspec(dllexport) XLOper12* TestCalls(
    XLOper12 *xToday,
    XLOper12 *xSpot,
    XLOper12 *xVol,
    XLOper12 *xRate,
    XLOper12 *xMat,
    XLOper12 *xStrikes,
    XLOper12 *xNumSim,
    XLOper12 *xSeed,
    XLOper12 *xNormal) {

    try {

        Date today = int(*xToday);
        double spot = double(*xSpot);
        double vol = double(*xVol);
        double rate = double(*xRate);
        unsigned numSim = (unsigned) int(*xNumSim);
        unsigned seed = (unsigned) int(*xSeed);

        if (today == 0 || spot == 0 || vol == 0 || numSim == 0) throw exception();

        Date mat = int(*xMat);

        vector<double> strikes;
        for (unsigned i = 0; i<xStrikes->Size(); ++i)
        {
            if (int((*xStrikes)(i)) > 0) strikes.push_back(double((*xStrikes)(i)));
        }

        bool normal = bool(*xNormal);

        vector<double> res;

        simpleBsCallsVal(today, spot, vol, rate, normal, mat, strikes, numSim, seed, res);

        XLOper12 xRes(strikes.size(), 1);
        for (unsigned i = 0; i < strikes.size(); ++i) xRes(i, 0) = XLOper12(res[i]);

        return return_xloper_raw_ptr(xRes);

    }
    catch (const exception& e) {

        XLOper12 xRes(e.what());
        return return_xloper_raw_ptr(xRes);
    }
    catch (...) {

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
		(LPXLOPER12)TempStr12(L"QQQQQQQQQQQQQQ"),
		(LPXLOPER12)TempStr12(L"TestScript"),
		(LPXLOPER12)TempStr12(L"today,spot,vol,rate,{evtDates},{events},numSim,[Seed],[FuzzyEval],[FuzzyEps],[SkipDomains],[Compile],[Normal]"),
		(LPXLOPER12)TempStr12(L"1"),
		(LPXLOPER12)TempStr12(L"myOwnCppFunctions"),
		(LPXLOPER12)TempStr12(L""),
		(LPXLOPER12)TempStr12(L""),
		(LPXLOPER12)TempStr12(L""),
		(LPXLOPER12)TempStr12(L""));

    Excel12f(xlfRegister, 0, 11, (LPXLOPER12)&xDLL,
        (LPXLOPER12)TempStr12(L"TestBar"),
        (LPXLOPER12)TempStr12(L"QQQQQQQQQQQQ"),
        (LPXLOPER12)TempStr12(L"TestBar"),
        (LPXLOPER12)TempStr12(L"today,spot,vol,rate,mat,strike,bar,barDates,numSim,[Seed],[Normal]"),
        (LPXLOPER12)TempStr12(L"1"),
        (LPXLOPER12)TempStr12(L"myOwnCppFunctions"),
        (LPXLOPER12)TempStr12(L""),
        (LPXLOPER12)TempStr12(L""),
        (LPXLOPER12)TempStr12(L""),
        (LPXLOPER12)TempStr12(L""));

    Excel12f(xlfRegister, 0, 11, (LPXLOPER12)&xDLL,
        (LPXLOPER12)TempStr12(L"TestAsian"),
        (LPXLOPER12)TempStr12(L"QQQQQQQQQ"),
        (LPXLOPER12)TempStr12(L"TestAsian"),
        (LPXLOPER12)TempStr12(L"today,spot,vol,rate,barDates,numSim,[Seed],[Normal]"),
        (LPXLOPER12)TempStr12(L"1"),
        (LPXLOPER12)TempStr12(L"myOwnCppFunctions"),
        (LPXLOPER12)TempStr12(L""),
        (LPXLOPER12)TempStr12(L""),
        (LPXLOPER12)TempStr12(L""),
        (LPXLOPER12)TempStr12(L""));

    Excel12f(xlfRegister, 0, 11, (LPXLOPER12)&xDLL,
        (LPXLOPER12)TempStr12(L"TestCalls"),
        (LPXLOPER12)TempStr12(L"QQQQQQQQQQ"),
        (LPXLOPER12)TempStr12(L"TestCalls"),
        (LPXLOPER12)TempStr12(L"today,spot,vol,rate,mat,strikes,numSim,[Seed],[Normal]"),
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
