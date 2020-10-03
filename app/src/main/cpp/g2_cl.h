//g2_cl.h - Include for OpenCL extension for Grapher 2A.
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
// Created by MSI on 9/24/2020.
//

#ifndef GRAPHER_2_G2_CL_H
#define GRAPHER_2_G2_CL_H
#include		"g2_common.h"
enum 			CLKernelIdx
{
	CL_NOOP,
//	V_INITIALIZE_CONSTANTS,
	V_INITIALIZE_PARAMETER,
	V_C2D_RGB,

	R_R_SETZERO, C_C_SETZERO, Q_Q_SETZERO,
	R_R_CEIL, C_C_CEIL, Q_Q_CEIL,		DISC_R_CEIL_O, DISC_C_CEIL_O, DISC_Q_CEIL_O,
	R_R_FLOOR, C_C_FLOOR, Q_Q_FLOOR,	DISC_R_FLOOR_O, DISC_C_FLOOR_O, DISC_Q_FLOOR_O,
	R_R_ROUND, C_C_ROUND, Q_Q_ROUND,	DISC_R_ROUND_O, DISC_C_ROUND_O, DISC_Q_ROUND_O,
	R_R_INT, C_C_INT, Q_Q_INT,			DISC_R_INT_O, DISC_C_INT_O, DISC_Q_INT_O,
	R_R_FRAC, C_C_FRAC, Q_Q_FRAC,		DISC_R_FRAC_I, DISC_C_FRAC_I, DISC_Q_FRAC_I,
	R_R_ABS, R_C_ABS, R_Q_ABS,
	R_R_ARG, R_C_ARG, R_Q_ARG,			DISC_R_ARG_I, DISC_C_ARG_I, DISC_Q_ARG_I,
	R_C_REAL,
	R_C_IMAG,
	C_C_CONJUGATE, Q_Q_CONJUGATE,
	C_R_POLAR, C_C_POLAR, C_Q_POLAR,	DISC_R_POLAR_I, DISC_C_POLAR_I, DISC_Q_POLAR_I,
	C_C_CARTESIAN, Q_Q_CARTESIAN,
	R_RR_PLUS, C_RC_PLUS, Q_RQ_PLUS, C_CR_PLUS, C_CC_PLUS, Q_CQ_PLUS, Q_QR_PLUS, Q_QC_PLUS, Q_QQ_PLUS,
	R_R_MINUS, C_C_MINUS, Q_Q_MINUS, R_RR_MINUS, C_RC_MINUS, Q_RQ_MINUS, C_CR_MINUS, C_CC_MINUS, Q_CQ_MINUS, Q_QR_MINUS, Q_QC_MINUS, Q_QQ_MINUS,
	R_RR_MULTIPLY, C_RC_MULTIPLY, Q_RQ_MULTIPLY, C_CR_MULTIPLY, C_CC_MULTIPLY, Q_CQ_MULTIPLY, Q_QR_MULTIPLY, Q_QC_MULTIPLY, Q_QQ_MULTIPLY,
	R_R_DIVIDE, C_C_DIVIDE, Q_Q_DIVIDE, R_RR_DIVIDE, C_RC_DIVIDE, Q_RQ_DIVIDE, C_CR_DIVIDE, C_CC_DIVIDE, Q_CQ_DIVIDE, Q_QR_DIVIDE, Q_QC_DIVIDE, Q_QQ_DIVIDE,
	DISC_R_DIVIDE_I, DISC_C_DIVIDE_I, DISC_Q_DIVIDE_I, DISC_RR_DIVIDE_I, DISC_RC_DIVIDE_I, DISC_RQ_DIVIDE_I, DISC_CR_DIVIDE_I, DISC_CC_DIVIDE_I, DISC_CQ_DIVIDE_I, DISC_QR_DIVIDE_I, DISC_QC_DIVIDE_I, DISC_QQ_DIVIDE_I,

