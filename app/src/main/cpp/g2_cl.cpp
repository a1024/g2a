//g2_cl.cpp - OpenCL extension for Grapher 2A.
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

#include "g2_cl.h"
#include "GLESAPI.h"
#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_TARGET_OPENCL_VERSION 120
#include<CL/opencl.h>
#include<dlfcn.h>
#include<vector>
//int cl_errorcode=0;
const int	g_buf_size=2048;
char		cl_error_msg[2048]={0};
static char g_buf[g_buf_size]={0};
void 		p_check(void *p, int line, const char *func_name)
{
	if(!p)
	{
		LOGE("Line %d error: %s is %lld.", line, func_name, (long long)p);
		abort();
	}
}
#define 	P_CHECK(pointer)	p_check(pointer, __LINE__, #pointer)
const char* clerr2str(int error)
{
#define 			EC(x)	case x:a=(const char*)#x;break
	const char *a=nullptr;
	switch(error)
	{
	EC(CL_SUCCESS);
	EC(CL_DEVICE_NOT_FOUND);
	EC(CL_DEVICE_NOT_AVAILABLE);
	EC(CL_COMPILER_NOT_AVAILABLE);
	EC(CL_MEM_OBJECT_ALLOCATION_FAILURE);
	EC(CL_OUT_OF_RESOURCES);
	EC(CL_OUT_OF_HOST_MEMORY);
	EC(CL_PROFILING_INFO_NOT_AVAILABLE);
	EC(CL_MEM_COPY_OVERLAP);
	EC(CL_IMAGE_FORMAT_MISMATCH);
	EC(CL_IMAGE_FORMAT_NOT_SUPPORTED);
	EC(CL_BUILD_PROGRAM_FAILURE);
	EC(CL_MAP_FAILURE);
#ifdef CL_VERSION_1_1
	EC(CL_MISALIGNED_SUB_BUFFER_OFFSET);
	EC(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST);
#endif
#ifdef CL_VERSION_1_2
	EC(CL_COMPILE_PROGRAM_FAILURE);
	EC(CL_LINKER_NOT_AVAILABLE);
	EC(CL_LINK_PROGRAM_FAILURE);
	EC(CL_DEVICE_PARTITION_FAILED);
	EC(CL_KERNEL_ARG_INFO_NOT_AVAILABLE);
#endif
	EC(CL_INVALID_VALUE);
	EC(CL_INVALID_DEVICE_TYPE);
	EC(CL_INVALID_PLATFORM);
	EC(CL_INVALID_DEVICE);
	EC(CL_INVALID_CONTEXT);
	EC(CL_INVALID_QUEUE_PROPERTIES);
	EC(CL_INVALID_COMMAND_QUEUE);
	EC(CL_INVALID_HOST_PTR);
	EC(CL_INVALID_MEM_OBJECT);
	EC(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR);
	EC(CL_INVALID_IMAGE_SIZE);
	EC(CL_INVALID_SAMPLER);
	EC(CL_INVALID_BINARY);
	EC(CL_INVALID_BUILD_OPTIONS);
	EC(CL_INVALID_PROGRAM);
	EC(CL_INVALID_PROGRAM_EXECUTABLE);
	EC(CL_INVALID_KERNEL_NAME);
	EC(CL_INVALID_KERNEL_DEFINITION);
	EC(CL_INVALID_KERNEL);
	EC(CL_INVALID_ARG_INDEX);
	EC(CL_INVALID_ARG_VALUE);
	EC(CL_INVALID_ARG_SIZE);
	EC(CL_INVALID_KERNEL_ARGS);
	EC(CL_INVALID_WORK_DIMENSION);
	EC(CL_INVALID_WORK_GROUP_SIZE);
	EC(CL_INVALID_WORK_ITEM_SIZE);
	EC(CL_INVALID_GLOBAL_OFFSET);
	EC(CL_INVALID_EVENT_WAIT_LIST);
	EC(CL_INVALID_EVENT);
	EC(CL_INVALID_OPERATION);
	EC(CL_INVALID_GL_OBJECT);
	EC(CL_INVALID_BUFFER_SIZE);
	EC(CL_INVALID_MIP_LEVEL);
	EC(CL_INVALID_GLOBAL_WORK_SIZE);
#ifdef CL_VERSION_1_1
	EC(CL_INVALID_PROPERTY);
#endif
#ifdef CL_VERSION_1_2
	EC(CL_INVALID_IMAGE_DESCRIPTOR);
	EC(CL_INVALID_COMPILER_OPTIONS);
	EC(CL_INVALID_LINKER_OPTIONS);
	EC(CL_INVALID_DEVICE_PARTITION_COUNT);
#endif
#ifdef CL_VERSION_2_0
	EC(CL_INVALID_PIPE_SIZE);
	EC(CL_INVALID_DEVICE_QUEUE);
#endif
#ifdef CL_VERSION_2_2
	EC(CL_INVALID_SPEC_ID);
	EC(CL_MAX_SIZE_RESTRICTION_EXCEEDED);
#endif
	}
	return a;
#undef				EC
}
void		cl_error(const char *msg, int line)
{
	if(!cl_error_msg[0])
		snprintf(cl_error_msg, g_buf_size, "Line %d: %s", line, msg);
}
#define 	CL_ERROR(msg)	cl_error(msg, __LINE__)
void		cl_check(int error, int line)
{//check writes first error only
	if(error)
	{
		if(!cl_error_msg[0])
		{
			snprintf(cl_error_msg, g_buf_size, "Line %d error %d: %s", line, error, clerr2str(error));
			LOGE("%s", cl_error_msg);
		}
		else
		{
			snprintf(g_buf, g_buf_size, "Line %d error %d: %s", line, error, clerr2str(error));
			LOGE("%s", g_buf);
		}
	}
}
#define 	CL_CHECK(error)	cl_check(error, __LINE__)
void 		cl_check2(int error, int line)
{//check2 overwrites error every time
	if(error)
	{
		snprintf(cl_error_msg, g_buf_size, "Line %d error %d: %s", line, error, clerr2str(error));
		LOGE("%s", cl_error_msg);
	}
}
#define 	CL_CHECK2(error)	cl_check2(error, __LINE__)
bool		API_not_loaded=true;
#define 	DECL_CL_FUNC(clFunc)	decltype(clFunc) *p_##clFunc __attribute__((weak))=nullptr
DECL_CL_FUNC(clGetPlatformIDs);
DECL_CL_FUNC(clGetDeviceIDs);
DECL_CL_FUNC(clCreateContext);
DECL_CL_FUNC(clCreateCommandQueue);
DECL_CL_FUNC(clCreateProgramWithSource);
DECL_CL_FUNC(clBuildProgram);
DECL_CL_FUNC(clGetProgramBuildInfo);
DECL_CL_FUNC(clCreateBuffer);
DECL_CL_FUNC(clCreateKernel);
DECL_CL_FUNC(clSetKernelArg);
DECL_CL_FUNC(clEnqueueWriteBuffer);
DECL_CL_FUNC(clEnqueueNDRangeKernel);
DECL_CL_FUNC(clEnqueueReadBuffer);
DECL_CL_FUNC(clFinish);
#undef		DECL_CL_FUNC
void		*hOpenCL=nullptr;
void 		load_OpenCL_API()
{
	if(API_not_loaded)
	{
		static const char *opencl_so_paths[]=//https://stackoverflow.com/questions/31611790/using-opencl-in-the-new-android-studio
		{
			"libOpenCL.so",
			//Android
			"/system/vendor/lib64/libOpenCL.so",
			"/system/lib/libOpenCL.so",
			"/system/vendor/lib/libOpenCL.so",
			"/system/vendor/lib/egl/libGLES_mali.so",
			"/system/vendor/lib/libPVROCL.so",
			"/data/data/org.pocl.libs/files/lib/libpocl.so",
			//Linux
			"/usr/lib/libOpenCL.so",
			"/usr/local/lib/libOpenCL.so",
			"/usr/local/lib/libpocl.so",
			"/usr/lib64/libOpenCL.so",
			"/usr/lib32/libOpenCL.so",
		};
		const int npaths=sizeof(opencl_so_paths)>>3;
		for(int k=0;k<npaths&&!hOpenCL;++k)
			hOpenCL=dlopen(opencl_so_paths[k], RTLD_LAZY);
		if(hOpenCL)
		{
			API_not_loaded=false;
#ifdef RELEASE
#define		GET_CL_FUNC(h, clFunc)		p_##clFunc=(decltype(p_##clFunc))dlsym(h, #clFunc)
#else
#define		GET_CL_FUNC(h, clFunc)		p_##clFunc=(decltype(p_##clFunc))dlsym(h, #clFunc), p_check((void*)p_##clFunc, __LINE__, #clFunc)
#endif
			GET_CL_FUNC(hOpenCL, clGetPlatformIDs);
			GET_CL_FUNC(hOpenCL, clGetDeviceIDs);
			GET_CL_FUNC(hOpenCL, clCreateContext);
			GET_CL_FUNC(hOpenCL, clCreateCommandQueue);
			GET_CL_FUNC(hOpenCL, clCreateProgramWithSource);
			GET_CL_FUNC(hOpenCL, clBuildProgram);
			GET_CL_FUNC(hOpenCL, clGetProgramBuildInfo);
			GET_CL_FUNC(hOpenCL, clCreateBuffer);
			GET_CL_FUNC(hOpenCL, clCreateKernel);
			GET_CL_FUNC(hOpenCL, clSetKernelArg);
			GET_CL_FUNC(hOpenCL, clEnqueueWriteBuffer);
			GET_CL_FUNC(hOpenCL, clEnqueueNDRangeKernel);
			GET_CL_FUNC(hOpenCL, clEnqueueReadBuffer);
			GET_CL_FUNC(hOpenCL, clFinish);
#undef		GET_CL_FUNC
		}
	}
}
void 		unload_OpenCL_API()
{
	if(hOpenCL)//when finishing opencl session
	{
		dlclose(hOpenCL), hOpenCL=nullptr;
		API_not_loaded=true;
	}
}

enum 		CLKernelIdx
{
	V_INITIALIZE_CONSTANTS,

