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

#define			CL_TARGET_OPENCL_VERSION 120
#include		<CL/opencl.h>
#include		<EGL/egl.h>
#include		<dlfcn.h>
#include		<string>
#include		<vector>
#include		"g2_common.h"
#include		"g2_cl.h"
#include		"GLESAPI.h"

	#define		DEBUG2

const int		g_buf_size=1048576;
static char		g_buf[g_buf_size]={0};//
int				OCL_state=CL_NOTHING;
bool			cl_gl_interop=false;
const char*		clerr2str(int error)
{
#define 			EC(x)		case x:a=(const char*)#x;break
#define 			EC2(n, x)	case n:a=(const char*)#x;break
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
//#ifdef CL_VERSION_1_1
	EC(CL_MISALIGNED_SUB_BUFFER_OFFSET);
	EC(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST);
//#endif
//#ifdef CL_VERSION_1_2
	EC(CL_COMPILE_PROGRAM_FAILURE);
	EC(CL_LINKER_NOT_AVAILABLE);
	EC(CL_LINK_PROGRAM_FAILURE);
	EC(CL_DEVICE_PARTITION_FAILED);
	EC(CL_KERNEL_ARG_INFO_NOT_AVAILABLE);
//#endif
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
//#ifdef CL_VERSION_1_1
	EC(CL_INVALID_PROPERTY);
//#endif
//#ifdef CL_VERSION_1_2
	EC(CL_INVALID_IMAGE_DESCRIPTOR);
	EC(CL_INVALID_COMPILER_OPTIONS);
	EC(CL_INVALID_LINKER_OPTIONS);
	EC(CL_INVALID_DEVICE_PARTITION_COUNT);
//#endif
//#ifdef CL_VERSION_2_0
	EC2(-69, CL_INVALID_PIPE_SIZE);
	EC2(-70, CL_INVALID_DEVICE_QUEUE);
//#endif
//#ifdef CL_VERSION_2_2
	EC2(-71, CL_INVALID_SPEC_ID);
	EC2(-72, CL_MAX_SIZE_RESTRICTION_EXCEEDED);
//#endif
	EC(CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR);
	EC(CL_PLATFORM_NOT_FOUND_KHR);
	EC2(-1002, CL_INVALID_D3D10_DEVICE_KHR);
	EC2(-1003, CL_INVALID_D3D10_RESOURCE_KHR);
	EC2(-1004, CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR);
	EC2(-1005, CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR);
	EC2(-1006, CL_INVALID_D3D11_DEVICE_KHR);
	EC2(-1007, CL_INVALID_D3D11_RESOURCE_KHR);
	EC2(-1008, CL_D3D11_RESOURCE_ALREADY_ACQUIRED_KHR);
	EC2(-1009, CL_D3D11_RESOURCE_NOT_ACQUIRED_KHR);
	EC2(-1010, CL_INVALID_D3D9_DEVICE_NV_or_CL_INVALID_DX9_DEVICE_INTEL);
	EC2(-1011, CL_INVALID_D3D9_RESOURCE_NV_or_CL_INVALID_DX9_RESOURCE_INTEL);
	EC2(-1012, CL_D3D9_RESOURCE_ALREADY_ACQUIRED_NV_or_CL_DX9_RESOURCE_ALREADY_ACQUIRED_INTEL);
	EC2(-1013, CL_D3D9_RESOURCE_NOT_ACQUIRED_NV_or_CL_DX9_RESOURCE_NOT_ACQUIRED_INTEL);
	EC2(-1092, CL_EGL_RESOURCE_NOT_ACQUIRED_KHR);
	EC2(-1093, CL_INVALID_EGL_OBJECT_KHR);
	EC2(-1094, CL_INVALID_ACCELERATOR_INTEL);
	EC2(-1095, CL_INVALID_ACCELERATOR_TYPE_INTEL);
	EC2(-1096, CL_INVALID_ACCELERATOR_DESCRIPTOR_INTEL);
	EC2(-1097, CL_ACCELERATOR_TYPE_NOT_SUPPORTED_INTEL);
	EC2(-1098, CL_INVALID_VA_API_MEDIA_ADAPTER_INTEL);
	EC2(-1099, CL_INVALID_VA_API_MEDIA_SURFACE_INTEL);
	EC2(-1101, CL_VA_API_MEDIA_SURFACE_NOT_ACQUIRED_INTEL);
	default:
		a="???";
		break;
	}
	return a;
#undef				EC
}
void			cl_check(int err, int line)
{
	if(err)
		LOGERROR_LINE(line, "CL Error %d: %s", err, clerr2str(err));
}
#define 		CL_CHECK(error)	cl_check(error, __LINE__)
void 			p_check(void *p, int line, const char *func_name)
{
	if(!p)
	{
		LOGERROR_LINE(line, "Error: %s is 0x%08llx", func_name, (long long)p);
	//	LOGE("Line %d error: %s is %lld.", line, func_name, (long long)p);
		OCL_state=CL_NOTHING;
	}
}
//#define 		P_CHECK(pointer)	p_check(pointer, __LINE__, #pointer)
void			LOGE_long(const char *msg, int length)
{
	const int quota=1000;
	for(int k=0;k+quota-1<length;k+=quota)
		LOGE("G2_CL: %*s", quota, msg+k);
	//	LOGE("%*s", quota, msg+k);
}
#define 		DECL_CL_FUNC(clFunc)	decltype(clFunc) *p_##clFunc __attribute__((weak))=nullptr
const int 		cl_api_decl_start=__LINE__;
DECL_CL_FUNC(clGetPlatformIDs);
DECL_CL_FUNC(clGetDeviceIDs);
DECL_CL_FUNC(clGetDeviceInfo);
DECL_CL_FUNC(clCreateContext);
DECL_CL_FUNC(clGetContextInfo);
DECL_CL_FUNC(clCreateCommandQueue);
DECL_CL_FUNC(clCreateProgramWithSource);
DECL_CL_FUNC(clBuildProgram);
DECL_CL_FUNC(clGetProgramBuildInfo);
DECL_CL_FUNC(clGetProgramInfo);
DECL_CL_FUNC(clCreateProgramWithBinary);
DECL_CL_FUNC(clCreateBuffer);
DECL_CL_FUNC(clCreateKernel);
DECL_CL_FUNC(clSetKernelArg);
DECL_CL_FUNC(clEnqueueFillBuffer);
DECL_CL_FUNC(clEnqueueWriteBuffer);
DECL_CL_FUNC(clEnqueueNDRangeKernel);
DECL_CL_FUNC(clEnqueueReadBuffer);
DECL_CL_FUNC(clFinish);
DECL_CL_FUNC(clCreateFromGLBuffer);
DECL_CL_FUNC(clCreateFromGLTexture);
DECL_CL_FUNC(clReleaseMemObject);
const int 		cl_api_decl_end=__LINE__;
#undef			DECL_CL_FUNC
void			*hOpenCL=nullptr;
void 			load_OpenCL_API()
{
	if(OCL_state<CL_API_LOADED)
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
		if(!hOpenCL)
		{
			OCL_state=CL_NOTHING;
			LOGERROR("Cannot find OpenCL library");
		}
		else
		{
			OCL_state=CL_LOADING_API;
#define		GET_CL_FUNC(handle, clFunc)		p_##clFunc=(decltype(p_##clFunc))dlsym(handle, #clFunc), p_check((void*)p_##clFunc, __LINE__, #clFunc)
			const int cl_api_init_start=__LINE__;
			GET_CL_FUNC(hOpenCL, clGetPlatformIDs);
			GET_CL_FUNC(hOpenCL, clGetDeviceIDs);
			GET_CL_FUNC(hOpenCL, clGetDeviceInfo);
			GET_CL_FUNC(hOpenCL, clCreateContext);
			GET_CL_FUNC(hOpenCL, clGetContextInfo);
			GET_CL_FUNC(hOpenCL, clCreateCommandQueue);
			GET_CL_FUNC(hOpenCL, clCreateProgramWithSource);
			GET_CL_FUNC(hOpenCL, clBuildProgram);
			GET_CL_FUNC(hOpenCL, clGetProgramBuildInfo);
			GET_CL_FUNC(hOpenCL, clGetProgramInfo);
			GET_CL_FUNC(hOpenCL, clCreateProgramWithBinary);
			GET_CL_FUNC(hOpenCL, clCreateBuffer);
			GET_CL_FUNC(hOpenCL, clCreateKernel);
			GET_CL_FUNC(hOpenCL, clSetKernelArg);
			GET_CL_FUNC(hOpenCL, clEnqueueFillBuffer);
			GET_CL_FUNC(hOpenCL, clEnqueueWriteBuffer);
			GET_CL_FUNC(hOpenCL, clEnqueueNDRangeKernel);
			GET_CL_FUNC(hOpenCL, clEnqueueReadBuffer);
			GET_CL_FUNC(hOpenCL, clFinish);
			GET_CL_FUNC(hOpenCL, clCreateFromGLBuffer);
			GET_CL_FUNC(hOpenCL, clCreateFromGLTexture);
			GET_CL_FUNC(hOpenCL, clReleaseMemObject);
			const int cl_api_init_end=__LINE__;
#undef		GET_CL_FUNC
			const int n_functions=cl_api_decl_end-(cl_api_decl_start+1), n_initialized=cl_api_init_end-(cl_api_init_start+1);
			static_assert(n_functions==n_initialized, "number of declared functions not equal to number of initialized functions");
			if(OCL_state!=CL_NOTHING)
				OCL_state=CL_API_LOADED;
		}
	}
}
void 			unload_OpenCL_API()
{
	if(hOpenCL)//when finishing opencl session
	{
		dlclose(hOpenCL), hOpenCL=nullptr;
		OCL_state=CL_NOTHING;
	}
}

enum 			CLDiscType
{
	DISC_C,//continuous
	DISC_I,//depends on input
	DISC_O,//depends on output
};
struct 			CLKernel
{
	int idx;//enum CLKernelIdx
	short
		signature,//enum CLKernelSignature
		disc_type;//enum CLDiscType
	const char
		*name,//kernel name string, if 0 then software
		*disc_name;//if 0 then always continuous
//	cl_kernel g2, disc;
};
struct			KernelDB
{
	CLKernel *kernels;
	int nkernels;
};
namespace		CLSource
{
	static const char program_common[]=R"CLSRC(
//math constants
#define	_pi			3.14159265358979f
#define	_2pi		6.28318530717959f
#define	_sqrt_2pi	2.506628274631f
#define	_sqrt2		1.4142135623731f
#define	_ln2		0.693147180559945f
#define	_inv_ln10	0.434294481903252f
#define	_ln_phi		0.481211825059603f
#define	_inv_sqrt5	0.447213595499958f

//macros for G2 functions
#define		ARG_CI(arg)		__global const int *arg
#define		ARG_F(arg)		__global float *arg
#define		ARG_CF(arg)		__global const float *arg
#define		ARG_C(arg)		__global char *arg
#define		G2_R_R(func)	__kernel void  r_r_##func(ARG_CI(size), ARG_F(rr), ARG_CF(xr))
#define		G2_C_C(func)	__kernel void  c_c_##func(ARG_CI(size), ARG_F(rr), ARG_F(ri), ARG_CF(xr), ARG_CF(xi))
#define		G2_Q_Q(func)	__kernel void  q_q_##func(ARG_CI(size), ARG_F(rr), ARG_F(ri), ARG_F(rj), ARG_F(rk), ARG_CF(xr), ARG_CF(xi), ARG_CF(xj), ARG_CF(xk))
#define		G2_R_RR(func)	__kernel void r_rr_##func(ARG_CI(size), ARG_F(rr), ARG_CF(xr), ARG_CF(yr))
#define		G2_C_RC(func)	__kernel void c_rc_##func(ARG_CI(size), ARG_F(rr), ARG_F(ri), ARG_CF(xr), ARG_CF(yr), ARG_CF(yi))
#define		G2_Q_RQ(func)	__kernel void q_rq_##func(ARG_CI(size), ARG_F(rr), ARG_F(ri), ARG_F(rj), ARG_F(rk), ARG_CF(xr), ARG_CF(yr), ARG_CF(yi), ARG_CF(yj), ARG_CF(yk))
#define		G2_C_CR(func)	__kernel void c_cr_##func(ARG_CI(size), ARG_F(rr), ARG_F(ri), ARG_CF(xr), ARG_CF(xi), ARG_CF(yr))
#define		G2_C_CC(func)	__kernel void c_cc_##func(ARG_CI(size), ARG_F(rr), ARG_F(ri), ARG_CF(xr), ARG_CF(xi), ARG_CF(yr), ARG_CF(yi))
#define		G2_Q_CQ(func)	__kernel void q_cq_##func(ARG_CI(size), ARG_F(rr), ARG_F(ri), ARG_F(rj), ARG_F(rk), ARG_CF(xr), ARG_CF(xi), ARG_CF(yr), ARG_CF(yi), ARG_CF(yj), ARG_CF(yk))
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

#define		DISC_R_O(func)	__kernel void disc_r_##func##_o(ARG_CI(size), const int offset, ARG_C(disc), ARG_F(xr))
#define		DISC_C_O(func)	__kernel void disc_c_##func##_o(ARG_CI(size), const int offset, ARG_C(disc), ARG_F(xr), ARG_F(xi))
#define		DISC_Q_O(func)	__kernel void disc_q_##func##_o(ARG_CI(size), const int offset, ARG_C(disc), ARG_F(xr), ARG_F(xi), ARG_F(xj), ARG_F(xk))
#define		DISC_R_I(func)	__kernel void disc_r_##func##_i(ARG_CI(size), const int offset, ARG_C(disc), ARG_F(xr))
#define		DISC_C_I(func)	__kernel void disc_c_##func##_i(ARG_CI(size), const int offset, ARG_C(disc), ARG_F(xr), ARG_F(xi))
#define		DISC_Q_I(func)	__kernel void disc_q_##func##_i(ARG_CI(size), const int offset, ARG_C(disc), ARG_F(xr), ARG_F(xi), ARG_F(xj), ARG_F(xk))
#define		DISC_RR_I(func)	__kernel void disc_rr_##func##_i(ARG_CI(size), const int offset, ARG_C(disc), ARG_F(xr), ARG_F(yr))
#define		DISC_RC_I(func)	__kernel void disc_rc_##func##_i(ARG_CI(size), const int offset, ARG_C(disc), ARG_F(xr), ARG_F(yr), ARG_F(yi))
#define		DISC_RQ_I(func)	__kernel void disc_rq_##func##_i(ARG_CI(size), const int offset, ARG_C(disc), ARG_F(xr), ARG_F(yr), ARG_F(yi), ARG_F(yj), ARG_F(yk))
#define		DISC_CR_I(func)	__kernel void disc_cr_##func##_i(ARG_CI(size), const int offset, ARG_C(disc), ARG_F(xr), ARG_F(xi), ARG_F(yr))
#define		DISC_CC_I(func)	__kernel void disc_cc_##func##_i(ARG_CI(size), const int offset, ARG_C(disc), ARG_F(xr), ARG_F(xi), ARG_F(yr), ARG_F(yi))
#define		DISC_CQ_I(func)	__kernel void disc_cq_##func##_i(ARG_CI(size), const int offset, ARG_C(disc), ARG_F(xr), ARG_F(xi), ARG_F(yr), ARG_F(yi), ARG_F(yj), ARG_F(yk))
#define		DISC_QR_I(func)	__kernel void disc_qr_##func##_i(ARG_CI(size), const int offset, ARG_C(disc), ARG_F(xr), ARG_F(xi), ARG_F(xj), ARG_F(xk), ARG_F(yr))
#define		DISC_QC_I(func)	__kernel void disc_qc_##func##_i(ARG_CI(size), const int offset, ARG_C(disc), ARG_F(xr), ARG_F(xi), ARG_F(xj), ARG_F(xk), ARG_F(yr), ARG_F(yi))
#define		DISC_QQ_I(func)	__kernel void disc_qq_##func##_i(ARG_CI(size), const int offset, ARG_C(disc), ARG_F(xr), ARG_F(xi), ARG_F(xj), ARG_F(xk), ARG_F(yr), ARG_F(yi), ARG_F(yj), ARG_F(yk))

#define		IDX						const unsigned idx=get_idx(size)
#define		ASSIGN_R(r)				rr[idx]=r
#define		ASSIGN_C(r, i)			rr[idx]=r, ri[idx]=i
#define		ASSIGN_Q(r, i, j, k)	rr[idx]=r, ri[idx]=i, rj[idx]=j, rk[idx]=k
#define		RET_C					ASSIGN_C(ret.x, ret.y)
#define		RET_Q					ASSIGN_Q(ret.x, ret.y, ret.z, ret.w)
#define		VEC2(name)				(float2)(name##r[idx], name##i[idx])
#define		VEC4(name)				(float4)(name##r[idx], name##i[idx], name##j[idx], name##k[idx])

//auxiliary functions
int		get_idx(__global const int *size){return size[0]*(size[1]*get_global_id(2)+get_global_id(1))+get_global_id(0);}
int		iscastable2int(float x){return x>=-2147483647.f&&x<=2147483647.f;}
int		f2i(float x){return (int)floor(x);}
bool	_1d_int_in_range(float a, float b){return floor(a)!=floor(b)||ceil(a)!=ceil(b);}
bool	_1d_zero_in_range(float a, float b){return a<0?b>=0:a==0?b!=0:b<0;}
float	_1d_zero_crossing(float x0, float y0, float x1, float y1){return x0+(0-y0)*(x1-x0)/(y1-y0);}
bool	istrue_c(float2 a){return a.x||a.y;}
bool	istrue_q(float4 a){return a.x||a.y||a.z||a.w;}
float	abs_c(float2 a){return sqrt(a.x*a.x+a.y*a.y);}
float	abs_q(float4 a){return sqrt(a.x*a.x+a.y*a.y+a.z*a.z+a.w*a.w);}
bool	equal_rc(float a, float2 b){return a==b.x&&0==b.y;}
bool	equal_rq(float a, float4 b){return a==b.x&&0==b.y&&0==b.z&&0==b.w;}
bool	equal_cr(float2 a, float b){return a.x==b&&a.y==0;}
bool	equal_cc(float2 a, float2 b){return a.x==b.x&&a.y==b.y;}
bool	equal_cq(float2 a, float4 b){return a.x==b.x&&a.y==b.y&&0==b.z&&0==b.w;}
bool	equal_qr(float4 a, float b){return a.x==b&&a.y==0&&a.z==0&&a.w==0;}
bool	equal_qc(float4 a, float2 b){return a.x==b.x&&a.y==b.y&&a.z==0&&a.w==0;}
bool	equal_qq(float4 a, float4 b){return a.x==b.x&&a.y==b.y&&a.z==b.z&&a.w==b.w;}
float2	floor_c(float2 a){return (float2)(floor(a.x), floor(a.y));}
float4	floor_q(float4 a){return (float4)(floor(a.x), floor(a.y), floor(a.z), floor(a.w));}
float2	sq_c(float2 a){float ri=a.x*a.y; return (float2)(a.x*a.x-a.y*a.y, ri+ri);}
float4	sq_q(float4 a){float _2r=a.x+a.x; return (float4)(a.x*a.x-a.y*a.y-a.z*a.z-a.w*a.w, a.y*_2r, a.z*_2r, a.w*_2r);}
float2	sqrt_c(float2 a)
{
	float s=abs_c(a)+a.x;
	if(s)//entire complex plane except origin & -ve real axis
	{
		s=sqrt(s+s);
		return (float2)(s*0.5f, a.y/s);
	}
	return (float2)(0, sqrt(-a.x));
}
float4	sqrt_q(float4 a)
{
	float s=abs_q(a)+a.x;
	if(s)
	{
		s=sqrt(s+s);
		float inv_s=1/s;
		return (float4)(s*0.5f, a.y*inv_s, a.z*inv_s, a.w*inv_s);
	}
	return (float4)(0, sqrt(-a.x), 0, 0);
}
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
	float v_mul=exp_r*sin_v/abs_v;
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
		atan2(a.y, a.x)
	);
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
		a.w*v_mul
	);
}
//complex trig & hyp functions
float2 coshsinh(float x)
{
	float exp_x=exp(x);
	float2 e=(float2)(exp_x, 1/exp_x)*0.5f;
	return (float2)(e.x+e.y, e.x-e.y);
}
float2 cos_c(float2 a){return (float2)(cos(a.x)*cosh(a.y), -sin(a.x)*sinh(a.y));}
float4 cos_q(float4 a)
{
	float cos_r, sin_r=sincos(a.x, &cos_r);
	float abs_v=sqrt(a.y*a.y+a.z*a.z+a.w*a.w);
	float2 chsh_absv=coshsinh(abs_v);
	float v_mul=-sin_r*chsh_absv.y/abs_v;
	return (float4)(cos_r*chsh_absv.x, v_mul*a.y, v_mul*a.z, v_mul*a.w);
}
float2 acos_c(float2 a)
{
	const float2 m_i=(float2)(0, -1);
	float2 temp=sq_c(a);
	temp.x-=1;
	return mul_cc(log_c(a+sqrt_c(temp)), m_i);
}
float4 acos_q(float4 a)
{
	const float2 m_i=(float2)(0, -1);
	float4 temp=sq_q(a);
	temp.x-=1;
	return mul_qc(log_q(a+sqrt_q(temp)), m_i);
}
float2 cosh_c(float2 a){return (float2)(cos(a.x)*cosh(a.y), -sin(a.x)*sinh(a.y));}
float4 cosh_q(float4 a)
{
	float4 exp_a=exp_q(a), exp_ma=inv_q(exp_a);
	return mul_qr(exp_a+exp_ma, 0.5f);
}
float2 acosh_c(float2 a)
{
	float2 temp=sq_c(a);
	temp.x-=1;
	return log_c(a+sqrt_c(temp));
}
float4 acosh_q(float4 a)
{
	float4 temp=sq_q(a);
	temp.x-=1;
	return log_q(a+sqrt_q(temp));
}
float2 sin_c(float2 a){return (float2)(sin(a.x)*cosh(a.y), cos(a.x)*sinh(a.y));}
float4 sin_q(float4 a)
{
	float cos_r, sin_r=sincos(a.x, &cos_r);
	float abs_v=sqrt(a.y*a.y+a.z*a.z+a.w*a.w);
	float2 chsh_absv=coshsinh(abs_v);
	float v_mul=-cos_r*chsh_absv.y/abs_v;
	return (float4)(sin_r*chsh_absv.x, v_mul*a.y, v_mul*a.z, v_mul*a.w);
}
float2 asin_c(float2 a)
{
	const float2 i=(float2)(0, 1), m_i=(float2)(0, -1);
	float2 temp=sq_c(a);
	temp.x=1-temp.x, temp.y=-temp.y;
	return mul_cc(log_c(mul_cc(a, i)+sqrt_c(temp)), m_i);
}
float4 asin_q(float4 a)
{
	const float2 i=(float2)(0, 1), m_i=(float2)(0, -1);
	float4 temp=sq_q(a);
	temp=(float4)(1, 0, 0, 0)-temp;
	return mul_qc(log_q(mul_qc(a, i)+sqrt_q(temp)), m_i);
}
float2 sinh_c(float2 a){return (float2)(sinh(a.x)*cos(a.y), cosh(a.x)*sin(a.y));}
float4 sinh_q(float4 a)
{
	float4 exp_a=exp_q(a), exp_ma=inv_q(exp_a);
	return mul_qr(exp_a-exp_ma, 0.5f);
}
float2 asinh_c(float2 a)
{
	float2 temp=sq_c(a);
	temp.x+=1;
	return log_c(a+sqrt_c(temp));
}
float4 asinh_q(float4 a)
{
	float4 temp=sq_q(a);
	temp.x+=1;
	return log_q(a+sqrt_q(temp));
}
float2 tan_c(float2 a)
{
	const float2 _2i=(float2)(0, 2), one=(float2)(1, 0);
	float2 exp2ia=exp_c(mul_cc(_2i, a));
	return div_cc(exp2ia-one, exp2ia+one);
}
float4 tan_q(float4 a)
{
	const float2 _2i=(float2)(0, 2);
	const float4 one=(float4)(1, 0, 0, 0);
	float4 exp2ia=exp_q(mul_cq(_2i, a));
	return div_qq(exp2ia-one, exp2ia+one);
}
float2 atan_c(float2 a)
{
	const float2 i=(float2)(0, 1), i_2=(float2)(0, 0.5f);
	return mul_cc(i_2, log_c(div_cc(i+a, i-a)));
}
float4 atan_q(float4 a)
{
	const float4 i=(float4)(0, 1, 0, 0);
	const float2 i_2=(float2)(0, 0.5f);
	return mul_cq(i_2, log_q(div_qq(i+a, i-a)));
}
float2 tanh_c(float2 a)
{
	const float2 one=(float2)(1, 0);
	float2 exp2a=exp_c(a+a);
	return div_cc(exp2a-one, exp2a+one);
}
float4 tanh_q(float4 a)
{
	const float4 one=(float4)(1, 0, 0, 0);
	float4 exp2a=exp_q(a+a);
	return div_qq(exp2a-one, exp2a+one);
}
float2 atanh_c(float2 a)
{
	const float2 one=(float2)(1, 0);
	return mul_rc(0.5f, log_c(div_cc(one+a, one-a)));
}
float4 atanh_q(float4 a)
{
	const float4 one=(float4)(1, 0, 0, 0);
	return mul_rq(0.5f, log_q(div_qq(one+a, one-a)));
}
//end complex trig & hyp functions

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
)CLSRC";
	static const char program00[]=R"CLSRC(
G2_R_R(setzero){IDX; ASSIGN_R(0);}
G2_C_C(setzero){IDX; ASSIGN_C(0, 0);}
G2_Q_Q(setzero){IDX; ASSIGN_Q(0, 0, 0, 0);}

G2_R_R(ceil){IDX; ASSIGN_R(ceil(xr[idx]));}
G2_C_C(ceil){IDX; ASSIGN_C(ceil(xr[idx]), ceil(xi[idx]));}
G2_Q_Q(ceil){IDX; ASSIGN_Q(ceil(xr[idx]), ceil(xi[idx]), ceil(xj[idx]), ceil(xk[idx]));}
DISC_R_O(ceil){IDX; disc[idx]=xr[idx]!=xr[idx+offset];}
DISC_C_O(ceil){IDX; disc[idx]=xr[idx]!=xr[idx+offset]||xi[idx]!=xi[idx+offset];}
DISC_Q_O(ceil){IDX; disc[idx]=xr[idx]!=xr[idx+offset]||xi[idx]!=xi[idx+offset]||xj[idx]!=xj[idx+offset]||xk[idx]!=xk[idx+offset];}

G2_R_R(floor){IDX; ASSIGN_R(floor(xr[idx]));}
G2_C_C(floor){IDX; ASSIGN_C(floor(xr[idx]), floor(xi[idx]));}
G2_Q_Q(floor){IDX; ASSIGN_Q(floor(xr[idx]), floor(xi[idx]), floor(xj[idx]), floor(xk[idx]));}
DISC_R_O(floor){disc_r_ceil_o(size, offset, disc, xr);}
DISC_C_O(floor){disc_c_ceil_o(size, offset, disc, xr, xi);}
DISC_Q_O(floor){disc_q_ceil_o(size, offset, disc, xr, xi, xj, xk);}

G2_R_R(round){IDX; ASSIGN_R(round(xr[idx]));}
G2_C_C(round){IDX; ASSIGN_C(round(xr[idx]), round(xi[idx]));}
G2_Q_Q(round){IDX; ASSIGN_Q(round(xr[idx]), round(xi[idx]), round(xj[idx]), round(xk[idx]));}
DISC_R_O(round){disc_r_ceil_o(size, offset, disc, xr);}
DISC_C_O(round){disc_c_ceil_o(size, offset, disc, xr, xi);}
DISC_Q_O(round){disc_q_ceil_o(size, offset, disc, xr, xi, xj, xk);}

G2_R_R(int){IDX; ASSIGN_R((int)xr[idx]);}
G2_C_C(int){IDX; ASSIGN_C((int)xr[idx], (int)xi[idx]);}
G2_Q_Q(int){IDX; ASSIGN_Q((int)xr[idx], (int)xi[idx], (int)xj[idx], (int)xk[idx]);}
DISC_R_O(int){disc_r_ceil_o(size, offset, disc, xr);}
DISC_C_O(int){disc_c_ceil_o(size, offset, disc, xr, xi);}
DISC_Q_O(int){disc_q_ceil_o(size, offset, disc, xr, xi, xj, xk);}

G2_R_R(frac){IDX; float fxr;				ASSIGN_R(fract(xr[idx], &fxr));}
G2_C_C(frac){IDX; float fxr, fxi;			ASSIGN_C(fract(xr[idx], &fxr), fract(xi[idx], &fxi));}
G2_Q_Q(frac){IDX; float fxr, fxi, fxj, fxk;	ASSIGN_Q(fract(xr[idx], &fxr), fract(xi[idx], &fxi), fract(xj[idx], &fxj), fract(xk[idx], &fxk));}
DISC_R_I(frac){IDX; disc[idx]=floor(xr[idx])!=floor(xr[idx+offset]);}
DISC_C_I(frac){IDX; disc[idx]=floor(xr[idx])!=floor(xr[idx+offset])||floor(xi[idx])!=floor(xi[idx+offset]);}
DISC_Q_I(frac){IDX; disc[idx]=floor(xr[idx])!=floor(xr[idx+offset])||floor(xi[idx])!=floor(xi[idx+offset])||floor(xj[idx])!=floor(xj[idx+offset])||floor(xk[idx])!=floor(xk[idx+offset]);}
)CLSRC";
	static const char program01[]=R"CLSRC(
G2_R_R(abs){IDX; rr[idx]=fabs(xr[idx]);}
G2_R_C(abs){IDX; float2 a=VEC2(x); rr[idx]=sqrt(a.x*a.x+a.y*a.y);}
G2_R_Q(abs){IDX; float4 a=VEC4(x); rr[idx]=sqrt(a.x*a.x+a.y*a.y+a.z*a.z+a.w*a.w);}

G2_R_R(arg)
{
	IDX;
	if(xr[idx]<0)
		rr[idx]=_pi;
	else if(xr[idx]==0)
		rr[idx]=NAN;
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
		rr[idx]=NAN;
	else
	{
		abs_a=sqrt(abs_a);
		rr[idx]=acos(a.x/abs_a);
	}
}
DISC_R_I(arg)
{
	IDX;
		 if(xr[idx]<0)	disc[idx]=xr[idx+offset]>=0;
	else if(xr[idx]>0)	disc[idx]=xr[idx+offset]<=0;
	else				disc[idx]=xr[idx+offset]!=0;
}
DISC_C_I(arg){IDX; disc[idx]=false;}//TODO
DISC_Q_I(arg){IDX; disc[idx]=false;}//TODO

G2_R_C(real){IDX; rr[idx]=xr[idx];}

G2_R_C(imag){IDX; rr[idx]=xi[idx];}

G2_C_C(conjugate){IDX; ASSIGN_C(xr[idx], -xi[idx]);}
G2_Q_Q(conjugate){IDX; ASSIGN_Q(xr[idx], -xi[idx], -xj[idx], -xk[idx]);}