	R_RR_LOGIC_DIVIDES, R_RC_LOGIC_DIVIDES, R_RQ_LOGIC_DIVIDES, R_CR_LOGIC_DIVIDES, R_CC_LOGIC_DIVIDES, R_CQ_LOGIC_DIVIDES, R_QR_LOGIC_DIVIDES, R_QC_LOGIC_DIVIDES, R_QQ_LOGIC_DIVIDES,
	DISC_R_LOGIC_DIVIDES_O,

	R_RR_POWER_REAL, C_CR_POWER_REAL, Q_QR_POWER_REAL,						DISC_RR_POWER_REAL_I, DISC_CR_POWER_REAL_I, DISC_QR_POWER_REAL_I,
	C_CR_POW, C_CC_POW, Q_CQ_POW, Q_QR_POW, Q_QC_POW, Q_QQ_POW,
	DISC_CR_POW_I, DISC_CC_POW_I, DISC_CQ_POW_I, DISC_QR_POW_I, DISC_QC_POW_I, DISC_QQ_POW_I,

	C_C_LN, Q_Q_LN,															DISC_C_LN_I, DISC_Q_LN_I,
	C_C_LOG,  Q_Q_LOG, C_CR_LOG, C_CC_LOG, Q_CQ_LOG, Q_QC_LOG, Q_QQ_LOG,	DISC_C_LOG_I, DISC_Q_LOG_I, DISC_CR_LOG_I, DISC_CC_LOG_I, DISC_CQ_LOG_I, DISC_QC_LOG_I, DISC_QQ_LOG_I,
	C_RR_TETRATE, C_RC_TETRATE, C_CR_TETRATE, C_CC_TETRATE, Q_QR_TETRATE,	DISC_RR_TETRATE_I, DISC_CR_TETRATE_I, DISC_QR_TETRATE_I, DISC_RC_TETRATE_I, DISC_CC_TETRATE_I,
	C_RR_PENTATE, C_CR_PENTATE,												DISC_RR_PENTATE_I, DISC_CR_PENTATE_I,
	R_R_BITWISE_SHIFT_LEFT_L, C_C_BITWISE_SHIFT_LEFT_L, Q_Q_BITWISE_SHIFT_LEFT_L, R_R_BITWISE_SHIFT_LEFT_R, C_C_BITWISE_SHIFT_LEFT_R, Q_Q_BITWISE_SHIFT_LEFT_R, R_RR_BITWISE_SHIFT_LEFT, C_RC_BITWISE_SHIFT_LEFT, Q_RQ_BITWISE_SHIFT_LEFT, C_CR_BITWISE_SHIFT_LEFT, C_CC_BITWISE_SHIFT_LEFT, Q_CQ_BITWISE_SHIFT_LEFT, Q_QR_BITWISE_SHIFT_LEFT, Q_QC_BITWISE_SHIFT_LEFT, Q_QQ_BITWISE_SHIFT_LEFT,
	DISC_R_BITWISE_SHIFT_LEFT_L_O, DISC_C_BITWISE_SHIFT_LEFT_L_O, DISC_Q_BITWISE_SHIFT_LEFT_L_O, DISC_RR_BITWISE_SHIFT_LEFT_I, DISC_RC_BITWISE_SHIFT_LEFT_I, DISC_RQ_BITWISE_SHIFT_LEFT_I, DISC_CR_BITWISE_SHIFT_LEFT_I, DISC_CC_BITWISE_SHIFT_LEFT_I, DISC_CQ_BITWISE_SHIFT_LEFT_I, DISC_QR_BITWISE_SHIFT_LEFT_I, DISC_QC_BITWISE_SHIFT_LEFT_I, DISC_QQ_BITWISE_SHIFT_LEFT_I,

