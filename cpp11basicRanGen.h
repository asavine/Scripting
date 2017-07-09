#pragma once

/*	Basic self contained normal random generator using C++11's native facilities
	Init with the required dimension
	Call genNextNormVec() repeatedly to generate the next point of dimension dim in the sequence
	Then call getNorm( i) with 0<=i<dim to access the i-th coordinate
	Each coordinate is an independent Normal( 0, 1) */

#include <random>
#include <vector>
#include <memory>
using namespace std;

struct randomgen_error : public runtime_error
{
	randomgen_error(const char msg[]) : runtime_error(msg) {}
};

//  Base
class RandomGen
{
public:

    //  Initialise for a given dimension
    virtual void init( const size_t dim) = 0;

    //  Generate the next random point
    virtual void genNextNormVec() = 0;

    //  Access Gaussian vector byRef
    virtual const vector<double>& getNorm() const = 0;

    //  Clone
    virtual unique_ptr<RandomGen> clone() const = 0;

	//	Skip ahead (for parallel Monte-Carlo)
	virtual void skipAhead(const long skip)
	{
		throw randomgen_error("Concrete random generator cannot be used for parallel simulations");
	}
};

//  Basic C++11
class BasicRanGen : public RandomGen
{
	default_random_engine	myEngine;
	normal_distribution<>	myDist;
	size_t				    myDim;

	vector<double>			myNormVec;

public:

	BasicRanGen( const unsigned seed = 0)
	{
		myEngine = seed > 0? default_random_engine( seed): default_random_engine();
		myDist = normal_distribution<>();
	}

    void init(const size_t dim) override
    {
        myDim = dim;
        myNormVec.resize(dim);
    }

	void genNextNormVec() override
	{
		for( size_t i=0; i<myDim; ++i)
		{
			myNormVec[i] = myDist( myEngine);
		}
	}

    const vector<double>& getNorm() const override
	{
		return myNormVec;
	}

    //  Clone
    unique_ptr<RandomGen> clone() const override
    {
        return unique_ptr<RandomGen>(new BasicRanGen(*this));
    }
};