G2_C_R(polar)
{
	IDX;
	float a=xr[idx];
	float2 ret=(float2)(fabs(a), a<0?_pi:a==0?NAN:0);
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
		 if(xr[idx]<0)	disc[idx]=xr[idx+offset]>=0;
	else if(xr[idx]>0)	disc[idx]=xr[idx+offset]<=0;
	else				disc[idx]=xr[idx+offset]!=0;
}
DISC_C_I(polar){IDX; disc[idx]=false;}//TODO
DISC_Q_I(polar){IDX; disc[idx]=false;}//TODO

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
)CLSRC";
	static const char program02[]=R"CLSRC(
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
G2_C_RC(multiply){IDX; float2 ret=mul_rc(xr[idx], VEC2(y)); RET_C;}
G2_Q_RQ(multiply){IDX; float4 ret=mul_rq(xr[idx], VEC4(y)); RET_Q;}
G2_C_CR(multiply){IDX; float2 ret=mul_cr(VEC2(x), yr[idx]); RET_C;}
G2_C_CC(multiply){IDX; float2 ret=mul_cc(VEC2(x), VEC2(y)); RET_C;}
G2_Q_CQ(multiply){IDX; float4 ret=mul_cq(VEC2(x), VEC4(y)); RET_Q;}
G2_Q_QR(multiply){IDX; float4 ret=mul_qr(VEC4(x), yr[idx]); RET_Q;}
G2_Q_QC(multiply){IDX; float4 ret=mul_qc(VEC4(x), VEC2(y)); RET_Q;}
G2_Q_QQ(multiply){IDX; float4 ret=mul_qq(VEC4(x), VEC4(y)); RET_Q;}

G2_R_R(divide){IDX; rr[idx]=1/xr[idx];}
G2_C_C(divide){IDX; float2 ret=inv_c(VEC2(x)); RET_C;}
G2_Q_Q(divide){IDX; float4 ret=inv_q(VEC4(x)); RET_Q;}
G2_R_RR(divide){IDX; ASSIGN_R(xr[idx]/yr[idx]);}
G2_C_RC(divide){IDX; float2 ret=div_rc(xr[idx], VEC2(y)); RET_C;}
G2_Q_RQ(divide){IDX; float4 ret=div_rq(xr[idx], VEC4(y)); RET_Q;}
G2_C_CR(divide){IDX; float2 ret=div_cr(VEC2(x), yr[idx]); RET_C;}
G2_C_CC(divide){IDX; float2 ret=div_cc(VEC2(x), VEC2(y)); RET_C;}
G2_Q_CQ(divide){IDX; float4 ret=div_cq(VEC2(x), VEC4(y)); RET_Q;}
G2_Q_QR(divide){IDX; float4 ret=div_qr(VEC4(x), yr[idx]); RET_Q;}
G2_Q_QC(divide){IDX; float4 ret=div_qc(VEC4(x), VEC2(y)); RET_Q;}
G2_Q_QQ(divide){IDX; float4 ret=div_qq(VEC4(x), VEC4(y)); RET_Q;}
DISC_R_I(divide){IDX; float x0r=xr[idx], x1r=xr[idx+offset]; disc[idx]=x0r<0?x1r>=0:x0r>0?x1r<=0:x1r!=0;}
DISC_C_I(divide){IDX; disc[idx]=false;}//TODO
DISC_Q_I(divide){IDX; disc[idx]=false;}//
DISC_RR_I(divide){IDX; float y0r=yr[idx], y1r=yr[idx+offset]; disc[idx]=y0r<0?y1r>=0:y0r>0?y1r<=0:y1r!=0;}
DISC_RC_I(divide){IDX; disc[idx]=false;}//
DISC_RQ_I(divide){IDX; disc[idx]=false;}//
DISC_CR_I(divide){IDX; float y0r=yr[idx], y1r=yr[idx+offset]; disc[idx]=y0r<0?y1r>=0:y0r>0?y1r<=0:y1r!=0;}
DISC_CC_I(divide){IDX; disc[idx]=false;}//
DISC_CQ_I(divide){IDX; disc[idx]=false;}//
DISC_QR_I(divide){IDX; float y0r=yr[idx], y1r=yr[idx+offset]; disc[idx]=y0r<0?y1r>=0:y0r>0?y1r<=0:y1r!=0;}
DISC_QC_I(divide){IDX; disc[idx]=false;}//
DISC_QQ_I(divide){IDX; disc[idx]=false;}//
)CLSRC";
	static const char program03[]=R"CLSRC(
G2_R_RR(logic_divides){IDX; float q=xr[idx]/yr[idx]; rr[idx]=q==floor(q);}
G2_R_RC(logic_divides){IDX; float2 q=div_rc(xr[idx], VEC2(y)); rr[idx]=equal_cc(q, floor_c(q));}
G2_R_RQ(logic_divides){IDX; float4 q=div_rq(xr[idx], VEC4(y)); rr[idx]=equal_qq(q, floor_q(q));}
G2_R_CR(logic_divides){IDX; float2 q=div_cr(VEC2(x), yr[idx]); rr[idx]=equal_cc(q, floor_c(q));}
G2_R_CC(logic_divides){IDX; float2 q=div_cc(VEC2(x), VEC2(y)); rr[idx]=equal_cc(q, floor_c(q));}
G2_R_CQ(logic_divides){IDX; float4 q=div_cq(VEC2(x), VEC4(y)); rr[idx]=equal_qq(q, floor_q(q));}
G2_R_QR(logic_divides){IDX; float4 q=div_qr(VEC4(x), yr[idx]); rr[idx]=equal_qq(q, floor_q(q));}
G2_R_QC(logic_divides){IDX; float4 q=div_qc(VEC4(x), VEC2(y)); rr[idx]=equal_qq(q, floor_q(q));}
G2_R_QQ(logic_divides){IDX; float4 q=div_qq(VEC4(x), VEC4(y)); rr[idx]=equal_qq(q, floor_q(q));}
DISC_R_O(logic_divides){IDX; disc[idx]=xr[idx]!=xr[idx+offset];}//for all logic_divides functions

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
G2_C_CR(power_real)
{
	IDX;
	float2 a=VEC2(x), ret=(float2)(1, 0);
	int b=(int)yr[idx], p=abs(b);
	for(;;)
	{
		if(p&1)
			ret=mul_cc(ret, a);
		p>>=1;
		if(!p)
			break;
		a=mul_cc(a, a);
	}
	if(b<0)
		ret=inv_c(ret);
	RET_C;
}
G2_Q_QR(power_real)
{
	IDX;
	float4 a=VEC4(x), ret=(float4)(1, 0, 0, 0);
	int b=(int)yr[idx], p=abs(b);
	for(;;)
	{
		if(p&1)
			ret=mul_qq(ret, a);
		p>>=1;
		if(!p)
			break;
		a=mul_qq(a, a);
	}
	if(b<0)
		ret=inv_q(ret);
	RET_Q;
}
DISC_RR_I(power_real){IDX; disc[idx]=floor(yr[idx])!=floor(yr[idx+offset]);}
DISC_CR_I(power_real){IDX; disc[idx]=floor(yr[idx])!=floor(yr[idx+offset]);}
DISC_QR_I(power_real){IDX; disc[idx]=floor(yr[idx])!=floor(yr[idx+offset]);}

G2_C_CR(pow)
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
G2_C_CC(pow)
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
G2_Q_CQ(pow)
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
G2_Q_QR(pow)
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
G2_Q_QC(pow)
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
G2_Q_QQ(pow)
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
DISC_CR_I(pow){IDX; disc[idx]=false;}//TODO
DISC_CC_I(pow){IDX; disc[idx]=false;}//TODO
DISC_CQ_I(pow){IDX; disc[idx]=false;}//TODO
DISC_QR_I(pow){IDX; disc[idx]=false;}//TODO
DISC_QC_I(pow){IDX; disc[idx]=false;}//TODO
DISC_QQ_I(pow){IDX; disc[idx]=false;}//TODO

G2_C_C(ln){IDX; float2 ret=log_c(VEC2(x)); RET_C;}
G2_Q_Q(ln){IDX; float4 ret=log_q(VEC4(x)); RET_Q;}
DISC_C_I(ln){IDX; disc[idx]=false;}//TODO
DISC_Q_I(ln){IDX; disc[idx]=false;}

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
	float2 ret=div_cc(log_a, log_b);
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
G2_Q_CQ(log)
{
	IDX;
	float2 log_a=log_c(VEC2(x));
	float4 log_b=log_q(VEC4(y));
	float4 ret=div_cq(log_a, log_b);
	RET_Q;
}
G2_Q_QC(log)
{
	IDX;
	float4 log_a=log_q(VEC4(x));
	float2 log_b=log_c(VEC2(y));
	float4 ret=div_qc(log_a, log_b);
	RET_Q;
}
G2_Q_QQ(log)
{
	IDX;
	float4 log_a=log_q(VEC4(x));
	float4 log_b=log_q(VEC4(y));
	float4 ret=div_qq(log_a, log_b);
	RET_Q;
}
DISC_C_I(log){IDX; disc[idx]=false;}//TODO disc c arg
DISC_Q_I(log){IDX; disc[idx]=false;}//TODO
DISC_CR_I(log){IDX; disc[idx]=false;}//TODO
DISC_CC_I(log){IDX; disc[idx]=false;}//TODO
DISC_CQ_I(log){IDX; disc[idx]=false;}//TODO
DISC_QC_I(log){IDX; disc[idx]=false;}//TODO
DISC_QQ_I(log){IDX; disc[idx]=false;}//TODO

//tetrate

//pentate
)CLSRC";
	static const char program04[]=R"CLSRC(
G2_R_R(bitwise_shift_left_l){IDX; ASSIGN_R(exp(floor(xr[idx])*_ln2));}
G2_C_C(bitwise_shift_left_l)
{
	IDX;
	float2 ret=exp_c(mul_cr(floor_c(VEC2(x)), _ln2));
	RET_C;
}
G2_Q_Q(bitwise_shift_left_l)
{
	IDX;
	float4 ret=exp_q(mul_qr(floor_q(VEC4(x)), _ln2));
	RET_Q;
}
G2_R_R(bitwise_shift_left_r){IDX; ASSIGN_R(xr[idx]+xr[idx]);}
G2_C_C(bitwise_shift_left_r)
{
	IDX;
	float2 ret=VEC2(x);
	ret+=ret;
	RET_C;
}
G2_Q_Q(bitwise_shift_left_r)
{
	IDX;
	float4 ret=VEC4(x);
	ret+=ret;
	RET_Q;
}
G2_R_RR(bitwise_shift_left){IDX; ASSIGN_R(xr[idx]*exp(floor(yr[idx])*_ln2));}
G2_C_RC(bitwise_shift_left)
{
	IDX;
	float a=xr[idx];
	float2 b=VEC2(y);
	float2 ret=mul_rc(a, exp_c(mul_cr(floor_c(b), _ln2)));
	RET_C;
}
G2_Q_RQ(bitwise_shift_left)
{
	IDX;
	float a=xr[idx];
	float4 b=VEC4(y);
	float4 ret=mul_rq(a, exp_q(mul_qr(floor_q(b), _ln2)));
	RET_Q;
}
G2_C_CR(bitwise_shift_left)
{
	IDX;
	float2 a=VEC2(x);
	float b=yr[idx];
	float2 ret=mul_cr(a, exp(floor(b)*_ln2));
	RET_C;
}
G2_C_CC(bitwise_shift_left)
{
	IDX;
	float2 a=VEC2(x);
	float2 b=VEC2(y);
	float2 ret=mul_cc(a, exp_c(mul_cr(floor_c(b), _ln2)));
	RET_C;
}
G2_Q_CQ(bitwise_shift_left)
{
	IDX;
	float2 a=VEC2(x);
	float4 b=VEC4(y);
	float4 ret=mul_cq(a, exp_q(mul_qr(floor_q(b), _ln2)));
	RET_Q;
}
G2_Q_QR(bitwise_shift_left)
{
	IDX;
	float4 a=VEC4(x);
	float b=yr[idx];
	float4 ret=mul_qr(a, exp(floor(b)*_ln2));
	RET_Q;
}
G2_Q_QC(bitwise_shift_left)
{
	IDX;
	float4 a=VEC4(x);
	float2 b=VEC2(y);
	float4 ret=mul_qc(a, exp_c(mul_cr(floor_c(b), _ln2)));
	RET_Q;
}
G2_Q_QQ(bitwise_shift_left)
{
	IDX;
	float4 a=VEC4(x);
	float4 b=VEC4(y);
	float4 ret=mul_qq(a, exp_q(mul_qr(floor_q(b), _ln2)));
	RET_Q;
}
DISC_R_O(bitwise_shift_left_l){IDX; disc[idx]=xr[idx]!=xr[idx+offset];}
DISC_C_O(bitwise_shift_left_l){IDX; disc[idx]=xr[idx]!=xr[idx+offset]||xi[idx]!=xi[idx+offset];}
DISC_Q_O(bitwise_shift_left_l){IDX; disc[idx]=xr[idx]!=xr[idx+offset]||xi[idx]!=xi[idx+offset]||xj[idx]!=xj[idx+offset]||xk[idx]!=xk[idx+offset];}
DISC_RR_I(bitwise_shift_left){IDX; disc[idx]=floor(xr[idx])!=floor(xr[idx+offset]);}
DISC_RC_I(bitwise_shift_left){IDX; disc[idx]=floor(xr[idx])!=floor(xr[idx+offset]);}
DISC_RQ_I(bitwise_shift_left){IDX; disc[idx]=floor(xr[idx])!=floor(xr[idx+offset]);}
DISC_CR_I(bitwise_shift_left){IDX; disc[idx]=floor(xr[idx])!=floor(xr[idx+offset])||floor(xi[idx])!=floor(xi[idx+offset]);}
DISC_CC_I(bitwise_shift_left){IDX; disc[idx]=floor(xr[idx])!=floor(xr[idx+offset])||floor(xi[idx])!=floor(xi[idx+offset]);}
DISC_CQ_I(bitwise_shift_left){IDX; disc[idx]=floor(xr[idx])!=floor(xr[idx+offset])||floor(xi[idx])!=floor(xi[idx+offset]);}
DISC_QR_I(bitwise_shift_left){IDX; disc[idx]=floor(xr[idx])!=floor(xr[idx+offset])||floor(xi[idx])!=floor(xi[idx+offset])||floor(xj[idx])!=floor(xj[idx+offset])||floor(xk[idx])!=floor(xk[idx+offset]);}
DISC_QC_I(bitwise_shift_left){IDX; disc[idx]=floor(xr[idx])!=floor(xr[idx+offset])||floor(xi[idx])!=floor(xi[idx+offset])||floor(xj[idx])!=floor(xj[idx+offset])||floor(xk[idx])!=floor(xk[idx+offset]);}
DISC_QQ_I(bitwise_shift_left){IDX; disc[idx]=floor(xr[idx])!=floor(xr[idx+offset])||floor(xi[idx])!=floor(xi[idx+offset])||floor(xj[idx])!=floor(xj[idx+offset])||floor(xk[idx])!=floor(xk[idx+offset]);}

G2_R_R(bitwise_shift_right_l){IDX; ASSIGN_R(exp(-floor(xr[idx])*_ln2));}
G2_C_C(bitwise_shift_right_l)
{
	IDX;
	float2 ret=exp_c(mul_cr(floor_c(VEC2(x)), -_ln2));
	RET_C;
}
G2_Q_Q(bitwise_shift_right_l)
{
	IDX;
	float4 ret=exp_q(mul_qr(floor_q(VEC4(x)), -_ln2));
	RET_Q;
}
G2_R_R(bitwise_shift_right_r){IDX; ASSIGN_R(xr[idx]*0.5f);}
G2_C_C(bitwise_shift_right_r)
{
	IDX;
	float2 ret=VEC2(x)*0.5f;
	RET_C;
}
G2_Q_Q(bitwise_shift_right_r)
{
	IDX;
	float4 ret=VEC4(x)*0.5f;
	RET_Q;
}
G2_R_RR(bitwise_shift_right){IDX; ASSIGN_R(xr[idx]*exp(-floor(yr[idx])*_ln2));}
G2_C_RC(bitwise_shift_right)
{
	IDX;
	float a=xr[idx];
	float2 b=VEC2(y);
	float2 ret=mul_rc(a, exp_c(mul_cr(floor_c(b), -_ln2)));
	RET_C;
}
G2_Q_RQ(bitwise_shift_right)
{
	IDX;
	float a=xr[idx];
	float4 b=VEC4(y);
	float4 ret=mul_rq(a, exp_q(mul_qr(floor_q(b), -_ln2)));
	RET_Q;
}
G2_C_CR(bitwise_shift_right)
{
	IDX;
	float2 a=VEC2(x);
	float b=yr[idx];
	float2 ret=mul_cr(a, exp(-floor(b)*_ln2));
	RET_C;
}
G2_C_CC(bitwise_shift_right)
{
	IDX;
	float2 a=VEC2(x);
	float2 b=VEC2(y);
	float2 ret=mul_cc(a, exp_c(mul_cr(floor_c(b), -_ln2)));
	RET_C;
}
G2_Q_CQ(bitwise_shift_right)
{
	IDX;
	float2 a=VEC2(x);
	float4 b=VEC4(y);
	float4 ret=mul_cq(a, exp_q(mul_qr(floor_q(b), -_ln2)));
	RET_Q;
}
G2_Q_QR(bitwise_shift_right)
{
	IDX;
	float4 a=VEC4(x);
	float b=yr[idx];
	float4 ret=mul_qr(a, exp(-floor(b)*_ln2));
	RET_Q;
}
G2_Q_QC(bitwise_shift_right)
{
	IDX;
	float4 a=VEC4(x);
	float2 b=VEC2(y);
	float4 ret=mul_qc(a, exp_c(mul_cr(floor_c(b), -_ln2)));
	RET_Q;
}
G2_Q_QQ(bitwise_shift_right)
{
	IDX;
	float4 a=VEC4(x);
	float4 b=VEC4(y);
	float4 ret=mul_qq(a, exp_q(mul_qr(floor_q(b), -_ln2)));
	RET_Q;
}
DISC_R_O(bitwise_shift_right_l){IDX; disc[idx]=xr[idx]!=xr[idx+offset];}
DISC_C_O(bitwise_shift_right_l){IDX; disc[idx]=xr[idx]!=xr[idx+offset]||xi[idx]!=xi[idx+offset];}
DISC_Q_O(bitwise_shift_right_l){IDX; disc[idx]=xr[idx]!=xr[idx+offset]||xi[idx]!=xi[idx+offset]||xj[idx]!=xj[idx+offset]||xk[idx]!=xk[idx+offset];}
DISC_RR_I(bitwise_shift_right){disc_rr_bitwise_shift_left_i(size, offset, disc, xr, yr);}
DISC_RC_I(bitwise_shift_right){disc_rc_bitwise_shift_left_i(size, offset, disc, xr, yr, yi);}
DISC_RQ_I(bitwise_shift_right){disc_rq_bitwise_shift_left_i(size, offset, disc, xr, yr, yi, yj, yk);}
DISC_CR_I(bitwise_shift_right){disc_cr_bitwise_shift_left_i(size, offset, disc, xr, xi, yr);}
DISC_CC_I(bitwise_shift_right){disc_cc_bitwise_shift_left_i(size, offset, disc, xr, xi, yr, yi);}
DISC_CQ_I(bitwise_shift_right){disc_cq_bitwise_shift_left_i(size, offset, disc, xr, xi, yr, yi, yj, yk);}
DISC_QR_I(bitwise_shift_right){disc_qr_bitwise_shift_left_i(size, offset, disc, xr, xi, xj, xk, yr);}
DISC_QC_I(bitwise_shift_right){disc_qc_bitwise_shift_left_i(size, offset, disc, xr, xi, xj, xk, yr, yi);}
DISC_QQ_I(bitwise_shift_right){disc_qq_bitwise_shift_left_i(size, offset, disc, xr, xi, xj, xk, yr, yi, yj, yk);}
)CLSRC";
	static const char program05[]=R"CLSRC(
float bitwise_not(float x){return iscastable2int(x)?~f2i(x):NAN;}
G2_R_R(bitwise_not){IDX; float a=xr[idx]; ASSIGN_R(bitwise_not(a));}
G2_C_C(bitwise_not){IDX; float2 a=VEC2(x); ASSIGN_C(bitwise_not(a.x), bitwise_not(a.y));}
G2_Q_Q(bitwise_not){IDX; float4 a=VEC4(x); ASSIGN_Q(bitwise_not(a.x), bitwise_not(a.y), bitwise_not(a.z), bitwise_not(a.w));}
DISC_R_I(bitwise_not){IDX; disc[idx]=sign(xr[idx])!=sign(xr[idx+offset]);}
DISC_C_I(bitwise_not){IDX; disc[idx]=sign(xr[idx])!=sign(xr[idx+offset])||sign(xi[idx])!=sign(xi[idx+offset]);}
DISC_Q_I(bitwise_not){IDX; disc[idx]=sign(xr[idx])!=sign(xr[idx+offset])||sign(xi[idx])!=sign(xi[idx+offset])||sign(xj[idx])!=sign(xj[idx+offset])||sign(xk[idx])!=sign(xk[idx+offset]);}

float bitwise_and1(float x){return iscastable2int(x)?f2i(x)==-1:NAN;}
float bitwise_and2(float a, float b){return iscastable2int(a)&&iscastable2int(b)?f2i(a)&f2i(b):NAN;}
G2_R_R(bitwise_and){IDX; ASSIGN_R(bitwise_and1(xr[idx]));}
G2_C_C(bitwise_and){IDX; ASSIGN_C(bitwise_and1(xr[idx]), bitwise_and1(xi[idx]));}
G2_Q_Q(bitwise_and){IDX; ASSIGN_Q(bitwise_and1(xr[idx]), bitwise_and1(xi[idx]), bitwise_and1(xj[idx]), bitwise_and1(xk[idx]));}
G2_R_RR(bitwise_and){IDX; ASSIGN_R(bitwise_and2(xr[idx], yr[idx]));}
G2_C_RC(bitwise_and){IDX; ASSIGN_C(bitwise_and2(xr[idx], yr[idx]), 0);}
G2_Q_RQ(bitwise_and){IDX; ASSIGN_Q(bitwise_and2(xr[idx], yr[idx]), 0, 0, 0);}
G2_C_CR(bitwise_and){IDX; ASSIGN_C(bitwise_and2(xr[idx], yr[idx]), 0);}
G2_C_CC(bitwise_and){IDX; ASSIGN_C(bitwise_and2(xr[idx], yr[idx]), bitwise_and2(xi[idx], yi[idx]));}
G2_Q_CQ(bitwise_and){IDX; ASSIGN_Q(bitwise_and2(xr[idx], yr[idx]), bitwise_and2(xi[idx], yi[idx]), 0, 0);}
G2_Q_QR(bitwise_and){IDX; ASSIGN_Q(bitwise_and2(xr[idx], yr[idx]), 0, 0, 0);}
G2_Q_QC(bitwise_and){IDX; ASSIGN_Q(bitwise_and2(xr[idx], yr[idx]), bitwise_and2(xi[idx], yi[idx]), 0, 0);}
G2_Q_QQ(bitwise_and){IDX; ASSIGN_Q(bitwise_and2(xr[idx], yr[idx]), bitwise_and2(xi[idx], yi[idx]), bitwise_and2(xj[idx], yj[idx]), bitwise_and2(xk[idx], yk[idx]));}
DISC_R_O(bitwise_and){IDX; disc[idx]=xr[idx]!=xr[idx+offset];}//repeat r,c,q for R_RR...Q_QQ
DISC_C_O(bitwise_and){IDX; disc[idx]=xr[idx]!=xr[idx+offset]||xi[idx]!=xi[idx+offset];}
DISC_Q_O(bitwise_and){IDX; disc[idx]=xr[idx]!=xr[idx+offset]||xi[idx]!=xi[idx+offset]||xj[idx]!=xj[idx+offset]||xk[idx]!=xk[idx+offset];}

float bitwise_nand1(float x){return iscastable2int(x)?f2i(x)!=-1:NAN;}
float bitwise_nand2(float a, float b){return iscastable2int(a)&&iscastable2int(b)?~(f2i(a)&f2i(b)):NAN;}
G2_R_R(bitwise_nand){IDX; ASSIGN_R(bitwise_nand1(xr[idx]));}
G2_C_C(bitwise_nand){IDX; ASSIGN_C(bitwise_nand1(xr[idx]), bitwise_nand1(xi[idx]));}
G2_Q_Q(bitwise_nand){IDX; ASSIGN_Q(bitwise_nand1(xr[idx]), bitwise_nand1(xi[idx]), bitwise_nand1(xj[idx]), bitwise_nand1(xk[idx]));}
G2_R_RR(bitwise_nand){IDX; ASSIGN_R(bitwise_nand2(xr[idx], yr[idx]));}
G2_C_RC(bitwise_nand){IDX; ASSIGN_C(bitwise_nand2(xr[idx], yr[idx]), 0);}
G2_Q_RQ(bitwise_nand){IDX; ASSIGN_Q(bitwise_nand2(xr[idx], yr[idx]), 0, 0, 0);}
G2_C_CR(bitwise_nand){IDX; ASSIGN_C(bitwise_nand2(xr[idx], yr[idx]), 0);}
G2_C_CC(bitwise_nand){IDX; ASSIGN_C(bitwise_nand2(xr[idx], yr[idx]), bitwise_nand2(xi[idx], yi[idx]));}
G2_Q_CQ(bitwise_nand){IDX; ASSIGN_Q(bitwise_nand2(xr[idx], yr[idx]), bitwise_nand2(xi[idx], yi[idx]), 0, 0);}
G2_Q_QR(bitwise_nand){IDX; ASSIGN_Q(bitwise_nand2(xr[idx], yr[idx]), 0, 0, 0);}
G2_Q_QC(bitwise_nand){IDX; ASSIGN_Q(bitwise_nand2(xr[idx], yr[idx]), bitwise_nand2(xi[idx], yi[idx]), 0, 0);}
G2_Q_QQ(bitwise_nand){IDX; ASSIGN_Q(bitwise_nand2(xr[idx], yr[idx]), bitwise_nand2(xi[idx], yi[idx]), bitwise_nand2(xj[idx], yj[idx]), bitwise_nand2(xk[idx], yk[idx]));}
DISC_R_O(bitwise_nand){IDX; disc[idx]=xr[idx]!=xr[idx+offset];}//repeat r,c,q for R_RR...Q_QQ
DISC_C_O(bitwise_nand){IDX; disc[idx]=xr[idx]!=xr[idx+offset]||xi[idx]!=xi[idx+offset];}
DISC_Q_O(bitwise_nand){IDX; disc[idx]=xr[idx]!=xr[idx+offset]||xi[idx]!=xi[idx+offset]||xj[idx]!=xj[idx+offset]||xk[idx]!=xk[idx+offset];}

float bitwise_or1(float x){return iscastable2int(x)?f2i(x)!=0:NAN;}
float bitwise_or2(float a, float b){return iscastable2int(a)&&iscastable2int(b)?f2i(a)|f2i(b):NAN;}
G2_R_R(bitwise_or){IDX; ASSIGN_R(bitwise_or1(xr[idx]));}
G2_C_C(bitwise_or){IDX; ASSIGN_C(bitwise_or1(xr[idx]), bitwise_or1(xi[idx]));}
G2_Q_Q(bitwise_or){IDX; ASSIGN_Q(bitwise_or1(xr[idx]), bitwise_or1(xi[idx]), bitwise_or1(xj[idx]), bitwise_or1(xk[idx]));}
G2_R_RR(bitwise_or){IDX; ASSIGN_R(bitwise_or2(xr[idx], yr[idx]));}
G2_C_RC(bitwise_or){IDX; ASSIGN_C(bitwise_or2(xr[idx], yr[idx]), yi[idx]);}
G2_Q_RQ(bitwise_or){IDX; ASSIGN_Q(bitwise_or2(xr[idx], yr[idx]), yi[idx], yj[idx], yk[idx]);}
G2_C_CR(bitwise_or){IDX; ASSIGN_C(bitwise_or2(xr[idx], yr[idx]), xi[idx]);}
G2_C_CC(bitwise_or){IDX; ASSIGN_C(bitwise_or2(xr[idx], yr[idx]), bitwise_or2(xi[idx], yi[idx]));}
G2_Q_CQ(bitwise_or){IDX; ASSIGN_Q(bitwise_or2(xr[idx], yr[idx]), bitwise_or2(xi[idx], yi[idx]), yj[idx], yk[idx]);}
G2_Q_QR(bitwise_or){IDX; ASSIGN_Q(bitwise_or2(xr[idx], yr[idx]), xi[idx], xj[idx], xk[idx]);}
G2_Q_QC(bitwise_or){IDX; ASSIGN_Q(bitwise_or2(xr[idx], yr[idx]), bitwise_or2(xi[idx], yi[idx]), xj[idx], xk[idx]);}
G2_Q_QQ(bitwise_or){IDX; ASSIGN_Q(bitwise_or2(xr[idx], yr[idx]), bitwise_or2(xi[idx], yi[idx]), bitwise_or2(xj[idx], yj[idx]), bitwise_or2(xk[idx], yk[idx]));}
DISC_R_O(bitwise_or){IDX; disc[idx]=xr[idx]!=xr[idx+offset];}//repeat r,c,q for R_RR...Q_QQ
DISC_C_O(bitwise_or){IDX; disc[idx]=xr[idx]!=xr[idx+offset]||xi[idx]!=xi[idx+offset];}
DISC_Q_O(bitwise_or){IDX; disc[idx]=xr[idx]!=xr[idx+offset]||xi[idx]!=xi[idx+offset]||xj[idx]!=xj[idx+offset]||xk[idx]!=xk[idx+offset];}
)CLSRC";
	static const char program06[]=R"CLSRC(
float bitwise_nor1(float x){return iscastable2int(x)?!f2i(x):NAN;}
float bitwise_nor2(float a, float b){return iscastable2int(a)&&iscastable2int(b)?~(f2i(a)|f2i(b)):NAN;}
G2_R_R(bitwise_nor){IDX; ASSIGN_R(bitwise_nor1(xr[idx]));}
G2_C_C(bitwise_nor){IDX; ASSIGN_C(bitwise_nor1(xr[idx]), bitwise_nor1(xi[idx]));}
G2_Q_Q(bitwise_nor){IDX; ASSIGN_Q(bitwise_nor1(xr[idx]), bitwise_nor1(xi[idx]), bitwise_nor1(xj[idx]), bitwise_nor1(xk[idx]));}
G2_R_RR(bitwise_nor){IDX; ASSIGN_R(bitwise_nor2(xr[idx], yr[idx]));}
G2_C_RC(bitwise_nor){IDX; ASSIGN_C(bitwise_nor2(xr[idx], yr[idx]), bitwise_nor2(0, yi[idx]));}
G2_Q_RQ(bitwise_nor){IDX; ASSIGN_Q(bitwise_nor2(xr[idx], yr[idx]), bitwise_nor2(0, yi[idx]), bitwise_nor2(0, yj[idx]), bitwise_nor2(0, yk[idx]));}
G2_C_CR(bitwise_nor){IDX; ASSIGN_C(bitwise_nor2(xr[idx], yr[idx]), bitwise_nor2(xi[idx], 0));}
G2_C_CC(bitwise_nor){IDX; ASSIGN_C(bitwise_nor2(xr[idx], yr[idx]), bitwise_nor2(xi[idx], yi[idx]));}
G2_Q_CQ(bitwise_nor){IDX; ASSIGN_Q(bitwise_nor2(xr[idx], yr[idx]), bitwise_nor2(xi[idx], yi[idx]), bitwise_nor2(0, yj[idx]), bitwise_nor2(0, yk[idx]));}
G2_Q_QR(bitwise_nor){IDX; ASSIGN_Q(bitwise_nor2(xr[idx], yr[idx]), bitwise_nor2(xi[idx], 0), bitwise_nor2(xj[idx], 0), bitwise_nor2(xk[idx], 0));}
G2_Q_QC(bitwise_nor){IDX; ASSIGN_Q(bitwise_nor2(xr[idx], yr[idx]), bitwise_nor2(xi[idx], yi[idx]), bitwise_nor2(xj[idx], 0), bitwise_nor2(xk[idx], 0));}
G2_Q_QQ(bitwise_nor){IDX; ASSIGN_Q(bitwise_nor2(xr[idx], yr[idx]), bitwise_nor2(xi[idx], yi[idx]), bitwise_nor2(xj[idx], yj[idx]), bitwise_nor2(xk[idx], yk[idx]));}
DISC_R_O(bitwise_nor){IDX; disc[idx]=xr[idx]!=xr[idx+offset];}//repeat r,c,q for R_RR...Q_QQ
DISC_C_O(bitwise_nor){IDX; disc[idx]=xr[idx]!=xr[idx+offset]||xi[idx]!=xi[idx+offset];}
DISC_Q_O(bitwise_nor){IDX; disc[idx]=xr[idx]!=xr[idx+offset]||xi[idx]!=xi[idx+offset]||xj[idx]!=xj[idx+offset]||xk[idx]!=xk[idx+offset];}

