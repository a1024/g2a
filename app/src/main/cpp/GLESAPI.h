//GLESAPI.h - Include for OpenGL extension for Grapher 2A.
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
// Created by Omar on 10/4/2019.
//

#ifndef GRAPHER_2_GLESAPI_H
#define GRAPHER_2_GLESAPI_H
//#include <jni.h>
//#include <android/log.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include<vector>
//#include<arm_neon.h>
//#include<immintrin.h>
//#define  LOG_TAG    "libgl2jni"
//#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
//#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#include"g2_common.h"
extern int		w, h, X0, Y0;

extern const unsigned char *GLversion;//OpenGL version info
extern int					glMajorVer, glMinorVer;

//extern int		broken_line;//ill state API
//extern char 	broken_msg[2048];
void			error(const char *msg, int line);
void 			check(int line);
#define 		ERROR(msg)	error(msg, __LINE__)
#define 		CHECK()		check(__LINE__)

//OpenGL API prerequisites
int				floor_log2(unsigned n);
void			prof_add(const char *label, int divisor);

const float _pi=acosf(-1.f), _2pi=2*_pi, pi_2=_pi*0.5f, inv_2pi=1/_2pi, sqrt2=sqrt(2.f), torad=_pi/180,
	inv255=1.f/255, inv256=1.f/256, inv128=1.f/128,
//	inv14=1.f/14,
	inv16=1.f/16;
