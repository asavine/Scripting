#pragma once

#include "scriptingScenarios.h"
#include "scriptingEvents.h"
#include "scriptingFuzzyEval.h"

#include "cpp11basicRanGen.h"

//  Base model for Monte-Carlo simulations
template <class T>
struct Model
{
	//	Clone
	virtual unique_ptr<Model> clone() const = 0;

    //  Initialize simulation dates
    virtual void initSimDates(const vector<Date>& simDates) = 0;

    //  Number of Gaussian numbers required for one path
    virtual size_t dim() const = 0;
    
    //  Apply the model SDE
    virtual void applySDE(
        const vector<double>&   G,              //  Gaussian numbers, dimension dim()
        vector<T>&              spots,          //  Populate spots for each event date
        vector<T>&              numeraires)     //  Populate numeraire for each event date
            const = 0;
};

template <class T>
class SimpleBlackScholes : public Model<T>
{
	Date				myToday;
    T                   mySpot;
    T                   myRate;
    T                   myVol;
    T                   myDrift;

	bool				myTime0;	//	If today is among simul dates
	vector<double>		myTimes;
	vector<double>		myDt;
	vector<double>		mySqrtDt;
		
private:

    //	Calculate all deterministic discount factors
    void calcDf( vector<T>& dfs) const
    {
        for (size_t i = 0; i<myTimes.size(); ++i)
            dfs[i] = exp(myRate * myTimes[i]);
    }

public:

	//	Construct with T0, S0, vol and rate
    SimpleBlackScholes( const Date& today, const double spot, const double vol, const double rate)
		: myToday( today), mySpot( spot), myVol( vol), myRate( rate),
        myDrift(-rate+0.5*vol*vol)
    {}

	//	Clone
	virtual unique_ptr<Model> clone() const override
	{
		return unique_ptr<Model>(new SimpleBlackScholes(*this));
	}

    //  Parameter accessors, read only
    const T& spot() { return mySpot; }
    const T& rate() { return myRate; }
    const T& vol() { return myVol; }

	//	Initialize simulation dates
	void initSimDates(const vector<Date>& simDates) override
	{
		myTime0 = simDates[0] == myToday;

		//	Fill array of times
		for( auto dateIt = simDates.begin(); dateIt != simDates.end(); ++dateIt)
		{
			myTimes.push_back( double( *dateIt - myToday) / 365);
		}
		myDt.resize( myTimes.size());
		myDt[0] = myTimes[0];
		for( size_t i=1; i<myTimes.size(); ++i)
		{
			myDt[i] = myTimes[i] - myTimes[i-1];
		}
		mySqrtDt.resize( myTimes.size());
		for(size_t i=0; i<myTimes.size(); ++i)
		{
			mySqrtDt[i] = sqrt( myDt[i]);
		}
	}

    size_t dim() const override { return myTimes.size() - myTime0; }

	//	Simulate one path 
    //  Apply the model SDE
    void applySDE(
        const vector<double>&   G,              //  Gaussian numbers, dimension dim()
        vector<T>&              spots,          //  Populate spots for each event date
        vector<T>&              numeraires)     //  Populate numeraire for each event date
        const override
    {
        //  Compute discount factors
        calcDf( numeraires);
        //  Note the ineffiency: in this case, numeraires could be computed only once

        //  Then apply the SDE
        size_t step = 0;

		//	First step
		spots[0] = myTime0? mySpot: 
			mySpot*exp(-myDrift*myDt[0]+myVol*mySqrtDt[0]*G[step++]);

		//	All steps
		for(size_t i=1; i<myTimes.size(); ++i)
		{
			spots[i] = spots[i-1]
			*exp(-myDrift*myDt[i]+myVol*mySqrtDt[i]*G[step++]);
		}
	}
};

template <class T>
class SimpleBachelier : public Model<T>
{
    Date				myToday;
    T                   mySpot;
    T                   myRate;
    T                   myVol;

    bool				myTime0;	//	If today is among simul dates
    vector<double>		myTimes;
    vector<double>		myDt;
    vector<double>		mySqrtDt;

private:

    //	Calculate all deterministic discount factors
    void calcDf(vector<T>& dfs) const
    {
        for (size_t i = 0; i<myTimes.size(); ++i)
            dfs[i] = exp(myRate * myTimes[i]);
    }

public:

    //	Construct with T0, S0, vol and rate
    SimpleBachelier(const Date& today, const double spot, const double vol, const double rate)
        : myToday(today), mySpot(spot), myVol(vol), myRate(rate)
    {}

	//	Clone
	virtual unique_ptr<Model> clone() const override
	{
		return unique_ptr<Model>(new SimpleBachelier(*this));
	}

    //  Parameter accessors, read only
    const T& spot() { return mySpot; }
    const T& rate() { return myRate; }
    const T& vol() { return myVol; }

    //	Initialize simulation dates
    void initSimDates(const vector<Date>& simDates) override
    {
        myTime0 = simDates[0] == myToday;

        //	Fill array of times
        for (auto dateIt = simDates.begin(); dateIt != simDates.end(); ++dateIt)
        {
            myTimes.push_back(double(*dateIt - myToday) / 365);
        }
        myDt.resize(myTimes.size());
        myDt[0] = myTimes[0];
        for (size_t i = 1; i<myTimes.size(); ++i)
        {
            myDt[i] = myTimes[i] - myTimes[i - 1];
        }
        mySqrtDt.resize(myTimes.size());
        for (size_t i = 0; i<myTimes.size(); ++i)
        {
            mySqrtDt[i] = sqrt(myDt[i]);
        }
    }