float bitwise_xor1(float x)
{
	if(iscastable2int(x))
	{
		int a=f2i(x);
		a^=a>>16, a^=a>>8, a^=a>>4;
		a&=15;
		return (0x6996>>a)&1;
	}
	return NAN;
}
float bitwise_xor2(float a, float b){return iscastable2int(a)&&iscastable2int(b)?f2i(a)^f2i(b):NAN;}
G2_R_R(bitwise_xor){IDX; ASSIGN_R(bitwise_xor1(xr[idx]));}
G2_C_C(bitwise_xor){IDX; ASSIGN_C(bitwise_xor1(xr[idx]), bitwise_xor1(xi[idx]));}
G2_Q_Q(bitwise_xor){IDX; ASSIGN_Q(bitwise_xor1(xr[idx]), bitwise_xor1(xi[idx]), bitwise_xor1(xj[idx]), bitwise_xor1(xk[idx]));}
G2_R_RR(bitwise_xor){IDX; ASSIGN_R(bitwise_xor2(xr[idx], yr[idx]));}
G2_C_RC(bitwise_xor){IDX; ASSIGN_C(bitwise_xor2(xr[idx], yr[idx]), yi[idx]);}
G2_Q_RQ(bitwise_xor){IDX; ASSIGN_Q(bitwise_xor2(xr[idx], yr[idx]), yi[idx], yj[idx], yk[idx]);}
G2_C_CR(bitwise_xor){IDX; ASSIGN_C(bitwise_xor2(xr[idx], yr[idx]), xi[idx]);}
G2_C_CC(bitwise_xor){IDX; ASSIGN_C(bitwise_xor2(xr[idx], yr[idx]), bitwise_xor2(xi[idx], yi[idx]));}
G2_Q_CQ(bitwise_xor){IDX; ASSIGN_Q(bitwise_xor2(xr[idx], yr[idx]), bitwise_xor2(xi[idx], yi[idx]), yj[idx], yk[idx]);}
G2_Q_QR(bitwise_xor){IDX; ASSIGN_Q(bitwise_xor2(xr[idx], yr[idx]), xi[idx], xj[idx], xk[idx]);}
G2_Q_QC(bitwise_xor){IDX; ASSIGN_Q(bitwise_xor2(xr[idx], yr[idx]), bitwise_xor2(xi[idx], yi[idx]), xj[idx], xk[idx]);}
G2_Q_QQ(bitwise_xor){IDX; ASSIGN_Q(bitwise_xor2(xr[idx], yr[idx]), bitwise_xor2(xi[idx], yi[idx]), bitwise_xor2(xj[idx], yj[idx]), bitwise_xor2(xk[idx], yk[idx]));}
DISC_R_O(bitwise_xor){IDX; disc[idx]=xr[idx]!=xr[idx+offset];}//repeat r,c,q for R_RR...Q_QQ
DISC_C_O(bitwise_xor){IDX; disc[idx]=xr[idx]!=xr[idx+offset]||xi[idx]!=xi[idx+offset];}
DISC_Q_O(bitwise_xor){IDX; disc[idx]=xr[idx]!=xr[idx+offset]||xi[idx]!=xi[idx+offset]||xj[idx]!=xj[idx+offset]||xk[idx]!=xk[idx+offset];}

float bitwise_xnor1(float x)
{
	if(iscastable2int(x))
	{
		int a=f2i(x);
		a^=a>>16, a^=a>>8, a^=a>>4;
		a&=15;
		return !((0x6996>>a)&1);
	}
	return NAN;
}
float bitwise_xnor2(float a, float b){return iscastable2int(a)&&iscastable2int(b)?~(f2i(a)^f2i(b)):NAN;}
G2_R_R(bitwise_xnor){IDX; ASSIGN_R(bitwise_xnor1(xr[idx]));}
G2_C_C(bitwise_xnor){IDX; ASSIGN_C(bitwise_xnor1(xr[idx]), bitwise_xnor1(xi[idx]));}
G2_Q_Q(bitwise_xnor){IDX; ASSIGN_Q(bitwise_xnor1(xr[idx]), bitwise_xnor1(xi[idx]), bitwise_xnor1(xj[idx]), bitwise_xnor1(xk[idx]));}
G2_R_RR(bitwise_xnor){IDX; ASSIGN_R(bitwise_xnor2(xr[idx], yr[idx]));}
G2_C_RC(bitwise_xnor){IDX; ASSIGN_C(bitwise_xnor2(xr[idx], yr[idx]), bitwise_xnor2(0, yi[idx]));}
G2_Q_RQ(bitwise_xnor){IDX; ASSIGN_Q(bitwise_xnor2(xr[idx], yr[idx]), bitwise_xnor2(0, yi[idx]), bitwise_xnor2(0, yj[idx]), bitwise_xnor2(0, yk[idx]));}
G2_C_CR(bitwise_xnor){IDX; ASSIGN_C(bitwise_xnor2(xr[idx], yr[idx]), bitwise_xnor2(xi[idx], 0));}
G2_C_CC(bitwise_xnor){IDX; ASSIGN_C(bitwise_xnor2(xr[idx], yr[idx]), bitwise_xnor2(xi[idx], yi[idx]));}
G2_Q_CQ(bitwise_xnor){IDX; ASSIGN_Q(bitwise_xnor2(xr[idx], yr[idx]), bitwise_xnor2(xi[idx], yi[idx]), bitwise_xnor2(0, yj[idx]), bitwise_xnor2(0, yk[idx]));}
G2_Q_QR(bitwise_xnor){IDX; ASSIGN_Q(bitwise_xnor2(xr[idx], yr[idx]), bitwise_xnor2(xi[idx], 0), bitwise_xnor2(xj[idx], 0), bitwise_xnor2(xk[idx], 0));}
G2_Q_QC(bitwise_xnor){IDX; ASSIGN_Q(bitwise_xnor2(xr[idx], yr[idx]), bitwise_xnor2(xi[idx], yi[idx]), bitwise_xnor2(xj[idx], 0), bitwise_xnor2(xk[idx], 0));}
G2_Q_QQ(bitwise_xnor){IDX; ASSIGN_Q(bitwise_xnor2(xr[idx], yr[idx]), bitwise_xnor2(xi[idx], yi[idx]), bitwise_xnor2(xj[idx], yj[idx]), bitwise_xnor2(xk[idx], yk[idx]));}
DISC_R_O(bitwise_xnor){IDX; disc[idx]=xr[idx]!=xr[idx+offset];}//repeat r,c,q for R_RR...Q_QQ
DISC_C_O(bitwise_xnor){IDX; disc[idx]=xr[idx]!=xr[idx+offset]||xi[idx]!=xi[idx+offset];}
DISC_Q_O(bitwise_xnor){IDX; disc[idx]=xr[idx]!=xr[idx+offset]||xi[idx]!=xi[idx+offset]||xj[idx]!=xj[idx+offset]||xk[idx]!=xk[idx+offset];}
)CLSRC";
	static const char program07[]=R"CLSRC(
G2_R_R(logic_equal){IDX; ASSIGN_R(xr[idx]==0);}
G2_R_C(logic_equal){IDX; ASSIGN_R(!istrue_c(VEC2(x)));}
G2_R_Q(logic_equal){IDX; ASSIGN_R(!istrue_q(VEC4(x)));}
G2_R_RR(logic_equal){IDX; ASSIGN_R(xr[idx]==yr[idx]);}
G2_R_RC(logic_equal){IDX; ASSIGN_R(equal_rc(xr[idx], VEC2(y)));}
G2_R_RQ(logic_equal){IDX; ASSIGN_R(equal_rq(xr[idx], VEC4(y)));}
G2_R_CR(logic_equal){IDX; ASSIGN_R(equal_cr(VEC2(x), yr[idx]));}
G2_R_CC(logic_equal){IDX; ASSIGN_R(equal_cc(VEC2(x), VEC2(y)));}
G2_R_CQ(logic_equal){IDX; ASSIGN_R(equal_cq(VEC2(x), VEC4(y)));}
G2_R_QR(logic_equal){IDX; ASSIGN_R(equal_qr(VEC4(x), yr[idx]));}
G2_R_QC(logic_equal){IDX; ASSIGN_R(equal_qc(VEC4(x), VEC2(y)));}
G2_R_QQ(logic_equal){IDX; ASSIGN_R(equal_qq(VEC4(x), VEC4(y)));}
DISC_R_O(logic_equal){IDX; disc[idx]=xr[idx]!=xr[idx+offset];}

G2_R_R(logic_not_equal){IDX; ASSIGN_R(xr[idx]!=0);}
G2_R_C(logic_not_equal){IDX; ASSIGN_R(istrue_c(VEC2(x)));}
G2_R_Q(logic_not_equal){IDX; ASSIGN_R(istrue_q(VEC4(x)));}
G2_R_RR(logic_not_equal){IDX; ASSIGN_R(xr[idx]!=yr[idx]);}
G2_R_RC(logic_not_equal){IDX; ASSIGN_R(!equal_rc(xr[idx], VEC2(y)));}
G2_R_RQ(logic_not_equal){IDX; ASSIGN_R(!equal_rq(xr[idx], VEC4(y)));}
G2_R_CR(logic_not_equal){IDX; ASSIGN_R(!equal_cr(VEC2(x), yr[idx]));}
G2_R_CC(logic_not_equal){IDX; ASSIGN_R(!equal_cc(VEC2(x), VEC2(y)));}
G2_R_CQ(logic_not_equal){IDX; ASSIGN_R(!equal_cq(VEC2(x), VEC4(y)));}
G2_R_QR(logic_not_equal){IDX; ASSIGN_R(!equal_qr(VEC4(x), yr[idx]));}
G2_R_QC(logic_not_equal){IDX; ASSIGN_R(!equal_qc(VEC4(x), VEC2(y)));}
G2_R_QQ(logic_not_equal){IDX; ASSIGN_R(!equal_qq(VEC4(x), VEC4(y)));}
DISC_R_O(logic_not_equal){IDX; disc[idx]=xr[idx]!=xr[idx+offset];}

G2_R_R(logic_less_l){IDX; ASSIGN_R(0<xr[idx]);}
G2_R_C(logic_less_l){IDX; ASSIGN_R(0<xr[idx]);}
G2_R_Q(logic_less_l){IDX; ASSIGN_R(0<xr[idx]);}
G2_R_R(logic_less_r){IDX; ASSIGN_R(xr[idx]<0);}
G2_R_C(logic_less_r){IDX; ASSIGN_R(xr[idx]<0);}
G2_R_Q(logic_less_r){IDX; ASSIGN_R(xr[idx]<0);}
G2_R_RR(logic_less){IDX; ASSIGN_R(xr[idx]<yr[idx]);}
G2_R_RC(logic_less){IDX; ASSIGN_R(xr[idx]<yr[idx]);}
G2_R_RQ(logic_less){IDX; ASSIGN_R(xr[idx]<yr[idx]);}
G2_R_CR(logic_less){IDX; ASSIGN_R(xr[idx]<yr[idx]);}
G2_R_CC(logic_less){IDX; ASSIGN_R(xr[idx]<yr[idx]);}
G2_R_CQ(logic_less){IDX; ASSIGN_R(xr[idx]<yr[idx]);}
G2_R_QR(logic_less){IDX; ASSIGN_R(xr[idx]<yr[idx]);}
G2_R_QC(logic_less){IDX; ASSIGN_R(xr[idx]<yr[idx]);}
G2_R_QQ(logic_less){IDX; ASSIGN_R(xr[idx]<yr[idx]);}
DISC_R_O(logic_less){IDX; disc[idx]=xr[idx]!=xr[idx+offset];}
)CLSRC";
	static const char program08[]=R"CLSRC(
G2_R_R(logic_less_equal_l){IDX; ASSIGN_R(0<=xr[idx]);}
G2_R_C(logic_less_equal_l){IDX; ASSIGN_R(0<=xr[idx]);}
G2_R_Q(logic_less_equal_l){IDX; ASSIGN_R(0<=xr[idx]);}
G2_R_R(logic_less_equal_r){IDX; ASSIGN_R(xr[idx]<=0);}
G2_R_C(logic_less_equal_r){IDX; ASSIGN_R(xr[idx]<=0);}
G2_R_Q(logic_less_equal_r){IDX; ASSIGN_R(xr[idx]<=0);}
G2_R_RR(logic_less_equal){IDX; ASSIGN_R(xr[idx]<=yr[idx]);}
G2_R_RC(logic_less_equal){IDX; ASSIGN_R(xr[idx]<=yr[idx]);}
G2_R_RQ(logic_less_equal){IDX; ASSIGN_R(xr[idx]<=yr[idx]);}
G2_R_CR(logic_less_equal){IDX; ASSIGN_R(xr[idx]<=yr[idx]);}
G2_R_CC(logic_less_equal){IDX; ASSIGN_R(xr[idx]<=yr[idx]);}
G2_R_CQ(logic_less_equal){IDX; ASSIGN_R(xr[idx]<=yr[idx]);}
G2_R_QR(logic_less_equal){IDX; ASSIGN_R(xr[idx]<=yr[idx]);}
G2_R_QC(logic_less_equal){IDX; ASSIGN_R(xr[idx]<=yr[idx]);}
G2_R_QQ(logic_less_equal){IDX; ASSIGN_R(xr[idx]<=yr[idx]);}
DISC_R_O(logic_less_equal){IDX; disc[idx]=xr[idx]!=xr[idx+offset];}

G2_R_R(logic_greater_l){IDX; ASSIGN_R(0>xr[idx]);}
G2_R_C(logic_greater_l){IDX; ASSIGN_R(0>xr[idx]);}
G2_R_Q(logic_greater_l){IDX; ASSIGN_R(0>xr[idx]);}
G2_R_R(logic_greater_r){IDX; ASSIGN_R(xr[idx]>0);}
G2_R_C(logic_greater_r){IDX; ASSIGN_R(xr[idx]>0);}
G2_R_Q(logic_greater_r){IDX; ASSIGN_R(xr[idx]>0);}
G2_R_RR(logic_greater){IDX; ASSIGN_R(xr[idx]>yr[idx]);}
G2_R_RC(logic_greater){IDX; ASSIGN_R(xr[idx]>yr[idx]);}
G2_R_RQ(logic_greater){IDX; ASSIGN_R(xr[idx]>yr[idx]);}
G2_R_CR(logic_greater){IDX; ASSIGN_R(xr[idx]>yr[idx]);}
G2_R_CC(logic_greater){IDX; ASSIGN_R(xr[idx]>yr[idx]);}
G2_R_CQ(logic_greater){IDX; ASSIGN_R(xr[idx]>yr[idx]);}
G2_R_QR(logic_greater){IDX; ASSIGN_R(xr[idx]>yr[idx]);}
G2_R_QC(logic_greater){IDX; ASSIGN_R(xr[idx]>yr[idx]);}
G2_R_QQ(logic_greater){IDX; ASSIGN_R(xr[idx]>yr[idx]);}
DISC_R_O(logic_greater){IDX; disc[idx]=xr[idx]!=xr[idx+offset];}

G2_R_R(logic_greater_equal_l){IDX; ASSIGN_R(0>=xr[idx]);}
G2_R_C(logic_greater_equal_l){IDX; ASSIGN_R(0>=xr[idx]);}
G2_R_Q(logic_greater_equal_l){IDX; ASSIGN_R(0>=xr[idx]);}
G2_R_R(logic_greater_equal_r){IDX; ASSIGN_R(xr[idx]>=0);}
G2_R_C(logic_greater_equal_r){IDX; ASSIGN_R(xr[idx]>=0);}
G2_R_Q(logic_greater_equal_r){IDX; ASSIGN_R(xr[idx]>=0);}
G2_R_RR(logic_greater_equal){IDX; ASSIGN_R(xr[idx]>=yr[idx]);}
G2_R_RC(logic_greater_equal){IDX; ASSIGN_R(xr[idx]>=yr[idx]);}
G2_R_RQ(logic_greater_equal){IDX; ASSIGN_R(xr[idx]>=yr[idx]);}
G2_R_CR(logic_greater_equal){IDX; ASSIGN_R(xr[idx]>=yr[idx]);}
G2_R_CC(logic_greater_equal){IDX; ASSIGN_R(xr[idx]>=yr[idx]);}
G2_R_CQ(logic_greater_equal){IDX; ASSIGN_R(xr[idx]>=yr[idx]);}
G2_R_QR(logic_greater_equal){IDX; ASSIGN_R(xr[idx]>=yr[idx]);}
G2_R_QC(logic_greater_equal){IDX; ASSIGN_R(xr[idx]>=yr[idx]);}
G2_R_QQ(logic_greater_equal){IDX; ASSIGN_R(xr[idx]>=yr[idx]);}
DISC_R_O(logic_greater_equal){IDX; disc[idx]=xr[idx]!=xr[idx+offset];}
)CLSRC";
	static const char program09[]=R"CLSRC(
G2_R_R(logic_not){IDX; ASSIGN_R(xr[idx]==0);}
G2_R_C(logic_not){IDX; ASSIGN_R(!istrue_c(VEC2(x)));}
G2_R_Q(logic_not){IDX; ASSIGN_R(!istrue_q(VEC4(x)));}
DISC_R_O(logic_not){IDX; disc[idx]=xr[idx]!=xr[idx+offset];}

G2_R_RR(logic_and){IDX; ASSIGN_R(xr[idx]&&yr[idx]);}
G2_R_RC(logic_and){IDX; ASSIGN_R(xr[idx]&&istrue_c(VEC2(y)));}
G2_R_RQ(logic_and){IDX; ASSIGN_R(xr[idx]&&istrue_q(VEC4(y)));}
G2_R_CR(logic_and){IDX; ASSIGN_R(istrue_c(VEC2(x))&&yr[idx]);}
G2_R_CC(logic_and){IDX; ASSIGN_R(istrue_c(VEC2(x))&&istrue_c(VEC2(y)));}
G2_R_CQ(logic_and){IDX; ASSIGN_R(istrue_c(VEC2(x))&&istrue_q(VEC4(y)));}
G2_R_QR(logic_and){IDX; ASSIGN_R(istrue_q(VEC4(x))&&yr[idx]);}
G2_R_QC(logic_and){IDX; ASSIGN_R(istrue_q(VEC4(x))&&istrue_c(VEC2(y)));}
G2_R_QQ(logic_and){IDX; ASSIGN_R(istrue_q(VEC4(x))&&istrue_q(VEC4(y)));}
DISC_R_O(logic_and){IDX; disc[idx]=xr[idx]!=xr[idx+offset];}

G2_R_RR(logic_or){IDX; ASSIGN_R(xr[idx]||yr[idx]);}
G2_R_RC(logic_or){IDX; ASSIGN_R(xr[idx]||istrue_c(VEC2(y)));}
G2_R_RQ(logic_or){IDX; ASSIGN_R(xr[idx]||istrue_q(VEC4(y)));}
G2_R_CR(logic_or){IDX; ASSIGN_R(istrue_c(VEC2(x))||yr[idx]);}
G2_R_CC(logic_or){IDX; ASSIGN_R(istrue_c(VEC2(x))||istrue_c(VEC2(y)));}
G2_R_CQ(logic_or){IDX; ASSIGN_R(istrue_c(VEC2(x))||istrue_q(VEC4(y)));}
G2_R_QR(logic_or){IDX; ASSIGN_R(istrue_q(VEC4(x))||yr[idx]);}
G2_R_QC(logic_or){IDX; ASSIGN_R(istrue_q(VEC4(x))||istrue_c(VEC2(y)));}
G2_R_QQ(logic_or){IDX; ASSIGN_R(istrue_q(VEC4(x))||istrue_q(VEC4(y)));}
DISC_R_O(logic_or){IDX; disc[idx]=xr[idx]!=xr[idx+offset];}

G2_R_RR(logic_xor){IDX; ASSIGN_R((xr[idx]!=0)^(yr[idx]!=0));}
G2_R_RC(logic_xor){IDX; ASSIGN_R((xr[idx]!=0)^istrue_c(VEC2(y)));}
G2_R_RQ(logic_xor){IDX; ASSIGN_R((xr[idx]!=0)^istrue_q(VEC4(y)));}
G2_R_CR(logic_xor){IDX; ASSIGN_R(istrue_c(VEC2(x))^(yr[idx]!=0));}
G2_R_CC(logic_xor){IDX; ASSIGN_R(istrue_c(VEC2(x))^istrue_c(VEC2(y)));}
G2_R_CQ(logic_xor){IDX; ASSIGN_R(istrue_c(VEC2(x))^istrue_q(VEC4(y)));}
G2_R_QR(logic_xor){IDX; ASSIGN_R(istrue_q(VEC4(x))^(yr[idx]!=0));}
G2_R_QC(logic_xor){IDX; ASSIGN_R(istrue_q(VEC4(x))^istrue_c(VEC2(y)));}
G2_R_QQ(logic_xor){IDX; ASSIGN_R(istrue_q(VEC4(x))^istrue_q(VEC4(y)));}
DISC_R_O(logic_xor){IDX; disc[idx]=xr[idx]!=xr[idx+offset];}
)CLSRC";
	static const char program10[]=R"CLSRC(
G2_R_RR(condition_zero){IDX; ASSIGN_R(xr[idx]!=0?xr[idx]:yr[idx]);}
G2_C_RC(condition_zero)
{
	IDX;
	if(xr[idx])
		ASSIGN_C(xr[idx], 0);
	else
		ASSIGN_C(yr[idx], yi[idx]);
}
G2_Q_RQ(condition_zero)
{
	IDX;
	if(xr[idx])
		ASSIGN_Q(xr[idx], 0, 0, 0);
	else
		ASSIGN_Q(yr[idx], yi[idx], yj[idx], yk[idx]);
}
G2_C_CR(condition_zero)
{
	IDX;
	if(istrue_c(VEC2(x)))
		ASSIGN_C(xr[idx], xi[idx]);
	else
		ASSIGN_C(yr[idx], 0);
}
G2_C_CC(condition_zero)
{
	IDX;
	if(istrue_c(VEC2(x)))
		ASSIGN_C(xr[idx], xi[idx]);
	else
		ASSIGN_C(yr[idx], yi[idx]);
}
G2_Q_CQ(condition_zero)
{
	IDX;
	if(istrue_c(VEC2(x)))
		ASSIGN_Q(xr[idx], xi[idx], 0, 0);
	else
		ASSIGN_Q(yr[idx], yi[idx], yj[idx], yk[idx]);
}
G2_Q_QR(condition_zero)
{
	IDX;
	if(istrue_q(VEC4(x)))
		ASSIGN_Q(xr[idx], xi[idx], xj[idx], xk[idx]);
	else
		ASSIGN_Q(yr[idx], 0, 0, 0);
}
G2_Q_QC(condition_zero)
{
	IDX;
	if(istrue_q(VEC4(x)))
		ASSIGN_Q(xr[idx], xi[idx], xj[idx], xk[idx]);
	else
		ASSIGN_Q(yr[idx], yi[idx], 0, 0);
}
G2_Q_QQ(condition_zero)
{
	IDX;
	if(istrue_q(VEC4(x)))
		ASSIGN_Q(xr[idx], xi[idx], xj[idx], xk[idx]);
	else
		ASSIGN_Q(yr[idx], yi[idx], yj[idx], yk[idx]);
}
DISC_RR_I(condition_zero){IDX; disc[idx]=_1d_zero_in_range(xr[idx], xr[idx+offset]);}
DISC_RC_I(condition_zero){IDX; disc[idx]=_1d_zero_in_range(xr[idx], xr[idx+offset]);}
DISC_RQ_I(condition_zero){IDX; disc[idx]=_1d_zero_in_range(xr[idx], xr[idx+offset]);}
DISC_CR_I(condition_zero){IDX; disc[idx]=_1d_zero_in_range(xr[idx], xr[idx+offset])||_1d_zero_in_range(xi[idx], xi[idx+offset]);}
DISC_CC_I(condition_zero){IDX; disc[idx]=_1d_zero_in_range(xr[idx], xr[idx+offset])||_1d_zero_in_range(xi[idx], xi[idx+offset]);}
DISC_CQ_I(condition_zero){IDX; disc[idx]=_1d_zero_in_range(xr[idx], xr[idx+offset])||_1d_zero_in_range(xi[idx], xi[idx+offset]);}
DISC_QR_I(condition_zero){IDX; disc[idx]=_1d_zero_in_range(xr[idx], xr[idx+offset])||_1d_zero_in_range(xi[idx], xi[idx+offset])||_1d_zero_in_range(xj[idx], xj[idx+offset])||_1d_zero_in_range(xk[idx], xk[idx+offset]);}
DISC_QC_I(condition_zero){IDX; disc[idx]=_1d_zero_in_range(xr[idx], xr[idx+offset])||_1d_zero_in_range(xi[idx], xi[idx+offset])||_1d_zero_in_range(xj[idx], xj[idx+offset])||_1d_zero_in_range(xk[idx], xk[idx+offset]);}
DISC_QQ_I(condition_zero){IDX; disc[idx]=_1d_zero_in_range(xr[idx], xr[idx+offset])||_1d_zero_in_range(xi[idx], xi[idx+offset])||_1d_zero_in_range(xj[idx], xj[idx+offset])||_1d_zero_in_range(xk[idx], xk[idx+offset]);}

G2_R_R(percent){IDX; ASSIGN_R(xr[idx]*0.01);}
G2_C_C(percent)
{
	IDX;
	float2 ret=VEC2(x)*0.01f;
	RET_C;
}
G2_Q_Q(percent)
{
	IDX;
	float4 ret=VEC4(x)*0.01f;
	RET_Q;
}

G2_R_RR(modulo)
{
	IDX;
	float a=xr[idx], b=yr[idx];
	ASSIGN_R(a-floor(a/b)*b);
}
G2_C_RC(modulo)
{
	IDX;
	float a=xr[idx];
	float2 b=VEC2(y);
	float2 ret=mul_cc(floor_c(div_rc(a, b)), b);
	ret.x=a-ret.x;
	ret.y=-ret.y;
	RET_C;
}
G2_Q_RQ(modulo)
{
	IDX;
	float a=xr[idx];
	float4 b=VEC4(y);
	float4 ret=mul_qq(floor_q(div_rq(a, b)), b);
	ret.x=a-ret.x;
	ret.y=-ret.y;
	ret.z=-ret.z;
	ret.w=-ret.w;
	RET_Q;
}
G2_C_CR(modulo)
{
	IDX;
	float2 a=VEC2(x);
	float b=yr[idx];
	float2 ret=mul_cr(floor_c(div_cr(a, b)), b);
	ret=a-ret;
	RET_C;
}
G2_C_CC(modulo)
{
	IDX;
	float2 a=VEC2(x);
	float2 b=VEC2(y);
	float2 ret=mul_cc(floor_c(div_cc(a, b)), b);
	ret=a-ret;
	RET_C;
}
G2_Q_CQ(modulo)
{
	IDX;
	float2 a=VEC2(x);
	float4 b=VEC4(y);
	float4 ret=mul_qq(floor_q(div_cq(a, b)), b);
	ret.x=a.x-ret.x;
	ret.y=a.y-ret.y;
	ret.z=-ret.z;
	ret.w=-ret.w;
	RET_Q;
}
G2_Q_QR(modulo)
{
	IDX;
	float4 a=VEC4(x);
	float b=yr[idx];
	float4 ret=mul_qr(floor_q(div_qr(a, b)), b);
	ret=a-ret;
	RET_Q;
}
G2_Q_QC(modulo)
{
	IDX;
	float4 a=VEC4(x);
	float2 b=VEC2(y);
	float4 ret=mul_qc(floor_q(div_qc(a, b)), b);
	ret=a-ret;
	RET_Q;
}
G2_Q_QQ(modulo)
{
	IDX;
	float4 a=VEC4(x);
	float4 b=VEC4(y);
	float4 ret=mul_qq(floor_q(div_qq(a, b)), b);
	ret=a-ret;
	RET_Q;
}
DISC_RR_I(modulo){IDX; disc[idx]=floor(xr[idx]/yr[idx])!=floor(xr[idx+offset]/yr[idx+offset]);}
DISC_RC_I(modulo){IDX; disc[idx]=false;}//TODO
DISC_RQ_I(modulo){IDX; disc[idx]=false;}
DISC_CR_I(modulo){IDX; disc[idx]=false;}
DISC_CC_I(modulo){IDX; disc[idx]=false;}
DISC_CQ_I(modulo){IDX; disc[idx]=false;}
DISC_QR_I(modulo){IDX; disc[idx]=false;}
DISC_QC_I(modulo){IDX; disc[idx]=false;}
DISC_QQ_I(modulo){IDX; disc[idx]=false;}
)CLSRC";
	static const char program11[]=R"CLSRC(
G2_R_R(sgn){IDX; float a=xr[idx]; ASSIGN_R((a>0)-(a<0));}
G2_C_C(sgn)
{
	IDX;
	float2 a=VEC2(x);
	float absa=abs_c(a);
	if(absa)
	{
		float2 ret=div_cr(a, absa);
		RET_C;
	}
	else
		ASSIGN_C(0, 0);
}
G2_Q_Q(sgn)
{
	IDX;
	float4 a=VEC4(x);
	float absa=abs_q(a);
	if(absa)
	{
		float4 ret=div_qr(a, absa);
		RET_Q;
	}
	else
		ASSIGN_Q(0, 0, 0, 0);
}
DISC_R_I(sgn){IDX; float x0r=xr[idx], x1r=xr[idx+offset]; disc[idx]=x0r<0?x1r>=0:x0r>0?x1r<=0:x1r!=0;}
DISC_C_I(sgn){IDX; disc[idx]=false;}//TODO
DISC_Q_I(sgn){IDX; disc[idx]=false;}//

G2_R_R(sq){IDX; float a=xr[idx]; ASSIGN_R(a*a);}
G2_C_C(sq){IDX; float2 a=VEC2(x); float2 ret=sq_c(a); RET_C;}
G2_Q_Q(sq){IDX; float4 a=VEC4(x); float4 ret=sq_q(a); RET_Q;}

G2_C_C(sqrt){IDX; float2 a=VEC2(x); float2 ret=sqrt_c(a); RET_C;}
G2_Q_Q(sqrt){IDX; float4 a=VEC4(x); float4 ret=sqrt_q(a); RET_Q;}

G2_R_R(invsqrt){IDX; ASSIGN_R(rsqrt(xr[idx]));}

