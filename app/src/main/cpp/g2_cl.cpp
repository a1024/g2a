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
#include "GLESAPI.h"//TODO: g2_common.h
#define CL_TARGET_OPENCL_VERSION 120
#include<CL/opencl.h>
#include<dlfcn.h>
//#include<vector>
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
	C_PI, C_2PI, C_SQRT2, C_INV_LN10
};
__global float *constants;
#define	_pi			(constants[C_PI])
#define	_2pi		(constants[C_2PI])
#define	_sqrt2		(constants[C_SQRT2])
#define	_inv_ln10	(constants[C_INV_LN10])
__kernel void initialize_constants()
{
	_pi=acos(-1.f);
	_2pi=_pi+_pi;
	_sqrt2=sqrt(2.f);
	_inv_ln10=1/log(10.f);
}

//auxiliary functions
int get_idx(__global const int *size){return size[0]*(size[1]*get_global_id(2)+get_global_id(1))+get_global_id(0);}
void	equal_cc(float2 a, float2 b){return a.x==b.x&&a.y==b.y;}
void	equal_qq(float4 a, float4 b){return a.x==b.x&&a.y==b.y&&a.z==b.z&&a.w==b.w;}
float2	floor_c(float2 a){return (float2)(floor(a.x), floor(a.y));}
float4	floor_q(float4 a){return (float4)(floor(a.x), floor(a.y), floor(a.z), floor(a.w));}
float2	mul_rc(float a, float2 b){return (float2)(a*b.x, a*b.y);}
float4	mul_rq(float a, float4 b){return (float4)(a*b.x, a*b.y, a*b.z, a*b.w);}
float2	mul_cr(float2 a, float b){return (float2)(a.x*b, a.y*b);}
float2	mul_cc(float2 a, float2 b){return (float2)(a.x*b.x-a.y*b.y, a.x*b.y+a.y*b.x);}
float4	mul_cq(float2 a, float4 b)
{
	return (float4)
	(
		a.x*b.x-a.y*b.y,
		a.x*b.y+a.y*b.x,
		a.x*b.z-a.y*b.w,
		a.x*b.w+a.y*b.z
	);
}
float4	mul_qr(float4 a, float b){return (float4)(a.x*b, a.y*b, a.z*b, a.w*b);}
float4	mul_qc(float4 a, float2 b)
{
	return (float4)
	(
		 a.x*b.x-a.y*b.y,
		 a.x*b.y+a.y*b.x,
		 a.z*b.x+a.w*b.y,
		-a.z*b.y+a.w*b.x
	);
}
float4	mul_qq(float4 a, float4 b)
{
	return (float4)
	(
		a.x*b.x-a.y*b.y-a.z*b.z-a.w*b.w,
		a.x*b.y+a.y*b.x+a.z*b.w-a.w*b.z,
		a.x*b.z-a.y*b.w+a.z*b.x+a.w*b.y,
		a.x*b.w+a.y*b.z-a.z*b.y+a.w*b.x
	);
}
float2	inv_c(float2 a)
{
	float inv_abs2=1/(a.x*a.x+a.y*a.y);
	return (float2)(a.x*inv_abs2, -a.y*inv_abs2);
}
float4	inv_q(float4 a)
{
	float inv_abs2=1/(a.x*a.x+a.y*a.y+a.z*a.z+a.w*a.w);
	return (float4)(a.x*inv_abs2, -a.y*inv_abs2, -a.z*inv_abs2, -a.w*inv_abs2);
}
float2	div_rc(float a, float2 b)
{
	float a_absb2=a/(b.x*b.x+b.y*b.y);
	return (float2)(b.x*a_absb2, -b.y*a_absb2);
}
float4	div_rq(float a, float4 b)
{
	float a_absb2=a/(b.x*b.x+b.y*b.y+b.z*b.z+b.w*b.w);
	return (float4)(b.x*a_absb2, -b.y*a_absb2, -b.z*a_absb2, -b.w*a_absb2);
}
float2	div_cr(float2 a, float b)
{
	float inv_b=1/b;
	return (float2)(a.x*inv_b, a.y*inv_b);
}
float2	div_cc(float2 a, float2 b)
{
	float inv_absb2=1/(b.x*b.x+b.y*b.y);
	return (float2)
	(
		(a.x*b.x+a.y*b.y)*inv_absb2,
		(a.y*b.x-a.x*b.y)*inv_absb2
	);
}
float4	div_cq(float2 a, float4 b)
{
	float inv_absb2=1/(b.x*b.x+b.y*b.y+b.z*b.z+b.w*b.w);
	return (float4)
	(
		( a.x*b.x+a.y*b.y)*inv_absb2,
		( a.y*b.x-a.x*b.y)*inv_absb2,
		(-a.x*b.z-a.y*b.w)*inv_absb2,
		( a.y*b.z-a.x*b.w)*inv_absb2
	);
}
float4	div_qr(float4 a, float b)
{
	float inv_b=1/b;
	return (float4)
	(
		a.x*inv_b,
		a.y*inv_b,
		a.z*inv_b,
		a.w*inv_b
	);
}
float4	div_qc(float4 a, float2 b)
{
	float inv_absb2=1/(b.x*b.x+b.y*b.y);
	return (float4)
	(
		(a.x*b.x+a.y*b.y)*inv_absb2,
		(a.y*b.x-a.x*b.y)*inv_absb2,
		(a.z*b.x+a.w*b.y)*inv_absb2,
		(a.w*b.x-a.z*b.y)*inv_absb2
	);
}
float4	div_qq(float4 a, float4 b)
{
	float inv_absb2=1/(b.x*b.x+b.y*b.y+b.z*b.z+b.w*b.w);
	return (float4)
	(
		(a.x*b.x+a.y*b.y+a.z*b.z+a.w*b.w)*inv_absb2,
		(a.y*b.x-a.x*b.y-a.w*b.z+a.z*b.w)*inv_absb2,
		(a.z*b.x+a.w*b.y-a.x*b.z-a.y*b.w)*inv_absb2,
		(a.w*b.x-a.z*b.y+a.y*b.z-a.x*b.w)*inv_absb2
	);
}
float2	exp_c(float2 a)
{
	float exp_r=exp(a.x);
	return (float2)
	(
		exp_r*cos(a.y),
		exp_r*sin(a.y)
	);
}
float4	exp_q(float4 a)
{
	float exp_r=exp(a.x), abs_v=sqrt(a.y*a.y+a.z*a.z+a.w*a.w);
	float cos_v, sin_v=sincos(abs_v, &cos_v);
	float v_mul=exp_r*sin_v/mag_v;
	return (float4)
	(
		exp_r*cos_v,
		a.y*v_mul,
		a.z*v_mul,
		a.w*v_mul
	);
}
float2	log_c(float2 a)
{
	return (float2)
	(
		log(sqrt(a.x*a.x+a.y*a.y)),
		atan2(a.y, a.x),
	);
	return ret;
}
float4	log_q(float4 a)
{
	float absv2=a.y*a.y+a.z*a.z+a.w*a.w, abs_a=sqrt(a.x*a.x+absv2);
	float v_mul=acos(a.x/abs_a)*rsqrt(absv2);
	return (float4)
	(
		log(abs_a),
		a.y*v_mul,
		a.z*v_mul,
		a.w*v_mul,
	);
}
float2	pow_cr(float2 a, float b)
{
	float2 lna=log_c(a);
	float2 temp=mul_cr(lna, b);
	return exp_c(temp);
}
float2	pow_cc(float2 a, float2 b)
{
	float2 lna=log_c(a);
	float2 temp=mul_cc(lna, b);
	return exp_c(temp);
}
float4	pow_cq(float2 a, float4 b)
{
	float2 lna=log_c(a);
	float4 temp=mul_cq(lna, b);
	return exp_q(temp);
}
float4	pow_qr(float4 a, float b)
{
	float4 lna=log_q(a);
	float4 temp=mul_qr(lna, b);
	return exp_q(temp);
}
float4	pow_qc(float4 a, float2 b)
{
	float4 lna=log_q(a);
	float4 temp=mul_qc(lna, b);
	return exp_q(temp);
}
float4	pow_qq(float4 a, float4 b)
{
	float4 lna=log_q(a);
	float4 temp=mul_qq(lna, b);
	return exp_q(temp);
}

