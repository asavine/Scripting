
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
#include "xlApi.h"
static myXlOper error = ErrorNotAvailable12;

//	Wrappers

extern "C" __declspec(dllexport)
double xMultiply2Numbers(double x, double y)
{
	return x * y;
}

#include "visitorHeaders.h"
#include "scriptingModel.h"

extern "C" __declspec(dllexport) myXlOper* TestScript(
	myXlOper *xToday,
	myXlOper *xSpot,
	myXlOper *xVol,
	myXlOper *xRate,
	myXlOper *xEvtDates,
	myXlOper *xEvts,
	myXlOper *xNumSim,
	myXlOper *xSeed,
	myXlOper *xFuzzy,
	myXlOper *xEps,
	myXlOper *xSkipDoms,
    myXlOper *xComp,
    myXlOper *xNormal){
	
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

		myXlOper res( unsigned(varNames.size()), 2);

		for( unsigned i=0; i<varNames.size(); ++i)
		{
			res(i,0) = myXlOper( varNames[i]);
			res(i,1) = myXlOper( varVals[i]);
		}

		return return_xloper_raw_ptr (res);

	} 
	catch (const exception& e){
		
		myXlOper res( e.what());
		return return_xloper_raw_ptr (res);
	}
	catch (...){
				
		return &error;
	}

}

extern "C" __declspec(dllexport) myXlOper* TestBar(
    myXlOper *xToday,
    myXlOper *xSpot,
    myXlOper *xVol,
    myXlOper *xRate,
    myXlOper *xMat,
    myXlOper *xStrike,
    myXlOper *xBar,
    myXlOper *xBarDates,
    myXlOper *xNumSim,
    myXlOper *xSeed,
    myXlOper *xNormal) {

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

        myXlOper xRes(1, 1);
        xRes(0, 0) = myXlOper(res);

        return return_xloper_raw_ptr(xRes);

    }
    catch (const exception& e) {

        myXlOper xRes(e.what());
        return return_xloper_raw_ptr(xRes);
    }
    catch (...) {

        return &error;
    }

}

extern "C" __declspec(dllexport) myXlOper* TestAsian(
    myXlOper *xToday,
    myXlOper *xSpot,
    myXlOper *xVol,
    myXlOper *xRate,
    myXlOper *xBarDates,
    myXlOper *xNumSim,
    myXlOper *xSeed,
    myXlOper *xNormal) {

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

        myXlOper xRes(1, 1);
        xRes(0, 0) = myXlOper(res);

        return return_xloper_raw_ptr(xRes);

    }
    catch (const exception& e) {

        myXlOper xRes(e.what());
        return return_xloper_raw_ptr(xRes);
    }
    catch (...) {

        return &error;
    }

}

extern "C" __declspec(dllexport) myXlOper* TestCalls(
    myXlOper *xToday,
    myXlOper *xSpot,
    myXlOper *xVol,
    myXlOper *xRate,
    myXlOper *xMat,
    myXlOper *xStrikes,
    myXlOper *xNumSim,
    myXlOper *xSeed,
    myXlOper *xNormal) {

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

        myXlOper xRes(strikes.size(), 1);
        for (unsigned i = 0; i < strikes.size(); ++i) xRes(i, 0) = myXlOper(res[i]);

        return return_xloper_raw_ptr(xRes);

    }
    catch (const exception& e) {

        myXlOper xRes(e.what());
        return return_xloper_raw_ptr(xRes);
    }
    catch (...) {

        return &error;
    }

}

//	Registers

extern "C" __declspec(dllexport) int xlAutoOpen(void)
{
	myXlOper xDLL;

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