G2_R_R(cbrt){IDX; ASSIGN_R(cbrt(xr[idx]));}
G2_C_C(cbrt)
{
	IDX;
	float2 a=VEC2(x);
	float2 ln_a=log_c(a);
	float2 temp=mul_cr(ln_a, 1.f/3);
	float2 ret=exp_c(temp);
	RET_C;
}
G2_Q_Q(cbrt)//TODO: optimize
{
	IDX;
	float4 a=VEC4(x);
	float4 ln_a=log_q(a);
	float4 temp=mul_qr(ln_a, 1.f/3);
	float4 ret=exp_q(temp);
	RET_Q;
}

G2_R_R(gauss)
{
	IDX;
	float a=xr[idx];
	ASSIGN_R(exp(-a*a));
}
G2_C_C(gauss)
{
	IDX;
	float2 a=VEC2(x);
	a=mul_cc(a, a);
	a.x=-a.x, a.y=-a.y;
	float2 ret=exp_c(a);
	RET_C;
}
G2_Q_Q(gauss)
{
	IDX;
	float4 a=VEC4(x);
	a=mul_qq(a, a);
	a.x=-a.x, a.y=-a.y, a.z=-a.z, a.w=-a.w;
	float4 ret=exp_q(a);
	RET_Q;
}

G2_R_R(erf){IDX; ASSIGN_R(erf(xr[idx]));}

//zeta
)CLSRC";
	static const char program12[]=R"CLSRC(
float2	tgamma_c(float2 a)//http://en.wikipedia.org/wiki/Lanczos_approximation
{
	const float g=7, p[]={0.99999999999980993f, 676.5203681218851f, -1259.1392167224028f, 771.32342877765313f, -176.61502916214059f, 12.507343278686905f, -0.13857109526572012f, 9.9843695780195716e-6f, 1.5056327351493116e-7f};
	if(a.x<0.5f)
	{
		float2 t1=(float2)(p[0], 0);
		for(int k=1;k<g+2;++k)
			t1+=div_rc(p[k], (float2)(k-a.x, -a.y));
		float2 t2=(float2)(g+0.5f-a.x, -a.y);

		float2 spa_s2p=mul_cr(sin_c(mul_rc(_pi, a)), _sqrt_2pi);
		float2 temp2=pow_cc(t2, (float2)(0.5f-a.x, -a.y));
		float2 temp3=mul_cc(exp_c((float2)(-t2.x, -t2.y)), t1);
		return div_rc(_pi, mul_cc(mul_cc(spa_s2p, temp2), temp3));
	//	return _pi/(sin(_pi*a)*_sqrt_2pi*pow(t2, 0.5f-a)*exp(-t2)*t1);//C++
	}
	else
	{
		float2 t1=(float2)(p[0], 0);
		for(int k=1;k<g+2;++k)
			t1+=div_rc(p[k], (float2)(k-1+a.x, a.y));
		float2 t2=(float2)(g+0.5f-1+a.x, a.y);

		float2 temp=mul_rc(_sqrt_2pi, pow_cc(t2, (float2)(0.5f-1+a.x, a.y)));
		float2 temp2=mul_cc(exp_c((float2)(-t2.x, -t2.y)), t1);
		return mul_cc(temp, temp2);
	//	return _sqrt_2pi*pow(t2, 0.5f-1.+x)*exp(-t2)*t1;//C++
	}
}
float4	tgamma_q(float4 a)
{
	const float g=7, p[]={0.99999999999980993f, 676.5203681218851f, -1259.1392167224028f, 771.32342877765313f, -176.61502916214059f, 12.507343278686905f, -0.13857109526572012f, 9.9843695780195716e-6f, 1.5056327351493116e-7f};
	if(a.x<0.5f)
	{
		float4 t1=(float4)(p[0], 0, 0, 0);
		for(int k=1;k<g+2;++k)
			t1+=div_rq(p[k], (float4)(k-a.x, -a.y, -a.z, -a.w));
		float4 t2=(float4)(g+0.5f-a.x, -a.y, -a.z, -a.w);

		float4 spa_s2p=mul_qr(sin_q(mul_rq(_pi, a)), _sqrt_2pi);
		float4 temp2=pow_qq(t2, (float4)(0.5f-a.x, -a.y, -a.z, -a.w));
		float4 temp3=mul_qq(exp_q((float4)(-t2.x, -t2.y, -t2.z, -t2.w)), t1);
		return div_rq(_pi, mul_qq(mul_qq(spa_s2p, temp2), temp3));
	//	return _pi/(sin(_pi*a)*_sqrt_2pi*pow(t2, 0.5f-a)*exp(-t2)*t1);//C++
	}
	else
	{
		float4 t1=(float4)(p[0], 0, 0, 0);
		for(int k=1;k<g+2;++k)
			t1+=div_rq(p[k], (float4)(k-1+a.x, a.y, a.z, a.w));
		float4 t2=(float4)(g+0.5f-1+a.x, a.y, a.z, a.w);

		float4 temp=mul_rq(_sqrt_2pi, pow_qq(t2, (float4)(0.5f-1+a.x, a.y, a.z, a.w)));
		float4 temp2=mul_qq(exp_q((float4)(-t2.x, -t2.y, -t2.z, -t2.w)), t1);
		return mul_qq(temp, temp2);
	//	return _sqrt_2pi*pow(t2, 0.5f-1.f+x)*exp(-t2)*t1;//C++
	}
}
G2_R_R(tgamma){IDX; ASSIGN_R(tgamma(xr[idx]));}
G2_C_C(tgamma)
{
	IDX;
	float2 a=VEC2(x);
	float2 ret=tgamma_c(a);
	RET_C;
}
G2_Q_Q(tgamma)
{
	IDX;
	float4 a=VEC4(x);
	float4 ret=tgamma_q(a);
	RET_Q;
}
//r_rr_tgamma		<-
DISC_R_I(tgamma){IDX; float x0=xr[idx], x1=xr[idx+offset]; disc[idx]=x0<=0&&x1<=0&&_1d_int_in_range(x0, x1);}
DISC_C_I(tgamma)
{
	IDX;
	float2 a=VEC2(x), b=(float2)(xr[idx+offset], xi[idx+offset]);
	if(a.x==b.x)
		disc[idx]=false;
	else if(a.y==b.y)
		disc[idx]=a.y==0&&_1d_int_in_range(a.x, b.x);
	else if(signbit(a.y)!=signbit(b.y))
	{
		float t=_1d_zero_crossing(a.x, a.y, b.x, b.y);
		disc[idx]=t<=0&&t==floor(t);
	}
	else
		disc[idx]=false;
}
DISC_Q_I(tgamma){IDX; disc[idx]=false;}//TODO

G2_R_R(loggamma){IDX; ASSIGN_R(lgamma(xr[idx]));}
DISC_R_I(loggamma){disc_r_tgamma_i(size, offset, disc, xr);}

G2_R_R(factorial){IDX; ASSIGN_R(tgamma(xr[idx]+1));}
G2_C_C(factorial)
{
	IDX;
	float2 a=(float2)(xr[idx]+1, xi[idx]);
	float2 ret=tgamma_c(a);
	RET_C;
}
G2_Q_Q(factorial)
{
	IDX;
	float4 a=(float4)(xr[idx]+1, xi[idx], xj[idx], xk[idx]);
	float4 ret=tgamma_q(a);
	RET_Q;
}
DISC_R_I(factorial)
{
	IDX;
	float x0=xr[idx]+1, x1=xr[idx+offset]+1;
	disc[idx]=x0<=0&&x1<=0&&_1d_int_in_range(x0, x1);
}
DISC_C_I(factorial)
{
	IDX;
	float2 a=VEC2(x), b=(float2)(xr[idx+offset], xi[idx+offset]);
	a.x+=1, b.x+=1;
	if(a.x==b.x)
		disc[idx]=false;
	else if(a.y==b.y)
		disc[idx]=a.y==0&&_1d_int_in_range(a.x, b.x);
	else if(signbit(a.y)!=signbit(b.y))
	{
		float t=_1d_zero_crossing(a.x, a.y, b.x, b.y);
		disc[idx]=t<=0&&t==floor(t);
	}
	else
		disc[idx]=false;
}
DISC_Q_I(factorial){IDX; disc[idx]=false;}//TODO

G2_R_R(permutation){IDX; ASSIGN_R(1);}
G2_C_C(permutation){IDX; ASSIGN_C(1, 0);}
G2_Q_Q(permutation){IDX; ASSIGN_Q(1, 0, 0, 0);}
G2_R_RR(permutation)
{
	IDX;
	float a=xr[idx]+1, b=yr[idx];
	ASSIGN_R(tgamma(a)/tgamma(a-b));
}
//G2_C_RC(permutation)
//{
//	IDX;
//	float a=xr[idx]+1;
//	float2 b=VEC2(y);
//	float2 ret=div_cc(tgamma_c(a), tgamma_c(a-b));
//	RET_C;
//}
//G2_Q_RQ(permutation)
//{
//	IDX;
//	float a=xr[idx]+1;
//	float4 b=VEC4(y);
//	float4 ret=div_qq(tgamma_q(a), tgamma_q(a-b));
//	RET_Q;
//}
G2_C_CR(permutation)
{
	IDX;
	float2 a=(float2)(xr[idx]+1, xi[idx]), b=(float2)(yr[idx], 0);
	float2 ret=div_cc(tgamma_c(a), tgamma_c(a-b));
	RET_C;
}
G2_C_CC(permutation)
{
	IDX;
	float2 a=(float2)(xr[idx]+1, xi[idx]), b=VEC2(y);
	float2 ret=div_cc(tgamma_c(a), tgamma_c(a-b));
	RET_C;
}
G2_Q_QQ(permutation)
{
	IDX;
	float4 a=(float4)(xr[idx]+1, xi[idx], xj[idx], xk[idx]), b=VEC4(y);
	float4 ret=div_qq(tgamma_q(a), tgamma_q(a-b));
	RET_Q;
}
DISC_RR_I(permutation){IDX; disc[idx]=false;}//TODO
DISC_CR_I(permutation){IDX; disc[idx]=false;}//
DISC_CC_I(permutation){IDX; disc[idx]=false;}//
DISC_QQ_I(permutation){IDX; disc[idx]=false;}//

G2_R_R(combination){IDX; ASSIGN_R(1);}
G2_C_C(combination){IDX; ASSIGN_C(1, 0);}
G2_Q_Q(combination){IDX; ASSIGN_Q(1, 0, 0, 0);}
G2_R_RR(combination)
{
	IDX;
	float a=xr[idx]+1, b=yr[idx];
	ASSIGN_R(tgamma(a)/(tgamma(a-b)*tgamma(b+1)));
}
G2_C_CR(combination)
{
	IDX;
	float2 a=VEC2(x);
	float2 b=(float2)(yr[idx], 0);
	a.x+=1;
	float2 ret=div_cc(tgamma_c(a), mul_cc(tgamma_c(a-b), tgamma_c((float2)(b.x+1, b.y))));
	RET_C;
}
G2_C_CC(combination)
{
	IDX;
	float2 a=VEC2(x), b=VEC2(y);
	a.x+=1;
	float2 ret=div_cc(tgamma_c(a), mul_cc(tgamma_c(a-b), tgamma_c((float2)(b.x+1, b.y))));
	RET_C;
}
G2_Q_QQ(combination)
{
	IDX;
	float4 a=VEC4(x), b=VEC4(y);
	a.x+=1;
	float4 ret=div_qq(tgamma_q(a), mul_qq(tgamma_q(a-b), tgamma_q((float4)(b.x+1, b.y, b.z, b.w))));
	RET_Q;
}
DISC_RR_I(combination){IDX; disc[idx]=false;}//TODO
DISC_CR_I(combination){IDX; disc[idx]=false;}
DISC_CC_I(combination){IDX; disc[idx]=false;}
DISC_QQ_I(combination){IDX; disc[idx]=false;}
)CLSRC";
	static const char program13[]=R"CLSRC(
//back to G2 kernels
G2_R_R(cos){IDX; ASSIGN_R(cos(xr[idx]));}
G2_C_C(cos){IDX; float2 ret=cos_c(VEC2(x)); RET_C;}
G2_Q_Q(cos){IDX; float4 ret=cos_q(VEC4(x)); RET_Q;}

G2_C_C(acos){IDX; float2 ret=acos_c(VEC2(x)); RET_C;}
G2_Q_Q(acos){IDX; float4 ret=acos_q(VEC4(x)); RET_Q;}
DISC_C_I(acos)
{
	IDX;
	float2 x0=VEC2(x), x1=(float2)(xr[idx+offset], xi[idx+offset]);
	if(x0.y==x0.y)
		disc[idx]=false;
	else if(x0.x==x0.x)
		disc[idx]=(x0.y<=0?x1.y>0:x1.y<=0)&&(x0.x<-1||x0.x>1);
	else if((x0.y<=0&&x1.y>0)||(x1.y<=0&&x0.y>0))
	{
		float t=_1d_zero_crossing(x0.x, x0.y, x1.x, x1.y);
		disc[idx]=t<-1||t>1;
	}
	else
		disc[idx]=false;
}
DISC_Q_I(acos){IDX; disc[idx]=false;}

G2_R_R(cosh){IDX; ASSIGN_R(cosh(xr[idx]));}
G2_C_C(cosh){IDX; float2 ret=cosh_c(VEC2(x)); RET_C;}
G2_Q_Q(cosh){IDX; float4 ret=cosh_q(VEC4(x)); RET_Q;}

G2_C_C(acosh){IDX; float2 ret=acosh_c(VEC2(x)); RET_C;}
G2_Q_Q(acosh){IDX; float4 ret=acosh_q(VEC4(x)); RET_Q;}

G2_R_R(cosc){IDX; float a=xr[idx]; ASSIGN_R(cos(a)/a);}
G2_C_C(cosc){IDX; float2 a=VEC2(x), ret=div_cc(cos_c(a), a); RET_C;}
G2_Q_Q(cosc){IDX; float4 a=VEC4(x), ret=div_qq(cos_q(a), a); RET_Q;}
DISC_R_I(cosc){IDX; float x0r=xr[idx], x1r=xr[idx+offset]; disc[idx]=x0r<0?x1r>=0:x0r>0?x1r<=0:x1r!=0;}
DISC_C_I(cosc){IDX; disc[idx]=false;}//TODO
DISC_Q_I(cosc){IDX; disc[idx]=false;}//

G2_R_R(sec){IDX; ASSIGN_R(1/cos(xr[idx]));}
G2_C_C(sec){IDX; float2 ret=inv_c(cos_c(VEC2(x))); RET_C;}
G2_Q_Q(sec){IDX; float4 ret=inv_q(cos_q(VEC4(x))); RET_Q;}
DISC_R_I(sec)
{
	IDX;
	float x0=xr[idx], x1=xr[idx+offset];
	disc[idx]=fabs(x1-x0)>3.2||_1d_int_in_range(x0/_pi-0.5f, x1/_pi-0.5f);
}
DISC_C_I(sec)
{
	IDX;
	float2 x0=VEC2(x), x1=(float2)(xr[idx+offset], xi[idx+offset]);
	if(x0.x==x1.x)
	{
		if(x0.y==x1.y)
			disc[idx]=false;
		else
		{
			float t=x0.x/_pi-0.5f;
			disc[idx]=t==floor(t);
		}
	}
	else if(x0.y==x1.y)
		disc[idx]=x0.y==0&&_1d_int_in_range(x0.x/_pi-0.5f, x1.x/_pi-0.5f);
	if(sign(x0.y)!=sign(x1.y))
	{
		float t=_1d_zero_crossing(x0.x, x0.y, x1.x, x0.y)/_pi-0.5f;
		disc[idx]=t==floor(t);
	}
	disc[idx]=false;
}
DISC_Q_I(sec){IDX; disc[idx]=false;}//TODO

G2_C_C(asec){IDX; float2 ret=acos_c(inv_c(VEC2(x))); RET_C;}
G2_Q_Q(asec){IDX; float4 ret=acos_q(inv_q(VEC4(x))); RET_Q;}
DISC_C_I(asec){IDX; disc[idx]=false;}//TODO	disc c divise i
DISC_Q_I(asec){IDX; disc[idx]=false;}//		disc q divide i

G2_R_R(sech){IDX; ASSIGN_R(1/cosh(xr[idx]));}
G2_C_C(sech){IDX; float2 ret=inv_c(cosh_c(VEC2(x))); RET_C;}
G2_Q_Q(sech){IDX; float4 ret=inv_q(cosh_q(VEC4(x))); RET_Q;}
DISC_C_I(sech)
{
	IDX;
	float2 x0=VEC2(x), x1=(float2)(xr[idx+offset], xi[idx+offset]);
	if(x0.y==x1.y)
	{
		if(x0.x==x1.x)
			disc[idx]=false;
		else
		{
			float i=x0.y/_pi+0.5f;
			disc[idx]=i==floor(i)&&(x0.x<0?x1.x>=0:x0.x>0?x1.x<=0:x1.x!=0);
		}
	}
	else if(x0.x==x1.x)
		disc[idx]=x0.x==0&&_1d_int_in_range(x0.y/_pi-.5, x1.y/_pi-.5);
	else
		disc[idx]=false;
}
DISC_Q_I(sech){IDX; disc[idx]=false;}//TODO

G2_C_C(asech){IDX; float2 ret=acosh_c(inv_c(VEC2(x))); RET_C;}
G2_Q_Q(asech){IDX; float4 ret=acosh_q(inv_q(VEC4(x))); RET_Q;}
DISC_C_I(asech)
{
	IDX;
	float2 x0=VEC2(x), x1=(float2)(xr[idx+offset], xi[idx+offset]);
	if(x0.y==x0.y)
		disc[idx]=x0.y==0&&x0.x!=x0.x&&signbit(x0.x)!=signbit(x1.x);
	else if(x0.x==x0.x)
		disc[idx]=(x0.y<=0?x1.y>0:x1.y<=0)&&(x0.x<=0||x0.x>1);
	else if((x0.y<=0&&x1.y>0)||(x0.y<=0&&x1.y>0))
	{
		float t=_1d_zero_crossing(x0.x, x0.y, x1.x, x1.y);
		disc[idx]=t<=0||t>1;
	}
	else
		disc[idx]=false;
}
DISC_Q_I(asech){IDX; disc[idx]=false;}
)CLSRC";
	static const char program14[]=R"CLSRC(
//back to G2 kernels
G2_R_R(sin){IDX; ASSIGN_R(sin(xr[idx]));}
G2_C_C(sin){IDX; float2 ret=sin_c(VEC2(x)); RET_C;}
G2_Q_Q(sin){IDX; float4 ret=sin_q(VEC4(x)); RET_Q;}

G2_C_C(asin){IDX; float2 ret=asin_c(VEC2(x)); RET_C;}
G2_Q_Q(asin){IDX; float4 ret=asin_q(VEC4(x)); RET_Q;}
DISC_C_I(asin)
{
	IDX;
	float2 x0=VEC2(x), x1=(float2)(xr[idx+offset], xi[idx+offset]);
	if(x0.y==x0.y)
		disc[idx]=false;
	else if(x0.x==x0.x)
		disc[idx]=(x0.y<=0?x1.y>0:x1.y<=0)&&(x0.x<-1||x0.x>1);
	else if((x0.y<=0&&x1.y>0)||(x1.y<=0&&x0.y>0))
	{
		float t=_1d_zero_crossing(x0.x, x0.y, x1.x, x1.y);
		disc[idx]=t<-1||t>1;
	}
	else
		disc[idx]=false;
}
DISC_Q_I(asin){IDX; disc[idx]=false;}//TODO

G2_R_R(sinh){IDX; ASSIGN_R(sinh(xr[idx]));}
G2_C_C(sinh){IDX; float2 ret=sinh_c(VEC2(x)); RET_C;}
G2_Q_Q(sinh){IDX; float4 ret=sinh_q(VEC4(x)); RET_Q;}

G2_R_R(asinh){IDX; ASSIGN_R(asinh(xr[idx]));}
G2_C_C(asinh){IDX; float2 ret=asinh_c(VEC2(x)); RET_C;}
G2_Q_Q(asinh){IDX; float4 ret=asinh_q(VEC4(x)); RET_Q;}
DISC_C_I(asinh)
{
	IDX;
	float2 x0=(float2)(xi[idx], xr[idx]), x1=(float2)(xi[idx+offset], xr[idx+offset]);//sic
	if(x0.y==x0.y)
		disc[idx]=false;
	else if(x0.x==x0.x)
		disc[idx]=(x0.y<=0?x1.y>0:x1.y<=0)&&(x0.x<-1||x0.x>1);
	else if((x0.y<=0&&x1.y>0)||(x1.y<=0&&x0.y>0))
	{
		float t=_1d_zero_crossing(x0.x, x0.y, x1.x, x1.y);
		disc[idx]=t<-1||t>1;
	}
	else
		disc[idx]=false;
}
DISC_Q_I(asinh){IDX; disc[idx]=false;}//TODO

G2_R_R(sinc){IDX; float a=xr[idx]; ASSIGN_R(a!=0?sin(a)/a:1);}
G2_C_C(sinc){IDX; float2 a=VEC2(x), ret=istrue_c(a)?div_cc(sin_c(a), a):(float2)(1, 0); RET_C;}
G2_Q_Q(sinc){IDX; float4 a=VEC4(x), ret=istrue_q(a)?div_qq(sin_q(a), a):(float4)(1, 0, 0, 0); RET_Q;}

G2_R_R(sinhc){IDX; float a=xr[idx]; ASSIGN_R(a!=0?sinh(a)/a:1);}
G2_C_C(sinhc){IDX; float2 a=VEC2(x), ret=istrue_c(a)?div_cc(sinh_c(a), a):(float2)(1, 0); RET_C;}
G2_Q_Q(sinhc){IDX; float4 a=VEC4(x), ret=istrue_q(a)?div_qq(sinh_q(a), a):(float4)(1, 0, 0, 0); RET_Q;}

G2_R_R(csc){IDX; ASSIGN_R(1/sin(xr[idx]));}
G2_C_C(csc){IDX; float2 ret=inv_c(sin_c(VEC2(x))); RET_C;}
G2_Q_Q(csc){IDX; float4 ret=inv_q(sin_q(VEC4(x))); RET_Q;}
DISC_R_I(csc)
{
	IDX;
	float x0=xr[idx], x1=xr[idx+offset];
	if(fabs(x1-x0)>3.2)
		disc[idx]=true;
	else
		disc[idx]=_1d_int_in_range(x0/_pi, x1/_pi);
}
DISC_C_I(csc)
{
	IDX;
	float2 x0=VEC2(x), x1=(float2)(xr[idx+offset], xi[idx+offset]);
	if(x0.x==x1.x)
		disc[idx]=true;
	else if(x0.y==x1.y)
		disc[idx]=x0.y==0&&_1d_int_in_range(x0.x/_pi, x1.x/_pi);
	else if(signbit(x0.y)!=signbit(x1.y))
	{
		float t=_1d_zero_crossing(x0.x, x0.y, x1.x, x0.y)/_pi;
		disc[idx]=t==floor(t);
	}
	else
		disc[idx]=false;
}
DISC_Q_I(csc){IDX; disc[idx]=false;}//TODO

G2_C_C(acsc){IDX; float2 ret=asin_c(inv_c(VEC2(x))); RET_C;}
G2_Q_Q(acsc){IDX; float4 ret=asin_q(inv_q(VEC4(x))); RET_Q;}
DISC_C_I(acsc)
{
	IDX;
	float2 x0=VEC2(x), x1=(float2)(xr[idx+offset], xi[idx+offset]);
	if(x0.y==x1.y)
		disc[idx]=x0.y==0&&(x0.x<0?x1.x>=0:x0.x>0?x1.x<=0:x1.x!=0);//x1.x<0||x1.x>0);
	else if(x0.x==x1.x)
	{
		if(x0.x<0)
			disc[idx]=x0.x>-1&&(x0.y<=0?x1.y>0:x1.y<=0);
		else if(x0.x==0)
			disc[idx]=x0.y<0?x1.y>=0:x0.y==0?x1.y<0||x1.y>0:x1.y<=0;
		else
			disc[idx]=x0.x<1&&(x0.y<0?x1.y>=0:x1.y<0);
	}
	else
		disc[idx]=false;
}
DISC_Q_I(acsc){IDX; disc[idx]=false;}//TODO

G2_R_R(csch){IDX; ASSIGN_R(1/sinh(xr[idx]));}
G2_C_C(csch){IDX; float2 ret=inv_c(sinh_c(VEC2(x))); RET_C;}
G2_Q_Q(csch){IDX; float4 ret=inv_q(sinh_q(VEC4(x))); RET_Q;}
DISC_R_I(csch){IDX; float x0r=xr[idx], x1r=xr[idx+offset]; disc[idx]=x0r<0?x1r>=0:x0r>0?x1r<=0:x1r!=0;}
DISC_C_I(csch){disc_c_csc_i(size, offset, disc, xi, xr);}//sic
DISC_Q_I(csch){IDX; disc[idx]=false;}//TODO

G2_R_R(acsch){IDX; ASSIGN_R(asinh(1/xr[idx]));}
G2_C_C(acsch){IDX; float2 ret=asinh_c(inv_c(VEC2(x))); RET_C;}
G2_Q_Q(acsch){IDX; float4 ret=asinh_q(inv_q(VEC4(x))); RET_Q;}
DISC_R_I(acsch){IDX; float x0r=xr[idx], x1r=xr[idx+offset]; disc[idx]=x0r<0?x1r>=0:x0r>0?x1r<=0:x1r!=0;}
DISC_C_I(acsch){disc_c_acsc_i(size, offset, disc, xi, xr);}//sic
DISC_Q_I(acsch){IDX; disc[idx]=false;}//TODO
)CLSRC";
	static const char program15[]=R"CLSRC(
//back to G2 kernels
G2_R_R(tan){IDX; ASSIGN_R(tan(xr[idx]));}
G2_C_C(tan){IDX; float2 ret=tan_c(VEC2(x)); RET_C;}
G2_Q_Q(tan){IDX; float4 ret=tan_q(VEC4(x)); RET_Q;}
DISC_R_I(tan)
{
	IDX;
	float x0=xr[idx], x1=xr[idx+offset];
	disc[idx]=fabs(x1-x0)>3.2||_1d_int_in_range(x0/_pi-0.5f, x1/_pi-0.5f);
}
DISC_C_I(tan)
{
	IDX;
	float2 x0=VEC2(x), x1=(float2)(xr[idx+offset], xi[idx+offset]);
	if(x0.x==x1.x)
	{
		if(x0.y==x1.y)
			disc[idx]=false;
		else
		{
			float t=x0.x/_pi-0.5f;
			disc[idx]=t==floor(t);
		}
	}
	else if(x0.y==x1.y)
		disc[idx]=x0.y==0&&_1d_int_in_range(x0.x/_pi-0.5f, x1.x/_pi-0.5f);
	if(sign(x0.y)!=sign(x1.y))
	{
		float t=_1d_zero_crossing(x0.x, x0.y, x1.x, x0.y)/_pi-0.5f;
		disc[idx]=t==floor(t);
	}
	disc[idx]=false;
}
DISC_Q_I(tan){IDX; disc[idx]=false;}//TODO

float atan_addition(float x, float y){return x<0?y<0?-_pi:_pi:0;}
G2_R_R(atan){IDX; ASSIGN_R(atan(xr[idx]));}
G2_C_C(atan){IDX; float2 ret=atan_c(VEC2(x)); RET_C;}
G2_Q_Q(atan){IDX; float4 ret=atan_q(VEC4(x)); RET_Q;}
G2_R_RR(atan){IDX; ASSIGN_R(atan2(yr[idx], xr[idx]));}
G2_C_RC(atan)
{
	IDX;
	float a=xr[idx];
	float2 b=VEC2(y);
	float2 ret=atan_c(div_rc(a, b))+(float2)(atan_addition(a, b.x), 0);
	RET_C;
}
G2_Q_RQ(atan)
{
	IDX;
	float a=xr[idx];
	float4 b=VEC4(y);
	float4 ret=atan_q(div_rq(a, b))+(float4)(atan_addition(a, b.x), 0, 0, 0);
	RET_Q;
}
G2_C_CR(atan)
{
	IDX;
	float2 a=VEC2(x);
	float b=yr[idx];
	float2 ret=atan_c(div_cr(a, b))+(float2)(atan_addition(a.x, b), 0);
	RET_C;
}
G2_C_CC(atan)
{
	IDX;
	float2 a=VEC2(x);
	float2 b=VEC2(y);
	float2 ret=atan_c(div_cc(a, b))+(float2)(atan_addition(a.x, b.x), 0);
	RET_C;
}
G2_Q_CQ(atan)
{
	IDX;
	float2 a=VEC2(x);
	float4 b=VEC4(y);
	float4 ret=atan_q(div_cq(a, b))+(float4)(atan_addition(a.x, b.x), 0, 0, 0);
	RET_Q;
}
G2_Q_QR(atan)
{
	IDX;
	float4 a=VEC4(x);
	float b=yr[idx];
	float4 ret=atan_q(div_qr(a, b))+(float4)(atan_addition(a.x, b), 0, 0, 0);
	RET_Q;
}
G2_Q_QC(atan)
{
	IDX;
	float4 a=VEC4(x);
	float2 b=VEC2(x);
	float4 ret=atan_q(div_qc(a, b))+(float4)(atan_addition(a.x, b.x), 0, 0, 0);
	RET_Q;
}
G2_Q_QQ(atan)
{
	IDX;
	float4 a=VEC4(x);
	float4 b=VEC4(x);
	float4 ret=atan_q(div_qq(a, b))+(float4)(atan_addition(a.x, b.x), 0, 0, 0);
	RET_Q;
}
DISC_C_I(atan)
{
	IDX;
	float2 x0=(float2)(xi[idx], xr[idx]), x1=(float2)(xi[idx+offset], xr[idx+offset]);//sic
	if(x0.y==x0.y)
		disc[idx]=false;
	else if(x0.x==x0.x)
		disc[idx]=(x0.y<=0?x1.y>0:x1.y<=0)&&(x0.x<-1||x0.x>1);
	else if((x0.y<=0&&x1.y>0)||(x1.y<=0&&x0.y>0))
	{
		float t=_1d_zero_crossing(x0.x, x0.y, x1.x, x1.y);
		disc[idx]=t<-1||t>1;
	}
	else
		disc[idx]=false;
}
DISC_Q_I(atan){IDX; disc[idx]=false;}//TODO
DISC_RR_I(atan)
{
	IDX;
	float x0=xr[idx], x1=xr[idx+offset], y0=yr[idx], y1=yr[idx+offset];
	if(y0<0)
	{
		if(x0<0)
		{
				 if(y1<0)	disc[idx]=x1>=0;
			else if(y1>0)	disc[idx]=x1>0&&y0+(0-x0)*(y1-y0)/(x1-x0)<=0;
			else			disc[idx]=x1>=0;
		}
		else if(x0>0)
		{
				 if(y1<0)	disc[idx]=x1<0;
			else if(y1>0)	disc[idx]=x1<0&&y0+(0-y0)*(y1-y0)/(x1-x1)<=0;
			else			disc[idx]=x1<=0;
		}
		else
		{
				 if(y1<0)	disc[idx]=x1<0;
			else if(y1>0)	disc[idx]=x1<=0;
			else			disc[idx]=x1<=0;
		}
	}
	else if(y0>0)
	{
		if(x0<0)
		{
				 if(y1<0)	disc[idx]=x1>=0&&(x1==0||y0+(0-y0)*(y1-y0)/(x1-x1)<=0);
			else if(y1>0)	disc[idx]=false;
			else			disc[idx]=x1==0;
		}
		else if(x0>0)
		{
				 if(y1<0)	disc[idx]=x1<0&&y0+(0-y0)*(y1-y0)/(x1-x1)<=0;
			else if(y1>0)	disc[idx]=false;
			else			disc[idx]=x1==0;
		}
		else
		{
				 if(y1<0)	disc[idx]=x1==0;
			else if(y1>0)	disc[idx]=false;
			else			disc[idx]=x1==0;
		}
	}
	else
	{
		if(x0<0)
		{
				 if(y1<0)	disc[idx]=x1>=0;
			else if(y1>0)	disc[idx]=false;
			else			disc[idx]=x1>=0;
		}
		else if(x0>0)
		{
				 if(y1<0)	disc[idx]=x1<0;
			else if(y1>0)	disc[idx]=false;
			else			disc[idx]=x1<=0;
		}
		else				disc[idx]=true;
	}
}
DISC_RC_I(atan){IDX; disc[idx]=false;}//TODO
DISC_RQ_I(atan){IDX; disc[idx]=false;}
DISC_CR_I(atan){IDX; disc[idx]=false;}
DISC_CC_I(atan){IDX; disc[idx]=false;}
DISC_CQ_I(atan){IDX; disc[idx]=false;}
DISC_QR_I(atan){IDX; disc[idx]=false;}
DISC_QC_I(atan){IDX; disc[idx]=false;}
DISC_QQ_I(atan){IDX; disc[idx]=false;}