//macros for G2 functions
#define		ARG_CI(arg)		__global const int *arg
#define		ARG_F(arg)		__global float *arg
#define		ARG_CF(arg)		__global const float *arg
#define		G2_R_R(func)	__kernel void  r_r_##func(ARG_CI(size), ARG_F(rr), ARG_CF(xr))
#define		G2_C_C(func)	__kernel void  c_c_##func(ARG_CI(size), ARG_F(rr), ARG_F(ri), ARG_CF(xr), ARG_CF(xi))
#define		G2_Q_Q(func)	__kernel void  q_q_##func(ARG_CI(size), ARG_F(rr), ARG_F(ri), ARG_F(rj), ARG_F(rk), ARG_CF(xr), ARG_CF(xi), ARG_CF(xj), ARG_CF(xk))
#define		G2_R_RR(func)	__kernel void r_rr_##func(ARG_CI(size), ARG_F(rr), ARG_CF(xr), ARG_CF(yr))
#define		G2_C_RC(func)	__kernel void r_rc_##func(ARG_CI(size), ARG_F(rr), ARG_F(ri), ARG_CF(xr), ARG_CF(yr), ARG_CF(yi))
#define		G2_Q_RQ(func)	__kernel void r_rq_##func(ARG_CI(size), ARG_F(rr), ARG_F(ri), ARG_F(rj), ARG_F(rk), ARG_CF(xr), ARG_CF(yr), ARG_CF(yi), ARG_CF(yj), ARG_CF(yk))
#define		G2_C_CR(func)	__kernel void c_cr_##func(ARG_CI(size), ARG_F(rr), ARG_F(ri), ARG_CF(xr), ARG_CF(xi), ARG_CF(yr))
#define		G2_C_CC(func)	__kernel void c_cc_##func(ARG_CI(size), ARG_F(rr), ARG_F(ri), ARG_CF(xr), ARG_CF(xi), ARG_CF(yr), ARG_CF(yi))
#define		G2_Q_CQ(func)	__kernel void c_cq_##func(ARG_CI(size), ARG_F(rr), ARG_F(ri), ARG_F(rj), ARG_F(rk), ARG_CF(xr), ARG_CF(xi), ARG_CF(yr), ARG_CF(yi), ARG_CF(yj), ARG_CF(yk))
#define		G2_Q_QR(func)	__kernel void q_qr_##func(ARG_CI(size), ARG_F(rr), ARG_F(ri), ARG_F(rj), ARG_F(rk), ARG_CF(xr), ARG_CF(xi), ARG_CF(xj), ARG_CF(xk), ARG_CF(yr))
#define		G2_Q_QC(func)	__kernel void q_qc_##func(ARG_CI(size), ARG_F(rr), ARG_F(ri), ARG_F(rj), ARG_F(rk), ARG_CF(xr), ARG_CF(xi), ARG_CF(xj), ARG_CF(xk), ARG_CF(yr), ARG_CF(yi))
#define		G2_Q_QQ(func)	__kernel void q_qq_##func(ARG_CI(size), ARG_F(rr), ARG_F(ri), ARG_F(rj), ARG_F(rk), ARG_CF(xr), ARG_CF(xi), ARG_CF(xj), ARG_CF(xk), ARG_CF(yr), ARG_CF(yi), ARG_CF(yj), ARG_CF(yk))
#define		G2_C_R(func)	__kernel void  c_r_##func(ARG_CI(size), ARG_F(rr), ARG_F(ri), ARG_CF(xr))
#define		G2_C_Q(func)	__kernel void  c_q_##func(ARG_CI(size), ARG_F(rr), ARG_F(ri), ARG_CF(xr), ARG_CF(xi), ARG_CF(xj), ARG_CF(xk))
#define		G2_R_C(func)	__kernel void  r_c_##func(ARG_CI(size), ARG_F(rr), ARG_CF(xr), ARG_CF(xi))
#define		G2_R_Q(func)	__kernel void  r_q_##func(ARG_CI(size), ARG_F(rr), ARG_CF(xr), ARG_CF(xi), ARG_CF(xj), ARG_CF(xk))
#define		G2_C_RR(func)	__kernel void c_rr_##func(ARG_CI(size), ARG_F(rr), ARG_F(ri), ARG_CF(xr), ARG_CF(yr))
#define		G2_R_RC(func)	__kernel void r_rc_##func(ARG_CI(size), ARG_F(rr), ARG_CF(xr), ARG_CF(yr), ARG_CF(yi))
#define		G2_R_RQ(func)	__kernel void r_rq_##func(ARG_CI(size), ARG_F(rr), ARG_CF(xr), ARG_CF(yr), ARG_CF(yi), ARG_CF(yj), ARG_CF(yk))
#define		G2_R_CR(func)	__kernel void r_cr_##func(ARG_CI(size), ARG_F(rr), ARG_CF(xr), ARG_CF(xi), ARG_CF(yr))
#define		G2_R_CC(func)	__kernel void r_cc_##func(ARG_CI(size), ARG_F(rr), ARG_CF(xr), ARG_CF(xi), ARG_CF(yr), ARG_CF(yi))
#define		G2_R_CQ(func)	__kernel void r_cq_##func(ARG_CI(size), ARG_F(rr), ARG_CF(xr), ARG_CF(xi), ARG_CF(yr), ARG_CF(yi), ARG_CF(yj), ARG_CF(yk))
#define		G2_R_QR(func)	__kernel void r_qr_##func(ARG_CI(size), ARG_F(rr), ARG_CF(xr), ARG_CF(xi), ARG_CF(xj), ARG_CF(xk), ARG_CF(yr))
#define		G2_R_QC(func)	__kernel void r_qc_##func(ARG_CI(size), ARG_F(rr), ARG_CF(xr), ARG_CF(xi), ARG_CF(xj), ARG_CF(xk), ARG_CF(yr), ARG_CF(yi))
#define		G2_R_QQ(func)	__kernel void r_qq_##func(ARG_CI(size), ARG_F(rr), ARG_CF(xr), ARG_CF(xi), ARG_CF(xj), ARG_CF(xk), ARG_CF(yr), ARG_CF(yi), ARG_CF(yj), ARG_CF(yk))
#define		G2_C_QC(func)	__kernel void c_qc_##func(ARG_CI(size), ARG_F(rr), ARG_F(ri), ARG_CF(xr), ARG_CF(xi), ARG_CF(xj), ARG_CF(xk), ARG_CF(yr), ARG_CF(yi))

