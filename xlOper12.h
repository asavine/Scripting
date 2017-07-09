#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "xlcall.h"
#include <string>

#include <limits>
using namespace std;

extern const char * xll_last_call;

/// XLOper12 is a C++ wrapper for XLOPER12 structure from Excel SDK API.
/// XLOPER is Excel's fundamental data type: can hold data of any type. 
/// Use "R" or "P" as the argument type in the REGISTER function.

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

        // allocates memory
        explicit XLOper12(const char* str);
        explicit XLOper12(const std::string& str);
        explicit XLOper12(unsigned int rows, unsigned int columns);
        
        ~XLOper12();

        // ----------------------------------------------------------------------------
        
        // Conversion operators
        
        operator int() const { return static_cast<int>(double(*this)+numeric_limits<double>::epsilon() ); }
        operator long() const { return static_cast<long>(double(*this)+numeric_limits<double>::epsilon() ); }

        operator double() const;
        operator bool() const;
        operator std::string() const;
        
        // Get value in array operator
        XLOper12& operator()(unsigned int row, unsigned int column);
        const XLOper12& operator()(unsigned int row, unsigned int column) const;
        const XLOper12& operator()(unsigned int idx) const;

        int Type() const;
        unsigned int Rows() const;
        unsigned int Cols() const;
        unsigned int Size()const;

        // ----------------------------------------------------------------------------
        
        // The XLL must allow Excel to free that memory using the xlFree function call.
        // The XLL can also return back an Excel allocated  XLOper to Excel, in that case we do not call xlFree but we set flag xlbitXLFree to true.
        // eg. flag xlbitXLFree should be set by the XLL on Excel XLOper to mark XLOper which are returned back to Excel who must free the memory.
        
        // To simplify this we always set this flag to true calling .XLFree on Excel returned XLOper , 
        // to remind us this is XL allocated memory and we must call xlFree within the destructor.
        
        // Memory allocated by XLL :
        // flag xlbitDLLFree is set by the XLL to mark XLOpers with XLL allocated memory.
        // Excel calls XLL's xlAutoFree callback to allows the XLL to free up allocated memory. 

        void ExplicitFree();
};

XLOper12* return_xloper_raw_ptr(XLOper12&); 

XLOper12 ErrorXLOper12(int e);
extern const XLOper12 ErrorNotAvailable12;