	R_R_SETZERO, C_C_SETZERO, Q_Q_SETZERO,
	R_R_CEIL, C_C_CEIL, Q_Q_CEIL,
	R_R_FLOOR, C_C_FLOOR, Q_Q_FLOOR,
	R_R_ROUND, C_C_ROUND, Q_Q_ROUND,
	R_R_INT, C_C_INT, Q_Q_INT,
	R_R_FRAC, C_C_FRAC, Q_Q_FRAC,
	R_R_ABS, R_C_ABS, R_Q_ABS,
	R_R_ARG, R_C_ARG, R_Q_ARG,
	R_C_REAL,
	R_C_IMAG,
	C_C_CONJUGATE, Q_Q_CONJUGATE,
	C_R_POLAR, C_C_POLAR, C_Q_POLAR,
	C_C_CARTESIAN, Q_Q_CARTESIAN,
	R_RR_PLUS, C_RC_PLUS, Q_RQ_PLUS, C_CR_PLUS, C_CC_PLUS, Q_CQ_PLUS, Q_QR_PLUS, Q_QC_PLUS, Q_QQ_PLUS,
	R_R_MINUS, C_C_MINUS, Q_Q_MINUS, R_RR_MINUS, C_RC_MINUS, Q_RQ_MINUS, C_CR_MINUS, C_CC_MINUS, Q_CQ_MINUS, Q_QR_MINUS, Q_QC_MINUS, Q_QQ_MINUS,
	R_RR_MULTIPLY, C_RC_MULTIPLY, Q_RQ_MULTIPLY, C_CR_MULTIPLY, C_CC_MULTIPLY, Q_CQ_MULTIPLY, Q_QR_MULTIPLY, Q_QC_MULTIPLY, Q_QQ_MULTIPLY,
	R_R_DIVIDE, C_C_DIVIDE, Q_Q_DIVIDE, R_RR_DIVIDE, C_RC_DIVIDE, Q_RQ_DIVIDE, C_CR_DIVIDE, C_CC_DIVIDE, Q_CQ_DIVIDE, Q_QR_DIVIDE, Q_QC_DIVIDE, Q_QQ_DIVIDE,
	R_RR_LOGIC_DIVIDES, R_RC_LOGIC_DIVIDES, R_RQ_LOGIC_DIVIDES, R_CR_LOGIC_DIVIDES, R_CC_LOGIC_DIVIDES, R_CQ_LOGIC_DIVIDES, R_QR_LOGIC_DIVIDES, R_QC_LOGIC_DIVIDES, R_QQ_LOGIC_DIVIDES,
	R_RR_POWER_REAL, C_CR_POWER_REAL, Q_QR_POWER_REAL,
	C_CR_POW, C_CC_POW, Q_CQ_POW, Q_QR_POW, Q_QC_POW, Q_QQ_POW,
	C_C_LN, Q_Q_LN,
	C_C_LOG,  Q_Q_LOG, C_CR_LOG, C_CC_LOG, Q_CQ_LOG, Q_QC_LOG, Q_QQ_LOG,
	C_RR_TETRATE, C_RC_TETRATE, C_CR_TETRATE, C_CC_TETRATE, Q_QR_TETRATE,
	C_RR_PENTATE, C_CR_PENTATE,
	R_R_BITWISE_SHIFT_LEFT_L, C_C_BITWISE_SHIFT_LEFT_L, Q_Q_BITWISE_SHIFT_LEFT_L, R_R_BITWISE_SHIFT_LEFT_R, C_C_BITWISE_SHIFT_LEFT_R, Q_Q_BITWISE_SHIFT_LEFT_R, R_RR_BITWISE_SHIFT_LEFT, C_RC_BITWISE_SHIFT_LEFT, Q_RQ_BITWISE_SHIFT_LEFT, C_CR_BITWISE_SHIFT_LEFT, C_CC_BITWISE_SHIFT_LEFT, Q_CQ_BITWISE_SHIFT_LEFT, Q_QR_BITWISE_SHIFT_LEFT, Q_QC_BITWISE_SHIFT_LEFT, Q_QQ_BITWISE_SHIFT_LEFT,
	R_R_BITWISE_SHIFT_RIGHT_L, C_C_BITWISE_SHIFT_RIGHT_L, Q_Q_BITWISE_SHIFT_RIGHT_L, R_R_BITWISE_SHIFT_RIGHT_R, C_C_BITWISE_SHIFT_RIGHT_R, Q_Q_BITWISE_SHIFT_RIGHT_R, R_RR_BITWISE_SHIFT_RIGHT, C_RC_BITWISE_SHIFT_RIGHT, Q_RQ_BITWISE_SHIFT_RIGHT, C_CR_BITWISE_SHIFT_RIGHT, C_CC_BITWISE_SHIFT_RIGHT, Q_CQ_BITWISE_SHIFT_RIGHT, Q_QR_BITWISE_SHIFT_RIGHT, Q_QC_BITWISE_SHIFT_RIGHT, Q_QQ_BITWISE_SHIFT_RIGHT,
	R_R_BITWISE_NOT, C_C_BITWISE_NOT, Q_Q_BITWISE_NOT,
	R_R_BITWISE_AND, C_C_BITWISE_AND, Q_Q_BITWISE_AND, R_RR_BITWISE_AND, C_RC_BITWISE_AND, Q_RQ_BITWISE_AND, C_CR_BITWISE_AND, C_CC_BITWISE_AND, Q_CQ_BITWISE_AND, Q_QR_BITWISE_AND, Q_QC_BITWISE_AND, Q_QQ_BITWISE_AND,
	R_R_BITWISE_NAND, C_C_BITWISE_NAND, Q_Q_BITWISE_NAND, R_RR_BITWISE_NAND, C_RC_BITWISE_NAND, Q_RQ_BITWISE_NAND, C_CR_BITWISE_NAND, C_CC_BITWISE_NAND, Q_CQ_BITWISE_NAND, Q_QR_BITWISE_NAND, Q_QC_BITWISE_NAND, Q_QQ_BITWISE_NAND,
	R_R_BITWISE_OR, C_C_BITWISE_OR, Q_Q_BITWISE_OR, R_RR_BITWISE_OR, C_RC_BITWISE_OR, Q_RQ_BITWISE_OR, C_CR_BITWISE_OR, C_CC_BITWISE_OR, Q_CQ_BITWISE_OR, Q_QR_BITWISE_OR, Q_QC_BITWISE_OR, Q_QQ_BITWISE_OR,
	R_R_BITWISE_NOR, C_C_BITWISE_NOR, Q_Q_BITWISE_NOR, R_RR_BITWISE_NOR, C_RC_BITWISE_NOR, Q_RQ_BITWISE_NOR, C_CR_BITWISE_NOR, C_CC_BITWISE_NOR, Q_CQ_BITWISE_NOR, Q_QR_BITWISE_NOR, Q_QC_BITWISE_NOR, Q_QQ_BITWISE_NOR,
	R_R_BITWISE_XOR, C_C_BITWISE_XOR, Q_Q_BITWISE_XOR, R_RR_BITWISE_XOR, C_RC_BITWISE_XOR, Q_RQ_BITWISE_XOR, C_CR_BITWISE_XOR, C_CC_BITWISE_XOR, Q_CQ_BITWISE_XOR, Q_QR_BITWISE_XOR, Q_QC_BITWISE_XOR, Q_QQ_BITWISE_XOR,
	R_R_BITWISE_XNOR, C_C_BITWISE_XNOR, Q_Q_BITWISE_XNOR, R_RR_BITWISE_XNOR, C_RC_BITWISE_XNOR, Q_RQ_BITWISE_XNOR, C_CR_BITWISE_XNOR, C_CC_BITWISE_XNOR, Q_CQ_BITWISE_XNOR, Q_QR_BITWISE_XNOR, Q_QC_BITWISE_XNOR, Q_QQ_BITWISE_XNOR,
	R_R_LOGIC_EQUAL, R_C_LOGIC_EQUAL, R_Q_LOGIC_EQUAL, R_RR_LOGIC_EQUAL, R_RC_LOGIC_EQUAL, R_RQ_LOGIC_EQUAL, R_CR_LOGIC_EQUAL, R_CC_LOGIC_EQUAL, R_CQ_LOGIC_EQUAL, R_QR_LOGIC_EQUAL, R_QC_LOGIC_EQUAL, R_QQ_LOGIC_EQUAL,
	R_R_LOGIC_NOT_EQUAL, R_C_LOGIC_NOT_EQUAL, R_Q_LOGIC_NOT_EQUAL, R_RR_LOGIC_NOT_EQUAL, R_RC_LOGIC_NOT_EQUAL, R_RQ_LOGIC_NOT_EQUAL, R_CR_LOGIC_NOT_EQUAL, R_CC_LOGIC_NOT_EQUAL, R_CQ_LOGIC_NOT_EQUAL, R_QR_LOGIC_NOT_EQUAL, R_QC_LOGIC_NOT_EQUAL, R_QQ_LOGIC_NOT_EQUAL,
	R_R_LOGIC_LESS_L, R_C_LOGIC_LESS_L, R_Q_LOGIC_LESS_L, R_R_LOGIC_LESS_R, R_C_LOGIC_LESS_R, R_Q_LOGIC_LESS_R, R_RR_LOGIC_LESS, R_RC_LOGIC_LESS, R_RQ_LOGIC_LESS, R_CR_LOGIC_LESS, R_CC_LOGIC_LESS, R_CQ_LOGIC_LESS, R_QR_LOGIC_LESS, R_QC_LOGIC_LESS, R_QQ_LOGIC_LESS,
	R_R_LOGIC_LESS_EQUAL_L, R_C_LOGIC_LESS_EQUAL_L, R_Q_LOGIC_LESS_EQUAL_L, R_R_LOGIC_LESS_EQUAL_R, R_C_LOGIC_LESS_EQUAL_R, R_Q_LOGIC_LESS_EQUAL_R, R_RR_LOGIC_LESS_EQUAL, R_RC_LOGIC_LESS_EQUAL, R_RQ_LOGIC_LESS_EQUAL, R_CR_LOGIC_LESS_EQUAL, R_CC_LOGIC_LESS_EQUAL, R_CQ_LOGIC_LESS_EQUAL, R_QR_LOGIC_LESS_EQUAL, R_QC_LOGIC_LESS_EQUAL, R_QQ_LOGIC_LESS_EQUAL,
	R_R_LOGIC_GREATER_L, R_C_LOGIC_GREATER_L, R_Q_LOGIC_GREATER_L, R_R_LOGIC_GREATER_R, R_C_LOGIC_GREATER_R, R_Q_LOGIC_GREATER_R, R_RR_LOGIC_GREATER, R_RC_LOGIC_GREATER, R_RQ_LOGIC_GREATER, R_CR_LOGIC_GREATER, R_CC_LOGIC_GREATER, R_CQ_LOGIC_GREATER, R_QR_LOGIC_GREATER, R_QC_LOGIC_GREATER, R_QQ_LOGIC_GREATER,
	R_R_LOGIC_GREATER_EQUAL_L, R_C_LOGIC_GREATER_EQUAL_L, R_Q_LOGIC_GREATER_EQUAL_L, R_R_LOGIC_GREATER_EQUAL_R, R_C_LOGIC_GREATER_EQUAL_R, R_Q_LOGIC_GREATER_EQUAL_R, R_RR_LOGIC_GREATER_EQUAL, R_RC_LOGIC_GREATER_EQUAL, R_RQ_LOGIC_GREATER_EQUAL, R_CR_LOGIC_GREATER_EQUAL, R_CC_LOGIC_GREATER_EQUAL, R_CQ_LOGIC_GREATER_EQUAL, R_QR_LOGIC_GREATER_EQUAL, R_QC_LOGIC_GREATER_EQUAL, R_QQ_LOGIC_GREATER_EQUAL,
	R_R_LOGIC_NOT, R_C_LOGIC_NOT, R_Q_LOGIC_NOT,
	R_RR_LOGIC_AND, R_RC_LOGIC_AND, R_RQ_LOGIC_AND, R_CR_LOGIC_AND, R_CC_LOGIC_AND, R_CQ_LOGIC_AND, R_QR_LOGIC_AND, R_QC_LOGIC_AND, R_QQ_LOGIC_AND,
	R_RR_LOGIC_OR, R_RC_LOGIC_OR, R_RQ_LOGIC_OR, R_CR_LOGIC_OR, R_CC_LOGIC_OR, R_CQ_LOGIC_OR, R_QR_LOGIC_OR, R_QC_LOGIC_OR, R_QQ_LOGIC_OR,
	R_RR_LOGIC_XOR, R_RC_LOGIC_XOR, R_RQ_LOGIC_XOR, R_CR_LOGIC_XOR, R_CC_LOGIC_XOR, R_CQ_LOGIC_XOR, R_QR_LOGIC_XOR, R_QC_LOGIC_XOR, R_QQ_LOGIC_XOR,
	R_RR_CONDITION_ZERO, C_RC_CONDITION_ZERO, Q_RQ_CONDITION_ZERO, C_CR_CONDITION_ZERO, C_CC_CONDITION_ZERO, Q_CQ_CONDITION_ZERO, Q_QR_CONDITION_ZERO, Q_QC_CONDITION_ZERO, Q_QQ_CONDITION_ZERO,
	R_R_PERCENT, C_C_PERCENT, Q_Q_PERCENT,
	R_RR_MODULO, C_RC_MODULO, Q_RQ_MODULO, C_CR_MODULO, C_CC_MODULO, Q_CQ_MODULO, Q_QR_MODULO, Q_QC_MODULO, Q_QQ_MODULO,
	R_R_SGN, C_C_SGN, Q_Q_SGN,
	R_R_SQ, C_C_SQ, Q_Q_SQ,
	C_C_SQRT, Q_Q_SQRT,
	R_R_INVSQRT,
	R_R_CBRT, C_C_CBRT, Q_Q_CBRT,
	R_R_GAUSS, C_C_GAUSS, Q_Q_GAUSS,
	R_R_ERF,
	R_R_ZETA,
	R_R_TGAMMA, C_C_TGAMMA, Q_Q_TGAMMA, R_RR_TGAMMA,
	R_R_LOGGAMMA,
	R_R_FACTORIAL, C_C_FACTORIAL, Q_Q_FACTORIAL,
	R_R_PERMUTATION, C_C_PERMUTATION, Q_Q_PERMUTATION, R_RR_PERMUTATION, C_CR_PERMUTATION, C_CC_PERMUTATION, Q_QQ_PERMUTATION,
	R_R_COMBINATION, C_C_COMBINATION, Q_Q_COMBINATION, R_RR_COMBINATION, C_CR_COMBINATION, C_CC_COMBINATION, Q_QQ_COMBINATION,
	R_R_COS, C_C_COS, Q_Q_COS,
	C_C_ACOS, Q_Q_ACOS,
	R_R_COSH, C_C_COSH, Q_Q_COSH,
	C_C_ACOSH, Q_Q_ACOSH,
	R_R_COSC, C_C_COSC, Q_Q_COSC,
	R_R_SEC, C_C_SEC, Q_Q_SEC,
	C_C_ASEC, Q_Q_ASEC,
	R_R_SECH, C_C_SECH, Q_Q_SECH,
	C_C_ASECH, Q_Q_ASECH,
	R_R_SIN, C_C_SIN, Q_Q_SIN,
	C_C_ASIN, Q_Q_ASIN,
	R_R_SINH, C_C_SINH, Q_Q_SINH,
	R_R_ASINH, C_C_ASINH, Q_Q_ASINH,
	R_R_SINC, C_C_SINC, Q_Q_SINC,
	R_R_SINHC, C_C_SINHC, Q_Q_SINHC,
	R_R_CSC, C_C_CSC, Q_Q_CSC,
	C_C_ACSC, Q_Q_ACSC,
	R_R_CSCH, C_C_CSCH, Q_Q_CSCH,
	R_R_ACSCH, C_C_ACSCH, Q_Q_ACSCH,
	R_R_TAN, C_C_TAN, Q_Q_TAN,
	R_R_ATAN, C_C_ATAN, Q_Q_ATAN, R_RR_ATAN, C_RC_ATAN, Q_RQ_ATAN, C_CR_ATAN, C_CC_ATAN, Q_CQ_ATAN, Q_QR_ATAN, Q_QC_ATAN, Q_QQ_ATAN,
	R_R_TANH, C_C_TANH, Q_Q_TANH,
	C_C_ATANH, Q_Q_ATANH,
	R_R_TANC, C_C_TANC, Q_Q_TANC,
	R_R_COT, C_C_COT, Q_Q_COT,
	R_R_ACOT, C_C_ACOT, Q_Q_ACOT,
	R_R_COTH, C_C_COTH, Q_Q_COTH,
	C_C_ACOTH, Q_Q_ACOTH,
	R_R_EXP, C_C_EXP, Q_Q_EXP,
	R_R_FIB, C_C_FIB, Q_Q_FIB,
	R_R_RANDOM, C_C_RANDOM, Q_Q_RANDOM, R_RR_RANDOM, C_CR_RANDOM, C_CC_RANDOM, Q_QQ_RANDOM,//nargs determine nD, arg mathtype determines return type
	R_R_BETA, R_RR_BETA,//software
	R_R_BESSEL_J, R_RR_BESSEL_J,//software
	R_R_BESSEL_Y, R_RR_BESSEL_Y,//software neumann
	C_R_HANKEL1, C_C_HANKEL1, C_RR_HANKEL1,//software
	R_R_STEP, C_C_STEP, Q_Q_STEP,
	R_R_RECT, C_C_RECT, Q_Q_RECT,
	R_R_TRGL, R_C_TRGL, R_Q_TRGL,
	R_R_SQWV, R_C_SQWV, R_Q_SQWV, R_RR_SQWV, R_RC_SQWV, R_RQ_SQWV, R_CR_SQWV, R_CC_SQWV, R_CQ_SQWV, R_QR_SQWV, R_QC_SQWV, R_QQ_SQWV,
	R_R_TRWV, R_C_TRWV, R_Q_TRWV, R_RR_TRWV, C_CR_TRWV, C_CC_TRWV, Q_QQ_TRWV,
	R_R_SAW, C_C_SAW, Q_Q_SAW, R_RR_SAW, C_RC_SAW, Q_RQ_SAW, C_CR_SAW, C_CC_SAW, Q_CQ_SAW, Q_QR_SAW, Q_QC_SAW, Q_QQ_SAW,
	R_RR_HYPOT,
	R_R_MANDELBROT, R_C_MANDELBROT, R_RR_MANDELBROT, R_CR_MANDELBROT,
	R_RR_MIN, C_CR_MIN, C_CC_MIN, Q_QQ_MIN,
	R_RR_MAX, C_CR_MAX, C_CC_MAX, Q_QQ_MAX,
	R_RR_CONDITIONAL_110, C_RC_CONDITIONAL_110, Q_RQ_CONDITIONAL_110, R_CR_CONDITIONAL_110, C_CC_CONDITIONAL_110, Q_CQ_CONDITIONAL_110, R_QR_CONDITIONAL_110, C_QC_CONDITIONAL_110, Q_QQ_CONDITIONAL_110,
	R_RR_CONDITIONAL_101, C_RC_CONDITIONAL_101, Q_RQ_CONDITIONAL_101, R_CR_CONDITIONAL_101, C_CC_CONDITIONAL_101, Q_CQ_CONDITIONAL_101, R_QR_CONDITIONAL_101, C_QC_CONDITIONAL_101, Q_QQ_CONDITIONAL_101,
	R_R_INCREMENT, C_C_INCREMENT, Q_Q_INCREMENT,
	R_R_DECREMENT, C_C_DECREMENT, Q_Q_DECREMENT,
	R_R_ASSIGN, C_C_ASSIGN, Q_Q_ASSIGN,
};
enum		CLKernelSignature
{
	CL_R_R, CL_C_C, CL_Q_Q,
	CL_R_RR, CL_C_RC, CL_Q_RQ,
	CL_C_CR, CL_C_CC, CL_Q_CQ,
	CL_Q_QR, CL_Q_QC, CL_Q_QQ,
	CL_C_R, CL_C_Q,
	CL_R_C, CL_R_Q,
	CL_C_RR, CL_R_RC, CL_R_RQ,
	CL_R_CR, CL_R_CC, CL_R_CQ,
	CL_R_QR, CL_R_QC, CL_R_QQ,
	CL_C_QC,
};
enum 		CLDiscType{DISC_CONT, DISC_INPUT, DISC_OUTPUT};
struct 		CLKernel
{
	int idx;//enum CLKernelIdx
	short
		signature,//enum CLKernelSignature
		disc_type;//enum CLDiscType
	const char
		*name,//kernel name string, if 0 then software
		*disc_name;//if 0 then always continuous, signature: bool(const int *size, const int idx_offset, const float *r[, const float *i, const float *j, const float *k])
};
namespace		CLSource
{
static const char program_src_pt1[]=R"CLSRC(
//auxiliary constants
enum Constants
{
	C_PI, C_2PI, C_SQRT2
};
__global float *constants;
#define	_pi		constants[C_PI]
#define	_2pi	constants[C_2PI]
#define	_sqrt2	constants[C_SQRT2]

//auxiliary functions
__kernel void initialize_constants()
{
	_pi=acos(-1.f);
	_2pi=_pi+_pi;
	_sqrt2=sqrt(2.f);
}
int get_idx(__global const int *size)
{
	return size[0]*(size[1]*get_global_id(2)+get_global_id(1))+get_global_id(0);
}

//kernels
__kernel void r_r_setzero(__global const int *size, __global float *rr, __global const float *xr){rr[get_idx(size)]=0;}
__kernel void c_c_setzero(__global const int *size,
	__global float *rr, __global float *ri,
	__global const float *xr, __global const float *xi)
{
	const unsigned idx=get_idx(size);
	rr[idx]=ri[idx]=0;
}
__kernel void q_q_setzero(__global const int *size,
	__global float *rr, __global float *ri, __global float *rj, __global float *rk,
	__global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk)
{
	const unsigned idx=get_idx(size);
	rr[idx]=ri[idx]=rj[idx]=rk[idx]=0;
}

__kernel void r_r_ceil(__global const int *size, __global float *rr, __global const float *xr)
{
	const unsigned idx=get_idx(size);
	rr[idx]=ceil(xr[idx]);
}
__kernel void c_c_ceil(__global const int *size,
	__global float *rr, __global float *ri,
	__global const float *xr, __global const float *xi)
{
	const unsigned idx=size[0]*(size[1]*get_global_id(2)+get_global_id(1))+get_global_id(0);
	rr[idx]=ceil(xr[idx]);
	ri[idx]=ceil(xi[idx]);
}
__kernel void q_q_ceil(__global const int *size,
	__global float *rr, __global float *ri, __global float *rj, __global float *rk,
	__global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk)
{
	const unsigned idx=get_idx(size);
	rr[idx]=ceil(xr[idx]);
	ri[idx]=ceil(xi[idx]);
	rj[idx]=ceil(xj[idx]);
	rk[idx]=ceil(xk[idx]);
}
__kernel bool disc_r_ceil_o(__global const int *size, const int offset, __global const float *xr)
{
	const unsigned idx=get_idx(size);
	return xr[idx]!=xr[idx+offset];
}
__kernel bool disc_c_ceil_o(__global const int *size, const int offset, __global const float *xr, __global const float *xi)
{
	const unsigned idx=get_idx(size);
	return xr[idx]!=xr[idx+offset]||xi[idx]!=xi[idx+offset];
}
__kernel bool disc_q_ceil_o(__global const int *size, const int offset, __global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk)
{
	const unsigned idx=get_idx(size);
	return xr[idx]!=xr[idx+offset]||xi[idx]!=xi[idx+offset]||xj[idx]!=xj[idx+offset]||xk[idx]!=xk[idx+offset];
}

__kernel void r_r_floor(__global const int *size, __global float *rr, __global const float *xr)
{
	const unsigned idx=get_idx(size);
	rr[idx]=floor(xr[idx]);
}
__kernel void c_c_floor(__global const int *size,
	__global float *rr, __global float *ri,
	__global const float *xr, __global const float *xi)
{
	const unsigned idx=get_idx(size);
	rr[idx]=floor(xr[idx]);
	ri[idx]=floor(xi[idx]);
}
__kernel void q_q_floor(__global const int *size,
	__global float *rr, __global float *ri, __global float *rj, __global float *rk,
	__global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk)
{
	const unsigned idx=get_idx(size);
	rr[idx]=floor(xr[idx]);
	ri[idx]=floor(xi[idx]);
	rj[idx]=floor(xj[idx]);
	rk[idx]=floor(xk[idx]);
}
__kernel bool disc_r_floor_o(__global const int *size, int offset, __global const float *xr){return disc_r_ceil_o(size, offset, xr);}
__kernel bool disc_c_floor_o(__global const int *size, int offset, __global const float *xr, __global const float *xi){return disc_c_ceil_o(size, offset, xr, xi);}
__kernel bool disc_q_floor_o(__global const int *size, int offset, __global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk){return disc_q_ceil_o(size, offset, xr, xi, xj, xk);}

__kernel void r_r_round(__global const int *size, __global float *rr, __global const float *xr)
{
	const unsigned idx=get_idx(size);
	rr[idx]=round(xr[idx]);
}
__kernel void c_c_round(__global const int *size,
	__global float *rr, __global float *ri,
	__global const float *xr, __global const float *xi)
{
	const unsigned idx=get_idx(size);
	rr[idx]=round(xr[idx]);
	ri[idx]=round(xi[idx]);
}
__kernel void q_q_round(__global const int *size, __global float *rr, __global float *ri, __global float *rj, __global float *rk, __global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk)
{
	const unsigned idx=get_idx(size);
	rr[idx]=round(xr[idx]);
	ri[idx]=round(xi[idx]);
	rj[idx]=round(xj[idx]);
	rk[idx]=round(xk[idx]);
}
__kernel bool disc_r_round_o(__global const int *size, int offset, __global const float *xr){return disc_r_ceil_o(size, offset, xr);}
__kernel bool disc_c_round_o(__global const int *size, int offset, __global const float *xr, __global const float *xi){return disc_c_ceil_o(size, offset, xr, xi);}
__kernel bool disc_r_round_o(__global const int *size, int offset, __global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk){return disc_q_ceil_o(size, offset, xr, xi, xj, xk);}

__kernel void r_r_int(__global const int *size, __global float *rr, __global const float *xr)
{
	const unsigned idx=get_idx(size);
	rr[idx]=int(xr[idx]);
}
__kernel void c_c_int(__global const int *size, __global float *rr, __global float *ri, __global const float *xr, __global const float *xi)
{
	const unsigned idx=get_idx(size);
	rr[idx]=int(xr[idx]);
	ri[idx]=int(xi[idx]);
}
__kernel void q_q_int(__global const int *size,
	__global float *rr, __global float *ri, __global float *rj, __global float *rk,
	__global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk)
{
	const unsigned idx=get_idx(size);
	rr[idx]=int(xr[idx]);
	ri[idx]=int(xi[idx]);
	rj[idx]=int(xj[idx]);
	rk[idx]=int(xk[idx]);
}
__kernel bool disc_r_int_o(__global const int *size, int offset, __global const float *xr){return disc_r_ceil_o(size, offset, xr);}
__kernel bool disc_c_int_o(__global const int *size, int offset, __global const float *xr, __global const float *xi){return disc_c_ceil_o(size, offset, xr, xi);}
__kernel bool disc_r_int_o(__global const int *size, int offset, __global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk){return disc_q_ceil_o(size, offset, xr, xi, xj, xk);}

__kernel void r_r_frac(__global const int *size, __global float *rr, __global const float *xr)
{
	const unsigned idx=get_idx(size);
	rr[idx]=frac(xr[idx]);
}
__kernel void c_c_frac(__global const int *size, __global float *rr, __global float *ri, __global const float *xr, __global const float *xi)
{
	const unsigned idx=get_idx(size);
	rr[idx]=frac(xr[idx]);
	ri[idx]=frac(xi[idx]);
}
__kernel void q_q_frac(__global const int *size, __global float *rr, __global float *ri, __global float *rj, __global float *rk, __global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk)
{
	const unsigned idx=get_idx(size);
	rr[idx]=frac(xr[idx]);
	ri[idx]=frac(xi[idx]);
	rj[idx]=frac(xj[idx]);
	rk[idx]=frac(xk[idx]);
}
__kernel bool disc_r_frac_i(__global const int *size, int offset, __global const float *xr)
{
	const unsigned idx=get_idx(size);
	return floor(xr[idx])!=floor(xr[idx+offset]);
}
__kernel bool disc_c_frac_i(__global const int *size, int offset, __global const float *xr, __global const float *xi)
{
	const unsigned idx=get_idx(size);
	return floor(xr[idx])!=floor(xr[idx+offset])||floor(xi[idx])!=floor(xi[idx+offset]);
}
__kernel bool disc_q_frac_i(__global const int *size, int offset, __global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk)
{
	const unsigned idx=get_idx(size);
	return floor(xr[idx])!=floor(xr[idx+offset])||floor(xi[idx])!=floor(xi[idx+offset])||floor(xj[idx])!=floor(xj[idx+offset])||floor(xk[idx])!=floor(xk[idx+offset]);
}

__kernel void r_r_abs(__global const int *size, __global float *rr, __global const float *xr)
{
	const unsigned idx=get_idx(size);
	rr[idx]=abs(xr[idx]);
}
__kernel void r_c_abs(__global const int *size, __global float *rr, __global const float *xr, __global const float *xi)
{
	const unsigned idx=get_idx(size);
	rr[idx]=sqrt(xr[idx]*xr[idx]+xi[idx]*xi[idx]);
}
__kernel void r_q_abs(__global const int *size, __global float *rr, __global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk)
{
	const unsigned idx=get_idx(size);
	rr[idx]=sqrt(xr[idx]*xr[idx]+xi[idx]*xi[idx]+xj[idx]*xj[idx]+xk[idx]*xk[idx]);
}

__kernel void r_r_arg(__global const int *size, __global float *rr, __global const float *xr)
{
	const unsigned idx=get_idx(size);
	const float _pi=acos(-1.f);
	if(xr[idx]<0)
		rr[idx]=_pi;
	else if(xr[idx]==0)
		rr[idx]=nan(0);
	else
		rr[idx]=0;
}
__kernel void r_c_arg(__global const int *size, __global float *rr, __global const float *xr, __global const float *xi)
{
	const unsigned idx=get_idx(size);
	rr[idx]=atan2(xi[idx], xr[idx]);
}
__kernel void r_q_arg(__global const int *size, __global float *rr, __global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk)
{
	const unsigned idx=size[0]*(size[1]*get_global_id(2)+get_global_id(1))+get_global_id(0);
	float abs_x=xr[idx]*xr[idx]+xi[idx]*xi[idx]+xj[idx]*xj[idx]+xk[idx]*xk[idx];
	if(!abs_x)
		rr[idx]=nan(0);
	else
	{
		abs_x=sqrt(abx_x);
		rr[idx]=acos(xr[idx]/abs_x);
	}
}
__kernel bool disc_r_arg_i(__global const int *size, int offset, __global const float *xr)
{
	const unsigned idx=get_idx(size);
	if(xr[idx]<0)
		return xr[idx+offset]>=0;
	if(xr[idx]>0)
		return xr[idx+offset]<=0;
	return xr[idx+offset]!=0;
//	return xr[idx]<0?xr[idx+offset]>=0:xr[idx]>0?xr[idx+offset]<=0:xr[idx+offset]!=0;
}
__kernel bool disc_c_arg_i(__global const int *size, int offset, __global const float *xr, __global const float *xi){return false;}//TODO
__kernel bool disc_r_arg_i(__global const int *size, int offset, __global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk){return false;}//TODO

__kernel void r_c_arg(__global const int *size, __global float *rr, __global const float *xr, __global const float *xi)
{
	const unsigned idx=get_idx(size);
	rr[idx]=xr[idx];
}

__kernel void r_c_imag(__global const int *size, __global float *rr, __global const float *xr, __global const float *xi)
{
	const unsigned idx=get_idx(size);
	rr[idx]=xi[idx];
}

__kernel void c_c_conjugate(__global const int *size, __global float *rr, __global float *ri, __global const float *xr, __global const float *xi)
{
	const unsigned idx=get_idx(size);
	rr[idx]=xr[idx];
	ri[idx]=-xi[idx];
}
__kernel void q_q_conjugate(__global const int *size, __global float *rr, __global float *ri, __global float *rj, __global float *rk, __global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk)
{
	const unsigned idx=get_idx(size);
	rr[idx]=xr[idx];
	ri[idx]=-xi[idx];
	rj[idx]=-xj[idx];
	rk[idx]=-xk[idx];
}

__kernel void c_r_polar(__global const int *size, __global float *rr, __global float *ri, __global const float *xr)
{
	const unsigned idx=get_idx(size);
	float retr=abs(xr[idx]), reti;
	if(xr[idx]<0)
		reti=_pi;
	else if(xr[idx]==0)
		reti=nan(0);
	else
		reti=0;
	rr[idx]=retr, rr[idx]=reti;
}
__kernel void c_c_polar(__global const int *size, __global float *rr, __global float *ri, __global const float *xr, __global const float *xi)
{
	const unsigned idx=get_idx(size);
	float r1=xr[idx], i1=xi[idx];
	rr[idx]=sqrt(r1*r1+i1*i1);
	ri[idx]=atan2(i1, r1);
}
__kernel void c_q_polar(__global const int *size, __global float *rr, __global float *ri, __global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk)
{
	const unsigned idx=get_idx(size);
	float r1=xr[idx], i1=xi[idx], j1=xj[idx], k1=xk[idx];
	rr[idx]=sqrt(r1*r1+i1*i1+j1*j1+k1*k1);
	ri[idx]=acos(r1/rr[idx]);
}
__kernel bool disc_r_polar_i(__global const int *size, int offset, __global const float *xr)
{
	const unsigned idx=get_idx(size);
	if(xr[idx]<0)
		return xr[idx+offset]>=0;
	if(xr[idx]>0)
		return xr[idx+offset]<=0;
	return xr[idx+offset]!=0;
//	return xr[idx]<0?xr[idx+offset]>=0:xr[idx]>0?xr[idx+offset]<=0:xr[idx+offset]!=0;
}
__kernel bool disc_c_polar_i(__global const int *size, int offset, __global const float *xr){return false;}//TODO
__kernel bool disc_q_polar_i(__global const int *size, int offset, __global const float *xr){return false;}//TODO

__kernel void c_c_cartesian(__global const int *size, __global float *rr, __global float *ri, __global const float *xr, __global const float *xi)
{
	const unsigned idx=get_idx(size);
	float r=xr[idx], i=xi[idx];
	rr[idx]=r*cos(i);
	ri[idx]=r*sin(i);
}
__kernel void q_q_cartesian(__global const int *size, __global float *rr, __global float *ri, __global float *rj, __global float *rk, __global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk)
{
	const unsigned idx=get_idx(size);
	float r=xr[idx], i=xi[idx], j=xj[idx], k=xk[idx];
	float cos_j=cos(j), r_cos_k=r*cos(k);
	rr[idx]=cos(i)*cos_j*r_cos_k;
	ri[idx]=sin(i)*cos_j*r_cos_k;
	rj[idx]=sin(j)*r_cos_k;
	rk[idx]=r*sin(k);
}

__kernel void r_rr_plus(__global const int *size, __global float *rr, __global const float *xr, __global const float *yr)
{
	const unsigned idx=get_idx(size);
	rr[idx]=xr[idx]+yr[idx];
}
__kernel void c_rc_plus(__global const int *size,
	__global float *rr, __global float *ri,
	__global const float *xr,
	__global const float *yr, __global const float *yi)
{
	const unsigned idx=get_idx(size);
	rr[idx]=xr[idx]+yr[idx];
	ri[idx]=yi[idx];
}
__kernel void q_rq_plus(__global const int *size,
	__global float *rr, __global float *ri, __global float *rj, __global float *rk,
	__global const float *xr,
	__global const float *yr, __global const float *yi, __global const float *yj, __global const float *yk)
{
	const unsigned idx=get_idx(size);
	rr[idx]=xr[idx]+yr[idx];
	ri[idx]=yi[idx];
	rj[idx]=yj[idx];
	rk[idx]=yk[idx];
}
__kernel void c_cr_plus(__global const int *size,
	__global float *rr, __global float *ri,
	__global const float *xr, __global const float *xi,
	__global const float *yr)
{
	const unsigned idx=get_idx(size);
	rr[idx]=xr[idx]+yr[idx];
	ri[idx]=xi[idx];
}
__kernel void c_cc_plus(__global const int *size,
	__global float *rr, __global float *ri,
	__global const float *xr, __global const float *xi,
	__global const float *yr, __global const float *yi)
{
	const unsigned idx=get_idx(size);
	rr[idx]=xr[idx]+yr[idx];
	ri[idx]=xi[idx]+yi[idx];
}
__kernel void q_cq_plus(__global const int *size,
	__global float *rr, __global float *ri, __global float *rj, __global float *rk,
	__global const float *xr, __global const float *xi,
	__global const float *yr, __global const float *yi, __global const float *yj, __global const float *yk)
{
	const unsigned idx=get_idx(size);
	rr[idx]=xr[idx]+yr[idx];
	ri[idx]=xi[idx]+yi[idx];
	rj[idx]=yj[idx];
	rk[idx]=yk[idx];
}
__kernel void q_qr_plus(__global const int *size,
	__global float *rr, __global float *ri, __global float *rj, __global float *rk,
	__global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk,
	__global const float *yr)
{
	const unsigned idx=get_idx(size);
	rr[idx]=xr[idx]+yr[idx];
	ri[idx]=xi[idx];
	rj[idx]=xj[idx];
	rk[idx]=xk[idx];
}
__kernel void q_qc_plus(__global const int *size,
	__global float *rr, __global float *ri, __global float *rj, __global float *rk,
	__global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk,
	__global const float *yr, __global const float *yi)
{
	const unsigned idx=get_idx(size);
	rr[idx]=xr[idx]+yr[idx];
	ri[idx]=xi[idx]+yi[idx];
	rj[idx]=xj[idx];
	rk[idx]=xk[idx];
}
__kernel void q_qq_plus(__global const int *size,
	__global float *rr, __global float *ri, __global float *rj, __global float *rk,
	__global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk,
	__global const float *yr, __global const float *yi, __global const float *yj, __global const float *yk)
{
	const unsigned idx=get_idx(size);
	rr[idx]=xr[idx]+yr[idx];
	ri[idx]=xi[idx]+yi[idx];
	rj[idx]=xj[idx]+yj[idx];
	rk[idx]=xk[idx]+yk[idx];
}

__kernel void r_r_minus(__global const int *size, __global float *rr, __global const float *xr)
{
	const unsigned idx=get_idx(size);
	rr[idx]=-xr[idx];
}
__kernel void c_c_minus(__global const int *size, __global float *rr, __global float *ri, __global const float *xr, __global const float *xi)
{
	const unsigned idx=get_idx(size);
	rr[idx]=-xr[idx];
	ri[idx]=-xi[idx];
}
__kernel void q_q_minus(__global const int *size, __global float *rr, __global float *ri, __global float *rj, __global float *rk, __global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk)
{
	const unsigned idx=get_idx(size);
	rr[idx]=-xr[idx];
	ri[idx]=-xi[idx];
	rj[idx]=-xj[idx];
	rk[idx]=-xk[idx];
}
__kernel void r_rr_minus(__global const int *size, __global float *rr, __global const float *xr, __global const float *yr)
{
	const unsigned idx=get_idx(size);
	rr[idx]=xr[idx]-yr[idx];
}
__kernel void c_rc_minus(__global const int *size,
	__global float *rr, __global float *ri,
	__global const float *xr,
	__global const float *yr, __global const float *yi)
{
	const unsigned idx=get_idx(size);
	rr[idx]=xr[idx]-yr[idx];
	ri[idx]=-yi[idx];
}
__kernel void q_rq_minus(__global const int *size,
	__global float *rr, __global float *ri, __global float *rj, __global float *rk,
	__global const float *xr,
	__global const float *yr, __global const float *yi, __global const float *yj, __global const float *yk)
{
	const unsigned idx=get_idx(size);
	rr[idx]=xr[idx]-yr[idx];
	ri[idx]=-yi[idx];
	rj[idx]=-yj[idx];
	rk[idx]=-yk[idx];
}
__kernel void c_cr_minus(__global const int *size,
	__global float *rr, __global float *ri,
	__global const float *xr, __global const float *xi,
	__global const float *yr)
{
	const unsigned idx=get_idx(size);
	rr[idx]=xr[idx]-yr[idx];
	ri[idx]=xi[idx];
}
__kernel void c_cc_minus(__global const int *size,
	__global float *rr, __global float *ri,
	__global const float *xr, __global const float *xi,
	__global const float *yr, __global const float *yi)
{
	const unsigned idx=get_idx(size);
	rr[idx]=xr[idx]-yr[idx];
	ri[idx]=xi[idx]-yi[idx];
}
__kernel void q_cq_minus(__global const int *size,
	__global float *rr, __global float *ri, __global float *rk, __global float *rk,
	__global const float *xr, __global const float *xi,
	__global const float *yr, __global const float *yi, __global const float *yj, __global const float *yk)
{
	const unsigned idx=get_idx(size);
	rr[idx]=xr[idx]-yr[idx];
	ri[idx]=xi[idx]-yi[idx];
	rj[idx]=-yj[idx];
	rk[idx]=-yk[idx];
}
__kernel void q_qr_minus(__global const int *size,
	__global float *rr, __global float *ri, __global float *rj, __global float *rk,
	__global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk,
	__global const float *yr)
{
	const unsigned idx=get_idx(size);
	rr[idx]=xr[idx]-yr[idx];
	ri[idx]=xi[idx];
	rj[idx]=xj[idx];
	rk[idx]=xk[idx];
}
__kernel void q_qc_minus(__global const int *size,
	__global float *rr, __global float *ri, __global float *rj, __global float *rk,
	__global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk,
	__global const float *yr, __global const float *yi)
{
	const unsigned idx=get_idx(size);
	rr[idx]=xr[idx]-yr[idx];
	ri[idx]=xi[idx]-yi[idx];
	rj[idx]=xj[idx];
	rk[idx]=xk[idx];
}
__kernel void q_qq_minus(__global const int *size,
	__global float *rr, __global float *ri, __global float *rj, __global float *rk,
	__global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk,
	__global const float *yr, __global const float *yi, __global const float *yj, __global const float *yk)
{
	const unsigned idx=get_idx(size);
	rr[idx]=xr[idx]-yr[idx];
	ri[idx]=xi[idx]-yi[idx];
	rj[idx]=xj[idx]-yj[idx];
	rk[idx]=xk[idx]-yk[idx];
}

__kernel void r_rr_multiply(__global const int *size, __global float *rr, __global const float *xr, __global const float *yr)
{
	const unsigned idx=get_idx(size);
	rr[idx]=xr[idx]*yr[idx];
}
__kernel void c_rc_multiply(__global const int *size,
	__global float *rr, __global float *ri,
	__global const float *xr,
	__global const float *yr, __global const float *yi)
{
	const unsigned idx=get_idx(size);
	float
		r1=xr[idx],
		r2=yr[idx], i2=yi[idx];
	rr[idx]=r1*r2;
	ri[idx]=r1*i2;
}
__kernel void q_rq_multiply(__global const int *size,
	__global float *rr, __global float *ri, __global float *rj, __global float *rk,
	__global const float *xr,
	__global const float *yr, __global const float *yi, __global const float *yj, __global const float *yk)
{
	const unsigned idx=get_idx(size);
	float
		r1=xr[idx],
		r2=yr[idx], i2=yi[idx], j2=yj[idx], k2=yk[idx];
	rr[idx]=r1*r2;
	ri[idx]=r1*i2;
	rj[idx]=r1*j2;
	rk[idx]=r1*k2;
}
__kernel void c_cr_multiply(__global const int *size,
	__global float *rr, __global float *ri,
	__global const float *xr, __global const float *xi,
	__global const float *yr)
{
	const unsigned idx=get_idx(size);
	float
		r1=xr[idx], i1=xi[idx],
		r2=yr[idx];
	rr[idx]=r1*r2;
	ri[idx]=i1*r2;
}
__kernel void c_cc_multiply(__global const int *size,
	__global float *rr, __global float *ri,
	__global const float *xr, __global const float *xi,
	__global const float *yr, __global const float *yi)
{
	const unsigned idx=get_idx(size);
	float
		r1=xr[idx], i1=xi[idx],
		r2=yr[idx], i2=yi[idx];
	rr[idx]=r1*r2-i1*i2;
	ri[idx]=r1*i2+i1*r2;
}
__kernel void q_cq_multiply(__global const int *size,
	__global float *rr, __global float *ri, __global float *rj, __global float *rk,
	__global const float *xr, __global const float *xi,
	__global const float *yr, __global const float *yi, __global const float *yj, __global const float *yk)
{
	const unsigned idx=get_idx(size);
	float
		r1=xr[idx], i1=xi[idx],
		r2=yr[idx], i2=yi[idx], j2=yj[idx], k2=yk[idx];
	rr[idx]=r1*r2-i1*i2;
	ri[idx]=r1*i2+i1*r2;
	rj[idx]=r1*j2-i1*k2;
	rk[idx]=r1*k2+i1*j2;
}
__kernel void q_qr_multiply(__global const int *size,
	__global float *rr, __global float *ri, __global float *rj, __global float *rk,
	__global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk,
	__global const float *yr)
{
	const unsigned idx=get_idx(size);
	float
		r1=xr[idx], i1=xi[idx], j1=xj[idx], k1=xk[idx],
		r2=yr[idx];
	rr[idx]=r1*r2;
	ri[idx]=i1*r2;
	rj[idx]=j1*r2;
	rk[idx]=k1*r2;
}
__kernel void q_qc_multiply(__global const int *size,
	__global float *rr, __global float *ri, __global float *rj, __global float *rk,
	__global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk,
	__global const float *yr, __global const float *yi)
{
	const unsigned idx=get_idx(size);
	float
		r1=xr[idx], i1=xi[idx], j1=xj[idx], k1=xk[idx],
		r2=yr[idx], i2=yi[idx];
	rr[idx]=r1*r2-i1*i2;
	ri[idx]=r1*i2+i1*r2;
	rj[idx]=j1*r2+k1*i2;
	rk[idx]=-j1*i2+k1*r2;
}
__kernel void q_qq_multiply(__global const int *size,
	__global float *rr, __global float *ri, __global float *rj, __global float *rk,
	__global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk,
	__global const float *yr, __global const float *yi, __global const float *yj, __global const float *yk)
{
	const unsigned idx=get_idx(size);
	float
		r1=xr[idx], i1=xi[idx], j1=xj[idx], k1=xk[idx],
		r2=yr[idx], i2=yi[idx], j2=yi[idx], k2=yi[idx];
	rr[idx]=r1*r2-i1*i2-j1*j2-k1*k2;
	ri[idx]=r1*i2+i1*r2+j1*k2-k1*j2;
	rj[idx]=r1*j2-i1*k2+j1*r2+k1*i2;
	rk[idx]=r1*k2+i1*j2-j1*i2+k1*r2;
}

__kernel void r_r_divide(__global const int *size, __global float *rr, __global const float *xr)
{
	const unsigned idx=size[0]*(size[1]*get_global_id(2)+get_global_id(1))+get_global_id(0);
	rr[idx]=1/xr[idx];
}
__kernel void c_c_divide(__global const int *size, __global float *rr, __global float *ri, __global const float *xr, __global const float *xi)
{
	const unsigned idx=size[0]*(size[1]*get_global_id(2)+get_global_id(1))+get_global_id(0);
	float r=xr[idx], i=xi[idx];
	float inv_abs_x=rsqrt(r*r+i*i);
	rr[idx]= r*inv_abs_x;
	ri[idx]=-i*inv_abs_x;
}
__kernel void q_q_divide(__global const int *size, __global float *rr, __global float *ri, __global float *rj, __global float *rk, __global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk)
{
	const unsigned idx=size[0]*(size[1]*get_global_id(2)+get_global_id(1))+get_global_id(0);
	float r=xr[idx], i=xi[idx], j=xj[idx], k=xk[idx];
	float inv_abs_x=rsqrt(r*r+i*i+j*j+k*k);
	rr[idx]= r*inv_abs_x;
	ri[idx]=-i*inv_abs_x;
	rj[idx]=-j*inv_abs_x;
	rk[idx]=-k*inv_abs_x;
}
__kernel void r_rr_divide(__global const int *size, __global float *rr, __global const float *xr, __global const float *yr)
{
	const unsigned idx=size[0]*(size[1]*get_global_id(2)+get_global_id(1))+get_global_id(0);
	rr[idx]=xr[idx]/yr[idx];
}
__kernel void c_rc_divide(__global const int *size,
	__global float *rr, __global float *ri,
	__global const float *xr,
	__global const float *yr, __global const float *yi)
{
	const unsigned idx=size[0]*(size[1]*get_global_id(2)+get_global_id(1))+get_global_id(0);
	float
		ra=xr[idx],
		rb=yr[idx], ib=yi[idx];
	float a_absb=ra/(rb*rb+ib*ib);
	rr[idx]= rb*a_absb;
	ri[idx]=-ib*a_absb;
}
__kernel void q_rq_divide(__global const int *size,
	__global float *rr, __global float *ri, __global float *rj, __global float *rk,
	__global const float *xr,
	__global const float *yr, __global const float *yi, __global const float *yj, __global const float *yk)
{
	const unsigned idx=size[0]*(size[1]*get_global_id(2)+get_global_id(1))+get_global_id(0);
	float
		ra=xr[idx],
		rb=yr[idx], ib=yi[idx], jb=yj[idx], kb=yk[idx];
	float a_absb=ra/(rb*rb+ib*ib+jb*jb+kb*kb);
	rr[idx]= rb*a_absb;
	ri[idx]=-ib*a_absb;
	rj[idx]=-jb*a_absb;
	rk[idx]=-kb*a_absb;
}
__kernel void c_cr_divide(__global const int *size,
	__global float *rr, __global float *ri,
	__global const float *xr, __global const float *xi,
	__global const float *yr)
{
	const unsigned idx=size[0]*(size[1]*get_global_id(2)+get_global_id(1))+get_global_id(0);
	float
		ra=xr[idx], ia=xi[idx],
		rb=yr[idx];
	rr[idx]=ra/rb;
	ri[idx]=ia/rb;
}
__kernel void c_cc_divide(__global const int *size,
	__global float *rr, __global float *ri,
	__global const float *xr, __global const float *xi,
	__global const float *yr, __global const float *yi)
{
	const unsigned idx=size[0]*(size[1]*get_global_id(2)+get_global_id(1))+get_global_id(0);
	float
		ra=xr[idx], ia=xi[idx],
		rb=yr[idx], ib=yi[idx];
	float inv_absb=1/(rb*rb+ib*ib);
	rr[idx]=(ra*rb+ia*ib)*inv_absb;
	ri[idx]=(ia*rb-ra*ib)*inv_absb;
}
__kernel void q_cq_divide(__global const int *size,
	__global float *rr, __global float *ri, __global float *rj, __global float *rk,
	__global const float *xr, __global const float *xi,
	__global const float *yr, __global const float *yi, __global const float *yj, __global const float *yk)
{
	const unsigned idx=size[0]*(size[1]*get_global_id(2)+get_global_id(1))+get_global_id(0);
	float
		ra=xr[idx], ia=xi[idx],
		rb=yr[idx], ib=yi[idx], jb=yj[idx], kb=yk[idx];
	float inv_absb=1/(rb*rb+ib*ib+jb*jb+kb*kb);
	rr[idx]=(ra*rb+ia*ib)*inv_absb;
	ri[idx]=(ia*rb-ra*ib)*inv_absb;
	rr[idx]=(-ra*jb-ia*kb)*inv_absb;
	ri[idx]=(ia*jb-ra*bk)*inv_absb;
}
__kernel void q_qr_divide(__global const int *size,
	__global float *rr, __global float *ri, __global float *rj, __global float *rk,
	__global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk,
	__global const float *yr)
{
	const unsigned idx=size[0]*(size[1]*get_global_id(2)+get_global_id(1))+get_global_id(0);
	float
		ra=xr[idx], ia=xi[idx], ja=xj[idx], ka=xk[idx],
		rb=yr[idx];
	float inv_b=1/rb;
	rr[idx]=ra/rb;
	ri[idx]=ia/rb;
	rr[idx]=ja/rb;
	ri[idx]=ka/rb;
}
__kernel void q_qc_divide(__global const int *size,
	__global float *rr, __global float *ri, __global float *rj, __global float *rk,
	__global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk,
	__global const float *yr, __global const float *yi)
{
	const unsigned idx=size[0]*(size[1]*get_global_id(2)+get_global_id(1))+get_global_id(0);
	float
		ra=xr[idx], ia=xi[idx], ja=xj[idx], ka=xk[idx],
		rb=yr[idx], ib=yi[idx];
	float inv_absb2=1/(rb*rb+ib*ib);
	rr[idx]=(ra*rb+ia*ib)*inv_absb2;
	ri[idx]=(ia*rb-ra*ib)*inv_absb2;
	rr[idx]=(ja*rb+ka*ib)*inv_absb2;
	ri[idx]=(ka*rb-ja*ib)*inv_absb2;
}
__kernel void q_qq_divide(__global const int *size,
	__global float *rr, __global float *ri, __global float *rj, __global float *rk,
	__global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk,
	__global const float *yr, __global const float *yi, __global const float *yj, __global const float *yk)
{
	const unsigned idx=size[0]*(size[1]*get_global_id(2)+get_global_id(1))+get_global_id(0);
	float
		ra=xr[idx], ia=xi[idx], ja=xj[idx], ka=xk[idx],
		rb=yr[idx], ib=yi[idx], jb=yj[idx], kb=yk[idx];
	float inv_absb2=1/(rb*rb+ib*ib+jb*jb+kb*kb);
	rr[idx]=(ra*rb+ia*ib+ja*jb+ka*kb)*inv_absb2;
	ri[idx]=(ia*rb-ra*ib-ka*jb+ja*kb)*inv_absb2;
	rr[idx]=(ja*rb+ka*ib-ra*jb-ia*kb)*inv_absb2;
	ri[idx]=(ka*rb-ja*ib+ia*jb-ra*kb)*inv_absb2;
}
__kernel bool disc_r_divide_i(__global const int *size, int offset, __global const float *xr)
{
	const unsigned idx=size[0]*(size[1]*get_global_id(2)+get_global_id(1))+get_global_id(0);
	float x0r=xr[idx], x1r=xr[idx+offset];
	return x0r<0?x1r>=0:x0r>0?x1r<=0:x1r!=0;
}
__kernel bool disc_c_divide_i(__global const int *size, int offset, __global const float *xr, __global const float *xi)
{
	const unsigned idx=size[0]*(size[1]*get_global_id(2)+get_global_id(1))+get_global_id(0);
	float
		x0r=xr[idx], x1r=xr[idx+offset],
		x0i=xi[idx], x1i=xi[idx+offset];
	return false;
}
__kernel bool disc_q_divide_i(__global const int *size, int offset, __global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk)
{
	const unsigned idx=size[0]*(size[1]*get_global_id(2)+get_global_id(1))+get_global_id(0);
	float
		x0r=xr[idx], x1r=xr[idx+offset],
		x0i=xi[idx], x1i=xi[idx+offset],
		x0j=xj[idx], x1j=xj[idx+offset],
		x0k=xk[idx], x1k=xk[idx+offset];
	return false;
}
__kernel bool disc_rr_divide_i(__global const int *size, int offset, __global const float *xr, __global const float *yr)
{
	const unsigned idx=size[0]*(size[1]*get_global_id(2)+get_global_id(1))+get_global_id(0);
	float y0r=yr[idx], y1r=yr[idx+offset];
	return y0r<0?y1r>=0:y0r>0?y1r<=0:y1r!=0;
}
__kernel bool disc_rc_divide_i(__global const int *size, int offset, __global const float *xr, __global const float *yr, __global const float *yi)
{
	const unsigned idx=size[0]*(size[1]*get_global_id(2)+get_global_id(1))+get_global_id(0);
	float
		y0r=yr[idx], y1r=yr[idx+offset],
		y0i=yi[idx], y1i=yi[idx+offset];
	return false;
}
__kernel bool disc_rq_divide_i(__global const int *size, int offset, __global const float *xr, __global const float *yr, __global const float *yi, __global const float *yj, __global const float *yk)
{
	const unsigned idx=size[0]*(size[1]*get_global_id(2)+get_global_id(1))+get_global_id(0);
	float
		y0r=yr[idx], y1r=yr[idx+offset],
		y0i=yi[idx], y1i=yi[idx+offset],
		y0j=yj[idx], y1j=yj[idx+offset],
		y0k=yk[idx], y1k=yk[idx+offset];
	return false;
}
__kernel bool disc_cr_divide_i(__global const int *size, int offset, __global const float *xr, __global const float *xi, __global const float *yr)
{
	const unsigned idx=size[0]*(size[1]*get_global_id(2)+get_global_id(1))+get_global_id(0);
	float y0r=yr[idx], y1r=yr[idx+offset];
	return y0r<0?y1r>=0:y0r>0?y1r<=0:y1r!=0;
}
__kernel bool disc_cc_divide_i(__global const int *size, int offset, __global const float *xr, __global const float *xi, __global const float *yr, __global const float *yi)
{
	const unsigned idx=size[0]*(size[1]*get_global_id(2)+get_global_id(1))+get_global_id(0);
	float y0r=yr[idx], y1r=yr[idx+offset];
	return false;
}
__kernel bool disc_cq_divide_i(__global const int *size, int offset, __global const float *xr, __global const float *xi, __global const float *yr, __global const float *yi, __global const float *yj, __global const float *yk)
{
	const unsigned idx=size[0]*(size[1]*get_global_id(2)+get_global_id(1))+get_global_id(0);
	float y0r=yr[idx], y1r=yr[idx+offset];
	return false;
}
__kernel bool disc_qr_divide_i(__global const int *size, int offset,
	__global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk,
	__global const float *yr)
{
	const unsigned idx=size[0]*(size[1]*get_global_id(2)+get_global_id(1))+get_global_id(0);
	float y0r=yr[idx], y1r=yr[idx+offset];
	return y0r<0?y1r>=0:y0r>0?y1r<=0:y1r!=0;
}
__kernel bool disc_qc_divide_i(__global const int *size, int offset,
	__global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk,
	__global const float *yr, __global const float *yi)
{
	const unsigned idx=size[0]*(size[1]*get_global_id(2)+get_global_id(1))+get_global_id(0);
	float
		y0r=yr[idx], y1r=yr[idx+offset],
		y0i=yi[idx], y1i=yi[idx+offset];
	return false;//TODO
}
__kernel bool disc_qq_divide_i(__global const int *size, int offset,
	__global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk,
	__global const float *yr, __global const float *yi, __global const float *yj, __global const float *yk)
{
	const unsigned idx=size[0]*(size[1]*get_global_id(2)+get_global_id(1))+get_global_id(0);
	float
		y0r=yr[idx], y1r=yr[idx+offset],
		y0i=yi[idx], y1i=yi[idx+offset],
		y0j=yj[idx], y1j=yj[idx+offset],
		y0k=yk[idx], y1k=yk[idx+offset];
	return false;//TODO
}
)CLSRC";