	R_R_BITWISE_SHIFT_RIGHT_L, C_C_BITWISE_SHIFT_RIGHT_L, Q_Q_BITWISE_SHIFT_RIGHT_L, R_R_BITWISE_SHIFT_RIGHT_R, C_C_BITWISE_SHIFT_RIGHT_R, Q_Q_BITWISE_SHIFT_RIGHT_R, R_RR_BITWISE_SHIFT_RIGHT, C_RC_BITWISE_SHIFT_RIGHT, Q_RQ_BITWISE_SHIFT_RIGHT, C_CR_BITWISE_SHIFT_RIGHT, C_CC_BITWISE_SHIFT_RIGHT, Q_CQ_BITWISE_SHIFT_RIGHT, Q_QR_BITWISE_SHIFT_RIGHT, Q_QC_BITWISE_SHIFT_RIGHT, Q_QQ_BITWISE_SHIFT_RIGHT,
	DISC_R_BITWISE_SHIFT_RIGHT_L_O, DISC_C_BITWISE_SHIFT_RIGHT_L_O, DISC_Q_BITWISE_SHIFT_RIGHT_L_O, DISC_RR_BITWISE_SHIFT_RIGHT_I, DISC_RC_BITWISE_SHIFT_RIGHT_I, DISC_RQ_BITWISE_SHIFT_RIGHT_I, DISC_CR_BITWISE_SHIFT_RIGHT_I, DISC_CC_BITWISE_SHIFT_RIGHT_I, DISC_CQ_BITWISE_SHIFT_RIGHT_I, DISC_QR_BITWISE_SHIFT_RIGHT_I, DISC_QC_BITWISE_SHIFT_RIGHT_I, DISC_QQ_BITWISE_SHIFT_RIGHT_I,

	R_R_BITWISE_NOT, C_C_BITWISE_NOT, Q_Q_BITWISE_NOT,						DISC_R_BITWISE_NOT_I, DISC_C_BITWISE_NOT_I, DISC_Q_BITWISE_NOT_I,
	R_R_BITWISE_AND, C_C_BITWISE_AND, Q_Q_BITWISE_AND, R_RR_BITWISE_AND, C_RC_BITWISE_AND, Q_RQ_BITWISE_AND, C_CR_BITWISE_AND, C_CC_BITWISE_AND, Q_CQ_BITWISE_AND, Q_QR_BITWISE_AND, Q_QC_BITWISE_AND, Q_QQ_BITWISE_AND,
	DISC_R_BITWISE_AND_O, DISC_C_BITWISE_AND_O, DISC_Q_BITWISE_AND_O,

	R_R_BITWISE_NAND, C_C_BITWISE_NAND, Q_Q_BITWISE_NAND, R_RR_BITWISE_NAND, C_RC_BITWISE_NAND, Q_RQ_BITWISE_NAND, C_CR_BITWISE_NAND, C_CC_BITWISE_NAND, Q_CQ_BITWISE_NAND, Q_QR_BITWISE_NAND, Q_QC_BITWISE_NAND, Q_QQ_BITWISE_NAND,
	DISC_R_BITWISE_NAND_O, DISC_C_BITWISE_NAND_O, DISC_Q_BITWISE_NAND_O,

	R_R_BITWISE_OR, C_C_BITWISE_OR, Q_Q_BITWISE_OR, R_RR_BITWISE_OR, C_RC_BITWISE_OR, Q_RQ_BITWISE_OR, C_CR_BITWISE_OR, C_CC_BITWISE_OR, Q_CQ_BITWISE_OR, Q_QR_BITWISE_OR, Q_QC_BITWISE_OR, Q_QQ_BITWISE_OR,
	DISC_R_BITWISE_OR_O, DISC_C_BITWISE_OR_O, DISC_Q_BITWISE_OR_O,

	R_R_BITWISE_NOR, C_C_BITWISE_NOR, Q_Q_BITWISE_NOR, R_RR_BITWISE_NOR, C_RC_BITWISE_NOR, Q_RQ_BITWISE_NOR, C_CR_BITWISE_NOR, C_CC_BITWISE_NOR, Q_CQ_BITWISE_NOR, Q_QR_BITWISE_NOR, Q_QC_BITWISE_NOR, Q_QQ_BITWISE_NOR,
	DISC_R_BITWISE_NOR_O, DISC_C_BITWISE_NOR_O, DISC_Q_BITWISE_NOR_O,