#define		DISC_R_O(func)	__kernel bool disc_r_##func##_o(ARG_CI(size), const int offset, ARG_F(or))
#define		DISC_C_O(func)	__kernel bool disc_c_##func##_o(ARG_CI(size), const int offset, ARG_F(or), ARG_F(oi))
#define		DISC_Q_O(func)	__kernel bool disc_q_##func##_o(ARG_CI(size), const int offset, ARG_F(or), ARG_F(oi), ARG_F(oj), ARG_F(ok))
#define		DISC_R_I(func)	__kernel bool disc_r_##func##_i(ARG_CI(size), const int offset, ARG_F(xr))
#define		DISC_C_I(func)	__kernel bool disc_c_##func##_i(ARG_CI(size), const int offset, ARG_F(xr), ARG_F(xi))
#define		DISC_Q_I(func)	__kernel bool disc_q_##func##_i(ARG_CI(size), const int offset, ARG_F(xr), ARG_F(xi), ARG_F(xj), ARG_F(xk))
#define		DISC_RR_I(func)	__kernel bool disc_rr_##func##_i(ARG_CI(size), const int offset, ARG_F(xr), ARG_F(yr))
#define		DISC_RC_I(func)	__kernel bool disc_rc_##func##_i(ARG_CI(size), const int offset, ARG_F(xr), ARG_F(yr), ARG_F(yi))
#define		DISC_RQ_I(func)	__kernel bool disc_rq_##func##_i(ARG_CI(size), const int offset, ARG_F(xr), ARG_F(yr), ARG_F(yi), ARG_F(yj), ARG_F(yk))
#define		DISC_CR_I(func)	__kernel bool disc_cr_##func##_i(ARG_CI(size), const int offset, ARG_F(xr), ARG_F(xi), ARG_F(yr))
#define		DISC_CC_I(func)	__kernel bool disc_cc_##func##_i(ARG_CI(size), const int offset, ARG_F(xr), ARG_F(xi), ARG_F(yr), ARG_F(yi))
#define		DISC_CQ_I(func)	__kernel bool disc_cq_##func##_i(ARG_CI(size), const int offset, ARG_F(xr), ARG_F(xi), ARG_F(yr), ARG_F(yi), ARG_F(yj), ARG_F(yk))
#define		DISC_QR_I(func)	__kernel bool disc_qr_##func##_i(ARG_CI(size), const int offset, ARG_F(xr), ARG_F(xi), ARG_F(xj), ARG_F(xk), ARG_F(yr))
#define		DISC_QC_I(func)	__kernel bool disc_qc_##func##_i(ARG_CI(size), const int offset, ARG_F(xr), ARG_F(xi), ARG_F(xj), ARG_F(xk), ARG_F(yr), ARG_F(yi))
#define		DISC_QQ_I(func)	__kernel bool disc_qq_##func##_i(ARG_CI(size), const int offset, ARG_F(xr), ARG_F(xi), ARG_F(xj), ARG_F(xk), ARG_F(yr), ARG_F(yi), ARG_F(yj), ARG_F(yk))

#define		IDX						const unsigned idx=get_idx(size)
#define		ASSIGN_R(r)				rr[idx]=r
#define		ASSIGN_C(r, i)			rr[idx]=r, ri[idx]=i
#define		ASSIGN_Q(r, i, j, k)	rr[idx]=r, ri[idx]=i, rj[idx]=j, rk[idx]=k
#define		RET_C;					ASSIGN_C(ret.x, ret.y)
#define		RET_Q					ASSIGN_Q(ret.x, ret.y, ret.z, ret.w)
#define		VEC2(name)				(float2)(name##r[idx], name##i[idx])
#define		VEC4(name)				(float4)(name##r[idx], name##i[idx], name##j[idx], name##k[idx])

//kernels pt1
G2_R_R(setzero){IDX; ASSIGN_R(0);}
G2_C_C(setzero){IDX; ASSIGN_C(0, 0);}
G2_Q_Q(setzero){IDX; ASSIGN_Q(0, 0, 0, 0);}

G2_R_R(ceil){IDX; ASSIGN_R(ceil(xr[idx]));}
G2_C_C(ceil){IDX; ASSIGN_C(ceil(xr[idx]), ceil(xi[idx]));}
G2_Q_Q(ceil){IDX; ASSIGN_Q(ceil(xr[idx]), ceil(xi[idx]), ceil(xj[idx]), ceil(xk[idx]));}
DISC_R_O(ceil)
{
	IDX;
	return or[idx]!=or[idx+offset];
}
DISC_C_O(ceil)
{
	IDX;
	return or[idx]!=or[idx+offset]||oi[idx]!=oi[idx+offset];
}
DISC_Q_O(ceil)
{
	IDX;
	return or[idx]!=or[idx+offset]||oi[idx]!=oi[idx+offset]||oj[idx]!=oj[idx+offset]||ok[idx]!=ok[idx+offset];
}

G2_R_R(floor){IDX; ASSIGN_R(floor(xr[idx]));}
G2_C_C(floor){IDX; ASSIGN_C(floor(xr[idx]), floor(xi[idx]));}
G2_Q_Q(floor){IDX; ASSIGN_Q(floor(xr[idx]), floor(xi[idx]), floor(xj[idx]), floor(xk[idx]));}
DISC_R_O(floor){return disc_r_ceil_o(size, offset, or);}
DISC_C_O(floor){return disc_c_ceil_o(size, offset, or, oi);}
DISC_Q_O(floor){return disc_q_ceil_o(size, offset, or, oi, oj, ok);}

G2_R_R(round){IDX; ASSIGN_R(round(xr[idx]));}
G2_C_C(round){IDX; ASSIGN_C(round(xr[idx]), round(xi[idx]));}
G2_Q_Q(round)
{
	IDX;
	ASSIGN_Q(round(xr[idx]), round(xi[idx]), round(xj[idx]), round(xk[idx]));
}
DISC_R_O(round){return disc_r_ceil_o(size, offset, or);}
DISC_C_O(round){return disc_c_ceil_o(size, offset, or, oi);}
DISC_Q_O(round){return disc_q_ceil_o(size, offset, or, oi, oj, ok);}

G2_R_R(int){IDX; ASSIGN_R((int)xr[idx]);}
G2_C_C(int){IDX; ASSIGN_C((int)xr[idx], (int)xi[idx]);}
G2_Q_Q(int){IDX; ASSIGN_Q((int)xr[idx], (int)xi[idx], (int)xj[idx], (int)xk[idx]);}
DISC_R_O(int){return disc_r_ceil_o(size, offset, or);}
DISC_C_O(int){return disc_c_ceil_o(size, offset, or, oi);}
DISC_Q_O(int){return disc_q_ceil_o(size, offset, or, oi, oj, ok);}