static const char program_src_pt2[]=R"CLSRC(
//declarations
int get_idx(__global const int *size);

__kernel void r_r_divide(__global const int *size, __global float *rr, __global const float *xr);
__kernel void c_c_divide(__global const int *size, __global float *rr, __global float *ri, __global const float *xr, __global const float *xi);
__kernel void q_q_divide(__global const int *size, __global float *rr, __global float *ri, __global float *rj, __global float *rk, __global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk);
__kernel void r_rr_divide(__global const int *size, __global float *rr, __global const float *xr, __global const float *yr);
__kernel void c_rc_divide(__global const int *size,
	__global float *rr, __global float *ri,
	__global const float *xr,
	__global const float *yr, __global const float *yi);
__kernel void q_rq_divide(__global const int *size,
	__global float *rr, __global float *ri, __global float *rj, __global float *rk,
	__global const float *xr,
	__global const float *yr, __global const float *yi, __global const float *yj, __global const float *yk);
__kernel void c_cr_divide(__global const int *size,
	__global float *rr, __global float *ri,
	__global const float *xr, __global const float *xi,
	__global const float *yr);
__kernel void c_cc_divide(__global const int *size,
	__global float *rr, __global float *ri,
	__global const float *xr, __global const float *xi,
	__global const float *yr, __global const float *yi);
