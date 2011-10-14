#ifndef FORMAT_H_
#define FORMAT_H_

#include <ostream>
#include <string>

class format
{
public:
	format() { store = ""; }
	// warning, format(const std::string, ...) can only format up to 500 bytes of parameters
	format(const std::string, ...);
	void format_string( const std::string, ... );
	const std::string	&str() const	{ return store; }
private:
	std::string	store;
};

class fmtspec
{
public:
	fmtspec(const std::string &);
private:
	enum type_
	{
		INVALID,
		INT,
		UNSIGNED,
		LONGLONG,
		ULONGLONG,
		DOUBLE,
		LDOUBLE,
		CHAR,
		CSTRING,
		STRING
	};

	bool	zeropad;
	bool	left;
	int	width;
	int	precision;
	int	base;
	int	length;
	type_	type;
	bool	sci;
	bool	fixed;
	bool	ucase;
	bool	alt;
	bool	sign;

	friend class format;
	friend std::ostream &operator<<(std::ostream &, const fmtspec &);
};

inline std::ostream &operator<<(std::ostream &o, const format &f)	{ o<<f.str(); return o; }

#endif