G2_R_R(tanh){IDX; ASSIGN_R(tanh(xr[idx]));}
G2_C_C(tanh){IDX; float2 ret=tanh_c(VEC2(x)); RET_C;}
G2_Q_Q(tanh){IDX; float4 ret=tanh_q(VEC4(x)); RET_Q;}

G2_C_C(atanh){IDX; float2 ret=atanh_c(VEC2(x)); RET_C;}
G2_Q_Q(atanh){IDX; float4 ret=atanh_q(VEC4(x)); RET_Q;}
DISC_C_I(atanh)
{
	IDX;
	float2 x0=(float2)(xi[idx], xr[idx]), x1=(float2)(xi[idx+offset], xr[idx+offset]);//sic
	if(x0.y==x0.y)
		disc[idx]=false;//disc c acos i
	else if(x0.x==x0.x)
		disc[idx]=(x0.y<=0?x1.y>0:x1.y<=0)&&(x0.x<-1||x0.x>1);
	else if((x0.y<=0&&x1.y>0)||(x1.y<=0&&x0.y>0))
	{
		float t=_1d_zero_crossing(x0.x, x0.y, x1.x, x1.y);
		disc[idx]=t<-1||t>1;
	}
	else
		disc[idx]=false;
}
DISC_Q_I(atanh){IDX; disc[idx]=false;}

G2_R_R(tanc){IDX; float a=xr[idx]; ASSIGN_R(a!=0?tan(a)/a:0);}
G2_C_C(tanc){IDX; float2 a=VEC2(x), ret=istrue_c(a)?div_cc(tan_c(a), a):(float2)(0, 0); RET_C;}
G2_Q_Q(tanc){IDX; float4 a=VEC4(x), ret=istrue_q(a)?div_qq(tan_q(a), a):(float4)(0, 0, 0, 0); RET_Q;}
DISC_R_I(tanc)
{
	IDX;
	float x0=xr[idx], x1=xr[idx+offset];
	disc[idx]=fabs(x1-x0)>3.2||_1d_int_in_range(x0/_pi-0.5f, x1/_pi-0.5f);
}
DISC_C_I(tanc)
{
	IDX;
	float2 x0=VEC2(x), x1=(float2)(xr[idx+offset], xi[idx+offset]);
	if(x0.x==x1.x)
	{
		if(x0.y==x1.y)
			disc[idx]=false;
		else
		{
			float t=x0.x/_pi-0.5f;
			disc[idx]=t==floor(t);
		}
	}
	else if(x0.y==x1.y)
		disc[idx]=x0.y==0&&_1d_int_in_range(x0.x/_pi-0.5f, x1.x/_pi-0.5f);
	if(sign(x0.y)!=sign(x1.y))
	{
		float t=_1d_zero_crossing(x0.x, x0.y, x1.x, x0.y)/_pi-0.5f;
		disc[idx]=t==floor(t);
	}
	disc[idx]=false;
}
DISC_Q_I(tanc){IDX; disc[idx]=false;}//TODO

G2_R_R(cot){IDX; ASSIGN_R(1/tan(xr[idx]));}
G2_C_C(cot){IDX; float2 ret=inv_c(tan_c(VEC2(x))); RET_C;}
G2_Q_Q(cot){IDX; float4 ret=inv_q(tan_q(VEC4(x))); RET_Q;}
DISC_R_I(cot)
{
	IDX;
	float x0=xr[idx], x1=xr[idx+offset];
	if(fabs(x1-x0)>3.2)
		disc[idx]=true;
	else
		disc[idx]=_1d_int_in_range(x0/_pi, x1/_pi);
}
DISC_C_I(cot)
{
	IDX;
	float2 x0=VEC2(x), x1=(float2)(xr[idx+offset], xi[idx+offset]);
	if(x0.x==x1.x)
		disc[idx]=true;
	else if(x0.y==x1.y)
		disc[idx]=x0.y==0&&_1d_int_in_range(x0.x/_pi, x1.x/_pi);
	else if(signbit(x0.y)!=signbit(x1.y))
	{
		float t=_1d_zero_crossing(x0.x, x0.y, x1.x, x0.y)/_pi;
		disc[idx]=t==floor(t);
	}
	else
		disc[idx]=false;
}
DISC_Q_I(cot){IDX; disc[idx]=false;}//TODO

G2_R_R(acot){IDX; float a=xr[idx]; ASSIGN_R(a!=0?atan(1/a):_pi*0.5f);}
G2_C_C(acot){IDX; float2 a=VEC2(x), ret=istrue_c(a)?atan_c(inv_c(a)):(float2)(_pi*0.5f, 0); RET_C;}
G2_Q_Q(acot){IDX; float4 a=VEC4(x), ret=istrue_q(a)?atan_q(inv_q(a)):(float4)(_pi*0.5f, 0, 0, 0); RET_Q;}
DISC_R_I(acot){IDX; float x0=xr[idx], x1=xr[idx+offset]; disc[idx]=x0<0?x1>=0:x1<0;}
DISC_C_I(acot)
{
	IDX;
	float2 x0=(float2)(xi[idx], xr[idx]), x1=(float2)(xi[idx+offset], xr[idx+offset]);//sic
	if(x0.y==x1.y)
		disc[idx]=x0.y==0&&(x0.x<0?x1.x>=0:x0.x>0?x1.x<=0:x1.x!=0);//x1.x<0||x1.x>0);
	else if(x0.x==x1.x)
	{
		if(x0.x<0)
			disc[idx]=x0.x>-1&&(x0.y<=0?x1.y>0:x1.y<=0);
		else if(x0.x==0)
			disc[idx]=x0.y<0?x1.y>=0:x0.y==0?x1.y<0||x1.y>0:x1.y<=0;
		else
			disc[idx]=x0.x<1&&(x0.y<0?x1.y>=0:x1.y<0);
	}
	else
		disc[idx]=false;
}
DISC_Q_I(acot){IDX; disc[idx]=false;}//TODO

G2_R_R(coth){IDX; ASSIGN_R(1/tanh(xr[idx]));}
G2_C_C(coth){IDX; float2 ret=inv_c(tanh_c(VEC2(x))); RET_C;}
G2_Q_Q(coth){IDX; float4 ret=inv_q(tanh_q(VEC4(x))); RET_Q;}
DISC_R_I(coth){IDX; float x0r=xr[idx], x1r=xr[idx+offset]; disc[idx]=x0r<0?x1r>=0:x0r>0?x1r<=0:x1r!=0;}//disc r csch i
DISC_C_I(coth)//disc c csch i
{
	IDX;
	float2 x0=(float2)(xi[idx], xr[idx]), x1=(float2)(xi[idx+offset], xr[idx+offset]);//sic
	if(x0.x==x1.x)
		disc[idx]=true;
	else if(x0.y==x1.y)
		disc[idx]=x0.y==0&&_1d_int_in_range(x0.x/_pi, x1.x/_pi);
	else if(signbit(x0.y)!=signbit(x1.y))
	{
		float t=_1d_zero_crossing(x0.x, x0.y, x1.x, x0.y)/_pi;
		disc[idx]=t==floor(t);
	}
	else
		disc[idx]=false;
}
DISC_Q_I(coth){IDX; disc[idx]=false;}

G2_C_C(acoth){IDX; float2 ret=atanh_c(inv_c(VEC2(x))); RET_C;}
G2_Q_Q(acoth){IDX; float4 ret=atanh_q(inv_q(VEC4(x))); RET_Q;}
DISC_C_I(acoth)
{
	IDX;
	float2 x0=(float2)(xi[idx], xr[idx]), x1=(float2)(xi[idx+offset], xr[idx+offset]);//sic
	if(x0.y==x1.y)
		disc[idx]=x0.y==0&&(x0.x<0?x1.x>=0:x0.x>0?x1.x<=0:x1.x!=0);//x1.x<0||x1.x>0);//disc c acsc i
	else if(x0.x==x1.x)
	{
			 if(x0.x<0)		disc[idx]=x0.x>-1&&(x0.y<=0?x1.y>0:x1.y<=0);
		else if(x0.x==0)	disc[idx]=x0.y<0?x1.y>=0:x0.y==0?x1.y<0||x1.y>0:x1.y<=0;
		else				disc[idx]=x0.x<1&&(x0.y<0?x1.y>=0:x1.y<0);
	}
	else
		disc[idx]=false;
}
DISC_Q_I(acoth){IDX; disc[idx]=false;}//TODO
)CLSRC";
	static const char program16[]=R"CLSRC(
G2_R_R(exp){IDX; ASSIGN_R(exp(xr[idx]));}
G2_C_C(exp){IDX; float2 ret=exp_c(VEC2(x)); RET_C;}
G2_Q_Q(exp){IDX; float4 ret=exp_q(VEC4(x)); RET_Q;}

G2_R_R(fib){IDX; float a=xr[idx], phi_p_x=exp(a*_ln_phi); ASSIGN_R((phi_p_x-cos(_pi*a)/phi_p_x)*_inv_sqrt5);}
G2_C_C(fib)
{
	IDX;
	float2 a=VEC2(x);
	float2 phi_p_x=exp_c(mul_cr(a, _ln_phi));
	float2 ret=mul_cr(phi_p_x-div_cc(cos_c(mul_rc(_pi, a)), phi_p_x), _inv_sqrt5);
	RET_C;
}
G2_Q_Q(fib)
{
	IDX;
	float4 a=VEC4(x);
	float4 phi_p_x=exp_q(mul_qr(a, _ln_phi));
	float4 ret=mul_qr(phi_p_x-div_qq(cos_q(mul_rq(_pi, a)), phi_p_x), _inv_sqrt5);
	RET_Q;
}

//random		//TODO

//beta

//bessel_j (x86: cyl_bessel_j)

//bessel_y (x86: cyl_neumann)

//hankel1

//aux functions for last part
float2 sign_c(float2 a){float abs_a=abs_c(a); return abs_a!=0?div_cr(a, abs_a):(float2)(0, 0);}
float4 sign_q(float4 a){float abs_a=abs_q(a); return abs_a!=0?div_qr(a, abs_a):(float4)(0, 0, 0, 0);}
float step_r(float a){return 0.5f+0.5f*sign(a);}
float2 step_c(float2 a){return (float2)(0.5f, 0)+mul_rc(0.5f, sign_c(a));}
float4 step_q(float4 a){return (float4)(0.5f, 0, 0, 0)+mul_rq(0.5f, sign_q(a));}

//back to kernels
G2_R_R(step){IDX; ASSIGN_R(step_r(xr[idx]));}
G2_C_C(step){IDX; float2 ret=step_c(VEC2(x)); RET_C;}
G2_Q_Q(step){IDX; float4 ret=step_q(VEC4(x)); RET_Q;}
DISC_R_I(step){IDX; float x0r=xr[idx], x1r=xr[idx+offset]; disc[idx]=x0r<0?x1r>=0:x0r>0?x1r<=0:x1r!=0;}
DISC_C_I(step){IDX; disc[idx]=false;}//TODO
DISC_Q_I(step){IDX; disc[idx]=false;}//

G2_R_R(rect){IDX; float a=xr[idx]; ASSIGN_R(step_r(a+0.5f)-step_r(a-0.5f));}
G2_C_C(rect){IDX; const float2 temp=(float2)(0.5f, 0); float2 a=VEC2(x), ret=step_c(a+temp)-step_c(a-temp); RET_C;}//'half' means half precision
G2_Q_Q(rect){IDX; const float4 temp=(float4)(0.5f, 0, 0, 0); float4 a=VEC4(x), ret=step_q(a+temp)-step_q(a-temp); RET_Q;}
bool disc_rect(float x0, float x1)
{
	const float2 d=(float2)(-0.5f, 0.5f);
		 if(x0<d.x)		return x1>=d.x;
	else if(x0==d.x)	return x1<d.x||x1>d.x;
	else if(x0<d.y)		return x1>=d.y;
	else if(x0==d.y)	return x1<d.y||x1>d.y;
	else				return x1<=d.y;
}
DISC_R_I(rect){IDX; disc[idx]=disc_rect(xr[idx], xr[idx+offset]);}
DISC_C_I(rect)
{
	IDX;
	float2 x0=VEC2(x), x1=(float2)(xr[idx+offset], xi[idx+offset]);
	if(x0.y==x1.y)
		disc[idx]=x0.y==0&&disc_rect(x0.x, x1.x);
	else if(x0.x==x1.x)
		disc[idx]=(x0.x==-0.5f||x0.x==0.5f)&&(x0.y<0?x1.y>=0:x0.y==0?x1.y<0||x1.y>0:x1.y<=0);
	else
	{
		float x=_1d_zero_crossing(x0.x, x0.y, x1.x, x1.y);
		disc[idx]=x==-0.5f||x==0.5f;
	}
}
DISC_Q_I(rect){IDX; disc[idx]=false;}//TODO

G2_R_R(trgl){IDX; float abs_a=fabs(xr[idx]); ASSIGN_R((abs_a<1)*(1-abs_a));}
G2_R_C(trgl){IDX; float abs_a=abs_c(VEC2(x)); ASSIGN_R((abs_a<1)*(1-abs_a));}
G2_R_Q(trgl){IDX; float abs_a=abs_q(VEC4(x)); ASSIGN_R((abs_a<1)*(1-abs_a));}
)CLSRC";
	static const char program17[]=R"CLSRC(
//aux functions for last part
float2 sign_c(float2 a){float abs_a=abs_c(a); return abs_a!=0?div_cr(a, abs_a):(float2)(0, 0);}
float4 sign_q(float4 a){float abs_a=abs_q(a); return abs_a!=0?div_qr(a, abs_a):(float4)(0, 0, 0, 0);}
float step_r(float a){return 0.5f+0.5f*sign(a);}
float2 step_c(float2 a){return (float2)(0.5f, 0)+mul_rc(0.5f, sign_c(a));}
float4 step_q(float4 a){return (float4)(0.5f, 0, 0, 0)+mul_rq(0.5f, sign_q(a));}

//back to G2 kernels
G2_R_R(sqwv){IDX; float a=xr[idx]; ASSIGN_R(a-floor(a)<0.5f);}
G2_R_C(sqwv){IDX; float a=xr[idx]; ASSIGN_R(a-floor(a)<0.5f);}
G2_R_Q(sqwv){IDX; float a=xr[idx]; ASSIGN_R(a-floor(a)<0.5f);}
G2_R_RR(sqwv){IDX; float a=xr[idx]; ASSIGN_R(a-floor(a)<yr[idx]);}
G2_R_RC(sqwv){IDX; float a=xr[idx]; ASSIGN_R(a-floor(a)<yr[idx]);}
G2_R_RQ(sqwv){IDX; float a=xr[idx]; ASSIGN_R(a-floor(a)<yr[idx]);}
G2_R_CR(sqwv){IDX; float a=xr[idx]; ASSIGN_R(a-floor(a)<yr[idx]);}
G2_R_CC(sqwv){IDX; float a=xr[idx]; ASSIGN_R(a-floor(a)<yr[idx]);}
G2_R_CQ(sqwv){IDX; float a=xr[idx]; ASSIGN_R(a-floor(a)<yr[idx]);}
G2_R_QR(sqwv){IDX; float a=xr[idx]; ASSIGN_R(a-floor(a)<yr[idx]);}
G2_R_QC(sqwv){IDX; float a=xr[idx]; ASSIGN_R(a-floor(a)<yr[idx]);}
G2_R_QQ(sqwv){IDX; float a=xr[idx]; ASSIGN_R(a-floor(a)<yr[idx]);}
DISC_R_O(sqwv)//for all sqwv functions
{
	IDX;
	float x0=xr[idx], x1=xr[idx+offset];
	disc[idx]=x0!=x1;
}

float clamp01(float x)
{
	float temp=x+fabs(x);//max(0, x)
	return (temp+2-fabs(temp-2))*0.25;//min(x, 1)
}
float trwv_dc(float x, float y)
{
	float t=x-floor(x), t2=1-x;
	t2-=floor(t2);
	float dc=clamp01(y);
	float dc2=1-dc, t_d=t/dc, t2_d2=t2/dc2;
	return (t_d<1?t_d:0)+(t2_d2<1?t2_d2:0);
}
G2_R_R(trwv)
{
	IDX;
	float a=xr[idx], t=fabs(a-floor(a)-0.5f);
	ASSIGN_R(t+t);
}
G2_R_C(trwv)
{
	IDX;
	float2 a=VEC2(x);
	float t=abs_c(a-floor_c(a)-(float2)(0.5f, 0));
	ASSIGN_R(t+t);
}
G2_R_Q(trwv)
{
	IDX;
	float4 a=VEC4(x);
	float t=abs_q(a-floor_q(a)-(float4)(0.5f, 0, 0, 0));
	ASSIGN_R(t+t);
}
G2_R_RR(trwv){IDX; ASSIGN_R(trwv_dc(xr[idx], yr[idx]));}
G2_R_RC(trwv){IDX; ASSIGN_R(trwv_dc(xr[idx], yr[idx]));}
G2_R_RQ(trwv){IDX; ASSIGN_R(trwv_dc(xr[idx], yr[idx]));}
G2_R_CR(trwv){IDX; ASSIGN_R(trwv_dc(xr[idx], yr[idx]));}
G2_R_CC(trwv){IDX; ASSIGN_R(trwv_dc(xr[idx], yr[idx]));}
G2_R_CQ(trwv){IDX; ASSIGN_R(trwv_dc(xr[idx], yr[idx]));}
G2_R_QR(trwv){IDX; ASSIGN_R(trwv_dc(xr[idx], yr[idx]));}
G2_R_QC(trwv){IDX; ASSIGN_R(trwv_dc(xr[idx], yr[idx]));}
G2_R_QQ(trwv){IDX; ASSIGN_R(trwv_dc(xr[idx], yr[idx]));}

float sawtooth(float x)
{
	float t=x-floor(x), t2=floor(1-t);//dc=1
	return (t2+1)*(t2*0.5+t);
}
float sawtooth_dc(float x, float y)
{
	if(!y)
		return 0;
	float t=x-floor(x), t2=floor(y-t);
	return (t2+1)*(t2*0.5+t)/y;
}
bool sawtooth_dc_disc(float t0, float t1){return floor(t0)!=floor(t1);}
G2_R_R(saw){IDX; ASSIGN_R(sawtooth(xr[idx]));}
G2_R_C(saw){IDX; ASSIGN_R(sawtooth(xr[idx]));}
G2_R_Q(saw){IDX; ASSIGN_R(sawtooth(xr[idx]));}
G2_R_RR(saw){IDX; ASSIGN_R(sawtooth_dc(xr[idx], yr[idx]));}
G2_R_RC(saw){IDX; ASSIGN_R(sawtooth_dc(xr[idx], yr[idx]));}
G2_R_RQ(saw){IDX; ASSIGN_R(sawtooth_dc(xr[idx], yr[idx]));}
G2_R_CR(saw){IDX; ASSIGN_R(sawtooth_dc(xr[idx], yr[idx]));}
G2_R_CC(saw){IDX; ASSIGN_R(sawtooth_dc(xr[idx], yr[idx]));}
G2_R_CQ(saw){IDX; ASSIGN_R(sawtooth_dc(xr[idx], yr[idx]));}
G2_R_QR(saw){IDX; ASSIGN_R(sawtooth_dc(xr[idx], yr[idx]));}
G2_R_QC(saw){IDX; ASSIGN_R(sawtooth_dc(xr[idx], yr[idx]));}
G2_R_QQ(saw){IDX; ASSIGN_R(sawtooth_dc(xr[idx], yr[idx]));}
DISC_R_I(saw){IDX; disc[idx]=ceil(xr[idx])!=ceil(xr[idx+offset]);}
DISC_C_I(saw){IDX; disc[idx]=ceil(xr[idx])!=ceil(xr[idx+offset]);}
DISC_Q_I(saw){IDX; disc[idx]=ceil(xr[idx])!=ceil(xr[idx+offset]);}
DISC_RR_I(saw){IDX; disc[idx]=sawtooth_dc_disc(xr[idx]-yr[idx], xr[idx+offset]-yr[idx+offset]);}
DISC_RC_I(saw){IDX; disc[idx]=sawtooth_dc_disc(xr[idx]-yr[idx], xr[idx+offset]-yr[idx+offset]);}
DISC_RQ_I(saw){IDX; disc[idx]=sawtooth_dc_disc(xr[idx]-yr[idx], xr[idx+offset]-yr[idx+offset]);}
DISC_CR_I(saw){IDX; disc[idx]=sawtooth_dc_disc(xr[idx]-yr[idx], xr[idx+offset]-yr[idx+offset]);}
DISC_CC_I(saw){IDX; disc[idx]=sawtooth_dc_disc(xr[idx]-yr[idx], xr[idx+offset]-yr[idx+offset]);}
DISC_CQ_I(saw){IDX; disc[idx]=sawtooth_dc_disc(xr[idx]-yr[idx], xr[idx+offset]-yr[idx+offset]);}
DISC_QR_I(saw){IDX; disc[idx]=sawtooth_dc_disc(xr[idx]-yr[idx], xr[idx+offset]-yr[idx+offset]);}
DISC_QC_I(saw){IDX; disc[idx]=sawtooth_dc_disc(xr[idx]-yr[idx], xr[idx+offset]-yr[idx+offset]);}
DISC_QQ_I(saw){IDX; disc[idx]=sawtooth_dc_disc(xr[idx]-yr[idx], xr[idx+offset]-yr[idx+offset]);}
)CLSRC";
	static const char program18[]=R"CLSRC(
G2_R_RR(hypot){IDX; float a=xr[idx], b=yr[idx]; ASSIGN_R(sqrt(a*a+b*b));}

int mandelbrot(float2 point, int n_iterations)
{
	float rez=0, imz=0, sq_rez=0, sq_imz=0;
	int k=0;
	for(;k<n_iterations&&sq_rez+sq_imz<16;++k)
	{
		imz=rez*imz;//calculate sq(z)
		imz+=imz;
		rez=sq_rez-sq_imz;

		rez+=point.x, imz+=point.y;//add x

		sq_rez=rez*rez, sq_imz=imz*imz;
	}
	return k;
}
G2_R_R(mandelbrot){IDX; ASSIGN_R(mandelbrot((float2)(xr[idx], 0), 200));}
G2_R_C(mandelbrot){IDX; ASSIGN_R(mandelbrot(VEC2(x), 200));}
G2_R_RR(mandelbrot){IDX; ASSIGN_R(mandelbrot((float2)(xr[idx], 0), (int)floor(yr[idx])));}
G2_R_CR(mandelbrot){IDX; ASSIGN_R(mandelbrot(VEC2(x), (int)floor(yr[idx])));}
DISC_R_O(mandelbrot){IDX; disc[idx]=xr[idx]!=xr[idx+offset];}

G2_R_RR(min){IDX; float a=xr[idx], b=yr[idx]; ASSIGN_R((a+b-fabs(a-b))*0.5f);}
G2_C_RC(min)
{
	IDX;
	float a=xr[idx], b=yr[idx];
	if(a<b)
		ASSIGN_C(a, 0);
	else
		ASSIGN_C(b, yi[idx]);
}
G2_Q_RQ(min)
{
	IDX;
	float a=xr[idx], b=yr[idx];
	if(a<b)
		ASSIGN_Q(a, 0, 0, 0);
	else
		ASSIGN_Q(b, yi[idx], yj[idx], yk[idx]);
}
G2_C_CR(min)
{
	IDX;
	float a=xr[idx], b=yr[idx];
	if(a<b)
		ASSIGN_C(a, xi[idx]);
	else
		ASSIGN_C(b, 0);
}
G2_C_CC(min)
{
	IDX;
	float a=xr[idx], b=yr[idx];
	if(a<b)
		ASSIGN_C(a, xi[idx]);
	else
		ASSIGN_C(b, yi[idx]);
}
G2_Q_CQ(min)
{
	IDX;
	float a=xr[idx], b=yr[idx];
	if(a<b)
		ASSIGN_Q(a, xi[idx], 0, 0);
	else
		ASSIGN_Q(b, yi[idx], yj[idx], yk[idx]);
}
G2_Q_QR(min)
{
	IDX;
	float a=xr[idx], b=yr[idx];
	if(a<b)
		ASSIGN_Q(a, xi[idx], xj[idx], xk[idx]);
	else
		ASSIGN_Q(b, 0, 0, 0);
}
G2_Q_QC(min)
{
	IDX;
	float a=xr[idx], b=yr[idx];
	if(a<b)
		ASSIGN_Q(a, xi[idx], xj[idx], xk[idx]);
	else
		ASSIGN_Q(b, yi[idx], 0, 0);
}
G2_Q_QQ(min)
{
	IDX;
	float a=xr[idx], b=yr[idx];
	if(a<b)
		ASSIGN_Q(a, xi[idx], xj[idx], xk[idx]);
	else
		ASSIGN_Q(b, yi[idx], yj[idx], yk[idx]);
}

G2_R_RR(max){IDX; float a=xr[idx], b=yr[idx]; ASSIGN_R((a+b+fabs(a-b))*0.5f);}
G2_C_RC(max)
{
	IDX;
	float a=xr[idx], b=yr[idx];
	if(a>b)
		ASSIGN_C(a, 0);
	else
		ASSIGN_C(b, yi[idx]);
}
G2_Q_RQ(max)
{
	IDX;
	float a=xr[idx], b=yr[idx];
	if(a>b)
		ASSIGN_Q(a, 0, 0, 0);
	else
		ASSIGN_Q(b, yi[idx], yj[idx], yk[idx]);
}
G2_C_CR(max)
{
	IDX;
	float a=xr[idx], b=yr[idx];
	if(a>b)
		ASSIGN_C(a, xi[idx]);
	else
		ASSIGN_C(b, 0);
}
G2_C_CC(max)
{
	IDX;
	float a=xr[idx], b=yr[idx];
	if(a>b)
		ASSIGN_C(a, xi[idx]);
	else
		ASSIGN_C(b, yi[idx]);
}
G2_Q_CQ(max)
{
	IDX;
	float a=xr[idx], b=yr[idx];
	if(a>b)
		ASSIGN_Q(a, xi[idx], 0, 0);
	else
		ASSIGN_Q(b, yi[idx], yj[idx], yk[idx]);
}
G2_Q_QR(max)
{
	IDX;
	float a=xr[idx], b=yr[idx];
	if(a>b)
		ASSIGN_Q(a, xi[idx], xj[idx], xk[idx]);
	else
		ASSIGN_Q(b, 0, 0, 0);
}
G2_Q_QC(max)
{
	IDX;
	float a=xr[idx], b=yr[idx];
	if(a>b)
		ASSIGN_Q(a, xi[idx], xj[idx], xk[idx]);
	else
		ASSIGN_Q(b, yi[idx], 0, 0);
}
G2_Q_QQ(max)
{
	IDX;
	float a=xr[idx], b=yr[idx];
	if(a>b)
		ASSIGN_Q(a, xi[idx], xj[idx], xk[idx]);
	else
		ASSIGN_Q(b, yi[idx], yj[idx], yk[idx]);
}
)CLSRC";
	static const char program19[]=R"CLSRC(
G2_R_RR(conditional_110){IDX; ASSIGN_R(xr[idx]!=0?yr[idx]:0);}
G2_C_RC(conditional_110){IDX; float2 ret=xr[idx]!=0?VEC2(y):(float2)(0, 0); RET_C;}
G2_Q_RQ(conditional_110){IDX; float4 ret=xr[idx]!=0?VEC4(y):(float4)(0, 0, 0, 0); RET_Q;}
G2_R_CR(conditional_110){IDX; ASSIGN_R(istrue_c(VEC2(x))?yr[idx]:0);}
G2_C_CC(conditional_110){IDX; float2 ret=istrue_c(VEC2(x))?VEC2(y):(float2)(0, 0); RET_C;}
G2_Q_CQ(conditional_110){IDX; float4 ret=istrue_c(VEC2(x))?VEC4(y):(float4)(0, 0, 0, 0); RET_Q;}
G2_R_QR(conditional_110){IDX; ASSIGN_R(istrue_q(VEC4(x))?yr[idx]:0);}
G2_C_QC(conditional_110){IDX; float2 ret=istrue_q(VEC4(x))?VEC2(y):(float2)(0, 0); RET_C;}
G2_Q_QQ(conditional_110){IDX; float4 ret=istrue_q(VEC4(x))?VEC4(y):(float4)(0, 0, 0, 0); RET_Q;}
DISC_RR_I(conditional_110){IDX; disc[idx]=_1d_zero_in_range(xr[idx], xr[idx+offset]);}
DISC_RC_I(conditional_110){IDX; disc[idx]=_1d_zero_in_range(xr[idx], xr[idx+offset]);}
DISC_RQ_I(conditional_110){IDX; disc[idx]=_1d_zero_in_range(xr[idx], xr[idx+offset]);}
DISC_CR_I(conditional_110){IDX; disc[idx]=_1d_zero_in_range(xr[idx], xr[idx+offset])&&_1d_zero_in_range(xi[idx], xi[idx+offset]);}
DISC_CC_I(conditional_110){IDX; disc[idx]=_1d_zero_in_range(xr[idx], xr[idx+offset])&&_1d_zero_in_range(xi[idx], xi[idx+offset]);}
DISC_CQ_I(conditional_110){IDX; disc[idx]=_1d_zero_in_range(xr[idx], xr[idx+offset])&&_1d_zero_in_range(xi[idx], xi[idx+offset]);}
DISC_QR_I(conditional_110){IDX; disc[idx]=_1d_zero_in_range(xr[idx], xr[idx+offset])&&_1d_zero_in_range(xi[idx], xi[idx+offset])&&_1d_zero_in_range(xj[idx], xj[idx+offset])&&_1d_zero_in_range(xk[idx], xk[idx+offset]);}
DISC_QC_I(conditional_110){IDX; disc[idx]=_1d_zero_in_range(xr[idx], xr[idx+offset])&&_1d_zero_in_range(xi[idx], xi[idx+offset])&&_1d_zero_in_range(xj[idx], xj[idx+offset])&&_1d_zero_in_range(xk[idx], xk[idx+offset]);}
DISC_QQ_I(conditional_110){IDX; disc[idx]=_1d_zero_in_range(xr[idx], xr[idx+offset])&&_1d_zero_in_range(xi[idx], xi[idx+offset])&&_1d_zero_in_range(xj[idx], xj[idx+offset])&&_1d_zero_in_range(xk[idx], xk[idx+offset]);}