	R_R_BITWISE_XOR, C_C_BITWISE_XOR, Q_Q_BITWISE_XOR, R_RR_BITWISE_XOR, C_RC_BITWISE_XOR, Q_RQ_BITWISE_XOR, C_CR_BITWISE_XOR, C_CC_BITWISE_XOR, Q_CQ_BITWISE_XOR, Q_QR_BITWISE_XOR, Q_QC_BITWISE_XOR, Q_QQ_BITWISE_XOR,
	DISC_R_BITWISE_XOR_O, DISC_C_BITWISE_XOR_O, DISC_Q_BITWISE_XOR_O,

	R_R_BITWISE_XNOR, C_C_BITWISE_XNOR, Q_Q_BITWISE_XNOR, R_RR_BITWISE_XNOR, C_RC_BITWISE_XNOR, Q_RQ_BITWISE_XNOR, C_CR_BITWISE_XNOR, C_CC_BITWISE_XNOR, Q_CQ_BITWISE_XNOR, Q_QR_BITWISE_XNOR, Q_QC_BITWISE_XNOR, Q_QQ_BITWISE_XNOR,
	DISC_R_BITWISE_XNOR_O, DISC_C_BITWISE_XNOR_O, DISC_Q_BITWISE_XNOR_O,

	R_R_LOGIC_EQUAL, R_C_LOGIC_EQUAL, R_Q_LOGIC_EQUAL, R_RR_LOGIC_EQUAL, R_RC_LOGIC_EQUAL, R_RQ_LOGIC_EQUAL, R_CR_LOGIC_EQUAL, R_CC_LOGIC_EQUAL, R_CQ_LOGIC_EQUAL, R_QR_LOGIC_EQUAL, R_QC_LOGIC_EQUAL, R_QQ_LOGIC_EQUAL,
	DISC_R_LOGIC_EQUAL_O,

	R_R_LOGIC_NOT_EQUAL, R_C_LOGIC_NOT_EQUAL, R_Q_LOGIC_NOT_EQUAL, R_RR_LOGIC_NOT_EQUAL, R_RC_LOGIC_NOT_EQUAL, R_RQ_LOGIC_NOT_EQUAL, R_CR_LOGIC_NOT_EQUAL, R_CC_LOGIC_NOT_EQUAL, R_CQ_LOGIC_NOT_EQUAL, R_QR_LOGIC_NOT_EQUAL, R_QC_LOGIC_NOT_EQUAL, R_QQ_LOGIC_NOT_EQUAL,
	DISC_R_LOGIC_NOT_EQUAL_O,

	R_R_LOGIC_LESS_L, R_C_LOGIC_LESS_L, R_Q_LOGIC_LESS_L, R_R_LOGIC_LESS_R, R_C_LOGIC_LESS_R, R_Q_LOGIC_LESS_R, R_RR_LOGIC_LESS, R_RC_LOGIC_LESS, R_RQ_LOGIC_LESS, R_CR_LOGIC_LESS, R_CC_LOGIC_LESS, R_CQ_LOGIC_LESS, R_QR_LOGIC_LESS, R_QC_LOGIC_LESS, R_QQ_LOGIC_LESS,
	DISC_R_LOGIC_LESS_O,

	R_R_LOGIC_LESS_EQUAL_L, R_C_LOGIC_LESS_EQUAL_L, R_Q_LOGIC_LESS_EQUAL_L, R_R_LOGIC_LESS_EQUAL_R, R_C_LOGIC_LESS_EQUAL_R, R_Q_LOGIC_LESS_EQUAL_R, R_RR_LOGIC_LESS_EQUAL, R_RC_LOGIC_LESS_EQUAL, R_RQ_LOGIC_LESS_EQUAL, R_CR_LOGIC_LESS_EQUAL, R_CC_LOGIC_LESS_EQUAL, R_CQ_LOGIC_LESS_EQUAL, R_QR_LOGIC_LESS_EQUAL, R_QC_LOGIC_LESS_EQUAL, R_QQ_LOGIC_LESS_EQUAL,
	DISC_R_LOGIC_LESS_EQUAL_O,