G2_R_R(frac){IDX; ASSIGN_R(frac(xr[idx]));}
G2_C_C(frac){IDX; ASSIGN_C(frac(xr[idx]), frac(xi[idx]));}
G2_Q_Q(frac){IDX; ASSIGN_Q(frac(xr[idx]), frac(xi[idx]), frac(xj[idx]), frac(xk[idx]));}
DISC_R_I(frac)
{
	IDX;
	return floor(xr[idx])!=floor(xr[idx+offset]);
}
DISC_C_I(frac)
{
	IDX;
	return floor(xr[idx])!=floor(xr[idx+offset])||floor(xi[idx])!=floor(xi[idx+offset]);
}
DISC_Q_I(frac)
{
	IDX;
	return floor(xr[idx])!=floor(xr[idx+offset])||floor(xi[idx])!=floor(xi[idx+offset])||floor(xj[idx])!=floor(xj[idx+offset])||floor(xk[idx])!=floor(xk[idx+offset]);
}

G2_R_R(abs)
{
	IDX;
	rr[idx]=abs(xr[idx]);
}
G2_R_C(abs)
{
	IDX;
	float2 a=VEC2(x);
	rr[idx]=sqrt(a.x*a.x+a.y*a.y);
}
G2_R_Q(abs)
{
	IDX;
	float4 a=VEC4(x);
	rr[idx]=sqrt(a.x*a.x+a.y*a.y+a.z*a.z+a.w*a.w);
}

G2_R_R(arg)
{
	IDX;
	const float _pi=acos(-1.f);
	if(xr[idx]<0)
		rr[idx]=_pi;
	else if(xr[idx]==0)
		rr[idx]=nan(0);
	else
		rr[idx]=0;
}
G2_R_C(arg){IDX; rr[idx]=atan2(xi[idx], xr[idx]);}
G2_R_Q(arg)
{
	IDX;
	float4 a=VEC4(x);
	float abs_a=a.x*a.x+a.y*a.y+a.z*a.z+a.w*a.w;
	if(!abs_a)
		rr[idx]=nan(0);
	else
	{
		abs_a=sqrt(abs_a);
		rr[idx]=acos(a.x/abs_a);
	}
}
DISC_R_I(arg)
{
	IDX;
	if(xr[idx]<0)
		return xr[idx+offset]>=0;
	if(xr[idx]>0)
		return xr[idx+offset]<=0;
	return xr[idx+offset]!=0;
}
DISC_C_I(arg){return false;}//TODO
DISC_Q_I(arg){return false;}//TODO

G2_R_C(real){IDX; rr[idx]=xr[idx];}

G2_R_C(imag){IDX; rr[idx]=xi[idx];}

G2_C_C(conjugate){IDX; ASSIGN_C(xr[idx], -xi[idx]);}
G2_Q_Q(conjugate){IDX; ASSIGN_Q(xr[idx], -xi[idx], -xj[idx], -xk[idx]);}

G2_C_R(polar)
{
	IDX;
	float a=xr[idx];
	float2 ret=(float2)(abs(a), a<0?_pi:a==0?nan(0):0);
	RET_C;
}
G2_C_C(polar)
{
	IDX;
	float2 a=VEC2(x);
	ASSIGN_C(sqrt(a.x*a.x+a.y*a.y), atan2(a.y, a.x));
}
G2_C_Q(polar)
{
	IDX;
	float4 a=VEC4(x);
	rr[idx]=sqrt(a.x*a.x+a.y*a.y+a.z*a.z+a.w*a.w);
	ri[idx]=acos(a.x/rr[idx]);
}
DISC_R_I(polar)
{
	IDX;
	if(xr[idx]<0)
		return xr[idx+offset]>=0;
	if(xr[idx]>0)
		return xr[idx+offset]<=0;
	return xr[idx+offset]!=0;
}
DISC_C_I(polar){return false;}//TODO
DISC_Q_I(polar){return false;}//TODO

G2_C_C(cartesian)
{
	IDX;
	float2 a=VEC2(x);
	ASSIGN_C(a.x*cos(a.y), a.x*sin(a.y));
}
G2_Q_Q(cartesian)
{
	IDX;
	float r=xr[idx], i=xi[idx], j=xj[idx], k=xk[idx];
	float cos_j=cos(j), r_cos_k=r*cos(k);
	rr[idx]=cos(i)*cos_j*r_cos_k;
	ri[idx]=sin(i)*cos_j*r_cos_k;
	rj[idx]=sin(j)*r_cos_k;
	rk[idx]=r*sin(k);
}

G2_R_RR(plus){IDX; ASSIGN_R(xr[idx]+yr[idx]);}
G2_C_RC(plus){IDX; ASSIGN_C(xr[idx]+yr[idx], yi[idx]);}
G2_Q_RQ(plus){IDX; ASSIGN_Q(xr[idx]+yr[idx], yi[idx], yj[idx], yk[idx]);}
G2_C_CR(plus){IDX; ASSIGN_C(xr[idx]+yr[idx], xi[idx]);}
G2_C_CC(plus){IDX; ASSIGN_C(xr[idx]+yr[idx], xi[idx]+yi[idx]);}
G2_Q_CQ(plus){IDX; ASSIGN_Q(xr[idx]+yr[idx], xi[idx]+yi[idx], yj[idx], yk[idx]);}
G2_Q_QR(plus){IDX; ASSIGN_Q(xr[idx]+yr[idx], xi[idx], xj[idx], xk[idx]);}
G2_Q_QC(plus){IDX; ASSIGN_Q(xr[idx]+yr[idx], xi[idx]+yi[idx], xj[idx], xk[idx]);}
G2_Q_QQ(plus){IDX; ASSIGN_Q(xr[idx]+yr[idx], xi[idx]+yi[idx], xj[idx]+yj[idx], xk[idx]+yk[idx]);}

G2_R_R(minus){IDX; ASSIGN_R(-xr[idx]);}
G2_C_C(minus){IDX; ASSIGN_C(-xr[idx], -xi[idx]);}
G2_Q_Q(minus){IDX; ASSIGN_Q(-xr[idx], -xi[idx], -xj[idx], -xk[idx]);}
G2_R_RR(minus){IDX; ASSIGN_R(xr[idx]-yr[idx]);}
G2_C_RC(minus){IDX; ASSIGN_C(xr[idx]-yr[idx], -yi[idx]);}
G2_Q_RQ(minus){IDX; ASSIGN_Q(xr[idx]-yr[idx], -yi[idx], -yj[idx], -yk[idx]);}
G2_C_CR(minus){IDX; ASSIGN_C(xr[idx]-yr[idx], xi[idx]);}
G2_C_CC(minus){IDX; ASSIGN_C(xr[idx]-yr[idx], xi[idx]-yi[idx]);}
G2_Q_CQ(minus){IDX; ASSIGN_Q(xr[idx]-yr[idx], xi[idx]-yi[idx], -yj[idx], -yk[idx]);}
G2_Q_QR(minus){IDX; ASSIGN_Q(xr[idx]-yr[idx], xi[idx], xj[idx], xk[idx]);}
G2_Q_QC(minus){IDX; ASSIGN_Q(xr[idx]-yr[idx], xi[idx]-yi[idx], xj[idx], xk[idx]);}
G2_Q_QQ(minus){IDX; ASSIGN_Q(xr[idx]-yr[idx], xi[idx]-yi[idx], xj[idx]-yj[idx], xk[idx]-yk[idx]);}

