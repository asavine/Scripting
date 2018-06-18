#pragma warning(disable:4786)
#include "XlApi.h"

#include <iostream>
#include <stdlib.h>
#include <string>
#include <assert.h>
#pragma warning ( disable : 4996 )
#pragma warning(disable : 4297)

using namespace std;

myXlOper ErrormyXlOper(int e) { myXlOper x; x.xltype=xltypeErr; x.val.err=e; return x; }
const myXlOper ErrorNotAvailable12 = ErrormyXlOper(xlerrNA);

const char * xll_last_call = 0;
static int xll12_alloc_counter_return = 0;
static int xll12_alloc_counter_string = 0;
static int xll12_alloc_counter_array = 0;

namespace {
        void init_string(myXlOper& op, const char* cstr)
        {
                op.xltype = xltypeStr | xlbitDLLFree;
                op.val.str = 0;

                auto len = strlen(cstr);
                op.val.str = new XCHAR[len+2];          
                op.val.str[0] = (XCHAR) min(len,32765u);
                op.val.str[len+1] = '\0';

                MultiByteToWideChar(CP_ACP, 0, cstr, int(len), op.val.str+1, int(len));

                xll12_alloc_counter_string++;
        }
}

myXlOper::myXlOper(const char* str) {
        init_string(*this, str);
}
myXlOper::myXlOper(const std::string& str) {
        init_string(*this, str.c_str());
}

myXlOper::myXlOper(size_t rows, size_t columns) {
        xltype = xltypeMulti | xlbitDLLFree; 
        val.array.rows = (unsigned short)rows;
        val.array.columns = (unsigned short)columns;
        val.array.lparray = new myXlOper[rows*columns];

        xll12_alloc_counter_array++;
}

// ----------------------------------------------------------------------------

myXlOper::operator double() const
{
        if (Type() != xltypeNum)
                return 0.0;

        return val.num;
}

myXlOper::operator bool() const
{
        if (Type()==xltypeMissing  || Type()==xltypeNil)
                return false;

        return val.xbool?true:false;
}

myXlOper::operator std::string() const
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

myXlOper& myXlOper::operator()(unsigned int row, unsigned int column)
{
        if(Type()!=xltypeMulti || (RW)row>=val.array.rows || (COL)column>=val.array.columns)
                throw WORD(xlretAbort);
        return (myXlOper&)val.array.lparray[row*val.array.columns+column];
}

const myXlOper& myXlOper::operator()(unsigned int row, unsigned int column) const
{
        if(Type()!=xltypeMulti || (RW)row>=val.array.rows || (COL)column>=val.array.columns)
                throw WORD(xlretAbort);
        return (myXlOper&)val.array.lparray[row*val.array.columns+column];
}
const myXlOper& myXlOper::operator()(unsigned int rank) const
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
        return (myXlOper&)val.array.lparray[rank];
}

int myXlOper::Type() const
{
        return xltype & ~(xlbitXLFree|xlbitDLLFree);
}

unsigned int myXlOper::Rows() const
{
        return Type()==xltypeMulti ? (unsigned int)val.array.rows : (unsigned int)0;
}

unsigned int myXlOper::Cols() const
{
        return Type()==xltypeMulti ? (unsigned int)val.array.columns : (unsigned int)0;
}

unsigned int myXlOper::Size()const
{
        return max( 1u, Rows()*Cols() );
}

// ----------------------------------------------------------------------------

myXlOper::~myXlOper() {
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

void myXlOper::ExplicitFree()
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
                        delete[] (myXlOper*)val.array.lparray;
                        val.array.rows = val.array.columns = 0;
                        val.array.lparray = NULL;
                        xll12_alloc_counter_array--;
                        break;
                }

                xll12_alloc_counter_return--;
                delete this;
        }
}

myXlOper* return_xloper_raw_ptr(myXlOper& val)
{
        myXlOper *ret = new myXlOper();
        (*ret) = val;
        (*ret).xltype = (*ret).xltype | xlbitDLLFree;
        
        xll12_alloc_counter_return++;
        return ret;
}
