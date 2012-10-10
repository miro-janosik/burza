#include <stdarg.h>
#include <iomanip>
#include <ostream>
#include <sstream>
#include "format.h"

#ifdef _UNIX
#include <stdlib.h>
#include <string.h>
#endif

using namespace std;

#define USED_LONGLONG long

// 2008-sept miroj: Fixed bug in fmtspec::fmtspec(const string &str):
// --i could be called on first character, and it caused failure on VS2008
// replaced with ++i on condition


format::format(const string fmt, ...)
{
	// only passes the arguments to the format_string()
	struct {
		char buffer[ 500 ];
	} data;

	va_list	args;

	va_start(args,fmt);
	memcpy( &data, args, 500 );

	format_string( fmt, data );
}

void format::format_string(const string fmt, ...)
{
	va_list	args;

	store = "";

	va_start(args,fmt);
	bool		field=false;
	string	spec_str;
	for(string::const_iterator i=fmt.begin(); i!=fmt.end(); ++i)
	{
		if(field)
		{
			if(*i=='%' && !spec_str.size())
			{
				store+='%';
				field=0;
			}
			else if(isalpha(*i) && *i!='l' && *i!='h')
			{
				spec_str+=*i;
				fmtspec	spec(spec_str);
				ostringstream	buf;
				buf<<spec;
				switch(spec.type)
				{
				case fmtspec::INT: {
					int	arg=va_arg(args,int);
					buf<<arg;
					break; }
				case fmtspec::UNSIGNED: {
					int	arg=va_arg(args,unsigned);
					buf<<arg;
					break; }
				case fmtspec::LONGLONG: {
					USED_LONGLONG	arg=va_arg(args,USED_LONGLONG);
					buf<<arg;
					break; }
				case fmtspec::ULONGLONG: {
					unsigned USED_LONGLONG	arg=va_arg(args,unsigned USED_LONGLONG);
					buf<<arg;
					break; }
				case fmtspec::DOUBLE: {
					double	arg=va_arg(args,double);
					buf<<arg;
					break; }
				case fmtspec::LDOUBLE: {
					long double	arg=va_arg(args,long double);
					buf<<arg;
					break; }
				case fmtspec::CHAR: {
					unsigned char	arg=va_arg(args,unsigned);
					buf<<arg;
					break; }
				case fmtspec::STRING: {
					// on unix - can't pass std::string through argvs
					// string	arg=va_arg(args,string);
					// buf<<arg;
					// assert( false );
					break; }
				case fmtspec::CSTRING: {
					char	*arg=va_arg(args,char *);
					buf<<arg;
					break; }
				default:
					buf<<spec_str;
				}
				store+=buf.str();
				field=false;
			}
			else
				spec_str+=*i;
		}
		else if(*i=='%')
		{
			field=true;
			spec_str="";
		}
		else
			store+=*i;
	}
	if(field) store+=spec_str;
	va_end(args);
}

fmtspec::fmtspec(const string &str):
	zeropad(false),
	left(false),
	width(0),
	precision(6),
	base(10),
	length(0),
	type(INVALID),
	sci(false),
	fixed(false),
	ucase(false),
	alt(false),
	sign(false)
{
	unsigned	phase=0;
	string	buf;
	bool	advance;
	for(string::const_iterator i=str.begin(); i!=str.end(); )
	{
		advance = true;

		if(phase==0)
		{
			if(*i=='-')
				left=true;
			else if(*i=='0')
				zeropad=true;
			else if(*i=='#')
				alt=true;
			else if(*i=='+')
				sign=true;
			else
			{
				++phase;
				advance = false;
			}
		}
		else if(phase==1)
		{
			if(*i>='0' && *i<='9')
				buf+=*i;
			else if(*i=='.')
				++phase;
			else
			{
				++phase;
				advance = false;
			}
			if(phase>1 && buf.size())
			{
				width=strtol(buf.c_str(),NULL,10);
				buf="";
			}
		}
		else if(phase==2)
		{
			if(*i>='0' && *i<='9')
				buf+=*i;
			else
			{
				if(buf.size()) precision=strtol(buf.c_str(),NULL,10);
				++phase;
				advance = false;
			}
		}
		else if(phase==3)
		{
			if(*i=='h')
				--length;
			else if(*i=='l')
				++length;
			else if(*i=='L')
				length+=2;
			else if(*i=='d' || *i=='i')
			{
				type=INT;
				++phase;
			}
			else if(*i=='o')
			{
				type=UNSIGNED;
				base=8;
				++phase;
			}
			else if(*i=='x' || *i=='X')
			{
				type=UNSIGNED;
				base=16;
				if(*i=='X') ucase=true;
				++phase;
			}
			else if(*i=='e' || *i=='E')
			{
				type=DOUBLE;
				sci=true;
				if(*i=='F') ucase=true;
				++phase;
			}
			else if(*i=='f' || *i=='F')
			{
				type=DOUBLE;
				fixed=true;
				if(*i=='F') ucase=true;
				++phase;
			}
			else if(*i=='g' || *i=='G')
			{
				type=DOUBLE;
				if(*i=='F') ucase=true;
				++phase;
			}
			else if(*i=='c')
			{
				type=CHAR;
				++phase;
			}
			else if(*i=='s')
			{
				type=CSTRING;
				++phase;
			}
			else if(*i=='S')
			{
				type=STRING;
				++phase;
			}
			else if(*i=='p')
			{
				type=UNSIGNED;
				base=16;
				alt=true;
				++phase;
			}
		}
		else
			return;

		if ( advance )
			++i;
	}
	if(length<-2 || length>2) return;
	if(length==2)
	{
		if(type==DOUBLE) type=LDOUBLE;
		else if(type==INT) type=LONGLONG;
		else if(type==UNSIGNED) type=ULONGLONG;
	}
	if(left) zeropad=false;
}

ostream &operator<<(ostream &out, const fmtspec &spec)
{
	if(spec.sci) out<<scientific;
	else if(spec.fixed) out<<fixed;
	out<<setw(spec.width)<<setprecision(spec.precision);
	if(spec.base==16) out<<hex;
	else if(spec.base==8) out<<oct;
	else out<<dec;
	if(spec.left) out<<left;
	if(spec.zeropad) out<<setfill('0');
	if(spec.ucase) out<<uppercase;
	if(spec.alt) out<<showpoint<<showbase;
	if(spec.sign) out<<showpos;
	return out;
}
