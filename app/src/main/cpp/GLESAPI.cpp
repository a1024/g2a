//GLESAPI.cpp - OpenGL extension for Grapher 2A.
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

#include"GLESAPI.h"
#include<string>
//static const float	_pi=acos(-1.f), _2pi=2*_pi, pi_2=_pi*0.5f, inv_2pi=1/_2pi, sqrt2=sqrt(2.f), torad=_pi/180, infinity=(float)_HUGE, inv255=1.f/255, inv256=1.f/256, inv128=1.f/128;
float				fontH=0, tb_fontH=0, tb_fontW=0,
					preferred_fontH=0,
					tb_offset=0,
					preferred_line_width=0,
					grid_step=100, preferred_point_size=1;
inline int			clamp(int lo, int x, int hi)
{
	hi<<=1;
	int temp=x+lo+abs(x-lo);
	return (temp+hi-abs(temp-hi))>>2;
}
static const int	g_buf_size=2048;//maximum line length
static char			g_buf[g_buf_size]={0};

const unsigned char *GLversion;//OpenGL version info
int					glMajorVer, glMinorVer;

int					broken_line=0;//ill state API
char 				broken_msg[g_buf_size]={0};
const char*			glerr2str(int error)
{
#define 			EC(x)	case x:a=(const char*)#x;break
	const char *a=nullptr;
	switch(error)
	{
	EC(GL_INVALID_ENUM);
	EC(GL_INVALID_VALUE);
	EC(GL_INVALID_OPERATION);
	case 0x503:a="GL_STACK_OVERFLOW";break;
	case 0x504:a="GL_STACK_UNDERFLOW";break;
	EC(GL_OUT_OF_MEMORY);
	EC(GL_INVALID_FRAMEBUFFER_OPERATION);
	case 0x507:a="GL_CONTEXT_LOST";break;
	case 0x8031:a="GL_TABLE_TOO_LARGE";break;
	}
	return a;
#undef				EC
}
void				error(const char *msg, int line)
{
	if(!*broken_msg)
	{
		broken_line=line;
		sprintf(broken_msg, "Line %d: %s", line, msg);
	//	sprintf(broken_msg, "Error at line %d: %s", line, msg);
		LOGE("%s", broken_msg);
	}
}
void 				check(int line)
{
	int err=glGetError();
	if(err&&!*broken_msg)
	{
		broken_line=line;
		sprintf(broken_msg, "Line %d: 0x%x - %s.", line, err, glerr2str(err));
	//	sprintf(broken_msg, "Error 0x%x: %s, line %d.", err, glerr2str(err), line);
		LOGE("%s", broken_msg);
	}
}
#define 			ERROR(msg)	error(msg, __LINE__)
#define 			CHECK()		check(__LINE__)

