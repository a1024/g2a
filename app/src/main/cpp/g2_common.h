//common.h - Include for common definitions for Grapher 2A.
//Copyright (C) 2019-2020  Ayman Wagih Mohsen
//
//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program.  If not, see <https://www.gnu.org/licenses/>.

//
// Created by MSI on 10/1/2020.
//

#ifndef GRAPHER_2_G2_COMMON_H
#define GRAPHER_2_G2_COMMON_H
#include		<jni.h>
#include		<android/log.h>

#include 		<math.h>
#include		<string>
#include		<complex>
#include		<vector>
//#define		LOG_TAG		"libgl2jni"
#define			LOGI(...)	__android_log_print(ANDROID_LOG_INFO, __FILE__, __VA_ARGS__)
#define			LOGE(...)	__android_log_print(ANDROID_LOG_ERROR, __FILE__, __VA_ARGS__)
//extern int 	hard_reset;
static const int e_msg_size=2048;
extern char		first_error_msg[e_msg_size], latest_error_msg[e_msg_size];
void 			log_error(const char *file, int line, const char *format, ...);
#define 		LOGERROR(...)				log_error(__FILE__, __LINE__, __VA_ARGS__)
#define 		LOGERROR_LINE(LINE, ...)	log_error(__FILE__, LINE, __VA_ARGS__)
inline double	now_seconds()
{//https://stackoverflow.com/questions/3832097/how-to-get-the-current-time-in-native-android-code/14311780
	static timespec ts={0, 0};
	clock_gettime(CLOCK_REALTIME, &ts);//Performance & modes::Solve used CLOCK_PROCESS_CPUTIME_ID, showed less elapsed time
	return ts.tv_sec+1e-9*ts.tv_nsec;
}
typedef std::pair<std::string, double> ProfInfo;
//ProfInfo	longest;
extern std::vector<ProfInfo> prof;
extern double prof_t1;
void prof_start();
void prof_add(const char *label, int divisor=1);
void prof_print(int y=0);
extern const double _HUGE;
inline int		floor_log2(unsigned n)
{
	int logn=0;
	int sh=(n>=1<<16)<<4;	logn+=sh, n>>=sh;
		sh=(n>=1<< 8)<<3;	logn+=sh, n>>=sh;
		sh=(n>=1<< 4)<<2;	logn+=sh, n>>=sh;
		sh=(n>=1<< 2)<<1;	logn+=sh, n>>=sh;
		sh= n>=1<< 1;		logn+=sh;
	return logn;
}
void			utf16ToAscii(unsigned short const *utf16, int len, std::string &ascii);
namespace		G2
{
	enum Map
	{
		M_IGNORED,

		M_N,

		M_LPR, M_RPR,																//(		)
		M_COMMA,																	//,
		M_QUESTION_MARK, M_COLON,													//?		:

			M_PROCEDURAL_START,
		M_IF, M_ELSE, M_FOR, M_DO, M_WHILE,											//if else for do while
		M_CONTINUE, M_BREAK, M_RETURN,												//continue break return
		M_LBRACE, M_RBRACE,															//{		}
		M_SEMICOLON,																//;

			M_PROCEDURAL_ASSIGN_START,
		M_ASSIGN, M_ASSIGN_MULTIPLY, M_ASSIGN_DIVIDE, M_ASSIGN_MOD,					//= *= /= %=
		M_ASSIGN_PLUS, M_ASSIGN_MINUS, M_ASSIGN_LEFT, M_ASSIGN_RIGHT,				//+= -= <<= >>=
		M_ASSIGN_AND, M_ASSIGN_XOR, M_ASSIGN_OR,									//&= #= |=
			M_PROCEDURAL_ASSIGN_END,

			M_PROCEDURAL_END,

		M_INCREMENT, M_DECREMENT,													//++ --
		M_FACTORIAL_LOGIC_NOT,														//!
		M_MODULO_PERCENT,															//%
		M_BITWISE_NOT,																//~
		M_PENTATE,																	//^^^
		M_TETRATE,																	//^^
		M_POWER, M_POWER_REAL,														//^		**
		M_MULTIPLY, M_DIVIDE, M_LOGIC_DIVIDES,										//*		/		@
		M_PLUS, M_MINUS,															//+		-
		M_BITWISE_SHIFT_LEFT, M_BITWISE_SHIFT_RIGHT,								//<<	>>
		M_LOGIC_LESS, M_LOGIC_LESS_EQUAL, M_LOGIC_GREATER, M_LOGIC_GREATER_EQUAL,	//<		<=		>		>=
		M_LOGIC_EQUAL, M_LOGIC_NOT_EQUAL,											//==	!=
		M_BITWISE_AND, M_BITWISE_NAND,												//&		~& |~
		M_BITWISE_XOR, M_BITWISE_XNOR,												//#		~# #~
		M_VERTICAL_BAR, M_BITWISE_NOR,												//|		~| &~
		M_LOGIC_AND,																//&&
		M_LOGIC_XOR,																//##
		M_LOGIC_OR,																	//||
		M_CONDITION_ZERO,														//??

		M_S_EQUAL_ASSIGN, M_S_NOT_EQUAL,											//= _!=				not supported
		M_S_LESS, M_S_LESS_EQUAL, M_S_GREATER, M_S_GREATER_EQUAL,					//= _< =< _> =>		not supported

			M_FSTART,

		M_COS, M_ACOS, M_COSH, M_ACOSH, M_COSC,
		M_SEC, M_ASEC, M_SECH, M_ASECH,
		M_SIN, M_ASIN, M_SINH, M_ASINH, M_SINC, M_SINHC,
		M_CSC, M_ACSC, M_CSCH, M_ACSCH,
		M_TAN,         M_TANH, M_ATANH, M_TANC,
		M_COT, M_ACOT, M_COTH, M_ACOTH,
		M_EXP, M_LN, M_SQRT, M_CBRT, M_INVSQRT, M_SQ,
		M_GAUSS, M_ERF, M_FIB, M_ZETA, M_LNGAMMA,
		M_STEP, M_SGN, M_RECT, M_TENT,
		M_CEIL, M_FLOOR, M_ROUND, M_INT, M_FRAC,
		M_ABS, M_ARG, M_REAL, M_IMAG, M_CONJUGATE, M_POLAR, M_CARTESIAN,

			M_BFSTART,

		M_RAND,
		M_ATAN,
		M_LOG,
		M_BETA, M_GAMMA, M_PERMUTATION, M_COMBINATION,
		M_BESSEL_J, M_BESSEL_Y, M_HANKEL1,
		M_SQWV, M_TRWV, M_SAW, M_MIN, M_MAX, M_HYPOT, M_MANDELBROT,

		M_USER_FUNCTION
	};
	const double	_atm	=101325,			_bbr	=5.670373e-8,
					_c		=299792458,			_ele	=8.854187817620e-12,
					_e		=::exp(1.),			_g		=9.80665,
					_G		=6.67384e-11,		_h		=6.62606957e-34,
					_mag	=1.2566370614e-6,	_me		=9.10938291e-31,
					_mn		=1.674927351e-27,	_mp		=1.672621777e-27,
					_Me		=5.9736e24,			_Ms		=1.9891e30,
					_Na		=6.02214129e23,		_phi	=1.6180339887498948482045868343656381177203091798057628621354486227052604628189,
					_pi		=::acos(-1.),		_q		=1.602176565e-19,
					_R		=8.3144621,			_qnan	=std::numeric_limits<double>::quiet_NaN(),

					_ln2	=::log(2.),			_ln8=::log(8.),		_ln10	=::log(10.),	_sqrt2=::sqrt(2.),	_sqrt3=::sqrt(3.),	_sqrt5=::sqrt(5.),
					_pi_2=_pi*0.5,	_2pi=2*_pi,	_sqrt_2pi=::sqrt(_2pi), _ln_pi=::log(_pi),	_ln_sqrt_2pi=::log(_sqrt_2pi),	_1_2pi=1/_2pi, _1_pi=1/_pi,
					_third=1./3;
}
enum			InstructionSignature
{
	SIG_NOOP,

	SIG_R_R,	SIG_C_C,	SIG_Q_Q,

	SIG_R_RR,	SIG_C_RC,	SIG_Q_RQ,
	SIG_C_CR,	SIG_C_CC,	SIG_Q_CQ,
	SIG_Q_QR,	SIG_Q_QC,	SIG_Q_QQ,

	SIG_C_R,	SIG_C_Q,
	SIG_R_C,	SIG_R_Q,

	SIG_C_RR,

				SIG_R_RC,	SIG_R_RQ,
	SIG_R_CR,	SIG_R_CC,	SIG_R_CQ,
	SIG_R_QR,	SIG_R_QC,	SIG_R_QQ,

	SIG_C_QC,

	SIG_INLINE_IF,