G2_R_RR(conditional_101){IDX; ASSIGN_R(xr[idx]!=0?yr[idx]:0);}
G2_C_RC(conditional_101){IDX; float2 ret=xr[idx]!=0?VEC2(y):(float2)(0, 0); RET_C;}
G2_Q_RQ(conditional_101){IDX; float4 ret=xr[idx]!=0?VEC4(y):(float4)(0, 0, 0, 0); RET_Q;}
G2_R_CR(conditional_101){IDX; ASSIGN_R(istrue_c(VEC2(x))?yr[idx]:0);}
G2_C_CC(conditional_101){IDX; float2 ret=istrue_c(VEC2(x))?VEC2(y):(float2)(0, 0); RET_C;}
G2_Q_CQ(conditional_101){IDX; float4 ret=istrue_c(VEC2(x))?VEC4(y):(float4)(0, 0, 0, 0); RET_Q;}
G2_R_QR(conditional_101){IDX; ASSIGN_R(istrue_q(VEC4(x))?yr[idx]:0);}
G2_C_QC(conditional_101){IDX; float2 ret=istrue_q(VEC4(x))?VEC2(y):(float2)(0, 0); RET_C;}
G2_Q_QQ(conditional_101){IDX; float4 ret=istrue_q(VEC4(x))?VEC4(y):(float4)(0, 0, 0, 0); RET_Q;}
DISC_RR_I(conditional_101){disc_rr_conditional_110_i(size, offset, disc, xr, yr);}
DISC_RC_I(conditional_101){disc_rc_conditional_110_i(size, offset, disc, xr, yr, yi);}
DISC_RQ_I(conditional_101){disc_rq_conditional_110_i(size, offset, disc, xr, yr, yi, yj, yk);}
DISC_CR_I(conditional_101){disc_cr_conditional_110_i(size, offset, disc, xr, xi, yr);}
DISC_CC_I(conditional_101){disc_cc_conditional_110_i(size, offset, disc, xr, xi, yr, yi);}
DISC_CQ_I(conditional_101){disc_cq_conditional_110_i(size, offset, disc, xr, xi, yr, yi, yj, yk);}
DISC_QR_I(conditional_101){disc_qr_conditional_110_i(size, offset, disc, xr, xi, xj, xk, yr);}
DISC_QC_I(conditional_101){disc_qc_conditional_110_i(size, offset, disc, xr, xi, xj, xk, yr, yi);}
DISC_QQ_I(conditional_101){disc_qq_conditional_110_i(size, offset, disc, xr, xi, xj, xk, yr, yi, yj, yk);}

//conditional_111: pass nullptr for real & complex			TODO: disc for conditional_111
#define		NPA(pointer)				(pointer?pointer[idx]:0)
#define		ASSIGN_NP(dest, source)		if(dest)dest[idx]=NPA(source)
__kernel void conditional_111(__global const int *size,
	__global float *rr, __global float *ri, __global float *rj, __global float *rk,
	__global const float *xr, __global const float *xi, __global const float *xj, __global const float *xk,
	__global const float *yr, __global const float *yi, __global const float *yj, __global const float *yk,
	__global const float *zr, __global const float *zi, __global const float *zj, __global const float *zk)
{
	IDX;
	float4 a=(float4)(NPA(xr), NPA(xi), NPA(xj), NPA(xk));
	if(istrue_q(a))
	{
		ASSIGN_NP(rr, yr);
		ASSIGN_NP(ri, yi);
		ASSIGN_NP(rj, yj);
		ASSIGN_NP(rk, yk);
	}
	else
	{
		ASSIGN_NP(rr, zr);
		ASSIGN_NP(ri, zi);
		ASSIGN_NP(rj, zj);
		ASSIGN_NP(rk, zk);
	}
}

G2_R_R(increment){IDX; ASSIGN_R(xr[idx]+1);}
G2_C_C(increment){IDX; float2 ret=VEC2(x)+(float2)(1, 0); RET_C;}
G2_Q_Q(increment){IDX; float4 ret=VEC4(x)+(float4)(1, 0, 0, 0); RET_Q;}

G2_R_R(decrement){IDX; ASSIGN_R(xr[idx]-1);}
G2_C_C(decrement){IDX; float2 ret=VEC2(x)-(float2)(1, 0); RET_C;}
G2_Q_Q(decrement){IDX; float4 ret=VEC4(x)-(float4)(1, 0, 0, 0); RET_Q;}

G2_R_R(assign){IDX; ASSIGN_R(xr[idx]);}
G2_C_C(assign){IDX; float2 ret=VEC2(x); RET_C;}
G2_Q_Q(assign){IDX; float4 ret=VEC4(x); RET_Q;}

__kernel void initialize_parameter(__global const int *size, __global float *buffer, __global const float *args)
{
	IDX;
//	uint idx=size[0]*(size[1]*get_global_id(2)+get_global_id(1))+get_global_id(0);
	//args[0]: start
	//args[1]: ratio
	//args[2]: dimension 0:x, 1:y, 2:z
	buffer[idx]=args[0]+args[1]*get_global_id((int)args[2]);
//	buffer[idx]=args[0]+args[1]*get_global_id(((int*)args)[2]);
}

#define		COS_PI_6		0.866025403784439f
#define		SIN_PI_6		0.5f
#define		THRESHOLD		10
#define		INV_THRESHOLD	0.1f
#define		COMP_MUL		0.00392156862745098f
__kernel void c2d_rgb(__global const int *size, __global const float *xr, __global const float *xi, __write_only image2d_t rgb)
{//size{Xplaces, Yplaces}
	const int2 coords=(int2)(get_global_id(0), get_global_id(1));
	const uint idx=size[0]*coords.y+coords.x;
	float r=xr[idx], i=xi[idx];
	float4 color;
	if(r!=r||i!=i)
		color=(float4)(1, 0.5f, 0.5f, 0.5f);
	else if(fabs(r)==INFINITY||fabs(i)==INFINITY)
		color=(float4)(1, 1, 1, 1);
	else
	{
		float hyp=sqrt(r*r+i*i), cosx=r/hyp, sinx=i/hyp,
			mag=255*exp(-hyp*_ln2*INV_THRESHOLD);
		float red=1+cosx*COS_PI_6-sinx*SIN_PI_6, green=1+sinx, blue=1+cosx*-COS_PI_6-sinx*SIN_PI_6;
		if(hyp<THRESHOLD)
			mag=255-mag, red*=mag, green*=mag, blue*=mag;
		else
			red=255-mag*(2-red), green=255-mag*(2-green), blue=255-mag*(2-blue);
		color=(float4)(red, green, blue, 255)*COMP_MUL;
	//	color.x=red*COMP_MUL, color.y=green*COMP_MUL, color.z=blue*COMP_MUL, color.w=1;
	//	color=(float4)(1, blue*COMP_MUL, green*COMP_MUL, red*COMP_MUL);
	//	rgb[idx]=0xFF<<24|(uchar)blue<<16|(uchar)green<<8|(uchar)red;
	}
//	color=(float4)(1, 1, coords.x/size[0], coords.y/size[1]);//
	write_imagef(rgb, coords, color);
}

__kernel void c2d_rgb2(__global const int *size, __global const float *xr, __global const float *xi, __global int *rgb)
{//size{Xplaces, Yplaces, 1, appwidth, appheight}
	const int2 coords=(int2)(get_global_id(0), get_global_id(1));
	const uint idx=size[0]*coords.y+coords.x, idx2=size[3]*coords.y+coords.x;
	float r=xr[idx], i=xi[idx];
	if(r!=r||i!=i)
		rgb[idx2]=0xFF7F7F7F;
	else if(fabs(r)==INFINITY||fabs(i)==INFINITY)
		rgb[idx2]=0xFFFFFFFF;
	else
	{
		float hyp=sqrt(r*r+i*i), cosx=r/hyp, sinx=i/hyp,
			mag=255*exp(-hyp*_ln2*INV_THRESHOLD);
		float red=1+cosx*COS_PI_6-sinx*SIN_PI_6, green=1+sinx, blue=1+cosx*-COS_PI_6-sinx*SIN_PI_6;
		if(hyp<THRESHOLD)
			mag=255-mag, red*=mag, green*=mag, blue*=mag;
		else
			red=255-mag*(2-red), green=255-mag*(2-green), blue=255-mag*(2-blue);
		rgb[idx2]=0xFF000000|(uchar)blue<<16|(uchar)green<<8|(uchar)red;
	}
}
)CLSRC";

	const char *programs[]=
	{
		CLSource::program00,
		CLSource::program01,
		CLSource::program02,
		CLSource::program03,
		CLSource::program04,
		CLSource::program05,
		CLSource::program06,
		CLSource::program07,
		CLSource::program08,
		CLSource::program09,
		CLSource::program10,
		CLSource::program11,
		CLSource::program12,
		CLSource::program13,
		CLSource::program14,
		CLSource::program15,
		CLSource::program16,
		CLSource::program17,
		CLSource::program18,
		CLSource::program19,
	};
}//end CLSource
const int		nprograms=sizeof(CLSource::programs)/sizeof(const char*);
struct 			ProgramBinary
{
	unsigned char *bin;
	size_t size;
};
ProgramBinary	binaries[nprograms]={};
bool			binaries_valid=false;
cl_platform_id	platform=nullptr;//0x00000000de763ed3
cl_device_id	device=nullptr;//changes when app is launched
cl_context		context=nullptr;//changes when resuming
cl_command_queue commandqueue=nullptr;//changes when resuming
cl_kernel		kernels[N_KERNELS]={nullptr};//all kernels
namespace 		G2_CL
{
	cl_program programs[nprograms]={nullptr};

//declare continuous kernel
#define	DECL_C(SIG, ret, arg, NAME, name)	{SIG##_##NAME, SIG_##SIG, DISC_C, #ret "_" #arg "_" #name, nullptr}
//declare kernel with discontinuities depending on input
#define	DECL_I(SIG, ret, arg, NAME, name)	{SIG##_##NAME, SIG_##SIG, DISC_I, #ret "_" #arg "_" #name, "disc_" #arg "_" #name"_i"}
//declare kernel with discontinuities depending on output
#define	DECL_O(SIG, ret, arg, NAME, name)	{SIG##_##NAME, SIG_##SIG, DISC_O, #ret "_" #arg "_" #name, "disc_" #ret "_" #name"_o"}

//kernel signatures, DISCTYPE: C continuous, I depends on input, or O depends on output
#define	DECL_R_R(NAME, name, DISCTYPE)		DECL_##DISCTYPE(R_R, r, r, NAME, name)
#define	DECL_C_C(NAME, name, DISCTYPE)		DECL_##DISCTYPE(C_C, c, c, NAME, name)
#define	DECL_Q_Q(NAME, name, DISCTYPE)		DECL_##DISCTYPE(Q_Q, q, q, NAME, name)
#define	DECL_R_RR(NAME, name, DISCTYPE)		DECL_##DISCTYPE(R_RR, r, rr, NAME, name)
#define	DECL_C_RC(NAME, name, DISCTYPE)		DECL_##DISCTYPE(C_RC, c, rc, NAME, name)
#define	DECL_Q_RQ(NAME, name, DISCTYPE)		DECL_##DISCTYPE(Q_RQ, q, rq, NAME, name)

#define	DECL_C_CR(NAME, name, DISCTYPE)		DECL_##DISCTYPE(C_CR, c, cr, NAME, name)
#define	DECL_C_CC(NAME, name, DISCTYPE)		DECL_##DISCTYPE(C_CC, c, cc, NAME, name)
#define	DECL_Q_CQ(NAME, name, DISCTYPE)		DECL_##DISCTYPE(Q_CQ, q, cq, NAME, name)

#define	DECL_Q_QR(NAME, name, DISCTYPE)		DECL_##DISCTYPE(Q_QR, q, qr, NAME, name)
#define	DECL_Q_QC(NAME, name, DISCTYPE)		DECL_##DISCTYPE(Q_QC, q, qc, NAME, name)
#define	DECL_Q_QQ(NAME, name, DISCTYPE)		DECL_##DISCTYPE(Q_QQ, q, qq, NAME, name)

#define	DECL_C_R(NAME, name, DISCTYPE)		DECL_##DISCTYPE(C_R, c, r, NAME, name)
#define	DECL_C_Q(NAME, name, DISCTYPE)		DECL_##DISCTYPE(C_Q, c, q, NAME, name)
#define	DECL_R_C(NAME, name, DISCTYPE)		DECL_##DISCTYPE(R_C, r, c, NAME, name)
#define	DECL_R_Q(NAME, name, DISCTYPE)		DECL_##DISCTYPE(R_Q, r, q, NAME, name)
#define	DECL_C_RR(NAME, name, DISCTYPE)		DECL_##DISCTYPE(C_RR, c, rr, NAME, name)

#define	DECL_R_RC(NAME, name, DISCTYPE)		DECL_##DISCTYPE(R_RC, r, rc, NAME, name)
#define	DECL_R_RQ(NAME, name, DISCTYPE)		DECL_##DISCTYPE(R_RQ, r, rq, NAME, name)
#define	DECL_R_CR(NAME, name, DISCTYPE)		DECL_##DISCTYPE(R_CR, r, cr, NAME, name)
#define	DECL_R_CC(NAME, name, DISCTYPE)		DECL_##DISCTYPE(R_CC, r, cc, NAME, name)
#define	DECL_R_CQ(NAME, name, DISCTYPE)		DECL_##DISCTYPE(R_CQ, r, cq, NAME, name)
#define	DECL_R_QR(NAME, name, DISCTYPE)		DECL_##DISCTYPE(R_QR, r, qr, NAME, name)
#define	DECL_R_QC(NAME, name, DISCTYPE)		DECL_##DISCTYPE(R_QC, r, qc, NAME, name)
#define	DECL_R_QQ(NAME, name, DISCTYPE)		DECL_##DISCTYPE(R_QQ, r, qq, NAME, name)

#define	DECL_C_QC(NAME, name, DISCTYPE)		DECL_##DISCTYPE(C_QC, c, qc, NAME, name)

//declare function implemented in software
#define	DECL_SW(SIG, NAME, DISCTYPE)		{SIG##_##NAME, SIG_##SIG, DISC_##DISCTYPE, nullptr, nullptr}

//declare kernel - outdated
//#define	DECL_K(signature, name, disctype, namestr, discstr)		{signature##_##name, CL_##signature, DISC_##disctype, namestr, discstr}