inline int clamp_positivec(char x){return x&-!(x&0x80);}
inline int clamp_positive(int x)
{
	return (x+abs(x))>>1;
//	return x&-!(x&0x80000000);
}
inline int minimum(int a, int b){return (a+b-abs(a-b))>>1;}
inline int maximum(int a, int b){return (a+b+abs(a-b))>>1;}
inline float minimum(float a, float b){return (a+b-abs(a-b))*0.5f;}
inline float maximum(float a, float b){return (a+b+abs(a-b))*0.5f;}
inline void update_angle(float &th, float &cth, float &sth)
{
	th=th-_2pi*floor(th*inv_2pi);
	sincosf(th, &sth, &cth);
}
struct vec2
{
	float x, y;
	vec2():x(0), y(0){}
	vec2(float x, float y):x(x), y(y){}
	void set(float x, float y){this->x=x, this->y=y;}
	float magnitude(){return sqrt(x*x+y*y);}
};
inline vec2 operator*(float a, vec2 const &b){return vec2(a*b.x, a*b.y);}
inline vec2 operator*(vec2 const &a, float b){return vec2(a.x*b, a.y*b);}
inline vec2 operator+(vec2 const &a, vec2 const &b){return vec2(a.x+b.x, a.y+b.y);}
inline vec2 operator-(vec2 const &a, vec2 const &b){return vec2(a.x-b.x, a.y-b.y);}
//struct vec2i
//{
//	int x, y;
//	vec2i():x(0), y(0){}
//	vec2i(int x, int y):x(x), y(y){}
//	vec2i(vec2 const &a):x(a.x), y(a.y){}
//	void set(int x, int y){this->x=x, this->y=y;}
//};
//vec2 operator-(vec2i const &a, vec2 const &b){return vec2(a.x-b.x, a.y-b.y);}
//vec2i operator-(vec2i const &a, vec2i const &b){return vec2i(a.x-b.x, a.y-b.y);}
//struct vec4i
//{
//	int x1, x2, y1, y2;
//	vec4i():x1(0), x2(0), y1(0), y2(0){}
//	vec4i(int x1, int x2, int y1, int y2):x1(x1), x2(x2), y1(y1), y2(y2){}
//	void set(int x1, int x2, int y1, int y2){this->x1=x1, this->x2=x2, this->y1=y1, this->y2=y2;}
//};
//https://www.3dgep.com/understanding-the-view-matrix/
struct			vec3
{
	float x, y, z;
	vec3():x(0), y(0), z(0){}
	vec3(float x, float y, float z):x(x), y(y), z(z){}
	void set(float x, float y, float z){this->x=x, this->y=y, this->z=z;}
	vec3& operator+=(vec3 const &b){x+=b.x, y+=b.y, z+=b.z; return *this;}
	float& operator[](int idx){return (&x)[idx];}
	float operator[](int idx)const{return (&x)[idx];}
	float dot(vec3 const &other)const{return x*other.x+y*other.y+z*other.z;}
	vec3 cross(vec3 const &other)const{return vec3(y*other.z-z*other.y, z*other.x-x*other.z, x*other.y-y*other.x);}
};
inline vec3		operator*(vec3 const &p, float x){return vec3(p.x*x, p.y*x, p.z*x);}
inline vec3		operator*(float x, vec3 const &p){return vec3(p.x*x, p.y*x, p.z*x);}
inline float	operator*(vec3 const &a, vec3 const &b){return a.x*b.x+a.y*b.y+a.z*b.z;}
inline vec3		operator-(vec3 const &a){return vec3(-a.x, -a.y, -a.z);}
inline vec3		operator-(vec3 const &a, vec3 const &b){return vec3(a.x-b.x, a.y-b.y, a.z-b.z);}
inline vec3		cross(vec3 const &a, vec3 const &b)
{
	return vec3(
			a.y*b.z-a.z*b.y,
			a.z*b.x-a.x*b.z,
			a.x*b.y-a.y*b.x);
}
inline vec3		normalize(vec3 const &a)
{
	float inv_mag=1/sqrt(a.x*a.x+a.y*a.y+a.z*a.z);
	return vec3(a.x*inv_mag, a.y*inv_mag, a.z*inv_mag);
}
struct			vec4
{
	float v[4];//x, y, z, w
	vec4(){v[0]=v[1]=v[2]=v[3]=0;}
	vec4(float x, float y, float z, float w){v[0]=x, v[1]=y, v[2]=z, v[3]=w;}
	vec4(vec3 const &v, float w){this->v[0]=v.x, this->v[1]=v.y, this->v[2]=v.z, this->v[3]=w;}
	const float& operator[](int i)const{return v[i];}
	float& operator[](int i){return v[i];}
	operator vec3()const{return vec3(v[0], v[1], v[2]);}
	void set(float x, float y, float z, float w){this->v[0]=x, this->v[1]=y, this->v[2]=z, this->v[3]=w;}
	void setzero(){memset(v, 0, 4<<2);}
};
inline vec4		operator+(vec4 const &a, vec4 const &b){return vec4(a[0]+b[0], a[1]+b[1], a[2]+b[2], a[3]+b[3]);}
inline vec4		operator*(vec4 const &v, float s)
{
	return vec4(v[0]*s, v[1]*s, v[2]*s, v[3]*s);
}
inline vec4		operator*(float s, vec4 const &v){return v*s;}
struct			mat3
{
	vec3	c[3];
	mat3(){}
	mat3(float gain){memset(c, 0, 9<<2), c[0][0]=c[1][1]=c[2][2]=gain;}
	mat3(vec3 const &c0, vec3 const &c1, vec3 const &c2){c[0]=c0, c[1]=c1, c[2]=c2;}
	float* data(){return &c[0][0];}
	vec3& operator[](int idx){return c[idx];}
	vec3 operator[](int idx)const{return c[idx];}
};
struct			mat4
{
	vec4 c[4];
	mat4()
	{
		c[0]=vec4(1, 0, 0, 0);
		c[1]=vec4(0, 1, 0, 0);
		c[2]=vec4(0, 0, 1, 0);
		c[3]=vec4(0, 0, 0, 1);
	}
	mat4(const float *v){memcpy(c, v, 16<<2);}
	mat4(float gain){memset(c, 0, 16<<2), c[0][0]=c[1][1]=c[2][2]=c[3][3]=gain;}
	mat4(vec4 const &x, vec4 const &y, vec4 const &z, vec4 const &w){c[0]=x, c[1]=y, c[2]=z, c[3]=w;}
	operator mat3()const{return mat3((vec3)c[0], (vec3)c[1], (vec3)c[2]);}
	float* data(){return &c[0][0];}
	void setzero(){memset(c, 0, 16<<2);}
	const vec4& operator[](int j)const{return c[j];}
	vec4& operator[](int j){return c[j];}
	const float& at(int i)const{return ((float*)&c)[i];}
	float& at(int i){return ((float*)&c)[i];}
	//void translate(vec3 const &t)
	//{
	//	at(12)=t.x, at(13)=t.y, at(14)=t.z;
	//}
};
inline mat4		transpose(mat4 const &m)
{
//	float32x4_t swap_low=vtrnq_f32(;
	return mat4(
			vec4(m[0][0], m[1][0], m[2][0], m[3][0]),
			vec4(m[0][1], m[1][1], m[2][1], m[3][1]),
			vec4(m[0][2], m[1][2], m[2][2], m[3][2]),
			vec4(m[0][3], m[1][3], m[2][3], m[3][3]));
}
inline vec4 operator*(mat4 const &m, vec4 const &v)
{
	return vec4(
		m[0][0]*v[0]+m[1][0]*v[1]+m[2][0]*v[2]+m[3][0]*v[3],
		m[0][1]*v[0]+m[1][1]*v[1]+m[2][1]*v[2]+m[3][1]*v[3],
		m[0][2]*v[0]+m[1][2]*v[1]+m[2][2]*v[2]+m[3][2]*v[3],
		m[0][3]*v[0]+m[1][3]*v[1]+m[2][3]*v[2]+m[3][3]*v[3]);
}
inline vec4 operator*(vec4 const &v, mat4 const &m)
{
	return vec4(
		v[0]*m[0][0]+v[1]*m[0][1]+v[2]*m[0][2]+v[3]*m[0][3],
		v[0]*m[1][0]+v[1]*m[1][1]+v[2]*m[1][2]+v[3]*m[1][3],
		v[0]*m[2][0]+v[1]*m[2][1]+v[2]*m[2][2]+v[3]*m[2][3],
		v[0]*m[3][0]+v[1]*m[3][1]+v[2]*m[3][2]+v[3]*m[3][3]);
}
inline mat4 operator*(mat4 const &m1, mat4 const &m2)
{
	return mat4(m1*m2[0], m1*m2[1], m1*m2[2], m1*m2[3]);
}
inline mat4		translate(mat4 const &m, vec3 const &delta)
{//from glm
	vec4 v2(delta, 1);
	mat4 r=m;
	r[3]=m[0]*v2[0]+m[1]*v2[1]+m[2]*v2[2]+m[3];
	return r;
}
inline mat4		rotate(mat4 const &m, float angle, vec3 const &dir)
{//from glm
	float ca=cos(angle), sa=sin(angle);
	vec3 axis=normalize(dir), temp=(1-ca)*axis;
	mat4 rotate(
		vec4(ca+temp[0]*axis[0],			temp[0]*axis[1]+sa*axis[2],	temp[0]*axis[2]-sa*axis[1],	0),//col 1
		vec4(temp[1]*axis[0]-sa*axis[2],	ca+	temp[1]*axis[1],		temp[1]*axis[2]+sa*axis[0],	0),
		vec4(temp[2]*axis[0]+sa*axis[1],	temp[2]*axis[1]-sa*axis[0],	ca+	temp[2]*axis[2],		0),
		vec4(0, 0, 0, 1));//col 4
	return m*rotate;
}
inline mat4		scale(mat4 const &m, vec3 const &ammount)
{
	mat4 r(m[0]*ammount.x, m[1]*ammount.y, m[2]*ammount.z, m[3]);
	return r;
}
inline mat4		lookAt(vec3 const &cam, vec3 const &center, vec3 const &up)
{//from glm
	vec3 f=normalize(center-cam),
		u=normalize(up), s=normalize(f.cross(u));
	u=s.cross(f);
	mat4 r(
		vec4(s, -s.dot(cam)),
		vec4(u, -u.dot(cam)),
		vec4(-f, f.dot(cam)),
		vec4(0, 0, 0, 1));
	r=transpose(r);
	return r;
}
inline mat4		matrixFPSViewRH(vec3 const &_cam, float pitch, float yaw)
{//https://www.3dgep.com/understanding-the-view-matrix/
	vec3 cam(_cam.y, _cam.z, _cam.x);//why yzx?
	float cos_p=cos(pitch), sin_p=sin(pitch), cos_y=cos(yaw), sin_y=sin(yaw);
	vec3
			xaxis(cos_y, 0, -sin_y),
			yaxis(sin_y*sin_p, cos_p, cos_y*sin_p),
			zaxis(sin_y*cos_p, -sin_p, cos_p*cos_y);

	return mat4(
		vec4(xaxis.z, yaxis.z, zaxis.z, 0),//why zxy?
		vec4(xaxis.x, yaxis.x, zaxis.x, 0),
		vec4(xaxis.y, yaxis.y, zaxis.y, 0),
		vec4(-xaxis.dot(cam), -yaxis.dot(cam), -zaxis.dot(cam), 1));
}
inline mat4		perspective(float tanfov, float ar, float znear, float zfar)
{
	return mat4(
		vec4(1/tanfov, 0, 0, 0),
		vec4(0, ar/tanfov, 0, 0),
		vec4(0, 0, -(zfar+znear)/(zfar-znear), -1),
		vec4(0, 0, -2*zfar*znear/(zfar-znear), 0));
}
inline mat3		normalMatrix(mat4 const &m)
{//inverse transpose of top left 3x3 submatrix
	mat3 A=(mat3)m, result;
	double determinant =+A[0][0]*(A[1][1]*A[2][2]-A[2][1]*A[1][2])//https://stackoverflow.com/questions/983999/simple-3x3-matrix-inverse-code-c
                        -A[0][1]*(A[1][0]*A[2][2]-A[1][2]*A[2][0])
                        +A[0][2]*(A[1][0]*A[2][1]-A[1][1]*A[2][0]);
	double invdet = 1/determinant;
	result[0][0]=  (A[1][1]*A[2][2]-A[2][1]*A[1][2])*invdet;
	result[1][0]= -(A[0][1]*A[2][2]-A[0][2]*A[2][1])*invdet;
	result[2][0]=  (A[0][1]*A[1][2]-A[0][2]*A[1][1])*invdet;
	result[0][1]= -(A[1][0]*A[2][2]-A[1][2]*A[2][0])*invdet;
	result[1][1]=  (A[0][0]*A[2][2]-A[0][2]*A[2][0])*invdet;
	result[2][1]= -(A[0][0]*A[1][2]-A[1][0]*A[0][2])*invdet;
	result[0][2]=  (A[1][0]*A[2][1]-A[2][0]*A[1][1])*invdet;
	result[1][2]= -(A[0][0]*A[2][1]-A[2][0]*A[0][1])*invdet;
	result[2][2]=  (A[0][0]*A[1][1]-A[1][0]*A[0][1])*invdet;
	return result;
//	mat4 r=GetTransformInverseNoScale(m);
//	return (mat3)transpose(r);
}
struct			ivec4
{
	union
	{
	//	struct{int x, y, z, w;};
		struct{int x, y, dx, dy;};
		struct{int x1, y1, x2, y2;};
	};
	ivec4(){memset(this, 0, sizeof(ivec4));}
	ivec4(int x, int y, int dx, int dy):x(x), y(y), dx(dx), dy(dy){}
	void set(int x, int y, int dx, int dy){this->x=x, this->y=y, this->dx=dx, this->dy=dy;}
};