	SIG_CALL='c',
	SIG_BIF='b',
	SIG_BIN='B',
	SIG_JUMP='j',
	SIG_RETURN='r',
};
inline int 		maximum(int a, int b, int c)
{
	int c2=c<<1, temp=a+b+abs(a-b);
	return (temp+c+abs(temp+c))>>2;
}
inline char 	returnMathSet_from_signature(int signature, char op1_ms, char op2_ms=0, char op3_ms=0)
{
	switch(signature)
	{
	case SIG_R_R:
	case SIG_R_RR:
	case SIG_R_C:
	case SIG_R_Q:
	case SIG_R_RC:
	case SIG_R_RQ:
	case SIG_R_CR:
	case SIG_R_CC:
	case SIG_R_CQ:
	case SIG_R_QR:
	case SIG_R_QC:
	case SIG_R_QQ:
		return 'R';
	case SIG_C_C:
	case SIG_C_RC:
	case SIG_C_CR:
	case SIG_C_CC:
	case SIG_C_R:
	case SIG_C_Q:
	case SIG_C_RR:
	case SIG_C_QC:
		return 'c';
	case SIG_Q_Q:
	case SIG_Q_RQ:
	case SIG_Q_CQ:
	case SIG_Q_QR:
	case SIG_Q_QC:
	case SIG_Q_QQ:
		return 'q';
	case SIG_INLINE_IF:
		return maximum(op1_ms, op2_ms, op3_ms);
	}
	return 0;
}
struct			Map
{
	G2::Map _0;
	int _1;
	int pos, len;
	Map(int pos=0, int len=0, G2::Map _0=G2::M_IGNORED, int _1=0):_0(_0), _1(_1), pos(pos), len(len){}
};
struct			Value;
struct			Quat1d;
struct			Comp1d
{
	double r, i;
	Comp1d(double r=0, double i=0):r(r), i(i){}
	Comp1d(std::complex<double> const &x):r(x.real()), i(x.imag()){}
//	operator std::complex<double>(){return std::complex<double>(r, i);}//doesn't work
	bool r_is_true()const{return r!=0;}
	bool c_is_true()const{return r!=0|i!=0;}
	Comp1d floor()const{return Comp1d(::floor(r), ::floor(i));}
	Comp1d ceil()const{return Comp1d(::ceil(r), ::ceil(i));}
	Comp1d round()const{return Comp1d(::round(r), ::round(i));}
	double abs_sq()const{return r*r+i*i;}
	double abs()const{return sqrt(r*r+i*i);}
	double arg()const{return double(::atan2(i, r));}
	Comp1d& operator+=(Comp1d const &b){r+=b.r, i+=b.i; return *this;}
	Comp1d& operator-=(Comp1d const &b){r-=b.r, i-=b.i; return *this;}
	Comp1d& operator*=(Comp1d const &b)
	{
		double rr=r*b.r-i*b.i, ri=r*b.i+i*b.r;
		r=rr, i=ri;
		return *this;
	}
	Comp1d& operator*=(double const &b){r*=b, i*=b; return *this;}
	Comp1d& operator/=(Comp1d const &b)
	{
		double _1_mag_b=1/sqrt(b.r*b.r+b.i*b.i);
		double
				rr=(b.r*r+b.i*i)*_1_mag_b,
				ri=(b.r*i-b.i*r)*_1_mag_b;
		r=rr, i=ri;
		return *this;
	}
	Comp1d& operator/=(double const &br){r/=br, i/=br; return *this;}
	Quat1d operator/=(Quat1d const &b);
	Comp1d& operator^=(Comp1d const &b)
	{
		Comp1d t(::log(sqrt(r*r+i*i)), atan2(i, r));
		t*=b;
		double r0=::exp(t.r), sin_ti, cos_ti;
		sincos(t.i, &sin_ti, &cos_ti);
		r=r0*cos_ti, i=r0*sin_ti;
		return *this;
	}
	Comp1d& operator^=(double const &br)
	{
		Comp1d t(::log(sqrt(r*r+i*i)), atan2(i, r));
		t*=br;
		double r0=::exp(t.r), sin_ti, cos_ti;
		sincos(t.i, &sin_ti, &cos_ti);
		r=r0*cos_ti, i=r0*sin_ti;
		return *this;
	}
};
struct			CompRef
{
	double &r, &i;
	CompRef(double &r, double &i):r(r), i(i){}
	CompRef& operator=(Comp1d const &x){r=x.r, i=x.i; return *this;}
	CompRef& operator*=(double x){r*=x, i*=x; return *this;}
};
inline Comp1d operator-(CompRef const &a, CompRef const &b){return Comp1d(a.r-b.r, a.i-b.i);}
inline Comp1d operator*(Comp1d const &a, Comp1d const &b){return Comp1d(a.r*b.r-a.i*b.i, a.r*b.i+a.i*b.r);}
inline Comp1d operator*(Comp1d const &a, double const &b){return Comp1d(a.r*b, a.i*b);}
inline Comp1d operator*(double const &a, Comp1d const &b){return Comp1d(a*b.r, a*b.i);}
inline Comp1d operator/(Comp1d const &a, Comp1d const &b)
{
	double _1_mag_b=1/(b.r*b.r+b.i*b.i);
	return Comp1d((b.r*a.r+b.i*a.i)*_1_mag_b, (b.r*a.i-b.i*a.r)*_1_mag_b);
}
inline Comp1d operator/(Comp1d const &a, double const &b){return Comp1d(a.r/b, a.i/b);}
inline Comp1d operator/(double const &a, Comp1d const &b)
{
	double _a_mag_b=a/(b.r*b.r+b.i*b.i);
	return Comp1d(b.r*_a_mag_b, -b.i*_a_mag_b);
}
inline Comp1d operator+(Comp1d const &a, Comp1d const &b){return Comp1d(a.r+b.r, a.i+b.i);}
inline Comp1d operator+(Comp1d const &a, double const &b){return Comp1d(a.r+b, a.i);}
inline Comp1d operator+(double const &a, Comp1d const &b){return Comp1d(a+b.r, b.i);}
inline Comp1d operator-(Comp1d const &a, Comp1d const &b){return Comp1d(a.r-b.r, a.i-b.i);}
inline Comp1d operator-(Comp1d const &a, double const &b){return Comp1d(a.r-b, a.i);}
inline Comp1d operator-(double const &a, Comp1d const &b){return Comp1d(a-b.r, -b.i);}
inline Comp1d operator-(Comp1d const &a){return Comp1d(-a.r, -a.i);}
inline double operator==(Comp1d const &a, Comp1d const &b){return (a.r==b.r)&(a.i==b.i);}
inline double operator==(Comp1d const &a, double const &b){return (a.r==b)&(a.i==0);}
inline double operator==(double const &a, Comp1d const &b){return (a==b.r)&(0==b.i);}
inline double operator!=(Comp1d const &a, Comp1d const &b){return (a.r!=b.r)|(a.i!=b.i);}
inline double operator!=(Comp1d const &a, double const &b){return (a.r!=b)|(a.i!=0);}
inline double operator!=(double const &a, Comp1d const &b){return (a!=b.r)|(0!=b.i);}
//inline Comp1d operator|(Comp1d const &a, Comp1d const &b){return Comp1d(a.r|b.r, a.i|b.i);}
//inline Comp1d operator|(Comp1d const &a, double const &b){return Comp1d(a.r|b, a.i);}
//inline Comp1d operator|(double const &a, Comp1d const &b){return Comp1d(a|b.r, b.i);}
inline Comp1d AND(Comp1d const &a, long long const &b)
{
	long long r=(long long&)a.r&b, i=(long long&)a.i&b;
	return Comp1d((double&)r, (double&)i);
}
inline Comp1d AND(long long const &a, Comp1d const &b)
{
	long long r=a&(long long&)b.r, i=a&(long long&)b.i;
	return Comp1d((double&)r, (double&)i);
}
inline double AND(double const &a, long long const &b)
{
	long long r=(long long&)a&b;
	return (double&)r;
}
//inline Comp1d AND(Comp1d const &a, double const &b)
//{
//	long long r=(long long&)a.r&(long long&)b, i=(long long&)a.i&(long long&)b;
//	return Comp1d((double&)r, (double&)i);
//}
//inline Comp1d AND(double const &a, Comp1d const &b)
//{
//	long long r=(long long&)a&(long long&)b.r, i=(long long&)a&(long long&)b.i;
//	return Comp1d((double&)r, (double&)i);
//}
//inline double AND(double const &a, double const &b)
//{
//	long long r=(long long&)a&(long long&)b;
//	return (double&)r;
//}
inline Comp1d OR(Comp1d const &a, long long const &b)
{
	long long r=(long long&)a.r|b, i=(long long&)a.i|b;
	return Comp1d((double&)r, (double&)i);
}
inline Comp1d OR(long long const &a, Comp1d const &b)
{
	long long r=a|(long long&)b.r, i=a|(long long&)b.i;
	return Comp1d((double&)r, (double&)i);
}
//inline Comp1d OR(Comp1d const &a, double const &b)
//{
//	long long r=(long long&)a.r|(long long&)b, i=(long long&)a.i|(long long&)b;
//	return Comp1d((double&)r, (double&)i);
//}
//inline Comp1d OR(double const &a, Comp1d const &b)
//{
//	long long r=(long long&)a|(long long&)b.r, i=(long long&)a|(long long&)b.i;
//	return Comp1d((double&)r, (double&)i);
//}
//inline double OR(double const &a, double const &b)
//{
//	long long r=(long long&)a|(long long&)b;
//	return (double&)r;
//}
inline double r_isTrue(double x)
{
	long long mask=-(x!=0);
	return (double&)mask;
}
inline Comp1d operator%(Comp1d const &a, Comp1d const &b){return AND((a-(a/b).floor()*b), -b.c_is_true());}
inline Comp1d operator%(Comp1d const &a, double const &b){return AND((a-(a/b).floor()*b), -r_isTrue(b));}
inline Comp1d operator%(double const &a, Comp1d const &b){return AND((a-(a/b).floor()*b), -b.c_is_true());}
inline Comp1d log(Comp1d const &x){return Comp1d(::log(sqrt(x.r*x.r+x.i*x.i)), ::atan2(x.i, x.r));}
inline Comp1d exp(Comp1d const &x)
{
	double sin_xi, cos_xi;
	sincos(x.i, &sin_xi, &cos_xi);
	double exp_xr=exp(x.r);
	return Comp1d(exp_xr*double(cos_xi), exp_xr*double(sin_xi));
}
inline Comp1d sqrt(Comp1d const &x)
{
	auto s=sqrt(2*(x.r+x.abs()));
	if(s==0)
		return Comp1d(0, sqrt(-x.r));
	return Comp1d(s*0.5, x.i/s);
}
inline Comp1d inv(Comp1d const &x)
{
	double inv_mag=1/x.abs();
	return Comp1d(x.r*inv_mag, -x.i*inv_mag);
}
inline Comp1d operator^(Comp1d const &a, Comp1d const &b)
{
	if(a.r==0&a.i==0&b.r==0&b.i==0)
		return Comp1d(1, 0);
	Comp1d t(::log(sqrt(a.r*a.r+a.i*a.i)), atan2(a.i, a.r));
	t*=b;
	double r0=::exp(t.r), sin_ti, cos_ti;
	sincos(t.i, &sin_ti, &cos_ti);
	return Comp1d(r0*cos_ti, r0*sin_ti);
}
inline Comp1d operator^(Comp1d const &a, double const &b)
{
	if(a.r==0&a.i==0&b==0)
		return Comp1d(1, 0);
	Comp1d t(::log(sqrt(a.r*a.r+a.i*a.i)), atan2(a.i, a.r));
	t*=b;
	double r0=::exp(t.r);
	double sin_ti, cos_ti;
	sincos(t.i, &sin_ti, &cos_ti);
	return Comp1d(r0*cos_ti, r0*sin_ti);
}
inline Comp1d operator^(double const &a, Comp1d const &b)
{
	if(a==0&b.r==0&b.i==0)
		return Comp1d(1, 0);
	Comp1d t(::log(abs(a)), atan2(0, a));
	t*=b;
	double r0=::exp(t.r);
	double sin_ti, cos_ti;
	sincos(t.i, &sin_ti, &cos_ti);
	return Comp1d(r0*cos_ti, r0*sin_ti);
}
struct			Quat1d
{
	double r, i, j, k;
	Quat1d(double r=0, double i=0, double j=0, double k=0):r(r), i(i), j(j), k(k){}
	Quat1d(Comp1d const &x):r(x.r), i(x.i), j(0), k(0){}
	Quat1d(Value const &v);
	bool r_is_true()const{return r!=0;}
	bool c_is_true()const{return r!=0|i!=0;}
	bool q_is_true()const{return r!=0|i!=0|j!=0|k!=0;}
	Quat1d floor()const{return Quat1d(::floor(r), ::floor(i), ::floor(j), ::floor(k));}
	Quat1d ceil()const{return Quat1d(::ceil(r), ::ceil(i), ::ceil(j), ::ceil(k));}
	Quat1d round()const{return Quat1d(::round(r), ::round(i), ::round(j), ::round(k));}
	double abs_sq()const{return r*r+i*i+j*j+k*k;}
	double abs()const{return sqrt(r*r+i*i+j*j+k*k);}
	Quat1d& operator=(Value const &x);
	Quat1d& operator=(double const &r){this->r=r, i=0, j=0, k=0; return *this;}
	Quat1d& operator+=(Quat1d const &b){r+=b.r, i+=b.i, j+=b.j, k+=b.k;return *this;}
	Quat1d& operator+=(Comp1d const &b){r+=b.r, i+=b.i;return *this;}
	Quat1d& operator+=(double const &b){r+=b;return *this;}
	Quat1d& operator-=(Quat1d const &b){r-=b.r, i-=b.i, j-=b.j, k-=b.k;return *this;}
	Quat1d& operator-=(Comp1d const &b){r-=b.r, i-=b.i;return *this;}
	Quat1d& operator-=(double const &b){r-=b;return *this;}
	Quat1d& operator*=(Quat1d const &b)
	{
		double
				rr=r*b.r+i*b.i+j*b.j+k*b.k,
				ri=r*b.i+i*b.r+j*b.k-k*b.j,
				rj=r*b.j-i*b.k+j*b.r+k*b.i,
				rk=r*b.k+i*b.j+j*b.i+k*b.r;
		r=rr, i=ri, j=rj, k=rk;
		return *this;
	}
	Quat1d& operator*=(Comp1d const &b)
	{
		double
				rr=r*b.r+i*b.i,
				ri=r*b.i+i*b.r,
				rj=j*b.r+k*b.i,
				rk=j*b.i+k*b.r;
		r=rr, i=ri, j=rj, k=rk;
		return *this;
	}
	Quat1d& operator*=(double const &b){r*=b, i*=b, j*=b, k*=b;return *this;}
	Quat1d& operator/=(Quat1d const &b)
	{
		double _1_mag_y=1/sqrt(b.r*b.r+b.i*b.i+b.j*b.j+b.k*b.k);
		double
				rr=(b.r*r+b.i*i+b.j*j+b.k*k)*_1_mag_y,
				ri=(b.r*i-b.i*r-b.j*k+b.k*j)*_1_mag_y,
				rj=(b.r*j+b.i*k-b.j*r-b.k*i)*_1_mag_y,
				rk=(b.r*k-b.i*j+b.j*i-b.k*r)*_1_mag_y;
		r=rr, i=ri, j=rj, k=rk;
		return *this;
	}
	Quat1d& operator/=(Comp1d const &b)
	{
		double _1_mag_y=1/sqrt(b.r*b.r+b.i*b.i);
		double
				rr=(b.r*r+b.i*i)*_1_mag_y,
				ri=(b.r*i-b.i*r)*_1_mag_y,
				rj=(b.r*j+b.i*k)*_1_mag_y,
				rk=(b.r*k-b.i*j)*_1_mag_y;
		r=rr, i=ri, j=rj, k=rk;
		return *this;
	}
	Quat1d& operator/=(double const &b){r/=b, i/=b, j/=b, k/=b;return *this;}
	Quat1d& operator^=(Quat1d const &b)
	{
		double mag_v=i*i+j*j+k*k;
		double ln_mag_a=log(sqrt(r*r+mag_v));
		mag_v=sqrt(mag_v);
		double v_mul=acos((r/ln_mag_a));
		v_mul/=mag_v;
		Quat1d t(ln_mag_a, i*v_mul, j*v_mul, k*v_mul);
		t=Quat1d(
				b.r*t.r+b.i*t.i+b.j*t.j+b.k*t.k,
				b.r*t.i+b.i*t.r+b.j*t.k-b.k*t.j,
				b.r*t.j-b.i*t.k+b.j*t.r+b.k*t.i,
				b.r*t.k+b.i*t.j+b.j*t.i+b.k*t.r);
		double mag_u=sqrt(t.i*t.i+t.j*t.j+t.k*t.k), sin_mu, cos_mu;
		sincos(mag_u, &sin_mu, &cos_mu);
		double exp_tr=exp(t.r);
		v_mul=exp_tr*double(sin_mu)/mag_u;
		r=exp_tr*double(cos_mu), i=t.i*v_mul, j=t.j*v_mul, k=t.k*v_mul;
		return *this;
	}
	Quat1d& operator^=(Comp1d const &b)
	{
		double mag_v=i*i+j*j+k*k;
		double ln_mag_a=log(sqrt(r*r+mag_v));
		mag_v=sqrt(mag_v);
		double v_mul=acos((r/ln_mag_a));
		v_mul/=mag_v;
		Quat1d t(ln_mag_a, i*v_mul, j*v_mul, k*v_mul);
		t=Quat1d(
				b.r*t.r+b.i*t.i,
				b.r*t.i+b.i*t.r,
				b.r*t.j-b.i*t.k,
				b.r*t.k+b.i*t.j);
		double mag_u=sqrt(t.i*t.i+t.j*t.j+t.k*t.k);
		double sin_mu, cos_mu;
		sincos(mag_u, &sin_mu, &cos_mu);
		double exp_tr=exp(t.r);
		v_mul=exp_tr*double(sin_mu)/mag_u;
		r=exp_tr*double(cos_mu), i=t.i*v_mul, j=t.j*v_mul, k=t.k*v_mul;
		return *this;
	}
	Quat1d& operator^=(double const &b)
	{
		double mag_v=i*i+j*j+k*k;
		double ln_mag_a=log(sqrt(r*r+mag_v));
		mag_v=sqrt(mag_v);
		double v_mul=acos((r/ln_mag_a));
		v_mul/=mag_v;
		Quat1d t(ln_mag_a, i*v_mul, j*v_mul, k*v_mul);
		t=Quat1d(b*t.r, b*t.i, b*t.j, b*t.k);
		double mag_u=sqrt(t.i*t.i+t.j*t.j+t.k*t.k);
		double sin_mu, cos_mu;
		sincos(mag_u, &sin_mu, &cos_mu);
		double exp_tr=exp(t.r);
		v_mul=exp_tr*double(sin_mu)/mag_u;
		r=exp_tr*double(cos_mu), i=t.i*v_mul, j=t.j*v_mul, k=t.k*v_mul;
		return *this;
	}
};
struct			QuatRef
{
	double &r, &i, &j, &k;
	QuatRef(double &r, double &i, double &j, double &k):r(r), i(i), j(j), k(k){}
	QuatRef& operator=(Quat1d const &x){r=x.r, i=x.i, j=x.j, k=x.k; return *this;}
	QuatRef& operator-=(Quat1d const &x){r-=x.r, i-=x.i, j-=x.j, k-=x.k; return *this;}
};
inline double operator==(Quat1d const &a, Quat1d const &b){return (a.r==b.r)&(a.i==b.i)&(a.j==b.j)&(a.k==b.k);}
inline double operator==(Quat1d const &a, Comp1d const &b){return (a.r==b.r)&(a.i==b.i)&(a.j==0)&(a.k==0);}
inline double operator==(Quat1d const &a, double const &b){return (a.r==b)&(a.i==0)&(a.j==0)&(a.k==0);}
inline double operator==(Comp1d const &a, Quat1d const &b){return (a.r==b.r)&(a.i==b.i)&(0==b.j)&(0==b.k);}
inline double operator==(double const &a, Quat1d const &b){return (a==b.r)&(0==b.i)&(0==b.j)&(0==b.k);}

