#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "xlcall.h"
#include <string>

#include <limits>
using namespace std;

extern const char * xll_last_call;

struct myXlOper : public XLOPER12 {

        myXlOper() {
                xltype = xltypeNil;
        }

        explicit myXlOper(double num)
        {
                xltype = xltypeNum;
                val.num = num;
        }

        explicit myXlOper(bool Bool) {
                xltype = xltypeBool;
                val.xbool = (unsigned short)(Bool ? TRUE : FALSE);
        }

        explicit myXlOper(const char* str);
        explicit myXlOper(const std::string& str);
        explicit myXlOper(size_t rows, size_t columns);
        
        ~myXlOper();

        // ----------------------------------------------------------------------------
                
        operator int() const { return static_cast<int>(double(*this)+numeric_limits<double>::epsilon() ); }
        operator long() const { return static_cast<long>(double(*this)+numeric_limits<double>::epsilon() ); }

        operator double() const;
        operator bool() const;
        operator std::string() const;
        
        myXlOper& operator()(unsigned int row, unsigned int column);
        const myXlOper& operator()(unsigned int row, unsigned int column) const;
        const myXlOper& operator()(unsigned int idx) const;

        int Type() const;
        unsigned int Rows() const;
        unsigned int Cols() const;
        unsigned int Size()const;

        // ----------------------------------------------------------------------------

        void ExplicitFree();
};

myXlOper* return_xloper_raw_ptr(myXlOper&); 

myXlOper ErrormyXlOper(int e);
extern const myXlOper ErrorNotAvailable12;