struct			Camera
{
	vec3		p0;
	vec2		a0;
	float		tanfov0, dcam0, da0, mouse_sensitivity;

	vec3		p;//position
	vec2		a;//ax: yaw, ay: pitch
	float		tanfov, dcam, da, da_tfov, cax, sax, cay, say;
	Camera():p0(5, 5, 5), a0(225*torad, 324.7356103172454f*torad), tanfov0(1), dcam0(0.04), da0(2*torad), mouse_sensitivity(0.003),
		p(p0), a(a0), dcam(dcam0), tanfov(tanfov0), da(da0), da_tfov(tanfov), cax(cos(a.x)), sax(sin(a.x)), cay(cos(a.y)), say(sin(a.y)){}
	Camera(float x, float y, float z, float ax, float ay, float tanfov):
		p0(x, y, z), a0(ax, ay), tanfov0(tanfov), dcam0(0.04), da0(2*torad), mouse_sensitivity(0.003),
		p(x, y, z), a(ax, ay), tanfov(tanfov), dcam(dcam0), da(da0), da_tfov(tanfov), cax(cos(a.x)), sax(sin(a.x)), cay(cos(a.y)), say(sin(a.y)){}
	void set(float x, float y, float z, float ax, float ay, float tanfov)
	{
		p0.set(x, y, z), a0.set(ax, ay), tanfov0=tanfov, dcam0=0.04, da0=2*torad, mouse_sensitivity=0.003;
		p.set(x, y, z), a.set(ax, ay), this->tanfov=tanfov, dcam=dcam0, da=da0, da_tfov=tanfov, cax=cos(a.x), sax=sin(a.x), cay=cos(a.y), say=sin(a.y);
	}
	void move			(float dx, float dy)
	{
		moveByForward(dy);
		moveByRight(dx);
	}
	void moveByForward	(float dx){p.x+=dx*dcam*cax*cay,	p.y+=dx*dcam*sax*cay,	p.z+=dx*dcam*say;}
	void moveByLeft		(float dx){p.x-=dx*dcam*sax,		p.y+=dx*dcam*cax;}
	void moveByBack		(float dx){p.x-=dx*dcam*cax*cay,	p.y-=dx*dcam*sax*cay,	p.z-=dx*dcam*say;}
	void moveByRight	(float dx){p.x+=dx*dcam*sax,		p.y-=dx*dcam*cax;}
	void moveByUp		(float dx){p.z+=dx*dcam;}
	void moveByDown		(float dx){p.z-=dx*dcam;}