	R_R_LOGIC_GREATER_L, R_C_LOGIC_GREATER_L, R_Q_LOGIC_GREATER_L, R_R_LOGIC_GREATER_R, R_C_LOGIC_GREATER_R, R_Q_LOGIC_GREATER_R, R_RR_LOGIC_GREATER, R_RC_LOGIC_GREATER, R_RQ_LOGIC_GREATER, R_CR_LOGIC_GREATER, R_CC_LOGIC_GREATER, R_CQ_LOGIC_GREATER, R_QR_LOGIC_GREATER, R_QC_LOGIC_GREATER, R_QQ_LOGIC_GREATER,
	DISC_R_LOGIC_GREATER_O,

	R_R_LOGIC_GREATER_EQUAL_L, R_C_LOGIC_GREATER_EQUAL_L, R_Q_LOGIC_GREATER_EQUAL_L, R_R_LOGIC_GREATER_EQUAL_R, R_C_LOGIC_GREATER_EQUAL_R, R_Q_LOGIC_GREATER_EQUAL_R, R_RR_LOGIC_GREATER_EQUAL, R_RC_LOGIC_GREATER_EQUAL, R_RQ_LOGIC_GREATER_EQUAL, R_CR_LOGIC_GREATER_EQUAL, R_CC_LOGIC_GREATER_EQUAL, R_CQ_LOGIC_GREATER_EQUAL, R_QR_LOGIC_GREATER_EQUAL, R_QC_LOGIC_GREATER_EQUAL, R_QQ_LOGIC_GREATER_EQUAL,
	DISC_R_LOGIC_GREATER_EQUAL_O,

	R_R_LOGIC_NOT, R_C_LOGIC_NOT, R_Q_LOGIC_NOT,							DISC_R_LOGIC_NOT_O,
	R_RR_LOGIC_AND, R_RC_LOGIC_AND, R_RQ_LOGIC_AND, R_CR_LOGIC_AND, R_CC_LOGIC_AND, R_CQ_LOGIC_AND, R_QR_LOGIC_AND, R_QC_LOGIC_AND, R_QQ_LOGIC_AND,
	DISC_R_LOGIC_AND_O,

	R_RR_LOGIC_OR, R_RC_LOGIC_OR, R_RQ_LOGIC_OR, R_CR_LOGIC_OR, R_CC_LOGIC_OR, R_CQ_LOGIC_OR, R_QR_LOGIC_OR, R_QC_LOGIC_OR, R_QQ_LOGIC_OR,
	DISC_R_LOGIC_OR_O,

	R_RR_LOGIC_XOR, R_RC_LOGIC_XOR, R_RQ_LOGIC_XOR, R_CR_LOGIC_XOR, R_CC_LOGIC_XOR, R_CQ_LOGIC_XOR, R_QR_LOGIC_XOR, R_QC_LOGIC_XOR, R_QQ_LOGIC_XOR,
	DISC_R_LOGIC_XOR_O,

	R_RR_CONDITION_ZERO, C_RC_CONDITION_ZERO, Q_RQ_CONDITION_ZERO, C_CR_CONDITION_ZERO, C_CC_CONDITION_ZERO, Q_CQ_CONDITION_ZERO, Q_QR_CONDITION_ZERO, Q_QC_CONDITION_ZERO, Q_QQ_CONDITION_ZERO,
	DISC_RR_CONDITION_ZERO_I, DISC_RC_CONDITION_ZERO_I, DISC_RQ_CONDITION_ZERO_I, DISC_CR_CONDITION_ZERO_I, DISC_CC_CONDITION_ZERO_I, DISC_CQ_CONDITION_ZERO_I, DISC_QR_CONDITION_ZERO_I, DISC_QC_CONDITION_ZERO_I, DISC_QQ_CONDITION_ZERO_I,