G2_R_RR(multiply){IDX; ASSIGN_R(xr[idx]*yr[idx]);}
G2_C_RC(multiply)
{
	IDX;
	float2 ret=mul_rc(xr[idx], VEC2(y));
	RET_C;
}
G2_Q_RQ(multiply)
{
	IDX;
	float4 ret=mul_rq(xr[idx], VEC4(y));
	RET_Q;
}
G2_C_CR(multiply)
{
	IDX;
	float2 ret=mul_cr(VEC2(x), yr[idx]);
	RET_C;
}
G2_C_CC(multiply)
{
	IDX;
	float2 ret=mul_cc(VEC2(x), VEC2(y));
	RET_C;
}
G2_Q_CQ(multiply)
{
	IDX;
	float4 ret=mul_cq(VEC2(x), VEC4(y));
	RET_Q;
}
G2_Q_QR(multiply)
{
	IDX;
	float4 ret=mul_qr(VEC4(x), yr[idx]);
	RET_Q;
}
G2_Q_QC(multiply)
{
	IDX;
	float4 ret=mul_qc(VEC4(x), VEC2(y));
	RET_Q;
}
G2_Q_QQ(multiply)
{
	IDX;
	float4 ret=mul_qq(VEC4(x), VEC4(y));
	RET_Q;
}

G2_R_R(divide)
{
	IDX;
	rr[idx]=1/xr[idx];
}
G2_C_C(divide)
{
	IDX;
	float2 ret=inv_c(VEC2(x));
	RET_C;
}
G2_Q_Q(divide)
{
	IDX;
	float4 ret=inv_q(VEC4(x));
	RET_Q;
}
G2_R_RR(divide)
{
	IDX;
	ASSIGN_R(xr[idx]/yr[idx]);
}
G2_C_RC(divide)
{
	IDX;
	float2 ret=div_rc(xr[idx], VEC2(y));
	RET_C;
}
G2_Q_RQ(divide)
{
	IDX;
	float4 ret=div_rq(xr[idx], VEC2(y));
	RET_Q;
}
G2_C_CR(divide)
{
	IDX;
	float2 ret=div_cr(VEC2(x), yr[idx]);
	RET_C;
}
G2_C_CC(divide)
{
	IDX;
	float2 ret=div_cc(VEC2(x), VEC2(y));
	RET_C;
}
G2_Q_CQ(divide)
{
	IDX;
	float4 ret=div_cq(VEC2(x), VEC4(y));
	RET_Q;
}
G2_Q_QR(divide)
{
	IDX;
	float4 ret=div_qr(VEC4(x), yr[idx]);
	RET_Q;
}
G2_Q_QC(divide)
{
	IDX;
	float4 ret=div_qc(VEC4(x), VEC2(y));
	RET_Q;
}
G2_Q_QQ(divide)
{
	IDX;
	float4 ret=div_qq(VEC4(x), VEC4(y));
	RET_Q;
}
DISC_R_I(divide)
{
	IDX;
	float x0r=xr[idx], x1r=xr[idx+offset];
	return x0r<0?x1r>=0:x0r>0?x1r<=0:x1r!=0;
}
DISC_C_I(divide)
{
	IDX;
	float
		x0r=xr[idx], x1r=xr[idx+offset],
		x0i=xi[idx], x1i=xi[idx+offset];
	return false;//TODO
}
DISC_Q_I(divide)
{
	IDX;
	float
		x0r=xr[idx], x1r=xr[idx+offset],
		x0i=xi[idx], x1i=xi[idx+offset],
		x0j=xj[idx], x1j=xj[idx+offset],
		x0k=xk[idx], x1k=xk[idx+offset];
	return false;//TODO
}
DISC_RR_I(divide)
{
	IDX;
	float y0r=yr[idx], y1r=yr[idx+offset];
	return y0r<0?y1r>=0:y0r>0?y1r<=0:y1r!=0;
}
DISC_RC_I(divide)
{
	IDX;
	float
		y0r=yr[idx], y1r=yr[idx+offset],
		y0i=yi[idx], y1i=yi[idx+offset];
	return false;//TODO
}
DISC_RQ_I(divide)
{
	IDX;
	float
		y0r=yr[idx], y1r=yr[idx+offset],
		y0i=yi[idx], y1i=yi[idx+offset],
		y0j=yj[idx], y1j=yj[idx+offset],
		y0k=yk[idx], y1k=yk[idx+offset];
	return false;//TODO
}
DISC_CR_I(divide)
{
	IDX;
	float y0r=yr[idx], y1r=yr[idx+offset];
	return y0r<0?y1r>=0:y0r>0?y1r<=0:y1r!=0;
}
DISC_CC_I(divide)
{
	IDX;
	float y0r=yr[idx], y1r=yr[idx+offset];
	return false;//TODO
}
DISC_CQ_I(divide)
{
	IDX;
	float y0r=yr[idx], y1r=yr[idx+offset];
	return false;//TODO
}
DISC_QR_I(divide)
{
	IDX;
	float y0r=yr[idx], y1r=yr[idx+offset];
	return y0r<0?y1r>=0:y0r>0?y1r<=0:y1r!=0;
}
DISC_QC_I(divide)
{
	IDX;
	float
		y0r=yr[idx], y1r=yr[idx+offset],
		y0i=yi[idx], y1i=yi[idx+offset];
	return false;//TODO
}
DISC_QQ_I(divide)
{
	IDX;
	float
		y0r=yr[idx], y1r=yr[idx+offset],
		y0i=yi[idx], y1i=yi[idx+offset],
		y0j=yj[idx], y1j=yj[idx+offset],
		y0k=yk[idx], y1k=yk[idx+offset];
	return false;//TODO
}
)CLSRC";

	static const char program_src_pt2[]=R"CLSRC(
//auxiliary constants
enum Constants
{
	C_PI, C_2PI, C_SQRT2, C_INV_LN10
};
__global float *constants;
#define	_pi			(constants[C_PI])
#define	_2pi		(constants[C_2PI])
#define	_sqrt2		(constants[C_SQRT2])
#define	_inv_ln10	(constants[C_INV_LN10])