	void moveForward	(){p.x+=dcam*cax*cay,	p.y+=dcam*sax*cay,	p.z+=dcam*say;}
	void moveLeft		(){p.x-=dcam*sax,		p.y+=dcam*cax;}
	void moveBack		(){p.x-=dcam*cax*cay,	p.y-=dcam*sax*cay,	p.z-=dcam*say;}
	void moveRight		(){p.x+=dcam*sax,		p.y-=dcam*cax;}
	void moveUp			(){p.z+=dcam;}
	void moveDown		(){p.z-=dcam;}

	void turnBy			(float ax, float ay)
	{
		update_angle(a.x+=ax, cax, sax);
		update_angle(a.y+=ay, cay, say);
	}
	void turnUp			(){update_angle(a.y+=da_tfov*da, cay, say);}
	void turnDown		(){update_angle(a.y-=da_tfov*da, cay, say);}
	void turnLeft		(){update_angle(a.x+=da_tfov*da, cax, sax);}
	void turnRight		(){update_angle(a.x-=da_tfov*da, cax, sax);}
	//void turnMouse		(long lParam)
	//{
	//	short dx=(short&)lParam, dy=((short*)&lParam)[1];
	//	a.x+=mouse_sensitivity*da_tfov*(X0-dx), update_angle(a.x, cax, sax);
	//	a.y+=mouse_sensitivity*da_tfov*(Y0-dy), update_angle(a.y, cay, say);
	//}
	void zoomIn			(){tanfov/=1.1f, da_tfov=tanfov>1?1:tanfov;}
	void zoomOut		(){tanfov*=1.1f, da_tfov=tanfov>1?1:tanfov;}
	void reset(){p=p0, a=a0, tanfov=tanfov0,	dcam=dcam0, da_tfov=tanfov;}
	void teleport(vec3 const &p, float ax, float ay, float tanfov)
	{
		this->p=p;
		update_angle(a.x=ax, cax, sax);
		update_angle(a.y=ay, cay, say);
		this->tanfov=tanfov;
		da_tfov=tanfov>1?1:tanfov;
	}
	void faster(float A){dcam*=A;}
	void faster(){dcam*=2;}
	void slower(){dcam*=0.5;}