inline double operator!=(Quat1d const &a, Quat1d const &b){return (a.r!=b.r)|(a.i!=b.i)|(a.j!=b.j)|(a.k!=b.k);}
inline double operator!=(Quat1d const &a, Comp1d const &b){return (a.r!=b.r)|(a.i!=b.i)|(a.j!=0)|(a.k!=0);}
inline double operator!=(Quat1d const &a, double const &b){return (a.r!=b)|(a.i!=0)|(a.j!=0)|(a.k!=0);}
inline double operator!=(Comp1d const &a, Quat1d const &b){return (a.r!=b.r)|(a.i!=b.i)|(0!=b.j)|(0!=b.k);}
inline double operator!=(double const &a, Quat1d const &b){return (a!=b.r)|(0!=b.i)|(0!=b.j)|(0!=b.k);}

inline Quat1d operator+(Quat1d const &a , Quat1d const &b){return Quat1d(a.r+b.r, a.i+b.i, a.j+b.j, a.k+b.k);}
inline Quat1d operator+(Quat1d const &a , Comp1d const &b){return Quat1d(a.r+b.r, a.i+b.i, a.j, a.k);}
inline Quat1d operator+(Quat1d const &a , double const &b){return Quat1d(a.r+b, a.i, a.j, a.k);}
inline Quat1d operator+(Comp1d const &a , Quat1d const &b){return Quat1d(a.r+b.r, a.i+b.i, b.j, b.k);}
inline Quat1d operator+(double const &ar, Quat1d const &b){return Quat1d(ar+b.r, b.i, b.j, b.k);}