__kernel void q_cq_divide(__global const int *size,
	__global float *rr, __global float *ri, __global float *rj, __global float *rk,
	__global const float *xr, __global const float *xi,
	__global const float *yr, __global const float *yi, __global const float *yj, __global const float *yk);
__kernel void q_qr_divide(__global const int *size,
	__global float *rr, __global float *ri, __global float *rj, __global float *rk,
	__global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk,
	__global const float *yr);
__kernel void q_qc_divide(__global const int *size,
	__global float *rr, __global float *ri, __global float *rj, __global float *rk,
	__global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk,
	__global const float *yr, __global const float *yi);
__kernel void q_qq_divide(__global const int *size,
	__global float *rr, __global float *ri, __global float *rj, __global float *rk,
	__global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk,
	__global const float *yr, __global const float *yi, __global const float *yj, __global const float *yk);

//kernels
__kernel void r_rr_logic_divides(__global const int *size,
	__global float *rr,
	__global const float *xr,
	__global const float *yr)
{
	const unsigned idx=size[0]*(size[1]*get_global_id(2)+get_global_id(1))+get_global_id(0);
	float
		ra=xr[idx],
		rb=yr[idx];
	float q=ra/rb;
	rr[idx]=q==floor(q);
}
__kernel bool disc_r_logic_divides_o(__global const int *size, int offset, __global const float *or)
{
	const unsigned idx=size[0]*(size[1]*get_global_id(2)+get_global_id(1))+get_global_id(0);
	return or[idx]!=or[idx+offset];
}
)CLSRC";
}//end CLSource
namespace 	G2_CL
{
	CLKernel kernels[]=
	{
		//special kernels
		{V_INITIALIZE_CONSTANTS, 0, 0, "initialize_constants", nullptr},

		//G2 functions
		{R_R_SETZERO, CL_R_R, DISC_CONT, "r_r_setzero", nullptr},
		{C_C_SETZERO, CL_C_C, DISC_CONT, "c_c_setzero", nullptr},
		{Q_Q_SETZERO, CL_Q_Q, DISC_CONT, "q_q_setzero", nullptr},

		{R_R_CEIL, CL_R_R, DISC_OUTPUT, "r_r_ceil", "disc_r_ceil_o"},
		{C_C_CEIL, CL_C_C, DISC_OUTPUT, "c_c_ceil", "disc_c_ceil_o"},
		{Q_Q_CEIL, CL_Q_Q, DISC_OUTPUT, "q_q_ceil", "disc_q_ceil_o"},

		{R_R_FLOOR, CL_R_R, DISC_OUTPUT, "r_r_floor", "disc_r_floor_o"},
		{C_C_FLOOR, CL_C_C, DISC_OUTPUT, "c_c_floor", "disc_c_floor_o"},
		{Q_Q_FLOOR, CL_Q_Q, DISC_OUTPUT, "q_q_floor", "disc_q_floor_o"},

		{R_R_ROUND, CL_R_R, DISC_OUTPUT, "r_r_round", "disc_r_round_o"},
		{C_C_ROUND, CL_C_C, DISC_OUTPUT, "c_c_round", "disc_c_round_o"},
		{Q_Q_ROUND, CL_Q_Q, DISC_OUTPUT, "q_q_round", "disc_c_round_o"},

		{R_R_INT, CL_R_R, DISC_OUTPUT, "r_r_int", "disc_r_int_o"},
		{C_C_INT, CL_C_C, DISC_OUTPUT, "c_c_int", "disc_c_int_o"},
		{Q_Q_INT, CL_Q_Q, DISC_OUTPUT, "q_q_int", "disc_c_int_o"},

		{R_R_FRAC, CL_R_R, DISC_INPUT, "r_r_frac", "disc_r_frac_i"},
		{C_C_FRAC, CL_C_C, DISC_INPUT, "c_c_frac", "disc_c_frac_i"},
		{Q_Q_FRAC, CL_Q_Q, DISC_INPUT, "q_q_frac", "disc_q_frac_i"},

		{R_R_ABS, CL_R_R, DISC_CONT, "r_r_abs", nullptr},
		{R_C_ABS, CL_R_C, DISC_CONT, "r_c_abs", nullptr},
		{R_Q_ABS, CL_R_Q, DISC_CONT, "r_q_abs", nullptr},

		{R_R_ARG, CL_R_R, DISC_INPUT, "r_r_arg", "disc_r_arg_i"},
		{R_C_ARG, CL_R_C, DISC_INPUT, "r_c_arg", "disc_c_arg_i"},
		{R_Q_ARG, CL_Q_Q, DISC_INPUT, "r_q_arg", "disc_c_arg_i"},

		{R_C_REAL, CL_R_C, DISC_CONT, "r_c_real", nullptr},

		{R_C_IMAG, CL_R_C, DISC_CONT, "r_c_imag", nullptr},

		{C_C_CONJUGATE, CL_C_C, DISC_CONT, "c_c_conjugate", nullptr},
		{Q_Q_CONJUGATE, CL_Q_Q, DISC_CONT, "q_q_conjugate", nullptr},

		{C_R_POLAR, CL_C_R, DISC_INPUT, "c_r_polar", "disc_r_polar_i"},
		{C_C_POLAR, CL_C_C, DISC_INPUT, "c_c_polar", "disc_c_polar_i"},
		{C_Q_POLAR, CL_C_Q, DISC_INPUT, "c_q_polar", "disc_q_polar_i"},

		{C_C_CARTESIAN, CL_C_C, DISC_CONT, "c_c_cartesian", nullptr},
		{Q_Q_CARTESIAN, CL_Q_Q, DISC_CONT, "q_q_cartesian", nullptr},

		{R_RR_PLUS, CL_R_RR, DISC_CONT, "r_rr_plus", nullptr},
		{C_RC_PLUS, CL_C_RC, DISC_CONT, "c_rc_plus", nullptr},
		{Q_RQ_PLUS, CL_Q_RQ, DISC_CONT, "q_rq_plus", nullptr},
		{C_CR_PLUS, CL_C_CR, DISC_CONT, "c_cr_plus", nullptr},
		{C_CC_PLUS, CL_C_CC, DISC_CONT, "c_cc_plus", nullptr},
		{Q_CQ_PLUS, CL_Q_CQ, DISC_CONT, "q_cq_plus", nullptr},
		{Q_QR_PLUS, CL_Q_QR, DISC_CONT, "q_qr_plus", nullptr},
		{Q_QC_PLUS, CL_Q_QC, DISC_CONT, "q_qc_plus", nullptr},
		{Q_QQ_PLUS, CL_Q_QQ, DISC_CONT, "q_qq_plus", nullptr},

		{R_R_MINUS, CL_R_R, DISC_CONT, "r_r_minus", nullptr},
		{C_C_MINUS, CL_C_C, DISC_CONT, "c_c_minus", nullptr},
		{Q_Q_MINUS, CL_Q_Q, DISC_CONT, "q_q_minus", nullptr},
		{R_RR_MINUS, CL_R_RR, DISC_CONT, "r_rr_minus", nullptr},
		{C_RC_MINUS, CL_C_RC, DISC_CONT, "c_rc_minus", nullptr},
		{Q_RQ_MINUS, CL_Q_RQ, DISC_CONT, "q_rq_minus", nullptr},
		{C_CR_MINUS, CL_C_CR, DISC_CONT, "c_cr_minus", nullptr},
		{C_CC_MINUS, CL_C_CC, DISC_CONT, "c_cc_minus", nullptr},
		{Q_CQ_MINUS, CL_Q_CQ, DISC_CONT, "q_cq_minus", nullptr},
		{Q_QR_MINUS, CL_Q_QR, DISC_CONT, "q_qr_minus", nullptr},
		{Q_QC_MINUS, CL_Q_QC, DISC_CONT, "q_qc_minus", nullptr},
		{Q_QQ_MINUS, CL_Q_QQ, DISC_CONT, "q_qq_minus", nullptr},
		
		{R_RR_MULTIPLY, CL_R_RR, DISC_CONT, "r_rr_multiply", nullptr},
		{C_RC_MULTIPLY, CL_C_RC, DISC_CONT, "c_rc_multiply", nullptr},
		{Q_RQ_MULTIPLY, CL_Q_RQ, DISC_CONT, "q_rq_multiply", nullptr},
		{C_CR_MULTIPLY, CL_C_CR, DISC_CONT, "c_cr_multiply", nullptr},
		{C_CC_MULTIPLY, CL_C_CC, DISC_CONT, "c_cc_multiply", nullptr},
		{Q_CQ_MULTIPLY, CL_Q_CQ, DISC_CONT, "q_cq_multiply", nullptr},
		{Q_QR_MULTIPLY, CL_Q_QR, DISC_CONT, "q_qr_multiply", nullptr},
		{Q_QC_MULTIPLY, CL_Q_QC, DISC_CONT, "q_qc_multiply", nullptr},
		{Q_QQ_MULTIPLY, CL_Q_QQ, DISC_CONT, "q_qq_multiply", nullptr},
		
		{R_R_DIVIDE, CL_R_R, DISC_INPUT, "r_r_divide", "disc_r_divide_i"},
		{C_C_DIVIDE, CL_C_C, DISC_INPUT, "c_c_divide", "disc_c_divide_i"},
		{Q_Q_DIVIDE, CL_Q_Q, DISC_INPUT, "q_q_divide", "disc_q_divide_i"},
		{R_RR_DIVIDE, CL_R_RR, DISC_INPUT, "r_rr_divide", "disc_rr_divide_i"},
		{C_RC_DIVIDE, CL_C_RC, DISC_INPUT, "c_rc_divide", "disc_rc_divide_i"},
		{Q_RQ_DIVIDE, CL_Q_RQ, DISC_INPUT, "q_rq_divide", "disc_rq_divide_i"},
		{C_CR_DIVIDE, CL_C_CR, DISC_INPUT, "c_cr_divide", "disc_cr_divide_i"},
		{C_CC_DIVIDE, CL_C_CC, DISC_INPUT, "c_cc_divide", "disc_cc_divide_i"},
		{Q_CQ_DIVIDE, CL_Q_CQ, DISC_INPUT, "q_cq_divide", "disc_cq_divide_i"},
		{Q_QR_DIVIDE, CL_Q_QR, DISC_INPUT, "q_qr_divide", "disc_qr_divide_i"},
		{Q_QC_DIVIDE, CL_Q_QC, DISC_INPUT, "q_qc_divide", "disc_qc_divide_i"},
		{Q_QQ_DIVIDE, CL_Q_QQ, DISC_INPUT, "q_qq_divide", "disc_qc_divide_i"},

		{R_RR_LOGIC_DIVIDES, CL_R_RR, DISC_OUTPUT, "r_rr_logic_divides", "disc_r_logic_divides_o"},
		{R_RC_LOGIC_DIVIDES, CL_R_RC, DISC_OUTPUT, "r_rc_logic_divides", "disc_r_logic_divides_o"},
	};
	const int CLKernel_size=sizeof(CLKernel), nkernels=sizeof(kernels)/CLKernel_size;
}
#if 0
const size_t worksize=100;
float		in1[worksize]={0}, in2[worksize]={0}, out[worksize]={0};
#endif
void 		cl_initiate()
{
	load_OpenCL_API();
	using namespace G2_CL;
	for(int k=0;k<nkernels;++k)//idx check
	{
		if(kernels[k].idx!=k)
		{
			break;
		}
	}
	cl_int error;
	cl_platform_id platform;
	cl_device_id device;
	unsigned platforms, devices;
	// Fetch the Platform and Device IDs; we only want one.
	error=p_clGetPlatformIDs(1, &platform, &platforms);							CL_CHECK(error);
	error=p_clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, &devices);	CL_CHECK(error);
	cl_context_properties properties[]=
	{
		CL_CONTEXT_PLATFORM,
		(cl_context_properties)platform,
		0
	};
	cl_context context=p_clCreateContext(properties, 1, &device, NULL, NULL, &error);	CL_CHECK(error);
	cl_command_queue cq=p_clCreateCommandQueue(context, device, 0, &error);				CL_CHECK(error);