    size_t dim() const override { return myTimes.size() - myTime0; }

    //	Simulate one path 
    //  Apply the model SDE
    void applySDE(
        const vector<double>&   G,              //  Gaussian numbers, dimension dim()
        vector<T>&              spots,          //  Populate spots for each event date
        vector<T>&              numeraires)     //  Populate numeraire for each event date
        const override
    {
        //  Compute discount factors
        calcDf(numeraires);
        //  Note the ineffiency: in this case, numeraires could be computed only once

        //  Then apply the SDE
        size_t step = 0;

        //  If rate ~0 the dynamics is simpler and can be simulated more efficiently
        if (fabs(myRate) < 0.0001)
        {
            //	First step
            spots[0] = myTime0 ? mySpot :
                mySpot + myVol * mySqrtDt[0] * G[step++];

            //	All steps
            for (size_t i = 1; i<myTimes.size(); ++i)
            {
                spots[i] = spots[i - 1] + myVol * mySqrtDt[i] * G[step++];
            }
        }
        //  General dynamics with non-zero rates
        else
        {
            //	First step
            spots[0] = myTime0 ? mySpot :
                mySpot * exp(myRate * myDt[0]) + myVol * sqrt ((exp (2 * myRate * myDt[0]) - 1) / (2 * myRate)) * G[step++];

            //	All steps
            for (size_t i = 1; i<myTimes.size(); ++i)
            {
                spots[i] = spots[i - 1] * exp(myRate * myDt[0]) + myVol * sqrt((exp(2 * myRate * myDt[0]) - 1) / (2 * myRate)) * G[step++];
            }
        }
    }
};

template <class T>
class MonteCarloSimulator
{
    RandomGen&          myRandomGen;
    Model<T>&           myModel;
    
public:

    MonteCarloSimulator( Model<T>& model, RandomGen& ranGen) : myRandomGen( ranGen), myModel( model) {}

    void init( const vector<Date>& simDates)
    {
        myModel.initSimDates( simDates);
        myRandomGen.init( myModel.dim());
    }

    void simulateOnePath( vector<T>& spots, vector<T>& numeraires)
    {
        myRandomGen.genNextNormVec();
        myModel.applySDE(myRandomGen.getNorm(), spots, numeraires);
    }
};

//  Model interface for communication with script
template <class T>
struct ScriptModelApi
{
    virtual void initForScripting(const vector<Date>& eventDates) = 0;

    virtual void nextScenario(Scenario<T>& s) = 0;
};

template <class T>
class ScriptSimulator : public MonteCarloSimulator<T>, public ScriptModelApi<T>
{

    vector<T>      myTempSpots;
    vector<T>      myTempNumeraires;

public:

    ScriptSimulator( Model<T>& model, RandomGen& ranGen) : MonteCarloSimulator( model, ranGen) {}

	void initForScripting( const vector<Date>& eventDates) override
	{
        MonteCarloSimulator::init( eventDates);
        myTempSpots.resize( eventDates.size());
        myTempNumeraires.resize(eventDates.size());
    }

	void nextScenario( Scenario<T>& s) override
	{
        MonteCarloSimulator::simulateOnePath( myTempSpots, myTempNumeraires);
		
        //  Note the inefficiency
        for(size_t i=0; i<s.size(); ++i)
		{
			s[i].spot = myTempSpots[i];
			s[i].numeraire = myTempNumeraires[i];
		}
	}
};

inline void simpleBsScriptVal(
	const Date&				today,
	const double			spot,
	const double			vol,
	const double			rate,
    const bool              normal,     //  true = normal, false = lognormal
	const map<Date,string>	events,
	const unsigned			numSim,
	const unsigned			seed,		//	0 = default
	//	Fuzzy
	const bool				fuzzy,		//	Use sharp (false) or fuzzy (true) eval
	const double			defEps,		//	Default epsilon, may be redefined by node
	const bool				skipDoms,	//	Skip domains (unless fuzzy)
	//	Results
	vector<string>&			varNames,
	vector<double>&			varVals)
{
	if( events.begin()->first < today)
		throw runtime_error("Events in the past are disallowed");

	//	Initialize product
	Product prd;
	prd.parseEvents( events.begin(), events.end());
	unsigned maxNestedIfs = prd.preProcess( fuzzy, skipDoms);

	//	Build evaluator and scenarios
	unique_ptr<Scenario<double>> scen = prd.buildScenario<double>();

	unique_ptr<Evaluator<double>> eval;
	if( fuzzy) eval = prd.buildFuzzyEvaluator<double>( maxNestedIfs, defEps);
	else eval = prd.buildEvaluator<double>();

    //  Initialize model and random generator
    BasicRanGen random( seed);
    unique_ptr<Model<double>> model;
    if (normal) model.reset( new SimpleBachelier<double>(today, spot, vol, rate));
    else model.reset(new SimpleBlackScholes<double>(today, spot, vol, rate));

	//	Initialize simulator
    ScriptSimulator<double> simulator( *model, random);
    simulator.initForScripting( prd.eventDates());

	//	Initialize results
	varNames = prd.varNames();
	varVals.resize( varNames.size(), 0.0);

	//	Loop over simulations
	for( size_t i=0; i<numSim; ++i)
	{
		//	Generate next scenario into scen
		simulator.nextScenario( *scen);
		//	Evaluate product 
		prd.evaluate( *scen, *eval);
		//	Update results
		for(size_t v=0; v<varVals.size(); ++v)
		{
			varVals[v] += eval->varVals()[v] / numSim;
		}
	}
}