namespace			resources
{
	const char sf10_height=16, sf8_height=12, sf7_height=10, sf6_height=8;
	const char sf10_widths[]=
	{
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,4,4,6,8,8,11,9,4,
		4,4,6,8,4,4,4,4,8,8,
		8,8,8,8,8,8,8,8,4,4,
		8,8,8,8,14,8,10,9,10,9,
		8,10,10,4,7,9,8,12,10,10,
		9,10,10,9,8,10,8,14,9,10,
		9,4,4,4,5,8,5,8,8,7,
		8,8,4,8,8,4,4,7,4,12,
		8,8,8,8,5,8,4,8,8,10,
		8,8,8,5,4,5,5,0
	};
	const char sf8_widths[]=
	{
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,2,3,4,6,6,9,7,2,
		3,3,4,6,3,5,3,3,6,4,
		6,6,6,6,6,6,6,6,3,3,
		6,6,6,6,10,7,7,7,7,6,
		6,8,7,2,5,7,6,8,7,8,
		7,8,7,7,6,7,7,9,7,7,
		6,3,3,3,3,6,3,6,6,6,
		6,6,3,6,6,2,2,5,2,8,
		6,6,6,6,3,5,3,6,5,7,
		5,5,5,3,3,3,3,0,
	};
	const char sf7_widths[]=
	{
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,2,2,4,6,4,5,5,2,
		3,3,3,4,2,3,2,3,5,3,
		5,5,5,4,5,4,5,5,2,2,
		4,4,4,5,8,6,6,6,6,5,
		5,6,5,2,4,5,4,8,6,6,
		5,6,6,5,5,6,6,8,6,6,
		5,3,3,3,4,5,3,4,5,4,
		5,4,3,5,4,2,2,4,2,6,
		4,5,5,5,3,4,3,4,4,6,
		4,4,4,3,2,3,4,0,
	};
	char sf6_widths[]=
	{
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,1,2,3,5,4,5,5,2,
		3,3,3,4,2,3,2,3,4,3,
		4,4,4,4,4,4,4,4,2,2,
		3,3,3,3,7,5,5,5,5,4,
		4,5,5,2,4,5,4,6,5,5,
		5,5,5,5,4,5,4,6,4,4,
		4,3,3,3,4,4,3,4,4,4,
		4,4,3,4,4,2,2,4,2,6,
		4,4,4,4,3,3,3,4,4,5,
		4,4,4,3,2,3,4,0,
	};
	const unsigned char sf10[]=//system font size 10+
	{
		25,209,255,159,199,24,205,243,60,99,80,217,216,252,217,216,108,108,254,108,108,50,182,140,55,207,131,7,195,179,199,96,86,29,188,177,205,108,195,13,176,195,54,179,141,61,184,204,41,199,
		102,179,113,28,219,249,204,159,17,250,207,76,123,219,182,109,155,49,211,179,109,219,182,61,140,21,243,51,158,148,178,99,24,230,103,24,6,59,99,15,72,241,89,145,199,16,51,51,155,153,
		205,204,100,76,189,121,158,231,121,158,103,47,67,202,207,204,204,204,156,49,245,230,25,198,24,99,24,254,140,169,55,195,48,7,195,240,236,101,76,97,156,231,109,123,254,97,56,99,250,15,
		195,240,205,48,60,123,25,83,111,30,134,111,158,231,217,203,152,254,48,134,49,12,99,24,70,198,212,155,231,217,155,231,121,246,50,166,222,60,207,179,15,195,179,151,18,124,128,71,77,218,
		0,96,47,100,9,99,140,49,24,12,6,199,140,126,0,248,67,150,131,193,96,48,198,24,67,198,212,155,103,24,99,24,128,97,34,220,240,192,57,6,102,110,179,61,219,155,189,205,219,108,
		123,6,192,113,240,129,65,197,192,224,225,33,49,51,243,27,30,206,160,254,13,15,15,255,13,15,15,15,255,101,78,121,230,225,96,48,24,12,205,188,12,234,207,216,240,240,240,240,240,240,
		216,79,230,244,63,24,12,126,131,193,96,240,207,156,254,7,131,193,111,48,24,12,6,25,84,62,227,193,129,129,249,225,97,99,222,12,234,240,240,240,240,255,240,240,240,240,112,70,244,255,
		255,49,166,48,12,195,48,12,207,179,151,65,29,155,217,120,56,120,216,152,25,27,206,156,14,6,131,193,96,48,24,12,254,25,213,129,7,62,252,240,231,159,223,123,239,153,103,206,160,14,
		31,63,63,111,111,207,207,143,15,103,80,121,204,134,135,135,135,135,135,205,120,50,168,127,195,195,195,195,127,3,3,3,3,25,84,30,179,225,225,225,225,225,121,51,254,76,234,159,97,195,
		134,13,251,51,108,216,176,193,153,83,223,120,60,112,224,192,227,177,15,131,250,143,129,129,129,129,129,129,129,129,145,65,29,30,30,30,30,30,30,30,54,227,193,160,14,15,155,153,153,145,
		240,240,96,96,96,92,135,225,97,120,24,54,207,204,51,150,134,231,193,48,48,12,12,3,147,58,120,176,49,54,56,112,176,49,54,120,48,70,117,224,129,13,99,6,15,24,96,128,1,6,
		24,48,169,255,1,3,3,3,3,3,3,3,3,254,103,166,191,109,219,182,237,24,226,153,25,51,51,102,102,204,244,109,219,182,109,31,83,67,220,198,99,252,71,104,206,56,102,231,205,188,
		61,207,126,198,116,24,134,111,158,231,121,254,98,118,222,60,12,195,236,101,76,97,24,246,231,121,158,103,63,102,231,205,255,48,204,30,134,148,205,222,204,204,44,102,234,207,243,60,207,62,
		60,123,25,211,97,24,190,121,158,231,121,206,136,62,255,63,102,218,134,109,219,182,151,49,29,134,97,222,158,227,217,230,140,232,255,255,199,244,254,155,121,230,153,103,158,121,230,152,221,55,
		207,243,60,207,49,59,111,158,231,121,246,98,166,223,60,207,243,252,13,195,16,51,245,231,121,158,103,31,134,225,24,221,223,204,204,196,236,188,121,240,224,217,3,81,154,189,153,153,113,204,
		110,158,231,121,158,125,24,222,240,176,153,25,15,6,6,76,111,224,153,109,179,205,63,102,152,129,225,13,155,241,96,240,152,13,195,80,135,135,205,204,120,120,48,48,24,12,98,118,63,140,
		49,198,240,103,168,217,204,204,198,204,204,56,35,253,255,255,63,134,122,204,204,140,205,204,108,48,37,183,3,
	};
	const unsigned char sf8[]=//small fonts size 8
	{
		145,184,111,100,164,141,212,41,245,213,87,42,82,72,92,53,172,58,146,185,27,142,32,8,130,56,28,49,52,146,196,72,163,200,141,132,28,145,180,170,50,34,153,170,90,68,228,67,85,132,
		124,66,146,34,22,163,248,100,196,136,160,106,85,164,208,197,24,99,140,46,50,200,147,36,25,41,116,17,34,34,194,143,20,186,8,25,132,209,69,10,17,83,41,125,132,34,133,63,132,7,
		97,116,145,66,23,195,139,49,186,72,225,135,8,17,66,72,164,208,197,232,98,140,46,82,232,98,244,16,70,151,138,138,169,176,56,67,135,36,33,132,41,154,135,207,208,133,16,146,68,164,
		208,69,136,8,1,36,131,200,35,148,89,89,153,38,192,19,49,100,24,73,146,254,48,140,24,126,97,248,133,97,248,69,12,189,48,8,130,32,244,34,134,95,24,134,97,24,126,145,194,31,
		194,11,33,252,72,225,15,225,133,16,66,228,208,23,12,4,226,193,160,47,98,24,134,225,31,134,97,24,9,252,143,16,34,34,98,166,69,12,195,40,57,142,36,10,35,133,33,132,16,66,
		248,145,195,241,184,90,77,38,131,193,136,225,56,150,101,154,198,113,228,208,23,12,6,131,193,160,47,98,248,133,97,248,5,65,16,185,244,5,131,193,96,52,236,3,70,12,191,48,12,191,
		48,12,35,134,94,24,120,32,24,122,145,194,79,8,33,132,144,136,97,24,134,97,24,134,94,196,48,12,67,73,146,24,38,130,24,24,24,40,164,165,69,66,18,49,12,67,137,97,164,48,
		132,28,6,131,34,10,2,129,64,68,10,63,68,68,132,240,35,146,87,85,103,228,170,213,136,228,170,234,144,17,21,106,241,71,68,50,85,229,160,143,62,82,24,66,120,49,198,151,170,114,
		49,68,23,41,132,16,250,24,163,79,85,185,248,131,139,8,90,87,165,234,124,140,209,195,23,41,12,33,180,25,99,204,196,245,51,145,253,35,132,17,145,53,149,145,192,255,212,213,47,153,
		76,38,83,85,47,198,24,83,85,46,198,232,82,117,47,198,248,66,72,213,249,24,163,135,48,37,117,85,138,202,195,240,66,98,93,153,170,138,49,102,155,162,202,76,179,84,85,172,85,169,
		20,85,166,101,166,232,50,83,73,34,69,245,36,241,17,73,213,84,35,145,255,35,146,85,86,65,70,60,
	};
	const unsigned char sf7[]=//small fonts size 7
	{
		145,176,55,50,210,70,202,212,87,234,171,200,156,62,249,138,144,205,146,52,71,200,164,164,75,35,33,71,4,173,42,35,130,169,106,17,145,15,205,232,202,133,156,146,56,23,49,34,166,86,
		17,50,203,204,180,136,152,171,70,200,44,209,226,35,100,150,132,105,17,50,50,171,167,200,216,51,114,17,50,139,203,180,200,216,41,73,17,50,75,203,180,8,153,165,99,90,40,40,67,81,
		57,51,69,69,12,205,28,207,76,69,84,34,100,150,36,32,145,59,142,40,89,205,9,56,145,50,66,148,139,49,82,246,226,139,241,69,202,92,12,33,186,72,217,139,49,198,23,33,251,184,
		136,143,144,125,92,68,68,202,92,12,57,250,8,89,230,103,102,36,236,143,140,145,100,23,33,203,154,169,140,140,37,73,30,57,27,143,171,213,100,50,82,54,103,173,57,71,202,92,140,49,
		186,8,217,101,94,68,164,204,197,88,147,141,148,189,24,95,140,17,50,75,161,52,72,217,39,132,16,18,41,139,49,198,232,34,101,49,42,69,72,228,44,24,84,169,40,148,72,89,84,132,
		168,24,41,139,138,16,66,34,100,143,36,241,17,185,171,58,34,86,169,17,185,85,61,50,162,2,43,126,66,36,67,67,99,29,33,139,184,204,11,13,57,113,132,12,209,51,61,52,164,198,
		17,49,187,10,145,121,166,163,69,198,146,181,141,132,245,145,192,254,200,88,210,181,145,176,63,84,244,106,173,161,161,181,13,17,89,166,133,200,46,243,34,66,100,158,233,136,33,161,171,208,
		144,195,101,164,186,12,13,181,117,104,168,149,66,69,181,42,21,26,170,212,208,88,91,41,161,161,51,143,200,169,169,145,184,127,68,174,178,130,144,104,1,
	};
	const unsigned char sf6[]=//small fonts size 6
	{
		145,168,27,17,249,8,149,189,189,69,198,244,249,202,16,37,73,70,168,164,180,52,18,114,68,204,42,35,98,169,22,17,249,204,140,174,88,200,33,137,99,17,35,82,122,69,166,212,86,17,
		41,87,35,83,163,242,200,212,40,46,50,69,247,140,76,61,231,34,83,202,170,200,212,41,41,50,165,170,138,76,169,167,66,49,13,5,117,72,198,76,73,28,146,201,34,82,25,69,204,24,
		169,237,5,38,66,101,150,159,17,170,203,203,139,80,89,70,90,132,234,50,243,34,83,207,242,200,212,179,36,66,229,209,233,17,170,204,207,140,68,253,200,20,201,46,66,149,53,149,145,169,
		36,121,164,42,238,26,99,132,42,183,51,35,84,150,153,22,161,186,188,136,8,149,101,150,70,168,46,47,51,66,229,97,120,145,169,75,82,132,42,51,211,34,83,109,165,72,85,172,85,169,
		200,84,171,54,50,213,74,138,76,157,202,35,98,87,29,145,170,53,34,182,234,145,17,21,71,241,17,145,12,205,184,142,76,37,235,66,51,142,35,83,164,235,208,140,186,136,148,93,161,33,
		215,140,76,37,107,35,81,61,18,214,71,166,146,174,145,168,31,170,121,181,134,102,214,134,102,222,135,134,214,37,52,228,154,33,153,11,52,99,45,35,212,25,154,105,29,154,105,21,162,201,
		252,208,76,213,208,80,165,132,102,174,71,196,180,52,18,245,35,98,165,21,25,241,0,
	};
	void			uncompress_bitmap_v4(const unsigned char *data, int data_size, char fontH, const char *widths, int *&rgb2, int &w2, int &h2)
	{
		const int red=0xFF0000FF, green=0xFF00FF00, blue=0xFFFF0000;//swapped in WinAPI bitmaps
		w2=128, h2=256;
		const int size=w2*h2;
		rgb2=(int*)malloc(size<<2);
		for(int k=0;k<size;++k)
			rgb2[k]=blue;
		int nbits=data_size<<3;
		std::vector<bool> cdata(nbits);
		for(int k=0;k<nbits;k+=8)
			for(int bit=0;bit<8;++bit)
				cdata[k+bit]=data[k>>3]>>bit&1;
		{
			int c=' ';
			int xstart=(c&7)<<4, ystart=(c>>3)<<4;
			int bkwidth=widths[c];
			for(int ky=0;ky<fontH;++ky)
				for(int kx=0;kx<bkwidth;++kx)
					rgb2[(ystart+ky)<<7|(xstart+kx)]=green;
		}
		for(int c='!', pos=0;c<127;++c)//95 printable characters, space is all-bk
		{
			int xstart=(c&7)<<4, ystart=(c>>3)<<4;
			int bkwidth=widths[c];
			for(int ky=0;ky<fontH;++ky)
				for(int kx=0;kx<bkwidth;++kx)
					rgb2[(ystart+ky)<<7|(xstart+kx)]=green;
			int	xoffset=(int)cdata[pos+2]<<2|(int)cdata[pos+1]<<1|(int)cdata[pos],
				yoffset=(int)cdata[pos+6]<<3|(int)cdata[pos+5]<<2|(int)cdata[pos+4]<<1|(int)cdata[pos+3],
				width=(int)cdata[pos+10]<<3|(int)cdata[pos+9]<<2|(int)cdata[pos+8]<<1|(int)cdata[pos+7],
				height=(int)cdata[pos+14]<<3|(int)cdata[pos+13]<<2|(int)cdata[pos+12]<<1|(int)cdata[pos+11];
			xstart+=xoffset, ystart+=yoffset;
			pos+=15;
			for(int ky=0;ky<height;++ky)
			{
				for(int kx=0;kx<width;++kx)
				{
					if(cdata[pos])
						rgb2[(ystart+ky)<<7|(xstart+kx)]=red;
					++pos;
				}
			}
		}
	}
}
struct				ShaderVar
{
	int *pvar;
	const char *name;
	int lineNo;//__LINE__
};
unsigned			CompileShader(const char *src, unsigned type)
{
	unsigned shaderID=glCreateShader(type);
	glShaderSource(shaderID, 1, &src, 0);
	glCompileShader(shaderID);
	int success=0;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
	if(!success)
	{
		int infoLogLength;
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
		std::string errorMessage;
		errorMessage.resize(infoLogLength+1, 0);
		glGetShaderInfoLog(shaderID, infoLogLength, 0, &errorMessage[0]);
	//	copy_to_clipboard(&errorMessage[0], infoLogLength);
		ERROR(errorMessage.c_str());
		return 0;
	}
	return shaderID;
}
unsigned			LoadShaders(const char *vertSrc, const char *fragSrc, ShaderVar *attributes, int n_attrib, ShaderVar *uniforms, int n_unif)
{
	unsigned
			vertShaderID=CompileShader(vertSrc, GL_VERTEX_SHADER),
			fragShaderID=CompileShader(fragSrc, GL_FRAGMENT_SHADER);
	//	prof_add("compile sh");
	if(!vertShaderID||!fragShaderID)
	{
		ERROR("Shaders didn\'t compile");
		return 0;
	}
	unsigned ProgramID=glCreateProgram();
	glAttachShader(ProgramID, vertShaderID);
	glAttachShader(ProgramID, fragShaderID);
	glLinkProgram(ProgramID);
	//	prof_add("link");
	int success=0;
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &success);
	if(!success)
	{
		int infoLogLength;
		glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &infoLogLength);
		std::string errorMessage;
		errorMessage.resize(infoLogLength+1, 0);
		glGetProgramInfoLog(ProgramID, infoLogLength, 0, &errorMessage[0]);
	//	copy_to_clipboard(&errorMessage[0], infoLogLength);
		ERROR(errorMessage.c_str());
		return 0;
	}
	glDetachShader(ProgramID, vertShaderID);
	glDetachShader(ProgramID, fragShaderID);
	glDeleteShader(vertShaderID);
	glDeleteShader(fragShaderID);
	//	prof_add("delete");
	CHECK();
	for(int ka=0;ka<n_attrib;++ka)
		if((*attributes[ka].pvar=glGetAttribLocation(ProgramID, attributes[ka].name))==-1)
			error("Can\'t get attribute location", attributes[ka].lineNo);
	for(int ku=0;ku<n_unif;++ku)
		if((*uniforms[ku].pvar=glGetUniformLocation(ProgramID, uniforms[ku].name))==-1)
			error("Can\'t get uniform location", uniforms[ku].lineNo);
	if(broken_line)//
		return 0;//
	return ProgramID;
}
unsigned			make_gpu_buffer(unsigned target, const void *pointer, int size_bytes)
{
	unsigned buffer_id=0;
	glGenBuffers(1, &buffer_id);
	glBindBuffer(target, buffer_id);
	glBufferData(target, size_bytes, pointer, GL_STATIC_DRAW);
	CHECK();
	return buffer_id;
}
void				LoadFontTexture(const unsigned char *data, int data_size, char fontH, const char *widths, unsigned &tx_id)
{
	int w2, h2;
	int *rgb2=nullptr;
	resources::uncompress_bitmap_v4(data, data_size, fontH, widths, rgb2, w2, h2);

	glGenTextures(1, &tx_id);			CHECK();
	glBindTexture(GL_TEXTURE_2D, tx_id);	CHECK();
	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);		CHECK();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	CHECK();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	CHECK();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w2, h2, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgb2);	CHECK();

	free(rgb2);
}
void				calculate_text_txcoords(const char *font_widths, int gl_fontH, float *txcoord)
{
	for(int c=0, idx=0;c<128;++c, idx+=4)
	{
		int width=font_widths[c];
		int xpos=(c&0x7)<<4, ypos=(c>>3&0xF)<<4;
		txcoord[idx  ]=xpos*inv128, txcoord[idx+1]=(xpos+width)*inv128;
		txcoord[idx+2]=ypos*inv256, txcoord[idx+3]=(ypos+gl_fontH)*inv256;
	}
}
unsigned			current_program=0;
inline void			gl_setProgram(unsigned program)
{
//	if(current_program!=program)
//	{
		glUseProgram(current_program=program);
		CHECK();
//	}
}
void				send_color(unsigned location, int color)
{
	auto p=(unsigned char*)&color;
	glUniform4f(location, p[0]*inv255, p[1]*inv255, p[2]*inv255, p[3]*inv255);
	CHECK();
	if(broken_msg[0])
	{
		return;
	}
}
void				send_color_rgb(unsigned location, int color)
{
	auto p=(unsigned char*)&color;
	glUniform3f(location, p[0]*inv255, p[1]*inv255, p[2]*inv255);
}
void				select_texture(unsigned tx_id, int u_location)
{
	glActiveTexture(GL_TEXTURE0);			CHECK();
	glBindTexture(GL_TEXTURE_2D, tx_id);	CHECK();
	glUniform1i(u_location, 0);				CHECK();
}
float				g_fbuf[16]={0};
int					pen_color=0xFF000000, brush_color=0xFFFFFFFF;
namespace			GL2_2D
{
	unsigned		program=0;
	int				u_color=-1, a_vertices=-1;
	unsigned		vertex_buffer=0;
	ivec4			region, current_region;//x1, y1, dx, dy
	bool			continuous=true;
	struct			DrawInfo
	{
		int count, color;
		DrawInfo(int count, int color):count(count), color(color){}
	};
	std::vector<DrawInfo> drawInfo;
	std::vector<vec2> vertices;
	void		set_region(int x1, int x2, int y1, int y2){region.set(x1, y1, x2-x1, y2-y1);}
	void		use_region()
	{
		glViewport(region.x1, region.y1, region.dx, region.dy);
		current_region=region;
	}
	void		drop_region()
	{
		glViewport(0, 0, w, h);
		current_region.set(0, 0, w, h);
	}
	void		toNDC(float xs, float ys, float &xn, float &yn)
	{
		xn=(xs-(float)current_region.x1)*2.f/(float)current_region.dx-1;
		yn=1-(ys-(float)current_region.y1)*2.f/(float)current_region.dy;
	}
	void		curve_begin(){vertices.clear(), drawInfo.clear();}
	void		curve_point(float x, float y)
	{
		vec2 ndc;
		toNDC(x, y, ndc.x, ndc.y);
		vertices.push_back(ndc);
		//float _2_w=2.f/w, _2_h=2.f/h;
		//vertices.push_back(vec2(x*_2_w-1, 1-y*_2_h));
		bool increment=false;
		if(!drawInfo.empty())
		{
			auto &di=*drawInfo.rbegin();
			increment=continuous&(di.color==pen_color);
		}
		if(increment)
			++drawInfo.rbegin()->count;
		else
			drawInfo.push_back(DrawInfo(1, pen_color));
		continuous=true;
	}
	void		draw_curve()
	{
		using namespace GL2_2D;
		gl_setProgram(program);									CHECK();

		glEnableVertexAttribArray(a_vertices);					CHECK();
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);			CHECK();
		glBufferData(GL_ARRAY_BUFFER, (int)vertices.size()*sizeof(vec2), &vertices[0], GL_STATIC_DRAW);	CHECK();
		glVertexAttribPointer(a_vertices, 2, GL_FLOAT, GL_FALSE, 0, 0);							CHECK();