inline Quat1d operator-(Quat1d const &a , Quat1d const &b){return Quat1d(a.r-b.r, a.i-b.i, a.j-b.j, a.k-b.k);}
inline Quat1d operator-(Quat1d const &a , Comp1d const &b){return Quat1d(a.r-b.r, a.i-b.i, a.j, a.k);}
inline Quat1d operator-(Quat1d const &a , double const &b){return Quat1d(a.r-b, a.i, a.j, a.k);}
inline Quat1d operator-(Comp1d const &a , Quat1d const &b){return Quat1d(a.r-b.r, a.i-b.i, -b.j, -b.k);}
inline Quat1d operator-(double const &ar, Quat1d const &b){return Quat1d(ar-b.r, -b.i, -b.j, -b.k);}
inline Quat1d operator-(Quat1d const &a){return Quat1d(-a.r, -a.i, -a.j, -a.k);}
inline Quat1d operator*(Quat1d const &a, Quat1d const &b)
{
	return Quat1d(
			a.r*b.r-a.i*b.i-a.j*b.j-a.k*b.k,
			a.r*b.i+a.i*b.r+a.j*b.k-a.k*b.j,
			a.r*b.j-a.i*b.k+a.j*b.r+a.k*b.i,
			a.r*b.k+a.i*b.j-a.j*b.i+a.k*b.r);
}
inline Quat1d operator*(Quat1d const &a, Comp1d const &b)
{
	return Quat1d(
			a.r*b.r-a.i*b.i,
			a.r*b.i+a.i*b.r,
			a.j*b.r+a.k*b.i,
			-a.j*b.i+a.k*b.r);
}
inline Quat1d operator*(Quat1d const &a, double const &b){return Quat1d(a.r*b, a.i*b, a.j*b, a.k*b);}
inline Quat1d operator*(Comp1d const &a, Quat1d const &b)
{
	return Quat1d(
			a.r*b.r-a.i*b.i,
			a.r*b.i+a.i*b.r,
			a.r*b.j-a.i*b.k,
			a.r*b.k+a.i*b.j);
}
inline Quat1d operator*(double const &ar, Quat1d const &b){return Quat1d(ar*b.r, ar*b.i, ar*b.j, ar*b.k);}
inline Quat1d operator/(Quat1d const &a, Quat1d const &b)
{
	double _1_mag_y=1/(b.r*b.r+b.i*b.i+b.j*b.j+b.k*b.k);
	return Quat1d(
			(b.r*a.r+b.i*a.i+b.j*a.j+b.k*a.k)*_1_mag_y,
			(b.r*a.i-b.i*a.r-b.j*a.k+b.k*a.j)*_1_mag_y,
			(b.r*a.j+b.i*a.k-b.j*a.r-b.k*a.i)*_1_mag_y,
			(b.r*a.k-b.i*a.j+b.j*a.i-b.k*a.r)*_1_mag_y);
}
inline Quat1d Comp1d::operator/=(Quat1d const &b)
{
	double _1_mag_y=1/(b.r*b.r+b.i*b.i+b.j*b.j+b.k*b.k);
	return Quat1d(
			(b.r*r+b.i*i)*_1_mag_y,
			(b.r*i-b.i*r)*_1_mag_y,
			(-b.j*r-b.k*i)*_1_mag_y,
			(b.j*i-b.k*r)*_1_mag_y);
}
inline Quat1d operator/(Quat1d const &a, Comp1d const &b)
{
	double _1_mag_y=1/(b.r*b.r+b.i*b.i);
	return Quat1d(
			(b.r*a.r+b.i*a.i)*_1_mag_y,
			(b.r*a.i-b.i*a.r)*_1_mag_y,
			(b.r*a.j+b.i*a.k)*_1_mag_y,
			(b.r*a.k-b.i*a.j)*_1_mag_y);
}
inline Quat1d operator/(Quat1d const &a, double const &b)
{
	double _1_mag_y=1/b;
	return Quat1d(a.r*_1_mag_y, a.i*_1_mag_y, a.j*_1_mag_y, a.k*_1_mag_y);
}
inline Quat1d operator/(Comp1d const &a, Quat1d const &b)
{
	double _1_mag_y=1/(b.r*b.r+b.i*b.i+b.j*b.j+b.k*b.k);
	return Quat1d(
			(b.r*a.r+b.i*a.i)*_1_mag_y,
			(b.r*a.i-b.i*a.r)*_1_mag_y,
			(-b.j*a.r-b.k*a.i)*_1_mag_y,
			(b.j*a.i-b.k*a.r)*_1_mag_y);
}
inline Quat1d operator/(double const &ar, Quat1d const &b)
{
	double _ar_mag_y=ar/(b.r*b.r+b.i*b.i+b.j*b.j+b.k*b.k);
	return Quat1d(b.r*_ar_mag_y, -b.i*_ar_mag_y, -b.j*_ar_mag_y, -b.k*_ar_mag_y);
}
inline Quat1d log(Quat1d const &x)
{
	double mag_v=x.i*x.i+x.j*x.j+x.k*x.k;
	double mag_x=sqrt(x.r*x.r+mag_v);
	mag_v=sqrt(mag_v);
	double ln_mag_x=::log(mag_x);
	double u_mul=double(::acos((x.r/mag_x)))/mag_v;
	return Quat1d(ln_mag_x, x.i*u_mul, x.j*u_mul, x.k*u_mul);
}
inline Quat1d exp(Quat1d const &x)
{
	double exp_r=::exp(x.r);
	double mag_v=sqrt(x.i*x.i+x.j*x.j+x.k*x.k), sin_v, cos_v;
	sincos(mag_v, &sin_v, &cos_v);
	double v_mul=exp_r*double(sin_v)/mag_v;
	return Quat1d(exp_r*double(cos_v), x.i*v_mul, x.j*v_mul, x.k*v_mul);
}
inline Quat1d sqrt(Quat1d const &x)
{
	auto s=sqrt(2*(x.r+x.abs()));
	if(s==0)
		return Quat1d(0, sqrt(-x.r), 0, 0);
	auto _1_s=1/s;
	return Quat1d(s*0.5, x.i*_1_s, x.j*_1_s, x.k*_1_s);
}
inline Quat1d inv(Quat1d const &x)
{
	double inv_mag=1/x.abs();
	return Quat1d(x.r*inv_mag, -x.i*inv_mag, -x.j*inv_mag, -x.k*inv_mag);
}
inline Quat1d operator^(Quat1d const &a, Quat1d const &b)
{
	if(a.r==0&a.i==0&a.j==0&a.k==0&b.r==0&b.i==0&b.j==0&b.k==0)
		return Quat1d(1, 0, 0, 0);
	return exp(b*log(a));
}
inline Quat1d operator^(Quat1d const &a, Comp1d const &b)
{
	if(a.r==0&a.i==0&a.j==0&a.k==0&b.r==0&b.i==0)
		return Quat1d(1, 0, 0, 0);
	return exp(b*log(a));
}
inline Quat1d operator^(Quat1d const &a, double const &b)
{
	if(a.r==0&a.i==0&a.j==0&a.k==0&b==0)
		return Quat1d(1, 0, 0, 0);
	return exp(b*log(a));
}
inline Quat1d operator^(Comp1d const &a, Quat1d const &b)
{
	if(a.r==0&a.i==0&b.r==0&b.i==0&b.j==0&b.k==0)
		return Quat1d(1, 0, 0, 0);
	return exp(b*log(a));
}
inline Quat1d operator^(double const &a, Quat1d const &b)
{
	if(a==0&b.r==0&b.i==0&b.j==0&b.k==0)
		return Quat1d(1, 0, 0, 0);
	return exp(b*::log(a));
}
//inline Quat1d operator|(Quat1d const &a, Quat1d const &b){return Quat1d(a.r|b.r, a.i|b.i, a.j|b.j, a.k|b.k);}
//inline Quat1d operator|(Quat1d const &a, Comp1d const &b){return Quat1d(a.r|b.r, a.i|b.i, a.j, a.k);}
//inline Quat1d operator|(Quat1d const &a, double const &b){return Quat1d(a.r|b, a.i, a.j, a.k);}
//inline Quat1d operator|(Comp1d const &a, Quat1d const &b){return Quat1d(a.r|b.r, a.i|b.i, b.j, b.k);}
//inline Quat1d operator|(double const &a, Quat1d const &b){return Quat1d(a|b.r, b.i, b.j, b.k);}
inline Quat1d AND(Quat1d const &a, long long const &b)
{
	long long
		r=(long long&)a.r&b,
		i=(long long&)a.i&b,
		j=(long long&)a.j&b,
		k=(long long&)a.k&b;
	return Quat1d((double&)r, (double&)i, (double&)j, (double&)k);
}
inline Quat1d AND(long long const &a, Quat1d const &b)
{
	long long
		r=a&(long long&)b.r,
		i=a&(long long&)b.i,
		j=a&(long long&)b.j,
		k=a&(long long&)b.k;
	return Quat1d((double&)r, (double&)i, (double&)j, (double&)k);
}
//inline Quat1d AND(Quat1d const &a, double const &b)
//{
//	long long
//		r=(long long&)a.r&(long long&)b,
//		i=(long long&)a.i&(long long&)b,
//		j=(long long&)a.j&(long long&)b,
//		k=(long long&)a.k&(long long&)b;
//	return Quat1d((double&)r, (double&)i, (double&)j, (double&)k);
//}
//inline Quat1d AND(double const &a, Quat1d const &b)
//{
//	long long
//			r=(long long&)a&(long long&)b.r,
//			i=(long long&)a&(long long&)b.i,
//			j=(long long&)a&(long long&)b.j,
//			k=(long long&)a&(long long&)b.k;
//	return Quat1d((double&)r, (double&)i, (double&)j, (double&)k);
//}
inline Quat1d OR(Quat1d const &a, long long const &b)
{
	long long r=(long long&)a.r|b, i=(long long&)a.i|b, j=(long long&)a.j|b, k=(long long&)a.k|b;
	return Quat1d((double&)r, (double&)i, (double&)j, (double&)k);
}
inline Quat1d OR(long long const &a, Quat1d const &b)
{
	long long r=a|(long long&)b.r, i=a|(long long&)b.i, j=a|(long long&)b.j, k=a|(long long&)b.k;
	return Quat1d((double&)r, (double&)i, (double&)j, (double&)k);
}
inline Quat1d operator%(Quat1d const &a, Quat1d const &b){return AND((a-(a/b).floor()*b), -a.q_is_true());}
inline Quat1d operator%(Quat1d const &a, Comp1d const &b){return AND((a-(a/b).floor()*b), -a.q_is_true());}
inline Quat1d operator%(Quat1d const &a, double const &b){return AND((a-(a/b).floor()*b), -a.q_is_true());}
inline Quat1d operator%(Comp1d const &a, Quat1d const &b){return AND((a-(a/b).floor()*b), -a.c_is_true());}
inline Quat1d operator%(double const &a, Quat1d const &b){return AND((a-(a/b).floor()*b), -r_isTrue(a));}
template<int buffer_size>inline bool printValue_real		(				char (&buffer)[buffer_size], int &offset, double const &value)
{
	if(value)
	{
		offset+=sprintf(buffer+offset,

			value!=value?//NAN
						  (long long&)value==0x7FF8000000000010?	"+-inf"
			:										"0/0"
			:value==_HUGE?	"inf"
			:value==-_HUGE?	"-inf"
			:				"%.15g"

			, value);//
		return true;
	}
	return false;
}
template<int buffer_size>inline bool printValue_real		(				char (&buffer)[buffer_size], int &offset, double const &value, int const base)
{
	if(value)
	{
		if(value!=value)
			offset+=sprintf(buffer+offset, (long long&)value==0x7FF8000000000010?"+-inf":"0/0");
		else if(value==_HUGE)
			offset+=sprintf(buffer+offset, "inf");
		else if(value==-_HUGE)
			offset+=sprintf(buffer+offset, "-inf");
		else if(base==10)
			offset+=sprintf(buffer+offset, "%.15g", value);
		else
		{
			const int l2int[]={0xFF, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4};
			int l2base=l2int[base];
			if(value<0)
				offset+=sprintf(buffer+offset, "-");
			const char *prefix[]={"", "B", "", "0", "0x"};
			offset+=sprintf(buffer+offset, "%s", prefix[l2base]);
			int exponent=(((int*)&value)[1]>>20&0x000007FF)-1023;
			auto mantissa=((long long&)value&0x000FFFFFFFFFFFFF)|0x0010000000000000;//1.<52bits>*2^<11bits>		1.m<<e
			long long digit;
			if(exponent>=-13&&exponent<=50)
			{
				if(exponent<0)
				{
					int nzeros=-(exponent+1)/l2base;
					if(nzeros)
						offset+=sprintf(buffer+offset, "0.%0*d", nzeros, 0);
					else
						offset+=sprintf(buffer+offset, "0.");
					for(int k=52-(l2base+exponent%l2base)%l2base;k>=0&&mantissa;k-=l2base)
					{
						digit=mantissa>>k, mantissa-=digit<<k;
						offset+=sprintf(buffer+offset, "%llX", digit);
					}
				}
				else for(int k=52-exponent%l2base;k>=0;k-=l2base)//>=0 <=50
					{
						digit=mantissa>>k, mantissa-=digit<<k;
						offset+=sprintf(buffer+offset, "%llX", digit);
						if(k==52-exponent)
						{
							if(mantissa)
							{
								offset+=sprintf(buffer+offset, ".");
								for(k-=l2base;k>=0&&mantissa;k-=l2base)
								{
									digit=mantissa>>k, mantissa-=digit<<k;
									offset+=sprintf(buffer+offset, "%llX", digit);
								}
								if(mantissa)
								{
									digit=mantissa<<((l2base-(52-exponent)%l2base)%l2base), mantissa-=digit<<k;
									offset+=sprintf(buffer+offset, "%llX", digit);
								}
							}
							break;
						}
					}
			}
			else
			{
				int k=52;//p=2
				//	int k=52-exponent%l2base;//p=16
				//	int k=52-(l2base+exponent%l2base)%l2base;
				digit=mantissa>>k, mantissa-=digit<<k;
				offset+=sprintf(buffer+offset, "%llX.", digit);
				for(k-=l2base;k>=0&&mantissa;k-=l2base)
				{
					digit=mantissa>>k, mantissa-=digit<<k;
					offset+=sprintf(buffer+offset, "%llX", digit);
				}
				if(exponent<0)
					offset+=sprintf(buffer+offset, "p-");
				else
					offset+=sprintf(buffer+offset, "p+");
				//	exponent/=l2base;
				for(int k=10-10%l2base, digit;k>=0&&exponent;k-=l2base)
				{
					if((digit=exponent>>k))
					{
						exponent-=digit<<k;
						offset+=sprintf(buffer+offset, "%X", digit);
						for(k-=l2base;k>=0&&exponent;k-=l2base)
						{
							digit=exponent>>k, exponent-=digit<<k;
							offset+=sprintf(buffer+offset, "%X", digit);
						}
						break;
					}
				}
			}
		}
		return true;
	}
	return false;
}
template<int buffer_size>inline void printValue_unreal		(bool &written, char (&buffer)[buffer_size], int &offset, double const &value, char const *component)
{
	if(value)
	{
		offset+=sprintf(buffer+offset,

			value!=value?//NAN
				(long long&)value==0x7FF8000000000010?	"+-inf"
				:										written?"+0/0":"0/0"
			:value==_HUGE?			written?"+inf":"inf"
			:value==-_HUGE?			"-inf"
			:value==-1?				"-"
			:value==1?				written?"+":""
			:						written&&value>0?"+%.15g":"%.15g"

			, value);//
		offset+=sprintf(buffer+offset, component);
		written=true;
	}
}
template<int buffer_size>inline void printValue_unreal		(bool &written, char (&buffer)[buffer_size], int &offset, double const &value, char const *component, int const base)
{
	if(value)
	{
		if(value!=value)
			offset+=sprintf(buffer+offset, (long long&)value==0x7FF8000000000010?"+-inf":written?"+0/0":"0/0");
		else if(value==_HUGE)
			offset+=sprintf(buffer+offset, written?"+inf":"inf");
		else if(value==-_HUGE)
			offset+=sprintf(buffer+offset, "-inf");
		else if(value==1)
			offset+=sprintf(buffer+offset, written?"+":"");
		else if(value==-1)
			offset+=sprintf(buffer+offset, "-");
		else if(base==10)
			offset+=sprintf(buffer+offset, written&&value>0?"+%.15g":"%.15g", value);
		else
		{
			const unsigned l2int[]={0x80000000, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4};
			int l2base=l2int[base];
			if(written&&value>0)
				offset+=sprintf(buffer+offset, "+");
			if(value<0)
				offset+=sprintf(buffer+offset, "-");
			const char *prefix[]={"", "B", "", "0", "0x"};
			offset+=sprintf(buffer+offset, "%s", prefix[l2base]);
			int exponent=(((int*)&value)[1]>>20&0x000007FF)-1023;
			auto mantissa=((long long&)value&0x000FFFFFFFFFFFFF)|0x0010000000000000;//1.<52bits>*2^<11bits>		1.m<<e
			long long digit;
			if(exponent>=-13&&exponent<=50)
			{
				if(exponent<0)
				{
					int nzeros=-(exponent+1)/l2base;
					if(nzeros)
						offset+=sprintf(buffer+offset, "0.%0*d", nzeros, 0);
					else
						offset+=sprintf(buffer+offset, "0.");
					for(int k=52-exponent%l2base;k>=0&&mantissa;k-=l2base)
					{
						digit=mantissa>>k, mantissa-=digit<<k;
						offset+=sprintf(buffer+offset, "%llX", digit);
					}
				}
				else for(int k=52-exponent%l2base;k>=0;k-=l2base)//>=0 <=50
					{
						digit=mantissa>>k, mantissa-=digit<<k;
						offset+=sprintf(buffer+offset, "%llX", digit);
						if(k==52-exponent)
						{
							if(mantissa)
							{
								offset+=sprintf(buffer+offset, ".");
								for(k-=l2base;k>=0&&mantissa;k-=l2base)
								{
									digit=mantissa>>k, mantissa-=digit<<k;
									offset+=sprintf(buffer+offset, "%llX", digit);
								}
								if(mantissa)
								{
									digit=mantissa<<((l2base-(52-exponent)%l2base)%l2base), mantissa-=digit<<k;
									offset+=sprintf(buffer+offset, "%llX", digit);
								}
							}
							break;
						}
					}
			}
			else
			{
				int k=52;
				//	int k=52-(l2base+exponent%l2base)%l2base;
				digit=mantissa>>k, mantissa-=digit<<k;
				offset+=sprintf(buffer+offset, "%llX.", digit);
				for(k-=l2base;k>=0&&mantissa;k-=l2base)
				{
					digit=mantissa>>k, mantissa-=digit<<k;
					offset+=sprintf(buffer+offset, "%llX", digit);
				}
				if(exponent<0)
					offset+=sprintf(buffer+offset, "p-");
				else
					offset+=sprintf(buffer+offset, "p+");
				//	exponent/=l2base;
				for(int k=10-10%l2base, digit;k>=0&&exponent;k-=l2base)
				{
					if((digit=exponent>>k))
					{
						exponent-=digit<<k;
						offset+=sprintf(buffer+offset, "%X", digit);
						for(k-=l2base;k>=0&&exponent;k-=l2base)
						{
							digit=exponent>>k, exponent-=digit<<k;
							offset+=sprintf(buffer+offset, "%X", digit);
						}
						break;
					}
				}
			}
		}
		offset+=sprintf(buffer+offset, "%s", component);
		written=true;
	}
}
struct			Value
{
	double r, i, j, k;//in order
	Value(double r=0, double i=0, double j=0, double k=0):r(r), i(i), j(j), k(k){}
//	Value(std::complex<double> const &x):r(x.real()), i(x.imag()), j(0), k(0){}
//	Value(boost::math::quaternion<double> const &x):r(x.R_component_1()), i(x.R_component_2()), j(x.R_component_3()), k(x.R_component_4()){}
	void set(double r, double i=0, double j=0, double k=0){this->r=r, this->i=i, this->j=j, this->k=k;}
	void set(Quat1d const &x){r=x.r, i=x.i, j=x.j, k=x.k;}
	operator double&						(){return r;}
	operator double							()const{return r;}
//	operator std::complex<double>			()const{return std::complex<double>(r, i);}
//	operator boost::math::quaternion<double>()const{return boost::math::quaternion<double>(r, i, j, k);}
	Value& operator=	(double							const &x){r=x, i=j=k=0;																				return *this;}
//	Value& operator=	(std::complex<double>			const &x){r=x.real(), i=x.imag(), j=k=0;															return *this;}
//	Value& operator=	(boost::math::quaternion<double>const &x){r=x.R_component_1(), i=x.R_component_2(), j=x.R_component_3(), k=x.R_component_4();		return *this;}
	Value& operator+=	(double							const &x){r+=x;																						return *this;}
//	Value& operator+=	(std::complex<double>			const &x){r+=x.real(), i+=x.imag();																	return *this;}
//	Value& operator+=	(boost::math::quaternion<double>const &x){r+=x.R_component_1(), i+=x.R_component_2(), j+=x.R_component_3(), k+=x.R_component_4();	return *this;}
	Value& operator-=	(double							const &x){r-=x;																						return *this;}
//	Value& operator-=	(std::complex<double>			const &x){r-=x.real(), i-=x.imag();																	return *this;}
//	Value& operator-=	(boost::math::quaternion<double>const &x){r-=x.R_component_1(), i-=x.R_component_2(), j-=x.R_component_3(), k-=x.R_component_4();	return *this;}
	bool r_isTrue()const{return r!=0;}
	bool c_isTrue()const{return r!=0||i!=0;}
	bool q_isTrue()const{return r!=0||i!=0||j!=0||k!=0;}
	template<int buffer_size>void printReal(char (&buffer)[buffer_size], int &offset)const
	{
		if(!printValue_real(buffer, offset, r))
			offset+=sprintf(buffer+offset, "0");
	}
	template<int buffer_size>void printReal(char (&buffer)[buffer_size], int &offset, int base)const
	{
		if(!printValue_real(buffer, offset, r, base))
			offset+=sprintf(buffer+offset, "0");
	}
	template<int _size>void printComplex(char(&)[_size], int&)const;
	template<int _size>void printComplex(char(&)[_size], int&, int)const;
	template<int _size>void printQuaternion(char(&)[_size], int&)const;
	template<int _size>void printQuaternion(char(&)[_size], int&, int)const;
	template<int _size>void print(char (&a)[_size], int &o, char mathSet)const
	{
		switch(mathSet)
		{
			case 'R':
				printReal(a, o);
				break;
			case 'c':
				printComplex(a, o);
				break;
			case 'h':
				printQuaternion(a, o);
				break;
		}
	}
	template<int _size>void print(char (&a)[_size], int &o, char mathSet, int base)const
	{
		switch(mathSet)
		{
			case 'R':
				printReal(a, o, base);
				break;
			case 'c':
				printComplex(a, o, base);
				break;
			case 'h':
				printQuaternion(a, o, base);
				break;
		}
	}
};
struct			DiscontinuityFunction
{
	bool disc_in, disc_out;//evaluated before/after the function
	union
	{
		bool
		(*d_o)(Value const&, Value const&),
		(*ud_i)(Value const&, Value const&),
		(*bd_i)(Value const&, Value const&, Value const&, Value const&),
		(*td_i)(Value const&, Value const&, Value const&, Value const&, Value const&, Value const&);
	};

