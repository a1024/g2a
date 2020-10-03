//common.cpp - Common definitions for Grapher 2A.
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
#include"g2_common.h"
#include"GLESAPI.h"

char			first_error_msg[e_msg_size]={0}, latest_error_msg[e_msg_size]={0};
static char 	g_buf[e_msg_size]={0};
void 			log_error(const char *file, int line, const char *format, ...)
{
	char *buf=first_error_msg[0]?latest_error_msg:first_error_msg;
	va_list args;
	va_start(args, format);
	vsnprintf(g_buf, e_msg_size, format, args);
	va_end(args);
	int size=strlen(file);
	int start=size-1;
	for(;start>=0&&file[start]!='/'&&file[start]!='\\';--start);
	++start;
//	int length=snprintf(buf, e_msg_size, "%s (%d)%s", g_buf, line, file+start);//TODO: implement 'drawtext'
//	int length=snprintf(buf, e_msg_size, "%s\n%s(%d)", g_buf, file, line);
//	int length=snprintf(buf, e_msg_size, "%s(%d):\n\t%s", file, line, g_buf);
	int length=snprintf(buf, e_msg_size, "%s(%d): %s", file+start, line, g_buf);
	if(buf!=latest_error_msg)
		memcpy(latest_error_msg, first_error_msg, length);
	LOGE("%s", latest_error_msg);
}
std::vector<ProfInfo> prof;
double prof_t1=0;
void prof_start(){prof_t1=now_seconds();}
void prof_add(const char *label, int divisor)
{
	double t2=now_seconds();
	prof.push_back(ProfInfo(std::string(label), 1000.*(t2-prof_t1)));
	prof_t1=now_seconds();
}
void prof_print(int y)
{
	Font font;
	Font::change(0xFF000000, 0xFFFFFFFF, preferred_fontH);
//	Font::change(0xFF000000, 0xFFFFFFFF, fontH);
	int xpos=w-400, xpos2=w-200;
	int k=0;
	for(int kEnd=prof.size();k<kEnd;++k)
	{
		auto &p=prof[k];
		//if(longest.second<p.second)
		//	longest=p;
		int ypos=y+int(k*fontH);
		GUIPrint(xpos, ypos, p.first.c_str());
		GUIPrint(xpos2, ypos, "%lf", p.second);
	//	GUIPrint(xpos2, ypos, "%g", p.second);
	}
	//GUIPrint(xpos, y+int(k*fontH), longest.first.c_str());
	//GUIPrint(xpos2, y+int(k*fontH), "%lf", longest.second);
	font.revert();
	//copy to clipboard?
	prof.clear();
}
const double	_HUGE=1./0;
void			utf16ToAscii(unsigned short const *utf16, int len, std::string &ascii)
{
	ascii.reserve(len);
	for(int k=0;k<len;++k)
	{
		unsigned short c=utf16[k];
		if(c<128)
			ascii+=(char)c;
	}
}
Quat1d::Quat1d(Value const &v):r(v.r), i(v.i), j(v.j), k(v.k){}
Quat1d& Quat1d::operator=(Value const &x){r=x.r, i=x.i, j=x.j, k=x.k; return *this;}
template<int buffer_size>void Value::printComplex	(				char (&buffer)[buffer_size], int &offset			)const
{
	bool written=printValue_real(buffer, offset, r);
	printValue_unreal(written, buffer, offset, i, "i");
	if(!written)
		offset+=sprintf(buffer+offset, "0");
}
template<int buffer_size>void Value::printComplex	(				char (&buffer)[buffer_size], int &offset, int base	)const
{
	bool written=printValue_real(buffer, offset, r, base);
	printValue_unreal(written, buffer, offset, i, "i", base);
	if(!written)
		offset+=sprintf(buffer+offset, "0");
}
template<int buffer_size>void Value::printQuaternion(				char (&buffer)[buffer_size], int &offset			)const
{
	bool written=printValue_real(buffer, offset, r);
	printValue_unreal(written, buffer, offset, i, "i");
	printValue_unreal(written, buffer, offset, j, "j");
	printValue_unreal(written, buffer, offset, k, "k");
	if(!written)
		offset+=sprintf(buffer+offset, "0");
}
template<int buffer_size>void Value::printQuaternion(				char (&buffer)[buffer_size], int &offset, int base	)const
{
	bool written=printValue_real(buffer, offset, r, base);
	printValue_unreal(written, buffer, offset, i, "i", base);
	printValue_unreal(written, buffer, offset, j, "j", base);
	printValue_unreal(written, buffer, offset, k, "k", base);
	if(!written)
		offset+=sprintf(buffer+offset, "0");
}
void			Expression::insertRVar(int pos, int len, unsigned short const *a, int varType)
{
//	std::vector<unsigned short> str(a, a+len);

	std::string str;
	utf16ToAscii(a, len, str);
//	str.reserve(len);
//	for(int k=0;k<len;++k)
//		str+=char(a[k]);

//	std::string str(a, len);

	int k=std::find_if(variables.begin(), variables.end(), [&](Variable &v){return v.name==str;})-variables.begin();
	m.push_back(Map(pos, len, G2::M_N, n.size()));
//	n.push_back(Term('r', k)), data.push_back(Value());
	n.push_back(Term('R', k)), data.push_back(Value());
	if(k==variables.size())
	{
		++nx;
		switch(varType)
		{
		case 's':
			switch(nISD)
			{
			case 0:
			case 1:
			case 2:
				varType='x'+nISD, ++nISD;
				break;
			case 3:
				if(nITD)
					varType='c';
				else
					varType='t', nITD=true;
				break;
			}
			break;
		case 't':
			if(nITD)
				switch(nISD)
				{
				case 0:
				case 1:
				case 2:
					varType='x'+nISD, ++nISD;
					break;
				case 3:
					varType='c';
					break;
				}
			else
				nITD=true;
			break;
		}
		variables.push_back(Variable(a, len, varType));
	}
}
void			Expression::insertCVar(int pos, int len, unsigned short const *a)
{
//	std::vector<unsigned short> str(a, a+len);
	std::string str;
	utf16ToAscii(a, len, str);
//	std::string str(a, len);
	int k=std::find_if(variables.begin(), variables.end(), [&](Variable &v){return v.name==str;})-variables.begin();
	m.push_back(Map(pos, len, G2::M_N, n.size()));
	n.push_back(Term('c', k)), data.push_back(Value());
//	n.push_back(Term('C', k)), data.push_back(Value());
	if(k==variables.size())
	{
		++nZ;
		switch(nISD)
		{
			case 0:
			case 1:
				variables.push_back(Variable(a, len,
											 'x'+nISD,
											 'x'+nISD+1)), nISD+=2;
				break;
			case 2:
				variables.push_back(Variable(a, len,
											 'z',
											 nITD?'c':'t')), ++nISD, nITD=true;
				break;
			case 3:
				variables.push_back(Variable(a, len,
											 nITD?'c':'t',
											 'c')), nITD=true;
				break;
		}
	}
}
void			Expression::insertHVar(int pos, int len, unsigned short const *a)
{
//	std::vector<unsigned short> str(a, a+len);
	std::string str;
	utf16ToAscii(a, len, str);
//	std::string str(a, len);
	int k=std::find_if(variables.begin(), variables.end(), [&](Variable &v){return v.name==str;})-variables.begin();
	m.push_back(Map(pos, len, G2::M_N, n.size()));
	n.push_back(Term('h', k)), data.push_back(Value());
//	n.push_back(Term('C', k)), data.push_back(Value());
	if(k==variables.size())
	{
		++nQ;
		switch(nISD)
		{
			case 0:
				variables.push_back(Variable(a, len,
											 nITD?'c':'t',
											 'x'+nISD,
											 'x'+nISD+1,
											 'x'+nISD+2)), nISD+=3, nITD=true;
				break;
			case 1:
				variables.push_back(Variable(a, len,
											 'x'+nISD,
											 'x'+nISD+1,
											 nITD?'c':'t', 'c')), nISD+=2, nITD=true;
				break;
			case 2:
				variables.push_back(Variable(a, len,
											 'x'+nISD,
											 nITD?'c':'t',
											 'c',
											 'c')), ++nISD, nITD=true;
				break;
			case 3:
				variables.push_back(Variable(a, len,
											 nITD?'c':'t',
											 'c',
											 'c',
											 'c')), nITD=true;
				break;
		}
	}
}