	//vertex conversions - single precision
	void relworld2camscreen(vec3 const &d, vec3 &cp, vec2 &s)const
	{
		float cpt=d.x*cax+d.y*sax;
		cp.x=d.x*sax-d.y*cax, cp.y=cpt*say-d.z*cay, cp.z=cpt*cay+d.z*say;
		cpt=X0/(cp.z*tanfov), s.x=X0+cp.x*cpt, s.y=Y0+cp.y*cpt;
	}
	void world2camscreen(vec3 const &p_world, vec3 &cp, vec2 &s)const{relworld2camscreen(p_world-p, cp, s);}
	void relworld2cam(vec3 const &d, vec3 &cp)const
	{
		float temp=d.x*cax+d.y*sax;
		cp.x=d.x*sax-d.y*cax, cp.y=temp*say-d.z*cay, cp.z=temp*cay+d.z*say;
	}
	void world2cam(vec3 const &p, vec3 &cp)const{relworld2cam(p-this->p, cp);}
	void cam2screen(vec3 const &cp, vec2 &s)const
	{
		float temp=X0/(cp.z*tanfov);
		s.set(X0+cp.x*temp, Y0+cp.y*temp);
	}

	void scaleXabout(float const &VX, float A){p.x=VX+(p.x-VX)*A;}
	void scaleYabout(float const &VY, float A){p.y=VY+(p.y-VY)*A;}
	void scaleZabout(float const &VZ, float A){p.z=VZ+(p.z-VZ)*A;}
	void scaleXYZabout(float const &VX, float const &VY, float const &VZ, float A)
	{
		scaleXabout(VX, A);
		scaleYabout(VY, A);
		scaleZabout(VZ, A);
	}
};