	//	glBindBuffer(GL_ARRAY_BUFFER, 0);												CHECK();
	//	glVertexAttribPointer(a_vertices, 2, GL_FLOAT, GL_FALSE, 0, &vertices[0]);		CHECK();//doesn't draw

		for(int k=0, kEnd=GL2_2D::drawInfo.size(), idx=0;k<kEnd;++k)
		{
			auto &di=drawInfo[k];
			send_color(GL2_2D::u_color, di.color);										CHECK();
			glDrawArrays(di.count==1?GL_POINTS:GL_LINE_STRIP, idx, di.count);	CHECK();
			idx+=di.count;
			prof_add("curve", 1);//
		}
		glDisableVertexAttribArray(a_vertices);					CHECK();
	}
	void		draw_line(float x1, float y1, float x2, float y2)
	{
		toNDC(x1, y1, g_fbuf[0], g_fbuf[1]);
		toNDC(x2, y2, g_fbuf[2], g_fbuf[3]);
		//float _2_w=2.f/w, _2_h=2.f/h;
		//g_fbuf[0]=x1*_2_w-1, g_fbuf[1]=1-y1*_2_h;
		//g_fbuf[2]=x2*_2_w-1, g_fbuf[3]=1-y2*_2_h;
		gl_setProgram(program);					CHECK();
		send_color(u_color, pen_color);			CHECK();
		glEnableVertexAttribArray(a_vertices);	CHECK();

		glBindBuffer(GL_ARRAY_BUFFER, GL2_2D::vertex_buffer);				CHECK();
		glBufferData(GL_ARRAY_BUFFER, 4<<2, g_fbuf, GL_STATIC_DRAW);	CHECK();//use glBufferSubData
		glVertexAttribPointer(a_vertices, 2, GL_FLOAT, GL_FALSE, 0, 0);	CHECK();

	//	glBindBuffer(GL_ARRAY_BUFFER, 0);										CHECK();
	//	glVertexAttribPointer(a_vertices, 2, GL_FLOAT, GL_FALSE, 0, g_fbuf);	CHECK();//use buffer

		glDrawArrays(GL_LINES, 0, 2);			CHECK();
	//	prof_add("point", 1);//
	}
	void		set_pixel(float x, float y)
	{
		toNDC(x, y, g_fbuf[0], g_fbuf[1]);
		//float _2_w=2.f/w, _2_h=2.f/h;
		//g_fbuf[0]=x*_2_w-1, g_fbuf[1]=1-y*_2_h;
		gl_setProgram(GL2_2D::program);			CHECK();
		send_color(u_color, pen_color);			CHECK();
		glEnableVertexAttribArray(a_vertices);	CHECK();

		glBindBuffer(GL_ARRAY_BUFFER, GL2_2D::vertex_buffer);				CHECK();
		glBufferData(GL_ARRAY_BUFFER, 2<<2, g_fbuf, GL_STATIC_DRAW);	CHECK();
		glVertexAttribPointer(a_vertices, 2, GL_FLOAT, GL_FALSE, 0, 0);	CHECK();

	//	glVertexAttribPointer(a_vertices, 2, GL_FLOAT, GL_FALSE, 0, g_fbuf);	CHECK();//use buffer

		glDrawArrays(GL_POINTS, 0, 1);			CHECK();
	}
	void		draw_rectangle_hollow(float x1, float x2, float y1, float y2)
	{
		float X1, X2, Y1, Y2;
		toNDC(x1, y1, X1, Y1);
		toNDC(x2, y2, X2, Y2);
		//float _2_w=2.f/w, _2_h=2.f/h;
		//float X1=x1*_2_w-1, X2=x2*_2_w-1, Y1=1-y1*_2_h, Y2=1-y2*_2_h;
		g_fbuf[0]=X1, g_fbuf[1]=Y1;
		g_fbuf[2]=X2, g_fbuf[3]=Y1;
		g_fbuf[4]=X2, g_fbuf[5]=Y2;
		g_fbuf[6]=X1, g_fbuf[7]=Y2;
		g_fbuf[8]=X1, g_fbuf[9]=Y1;
	//	g_fbuf[10]=X1, g_fbuf[11]=Y1;
		gl_setProgram(program);								CHECK();
		send_color(u_color, 0xFF000000|pen_color);	CHECK();

		glBindBuffer(GL_ARRAY_BUFFER, 0);											CHECK();
		glVertexAttribPointer(a_vertices, 2, GL_FLOAT, GL_FALSE, 0, g_fbuf);	CHECK();

		glEnableVertexAttribArray(a_vertices);			CHECK();
		glDrawArrays(GL_LINE_STRIP, 0, 5);	CHECK();
	//	glDrawArrays(GL_POINTS, 5, 1);					CHECK();
	}
	void		draw_rectangle_hollow(ivec4 const &p)
	{
		draw_rectangle_hollow((float)p.x1, (float)p.y1, (float)p.x2, (float)p.y2);
	}
	void		draw_rectangle(float x1, float x2, float y1, float y2)
	{
		float X1, X2, Y1, Y2;
		toNDC(x1, y1, X1, Y1);
		toNDC(x2, y2, X2, Y2);
		//float _2_w=2.f/w, _2_h=2.f/h;
		//float X1=x1*_2_w-1, X2=x2*_2_w-1, Y1=1-y1*_2_h, Y2=1-y2*_2_h;
		g_fbuf[0]=X1, g_fbuf[1]=Y1;
		g_fbuf[2]=X2, g_fbuf[3]=Y1;
		g_fbuf[4]=X2, g_fbuf[5]=Y2;
		g_fbuf[6]=X1, g_fbuf[7]=Y2;
		g_fbuf[8]=X1, g_fbuf[9]=Y1;
		gl_setProgram(program);							CHECK();
		//if(opaque)
		//	send_color(u_color, 0xFF000000|brush_color);
		//else
			send_color(u_color, brush_color);
		CHECK();
		glEnableVertexAttribArray(a_vertices);			CHECK();

		glBindBuffer(GL_ARRAY_BUFFER, GL2_2D::vertex_buffer);			CHECK();
		glBufferData(GL_ARRAY_BUFFER, 10<<2, g_fbuf, GL_STATIC_DRAW);	CHECK();
		glVertexAttribPointer(a_vertices, 2, GL_FLOAT, GL_FALSE, 0, 0);	CHECK();

	//	glVertexAttribPointer(a_vertices, 2, GL_FLOAT, GL_FALSE, 0, g_fbuf);	CHECK();//use buffer

		glDrawArrays(GL_TRIANGLE_FAN, 0, 5);	CHECK();
		if(brush_color!=pen_color)
			draw_rectangle_hollow(x1, x2, y1, y2);
	}
}
namespace			GL2_L3D
{//light 3D
	struct		TriangleInfo
	{
		int count, color;
		TriangleInfo(int count, int color):count(count), color(color){}
	};
	unsigned	program=0;
	int			a_vertices=-1, a_normals=-1, u_vpmatrix=-1, u_modelmatrix=-1, u_normalmatrix=-1, u_objectcolor=-1, u_lightcolor=-1, u_lightpos=-1, u_viewpos=-1;
	unsigned	VBO=0, EBO=0;
	std::vector<vec3> vertices;
	std::vector<int> indices;
	std::vector<TriangleInfo> drawInfo;
	void		begin()
	{
		vertices.clear();
		indices.clear();
		drawInfo.clear();
	}
	void		push_surface(vec3 const *vn, int vcount_x2, int *idx, int icount, int color)
	{
		color|=0x7F000000;
		int increment=vertices.size()>>1, istart=indices.size();
		vertices.insert(vertices.end(), vn, vn+vcount_x2);
		indices.insert(indices.end(), idx, idx+icount);
		for(int k=istart, kEnd=indices.size();k<kEnd;++k)//use glDrawElementsBaseVertex
		{
			auto &ik=indices[k];
			ik=(ik>>1)+increment;
		}
		//	indices[k]+=increment;
		drawInfo.push_back(TriangleInfo(icount, color));
	}
	void		end()
	{
	//	vbo_to_clipboard(&vertices[0], vertices.size(), &indices[0], indices.size(), 0);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);				CHECK();
		glBufferData(GL_ARRAY_BUFFER, (int)vertices.size()*sizeof(vec3), &vertices[0], GL_STATIC_DRAW);	CHECK();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);		CHECK();
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, (int)indices.size()<<2, &indices[0], GL_STATIC_DRAW);		CHECK();
	}
	void		draw(Camera const &cam, vec3 const &lightpos)
	{
		gl_setProgram(program);
		mat4
			mView=matrixFPSViewRH(cam.p, (float)cam.a.y, (float)cam.a.x-_pi),
			mProj=perspective((float)cam.tanfov, float(w)/h, 0.1f, 1000.f),
			vp=mProj*mView;
		mat4 model=mat4(1);
		mat3 m_normal=normalMatrix(model);
		mat4 mvp=vp*model;
		glUniformMatrix4fv(u_vpmatrix, 1, GL_FALSE, mvp.data());			CHECK();
		glUniformMatrix4fv(u_modelmatrix, 1, GL_FALSE, model.data());	CHECK();
		glUniformMatrix3fv(u_normalmatrix, 1, GL_FALSE, m_normal.data());CHECK();
		send_color_rgb(u_lightcolor, 0xFFFFFF);							CHECK();
		glUniform3fv(u_lightpos, 1, &lightpos.x);						CHECK();
		vec3 cam_p=cam.p;
		glUniform3fv(u_viewpos, 1, &cam_p.x);							CHECK();

		glBindBuffer(GL_ARRAY_BUFFER, VBO);										CHECK();
		glVertexAttribPointer(a_vertices, 3, GL_FLOAT, GL_FALSE, 6<<2, 0);				CHECK();
		glEnableVertexAttribArray(a_vertices);									CHECK();
		glVertexAttribPointer(a_normals, 3, GL_FLOAT, GL_FALSE, 6<<2, (void*)(3<<2));	CHECK();
		glEnableVertexAttribArray(a_normals);									CHECK();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);								CHECK();
		glDepthMask(GL_FALSE);													CHECK();//all L3D objects are transparent for now

		for(int k=0, kEnd=drawInfo.size(), start=0;k<kEnd;++k)
		{
			auto &dk=drawInfo[k];
			send_color(u_objectcolor, dk.color);	CHECK();
			glDrawElements(GL_TRIANGLES, dk.count, GL_UNSIGNED_INT, (void*)((long long)(start<<2)));	CHECK();//use glDrawElementsBaseVertex
			start+=dk.count;
		}

		//int sum=0;
		//for(int k=0;k<drawInfo.size();++k)
		//	sum+=drawInfo[k].count;
		//send_color(u_objectcolor, drawInfo[0].color);						CHECK();//
		//glDrawElements(GL_TRIANGLES, sum, GL_UNSIGNED_INT, 0);			CHECK();//

		glDepthMask(GL_TRUE);					CHECK();
		glDisableVertexAttribArray(a_vertices);	CHECK();
		glDisableVertexAttribArray(a_normals);	CHECK();
	}
	void		draw_buffer(Camera const &cam, GPUBuffer const &buffer, vec3 const &modelpos, vec3 const &lightpos)
	{
		gl_setProgram(program);
		mat4
			mView=matrixFPSViewRH(cam.p, (float)cam.a.y, (float)cam.a.x-_pi),
		//	mView=matrixFPSViewRH(vec3((float)cam.p.x, (float)cam.p.y, (float)cam.p.z), cam.a.y, cam.a.x-_pi),
			mProj=perspective((float)cam.tanfov, float(w)/h, 0.1f, 1000.f),
			vp=mProj*mView;
		mat4 model=translate(mat4(1), modelpos);
		mat3 m_normal=normalMatrix(model);
		mat4 mvp=vp*model;
		glUniformMatrix4fv(u_vpmatrix, 1, GL_FALSE, mvp.data());			CHECK();
		glUniformMatrix4fv(u_modelmatrix, 1, GL_FALSE, model.data());		CHECK();
		glUniformMatrix3fv(u_normalmatrix, 1, GL_FALSE, m_normal.data());	CHECK();
		send_color(u_objectcolor, 0xFF0000FF);								CHECK();
		send_color_rgb(u_lightcolor, 0xFFFFFF);							CHECK();
		glUniform3fv(u_lightpos, 1, &lightpos.x);							CHECK();
		vec3 cam_p=cam.p;
		glUniform3fv(u_viewpos, 1, &cam_p.x);								CHECK();
	//	glPointSize(10);															CHECK();

		glBindBuffer(GL_ARRAY_BUFFER, buffer.VBO);												CHECK();
		glVertexAttribPointer(a_vertices, 3, GL_FLOAT, GL_FALSE, buffer.vertices_stride, (void*)(long long)buffer.vertices_start);CHECK();
		glEnableVertexAttribArray(a_vertices);											CHECK();
		glVertexAttribPointer(a_normals, 3, GL_FLOAT, GL_FALSE, buffer.normals_stride, (void*)(long long)buffer.normals_start);	CHECK();
		glEnableVertexAttribArray(a_normals);											CHECK();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.EBO);										CHECK();

		glDrawElements(GL_TRIANGLES, buffer.n_vertices, GL_UNSIGNED_INT, 0);		CHECK();
	//	glDrawArrays(GL_POINTS, 0, s_vcount);										CHECK();
	}
}
namespace			GL2_3D
{
	unsigned	program=0;
	int			a_vertices=-1, a_texcoord=-1, u_matrix=-1, u_pointSize=-1, u_texture=-1, u_isTextured=-1, u_color=-1;
	unsigned	vertex_buffer=0, texcoord_buffer=0;
	struct		VertexInfo
	{
		int count;
		unsigned type;//GL_POINTS/GL_LINES/GL_LINE_STRIP/GL_TRIANGLES/GL_TRIANGLE_FAN/no GL_QUADS/...
		bool textured;
		int color, *tx, txw, txh;
		VertexInfo(int count, unsigned type, int color):count(count), type(type), textured(false), color(color), tx(nullptr), txw(0), txh(0){}
		VertexInfo(int count, unsigned type, int *tx, int txw, int txh):count(count), type(type), textured(true), color(0), tx(tx), txw(txw), txh(txh){}
	};
	std::vector<vec3> vertices;
	std::vector<vec2> texcoord;
	std::vector<VertexInfo> drawInfo;
	int			transparent_start_idx=0;
	void		begin(){vertices.clear(), texcoord.clear(), drawInfo.clear();}
	void		begin_transparent(){transparent_start_idx=drawInfo.size();}
	void		push_square(float x1, float x2, float y1, float y2, float z, int *tx, int txw, int txh)
	{
		vertices.push_back(vec3(x1, y1, z)), texcoord.push_back(vec2(0, 0));
		vertices.push_back(vec3(x2, y1, z)), texcoord.push_back(vec2(1, 0));
		vertices.push_back(vec3(x2, y2, z)), texcoord.push_back(vec2(1, 1));
		vertices.push_back(vec3(x1, y2, z)), texcoord.push_back(vec2(0, 1));
		drawInfo.push_back(VertexInfo(4, GL_TRIANGLE_FAN, tx, txw, txh));
	}
	void		push_triangle(vec3 const &a, vec3 const &b, vec3 const &c, int color, int *tx=0, int txw=0, int txh=0)
	{
		vertices.push_back(a), texcoord.push_back(vec2(0, 0));//
		vertices.push_back(b), texcoord.push_back(vec2(1, 0));//
		vertices.push_back(c), texcoord.push_back(vec2(1, 1));//
		bool increment=false;
		if(drawInfo.size())
		{
			auto &di=*drawInfo.rbegin();
			increment=di.type==GL_TRIANGLES&&di.color==color&&di.tx==tx&&di.txw==txw&&di.txh==txh;
		}
		if(increment)
			drawInfo.rbegin()->count+=3;
		else if(tx)
			drawInfo.push_back(VertexInfo(3, GL_TRIANGLES, tx, txw, txh));//missing texcoord
		else
			drawInfo.push_back(VertexInfo(3, GL_TRIANGLES, color));
	}
	void		push_triangle(vec3 const &a, vec3 const &b, vec3 const &c, int color)
	{
		color|=0x7F000000;
		vertices.push_back(a), texcoord.push_back(vec2(0, 0));
		vertices.push_back(b), texcoord.push_back(vec2(0, 0));
		vertices.push_back(c), texcoord.push_back(vec2(0, 0));
		bool increment=false;
		if(drawInfo.size())
		{
			auto &di=*drawInfo.rbegin();
			increment=di.type==GL_TRIANGLES&&di.color==color;
		}
		if(increment)
			drawInfo.rbegin()->count+=3;
		else
			drawInfo.push_back(VertexInfo(3, GL_TRIANGLES, color));
	}
	void 		curve_start(int color)
	{
		if(drawInfo.size())//check if previous curve contains just 1 vertex
		{
			auto &latest=*drawInfo.rbegin();
			if(latest.type==GL_LINE_STRIP&&latest.count==1)//then make it a single point
				latest.type=GL_POINTS;
		}
		drawInfo.push_back(VertexInfo(0, GL_LINE_STRIP, color));
	}
	void 		curve_point(vec3 const &p)
	{
		vertices.push_back(p), texcoord.push_back(vec2(0, 0));
		++drawInfo.rbegin()->count;
	}
	void		push_line_segment(vec3 const &p1, vec3 const &p2, int color)
	{
		color|=0xFF000000;
		vertices.push_back(p1), texcoord.push_back(vec2(0, 0));
		vertices.push_back(p2), texcoord.push_back(vec2(0, 0));
		bool increment=false;
		if(drawInfo.size())
		{
			auto &di=*drawInfo.rbegin();
			increment=di.type==GL_LINES&&di.color==color;
		}
		if(increment)
			drawInfo.rbegin()->count+=2;
		else
			drawInfo.push_back(VertexInfo(2, GL_LINES, color));
	}
	void		push_point(vec3 const &p, int color)
	{
		color|=0xFF000000;
		vertices.push_back(p), texcoord.push_back(vec2(0, 0));
		bool increment=false;
		if(drawInfo.size())
		{
			auto &di=*drawInfo.rbegin();
			increment=di.type==GL_POINTS&&di.color==color;
		}
		if(increment)
			drawInfo.rbegin()->count+=1;
		else
			drawInfo.push_back(VertexInfo(1, GL_POINTS, color));
	}
	void		end()
	{
		glBindBuffer(GL_ARRAY_BUFFER, GL2_3D::vertex_buffer);													CHECK();
		glBufferData(GL_ARRAY_BUFFER, (int)vertices.size()*sizeof(vec3), &GL2_3D::vertices[0], GL_STATIC_DRAW);	CHECK();
		glBindBuffer(GL_ARRAY_BUFFER, GL2_3D::texcoord_buffer);													CHECK();
		glBufferData(GL_ARRAY_BUFFER, (int)texcoord.size()*sizeof(vec2), &GL2_3D::texcoord[0], GL_STATIC_DRAW);	CHECK();
	}
	void		draw(Camera const &cam)
	{
		gl_setProgram(GL2_3D::program);		CHECK();

		mat4
			mView=matrixFPSViewRH(vec3((float)cam.p.x, (float)cam.p.y, (float)cam.p.z), (float)cam.a.y, (float)cam.a.x-_pi),
			mProj=perspective((float)cam.tanfov, float(w)/h, 0.1f, 1000.f),
			mVP=mProj*mView;
		//mat4 mVP(1);
		glUniformMatrix4fv(GL2_3D::u_matrix, 1, GL_FALSE, mVP.data());					CHECK();

		static unsigned tx_id=0;
		if(!tx_id)
			{glGenTextures(1, &tx_id);			CHECK();}

		glEnableVertexAttribArray(GL2_3D::a_vertices);							CHECK();
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);							CHECK();
		glVertexAttribPointer(GL2_3D::a_vertices, 3, GL_FLOAT, GL_FALSE, 0, 0);	CHECK();
		glEnableVertexAttribArray(GL2_3D::a_texcoord);							CHECK();
		glBindBuffer(GL_ARRAY_BUFFER, GL2_3D::texcoord_buffer);					CHECK();
		glVertexAttribPointer(GL2_3D::a_texcoord, 2, GL_FLOAT, GL_FALSE, 0, 0);	CHECK();
	//	glPointSize(1);
		for(int k=0, p_idx=0, kEnd=drawInfo.size();k<kEnd;++k)
		{
			auto &di=GL2_3D::drawInfo[k];
			if(k==transparent_start_idx)
				glDepthMask(GL_FALSE);
			glUniform1i(GL2_3D::u_isTextured, di.textured);
			send_color(GL2_3D::u_color, di.textured?0xFFFF00FF:di.color);		CHECK();//dummy color if textured
			if(di.textured)
			{
				glBindTexture(GL_TEXTURE_2D, tx_id);															CHECK();
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);								CHECK();
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);								CHECK();
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, di.txw, di.txh, 0, GL_RGBA, GL_UNSIGNED_BYTE, di.tx);	CHECK();//TODO: send textures to GPU with the vertices
				select_texture(tx_id, GL2_3D::u_texture);														CHECK();

				//glBindBuffer(GL_ARRAY_BUFFER, GL2_3D::texcoord_buffer);					CHECK();
				//glVertexAttribPointer(GL2_3D::a_texcoord, 2, GL_FLOAT, GL_FALSE, 0, 0);	CHECK();
			}
			else
				{select_texture(tx_id, GL2_3D::u_texture);		CHECK();}//dummy texture if not textured
			glDrawArrays(di.type, p_idx, di.count);				CHECK();
			p_idx+=di.count;
		}
		glDisableVertexAttribArray(GL2_3D::a_vertices);	CHECK();
		glDisableVertexAttribArray(GL2_3D::a_texcoord);	CHECK();
	//	glBindBuffer(GL_ARRAY_BUFFER, 0);				CHECK();
		glDepthMask(GL_TRUE);
	}
}
namespace			GL2_Text
{//font & textures
	unsigned		program=0;
	int				a_coord2d=-1,
					u_mytexture=-1, u_txtColor=-1, u_bkColor=-1, u_isTexture=-1;
	unsigned		buffer=0;
	unsigned		tx_id_sf10=0, tx_id_sf8=0, tx_id_sf7=0, tx_id_sf6=0,
					font_tx_id=0;
}
float			sf10_txcoords[128<<2]={0}, sf8_txcoords[128<<2]={0}, sf7_txcoords[128<<2]={0}, sf6_txcoords[128<<2]={0},//txx1 txx2 txy1 txy2
				*text_txcoords=nullptr;
