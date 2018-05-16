
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