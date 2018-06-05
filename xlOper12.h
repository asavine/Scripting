#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "xlcall.h"
#include <string>

#include <limits>
using namespace std;

extern const char * xll_last_call;

struct XLOper12 : public XLOPER12 {

        XLOper12() {
                xltype = xltypeNil;
        }

        explicit XLOper12(double num)
        {
                xltype = xltypeNum;
                val.num = num;
        }

        explicit XLOper12(bool Bool) {
                xltype = xltypeBool;
                val.xbool = (unsigned short)(Bool ? TRUE : FALSE);
        }

        explicit XLOper12(const char* str);
        explicit XLOper12(const std::string& str);
        explicit XLOper12(unsigned int rows, unsigned int columns);
        
        ~XLOper12();

        // ----------------------------------------------------------------------------
                
        operator int() const { return static_cast<int>(double(*this)+numeric_limits<double>::epsilon() ); }
        operator long() const { return static_cast<long>(double(*this)+numeric_limits<double>::epsilon() ); }

        operator double() const;
        operator bool() const;
        operator std::string() const;
        
        XLOper12& operator()(unsigned int row, unsigned int column);
        const XLOper12& operator()(unsigned int row, unsigned int column) const;
        const XLOper12& operator()(unsigned int idx) const;

        int Type() const;
        unsigned int Rows() const;
        unsigned int Cols() const;
        unsigned int Size()const;

        // ----------------------------------------------------------------------------

        void ExplicitFree();
};

XLOper12* return_xloper_raw_ptr(XLOper12&); 

XLOper12 ErrorXLOper12(int e);
extern const XLOper12 ErrorNotAvailable12;