//declarations for pt2
int get_idx(__global const int *size);
void	equal_cc(float2 a, float2 b);
void	equal_qq(float4 a, float4 b);
float2	floor_c(float2 a);
float4	floor_q(float4 a);
float2	mul_rc(float a, float2 b);
float4	mul_rq(float a, float4 b);
float2	mul_cr(float2 a, float b);
float2	mul_cc(float2 a, float2 b);
float4	mul_cq(float2 a, float4 b);
float4	mul_qr(float4 a, float b);
float4	mul_qc(float4 a, float2 b);
float4	mul_qq(float4 a, float4 b);
float2	inv_c(float2 a);
float4	inv_q(float4 a);
float2	div_rc(float a, float2 b);
float4	div_rq(float a, float4 b);
float2	div_cr(float2 a, float b);
float2	div_cc(float2 a, float2 b);
float4	div_cq(float2 a, float4 b);
float4	div_qr(float4 a, float b);
float4	div_qc(float4 a, float2 b);
float4	div_qq(float4 a, float4 b);
float2	exp_c(float2 a);
float4	exp_q(float4 a);
float2	log_c(float2 a);
float4	log_q(float4 a);
float2	pow_cr(float2 a, float b);
float2	pow_cc(float2 a, float2 b);
float4	pow_cq(float2 a, float4 b);
float4	pow_qr(float4 a, float b);
float4	pow_qc(float4 a, float2 b);
float4	pow_qq(float4 a, float4 b);

//macros for G2 functions
#define		ARG_CI(arg)		__global const int *arg
#define		ARG_F(arg)		__global float *arg
#define		ARG_CF(arg)		__global const float *arg
#define		G2_R_R(func)	__kernel void  r_r_##func(ARG_CI(size), ARG_F(rr), ARG_CF(xr))
#define		G2_C_C(func)	__kernel void  c_c_##func(ARG_CI(size), ARG_F(rr), ARG_F(ri), ARG_CF(xr), ARG_CF(xi))
#define		G2_Q_Q(func)	__kernel void  q_q_##func(ARG_CI(size), ARG_F(rr), ARG_F(ri), ARG_F(rj), ARG_F(rk), ARG_CF(xr), ARG_CF(xi), ARG_CF(xj), ARG_CF(xk))
#define		G2_R_RR(func)	__kernel void r_rr_##func(ARG_CI(size), ARG_F(rr), ARG_CF(xr), ARG_CF(yr))
#define		G2_C_RC(func)	__kernel void r_rc_##func(ARG_CI(size), ARG_F(rr), ARG_F(ri), ARG_CF(xr), ARG_CF(yr), ARG_CF(yi))
#define		G2_Q_RQ(func)	__kernel void r_rq_##func(ARG_CI(size), ARG_F(rr), ARG_F(ri), ARG_F(rj), ARG_F(rk), ARG_CF(xr), ARG_CF(yr), ARG_CF(yi), ARG_CF(yj), ARG_CF(yk))
#define		G2_C_CR(func)	__kernel void c_cr_##func(ARG_CI(size), ARG_F(rr), ARG_F(ri), ARG_CF(xr), ARG_CF(xi), ARG_CF(yr))
#define		G2_C_CC(func)	__kernel void c_cc_##func(ARG_CI(size), ARG_F(rr), ARG_F(ri), ARG_CF(xr), ARG_CF(xi), ARG_CF(yr), ARG_CF(yi))
#define		G2_Q_CQ(func)	__kernel void c_cq_##func(ARG_CI(size), ARG_F(rr), ARG_F(ri), ARG_F(rj), ARG_F(rk), ARG_CF(xr), ARG_CF(xi), ARG_CF(yr), ARG_CF(yi), ARG_CF(yj), ARG_CF(yk))
#define		G2_Q_QR(func)	__kernel void q_qr_##func(ARG_CI(size), ARG_F(rr), ARG_F(ri), ARG_F(rj), ARG_F(rk), ARG_CF(xr), ARG_CF(xi), ARG_CF(xj), ARG_CF(xk), ARG_CF(yr))
#define		G2_Q_QC(func)	__kernel void q_qc_##func(ARG_CI(size), ARG_F(rr), ARG_F(ri), ARG_F(rj), ARG_F(rk), ARG_CF(xr), ARG_CF(xi), ARG_CF(xj), ARG_CF(xk), ARG_CF(yr), ARG_CF(yi))
#define		G2_Q_QQ(func)	__kernel void q_qq_##func(ARG_CI(size), ARG_F(rr), ARG_F(ri), ARG_F(rj), ARG_F(rk), ARG_CF(xr), ARG_CF(xi), ARG_CF(xj), ARG_CF(xk), ARG_CF(yr), ARG_CF(yi), ARG_CF(yj), ARG_CF(yk))
#define		G2_C_R(func)	__kernel void  c_r_##func(ARG_CI(size), ARG_F(rr), ARG_F(ri), ARG_CF(xr))
#define		G2_C_Q(func)	__kernel void  c_q_##func(ARG_CI(size), ARG_F(rr), ARG_F(ri), ARG_CF(xr), ARG_CF(xi), ARG_CF(xj), ARG_CF(xk))
#define		G2_R_C(func)	__kernel void  r_c_##func(ARG_CI(size), ARG_F(rr), ARG_CF(xr), ARG_CF(xi))
#define		G2_R_Q(func)	__kernel void  r_q_##func(ARG_CI(size), ARG_F(rr), ARG_CF(xr), ARG_CF(xi), ARG_CF(xj), ARG_CF(xk))
#define		G2_C_RR(func)	__kernel void c_rr_##func(ARG_CI(size), ARG_F(rr), ARG_F(ri), ARG_CF(xr), ARG_CF(yr))
#define		G2_R_RC(func)	__kernel void r_rc_##func(ARG_CI(size), ARG_F(rr), ARG_CF(xr), ARG_CF(yr), ARG_CF(yi))
#define		G2_R_RQ(func)	__kernel void r_rq_##func(ARG_CI(size), ARG_F(rr), ARG_CF(xr), ARG_CF(yr), ARG_CF(yi), ARG_CF(yj), ARG_CF(yk))
#define		G2_R_CR(func)	__kernel void r_cr_##func(ARG_CI(size), ARG_F(rr), ARG_CF(xr), ARG_CF(xi), ARG_CF(yr))
#define		G2_R_CC(func)	__kernel void r_cc_##func(ARG_CI(size), ARG_F(rr), ARG_CF(xr), ARG_CF(xi), ARG_CF(yr), ARG_CF(yi))
#define		G2_R_CQ(func)	__kernel void r_cq_##func(ARG_CI(size), ARG_F(rr), ARG_CF(xr), ARG_CF(xi), ARG_CF(yr), ARG_CF(yi), ARG_CF(yj), ARG_CF(yk))
#define		G2_R_QR(func)	__kernel void r_qr_##func(ARG_CI(size), ARG_F(rr), ARG_CF(xr), ARG_CF(xi), ARG_CF(xj), ARG_CF(xk), ARG_CF(yr))
#define		G2_R_QC(func)	__kernel void r_qc_##func(ARG_CI(size), ARG_F(rr), ARG_CF(xr), ARG_CF(xi), ARG_CF(xj), ARG_CF(xk), ARG_CF(yr), ARG_CF(yi))
#define		G2_R_QQ(func)	__kernel void r_qq_##func(ARG_CI(size), ARG_F(rr), ARG_CF(xr), ARG_CF(xi), ARG_CF(xj), ARG_CF(xk), ARG_CF(yr), ARG_CF(yi), ARG_CF(yj), ARG_CF(yk))
#define		G2_C_QC(func)	__kernel void c_qc_##func(ARG_CI(size), ARG_F(rr), ARG_F(ri), ARG_CF(xr), ARG_CF(xi), ARG_CF(xj), ARG_CF(xk), ARG_CF(yr), ARG_CF(yi))