	//continuous function
	DiscontinuityFunction():
			disc_in(false), disc_out(false){}

	//discontinuity takes unary function argument or any function's output
	DiscontinuityFunction(bool (*d)(Value const&, Value const&), bool disc_in):
			disc_in(disc_in), disc_out(!disc_in), ud_i(d){}

	//discontinuity takes binary function arguments
	DiscontinuityFunction(bool (*bd_i)(Value const&, Value const&, Value const&, Value const&)):
			disc_in(true), disc_out(false), bd_i(bd_i){}

	//discontinuity takes triary function arguments
	DiscontinuityFunction(bool (*td_i)(Value const&, Value const&, Value const&, Value const&, Value const&, Value const&)):
			disc_in(true), disc_out(false), td_i(td_i){}

	//continuous function
	void operator()()
	{disc_out=disc_in=false;}

	//discontinuity takes unary function argument or any function's output
	void operator()(bool (*d)(Value const&, Value const&), bool disc_in=true)
	{disc_out=!(this->disc_in=disc_in), ud_i=d;}

	//discontinuity takes binary function arguments
	void operator()(bool (*bd_i)(Value const&, Value const&, Value const&, Value const&))
	{disc_out=!(disc_in=true), this->bd_i=bd_i;}

	//discontinuity takes triary function arguments
	void operator()(bool (*td_i)(Value const&, Value const&, Value const&, Value const&, Value const&, Value const&))
	{disc_out=!(disc_in=true), this->td_i=td_i;}
};
struct			FunctionPointer
{
	int type;
	union
	{
		double (*r_r)(double const&);//1
		Comp1d (*c_c)(Comp1d const&);//2
		Quat1d (*q_q)(Quat1d const&);//3