	CLKernel kernels_p00[]=
	{
		//G2 functions
		DECL_R_R(SETZERO, setzero, C),
		DECL_C_C(SETZERO, setzero, C),
		DECL_Q_Q(SETZERO, setzero, C),

		DECL_R_R(CEIL, ceil, O),
		DECL_C_C(CEIL, ceil, O),
		DECL_Q_Q(CEIL, ceil, O),

		DECL_R_R(FLOOR, floor, O),
		DECL_C_C(FLOOR, floor, O),
		DECL_Q_Q(FLOOR, floor, O),

		DECL_R_R(ROUND, round, O),
		DECL_C_C(ROUND, round, O),
		DECL_Q_Q(ROUND, round, O),

		DECL_R_R(INT, int, O),
		DECL_C_C(INT, int, O),
		DECL_Q_Q(INT, int, O),

		DECL_R_R(FRAC, frac, I),
		DECL_C_C(FRAC, frac, I),
		DECL_Q_Q(FRAC, frac, I),
	};
	CLKernel kernels_p01[]=
	{
		DECL_R_R(ABS, abs, C),
		DECL_R_C(ABS, abs, C),
		DECL_R_Q(ABS, abs, C),

		DECL_R_R(ARG, arg, I),
		DECL_R_C(ARG, arg, I),
		DECL_R_Q(ARG, arg, I),

		DECL_R_C(REAL, real, C),

		DECL_R_C(IMAG, imag, C),

		DECL_C_C(CONJUGATE, conjugate, C),
		DECL_Q_Q(CONJUGATE, conjugate, C),

		DECL_C_R(POLAR, polar, I),
		DECL_C_C(POLAR, polar, I),
		DECL_C_Q(POLAR, polar, I),

		DECL_C_C(CARTESIAN, cartesian, C),
		DECL_Q_Q(CARTESIAN, cartesian, C),
	};
	CLKernel kernels_p02[]=
	{
		DECL_R_RR(PLUS, plus, C),
		DECL_C_RC(PLUS, plus, C),
		DECL_Q_RQ(PLUS, plus, C),
		DECL_C_CR(PLUS, plus, C),
		DECL_C_CC(PLUS, plus, C),
		DECL_Q_CQ(PLUS, plus, C),
		DECL_Q_QR(PLUS, plus, C),
		DECL_Q_QC(PLUS, plus, C),
		DECL_Q_QQ(PLUS, plus, C),

		DECL_R_R(MINUS, minus, C),
		DECL_C_C(MINUS, minus, C),
		DECL_Q_Q(MINUS, minus, C),
		DECL_R_RR(MINUS, minus, C),
		DECL_C_RC(MINUS, minus, C),
		DECL_Q_RQ(MINUS, minus, C),
		DECL_C_CR(MINUS, minus, C),
		DECL_C_CC(MINUS, minus, C),
		DECL_Q_CQ(MINUS, minus, C),
		DECL_Q_QR(MINUS, minus, C),
		DECL_Q_QC(MINUS, minus, C),
		DECL_Q_QQ(MINUS, minus, C),
		
		DECL_R_RR(MULTIPLY, multiply, C),
		DECL_C_RC(MULTIPLY, multiply, C),
		DECL_Q_RQ(MULTIPLY, multiply, C),
		DECL_C_CR(MULTIPLY, multiply, C),
		DECL_C_CC(MULTIPLY, multiply, C),
		DECL_Q_CQ(MULTIPLY, multiply, C),
		DECL_Q_QR(MULTIPLY, multiply, C),
		DECL_Q_QC(MULTIPLY, multiply, C),
		DECL_Q_QQ(MULTIPLY, multiply, C),
		
		DECL_R_R(DIVIDE, divide, I),
		DECL_C_C(DIVIDE, divide, I),
		DECL_Q_Q(DIVIDE, divide, I),
		DECL_R_RR(DIVIDE, divide, I),
		DECL_C_RC(DIVIDE, divide, I),
		DECL_Q_RQ(DIVIDE, divide, I),
		DECL_C_CR(DIVIDE, divide, I),
		DECL_C_CC(DIVIDE, divide, I),
		DECL_Q_CQ(DIVIDE, divide, I),
		DECL_Q_QR(DIVIDE, divide, I),
		DECL_Q_QC(DIVIDE, divide, I),
		DECL_Q_QQ(DIVIDE, divide, I),
	};
	CLKernel kernels_p03[]=
	{
		DECL_R_RR(LOGIC_DIVIDES, logic_divides, O),
		DECL_R_RC(LOGIC_DIVIDES, logic_divides, O),
		DECL_R_RQ(LOGIC_DIVIDES, logic_divides, O),
		DECL_R_CR(LOGIC_DIVIDES, logic_divides, O),
		DECL_R_CC(LOGIC_DIVIDES, logic_divides, O),
		DECL_R_CQ(LOGIC_DIVIDES, logic_divides, O),
		DECL_R_QR(LOGIC_DIVIDES, logic_divides, O),
		DECL_R_QC(LOGIC_DIVIDES, logic_divides, O),
		DECL_R_QQ(LOGIC_DIVIDES, logic_divides, O),

		DECL_R_RR(POWER_REAL, power_real, I),
		DECL_C_CR(POWER_REAL, power_real, I),
		DECL_Q_QR(POWER_REAL, power_real, I),

		DECL_C_CR(POW, pow, I),
		DECL_C_CC(POW, pow, I),
		DECL_Q_CQ(POW, pow, I),
		DECL_Q_QR(POW, pow, I),
		DECL_Q_QC(POW, pow, I),
		DECL_Q_QQ(POW, pow, I),

		DECL_C_C(LN, ln, I),
		DECL_Q_Q(LN, ln, I),

		DECL_C_C(LOG, log, I),
		DECL_Q_Q(LOG, log, I),
		DECL_C_CR(LOG, log, I),
		DECL_C_CC(LOG, log, I),
		DECL_Q_CQ(LOG, log, I),
		DECL_Q_QC(LOG, log, I),
		DECL_Q_QQ(LOG, log, I),
		
		DECL_SW(C_RR, TETRATE, I),
		DECL_SW(C_RC, TETRATE, I),
		DECL_SW(C_CR, TETRATE, I),
		DECL_SW(C_CC, TETRATE, I),
		DECL_SW(Q_QR, TETRATE, I),
		//{C_RR_TETRATE, CL_, 0, nullptr, nullptr},
		//{C_RC_TETRATE, CL_, 0, nullptr, nullptr},
		//{C_CR_TETRATE, CL_, 0, nullptr, nullptr},
		//{C_CC_TETRATE, CL_, 0, nullptr, nullptr},
		//{Q_QR_TETRATE, CL_, 0, nullptr, nullptr},
		
		DECL_SW(C_RR, PENTATE, I),
		DECL_SW(C_CR, PENTATE, I),
	};
	CLKernel kernels_p04[]=
	{
		DECL_R_R(BITWISE_SHIFT_LEFT_L, bitwise_shift_left_l, O),
		DECL_C_C(BITWISE_SHIFT_LEFT_L, bitwise_shift_left_l, O),
		DECL_Q_Q(BITWISE_SHIFT_LEFT_L, bitwise_shift_left_l, O),
		DECL_R_R(BITWISE_SHIFT_LEFT_R, bitwise_shift_left_r, C),
		DECL_C_C(BITWISE_SHIFT_LEFT_R, bitwise_shift_left_r, C),
		DECL_Q_Q(BITWISE_SHIFT_LEFT_R, bitwise_shift_left_r, C),
		DECL_R_RR(BITWISE_SHIFT_LEFT, bitwise_shift_left, I),
		DECL_C_RC(BITWISE_SHIFT_LEFT, bitwise_shift_left, I),
		DECL_Q_RQ(BITWISE_SHIFT_LEFT, bitwise_shift_left, I),
		DECL_C_CR(BITWISE_SHIFT_LEFT, bitwise_shift_left, I),
		DECL_C_CC(BITWISE_SHIFT_LEFT, bitwise_shift_left, I),
		DECL_Q_CQ(BITWISE_SHIFT_LEFT, bitwise_shift_left, I),
		DECL_Q_QR(BITWISE_SHIFT_LEFT, bitwise_shift_left, I),
		DECL_Q_QC(BITWISE_SHIFT_LEFT, bitwise_shift_left, I),
		DECL_Q_QQ(BITWISE_SHIFT_LEFT, bitwise_shift_left, I),

		DECL_R_R(BITWISE_SHIFT_RIGHT_L, bitwise_shift_right_l, O),
		DECL_C_C(BITWISE_SHIFT_RIGHT_L, bitwise_shift_right_l, O),
		DECL_Q_Q(BITWISE_SHIFT_RIGHT_L, bitwise_shift_right_l, O),
		DECL_R_R(BITWISE_SHIFT_RIGHT_R, bitwise_shift_right_r, C),
		DECL_C_C(BITWISE_SHIFT_RIGHT_R, bitwise_shift_right_r, C),
		DECL_Q_Q(BITWISE_SHIFT_RIGHT_R, bitwise_shift_right_r, C),
		DECL_R_RR(BITWISE_SHIFT_RIGHT, bitwise_shift_right, I),
		DECL_C_RC(BITWISE_SHIFT_RIGHT, bitwise_shift_right, I),
		DECL_Q_RQ(BITWISE_SHIFT_RIGHT, bitwise_shift_right, I),
		DECL_C_CR(BITWISE_SHIFT_RIGHT, bitwise_shift_right, I),
		DECL_C_CC(BITWISE_SHIFT_RIGHT, bitwise_shift_right, I),
		DECL_Q_CQ(BITWISE_SHIFT_RIGHT, bitwise_shift_right, I),
		DECL_Q_QR(BITWISE_SHIFT_RIGHT, bitwise_shift_right, I),
		DECL_Q_QC(BITWISE_SHIFT_RIGHT, bitwise_shift_right, I),
		DECL_Q_QQ(BITWISE_SHIFT_RIGHT, bitwise_shift_right, I),
	};
	CLKernel kernels_p05[]=
	{
		DECL_R_R(BITWISE_NOT, bitwise_not, I),
		DECL_C_C(BITWISE_NOT, bitwise_not, I),
		DECL_Q_Q(BITWISE_NOT, bitwise_not, I),

		DECL_R_R(BITWISE_AND, bitwise_and, O),
		DECL_C_C(BITWISE_AND, bitwise_and, O),
		DECL_Q_Q(BITWISE_AND, bitwise_and, O),
		DECL_R_RR(BITWISE_AND, bitwise_and, O),
		DECL_C_RC(BITWISE_AND, bitwise_and, O),
		DECL_Q_RQ(BITWISE_AND, bitwise_and, O),
		DECL_C_CR(BITWISE_AND, bitwise_and, O),
		DECL_C_CC(BITWISE_AND, bitwise_and, O),
		DECL_Q_CQ(BITWISE_AND, bitwise_and, O),
		DECL_Q_QR(BITWISE_AND, bitwise_and, O),
		DECL_Q_QC(BITWISE_AND, bitwise_and, O),
		DECL_Q_QQ(BITWISE_AND, bitwise_and, O),

		DECL_R_R(BITWISE_NAND, bitwise_nand, O),
		DECL_C_C(BITWISE_NAND, bitwise_nand, O),
		DECL_Q_Q(BITWISE_NAND, bitwise_nand, O),
		DECL_R_RR(BITWISE_NAND, bitwise_nand, O),
		DECL_C_RC(BITWISE_NAND, bitwise_nand, O),
		DECL_Q_RQ(BITWISE_NAND, bitwise_nand, O),
		DECL_C_CR(BITWISE_NAND, bitwise_nand, O),
		DECL_C_CC(BITWISE_NAND, bitwise_nand, O),
		DECL_Q_CQ(BITWISE_NAND, bitwise_nand, O),
		DECL_Q_QR(BITWISE_NAND, bitwise_nand, O),
		DECL_Q_QC(BITWISE_NAND, bitwise_nand, O),
		DECL_Q_QQ(BITWISE_NAND, bitwise_nand, O),

		DECL_R_R(BITWISE_OR, bitwise_or, O),
		DECL_C_C(BITWISE_OR, bitwise_or, O),
		DECL_Q_Q(BITWISE_OR, bitwise_or, O),
		DECL_R_RR(BITWISE_OR, bitwise_or, O),
		DECL_C_RC(BITWISE_OR, bitwise_or, O),
		DECL_Q_RQ(BITWISE_OR, bitwise_or, O),
		DECL_C_CR(BITWISE_OR, bitwise_or, O),
		DECL_C_CC(BITWISE_OR, bitwise_or, O),
		DECL_Q_CQ(BITWISE_OR, bitwise_or, O),
		DECL_Q_QR(BITWISE_OR, bitwise_or, O),
		DECL_Q_QC(BITWISE_OR, bitwise_or, O),
		DECL_Q_QQ(BITWISE_OR, bitwise_or, O),
	};
	CLKernel kernels_p06[]=
	{
		DECL_R_R(BITWISE_NOR, bitwise_nor, O),
		DECL_C_C(BITWISE_NOR, bitwise_nor, O),
		DECL_Q_Q(BITWISE_NOR, bitwise_nor, O),
		DECL_R_RR(BITWISE_NOR, bitwise_nor, O),
		DECL_C_RC(BITWISE_NOR, bitwise_nor, O),
		DECL_Q_RQ(BITWISE_NOR, bitwise_nor, O),
		DECL_C_CR(BITWISE_NOR, bitwise_nor, O),
		DECL_C_CC(BITWISE_NOR, bitwise_nor, O),
		DECL_Q_CQ(BITWISE_NOR, bitwise_nor, O),
		DECL_Q_QR(BITWISE_NOR, bitwise_nor, O),
		DECL_Q_QC(BITWISE_NOR, bitwise_nor, O),
		DECL_Q_QQ(BITWISE_NOR, bitwise_nor, O),

		DECL_R_R(BITWISE_XOR, bitwise_xor, O),
		DECL_C_C(BITWISE_XOR, bitwise_xor, O),
		DECL_Q_Q(BITWISE_XOR, bitwise_xor, O),
		DECL_R_RR(BITWISE_XOR, bitwise_xor, O),
		DECL_C_RC(BITWISE_XOR, bitwise_xor, O),
		DECL_Q_RQ(BITWISE_XOR, bitwise_xor, O),
		DECL_C_CR(BITWISE_XOR, bitwise_xor, O),
		DECL_C_CC(BITWISE_XOR, bitwise_xor, O),
		DECL_Q_CQ(BITWISE_XOR, bitwise_xor, O),
		DECL_Q_QR(BITWISE_XOR, bitwise_xor, O),
		DECL_Q_QC(BITWISE_XOR, bitwise_xor, O),
		DECL_Q_QQ(BITWISE_XOR, bitwise_xor, O),

		DECL_R_R(BITWISE_XNOR, bitwise_xnor, O),
		DECL_C_C(BITWISE_XNOR, bitwise_xnor, O),
		DECL_Q_Q(BITWISE_XNOR, bitwise_xnor, O),
		DECL_R_RR(BITWISE_XNOR, bitwise_xnor, O),
		DECL_C_RC(BITWISE_XNOR, bitwise_xnor, O),
		DECL_Q_RQ(BITWISE_XNOR, bitwise_xnor, O),
		DECL_C_CR(BITWISE_XNOR, bitwise_xnor, O),
		DECL_C_CC(BITWISE_XNOR, bitwise_xnor, O),
		DECL_Q_CQ(BITWISE_XNOR, bitwise_xnor, O),
		DECL_Q_QR(BITWISE_XNOR, bitwise_xnor, O),
		DECL_Q_QC(BITWISE_XNOR, bitwise_xnor, O),
		DECL_Q_QQ(BITWISE_XNOR, bitwise_xnor, O),
	};
	CLKernel kernels_p07[]=
	{
		DECL_R_R(LOGIC_EQUAL, logic_equal, O),
		DECL_R_C(LOGIC_EQUAL, logic_equal, O),
		DECL_R_Q(LOGIC_EQUAL, logic_equal, O),
		DECL_R_RR(LOGIC_EQUAL, logic_equal, O),
		DECL_R_RC(LOGIC_EQUAL, logic_equal, O),
		DECL_R_RQ(LOGIC_EQUAL, logic_equal, O),
		DECL_R_CR(LOGIC_EQUAL, logic_equal, O),
		DECL_R_CC(LOGIC_EQUAL, logic_equal, O),
		DECL_R_CQ(LOGIC_EQUAL, logic_equal, O),
		DECL_R_QR(LOGIC_EQUAL, logic_equal, O),
		DECL_R_QC(LOGIC_EQUAL, logic_equal, O),
		DECL_R_QQ(LOGIC_EQUAL, logic_equal, O),

		DECL_R_R(LOGIC_NOT_EQUAL, logic_not_equal, O),
		DECL_R_C(LOGIC_NOT_EQUAL, logic_not_equal, O),
		DECL_R_Q(LOGIC_NOT_EQUAL, logic_not_equal, O),
		DECL_R_RR(LOGIC_NOT_EQUAL, logic_not_equal, O),
		DECL_R_RC(LOGIC_NOT_EQUAL, logic_not_equal, O),
		DECL_R_RQ(LOGIC_NOT_EQUAL, logic_not_equal, O),
		DECL_R_CR(LOGIC_NOT_EQUAL, logic_not_equal, O),
		DECL_R_CC(LOGIC_NOT_EQUAL, logic_not_equal, O),
		DECL_R_CQ(LOGIC_NOT_EQUAL, logic_not_equal, O),
		DECL_R_QR(LOGIC_NOT_EQUAL, logic_not_equal, O),
		DECL_R_QC(LOGIC_NOT_EQUAL, logic_not_equal, O),
		DECL_R_QQ(LOGIC_NOT_EQUAL, logic_not_equal, O),

		DECL_R_R(LOGIC_LESS_L, logic_less_l, O),
		DECL_R_C(LOGIC_LESS_L, logic_less_l, O),
		DECL_R_Q(LOGIC_LESS_L, logic_less_l, O),
		DECL_R_R(LOGIC_LESS_R, logic_less_r, O),
		DECL_R_C(LOGIC_LESS_R, logic_less_r, O),
		DECL_R_Q(LOGIC_LESS_R, logic_less_r, O),
		DECL_R_RR(LOGIC_LESS, logic_less, O),
		DECL_R_RC(LOGIC_LESS, logic_less, O),
		DECL_R_RQ(LOGIC_LESS, logic_less, O),
		DECL_R_CR(LOGIC_LESS, logic_less, O),
		DECL_R_CC(LOGIC_LESS, logic_less, O),
		DECL_R_CQ(LOGIC_LESS, logic_less, O),
		DECL_R_QR(LOGIC_LESS, logic_less, O),
		DECL_R_QC(LOGIC_LESS, logic_less, O),
		DECL_R_QQ(LOGIC_LESS, logic_less, O),
	};
	CLKernel kernels_p08[]=
	{
		DECL_R_R(LOGIC_LESS_EQUAL_L, logic_less_equal_l, O),
		DECL_R_C(LOGIC_LESS_EQUAL_L, logic_less_equal_l, O),
		DECL_R_Q(LOGIC_LESS_EQUAL_L, logic_less_equal_l, O),
		DECL_R_R(LOGIC_LESS_EQUAL_R, logic_less_equal_r, O),
		DECL_R_C(LOGIC_LESS_EQUAL_R, logic_less_equal_r, O),
		DECL_R_Q(LOGIC_LESS_EQUAL_R, logic_less_equal_r, O),
		DECL_R_RR(LOGIC_LESS_EQUAL, logic_less_equal, O),
		DECL_R_RC(LOGIC_LESS_EQUAL, logic_less_equal, O),
		DECL_R_RQ(LOGIC_LESS_EQUAL, logic_less_equal, O),
		DECL_R_CR(LOGIC_LESS_EQUAL, logic_less_equal, O),
		DECL_R_CC(LOGIC_LESS_EQUAL, logic_less_equal, O),
		DECL_R_CQ(LOGIC_LESS_EQUAL, logic_less_equal, O),
		DECL_R_QR(LOGIC_LESS_EQUAL, logic_less_equal, O),
		DECL_R_QC(LOGIC_LESS_EQUAL, logic_less_equal, O),
		DECL_R_QQ(LOGIC_LESS_EQUAL, logic_less_equal, O),

		DECL_R_R(LOGIC_GREATER_L, logic_greater_l, O),
		DECL_R_C(LOGIC_GREATER_L, logic_greater_l, O),
		DECL_R_Q(LOGIC_GREATER_L, logic_greater_l, O),
		DECL_R_R(LOGIC_GREATER_R, logic_greater_r, O),
		DECL_R_C(LOGIC_GREATER_R, logic_greater_r, O),
		DECL_R_Q(LOGIC_GREATER_R, logic_greater_r, O),
		DECL_R_RR(LOGIC_GREATER, logic_greater, O),
		DECL_R_RC(LOGIC_GREATER, logic_greater, O),
		DECL_R_RQ(LOGIC_GREATER, logic_greater, O),
		DECL_R_CR(LOGIC_GREATER, logic_greater, O),
		DECL_R_CC(LOGIC_GREATER, logic_greater, O),
		DECL_R_CQ(LOGIC_GREATER, logic_greater, O),
		DECL_R_QR(LOGIC_GREATER, logic_greater, O),
		DECL_R_QC(LOGIC_GREATER, logic_greater, O),
		DECL_R_QQ(LOGIC_GREATER, logic_greater, O),

		DECL_R_R(LOGIC_GREATER_EQUAL_L, logic_greater_equal_l, O),
		DECL_R_C(LOGIC_GREATER_EQUAL_L, logic_greater_equal_l, O),
		DECL_R_Q(LOGIC_GREATER_EQUAL_L, logic_greater_equal_l, O),
		DECL_R_R(LOGIC_GREATER_EQUAL_R, logic_greater_equal_r, O),
		DECL_R_C(LOGIC_GREATER_EQUAL_R, logic_greater_equal_r, O),
		DECL_R_Q(LOGIC_GREATER_EQUAL_R, logic_greater_equal_r, O),
		DECL_R_RR(LOGIC_GREATER_EQUAL, logic_greater_equal, O),
		DECL_R_RC(LOGIC_GREATER_EQUAL, logic_greater_equal, O),
		DECL_R_RQ(LOGIC_GREATER_EQUAL, logic_greater_equal, O),
		DECL_R_CR(LOGIC_GREATER_EQUAL, logic_greater_equal, O),
		DECL_R_CC(LOGIC_GREATER_EQUAL, logic_greater_equal, O),
		DECL_R_CQ(LOGIC_GREATER_EQUAL, logic_greater_equal, O),
		DECL_R_QR(LOGIC_GREATER_EQUAL, logic_greater_equal, O),
		DECL_R_QC(LOGIC_GREATER_EQUAL, logic_greater_equal, O),
		DECL_R_QQ(LOGIC_GREATER_EQUAL, logic_greater_equal, O),
	};
	CLKernel kernels_p09[]=
	{
		DECL_R_R(LOGIC_NOT, logic_not, O),
		DECL_R_C(LOGIC_NOT, logic_not, O),
		DECL_R_Q(LOGIC_NOT, logic_not, O),

		DECL_R_RR(LOGIC_AND, logic_and, O),
		DECL_R_RC(LOGIC_AND, logic_and, O),
		DECL_R_RQ(LOGIC_AND, logic_and, O),
		DECL_R_CR(LOGIC_AND, logic_and, O),
		DECL_R_CC(LOGIC_AND, logic_and, O),
		DECL_R_CQ(LOGIC_AND, logic_and, O),
		DECL_R_QR(LOGIC_AND, logic_and, O),
		DECL_R_QC(LOGIC_AND, logic_and, O),
		DECL_R_QQ(LOGIC_AND, logic_and, O),

		DECL_R_RR(LOGIC_OR, logic_or, O),
		DECL_R_RC(LOGIC_OR, logic_or, O),
		DECL_R_RQ(LOGIC_OR, logic_or, O),
		DECL_R_CR(LOGIC_OR, logic_or, O),
		DECL_R_CC(LOGIC_OR, logic_or, O),
		DECL_R_CQ(LOGIC_OR, logic_or, O),
		DECL_R_QR(LOGIC_OR, logic_or, O),
		DECL_R_QC(LOGIC_OR, logic_or, O),
		DECL_R_QQ(LOGIC_OR, logic_or, O),

		DECL_R_RR(LOGIC_XOR, logic_xor, O),
		DECL_R_RC(LOGIC_XOR, logic_xor, O),
		DECL_R_RQ(LOGIC_XOR, logic_xor, O),
		DECL_R_CR(LOGIC_XOR, logic_xor, O),
		DECL_R_CC(LOGIC_XOR, logic_xor, O),
		DECL_R_CQ(LOGIC_XOR, logic_xor, O),
		DECL_R_QR(LOGIC_XOR, logic_xor, O),
		DECL_R_QC(LOGIC_XOR, logic_xor, O),
		DECL_R_QQ(LOGIC_XOR, logic_xor, O),
	};
	CLKernel kernels_p10[]=
	{
		DECL_R_RR(CONDITION_ZERO, condition_zero, I),
		DECL_C_RC(CONDITION_ZERO, condition_zero, I),
		DECL_Q_RQ(CONDITION_ZERO, condition_zero, I),
		DECL_C_CR(CONDITION_ZERO, condition_zero, I),
		DECL_C_CC(CONDITION_ZERO, condition_zero, I),
		DECL_Q_CQ(CONDITION_ZERO, condition_zero, I),
		DECL_Q_QR(CONDITION_ZERO, condition_zero, I),
		DECL_Q_QC(CONDITION_ZERO, condition_zero, I),
		DECL_Q_QQ(CONDITION_ZERO, condition_zero, I),

		DECL_R_R(PERCENT, percent, C),
		DECL_C_C(PERCENT, percent, C),
		DECL_Q_Q(PERCENT, percent, C),

		DECL_R_RR(MODULO, modulo, I),
		DECL_C_RC(MODULO, modulo, I),
		DECL_Q_RQ(MODULO, modulo, I),
		DECL_C_CR(MODULO, modulo, I),
		DECL_C_CC(MODULO, modulo, I),
		DECL_Q_CQ(MODULO, modulo, I),
		DECL_Q_QR(MODULO, modulo, I),
		DECL_Q_QC(MODULO, modulo, I),
		DECL_Q_QQ(MODULO, modulo, I),
	};
	CLKernel kernels_p11[]=
	{
		DECL_R_R(SGN, sgn, I),
		DECL_C_C(SGN, sgn, I),
		DECL_Q_Q(SGN, sgn, I),

		DECL_R_R(SQ, sq, C),
		DECL_C_C(SQ, sq, C),
		DECL_Q_Q(SQ, sq, C),

		DECL_C_C(SQRT, sqrt, C),
		DECL_Q_Q(SQRT, sqrt, C),

		DECL_R_R(INVSQRT, invsqrt, C),

		DECL_R_R(CBRT, cbrt, C),
		DECL_C_C(CBRT, cbrt, C),
		DECL_Q_Q(CBRT, cbrt, C),

		DECL_R_R(GAUSS, gauss, C),
		DECL_C_C(GAUSS, gauss, C),
		DECL_Q_Q(GAUSS, gauss, C),

		DECL_R_R(ERF, erf, C),

		DECL_SW(R_R, ZETA, I),
	};
	CLKernel kernels_p12[]=
	{
		DECL_R_R(TGAMMA, tgamma, I),
		DECL_C_C(TGAMMA, tgamma, I),
		DECL_Q_Q(TGAMMA, tgamma, I),
		DECL_SW(R_RR, TGAMMA, I),

		DECL_R_R(LOGGAMMA, loggamma, I),

		DECL_R_R(FACTORIAL, factorial, I),
		DECL_C_C(FACTORIAL, factorial, I),
		DECL_Q_Q(FACTORIAL, factorial, I),

		DECL_R_R(PERMUTATION, permutation, I),
		DECL_C_C(PERMUTATION, permutation, I),
		DECL_Q_Q(PERMUTATION, permutation, I),
		DECL_R_RR(PERMUTATION, permutation, I),
		DECL_C_CR(PERMUTATION, permutation, I),
		DECL_C_CC(PERMUTATION, permutation, I),
		DECL_Q_QQ(PERMUTATION, permutation, I),

		DECL_R_R(COMBINATION, combination, I),
		DECL_C_C(COMBINATION, combination, I),
		DECL_Q_Q(COMBINATION, combination, I),
		DECL_R_RR(COMBINATION, combination, I),
		DECL_C_CR(COMBINATION, combination, I),
		DECL_C_CC(COMBINATION, combination, I),
		DECL_Q_QQ(COMBINATION, combination, I),
	};
	CLKernel kernels_p13[]=
	{
		DECL_R_R(COS, cos, C),
		DECL_C_C(COS, cos, C),
		DECL_Q_Q(COS, cos, C),

		DECL_C_C(ACOS, acos, I),
		DECL_Q_Q(ACOS, acos, I),

		DECL_R_R(COSH, cosh, C),
		DECL_C_C(COSH, cosh, C),
		DECL_Q_Q(COSH, cosh, C),

		DECL_C_C(ACOSH, acosh, C),
		DECL_Q_Q(ACOSH, acosh, C),

		DECL_R_R(COSC, cosc, I),
		DECL_C_C(COSC, cosc, I),
		DECL_Q_Q(COSC, cosc, I),

		DECL_R_R(SEC, sec, I),
		DECL_C_C(SEC, sec, I),
		DECL_Q_Q(SEC, sec, I),

		DECL_C_C(ASEC, asec, I),
		DECL_Q_Q(ASEC, asec, I),

		DECL_R_R(SECH, sech, C),
		DECL_C_C(SECH, sech, I),
		DECL_Q_Q(SECH, sech, I),

		DECL_C_C(ASECH, asech, I),
		DECL_Q_Q(ASECH, asech, I),
	};
	CLKernel kernels_p14[]=
	{
		DECL_R_R(SIN, sin, C),
		DECL_C_C(SIN, sin, C),
		DECL_Q_Q(SIN, sin, C),

		DECL_C_C(ASIN, asin, I),
		DECL_Q_Q(ASIN, asin, I),

		DECL_R_R(SINH, sinh, C),
		DECL_C_C(SINH, sinh, C),
		DECL_Q_Q(SINH, sinh, C),

		DECL_R_R(ASINH, asinh, C),
		DECL_C_C(ASINH, asinh, I),
		DECL_Q_Q(ASINH, asinh, I),

		DECL_R_R(SINC, sinc, C),
		DECL_C_C(SINC, sinc, C),
		DECL_Q_Q(SINC, sinc, C),

		DECL_R_R(SINHC, sinhc, C),
		DECL_C_C(SINHC, sinhc, C),
		DECL_Q_Q(SINHC, sinhc, C),

		DECL_R_R(CSC, csc, I),
		DECL_C_C(CSC, csc, I),
		DECL_Q_Q(CSC, csc, I),

		DECL_C_C(ACSC, acsc, I),
		DECL_Q_Q(ACSC, acsc, I),

		DECL_R_R(CSCH, csch, I),
		DECL_C_C(CSCH, csch, I),
		DECL_Q_Q(CSCH, csch, I),

		DECL_R_R(ACSCH, acsch, I),
		DECL_C_C(ACSCH, acsch, I),
		DECL_Q_Q(ACSCH, acsch, I),
	};
	CLKernel kernels_p15[]=
	{
		DECL_R_R(TAN, tan, I),
		DECL_C_C(TAN, tan, I),
		DECL_Q_Q(TAN, tan, I),

		DECL_R_R(ATAN, atan, C),
		DECL_C_C(ATAN, atan, I),
		DECL_Q_Q(ATAN, atan, I),
		DECL_R_RR(ATAN, atan, I),
		DECL_C_RC(ATAN, atan, I),
		DECL_Q_RQ(ATAN, atan, I),
		DECL_C_CR(ATAN, atan, I),
		DECL_C_CC(ATAN, atan, I),
		DECL_Q_CQ(ATAN, atan, I),
		DECL_Q_QR(ATAN, atan, I),
		DECL_Q_QC(ATAN, atan, I),
		DECL_Q_QQ(ATAN, atan, I),

		DECL_R_R(TANH, tanh, C),
		DECL_C_C(TANH, tanh, C),
		DECL_Q_Q(TANH, tanh, C),

		DECL_C_C(ATANH, atanh, I),
		DECL_Q_Q(ATANH, atanh, I),

		DECL_R_R(TANC, tanc, I),
		DECL_C_C(TANC, tanc, I),
		DECL_Q_Q(TANC, tanc, I),

		DECL_R_R(COT, cot, I),
		DECL_C_C(COT, cot, I),
		DECL_Q_Q(COT, cot, I),

		DECL_R_R(ACOT, acot, I),
		DECL_C_C(ACOT, acot, I),
		DECL_Q_Q(ACOT, acot, I),

		DECL_R_R(COTH, coth, I),
		DECL_C_C(COTH, coth, I),
		DECL_Q_Q(COTH, coth, I),

		DECL_C_C(ACOTH, acoth, I),
		DECL_Q_Q(ACOTH, acoth, I),
	};
	CLKernel kernels_p16[]=
	{
		DECL_R_R(EXP, exp, C),
		DECL_C_C(EXP, exp, C),
		DECL_Q_Q(EXP, exp, C),

		DECL_R_R(FIB, fib, C),
		DECL_C_C(FIB, fib, C),
		DECL_Q_Q(FIB, fib, C),

		DECL_SW(R_R, RANDOM, O),
		DECL_SW(C_C, RANDOM, O),
		DECL_SW(Q_Q, RANDOM, O),
		DECL_SW(R_RR, RANDOM, O),
		DECL_SW(C_CR, RANDOM, O),
		DECL_SW(C_CC, RANDOM, O),
		DECL_SW(Q_QQ, RANDOM, O),

		DECL_SW(R_R, BETA, I),
		DECL_SW(R_RR, BETA, I),

		DECL_SW(R_R, BESSEL_J, I),
		DECL_SW(R_RR, BESSEL_J, I),

		DECL_SW(R_R, BESSEL_Y, I),
		DECL_SW(R_RR, BESSEL_Y, I),

		DECL_SW(C_R, HANKEL1, I),
		DECL_SW(C_C, HANKEL1, I),
		DECL_SW(C_RR, HANKEL1, I),

		DECL_R_R(STEP, step, I),
		DECL_C_C(STEP, step, I),
		DECL_Q_Q(STEP, step, I),

		DECL_R_R(RECT, rect, I),
		DECL_C_C(RECT, rect, I),
		DECL_Q_Q(RECT, rect, I),

		DECL_R_R(TRGL, trgl, C),
		DECL_R_C(TRGL, trgl, C),
		DECL_R_Q(TRGL, trgl, C),
	};
	CLKernel kernels_p17[]=
	{
		DECL_R_R(SQWV, sqwv, O),
		DECL_R_C(SQWV, sqwv, O),
		DECL_R_Q(SQWV, sqwv, O),
		DECL_R_RR(SQWV, sqwv, O),
		DECL_R_RC(SQWV, sqwv, O),
		DECL_R_RQ(SQWV, sqwv, O),
		DECL_R_CR(SQWV, sqwv, O),
		DECL_R_CC(SQWV, sqwv, O),
		DECL_R_CQ(SQWV, sqwv, O),
		DECL_R_QR(SQWV, sqwv, O),
		DECL_R_QC(SQWV, sqwv, O),
		DECL_R_QQ(SQWV, sqwv, O),

		DECL_R_R(TRWV, trwv, C),
		DECL_R_C(TRWV, trwv, C),
		DECL_R_Q(TRWV, trwv, C),
		DECL_R_RR(TRWV, trwv, C),
		DECL_R_RC(TRWV, trwv, C),
		DECL_R_RQ(TRWV, trwv, C),
		DECL_R_CR(TRWV, trwv, C),
		DECL_R_CC(TRWV, trwv, C),
		DECL_R_CQ(TRWV, trwv, C),
		DECL_R_QR(TRWV, trwv, C),
		DECL_R_QC(TRWV, trwv, C),
		DECL_R_QQ(TRWV, trwv, C),

		DECL_R_R(SAW, saw, I),
		DECL_R_C(SAW, saw, I),
		DECL_R_Q(SAW, saw, I),
		DECL_R_RR(SAW, saw, I),
		DECL_R_RC(SAW, saw, I),
		DECL_R_RQ(SAW, saw, I),
		DECL_R_CR(SAW, saw, I),
		DECL_R_CC(SAW, saw, I),
		DECL_R_CQ(SAW, saw, I),
		DECL_R_QR(SAW, saw, I),
		DECL_R_QC(SAW, saw, I),
		DECL_R_QQ(SAW, saw, I),
	};
	CLKernel kernels_p18[]=
	{
		DECL_R_RR(HYPOT, hypot, C),

		DECL_R_R(MANDELBROT, mandelbrot, O),
		DECL_R_C(MANDELBROT, mandelbrot, O),
		DECL_R_RR(MANDELBROT, mandelbrot, O),
		DECL_R_CR(MANDELBROT, mandelbrot, O),

		DECL_R_RR(MIN, min, C),
		DECL_C_CR(MIN, min, C),
		DECL_C_CC(MIN, min, C),
		DECL_Q_QQ(MIN, min, C),

		DECL_R_RR(MAX, max, C),
		DECL_C_CR(MAX, max, C),
		DECL_C_CC(MAX, max, C),
		DECL_Q_QQ(MAX, max, C),
	};
	CLKernel kernels_p19[]=
	{
		DECL_R_RR(CONDITIONAL_110, conditional_110, I),
		DECL_C_RC(CONDITIONAL_110, conditional_110, I),
		DECL_Q_RQ(CONDITIONAL_110, conditional_110, I),
		DECL_R_CR(CONDITIONAL_110, conditional_110, I),
		DECL_C_CC(CONDITIONAL_110, conditional_110, I),
		DECL_Q_CQ(CONDITIONAL_110, conditional_110, I),
		DECL_R_QR(CONDITIONAL_110, conditional_110, I),
		DECL_C_QC(CONDITIONAL_110, conditional_110, I),
		DECL_Q_QQ(CONDITIONAL_110, conditional_110, I),

		DECL_R_RR(CONDITIONAL_101, conditional_101, I),
		DECL_C_RC(CONDITIONAL_101, conditional_101, I),
		DECL_Q_RQ(CONDITIONAL_101, conditional_101, I),
		DECL_R_CR(CONDITIONAL_101, conditional_101, I),
		DECL_C_CC(CONDITIONAL_101, conditional_101, I),
		DECL_Q_CQ(CONDITIONAL_101, conditional_101, I),
		DECL_R_QR(CONDITIONAL_101, conditional_101, I),
		DECL_C_QC(CONDITIONAL_101, conditional_101, I),
		DECL_Q_QQ(CONDITIONAL_101, conditional_101, I),

		{CONDITIONAL_111, 0, 0, "conditional_111", nullptr},//TODO: disc for conditional_111

		DECL_R_R(INCREMENT, increment, C),
		DECL_C_C(INCREMENT, increment, C),
		DECL_Q_Q(INCREMENT, increment, C),

		DECL_R_R(DECREMENT, decrement, C),
		DECL_C_C(DECREMENT, decrement, C),
		DECL_Q_Q(DECREMENT, decrement, C),

		DECL_R_R(ASSIGN, assign, C),
		DECL_C_C(ASSIGN, assign, C),
		DECL_Q_Q(ASSIGN, assign, C),

		//special kernels
		{V_INITIALIZE_PARAMETER, 0, 0, "initialize_parameter", nullptr},
		{V_C2D_RGB, 0, 0, "c2d_rgb", nullptr},
		{V_C2D_RGB2, 0, 0, "c2d_rgb2", nullptr},
		//{V_INITIALIZE_CONSTANTS, 0, 0, "initialize_constants", nullptr},
	};
	KernelDB kernel_db[]=
	{
		{kernels_p00, sizeof(kernels_p00)/sizeof(CLKernel)},
		{kernels_p01, sizeof(kernels_p01)/sizeof(CLKernel)},
		{kernels_p02, sizeof(kernels_p02)/sizeof(CLKernel)},
		{kernels_p03, sizeof(kernels_p03)/sizeof(CLKernel)},
		{kernels_p04, sizeof(kernels_p04)/sizeof(CLKernel)},
		{kernels_p05, sizeof(kernels_p05)/sizeof(CLKernel)},
		{kernels_p06, sizeof(kernels_p06)/sizeof(CLKernel)},
		{kernels_p07, sizeof(kernels_p07)/sizeof(CLKernel)},
		{kernels_p08, sizeof(kernels_p08)/sizeof(CLKernel)},
		{kernels_p09, sizeof(kernels_p09)/sizeof(CLKernel)},
		{kernels_p10, sizeof(kernels_p10)/sizeof(CLKernel)},
		{kernels_p11, sizeof(kernels_p11)/sizeof(CLKernel)},
		{kernels_p12, sizeof(kernels_p12)/sizeof(CLKernel)},
		{kernels_p13, sizeof(kernels_p13)/sizeof(CLKernel)},
		{kernels_p14, sizeof(kernels_p14)/sizeof(CLKernel)},
		{kernels_p15, sizeof(kernels_p15)/sizeof(CLKernel)},
		{kernels_p16, sizeof(kernels_p16)/sizeof(CLKernel)},
		{kernels_p17, sizeof(kernels_p17)/sizeof(CLKernel)},
		{kernels_p18, sizeof(kernels_p18)/sizeof(CLKernel)},
		{kernels_p19, sizeof(kernels_p19)/sizeof(CLKernel)},
	};
//	const int CLKernel_size=sizeof(CLKernel), nkernels=sizeof(kernels)/CLKernel_size;
}
#if 0
const size_t	worksize=100;
float			in1[worksize]={0}, in2[worksize]={0}, out[worksize]={0};
#endif
size_t 			g_maxlocalsize=0, g_maxlocaldim=0;
unsigned		blocking=CL_FALSE;//bad performance when set to true
//#define		CL_BLOCKING
void			checkforbuildfailure(int error, cl_program *programs, int prog_idx, cl_device_id device, std::string &err_msg)
{
	if(error)
	{
		size_t length=0;
		error=p_clGetProgramBuildInfo(programs[prog_idx], device, CL_PROGRAM_BUILD_LOG, g_buf_size, g_buf, &length);	CL_CHECK(error);
		err_msg+="\n\n\tPROGRAM "+std::to_string(prog_idx)+"\n\n"+g_buf;
		LOGE("PROGRAM %d: %s", prog_idx, g_buf);
	}
}
inline bool 	newpointer(void *p0, void *p1){return p0&&p1!=p0;}
#define 		NEWPOINTER(pointer)		newpointer(pointer##0, pointer)
//void			update_cl_state(int state)
//{
//	OCL_state=state;
//}
double			cl_progress(std::string &ret)
{
	switch(OCL_state)
	{
	case CL_NOTHING:			ret="OpenCL API not loaded.";		break;
	case CL_LOADING_API:		ret="Loading OpenCL API...";		break;
	case CL_API_LOADED:			ret="OpenCL API loaded.";			break;
	case CL_CREATING_CONTEXT:	ret="Creating OpenCL context...";	break;
	case CL_CONTEXT_CREATED:	ret="OpenCL context created.";		break;

	case CL_COMPILING_PROGRAM00:
	case CL_COMPILING_PROGRAM01:
	case CL_COMPILING_PROGRAM02:
	case CL_COMPILING_PROGRAM03:
	case CL_COMPILING_PROGRAM04:
	case CL_COMPILING_PROGRAM05:
	case CL_COMPILING_PROGRAM06:
	case CL_COMPILING_PROGRAM07:
	case CL_COMPILING_PROGRAM08:
	case CL_COMPILING_PROGRAM09:
	case CL_COMPILING_PROGRAM10:
	case CL_COMPILING_PROGRAM11:
	case CL_COMPILING_PROGRAM12:
	case CL_COMPILING_PROGRAM13:
	case CL_COMPILING_PROGRAM14:
	case CL_COMPILING_PROGRAM15:
	case CL_COMPILING_PROGRAM16:
	case CL_COMPILING_PROGRAM17:
	case CL_COMPILING_PROGRAM18:
	case CL_COMPILING_PROGRAM19:
		ret="Compiling OpenCL program "+std::to_string(OCL_state+1-CL_COMPILING_PROGRAM00)
				+" of "+std::to_string(nprograms)+"...";
		break;
	case CL_PROGRAMS_COMPILED:
		ret="OpenCL programs compiled.";
		break;
	case CL_RETRIEVING_BINARIES:
		ret="Retrieving OpenCL binaries.";
		break;
	case CL_LOADING_PROGRAM00:
	case CL_LOADING_PROGRAM01:
	case CL_LOADING_PROGRAM02:
	case CL_LOADING_PROGRAM03:
	case CL_LOADING_PROGRAM04:
	case CL_LOADING_PROGRAM05:
	case CL_LOADING_PROGRAM06:
	case CL_LOADING_PROGRAM07:
	case CL_LOADING_PROGRAM08:
	case CL_LOADING_PROGRAM09:
	case CL_LOADING_PROGRAM10:
	case CL_LOADING_PROGRAM11:
	case CL_LOADING_PROGRAM12:
	case CL_LOADING_PROGRAM13:
	case CL_LOADING_PROGRAM14:
	case CL_LOADING_PROGRAM15:
	case CL_LOADING_PROGRAM16:
	case CL_LOADING_PROGRAM17:
	case CL_LOADING_PROGRAM18:
	case CL_LOADING_PROGRAM19:
		ret="Loading OpenCL program "+std::to_string(OCL_state+1-CL_LOADING_PROGRAM00)
				+" of "+std::to_string(nprograms)+"...";
		break;
	case CL_PROGRAMS_LOADED:
		ret="OpenCL programs loaded.";
		break;
	case CL_TESTING:
		ret="Testing OpenCL";
		break;
	case CL_READY:
		ret="OpenCL is ready.";
		break;
	default:
		ret="Unknown state";
		return 0.5;
	}
	return (double)OCL_state/CL_READY;
}
void 			cl_initiate()
{
	load_OpenCL_API();
	using namespace G2_CL;
	static_assert(sizeof(kernel_db)/sizeof(KernelDB)==nprograms, "kernel_db size is wrong");

	if(OCL_state<CL_API_LOADED)
		LOGERROR("Can't create context without OpenCL API");
	else
	{
		OCL_state=CL_CREATING_CONTEXT;
		cl_int error=0;
		size_t retlen=0;
		//static bool firsttime=true;
		//if(firsttime)
		//{
		//	firsttime=false;
		unsigned n_platforms=0, n_devices=0;
		// Fetch the Platform and Device IDs; we only want one.		//https://donkey.vernier.se/~yann/hellocl.c
		error=p_clGetPlatformIDs(1, &platform, &n_platforms);							CL_CHECK(error);
		error=p_clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, &n_devices);	CL_CHECK(error);//was CL_DEVICE_TYPE_ALL

		error=p_clGetDeviceInfo(device, CL_DEVICE_NAME, g_buf_size, g_buf, &retlen);	CL_CHECK(error);	//query device info
		LOGI("\n\n\t%*s\n\n", (int)retlen, g_buf);
		error=p_clGetDeviceInfo(device, CL_DEVICE_VENDOR, g_buf_size, g_buf, &retlen);	CL_CHECK(error);
		LOGI("\n\n\t%*s\n\n", (int)retlen, g_buf);
		error=p_clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &g_maxlocalsize, &retlen);	CL_CHECK(error);
		LOGI("\n\n\tApp resolution: w=%d, h=%d\n\n", w, h);
		LOGI("\n\n\tCL_DEVICE_MAX_WORK_GROUP_SIZE = %d\n\n", (int)g_maxlocalsize);
		error=p_clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, g_buf_size, g_buf, &retlen);	CL_CHECK(error);
		LOGI("\n\n\t%*s\n\n", (int)retlen, g_buf);
		std::string extensions=g_buf;
		auto extpos=extensions.find("cl_khr_gl_sharing");
		cl_gl_interop=extpos!=std::string::npos;
		if(!cl_gl_interop)
			LOGI("cl_khr_gl_sharing not supported");
		//}

		cl_context_properties properties[8]={};
		if(cl_gl_interop)
		{
			auto gl_context=eglGetCurrentContext();//changes when resuming
			auto egl_display=eglGetCurrentDisplay();
			properties[0]=CL_GL_CONTEXT_KHR, properties[1]=(cl_context_properties)gl_context;//https://stackoverflow.com/questions/26802905/getting-opengl-buffers-using-opencl
			properties[2]=CL_EGL_DISPLAY_KHR, properties[3]=(cl_context_properties)egl_display;
			properties[4]=CL_CONTEXT_PLATFORM, properties[5]=(cl_context_properties)platform;
			properties[6]=0, properties[7]=0;
		}
		else
		{
			properties[0]=CL_CONTEXT_PLATFORM, properties[1]=(cl_context_properties)platform;
			properties[2]=0, properties[3]=0;
		}
		context=p_clCreateContext(properties, 1, &device, nullptr, nullptr, &error);	CL_CHECK(error);
		commandqueue=p_clCreateCommandQueue(context, device, 0, &error);				CL_CHECK(error);
		OCL_state=CL_CONTEXT_CREATED;