#define		DISC_R_O(func)	__kernel bool disc_r_##func##_o(ARG_CI(size), const int offset, ARG_F(or))
#define		DISC_C_O(func)	__kernel bool disc_c_##func##_o(ARG_CI(size), const int offset, ARG_F(or), ARG_F(oi))
#define		DISC_Q_O(func)	__kernel bool disc_q_##func##_o(ARG_CI(size), const int offset, ARG_F(or), ARG_F(oi), ARG_F(oj), ARG_F(ok))
#define		DISC_R_I(func)	__kernel bool disc_r_##func##_i(ARG_CI(size), const int offset, ARG_F(xr))
#define		DISC_C_I(func)	__kernel bool disc_c_##func##_i(ARG_CI(size), const int offset, ARG_F(xr), ARG_F(xi))
#define		DISC_Q_I(func)	__kernel bool disc_q_##func##_i(ARG_CI(size), const int offset, ARG_F(xr), ARG_F(xi), ARG_F(xj), ARG_F(xk))
#define		DISC_RR_I(func)	__kernel bool disc_rr_##func##_i(ARG_CI(size), const int offset, ARG_F(xr), ARG_F(yr))
#define		DISC_RC_I(func)	__kernel bool disc_rc_##func##_i(ARG_CI(size), const int offset, ARG_F(xr), ARG_F(yr), ARG_F(yi))
#define		DISC_RQ_I(func)	__kernel bool disc_rq_##func##_i(ARG_CI(size), const int offset, ARG_F(xr), ARG_F(yr), ARG_F(yi), ARG_F(yj), ARG_F(yk))
#define		DISC_CR_I(func)	__kernel bool disc_cr_##func##_i(ARG_CI(size), const int offset, ARG_F(xr), ARG_F(xi), ARG_F(yr))
#define		DISC_CC_I(func)	__kernel bool disc_cc_##func##_i(ARG_CI(size), const int offset, ARG_F(xr), ARG_F(xi), ARG_F(yr), ARG_F(yi))
#define		DISC_CQ_I(func)	__kernel bool disc_cq_##func##_i(ARG_CI(size), const int offset, ARG_F(xr), ARG_F(xi), ARG_F(yr), ARG_F(yi), ARG_F(yj), ARG_F(yk))
#define		DISC_QR_I(func)	__kernel bool disc_qr_##func##_i(ARG_CI(size), const int offset, ARG_F(xr), ARG_F(xi), ARG_F(xj), ARG_F(xk), ARG_F(yr))
#define		DISC_QC_I(func)	__kernel bool disc_qc_##func##_i(ARG_CI(size), const int offset, ARG_F(xr), ARG_F(xi), ARG_F(xj), ARG_F(xk), ARG_F(yr), ARG_F(yi))
#define		DISC_QQ_I(func)	__kernel bool disc_qq_##func##_i(ARG_CI(size), const int offset, ARG_F(xr), ARG_F(xi), ARG_F(xj), ARG_F(xk), ARG_F(yr), ARG_F(yi), ARG_F(yj), ARG_F(yk))

#define		IDX						const unsigned idx=get_idx(size)
#define		ASSIGN_R(r)				rr[idx]=r
#define		ASSIGN_C(r, i)			rr[idx]=r, ri[idx]=i
#define		ASSIGN_Q(r, i, j, k)	rr[idx]=r, ri[idx]=i, rj[idx]=j, rk[idx]=k
#define		RET_C;					ASSIGN_C(ret.x, ret.y)
#define		RET_Q					ASSIGN_Q(ret.x, ret.y, ret.z, ret.w)

//kernels pt2
G2_R_RR(logic_divides)
{
	IDX;
	float q=xr[idx]/yr[idx];
	rr[idx]=q==floor(q);
}
G2_R_RC(logic_divides)
{
	IDX;
	float2 q=div_rc(xr[idx], VEC2(y));
	float2 fq=floor_c(q);
	rr[idx]=equal_cc(q, fq);
}
G2_R_RQ(logic_divides)
{
	IDX;
	float4 q=div_rq(xr[idx], VEC4(y));
	float4 fq=floor_q(q);
	rr[idx]=equal_qq(q, fq);
}
G2_R_CR(logic_divides)
{
	IDX;
	float2 q=div_cr(VEC2(x), yr[idx]);
	float2 fq=floor_c(q);
	rr[idx]=equal_cc(q, fq);
}
G2_R_CC(logic_divides)
{
	IDX;
	float2 q=div_cc(VEC2(x), VEC2(y));
	float2 fq=floor_c(q);
	rr[idx]=equal_cc(q, fq);
}
G2_R_CQ(logic_divides)
{
	IDX;
	float4 q=div_cq(VEC2(x), VEC4(y));
	float4 fq=floor_q(q);
	rr[idx]=equal_qq(q, fq);
}
G2_R_QR(logic_divides)
{
	IDX;
	float4 q=div_qr(VEC4(x), yr[idx]);
	float4 fq=floor_q(q);
	rr[idx]=equal_qq(q, fq);
}
G2_R_QC(logic_divides)
{
	IDX;
	float4 q=div_qc(VEC4(x), VEC2(y));
	float4 fq=floor_q(q);
	rr[idx]=equal_qq(q, fq);
}
G2_R_QQ(logic_divides)
{
	IDX;
	float4 q=div_qq(VEC4(x), VEC4(y));
	float4 fq=floor_q(q);
	rr[idx]=equal_qq(q, fq);
}
DISC_R_O(logic_divides)//for all logic_divides functions
{
	IDX;
	return or[idx]!=or[idx+offset];
}

G2_R_RR(power_real)
{
	IDX;
	float a=xr[idx];
	int b=(int)yr[idx];
	float m[]={1, 0}, r=1;//may not compile
	if(b<0)
		m[1]=1/a, b=-b;
	else
		m[1]=a;
	for(;;)
	{
		r*=m[b&1], b>>=1;
		if(!b)
			break;
		m[1]*=m[1];
	}
	rr[idx]=r;
}
G2_R_CR(power_real)
{
	IDX;
	float2 a=VEC2(x), r=(float2)(1, 0);
	int b=(int)yr[idx], p=abs(b);
	for(;;)
	{
		if(p&1)
			r=mul_cc(r, a);
		p>>=1;
		if(!p)
			break;
		a=mul_cc(a, a);
	}
	if(b<0)
		r=inv_c(r);
	rr[idx]=r.x, ri[idx]=r.y;
}
G2_R_QR(power_real)
{
	IDX;
	float4 a=VEC4(x), r=(float4)(1, 0, 0, 0);
	int b=(int)yr[idx], p=abs(b);
	for(;;)
	{
		if(p&1)
			r=mul_qq(r, a);
		p>>=1;
		if(!p)
			break;
		a=mul_qq(a, a);
	}
	if(b<0)
		r=inv_q(r);
	rr[idx]=r.x, ri[idx]=r.y, ri[idx]=r.z, ri[idx]=r.w;
}
DISC_RR_I(power_real)
{
	IDX;
	return floor(yr[idx])!=floor(yr[idx+offset]);
}
DISC_CR_I(power_real)
{
	IDX;
	return floor(yr[idx])!=floor(yr[idx+offset]);
}
DISC_QR_I(power_real)
{
	IDX;
	return floor(yr[idx])!=floor(yr[idx+offset]);
}