		double (*r_rr)(double const&, double const&);//4
		Comp1d (*c_rc)(double const&, Comp1d const&);//5
		Quat1d (*q_rq)(double const&, Quat1d const&);//6
		Comp1d (*c_cr)(Comp1d const&, double const&);//7
		Comp1d (*c_cc)(Comp1d const&, Comp1d const&);//8
		Quat1d (*q_cq)(Comp1d const&, Quat1d const&);//9
		Quat1d (*q_qr)(Quat1d const&, double const&);//10
		Quat1d (*q_qc)(Quat1d const&, Comp1d const&);//11
		Quat1d (*q_qq)(Quat1d const&, Quat1d const&);//12

		Comp1d	(*c_r)(double const &x);//13
		Comp1d	(*c_q)(Quat1d const &x);//14

		double  (*r_c)(Comp1d const &x);//15
		double  (*r_q)(Quat1d const &x);//16

		Comp1d	(*c_rr)(double const &x,	double const &y);//17	//pow, tetrate

		double	(*r_rc)(double const &x,	Comp1d const &y);//18	//&& || ##
		double	(*r_rq)(double const &x,	Quat1d const &y);//19
		double	(*r_cr)(Comp1d const &x,	double const &y);//20
		double	(*r_cc)(Comp1d const &x,	Comp1d const &y);//21
		double	(*r_cq)(Comp1d const &x,	Quat1d const &y);//22
		double	(*r_qr)(Quat1d const &x,	double const &y);//23
		double	(*r_qc)(Quat1d const &x,	Comp1d const &y);//24
		double	(*r_qq)(Quat1d const &x,	Quat1d const &y);//25

