#pragma warning(disable:4786)
#include "XLOper12.h"

#include <iostream>
#include <stdlib.h>
#include <string>
#include <assert.h>
#pragma warning ( disable : 4996 )
#pragma warning(disable : 4297)

using namespace std;

XLOper12 ErrorXLOper12(int e) { XLOper12 x; x.xltype=xltypeErr; x.val.err=e; return x; }
const XLOper12 ErrorNotAvailable12 = ErrorXLOper12(xlerrNA);

const char * xll_last_call = 0;
static int xll12_alloc_counter_return = 0;
static int xll12_alloc_counter_string = 0;
static int xll12_alloc_counter_array = 0;

namespace {
        void init_string(XLOper12& op, const char* cstr)
        {
                op.xltype = xltypeStr | xlbitDLLFree;
                op.val.str = 0;

                unsigned int len = strlen(cstr);
                op.val.str = new XCHAR[len+2];          
                op.val.str[0] = (XCHAR) min(len,32765u);
                op.val.str[len+1] = '\0';

                MultiByteToWideChar(CP_ACP, 0, cstr, len, op.val.str+1, len);

                xll12_alloc_counter_string++;
        }
}

XLOper12::XLOper12(const char* str) {
        init_string(*this, str);
}
XLOper12::XLOper12(const std::string& str) {
        init_string(*this, str.c_str());
}

XLOper12::XLOper12(unsigned int rows, unsigned int columns) {
        xltype = xltypeMulti | xlbitDLLFree; 
        val.array.rows = (unsigned short)rows;
        val.array.columns = (unsigned short)columns;
        val.array.lparray = new XLOper12[rows*columns];

        xll12_alloc_counter_array++;
}

// ----------------------------------------------------------------------------

XLOper12::operator double() const
{
        if (Type() != xltypeNum)
                return 0.0;

        return val.num;
}

XLOper12::operator bool() const
{
        if (Type()==xltypeMissing  || Type()==xltypeNil)
                return false;

        return val.xbool?true:false;
}

XLOper12::operator std::string() const
{
        if (Type() != xltypeStr)
                return "";

        int len = val.str[0];
        char *str = new char[len+2];
        
        WideCharToMultiByte(CP_ACP, 0, val.str+1, len, str, len, NULL, NULL);
        str[len] = '\0';
        std::string res(str);

        delete[] str;
        return res;
}

XLOper12& XLOper12::operator()(unsigned int row, unsigned int column)
{
        if(Type()!=xltypeMulti || (RW)row>=val.array.rows || (COL)column>=val.array.columns)
                throw WORD(xlretAbort);
        return (XLOper12&)val.array.lparray[row*val.array.columns+column];
}

const XLOper12& XLOper12::operator()(unsigned int row, unsigned int column) const
{
        if(Type()!=xltypeMulti || (RW)row>=val.array.rows || (COL)column>=val.array.columns)
                throw WORD(xlretAbort);
        return (XLOper12&)val.array.lparray[row*val.array.columns+column];
}
const XLOper12& XLOper12::operator()(unsigned int rank) const
{
        // case single value
        if(Type()!=xltypeMulti) {
                if (rank==0) 
                        return *this;
                else 
                        throw WORD(xlretAbort);
        }

        // case vector
        unsigned int max_rank = val.array.rows*val.array.columns;
        if(rank>=max_rank)
                throw WORD(xlretAbort);
        return (XLOper12&)val.array.lparray[rank];
}

int XLOper12::Type() const
{
        return xltype & ~(xlbitXLFree|xlbitDLLFree);
}

unsigned int XLOper12::Rows() const
{
        return Type()==xltypeMulti ? (unsigned int)val.array.rows : (unsigned int)0;
}

unsigned int XLOper12::Cols() const
{
        return Type()==xltypeMulti ? (unsigned int)val.array.columns : (unsigned int)0;
}

unsigned int XLOper12::Size()const
{
        return max( 1u, Rows()*Cols() );
}

// ----------------------------------------------------------------------------

XLOper12::~XLOper12() {
        bool bFreeByExcel = ((xltype&xlbitXLFree)!=0);
        bool bFreeByDll   = ((xltype&xlbitDLLFree)!=0) ; 
        assert( !(bFreeByExcel&&bFreeByDll) ); // exclusive ownership
        assert( bFreeByExcel || bFreeByDll || true );  // ownership expected

        if (bFreeByExcel) {
                bool bNeedsFreeing = xltype==xltypeStr||xltype==xltypeMulti||xltype==xltypeRef;
                if (bNeedsFreeing) {
                        WORD xlret = Excel12(xlFree, 0, 1, this);
                        if(xlret!=xlretSuccess) throw xlret;
                }
        }
}

void XLOper12::ExplicitFree()
{
        bool bFreeByExcel = ((xltype&xlbitXLFree)!=0);
        bool bFreeByDll   = ((xltype&xlbitDLLFree)!=0) ; 
        assert( !(bFreeByExcel&&bFreeByDll) ); 
        assert( bFreeByExcel || bFreeByDll || true );  
        
        if (bFreeByDll) {
                switch(Type()) {

                case xltypeErr:
                        return;
                        break;

                case xltypeStr:
                        delete[] val.str;
                        val.str = NULL;
                        xll12_alloc_counter_string--;
                        break;
                
                case xltypeMulti:
                        int n = val.array.rows*val.array.columns;
                        for (int i=0; i<n; i++) {
                                if (val.array.lparray[i].xltype & xltypeStr) {
                                        delete[] val.array.lparray[i].val.str;
                                        val.array.lparray[i].val.str = NULL;
                                        xll12_alloc_counter_string--;
                                }
                        }       
                        delete[] (XLOper12*)val.array.lparray;
                        val.array.rows = val.array.columns = 0;
                        val.array.lparray = NULL;
                        xll12_alloc_counter_array--;
                        break;
                }

                xll12_alloc_counter_return--;
                delete this;
        }
}

XLOper12* return_xloper_raw_ptr(XLOper12& val)
{
        XLOper12 *ret = new XLOper12();
        (*ret) = val;
        (*ret).xltype = (*ret).xltype | xlbitDLLFree;
        
        xll12_alloc_counter_return++;
        return ret;
}
