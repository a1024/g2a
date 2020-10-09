//g2_sw.cpp - Implementation of software versions of G2 functions.
//Copyright (C) 2020  Ayman Wagih Mohsen
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
// Created by MSI on 10/9/2020.
//

#include	"g2_common.h"
#include	"g2_sw.h"
namespace	G2
{
#if 0
	char returns_rcq(char optype){return optype;}
	char returns_rrr(char optype){return 'R';}
	char returns_ccc(char optype){return 'c';}
	char returns_ccq(char optype)
	{
		switch(optype)
		{
		case 'R':return 'c';
		case 'c':return 'c';
		case 'h':return 'h';
		}
		return 0;
	}
	char returns_rXX(char optype)
	{
		switch(optype)
		{
		case 'R':return 'R';


		}
		return 0;
	}
	char returns_cXX(char optype)
	{
		switch(optype)
		{
		case 'R':return 'c';


		}
		return 0;
	}
	char returns_ccq_ccq_qqq(char op1type, char op2type)
	{
		switch(op1type)
		{
		case 'R':switch(op2type){case 'R':return 'c';	case 'c':return 'c';	case 'h':return 'h';}break;
		case 'c':switch(op2type){case 'R':return 'c';	case 'c':return 'c';	case 'h':return 'h';}break;
		case 'h':switch(op2type){case 'R':return 'h';	case 'c':return 'h';	case 'h':return 'h';}break;
		}
		return 0;
	}
	char returns_rrr_rrr_rrr(char op1type, char op2type){return 'R';}
	char returns_ccX_ccX_qXX(char op1type, char op2type)
	{
		switch(op1type)
		{
		case 'R':switch(op2type){case 'R':return 'c';	case 'c':return 'c';						}break;
		case 'c':switch(op2type){case 'R':return 'c';	case 'c':return 'c';						}break;
		case 'h':switch(op2type){case 'R':return 'h';												}break;
		}
		return 0;
	}
	char returns_rXX_rXX_XXX(char op1type, char op2type)
	{
		switch(op1type)
		{
		case 'R':switch(op2type){case 'R':return 'R';												}break;
		case 'c':switch(op2type){case 'R':return 'R';												}break;
		case 'h':																					break;
		}
		return 0;
	}
	char returns_rXX_XXX_XXX(char op1type, char op2type)
	{
		switch(op1type)
		{
		case 'R':switch(op2type){case 'R':return 'R';												}break;
		case 'c':																					break;
		case 'h':																					break;
		}
		return 0;
	}
	char returns_cXX_XXX_XXX(char op1type, char op2type)
	{
		switch(op1type)
		{
		case 'R':switch(op2type){case 'R':return 'c';												}break;
		case 'c':																					break;
		case 'h':																					break;
		}
		return 0;
	}
	char returns_rcq_ccq_qqq(char op1type, char op2type){return maximum((int)op1type, (int)op2type);}
	char returns_rrr_ccc_qqq(char op1type, char op2type){return op1type;}
	char returns_rcq_rcq_rcq(char op1type, char op2type){return op2type;}
	char returns_rXX_cXX_qXX(char op1type, char op2type){return op2type;}
	char returns_conditional(char op1type, char op2type, char op3type){return maximum((int)op2type, (int)op3type);}//
#endif
	bool _2d_between(double x1, double y1, double x, double y, double x2, double y2)
	{
		if(x1==x2)
		{
			if(x==x1)
			{
				if(y1==y2)
					return x==x1&&y==y1;
				else
					return y1<y2?y1<=y&&y<=y2:y2<=y&&y<=y1;
			}
		}
		else
		{
			if(y1==y2)
			{
				if(y==y1)
					return x1<x2?x1<=x&&x<=x2:x2<=x&&x<=x1;
			}
			else if(x==x1)
				return y==y1;
			else if(x==x2)
				return y==y2;
			else if(y==y1)
				return x==x1;
			else if(y==y2)
				return x==x2;
			else if((y2-y1)*(x-x1)==(x2-x1)*(y-y1))
			{
				if(x1<x2)
				{
					if(x1<=x&&x<=x2)
						return y1<y2?y1<=y&&y<=y2:y2<=y&&y<=y1;
				}
				else
				{
					if(x2<=x&&x<=x1)
						return y1<y2?y1<=y&&y<=y2:y2<=y&&y<=y1;
				}
			}
		}
		return false;
	}
	bool _1d_int_in_range(double x0, double x1){return std::floor(x0)!=std::floor(x1)||std::ceil(x0)!=std::ceil(x1);}
	double _1d_zero_crossing(double x0, double y0, double x1, double y1)
	{
		return x0+(0-y0)*(x1-x0)/(y1-y0);
	}
	const double ll_max=9.22337203685478e+018;
	inline long long convert_d2ll(double const x)
	{
		return x!=x||x<-ll_max||x>ll_max?(long long&)x:(long long)x;
	}
	inline long long convert_d2ll_zero(double const x)
	{
		return x!=x||x<-ll_max||x>ll_max?0:(long long)x;
	}
	inline Comp1d sq(Comp1d const& x)
	{
		return Comp1d(x.r*x.r-x.i*x.i, 2*x.r*x.i);
	}
	inline Quat1d sq(Quat1d const& x)
	{
		auto _2r=2*x.r;
		return Quat1d(x.r*x.r-x.i*x.i-x.j*x.j-x.k*x.k, x.i*_2r, x.j*_2r, x.k*_2r);
	}
	const Comp1d _i(0, 1), _i_2(0, 0.5), _2_i(0, 2);
	inline Quat1d sgnu(Quat1d const& x)
	{
		return Quat1d(0, x.i, x.j, x.k);
	}
	inline Quat1d acosh(Quat1d const& x)
	{
		return log(x+sqrt(x*x-1));
	}
	inline Comp1d asinh(Comp1d const& x)
	{
		return log(x+sqrt(x*x+1));
	}
	inline Quat1d asinh(Quat1d const& x)
	{
		return log(x+sqrt(x*x+1));
	}
	inline double sinhc(double const& x)
	{
		return ::sinh(x)/x;
	}
	inline Comp1d cos(Comp1d const& x)
	{
		return (exp(_i*x)+exp(-_i*x))*0.5;
	}
	inline Quat1d cos(Quat1d const& x)//boost::math
	{
		double z=sqrt(x.i*x.i+x.j*x.j+x.k*x.k);
		double sin_xr, cos_xr;
		sincos(x.r, &sin_xr, &cos_xr);
		double w=-sin_xr*sinhc(z);
		return Quat1d(cos_xr*::cosh(z), w*x.i, w*x.j, w*x.k);
	}
	inline Comp1d sin(Comp1d const& x)
	{
		return (exp(_i*x)-exp(-_i*x))*-_i_2;
	}
	inline Quat1d sin(Quat1d const& x)//boost::math
	{
		double z=sqrt(x.i*x.i+x.j*x.j+x.k*x.k);
		double sin_xr, cos_xr;
		sincos(x.r, &sin_xr, &cos_xr);
		double w=-cos_xr*sinhc(z);
		return Quat1d(sin_xr*::cosh(z), w*x.i, w*x.j, w*x.k);
	}
	inline Comp1d acos(Comp1d const& x)
	{
		return -_i*log(x+sqrt(x*x-1));
	}
	inline Quat1d acos(Quat1d const& x)
	{
		return -sgnu(x)*acosh(x);
	}
	inline Comp1d cosh(Comp1d const& x)
	{
		return (exp(x)+exp(-x))*0.5;
	}
	inline Quat1d cosh(Quat1d const& x)
	{
		return (exp(x)+exp(-x))*0.5;
	}
	inline Comp1d acosh(Comp1d const& x)
	{
		return log(x+sqrt(x*x-1));
	}
	inline Comp1d asin(Comp1d const& x)
	{
		return -_i*log(_i*x+sqrt(1-x*x));
	}
	inline Quat1d asin(Quat1d const& x)
	{
		Quat1d t=sgnu(x);
		return -t*asinh(x*t);
	}
	inline Comp1d sinh(Comp1d const& x)
	{
		return (exp(x)-exp(-x))*0.5;
	}
	inline Quat1d sinh(Quat1d const& x)
	{
		return (exp(x)-exp(-x))*0.5;
	}
	inline Comp1d tan(Comp1d const& x)
	{
		Comp1d exp_2ix=exp(_2_i*x);
		return (exp_2ix-1)/((exp_2ix+1)*_i);
	}
	inline Quat1d tan(Quat1d const& x)
	{
		Quat1d exp_2ix=exp(_2_i*x);
		return (exp_2ix-1)/((exp_2ix+1)*_i);
	}
	inline Comp1d atan(Comp1d const& x)
	{
		return (_i*0.5)*log((_i+x)/(_i-x));
	}
	inline Quat1d atan(Quat1d const& x)
	{
		return (_i*0.5)*log((_i+x)/(_i-x));
	}
	const long long sign_mask_complement=0x8000000000000000LL;
	inline Comp1d tanh(Comp1d const& x)
	{
		Comp1d e2x=exp(2*x);
		return (e2x-1)/(e2x+1);
	}
	inline Quat1d tanh(Quat1d const& x)
	{
		Quat1d e2x=exp(2*x);
		return (e2x-1)/(e2x+1);
	}
	inline Comp1d atanh(Comp1d const& x)
	{
		return 0.5*log((1+x)/(1-x));
	}
	inline Quat1d atanh(Quat1d const& x)
	{
		return 0.5*log((1+x)/(1-x));
	}
	//inline double pow(double const &x, double const &y){return ::exp(y*::log(x));}
	const double _ln_phi=::log(_phi);
	inline double cyl_bessel_j(double x, double y)
	{
		try
		{
			return jn((int)x, y);
		}catch(...)
		{
			return _qnan;
		}
	}
	inline Comp1d r_hankel1(double x, double y)
	{
		try
		{
			return jn((int)x, y)+_i*yn((int)x, y);
		}catch(std::domain_error&)
		{
			return _qnan;
		}catch(std::overflow_error&)
		{
			return -_HUGE;
		}catch(...)
		{
			return _qnan;
		}
	}
	inline double sgn(double const& x)
	{
		return (x>0)-(x<0);
	}
	inline Comp1d sgn(Comp1d const& x)
	{
		double mag=x.abs();
		return mag?x/mag:0;
	}
	inline Quat1d sgn(Quat1d const& x)
	{
		double mag=x.abs();
		return mag?x/mag:0;
	}
	inline double step(double const& x){return 0.5+0.5*sgn(x);}
	inline Comp1d step(Comp1d const& x){return 0.5+0.5*sgn(x);}
	inline Quat1d step(Quat1d const& x){return 0.5+0.5*sgn(x);}

	double r_r_ceil(double const& x){return ceil(x);}
	Comp1d c_c_ceil(Comp1d const& x){return x.ceil();}
	Quat1d q_q_ceil(Quat1d const& x){return x.ceil();}
	bool disc_r_ceil_o(Value const& o0, Value const& o1)
	{
		return o0.r!=o1.r;
	}
	bool disc_c_ceil_o(Value const& o0, Value const& o1)
	{
		return o0.r!=o1.r||o0.i!=o1.i;
	}
	bool disc_q_ceil_o(Value const& o0, Value const& o1)
	{
		return o0.r!=o1.r||o0.i!=o1.i||o0.j!=o1.j||o0.k!=o1.k;
	}

	double r_r_floor(double const& x){return floor(x);}
	Comp1d c_c_floor(Comp1d const& x){return x.floor();}
	Quat1d q_q_floor(Quat1d const& x){return x.floor();}
	auto disc_r_floor_o=disc_r_ceil_o;
	auto disc_c_floor_o=disc_c_ceil_o;
	auto disc_q_floor_o=disc_q_ceil_o;

	double r_r_round(double const& x){return round(x);}
	Comp1d c_c_round(Comp1d const& x){return x.round();}
	Quat1d q_q_round(Quat1d const& x){return x.round();}
	auto disc_r_round_o=disc_r_ceil_o;
	auto disc_c_round_o=disc_c_ceil_o;
	auto disc_q_round_o=disc_q_ceil_o;

	double  r_r_int					(double const &x)					{return (int)(double)x;}
	Comp1d  c_c_int					(Comp1d const &x)					{return Comp1d((int)x.r, (int)x.i);}
	Quat1d  q_q_int					(Quat1d const &x)					{return Quat1d((int)x.r, (int)x.i, (int)x.j, (int)x.k);}
	auto  disc_r_int_o				=disc_r_ceil_o;
	auto  disc_c_int_o				=disc_c_ceil_o;
	auto  disc_q_int_o				=disc_q_ceil_o;

	double  r_r_frac				(double const &x)					{return x-std::floor((double)x);}
	Comp1d  c_c_frac				(Comp1d const &x)					{return Comp1d(x.r-std::floor(x.r), x.i-std::floor(x.i));}
	Quat1d  q_q_frac				(Quat1d const &x)					{return Quat1d(x.r-std::floor(x.r), x.i-std::floor(x.i), x.j-std::floor(x.j), x.k-std::floor(x.k));}
	bool  disc_r_frac_i				(Value const &o0, Value const &o1){return std::floor(o0.r)!=std::floor(o1.r);}
	bool  disc_c_frac_i				(Value const &o0, Value const &o1){return std::floor(o0.r)!=std::floor(o1.r)||std::floor(o0.i)!=std::floor(o1.i);}
	bool  disc_q_frac_i				(Value const &o0, Value const &o1){return std::floor(o0.r)!=std::floor(o1.r)||std::floor(o0.i)!=std::floor(o1.i)||std::floor(o0.j)!=std::floor(o1.j)||std::floor(o0.k)!=std::floor(o1.k);}

	double r_r_abs(double const& x){return abs(x);}
	double r_c_abs(Comp1d const& x){return x.abs();}
	double r_q_abs(Quat1d const& x){return x.abs();}

	double r_r_arg(double const& x)
	{
		if(x<0)
			return _pi;
		if(x>0)
			return 0;
		return _qnan;
		//return _pi&(x>0)|_qnan&(x==0);
	}
	double r_c_arg(Comp1d const& x)
	{
		if(x.r==0&&x.i==0)
			return _qnan;
		return ::atan2(x.i, x.r);
	}
	double r_q_arg(Quat1d const& x)
	{
		return ::acos(x.r/x.abs());
	}
	bool disc_r_arg_i(Value const& x0, Value const& x1)
	{
		return x0.r<0?x1.r>=0:x0.r>0?x1.r<=0:x1.r!=0;
	}//return std::signbit(x0.r)!=std::signbit(x1.r);
	bool disc_c_arg_i(Value const& x0, Value const& x1)
	{
		if(x0.r<0)
		{
			if(x0.i<0)
			{
				if(x1.r<0)
					return x1.i>=0;
				if(x1.r>0)
					return x1.i>0&&x0.r+(0-x0.i)*(x1.r-x0.r)/(x1.i-x0.i)<=0;
				return x1.i>=0;
			}else if(x0.i>0)
			{
				if(x1.r<0)
					return x1.i<0;
				if(x1.r>0)
					return x1.i<0&&x0.r+(0-x0.r)*(x1.r-x0.r)/(x1.i-x1.i)<=0;
				return x1.i<=0;
			}else
			{
				if(x1.r<0)
					return x1.i<0;
				if(x1.r>0)
					return x1.i<=0;
				return x1.i<=0;
			}
		}else if(x0.r>0)
		{
			if(x0.i<0)
			{
				if(x1.r<0)
					return x1.i>=0&&(x1.i==0||x0.r+(0-x0.r)*(x1.r-x0.r)/(x1.i-x1.i)<=0);
				if(x1.r>0)
					return false;
				return x1.i==0;
			}else if(x0.i>0)
			{
				if(x1.r<0)
					return x1.i<0&&x0.r+(0-x0.r)*(x1.r-x0.r)/(x1.i-x1.i)<=0;
				if(x1.r>0)
					return false;
				return x1.i==0;
			}else
			{
				if(x1.r<0)
					return x1.i==0;
				if(x1.r>0)
					return false;
				return x1.i==0;
			}
		}else
		{
			if(x0.i<0)
			{
				if(x1.r<0)
					return x1.i>=0;
				if(x1.r>0)
					return false;
				return x1.i>=0;
			}else if(x0.i>0)
			{
				if(x1.r<0)
					return x1.i<0;
				if(x1.r>0)
					return false;
				return x1.i<=0;
			}
			return true;
		}
	}
	bool disc_q_arg_i(Value const& x0, Value const& x1)
	{
		return false;
	}//

	double r_c_real(Comp1d const& x){return x.r;}
	double r_c_imag(Comp1d const& x){return x.i;}

	//r_r_conjugate: assign
	Comp1d c_c_conjugate(Comp1d const& x){return Comp1d(x.r, -x.i);}
	Quat1d q_q_conjugate(Quat1d const& x){return Quat1d(x.r, -x.i, -x.j, -x.k);}

	Comp1d c_r_polar(double const& x){return Comp1d(abs(x), x<0?_pi:x>0?0:_qnan);}
	Comp1d c_c_polar(Comp1d const& x)
	{
		double mag=x.abs();
		return Comp1d(mag, mag?::atan2(x.i, x.r):_qnan);
	}
	Comp1d c_q_polar(Quat1d const& x)
	{
		double mag=x.abs();
		return Comp1d(mag, ::acos(x.r/mag));
	}
	auto disc_r_polar_i=disc_r_arg_i;
	auto disc_c_polar_i=disc_c_arg_i;
	auto disc_q_polar_i=disc_q_arg_i;

	//c_r_cartesian	assign
	Comp1d c_c_cartesian(Comp1d const& x)
	{
		double sin_i, cos_i;
		sincos(x.i, &sin_i, &cos_i);
		return Comp1d(x.r*cos_i, x.r*sin_i);
	}
	Quat1d q_q_cartesian(Quat1d const& x)
	{
		double sin_i, cos_i, sin_j, cos_j, sin_k, cos_k;
		sincos(x.i, &sin_i, &cos_i);
		sincos(x.j, &sin_j, &cos_j);
		sincos(x.k, &sin_k, &cos_k);
		return Quat1d(x.r*cos_i*cos_j*cos_k, x.r*sin_i*cos_j*cos_k, x.r*sin_j*cos_k, x.r*sin_k);
	}

	double r_rr_plus(double const& x, double const& y){return x+y;}
	Comp1d c_rc_plus(double const& x, Comp1d const& y){return x+y;}
	Quat1d q_rq_plus(double const& x, Quat1d const& y){return x+y;}
	Comp1d c_cr_plus(Comp1d const& x, double const& y){return x+y;}
	Comp1d c_cc_plus(Comp1d const& x, Comp1d const& y){return x+y;}
	Quat1d q_cq_plus(Comp1d const& x, Quat1d const& y){return x+y;}
	Quat1d q_qr_plus(Quat1d const& x, double const& y){return x+y;}
	Quat1d q_qc_plus(Quat1d const& x, Comp1d const& y){return x+y;}
	Quat1d q_qq_plus(Quat1d const& x, Quat1d const& y){return x+y;}

	double r_r_minus(double const& x){return -x;}
	Comp1d c_c_minus(Comp1d const& x){return -x;}
	Quat1d q_q_minus(Quat1d const& x){return -x;}
	double r_rr_minus(double const& x, double const& y){return x-y;}
	Comp1d c_rc_minus(double const& x, Comp1d const& y){return x-y;}
	Quat1d q_rq_minus(double const& x, Quat1d const& y){return x-y;}
	Comp1d c_cr_minus(Comp1d const& x, double const& y){return x-y;}
	Comp1d c_cc_minus(Comp1d const& x, Comp1d const& y){return x-y;}
	Quat1d q_cq_minus(Comp1d const& x, Quat1d const& y){return x-y;}
	Quat1d q_qr_minus(Quat1d const& x, double const& y){return x-y;}
	Quat1d q_qc_minus(Quat1d const& x, Comp1d const& y){return x-y;}
	Quat1d q_qq_minus(Quat1d const& x, Quat1d const& y){return x-y;}

	double r_rr_multiply(double const& x, double const& y){return x*y;}
	Comp1d c_rc_multiply(double const& x, Comp1d const& y){return x*y;}
	Quat1d q_rq_multiply(double const& x, Quat1d const& y){return x*y;}
	Comp1d c_cr_multiply(Comp1d const& x, double const& y){return x*y;}
	Comp1d c_cc_multiply(Comp1d const& x, Comp1d const& y){return x*y;}//(xr+i*xi)(yr+i*yi) = xr*yr-xi*yi+i(xr*yi+xi*yr)
	Quat1d q_cq_multiply(Comp1d const& x, Quat1d const& y){return x*y;}
	Quat1d q_qr_multiply(Quat1d const& x, double const& y){return x*y;}
	Quat1d q_qc_multiply(Quat1d const& x, Comp1d const& y){return x*y;}
	Quat1d q_qq_multiply(Quat1d const& x, Comp1d const& y){return x*y;}