void 			gl_initiate();
void			gl_finish();
inline void		gl_resize(int w, int h){glViewport(0, 0, w, h);}
inline void		gl_newframe(){glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);}
inline void		gl_disabledepthtest(){glDisable(GL_DEPTH_TEST);}
inline void		gl_enabledepthtest(){glEnable(GL_DEPTH_TEST);}
struct			GPUBuffer
{
	unsigned	VBO, EBO;
	int			vertices_stride, vertices_start, normals_stride, normals_start, n_vertices;
	GPUBuffer():VBO(0), EBO(0), n_vertices(0){}
//	void create_VN_I(float *VVVNNN, int n_floats, int *indices, int n_ints);
};
int				print_array(int x, int y, const char *msg, int msg_length, int tab_origin=0);
int				gl_print(int tab_origin, int x, int y, const char *format, ...);
int				gl_vprint(int tab_origin, int x, int y, const char *format, va_list args);
inline int		GUIPrint(int x, int y, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	int result=gl_vprint(0, x, y, format, args);
	va_end(args);
	return result;
}
inline int		print(int x, int y, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	int result=gl_vprint(0, x, y, format, args);
	va_end(args);
	return result;
}
extern int		pen_color, brush_color;
inline void 	set_color(int color)
{
	pen_color=brush_color=color;
}
namespace		GL2_2D
{
	extern bool	continuous;
	void		curve_begin();
	void		curve_point(float x, float y);
	void		draw_curve();
	void		draw_line(float x1, float y1, float x2, float y2);
	void		set_pixel(float x, float y);
	void		draw_rectangle_hollow(float x1, float x2, float y1, float y2);
	void		draw_rectangle_hollow(ivec4 const &p);
	void		draw_rectangle(float x1, float x2, float y1, float y2);
}
namespace		GL2_L3D
{
	void		begin();
	void		push_surface(vec3 const *vn, int vcount_x2, int *idx, int icount, int color);
	void		end();
	void		draw(Camera const &cam, vec3 const &lightpos);
	void		draw_buffer(Camera const &cam, GPUBuffer const &buffer, vec3 const &modelpos, vec3 const &lightpos);
}
namespace		GL2_3D
{
	void		begin();
	void		begin_transparent();
	void		push_square(float x1, float x2, float y1, float y2, float z, int *tx, int txw, int txh);
	//void		push_triangle(vec3 const &a, vec3 const &b, vec3 const &c, int color, int *tx=0, int txw=0, int txh=0);
	void		push_triangle(vec3 const &a, vec3 const &b, vec3 const &c, int color);
	void 		curve_start(int color);
	void 		curve_point(vec3 const &p);
	void		push_line_segment(vec3 const &p1, vec3 const &p2, int color);
	void		push_point(vec3 const &p, int color);
	void		end();
	void		draw(Camera const &cam);
}