	R_R_PERCENT, C_C_PERCENT, Q_Q_PERCENT,
	R_RR_MODULO, C_RC_MODULO, Q_RQ_MODULO, C_CR_MODULO, C_CC_MODULO, Q_CQ_MODULO, Q_QR_MODULO, Q_QC_MODULO, Q_QQ_MODULO,
	DISC_RR_MODULO_I, DISC_RC_MODULO_I, DISC_RQ_MODULO_I, DISC_CR_MODULO_I, DISC_CC_MODULO_I, DISC_CQ_MODULO_I, DISC_QR_MODULO_I, DISC_QC_MODULO_I, DISC_QQ_MODULO_I,

	R_R_SGN, C_C_SGN, Q_Q_SGN,												DISC_R_SGN_I, DISC_C_SGN_I, DISC_Q_SGN_I,
	R_R_SQ, C_C_SQ, Q_Q_SQ,
	C_C_SQRT, Q_Q_SQRT,
	R_R_INVSQRT,
	R_R_CBRT, C_C_CBRT, Q_Q_CBRT,
	R_R_GAUSS, C_C_GAUSS, Q_Q_GAUSS,
	R_R_ERF,
	R_R_ZETA,																DISC_R_ZETA_I,
	R_R_TGAMMA, C_C_TGAMMA, Q_Q_TGAMMA, R_RR_TGAMMA,						DISC_R_TGAMMA_I, DISC_C_TGAMMA_I, DISC_Q_TGAMMA_I, DISC_RR_TGAMMA_I,
	R_R_LOGGAMMA,															DISC_R_LOGGAMMA_I,
	R_R_FACTORIAL, C_C_FACTORIAL, Q_Q_FACTORIAL,							DISC_R_FACTORIAL_I, DISC_C_FACTORIAL_I, DISC_Q_FACTORIAL_I,
	R_R_PERMUTATION, C_C_PERMUTATION, Q_Q_PERMUTATION, R_RR_PERMUTATION, C_CR_PERMUTATION, C_CC_PERMUTATION, Q_QQ_PERMUTATION,
	DISC_RR_PERMUTATION_I, DISC_RC_PERMUTATION_I, DISC_RQ_PERMUTATION_I, DISC_CR_PERMUTATION_I, DISC_CC_PERMUTATION_I, DISC_CQ_PERMUTATION_I, DISC_QR_PERMUTATION_I, DISC_QC_PERMUTATION_I, DISC_QQ_PERMUTATION_I,

	R_R_COMBINATION, C_C_COMBINATION, Q_Q_COMBINATION, R_RR_COMBINATION, C_CR_COMBINATION, C_CC_COMBINATION, Q_QQ_COMBINATION,
	DISC_RR_COMBINATION_I, DISC_RC_COMBINATION_I, DISC_RQ_COMBINATION_I, DISC_CR_COMBINATION_I, DISC_CC_COMBINATION_I, DISC_CQ_COMBINATION_I, DISC_QR_COMBINATION_I, DISC_QC_COMBINATION_I, DISC_QQ_COMBINATION_I,