	double r_r_divide(double const& y){return 1/y;}
	Comp1d c_c_divide(Comp1d const& y){return 1/y;}
	Quat1d q_q_divide(Quat1d const& y){return 1/y;}
	double r_rr_divide(double const& x, double const& y){return x/y;}
	Comp1d c_rc_divide(double const& x, Comp1d const& y){return x/y;}
	Quat1d q_rq_divide(double const& x, Quat1d const& y){return x/y;}
	Comp1d c_cr_divide(Comp1d const& x, double const& y){return x/y;}
	Comp1d c_cc_divide(Comp1d const& x, Comp1d const& y){return x/y;}
	Quat1d q_cq_divide(Comp1d const& x, Quat1d const& y){return x/y;}
	Quat1d q_qr_divide(Quat1d const& x, double const& y){return x/y;}
	Quat1d q_qc_divide(Quat1d const& x, Comp1d const& y){return x/y;}
	Quat1d q_qq_divide(Quat1d const& x, Quat1d const& y){return x/y;}
	auto disc_r_divide_i=disc_r_arg_i;
	bool disc_c_divide_i(Value const& x0, Value const& x1)
	{
		double const pole[2]={0, 0};
		if(x0.r<0)
		{
			if(x0.i<0)
			{
				if(x1.r<0)
					return false;
				if(x1.r>0)
					return _2d_between(x0.r, x0.i, pole[0], pole[1], x1.r, x1.i);
				return x1.i==0;
			}
			if(x0.i>0)
			{
				if(x1.r<0)
					return false;
				if(x1.r>0)
					return _2d_between(x0.r, x0.i, pole[0], pole[1], x1.r, x1.i);
				return x1.i==0;
			}
			{
				if(x1.r<0)
					return false;
				if(x1.r>0)
					return x1.i==0;
				return true;
			}
		}
		if(x0.r>0)
		{
			if(x0.i<0)
			{
				if(x1.r<0)
					return _2d_between(x0.r, x0.i, pole[0], pole[1], x1.r, x1.i);
				if(x1.r>0)
					return false;
				return x1.i==0;
			}
			if(x0.i>0)
			{
				if(x1.r<0)
					return _2d_between(x0.r, x0.i, pole[0], pole[1], x1.r, x1.i);
				if(x1.r>0)
					return false;
				return x1.i==0;
			}
			{
				if(x1.r<0)
					return x1.i==0;
				if(x1.r>0)
					return false;
				return true;
			}
		}
		{
			if(x0.i<0)
			{
				if(x1.r<0)
					return false;
				if(x1.r>0)
					return false;
				return x1.i>=0;
			}
			if(x0.i>0)
			{
				if(x1.r<0)
					return false;
				if(x1.r>0)
					return false;
				return x1.i<=0;
			}
			{
				return true;
			}
		}
	}
	bool disc_q_divide_i(Value const& x0, Value const& x1)
	{
		return false;
	}//
	bool disc_rr_divide_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	{
		return disc_r_divide_i(y0, y1);
	}
	bool disc_rc_divide_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	{
		return disc_c_divide_i(y0, y1);
	}
	bool disc_rq_divide_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	{
		return disc_q_divide_i(y0, y1);
	}
	bool disc_cr_divide_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	{
		return disc_r_divide_i(y0, y1);
	}
	bool disc_cc_divide_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	{
		return disc_c_divide_i(y0, y1);
	}
	bool disc_cq_divide_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	{
		return disc_q_divide_i(y0, y1);
	}
	bool disc_qr_divide_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	{
		return disc_r_divide_i(y0, y1);
	}
	bool disc_qc_divide_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	{
		return disc_c_divide_i(y0, y1);
	}
	bool disc_qq_divide_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	{
		return disc_q_divide_i(y0, y1);
	}

	double r_rr_logic_divides(double const& y, double const& x)
	{
		auto r=x/y;
		return r==floor(r);
	}//rc_divides, rq_divides: applied to each component
	double r_rc_logic_divides(double const& y, Comp1d const& x)
	{
		auto r=x/y;
		return r==r.floor();
	}
	double r_rq_logic_divides(double const& y, Quat1d const& x)
	{
		auto r=x/y;
		return r==r.floor();
	}
	double r_cr_logic_divides(Comp1d const& y, double const& x)
	{
		auto r=x/y;
		return (r.r==floor(r.r))&(r.i==floor(r.i));
	}
	double r_cc_logic_divides(Comp1d const& y, Comp1d const& x)
	{
		auto r=x/y;
		return (r.r==floor(r.r))&(r.i==floor(r.i));
	}
	double r_cq_logic_divides(Comp1d const& y, Quat1d const& x)
	{
		auto r=x/y;
		return (r.r==floor(r.r))&(r.i==floor(r.i))&(r.j==floor(r.j))&(r.k==floor(r.k));
	}
	double r_qr_logic_divides(Quat1d const& y, double const& x)
	{
		auto r=x/y;
		return (r.r==floor(r.r))&(r.i==floor(r.i))&(r.j==floor(r.j))&(r.k==floor(r.k));
	}
	double r_qc_logic_divides(Quat1d const& y, Comp1d const& x)
	{
		auto r=x/y;
		return (r.r==floor(r.r))&(r.i==floor(r.i))&(r.j==floor(r.j))&(r.k==floor(r.k));
	}
	double r_qq_logic_divides(Quat1d const& y, Quat1d const& x)
	{
		auto r=x/y;
		return (r.r==floor(r.r))&(r.i==floor(r.i))&(r.j==floor(r.j))&(r.k==floor(r.k));
	}
	bool disc_r_logic_divides_o		(Value const &x0, Value const &x1){return x0.r!=x1.r;}//for all logic functions

	inline double power_real(double const &x, int y)
	{
		double m[]={1, 0}, r=1;
		if(y<0)
			m[1]=1/x, y=-y;
		else
			m[1]=x;
		for(;;)
		{
			r*=m[y&1], y>>=1;
			if(!y)
				return r;
			m[1]*=m[1];
		}
		return r;
	}
	inline Quat1d power_real(Quat1d const &x, int y)
	{
		Quat1d t=x, r(1.);
		int p=abs(y);
		for(;;)
		{
			if(p&1)
				r*=t;
			p>>=1;
			if(!p)
				return y<0?inv(r):r;
			t*=t;
		}
		return y<0?inv(r):r;
	}
	double r_rr_power_real			(double const &x, double const &y)	{return power_real((double)x, (int)std::floor((double)y));}//long long y
	Comp1d c_cr_power_real			(Comp1d const &x, double const &y)	{return std::pow(std::complex<double>(x.r, x.i), (int)std::floor((double)y));}
//	Comp1d c_cr_power_real			(Comp1d const &x, double const &y)	{auto result=std::pow(std::complex<double>(x.r, x.i), (int)std::floor((double)y)); return Comp1d(result.real(), result.imag());}
	Quat1d q_qr_power_real			(Quat1d const &x, double const &y)	{return power_real((Quat1d)x, (int)std::floor((double)y));}//boost::math::pow is recursive
	bool disc_rr_power_real_i		(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return std::floor(y0.r)!=std::floor(y1.r);}//truncation vs floor
	auto disc_cr_power_real_i		=disc_rr_power_real_i;
	auto disc_qr_power_real_i		=disc_rr_power_real_i;

	Comp1d c_cr_pow(Comp1d const& x, double const& y){return x^y;}
	Comp1d c_cc_pow(Comp1d const& x, Comp1d const& y){return x^y;}
	Quat1d q_cq_pow(Comp1d const& x, Quat1d const& y){return x^y;}
	Quat1d q_qr_pow(Quat1d const& x, double const& y){return x^y;}
	Quat1d q_qc_pow(Quat1d const& x, Comp1d const& y){return x^y;}
	Quat1d q_qq_pow(Quat1d const& x, Quat1d const& y){return x^y;}
	bool disc_cr_pow_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	{//pole: y<0: x.r<=0, x.i=0;	y>0: x.r<0, x.i=0
		if(y0.r==y1.r)
		{
		}else
		{
		}
		//	if(x0.i==0&&x1.i==0&&std::signbit(x0.r)!=std::signbit(x1.r))
		//	{
		//	}
		return false;
	}
	bool disc_cc_pow_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	{
		return false;
	}//
	bool disc_cq_pow_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	{
		return false;
	}//
	bool disc_qr_pow_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	{
		return false;
	}//
	bool disc_qc_pow_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	{
		return false;
	}//
	bool disc_qq_pow_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	{
		return false;
	}//

	Comp1d c_c_ln(Comp1d const& x){return log(x);}
	Quat1d q_q_ln(Quat1d const& x){return log(x);}
	auto disc_c_ln_i=disc_c_arg_i;
	bool disc_q_ln_i(Value const& x0, Value const& x1)
	{
		return false;
	}//

	Comp1d c_c_log(Comp1d const& x){return log(x)/_ln10;}
	Quat1d q_q_log(Quat1d const& x){return log(x)/_ln10;}
	Comp1d c_cr_log(Comp1d const& x, double const& y){return log(x)/log(Comp1d(y, 0));}
	Comp1d c_cc_log(Comp1d const& x, Comp1d const& y){return log(x)/log(y);}
	Quat1d q_cq_log(Quat1d const& y, Comp1d const& x){return log(x)/log(y);}
	Quat1d q_qc_log(Quat1d const& x, Comp1d const& y){return log(x)/log(y);}
	Quat1d q_qq_log(Quat1d const& x, Quat1d const& y){return log(x)/log(y);}
	auto disc_c_log_i=disc_c_arg_i;
	bool disc_q_log_i(Value const& x0, Value const& x1)
	{
		return false;
	}//
	bool disc_cr_log_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	{
		return false;
	}//
	bool disc_cc_log_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	{
		return false;
	}//
	bool disc_cq_log_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	{
		return false;
	}//
	bool disc_qc_log_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	{
		return false;
	}//
	bool disc_qq_log_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	{
		return false;
	}//

	typedef std::complex<double> Complex1d;
	//typedef boost::math::quaternion<double> Quaternion1d;
	Complex1d tetrate(double x, double y)
	{
		double log_x=std::log((double)x), ry=y;
		int h=int(::floor(ry-.5)+1);//rounded real part
		Complex1d t(ry-(double)h);//real-round(real)+i*imag
		{
			auto q=std::sqrt(log_x);
			t=(t+1.)*((log_x>_ln2?(q*(0.137467+q*(4.94969+q*0.0474179))/(1.+q*(3.23171+q*0.471222))+
					(-.5+_ln2)/log_x-1.)+t*
					((q*(-0.424278+q*(1.75166+q*(-1.46524+q*0.93347)))/(0.0312142+q*(-0.267478+q))+
							(-.625+_ln2)/log_x-1.)+t*
							((q*(3.39255+q*(16.1046+q*(-19.5216+q*10.7458)))/
									(1.+q*(4.1274+q*5.25449))+(-2./3+_ln2)/log_x-1.)+t*
									(0.16*q*(1.+q*(27.7934+q*(358.688+q*(-259.233+log_x*61.6566))))/
											(1.-8.1192*q+37.087*log_x)+(-131./192.+_ln2)/log_x-
											1.))):(-1.0018+
					(0.15128484821526975*(1.+33.04715298851381*q-3.51771875598067*log_x)*q)/
							(1.+3.2255053261256337*q)+(-0.5+_ln2)/log_x)+t*
					((1.1-2.608785958462561*(1.-0.6663562294911147*q)*q-(-0.625+_ln2)/log_x)+t*
							((-0.96+3.0912038297987596*(1.+0.6021398048785328*log_x)*q/
									(1.+4.240467556480155*log_x)+(-2./3+_ln2)/log_x)+t*(1.2-
									10.44604984418533*
											(1.+0.2137568928431227*q+0.3693275254470449*log_x)*q/
											(1.+4.95715636660691*q+7.70233216637738*log_x)-
									(-131./192.+_ln2)/log_x))))*t+1.)+std::log(t+2.)/log_x-
					_ln2/log_x*(1.+t);
		}
		for(;h>0;--h)
			t=std::exp(log_x*t);
		for(;h<0;++h)
			t=std::log(t)/log_x;
		return t;
	}
	Complex1d tetrate(Complex1d& x, double y)
	{
		Complex1d log_x=std::log((Complex1d)x);
		double ry=y;
		int h=int(::floor(ry-.5)+1);//rounded real part
		Complex1d t(ry-(double)h);//real-round(real)+i*imag
		{
			auto q=std::sqrt(log_x);
			t=(t+1.)*((log_x.real()>_ln2?
					(q*(0.137467+q*(4.94969+q*0.0474179))/(1.+q*(3.23171+q*0.471222))+
							(-.5+_ln2)/log_x-1.)+t*
							((q*(-0.424278+q*(1.75166+q*(-1.46524+q*0.93347)))/
									(0.0312142+q*(-0.267478+q))+(-.625+_ln2)/log_x-1.)+t*
									((q*(3.39255+q*(16.1046+q*(-19.5216+q*10.7458)))/
											(1.+q*(4.1274+q*5.25449))+(-2./3+_ln2)/log_x-1.)+t*
											(0.16*q*(1.+q*(27.7934+
													q*(358.688+q*(-259.233+log_x*61.6566))))/
													(1.-8.1192*q+37.087*log_x)+
													(-131./192.+_ln2)/log_x-1.))):(-1.0018+
							(0.15128484821526975*(1.+33.04715298851381*q-3.51771875598067*log_x)*q)/
									(1.+3.2255053261256337*q)+(-0.5+_ln2)/log_x)+t*
							((1.1-2.608785958462561*(1.-0.6663562294911147*q)*q-
									(-0.625+_ln2)/log_x)+t*((-0.96+
									3.0912038297987596*(1.+0.6021398048785328*log_x)*q/
											(1.+4.240467556480155*log_x)+(-2./3+_ln2)/log_x)+t*(1.2-
									10.44604984418533*
											(1.+0.2137568928431227*q+0.3693275254470449*log_x)*q/
											(1.+4.95715636660691*q+7.70233216637738*log_x)-
									(-131./192.+_ln2)/log_x))))*t+1.)+std::log(t+2.)/log_x-
					_ln2/log_x*(1.+t);
		}
		for(;h>0;--h)
			t=std::exp(log_x*t);
		for(;h<0;++h)
			t=std::log(t)/log_x;
		return t;
	}
	//	Quat1d tetrate_qr(Quat1d &x, double y)
	//	{
	//	}
	Complex1d tetrate(double x, Complex1d const& y)
	{
		double log_x=std::log((double)x);
		std::complex<double> cy=y;
		int h=int(::floor(cy.real()-.5)+1);//rounded real part
		std::complex<double> t(cy-(double)h);//real-round(real)+i*imag
		{
			auto q=std::sqrt(log_x);
			t=(t+1.)*((log_x>_ln2?(q*(0.137467+q*(4.94969+q*0.0474179))/(1.+q*(3.23171+q*0.471222))+
					(-.5+_ln2)/log_x-1.)+t*
					((q*(-0.424278+q*(1.75166+q*(-1.46524+q*0.93347)))/(0.0312142+q*(-0.267478+q))+
							(-.625+_ln2)/log_x-1.)+t*
							((q*(3.39255+q*(16.1046+q*(-19.5216+q*10.7458)))/
									(1.+q*(4.1274+q*5.25449))+(-2./3+_ln2)/log_x-1.)+t*
									(0.16*q*(1.+q*(27.7934+q*(358.688+q*(-259.233+log_x*61.6566))))/
											(1.-8.1192*q+37.087*log_x)+(-131./192.+_ln2)/log_x-
											1.))):(-1.0018+
					(0.15128484821526975*(1.+33.04715298851381*q-3.51771875598067*log_x)*q)/
							(1.+3.2255053261256337*q)+(-0.5+_ln2)/log_x)+t*
					((1.1-2.608785958462561*(1.-0.6663562294911147*q)*q-(-0.625+_ln2)/log_x)+t*
							((-0.96+3.0912038297987596*(1.+0.6021398048785328*log_x)*q/
									(1.+4.240467556480155*log_x)+(-2./3+_ln2)/log_x)+t*(1.2-
									10.44604984418533*
											(1.+0.2137568928431227*q+0.3693275254470449*log_x)*q/
											(1.+4.95715636660691*q+7.70233216637738*log_x)-
									(-131./192.+_ln2)/log_x))))*t+1.)+std::log(t+2.)/log_x-
					_ln2/log_x*(1.+t);
		}
		for(;h>0;--h)
			t=std::exp(log_x*t);
		for(;h<0;++h)
			t=std::log(t)/log_x;
		return t;
	}
	Complex1d tetrate(Complex1d const& x, Complex1d const& y)
	{
		std::complex<double> log_x=std::log((std::complex<double>)x), cy=y;
		//	if(log_x.real()<.03)//abs(log_x)<1.03045453395352
		//	{
		//		if(cy.real()<-1.)
		//			return -30.;
		//		return 1.;
		//	}
		int h=int(::floor(cy.real()-.5)+1);
		std::complex<double> t(cy-(double)h);//real-round(real)+i*imag
		{
			//	bool unassigned=true;
			//	if(log_x.real()<.001)//abs(log_x)<1.00100050016671
			//	{
			//		if(t.real()>-1)//real-round(real)>-1
			//			unassigned=false, t=1.;
			//		else if(t.real()<-1)
			//			unassigned=false, t=-990.;
			//	}
			//	if(unassigned)
			//	{
			std::complex<double> q=std::sqrt(log_x);
			t=(t+1.)*((log_x.real()>_ln2?
					(q*(0.137467+q*(4.94969+q*0.0474179))/(1.+q*(3.23171+q*0.471222))+
							(-.5+_ln2)/log_x-1.)+t*
							((q*(-0.424278+q*(1.75166+q*(-1.46524+q*0.93347)))/
									(0.0312142+q*(-0.267478+q))+(-.625+_ln2)/log_x-1.)+t*
									((q*(3.39255+q*(16.1046+q*(-19.5216+q*10.7458)))/
											(1.+q*(4.1274+q*5.25449))+(-2./3+_ln2)/log_x-1.)+t*
											(0.16*q*(1.+q*(27.7934+
													q*(358.688+q*(-259.233+log_x*61.6566))))/
													(1.-8.1192*q+37.087*log_x)+
													(-131./192.+_ln2)/log_x-1.))):(-1.0018+
							(0.15128484821526975*(1.+33.04715298851381*q-3.51771875598067*log_x)*q)/
									(1.+3.2255053261256337*q)+(-0.5+_ln2)/log_x)+t*
							((1.1-2.608785958462561*(1.-0.6663562294911147*sqrt(log_x))*sqrt(log_x)-
									(-0.625+_ln2)/log_x)+t*((-0.96+
									3.0912038297987596*(1.+0.6021398048785328*log_x)*q/
											(1.+4.240467556480155*log_x)+(-2./3+_ln2)/log_x)+t*(1.2-
									10.44604984418533*
											(1.+0.2137568928431227*q+0.3693275254470449*log_x)*q/
											(1.+4.95715636660691*q+7.70233216637738*log_x)-
									(-131./192.+_ln2)/log_x))))*t+1.)+log(t+2.)/log_x-
					_ln2/log_x*(1.+t);
			//	}
		}
		for(;h>0;--h)
			t=exp(log_x*t);
		for(;h<0;++h)
			t=std::log(t)/log_x;
		return t;
	}
	Comp1d c_rr_tetrate(double const& x, double const& y)
	{
		Complex1d lo=tetrate(x, y);
		return Comp1d(lo.real(), lo.imag());
	}
	Comp1d c_rc_tetrate(double const& x, Comp1d const& y)
	{
		Complex1d lo=tetrate(x, Complex1d(y.r, y.i));
		return Comp1d(lo.real(), lo.imag());
	}
	Comp1d c_cr_tetrate(Comp1d const& x, double const& y)
	{
		Complex1d lo=tetrate(Complex1d(x.r, x.i), y);
		return Comp1d(lo.real(), lo.imag());
	}
	Comp1d c_cc_tetrate(Comp1d const& x, Comp1d const& y)
	{
		Complex1d lo=tetrate(Complex1d(x.r, x.i), Complex1d(y.r, y.i));
		return Comp1d(lo.real(), lo.imag());
	}
	Quat1d q_qr_tetrate(Quat1d const& x, double const& y)
	{
		if(y<-1)
		{
			int steps=int(abs(y));
			Quat1d t(y-floor(y)), lrx=inv(log(x));
			for(int k=0;k<steps;++k)
				t=log(t)*lrx;
			return t;
		}else if(y<=0)
			return Quat1d(1+y);
		else
		{
			int h=int(y)+1;
			Quat1d t(y-::floor(y));
			for(int k=0;k<h;++k)
				t=x^t;
			//	t=pow(x, t);
			return t;
		}
		//return tetrate_qr(Quat1d(x.r, x.i, x.j, x.k), y);
	}
	bool disc_rr_tetrate_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	{
		return false;
	}//
	bool disc_cr_tetrate_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	{
		return false;
	}//
	bool disc_qr_tetrate_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	{
		return false;
	}//
	bool disc_rc_tetrate_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	{
		return false;
	}//
	bool disc_cc_tetrate_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	{
		return false;
	}//

	class Tetrate//http://en.citizendium.org/wiki/Fit1.cin
	{
		static std::complex<double> _fit1(std::complex<double> x, std::complex<double> y)
		{
			if(x.real()<.001)
			{
				if(y.real()>-1)
					return 1.;
				if(y.real()<-1)
					return -990.;
			}
			std::complex<double> q=std::sqrt(x);
			return (y+1.)*((x.real()>_ln2?
					(q*(0.137467+q*(4.94969+q*0.0474179))/(1.+q*(3.23171+q*0.471222))+(-.5+_ln2)/x-
							1.)+y*((q*(-0.424278+q*(1.75166+q*(-1.46524+q*0.93347)))/
							(0.0312142+q*(-0.267478+q))+(-.625+_ln2)/x-1.)+y*
							((q*(3.39255+q*(16.1046+q*(-19.5216+q*10.7458)))/
									(1.+q*(4.1274+q*5.25449))+(-2./3+_ln2)/x-1.)+y*
									(0.16*q*(1.+q*(27.7934+q*(358.688+q*(-259.233+x*61.6566))))/
											(1.-8.1192*q+37.087*x)+(-131./192.+_ln2)/x-1.))):
					(-1.0018+(0.15128484821526975*(1.+33.04715298851381*q-3.51771875598067*x)*q)/
							(1.+3.2255053261256337*q)+(-0.5+_ln2)/x)+y*
							((1.1-2.608785958462561*(1.-0.6663562294911147*sqrt(x))*sqrt(x)-
									(-0.625+_ln2)/x)+y*((-0.96+
									3.0912038297987596*(1.+0.6021398048785328*x)*q/
											(1.+4.240467556480155*x)+(-2./3+_ln2)/x)+y*(1.2-
									10.44604984418533*
											(1.+0.2137568928431227*q+0.3693275254470449*x)*q/
											(1.+4.95715636660691*q+7.70233216637738*x)-
									(-131./192.+_ln2)/x))))*y+1.)+log(y+2.)/x-_ln2/x*(1.+y);
		}
	public:
		static std::complex<double> fit1(std::complex<double> x, std::complex<double> y)
		{
			x=std::log(x);
			if(x.real()<.03)
			{
				if(y.real()<-1.)
					return -30.;
				return 1.;
			}
			int h=int(::floor(y.real()-.5)+1);
			std::complex<double> result=_fit1(x, y-(double)h);
			for(;h>0;--h)
				result=exp(x*result);
			for(;h<0;++h)
				result=std::log(result)/x;
			return result;
		}
	};
	Complex1d pentate(double x, double y)
	{
		long long h=convert_d2ll_zero(y);
		//	long long h=y.r!=y.r||y.r<-ll_max||y.r>ll_max?0:long long(y);
		//	long long h=std::isnan(y.r)||std::isinf(y.r)?0:long long(y);
		if(h<-2)
			return _HUGE;//1/::sin(0);
		if(h==-2)
			return -1;
		if(h==-1)
			return 0;
		if(h==0)
			return 1;
		if(h==1)
			return x;
		Complex1d result(x);
		for(int k=0;k<h;++k)
			result=Tetrate::fit1(x, result);
		return result;
	}
	Complex1d pentate(Complex1d const& x, double y)
	{
		long long h=convert_d2ll_zero(y);
		if(h<-2)
			return _HUGE;//1/::sin(0);
		if(h==-2)
			return -1;
		if(h==-1)
			return 0;
		if(h==0)
			return 1;
		if(h==1)
			return x;
		Complex1d cx=x, result(cx);
		for(int k=0;k<h;++k)
			result=Tetrate::fit1(cx, result);
		return result;
	}
	Comp1d c_rr_pentate(double const& x, double const& y)
	{
		Complex1d lo=pentate(x, y);
		return Comp1d(lo.real(), lo.imag());
	}
	Comp1d c_cr_pentate(Comp1d const& x, double const& y)
	{
		Complex1d lo=pentate(Complex1d(x.r, x.i), y);
		return Comp1d(lo.real(), lo.imag());
	}
	bool disc_rr_pentate_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	{
		return false;
	}//
	bool disc_cr_pentate_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	{
		return false;
	}//