		Comp1d	(*c_qc)(Quat1d const &x,	Comp1d const &y);//26	//conditional 110 & 101
	};
	FunctionPointer():r_r(0), type(0){}
	void setzero(){this->r_r=(double(*)(double const&))-1, type=-1;}//-> constant zero
	void set(){this->r_r=0, type=0;}
	void set(double (*r_r)(double const&)){this->r_r=r_r, type=1;}
	void set(Comp1d (*c_c)(Comp1d const&)){this->c_c=c_c, type=2;}
	void set(Quat1d (*q_q)(Quat1d const&)){this->q_q=q_q, type=3;}
	void set(double (*r_rr)(double const&, double const&)){this->r_rr=r_rr, type=4;}
	void set(Comp1d (*c_rc)(double const&, Comp1d const&)){this->c_rc=c_rc, type=5;}
	void set(Quat1d (*q_rq)(double const&, Quat1d const&)){this->q_rq=q_rq, type=6;}
	void set(Comp1d (*c_cr)(Comp1d const&, double const&)){this->c_cr=c_cr, type=7;}
	void set(Comp1d (*c_cc)(Comp1d const&, Comp1d const&)){this->c_cc=c_cc, type=8;}
	void set(Quat1d (*q_cq)(Comp1d const&, Quat1d const&)){this->q_cq=q_cq, type=9;}
	void set(Quat1d (*q_qr)(Quat1d const&, double const&)){this->q_qr=q_qr, type=10;}
	void set(Quat1d (*q_qc)(Quat1d const&, Comp1d const&)){this->q_qc=q_qc, type=11;}
	void set(Quat1d (*q_qq)(Quat1d const&, Quat1d const&)){this->q_qq=q_qq, type=12;}

	void set(Comp1d (*c_r)(double const&)){this->c_r=c_r, type=13;}
	void set(Comp1d (*c_q)(Quat1d const&)){this->c_q=c_q, type=14;}

	void set(double (*r_c)(Comp1d const&)){this->r_c=r_c, type=15;}
	void set(double (*r_q)(Quat1d const&)){this->r_q=r_q, type=16;}

	void set(Comp1d (*c_rr)(double const&, double const&)){this->c_rr=c_rr, type=17;}

	void set(double (*r_rc)(double const&, Comp1d const&)){this->r_rc=r_rc, type=18;}
	void set(double (*r_rq)(double const&, Quat1d const&)){this->r_rq=r_rq, type=19;}
	void set(double (*r_cr)(Comp1d const&, double const&)){this->r_cr=r_cr, type=20;}
	void set(double (*r_cc)(Comp1d const&, Comp1d const&)){this->r_cc=r_cc, type=21;}
	void set(double (*r_cq)(Comp1d const&, Quat1d const&)){this->r_cq=r_cq, type=22;}
	void set(double (*r_qr)(Quat1d const&, double const&)){this->r_qr=r_qr, type=23;}
	void set(double (*r_qc)(Quat1d const&, Comp1d const&)){this->r_qc=r_qc, type=24;}
	void set(double (*r_qq)(Quat1d const&, Quat1d const&)){this->r_qq=r_qq, type=25;}

	void set(Comp1d (*c_qc)(Quat1d const&, Comp1d const&)){this->c_qc=c_qc, type=26;}
};
struct			Instruction
{
	// 1	 r(double const &x)
	// 2	 c(Comp1d const &x)
	// 3	 q(Quat1d const &x)
	// 4	rr(double const &x, double const &y)
	// 5 *	rc(double const &x, Comp1d const &y)
	// 6 *	rq(double const &x, Quat1d const &y)
	// 7	cr(Comp1d const &x, double const &y)
	// 8	cc(Comp1d const &x, Comp1d const &y)
	// 9 *	cq(Comp1d const &x, Quat1d const &y)
	//10	qr(Quat1d const &x, double const &y)
	//11	qc(Quat1d const &x, Comp1d const &y)
	//12	qq(Quat1d const &x, Quat1d const &y)

	//13	Comp1d	c_r(double const &x);				//sqrt, ln, polar
	//14 +	Comp1d	c_q(Quat1d const &x);				//polar

	//15 +	double  r_c(Comp1d const &x);				//re, im, arg
	//16 +	double  r_q(Quat1d const &x);				//abs

	//17 +	Comp1d	c_rr(double const &x, double const &y);//pow, tetrate

	//18	double	r_rc(double const &x, Comp1d const &y);//&& || ##
	//19	double	r_rq(double const &x, Quat1d const &y);
	//20	double	r_cr(Comp1d const &x, double const &y);
	//21	double	r_cc(Comp1d const &x, Comp1d const &y);
	//22	double	r_cq(Comp1d const &x, Quat1d const &y);
	//23	double	r_qr(Quat1d const &x, double const &y);
	//24	double	r_qc(Quat1d const &x, Comp1d const &y);
	//25	double	r_qq(Quat1d const &x, Quat1d const &y);

	//26	Comp1d	c_qc(Quat1d const &x, Comp1d const &y);

	//27	a ? b : c
	//'c' call						n[result]=ufd[op1]
	//'b' branch if					if(data[op1])i=result
	//'B' branch if not				if(!data[op1])i=result
	//'j' jump						i=result
	//'r' return					data[result]
	char type;
	int cl_idx, cl_disc_idx;

	int result, op1, op2, op3;
	char r_ms, op1_ms, op2_ms, op3_ms;
	union
	{
		double (*r_r)(double const&);//1
		Comp1d (*c_c)(Comp1d const&);//2
		Quat1d (*q_q)(Quat1d const&);//3

		double (*r_rr)(double const&, double const&);//4
		Comp1d (*c_rc)(double const&, Comp1d const&);//5
		Quat1d (*q_rq)(double const&, Quat1d const&);//6
		Comp1d (*c_cr)(Comp1d const&, double const&);//7
		Comp1d (*c_cc)(Comp1d const&, Comp1d const&);//8
		Quat1d (*q_cq)(Comp1d const&, Quat1d const&);//9
		Quat1d (*q_qr)(Quat1d const&, double const&);//10
		Quat1d (*q_qc)(Quat1d const&, Comp1d const&);//11
		Quat1d (*q_qq)(Quat1d const&, Quat1d const&);//12

		Comp1d	(*c_r)(double const &x);//13	//sqrt, ln, polar
		Comp1d	(*c_q)(Quat1d const &x);//14	//polar

		double  (*r_c)(Comp1d const &x);//15	//re, im, arg
		double  (*r_q)(Quat1d const &x);//16	//abs

		Comp1d	(*c_rr)(double const &x,	double const &y);//17	//pow, tetrate

		double	(*r_rc)(double const &x,	Comp1d const &y);//18	//&& || ##
		double	(*r_rq)(double const &x,	Quat1d const &y);//19
		double	(*r_cr)(Comp1d const &x,	double const &y);//20
		double	(*r_cc)(Comp1d const &x,	Comp1d const &y);//21
		double	(*r_cq)(Comp1d const &x,	Quat1d const &y);//22
		double	(*r_qr)(Quat1d const &x,	double const &y);//23
		double	(*r_qc)(Quat1d const &x,	Comp1d const &y);//24
		double	(*r_qq)(Quat1d const &x,	Quat1d const &y);//25

		Comp1d	(*c_qc)(Quat1d const &x,	Comp1d const &y);//26	//conditional 110 & 101
	};
	DiscontinuityFunction d;

	std::vector<int> args;//arg positions

	//call
	Instruction(int function, std::vector<int> const &args, int n_result):type('c'), op1(function), args(std::move(args)), result(n_result),
		cl_idx(0), cl_disc_idx(0), op2(-1), op3(-1), r_ms(0), op1_ms(0), op2_ms(0), op3_ms(0), r_r(nullptr){}

	Instruction(char type, int n_condition):type(type), op1(n_condition),//branch:		'b' branch if		'B' branch if not		op1 condition, result dest
		cl_idx(0), cl_disc_idx(0), result(0), op2(-1), op3(-1), r_ms(0), op1_ms(0), op2_ms(0), op3_ms(0), r_r(nullptr){}
	Instruction():type('j'),//jump		result dest
		cl_idx(0), cl_disc_idx(0), result(0), op1(-1), op2(-1), op3(-1), r_ms(0), op1_ms(0), op2_ms(0), op3_ms(0), r_r(nullptr){}
	Instruction(int n_result):type('r'), result(n_result),//return		result result
		cl_idx(0), cl_disc_idx(0), op1(-1), op2(-1), op3(-1), r_ms(0), op1_ms(0), op2_ms(0), op3_ms(0), r_r(nullptr){}

	//unary function
	Instruction(FunctionPointer &fp, int op1, char op1_ms, int result, char r_ms, DiscontinuityFunction &d, int cl_idx, int cl_disc_idx):
		r_r(fp.r_r), d(d), type(fp.type), op1(op1), op1_ms(op1_ms), op2(-1), op2_ms(0), op3(-1), op3_ms(0), result(result), r_ms(r_ms), cl_idx(cl_idx), cl_disc_idx(cl_disc_idx){}

