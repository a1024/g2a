//g2_sw.h - Include for software versions of G2 functions.
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

#ifndef GRAPHER_2_G2_SW_H
#define GRAPHER_2_G2_SW_H
namespace	G2
{
	double r_r_ceil(double const& x);
	Comp1d c_c_ceil(Comp1d const& x);
	Quat1d q_q_ceil(Quat1d const& x);
	bool disc_r_ceil_o(Value const& o0, Value const& o1);
	bool disc_c_ceil_o(Value const& o0, Value const& o1);
	bool disc_q_ceil_o(Value const& o0, Value const& o1);

	typedef decltype(&disc_r_ceil_o) DISC_O;//output

	double r_r_floor(double const& x);
	Comp1d c_c_floor(Comp1d const& x);
	Quat1d q_q_floor(Quat1d const& x);
	extern DISC_O disc_r_floor_o;
	extern DISC_O disc_c_floor_o;
	extern DISC_O disc_q_floor_o;

	double r_r_round(double const& x);
	Comp1d c_c_round(Comp1d const& x);
	Quat1d q_q_round(Quat1d const& x);
	extern DISC_O disc_r_round_o;
	extern DISC_O disc_c_round_o;
	extern DISC_O disc_q_round_o;

	double  r_r_int					(double const &x);
	Comp1d  c_c_int					(Comp1d const &x);
	Quat1d  q_q_int					(Quat1d const &x);
	extern DISC_O disc_r_int_o;
	extern DISC_O disc_c_int_o;
	extern DISC_O disc_q_int_o;

	double  r_r_frac				(double const &x);
	Comp1d  c_c_frac				(Comp1d const &x);
	Quat1d  q_q_frac				(Quat1d const &x);
	bool  disc_r_frac_i				(Value const &o0, Value const &o1);
	bool  disc_c_frac_i				(Value const &o0, Value const &o1);
	bool  disc_q_frac_i				(Value const &o0, Value const &o1);

	double r_r_abs(double const& x);
	double r_c_abs(Comp1d const& x);
	double r_q_abs(Quat1d const& x);

	double r_r_arg(double const& x);
	double r_c_arg(Comp1d const& x);
	double r_q_arg(Quat1d const& x);
	bool disc_r_arg_i(Value const& x0, Value const& x1);
	bool disc_c_arg_i(Value const& x0, Value const& x1);
	bool disc_q_arg_i(Value const& x0, Value const& x1);

	typedef decltype(&disc_r_arg_i) DISC_IU;//input, unary

	double r_c_real(Comp1d const& x);
	double r_c_imag(Comp1d const& x);

	//r_r_conjugate: assign
	Comp1d c_c_conjugate(Comp1d const& x);
	Quat1d q_q_conjugate(Quat1d const& x);

	Comp1d c_r_polar(double const& x);
	Comp1d c_c_polar(Comp1d const& x);
	Comp1d c_q_polar(Quat1d const& x);
	extern DISC_IU disc_r_polar_i;
	extern DISC_IU disc_c_polar_i;
	extern DISC_IU disc_q_polar_i;

	//c_r_cartesian	assign
	Comp1d c_c_cartesian(Comp1d const& x);
	Quat1d q_q_cartesian(Quat1d const& x);

	double r_rr_plus(double const& x, double const& y);
	Comp1d c_rc_plus(double const& x, Comp1d const& y);
	Quat1d q_rq_plus(double const& x, Quat1d const& y);
	Comp1d c_cr_plus(Comp1d const& x, double const& y);
	Comp1d c_cc_plus(Comp1d const& x, Comp1d const& y);
	Quat1d q_cq_plus(Comp1d const& x, Quat1d const& y);
	Quat1d q_qr_plus(Quat1d const& x, double const& y);
	Quat1d q_qc_plus(Quat1d const& x, Comp1d const& y);
	Quat1d q_qq_plus(Quat1d const& x, Quat1d const& y);

	double r_r_minus(double const& x);
	Comp1d c_c_minus(Comp1d const& x);
	Quat1d q_q_minus(Quat1d const& x);
	double r_rr_minus(double const& x, double const& y);
	Comp1d c_rc_minus(double const& x, Comp1d const& y);
	Quat1d q_rq_minus(double const& x, Quat1d const& y);
	Comp1d c_cr_minus(Comp1d const& x, double const& y);
	Comp1d c_cc_minus(Comp1d const& x, Comp1d const& y);
	Quat1d q_cq_minus(Comp1d const& x, Quat1d const& y);
	Quat1d q_qr_minus(Quat1d const& x, double const& y);
	Quat1d q_qc_minus(Quat1d const& x, Comp1d const& y);
	Quat1d q_qq_minus(Quat1d const& x, Quat1d const& y);

	double r_rr_multiply(double const& x, double const& y);
	Comp1d c_rc_multiply(double const& x, Comp1d const& y);
	Quat1d q_rq_multiply(double const& x, Quat1d const& y);
	Comp1d c_cr_multiply(Comp1d const& x, double const& y);
	Comp1d c_cc_multiply(Comp1d const& x, Comp1d const& y);
	Quat1d q_cq_multiply(Comp1d const& x, Quat1d const& y);
	Quat1d q_qr_multiply(Quat1d const& x, double const& y);
	Quat1d q_qc_multiply(Quat1d const& x, Comp1d const& y);
	Quat1d q_qq_multiply(Quat1d const& x, Comp1d const& y);

	double r_r_divide(double const& y);
	Comp1d c_c_divide(Comp1d const& y);
	Quat1d q_q_divide(Quat1d const& y);
	double r_rr_divide(double const& x, double const& y);
	Comp1d c_rc_divide(double const& x, Comp1d const& y);
	Quat1d q_rq_divide(double const& x, Quat1d const& y);
	Comp1d c_cr_divide(Comp1d const& x, double const& y);
	Comp1d c_cc_divide(Comp1d const& x, Comp1d const& y);
	Quat1d q_cq_divide(Comp1d const& x, Quat1d const& y);
	Quat1d q_qr_divide(Quat1d const& x, double const& y);
	Quat1d q_qc_divide(Quat1d const& x, Comp1d const& y);
	Quat1d q_qq_divide(Quat1d const& x, Quat1d const& y);
	extern DISC_IU disc_r_divide_i;
	bool disc_c_divide_i(Value const& x0, Value const& x1);
	bool disc_q_divide_i(Value const& x0, Value const& x1);
	bool disc_rr_divide_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_rc_divide_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_rq_divide_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_cr_divide_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_cc_divide_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_cq_divide_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_qr_divide_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_qc_divide_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_qq_divide_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);

	typedef decltype(&disc_rr_divide_i) DISC_IB;//input, binary

	double r_rr_logic_divides(double const& y, double const& x);
	double r_rc_logic_divides(double const& y, Comp1d const& x);
	double r_rq_logic_divides(double const& y, Quat1d const& x);
	double r_cr_logic_divides(Comp1d const& y, double const& x);
	double r_cc_logic_divides(Comp1d const& y, Comp1d const& x);
	double r_cq_logic_divides(Comp1d const& y, Quat1d const& x);
	double r_qr_logic_divides(Quat1d const& y, double const& x);
	double r_qc_logic_divides(Quat1d const& y, Comp1d const& x);
	double r_qq_logic_divides(Quat1d const& y, Quat1d const& x);
	bool disc_r_logic_divides_o		(Value const &x0, Value const &x1);

	double r_rr_power_real			(double const &x, double const &y);
	Comp1d c_cr_power_real			(Comp1d const &x, double const &y);