#if 0
		{
			error=p_clGetPlatformIDs(1, &platform, &n_platforms);							CL_CHECK(error);//
			error=p_clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, &n_devices);	CL_CHECK(error);//
			cl_context_properties properties[]=//https://stackoverflow.com/questions/26802905/getting-opengl-buffers-using-opencl
			{
				CL_GL_CONTEXT_KHR,   (cl_context_properties)eglGetCurrentContext(),
				CL_EGL_DISPLAY_KHR,  (cl_context_properties)eglGetCurrentDisplay(),
				CL_CONTEXT_PLATFORM, (cl_context_properties)platform, // OpenCL platform object
				0, 0,
			};
			context=p_clCreateContext(properties, 1, &device, nullptr, nullptr, &error);	CL_CHECK(error);//
			commandqueue=p_clCreateCommandQueue(context, device, 0, &error);				CL_CHECK(error);//
		}
#endif
		//if(!error)
		//	OCL_state=CL_CONTEXT_CREATED;
#if 0
		float testvalue=42;//DEBUG		always succeeds
		cl_mem mem_in1=p_clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(float), nullptr, &error);	CL_CHECK(error);
		error=p_clEnqueueWriteBuffer(commandqueue, mem_in1, CL_FALSE, 0, sizeof(float), &testvalue, 0, nullptr, nullptr);	CL_CHECK(error);
		testvalue=0;
		error=p_clEnqueueReadBuffer(commandqueue, mem_in1, CL_FALSE, 0, sizeof(float), &testvalue, 0, nullptr, nullptr);	CL_CHECK(error);
		error=p_clFinish(commandqueue);		CL_CHECK(error);//END DEBUG
		//if(NEWPOINTER(platform)||NEWPOINTER(device)||NEWPOINTER(context)||NEWPOINTER(commandqueue))
		//	int LOL_1=0;
#endif

		std::string err_msg;
		if(binaries_valid)
		{
			for(int kp=0;kp<nprograms;++kp)
			{
				OCL_state=CL_COMPILING_PROGRAM00+kp;
				auto &bin=binaries[kp];
				int status=0;
				programs[kp]=p_clCreateProgramWithBinary(context, 1, &device, &bin.size, (const unsigned char**)&bin.bin, &status, &error);	CL_CHECK(error);
				error=p_clBuildProgram(programs[kp], 0, nullptr, "", nullptr, nullptr);		CL_CHECK(error);
				checkforbuildfailure(error, programs, kp, device, err_msg);
			}
			OCL_state=CL_PROGRAMS_COMPILED;
		}
		else//compile from source first
		{
			std::string src_common=CLSource::program_common, src[nprograms];
			for(int kp=0;kp<nprograms;++kp)
			{
				OCL_state=CL_COMPILING_PROGRAM00+kp;
				src[kp]=src_common+CLSource::programs[kp];
				const char *sources[]=
				{
					src[kp].c_str(),
				};
				size_t srclen[]=
				{
					src[kp].size(),
				};
				G2_CL::programs[kp]=p_clCreateProgramWithSource(context, 1, sources, srclen, &error);	CL_CHECK(error);
				error=p_clBuildProgram(programs[kp], 0, nullptr, "", nullptr, nullptr);					CL_CHECK(error);
				checkforbuildfailure(error, programs, kp, device, err_msg);
			}
			OCL_state=CL_PROGRAMS_COMPILED;
			if(!err_msg.empty())
				LOGE_long(err_msg.c_str(), err_msg.size());
			else//build successful, retrieve binaries
			{
				OCL_state=CL_RETRIEVING_BINARIES;
				for(int kp=0;kp<nprograms;++kp)
				{
					auto &bin=binaries[kp];
					error=p_clGetProgramInfo(programs[kp], CL_PROGRAM_BINARY_SIZES, sizeof(size_t), &bin.size, &retlen);	CL_CHECK(error);
					bin.bin=(unsigned char*)malloc(bin.size);
					error=p_clGetProgramInfo(programs[kp], CL_PROGRAM_BINARIES, sizeof(unsigned char*), &bin.bin, &retlen);	CL_CHECK(error);
				}
				if(!error)
					binaries_valid=true;
			}
		}
		if(!error)//build successful: extract kernel handles
		{
			for(int kp=0;kp<nprograms;++kp)
			{
				OCL_state=CL_LOADING_PROGRAM00+kp;
				auto &kernel_batch=kernel_db[kp];
				for(int kk=0;kk<kernel_batch.nkernels;++kk)
				{
					auto &kernel=kernel_batch.kernels[kk];
					if(!kernels[kernel.idx]&&kernel.name)
					{
						kernels[kernel.idx]=p_clCreateKernel(programs[kp], kernel.name, &error);		CL_CHECK(error);
						if(error)
							LOGE("Error line %d:\n\n\tFailed to retrieve kernel %d from program %d:\t%s\n\n", __LINE__, kk, kp, kernel.name);
					}
					//TODO: disc_idx
				}
			}
			OCL_state=CL_PROGRAMS_LOADED;
		}
		OCL_state=CL_TESTING;
#if 1
		//static cl_platform_id platform0=nullptr;
		//static cl_device_id device0=nullptr;
		//static cl_context context0=nullptr;
		//static cl_command_queue commandqueue0=nullptr;
		//platform0=platform;
		//device0=device;
		//context0=context;
		//commandqueue0=commandqueue;
		size_t host_sizes[3]={1, 1, 1}, host_sizes_local[3]={1, 1, 1};//DEBUG		fails after lock
		float host_args[3]={42, 1, 0};
		float testvalue=0;

		cl_mem size_buf=p_clCreateBuffer(context, CL_MEM_READ_WRITE, 2*sizeof(int), nullptr, &error);	CL_CHECK(error);
		error=p_clEnqueueWriteBuffer(commandqueue, size_buf, blocking, 0, 2*sizeof(int), host_sizes, 0, nullptr, nullptr);	CL_CHECK(error);
		//error=p_clFinish(commandqueue), CL_CHECK(error);//succeeds second time
		cl_mem args_buf=p_clCreateBuffer(context, CL_MEM_READ_WRITE, 2*sizeof(float)+sizeof(int), nullptr, &error);	CL_CHECK(error);
		error=p_clEnqueueWriteBuffer(commandqueue, args_buf, blocking, 0, 2*sizeof(float)+sizeof(int), host_args, 0, nullptr, nullptr);	CL_CHECK(error);
		//error=p_clFinish(commandqueue), CL_CHECK(error);//

		cl_mem buffer=p_clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(float), nullptr, &error);	CL_CHECK(error);

		cl_kernel k_fill=kernels[V_INITIALIZE_PARAMETER];
		error=p_clSetKernelArg(k_fill, 0, sizeof(cl_mem), &size_buf);	CL_CHECK(error);
		error=p_clSetKernelArg(k_fill, 1, sizeof(cl_mem), &buffer);		CL_CHECK(error);
		error=p_clSetKernelArg(k_fill, 2, sizeof(cl_mem), &args_buf);	CL_CHECK(error);
		error=p_clEnqueueNDRangeKernel(commandqueue, k_fill, 3, nullptr, host_sizes, host_sizes_local, 0, nullptr, nullptr);	CL_CHECK(error);
		error=p_clEnqueueReadBuffer(commandqueue, buffer, CL_FALSE, 0, sizeof(float), &testvalue, 0, nullptr, nullptr);	CL_CHECK(error);
		error=p_clFinish(commandqueue);		CL_CHECK(error);
		LOGI("G2_CL: TESTVALUE = %f", testvalue);//
		float testvalue2=testvalue;//END DEBUG
#endif
#if 0
		//hellocl.c		//https://donkey.vernier.se/~yann/hellocl.c
		cl_int error=0;
		cl_platform_id platform;
		cl_device_id device;
		unsigned n_platforms, n_devices;
		// Fetch the Platform and Device IDs; we only want one.
		error=p_clGetPlatformIDs(1, &platform, &n_platforms);						CL_CHECK(error);//get available OpenCL platforms & devices
		error=p_clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, &n_devices);	CL_CHECK(error);
		cl_context_properties properties[]=
		{
			CL_CONTEXT_PLATFORM,
			(cl_context_properties)platform,
			0
		};
		cl_context context=p_clCreateContext(properties, 1, &device, nullptr, nullptr, &error);	CL_CHECK(error);//create OpenCL context
		cl_command_queue cq=p_clCreateCommandQueue(context, device, 0, &error);					CL_CHECK(error);

		//compile program & extract kernels
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
		if(error)
		{
			size_t length=0;
			error=p_clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, g_buf_size, g_buf, &length);	CL_CHECK(error);
		}
		cl_kernel k_add=p_clCreateKernel(program, "myfunc", &error);						CL_CHECK(error);

		//use kernel
	//	glFlush();
		for(int k=0;k<worksize;++k)//initialize	TODO: ititialize in kernel
		{
			in1[k]=k;
			in2[k]=worksize-1-k;
			out[k]=rand();//
		}
		cl_mem mem_in1=p_clCreateBuffer(context, CL_MEM_READ_ONLY, worksize*sizeof(float), nullptr, &error);	CL_CHECK(error);//create OpenCL buffers (or use OpenGL buffers)
		cl_mem mem_in2=p_clCreateBuffer(context, CL_MEM_READ_ONLY, worksize*sizeof(float), nullptr, &error);	CL_CHECK(error);
		cl_mem mem_out=p_clCreateBuffer(context, CL_MEM_WRITE_ONLY, worksize*sizeof(float), nullptr, &error);	CL_CHECK(error);
		error=p_clSetKernelArg(k_add, 0, sizeof(cl_mem), &mem_in1); CL_CHECK(error);//set arguments
		error=p_clSetKernelArg(k_add, 1, sizeof(cl_mem), &mem_in2); CL_CHECK(error);
		error=p_clSetKernelArg(k_add, 2, sizeof(cl_mem), &mem_out); CL_CHECK(error);
		error=p_clEnqueueWriteBuffer(commandqueue, mem_in1, CL_FALSE, 0, worksize*sizeof(float), in1, 0, nullptr, nullptr);	CL_CHECK(error);//send arguments
		error=p_clEnqueueWriteBuffer(commandqueue, mem_in2, CL_FALSE, 0, worksize*sizeof(float), in2, 0, nullptr, nullptr);	CL_CHECK(error);
		error=p_clEnqueueNDRangeKernel(commandqueue, k_add, 1, nullptr, &worksize, &worksize, 0, nullptr, nullptr);			CL_CHECK(error);//execute
		error=p_clEnqueueReadBuffer(commandqueue, mem_out, CL_FALSE, 0, worksize*sizeof(float), out, 0, nullptr, nullptr);	CL_CHECK(error);
		error=p_clFinish(commandqueue);	CL_CHECK(error);
		unload_OpenCL_API();//
#endif
		if(!error)
			OCL_state=CL_READY;
		//	OCL_state=CL_KERNELS_LOADED;
	}
}
struct			CLTerm
{
	char mathSet;
	cl_mem r, i, j, k;
	CLTerm():mathSet(0), r(nullptr), i(nullptr), j(nullptr), k(nullptr){}
};
struct 			DebugBuffer
{
	cl_mem buffer;
	const char *name;
};
void			debug_printbuffer(cl_mem buffer, int nfloats, int stride, const char *bufname=nullptr)
{
	auto result=(float*)malloc(nfloats*sizeof(float));
	int error=p_clEnqueueReadBuffer(commandqueue, buffer, CL_TRUE, 0, nfloats*sizeof(float), result, 0, nullptr, nullptr);	CL_CHECK(error);
	for(int k=0;k<nfloats;k+=stride)
		LOGI("G2_CL: %s[%d] = %g", bufname?bufname:"buffer", k, (double)result[k]);
}
void			debug_printbuffers(DebugBuffer *buffers, int nbuffers, int nfloats, int stride)
{
	auto result=(float*)malloc(nbuffers*nfloats*sizeof(float));
	for(int k=0;k<nbuffers;++k)
	{
		int error=p_clEnqueueReadBuffer(commandqueue, buffers[k].buffer, CL_TRUE, 0, nfloats*sizeof(float), result+k*nfloats, 0, nullptr, nullptr);
		CL_CHECK(error);
	}
	for(int k=0;k<nfloats;k+=stride)
	{
		std::stringstream LOL_1;
		LOL_1<<"G2_CL: ["<<k<<"] ";
		for(int kb=0;kb<nbuffers;++kb)
		{
			const char *name=buffers[kb].name;
			LOL_1<<(name?name:"buffer")<<": "<<result[nfloats*kb+k]<<", ";
		}
		LOGI("%s", LOL_1.str().c_str());
	}
	free(result);
}
void 			create_resize_buffer(cl_mem &buffer, unsigned nfloats)
{
	int error=0;
	buffer=p_clCreateBuffer(context, CL_MEM_READ_WRITE, nfloats*sizeof(float), nullptr, &error);
	CL_CHECK(error);
}
void			initialize_const_comp(cl_mem &buffer, unsigned nfloats, float value)
{
	int error=0;
	if(!buffer)
		create_resize_buffer(buffer, nfloats);
	error=p_clEnqueueFillBuffer(commandqueue, buffer, &value, sizeof(float), 0, nfloats*sizeof(float), 0, nullptr, nullptr); CL_CHECK(error);
#ifdef DEBUG2
	float testval=0;
	error=p_clEnqueueReadBuffer(commandqueue, buffer, CL_TRUE, 0, 1*sizeof(float), &testval, 0, nullptr, nullptr);	CL_CHECK(error);
	LOGI("G2_CL: const = %f", testval);
#endif
}
static size_t	host_sizes[3]={},//{globalwidth, globalheight, globaldepth}
				host_sizes_local[3]={};//{localwidth, localheight, localdepth}
static float 	Xstart=0, Xsample=0,
				Yend=0, mYsample=0;//negative Ysample for C2D
unsigned		dimension_work_size(unsigned places){return places&~(g_maxlocaldim-1);}
void 			initialize_component(cl_mem &buffer, int nfloats, char varType, float value, float time, cl_mem size_buf, cl_mem args_buf)
{
	int dimension=-1;//-1: constant
	float data=0;
	switch(varType)
	{
	case 'x':dimension=0;break;
	case 'y':dimension=1;break;
	case 'z':dimension=2;break;
	case 'c':data=value;break;
	case 't':data=time;break;
	default:dimension=3;break;
	}
	if(dimension==-1)
		initialize_const_comp(buffer, nfloats, data);
	else if(dimension<2)//C2D: exactly 2 input dimensions
	{
		if(!buffer)
			create_resize_buffer(buffer, nfloats);
		int error=0;
		float host_args[3];
		if(dimension==0)
			host_args[0]=Xstart, host_args[1]=Xsample;
		else
			host_args[0]=Yend, host_args[1]=mYsample;
		host_args[2]=(float)dimension;
		error=p_clEnqueueWriteBuffer(commandqueue, args_buf, blocking, 0, 3*sizeof(float), host_args, 0, nullptr, nullptr);	CL_CHECK(error);
		cl_kernel k_fill=kernels[V_INITIALIZE_PARAMETER];
		error=p_clSetKernelArg(k_fill, 0, sizeof(cl_mem), &size_buf);	CL_CHECK(error);
		error=p_clSetKernelArg(k_fill, 1, sizeof(cl_mem), &buffer);		CL_CHECK(error);
		error=p_clSetKernelArg(k_fill, 2, sizeof(cl_mem), &args_buf);	CL_CHECK(error);
		int error0=error;
		error=p_clEnqueueNDRangeKernel(commandqueue, k_fill, 3, nullptr, host_sizes, host_sizes_local, 0, nullptr, nullptr);	CL_CHECK(error);
#ifdef DEBUG2
		LOGI("G2_CL: error: %d -> %d, lsizes={%d, %d, %d}", error0, error, (int)host_sizes_local[0], (int)host_sizes_local[1], (int)host_sizes_local[2]);
#endif
		if(blocking)
			{error=p_clFinish(commandqueue);		CL_CHECK(error);}
#ifdef DEBUG2
		float testval=0;
		error=p_clEnqueueReadBuffer(commandqueue, buffer, CL_TRUE, 0, 1*sizeof(float), &testval, 0, nullptr, nullptr);	CL_CHECK(error);
		LOGI("G2_CL: buffer[0] = %f", testval);
#endif
	}
}
void			cl_free_buffer(cl_mem &buffer)
{
	if(buffer)
	{
		int error=p_clReleaseMemObject(buffer);
		CL_CHECK(error);
		if(!error)
			buffer=nullptr;
	}
}
int				*rgb=nullptr;
void			colorFunction_bcw(Comp1d const &v, int *color)
{
	const double &r=v.r, &i=v.i;
	if(r!=r||i!=i)
		*color=0x7F7F7F;
	else if(abs(r)==_HUGE||abs(i)==_HUGE)
		*color=0xFFFFFF;
	else
	{
		double threshold=10, inv_th=1/threshold;//1			//black->color->white		81.98 cycles/px
		const double cos_pi_6=0.866025403784439, sin_pi_6=0.5;
		double hyp=sqrt(r*r+i*i), cosx=r/hyp, sinx=i/hyp,
			mag=255*exp(-hyp*G2::_ln2*inv_th);
		double red=1+cosx*cos_pi_6-sinx*sin_pi_6, green=1+sinx, blue=1+cosx*-cos_pi_6-sinx*sin_pi_6;
		if(hyp<threshold)
			mag=255-mag, red*=mag, green*=mag, blue*=mag;
		else
			red=255-mag*(2-red), green=255-mag*(2-green), blue=255-mag*(2-blue);
		auto p=(unsigned char*)color;
		p[0]=(unsigned char)red, p[1]=(unsigned char)green, p[2]=(unsigned char)blue, p[3]=0xFF;//0xAABBGGRR	//argb
	}
}
void 			cl_solve_c2d(Expression const &ex, double VX, double DX, double VY, double DY, unsigned Xplaces, unsigned Yplaces, double time, unsigned gl_texture)
{//expression -> OpenGL texture
	if(OCL_state<CL_READY)
		LOGERROR("Solve: OCL_state = %d", OCL_state);
	else
	{
		//{//size_t
		//	auto sp=sizeof(void*);				//8
		//	auto ssize=sizeof(size_t);			//8
		//	auto schar=sizeof(char);			//1
		//	auto sshort=sizeof(short);			//2
		//	auto swchar_t=sizeof(wchar_t);		//4
		//	auto sint=sizeof(int);				//4
		//	auto suint=sizeof(unsigned);		//4
		//	auto slong=sizeof(long);			//8
		//	auto sllong=sizeof(long long);		//8
		//	auto sfloat=sizeof(float);			//4
		//	auto sdouble=sizeof(double);		//8
		//	auto sldouble=sizeof(long double);	//16
		//	int LOL_1=0;
		//}//
		prof_add("cl_solve entry");
		glFlush();
		prof_add("glFlush");
		unsigned ndrSize=Xplaces*Yplaces;
		Xstart	=float(VX-DX*0.5), Xsample	=float(DX/Xplaces);
		Yend	=float(VY+DY*0.5), mYsample	=float(-DY/Yplaces);
		auto ftime=(float)time;
		int error=0;
		cl_mem size_buf=p_clCreateBuffer(context, CL_MEM_READ_ONLY, 5*sizeof(int), nullptr, &error);	CL_CHECK(error);
		cl_mem args_buf=p_clCreateBuffer(context, CL_MEM_READ_ONLY, 3*sizeof(float), nullptr, &error);	CL_CHECK(error);
		g_maxlocaldim=(size_t)sqrt(g_maxlocalsize);//determine local work dimensions
		g_maxlocaldim=1<<floor_log2(g_maxlocaldim);
		LOGI("\n\n\tg_maxlocalsize = %d\n\n", (int)g_maxlocaldim);
		unsigned host_sizes32[]=//{globalwidth, globalheight, globaldepth, appwidth, appheight}
		{
			dimension_work_size(Xplaces), dimension_work_size(Yplaces), 1,
			Xplaces, Yplaces
		};
		host_sizes[0]=host_sizes32[0], host_sizes[1]=host_sizes32[1], host_sizes[2]=host_sizes32[2];
		host_sizes_local[0]=g_maxlocaldim, host_sizes_local[1]=g_maxlocaldim, host_sizes_local[2]=1;
		error=p_clEnqueueWriteBuffer(commandqueue, size_buf, blocking, 0, 5*sizeof(int), host_sizes32, 0, nullptr, nullptr);	CL_CHECK(error);//send size buffer
#ifdef DEBUG2
		LOGI("G2_CL: hsizes={%d, %d, %d, %d, %d}", host_sizes32[0], host_sizes32[1], host_sizes32[2], host_sizes32[3], host_sizes32[4]);
		int sizes[5]={};
		error=p_clEnqueueReadBuffer(commandqueue, size_buf, CL_TRUE, 0, 5*sizeof(int), sizes, 0, nullptr, nullptr);	CL_CHECK(error);
		LOGI("G2_CL: sizes={%d, %d, %d, %d, %d}", sizes[0], sizes[1], sizes[2], sizes[3], sizes[4]);
#endif
		int nterms=ex.n.size();
		std::vector<CLTerm> terms(nterms);
		prof_add("prep");
		for(int kn=0;kn<nterms;++kn)//initialize terms
		{
			auto &term=terms[kn];
			auto &n=ex.n[kn];
			auto &val=ex.data[kn];
			if(n.constant)
			{
				term.mathSet=n.mathSet;
				initialize_const_comp(term.r, ndrSize, (float)val.r);
				if(n.mathSet>='c')
				{
					initialize_const_comp(term.i, ndrSize, (float)val.i);
					if(n.mathSet>='h')
					{
						initialize_const_comp(term.j, ndrSize, (float)val.j);
						initialize_const_comp(term.k, ndrSize, (float)val.k);
					}
				}
			}
			else//variable term
			{
				auto &var=ex.variables[n.varNo];
				term.mathSet=var.mathSet;
				initialize_component(term.r, ndrSize, var.varTypeR, (float)val.r, ftime, size_buf, args_buf);
				if(n.mathSet>='c')
				{
					initialize_component(term.i, ndrSize, var.varTypeI, (float)val.i, ftime, size_buf, args_buf);
					if(n.mathSet>='h')
					{
						initialize_component(term.j, ndrSize, var.varTypeJ, (float)val.j, ftime, size_buf, args_buf);
						initialize_component(term.k, ndrSize, var.varTypeK, (float)val.k, ftime, size_buf, args_buf);
					}
				}
			}
		}
#if 0
		if(terms.size()==3)
		{
			DebugBuffer buffers[]=
			{
				{terms[0].r, "x"},
				{terms[1].i, "i"},
				{terms[2].r, "y"},
			};
			debug_printbuffers(buffers, sizeof(buffers)/sizeof(DebugBuffer), ndrSize, 512);
		}
#endif
		prof_add("initialization");
		for(int i=0, nInstr=ex.i.size();i<nInstr;++i)//solve (C2D doesn't have interpolation or disc)
		{
			auto &in=ex.i[i];
			auto kernel=kernels[in.cl_idx];
			if(kernel&&in.type<SIG_INLINE_IF)//TODO: user function -> OpenCL kernel		TODO: inline if
			{
				int arg_number=0;
				error=p_clSetKernelArg(kernel, arg_number, sizeof(cl_mem), &size_buf);	CL_CHECK(error);	++arg_number;
				auto &res=terms[in.result], &op1=terms[in.op1];
				error=p_clSetKernelArg(kernel, arg_number, sizeof(cl_mem), &res.r);	CL_CHECK(error);	++arg_number;
				if(in.r_ms>='c')
				{
					error=p_clSetKernelArg(kernel, arg_number, sizeof(cl_mem), &res.i);	CL_CHECK(error);	++arg_number;
					if(in.r_ms>='h')
					{
						error=p_clSetKernelArg(kernel, arg_number, sizeof(cl_mem), &res.j);	CL_CHECK(error);	++arg_number;
						error=p_clSetKernelArg(kernel, arg_number, sizeof(cl_mem), &res.j);	CL_CHECK(error);	++arg_number;
					}
				}
				error=p_clSetKernelArg(kernel, arg_number, sizeof(cl_mem), &op1.r);	CL_CHECK(error);	++arg_number;
				if(in.op1_ms>='c')
				{
					error=p_clSetKernelArg(kernel, arg_number, sizeof(cl_mem), &op1.i);	CL_CHECK(error);	++arg_number;
					if(in.op1_ms>='h')
					{
						error=p_clSetKernelArg(kernel, arg_number, sizeof(cl_mem), &op1.j);	CL_CHECK(error);	++arg_number;
						error=p_clSetKernelArg(kernel, arg_number, sizeof(cl_mem), &op1.j);	CL_CHECK(error);	++arg_number;
					}
				}
				if(in.op2_ms>='R')
				{
					auto &op2=terms[in.op2];
					error=p_clSetKernelArg(kernel, arg_number, sizeof(cl_mem), &op2.r);	CL_CHECK(error);	++arg_number;
					if(in.op2_ms>='c')
					{
						error=p_clSetKernelArg(kernel, arg_number, sizeof(cl_mem), &op2.i);	CL_CHECK(error);	++arg_number;
						if(in.op2_ms>='h')
						{
							error=p_clSetKernelArg(kernel, arg_number, sizeof(cl_mem), &op2.j);	CL_CHECK(error);	++arg_number;
							error=p_clSetKernelArg(kernel, arg_number, sizeof(cl_mem), &op2.j);	CL_CHECK(error);//	++arg_number;
						}
					}
				}
				error=p_clEnqueueNDRangeKernel(commandqueue, kernel, 3, nullptr, host_sizes, host_sizes_local, 0, nullptr, nullptr);	CL_CHECK(error);
				if(blocking)
					{error=p_clFinish(commandqueue);		CL_CHECK(error);}
			}
		}
#if 0
		if(terms.size()==3)
		{
			DebugBuffer buffers[]=
			{
				{terms[0].r, "x"},
				{terms[1].i, "i"},
				{terms[2].r, "y"},
			};
			debug_printbuffers(buffers, sizeof(buffers)/sizeof(DebugBuffer), ndrSize, 512);
		}
#endif
		prof_add("solve");
		static cl_context context0=nullptr;
		static cl_mem image=nullptr;
		auto &result=terms[ex.resultTerm];					//result -> rgb
		if(result.mathSet=='c')//always true for C2D
		{
			cl_kernel kernel=nullptr;
			if(cl_gl_interop)
			{
				kernel=kernels[V_C2D_RGB];
				if(context0!=context)
				{
					context0=context;
				//	image=p_clCreateFromGLTexture(context, CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, gl_texture, &error);	CL_CHECK(error);//
					image=p_clCreateFromGLTexture(context, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, gl_texture, &error);	CL_CHECK(error);
				}
			}
			else
			{
				kernel=kernels[V_C2D_RGB2];
				image=p_clCreateBuffer(context, CL_MEM_WRITE_ONLY, ndrSize*sizeof(int), nullptr, &error);	CL_CHECK(error);
			}
			error=p_clSetKernelArg(kernel, 0, sizeof(cl_mem), &size_buf);	CL_CHECK(error);
			error=p_clSetKernelArg(kernel, 1, sizeof(cl_mem), &result.r);	CL_CHECK(error);
			error=p_clSetKernelArg(kernel, 2, sizeof(cl_mem), &result.i);	CL_CHECK(error);
			error=p_clSetKernelArg(kernel, 3, sizeof(cl_mem), &image);		CL_CHECK(error);
			error=p_clEnqueueNDRangeKernel(commandqueue, kernel, 2, nullptr, host_sizes, host_sizes_local, 0, nullptr, nullptr);	CL_CHECK(error);
			if(!cl_gl_interop)
			{
				rgb=(int*)realloc(rgb, ndrSize*sizeof(int));
				error=p_clEnqueueReadBuffer(commandqueue, image, CL_FALSE, 0, ndrSize*sizeof(int), rgb, 0, nullptr, nullptr);	CL_CHECK(error);
				cl_free_buffer(image);
#ifdef DEBUG2
				LOGI("G2_CL: rgb[0] = 0x%08X", rgb[0]);
#endif
			}
			if(blocking)
				{error=p_clFinish(commandqueue);		CL_CHECK(error);}
		}
		prof_add("rgb");
#if 0
		{
			DebugBuffer buffers[]=
			{
				{image, "image"},
			};
			debug_printbuffers(buffers, sizeof(buffers)/sizeof(DebugBuffer), ndrSize, 512);
		}
#endif
#if 0
		error=p_clFinish(commandqueue);	CL_CHECK(error);	//DEBUG
		prof_add("clFinish");
		auto
			ndr_r=(float*)malloc(ndrSize*sizeof(float)),
			ndr_i=(float*)malloc(ndrSize*sizeof(float));
		error=p_clEnqueueReadBuffer(commandqueue, result.r, CL_TRUE, 0, ndrSize*sizeof(float), ndr_r, 0, nullptr, nullptr);	CL_CHECK(error);
		error=p_clEnqueueReadBuffer(commandqueue, result.i, CL_TRUE, 0, ndrSize*sizeof(float), ndr_i, 0, nullptr, nullptr);	CL_CHECK(error);
		prof_add("DEBUG read");
		rgb=(int*)realloc(rgb, ndrSize*sizeof(int));
		for(unsigned k=0;k<ndrSize;++k)
		{
			colorFunction_bcw(Comp1d(ndr_r[k], ndr_i[k]), rgb+k);
			//if(!(k&15))
			//	LOGE("\nXY(%g, %g)\t0x%08X", ndr_r[k], ndr_i[k], rgb[k]);
		}
		prof_add("DEBUG rgb");
		LOGI("G2_CL: TOPRIGHT CORNER = %f + %f i, rgb[0] = 0x%08X", ndr_r[0], ndr_i[0], rgb[0]);//DEBUG2
	//	display_texture(0, Xplaces, 0, Yplaces, rgb, Xplaces, Yplaces);
	//	glFlush();
		free(ndr_r);
		free(ndr_i);
		//free(rgb);
#endif
		for(int kn=0;kn<nterms;++kn)//free memory
		{
			auto &term=terms[kn];
			cl_free_buffer(term.r);
			cl_free_buffer(term.i);
			cl_free_buffer(term.j);
			cl_free_buffer(term.k);
		}
		prof_add("free");
#if 0//DEBUG
		error=p_clFinish(commandqueue);	CL_CHECK(error);//DEBUG
		prof_add("clFinish");
		rgb=(int*)realloc(rgb, ndrSize*sizeof(int));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gl_texture);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgb);
#endif
	}
}
void			cl_finish()
{
	if(OCL_state<CL_READY)
		LOGERROR("cl_finish: OCL_state = %d", OCL_state);
//	if(OCL_API_not_loaded)
//		LOGERROR("OpenCL API not loaded.");
	else
	{
		int error=p_clFinish(commandqueue);
		CL_CHECK(error);
		prof_add("clFinish");
	}
}
//void 			show_c2d()
//{//show texture
//	//if(OCL_API_not_loaded)
//	//	LOGERROR("Can't draw without OpenCL API");
//	//else
//	//{
//	//}
//}