const char		*font_widths=nullptr;
int				gl_tabWidth=0;
int				gl_font_size=0;
float			pixel_x=1, pixel_y=1, gl_fontH=resources::sf10_height;
int				txtColor=0xFF000000, bkColor=0xFFFFFFFF;//OpenGL: 0xAABBGGRR, WinAPI: 0xAARRGGBB
float			getCharWidth(char c){return pixel_x*(float)font_widths[c];}
float			getTextWidth(const char *a, int length)
{
	float width=0;
	for(int k=0;k<length;++k)
		if(a[k]>0&&a[k]<127)
			width+=getCharWidth(a[k]);
	return width;
}
float			getTextWidth(const char *a, int i, int f)
{
	float width=0;
	for(int k=i;k<f;++k)
		if(a[k]>0&&a[k]<127)
			width+=getCharWidth(a[k]);
	return width;
}
int 			getTextColor()
{//with alpha
	return txtColor;//&0x00FFFFFF;//
}
int				setTextColor(int color)
{//with alpha
	int oldColor=txtColor;
	txtColor=color;
	gl_setProgram(GL2_Text::program);			CHECK();
	send_color(GL2_Text::u_txtColor, color);	CHECK();
	return oldColor;
}
int 			getBkColor()
{//with alpha
	return bkColor;//&0x00FFFFFF;//
}
int				setBkColor(int color)
{//with alpha
	int oldColor=bkColor;
	gl_setProgram(GL2_Text::program);
	send_color(GL2_Text::u_bkColor, color);
	CHECK();
	return oldColor;
}
int				getBkMode()
{
	return 1+(((unsigned char*)&bkColor)[3]==0xFF);//TRANSPARENT=1, OPAQUE=2
}
int				setBkMode(int mode)
{
	gl_setProgram(GL2_Text::program);
	unsigned char oldalpha=((unsigned char*)&bkColor)[3];
	((unsigned char*)&bkColor)[3]=-(mode==2);
	send_color(GL2_Text::u_bkColor, bkColor);
	CHECK();
	return 1+(oldalpha==0xFF);
}
//int				gl_setTextSize(int size)
//{//{0, ..., 9}
//	using namespace GL2_Text;
//	using namespace resources;
//	switch(size=clamp(0, size, 9))
//	{
//	case 0:font_tx_id=tx_id_sf6,	text_txcoords=sf6_txcoords,		font_widths=sf6_widths,		pixel_x=pixel_y=1,		gl_fontH= 8, gl_tabWidth=32;	break;
//	case 1:font_tx_id=tx_id_sf7,	text_txcoords=sf7_txcoords,		font_widths=sf7_widths,		pixel_x=pixel_y=1,		gl_fontH=10, gl_tabWidth=40;	break;
//	case 2:font_tx_id=tx_id_sf8,	text_txcoords=sf8_txcoords,		font_widths=sf8_widths,		pixel_x=pixel_y=1,		gl_fontH=12, gl_tabWidth=48;	break;
//	case 3:font_tx_id=tx_id_sf10,	text_txcoords=sf10_txcoords,	font_widths=sf10_widths,	pixel_x=pixel_y=1,		gl_fontH=16, gl_tabWidth=64;	break;
//	case 4:font_tx_id=tx_id_sf10,	text_txcoords=sf10_txcoords,	font_widths=sf10_widths,	pixel_x=pixel_y=2,		gl_fontH=16, gl_tabWidth=136;	break;
//	case 5:font_tx_id=tx_id_sf10,	text_txcoords=sf10_txcoords,	font_widths=sf10_widths,	pixel_x=pixel_y=3,		gl_fontH=16, gl_tabWidth=200;	break;
//	case 6:font_tx_id=tx_id_sf10,	text_txcoords=sf10_txcoords,	font_widths=sf10_widths,	pixel_x=pixel_y=4,		gl_fontH=16, gl_tabWidth=264;	break;
//	case 7:font_tx_id=tx_id_sf10,	text_txcoords=sf10_txcoords,	font_widths=sf10_widths,	pixel_x=5, pixel_y=6,	gl_fontH=16, gl_tabWidth=328;	break;
//	case 8:font_tx_id=tx_id_sf10,	text_txcoords=sf10_txcoords,	font_widths=sf10_widths,	pixel_x=5, pixel_y=7,	gl_fontH=16, gl_tabWidth=328;	break;
//	case 9:font_tx_id=tx_id_sf10,	text_txcoords=sf10_txcoords,	font_widths=sf10_widths,	pixel_x=5, pixel_y=8,	gl_fontH=16, gl_tabWidth=328;	break;
//	}
//	return size;
//}
std::vector<float> vrtx;//2D vertex & texcoords
int				print_array(int x, int y, const char *msg, int msg_length, int tab_origin)
{
	gl_setProgram(GL2_Text::program);					CHECK();
	glUniform2f(GL2_Text::u_isTexture, 0, 1);	CHECK();
//	glUniform1i(GL2_Text::u_isTexture, false);			CHECK();
	int msg_width=0;
	float _2_w=2.f/w, _2_h=2.f/h;
	select_texture(GL2_Text::font_tx_id, GL2_Text::u_mytexture);
	vrtx.resize(msg_length*24);//vx, vy, txx, txy		6.5fps
	float rect[4], *txc;
	int width, idx;
	int fontH_px=gl_fontH*pixel_y;
	for(int k=0;k<msg_length;++k)
	{
		char c=msg[k];
		width=0;
		if(c=='\t')
			width=gl_tabWidth-(msg_width-tab_origin)%gl_tabWidth, c=' ';
		else if(c>=32&&c<127&&(width=font_widths[c]))
			width*=pixel_x;
		rect[0]=(x+msg_width		)*_2_w-1, rect[1]=1- y			*_2_h;
		rect[2]=(x+msg_width+width	)*_2_w-1, rect[3]=1-(y+fontH_px)*_2_h;//y2<y1
		txc=text_txcoords+(c<<2);
		idx=k*24;
		vrtx[idx   ]=rect[0], vrtx[idx+ 1]=rect[1],		vrtx[idx+ 2]=txc[0], vrtx[idx+ 3]=txc[2];//top left
		vrtx[idx+ 4]=rect[0], vrtx[idx+ 5]=rect[3],		vrtx[idx+ 6]=txc[0], vrtx[idx+ 7]=txc[3];//bottom left
		vrtx[idx+ 8]=rect[2], vrtx[idx+ 9]=rect[3],		vrtx[idx+10]=txc[1], vrtx[idx+11]=txc[3];//bottom right

		vrtx[idx+12]=rect[2], vrtx[idx+13]=rect[3],		vrtx[idx+14]=txc[1], vrtx[idx+15]=txc[3];//bottom right
		vrtx[idx+16]=rect[2], vrtx[idx+17]=rect[1],		vrtx[idx+18]=txc[1], vrtx[idx+19]=txc[2];//top right
		vrtx[idx+20]=rect[0], vrtx[idx+21]=rect[1],		vrtx[idx+22]=txc[0], vrtx[idx+23]=txc[2];//top left

		msg_width+=width;
	}
	glBindBuffer(GL_ARRAY_BUFFER, GL2_Text::buffer);												CHECK();
	glBufferData(GL_ARRAY_BUFFER, msg_length*24<<2, &vrtx[0], GL_STATIC_DRAW);			CHECK();//set vertices & texcoords
	glVertexAttribPointer(GL2_Text::a_coord2d, 4, GL_FLOAT, GL_FALSE, 0, 0);	CHECK();

	glEnableVertexAttribArray(GL2_Text::a_coord2d);				CHECK();
	glDrawArrays(GL_TRIANGLES, 0, msg_length*6);	CHECK();//draw the quads: 4 vertices per character quad
	glDisableVertexAttribArray(GL2_Text::a_coord2d);			CHECK();
	return msg_width;
}
int				gl_print(int tab_origin, int x, int y, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	int msg_length=vsnprintf(g_buf, g_buf_size, format, args);
	va_end(args);
	return print_array(x, y, g_buf, msg_length, tab_origin);//38fps 41fps	48fps		undebugged 6.4fps
//	return print_array(x, y, g_buf, msg_length);//7.6fps
}
int				gl_vprint(int tab_origin, int x, int y, const char *format, va_list args)
{
	int msg_length=vsnprintf(g_buf, g_buf_size, format, args);
	return print_array(x, y, g_buf, msg_length, tab_origin);
}
void			display_texture(int x1, int x2, int y1, int y2, int *rgb, int txw, int txh, unsigned char alpha)
{
	static unsigned tx_id=0;
	float _2_w=2.f/w, _2_h=2.f/h;
	float rect[]=
	{
		x1*_2_w-1, 1-y1*_2_h,
		x2*_2_w-1, 1-y2*_2_h//y2<y1
	};
	float vrtx[]=
	{
		rect[0], rect[1],		0, 0,//top left
		rect[0], rect[3],		0, 1,//bottom left
		rect[2], rect[3],		1, 1,//bottom right

		rect[2], rect[3],		1, 1,//bottom right
		rect[2], rect[1],		1, 0,//top right
		rect[0], rect[1],		0, 0,//top left
	};
	if(rgb)
	{
	#define	NPOT_ATIX2300_FIX
		int *rgb2, w2, h2;
#ifdef NPOT_ATIX2300_FIX
		int logw=floor_log2(txw), logh=floor_log2(txh);
		bool expand=glMajorVer<3&&(txw>1<<logw||txh>1<<logh);
		if(expand)
		{
			w2=txw>1<<logw?1<<(logw+1):txw;
			h2=txh>1<<logh?1<<(logh+1):txh;
			int size=w2*h2;
			rgb2=(int*)malloc(size<<2);
			memset(rgb2, 0, size<<2);
			for(int ky=0;ky<txh;++ky)
				memcpy(rgb2+w2*ky, rgb+txw*ky, txw<<2);
		//	memcpy(rgb2, rgb, size<<2);
			float nw=(float)txw/w2, nh=(float)txh/h2;
			vrtx[ 2]=0,		vrtx[ 3]=0;
			vrtx[ 6]=0,		vrtx[ 7]=nh;
			vrtx[10]=nw,	vrtx[11]=nh;

			vrtx[14]=nw,	vrtx[15]=nh;
			vrtx[18]=nw,	vrtx[19]=0;
			vrtx[22]=0,		vrtx[23]=0;
		}
		else
#endif
			rgb2=rgb, w2=txw, h2=txh;
		gl_setProgram(GL2_Text::program);						CHECK();//select Text program
		glUniform2f(GL2_Text::u_isTexture, 1, 0);		CHECK();//send isTexture
	//	glUniform1i(GL2_Text::u_isTexture, true);				CHECK();
		send_color(GL2_Text::u_bkColor, alpha<<24);		CHECK();//send apha
		if(!tx_id)
			{glGenTextures(1, &tx_id);										CHECK();}//generate texture id once
		glBindTexture(GL_TEXTURE_2D, tx_id);									CHECK();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);		CHECK();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);		CHECK();
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w2, h2, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgb2);	CHECK();//send bitmap to GPU

		select_texture(tx_id, GL2_Text::u_mytexture);
		glBindBuffer(GL_ARRAY_BUFFER, GL2_Text::buffer);										CHECK();
		glBufferData(GL_ARRAY_BUFFER, 24<<2, vrtx, GL_STATIC_DRAW);						CHECK();//send vertices & texcoords
		glVertexAttribPointer(GL2_Text::a_coord2d, 4, GL_FLOAT, GL_FALSE, 4<<2, nullptr);	CHECK();//select vertices & texcoord

		glEnableVertexAttribArray(GL2_Text::a_coord2d);		CHECK();
		glDrawArrays(GL_TRIANGLES, 0, 6);		CHECK();//draw the quad
		glDisableVertexAttribArray(GL2_Text::a_coord2d);	CHECK();