#if 0
	cl_int error;
	cl_platform_id platform;
	cl_device_id device;
	unsigned platforms, devices;
	// Fetch the Platform and Device IDs; we only want one.
	error=p_clGetPlatformIDs(1, &platform, &platforms);							CL_CHECK(error);
	error=p_clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, &devices);	CL_CHECK(error);
	cl_context_properties properties[]=
	{
		CL_CONTEXT_PLATFORM,
		(cl_context_properties)platform,
		0
	};
	cl_context context=p_clCreateContext(properties, 1, &device, NULL, NULL, &error);	CL_CHECK(error);
	cl_command_queue cq=p_clCreateCommandQueue(context, device, 0, &error);				CL_CHECK(error);
	const char src[]=R"CLSRC(
__kernel void myfunc(__global const float *in1, __global const float *in2, __global float *out)
{
	const unsigned idx=get_global_id(0);
	out[idx]=in1[idx]+in2[idx];
//	out[idx]=get_global_id(0);
}
)CLSRC";
	const char *sources[]=
	{
		src,
	};
	size_t srclen[]=
	{
		strlen(src),
	};
	cl_program program=p_clCreateProgramWithSource(context, 1, sources, srclen, &error);CL_CHECK(error);
	error=p_clBuildProgram(program, 0, nullptr, "", nullptr, nullptr);					CL_CHECK(error);
	cl_kernel k_add=p_clCreateKernel(program, "myfunc", &error);						CL_CHECK(error);
	if(error)
	{
		size_t length=0;
		error=p_clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, g_buf_size, g_buf, &length);	CL_CHECK2(error);
	}
	for(int k=0;k<worksize;++k)//initialize	TODO: ititialize in kernel
	{
		in1[k]=k;
		in2[k]=worksize-1-k;
		out[k]=rand();//
	}
	cl_mem mem_in1=p_clCreateBuffer(context, CL_MEM_READ_ONLY, worksize*sizeof(float), nullptr, &error);	CL_CHECK(error);
	cl_mem mem_in2=p_clCreateBuffer(context, CL_MEM_READ_ONLY, worksize*sizeof(float), nullptr, &error);	CL_CHECK(error);
	cl_mem mem_out=p_clCreateBuffer(context, CL_MEM_WRITE_ONLY, worksize*sizeof(float), nullptr, &error);	CL_CHECK(error);
	error=p_clSetKernelArg(k_add, 0, sizeof(cl_mem), &mem_in1); CL_CHECK(error);
	error=p_clSetKernelArg(k_add, 1, sizeof(cl_mem), &mem_in2); CL_CHECK(error);
	error=p_clSetKernelArg(k_add, 2, sizeof(cl_mem), &mem_out); CL_CHECK(error);
	error=p_clEnqueueWriteBuffer(cq, mem_in1, CL_FALSE, 0, worksize*sizeof(float), in1, 0, nullptr, nullptr);	CL_CHECK(error);
	error=p_clEnqueueWriteBuffer(cq, mem_in2, CL_FALSE, 0, worksize*sizeof(float), in2, 0, nullptr, nullptr);	CL_CHECK(error);
	error=p_clEnqueueNDRangeKernel(cq, k_add, 1, nullptr, &worksize, &worksize, 0, nullptr, nullptr);			CL_CHECK(error);
	error=p_clEnqueueReadBuffer(cq, mem_out, CL_FALSE, 0, worksize*sizeof(float), out, 0, nullptr, nullptr);	CL_CHECK(error);
	error=p_clFinish(cq);	CL_CHECK(error);
	unload_OpenCL_API();//
#endif
}
void 		cl_step()
{
}