	R_R_COS, C_C_COS, Q_Q_COS,
	C_C_ACOS, Q_Q_ACOS,														DISC_C_ACOS_I, DISC_Q_ACOS_I,
	R_R_COSH, C_C_COSH, Q_Q_COSH,
	C_C_ACOSH, Q_Q_ACOSH,
	R_R_COSC, C_C_COSC, Q_Q_COSC,											DISC_R_COSC_I, DISC_C_COSC_I, DISC_Q_COSC_I,
	R_R_SEC, C_C_SEC, Q_Q_SEC,												DISC_R_SEC_I, DISC_C_SEC_I, DISC_Q_SEC_I,
	C_C_ASEC, Q_Q_ASEC,														DISC_C_ASEC_I, DISC_Q_ASEC_I,
	R_R_SECH, C_C_SECH, Q_Q_SECH,											DISC_C_SECH_I, DISC_Q_SECH_I,
	C_C_ASECH, Q_Q_ASECH,													DISC_C_ASECH_I, DISC_Q_ASECH_I,
	R_R_SIN, C_C_SIN, Q_Q_SIN,
	C_C_ASIN, Q_Q_ASIN,														DISC_C_ASIN_I, DISC_Q_ASIN_I,
	R_R_SINH, C_C_SINH, Q_Q_SINH,
	R_R_ASINH, C_C_ASINH, Q_Q_ASINH,										DISC_C_ASINH_I, DISC_Q_ASINH_I,
	R_R_SINC, C_C_SINC, Q_Q_SINC,
	R_R_SINHC, C_C_SINHC, Q_Q_SINHC,
	R_R_CSC, C_C_CSC, Q_Q_CSC,												DISC_R_CSC_I, DISC_C_CSC_I, DISC_Q_CSC_I,
	C_C_ACSC, Q_Q_ACSC,														DISC_C_ACSC_I, DISC_Q_ACSC_I,
	R_R_CSCH, C_C_CSCH, Q_Q_CSCH,											DISC_R_CSCH_I, DISC_C_CSCH_I, DISC_Q_CSCH_I,
	R_R_ACSCH, C_C_ACSCH, Q_Q_ACSCH,										DISC_R_ACSCH_I, DISC_C_ACSCH_I, DISC_Q_ACSCH_I,
	R_R_TAN, C_C_TAN, Q_Q_TAN,												DISC_R_TAN_I, DISC_C_TAN_I, DISC_Q_TAN_I,
	R_R_ATAN, C_C_ATAN, Q_Q_ATAN, R_RR_ATAN, C_RC_ATAN, Q_RQ_ATAN, C_CR_ATAN, C_CC_ATAN, Q_CQ_ATAN, Q_QR_ATAN, Q_QC_ATAN, Q_QQ_ATAN,
	DISC_C_ATAN_I, DISC_Q_ATAN_I, DISC_RR_ATAN_I, DISC_RC_ATAN_I, DISC_RQ_ATAN_I, DISC_CR_ATAN_I, DISC_CC_ATAN_I, DISC_CQ_ATAN_I, DISC_QR_ATAN_I, DISC_QC_ATAN_I, DISC_QQ_ATAN_I,

	R_R_TANH, C_C_TANH, Q_Q_TANH,
	C_C_ATANH, Q_Q_ATANH,													DISC_C_ATANH_I, DISC_Q_ATANH_I,
	R_R_TANC, C_C_TANC, Q_Q_TANC,											DISC_R_TANC_I, DISC_C_TANC_I, DISC_Q_TANC_I,
	R_R_COT, C_C_COT, Q_Q_COT,												DISC_R_COT_I, DISC_C_COT_I, DISC_Q_COT_I,
	R_R_ACOT, C_C_ACOT, Q_Q_ACOT,											DISC_R_ACOT_I, DISC_C_ACOT_I, DISC_Q_ACOT_I,
	R_R_COTH, C_C_COTH, Q_Q_COTH,											DISC_R_COTH_I, DISC_C_COTH_I, DISC_Q_COTH_I,
	C_C_ACOTH, Q_Q_ACOTH,													DISC_C_ACOTH_I, DISC_Q_ACOTH_I,
	R_R_EXP, C_C_EXP, Q_Q_EXP,
	R_R_FIB, C_C_FIB, Q_Q_FIB,
	R_R_RANDOM, C_C_RANDOM, Q_Q_RANDOM, R_RR_RANDOM, C_CR_RANDOM, C_CC_RANDOM, Q_QQ_RANDOM,//nargs determine nD, arg mathtype determines return type
	DISC_R_RANDOM_O, DISC_C_RANDOM_O, DISC_Q_RANDOM_O,