#ifdef NPOT_ATIX2300_FIX
		if(expand)
			free(rgb2);
#endif
	}
}
namespace		GL2_TI2D
{
	unsigned	program=0;
	int			a_coords=-1, u_ndr=-1, u_invs=-1, u_curvecolor=-1;
	void		render(float *ndr, int curvecolor)
	{
		static unsigned tx_id=0;
		int x1=0, x2=w, y1=0, y2=h, txw=w+2, txh=h+2;
		float _2_w=2.f/w, _2_h=2.f/h;
		float rect[]=
		{
			x1*_2_w-1, 1-y1*_2_h,
			x2*_2_w-1, 1-y2*_2_h//y2<y1
		};
		float txrect[]=
		{
			1.f/txw, 1.f/txh,
			1-1.f/txw, 1-1.f/txh,
		};
		float vrtx[]=
		{
			rect[0], rect[1],		0, 0,//top left
			rect[0], rect[3],		0, 1,//bottom left
			rect[2], rect[3],		1, 1,//bottom right

			rect[2], rect[3],		1, 1,//bottom right
			rect[2], rect[1],		1, 0,//top right
			rect[0], rect[1],		0, 0,//top left
		};
		if(ndr)
		{
	#define	NPOT_ATIX2300_FIX
			float *ndr2;
			int txw2, txh2;
#ifdef NPOT_ATIX2300_FIX
			int logw=floor_log2(txw), logh=floor_log2(txh);
			bool expand=glMajorVer<3&&(txw>1<<logw||txh>1<<logh);
			if(expand)
			{
				txw2=txw>1<<logw?1<<(logw+1):txw;
				txh2=txh>1<<logh?1<<(logh+1):txh;
				int size=txw2*txh2;
				ndr2=(float*)malloc(size<<2);
				memset(ndr2, 0, size<<2);
				for(int ky=0;ky<txh;++ky)
					memcpy(ndr2+txw2*ky, ndr+txw*ky, txw<<2);
				//for(int ky=0;ky<h;++ky)
				//	for(int kx=0;kx<w;++kx)
				//		SetPixel(ghDC, kx, ky, (int)ndr2[txw2*ky+kx]&0xFFFFFF);
				float
					txx1=1.f/txw2, txy1=1.f/txh2,
					txx2=((float)txw-1)/txw2, txy2=((float)txh-1)/txh2;
				vrtx[ 2]=txx1,	vrtx[ 3]=txy1;
				vrtx[ 6]=txx1,	vrtx[ 7]=txy2;
				vrtx[10]=txx2,	vrtx[11]=txy2;

				vrtx[14]=txx2,	vrtx[15]=txy2;
				vrtx[18]=txx2,	vrtx[19]=txy1;
				vrtx[22]=txx1,	vrtx[23]=txy1;
			}
			else
#endif
				ndr2=ndr, txw2=txw, txh2=txh;
			gl_setProgram(GL2_TI2D::program);							CHECK();//select TI2D program
			send_color(GL2_TI2D::u_curvecolor, curvecolor);				CHECK();//send color
			glUniform2f(GL2_TI2D::u_invs, 1.f/txw2, 1.f/txh2);	CHECK();
			if(!tx_id)
				{glGenTextures(1, &tx_id);										CHECK();}//generate texture id once
			glBindTexture(GL_TEXTURE_2D, tx_id);									CHECK();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);		CHECK();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);		CHECK();
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, txw2, txh2, 0, GL_RGBA, GL_FLOAT, ndr2);	CHECK();//send ndr to GPU		GL_RED not defined!

			select_texture(tx_id, GL2_TI2D::u_ndr);											CHECK();
			glBindBuffer(GL_ARRAY_BUFFER, 0);										CHECK();
			glVertexAttribPointer(GL2_TI2D::a_coords, 4, GL_FLOAT, GL_FALSE, 4<<2, vrtx);	CHECK();//send vertices & texcoords

			glEnableVertexAttribArray(GL2_TI2D::a_coords);		CHECK();
			glDrawArrays(GL_TRIANGLES, 0, 6);		CHECK();//draw the quad
			glDisableVertexAttribArray(GL2_TI2D::a_coords);		CHECK();