DISC_C_CR(pow)
{
	IDX;
	float2 a=VEC2(x);
	float b=yr[idx];
	if(!a.x&&!a.y && !b)
		ASSIGN_C(1, 0);
	else
	{
		float2 ret=pow_cr(a, b);
		RET_C;
	}
}
DISC_C_CC(pow)
{
	IDX;
	float2 a=VEC2(x);
	float2 b=VEC2(y);
	if(!a.x&&!a.y && !b.x&&!b.y)
		ASSIGN_C(1, 0);
	else
	{
		float2 ret=pow_cc(a, b);
		RET_C;
	}
}
DISC_Q_CQ(pow)
{
	IDX;
	float2 a=VEC2(x);
	float4 b=VEC4(y);
	if(!a.x&&!a.y && !b.x&&!b.y&&!b.z&&!b.w)
		ASSIGN_Q(1, 0, 0, 0);
	else
	{
		float4 ret=pow_cq(a, b);
		RET_Q;
	}
}
DISC_Q_QR(pow)
{
	IDX;
	float4 a=VEC4(x);
	float b=yr[idx];
	if(!a.x&&!a.y&&!a.z&&!a.w && !b)
		ASSIGN_Q(1, 0, 0, 0);
	else
	{
		float4 ret=pow_qr(a, b);
		RET_Q;
	}
}
DISC_Q_QC(pow)
{
	IDX;
	float4 a=VEC4(x);
	float2 b=VEC2(y);
	if(!a.x&&!a.y&&!a.z&&!a.w && !b.x&&!b.y)
		ASSIGN_Q(1, 0, 0, 0);
	else
	{
		float4 ret=pow_qc(a, b);
		RET_Q;
	}
}
DISC_Q_QQ(pow)
{
	IDX;
	float4 a=VEC4(x);
	float4 b=VEC4(y);
	if(!a.x&&!a.y&&!a.z&&!a.w && !b.x&&!b.y&&!b.z&&!b.w)
		ASSIGN_Q(1, 0, 0, 0);
	else
	{
		float4 ret=pow_qq(a, b);
		RET_Q;
	}
}
DISC_CR_I(pow){return false;}//TODO
DISC_CC_I(pow){return false;}//TODO
DISC_CQ_I(pow){return false;}//TODO
DISC_QR_I(pow){return false;}//TODO
DISC_QC_I(pow){return false;}//TODO
DISC_QQ_I(pow){return false;}//TODO

G2_C_C(ln)
{
	IDX;
	float2 ret=log_c(VEC2(x));
	RET_C;
}
G2_Q_Q(ln)
{
	IDX;
	float4 ret=log_q(VEC4(x));
	RET_Q;
}
DISC_C_I(ln){return disc_c_arg_i(size, offset, xr, xi);}
DISC_Q_I(ln){return disc_q_arg_i(size, offset, xr, xi, xj, xk);}

G2_C_C(log)
{
	IDX;
	float2 log_a=log_c(VEC2(x));
	float2 ret=mul_cr(log_a, _inv_ln10);
	RET_C;
}
G2_Q_Q(log)
{
	IDX;
	float4 log_a=log_q(VEC4(x));
	float4 ret=mul_qr(log_a, _inv_ln10);
	RET_Q;
}
G2_C_CR(log)
{
	IDX;
	float2 log_a=log_c(VEC2(x));
	float2 log_b=log_c((float2)(yr[idx], 0));
	float2 ret=div_cr(log_a, log_b);
	RET_C;
}
G2_C_CC(log)
{
	IDX;
	float2 log_a=log_c(VEC2(x));
	float2 log_b=log_c(VEC2(y));
	float2 ret=div_cc(log_a, log_b);
	RET_C;
}
G2_C_CQ(log)
{
	IDX;
	float2 log_a=log_c(VEC2(x));
	float4 log_b=log_q(VEC4(y));
	float4 ret=div_cq(log_a, log_b);
	RET_Q;
}
G2_C_QC(log)
{
	IDX;
	float4 log_a=log_q(VEC4(x));
	float2 log_b=log_c(VEC2(y));
	float4 ret=div_qc(log_a, log_b);
	RET_Q;
}
G2_C_QQ(log)
{
	IDX;
	float4 log_a=log_q(VEC4(x));
	float4 log_b=log_q(VEC4(y));
	float4 ret=div_qc(log_a, log_b);
	RET_Q;
}
DISC_C_I(log){return disc_c_arg_i(size, offset, xr, xi);}
DISC_Q_I(log){return false;}//TODO
DISC_CR_I(log){return false;}//TODO
DISC_CC_I(log){return false;}//TODO
DISC_CQ_I(log){return false;}//TODO
DISC_QC_I(log){return false;}//TODO
DISC_QQ_I(log){return false;}//TODO

//tetrate

//pentate

G2_R_R(bitwise_shift_left_l)
{
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
		{R_RQ_LOGIC_DIVIDES, CL_R_RQ, DISC_OUTPUT, "r_rq_logic_divides", "disc_r_logic_divides_o"},
		{R_CR_LOGIC_DIVIDES, CL_R_CR, DISC_OUTPUT, "r_cr_logic_divides", "disc_r_logic_divides_o"},
		{R_CC_LOGIC_DIVIDES, CL_R_CC, DISC_OUTPUT, "r_cc_logic_divides", "disc_r_logic_divides_o"},
		{R_CQ_LOGIC_DIVIDES, CL_R_CQ, DISC_OUTPUT, "r_cq_logic_divides", "disc_r_logic_divides_o"},
		{R_QR_LOGIC_DIVIDES, CL_R_QR, DISC_OUTPUT, "r_qr_logic_divides", "disc_r_logic_divides_o"},
		{R_QC_LOGIC_DIVIDES, CL_R_QC, DISC_OUTPUT, "r_qc_logic_divides", "disc_r_logic_divides_o"},
		{R_QQ_LOGIC_DIVIDES, CL_R_QQ, DISC_OUTPUT, "r_qq_logic_divides", "disc_r_logic_divides_o"},

		{R_RR_LOGIC_DIVIDES, CL_R_RR, DISC_INPUT, "r_rr_power_real", "disc_rr_power_real_i"},
		{R_CR_LOGIC_DIVIDES, CL_R_CR, DISC_INPUT, "c_cr_power_real", "disc_cr_power_real_i"},
		{R_QR_LOGIC_DIVIDES, CL_R_QR, DISC_INPUT, "q_qr_power_real", "disc_qr_power_real_i"},
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