//	Comp1d c_cr_power_real			(Comp1d const &x, double const &y);
	Quat1d q_qr_power_real			(Quat1d const &x, double const &y);
	bool disc_rr_power_real_i		(Value const &x0, Value const &y0, Value const &x1, Value const &y1);
	extern DISC_IB disc_cr_power_real_i;
	extern DISC_IB disc_qr_power_real_i;

	Comp1d c_cr_pow(Comp1d const& x, double const& y);
	Comp1d c_cc_pow(Comp1d const& x, Comp1d const& y);
	Quat1d q_cq_pow(Comp1d const& x, Quat1d const& y);
	Quat1d q_qr_pow(Quat1d const& x, double const& y);
	Quat1d q_qc_pow(Quat1d const& x, Comp1d const& y);
	Quat1d q_qq_pow(Quat1d const& x, Quat1d const& y);
	bool disc_cr_pow_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_cc_pow_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_cq_pow_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_qr_pow_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_qc_pow_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_qq_pow_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);

	Comp1d c_c_ln(Comp1d const& x);
	Quat1d q_q_ln(Quat1d const& x);
	extern DISC_IU disc_c_ln_i;
	bool disc_q_ln_i(Value const& x0, Value const& x1);

	Comp1d c_c_log(Comp1d const& x);
	Quat1d q_q_log(Quat1d const& x);
	Comp1d c_cr_log(Comp1d const& x, double const& y);
	Comp1d c_cc_log(Comp1d const& x, Comp1d const& y);
	Quat1d q_cq_log(Quat1d const& y, Comp1d const& x);
	Quat1d q_qc_log(Quat1d const& x, Comp1d const& y);
	Quat1d q_qq_log(Quat1d const& x, Quat1d const& y);
	extern DISC_IU disc_c_log_i;
	bool disc_q_log_i(Value const& x0, Value const& x1);
	bool disc_cr_log_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_cc_log_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_cq_log_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_qc_log_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_qq_log_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);

	Comp1d c_rr_tetrate(double const& x, double const& y);
	Comp1d c_rc_tetrate(double const& x, Comp1d const& y);
	Comp1d c_cr_tetrate(Comp1d const& x, double const& y);
	Comp1d c_cc_tetrate(Comp1d const& x, Comp1d const& y);
	Quat1d q_qr_tetrate(Quat1d const& x, double const& y);
	bool disc_rr_tetrate_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_cr_tetrate_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_qr_tetrate_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_rc_tetrate_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_cc_tetrate_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);

	Comp1d c_rr_pentate(double const& x, double const& y);
	Comp1d c_cr_pentate(Comp1d const& x, double const& y);
	bool disc_rr_pentate_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_cr_pentate_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);

	double r_r_bitwise_shift_left_l(double const& x);
	Comp1d c_c_bitwise_shift_left_l(Comp1d const& x);
	Quat1d q_q_bitwise_shift_left_l(Quat1d const& x);
	double r_r_bitwise_shift_left_r(double const& x);
	Comp1d c_c_bitwise_shift_left_r(Comp1d const& x);
	Quat1d q_q_bitwise_shift_left_r(Quat1d const& x);
	double r_rr_bitwise_shift_left(double const& x, double const& y);
	Comp1d c_rc_bitwise_shift_left(double const& x, Comp1d const& y);
	Quat1d q_rq_bitwise_shift_left(double const& x, Quat1d const& y);
	Comp1d c_cr_bitwise_shift_left(Comp1d const& x, double const& y);
	Comp1d c_cc_bitwise_shift_left(Comp1d const& x, Comp1d const& y);
	Quat1d q_cq_bitwise_shift_left(Comp1d const& x, Quat1d const& y);
	Quat1d q_qr_bitwise_shift_left(Quat1d const& x, double const& y);
	Quat1d q_qc_bitwise_shift_left(Quat1d const& x, Comp1d const& y);
	Quat1d q_qq_bitwise_shift_left(Quat1d const& x, Quat1d const& y);
	extern DISC_O disc_r_bitwise_shift_left_l_o;
	extern DISC_O disc_c_bitwise_shift_left_l_o;
	extern DISC_O disc_q_bitwise_shift_left_l_o;
	bool disc_rr_bitwise_shift_left_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_rc_bitwise_shift_left_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_rq_bitwise_shift_left_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_cr_bitwise_shift_left_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_cc_bitwise_shift_left_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_cq_bitwise_shift_left_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_qr_bitwise_shift_left_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_qc_bitwise_shift_left_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_qq_bitwise_shift_left_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);

	double r_r_bitwise_shift_right_l(double const& x);
	Comp1d c_c_bitwise_shift_right_l(Comp1d const& x);
	Quat1d q_q_bitwise_shift_right_l(Quat1d const& x);
	double r_r_bitwise_shift_right_r(double const& x);
	Comp1d c_c_bitwise_shift_right_r(Comp1d const& x);
	Quat1d q_q_bitwise_shift_right_r(Quat1d const& x);
	double r_rr_bitwise_shift_right(double const& x, double const& y);
	Comp1d c_rc_bitwise_shift_right(double const& x, Comp1d const& y);
	Quat1d q_rq_bitwise_shift_right(double const& x, Quat1d const& y);
	Comp1d c_cr_bitwise_shift_right(Comp1d const& x, double const& y);
	Comp1d c_cc_bitwise_shift_right(Comp1d const& x, Comp1d const& y);
	Quat1d q_cq_bitwise_shift_right(Comp1d const& x, Quat1d const& y);
	Quat1d q_qr_bitwise_shift_right(Quat1d const& x, double const& y);
	Quat1d q_qc_bitwise_shift_right(Quat1d const& x, Comp1d const& y);
	Quat1d q_qq_bitwise_shift_right(Quat1d const& x, Quat1d const& y);
	extern DISC_O disc_r_bitwise_shift_right_l_o;
	extern DISC_O disc_c_bitwise_shift_right_l_o;
	extern DISC_O disc_q_bitwise_shift_right_l_o;
	extern DISC_IB disc_rr_bitwise_shift_right_i;
	extern DISC_IB disc_rc_bitwise_shift_right_i;
	extern DISC_IB disc_rq_bitwise_shift_right_i;
	extern DISC_IB disc_cr_bitwise_shift_right_i;
	extern DISC_IB disc_cc_bitwise_shift_right_i;
	extern DISC_IB disc_cq_bitwise_shift_right_i;
	extern DISC_IB disc_qr_bitwise_shift_right_i;
	extern DISC_IB disc_qc_bitwise_shift_right_i;
	extern DISC_IB disc_qq_bitwise_shift_right_i;

	double r_r_bitwise_not(double const& x);
	Comp1d c_c_bitwise_not(Comp1d const& x);
	Quat1d q_q_bitwise_not(Quat1d const& x);
	extern DISC_IU disc_r_bitwise_not_i;
	bool disc_c_bitwise_not_i(Value const &x0, Value const &x1);
	bool disc_q_bitwise_not_i(Value const &x0, Value const &x1);

	double r_r_bitwise_and(double const& x);
	Comp1d c_c_bitwise_and(Comp1d const& x);
	Quat1d q_q_bitwise_and(Quat1d const& x);
	double r_rr_bitwise_and(double const& x, double const& y);
	Comp1d c_rc_bitwise_and(double const& x, Comp1d const& y);
	Quat1d q_rq_bitwise_and(double const& x, Quat1d const& y);
	Comp1d c_cr_bitwise_and(Comp1d const& x, double const& y);
	Comp1d c_cc_bitwise_and(Comp1d const& x, Comp1d const& y);
	Quat1d q_cq_bitwise_and(Comp1d const& x, Quat1d const& y);
	Quat1d q_qr_bitwise_and(Quat1d const& x, double const& y);
	Quat1d q_qc_bitwise_and(Quat1d const& x, Comp1d const& y);
	Quat1d q_qq_bitwise_and(Quat1d const& x, Quat1d const& y);
	extern DISC_O disc_r_bitwise_and_o;
	extern DISC_O disc_c_bitwise_and_o;
	extern DISC_O disc_q_bitwise_and_o;

	double r_r_bitwise_nand(double const& x);
	Comp1d c_c_bitwise_nand(Comp1d const& x);
	Quat1d q_q_bitwise_nand(Quat1d const& x);
	double r_rr_bitwise_nand(double const& x, double const& y);
	Comp1d c_rc_bitwise_nand(double const& x, Comp1d const& y);
	Quat1d q_rq_bitwise_nand(double const& x, Quat1d const& y);
	Comp1d c_cr_bitwise_nand(Comp1d const& x, double const& y);
	Comp1d c_cc_bitwise_nand(Comp1d const& x, Comp1d const& y);
	Quat1d q_cq_bitwise_nand(Comp1d const& x, Quat1d const& y);
	Quat1d q_qr_bitwise_nand(Quat1d const& x, double const& y);
	Quat1d q_qc_bitwise_nand(Quat1d const& x, Comp1d const& y);
	Quat1d q_qq_bitwise_nand(Quat1d const& x, Quat1d const& y);
	extern DISC_O disc_r_bitwise_nand_o;
	extern DISC_O disc_c_bitwise_nand_o;
	extern DISC_O disc_q_bitwise_nand_o;

	double r_r_bitwise_or(double const& x);
	Comp1d c_c_bitwise_or(Comp1d const& x);
	Quat1d q_q_bitwise_or(Quat1d const& x);
	double r_rr_bitwise_or(double const& x, double const& y);
	Comp1d c_rc_bitwise_or(double const& x, Comp1d const& y);
	Quat1d q_rq_bitwise_or(double const& x, Quat1d const& y);
	Comp1d c_cr_bitwise_or(Comp1d const& x, double const& y);
	Comp1d c_cc_bitwise_or(Comp1d const& x, Comp1d const& y);
	Quat1d q_cq_bitwise_or(Comp1d const& x, Quat1d const& y);
	Quat1d q_qr_bitwise_or(Quat1d const& x, double const& y);
	Quat1d q_qc_bitwise_or(Quat1d const& x, Comp1d const& y);
	Quat1d q_qq_bitwise_or(Quat1d const& x, Quat1d const& y);
	extern DISC_O disc_r_bitwise_or_o;
	extern DISC_O disc_c_bitwise_or_o;
	extern DISC_O disc_q_bitwise_or_o;

	double r_r_bitwise_nor(double const& x);
	Comp1d c_c_bitwise_nor(Comp1d const& x);
	Quat1d q_q_bitwise_nor(Quat1d const& x);
	double r_rr_bitwise_nor(double const& x, double const& y);
	Comp1d c_rc_bitwise_nor(double const& x, Comp1d const& y);
	Quat1d q_rq_bitwise_nor(double const& x, Quat1d const& y);
	Comp1d c_cr_bitwise_nor(Comp1d const& x, double const& y);
	Comp1d c_cc_bitwise_nor(Comp1d const& x, Comp1d const& y);
	Quat1d q_cq_bitwise_nor(Comp1d const& x, Quat1d const& y);
	Quat1d q_qr_bitwise_nor(Quat1d const& x, double const& y);
	Quat1d q_qc_bitwise_nor(Quat1d const& x, Comp1d const& y);
	Quat1d q_qq_bitwise_nor(Quat1d const& x, Quat1d const& y);
	extern DISC_O disc_r_bitwise_nor_o;
	extern DISC_O disc_c_bitwise_nor_o;
	extern DISC_O disc_q_bitwise_nor_o;

	double r_r_bitwise_xor(double const& x);
	Comp1d c_c_bitwise_xor(Comp1d const& x);
	Quat1d q_q_bitwise_xor(Quat1d const& x);
	double r_rr_bitwise_xor(double const& x, double const& y);
	Comp1d c_rc_bitwise_xor(double const& x, Comp1d const& y);
	Quat1d q_rq_bitwise_xor(double const& x, Quat1d const& y);
	Comp1d c_cr_bitwise_xor(Comp1d const& x, double const& y);
	Comp1d c_cc_bitwise_xor(Comp1d const& x, Comp1d const& y);
	Quat1d q_cq_bitwise_xor(Comp1d const& x, Quat1d const& y);
	Quat1d q_qr_bitwise_xor(Quat1d const& x, double const& y);
	Quat1d q_qc_bitwise_xor(Quat1d const& x, Comp1d const& y);
	Quat1d q_qq_bitwise_xor(Quat1d const& x, Quat1d const& y);
	extern DISC_O disc_r_bitwise_xor_o;
	extern DISC_O disc_c_bitwise_xor_o;
	extern DISC_O disc_q_bitwise_xor_o;

	double r_r_bitwise_xnor(double const& x);
	Comp1d c_c_bitwise_xnor(Comp1d const& x);
	Quat1d q_q_bitwise_xnor(Quat1d const& x);
	double r_rr_bitwise_xnor(double const& x, double const& y);
	Comp1d c_rc_bitwise_xnor(double const& x, Comp1d const& y);
	Quat1d q_rq_bitwise_xnor(double const& x, Quat1d const& y);
	Comp1d c_cr_bitwise_xnor(Comp1d const& x, double const& y);
	Comp1d c_cc_bitwise_xnor(Comp1d const& x, Comp1d const& y);
	Quat1d q_cq_bitwise_xnor(Comp1d const& x, Quat1d const& y);
	Quat1d q_qr_bitwise_xnor(Quat1d const& x, double const& y);
	Quat1d q_qc_bitwise_xnor(Quat1d const& x, Comp1d const& y);
	Quat1d q_qq_bitwise_xnor(Quat1d const& x, Quat1d const& y);
	extern DISC_O disc_r_bitwise_xnor_o;
	extern DISC_O disc_c_bitwise_xnor_o;
	extern DISC_O disc_q_bitwise_xnor_o;

	double r_r_logic_equal(double const& x);
	double r_c_logic_equal(Comp1d const& x);
	double r_q_logic_equal(Quat1d const& x);
	double r_rr_logic_equal(double const& x, double const& y);
	double r_rc_logic_equal(double const& x, Comp1d const& y);
	double r_rq_logic_equal(double const& x, Quat1d const& y);
	double r_cr_logic_equal(Comp1d const& x, double const& y);
	double r_cc_logic_equal(Comp1d const& x, Comp1d const& y);
	double r_cq_logic_equal(Comp1d const& x, Quat1d const& y);
	double r_qr_logic_equal(Quat1d const& x, double const& y);
	double r_qc_logic_equal(Quat1d const& x, Comp1d const& y);
	double r_qq_logic_equal(Quat1d const& x, Quat1d const& y);
	extern DISC_O disc_r_logic_equal_o;

	double r_r_logic_not_equal(double const& x);
	double r_c_logic_not_equal(Comp1d const& x);
	double r_q_logic_not_equal(Quat1d const& x);
	double r_rr_logic_not_equal(double const& x, double const& y);
	double r_rc_logic_not_equal(double const& x, Comp1d const& y);
	double r_rq_logic_not_equal(double const& x, Quat1d const& y);
	double r_cr_logic_not_equal(Comp1d const& x, double const& y);
	double r_cc_logic_not_equal(Comp1d const& x, Comp1d const& y);
	double r_cq_logic_not_equal(Comp1d const& x, Quat1d const& y);
	double r_qr_logic_not_equal(Quat1d const& x, double const& y);
	double r_qc_logic_not_equal(Quat1d const& x, Comp1d const& y);
	double r_qq_logic_not_equal(Quat1d const& x, Quat1d const& y);
	extern DISC_O disc_r_logic_not_equal_o;

	double r_r_logic_less_l(double const& x);
	double r_c_logic_less_l(Comp1d const& x);
	double r_q_logic_less_l(Quat1d const& x);
	double r_r_logic_less_r(double const& x);
	double r_c_logic_less_r(Comp1d const& x);
	double r_q_logic_less_r(Quat1d const& x);
	double r_rr_logic_less(double const& x, double const& y);
	double r_rc_logic_less(double const& x, Comp1d const& y);
	double r_rq_logic_less(double const& x, Quat1d const& y);
	double r_cr_logic_less(Comp1d const& x, double const& y);
	double r_cc_logic_less(Comp1d const& x, Comp1d const& y);
	double r_cq_logic_less(Comp1d const& x, Quat1d const& y);
	double r_qr_logic_less(Quat1d const& x, double const& y);
	double r_qc_logic_less(Quat1d const& x, Comp1d const& y);
	double r_qq_logic_less(Quat1d const& x, Quat1d const& y);
	extern DISC_O disc_r_logic_less_o;

	double r_r_logic_less_equal_l(double const& x);
	double r_c_logic_less_equal_l(Comp1d const& x);
	double r_q_logic_less_equal_l(Quat1d const& x);
	double r_r_logic_less_equal_r(double const& x);
	double r_c_logic_less_equal_r(Comp1d const& x);
	double r_q_logic_less_equal_r(Quat1d const& x);
	double r_rr_logic_less_equal(double const& x, double const& y);
	double r_rc_logic_less_equal(double const& x, Comp1d const& y);
	double r_rq_logic_less_equal(double const& x, Quat1d const& y);
	double r_cr_logic_less_equal(Comp1d const& x, double const& y);
	double r_cc_logic_less_equal(Comp1d const& x, Comp1d const& y);
	double r_cq_logic_less_equal(Comp1d const& x, Quat1d const& y);
	double r_qr_logic_less_equal(Quat1d const& x, double const& y);
	double r_qc_logic_less_equal(Quat1d const& x, Comp1d const& y);
	double r_qq_logic_less_equal(Quat1d const& x, Quat1d const& y);
	extern DISC_O disc_r_logic_less_equal_o;

	double r_r_logic_greater_l(double const& x);
	double r_c_logic_greater_l(Comp1d const& x);
	double r_q_logic_greater_l(Quat1d const& x);
	double r_r_logic_greater_r(double const& x);
	double r_c_logic_greater_r(Comp1d const& x);
	double r_q_logic_greater_r(Quat1d const& x);
	double r_rr_logic_greater(double const& x, double const& y);
	double r_rc_logic_greater(double const& x, Comp1d const& y);
	double r_rq_logic_greater(double const& x, Quat1d const& y);
	double r_cr_logic_greater(Comp1d const& x, double const& y);
	double r_cc_logic_greater(Comp1d const& x, Comp1d const& y);
	double r_cq_logic_greater(Comp1d const& x, Quat1d const& y);
	double r_qr_logic_greater(Quat1d const& x, double const& y);
	double r_qc_logic_greater(Quat1d const& x, Comp1d const& y);
	double r_qq_logic_greater(Quat1d const& x, Quat1d const& y);
	extern DISC_O disc_r_logic_greater_o;

	double r_r_logic_greater_equal_l(double const& x);
	double r_c_logic_greater_equal_l(Comp1d const& x);
	double r_q_logic_greater_equal_l(Quat1d const& x);
	double r_r_logic_greater_equal_r(double const& x);
	double r_c_logic_greater_equal_r(Comp1d const& x);
	double r_q_logic_greater_equal_r(Quat1d const& x);
	double r_rr_logic_greater_equal(double const& x, double const& y);
	double r_rc_logic_greater_equal(double const& x, Comp1d const& y);
	double r_rq_logic_greater_equal(double const& x, Quat1d const& y);
	double r_cr_logic_greater_equal(Comp1d const& x, double const& y);
	double r_cc_logic_greater_equal(Comp1d const& x, Comp1d const& y);
	double r_cq_logic_greater_equal(Comp1d const& x, Quat1d const& y);
	double r_qr_logic_greater_equal(Quat1d const& x, double const& y);
	double r_qc_logic_greater_equal(Quat1d const& x, Comp1d const& y);
	double r_qq_logic_greater_equal(Quat1d const& x, Quat1d const& y);
	extern DISC_O disc_r_logic_greater_equal_o;

	double r_r_logic_not(double const& x);
	double r_c_logic_not(Comp1d const& x);
	double r_q_logic_not(Quat1d const& x);
	extern DISC_O disc_r_logic_not_o;

	double r_rr_logic_and(double const& x, double const& y);
	double r_rc_logic_and(double const& x, Comp1d const& y);
	double r_rq_logic_and(double const& x, Quat1d const& y);
	double r_cr_logic_and(Comp1d const& x, double const& y);
	double r_cc_logic_and(Comp1d const& x, Comp1d const& y);
	double r_cq_logic_and(Comp1d const& x, Quat1d const& y);
	double r_qr_logic_and(Quat1d const& x, double const& y);
	double r_qc_logic_and(Quat1d const& x, Comp1d const& y);
	double r_qq_logic_and(Quat1d const& x, Quat1d const& y);
	extern DISC_O disc_r_logic_and_o;

	double r_rr_logic_or(double const& x, double const& y);
	double r_rc_logic_or(double const& x, Comp1d const& y);
	double r_rq_logic_or(double const& x, Quat1d const& y);
	double r_cr_logic_or(Comp1d const& x, double const& y);
	double r_cc_logic_or(Comp1d const& x, Comp1d const& y);
	double r_cq_logic_or(Comp1d const& x, Quat1d const& y);
	double r_qr_logic_or(Quat1d const& x, double const& y);
	double r_qc_logic_or(Quat1d const& x, Comp1d const& y);
	double r_qq_logic_or(Quat1d const& x, Quat1d const& y);
	extern DISC_O disc_r_logic_or_o;

	double r_rr_logic_xor(double const& x, double const& y);
	double r_rc_logic_xor(double const& x, Comp1d const& y);
	double r_rq_logic_xor(double const& x, Quat1d const& y);
	double r_cr_logic_xor(Comp1d const& x, double const& y);
	double r_cc_logic_xor(Comp1d const& x, Comp1d const& y);
	double r_cq_logic_xor(Comp1d const& x, Quat1d const& y);
	double r_qr_logic_xor(Quat1d const& x, double const& y);
	double r_qc_logic_xor(Quat1d const& x, Comp1d const& y);
	double r_qq_logic_xor(Quat1d const& x, Quat1d const& y);
	extern DISC_O disc_r_logic_xor_o;

	double r_rr_condition_zero(double const& x, double const& y);
	Comp1d c_rc_condition_zero(double const& x, Comp1d const& y);
	Quat1d q_rq_condition_zero(double const& x, Quat1d const& y);
	Comp1d c_cr_condition_zero(Comp1d const& x, double const& y);
	Comp1d c_cc_condition_zero(Comp1d const& x, Comp1d const& y);
	Quat1d q_cq_condition_zero(Comp1d const& x, Quat1d const& y);
	Quat1d q_qr_condition_zero(Quat1d const& x, double const& y);
	Quat1d q_qc_condition_zero(Quat1d const& x, Comp1d const& y);
	Quat1d q_qq_condition_zero(Quat1d const& x, Quat1d const& y);
	bool disc_rr_condition_zero_i	(Value const &x0, Value const &y0, Value const &x1, Value const &y1);
	bool disc_rc_condition_zero_i	(Value const &x0, Value const &y0, Value const &x1, Value const &y1);
	bool disc_rq_condition_zero_i	(Value const &x0, Value const &y0, Value const &x1, Value const &y1);
	bool disc_cr_condition_zero_i	(Value const &x0, Value const &y0, Value const &x1, Value const &y1);
	bool disc_cc_condition_zero_i	(Value const &x0, Value const &y0, Value const &x1, Value const &y1);
	bool disc_cq_condition_zero_i	(Value const &x0, Value const &y0, Value const &x1, Value const &y1);
	bool disc_qr_condition_zero_i	(Value const &x0, Value const &y0, Value const &x1, Value const &y1);
	bool disc_qc_condition_zero_i	(Value const &x0, Value const &y0, Value const &x1, Value const &y1);
	bool disc_qq_condition_zero_i	(Value const &x0, Value const &y0, Value const &x1, Value const &y1);

	double r_r_percent(double const& x);
	Comp1d c_c_percent(Comp1d const& x);
	Quat1d q_q_percent(Quat1d const& x);

	double r_rr_modulo(double const& x, double const& y);
	Comp1d c_rc_modulo(double const& x, Comp1d const& y);
	Quat1d q_rq_modulo(double const& x, Quat1d const& y);
	Comp1d c_cr_modulo(Comp1d const& x, double const& y);
	Comp1d c_cc_modulo(Comp1d const& x, Comp1d const& y);
	Quat1d q_cq_modulo(Comp1d const& x, Quat1d const& y);
	Quat1d q_qr_modulo(Quat1d const& x, double const& y);
	Quat1d q_qc_modulo(Quat1d const& x, Comp1d const& y);
	Quat1d q_qq_modulo(Quat1d const& x, Quat1d const& y);
	bool disc_rr_modulo_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_rc_modulo_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_rq_modulo_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_cr_modulo_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_cc_modulo_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_cq_modulo_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_qr_modulo_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_qc_modulo_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_qq_modulo_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);

	double r_r_sgn(double const& x);
	Comp1d c_c_sgn(Comp1d const& x);
	Quat1d q_q_sgn(Quat1d const& x);
	extern DISC_IU disc_r_sgn_i;
	extern DISC_IU disc_c_sgn_i;
	extern DISC_IU disc_q_sgn_i;

	double r_r_sq(double const& x);
	Comp1d c_c_sq(Comp1d const& x);
	Quat1d q_q_sq(Quat1d const& x);
	Comp1d c_c_sqrt(Comp1d const& x);
	Quat1d q_q_sqrt(Quat1d const& x);

	double r_r_invsqrt(double const& x);

	double r_r_cbrt(double const& x);
	Comp1d c_c_cbrt(Comp1d const& x);
	Quat1d q_q_cbrt(Quat1d const& x);
	//Comp1d  c_c_cbrt					(Comp1d const &x);
	//Quat1d  q_q_cbrt					(Quat1d const &x);

	double r_r_gauss(double const& x);
	Comp1d c_c_gauss(Comp1d const& x);
	Quat1d q_q_gauss(Quat1d const& x);

	double r_r_erf(double const& x);

	double r_r_zeta(double const& s);
	bool disc_r_zeta_i(Value const& x0, Value const& x1);
	bool disc_c_zeta_i(Value const& x0, Value const& x1);
	bool disc_q_zeta_i(Value const& x0, Value const& x1);

	double r_r_tgamma(double const& x);
	Comp1d c_c_tgamma(Comp1d const& x);
	Quat1d q_q_tgamma(Quat1d const& x);
	double r_rr_tgamma(double const& x, double const& y);
	bool disc_r_tgamma_i(Value const& x0, Value const& x1);
	bool disc_c_tgamma_i(Value const& x0, Value const& x1);
	bool disc_q_tgamma_i(Value const& x0, Value const& x1);
	bool disc_rr_tgamma_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);

	double r_r_loggamma(double const& x);
	bool disc_r_loggamma_i(Value const& x0, Value const& x1);

	double r_r_factorial(double const& x);
	Comp1d c_c_factorial(Comp1d const& x);
	Quat1d q_q_factorial(Quat1d const& x);
	bool disc_r_factorial_i(Value const& x0, Value const& x1);
	bool disc_c_factorial_i(Value const& x0, Value const& x1);
	bool disc_q_factorial_i(Value const& x0, Value const& x1);

	double r_r_permutation(double const& x);
	Comp1d c_c_permutation(Comp1d const& x);
	Quat1d q_q_permutation(Quat1d const& x);
	double r_rr_permutation(double const& x, double const& y);
	Comp1d c_cr_permutation(Comp1d const& x, double const& y);
	Comp1d c_cc_permutation(Comp1d const& x, Comp1d const& y);
	Quat1d q_qq_permutation(Quat1d const& x, Quat1d const& y);
	bool disc_rr_permutation_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_rc_permutation_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_rq_permutation_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_cr_permutation_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_cc_permutation_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_cq_permutation_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_qr_permutation_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_qc_permutation_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_qq_permutation_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);

	double r_r_combination(double const& x);
	Comp1d c_c_combination(Comp1d const& x);
	Quat1d q_q_combination(Quat1d const& x);
	double r_rr_combination(double const& x, double const& y);
	Comp1d c_cr_combination(Comp1d const& x, double const& y);
	Comp1d c_cc_combination(Comp1d const& x, Comp1d const& y);
	Quat1d q_qq_combination(Quat1d const& x, Quat1d const& y);
	bool disc_rr_combination_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_rc_combination_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_rq_combination_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_cr_combination_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_cc_combination_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_cq_combination_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_qr_combination_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_qc_combination_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);
	bool disc_qq_combination_i(Value const& x0, Value const& y0, Value const& x1, Value const& y1);

	double r_r_cos(double const& x);
	Comp1d c_c_cos(Comp1d const& x);
	Quat1d q_q_cos(Quat1d const& x);

	Comp1d c_c_acos(Comp1d const& x);
	Quat1d q_q_acos(Quat1d const& x);
	bool disc_c_acos_i(Value const& x0, Value const& x1);
	bool disc_q_acos_i(Value const& x0, Value const& x1);


	double r_r_cosh(double const& x);
	Comp1d c_c_cosh(Comp1d const& x);
	Quat1d q_q_cosh(Quat1d const& x);

	Comp1d c_c_acosh(Comp1d const& x);
	Quat1d q_q_acosh(Quat1d const& x);

	double r_r_cosc(double const& x);
	Comp1d c_c_cosc(Comp1d const& x);
	Quat1d q_q_cosc(Quat1d const& x);
	extern DISC_IU disc_r_cosc_i;
	extern DISC_IU disc_c_cosc_i;
	extern DISC_IU disc_q_cosc_i;

	double r_r_sec(double const& x);
	Comp1d c_c_sec(Comp1d const& x);
	Quat1d q_q_sec(Quat1d const& x);
	bool disc_r_sec_i(Value const& x0, Value const& x1);
	bool disc_c_sec_i(Value const& x0, Value const& x1);
	bool disc_q_sec_i(Value const& x0, Value const& x1);

	Comp1d c_c_asec(Comp1d const& x);
	Quat1d q_q_asec(Quat1d const& x);
	extern DISC_IU disc_c_asec_i;
	extern DISC_IU disc_q_asec_i;

	double r_r_sech(double const& x);
	Comp1d c_c_sech(Comp1d const& x);
	Quat1d q_q_sech(Quat1d const& x);
	bool disc_c_sech_i(Value const& x0, Value const& x1);
	bool disc_q_sech_i(Value const& x0, Value const& x1);

	Comp1d c_c_asech(Comp1d const& x);
	Quat1d q_q_asech(Quat1d const& x);
	bool disc_c_asech_i(Value const& x0, Value const& x1);
	bool disc_q_asech_i(Value const& x0, Value const& x1);

	double r_r_sin(double const& x);
	Comp1d c_c_sin(Comp1d const& x);
	Quat1d q_q_sin(Quat1d const& x);

	Comp1d c_c_asin(Comp1d const& x);
	Quat1d q_q_asin(Quat1d const& x);
	extern DISC_IU disc_c_asin_i;
	extern DISC_IU disc_q_asin_i;

	double r_r_sinh(double const& x);
	Comp1d c_c_sinh(Comp1d const& x);
	Quat1d q_q_sinh(Quat1d const& x);

	double r_r_asinh(double const& x);
	Comp1d c_c_asinh(Comp1d const& x);
	Quat1d q_q_asinh(Quat1d const& x);
	bool disc_c_asinh_i(Value const& x0, Value const& x1);
	bool disc_q_asinh_i(Value const& x0, Value const& x1);

	double r_r_sinc(double const& x);
	Comp1d c_c_sinc(Comp1d const& x);
	Quat1d q_q_sinc(Quat1d const& x);

	double r_r_sinhc(double const& x);
	Comp1d c_c_sinhc(Comp1d const& x);
	Quat1d q_q_sinhc(Quat1d const& x);

	double r_r_csc(double const& x);
	Comp1d c_c_csc(Comp1d const& x);
	Quat1d q_q_csc(Quat1d const& x);
	bool disc_r_csc_i(Value const& x0, Value const& x1);
	bool disc_c_csc_i(Value const& x0, Value const& x1);
	bool disc_q_csc_i(Value const& x0, Value const& x1);

	Comp1d c_c_acsc(Comp1d const& x);
	Quat1d q_q_acsc(Quat1d const& x);
	bool disc_c_acsc_i(Value const& x0, Value const& x1);
	bool disc_q_acsc_i(Value const& x0, Value const& x1);

	double r_r_csch(double const& x);
	Comp1d c_c_csch(Comp1d const& x);
	Quat1d q_q_csch(Quat1d const& x);
	extern DISC_IU disc_r_csch_i;
	bool disc_c_csch_i(Value const& x0, Value const& x1);
	bool disc_q_csch_i(Value const& x0, Value const& x1);

	double r_r_acsch(double const& x);
	Comp1d c_c_acsch(Comp1d const& x);
	Quat1d q_q_acsch(Quat1d const& x);
	extern DISC_IU disc_r_acsch_i;
	bool disc_c_acsch_i(Value const& x0, Value const& x1);
	bool disc_q_acsch_i(Value const& x0, Value const& x1);

	double r_r_tan(double const& x);
	Comp1d c_c_tan(Comp1d const& x);
	Quat1d q_q_tan(Quat1d const& x);
	extern DISC_IU disc_r_tan_i;
	extern DISC_IU disc_c_tan_i;
	extern DISC_IU disc_q_tan_i;

	double  r_r_atan					(double const &x);
	Comp1d  c_c_atan					(Comp1d const &x);
	Quat1d  q_q_atan					(Quat1d const &x);
	double r_rr_atan					(double const &x, double const &y);
	Comp1d c_rc_atan					(double const &x, Comp1d const &y);
	Quat1d q_rq_atan					(double const &x, Quat1d const &y);
	Comp1d c_cr_atan					(Comp1d const &x, double const &y);
	Comp1d c_cc_atan					(Comp1d const &x, Comp1d const &y);
	Quat1d q_cq_atan					(Comp1d const &x, Quat1d const &y);
	Quat1d q_qr_atan					(Quat1d const &x, double const &y);
	Quat1d q_qc_atan					(Quat1d const &x, Comp1d const &y);
	Quat1d q_qq_atan					(Quat1d const &x, Quat1d const &y);
	bool disc_c_atan_i				(Value const &x0, Value const &x1);
	bool disc_q_atan_i				(Value const &x0, Value const &x1);
	bool disc_rr_atan_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1);
	bool disc_rc_atan_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1);
	bool disc_rq_atan_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1);
	bool disc_cr_atan_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1);
	bool disc_cc_atan_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1);
	bool disc_cq_atan_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1);
	bool disc_qr_atan_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1);
	bool disc_qc_atan_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1);
	bool disc_qq_atan_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1);

	double  r_r_tanh					(double const &x);
	Comp1d  c_c_tanh					(Comp1d const &x);
	Quat1d  q_q_tanh					(Quat1d const &x);
	//extern DISC_IU disc_c_tanh_i;
	//extern DISC_IU disc_q_tanh_i;

	Comp1d  c_c_atanh					(Comp1d const &x);
	Quat1d  q_q_atanh					(Quat1d const &x);
	bool disc_c_atanh_i				(Value const &x0, Value const &x1);
	bool disc_q_atanh_i				(Value const &x0, Value const &x1);

	double  r_r_tanc					(double const &x);
	Comp1d  c_c_tanc					(Comp1d const &x);
	Quat1d  q_q_tanc					(Quat1d const &x);
	extern DISC_IU disc_r_tanc_i;
	extern DISC_IU disc_c_tanc_i;
	extern DISC_IU disc_q_tanc_i;

	double  r_r_cot						(double const &x);
	Comp1d  c_c_cot						(Comp1d const &x);
	Quat1d  q_q_cot						(Quat1d const &x);
	extern DISC_IU disc_r_cot_i;
	extern DISC_IU disc_c_cot_i;
	extern DISC_IU disc_q_cot_i;

	double  r_r_acot					(double const &x);
	Comp1d  c_c_acot					(Comp1d const &x);
	Quat1d  q_q_acot					(Quat1d const &x);
	bool disc_r_acot_i				(Value const &x0, Value const &x1);
	extern DISC_IU disc_c_acot_i;
	extern DISC_IU disc_q_acot_i;

	double  r_r_coth					(double const &x);
	Comp1d  c_c_coth					(Comp1d const &x);
	Quat1d  q_q_coth					(Quat1d const &x);
	extern DISC_IU disc_r_coth_i;
	extern DISC_IU disc_c_coth_i;
	bool disc_q_coth_i				(Value const &x0, Value const &x1);

	Comp1d  c_c_acoth					(Comp1d const &x);
	Quat1d  q_q_acoth					(Quat1d const &x);
	bool disc_c_acoth_i				(Value const &x0, Value const &x1);
	bool disc_q_acoth_i				(Value const &x0, Value const &x1);

	double  r_r_exp						(double const &x);
	Comp1d  c_c_exp						(Comp1d const &x);
	Quat1d  q_q_exp						(Quat1d const &x);

	double  r_r_fib						(double const &x);
	Comp1d  c_c_fib						(Comp1d const &x);
	Quat1d  q_q_fib						(Quat1d const &x);

	double  r_r_random					(double const &x);
	Comp1d  c_c_random					(Comp1d const &x);
	Quat1d  q_q_random					(Quat1d const &x);
	double r_rr_random					(double const &x, double const &y);
	Comp1d c_cr_random					(Comp1d const &x, double const &y);
	Comp1d c_cc_random					(Comp1d const &x, Comp1d const &y);
	Quat1d q_qq_random					(Quat1d const &x, Quat1d const &y);
	bool disc_r_random_o			(Value const &o0, Value const &o1);
	bool disc_c_random_o			(Value const &o0, Value const &o1);
	bool disc_q_random_o			(Value const &o0, Value const &o1);

	double  r_r_beta					(double const &x);
	double r_rr_beta					(double const &x, double const &y);
	bool disc_r_beta_i				(Value const &x0, Value const &x1);
	bool disc_rr_beta_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1);

	double  r_r_bessel_j				(double const &x);
	double r_rr_bessel_j				(double const &x, double const &y);
	bool disc_r_bessel_j_i			(Value const &x0, Value const &x1);
	bool disc_rr_bessel_j_i			(Value const &x0, Value const &y0, Value const &x1, Value const &y1);

	double  r_r_bessel_y				(double const &x);
	double r_rr_bessel_y				(double const &x, double const &y);
	bool disc_r_bessel_y_i			(Value const &x0, Value const &x1);
	bool disc_rr_bessel_y_i			(Value const &x0, Value const &y0, Value const &x1, Value const &y1);

	Comp1d  c_r_hankel1					(double const &x);
	Comp1d  c_c_hankel1					(Comp1d const &x);
	Comp1d c_rr_hankel1					(double const &x, double const &y);
	bool disc_r_hankel1_i			(Value const &x0, Value const &x1);
	bool disc_rr_hankel1_i			(Value const &x0, Value const &y0, Value const &x1, Value const &y1);

	double  r_r_step					(double const &x);
	Comp1d  c_c_step					(Comp1d const &x);
	Quat1d  q_q_step					(Quat1d const &x);
	extern DISC_IU disc_r_step_i;
	extern DISC_IU disc_c_step_i;
	extern DISC_IU disc_q_step_i;

	double  r_r_rect					(double const &x);
	Comp1d  c_c_rect					(Comp1d const &x);
	Quat1d  q_q_rect					(Quat1d const &x);
	bool disc_r_rect_i				(Value const &x0, Value const &x1);
	bool disc_c_rect_i				(Value const &x0, Value const &x1);
	bool disc_q_rect_i				(Value const &x0, Value const &x1);

	double  r_r_trgl					(double const &x);
	double  r_c_trgl					(Comp1d const &x);
	double  r_q_trgl					(Quat1d const &x);

	double  r_r_sqwv					(double const &x);
	double  r_c_sqwv					(Comp1d const &x);
	double  r_q_sqwv					(Quat1d const &x);
	double r_rr_sqwv					(double const &x, double const &y);
	double r_rc_sqwv					(double const &x, Comp1d const &y);
	double r_rq_sqwv					(double const &x, Quat1d const &y);
	double r_cr_sqwv					(Comp1d const &x, double const &y);
	double r_cc_sqwv					(Comp1d const &x, Comp1d const &y);
	double r_cq_sqwv					(Comp1d const &x, Quat1d const &y);
	double r_qr_sqwv					(Quat1d const &x, double const &y);
	double r_qc_sqwv					(Quat1d const &x, Comp1d const &y);
	double r_qq_sqwv					(Quat1d const &x, Quat1d const &y);
	bool disc_r_sqwv_o				(Value const &o0, Value const &o1);

	double  r_r_trwv					(double const &x);
	double  r_c_trwv					(Comp1d const &x);
	double  r_q_trwv					(Quat1d const &x);
	double r_rr_trwv					(double const &x, double const &y);
	double r_rc_trwv					(double const &x, Comp1d const &y);
	double r_rq_trwv					(double const &x, Quat1d const &y);
	double r_cr_trwv					(Comp1d const &x, double const &y);
	double r_cc_trwv					(Comp1d const &x, Comp1d const &y);
	double r_cq_trwv					(Comp1d const &x, Quat1d const &y);
	double r_qr_trwv					(Quat1d const &x, double const &y);
	double r_qc_trwv					(Quat1d const &x, Comp1d const &y);
	double r_qq_trwv					(Quat1d const &x, Quat1d const &y);

	double  r_r_saw					(double const &x);
	double  r_c_saw					(Comp1d const &x);
	double  r_q_saw					(Quat1d const &x);
	double r_rr_saw					(double const &x, double const &y);
	double r_rc_saw					(double const &x, Comp1d const &y);
	double r_rq_saw					(double const &x, Quat1d const &y);
	double r_cr_saw					(Comp1d const &x, double const &y);
	double r_cc_saw					(Comp1d const &x, Comp1d const &y);
	double r_cq_saw					(Comp1d const &x, Quat1d const &y);
	double r_qr_saw					(Quat1d const &x, double const &y);
	double r_qc_saw					(Quat1d const &x, Comp1d const &y);
	double r_qq_saw					(Quat1d const &x, Quat1d const &y);
	bool disc_r_saw_i				(Value const &x0, Value const &x1);
	bool disc_c_saw_i				(Value const &x0, Value const &x1);
	bool disc_q_saw_i				(Value const &x0, Value const &x1);
	bool disc_rr_saw_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1);
	bool disc_rc_saw_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1);
	bool disc_rq_saw_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1);
	bool disc_cr_saw_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1);
	bool disc_cc_saw_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1);
	bool disc_cq_saw_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1);
	bool disc_qr_saw_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1);
	bool disc_qc_saw_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1);
	bool disc_qq_saw_i				(Value const &x0, Value const &y0, Value const &x1, Value const &y1);

	double r_rr_hypot					(double const &x, double const &y);
	//Comp1d c_cc_hypot					(Comp1d const &x, Comp1d const &y);
	//Quat1d q_qq_hypot					(Quat1d const &x, Quat1d const &y);

	double r_r_mandelbrot			(double const &x);
	double r_c_mandelbrot			(Comp1d const &x);
	double r_rr_mandelbrot			(double const &x, double const &y);
	double r_cr_mandelbrot			(Comp1d const &x, double const &y);
	bool disc_r_mandelbrot_o		(Value const &x0, Value const &x1);

	double r_rr_min						(double const &x, double const &y);
	Comp1d c_cr_min						(Comp1d const &x, double const &y);
	Comp1d c_cc_min						(Comp1d const &x, Comp1d const &y);
	Quat1d q_qq_min						(Quat1d const &x, Quat1d const &y);

	double r_rr_max						(double const &x, double const &y);
	Comp1d c_cr_max						(Comp1d const &x, double const &y);
	Comp1d c_cc_max						(Comp1d const &x, Comp1d const &y);
	Quat1d q_qq_max						(Quat1d const &x, Quat1d const &y);

	double r_rr_conditional_110			(double const &x, double const &y);
	Comp1d c_rc_conditional_110			(double const &x, Comp1d const &y);
	Quat1d q_rq_conditional_110			(double const &x, Quat1d const &y);
	double r_cr_conditional_110			(Comp1d const &x, double const &y);
	Comp1d c_cc_conditional_110			(Comp1d const &x, Comp1d const &y);
	Quat1d q_cq_conditional_110			(Comp1d const &x, Quat1d const &y);
	double r_qr_conditional_110			(Quat1d const &x, double const &y);
	Comp1d c_qc_conditional_110			(Quat1d const &x, Comp1d const &y);
	Quat1d q_qq_conditional_110			(Quat1d const &x, Quat1d const &y);
	bool disc_conditional_110_i		(Value const &x0, Value const &y0, Value const &x1, Value const &y1);

	double r_rr_conditional_101			(double const &x, double const &y);
	Comp1d c_rc_conditional_101			(double const &x, Comp1d const &y);
	Quat1d q_rq_conditional_101			(double const &x, Quat1d const &y);
	double r_cr_conditional_101			(Comp1d const &x, double const &y);
	Comp1d c_cc_conditional_101			(Comp1d const &x, Comp1d const &y);
	Quat1d q_cq_conditional_101			(Comp1d const &x, Quat1d const &y);
	double r_qr_conditional_101			(Quat1d const &x, double const &y);
	Comp1d c_qc_conditional_101			(Quat1d const &x, Comp1d const &y);
	Quat1d q_qq_conditional_101			(Quat1d const &x, Quat1d const &y);
	extern DISC_IB disc_conditional_101_i;

	double  r_r_increment				(double const &x);
	Comp1d  c_c_increment				(Comp1d const &x);
	Quat1d  q_q_increment				(Quat1d const &x);

	double  r_r_decrement				(double const &x);
	Comp1d  c_c_decrement				(Comp1d const &x);
	Quat1d  q_q_decrement				(Quat1d const &x);

	double  r_r_assign					(double const &x);
	Comp1d  c_c_assign					(Comp1d const &x);
	Quat1d  q_q_assign					(Quat1d const &x);
}
#endif //GRAPHER_2_G2_SW_H