	double r_r_bitwise_shift_left_l(double const& x){return ::exp(floor(x)*_ln2);}
	Comp1d c_c_bitwise_shift_left_l(Comp1d const& x){return exp(x.floor()*_ln2);}
	Quat1d q_q_bitwise_shift_left_l(Quat1d const& x){return exp(x.floor()*_ln2);}
	double r_r_bitwise_shift_left_r(double const& x){return x*2;}
	Comp1d c_c_bitwise_shift_left_r(Comp1d const& x){return x*2;}
	Quat1d q_q_bitwise_shift_left_r(Quat1d const& x){return x*2;}
	double r_rr_bitwise_shift_left(double const& x, double const& y){return x*::exp(floor(y)*_ln2);}//x<<y = x*2^floory
	Comp1d c_rc_bitwise_shift_left(double const& x, Comp1d const& y){return x*exp(y.floor()*_ln2);}
	Quat1d q_rq_bitwise_shift_left(double const& x, Quat1d const& y){return x*exp(y.floor()*_ln2);}
	Comp1d c_cr_bitwise_shift_left(Comp1d const& x, double const& y){return x*::exp(floor(y)*_ln2);}
	Comp1d c_cc_bitwise_shift_left(Comp1d const& x, Comp1d const& y){return x*exp(y.floor()*_ln2);}
	Quat1d q_cq_bitwise_shift_left(Comp1d const& x, Quat1d const& y){return x*exp(y.floor()*_ln2);}
	Quat1d q_qr_bitwise_shift_left(Quat1d const& x, double const& y){return x*::exp(floor(y)*_ln2);}
	Quat1d q_qc_bitwise_shift_left(Quat1d const& x, Comp1d const& y){return x*exp(y.floor()*_ln2);}
	Quat1d q_qq_bitwise_shift_left(Quat1d const& x, Quat1d const& y){return x*exp(y.floor()*_ln2);}
	auto disc_r_bitwise_shift_left_l_o=disc_r_ceil_o;
	auto disc_c_bitwise_shift_left_l_o=disc_c_ceil_o;
	auto disc_q_bitwise_shift_left_l_o=disc_q_ceil_o;
	bool disc_rr_bitwise_shift_left_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return std::floor(x1.r)-std::floor(x0.r)!=0;}
	bool disc_rc_bitwise_shift_left_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return std::floor(x1.r)-std::floor(x0.r)!=0;}
	bool disc_rq_bitwise_shift_left_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return std::floor(x1.r)-std::floor(x0.r)!=0;}
	bool disc_cr_bitwise_shift_left_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return std::floor(x1.r)-std::floor(x0.r)!=0||std::floor(x1.i)-std::floor(x0.i)!=0;}
	bool disc_cc_bitwise_shift_left_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return std::floor(x1.r)-std::floor(x0.r)!=0||std::floor(x1.i)-std::floor(x0.i)!=0;}
	bool disc_cq_bitwise_shift_left_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return std::floor(x1.r)-std::floor(x0.r)!=0||std::floor(x1.i)-std::floor(x0.i)!=0;}
	bool disc_qr_bitwise_shift_left_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return std::floor(x1.r)-std::floor(x0.r)!=0||std::floor(x1.i)-std::floor(x0.i)!=0||std::floor(x1.j)-std::floor(x0.j)!=0||std::floor(x1.k)-std::floor(x0.k)!=0;}
	bool disc_qc_bitwise_shift_left_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return std::floor(x1.r)-std::floor(x0.r)!=0||std::floor(x1.i)-std::floor(x0.i)!=0||std::floor(x1.j)-std::floor(x0.j)!=0||std::floor(x1.k)-std::floor(x0.k)!=0;}
	bool disc_qq_bitwise_shift_left_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return std::floor(x1.r)-std::floor(x0.r)!=0||std::floor(x1.i)-std::floor(x0.i)!=0||std::floor(x1.j)-std::floor(x0.j)!=0||std::floor(x1.k)-std::floor(x0.k)!=0;}

	double r_r_bitwise_shift_right_l(double const& x){return ::exp(-floor(x)*_ln2);}
	Comp1d c_c_bitwise_shift_right_l(Comp1d const& x){return exp(-x.floor()*_ln2);}
	Quat1d q_q_bitwise_shift_right_l(Quat1d const& x){return exp(-x.floor()*_ln2);}
	double r_r_bitwise_shift_right_r(double const& x){return x*0.5;}
	Comp1d c_c_bitwise_shift_right_r(Comp1d const& x){return x*0.5;}
	Quat1d q_q_bitwise_shift_right_r(Quat1d const& x){return x*0.5;}
	double r_rr_bitwise_shift_right(double const& x, double const& y){return x*::exp(-floor(y)*_ln2);}
	Comp1d c_rc_bitwise_shift_right(double const& x, Comp1d const& y){return Comp1d(x*exp(-y.floor()*_ln2));}
	Quat1d q_rq_bitwise_shift_right(double const& x, Quat1d const& y){return Quat1d(x*exp(-y.floor()*_ln2));}
	Comp1d c_cr_bitwise_shift_right(Comp1d const& x, double const& y){return Comp1d(x*::exp(-floor(y)*_ln2));}
	Comp1d c_cc_bitwise_shift_right(Comp1d const& x, Comp1d const& y){return Comp1d(x*exp(-y.floor()*_ln2));}
	Quat1d q_cq_bitwise_shift_right(Comp1d const& x, Quat1d const& y){return Quat1d(x*exp(-y.floor()*_ln2));}
	Quat1d q_qr_bitwise_shift_right(Quat1d const& x, double const& y){return Quat1d(x*::exp(-floor(y)*_ln2));}
	Quat1d q_qc_bitwise_shift_right(Quat1d const& x, Comp1d const& y){return Quat1d(x*exp(-y.floor()*_ln2));}
	Quat1d q_qq_bitwise_shift_right(Quat1d const& x, Quat1d const& y){return Quat1d(x*exp(-y.floor()*_ln2));}
	auto disc_r_bitwise_shift_right_l_o=disc_r_ceil_o;
	auto disc_c_bitwise_shift_right_l_o=disc_c_ceil_o;
	auto disc_q_bitwise_shift_right_l_o=disc_q_ceil_o;
	auto disc_rr_bitwise_shift_right_i=disc_rr_bitwise_shift_left_i;
	auto disc_rc_bitwise_shift_right_i=disc_rc_bitwise_shift_left_i;
	auto disc_rq_bitwise_shift_right_i=disc_rq_bitwise_shift_left_i;
	auto disc_cr_bitwise_shift_right_i=disc_cr_bitwise_shift_left_i;
	auto disc_cc_bitwise_shift_right_i=disc_cc_bitwise_shift_left_i;
	auto disc_cq_bitwise_shift_right_i=disc_cq_bitwise_shift_left_i;
	auto disc_qr_bitwise_shift_right_i=disc_qr_bitwise_shift_left_i;
	auto disc_qc_bitwise_shift_right_i=disc_qc_bitwise_shift_left_i;
	auto disc_qq_bitwise_shift_right_i=disc_qq_bitwise_shift_left_i;

	inline double bitwise_not(double const& x)
	{
		long long mask=-is_real(x);
		return ~convert_d2ll(x)&mask;
	}
	double r_r_bitwise_not(double const& x){return bitwise_not(x);}
	Comp1d c_c_bitwise_not(Comp1d const& x){return Comp1d(bitwise_not(x.r), bitwise_not(x.i));}
	Quat1d q_q_bitwise_not(Quat1d const& x){return Quat1d(bitwise_not(x.r), bitwise_not(x.i), bitwise_not(x.j), bitwise_not(x.k));}
	auto disc_r_bitwise_not_i=disc_r_arg_i;
	bool disc_c_bitwise_not_i(Value const &x0, Value const &x1){return std::signbit(x0.r)!=std::signbit(x1.r)||std::signbit(x0.i)!=std::signbit(x1.i);}
	bool disc_q_bitwise_not_i(Value const &x0, Value const &x1){return std::signbit(x0.r)!=std::signbit(x1.r)||std::signbit(x0.i)!=std::signbit(x1.i)||std::signbit(x0.j)!=std::signbit(x1.j)||std::signbit(x0.k)!=std::signbit(x1.k);}

	inline double bitwise_and(double const& x)
	{
		long long mask=-is_real(x);
		return (!~convert_d2ll(x))&mask;
	}
	inline double bitwise_and(double const& x, double const& y)
	{
		long long mask=-(is_real(x)&is_real(x));
		return convert_d2ll(x)&convert_d2ll(y)&mask;
	}
	inline double bitwise_and_ll(double const& x, double const& y)
	{
		long long mask=-(is_real(x)&is_real(x));
		return convert_d2ll(x)&convert_d2ll(y)&mask;
	}
	inline double convert_ll2d(double const& x){return (double)(long long const&)x;}
	double r_r_bitwise_and(double const& x){return bitwise_and(x);}
	Comp1d c_c_bitwise_and(Comp1d const& x){return Comp1d(bitwise_and(x.r), bitwise_and(x.i));}
	Quat1d q_q_bitwise_and(Quat1d const& x){return Quat1d(bitwise_and(x.r), bitwise_and(x.i), bitwise_and(x.j), bitwise_and(x.k));}
	double r_rr_bitwise_and(double const& x, double const& y){return bitwise_and(x, y);}
	Comp1d c_rc_bitwise_and(double const& x, Comp1d const& y){return Comp1d(bitwise_and(x, y.r), bitwise_and(x, y.i));}
	Quat1d q_rq_bitwise_and(double const& x, Quat1d const& y){return Quat1d(bitwise_and(x, y.r), bitwise_and(x, y.i), bitwise_and(x, y.j), bitwise_and(x, y.k));}
	Comp1d c_cr_bitwise_and(Comp1d const& x, double const& y){return Comp1d(bitwise_and(x.r, y), bitwise_and(x.i, y));}
	Comp1d c_cc_bitwise_and(Comp1d const& x, Comp1d const& y)
	{
		double xr_yr=bitwise_and_ll(x.r, y.r), xi_yr=bitwise_and_ll(x.i, y.r), xr_yi=bitwise_and_ll(x.r, y.i), xi_yi=bitwise_and_ll(x.i, y.i);
		return Comp1d(convert_ll2d(xr_yr-xi_yi), convert_ll2d(xr_yi+xi_yr));
	}
	Quat1d q_cq_bitwise_and(Comp1d const& x, Quat1d const& y)
	{
		double xr_yr=bitwise_and_ll(x.r, y.r), xi_yr=bitwise_and_ll(x.i, y.r), xr_yi=bitwise_and_ll(x.r, y.i), xi_yi=bitwise_and_ll(x.i, y.i), xr_yj=bitwise_and_ll(x.r, y.j), xi_yj=bitwise_and_ll(x.i, y.j), xr_yk=bitwise_and_ll(x.r, y.k), xi_yk=bitwise_and_ll(x.i, y.k);
		return Quat1d(convert_ll2d(xr_yr-xi_yi), convert_ll2d(xr_yi+xi_yr), convert_ll2d(
				xr_yj-xi_yk), convert_ll2d(xr_yk+xi_yj));
	}
	Quat1d q_qr_bitwise_and(Quat1d const& x, double const& y)
	{
		return Quat1d(bitwise_and(x.r, y), bitwise_and(x.i, y), bitwise_and(x.j, y), bitwise_and(x.k, y));
	}
	Quat1d q_qc_bitwise_and(Quat1d const& x, Comp1d const& y)
	{
		double xr_yr=bitwise_and_ll(x.r, y.r), xi_yr=bitwise_and_ll(x.i, y.r), xj_yr=bitwise_and_ll(x.j, y.r), xk_yr=bitwise_and_ll(x.k, y.r), xr_yi=bitwise_and_ll(x.r, y.i), xi_yi=bitwise_and_ll(x.i, y.i), xj_yi=bitwise_and_ll(x.j, y.i), xk_yi=bitwise_and_ll(x.k, y.i);
		return Quat1d(convert_ll2d(xr_yr-xi_yi), convert_ll2d(xr_yi+xi_yr), convert_ll2d(
				xj_yr+xk_yi), convert_ll2d(-xj_yi+xk_yr));
	}
	Quat1d q_qq_bitwise_and(Quat1d const& x, Quat1d const& y)
	{
		double xr_yr=bitwise_and_ll(x.r, y.r), xi_yr=bitwise_and_ll(x.i, y.r), xj_yr=bitwise_and_ll(x.j, y.r), xk_yr=bitwise_and_ll(x.k, y.r), xr_yi=bitwise_and_ll(x.r, y.i), xi_yi=bitwise_and_ll(x.i, y.i), xj_yi=bitwise_and_ll(x.j, y.i), xk_yi=bitwise_and_ll(x.k, y.i), xr_yj=bitwise_and_ll(x.r, y.j), xi_yj=bitwise_and_ll(x.i, y.j), xj_yj=bitwise_and_ll(x.j, y.j), xk_yj=bitwise_and_ll(x.k, y.j), xr_yk=bitwise_and_ll(x.r, y.k), xi_yk=bitwise_and_ll(x.i, y.k), xj_yk=bitwise_and_ll(x.j, y.k), xk_yk=bitwise_and_ll(x.k, y.k);
		return Quat1d(convert_ll2d(xr_yr-xi_yi-xj_yj-xk_yk), convert_ll2d(
				xr_yi+xi_yr+xj_yk-xk_yj), convert_ll2d(xj_yj-xi_yk+xj_yr+xk_yi), convert_ll2d(
				xr_yk+xi_yj-xj_yi+xk_yr));
	}
	auto disc_r_bitwise_and_o=disc_r_ceil_o;
	auto disc_c_bitwise_and_o=disc_c_ceil_o;
	auto disc_q_bitwise_and_o=disc_q_ceil_o;
	//auto disc_rr_bitwise_and_o=disc_r_ceil_o;
	//auto disc_rc_bitwise_and_o=disc_c_ceil_o;
	//auto disc_rq_bitwise_and_o=disc_q_ceil_o;
	//auto disc_cr_bitwise_and_o=disc_c_ceil_o;
	//auto disc_cc_bitwise_and_o=disc_c_ceil_o;
	//auto disc_cq_bitwise_and_o=disc_q_ceil_o;
	//auto disc_qr_bitwise_and_o=disc_q_ceil_o;
	//auto disc_qc_bitwise_and_o=disc_q_ceil_o;
	//auto disc_qq_bitwise_and_o=disc_q_ceil_o;

	inline double bitwise_nand(double const& x)
	{
		long long mask=-is_real(x);
		return (~convert_d2ll(x)!=0)&mask;
	}
	inline double bitwise_nand(double const& x, double const& y)
	{
		long long mask=-(is_real(x)&is_real(x));
		return (~(convert_d2ll(x)&convert_d2ll(y)))&mask;
	}
	inline double bitwise_nand_ll(double const& x, double const& y)
	{
		long long mask=-(is_real(x)&is_real(y));
		return ~(convert_d2ll(x)&convert_d2ll(y))&mask;
	}
	double r_r_bitwise_nand(double const& x){return bitwise_nand(x);}
	Comp1d c_c_bitwise_nand(Comp1d const& x){return Comp1d(bitwise_nand(x.r), bitwise_nand(x.i));}
	Quat1d q_q_bitwise_nand(Quat1d const& x){return Quat1d(bitwise_nand(x.r), bitwise_nand(x.i), bitwise_nand(x.j), bitwise_nand(x.k));}
	double r_rr_bitwise_nand(double const& x, double const& y){return bitwise_nand(x, y);}
	Comp1d c_rc_bitwise_nand(double const& x, Comp1d const& y){return Comp1d(bitwise_nand(x, y.r), bitwise_nand(x, y.i));}
	Quat1d q_rq_bitwise_nand(double const& x, Quat1d const& y){return Quat1d(bitwise_nand(x, y.r), bitwise_nand(x, y.i), bitwise_nand(x, y.j), bitwise_nand(x, y.k));}
	Comp1d c_cr_bitwise_nand(Comp1d const& x, double const& y){return Comp1d(bitwise_nand(x.r, y), bitwise_nand(x.i, y));}
	Comp1d c_cc_bitwise_nand(Comp1d const& x, Comp1d const& y)
	{
		double xr_yr=bitwise_nand_ll(x.r, y.r), xi_yr=bitwise_nand_ll(x.i, y.r), xr_yi=bitwise_nand_ll(x.r, y.i), xi_yi=bitwise_nand_ll(x.i, y.i);
		return Comp1d(convert_ll2d(xr_yr-xi_yi), convert_ll2d(xr_yi+xi_yr));
	}
	Quat1d q_cq_bitwise_nand(Comp1d const& x, Quat1d const& y)
	{
		double xr_yr=bitwise_nand_ll(x.r, y.r), xi_yr=bitwise_nand_ll(x.i, y.r), xr_yi=bitwise_nand_ll(x.r, y.i), xi_yi=bitwise_nand_ll(x.i, y.i), xr_yj=bitwise_nand_ll(x.r, y.j), xi_yj=bitwise_nand_ll(x.i, y.j), xr_yk=bitwise_nand_ll(x.r, y.k), xi_yk=bitwise_nand_ll(x.i, y.k);
		return Quat1d(convert_ll2d(xr_yr-xi_yi), convert_ll2d(xr_yi+xi_yr), convert_ll2d(
				xr_yj-xi_yk), convert_ll2d(xr_yk+xi_yj));
	}
	Quat1d q_qr_bitwise_nand(Quat1d const& x, double const& y)
	{
		return Quat1d(bitwise_nand(x.r, y), bitwise_nand(x.i, y), bitwise_nand(x.j, y), bitwise_nand(x.k, y));
	}
	Quat1d q_qc_bitwise_nand(Quat1d const& x, Comp1d const& y)
	{
		double xr_yr=bitwise_nand_ll(x.r, y.r), xi_yr=bitwise_nand_ll(x.i, y.r), xj_yr=bitwise_nand_ll(x.j, y.r), xk_yr=bitwise_nand_ll(x.k, y.r), xr_yi=bitwise_nand_ll(x.r, y.i), xi_yi=bitwise_nand_ll(x.i, y.i), xj_yi=bitwise_nand_ll(x.j, y.i), xk_yi=bitwise_nand_ll(x.k, y.i);
		return Quat1d(convert_ll2d(xr_yr-xi_yi), convert_ll2d(xr_yi+xi_yr), convert_ll2d(
				xj_yr+xk_yi), convert_ll2d(-xj_yi+xk_yr));
	}
	Quat1d q_qq_bitwise_nand(Quat1d const& x, Quat1d const& y)
	{
		double xr_yr=bitwise_nand_ll(x.r, y.r), xi_yr=bitwise_nand_ll(x.i, y.r), xj_yr=bitwise_nand_ll(x.j, y.r), xk_yr=bitwise_nand_ll(x.k, y.r), xr_yi=bitwise_nand_ll(x.r, y.i), xi_yi=bitwise_nand_ll(x.i, y.i), xj_yi=bitwise_nand_ll(x.j, y.i), xk_yi=bitwise_nand_ll(x.k, y.i), xr_yj=bitwise_nand_ll(x.r, y.j), xi_yj=bitwise_nand_ll(x.i, y.j), xj_yj=bitwise_nand_ll(x.j, y.j), xk_yj=bitwise_nand_ll(x.k, y.j), xr_yk=bitwise_nand_ll(x.r, y.k), xi_yk=bitwise_nand_ll(x.i, y.k), xj_yk=bitwise_nand_ll(x.j, y.k), xk_yk=bitwise_nand_ll(x.k, y.k);
		return Quat1d(convert_ll2d(xr_yr-xi_yi-xj_yj-xk_yk), convert_ll2d(
				xr_yi+xi_yr+xj_yk-xk_yj), convert_ll2d(xj_yj-xi_yk+xj_yr+xk_yi), convert_ll2d(
				xr_yk+xi_yj-xj_yi+xk_yr));
	}
	auto disc_r_bitwise_nand_o=disc_r_bitwise_and_o;
	auto disc_c_bitwise_nand_o=disc_c_bitwise_and_o;
	auto disc_q_bitwise_nand_o=disc_q_bitwise_and_o;
	//auto disc_rr_bitwise_nand_o=disc_rr_bitwise_and_o;
	//auto disc_rc_bitwise_nand_o=disc_rc_bitwise_and_o;
	//auto disc_rq_bitwise_nand_o=disc_rq_bitwise_and_o;
	//auto disc_cr_bitwise_nand_o=disc_cr_bitwise_and_o;
	//auto disc_cc_bitwise_nand_o=disc_cc_bitwise_and_o;
	//auto disc_cq_bitwise_nand_o=disc_cq_bitwise_and_o;
	//auto disc_qr_bitwise_nand_o=disc_qr_bitwise_and_o;
	//auto disc_qc_bitwise_nand_o=disc_qc_bitwise_and_o;
	//auto disc_qq_bitwise_nand_o=disc_qq_bitwise_and_o;

	inline double bitwise_or(double const& x)
	{
		long long mask=-is_real(x);
		return (convert_d2ll(x)!=0)&mask;
	}
	inline double bitwise_or(double const& x, double const& y)
	{
		long long mask=-(is_real(x)&is_real(y));
		return (convert_d2ll(x)|convert_d2ll(y))&mask;
	}
	inline double bitwise_or_ll_c(double const& x, double const& y)
	{
		long long mask=-(is_real(x)&is_real(y));
		return (~convert_d2ll(x)&~convert_d2ll(y))&mask;
	}
	inline double convert_ll2d_c(double const& x){return (double)~(long long)(long long const&)x;}
	double r_r_bitwise_or(double const& x){return bitwise_or(x);}
	Comp1d c_c_bitwise_or(Comp1d const& x){return Comp1d(bitwise_or(x.r), bitwise_or(x.i));}
	Quat1d q_q_bitwise_or(Quat1d const& x){return Quat1d(bitwise_or(x.r), bitwise_or(x.i), bitwise_or(x.j), bitwise_or(x.k));}
	double r_rr_bitwise_or(double const& x, double const& y){return bitwise_or(x, y);}
	Comp1d c_rc_bitwise_or(double const& x, Comp1d const& y){return Comp1d(bitwise_or(x, y.r), bitwise_or(x, y.i));}
	Quat1d q_rq_bitwise_or(double const& x, Quat1d const& y){return Quat1d(bitwise_or(x, y.r), bitwise_or(x, y.i), bitwise_or(x, y.j), bitwise_or(x, y.k));}
	Comp1d c_cr_bitwise_or(Comp1d const& x, double const& y){return Comp1d(bitwise_or(x.r, y), bitwise_or(x.i, y));}
	Comp1d c_cc_bitwise_or(Comp1d const& x, Comp1d const& y)
	{
		double xr_yr=bitwise_or_ll_c(x.r, y.r), xi_yr=bitwise_or_ll_c(x.i, y.r), xr_yi=bitwise_or_ll_c(x.r, y.i), xi_yi=bitwise_or_ll_c(x.i, y.i);
		return Comp1d(convert_ll2d_c(xr_yr-xi_yi), convert_ll2d_c(xr_yi+xi_yr));
	}
	Quat1d q_cq_bitwise_or(Comp1d const& x, Quat1d const& y)
	{
		double xr_yr=bitwise_or_ll_c(x.r, y.r), xi_yr=bitwise_or_ll_c(x.i, y.r), xr_yi=bitwise_or_ll_c(x.r, y.i), xi_yi=bitwise_or_ll_c(x.i, y.i), xr_yj=bitwise_or_ll_c(x.r, y.j), xi_yj=bitwise_or_ll_c(x.i, y.j), xr_yk=bitwise_or_ll_c(x.r, y.k), xi_yk=bitwise_or_ll_c(x.i, y.k);
		return Quat1d(convert_ll2d_c(xr_yr-xi_yi), convert_ll2d_c(xr_yi+xi_yr), convert_ll2d_c(
				xr_yj-xi_yk), convert_ll2d_c(xr_yk+xi_yj));
	}
	Quat1d q_qr_bitwise_or(Quat1d const& x, double const& y)
	{
		return Quat1d(bitwise_or(x.r, y), bitwise_or(x.i, y), bitwise_or(x.j, y), bitwise_or(x.k, y));
	}
	Quat1d q_qc_bitwise_or(Quat1d const& x, Comp1d const& y)
	{
		double xr_yr=bitwise_or_ll_c(x.r, y.r), xi_yr=bitwise_or_ll_c(x.i, y.r), xj_yr=bitwise_or_ll_c(x.j, y.r), xk_yr=bitwise_or_ll_c(x.k, y.r), xr_yi=bitwise_or_ll_c(x.r, y.i), xi_yi=bitwise_or_ll_c(x.i, y.i), xj_yi=bitwise_or_ll_c(x.j, y.i), xk_yi=bitwise_or_ll_c(x.k, y.i);
		return Quat1d(convert_ll2d_c(xr_yr-xi_yi), convert_ll2d_c(xr_yi+xi_yr), convert_ll2d_c(
				xj_yr+xk_yi), convert_ll2d_c(-xj_yi+xk_yr));
	}
	Quat1d q_qq_bitwise_or(Quat1d const& x, Quat1d const& y)
	{
		double xr_yr=bitwise_or_ll_c(x.r, y.r), xi_yr=bitwise_or_ll_c(x.i, y.r), xj_yr=bitwise_or_ll_c(x.j, y.r), xk_yr=bitwise_or_ll_c(x.k, y.r), xr_yi=bitwise_or_ll_c(x.r, y.i), xi_yi=bitwise_or_ll_c(x.i, y.i), xj_yi=bitwise_or_ll_c(x.j, y.i), xk_yi=bitwise_or_ll_c(x.k, y.i), xr_yj=bitwise_or_ll_c(x.r, y.j), xi_yj=bitwise_or_ll_c(x.i, y.j), xj_yj=bitwise_or_ll_c(x.j, y.j), xk_yj=bitwise_or_ll_c(x.k, y.j), xr_yk=bitwise_or_ll_c(x.r, y.k), xi_yk=bitwise_or_ll_c(x.i, y.k), xj_yk=bitwise_or_ll_c(x.j, y.k), xk_yk=bitwise_or_ll_c(x.k, y.k);
		return Quat1d(convert_ll2d_c(xr_yr-xi_yi-xj_yj-xk_yk), convert_ll2d_c(
				xr_yi+xi_yr+xj_yk-xk_yj), convert_ll2d_c(xj_yj-xi_yk+xj_yr+xk_yi), convert_ll2d_c(
				xr_yk+xi_yj-xj_yi+xk_yr));
	}
	auto disc_r_bitwise_or_o=disc_r_bitwise_and_o;
	auto disc_c_bitwise_or_o=disc_c_bitwise_and_o;
	auto disc_q_bitwise_or_o=disc_q_bitwise_and_o;
	//auto disc_rr_bitwise_or_o=disc_rr_bitwise_and_o;
	//auto disc_rc_bitwise_or_o=disc_rc_bitwise_and_o;
	//auto disc_rq_bitwise_or_o=disc_rq_bitwise_and_o;
	//auto disc_cr_bitwise_or_o=disc_cr_bitwise_and_o;
	//auto disc_cc_bitwise_or_o=disc_cc_bitwise_and_o;
	//auto disc_cq_bitwise_or_o=disc_cq_bitwise_and_o;
	//auto disc_qr_bitwise_or_o=disc_qr_bitwise_and_o;
	//auto disc_qc_bitwise_or_o=disc_qc_bitwise_and_o;
	//auto disc_qq_bitwise_or_o=disc_qq_bitwise_and_o;

	inline double bitwise_nor(double const& x)
	{
		long long mask=-is_real(x);
		return (double)((!convert_d2ll(x))&mask);
	}
	inline double bitwise_nor(double const& x, double const& y)
	{
		long long mask=-(is_real(x)&is_real(y));
		return (~(convert_d2ll(x)|convert_d2ll(y)))&mask;
	}
	inline double bitwise_nor_ll_c(double const& x, double const& y)
	{
		long long mask=-(is_real(x)&is_real(y));
		return (~convert_d2ll(x)|~convert_d2ll(y))&mask;
	}
	double r_r_bitwise_nor(double const& x){return bitwise_nor(x);}
	Comp1d c_c_bitwise_nor(Comp1d const& x){return Comp1d(bitwise_nor(x.r), bitwise_nor(x.i));}
	Quat1d q_q_bitwise_nor(Quat1d const& x){return Quat1d(bitwise_nor(x.r), bitwise_nor(x.i), bitwise_nor(x.j), bitwise_nor(x.k));}
	double r_rr_bitwise_nor(double const& x, double const& y){return bitwise_nor(x, y);}
	Comp1d c_rc_bitwise_nor(double const& x, Comp1d const& y){return Comp1d(bitwise_nor(x, y.r), bitwise_nor(x, y.i));}
	Quat1d q_rq_bitwise_nor(double const& x, Quat1d const& y){return Quat1d(bitwise_nor(x, y.r), bitwise_nor(x, y.i), bitwise_nor(x, y.j), bitwise_nor(x, y.k));}
	Comp1d c_cr_bitwise_nor(Comp1d const& x, double const& y){return Comp1d(bitwise_nor(x.r, y), bitwise_nor(x.i, y));}
	Comp1d c_cc_bitwise_nor(Comp1d const& x, Comp1d const& y)
	{
		double xr_yr=bitwise_nor_ll_c(x.r, y.r), xi_yr=bitwise_nor_ll_c(x.i, y.r), xr_yi=bitwise_nor_ll_c(x.r, y.i), xi_yi=bitwise_nor_ll_c(x.i, y.i);
		return Comp1d(convert_ll2d_c(xr_yr-xi_yi), convert_ll2d_c(xr_yi+xi_yr));
	}
	Quat1d q_cq_bitwise_nor(Comp1d const& x, Quat1d const& y)
	{
		double xr_yr=bitwise_nor_ll_c(x.r, y.r), xi_yr=bitwise_nor_ll_c(x.i, y.r), xr_yi=bitwise_nor_ll_c(x.r, y.i), xi_yi=bitwise_nor_ll_c(x.i, y.i), xr_yj=bitwise_nor_ll_c(x.r, y.j), xi_yj=bitwise_nor_ll_c(x.i, y.j), xr_yk=bitwise_nor_ll_c(x.r, y.k), xi_yk=bitwise_nor_ll_c(x.i, y.k);
		return Quat1d(convert_ll2d_c(xr_yr-xi_yi), convert_ll2d_c(xr_yi+xi_yr), convert_ll2d_c(
				xr_yj-xi_yk), convert_ll2d_c(xr_yk+xi_yj));
	}
	Quat1d q_qr_bitwise_nor(Quat1d const& x, double const& y)
	{
		return Quat1d(bitwise_nor(x.r, y), bitwise_nor(x.i, y), bitwise_nor(x.j, y), bitwise_nor(x.k, y));
	}
	Quat1d q_qc_bitwise_nor(Quat1d const& x, Comp1d const& y)
	{
		double xr_yr=bitwise_nor_ll_c(x.r, y.r), xi_yr=bitwise_nor_ll_c(x.i, y.r), xj_yr=bitwise_nor_ll_c(x.j, y.r), xk_yr=bitwise_nor_ll_c(x.k, y.r), xr_yi=bitwise_nor_ll_c(x.r, y.i), xi_yi=bitwise_nor_ll_c(x.i, y.i), xj_yi=bitwise_nor_ll_c(x.j, y.i), xk_yi=bitwise_nor_ll_c(x.k, y.i);
		return Quat1d(convert_ll2d_c(xr_yr-xi_yi), convert_ll2d_c(xr_yi+xi_yr), convert_ll2d_c(
				xj_yr+xk_yi), convert_ll2d_c(-xj_yi+xk_yr));
	}
	Quat1d q_qq_bitwise_nor(Quat1d const& x, Quat1d const& y)
	{
		double xr_yr=bitwise_nor_ll_c(x.r, y.r), xi_yr=bitwise_nor_ll_c(x.i, y.r), xj_yr=bitwise_nor_ll_c(x.j, y.r), xk_yr=bitwise_nor_ll_c(x.k, y.r), xr_yi=bitwise_nor_ll_c(x.r, y.i), xi_yi=bitwise_nor_ll_c(x.i, y.i), xj_yi=bitwise_nor_ll_c(x.j, y.i), xk_yi=bitwise_nor_ll_c(x.k, y.i), xr_yj=bitwise_nor_ll_c(x.r, y.j), xi_yj=bitwise_nor_ll_c(x.i, y.j), xj_yj=bitwise_nor_ll_c(x.j, y.j), xk_yj=bitwise_nor_ll_c(x.k, y.j), xr_yk=bitwise_nor_ll_c(x.r, y.k), xi_yk=bitwise_nor_ll_c(x.i, y.k), xj_yk=bitwise_nor_ll_c(x.j, y.k), xk_yk=bitwise_nor_ll_c(x.k, y.k);
		return Quat1d(convert_ll2d_c(xr_yr-xi_yi-xj_yj-xk_yk), convert_ll2d_c(
				xr_yi+xi_yr+xj_yk-xk_yj), convert_ll2d_c(xj_yj-xi_yk+xj_yr+xk_yi), convert_ll2d_c(
				xr_yk+xi_yj-xj_yi+xk_yr));
	}
	auto disc_r_bitwise_nor_o=disc_r_bitwise_and_o;
	auto disc_c_bitwise_nor_o=disc_c_bitwise_and_o;
	auto disc_q_bitwise_nor_o=disc_q_bitwise_and_o;
	//auto disc_rr_bitwise_nor_o=disc_rr_bitwise_and_o;
	//auto disc_rc_bitwise_nor_o=disc_rc_bitwise_and_o;
	//auto disc_rq_bitwise_nor_o=disc_rq_bitwise_and_o;
	//auto disc_cr_bitwise_nor_o=disc_cr_bitwise_and_o;
	//auto disc_cc_bitwise_nor_o=disc_cc_bitwise_and_o;
	//auto disc_cq_bitwise_nor_o=disc_cq_bitwise_and_o;
	//auto disc_qr_bitwise_nor_o=disc_qr_bitwise_and_o;
	//auto disc_qc_bitwise_nor_o=disc_qc_bitwise_and_o;
	//auto disc_qq_bitwise_nor_o=disc_qq_bitwise_and_o;

	inline double bitwise_xor(double const& x)
	{
		long long mask=-is_real(x);
		long long t1=convert_d2ll(x)&mask;
		t1^=t1>>32, t1^=t1>>16, t1^=t1>>8, t1^=t1>>4;
		t1&=15;
		return (0x6996>>t1)&1;
	}
	inline double bitwise_xor(double const& x, double const& y)
	{
		long long mask=-(is_real(x)&is_real(y));
		return (convert_d2ll(x)^convert_d2ll(y))&mask;
	}
	double r_r_bitwise_xor(double const& x)
	{
		return bitwise_xor(x);
	}
	Comp1d c_c_bitwise_xor(Comp1d const& x)
	{
		return Comp1d(bitwise_xor(x.r), bitwise_xor(x.i));
	}
	Quat1d q_q_bitwise_xor(Quat1d const& x)
	{
		return Quat1d(bitwise_xor(x.r), bitwise_xor(x.i), bitwise_xor(x.j), bitwise_xor(x.k));
	}
	double r_rr_bitwise_xor(double const& x, double const& y)
	{
		return bitwise_xor(x, y);
	}
	Comp1d c_rc_bitwise_xor(double const& x, Comp1d const& y)
	{
		return Comp1d(bitwise_xor(x, y.r), y.i);
	}
	Quat1d q_rq_bitwise_xor(double const& x, Quat1d const& y)
	{
		return Quat1d(bitwise_xor(x, y.r), y.i, y.j, y.k);
	}
	Comp1d c_cr_bitwise_xor(Comp1d const& x, double const& y)
	{
		return Comp1d(bitwise_xor(x.r, y), x.i);
	}
	Comp1d c_cc_bitwise_xor(Comp1d const& x, Comp1d const& y)
	{
		return Comp1d(bitwise_xor(x.r, y.r), bitwise_xor(x.i, y.i));
	}
	Quat1d q_cq_bitwise_xor(Comp1d const& x, Quat1d const& y)
	{
		return Quat1d(bitwise_xor(x.r, y.r), bitwise_xor(x.i, y.i), y.j, y.k);
	}
	Quat1d q_qr_bitwise_xor(Quat1d const& x, double const& y)
	{
		return Quat1d(bitwise_xor(x.r, y), x.i, x.j, x.k);
	}
	Quat1d q_qc_bitwise_xor(Quat1d const& x, Comp1d const& y)
	{
		return Quat1d(bitwise_xor(x.r, y.r), bitwise_xor(x.i, y.i), x.j, x.k);
	}
	Quat1d q_qq_bitwise_xor(Quat1d const& x, Quat1d const& y)
	{
		return Quat1d(bitwise_xor(x.r, y.r), bitwise_xor(x.i, y.i), bitwise_xor(x.j, y.j), bitwise_xor(x.k, y.k));
	}
	auto disc_r_bitwise_xor_o=disc_r_bitwise_and_o;
	auto disc_c_bitwise_xor_o=disc_c_bitwise_and_o;
	auto disc_q_bitwise_xor_o=disc_q_bitwise_and_o;
	//auto disc_rr_bitwise_xor_o=disc_rr_bitwise_and_o;
	//auto disc_rc_bitwise_xor_o=disc_rc_bitwise_and_o;
	//auto disc_rq_bitwise_xor_o=disc_rq_bitwise_and_o;
	//auto disc_cr_bitwise_xor_o=disc_cr_bitwise_and_o;
	//auto disc_cc_bitwise_xor_o=disc_cc_bitwise_and_o;
	//auto disc_cq_bitwise_xor_o=disc_cq_bitwise_and_o;
	//auto disc_qr_bitwise_xor_o=disc_qr_bitwise_and_o;
	//auto disc_qc_bitwise_xor_o=disc_qc_bitwise_and_o;
	//auto disc_qq_bitwise_xor_o=disc_qq_bitwise_and_o;

	inline double bitwise_xnor(double const& x)
	{
		long long mask=-is_real(x);
		return (!bitwise_xor(x))&mask;
	}
	inline double bitwise_xnor(double const& x, double const& y)
	{
		long long mask=-(is_real(x)&is_real(y));
		return ~(convert_d2ll(x)^convert_d2ll(y))&mask;
	}
	double r_r_bitwise_xnor(double const& x){return bitwise_xnor(x);}
	Comp1d c_c_bitwise_xnor(Comp1d const& x){return Comp1d(bitwise_xnor(x.r), bitwise_xnor(x.i));}
	Quat1d q_q_bitwise_xnor(Quat1d const& x){return Quat1d(bitwise_xnor(x.r), bitwise_xnor(x.i), bitwise_xnor(x.j), bitwise_xnor(x.k));}
	double r_rr_bitwise_xnor(double const& x, double const& y){return bitwise_xnor(x, y);}
	Comp1d c_rc_bitwise_xnor(double const& x, Comp1d const& y){return Comp1d(bitwise_xnor(x, y.r), y.i);}
	Quat1d q_rq_bitwise_xnor(double const& x, Quat1d const& y){return Quat1d(bitwise_xnor(x, y.r), y.i, y.j, y.k);}
	Comp1d c_cr_bitwise_xnor(Comp1d const& x, double const& y){return Comp1d(bitwise_xnor(x.r, y), x.i);}
	Comp1d c_cc_bitwise_xnor(Comp1d const& x, Comp1d const& y){return Comp1d(bitwise_xnor(x.r, y.r), bitwise_xnor(x.i, y.i));}
	Quat1d q_cq_bitwise_xnor(Comp1d const& x, Quat1d const& y){return Quat1d(bitwise_xnor(x.r, y.r), bitwise_xnor(x.i, y.i), y.j, y.k);}
	Quat1d q_qr_bitwise_xnor(Quat1d const& x, double const& y){return Quat1d(bitwise_xnor(x.r, y), x.i, x.j, x.k);}
	Quat1d q_qc_bitwise_xnor(Quat1d const& x, Comp1d const& y){return Quat1d(bitwise_xnor(x.r, y.r), bitwise_xnor(x.i, y.i), x.j, x.k);}
	Quat1d q_qq_bitwise_xnor(Quat1d const& x, Quat1d const& y){return Quat1d(bitwise_xnor(x.r, y.r), bitwise_xnor(x.i, y.i), bitwise_xnor(x.j, y.j), bitwise_xnor(x.k, y.k));}
	auto disc_r_bitwise_xnor_o=disc_r_bitwise_and_o;
	auto disc_c_bitwise_xnor_o=disc_c_bitwise_and_o;
	auto disc_q_bitwise_xnor_o=disc_q_bitwise_and_o;
	//auto disc_rr_bitwise_xnor_o=disc_rr_bitwise_and_o;
	//auto disc_rc_bitwise_xnor_o=disc_rc_bitwise_and_o;
	//auto disc_rq_bitwise_xnor_o=disc_rq_bitwise_and_o;
	//auto disc_cr_bitwise_xnor_o=disc_cr_bitwise_and_o;
	//auto disc_cc_bitwise_xnor_o=disc_cc_bitwise_and_o;
	//auto disc_cq_bitwise_xnor_o=disc_cq_bitwise_and_o;
	//auto disc_qr_bitwise_xnor_o=disc_qr_bitwise_and_o;
	//auto disc_qc_bitwise_xnor_o=disc_qc_bitwise_and_o;
	//auto disc_qq_bitwise_xnor_o=disc_qq_bitwise_and_o;

	double r_r_logic_equal(double const& x){return x==0;}
	double r_c_logic_equal(Comp1d const& x){return !x.c_is_true();}
	double r_q_logic_equal(Quat1d const& x){return !x.q_is_true();}
	double r_rr_logic_equal(double const& x, double const& y){return x==y;}
	double r_rc_logic_equal(double const& x, Comp1d const& y){return x==y;}
	double r_rq_logic_equal(double const& x, Quat1d const& y){return x==y;}
	double r_cr_logic_equal(Comp1d const& x, double const& y){return x==y;}
	double r_cc_logic_equal(Comp1d const& x, Comp1d const& y){return x==y;}
	double r_cq_logic_equal(Comp1d const& x, Quat1d const& y){return x==y;}
	double r_qr_logic_equal(Quat1d const& x, double const& y){return x==y;}
	double r_qc_logic_equal(Quat1d const& x, Comp1d const& y){return x==y;}
	double r_qq_logic_equal(Quat1d const& x, Quat1d const& y){return x==y;}
	auto disc_r_logic_equal_o		=disc_r_logic_divides_o;
	//bool disc_x_logic_equal(double x0, double x1)
	//{
	//	if(x0<0)
	//		return x1>=0;
	//	if(x0==0)
	//		return x1<0||x1>0;
	//	return x1<=0;
	//}
	//bool disc_r_logic_equal_i(Value const& x0, Value const& x1){return disc_x_logic_equal(x0.r, x1.r);}
	//bool disc_c_logic_equal_i(Value const& x0, Value const& x1){return disc_x_logic_equal(x0.r, x1.r)&&disc_x_logic_equal(x0.i, x1.i);}
	//bool disc_q_logic_equal_i(Value const& x0, Value const& x1){return disc_x_logic_equal(x0.r, x1.r)&&disc_x_logic_equal(x0.i, x1.i)&&disc_x_logic_equal(x0.j, x1.j)&&disc_x_logic_equal(x0.k, x1.k);}
	//bool disc_rr_logic_equal_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return disc_x_logic_equal(x0.r-y0.r, x1.r-y1.r);}
	//bool disc_rc_logic_equal_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return disc_x_logic_equal(x0.r-y0.r, x1.r-y1.r)&&disc_x_logic_equal(-y0.i, -y1.i);}
	//bool disc_rq_logic_equal_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	//{
	//	return disc_x_logic_equal(x0.r-y0.r, x1.r-y1.r)&&disc_x_logic_equal(-y0.i, -y1.i)&&disc_x_logic_equal(-y0.j, -y1.j)&&disc_x_logic_equal(-y0.k, -y1.k);
	//}
	//bool disc_cr_logic_equal_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	//{
	//	return disc_x_logic_equal(x0.r-y0.r, x1.r-y1.r)&&disc_x_logic_equal(x0.i, x1.i);
	//}
	//bool disc_cc_logic_equal_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	//{
	//	return disc_x_logic_equal(x0.r-y0.r, x1.r-y1.r)&&disc_x_logic_equal(x0.i-y0.i, x1.i-y1.i);
	//}
	//bool disc_cq_logic_equal_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	//{
	//	return disc_x_logic_equal(x0.r-y0.r, x1.r-y1.r)&&disc_x_logic_equal(x0.i-y0.i, x1.i-y1.i)&&
	//			disc_x_logic_equal(-y0.j, -y1.j)&&disc_x_logic_equal(-y0.k, -y1.k);
	//}
	//bool disc_qr_logic_equal_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	//{
	//	return disc_x_logic_equal(x0.r-y0.r, x1.r-y1.r)&&disc_x_logic_equal(x0.i, x1.i)&&
	//			disc_x_logic_equal(x0.j, x1.j)&&disc_x_logic_equal(x0.k, x1.k);
	//}
	//bool disc_qc_logic_equal_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	//{
	//	return disc_x_logic_equal(x0.r-y0.r, x1.r-y1.r)&&disc_x_logic_equal(x0.i-y0.i, x1.i-y1.i)&&
	//			disc_x_logic_equal(x0.j, x1.j)&&disc_x_logic_equal(x0.k, x1.k);
	//}
	//bool disc_qq_logic_equal_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	//{
	//	return disc_x_logic_equal(x0.r-y0.r, x1.r-y1.r)&&disc_x_logic_equal(x0.i-y0.i, x1.i-y1.i)&&
	//			disc_x_logic_equal(x0.j-y0.j, x1.j-y1.j)&&disc_x_logic_equal(x0.k-y0.k, x1.k-y1.k);
	//}

	double r_r_logic_not_equal(double const& x){return x!=0;}
	double r_c_logic_not_equal(Comp1d const& x){return x.c_is_true();}
	double r_q_logic_not_equal(Quat1d const& x){return x.q_is_true();}
	double r_rr_logic_not_equal(double const& x, double const& y){return x!=y;}
	double r_rc_logic_not_equal(double const& x, Comp1d const& y){return x!=y;}
	double r_rq_logic_not_equal(double const& x, Quat1d const& y){return x!=y;}
	double r_cr_logic_not_equal(Comp1d const& x, double const& y){return x!=y;}
	double r_cc_logic_not_equal(Comp1d const& x, Comp1d const& y){return x!=y;}
	double r_cq_logic_not_equal(Comp1d const& x, Quat1d const& y){return x!=y;}
	double r_qr_logic_not_equal(Quat1d const& x, double const& y){return x!=y;}
	double r_qc_logic_not_equal(Quat1d const& x, Comp1d const& y){return x!=y;}
	double r_qq_logic_not_equal(Quat1d const& x, Quat1d const& y){return x!=y;}
	auto disc_r_logic_not_equal_o		=disc_r_logic_divides_o;
	//auto disc_r_logic_not_equal_i=disc_r_logic_equal_i;
	//auto disc_c_logic_not_equal_i=disc_c_logic_equal_i;
	//auto disc_q_logic_not_equal_i=disc_q_logic_equal_i;
	//auto disc_rr_logic_not_equal_i=disc_rr_logic_equal_i;
	//auto disc_rc_logic_not_equal_i=disc_rc_logic_equal_i;
	//auto disc_rq_logic_not_equal_i=disc_rq_logic_equal_i;
	//auto disc_cr_logic_not_equal_i=disc_cr_logic_equal_i;
	//auto disc_cc_logic_not_equal_i=disc_cc_logic_equal_i;
	//auto disc_cq_logic_not_equal_i=disc_cq_logic_equal_i;
	//auto disc_qr_logic_not_equal_i=disc_qr_logic_equal_i;
	//auto disc_qc_logic_not_equal_i=disc_qc_logic_equal_i;
	//auto disc_qq_logic_not_equal_i=disc_qq_logic_equal_i;

	double r_r_logic_less_l(double const& x){return 0<x;}
	double r_c_logic_less_l(Comp1d const& x){return 0<x.r;}
	double r_q_logic_less_l(Quat1d const& x){return 0<x.r;}
	double r_r_logic_less_r(double const& x){return x<0;}
	double r_c_logic_less_r(Comp1d const& x){return x.r<0;}
	double r_q_logic_less_r(Quat1d const& x){return x.r<0;}
	double r_rr_logic_less(double const& x, double const& y){return x<y;}
	double r_rc_logic_less(double const& x, Comp1d const& y){return x<y.r;}
	double r_rq_logic_less(double const& x, Quat1d const& y){return x<y.r;}
	double r_cr_logic_less(Comp1d const& x, double const& y){return x.r<y;}
	double r_cc_logic_less(Comp1d const& x, Comp1d const& y){return x.r<y.r;}
	double r_cq_logic_less(Comp1d const& x, Quat1d const& y){return x.r<y.r;}
	double r_qr_logic_less(Quat1d const& x, double const& y){return x.r<y;}
	double r_qc_logic_less(Quat1d const& x, Comp1d const& y){return x.r<y.r;}
	double r_qq_logic_less(Quat1d const& x, Quat1d const& y){return x.r<y.r;}
	auto disc_r_logic_less_o		=disc_r_logic_divides_o;
	//bool disc_x_logic_less(double x0, double x1){return x0<0?x1>=0:x1<0;}//	__|0__
	//bool disc_r_logic_less_l_i(Value const& x0, Value const& x1){return disc_x_logic_less(-x0.r, -x1.r);}
	//auto disc_c_logic_less_l_i=disc_r_logic_less_l_i;
	//auto disc_q_logic_less_l_i=disc_r_logic_less_l_i;
	//bool disc_r_logic_less_r_i(Value const& x0, Value const& x1){return disc_x_logic_less(x0.r, x1.r);}
	//auto disc_c_logic_less_r_i=disc_r_logic_less_r_i;
	//auto disc_q_logic_less_r_i=disc_r_logic_less_r_i;
	//bool disc_rr_logic_less_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return disc_x_logic_less(x0.r-y0.r, x1.r-y1.r);}
	//auto disc_rc_logic_less_i=disc_rr_logic_less_i;
	//auto disc_rq_logic_less_i=disc_rr_logic_less_i;
	//auto disc_cr_logic_less_i=disc_rr_logic_less_i;
	//auto disc_cc_logic_less_i=disc_rr_logic_less_i;
	//auto disc_cq_logic_less_i=disc_rr_logic_less_i;
	//auto disc_qr_logic_less_i=disc_rr_logic_less_i;
	//auto disc_qc_logic_less_i=disc_rr_logic_less_i;
	//auto disc_qq_logic_less_i=disc_rr_logic_less_i;

	double r_r_logic_less_equal_l(double const& x){return 0<=x;}
	double r_c_logic_less_equal_l(Comp1d const& x){return 0<=x.r;}
	double r_q_logic_less_equal_l(Quat1d const& x){return 0<=x.r;}
	double r_r_logic_less_equal_r(double const& x){return x<=0;}
	double r_c_logic_less_equal_r(Comp1d const& x){return x.r<=0;}
	double r_q_logic_less_equal_r(Quat1d const& x){return x.r<=0;}
	double r_rr_logic_less_equal(double const& x, double const& y){return x<=y;}
	double r_rc_logic_less_equal(double const& x, Comp1d const& y){return x<=y.r;}
	double r_rq_logic_less_equal(double const& x, Quat1d const& y){return x<=y.r;}
	double r_cr_logic_less_equal(Comp1d const& x, double const& y){return x.r<=y;}
	double r_cc_logic_less_equal(Comp1d const& x, Comp1d const& y){return x.r<=y.r;}
	double r_cq_logic_less_equal(Comp1d const& x, Quat1d const& y){return x.r<=y.r;}
	double r_qr_logic_less_equal(Quat1d const& x, double const& y){return x.r<=y;}
	double r_qc_logic_less_equal(Quat1d const& x, Comp1d const& y){return x.r<=y.r;}
	double r_qq_logic_less_equal(Quat1d const& x, Quat1d const& y){return x.r<=y.r;}
	auto disc_r_logic_less_equal_o		=disc_r_logic_divides_o;
	//bool disc_x_logic_less_equal(double x0, double x1){return x0<=0?x1>0:x1<=0;}//	__0|__
	//bool disc_r_logic_less_equal_l_i(Value const& x0, Value const& x1){return disc_x_logic_less_equal(-x0.r, -x1.r);}
	//auto disc_c_logic_less_equal_l_i=disc_r_logic_less_equal_l_i;
	//auto disc_q_logic_less_equal_l_i=disc_r_logic_less_equal_l_i;
	//bool disc_r_logic_less_equal_r_i(Value const& x0, Value const& x1){return disc_x_logic_less_equal(x0.r, x1.r);}
	//auto disc_c_logic_less_equal_r_i=disc_r_logic_less_equal_r_i;
	//auto disc_q_logic_less_equal_r_i=disc_r_logic_less_equal_r_i;
	//bool disc_rr_logic_less_equal_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return disc_r_logic_less_equal_r_i(x0.r-y0.r, x1.r-y1.r);}
	//auto disc_rc_logic_less_equal_i=disc_rr_logic_less_equal_i;
	//auto disc_rq_logic_less_equal_i=disc_rr_logic_less_equal_i;
	//auto disc_cr_logic_less_equal_i=disc_rr_logic_less_equal_i;
	//auto disc_cc_logic_less_equal_i=disc_rr_logic_less_equal_i;
	//auto disc_cq_logic_less_equal_i=disc_rr_logic_less_equal_i;
	//auto disc_qr_logic_less_equal_i=disc_rr_logic_less_equal_i;
	//auto disc_qc_logic_less_equal_i=disc_rr_logic_less_equal_i;
	//auto disc_qq_logic_less_equal_i=disc_rr_logic_less_equal_i;

	double r_r_logic_greater_l(double const& x){return 0>x;}
	double r_c_logic_greater_l(Comp1d const& x){return 0>x.r;}
	double r_q_logic_greater_l(Quat1d const& x){return 0>x.r;}
	double r_r_logic_greater_r(double const& x){return x>0;}
	double r_c_logic_greater_r(Comp1d const& x){return x.r>0;}
	double r_q_logic_greater_r(Quat1d const& x){return x.r>0;}
	double r_rr_logic_greater(double const& x, double const& y){return x>y;}
	double r_rc_logic_greater(double const& x, Comp1d const& y){return x>y.r;}
	double r_rq_logic_greater(double const& x, Quat1d const& y){return x>y.r;}
	double r_cr_logic_greater(Comp1d const& x, double const& y){return x.r>y;}
	double r_cc_logic_greater(Comp1d const& x, Comp1d const& y){return x.r>y.r;}
	double r_cq_logic_greater(Comp1d const& x, Quat1d const& y){return x.r>y.r;}
	double r_qr_logic_greater(Quat1d const& x, double const& y){return x.r>y;}
	double r_qc_logic_greater(Quat1d const& x, Comp1d const& y){return x.r>y.r;}
	double r_qq_logic_greater(Quat1d const& x, Quat1d const& y){return x.r>y.r;}
	auto disc_r_logic_greater_o		=disc_r_logic_divides_o;
	//auto disc_r_logic_greater_l_i=disc_r_logic_less_equal_l_i;
	//auto disc_c_logic_greater_l_i=disc_c_logic_less_equal_l_i;
	//auto disc_q_logic_greater_l_i=disc_q_logic_less_equal_l_i;
	//auto disc_r_logic_greater_r_i=disc_r_logic_less_equal_r_i;
	//auto disc_c_logic_greater_r_i=disc_c_logic_less_equal_r_i;
	//auto disc_q_logic_greater_r_i=disc_q_logic_less_equal_r_i;
	//auto disc_rr_logic_greater_i=disc_rr_logic_less_equal_i;
	//auto disc_rc_logic_greater_i=disc_rc_logic_less_equal_i;
	//auto disc_rq_logic_greater_i=disc_rq_logic_less_equal_i;
	//auto disc_cr_logic_greater_i=disc_cr_logic_less_equal_i;
	//auto disc_cc_logic_greater_i=disc_cc_logic_less_equal_i;
	//auto disc_cq_logic_greater_i=disc_cq_logic_less_equal_i;
	//auto disc_qr_logic_greater_i=disc_qr_logic_less_equal_i;
	//auto disc_qc_logic_greater_i=disc_qc_logic_less_equal_i;
	//auto disc_qq_logic_greater_i=disc_qq_logic_less_equal_i;

	double r_r_logic_greater_equal_l(double const& x){return 0>=x;}
	double r_c_logic_greater_equal_l(Comp1d const& x){return 0>=x.r;}
	double r_q_logic_greater_equal_l(Quat1d const& x){return 0>=x.r;}
	double r_r_logic_greater_equal_r(double const& x){return x>=0;}
	double r_c_logic_greater_equal_r(Comp1d const& x){return x.r>=0;}
	double r_q_logic_greater_equal_r(Quat1d const& x){return x.r>=0;}
	double r_rr_logic_greater_equal(double const& x, double const& y){return x>=y;}
	double r_rc_logic_greater_equal(double const& x, Comp1d const& y){return x>=y.r;}
	double r_rq_logic_greater_equal(double const& x, Quat1d const& y){return x>=y.r;}
	double r_cr_logic_greater_equal(Comp1d const& x, double const& y){return x.r>=y;}
	double r_cc_logic_greater_equal(Comp1d const& x, Comp1d const& y){return x.r>=y.r;}
	double r_cq_logic_greater_equal(Comp1d const& x, Quat1d const& y){return x.r>=y.r;}
	double r_qr_logic_greater_equal(Quat1d const& x, double const& y){return x.r>=y;}
	double r_qc_logic_greater_equal(Quat1d const& x, Comp1d const& y){return x.r>=y.r;}
	double r_qq_logic_greater_equal(Quat1d const& x, Quat1d const& y){return x.r>=y.r;}
	auto disc_r_logic_greater_equal_o		=disc_r_logic_divides_o;
	//auto disc_r_logic_greater_equal_l_i=disc_r_logic_less_l_i;
	//auto disc_c_logic_greater_equal_l_i=disc_c_logic_less_l_i;
	//auto disc_q_logic_greater_equal_l_i=disc_q_logic_less_l_i;
	//auto disc_r_logic_greater_equal_r_i=disc_r_logic_less_r_i;
	//auto disc_c_logic_greater_equal_r_i=disc_c_logic_less_r_i;
	//auto disc_q_logic_greater_equal_r_i=disc_q_logic_less_r_i;
	//auto disc_rr_logic_greater_equal_i=disc_rr_logic_less_i;
	//auto disc_rc_logic_greater_equal_i=disc_rc_logic_less_i;
	//auto disc_rq_logic_greater_equal_i=disc_rq_logic_less_i;
	//auto disc_cr_logic_greater_equal_i=disc_cr_logic_less_i;
	//auto disc_cc_logic_greater_equal_i=disc_cc_logic_less_i;
	//auto disc_cq_logic_greater_equal_i=disc_cq_logic_less_i;
	//auto disc_qr_logic_greater_equal_i=disc_qr_logic_less_i;
	//auto disc_qc_logic_greater_equal_i=disc_qc_logic_less_i;
	//auto disc_qq_logic_greater_equal_i=disc_qq_logic_less_i;

	double r_r_logic_not(double const& x){return x==0;}
	double r_c_logic_not(Comp1d const& x){return (x.r==0)&(x.i==0);}
	double r_q_logic_not(Quat1d const& x){return (x.r==0)&(x.i==0)&(x.j==0)&(x.k==0);}
	auto disc_r_logic_not_o		=disc_r_logic_divides_o;
	//auto disc_r_logic_not_i=disc_r_divide_i;
	//auto disc_c_logic_not_i=disc_c_divide_i;
	//auto disc_q_logic_not_i=disc_q_divide_i;

	double r_rr_logic_and(double const& x, double const& y){return x!=0&y!=0;}
	double r_rc_logic_and(double const& x, Comp1d const& y){return (x!=0)&y.c_is_true();}
	double r_rq_logic_and(double const& x, Quat1d const& y){return (x!=0)&y.q_is_true();}
	double r_cr_logic_and(Comp1d const& x, double const& y){return x.c_is_true()&(y!=0);}
	double r_cc_logic_and(Comp1d const& x, Comp1d const& y){return x.c_is_true()&y.c_is_true();}
	double r_cq_logic_and(Comp1d const& x, Quat1d const& y){return x.c_is_true()&y.q_is_true();}
	double r_qr_logic_and(Quat1d const& x, double const& y){return x.q_is_true()&(y!=0);}
	double r_qc_logic_and(Quat1d const& x, Comp1d const& y){return x.q_is_true()&y.c_is_true();}
	double r_qq_logic_and(Quat1d const& x, Quat1d const& y){return x.q_is_true()&y.q_is_true();}
	auto disc_r_logic_and_o		=disc_r_logic_divides_o;
	//bool disc_x_logic_and(double x0, double x1)
	//{
	//	if(x0<0)
	//		return x1>=0;
	//	if(x0==0)
	//		return x1<0||x1>0;
	//	return x1<=0;
	//}
	//bool disc_rr_logic_and_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	//{
	//	return disc_x_logic_and(x0.r, x1.r)||disc_x_logic_and(y0.r, y1.r);
	//}
	//bool disc_rc_logic_and_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	//{
	//	return disc_x_logic_and(x0.r, x1.r)||(disc_x_logic_and(y0.r, y1.r)&&disc_x_logic_and(y0.i, y1.i));
	//}
	//bool disc_rq_logic_and_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	//{
	//	return disc_x_logic_and(x0.r, x1.r)||
	//		(disc_x_logic_and(y0.r, y1.r)&&disc_x_logic_and(y0.i, y1.i)&&disc_x_logic_and(y0.j, y1.j)&&disc_x_logic_and(y0.k, y1.k));
	//}
	//bool disc_cr_logic_and_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	//{
	//	return (disc_x_logic_and(x0.r, x1.r)&&disc_x_logic_and(x0.i, x1.i))||
	//			disc_x_logic_and(y0.r, y1.r);
	//}
	//bool disc_cc_logic_and_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	//{
	//	return (disc_x_logic_and(x0.r, x1.r)&&disc_x_logic_and(x0.i, x1.i))||
	//			(disc_x_logic_and(y0.r, y1.r)&&disc_x_logic_and(y0.i, y1.i));
	//}
	//bool disc_cq_logic_and_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	//{
	//	return (disc_x_logic_and(x0.r, x1.r)&&disc_x_logic_and(x0.i, x1.i))||
	//			(disc_x_logic_and(y0.r, y1.r)&&disc_x_logic_and(y0.i, y1.i)&&disc_x_logic_and(y0.j, y1.j)&&disc_x_logic_and(y0.k, y1.k));
	//}
	//bool disc_qr_logic_and_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	//{
	//	return (disc_x_logic_and(x0.r, x1.r)&&disc_x_logic_and(x0.i, x1.i)&&disc_x_logic_and(x0.j, x1.j)&&disc_x_logic_and(x0.k, x1.k))||
	//			disc_x_logic_and(y0.r, y1.r);
	//}
	//bool disc_qc_logic_and_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	//{
	//	return (disc_x_logic_and(x0.r, x1.r)&&disc_x_logic_and(x0.i, x1.i)&&disc_x_logic_and(x0.j, x1.j)&&disc_x_logic_and(x0.k, x1.k))||
	//			(disc_x_logic_and(y0.r, y1.r)&&disc_x_logic_and(y0.i, y1.i));
	//}
	//bool disc_qq_logic_and_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	//{
	//	return (disc_x_logic_and(x0.r, x1.r)&&disc_x_logic_and(x0.i, x1.i)&&disc_x_logic_and(x0.j, x1.j)&&disc_x_logic_and(x0.k, x1.k))||
	//			(disc_x_logic_and(y0.r, y1.r)&&disc_x_logic_and(y0.i, y1.i)&&disc_x_logic_and(y0.j, y1.j)&&disc_x_logic_and(y0.k, y1.k));
	//}

	double r_rr_logic_or(double const& x, double const& y){return x!=0|y!=0;}
	double r_rc_logic_or(double const& x, Comp1d const& y){return (x!=0)|y.c_is_true();}
	double r_rq_logic_or(double const& x, Quat1d const& y){return (x!=0)|y.q_is_true();}
	double r_cr_logic_or(Comp1d const& x, double const& y){return x.c_is_true()|(y!=0);}
	double r_cc_logic_or(Comp1d const& x, Comp1d const& y){return x.c_is_true()|y.c_is_true();}
	double r_cq_logic_or(Comp1d const& x, Quat1d const& y){return x.c_is_true()|y.q_is_true();}
	double r_qr_logic_or(Quat1d const& x, double const& y){return x.q_is_true()|(y!=0);}
	double r_qc_logic_or(Quat1d const& x, Comp1d const& y){return x.q_is_true()|y.c_is_true();}
	double r_qq_logic_or(Quat1d const& x, Quat1d const& y){return x.q_is_true()|y.q_is_true();}
	auto disc_r_logic_or_o		=disc_r_logic_divides_o;
	//bool disc_rr_logic_or_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	//{
	//	return disc_x_logic_and(x0.r, x1.r)&&disc_x_logic_and(y0.r, y1.r);
	//}
	//bool disc_rc_logic_or_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	//{
	//	return disc_x_logic_and(x0.r, x1.r)&&disc_x_logic_and(y0.r, y1.r)&&
	//			disc_x_logic_and(y0.i, y1.i);
	//}
	//bool disc_rq_logic_or_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	//{
	//	return disc_x_logic_and(x0.r, x1.r)&&disc_x_logic_and(y0.r, y1.r)&&
	//			disc_x_logic_and(y0.i, y1.i)&&disc_x_logic_and(y0.j, y1.j)&&
	//			disc_x_logic_and(y0.k, y1.k);
	//}
	//bool disc_cr_logic_or_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	//{
	//	return disc_x_logic_and(x0.r, x1.r)&&disc_x_logic_and(x0.i, x1.i)&&
	//			disc_x_logic_and(y0.r, y1.r);
	//}
	//bool disc_cc_logic_or_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	//{
	//	return disc_x_logic_and(x0.r, x1.r)&&disc_x_logic_and(x0.i, x1.i)&&
	//			disc_x_logic_and(y0.r, y1.r)&&disc_x_logic_and(y0.i, y1.i);
	//}
	//bool disc_cq_logic_or_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	//{
	//	return disc_x_logic_and(x0.r, x1.r)&&disc_x_logic_and(x0.i, x1.i)&&
	//			disc_x_logic_and(y0.r, y1.r)&&disc_x_logic_and(y0.i, y1.i)&&
	//			disc_x_logic_and(y0.j, y1.j)&&disc_x_logic_and(y0.k, y1.k);
	//}
	//bool disc_qr_logic_or_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	//{
	//	return disc_x_logic_and(x0.r, x1.r)&&disc_x_logic_and(x0.i, x1.i)&&
	//			disc_x_logic_and(x0.j, x1.j)&&disc_x_logic_and(x0.k, x1.k)&&
	//			disc_x_logic_and(y0.r, y1.r);
	//}
	//bool disc_qc_logic_or_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	//{
	//	return disc_x_logic_and(x0.r, x1.r)&&disc_x_logic_and(x0.i, x1.i)&&
	//			disc_x_logic_and(x0.j, x1.j)&&disc_x_logic_and(x0.k, x1.k)&&
	//			disc_x_logic_and(y0.r, y1.r)&&disc_x_logic_and(y0.i, y1.i);
	//}
	//bool disc_qq_logic_or_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1)
	//{
	//	return disc_x_logic_and(x0.r, x1.r)&&disc_x_logic_and(x0.i, x1.i)&&
	//			disc_x_logic_and(x0.j, x1.j)&&disc_x_logic_and(x0.k, x1.k)&&
	//			disc_x_logic_and(y0.r, y1.r)&&disc_x_logic_and(y0.i, y1.i)&&
	//			disc_x_logic_and(y0.j, y1.j)&&disc_x_logic_and(y0.k, y1.k);
	//}

	double r_rr_logic_xor(double const& x, double const& y){return x!=0^y!=0;}
	double r_rc_logic_xor(double const& x, Comp1d const& y){return (x!=0)^y.c_is_true();}
	double r_rq_logic_xor(double const& x, Quat1d const& y){return (x!=0)^y.q_is_true();}
	double r_cr_logic_xor(Comp1d const& x, double const& y){return x.c_is_true()^(y!=0);}
	double r_cc_logic_xor(Comp1d const& x, Comp1d const& y){return x.c_is_true()^y.c_is_true();}
	double r_cq_logic_xor(Comp1d const& x, Quat1d const& y){return x.c_is_true()^y.q_is_true();}
	double r_qr_logic_xor(Quat1d const& x, double const& y){return x.q_is_true()^(y!=0);}
	double r_qc_logic_xor(Quat1d const& x, Comp1d const& y){return x.q_is_true()^y.c_is_true();}
	double r_qq_logic_xor(Quat1d const& x, Quat1d const& y){return x.q_is_true()^y.q_is_true();}
	auto disc_r_logic_xor_o		=disc_r_logic_divides_o;
	//auto disc_rr_logic_xor_i=disc_rr_logic_and_i;
	//auto disc_rc_logic_xor_i=disc_rc_logic_and_i;
	//auto disc_rq_logic_xor_i=disc_rq_logic_and_i;
	//auto disc_cr_logic_xor_i=disc_cr_logic_and_i;
	//auto disc_cc_logic_xor_i=disc_cc_logic_and_i;
	//auto disc_cq_logic_xor_i=disc_cq_logic_and_i;
	//auto disc_qr_logic_xor_i=disc_qr_logic_and_i;
	//auto disc_qc_logic_xor_i=disc_qc_logic_and_i;
	//auto disc_qq_logic_xor_i=disc_qq_logic_and_i;

	double r_rr_condition_zero(double const& x, double const& y){return x?x:y;}
	Comp1d c_rc_condition_zero(double const& x, Comp1d const& y){return x?x:y;}
	Quat1d q_rq_condition_zero(double const& x, Quat1d const& y){return x?Quat1d(x):y;}
	Comp1d c_cr_condition_zero(Comp1d const& x, double const& y){return x.c_is_true()?x:Comp1d(y);}
	Comp1d c_cc_condition_zero(Comp1d const& x, Comp1d const& y){return x.c_is_true()?x:y;}
	Quat1d q_cq_condition_zero(Comp1d const& x, Quat1d const& y){return x.c_is_true()?Quat1d(x):y;}
	Quat1d q_qr_condition_zero(Quat1d const& x, double const& y){return x.q_is_true()?x:Quat1d(y);}
	Quat1d q_qc_condition_zero(Quat1d const& x, Comp1d const& y){return x.q_is_true()?x:Quat1d(y);}
	Quat1d q_qq_condition_zero(Quat1d const& x, Quat1d const& y){return x.q_is_true()?x:y;}
	bool disc_rr_condition_zero_i	(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return _1d_zero_in_range(x0.r, x1.r);}
	bool disc_rc_condition_zero_i	(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return _1d_zero_in_range(x0.r, x1.r);}
	bool disc_rq_condition_zero_i	(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return _1d_zero_in_range(x0.r, x1.r);}
	bool disc_cr_condition_zero_i	(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return _1d_zero_in_range(x0.r, x1.r)&&_1d_zero_in_range(x0.i, x1.i);}
	bool disc_cc_condition_zero_i	(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return _1d_zero_in_range(x0.r, x1.r)&&_1d_zero_in_range(x0.i, x1.i);}
	bool disc_cq_condition_zero_i	(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return _1d_zero_in_range(x0.r, x1.r)&&_1d_zero_in_range(x0.i, x1.i);}
	bool disc_qr_condition_zero_i	(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return _1d_zero_in_range(x0.r, x1.r)&&_1d_zero_in_range(x0.i, x1.i)&&_1d_zero_in_range(x0.j, x1.j)&&_1d_zero_in_range(x0.k, x1.k);}
	bool disc_qc_condition_zero_i	(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return _1d_zero_in_range(x0.r, x1.r)&&_1d_zero_in_range(x0.i, x1.i)&&_1d_zero_in_range(x0.j, x1.j)&&_1d_zero_in_range(x0.k, x1.k);}
	bool disc_qq_condition_zero_i	(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return _1d_zero_in_range(x0.r, x1.r)&&_1d_zero_in_range(x0.i, x1.i)&&_1d_zero_in_range(x0.j, x1.j)&&_1d_zero_in_range(x0.k, x1.k);}

	double r_r_percent(double const& x){return x*0.01;}
	Comp1d c_c_percent(Comp1d const& x){return x*0.01;}
	Quat1d q_q_percent(Quat1d const& x){return x*0.01;}

	double r_rr_modulo(double const& x, double const& y){return fmod(x, y);}//
	Comp1d c_rc_modulo(double const& x, Comp1d const& y){return x%y;}
	Quat1d q_rq_modulo(double const& x, Quat1d const& y){return x%y;}
	Comp1d c_cr_modulo(Comp1d const& x, double const& y){return x%y;}
	Comp1d c_cc_modulo(Comp1d const& x, Comp1d const& y){return x%y;}
	Quat1d q_cq_modulo(Comp1d const& x, Quat1d const& y){return x%y;}
	Quat1d q_qr_modulo(Quat1d const& x, double const& y){return x%y;}
	Quat1d q_qc_modulo(Quat1d const& x, Comp1d const& y){return x%y;}
	Quat1d q_qq_modulo(Quat1d const& x, Quat1d const& y){return x%y;}
	bool disc_rr_modulo_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return std::floor(x0.r/y0.r)!=std::floor(x1.r/y1.r);}
	bool disc_rc_modulo_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return false;}//
	bool disc_rq_modulo_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return false;}//
	bool disc_cr_modulo_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return false;}//
	bool disc_cc_modulo_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return false;}//
	bool disc_cq_modulo_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return false;}//
	bool disc_qr_modulo_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return false;}//
	bool disc_qc_modulo_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return false;}//
	bool disc_qq_modulo_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return false;}//

	double r_r_sgn(double const& x){return (x>0)-(x<0);}
	Comp1d c_c_sgn(Comp1d const& x)
	{
		double mag=x.abs();
		return AND(x/mag, -(mag!=0));
	}
	Quat1d q_q_sgn(Quat1d const& x)
	{
		double mag=x.abs();
		return AND(x/mag, -(mag!=0));
	}
	auto disc_r_sgn_i=disc_r_divide_i;
	auto disc_c_sgn_i=disc_c_divide_i;
	auto disc_q_sgn_i=disc_q_divide_i;

	double r_r_sq(double const& x){return x*x;}
	Comp1d c_c_sq(Comp1d const& x){return sq(x);}
	Quat1d q_q_sq(Quat1d const& x){return sq(x);}
	Comp1d c_c_sqrt(Comp1d const& x){return sqrt(x);}
	Quat1d q_q_sqrt(Quat1d const& x){return sqrt(x);}

	double r_r_invsqrt(double const& x)
	{
		double t0;
		(long long&)t0=0x5FE6EC85E7DE30DA-((long long&)x>>1);
		return t0*(1.5-.5*x*t0*t0);
	}

	double r_r_cbrt(double const& x){return ::cbrt(x);}
	Comp1d c_c_cbrt(Comp1d const& x){return x^_third;}//catches 0^0
	Quat1d q_q_cbrt(Quat1d const& x){return x^_third;}//optimize
	//Comp1d  c_c_cbrt					(Comp1d const &x)					{return exp(_third*log(x));}
	//Quat1d  q_q_cbrt					(Quat1d const &x)					{return exp(_third*log(x));}

	double r_r_gauss(double const& x){return ::exp(-x*x);}
	Comp1d c_c_gauss(Comp1d const& x){return exp(-sq(x));}
	Quat1d q_q_gauss(Quat1d const& x){return exp(-sq(x));}

	double r_r_erf(double const& x){return erf(x);}//C++11

	struct ZetaHelper
	{
		static const int size=1000;
		double* ln;
		ZetaHelper():ln(0)
		{
		}
		void compute()
		{
			ln=(double*)malloc(size*sizeof(double));
			for(int k=2;k<size;++k)
				ln[k]=log(k);
		}
		~ZetaHelper()
		{
			if(ln)
				free(ln);
		}
	}zetaHelper;
	inline double zeta_by_integral(double s)
	{
		double integral=0;
		double x1=1000-0.05, f1=exp((s-1)*log(x1))/(exp(x1)-1);
		for(double x=1/x1+0.05;x<100;x+=0.05)
			//	for(double x=1/x1+0.1;x<100;x+=0.1)
		{
			double x2=1/x-0.01, f2=exp((s-1)*log(x2))/(exp(x2)-1);
			integral+=0.5*(f1+f2)*(x1-x2);//x2<x1
			x1=x2, f1=f2;
		}
		return integral/tgamma(s);
	}
	double r_r_zeta(double const& s)
	{
#if 1
		if(!zetaHelper.ln)
			zetaHelper.compute();
		static const double precision=1e-7;
		if(s>1)
		{
			if(s<2)//zeta(s)=1/gamma(s) * integral from 0 to inf: x^(s-1)/(e^x-1) dx
				//	if(s<2.334)
			{
				double integral=0;
				return zeta_by_integral(s);
			}
			double result=1, term;
			int k=2;
			do
			{
				if(k<ZetaHelper::size)
					term=exp(-s*zetaHelper.ln[k]);
				else
					term=exp(-s*log(k));
				result+=term;
				++k;
			}while(term>precision);
			return result;
		}
		if(s<1)
		{
			if(s>0)//0<s<1: critical strip
			{
				return zeta_by_integral(s);
				//				double gain=1/(1-ldexp(1, 1-s));
				//				double result=1, term;
				//				unsigned k=2;
				//				do
				//				{
				//					if(k<ZetaHelper::size)
				//						term=exp(-s*zetaHelper.ln[k]);
				//					else
				//						term=exp(-s*log(k));
				//					((int*)&term)[1]|=(k&1)<<31;
				//					result+=term;
				//					++k;
				//				}
				//				while(abs(term)>precision);
				//				return gain*result;
			}
			if(s<0)
			{
				if(s<-250)
					return _qnan;
				return ldexp(1, s)*exp((s-1)*_ln_pi)*::sin(_pi_2*s)*tgamma(1-s)*r_r_zeta(1-s);
			}
			return -0.5;//s==0
		}
		return _qnan;
#endif

		//		if(s==1)
		//			return _qnan;
		//		static const double prec=1e-7;//http://www.cplusplus.com/forum/general/112172/
		//		double result=0, calc=1;
		//		for(int denom=2;abs(result-calc)>prec;++denom)
		//		{
		//			result+=calc;
		//			calc=1/pow(denom, s);
		//		}
		//		return result;
	}
	/*	Comp1d  c_c_zeta					(Comp1d const &x)
	{
		static const double prec=1e-7;
		Comp1d result, calc;
		for(int denom=2;(result-calc).abs_sq()>prec;++denom)
		{
			result+=calc;
			calc=inv(denom^x);
		}
		return result;
	}
	Quat1d  q_q_zeta					(Quat1d const &x)
	{
		static const double prec=1e-7;
		Quat1d result, calc;
		for(int denom=2;(result-calc).abs_sq()>prec;++denom)
		{
			result+=calc;
			calc=inv(denom^x);
		}
		return result;
	}//*/
	bool disc_r_zeta_i(Value const& x0, Value const& x1){return x0.r<1!=x1.r<1;}
	bool disc_c_zeta_i(Value const& x0, Value const& x1){return false;}//
	bool disc_q_zeta_i(Value const& x0, Value const& x1){return false;}//

	namespace gamma//http://en.wikipedia.org/wiki/Lanczos_approximation
	{
		const double g=7, p[]={0.99999999999980993, 676.5203681218851, -1259.1392167224028, 771.32342877765313, -176.61502916214059, 12.507343278686905, -0.13857109526572012, 9.9843695780195716e-6, 1.5056327351493116e-7};
	}
	inline Comp1d tgamma(Comp1d const& x)
	{
		using namespace gamma;
		if(x.r<.5)
		{
			Comp1d t1(p[0]);
			for(int k=1;k<g+2;++k)
				t1+=p[k]/(double(k)-x);
			Comp1d t2=g+.5-x;
			return _pi/(sin(_pi*x)*_sqrt_2pi*(t2^(.5-x))*exp(-t2)*t1);
		}
		Comp1d t1(p[0]);
		for(int k=1;k<g+2;++k)
			t1+=p[k]/(double(k)+x-1.);
		Comp1d t2=g+.5+x-1.;
		return _sqrt_2pi*(t2^(.5+x-1.))*exp(-t2)*t1;
	}
	inline Quat1d tgamma(Quat1d const& x)
	{
		using namespace gamma;
		if(x.r<.5)
		{
			Quat1d t1(p[0]);
			for(int k=1;k<g+2;++k)
				t1+=p[k]/(double(k)-x);
			Quat1d t2=g+.5-x;
			return _pi/(sin(_pi*x)*_sqrt_2pi*(t2^(.5-x))*exp(-t2)*t1);
		}
		Quat1d t1(p[0]);
		for(int k=1;k<g+2;++k)
			t1+=p[k]/(double(k)+x-1.);
		Quat1d t2=g+.5+x-1.;
		return _sqrt_2pi*(t2^(.5+x-1.))*exp(-t2)*t1;
	}
	double upper_incomplete_gamma(double s, double x)//G(x, 0) = G(x)
	{//https://social.msdn.microsoft.com/Forums/vstudio/en-US/784bc42b-3aa9-4754-8774-403f23d789fb/statistical-function-upper-incomplete-gamma-function
		static const double T_0=0, T_1=1, T_2=2, T_3=3;
		double A_prev=T_0;
		double B_prev=T_1;
		double A_cur=pow(x, s)/exp(x);
		double B_cur=x-s+T_1;
		double a=s-T_1;
		double b=B_cur+T_2;
		double n=s-T_3;
		for(;;)
		{
			const double A_next=b*A_cur+a*A_prev;
			const double B_next=b*B_cur+a*B_prev;
			if(A_next*B_cur==A_cur*B_next)
				return A_cur/B_cur;
			A_prev=A_cur;
			A_cur=A_next;
			B_prev=B_cur;
			B_cur=B_next;
			a=a+n;
			b=b+T_2;
			n=n-T_2;
		}
	}
	double r_r_tgamma(double const& x){return ::tgamma(x);}//C++11
	Comp1d c_c_tgamma(Comp1d const& x){return tgamma(x);}
	Quat1d q_q_tgamma(Quat1d const& x){return tgamma(x);}
	double r_rr_tgamma(double const& x, double const& y){return upper_incomplete_gamma(x, y);}
	bool disc_r_tgamma_i(Value const& x0, Value const& x1){return (x0.r<=0||x1.r<=0)&&_1d_int_in_range(x0.r, x1.r);}
	bool disc_c_tgamma_i(Value const& x0, Value const& x1)
	{
		if(x0.r==x1.r)
			return false;
		if(x0.i==x1.i)
			return x0.i==0&&_1d_int_in_range(x0.r, x1.r);
		if(std::signbit(x0.i)!=std::signbit(x1.i))
		{
			double t=_1d_zero_crossing(x0.r, x0.i, x1.r, x1.i);
			return t<=0&&t==std::floor(t);
		}
		return false;
	}
	bool disc_q_tgamma_i(Value const& x0, Value const& x1){return false;}//
	bool disc_rr_tgamma_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return false;}//

	double r_r_loggamma(double const& x)
	{
		return lgamma(x);//C++11
		//__declspec(align(16)) const double coef[6]={_mm_set1_pd(76.18009172947146), _mm_set1_pd(-86.50532032941677), _mm_set1_pd(24.01409824083091),
		//	_mm_set1_pd(-1.231739572450155), _mm_set1_pd(0.1208650973866179e-2), _mm_set1_pd(-0.5395239384953e-5)};
		//double denom=x+1, y=x+double(_mm_set1_pd(5.5)), series=double(_mm_set1_pd(1.000000000190015));
		//for(int i=0;i<6;++i)//https://jamesmccaffrey.wordpress.com/2013/06/19/the-log-gamma-function-with-c/
		//{
		//	series+=coef[i]/denom;
		//	denom+=1;
		//}
		//return double(_mm_set1_pd(_ln_sqrt_2pi))+(x+0.5)*double(::log(y.v))-y+double(::log((series/x).v));
	}
	bool disc_r_loggamma_i(Value const& x0, Value const& x1){return (x0.r<=0||x1.r<=0)&&_1d_int_in_range(x0.r, x1.r);}

	double r_r_factorial(double const& x){return ::tgamma(x+1);}
	Comp1d c_c_factorial(Comp1d const& x){return tgamma(x+1);}
	Quat1d q_q_factorial(Quat1d const& x){return tgamma(x+1);}
	bool disc_r_factorial_i(Value const& x0, Value const& x1)
	{
		Value _x0=x0.r+1, _x1=x1.r+1;
		return disc_r_tgamma_i(_x0, _x1);
	}
	bool disc_c_factorial_i(Value const& x0, Value const& x1)
	{
		Value _x0=x0, _x1=x1.r;
		_x0.r+=1, _x1.r+=1;
		return disc_c_tgamma_i(_x0, _x1);
	}
	bool disc_q_factorial_i(Value const& x0, Value const& x1)
	{
		Value _x0=x0, _x1=x1.r;
		_x0.r+=1, _x1.r+=1;
		return disc_q_tgamma_i(_x0, _x1);
	}

	inline double permutation(double x, double y)
	{
		try
		{
			return ::tgamma(x+1)/::tgamma(x-y+1);//C++11
		}
		catch(std::domain_error&)
		{
			long long valueBits=(long long&)x;
			if(!valueBits)
				return _HUGE;
			if(valueBits==0x8000000000000000)
				return -_HUGE;
			long long divergentBits=0x7FF8000000000010;
			return *(double*)&divergentBits;
		}
		catch(std::overflow_error&)
		{
			return _HUGE;
		}
	}
	inline Comp1d permutation(Comp1d const& x, Comp1d const& y)
	{
		try
		{
			return tgamma(x+1.)/tgamma(x-y+1.);
		}
		catch(std::domain_error&)
		{
			long long valueBits=(long long&)x;
			if(!valueBits)
				return _HUGE;
			if(valueBits==0x8000000000000000)
				return -_HUGE;
			long long divergentBits=0x7FF8000000000010;
			return *(double*)&divergentBits;
		}
		catch(std::overflow_error&)
		{
			return _HUGE;
		}
	}
	inline Quat1d permutation(Quat1d const& x, Quat1d const& y)
	{
		try
		{
			return tgamma(x+1.)/tgamma(x-y+1.);
		}
		catch(std::domain_error&)
		{
			long long valueBits=(long long&)x;
			if(!valueBits)
				return Quat1d(_HUGE);
			if(valueBits==0x8000000000000000)
				return Quat1d(-_HUGE);
			long long divergentBits=0x7FF8000000000010;
			return Quat1d((double&)divergentBits);
		}
		catch(std::overflow_error&)
		{
			return Quat1d(_HUGE);
		}
	}
	double r_r_permutation(double const& x){return 1;}
	Comp1d c_c_permutation(Comp1d const& x){return Comp1d(1, 0);}
	Quat1d q_q_permutation(Quat1d const& x){return Quat1d(1, 0, 0, 0);}
	double r_rr_permutation(double const& x, double const& y){return permutation(x, y);}
	Comp1d c_cr_permutation(Comp1d const& x, double const& y){return permutation(x, Comp1d(y));}
	Comp1d c_cc_permutation(Comp1d const& x, Comp1d const& y){return permutation(x, y);}
	Quat1d q_qq_permutation(Quat1d const& x, Quat1d const& y){return permutation(x, y);}
	bool disc_rr_permutation_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return false;}//
	bool disc_rc_permutation_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return false;}//
	bool disc_rq_permutation_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return false;}//
	bool disc_cr_permutation_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return false;}//
	bool disc_cc_permutation_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return false;}//
	bool disc_cq_permutation_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return false;}//
	bool disc_qr_permutation_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return false;}//
	bool disc_qc_permutation_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return false;}//
	bool disc_qq_permutation_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return false;}//

	inline double combination(double x, double y)
	{
		try
		{
			return ::tgamma(x+1)/(::tgamma(x-y+1)*::tgamma(y+1));//C++11
		}catch(std::domain_error&)
		{
			long long valueBits=(long long&)x;
			if(!valueBits)
				return _HUGE;
			if(valueBits==0x8000000000000000)
				return -_HUGE;
			long long divergentBits=0x7FF8000000000010;
			return *(double*)&divergentBits;
		}catch(std::overflow_error&)
		{
			return _HUGE;
		}
	}
	inline Comp1d combination(Comp1d const& x, Comp1d const& y)
	{
		try
		{
			return tgamma(x+1.)/(tgamma(x-y+1.)*tgamma(y+1.));
		}catch(std::domain_error&)
		{
			long long valueBits=(long long&)x;
			if(!valueBits)
				return _HUGE;
			if(valueBits==0x8000000000000000)
				return -_HUGE;
			long long divergentBits=0x7FF8000000000010;
			return *(double*)&divergentBits;
		}catch(std::overflow_error&)
		{
			return _HUGE;
		}
	}
	inline Quat1d combination(Quat1d const& x, Quat1d const& y)
	{
		try
		{
			return tgamma(x+1.)/(tgamma(x-y+1.)*tgamma(y+1.));
		}catch(std::domain_error&)
		{
			long long valueBits=(long long&)x;
			if(!valueBits)
				return Quat1d(_HUGE);
			if(valueBits==0x8000000000000000)
				return Quat1d(-_HUGE);
			long long divergentBits=0x7FF8000000000010;
			return Quat1d((double&)divergentBits);
		}catch(std::overflow_error&)
		{
			return Quat1d(_HUGE);
		}
	}
	double r_r_combination(double const& x){return 1;}
	Comp1d c_c_combination(Comp1d const& x){return Comp1d(1, 0);}
	Quat1d q_q_combination(Quat1d const& x){return Quat1d(1, 0, 0, 0);}
	double r_rr_combination(double const& x, double const& y){return combination(x, y);}
	Comp1d c_cr_combination(Comp1d const& x, double const& y){return combination(x, Comp1d(y));}
	Comp1d c_cc_combination(Comp1d const& x, Comp1d const& y){return combination(x, y);}
	Quat1d q_qq_combination(Quat1d const& x, Quat1d const& y){return combination(x, y);}
	bool disc_rr_combination_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return false;}//
	bool disc_rc_combination_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return false;}//
	bool disc_rq_combination_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return false;}//
	bool disc_cr_combination_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return false;}//
	bool disc_cc_combination_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return false;}//
	bool disc_cq_combination_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return false;}//
	bool disc_qr_combination_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return false;}//
	bool disc_qc_combination_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return false;}//
	bool disc_qq_combination_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1){return false;}//

	double r_r_cos(double const& x){return ::cos(x);}
	Comp1d c_c_cos(Comp1d const& x){return cos(x);}
	Quat1d q_q_cos(Quat1d const& x){return cos(x);}

	Comp1d c_c_acos(Comp1d const& x){return acos(x);}
	Quat1d q_q_acos(Quat1d const& x){return acos(x);}
	bool disc_c_acos_i(Value const& x0, Value const& x1)
	{
		if(x0.i==x0.i)
			return false;
		if(x0.r==x0.r)
			return (x0.i<=0?x1.i>0:x1.i<=0)&&(x0.r<-1||x0.r>1);
		if((x0.i<=0&&x1.i>0)||(x1.i<=0&&x0.i>0))
		{
			double t=_1d_zero_crossing(x0.r, x0.i, x1.r, x1.i);
			return t<-1||t>1;
		}
		return false;
	}
	bool disc_q_acos_i(Value const& x0, Value const& x1)
	{
		return false;
	}//


	double r_r_cosh(double const& x){return ::cosh(x);}
	Comp1d c_c_cosh(Comp1d const& x){return cosh(x);}
	Quat1d q_q_cosh(Quat1d const& x){return cosh(x);}

	Comp1d c_c_acosh(Comp1d const& x){return acosh(x);}
	Quat1d q_q_acosh(Quat1d const& x){return acosh(x);}

	double r_r_cosc(double const& x){return ::cos(x)/x;}
	Comp1d c_c_cosc(Comp1d const& x){return cos(x)/x;}
	Quat1d q_q_cosc(Quat1d const& x){return cos(x)/x;}
	auto disc_r_cosc_i=disc_r_divide_i;
	auto disc_c_cosc_i=disc_c_divide_i;
	auto disc_q_cosc_i=disc_q_divide_i;

	double r_r_sec(double const& x){return 1/::cos(x);}
	Comp1d c_c_sec(Comp1d const& x){return 1/cos(x);}
	Quat1d q_q_sec(Quat1d const& x){return 1/cos(x);}
	bool disc_r_sec_i(Value const& x0, Value const& x1)
	{
		if(std::abs(x1.r-x0.r)>3.2)
			return true;
		return _1d_int_in_range(x0.r/_pi-.5, x1.r/_pi-.5);
	}
	bool disc_c_sec_i(Value const& x0, Value const& x1)
	{
		if(x0.r==x1.r)
		{
			if(x0.i==x1.i)
				return false;
			double t=x0.r/_pi-.5;
			return t==std::floor(t);
		}
		if(x0.i==x1.i)
			return x0.i==0&&_1d_int_in_range(x0.r/_pi-.5, x1.r/_pi-.5);
		if(std::signbit(x0.i)!=std::signbit(x1.i))
		{
			double t=_1d_zero_crossing(x0.r, x0.i, x1.r, x0.i)/_pi-.5;
			return t==std::floor(t);
		}
		return false;
	}
	bool disc_q_sec_i(Value const& x0, Value const& x1)
	{
		return false;
	}//

	Comp1d c_c_asec(Comp1d const& x){return acos(1/x);}
	Quat1d q_q_asec(Quat1d const& x){return acos(1/x);}
	auto disc_c_asec_i=disc_c_divide_i;
	auto disc_q_asec_i=disc_q_divide_i;

	double r_r_sech(double const& x){return 1/::cosh(x);}
	Comp1d c_c_sech(Comp1d const& x){return 1/cosh(x);}
	Quat1d q_q_sech(Quat1d const& x){return 1/cosh(x);}
	bool disc_c_sech_i(Value const& x0, Value const& x1)
	{
		if(x0.i==x1.i)
		{
			if(x0.r==x1.r)
				return false;
			double i=x0.i/_pi+.5;
			return i==std::floor(i)&&(x0.r<0?x1.r>=0:x0.r>0?x1.r<=0:x1.r!=0);
		}
		if(x0.r==x1.r)
			return x0.r==0&&_1d_int_in_range(x0.i/_pi-.5, x1.i/_pi-.5);
		return false;
		/*	if(x0.i==x1.i)	return true;
		if(x0.r==x1.r)	return x0.r==0&&_1d_int_in_range(x0.i/_pi-.5, x1.i/_pi-.5);
		if(std::signbit(x0.r)!=std::signbit(x1.r))
		{
			double t=_1d_zero_crossing(x0.i, x0.r, x1.i, x1.r)/_pi-.5;
			return t==std::floor(t);
		}
		return false;//*/
	}
	bool disc_q_sech_i(Value const& x0, Value const& x1){return false;}//

	Comp1d c_c_asech(Comp1d const& x){return acosh(1/x);}
	Quat1d q_q_asech(Quat1d const& x){return acosh(1/x);}
	bool disc_c_asech_i(Value const& x0, Value const& x1)
	{
		if(x0.i==x0.i)
			return x0.i==0&&x0.r!=x0.r&&std::signbit(x0.r)!=std::signbit(x1.r);
		if(x0.r==x0.r)
			return (x0.i<=0?x1.i>0:x1.i<=0)&&(x0.r<=0||x0.r>1);
		if((x0.i<=0&&x1.i>0)||(x0.i<=0&&x1.i>0))
		{
			double t=_1d_zero_crossing(x0.r, x0.i, x1.r, x1.i);
			return t<=0||t>1;
		}
		return false;
	}
	bool disc_q_asech_i(Value const& x0, Value const& x1){return false;}//

	double r_r_sin(double const& x){return ::sin(x);}
	Comp1d c_c_sin(Comp1d const& x){return sin(x);}
	Quat1d q_q_sin(Quat1d const& x){return sin(x);}

	Comp1d c_c_asin(Comp1d const& x){return asin(x);}
	Quat1d q_q_asin(Quat1d const& x){return asin(x);}
	auto disc_c_asin_i=disc_c_acos_i;
	auto disc_q_asin_i=disc_q_acos_i;

	double r_r_sinh(double const& x){return ::sinh(x);}
	Comp1d c_c_sinh(Comp1d const& x){return sinh(x);}
	Quat1d q_q_sinh(Quat1d const& x){return sinh(x);}

	double r_r_asinh(double const& x){return ::asinh(x);}
	Comp1d c_c_asinh(Comp1d const& x){return asinh(x);}
	Quat1d q_q_asinh(Quat1d const& x){return asinh(x);}
	bool disc_c_asinh_i(Value const& x0, Value const& x1)
	{
		Value _x0(x0.i, x0.r), _x1(x1.i, x0.r);
		return disc_c_acos_i(_x0, _x1);
	}
	bool disc_q_asinh_i(Value const& x0, Value const& x1)
	{
		return false;
	}//

	double r_r_sinc(double const& x){return x?::sin(x)/x:1;}
	Comp1d c_c_sinc(Comp1d const& x){return x.c_is_true()?sin(x)/x:Comp1d(1);}
	Quat1d q_q_sinc(Quat1d const& x){return x.q_is_true()?sin(x)/x:Quat1d(1);}

	double r_r_sinhc(double const& x){return x?::sinh(x)/x:1;}
	Comp1d c_c_sinhc(Comp1d const& x){return x.c_is_true()?sinh(x)/x:Comp1d(1);}
	Quat1d q_q_sinhc(Quat1d const& x){return x.q_is_true()?sinh(x)/x:Quat1d(1);}

	double r_r_csc(double const& x){return 1/::sin(x);}
	Comp1d c_c_csc(Comp1d const& x){return 1/sin(x);}
	Quat1d q_q_csc(Quat1d const& x){return 1/sin(x);}
	bool disc_r_csc_i(Value const& x0, Value const& x1)
	{
		if(std::abs(x1.r-x0.r)>3.2)
			return true;
		return _1d_int_in_range(x0.r/_pi, x1.r/_pi);
	}
	bool disc_c_csc_i(Value const& x0, Value const& x1)
	{
		if(x0.r==x1.r)
			return true;
		if(x0.i==x1.i)
			return x0.i==0&&_1d_int_in_range(x0.r/_pi, x1.r/_pi);
		if(std::signbit(x0.i)!=std::signbit(x1.i))
		{
			double t=_1d_zero_crossing(x0.r, x0.i, x1.r, x0.i)/_pi;
			return t==std::floor(t);
		}
		return false;
	}
	bool disc_q_csc_i(Value const& x0, Value const& x1)
	{
		return false;
	}

	Comp1d c_c_acsc(Comp1d const& x){return asin(1/x);}
	Quat1d q_q_acsc(Quat1d const& x){return asin(1/x);}
	bool disc_c_acsc_i(Value const& x0, Value const& x1)
	{
		if(x0.i==x1.i)
			return x0.i==0&&(x0.r<0?x1.r>=0:x0.r>0?x1.r<=0:x1.r!=0);//x1.r<0||x1.r>0);
		if(x0.r==x1.r)
		{
			if(x0.r<0)
				return x0.r>-1&&(x0.i<=0?x1.i>0:x1.i<=0);
			if(x0.r==0)
				return x0.i<0?x1.i>=0:x0.i==0?x1.i<0||x1.i>0:x1.i<=0;
			return x0.r<1&&(x0.i<0?x1.i>=0:x1.i<0);
		}
		return false;
	}
	bool disc_q_acsc_i(Value const& x0, Value const& x1){return false;}//

	double r_r_csch(double const& x){return 1/::sinh(x);}
	Comp1d c_c_csch(Comp1d const& x){return 1/sinh(x);}
	Quat1d q_q_csch(Quat1d const& x){return 1/sinh(x);}
	auto disc_r_csch_i=disc_r_divide_i;
	bool disc_c_csch_i(Value const& x0, Value const& x1)
	{
		Value _x0(x0.i, x0.r), _x1(x1.i, x1.r);
		return disc_c_csc_i(_x0, _x1);
	}
	bool disc_q_csch_i(Value const& x0, Value const& x1){return false;}//

	double r_r_acsch(double const& x){return ::asinh(1/x);}
	Comp1d c_c_acsch(Comp1d const& x){return asinh(1/x);}
	Quat1d q_q_acsch(Quat1d const& x){return asinh(1/x);}
	auto disc_r_acsch_i=disc_r_divide_i;
	bool disc_c_acsch_i(Value const& x0, Value const& x1)
	{
		Value _x0(x0.i, x0.r), _x1(x1.i, x1.r);
		return disc_c_acsc_i(_x0, _x1);
	}
	bool disc_q_acsch_i(Value const& x0, Value const& x1){return false;}//

	double r_r_tan(double const& x){return ::tan(x);}
	Comp1d c_c_tan(Comp1d const& x){return tan(x);}
	Quat1d q_q_tan(Quat1d const& x){return tan(x);}
	auto disc_r_tan_i=disc_r_sec_i;
	auto disc_c_tan_i=disc_c_sec_i;
	auto disc_q_tan_i=disc_q_sec_i;

	inline double atan_addition(double x, double y)
	{
		long long addition=((long long&)_pi&-(x<0))^(sign_mask_complement&-(y<0));
		return (double&)addition;
	}
	double  r_r_atan					(double const &x)					{return ::atan(x);}
	Comp1d  c_c_atan					(Comp1d const &x)					{return atan(x);}
	Quat1d  q_q_atan					(Quat1d const &x)					{return atan(x);}
	double r_rr_atan					(double const &x, double const &y)	{return ::atan2(x, y);}
	Comp1d c_rc_atan					(double const &x, Comp1d const &y)	{return atan(x/y)+atan_addition(x, y.r);}
	Quat1d q_rq_atan					(double const &x, Quat1d const &y)	{return atan(x/y)+atan_addition(x, y.r);}
	Comp1d c_cr_atan					(Comp1d const &x, double const &y)	{return atan(x/y)+atan_addition(x.r, y);}
	Comp1d c_cc_atan					(Comp1d const &x, Comp1d const &y)	{return atan(x/y)+atan_addition(x.r, y.r);}
	Quat1d q_cq_atan					(Comp1d const &x, Quat1d const &y)	{return atan(x/y)+atan_addition(x.r, y.r);}
	Quat1d q_qr_atan					(Quat1d const &x, double const &y)	{return atan(x/y)+atan_addition(x.r, y);}
	Quat1d q_qc_atan					(Quat1d const &x, Comp1d const &y)	{return atan(x/y)+atan_addition(x.r, y.r);}
	Quat1d q_qq_atan					(Quat1d const &x, Quat1d const &y)	{return atan(x/y)+atan_addition(x.r, y.r);}
	bool disc_c_atan_i				(Value const &x0, Value const &x1)
	{
		Value _x0(x0.i, x0.r), _x1(x1.i, x1.r);
		return disc_c_acos_i(_x0, _x1);
	}
	bool disc_q_atan_i				(Value const &x0, Value const &x1){return false;}//
	bool disc_rr_atan_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1)
	{
		if(y0.r<0)
		{
			if(x0.r<0)
			{
				if(y1.r<0)	return x1.r>=0;
				if(y1.r>0)	return x1.r>0&&y0.r+(0-x0.r)*(y1.r-y0.r)/(x1.r-x0.r)<=0;
							return x1.r>=0;
			}
			else if(x0.r>0)
			{
				if(y1.r<0)	return x1.r<0;
				if(y1.r>0)	return x1.r<0&&y0.r+(0-y0.r)*(y1.r-y0.r)/(x1.r-x1.r)<=0;
							return x1.r<=0;
			}
			else
			{
				if(y1.r<0)	return x1.r<0;
				if(y1.r>0)	return x1.r<=0;
							return x1.r<=0;
			}
		}
		else if(y0.r>0)
		{
			if(x0.r<0)
			{
				if(y1.r<0)	return x1.r>=0&&(x1.r==0||y0.r+(0-y0.r)*(y1.r-y0.r)/(x1.r-x1.r)<=0);
				if(y1.r>0)	return false;
							return x1.r==0;
			}
			else if(x0.r>0)
			{
				if(y1.r<0)	return x1.r<0&&y0.r+(0-y0.r)*(y1.r-y0.r)/(x1.r-x1.r)<=0;
				if(y1.r>0)	return false;
							return x1.r==0;
			}
			else
			{
				if(y1.r<0)	return x1.r==0;
				if(y1.r>0)	return false;
							return x1.r==0;
			}
		}
		else
		{
			if(x0.r<0)
			{
				if(y1.r<0)	return x1.r>=0;
				if(y1.r>0)	return false;
							return x1.r>=0;
			}
			else if(x0.r>0)
			{
				if(y1.r<0)	return x1.r<0;
				if(y1.r>0)	return false;
							return x1.r<=0;
			}
							return true;
		}
	}
	bool disc_rc_atan_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return false;}//
	bool disc_rq_atan_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return false;}//
	bool disc_cr_atan_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return false;}//
	bool disc_cc_atan_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return false;}//
	bool disc_cq_atan_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return false;}//
	bool disc_qr_atan_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return false;}//
	bool disc_qc_atan_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return false;}//
	bool disc_qq_atan_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return false;}//

	double  r_r_tanh					(double const &x)					{return ::tanh(x);}
	Comp1d  c_c_tanh					(Comp1d const &x)					{return tanh(x);}
	Quat1d  q_q_tanh					(Quat1d const &x)					{return tanh(x);}
	//auto disc_c_tanh_i				=disc_c_sech_i;
	//auto disc_q_tanh_i				=disc_q_sech_i;

	Comp1d  c_c_atanh					(Comp1d const &x)					{return atanh(x);}
	Quat1d  q_q_atanh					(Quat1d const &x)					{return atanh(x);}
	bool disc_c_atanh_i				(Value const &x0, Value const &x1)
	{
		Value _x0(x0.r, -x0.i), _x1(x1.r, -x1.i);
		return disc_c_acos_i(_x0, _x1);
	}
	bool disc_q_atanh_i				(Value const &x0, Value const &x1){return false;}//

	double  r_r_tanc					(double const &x)					{return x?::tan(x)/x:1;}
	Comp1d  c_c_tanc					(Comp1d const &x)					{return x.c_is_true()?tan(x)/x:1;}
	Quat1d  q_q_tanc					(Quat1d const &x)					{return x.q_is_true()?tan(x)/x:1;}
	auto disc_r_tanc_i				=disc_r_sec_i;
	auto disc_c_tanc_i				=disc_c_sec_i;
	auto disc_q_tanc_i				=disc_q_sec_i;

	double  r_r_cot						(double const &x)					{return 1/::tan(x);}
	Comp1d  c_c_cot						(Comp1d const &x)					{return 1/tan(x);}
	Quat1d  q_q_cot						(Quat1d const &x)					{return 1/tan(x);}
	auto disc_r_cot_i				=disc_r_csc_i;
	auto disc_c_cot_i				=disc_c_csc_i;
	auto disc_q_cot_i				=disc_q_csc_i;

	double  r_r_acot					(double const &x)					{return x?::atan(1/x):_pi_2;}
	Comp1d  c_c_acot					(Comp1d const &x)					{return x.c_is_true()?atan(1/x):_pi_2;}
	Quat1d  q_q_acot					(Quat1d const &x)					{return x.q_is_true()?atan(1/x):_pi_2;}
	bool disc_r_acot_i				(Value const &x0, Value const &x1){return x0.r<0?x1.r>=0:x1.r<0;}
	auto disc_c_acot_i				=disc_c_acsch_i;
	auto disc_q_acot_i				=disc_q_acsch_i;

	double  r_r_coth					(double const &x)					{return 1/::tanh(x);}
	Comp1d  c_c_coth					(Comp1d const &x)					{return 1/tanh(x);}
	Quat1d  q_q_coth					(Quat1d const &x)					{return 1/tanh(x);}
	auto disc_r_coth_i				=disc_r_csch_i;
	auto disc_c_coth_i				=disc_c_csch_i;
	bool disc_q_coth_i				(Value const &x0, Value const &x1){return false;}//

	Comp1d  c_c_acoth					(Comp1d const &x)					{return atanh(1/x);}
	Quat1d  q_q_acoth					(Quat1d const &x)					{return atanh(1/x);}
	bool disc_c_acoth_i				(Value const &x0, Value const &x1)
	{
		Value _x0(x0.r, -x0.i), _x1(x1.r, -x1.i);
		return disc_c_acsc_i(_x0, _x1);
	}
	bool disc_q_acoth_i				(Value const &x0, Value const &x1){return false;}//

	double  r_r_exp						(double const &x)					{return ::exp(x);}
	Comp1d  c_c_exp						(Comp1d const &x)					{return exp(x);}
	Quat1d  q_q_exp						(Quat1d const &x)					{return exp(x);}

	double  r_r_fib						(double const &x)					{return (::exp(x*_ln_phi)-::cos(_pi*x)*::exp(-x*_ln_phi))/_sqrt5;}
	Comp1d  c_c_fib						(Comp1d const &x)					{return ((_phi^x)-cos(_pi*x)*(_phi^-x))/_sqrt5;}
	Quat1d  q_q_fib						(Quat1d const &x)					{return ((_phi^x)-cos(_pi*x)*(_phi^-x))/_sqrt5;}

	const double rand_norm=9.31322574615479e-010;
	double my_rand(){return (rand()<<15|rand())*rand_norm;}
	double  r_r_random					(double const &x)					{return my_rand();}
	Comp1d  c_c_random					(Comp1d const &x)					{return Comp1d(my_rand(), my_rand());}
	Quat1d  q_q_random					(Quat1d const &x)					{return Quat1d(my_rand(), my_rand(), my_rand(), my_rand());}
	double r_rr_random					(double const &x, double const &y)	{return my_rand();}
	Comp1d c_cr_random					(Comp1d const &x, double const &y)	{return Comp1d(my_rand(), my_rand());}
	Comp1d c_cc_random					(Comp1d const &x, Comp1d const &y)	{return Comp1d(my_rand(), my_rand());}
	Quat1d q_qq_random					(Quat1d const &x, Quat1d const &y)	{return Quat1d(my_rand(), my_rand(), my_rand(), my_rand());}
	bool disc_r_random_o			(Value const &o0, Value const &o1){return true;}
	bool disc_c_random_o			(Value const &o0, Value const &o1){return true;}
	bool disc_q_random_o			(Value const &o0, Value const &o1){return true;}

	double  r_r_beta					(double const &x)					{return 2*exp(lgamma(x)-lgamma(2*x));}
	double r_rr_beta					(double const &x, double const &y)	{return exp(lgamma(x)+lgamma(y)-lgamma(x+y));}
	bool disc_r_beta_i				(Value const &x0, Value const &x1){return false;}//
	bool disc_rr_beta_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return false;}//

	double  r_r_bessel_j				(double const &x)					{return j0(x);}
	double r_rr_bessel_j				(double const &x, double const &y)	{return jn((int)x, y);}
	bool disc_r_bessel_j_i			(Value const &x0, Value const &x1){return false;}
	bool disc_rr_bessel_j_i			(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return false;}//

	double  r_r_bessel_y				(double const &x)					{return y0(x);}
	double r_rr_bessel_y				(double const &x, double const &y)	{return yn((int)x, y);}
	bool disc_r_bessel_y_i			(Value const &x0, Value const &x1){return false;}//
	bool disc_rr_bessel_y_i			(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return false;}//

	Comp1d  c_r_hankel1					(double const &x)					{return r_hankel1(x, x);}
	Comp1d  c_c_hankel1					(Comp1d const &x)					{return r_hankel1(x.r, x.i);}
	Comp1d c_rr_hankel1					(double const &x, double const &y)	{return r_hankel1(x, y);}
	bool disc_r_hankel1_i			(Value const &x0, Value const &x1){return false;}//
	bool disc_rr_hankel1_i			(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return false;}//

	double  r_r_step					(double const &x)					{return step(x);}
	Comp1d  c_c_step					(Comp1d const &x)					{return step(x);}
	Quat1d  q_q_step					(Quat1d const &x)					{return step(x);}
	auto disc_r_step_i				=disc_r_divide_i;
	auto disc_c_step_i				=disc_c_divide_i;
	auto disc_q_step_i				=disc_q_divide_i;

	double  r_r_rect					(double const &x)					{return step(x+0.5)-step(x-0.5);}
	Comp1d  c_c_rect					(Comp1d const &x)					{return step(x+0.5)-step(x-0.5);}
	Quat1d  q_q_rect					(Quat1d const &x)					{return step(x+0.5)-step(x-0.5);}
	bool disc_rect(double x0, double x1)
	{
		double const d[2]={-.5, .5};
		if(x0<d[0])		return x1>=d[0];
		if(x0==d[0])	return x1<d[0]||x1>d[0];
		if(x0<d[1])		return x1>=d[1];
		if(x0==d[1])	return x1<d[1]||x1>d[1];
						return x1<=d[1];
	}
	bool disc_r_rect_i				(Value const &x0, Value const &x1){return disc_rect(x0.r, x1.r);}
	bool disc_c_rect_i				(Value const &x0, Value const &x1)
	{
		if(x0.i==x1.i)	return x0.i==0&&disc_rect(x0.r, x1.r);
		if(x0.r==x1.r)	return (x0.r==-.5||x0.r==.5)&&(x0.i<0?x1.i>=0:x0.i==0?x1.i<0||x1.i>0:x1.i<=0);
		double x=_1d_zero_crossing(x0.r, x0.i, x1.r, x1.i);
		return x==-.5||x==.5;
	}
	bool disc_q_rect_i				(Value const &x0, Value const &x1){return false;}//

	double  r_r_trgl					(double const &x)					{double t=abs(x); return (1-t)*(t<1);}
	double  r_c_trgl					(Comp1d const &x)					{double t=x.abs(); return (1-t)*(t<1);}
	double  r_q_trgl					(Quat1d const &x)					{double t=x.abs(); return (1-t)*(t<1);}

	double  r_r_sqwv					(double const &x)					{return x-floor(x)<0.5;}
	double  r_c_sqwv					(Comp1d const &x)					{return x.r-floor(x.r)<0.5;}
	double  r_q_sqwv					(Quat1d const &x)					{return x.r-floor(x.r)<0.5;}
	double r_rr_sqwv					(double const &x, double const &y)	{return x-floor(x)<y;}
	double r_rc_sqwv					(double const &x, Comp1d const &y)	{return x-floor(x)<y.r;}
	double r_rq_sqwv					(double const &x, Quat1d const &y)	{return x-floor(x)<y.r;}
	double r_cr_sqwv					(Comp1d const &x, double const &y)	{return x.r-floor(x.r)<y;}
	double r_cc_sqwv					(Comp1d const &x, Comp1d const &y)	{return x.r-floor(x.r)<y.r;}
	double r_cq_sqwv					(Comp1d const &x, Quat1d const &y)	{return x.r-floor(x.r)<y.r;}
	double r_qr_sqwv					(Quat1d const &x, double const &y)	{return x.r-floor(x.r)<y;}
	double r_qc_sqwv					(Quat1d const &x, Comp1d const &y)	{return x.r-floor(x.r)<y.r;}
	double r_qq_sqwv					(Quat1d const &x, Quat1d const &y)	{return x.r-floor(x.r)<y.r;}
	bool disc_r_sqwv_o				(Value const &o0, Value const &o1)			{return o0.r!=o0.r;}
	//bool disc_r_sqwv_i				(Value const &x0, Value const &x1)
	//{
	//	if(x0.r==x1.r)	return false;
	//	double t0=2*x0.r, t1=2*x1.r;
	//	if(t0==std::floor(t0)||t1==std::floor(t1))
	//		return true;
	//	return std::floor(t0)!=std::floor(t1);
	//}
	//bool disc_c_sqwv_i				(Value const &x0, Value const &x1)
	//{
	//	if(std::floor(x0.r)!=std::floor(x1.r)||std::floor(x0.i)!=std::floor(x1.i))
	//		return true;
	//	if(x0.r==x1.r&&x0.i==x1.i)
	//		return true;
	//	{
	//		double r0=2*x0.r;
	//		if(r0==std::floor(r0)&&x0.i==std::floor(x0.i))
	//			return true;
	//	}
	//	{
	//		double r1=2*x1.r;
	//		if(r1==std::floor(r1)&&x1.i==std::floor(x1.i))
	//			return true;
	//	}
	//	return false;
	//}
	//bool disc_q_sqwv_i				(Value const &x0, Value const &x1){return false;}//
	//bool disc_rr_sqwv_o				(Value const &o0, Value const &o1){return o0.r!=o1.r;}
	//bool disc_rc_sqwv_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return false;}//
	//bool disc_rq_sqwv_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return false;}//
	//bool disc_cr_sqwv_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return false;}//
	//bool disc_cc_sqwv_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return false;}//
	//bool disc_cq_sqwv_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return false;}//
	//bool disc_qr_sqwv_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return false;}//
	//bool disc_qc_sqwv_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return false;}//
	//bool disc_qq_sqwv_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return false;}//

	inline double clamp01(double x)
	{
		double temp=x+abs(x);//max(0, x)
		return (temp+2-abs(temp-2))*0.25;//min(x, 1)
	}
	inline double trwv_dc(double x, double y)
	{
		double t=x-::floor(x), t2=1-x;
		t2-=::floor(t2);
		double dc=clamp01(y);
		double dc2=1-dc, t_d=t/dc, t2_d2=t2/dc2;
		return (t_d<1?t_d:0)+(t2_d2<1?t2_d2:0);
	}
	double  r_r_trwv					(double const &x)					{double t=abs(x-::floor(x)-0.5); return t+t;}
	double  r_c_trwv					(Comp1d const &x)					{Comp1d cx=x; double t=(cx-cx.floor()-0.5).abs(); return t+t;}
	double  r_q_trwv					(Quat1d const &x)					{Quat1d qx=x; double t=(qx-qx.floor()-0.5).abs(); return t+t;}
	double r_rr_trwv					(double const &x, double const &y)	{return trwv_dc(x  , y  );}
	double r_rc_trwv					(double const &x, Comp1d const &y)	{return trwv_dc(x  , y.r);}
	double r_rq_trwv					(double const &x, Quat1d const &y)	{return trwv_dc(x  , y.r);}
	double r_cr_trwv					(Comp1d const &x, double const &y)	{return trwv_dc(x.r, y  );}
	double r_cc_trwv					(Comp1d const &x, Comp1d const &y)	{return trwv_dc(x.r, y.r);}
	double r_cq_trwv					(Comp1d const &x, Quat1d const &y)	{return trwv_dc(x.r, y.r);}
	double r_qr_trwv					(Quat1d const &x, double const &y)	{return trwv_dc(x.r, y  );}
	double r_qc_trwv					(Quat1d const &x, Comp1d const &y)	{return trwv_dc(x.r, y.r);}
	double r_qq_trwv					(Quat1d const &x, Quat1d const &y)	{return trwv_dc(x.r, y.r);}
	//double  r_r_trwv					(double const &x)					{return abs(2*(x-floor(x)-0.5));}
	//double  r_c_trwv					(Comp1d const &x)					{return 2*(x-x.floor()-0.5).abs();}
	//double  r_q_trwv					(Quat1d const &x)					{return 2*(x-x.floor()-0.5).abs();}
	//double r_rr_trwv					(double const &x, double const &y)
	//{
	//	double t=x-floor(x), t2=1-x;
	//	t2-=floor(t2);
	//	double d=clamp(0, y, 1), d2=1-d;//duty cycle: [0, 1]
	//	double t_d=t/d, t2_d2=t2/d2;
	//	return (t_d<1)*t_d+(t2_d2<1)*t2_d2;
	//}
	//Comp1d c_cr_trwv					(Comp1d const &x, double const &y)
	//{
	//	auto t=x-x.floor(), t2=1-x;
	//	t2=t2-t2.floor();
	//	auto d=clamp(0, y, 1), d2=1-d;
	//	auto t_d=t/d, t2_d2=t2/d2;
	//	return (t_d.r<1)*t_d+(t2_d2.r<1)*t2_d2;
	//}
	//Comp1d c_cc_trwv					(Comp1d const &x, Comp1d const &y)
	//{
	//	Comp1d t=x-x.floor(), t2=1-x;
	//	t2=t2-t2.floor();
	//	Comp1d d=AND(y, -(y.r>0));
	//	long long mask=d.r>1;
	//	d=OR(AND(d, ~-mask), mask);
	//	Comp1d d2=1-d;
	//	Comp1d t_d=t/d, t2_d2=t2/d2;
	//	return AND(-(t_d.r<1), t_d)+AND(-(t2_d2.r<1), t2_d2);
	//}
	//Quat1d q_qq_trwv					(Quat1d const &x, Quat1d const &y)
	//{
	//	Quat1d t=x-x.floor(), t2=1-x;
	//	t2=t2-t2.floor();
	//	Quat1d d=AND(y, -(y.r>0));
	//	long long mask=d.r>1;
	//	d=OR(AND(d, ~-mask), mask);
	//	Quat1d d2=1-d;
	//	Quat1d t_d=t/d, t2_d2=t2/d2;
	//	return AND(-(t_d.r<1), t_d)+AND(-(t2_d2.r<1), t2_d2);
	//}
	//bool disc_c_trwv_i				(Value const &x0, Value const &x1){return std::floor(x0.r)!=std::floor(x1.r)||std::floor(x0.i)!=std::floor(x1.i);}//trwv is continuous
	//bool disc_q_trwv_i				(Value const &x0, Value const &x1){return false;}//
	//bool disc_rr_trwv_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return std::floor(x0.r)!=std::floor(x1.r);}
	//bool disc_rc_trwv_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return std::floor(x0.r)!=std::floor(x1.r);}
	//bool disc_rq_trwv_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return std::floor(x0.r)!=std::floor(x1.r);}
	//bool disc_cr_trwv_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1)
	//{
	//	if(std::floor(x0.r)!=std::floor(x1.r)||std::floor(x0.i)!=std::floor(x1.i))
	//		return true;
	//	return y0.r<1?y1.r>=1:y0.r==1?y1.r<1||y1.r>1:y1.r<=1;
	//}
	//bool disc_cc_trwv_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return false;}//
	//bool disc_cq_trwv_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return false;}//
	//bool disc_qr_trwv_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return false;}//
	//bool disc_qc_trwv_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return false;}//
	//bool disc_qq_trwv_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return false;}//

	inline double sawtooth(double x)
	{
		double t=x-::floor(x), t2=::floor(1-t);//dc=1
		return (t2+1)*(t2*0.5+t);
	}
	inline double sawtooth_dc(double x, double y)
	{
		if(!y)
			return 0;
		auto t=x-::floor(x), t2=::floor(y-t);
		return (t2+1)*(t2*0.5+t)/y;
	}
	inline bool sawtooth_dc_disc(double t0, double t1){return std::floor(t0)!=std::floor(t1);}
	double  r_r_saw					(double const &x)					{return sawtooth(x);}
	double  r_c_saw					(Comp1d const &x)					{return sawtooth(x.r);}
	double  r_q_saw					(Quat1d const &x)					{return sawtooth(x.r);}
	double r_rr_saw					(double const &x, double const &y)	{return sawtooth_dc(x  , y  );}
	double r_rc_saw					(double const &x, Comp1d const &y)	{return sawtooth_dc(x  , y.r);}
	double r_rq_saw					(double const &x, Quat1d const &y)	{return sawtooth_dc(x  , y.r);}
	double r_cr_saw					(Comp1d const &x, double const &y)	{return sawtooth_dc(x.r, y  );}
	double r_cc_saw					(Comp1d const &x, Comp1d const &y)	{return sawtooth_dc(x.r, y.r);}
	double r_cq_saw					(Comp1d const &x, Quat1d const &y)	{return sawtooth_dc(x.r, y.r);}
	double r_qr_saw					(Quat1d const &x, double const &y)	{return sawtooth_dc(x.r, y  );}
	double r_qc_saw					(Quat1d const &x, Comp1d const &y)	{return sawtooth_dc(x.r, y.r);}
	double r_qq_saw					(Quat1d const &x, Quat1d const &y)	{return sawtooth_dc(x.r, y.r);}
	bool disc_r_saw_i				(Value const &x0, Value const &x1){return std::ceil(x0.r)!=std::ceil(x1.r);}
	bool disc_c_saw_i				(Value const &x0, Value const &x1){return std::ceil(x0.r)!=std::ceil(x1.r);}
	bool disc_q_saw_i				(Value const &x0, Value const &x1){return std::ceil(x0.r)!=std::ceil(x1.r);}
	bool disc_rr_saw_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return sawtooth_dc_disc(x0.r-y0.r, x1.r-y1.r);}
	bool disc_rc_saw_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return sawtooth_dc_disc(x0.r-y0.r, x1.r-y1.r);}
	bool disc_rq_saw_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return sawtooth_dc_disc(x0.r-y0.r, x1.r-y1.r);}
	bool disc_cr_saw_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return sawtooth_dc_disc(x0.r-y0.r, x1.r-y1.r);}
	bool disc_cc_saw_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return sawtooth_dc_disc(x0.r-y0.r, x1.r-y1.r);}
	bool disc_cq_saw_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return sawtooth_dc_disc(x0.r-y0.r, x1.r-y1.r);}
	bool disc_qr_saw_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return sawtooth_dc_disc(x0.r-y0.r, x1.r-y1.r);}
	bool disc_qc_saw_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return sawtooth_dc_disc(x0.r-y0.r, x1.r-y1.r);}
	bool disc_qq_saw_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return sawtooth_dc_disc(x0.r-y0.r, x1.r-y1.r);}

	double r_rr_hypot					(double const &x, double const &y)	{return sqrt(x*x+y*y);}
	//Comp1d c_cc_hypot					(Comp1d const &x, Comp1d const &y)	{return sqrt(sq(x)+sq(y));}
	//Quat1d q_qq_hypot					(Quat1d const &x, Quat1d const &y)	{return sqrt(sq(x)+sq(y));}

	inline int mandelbrot(Comp1d const &x, int n_iterations)
	{
		double rez=0, imz=0, sq_rez=0, sq_imz=0;
		int k=0;
		for(;k<n_iterations&&sq_rez+sq_imz<16;++k)
		{
			imz=rez*imz;//calculate sq(z)
			imz+=imz;
			rez=sq_rez-sq_imz;

			rez+=x.r, imz+=x.i;//add x

			sq_rez=rez*rez, sq_imz=imz*imz;
		}
		return k;

		//Comp1d z;
		//int k=0;
		//for(;k<n_iterations&&z.real()*z.real()+z.imag()*z.imag()<16;++k)
		//	z=sq(z)+x;
		//return k;
	}
	double r_r_mandelbrot			(double const &x)					{return mandelbrot((Comp1d)(double)x, 200);}
	double r_c_mandelbrot			(Comp1d const &x)					{return mandelbrot(x, 200);}
	double r_rr_mandelbrot			(double const &x, double const &y)	{return mandelbrot((Comp1d)(double)x, (int)(double)y);}
	double r_cr_mandelbrot			(Comp1d const &x, double const &y)	{return mandelbrot(x, (int)(double)y);}
	bool disc_r_mandelbrot_o		(Value const &x0, Value const &x1){return x0.r!=x1.r;}

	double r_rr_min						(double const &x, double const &y)	{return (x+y-abs(x-y))*0.5;}
	Comp1d c_cr_min						(Comp1d const &x, double const &y)	{return (x+y-(x-y).abs())*0.5;}
	Comp1d c_cc_min						(Comp1d const &x, Comp1d const &y)	{return (x+y-(x-y).abs())*0.5;}
	Quat1d q_qq_min						(Quat1d const &x, Quat1d const &y)	{return (x+y-(x-y).abs())*0.5;}

	double r_rr_max						(double const &x, double const &y)	{return (x+y+abs(x-y))*0.5;}
	Comp1d c_cr_max						(Comp1d const &x, double const &y)	{return (x+y+(x-y).abs())*0.5;}
	Comp1d c_cc_max						(Comp1d const &x, Comp1d const &y)	{return (x+y+(x-y).abs())*0.5;}
	Quat1d q_qq_max						(Quat1d const &x, Quat1d const &y)	{return (x+y+(x-y).abs())*0.5;}

	double r_rr_conditional_110			(double const &x, double const &y)	{return AND(y, -(x!=0));}
	Comp1d c_rc_conditional_110			(double const &x, Comp1d const &y)	{return AND(y, -(x!=0));}
	Quat1d q_rq_conditional_110			(double const &x, Quat1d const &y)	{return AND(y, -(x!=0));}
	double r_cr_conditional_110			(Comp1d const &x, double const &y)	{return AND(y, -(x!=0));}
	Comp1d c_cc_conditional_110			(Comp1d const &x, Comp1d const &y)	{return AND(y, -(x!=0));}
	Quat1d q_cq_conditional_110			(Comp1d const &x, Quat1d const &y)	{return AND(y, -(x!=0));}
	double r_qr_conditional_110			(Quat1d const &x, double const &y)	{return AND(y, -(x!=0));}
	Comp1d c_qc_conditional_110			(Quat1d const &x, Comp1d const &y)	{return AND(y, -(x!=0));}
	Quat1d q_qq_conditional_110			(Quat1d const &x, Quat1d const &y)	{return AND(y, -(x!=0));}
	bool disc_conditional_110_i		(Value const &x0, Value const &y0, Value const &x1, Value const &y1){return _1d_zero_in_range(x0.r, x1.r)||_1d_zero_in_range(x0.i, x1.i)||_1d_zero_in_range(x0.j, x1.j)||_1d_zero_in_range(x0.k, x1.k);}

	double r_rr_conditional_101			(double const &x, double const &y)	{return AND(y, -(x==0));}
	Comp1d c_rc_conditional_101			(double const &x, Comp1d const &y)	{return AND(y, -(x==0));}
	Quat1d q_rq_conditional_101			(double const &x, Quat1d const &y)	{return AND(y, -(x==0));}
	double r_cr_conditional_101			(Comp1d const &x, double const &y)	{return AND(y, -(x==0));}
	Comp1d c_cc_conditional_101			(Comp1d const &x, Comp1d const &y)	{return AND(y, -(x==0));}
	Quat1d q_cq_conditional_101			(Comp1d const &x, Quat1d const &y)	{return AND(y, -(x==0));}
	double r_qr_conditional_101			(Quat1d const &x, double const &y)	{return AND(y, -(x==0));}
	Comp1d c_qc_conditional_101			(Quat1d const &x, Comp1d const &y)	{return AND(y, -(x==0));}
	Quat1d q_qq_conditional_101			(Quat1d const &x, Quat1d const &y)	{return AND(y, -(x==0));}
	auto disc_conditional_101_i		=disc_conditional_110_i;

	double  r_r_increment				(double const &x)					{return x+1;}
	Comp1d  c_c_increment				(Comp1d const &x)					{return x+1;}
	Quat1d  q_q_increment				(Quat1d const &x)					{return x+1;}

	double  r_r_decrement				(double const &x)					{return x-1;}
	Comp1d  c_c_decrement				(Comp1d const &x)					{return x-1;}
	Quat1d  q_q_decrement				(Quat1d const &x)					{return x-1;}

	double  r_r_assign					(double const &x)					{return x;}
	Comp1d  c_c_assign					(Comp1d const &x)					{return x;}
	Quat1d  q_q_assign					(Quat1d const &x)					{return x;}
}