//void printGLString(const char *name, GLenum s);
//void checkGlError(const char* op);
//GLuint loadShader(GLenum shaderType, const char* pSource);
//GLuint createProgram(const char* pVertexSource, const char* pFragmentSource);

enum BkMode{TRANSPARENT=1, OPAQUE=2};
extern float	fontH, tb_fontH, tb_fontW, preferred_fontH, tb_offset, preferred_line_width, grid_step, preferred_point_size;
extern int		gl_tabWidth;
extern int		gl_font_size;
extern float	pixel_x, pixel_y, gl_fontH;
extern int		txtColor, bkColor;//OpenGL: 0xAABBGGRR, WinAPI: 0xAARRGGBB
inline void set_font_size(int size)
{//size={1, 17, 27, 37, 47,		56, 66, 76}
//	using namespace Text;
		 if(size<17)	pixel_x=	pixel_y=1;
	else if(size<27)	pixel_x=	pixel_y=2;
	else if(size<37)	pixel_x=	pixel_y=3;
	else if(size<47)	pixel_x=	pixel_y=4;
	else if(size<56)	pixel_x=	pixel_y=5;
	else if(size<66)	pixel_x=5,	pixel_y=6;
	else if(size<76)	pixel_x=5,	pixel_y=7;
	else				pixel_x=5,	pixel_y=8;
	fontH=pixel_y*18;
}
inline void set_font_size_exact(float height, float width=0)
{//width of hypothetical character 16x16 pixels		X at character width=14, height=16 points
//	using namespace Text;
	pixel_y=height*inv16;
	pixel_x=width?width*inv16:pixel_y;
	fontH=height;
}
float			getCharWidth(char c);
float			getTextWidth(const char *a, int length);
float			getTextWidth(const char *a, int i, int f);
int				getTextColor();
int				setTextColor(int color);
int 			getBkColor();
int				setBkColor(int color);
int 			getBkMode();
int				setBkMode(int mode);
struct		Font
{
	float pixel_x, pixel_y;
	int txtColor, bkColor;
//	bool bkOpaque;
	Font():pixel_x(::pixel_x), pixel_y(::pixel_y), txtColor(::txtColor), bkColor(::bkColor){}
	static void change(int txtColor, int bkColor, float font_h, float font_at_w=0)
	{
		::pixel_y=font_h*inv16;
		::pixel_x=font_at_w?font_at_w*inv16: ::pixel_y;
		fontH=font_h;
		setTextColor(txtColor), setBkColor(bkColor);
	//	setBkOpaque(bkOpaque);
		CHECK();
	}
	void revert(){change(txtColor, bkColor, pixel_x, pixel_y);}
};
void 			generate_glcl_texture(unsigned &tx_id, int Xplaces, int Yplaces);
void 			display_gl_texture(unsigned &tx_id);
void			display_texture(int x1, int x2, int y1, int y2, int *rgb, int txw, int txh, unsigned char alpha=0xFF);
inline void 	print_if_error()//TODO: multiline message, implement 'drawtext'
{//ill state API
	if(*first_error_msg)
	{
		print(0, h>>2, "%s", first_error_msg);
		if(strcmp(first_error_msg, latest_error_msg))
			print(0, (h>>2)+fontH, "%s", latest_error_msg);
	}
	//if(broken_line||broken_msg[0])
	//	print(0, h>>2, "%s", broken_msg);
	//	print(0, h>>2, "Line %d: %s", broken_line, broken_msg);
	//	print(0, h>>2, "Error at line %d: %s", broken_line, broken_msg);
}
#endif //GRAPHER_2_GLESAPI_H