#ifdef NPOT_ATIX2300_FIX
			if(expand)
				free(ndr2);
#endif
		}
	}
}
int				gl_setTextSize(int size)
{//size in {0, ..., 9}
	using namespace GL2_Text;
	using namespace resources;
	switch(size=clamp(0, size, 9))
	{
	case 0:font_tx_id=tx_id_sf6,	text_txcoords=sf6_txcoords,		font_widths=sf6_widths,		pixel_x=pixel_y=1,		gl_fontH= 8, gl_tabWidth=32;	break;
	case 1:font_tx_id=tx_id_sf7,	text_txcoords=sf7_txcoords,		font_widths=sf7_widths,		pixel_x=pixel_y=1,		gl_fontH=10, gl_tabWidth=40;	break;
	case 2:font_tx_id=tx_id_sf8,	text_txcoords=sf8_txcoords,		font_widths=sf8_widths,		pixel_x=pixel_y=1,		gl_fontH=12, gl_tabWidth=48;	break;
	case 3:font_tx_id=tx_id_sf10,	text_txcoords=sf10_txcoords,	font_widths=sf10_widths,	pixel_x=pixel_y=1,		gl_fontH=16, gl_tabWidth=64;	break;
	case 4:font_tx_id=tx_id_sf10,	text_txcoords=sf10_txcoords,	font_widths=sf10_widths,	pixel_x=pixel_y=2,		gl_fontH=16, gl_tabWidth=136;	break;
	case 5:font_tx_id=tx_id_sf10,	text_txcoords=sf10_txcoords,	font_widths=sf10_widths,	pixel_x=pixel_y=3,		gl_fontH=16, gl_tabWidth=200;	break;
	case 6:font_tx_id=tx_id_sf10,	text_txcoords=sf10_txcoords,	font_widths=sf10_widths,	pixel_x=pixel_y=4,		gl_fontH=16, gl_tabWidth=264;	break;
	case 7:font_tx_id=tx_id_sf10,	text_txcoords=sf10_txcoords,	font_widths=sf10_widths,	pixel_x=5, pixel_y=6,	gl_fontH=16, gl_tabWidth=328;	break;
	case 8:font_tx_id=tx_id_sf10,	text_txcoords=sf10_txcoords,	font_widths=sf10_widths,	pixel_x=5, pixel_y=7,	gl_fontH=16, gl_tabWidth=328;	break;
	case 9:font_tx_id=tx_id_sf10,	text_txcoords=sf10_txcoords,	font_widths=sf10_widths,	pixel_x=5, pixel_y=8,	gl_fontH=16, gl_tabWidth=328;	break;
	}
	return size;
}
void			gl_initiate()
{
	if(h<=0)
		h=1;
	glViewport(0, 0, w, h);	CHECK();
//	prof_add("glViewport");
	glClearColor(1, 1, 1, 1);	CHECK();
//	glClearColor(0, 0, 0, 1);
//	prof_add("glClearColor");
	glEnable(GL_DEPTH_TEST);		CHECK();
//	prof_add("glEnable");
	glDepthFunc(GL_LEQUAL);			CHECK();
//	prof_add("glDepthFunc");

	//glGetIntegerv(GL_MAJOR_VERSION, &glMajorVer);
	//glGetIntegerv(GL_MINOR_VERSION, &glMinorVer);
	glMajorVer=2, glMinorVer=0;//
	GLversion=glGetString(GL_VERSION);	CHECK();
#if 1
	GL2_2D::vertex_buffer=make_gpu_buffer(GL_ARRAY_BUFFER, 0, 2<<2);//dummy size

	glGenBuffers(1, &GL2_L3D::VBO);	CHECK();
	glGenBuffers(1, &GL2_L3D::EBO);	CHECK();

	GL2_3D::vertex_buffer=make_gpu_buffer(GL_ARRAY_BUFFER, 0, 3<<2);//dummy sizes
	float texcoords[]=
	{
		0, 0,
		0, 1,
		1, 1,
		1, 0
	};
	GL2_3D::texcoord_buffer=make_gpu_buffer(GL_ARRAY_BUFFER, texcoords, sizeof(texcoords));

	using namespace resources;
	calculate_text_txcoords(sf10_widths, sf10_height, sf10_txcoords);
	calculate_text_txcoords(sf8_widths, sf8_height, sf8_txcoords);
	calculate_text_txcoords(sf7_widths, sf7_height, sf7_txcoords);
	calculate_text_txcoords(sf6_widths, sf6_height, sf6_txcoords);
	GL2_Text::buffer=make_gpu_buffer(GL_ARRAY_BUFFER, 0, 8<<2);		CHECK();//dummy size
//	prof_add("Alloc bufs");

	//if(one_shot)
	//{
	ShaderVar _2d_attr[]=
	{
		{&GL2_2D::a_vertices, "a_vertex", __LINE__}
	};
	ShaderVar _2d_unif[]=
	{
		{&GL2_2D::u_color, "u_color", __LINE__}
	};
	GL2_2D::program=LoadShaders(				//2D program
		"#version 100\n"
		"attribute vec2 a_vertex;\n"
		"void main()\n"
		"{\n"
		"    gl_Position=vec4(a_vertex, 0., 1.);\n"
		"}",
		"#version 100\n"
		"precision lowp float;\n"
		"uniform vec4 u_color;\n"
		"void main()\n"
		"{\n"
		"    gl_FragColor=u_color;\n"
	//	"    gl_FragColor=vec4(u_color.rgb, 0.5);\n"
		"}",
		_2d_attr, sizeof(_2d_attr)/sizeof(ShaderVar), _2d_unif, sizeof(_2d_unif)/sizeof(ShaderVar));
	//if(!GL2_2D::program)
	//	ERROR("2D program not compiled.");

	ShaderVar l3d_attr[]=
	{
		{&GL2_L3D::a_vertices, "a_vertex", __LINE__},
		{&GL2_L3D::a_normals, "a_normal", __LINE__},
	//	{&GL2_L3D::a_texcoord, "a_texcoord", __LINE__}
	};
	ShaderVar l3d_unif[]=
	{
		{&GL2_L3D::u_vpmatrix, "u_vpmatrix", __LINE__},
		{&GL2_L3D::u_modelmatrix, "u_modelmatrix", __LINE__},
		{&GL2_L3D::u_normalmatrix, "u_normalmatrix", __LINE__},
	//	{&GL2_L3D::u_pointSize, "u_pointSize", __LINE__},
	//	{&GL2_L3D::u_texture, "u_texture", __LINE__},
	//	{&GL2_L3D::u_isTextured, "u_isTextured", __LINE__},
		{&GL2_L3D::u_objectcolor, "u_objectcolor", __LINE__},
		{&GL2_L3D::u_lightcolor, "u_lightcolor", __LINE__},
		{&GL2_L3D::u_lightpos, "u_lightpos", __LINE__},
		{&GL2_L3D::u_viewpos, "u_viewpos", __LINE__},
	};
	GL2_L3D::program=LoadShaders(						//3D program
		"#version 300 es\n"
		"uniform mat4 u_vpmatrix, u_modelmatrix;\n"
		"uniform mat3 u_normalmatrix;\n"
	//	"uniform float u_pointSize;\n"
		"in vec3 a_vertex;\n"
		"in vec3 a_normal;\n"
	//	"in vec2 a_texcoord;\n"
		"out vec3 v_fragpos;\n"
		"out vec3 v_normal;\n"
	//	"out vec2 v_texcoord;\n"
		"out vec4 v_glposition;\n"
		"void main()\n"
		"{\n"
		"    vec4 fullpos=vec4(a_vertex, 1.);\n"
		"    gl_Position=u_vpmatrix*fullpos;\n"
		"    v_glposition=gl_Position;\n"
		"    gl_Position.z=0.;\n"
		"    v_fragpos=vec3(u_modelmatrix*fullpos);\n"
		"    v_normal=u_normalmatrix*a_normal;\n"
	//	"    v_texcoord=a_texcoord;\n"
	//	"    gl_PointSize=u_pointSize;\n"
	//	"    gl_PointSize=2.;\n"
		"}",
		"#version 300 es\n"
		"precision lowp float;\n"
		"in vec3 v_fragpos;\n"
		"in vec3 v_normal;\n"
		"in vec4 v_glposition;\n"
	//	"in vec2 v_texcoord;\n"
	//	"uniform bool u_isTextured;\n"
		"uniform vec4 u_objectcolor;\n"
		"uniform vec3 u_lightcolor, u_lightpos, u_viewpos;\n"
	//	"uniform sampler2D u_texture;\n"
	//	"out highp float gl_FragDepth;\n"
		"out vec4 fragcolor;\n"
		"void main()\n"
		"{\n"
		"    vec3 normal=normalize(v_normal);\n"
		"    vec3 lightdir=normalize(u_lightpos-v_fragpos);\n"

		"    float specularstrength=0.5;\n"
		"    vec3 viewdir=normalize(u_viewpos-v_fragpos), reflectdir=reflect(-lightdir, normal);\n"
		"    vec3 specular=specularstrength*u_lightcolor*pow(abs(dot(viewdir, reflectdir)), 32.);\n"
	//	"    vec3 specular=specularstrength*u_lightcolor*pow(max(dot(viewdir, reflectdir), 0.), 32.);\n"

		"    vec3 diffuse=abs(dot(normal, lightdir))*u_lightcolor;\n"
	//	"    vec3 diffuse=max(dot(normal, lightdir), 0.)*u_lightcolor;\n"
		"    fragcolor=vec4((0.1*u_lightcolor+diffuse+specular)*u_objectcolor.rgb, u_objectcolor.a);\n"

		"    gl_FragDepth=(-(1000.+0.1)*(-v_glposition.w)-2.*1000.*0.1)/((1000.-0.1)*v_glposition.w);\n"
		"}",
		l3d_attr, sizeof(l3d_attr)/sizeof(ShaderVar), l3d_unif, sizeof(l3d_unif)/sizeof(ShaderVar));

//	glEnable(GL_PROGRAM_POINT_SIZE);
	ShaderVar _3d_attr[]=
	{
		{&GL2_3D::a_vertices, "a_vertex", __LINE__},
	//	{&GL2_3D::a_normals, "a_normal", __LINE__},
		{&GL2_3D::a_texcoord, "a_texcoord", __LINE__}
	};
	ShaderVar _3d_unif[]=
	{
		{&GL2_3D::u_matrix, "u_matrix", __LINE__},
	//	{&GL2_3D::u_pointSize, "u_pointSize", __LINE__},
		{&GL2_3D::u_texture, "u_texture", __LINE__},
		{&GL2_3D::u_isTextured, "u_isTextured", __LINE__},
		{&GL2_3D::u_color, "u_color", __LINE__}
	};
	GL2_3D::program=LoadShaders(						//3D program
		"#version 300 es\n"
		"uniform mat4 u_matrix;\n"
	//	"uniform float u_pointSize;\n"//can't get u_pointSize, -1
	//	"uniform u_depthcoeff;\n"//
	//	"uniform u_zfarcoeff;\n"//2/log(C*F+1)
		"in vec3 a_vertex;\n"
		"in vec2 a_texcoord;\n"
		"out vec2 f_texcoord;\n"
	//	"noperspective out vec4 v_fragpos;\n"
		"out vec4 v_fragpos;\n"
		"void main()\n"
		"{\n"
		"    gl_Position=u_matrix*vec4(a_vertex, 1.);\n"
		"    v_fragpos=gl_Position;\n"
	//	"    gl_Position.z=(1.-2.*exp(-0.01*gl_Position.w))*gl_Position.w;\n"//near clipping, no depth test
		"    gl_Position.z=0.;\n"//overlays text, no depth test
	//	"    gl_Position.z=gl_Position.w;\n"//depth=1, broken
	//	"    gl_Position.z=(log2(max(1e-6, 1.+gl_Position.w))*0.200657630123224-1.)*gl_Position.w;\n"//broken
	//	"    gl_Position.z=(2*log2(0.001*gl_Position.w+1.)/log2(0.001*1000.+1.)-1.)*gl_Position.w;\n"//broken
		"    f_texcoord=a_texcoord;\n"
	//	"    gl_PointSize=u_pointSize;\n"
	//	"    gl_PointSize=2.;\n"
		"}",
		"#version 300 es\n"
		"precision lowp float;\n"
		"in vec2 f_texcoord;\n"
	//	"noperspective in vec4 v_fragpos;\n"
		"in vec4 v_fragpos;\n"
	//	"uniform float u_isTextured;\n"
		"uniform bool u_isTextured;\n"
		"uniform vec4 u_color;\n"
		"uniform sampler2D u_texture;\n"
		"uniform int u_debug;\n"
	//	"out highp float gl_FragDepth;"
		"out vec4 fragcolor;\n"
		"void main()\n"
		"{\n"
		//"    fragcolor=mix(u_color, texture(u_texture, f_texcoord), u_isTextured);\n"
		"    if(u_isTextured)\n"
		"	     fragcolor=texture(u_texture, f_texcoord);\n"//alpha is in the texture
		"    else\n"
		"        fragcolor=u_color;\n"//alpha is in the color

	//	"    fragcolor.rgb=vec3(gl_FragCoord.z/gl_FragCoord.w);\n"//1 for most distance
	//	"    fragcolor.rgb=vec3(gl_FragCoord.z)-vec3((-(1000.+0.1)*(-v_fragpos.w)-2.*1000.*0.1)/((1000.-0.1)*v_fragpos.w));\n"//larger difference close to cam
	//	"    fragcolor.rgb=vec3(gl_FragCoord.z);\n"
	//	"    fragcolor.rgb=vec3((-(1000.+0.1)*(-v_fragpos.w)-2.*1000.*0.1)/((1000.-0.1)*v_fragpos.w));\n"

	//	"    gl_FragDepth=gl_FragCoord.z;\n"					//works
		"    gl_FragDepth=(-(1000.+0.1)*(-v_fragpos.w)-2.*1000.*0.1)/((1000.-0.1)*v_fragpos.w);\n"//works, L3D needs similar modification
	//	"    gl_FragDepth=1.-2.*exp(0.01/gl_FragCoord.w);\n"	//draws from 0 to inf, broken: no depth test
	//	"    gl_FragDepth=1.-2.*exp(-0.01/gl_FragCoord.w);\n"	//draws from 0 to inf, broken: no depth test
	//	"    gl_FragDepth=v_fragpos.w;\n"						//broken
	//	"    gl_FragDepth=v_fragpos.z;\n"						//broken
	//	"    gl_FragDepth=1.;\n"								//broken
	//	"    gl_FragDepth=-1./v_fragpos.w;\n"					//broken
	//	"    gl_FragDepth=1.-2.*exp(-0.01*v_fragpos.w);\n"		//broken
	//	"    gl_FragDepth=v_fragpos.z/v_fragpos.w;\n"			//broken
		"}",
		_3d_attr, sizeof(_3d_attr)/sizeof(ShaderVar), _3d_unif, sizeof(_3d_unif)/sizeof(ShaderVar));
	if(!GL2_3D::program)
		ERROR("3D program not compiled");

	ShaderVar ti2d_attr[]=
	{
		{&GL2_TI2D::a_coords, "coords", __LINE__},
	};
	ShaderVar ti2d_unif[]=
	{
		{&GL2_TI2D::u_ndr, "u_ndr", __LINE__},
		{&GL2_TI2D::u_invs, "u_invs", __LINE__},
		{&GL2_TI2D::u_curvecolor, "u_curvecolor", __LINE__}
	};
	GL2_TI2D::program=LoadShaders(					//TI2D program
		"#version 100\n"
		"attribute vec4 coords;"
		"varying vec2 f_texcoord;\n"
		"void main()\n"
		"{\n"
		"    gl_Position=vec4(coords.xy, 0., 1.);\n"
		"    f_texcoord=coords.zw;\n"
		"}",
		"#version 100\n"
		"precision lowp float;\n"
		"varying highp vec2 f_texcoord;\n"
		"uniform vec2 u_invs;\n"//x: 1/w, y: 1/h
		"uniform sampler2D u_ndr;\n"
		"uniform vec4 u_curvecolor;\n"

		"float alpha_from_line(float x1, float y1, float x2, float y2)\n"
		"{\n"
		"	float a=x2-x1, b=y1-y2;\n"
		"	return max(0., 1.-abs(a*y1+b*x1)*inversesqrt(a*a+b*b));\n"
		"}\n"
		"float do_quadrant(float m, float R, float U, float UR)\n"//9.2fps
		"{\n"
		"	bool down=(m>0.)!=(R>0.), right=(R>0.)!=(UR>0.), up=(UR>0.)!=(U>0.), left=(U>0.)!=(m>0.);\n"
	//	"	return float(down||right||up||left);\n"
		"	float yL=m/(m-U), xD=m/(m-R), yR=R/(R-UR), xU=U/(U-UR);\n"
		"	if(left)\n"
		"	{\n"
		"		if(down)\n"//case 4 & 16
		"			return alpha_from_line(0., yL, xD, 0.);\n"
		"		if(right)\n"//case 7
		"			return alpha_from_line(0., yL, 1., yR);\n"
		"		return alpha_from_line(0., yL, xU, 1.);\n"//up	case 11
		"	}\n"
		"	if(down)\n"//cases 6 & 10
		"	{\n"
		"		if(right)\n"//	case 6
		"			return alpha_from_line(xD, 0., 1., yR);\n"
		"		return alpha_from_line(xD, 0., xU, 1.);\n"//up	case 10
		"	}\n"
		"	if(up)\n"//&&right	case 13
		"		return alpha_from_line(xU, 1., 1., yR);\n"
		"	return 0.;\n"//case 1
		"}\n"
		"void main()\n"
		"{\n"
		"	float\n"
		"		xw=f_texcoord.x-u_invs.x, xm=f_texcoord.x, xe=f_texcoord.x+u_invs.x,\n"
		"		ys=f_texcoord.y-u_invs.y, ym=f_texcoord.y, yn=f_texcoord.y+u_invs.y;\n"
		"	float Vnw=texture2D(u_ndr, vec2(xw, yn)).r-0.5, Vnm=texture2D(u_ndr, vec2(xm, yn)).r-0.5, Vne=texture2D(u_ndr, vec2(xe, yn)).r-0.5;\n"//texture values are clamped in [0, 1]
		"	float Vmw=texture2D(u_ndr, vec2(xw, ym)).r-0.5, Vmm=texture2D(u_ndr, vec2(xm, ym)).r-0.5, Vme=texture2D(u_ndr, vec2(xe, ym)).r-0.5;\n"
		"	float Vsw=texture2D(u_ndr, vec2(xw, ys)).r-0.5, Vsm=texture2D(u_ndr, vec2(xm, ys)).r-0.5, Vse=texture2D(u_ndr, vec2(xe, ys)).r-0.5;\n"
		"	float alpha=float(Vmm==0.);\n"
		"	if(alpha!=1.)\n"
		"	{\n"
		"		alpha=do_quadrant(Vmm, Vme, Vnm, Vne);\n"
		"		alpha=max(alpha, do_quadrant(Vmm, Vnm, Vmw, Vnw));\n"
		"		alpha=max(alpha, do_quadrant(Vmm, Vmw, Vsm, Vsw));\n"
		"		alpha=max(alpha, do_quadrant(Vmm, Vsm, Vme, Vse));\n"
		"	}\n"
		"	gl_FragColor=vec4(u_curvecolor.rgb, u_curvecolor.a*alpha);\n"
		"}",
		ti2d_attr, sizeof(ti2d_attr)/sizeof(ShaderVar), ti2d_unif, sizeof(ti2d_unif)/sizeof(ShaderVar));
	if(!GL2_TI2D::program)
		ERROR("TI2D prigram not compiled.");

	ShaderVar text_attr[]=
	{
		{&GL2_Text::a_coord2d, "coords", __LINE__},
	//	{&GL2_Text::a_coord2d, "coord2d", __LINE__},
	//	{&GL2_Text::a_texcoord, "texcoord", __LINE__}
	};
	ShaderVar text_unif[]=
	{
		{&GL2_Text::u_mytexture, "mytexture", __LINE__},
		{&GL2_Text::u_isTexture, "isTexture", __LINE__},
		{&GL2_Text::u_txtColor, "txtColor", __LINE__},
		{&GL2_Text::u_bkColor, "bkColor", __LINE__}
	};
	GL2_Text::program=LoadShaders(					//Text program
		"#version 100\n"
		"attribute vec4 coords;"
		//"attribute vec2 coord2d;\n"			//coord2d, texcoord
		//"attribute vec2 texcoord;\n"
		"varying vec2 f_texcoord;\n"
		"void main()\n"
		"{\n"
		"    gl_Position=vec4(coords.xy, 0., 1.);\n"
		"    f_texcoord=coords.zw;\n"
		//"    gl_Position=vec4(coord2d, 0., 1.);\n"
		//"    f_texcoord=texcoord;\n"
		"}",
		"#version 100\n"
		"precision lowp float;\n"
		"varying vec2 f_texcoord;\n"
		"uniform sampler2D mytexture;\n"	//mytexture, isTexture, txtColor, bkColor
		"uniform vec2 isTexture;\n"
	//	"uniform bool isTexture;\n"
		"uniform vec4 txtColor;\n"
		"uniform vec4 bkColor;\n"
		"void main()\n"
		"{\n"
		"    vec4 region=texture2D(mytexture, f_texcoord);\n"
		"    gl_FragColor=mix(txtColor*region.r+bkColor*region.g, vec4(region.rgb, bkColor.a), isTexture.x);\n"//36.53fps

	//	"    gl_FragColor=mix(txtColor*region.r+vec4(bkColor.a, 0., 0., 1.)*region.g, vec4(region.rgb, bkColor.a), isTexture.x);\n"
	//	"    gl_FragColor=mix(txtColor*region.r+vec4(bkColor.rgb, 0.)*region.g, vec4(region.rgb, bkColor.a), isTexture.x);\n"
	//	"    gl_FragColor=txtColor*region.r+bkColor*region.g+0.1*isTexture.x;\n"
		//"	if(isTexture.x==0.)\n"
		//"	{\n"
		//"		if(region.r==1.)\n"//inside
		//"			gl_FragColor=txtColor;\n"
		//"		else if(region.g==1.)\n"//bk
		//"			gl_FragColor=vec4(bkColor.rgb, 0.);\n"
		//"	}\n"
		//"	else\n"
		//"		gl_FragColor=vec4(region.rgb, bkColor.a);\n"
		"}",
		text_attr, sizeof(text_attr)/sizeof(ShaderVar), text_unif, sizeof(text_unif)/sizeof(ShaderVar));
	if(!GL2_Text::program)
		ERROR("Text program not compiled.");
//	prof_add("Compile shaders");//26.7ms
	//}

	{
		using namespace resources;
		LoadFontTexture(sf10, sizeof(sf10), sf10_height, sf10_widths, GL2_Text::tx_id_sf10);
		LoadFontTexture(sf8, sizeof(sf8), sf8_height, sf8_widths, GL2_Text::tx_id_sf8);
		LoadFontTexture(sf7, sizeof(sf7), sf7_height, sf7_widths, GL2_Text::tx_id_sf7);
		LoadFontTexture(sf6, sizeof(sf6), sf6_height, sf6_widths, GL2_Text::tx_id_sf6);
	}
//	prof_add("Unpack textures");
	current_program=0;
//	gl_setProgram(GL2_Text::program);	CHECK();
	int length=maximum(w, h);
	switch(length)
	{
	case 800:
		tb_fontH=tb_fontW=35;
	//	tb_fontH=35, tb_fontW=30;
		preferred_fontH=14;
		tb_offset=24;
		preferred_line_width=1;
		grid_step=100;
		preferred_point_size=2;
		gl_setTextSize(3);
		break;
	case 1920://galaxy s5
	case 2160:
		tb_fontH=63, tb_fontW=56, preferred_fontH=42;
		tb_offset=50;
		preferred_line_width=3;//2.4;
		grid_step=200;
		preferred_point_size=5;
		gl_setTextSize(6);
		break;
	case 2198://galaxy a70
		tb_fontH=61, tb_fontW=54, preferred_fontH=42;
		tb_offset=32;
		preferred_line_width=3;//2.4;
		grid_step=200;
		preferred_point_size=5;
		gl_setTextSize(6);
		break;
	default:
		tb_fontH=tb_fontW=length/25, preferred_fontH=length/45.f;
		tb_offset=length/38.4f;
		preferred_line_width=maximum((float)length/800.f, 1.f);
		grid_step=length/19.2f;
		preferred_point_size=length/400.f;
		gl_setTextSize(6);
		break;
	}
	set_font_size_exact(tb_fontH, tb_fontW);
	setTextColor(0xFF000000);		CHECK();
	setBkColor(0xFFFFFFFF);		CHECK();

		GL2_2D::set_region(0, w, 0, h);
		GL2_2D::use_region();

	glEnable(GL_BLEND);		CHECK();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	CHECK();
//	prof_add("Final preps");
#endif
}
void			gl_finish()
{
	glDeleteProgram(GL2_2D::program);
	glDeleteBuffers(1, &GL2_2D::vertex_buffer);

	glDeleteProgram(GL2_L3D::program);

	glDeleteProgram(GL2_3D::program);
	glDeleteBuffers(1, &GL2_3D::vertex_buffer);
	glDeleteBuffers(1, &GL2_3D::texcoord_buffer);

	glDeleteProgram(GL2_TI2D::program);

	glDeleteProgram(GL2_Text::program);
	glDeleteBuffers(1, &GL2_Text::buffer);

	current_program=0;
}
void			printGLString(const char *name, GLenum s)
{
	const char *v=(const char*)glGetString(s);
	LOGI("GL %s = %s\n", name, v);
}