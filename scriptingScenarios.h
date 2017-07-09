#pragma once

#include <vector>
using namespace std;

template <class T>
struct SimulData
{
	T           spot;
	T           numeraire;
};

template <class T>
using Scenario = vector<SimulData<T>>;