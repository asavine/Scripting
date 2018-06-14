
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

#include <iterator>
#include <vector>

using namespace std;

template <class T, size_t DefaultSize = 64>
class quickStack
{

private:

	T*				myData;
	size_t			mySize;
	size_t			mySp;

public:

	//	Constructor, destructor

	quickStack(const size_t chunk_size = DefaultSize)
	{
		mySize = chunk_size;
		if (mySize) myData = new T[mySize];
		else myData = nullptr;
		mySp = 0;
	}

	~quickStack()
	{
		if (myData) delete[] myData;
	}

	//	Copier, mover

	quickStack(const quickStack& rhs)
	{
		mySize = rhs.mySize;
		mySp = rhs.mySp;
		if (mySize) myData = new T[mySize];
		else myData = nullptr;
		if (mySp>0) copy(rhs.myData, rhs.myData + mySp, myData);
	}

	quickStack& operator=(const quickStack& rhs)
	{
		if (this == &rhs) return *this;
		if (mySize < rhs.mySize)
		{
			if (myData) delete[] myData;
			if (rhs.mySize) myData = new T[rhs.mySize];
			else myData = nullptr;
		}
		mySize = rhs.mySize;
		mySp = rhs.mySp;
		if (mySp>0) copy(rhs.myData, rhs.myData + mySp, myData);

		return *this;
	}

	quickStack(quickStack&& rhs)
	{
		mySize = rhs.mySize;
		mySp = rhs.mySp;
		myData = rhs.myData;
		rhs.myData = nullptr;
		rhs.mySize = rhs.mySp = 0;
	}

	quickStack& operator=(quickStack&& rhs)
	{
		if (this == &rhs) return *this;
		if (myData) delete[] myData;
		mySize = rhs.mySize;
		mySp = rhs.mySp;
		myData = rhs.myData;
		rhs.myData = nullptr;
		rhs.mySize = rhs.mySp = 0;

		return *this;
	}

	typedef reverse_iterator<T*> iterator;
	typedef reverse_iterator<const T*> const_iterator;

	inline iterator begin()
	{
		return reverse_iterator<T*>(myData + mySp);
	}

	inline const_iterator begin() const
	{
		return reverse_iterator<const T*>(myData + mySp);
	}

	inline iterator end()
	{
		return reverse_iterator<T*>(myData);
	}

	inline const_iterator end() const
	{
		return reverse_iterator<const T*>(myData);
	}

	template <typename T2>
	void push(T2&& value)
	{
		myData[mySp] = forward<T2>(value);
		++mySp;
		if (mySp >= mySize)
		{
			T* newData = new T[mySize << 1];

#ifdef _MSC_VER
			move(myData, myData + mySize, stdext::make_unchecked_array_iterator(newData));
#else
			move(myData, myData + mySize, newData);
#endif
			
			delete[] myData;
			myData = newData;
			mySize <<= 1;
		}
	}

	inline T& top()
	{
		return myData[mySp - 1];
	}

	inline const T& top() const
	{
		return myData[mySp - 1];
	}

	//	Random access
	inline T& operator[](const size_t i)
	{
		return myData[mySp - 1 - i];
	}

	inline const T& operator[](const size_t i) const
	{
		return myData[mySp - 1 - i];
	}

	inline T topAndPop()
	{
		--mySp;
		return move(myData[mySp]);
	}

	void pop()
	{
		--mySp;
	}

	void pop(const size_t n)
	{
		mySp -= n;
	}

	void reset()
	{
		mySp = 0;
	}

	void clear()
	{
		if (myData) delete[] myData;
		myData = nullptr;
		mySize = mySp = 0;
	}

	size_t size() const
	{
		return mySp;
	}

	size_t capacity() const
	{
		return mySize;
	}

	bool empty() const
	{
		return mySp == 0;
	}
};

template <class T, size_t Size = 64>
class staticStack
{

private:

    T               myData[Size];
    int			    mySp = -1;

public:

    template <typename T2>
    inline void push(T2&& value)
    {
        myData[++mySp] = forward<T2>(value);
    }

    inline T& top()
    {
        return myData[mySp];
    }

    inline const T& top() const
    {
        return myData[mySp];
    }

    //	Random access
    inline T& operator[](const int i)
    {
        return myData[mySp - i];
    }

    inline const T& operator[](const int i) const
    {
        return myData[mySp - i];
    }

    inline T topAndPop()
    {
        return move(myData[--mySp]);
    }

    void pop()
    {
        --mySp;
    }

    void pop(const int n)
    {
        mySp -= n;
    }

    void reset()
    {
        mySp = -1;
    }

    size_t size() const
    {
        return static_cast<size_t>((mySp+1));
    }

    bool empty() const
    {
        return mySp < 0;
    }
};