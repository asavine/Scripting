
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

#include "scriptingParser.h"

#include <regex>
#include <algorithm>

vector<string> tokenize( const string& str)
{	
	//	Regex matching tokens of interest
	static const regex r( "[\\w.]+|[/-]|,|;|:|[\\(\\)\\+\\*\\^]|!=|>=|<=|[<>=]");

	//	Result, with max possible size reserved
	vector<string> v;
	v.reserve( str.size());

	//	Loop over matches
	for( sregex_iterator it( str.begin(), str.end(), r), end; it != end; ++it)
	{
		//	Copy match into results
		v.push_back( (*it)[0]);
		//	Uppercase
		transform( v.back().begin(), v.back().end(), v.back().begin(), toupper);
	}

	//	C++11 move semantics means no copy
	return v;
}

//	Event = vector<Statement>
Event parse( const string& eventString)
{
    Event e;

	auto tokens = tokenize( eventString);

	auto it = tokens.begin();
	while( it != tokens.end())
	{
		e.push_back( Parser<decltype(it)>::parseStatement( it, tokens.end()));
	}

	//	C++11 --> vectors are moved, not copied
	return e;
}

//	Single expression
Expression parseExpression(const string& exprString)
{
    auto tokens = tokenize(exprString);
    auto it = tokens.begin();
    return Parser<decltype(tokens.begin())>::parseStatement(it, tokens.end());
}