	//binary function
	Instruction(FunctionPointer &fp, int op1, char op1_ms, int op2, char op2_ms, int result, char r_ms, DiscontinuityFunction &d, int cl_idx, int cl_disc_idx):
		r_r(fp.r_r), d(d), type(fp.type), op1(op1), op1_ms(op1_ms), op2(op2), op2_ms(op2_ms), op3(-1), op3_ms(0), result(result), r_ms(r_ms), cl_idx(cl_idx), cl_disc_idx(cl_disc_idx){}

	//inline if
	Instruction(int op1, char op1_ms, int op2, char op2_ms, int op3, char op3_ms, int result, char r_ms, DiscontinuityFunction &d, int cl_idx, int cl_disc_idx):
		r_r(0), d(d), type(27), op1(op1), op1_ms(op1_ms), op2(op2), op2_ms(op2_ms), op3(op3), op3_ms(op3_ms), result(result), r_ms(r_ms), cl_idx(cl_idx), cl_disc_idx(cl_disc_idx){}
};
struct			Variable
{
	//std::vector<unsigned short> name;
	std::string name;
	char mathSet,//R real, c complex, h quaternion
			varTypeR, varTypeI, varTypeJ, varTypeK;//x y z space, t time, c constant
	Value val;

	//real variable
	Variable(unsigned short const *a, int len, int varTypeR):mathSet('R'), varTypeR(varTypeR)
	{
		utf16ToAscii(a, len, name);
	}
//	Variable(unsigned short const *a, int len, int varTypeR):
//			name(a, a+len), mathSet('R'), varTypeR(varTypeR){}
	//	name(a, len), mathSet('R'), varTypeR(varTypeR){}

	//complex variable
	Variable(unsigned short const *a, int len, int varTypeR, int varTypeI):mathSet('c'), varTypeR(varTypeR), varTypeI(varTypeI)
	{
		utf16ToAscii(a, len, name);
	}
//	Variable(unsigned short const *a, int len, int varTypeR, int varTypeI):
//		name(a, a+len), mathSet('c'), varTypeR(varTypeR), varTypeI(varTypeI){}
	//	name(a, len), mathSet('C'), varTypeR(varTypeR), varTypeI(varTypeI){}

	//quaternion variable
	Variable(unsigned short const *a, int len, int varTypeR, int varTypeI, int varTypeJ, int varTypeK):
		mathSet('h'), varTypeR(varTypeR), varTypeI(varTypeI), varTypeJ(varTypeJ), varTypeK(varTypeK)
	{
		utf16ToAscii(a, len, name);
	}
//	Variable(unsigned short const *a, int len, int varTypeR, int varTypeI, int varTypeJ, int varTypeK):
//			name(a, a+len), mathSet('h'), varTypeR(varTypeR), varTypeI(varTypeI), varTypeJ(varTypeJ), varTypeK(varTypeK){}
};
struct			UFVariableName
{
//	std::vector<unsigned short> name;
	std::string name;
	int scopeLevel, data_idx;
//	UFVariableName(const unsigned short *a, int len, int scopeLevel, int data_idx):name(a, a+len), scopeLevel(scopeLevel), data_idx(data_idx){}
	UFVariableName(const unsigned short *a, int len, int scopeLevel, int data_idx):scopeLevel(scopeLevel), data_idx(data_idx)
	{
		utf16ToAscii(a, len, name);
	}
};
struct			Term
{
	bool constant;
	char mathSet;//'R' real, 'c' complex, 'h' quaternion	larger value = superset	//'C'==67, 'H'==72, ['R'==82, 'c'==99, 'h'==104], 'r'==114
	int varNo;
	std::vector<double> r, i, j, k;
	void assign(int position, Quat1d const &v)
	{
		if(mathSet=='R')
			r[position]=v.r;
		else if(mathSet=='c')
			r[position]=v.r, i[position]=v.i;
		else
			r[position]=v.r, i[position]=v.i, j[position]=v.j, k[position]=v.k;
	}
	void assign(int position, Value const &v)
	{
		if(mathSet=='R')
			r[position]=v.r;
		else if(mathSet=='c')
			r[position]=v.r, i[position]=v.i;
		else
			r[position]=v.r, i[position]=v.i, j[position]=v.j, k[position]=v.k;
	}
	void assign(int position, double r, double i=0, double j=0, double k=0)
	{
		if(mathSet=='R')
			this->r[position]=r;
		else if(mathSet=='c')
			this->r[position]=r, this->i[position]=i;
		else
			this->r[position]=r, this->i[position]=i, this->j[position]=j, this->k[position]=k;
	}

	//constant
	Term(char mathSet='R'):constant(true), mathSet(mathSet){}

	//function variable constant==false//
	Term(char mathSet, bool constant):constant(constant), mathSet(mathSet), varNo(0){}

	//expr variable
	Term(char mathSet, int varNo):constant(false), mathSet(mathSet), varNo(varNo){}
};
struct			Expression
{
	std::vector<std::pair<int, int>> syntaxErrors;//highlight text[first, second[
	void insertSyntaxError(int first, int second)
	{
		int k=0, kEnd=syntaxErrors.size();
		for(;k<kEnd&&syntaxErrors[k].first<first;++k);
		if(k==kEnd||syntaxErrors[k].first!=first)
			syntaxErrors.insert(syntaxErrors.begin()+k, std::pair<int, int>(first, second));
	}

	std::vector<bool> discontinuities;

	int lineNo, endLineNo, boundNo,
			color,//argb
		//	winColor,//abgr
			nx, nZ, nQ,
			nISD;//3 space dimentions max
	bool nITD;//1 time dimention max
	char resultMathSet;//'R' real, 'c' complex, 'h' quaternion	larger value = superset	//'C'==67, 'H'==72, ['R'==82, 'c'==99, 'h'==104], 'r'==114
	int resultTerm;

	std::vector<Map> m;
	std::vector<Variable> variables;
	std::vector<Term> n;
	std::vector<Value> data;
	std::vector<Instruction> i;
	int resultLogicType;//0: not bool, 1: && ## || < <= > >= logic/inequality, 2: = equation, 3: != anti-equation
	int lastInstruction;
	std::vector<int> rmode;

	//user function
	bool valid;
	int name_id;//unique name_id > M_USER_FUNCTION_START
	int nArgs;//, lineNo;
	bool functionStuck;//true: user function returns nan without evaluation when markFunctionsStuck=true

	Expression():color(0), nx(0), nZ(0), nQ(0), nISD(0), nITD(false), rmode(1, 0),
				 valid(true), nArgs(0), resultTerm(0){}

	void free()
	{
		syntaxErrors.clear();
		discontinuities.clear();
		lineNo=0, endLineNo=0, boundNo=0, color=0, nx=0, nZ=0, nQ=0, nISD=0, nITD=0;
		resultMathSet=0;
		m.clear();
		variables.clear();
//		if(n.size()&&n[0].r.size())//
//		{
//			AVector_v2d test=n[0].r;
//			_aligned_free(n[0].r.p), n[0].r.p=0;
//			n[0].r=test;
//		}
		//if(n.size()&&n[0].i.size())//
		//{
		//	AVector_v2d test=n[0].i;
		//	_aligned_free(n[0].i.p), n[0].i.p=0;
		//	n[0].i=test;
		//}
		n.clear();
		data.clear();
		i.clear();
		resultLogicType=0, lastInstruction=0;
		rmode.assign(1, 0);
		valid=false;
		name_id=0;
		nArgs=0;
		functionStuck=0;
	}
	void insertMap(int pos, int len, G2::Map _0, int _1=0){m.push_back(Map(pos, len, _0, _1));}
//	void insertMap(G2::Map _0, int _1=0){m.push_back(Map(_0, _1));}
	void insertMapData(int pos, int len, char mathSet, double r=0, double i=0, double j=0, double k=0)
	{
		m.push_back(Map(pos, len, G2::M_N, data.size()));
		n.push_back(Term(mathSet)), data.push_back(Value(r, i, j, k));
	}
//	void insertMapData(char mathSet, double r=0, double i=0, double j=0, double k=0)
//	{
//		m.push_back(Map(G2::M_N, data.size()));
//		n.push_back(Term(mathSet)), data.push_back(Value(r, i, j, k));
//	}
//	void insertData(char mathSet, Value &x){n.push_back(Term(mathSet)), data.push_back(x);}//ambiguous
	void insertData(char mathSet, Value x=Value()){n.push_back(Term(mathSet)), data.push_back(x);}

//	void insertData(char mathSet, double const &x){n.push_back(Term(mathSet)), data.push_back(Value(x));}
//	void insertData(char mathSet, Comp1d const &c){n.push_back(Term(mathSet)), data.push_back(Value(c.r, c.i));}

	void insertRVar(int, int, unsigned short const*, int);
	void insertCVar(int, int, unsigned short const*);
	void insertHVar(int, int, unsigned short const*);
//	void insertRVar(int, int, char const*, int);
//	void insertCVar(int, int, char const*);
//	void insertHVar(int, int, char const*);
	void setColor_random()
	{
		auto c=(unsigned char*)&color;
		c[0]=rand();//r
		c[1]=rand();//g
		c[2]=rand();//b
		c[3]=0xFF;	//a
		//auto c=(unsigned char*)&color, wc=(unsigned char*)&winColor;
		//c[0]=wc[2]=rand();
		//c[1]=wc[1]=rand();
		//c[2]=wc[0]=rand();
		//c[3]=wc[3]=0;
	}
	void setColor_black()
	{
		color=0xFF000000;
	//	color=winColor=0;
	}

	//user function
	void insertFVar(char mathSet){n.push_back(Term(mathSet, false)), data.push_back(Value());}
};
#endif //GRAPHER_2_G2_COMMON_H