	R_R_BETA, R_RR_BETA,													DISC_R_BETA_I, DISC_RR_BETA_I,			//software
	R_R_BESSEL_J, R_RR_BESSEL_J,											DISC_R_BESSEL_J_I, DISC_RR_BESSEL_J_I,	//software
	R_R_BESSEL_Y, R_RR_BESSEL_Y,											DISC_R_BESSEL_Y_I, DISC_RR_BESSEL_Y_I,	//software neumann
	C_R_HANKEL1, C_C_HANKEL1, C_RR_HANKEL1,									DISC_R_HANKEL1_I, DISC_RR_HANKEL1_I,	//software
	R_R_STEP, C_C_STEP, Q_Q_STEP,											DISC_R_STEP_I, DISC_C_STEP_I, DISC_Q_STEP_I,
	R_R_RECT, C_C_RECT, Q_Q_RECT,											DISC_R_RECT_I, DISC_C_RECT_I, DISC_Q_RECT_I,
	R_R_TRGL, R_C_TRGL, R_Q_TRGL,
	R_R_SQWV, R_C_SQWV, R_Q_SQWV, R_RR_SQWV, R_RC_SQWV, R_RQ_SQWV, R_CR_SQWV, R_CC_SQWV, R_CQ_SQWV, R_QR_SQWV, R_QC_SQWV, R_QQ_SQWV,
	DISC_R_SQWV_O,

	R_R_TRWV, R_C_TRWV, R_Q_TRWV, R_RR_TRWV, R_RC_TRWV, R_RQ_TRWV, R_CR_TRWV, R_CC_TRWV, R_CQ_TRWV, R_QR_TRWV, R_QC_TRWV, R_QQ_TRWV,//TODO: update trwv		DONE
	R_R_SAW, R_C_SAW, R_Q_SAW, R_RR_SAW, R_RC_SAW, R_RQ_SAW, R_CR_SAW, R_CC_SAW, R_CQ_SAW, R_QR_SAW, R_QC_SAW, R_QQ_SAW,//TODO: update sawtooth		DONE
	DISC_R_SAW_I, DISC_C_SAW_I, DISC_Q_SAW_I, DISC_RR_SAW_I, DISC_RC_SAW_I, DISC_RQ_SAW_I, DISC_CR_SAW_I, DISC_CC_SAW_I, DISC_CQ_SAW_I, DISC_QR_SAW_I, DISC_QC_SAW_I, DISC_QQ_SAW_I,

	R_RR_HYPOT,
	R_R_MANDELBROT, R_C_MANDELBROT, R_RR_MANDELBROT, R_CR_MANDELBROT,		DISC_R_MANDELBROT_O,
	R_RR_MIN, C_CR_MIN, C_CC_MIN, Q_QQ_MIN,
	R_RR_MAX, C_CR_MAX, C_CC_MAX, Q_QQ_MAX,
	R_RR_CONDITIONAL_110, C_RC_CONDITIONAL_110, Q_RQ_CONDITIONAL_110, R_CR_CONDITIONAL_110, C_CC_CONDITIONAL_110, Q_CQ_CONDITIONAL_110, R_QR_CONDITIONAL_110, C_QC_CONDITIONAL_110, Q_QQ_CONDITIONAL_110,
	DISC_CONDITIONAL_110_I,

	R_RR_CONDITIONAL_101, C_RC_CONDITIONAL_101, Q_RQ_CONDITIONAL_101, R_CR_CONDITIONAL_101, C_CC_CONDITIONAL_101, Q_CQ_CONDITIONAL_101, R_QR_CONDITIONAL_101, C_QC_CONDITIONAL_101, Q_QQ_CONDITIONAL_101,
	DISC_CONDITIONAL_101_I,

	CONDITIONAL_111,														DISC_CONDITIONAL_111_I,
	R_R_INCREMENT, C_C_INCREMENT, Q_Q_INCREMENT,
	R_R_DECREMENT, C_C_DECREMENT, Q_Q_DECREMENT,
	R_R_ASSIGN, C_C_ASSIGN, Q_Q_ASSIGN,

	N_KERNELS,
};
void 			cl_initiate();
extern int 		*rgb;
void 			cl_solve_c2d(Expression const &ex, double VX, double DX, double VY, double DY, int Xplaces, int Yplaces, double time, unsigned gl_texture);
void			cl_finish();
//void 			show_c2d();
#endif //GRAPHER_2_G2_CL_H
