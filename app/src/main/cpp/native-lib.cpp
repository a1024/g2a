//native-lib.cpp - Grapher 2A main implementation file.
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


#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctime>
#include	<cmath>
#include	<complex>
#include	<list>
#include	<stack>
#include	<vector>
#include	<set>
#include	<unordered_set>
#include	<map>
#include	<unordered_map>
#include	<string>
#include	<algorithm>

int			w, h, X0, Y0;
int			nthreads=0;//
static const int	g_buf_size=1024;//maximum line length
static char			g_buf[g_buf_size]={0};
#include	"g2_common.h"
#include	"GLESAPI.h"
#include	"g2_cl.h"

	#define		DEBUG
//	#define		SELECTION_FOLLOW_CURSOR_EXACT

double inv_sqrt(double x)
{//http://stackoverflow.com/questions/11513344/how-to-implement-the-fast-inverse-square-root-in-java
	double t0;
	(long long&)t0=0x5FE6EC85E7DE30DA-((long long&)x>>1);
	return t0*(1.5-.5*x*t0*t0);
}
int floor_log10(double x)
{
	if(x<=0)
		return 0x80000000;
	static const double pmask[]=//positive powers
	{
		1, 10,		//10^2^0
		1, 100,		//10^2^1
		1, 1e4,		//10^2^2
		1, 1e8,		//10^2^3
		1, 1e16,	//10^2^4
		1, 1e32,	//10^2^5
		1, 1e64,	//10^2^6
		1, 1e128,	//10^2^7
		1, 1e256	//10^2^8
	};
	static const double nmask[]=//negative powers
	{
		1, 0.1,		//1/10^2^0
		1, 0.01,	//1/10^2^1
		1, 1e-4,	//1/10^2^2
		1, 1e-8,	//1/10^2^3
		1, 1e-16,	//1/10^2^4
		1, 1e-32,	//1/10^2^5
		1, 1e-64,	//1/10^2^6
		1, 1e-128,	//1/10^2^7
		1, 1e-256	//1/10^2^8
	};
	if(x>=1)
	{
		int logn=0;
		int sh=x>=pmask[17];	logn+=sh<<8, x*=nmask[16+sh];
		sh=x>=pmask[15];		logn+=sh<<7, x*=nmask[14+sh];
		sh=x>=pmask[13];		logn+=sh<<6, x*=nmask[12+sh];
		sh=x>=pmask[11];		logn+=sh<<5, x*=nmask[10+sh];
		sh=x>=pmask[9];			logn+=sh<<4, x*=nmask[8+sh];
		sh=x>=pmask[7];			logn+=sh<<3, x*=nmask[6+sh];
		sh=x>=pmask[5];			logn+=sh<<2, x*=nmask[4+sh];
		sh=x>=pmask[3];			logn+=sh<<1, x*=nmask[2+sh];
		sh=x>=pmask[1];			logn+=sh;
		return logn;
	}
	int logn=-1;
	int sh=x<nmask[17];	logn-=sh<<8;	x*=pmask[16+sh];
	sh=x<nmask[15];		logn-=sh<<7;	x*=pmask[14+sh];
	sh=x<nmask[13];		logn-=sh<<6;	x*=pmask[12+sh];
	sh=x<nmask[11];		logn-=sh<<5;	x*=pmask[10+sh];
	sh=x<nmask[9];		logn-=sh<<4;	x*=pmask[8+sh];
	sh=x<nmask[7];		logn-=sh<<3;	x*=pmask[6+sh];
	sh=x<nmask[5];		logn-=sh<<2;	x*=pmask[4+sh];
	sh=x<nmask[3];		logn-=sh<<1;	x*=pmask[2+sh];
	sh=x<nmask[1];		logn-=sh;
	return logn;
}
double power(double x, int y)
{
	double mask[]={1, 0}, product=1;
	if(y<0)
		mask[1]=1/x, y=-y;
	else
		mask[1]=x;
	for(;;)
	{
		product*=mask[y&1], y>>=1;
		if(!y)
			return product;
		mask[1]*=mask[1];
	}
	return product;
}
double _10pow(int n)
{
	static double *mask=0;
	if(!mask)
	{
		const double _ln10=log(10.);
		mask=new double[616];
		for(int k=-308;k<308;++k)
			mask[k+308]=power(10., k);
		//	mask[k+308]=exp(k*_ln10);//inaccurate
	}
	if(n<-308)
		return 0;
	if(n>307)
		return _HUGE;
	return mask[n+308];
}
inline bool			is_real(double x){return (x==x)&!isinf(x);}
inline int			conditional_negate(int x, bool flag){return (x^-(int)flag)+flag;}
inline long long	clamp_positive(double x){return (long long)((x+abs(x))*0.5);}
inline int			clamp_negative(int x){return x&-(x<0);}
inline double		maximum(double a, double b){return (a+b+abs(a-b))*0.5;}
inline double		minimum(double a, double b){return (a+b-abs(a-b))*0.5;}
inline double		clamp(double lo, double x, double hi)
{
	hi+=hi;
	double temp=x+lo+abs(x-lo);
	return (temp+hi-abs(temp-hi))*0.25;
}
bool		_2d_drag_graph_not_window=false, _dangerous_code=false, _3d_stretch_move_cam=true, _3d_shift_move_cam=false, _3d_zoom_move_cam=false,
			commasInNumbers=false, commentIncompleteScope=true, nestedComments=false,
			showBenchmark=true, showLastModeOnIdle=true, function_timeout=true, contextHelp=false;
int			cursorB, cursorEx, prevCursorEx=0;

std::vector<Expression> expr(1), userFunctionDefinitions(1);

struct		NameTreeNode
{
	bool success;//subsequence support
	int name_id;
	NameTreeNode *nextChar[128];
	std::vector<char> children;
//	NameTreeNode():success(false), nextChar{}{}//warning C4351: new behavior: default initialization	inspect your code
	NameTreeNode():success(false)
	{
		for(int k=0;k<128;++k)
			nextChar[k]=nullptr;
	}
	void allocate_leaky(char c)
	{
		nextChar[c]=new NameTreeNode;
		children.push_back(c);
	}
	void allocate(char c)
	{
		if(!nextChar[c])
		{
			nextChar[c]=new NameTreeNode;
			children.push_back(c);
		}
	}
	void free(char c)
	{
		delete nextChar[c];
		nextChar[c]=nullptr;
		for(int k=0, kEnd=children.size();k<kEnd;++k)
		{
			if(children[k]==c)
			{
				children.erase(children.begin()+k);
				break;
			}
		}
	}
	~NameTreeNode()
	{
		for(unsigned k=0;k<children.size();++k)
			delete nextChar[children[k]];
	}
};
class		NameTree
{
	NameTreeNode userFunctionNamesRoot;
	int name_id_counter;
public:
	NameTree():name_id_counter(0){}
//	NameTree():name_id_counter(G2::M_USER_FUNCTION_START){}
	NameTreeNode* matchName_free(unsigned short *text, int i, int kEnd, int &nameEnd)//free match		inline
	{
		auto node=userFunctionNamesRoot.nextChar[text[i]];
		for(int ch=i+1;node;++ch)
		{
			if(node->success)
			{
				nameEnd=ch;
				return node;
			}
			if(ch==kEnd)
				break;
			node=node->nextChar[text[ch]];
		}
		return nullptr;
	}
	NameTreeNode* matchName		(unsigned short *text, int i, int f, int &nameEnd)//bounded match
	{
		auto node=userFunctionNamesRoot.nextChar[text[i]];
		for(int ch=i+1;node;++ch)
		{
			if(ch==f)
			{
				if(node->success)
				{
					nameEnd=ch;
					return node;
				}
				break;
			}
			node=node->nextChar[text[ch]];
		}
		return nullptr;
	}
	NameTreeNode* addName		(unsigned short *text, int i, int f)
	{
		if(		(text[0]=='b'&&text[1]=='r'&&text[2]=='e'&&text[3]=='a'&&text[4]=='k')
		   ||	(text[0]=='c'&&text[1]=='o'&&text[2]=='n'&&text[3]=='t'&&text[4]=='i'&&text[5]=='n'&&text[6]=='u'&&text[7]=='e')
		   ||	(text[0]=='d'&&text[1]=='o')
		   ||	(text[0]=='e'&&text[1]=='l'&&text[2]=='s'&&text[3]=='e')
		   ||	(text[0]=='f'&&text[1]=='o'&&text[2]=='r')
		   ||	(text[0]=='g'&&text[1]=='o'&&text[2]=='t'&&text[3]=='o')
		   ||	(text[0]=='i'&&text[1]=='f')
		   ||	(text[0]=='r'&&text[1]=='e'&&text[2]=='t'&&text[3]=='u'&&text[4]=='r'&&text[5]=='n')
		   ||	(text[0]=='s'&&text[1]=='w'&&text[2]=='i'&&text[3]=='t'&&text[4]=='c'&&text[5]=='h')
		   ||	(text[0]=='w'&&text[1]=='h'&&text[2]=='i'&&text[3]=='l'&&text[4]=='e'))
			return nullptr;
		auto node=&userFunctionNamesRoot;
		for(int k2=i;;++k2)
		{
			if(k2==f)//subsequence
			{
				if(!node->success)
					node->success=true, node->name_id=name_id_counter++;
				return node;
			}
			auto next=node->nextChar[text[k2]];
			if(next)
				node=next;
			else
			{
				for(;k2<f;++k2)
				{
					node->allocate_leaky(text[k2]);
					node=node->nextChar[text[k2]];
				}
				node->success=true, node->name_id=name_id_counter++;
				return node;
			}
		}
		return nullptr;
	}
	void removeName				(unsigned short *text, int i, int f)
	{
		auto node=userFunctionNamesRoot.nextChar[text[i]];
		std::stack<std::pair<NameTreeNode*, int>> depth;//parentNode, child char
		//	depth.push(std::pair<NameTreeNode*, int>(node, text[i]));
		for(int ch=i+1;node;++ch)//find
		{
			if(ch==f)//found, remove
			{
				if(node->children.size())//remove subsequence
					node->success=false;
				else//remove pillar sequence
				{
					for(;depth.size()&&depth.top().first->children.size()==1&&!depth.top().first->success;)//seek start of free branch
						depth.pop();
					if(depth.size())//remove child & pillar branch
						depth.top().first->free(depth.top().second);
					else//remove last entry
						userFunctionNamesRoot.free(userFunctionNamesRoot.children[0]);
				}
				break;
			}
			depth.push(std::pair<NameTreeNode*, int>(node, text[ch]));
			node=node->nextChar[text[ch]];
		}
	}
	void cleanTree()
	{
		if(userFunctionNamesRoot.children.size())
		{
			auto ufdBegin=userFunctionDefinitions.begin(), ufdEnd=userFunctionDefinitions.end();
			auto node=userFunctionNamesRoot.nextChar[userFunctionNamesRoot.children[0]];
			std::vector<std::pair<NameTreeNode*, int>> depth(1, std::pair<NameTreeNode*, int>(&userFunctionNamesRoot, 0));//stack: &pauseParentNode, pauseChildNo
			for(int br=0;;)
			{
				if(node->success&&std::find_if(ufdBegin, ufdEnd, [&](Expression const &def){return def.name_id==node->name_id;})==ufdEnd)//unused name, remove
				{
					if(node->children.size())//remove subsequence
						node->success=false;
					else//remove pillar sequence
					{
						for(;depth.size()&&depth.rbegin()->first->children.size()==1&&!depth.rbegin()->first->success;)//seek start of free branch
							depth.pop_back();
						if(depth.size())
						{
							auto parent=depth.rbegin()->first;
							parent->free(depth.rbegin()->second);//remove pillar branch
							if(depth.rbegin()->second==parent->children.size())//no next child, find from parents above
							{
								depth.pop_back();
								for(;depth.size()&&depth.rbegin()->second==depth.rbegin()->first->children.size()-1;)//if parent branches exhausted pop and repeat
									depth.pop_back();
								if(depth.size())
									br=++depth.rbegin()->second, node=parent->nextChar[parent->children[br]];
								else//scan finish
									break;
							}
							else//next child is where goner was
								br=depth.rbegin()->second, node=parent->nextChar[parent->children[br]];
						}
						else//remove last entry
						{
							userFunctionNamesRoot.free(userFunctionNamesRoot.children[0]);
							break;
						}
					}
				}
				if(br==depth.rbegin()->first->children.size())//branches exhausted
				{
					for(;depth.size()&&depth.rbegin()->second==depth.rbegin()->first->children.size()-1;)//if parent branches exhausted pop and repeat
						depth.pop_back();
					if(depth.size())//if parents left goto next child
					{
						auto parent=depth.rbegin()->first;
						br=++depth.rbegin()->second, node=parent->nextChar[parent->children[br]];
					}
					else//root exhausted
						break;
				}
				else
				{
					depth.push_back(std::pair<NameTreeNode*, int>(node, br));
					node=node->nextChar[node->children[br]];
				}
			}
		}
	}
} userFunctionNames;

int			lineChangeStart=-1, lineRemoveEnd=-1, lineInsertEnd=-1, nLines=1;
int			boundChangeStart=-1, boundRemoveEnd=-1, boundInsertEnd=-1;
int			exprChangeStart=-1, exprRemoveEnd=-1, exprInsertEnd=-1;
int			functionChangeStart=-1, functionRemoveEnd=-1, functionInsertEnd=-1;
std::vector<std::pair<int, char>> bounds(1, std::pair<int, char>(0, 'c'));//e expression, f function, c clear/empty
std::vector<std::pair<int, int>> allComments;

struct		Performance
{
//	static int nCalls, odd;
	static double d_t1;
//	static double best;
	Performance(){d_t1=now_seconds();}
	Performance(int x, int y)
	{
	//	++nCalls;

		double d_t0=d_t1;
		d_t1=now_seconds();
	//	double current=1000.*(d_t1-d_t0);
	//
	//	if(nCalls==200)
	//		best=current, nCalls=0, odd=!odd;
	//	else if(current<best)
	//		best=current;

		Font::change(0xFF000000, 0xFFFFFFFF, preferred_fontH);

		double diff=d_t1-d_t0;
		print(x, y-preferred_fontH, "fps=%.2f, T=%fms", 1/diff, 1000*diff);
	//	print(x, y-preferred_fontH, "fps=%f, T=%f", 1/diff, diff);
		//print(x, y-preferred_fontH, odd?"[%.10fms], %.10fms, %d":"%.10fms, %.10fms, %d", best, current, nCalls);
	}
};
//int				Performance::nCalls=0, Performance::odd=0;
double			Performance::d_t1=0;
//double			Performance::best=_HUGE;

//void print_value(Term &n, int idx, char (&buffer)[], buffer_size, int &offset, int mathSet, int base=10)
void print_value(Term &n, int idx, int &offset, int mathSet, int base=10)
{
	switch(mathSet)
	{
		case 'R':
			if(!printValue_real(g_buf, offset, n.r[idx], base))
				offset+=snprintf(g_buf+offset, g_buf_size-offset, "0");
			break;
		case 'c':
		{
			bool written=printValue_real(g_buf, offset, n.r[idx], base);
			printValue_unreal(written, g_buf, offset, n.i[idx], "i", base);
			if(!written)
				offset+=snprintf(g_buf+offset, g_buf_size-offset, "0");
		}
			break;
		case 'h':
		{
			bool written=printValue_real(g_buf, offset, n.r[idx], base);
			printValue_unreal(written, g_buf, offset, n.i[idx], "i", base);
			printValue_unreal(written, g_buf, offset, n.j[idx], "j", base);
			printValue_unreal(written, g_buf, offset, n.k[idx], "k", base);
			if(!written)
				offset+=snprintf(g_buf+offset, g_buf_size-offset, "0");
		}
			break;
	}
}
//const float half=0.5f;

JavaVM *jvm;
bool landscape=false;
int init_counter=0;
bool inputBoxOn=true;
std::vector<unsigned short> text;//utf16
//std::wstring text;
//std::string text;
int tb_cursor=0, cursor=0, selcur=0;
vec2 textPos, cursorPos, selcurPos;
//int LOL_1=0;
//char LOL_2[1024]={0};
//int LOL_2_len=0;
//int broken=0;
int			getLineNo(int i, int i_lineNo, int f)
{
	int lineNo=i_lineNo;
	for(int k=i;k<f;++k)
		if(text[k]=='\n'||(text[k]=='\r'&&text[k+1]!='\n'))
			++lineNo;
	return lineNo;
}
void		closeComment(int cStart, int cEnd)
{
	int min=cStart;
	for(;allComments.size()&&allComments.rbegin()->second>cStart;)
	{
		if(min>allComments.rbegin()->first)
			min=allComments.rbegin()->first;
		allComments.pop_back();
	}
	allComments.push_back(std::pair<int, int>(min, cEnd));
}
void		replaceText_update(int i, int fr, int fi)
{
	std::vector<std::pair<int, char>> oldbounds=std::move(bounds);
	{
		int textlen=text.size();
		nLines=1;
		allComments.clear();
		{
			std::stack<int> cStack;//positions of / *
			bool cFound=false, lineComment=false;
			int cFirst, lcFirst;
			for(int k=0;;++k)//A: comments					block 5
			{
				if(k>=textlen)
				{
					if(lineComment)
						closeComment(lcFirst, textlen);
					if(commentIncompleteScope)
					{
						if(nestedComments)
						{
							if(cStack.size())
							{
								for(;cStack.size()>1;)
									cStack.pop();
								closeComment(cStack.top(), textlen);
								cStack.pop();
							}
						}
						else if(cFound)
							closeComment(cFirst, textlen);
					}
					break;
				}
				switch(text[k])
				{
				case '\\':
					if(k+1<textlen&&text[k+1]=='\r')//robust
					{
						k+=1+(k+2<textlen&&text[k+2]=='\n');
						++nLines;
					}
					continue;
				case '\r':
					if(k+1<textlen&&text[k+1]=='\n')//
						++k;
				case '\n':
					if(lineComment)
					{
						closeComment(lcFirst, k-1);
						lineComment=false;
					}
					++nLines;
					continue;
				case '/':
					if(k+1<textlen)
					{
						if(text[k+1]=='*')
						{
							if(nestedComments)
								cStack.push(k);
							else if(!cFound)
								cFirst=k, cFound=true;
							++k;
						}
						else if(text[k+1]=='/')
						{
							if(!lineComment&&((nestedComments&&!cStack.size())||!cFound))
								lcFirst=k, lineComment=true;
							++k;
						}
					}
					continue;
				case '*':
					if(k+1<textlen&&text[k+1]=='/')
					{
						if(nestedComments)
						{
							if(cStack.size())
							{
								closeComment(cStack.top(), k+2);
								cStack.pop();
							}
							++k;
						}
						else if(cFound)
						{
							closeComment(cFirst, k+2);
							cFound=false;
							++k;//c c++
						}
					}
					continue;
				}
			}
		}
		const char *rHeaderTable[]=//C: extend functions
		{
		//allow newlines in header		args with mathset specifier
		//	          tn  r                  s!"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~
			"`````````ab``b``````````````````a````````e``````cccccccccc```````dddddddddddddddddddddddddd````d`dddddddddddddddddddddddddd`````",//a	[f]	[)]
			"`````````ab``b``````````````````a````````e``````cccccccccc```````dddddddddddddddddddddddddd`a``d`dddddddddddddddddddddddddd`````",//b	\\\r\n
			"````````````````````````````````````````````````cccccccccc```````dddddddddddddddddddddddddd````d`dddddddddddddddddddddddddd`````",//c	number	f{}
			"^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^cccccccccc^^^^^^^dddddddddddddddddddddddddd^^^^d^dddddddddddddddddddddddddd^^^^^",//d	letter	f{}

			"`````````ef``f``````````````````e```````o```````iiiiiiiiii```````jjjjjjjjjjjjjjjjjjjjjjjjjj````j`jjjjjjjjjjjjjjjjjjjjjjjjjj`````",//e	(	[arg]
			"`````````ef``f``````````````````e```````````````iiiiiiiiii```````jjjjjjjjjjjjjjjjjjjjjjjjjj`e``j`jjjjjjjjjjjjjjjjjjjjjjjjjj`````",//f	\\\r\n

			"`````````eh``h``````````````````g```````````````iiiiiiiiii```````jjjjjjjjjjjjjjjjjjjjjjjjjj````j`jjjjjjjjjjjjjjjjjjjjjjjjjj`````",//g	arg
			"`````````eh``h``````````````````g```````````````iiiiiiiiii```````jjjjjjjjjjjjjjjjjjjjjjjjjj`g``j`jjjjjjjjjjjjjjjjjjjjjjjjjj`````",//h	\\\r\n
			"````````````````````````````````````````````````iiiiiiiiii```````jjjjjjjjjjjjjjjjjjjjjjjjjj````j`jjjjjjjjjjjjjjjjjjjjjjjjjj`````",//i	number	arg
			"`````````kl`````````````````````k```````o```g```iiiiiiiiii```````jjjjjjjjjjjjjjjjjjjjjjjjjj````j`jjjjjjjjjjjjjjjjjjjjjjjjjj`````",//j	letter	arg

			"`````````kl``l``````````````````k```````o```g``````````````````````m````m````````mm````````````````m````m````````mm`````````````",//k	, [mathset]
			"`````````kl``l``````````````````k```````````g``````````````````````m````m````````mm`````````k``````m````m````````mm`````````````",//l	\\\r\n

			"`````````mn``n``````````````````m```````o```g```````````````````````````````````````````````````````````````````````````````````",//m	,
			"`````````mn``n``````````````````m```````````g```````````````````````````````````````````````m```````````````````````````````````",//n	\\\r\n

			"`````````op``p``````````````````o```````````````qqqqqqqqqq```````rrrrrrrrrrrrrrrrrrrrrrrrrr````r`rrrrrrrrrrrrrrrrrrrrrrrrrr`````",//o	[f]
			"`````````op``p``````````````````o```````````````qqqqqqqqqq```````rrrrrrrrrrrrrrrrrrrrrrrrrr`o``r`rrrrrrrrrrrrrrrrrrrrrrrrrr`````",//p	\\\r\n
			"````````````````````````````````````````````````qqqqqqqqqq```````rrrrrrrrrrrrrrrrrrrrrrrrrr````r`rrrrrrrrrrrrrrrrrrrrrrrrrr`````",//q	number	f
			"^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^qqqqqqqqqq^^^^^^^rrrrrrrrrrrrrrrrrrrrrrrrrr^^^^r^rrrrrrrrrrrrrrrrrrrrrrrrrr^^^^^" //r	letter	f
//							//allow newlines in header
//							//	          tn  r                  s!"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~
//								"`````````ab``b``````````````````a````````e``````cccccccccc```````dddddddddddddddddddddddddd````d`dddddddddddddddddddddddddd`````",//a	[f]	[)]
//								"`````````ab``b``````````````````a````````e``````cccccccccc```````dddddddddddddddddddddddddd`a``d`dddddddddddddddddddddddddd`````",//b	\\\r\n
//								"````````````````````````````````````````````````cccccccccc```````dddddddddddddddddddddddddd````d`dddddddddddddddddddddddddd`````",//c	number	f{}
//								"^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^cccccccccc^^^^^^^dddddddddddddddddddddddddd^^^^d^dddddddddddddddddddddddddd^^^^^",//d	letter	f{}
//
//								"`````````ef``f``````````````````e```````m```````iiiiiiiiii```````jjjjjjjjjjjjjjjjjjjjjjjjjj````j`jjjjjjjjjjjjjjjjjjjjjjjjjj`````",//e	(	[arg]
//								"`````````ef``f``````````````````e```````````````iiiiiiiiii```````jjjjjjjjjjjjjjjjjjjjjjjjjj`e``j`jjjjjjjjjjjjjjjjjjjjjjjjjj`````",//f	\\\r\n
//
//								"`````````eh``h``````````````````g```````````````iiiiiiiiii```````jjjjjjjjjjjjjjjjjjjjjjjjjj````j`jjjjjjjjjjjjjjjjjjjjjjjjjj`````",//g	arg
//								"`````````eh``h``````````````````g```````````````iiiiiiiiii```````jjjjjjjjjjjjjjjjjjjjjjjjjj`g``j`jjjjjjjjjjjjjjjjjjjjjjjjjj`````",//h	\\\r\n
//								"````````````````````````````````````````````````iiiiiiiiii```````jjjjjjjjjjjjjjjjjjjjjjjjjj````j`jjjjjjjjjjjjjjjjjjjjjjjjjj`````",//i	number	arg
//								"`````````kl`````````````````````k```````m```g```iiiiiiiiii```````jjjjjjjjjjjjjjjjjjjjjjjjjj````j`jjjjjjjjjjjjjjjjjjjjjjjjjj`````",//j	letter	arg
//
//								"`````````kl``l``````````````````k```````m```g```````````````````````````````````````````````````````````````````````````````````",//k	,
//								"`````````kl``l``````````````````k```````````g```````````````````````````````````````````````k```````````````````````````````````",//l	\\\r\n
//
//								"`````````mn``n``````````````````m```````````````oooooooooo```````pppppppppppppppppppppppppp````p`pppppppppppppppppppppppppp`````",//m	[f]
//								"`````````mn``n``````````````````m```````````````oooooooooo```````pppppppppppppppppppppppppp`m``p`pppppppppppppppppppppppppp`````",//n	\\\r\n
//								"````````````````````````````````````````````````oooooooooo```````pppppppppppppppppppppppppp````p`pppppppppppppppppppppppppp`````",//o	number	f
//								"^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^oooooooooo^^^^^^^pppppppppppppppppppppppppp^^^^p^pppppppppppppppppppppppppp^^^^^" //p	letter	f
		}, *tailTable[]=
		{
		//	          tn  r                  s!"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~
			"_````````a^``a``````````````````a```````````````````````````````````````````````````````````b```````````````````````````````````",//a	ws
			"_`````````a``b``````````````````````````````````````````````````````````````````````````````````````````````````````````````````" //b	\\\r\n
		};
		//state=table[state-'a'][text[k]]	>'`': continue;		=='`': return;		<'`': i=k+'_'-state;return;
		{
			std::stack<int> fStack;
			if(allComments.begin()!=allComments.end())
				int LOL_1=0;
			auto cIT=allComments.begin();
			for(int k=0, kEnd=cIT!=allComments.end()?cIT->first:textlen;;++k)//B: bounds
			{
				if(k>=kEnd)
				{
					if(cIT!=allComments.end())
					{
						k=cIT->second;
						++cIT;
						kEnd=cIT!=allComments.end()?cIT->first:textlen;
					}
					else
					{
						if(commentIncompleteScope&&fStack.size())
						{
							while(fStack.size()>1)		//find first '{', extend as header, push as comment
								fStack.pop();
							int cStart=fStack.top();//first '{'
							auto acIT=allComments.end();
							for(;acIT!=allComments.begin()&&acIT[-1].first>cStart;--acIT);
							char state='a';
							for(int rk=cStart, rkEnd=acIT!=allComments.begin()?acIT[-1].second:-1, newStart=-1;;--rk)//extend as header
							{
								if(rk<rkEnd)//check counter
								{
									if(acIT!=allComments.begin())
									{
										state=rHeaderTable[state-'a'][' '];
										rk=acIT[-1].first-1;
										--acIT;
										rkEnd=acIT!=allComments.begin()?acIT[-1].second:-1;
									}
									else break;
								}
								else state=rHeaderTable[state-'a'][rk<0?'\0':text[rk]];
								if(state<'a')//check for final state
								{
									if(state<'`'||newStart!=-1)//success final state or optional end reached earlier
									{
										if(state<'`')
											newStart=rk+'_'-state;
										cStart=newStart;
									}
									break;
								}
								if(state=='m'&&newStart==-1)
									newStart=rk;
							}
							closeComment(cStart, textlen);
							for(;bounds.size()>1&&bounds.rbegin()[1].first>cStart;)
								bounds.pop_back();
							if(bounds.size())
								bounds.rbegin()->first=cStart;
							bounds.push_back(std::pair<int, char>(textlen, 'e'));//separate empty expr
						}
						else if(!bounds.size()||bounds.rbegin()->first<textlen||(text[textlen-1]=='\n'&&(textlen-3<0||text[textlen-3]!='\\')))
							bounds.push_back(std::pair<int, char>(textlen, 'e'));
						break;
					}
				}
				switch(text[k])
				{
				case '\\':
					if(k+1<textlen&&text[k+1]=='\r')//robust
						k+=1+(k+2<textlen&&text[k+2]=='\n');
					continue;
				case '\r':
					if(k+1<textlen&&text[k+1]=='\n')//
						++k;
				case '\n':
					bounds.push_back(std::pair<int, char>(k+1, 'e'));
					continue;
				case '{':
					fStack.push(k);
					continue;
				case '}':
					if(fStack.size())
					{
						for(;bounds.size()&&fStack.top()<bounds.rbegin()->first;)
							bounds.pop_back();
						int start=bounds.size()?bounds.rbegin()->first:0;
						if(start<fStack.top())
							bounds.push_back(std::pair<int, char>(fStack.top(), 'e'));
						bounds.push_back(std::pair<int, char>(k+1, 'f'));
						fStack.pop();
					}
					continue;
				}
			}
		}
		{
			auto cIT=allComments.begin(), acIT=cIT;
			for(int b=0, bEnd=bounds.size();b<bEnd;++b)
			{
				if(bounds[b].second=='f')
				{
					if(b)
					{
						for(;cIT!=allComments.end()&&cIT->first<bounds[b-1].first;++cIT);//cIT[-1]: last comment before function b start
						acIT=cIT;
						char state='a';
						for(int rk=bounds[b-1].first-1, rkEnd=acIT!=allComments.begin()?acIT[-1].second:-1, newStart=-1;;--rk)//header/whitespace before function
						{
							if(rk<rkEnd)//check counter
							{
								if(acIT!=allComments.begin())
								{
									state=rHeaderTable[state-'a'][' '];
									rk=acIT[-1].first-1;
									--acIT;
									rkEnd=acIT!=allComments.begin()?acIT[-1].second:-1;
								}
								else break;
							}
							else state=rHeaderTable[state-'a'][rk<0?'\0':text[rk]];
							if(state<'a')//check state
							{
								if(state<'`'||newStart!=-1)
								{
									if(state<'`')
										newStart=rk+'_'-state;
									if(newStart==0)
									{
										bounds.erase(bounds.begin(), bounds.begin()+b);
										bEnd-=b, b=0;
									}
									else
									{
										for(int rb2=b-1;rb2>=0;--rb2)
										{
											if(rb2==0||bounds[rb2-1].first<newStart)//prev bound rb2 start
											{
												if(rb2+1<b)
												{
													bounds.erase(bounds.begin()+rb2+1, bounds.begin()+b);
													bEnd-=b-(rb2+1), b=rb2+1;
												}
												break;
											}
										}
										bounds[b-1].first=newStart;
									}
								}
								break;
							}
							if(state=='m'&&newStart==-1)
								newStart=rk;
						}
					}
					if(b+1<bEnd)
					{
						for(;cIT!=allComments.end()&&cIT->first<bounds[b].first;++cIT);//cIT: first comment after function b end
						acIT=cIT;
						char state='a';
						for(int k=bounds[b].first, kEnd=acIT!=allComments.end()?acIT->first:textlen+1;;++k)//whitespace after function
						{
							if(k>=kEnd)
							{
								if(acIT!=allComments.end())
								{
									k=acIT->second;
									state=tailTable[state-'a'][' '];
									++acIT;
									kEnd=acIT!=allComments.end()?acIT->first:textlen+1;
								}
								else break;
							}
							else state=tailTable[state-'a'][text[k]];
							if(state<'a')
							{
								if(state<'`')
								{
									int newEnd=k+'_'-state;
									for(int b2=b+1;;++b2)
									{
										if(b2==bounds.size()||newEnd<=bounds[b2].first)//next bound end b+1
										{
											b2+=newEnd==bounds[b2].first;
											if(b+1<b2)
											{
												bounds.erase(bounds.begin()+b+1, bounds.begin()+b2);
												bEnd-=b2-(b+1);
											}
											bounds[b].first=newEnd;
											break;
										}
									}
								}
								break;
							}
						}
					}
				}
			}
		}
	}
	for(int b=0, bEnd=bounds.size(), e=0, func=0;b<bEnd;++b)//D: find change extent
	{
		if(bounds[b].first>i||bounds[b].first!=oldbounds[b].first)
		{
			boundChangeStart=b;
			exprChangeStart=e, functionChangeStart=func;
			if(b==bounds.size()||b==oldbounds.size()||bounds[b].second=='f')//recompile everything below when changing a function
		//	if(bounds[b].second=='f')
			{
				boundRemoveEnd=oldbounds.size(), boundInsertEnd=bounds.size();
				break;
			}
			int dk=fi-fr;
			bool diff_enc=bounds[b].first!=oldbounds[b].first+dk*(bounds[b].first>i), end_unas=true;
			if(!diff_enc)
				++b;
			for(int ob=b, oldBoundEnd=oldbounds.size();;)
			{
				if(b>=bEnd||ob>=oldBoundEnd)
				{
					if(diff_enc||end_unas)
						boundRemoveEnd=oldbounds.size(), boundInsertEnd=bEnd;
					else if(boundRemoveEnd==oldbounds.size())
						boundInsertEnd=bEnd;
					else if(boundInsertEnd==bEnd)
						boundRemoveEnd=oldbounds.size();
					break;
				}
				if(oldbounds[ob].second=='f'||bounds[b].second=='f')//recompile everything below when changing a function
				{
					boundRemoveEnd=oldbounds.size(), boundInsertEnd=bounds.size();
					break;
				}
				int obf=oldbounds[ob].first+dk*(oldbounds[ob].first>i);
				if(bounds[b].first<obf)
				{
					diff_enc=true;
					++b;
				}
				else if(bounds[b].first>obf)
				{
					diff_enc=true;
					++ob;
				}
				else
				{
					if(bounds[b].second!=oldbounds[ob].second)
					{
						if(b+1<bEnd&&ob+1<oldBoundEnd&&bounds[b+1].first==oldbounds[ob+1].first+dk&&bounds[b+1].second==oldbounds[ob+1].second&&fr<oldbounds[ob+1].first&&fi<bounds[b+1].first)//diff typ~, same>f
							boundRemoveEnd=ob+1, boundInsertEnd=b+1, diff_enc=false, end_unas=false;
						else
							diff_enc=true;//diff typ~
					}
					else if(diff_enc)//diff~, diff start~
						boundRemoveEnd=ob+1, boundInsertEnd=b+1, diff_enc=false, end_unas=false;
					else if(ob>0&&b>0&&oldbounds[ob-1].first>=fr&&bounds[b-1].first>=fi&&(ob==1||b==1||oldbounds[ob-2].first<fr||bounds[b-2].first<fi))
						boundRemoveEnd=ob, boundInsertEnd=b, diff_enc=false, end_unas=false;
					++b, ++ob;
				}
			}
			break;
		}
		if(bounds[b].second=='e')
			++e;
		else
			++func;
	}
	int b=boundChangeStart, e=exprChangeStart, func=functionChangeStart;//E: change in expr and functions
	for(;b<boundRemoveEnd;++b)
		e+=oldbounds[b].second&1, func+=(oldbounds[b].second&2)!=0;
//	{
//		if(oldbounds[b].second=='e')
//			++e;
//		else
//			++func;
//	}
	exprRemoveEnd=e, functionRemoveEnd=func;
	b=boundChangeStart, e=exprChangeStart, func=functionChangeStart;
	for(;b<boundInsertEnd;++b)
	{
		if(bounds[b].second=='e')
			++e;
		else
			++func;
	}
	exprInsertEnd=e, functionInsertEnd=func;
}

namespace	G2
{
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
		}else
		{
			if(y1==y2)
			{
				if(y==y1)
					return x1<x2?x1<=x&&x<=x2:x2<=x&&x<=x1;
			}else if(x==x1)
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
				}else
				{
					if(x2<=x&&x<=x1)
						return y1<y2?y1<=y&&y<=y2:y2<=y&&y<=y1;
				}
			}
		}
		return false;
	}
	bool _1d_int_in_range(double x0, double x1)
	{
		return std::floor(x0)!=std::floor(x1)||std::ceil(x0)!=std::ceil(x1);
	}
	bool _1d_zero_in_range(double x0, double x1)
	{
		return x0<0?x1>=0:x0==0?x1<0||x1>0:x1<0;
	}
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
	auto disc_r_zeta_i(Value const& x0, Value const& x1){return x0.r<1!=x1.r<1;}
	auto disc_c_zeta_i(Value const& x0, Value const& x1){return false;}//
	auto disc_q_zeta_i(Value const& x0, Value const& x1){return false;}//

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

bool UserFunctionsDefaultReturnZero=false;
bool markFunctionsStuck=false;//true: stuck once return nan regardless of input, false: stuck every time, return nan where stuck only
int userFunctionTimeLimit=400;//ms		4000
void expression_to_clipboard(Expression &ex)
{
	std::stringstream LOL_1;
	for(int kn=0, knEnd=ex.n.size();kn<knEnd;++kn)
	{
		auto &n=ex.n[kn];
		auto &val=ex.data[kn];
		LOL_1<<kn<<'\t'<<n.mathSet<<'\t';
		if(kn<ex.nArgs)
			LOL_1<<"arg\t";
		if(n.constant)
			LOL_1<<"const\t";
		else
			LOL_1<<"var "<<n.varNo<<'\t';
		LOL_1<<val.r<<'+'<<val.i<<"i+"<<val.j<<"j+"<<val.k<<"k\r\n";
	}
	LOL_1<<"\r\n";
	for(int i=0, nInstr=ex.i.size();i<nInstr;++i)
	{
		auto &in=ex.i[i];
		using namespace G2;
		LOL_1<<i<<'\t';
		if(in.r_c==r_c_real)
			LOL_1<<"re ";
		else if(in.r_c==r_c_imag)
			LOL_1<<"im ";
		else if(in.r_r==r_r_assign||in.c_c==c_c_assign)
			LOL_1<<"mov ";
		else if(in.r_rr==r_rr_logic_less)
			LOL_1<<"lt ";
		else if(in.r_rr==r_rr_multiply||in.c_rc==c_rc_multiply||in.c_cr==c_cr_multiply||in.c_cc==c_cc_multiply)
			LOL_1<<"mul ";
		else if(in.r_rr==r_rr_plus||in.c_cc==c_cc_plus)
			LOL_1<<"add ";
		else if(in.r_rr==r_rr_minus)
			LOL_1<<"sub ";
		else if(in.r_rr==r_rr_logic_and)
			LOL_1<<"andl ";
		else if(in.r_r==r_r_increment)
			LOL_1<<"inc ";
		else if(in.r_c==r_c_abs)
			LOL_1<<"abs ";
		switch(in.type)
		{
		case 'c':LOL_1<<"call "<<in.op1<<" -> "<<in.result;break;
		case 'b':LOL_1<<"bif "<<in.op1<<" to "<<in.result;break;
		case 'B':LOL_1<<"bin "<<in.op1<<" to "<<in.result;break;
		case 'j':LOL_1<<"j "<<in.result;break;
		case 'r':LOL_1<<"r "<<in.result;break;
		case 1:case 2:case 3:
		case 13:case 14:case 15:case 16:
			LOL_1<<in.result<<in.r_ms<<' '<<in.op1<<in.op1_ms;
			break;
		case 4:case 5:case 6:case 7:case 8:case 9:case 10:case 11:case 12:
		case 17:case 18:case 19:case 20:case 21:case 22:case 23:case 24:case 25:case 26:
			LOL_1<<in.result<<in.r_ms<<' '<<in.op1<<in.op1_ms<<' '<<in.op2<<in.op2_ms;
			break;
		case 27:
			LOL_1<<in.result<<in.r_ms<<' '<<in.op1<<in.op1_ms<<' '<<in.op2<<in.op2_ms<<' '<<in.op3<<in.op3_ms;
			break;
		}
		LOL_1<<"\r\n";
	}
	const auto &str=LOL_1.str();
	LOGI("%s", str.c_str());
}
void func_data_to_clipboard(std::vector<Quat1d> &fData)
{
	std::stringstream LOL_1;
	for(unsigned k=0;k<fData.size();++k)
	{
		auto &pq=fData[k];
		LOL_1<<k<<'\t'<<pq.r<<'+'<<pq.i<<"i+"<<pq.j<<"j+"<<pq.k<<"k\r\n";
	}
	const auto &str=LOL_1.str();
	LOGI("%s", str.c_str());
}
struct CallInfo
{
	int i;//call address		index in instructions vector
	Expression *func;

	std::vector<Quat1d> fData;
	CallInfo(Expression *func, int i):func(func), i(i){}
};
struct Solve_UserFunction
{
	Expression &expr;
	Instruction &in;
	bool operate_on_const;
	Solve_UserFunction(Expression &expr, Instruction &in, bool operate_on_const):expr(expr), in(in), operate_on_const(operate_on_const){}
	void operator()(int v)const
	{
		auto func=&userFunctionDefinitions[in.op1];
		std::vector<Quat1d> fData(func->data.size());
		{
			int k=0;
			if(operate_on_const)
			{
				for(int kEnd=in.args.size();k<kEnd;++k)//copy args
					fData[k]=Quat1d(expr.data[in.args[k]]);
			}
			else
			{
				for(int kEnd=in.args.size();k<kEnd;++k)//copy args
				{
					auto &n=expr.n[in.args[k]];
					fData[k].r=n.r[v];
					if(n.mathSet>='c')
					{
						fData[k].i=n.i[v];
						if(n.mathSet=='h')
						{
							fData[k].j=n.j[v];
							fData[k].k=n.k[v];
						}
					}
				}
			}
			for(int kEnd=func->data.size();k<kEnd;++k)//initialize const data
				fData[k]=Quat1d(func->data[k]);
		}
		int startTime=clock(), expiryTime=startTime+userFunctionTimeLimit, offset=(startTime>0&&expiryTime<0)*userFunctionTimeLimit;
		std::stack<CallInfo> callStack;
		for(int i2=0, nInstr=func->i.size();;)
		{
			if(i2>=nInstr)//program counter oob, return 0	unreachable
			{
				if(callStack.size())
				{
					auto &cst=callStack.top();
					func=cst.func;
					i2=cst.i, nInstr=func->i.size();
					cst.fData[func->i[i2].result]=0;
				//	cst.fData[func->i[i2].result]=fData[0];//
					std::swap(cst.fData, fData);
					callStack.pop();
					continue;
				}
				if(operate_on_const)
					expr.data[in.result]=Value();
				else
					expr.n[in.result].assign(v, Value());
				break;
			}
			if(clock()+offset>expiryTime)
			{
				func->functionStuck=true;//mark function

				//return nan
				if(callStack.size())
				{
					auto &cst=callStack.top();
					func=cst.func;
					i2=cst.i, nInstr=func->i.size();
					cst.fData[func->i[i2].result]=G2::_qnan;
					std::swap(cst.fData, fData);
					callStack.pop();
					++i2;
					continue;
				}
				if(operate_on_const)
					expr.data[in.result]=G2::_qnan;
				else
					expr.n[in.result].assign(v, G2::_qnan);
				break;
			}
			auto &in2=func->i[i2];
		//	func_data_to_clipboard(fData);
			switch(in2.type)
			{
			case 'c'://call user function
				if(markFunctionsStuck&&userFunctionDefinitions[in2.op1].functionStuck)//return nan without call
				{
					fData[in2.result]=G2::_qnan;
					++i2;
				}
				else
				{
					callStack.push(CallInfo(func, i2));
					auto &cst=callStack.top();
					cst.fData=std::move(fData);
				//	func=in2.function;
					func=&userFunctionDefinitions[in2.op1], i2=0;
					fData.resize(func->data.size());
					{
						int k=0;
						for(int kEnd=in2.args.size();k<kEnd;++k)//copy args
							fData[k]=cst.fData[in2.args[k]];
						//	fData[k]=cst.func->data[in2.args[k]];
						for(int kEnd=func->data.size();k<kEnd;++k)//initialize const data
							fData[k]=Quat1d(func->data[k]);
					}
				}
				continue;
			case 'b'://branch if
				if(fData[in2.op1].q_is_true())
					i2=in2.result;
				else
					++i2;
				continue;
			case 'B'://branch if not
				if(!fData[in2.op1].q_is_true())
					i2=in2.result;
				else
					++i2;
				continue;
			case 'j'://jump
				i2=in2.result;
				continue;
			case 'r'://return
				if(callStack.size())
				{
					auto &cst=callStack.top();
					func=cst.func;
					i2=cst.i, nInstr=func->i.size();
					cst.fData[func->i[i2].result]=fData[in2.result];
					std::swap(cst.fData, fData);
					callStack.pop();
					++i2;
					continue;
				}
				if(operate_on_const)
					expr.data[in.result].set(fData[in2.result]);
				else
					expr.n[in.result].assign(v, fData[in2.result]);
				//	expr.n[in.result].assign(v, fData[in2.result], func->resultMathSet);
				break;
			case 1://r
				fData[in2.result].r=in2.r_r(fData[in2.op1].r);
				++i2;
				continue;
			case 2://c
				{
					auto &res=fData[in2.result], &op1=fData[in2.op1];
					CompRef(res.r, res.i)=in2.c_c(Comp1d(op1.r, op1.i));
					++i2;
				}
				continue;
			case 3://q
				{
					auto &res=fData[in2.result], &op1=fData[in2.op1];
					QuatRef(res.r, res.i, res.j, res.k)=in2.q_q(Quat1d(op1.r, op1.i, op1.j, op1.k));
					++i2;
				}
				continue;
			case 4://rr
				{
					auto &res=fData[in2.result], &op1=fData[in2.op1], &op2=fData[in2.op2];
					res.r=in2.r_rr(op1.r, op2.r);
					++i2;
				}
				continue;
			case 5://rc
				{
					auto &res=fData[in2.result], &op1=fData[in2.op1], &op2=fData[in2.op2];
					CompRef(res.r, res.i)=in2.c_rc(op1.r, Comp1d(op2.r, op2.i));
					++i2;
				}
				continue;
			case 6://rq
				{
					auto &res=fData[in2.result], &op1=fData[in2.op1], &op2=fData[in2.op2];
					QuatRef(res.r, res.i, res.j, res.k)=in2.q_rq(op1.r, Quat1d(op2.r, op2.i, op2.j, op2.k));
					++i2;
				}
				continue;
			case 7://cr
				{
					auto &res=fData[in2.result], &op1=fData[in2.op1], &op2=fData[in2.op2];
					CompRef(res.r, res.i)=in2.c_cr(Comp1d(op1.r, op1.i), op2.r);
					++i2;
				}
				continue;
			case 8://cc
				{
					auto &res=fData[in2.result], &op1=fData[in2.op1], &op2=fData[in2.op2];
					CompRef(res.r, res.i)=in2.c_cc(Comp1d(op1.r, op1.i), Comp1d(op2.r, op2.i));
					++i2;
				}
				continue;
			case 9://cq
				{
					auto &res=fData[in2.result], &op1=fData[in2.op1], &op2=fData[in2.op2];
					QuatRef(res.r, res.i, res.j, res.k)=in2.q_cq(Comp1d(op1.r, op1.i), Quat1d(op2.r, op2.i, op2.j, op2.k));
					++i2;
				}
				continue;
			case 10://qr
				{
					auto &res=fData[in2.result], &op1=fData[in2.op1], &op2=fData[in2.op2];
					QuatRef(res.r, res.i, res.j, res.k)=in2.q_qr(Quat1d(op1.r, op1.i, op1.j, op1.k), op2.r);
					++i2;
				}
				continue;
			case 11://qc
				{
					auto &res=fData[in2.result], &op1=fData[in2.op1], &op2=fData[in2.op2];
					QuatRef(res.r, res.i, res.j, res.k)=in2.q_qc(Quat1d(op1.r, op1.i, op1.j, op1.k), Comp1d(op2.r, op2.i));
					++i2;
				}
				continue;
			case 12://qq
				{
					auto &res=fData[in2.result], &op1=fData[in2.op1], &op2=fData[in2.op2];
					QuatRef(res.r, res.i, res.j, res.k)=in2.q_qq(Quat1d(op1.r, op1.i, op1.j, op1.k), Quat1d(op2.r, op2.i, op2.j, op2.k));
					++i2;
				}
				continue;

			case 13://c_r
				{
					auto &res=fData[in2.result], &op1=fData[in2.op1];
					CompRef(res.r, res.i)=in2.c_r(op1.r);
					++i2;
				}
				continue;
			case 14://c_q
				{
					auto &res=fData[in2.result], &op1=fData[in2.op1];
					CompRef(res.r, res.i)=in2.c_q(Quat1d(op1.r, op1.i, op1.j, op1.k));
					++i2;
				}
				continue;

			case 15://r_c
				{
					auto &res=fData[in2.result], &op1=fData[in2.op1];
					res.r=in2.r_c(Comp1d(op1.r, op1.i));
					++i2;
				}
				continue;
			case 16://r_q
				{
					auto &res=fData[in2.result], &op1=fData[in2.op1];
					res.r=in2.r_q(Quat1d(op1.r, op1.i, op1.j, op1.k));
					++i2;
				}
				continue;

			case 17://c_rr
				{
					auto &res=fData[in2.result], &op1=fData[in2.op1], &op2=fData[in2.op2];
					CompRef(res.r, res.i)=in2.c_rr(op1.r, op2.r);
					++i2;
				}
				continue;

			case 18://r_rc
				{
					auto &res=fData[in2.result], &op1=fData[in2.op1], &op2=fData[in2.op2];
					res.r=in2.r_rc(op1.r, Comp1d(op2.r, op2.i));
					++i2;
				}
				continue;
			case 19://r_rq
				{
					auto &res=fData[in2.result], &op1=fData[in2.op1], &op2=fData[in2.op2];
					res.r=in2.r_rq(op1.r, Quat1d(op2.r, op2.i, op2.j, op2.k));
					++i2;
				}
				continue;
			case 20://r_cr
				{
					auto &res=fData[in2.result], &op1=fData[in2.op1], &op2=fData[in2.op2];
					res.r=in2.r_cr(Comp1d(op1.r, op1.i), op2.r);
					++i2;
				}
				continue;
			case 21://r_cc
				{
					auto &res=fData[in2.result], &op1=fData[in2.op1], &op2=fData[in2.op2];
					res.r=in2.r_cc(Comp1d(op1.r, op1.i), Comp1d(op2.r, op2.i));
					++i2;
				}
				continue;
			case 22://r_cq
				{
					auto &res=fData[in2.result], &op1=fData[in2.op1], &op2=fData[in2.op2];
					res.r=in2.r_cq(Comp1d(op1.r, op1.i), Quat1d(op2.r, op2.i, op2.j, op2.k));
					++i2;
				}
				continue;
			case 23://r_qr
				{
					auto &res=fData[in2.result], &op1=fData[in2.op1], &op2=fData[in2.op2];
					res.r=in2.r_qr(Quat1d(op1.r, op1.i, op1.j, op1.k), op2.r);
					++i2;
				}
				continue;
			case 24://r_qc
				{
					auto &res=fData[in2.result], &op1=fData[in2.op1], &op2=fData[in2.op2];
					res.r=in2.r_qc(Quat1d(op1.r, op1.i, op1.j, op1.k), Comp1d(op2.r, op2.i));
					++i2;
				}
				continue;
			case 25://r_qq
				{
					auto &res=fData[in2.result], &op1=fData[in2.op1], &op2=fData[in2.op2];
					res.r=in2.r_qq(Quat1d(op1.r, op1.i, op1.j, op1.k), Quat1d(op2.r, op2.i, op2.j, op2.k));
					++i2;
				}
				continue;

			case 26://c_qc
				{
					auto &res=fData[in2.result], &op1=fData[in2.op1], &op2=fData[in2.op2];
					CompRef(res.r, res.i)=in2.c_qc(Quat1d(op1.r, op1.i, op1.j, op1.k), Comp1d(op2.r, op2.i));
				}
				continue;

			case 27://a ? b : c
				{
					auto &res=fData[in2.result], &op1=fData[in2.op1], &op2=fData[in2.op2], &op3=fData[in2.op3];
					switch(in2.op1_ms)
					{
					case 'R':
						res=op1.r_is_true()?op2:op3;
						break;
					case 'c':
						res=op1.c_is_true()?op2:op3;
						break;
					case 'h':
						res=op1.q_is_true()?op2:op3;
						break;
					}
				/*	char r_ms=maximum(in2.op2_ms, in2.op3_ms), op_ms=(in2.op2_ms=='c')+2*(in2.op2_ms=='h')+3*(in2.op3_ms=='c')+6*(in2.op3_ms=='h');
					switch(op_ms)
					{
					case 0://r_rr
						res=op1.r_is_true()?op2:op3;
//						{
//							auto &X=op1.r, &Y=op2.r, &Z=op3.r;
//							res.r=X?Y:Z;
//						}
						break;
					case 1://c_rc
						{
							auto X=Comp1d(op1.r, op1.i);
							auto Y=op2.r;
							auto Z=Comp1d(op3.r, op3.i);
							CompRef(res.r, res.i)=X.c_is_true()?Y:Z;
						}
						break;
					case 2://q_rq
						{
							auto X=Quat1d(op1.r, op1.i, op1.j, op1.k);
							auto Y=op2.r;
							auto Z=Quat1d(op3.r, op3.i, op3.j, op3.k);
							QuatRef(res.r, res.i, res.j, res.k)=X.q_is_true()?Y:Z;
						}
						break;
					case 3://c_cr
						{
							auto X=Comp1d(op1.r, op1.i);
							auto Y=Comp1d(op2.r, op2.i);
							auto Z=op3.r;
							CompRef(res.r, res.i)=X.c_is_true()?Y:Z;
						}
						break;
					case 4://c_cc
						{
							auto X=Comp1d(op1.r, op1.i);
							auto Y=Comp1d(op2.r, op2.i);
							auto Z=Comp1d(op3.r, op3.i);
							CompRef(res.r, res.i)=X.c_is_true()?Y:Z;
						}
						break;
					case 5://q_cq
						{
							auto X=Quat1d(op1.r, op1.i, op1.j, op1.k);
							auto Y=Comp1d(op2.r, op2.i);
							auto Z=Quat1d(op3.r, op3.i, op3.j, op3.k);
							QuatRef(res.r, res.i, res.j, res.k)=X.q_is_true()?Y:Z;
						}
						break;
					case 6://q_qr
						{
							auto X=Quat1d(op1.r, op1.i, op1.j, op1.k);
							auto Y=Quat1d(op2.r, op2.i, op2.j, op2.k);
							auto Z=op3.r;
							QuatRef(res.r, res.i, res.j, res.k)=X.q_is_true()?Y:Z;
						}
						break;
					case 7://q_qc
						{
							auto X=Quat1d(op1.r, op1.i, op1.j, op1.k);
							auto Y=Quat1d(op2.r, op2.i, op2.j, op2.k);
							auto Z=Comp1d(op3.r, op3.i);
							QuatRef(res.r, res.i, res.j, res.k)=X.q_is_true()?Y:Z;
						}
						break;
					case 8://q_qq
						{
							auto X=Quat1d(op1.r, op1.i, op1.j, op1.k);
							auto Y=Quat1d(op2.r, op2.i, op2.j, op2.k);
							auto Z=Quat1d(op3.r, op3.i, op3.j, op3.k);
							QuatRef(res.r, res.i, res.j, res.k)=X.q_is_true()?Y:Z;
						}
						break;
					}//*/
				}
				++i2;
				continue;
			}
			break;
		}
	}
};
bool inline_if_default_true=true;
bool verticalBarAbs=false;
bool const omitExprWithUnexpectedCommas=false;
class		Compile
{
	static char const *precedence;
	static int const nOperators;
	static char *ub;
	static struct CompileTerm
	{
		bool constant, fresh;
		char mathSet;
		CompileTerm(bool constant, char mathSet, bool fresh=true):constant(constant), mathSet(mathSet), fresh(fresh){}
	} *term;
	static int bi_mass		(int);
	static int prec_convert	(int);
	static char prec		(int, char, int, char);
	static int default_overload(int);

	static void compile_instruction_f_def	(int);
//	static void compile_instruction_select_u(int, char, char, FunctionPointer&, char (*&)(char), DiscontinuityFunction&);
//	static void compile_instruction_select_b(int, char, char, FunctionPointer&, char (*&)(char, char), DiscontinuityFunction&);
	static void compile_instruction_select_u(int, char, char, FunctionPointer&, int &signature, DiscontinuityFunction&, int &cl_idx, int &cl_disc_idx);
	static void compile_instruction_select_b(int, char, char, FunctionPointer&, int &signature, DiscontinuityFunction&, int &cl_idx, int &cl_disc_idx);
	static void compile_instruction_u		(int, char, int, bool=false);
	static void compile_instruction_b		(int, int, int, bool=false);
	static void compile_instruction_b2		(int, int, int);
	static void compile_instruction_condition_111	(int, int, int);
	static void compile_instruction_condition_110	(int, int);
	static void compile_instruction_condition_101	(int, int);
	static void compile_instruction_condition_100	(int);
	static void compile_instruction_condition_011	(int, int);
	static void compile_instruction_condition_010	(int);
	static char compile_instruction			(int, char, int=-1, int=-1, int=-1);
	
	static int compile_instruction_userFunctionCall(int, std::vector<int>const&, bool=false);
	static int compile_instruction_branch_if		(int);
	static int compile_instruction_branch_if_not	(int);
	static int compile_instruction_jump				();
	static int compile_instruction_return			(int);
	static int compile_instruction_return			(Value const&);
	static void compile_instruction_assign			(int, int);
	static void compile_instruction_assign_value	(int, Value const&);

	static void compile_execute				(int, int, int);
	static void compile_abs					(int, int, int);
	static void compile_inline_if			(int, int, int);

	static void compile_assignment			(int, int);
	static void compile_expression_local	(int, int);
	static bool compile_exprStatement		(int&, int);
	static bool comp_seek_allowNewline		(int&, int, int);
	static bool comp_seek_condition			(int&, int, int&, int&);
	static bool comp_seek_forHeader			(int&, int, int&, int&, int&, int&);
	static bool comp_seek_block				(int&, int, int&, int&);

	static std::stack<std::pair<int, std::vector<int>>> loopInfo;//i_loop, i_break[]
	static void compile_statement			(int&, int);
	static void compile_block				(int, int);
public:
	static void compile_function			(Expression&);
	static void compile_expression_global	(Expression&);
	static int expressionResultLogicType();
	static char predictedMathSet;
	static Expression *expr;
	static bool procedural, recursiveFunction;
};
const char		*Compile::precedence=//"uu ub bu bb"	u: unary, b: binary
//<\>	++		!		%		~		p		t		^				*		+		s		c		=		&		#		|		a		x		o		@		f		_
/*++*/	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"_<_<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"
/*!*/	"><><"	">><<"	">><<"	">><<"	">>>>"	">>>>"	">>>>"	"_>_<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"
/*%*/	"><><"	">>>>"	"><><"	">>>>"	">>>>"	">>>>"	">>>>"	"_>_>"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"
/*~*/	"><><"	">>>>"	"><><"	"><><"	">>>>"	">>>>"	">>>>"	"_>_<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"
/*p*/	"><><"	">>>>"	"><><"	">>>>"	">>>>"	"<<<<"	"<<<<"	"_>_>"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"
/*t*/	"><><"	">>>>"	"><><"	">>>>"	">>>>"	">>>>"	"<<<<"	"_>_>"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"
/*^*/	"><><"	">>>>"	"><><"	">>>>"	">>>>"	">>>>"	">>>>"	"_>_>"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"
/* */	"__><"	"__>>"	"__><"	"__><"	"__>>"	"__>>"	"__>>"	"___<"	"__<<"	"__<<"	"__<<"	"__<<"	"__<<"	"__<<"	"__<<"	"__<<"	"__<<"	"__<<"	"__<<"	"__<<"	"__<<"	"__<<"
/***/	"><><"	">>>>"	"><><"	"><><"	">>>>"	">>>>"	">>>>"	"_>_>"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"
/*+*/	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	"_>_>"	"<<>>"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	">>>>"	"<<<<"//2^-2*3
/*s*/	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	"_>_>"	">>>>"	">>>>"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	">>>>"	"<<<<"
/*c*/	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	"_>_>"	">>>>"	">>>>"	">>>>"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	">>>>"	"<<<<"
/*=*/	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	"_>_>"	">>>>"	">>>>"	">>>>"	">>>>"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	">>>>"	"<<<<"
/*&*/	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	"_>_>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	">>>>"	"<<<<"
/*#*/	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	"_>_>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	">>>>"	"<<<<"
/*|*/	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	"_>_>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	">>>>"	"<<<<"
/*a*/	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	"_>_>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	">>>>"	"<<<<"
/*x*/	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	"_>_>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	"<<<<"	"<<<<"	"<<<<"	">>>>"	"<<<<"
/*o*/	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	"_>_>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	"<<<<"	"<<<<"	">>>>"	"<<<<"
/*@*/	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	"_>_>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	"<<<<"	">>>>"	"<<<<"
/*f*/	">>>>"	">>>>"	"><><"	">>>>"	">>>>"	">>>>"	">>>>"	"_>_>"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"	"<<<<"
/*_*/	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	"_>_>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	">>>>"	"<<<<";
int const		Compile::nOperators=(int)std::sqrt(std::strlen(Compile::precedence)/4.);
char			*Compile::ub;
Compile::CompileTerm *Compile::term=nullptr;
char			Compile::predictedMathSet;
Expression		*Compile::expr=nullptr;
bool			Compile::procedural=false, Compile::recursiveFunction=false;
int				Compile::bi_mass		(int m)
{
	using namespace G2;
//	if(m>M_FUNCTION_START)																					return  1;
	if(m>M_FSTART)																							return  1;
	switch(m)
	{
	case M_FACTORIAL_LOGIC_NOT:	case M_BITWISE_NOT:case M_DECREMENT:case M_INCREMENT:						return  1;
	case M_PLUS:				case M_MINUS:																return  2;
	case M_BITWISE_AND:			case M_BITWISE_NAND:														return  3;
	case M_BITWISE_XOR:			case M_BITWISE_XNOR:														return  4;
	case M_VERTICAL_BAR:		case M_BITWISE_NOR:															return  5;
	case M_MULTIPLY:			case M_DIVIDE:case M_MODULO_PERCENT:case M_LOGIC_DIVIDES:					return  6;
	case M_BITWISE_SHIFT_LEFT:	case M_BITWISE_SHIFT_RIGHT:													return  7;
	case M_POWER:																							return  8;
	case M_TETRATE:																							return  9;
	case M_PENTATE:																							return 10;
	case M_LOGIC_LESS:			case M_LOGIC_LESS_EQUAL:case M_LOGIC_GREATER:case M_LOGIC_GREATER_EQUAL:	return 11;
	case M_LOGIC_EQUAL:			case M_LOGIC_NOT_EQUAL:														return 12;
	case M_LOGIC_AND:																						return 13;
	case M_LOGIC_XOR:																						return 14;
	case M_LOGIC_OR:																						return 15;
	case M_CONDITION_ZERO:																					return 16;
	case M_S_EQUAL_ASSIGN:		case M_S_LESS:case M_S_LESS_EQUAL:case M_S_GREATER:case M_S_GREATER_EQUAL:	return 17;
	}
																											return -1;
}
int				Compile::prec_convert	(int m)
{
	using namespace G2;
	switch(m)
	{
	case M_DECREMENT:case M_INCREMENT:																		return  0;
	case M_FACTORIAL_LOGIC_NOT:																				return  1;
	case M_MODULO_PERCENT:																					return  2;
	case M_BITWISE_NOT:																						return  3;
	case M_PENTATE:																							return  4;
	case M_TETRATE:																							return  5;
	case M_POWER:																							return  6;
	case M_N:																								return  7;
	case M_MULTIPLY:case M_DIVIDE:																			return  8;
	case M_PLUS:case M_MINUS:																				return  9;
	case M_BITWISE_SHIFT_LEFT:	case M_BITWISE_SHIFT_RIGHT:													return 10;
	case M_LOGIC_LESS:			case M_LOGIC_LESS_EQUAL:case M_LOGIC_GREATER:case M_LOGIC_GREATER_EQUAL:	return 11;
	case M_LOGIC_EQUAL:			case M_LOGIC_NOT_EQUAL:														return 12;
	case M_BITWISE_AND:			case M_BITWISE_NAND:														return 13;
	case M_BITWISE_XOR:			case M_BITWISE_XNOR:														return 14;
	case M_VERTICAL_BAR:		case M_BITWISE_NOR:															return 15;
	case M_LOGIC_AND:																						return 16;
	case M_LOGIC_XOR:																						return 17;
	case M_LOGIC_OR:																						return 18;
	case M_CONDITION_ZERO:																					return 19;
	case M_S_EQUAL_ASSIGN:		case M_S_LESS:case M_S_LESS_EQUAL:case M_S_GREATER:case M_S_GREATER_EQUAL:	return 21;
	}
//	if(m>M_FUNCTION_START)																					return 20;
	if(m>M_FSTART)																							return 20;
																											return 0;
																											//return -1;//stuck
}
char			Compile::prec			(int lop, char lub, int rop, char rub){return precedence[((nOperators*prec_convert(lop)+prec_convert(rop))<<2)|((lub=='b')<<1)|(rub=='b')];}
void			Compile::compile_instruction_f_def		(int f)
{
	using namespace G2;
	double r=0;
	Comp1d c;
	bool complex=false;
	switch(expr->m[f]._0)
	{
	case M_REAL:		r=r_c_real(c);		complex=true;	break;
	case M_IMAG:		r=r_c_imag(c);		complex=true;	break;
	case M_COS:			r=r_r_cos(r);						break;
	case M_ACOS:		c=c_c_acos(c);		complex=true;	break;
	case M_COSH:		r=r_r_cosh(r);						break;
	case M_ACOSH:		c=c_c_acosh(c);		complex=true;	break;
	case M_COSC:		r=r_r_cosc(r);						break;
	case M_SEC:			r=r_r_sec(r);						break;
	case M_ASEC:		c=c_c_asec(c);		complex=true;	break;
	case M_SECH:		r=r_r_sech(r);						break;
	case M_ASECH:		c=c_c_asech(c);		complex=true;	break;
	case M_SIN:			r=r_r_sin(r);						break;
	case M_ASIN:		c=c_c_asin(c);		complex=true;	break;
	case M_SINH:		r=r_r_sinh(r);						break;
	case M_ASINH:		r=r_r_asinh(r);						break;
	case M_SINC:		r=r_r_sinc(r);						break;
	case M_SINHC:		r=r_r_sinhc(r);						break;
	case M_CSC:			r=r_r_csc(r);						break;
	case M_ACSC:		c=c_c_acsc(c);		complex=true;	break;
	case M_CSCH:		r=r_r_csch(r);						break;
	case M_ACSCH:		r=r_r_acsch(r);						break;
	case M_TAN:			r=r_r_tan(r);						break;
	case M_TANH:		r=r_r_tanh(r);						break;
	case M_ATANH:		c=c_c_atanh(c);		complex=true;	break;
	case M_TANC:		r=r_r_tanc(r);						break;
	case M_COT:			r=r_r_cot(r);						break;
	case M_ACOT:		r=r_r_acot(r);						break;
	case M_COTH:		r=r_r_coth(r);						break;
	case M_ACOTH:		c=c_c_acoth(c);		complex=true;	break;
	case M_EXP:			r=r_r_exp(r);						break;
	case M_LN:			c=c_c_ln(c);		complex=true;	break;
	case M_SQRT:		c=c_c_sqrt(c);		complex=true;	break;
	case M_CBRT:		r=r_r_cbrt(r);						break;
	case M_INVSQRT:		r=r_r_invsqrt(r);					break;
	case M_SQ:			r=r_r_sq(r);						break;
	case M_ERF:			r=r_r_erf(r);						break;
	case M_FIB:			r=r_r_fib(r);						break;
	case M_ZETA:		r=r_r_zeta(r);						break;
	case M_STEP:		r=r_r_step(r);						break;
	case M_SGN:			r=r_r_sgn(r);						break;
	case M_RECT:		r=r_r_rect(r);						break;
	case M_TENT:		r=r_r_trgl(r);						break;
	case M_CEIL:		r=r_r_ceil(r);						break;
	case M_FLOOR:		r=r_r_floor(r);						break;
	case M_ROUND:		r=r_r_round(r);						break;
	case M_ABS:			r=r_r_abs(r);						break;
	case M_ARG:			r=r_r_arg(r);						break;
	case M_RAND:		r=r_r_random(r);					break;
	case M_ATAN:		r=r_r_atan(r);						break;
	case M_LOG:			c=c_c_log(c);		complex=true;	break;
	case M_BETA:		r=r_r_beta(r);						break;
	case M_GAMMA:		r=r_r_tgamma(r);					break;
	case M_LNGAMMA:		r=r_r_loggamma(r);					break;
	case M_GAUSS:		r=r_r_gauss(r);						break;
	case M_PERMUTATION:	r=r_r_permutation(r);				break;
	case M_COMBINATION:	r=r_r_combination(r);				break;
	case M_BESSEL_J:	r=r_r_bessel_j(r);					break;
	case M_BESSEL_Y:	r=r_r_bessel_y(r);					break;
	case M_HANKEL1:		c=c_rr_hankel1(r, r);complex=true;	break;
	case M_SQWV:		r=r_r_sqwv(r);						break;
	case M_TRWV:		r=r_r_trwv(r);						break;
	case M_SAW:			r=r_r_saw(r);						break;
	case M_HYPOT:		r=0;								break;
//	case M_MIN:			r=r_min(r);							break;
//	case M_MAX:			r=r_max(r);							break;
	default:												break;
	}
	expr->m[f]=::Map(expr->m[f].pos, expr->m[f].len, M_N, expr->n.size());
	if(complex)
		expr->insertData('c', Value(c.r, c.i));
	else
		expr->insertData('R', r);
	term=(CompileTerm*)realloc(term, expr->n.size()*sizeof(CompileTerm));
	term[expr->n.size()-1]=CompileTerm(true, expr->n.rbegin()->mathSet);
}
#define			ISEL_C(TOKEN, ret, args, RET, ARGS, name, NAME)		case M_##TOKEN:function.set(ret##_##args##_##name), signature=SIG_##RET##_##ARGS, d(), cl_idx=RET##_##ARGS##_##NAME, cl_disc_idx=-1; return;
#define			ISEL_I(TOKEN, ret, args, RET, ARGS, name, NAME)		case M_##TOKEN:function.set(ret##_##args##_##name), signature=SIG_##RET##_##ARGS, d(disc_##args##_##name##_i), cl_idx=RET##_##ARGS##_##NAME, cl_disc_idx=DISC_##ARGS##_##NAME##_I; return;
#define			ISEL_O(TOKEN, ret, args, RET, ARGS, name, NAME)		case M_##TOKEN:function.set(ret##_##args##_##name), signature=SIG_##RET##_##ARGS, d(disc_##ret##_##name##_o, false), cl_idx=RET##_##ARGS##_##NAME, cl_disc_idx=DISC_##RET##_##NAME##_O; return;

#define			CASE_NONE(TOKEN)							case M_##TOKEN:function.set(), signature=SIG_NOOP, d(), cl_idx=-1; return;
#define			CASE_R_R(TOKEN, name, NAME, DISCTYPE)		ISEL_##DISCTYPE(TOKEN, r, r, R, R, name, NAME)
#define			CASE_C_C(TOKEN, name, NAME, DISCTYPE)		ISEL_##DISCTYPE(TOKEN, c, c, C, C, name, NAME)
#define			CASE_Q_Q(TOKEN, name, NAME, DISCTYPE)		ISEL_##DISCTYPE(TOKEN, q, q, Q, Q, name, NAME)
#define			CASE_R_RR(TOKEN, name, NAME, DISCTYPE)		ISEL_##DISCTYPE(TOKEN, r, rr, R, RR, name, NAME)
#define			CASE_C_RC(TOKEN, name, NAME, DISCTYPE)		ISEL_##DISCTYPE(TOKEN, c, rc, C, RC, name, NAME)
#define			CASE_Q_RQ(TOKEN, name, NAME, DISCTYPE)		ISEL_##DISCTYPE(TOKEN, q, rq, Q, RQ, name, NAME)
#define			CASE_C_CR(TOKEN, name, NAME, DISCTYPE)		ISEL_##DISCTYPE(TOKEN, c, cr, C, CR, name, NAME)
#define			CASE_C_CC(TOKEN, name, NAME, DISCTYPE)		ISEL_##DISCTYPE(TOKEN, c, cc, C, CC, name, NAME)
#define			CASE_Q_CQ(TOKEN, name, NAME, DISCTYPE)		ISEL_##DISCTYPE(TOKEN, q, cq, Q, CQ, name, NAME)
#define			CASE_Q_QR(TOKEN, name, NAME, DISCTYPE)		ISEL_##DISCTYPE(TOKEN, q, qr, Q, QR, name, NAME)
#define			CASE_Q_QC(TOKEN, name, NAME, DISCTYPE)		ISEL_##DISCTYPE(TOKEN, q, qc, Q, QC, name, NAME)
#define			CASE_Q_QQ(TOKEN, name, NAME, DISCTYPE)		ISEL_##DISCTYPE(TOKEN, q, qq, Q, QQ, name, NAME)
#define			CASE_C_R(TOKEN, name, NAME, DISCTYPE)		ISEL_##DISCTYPE(TOKEN, c, r, C, R, name, NAME)
#define			CASE_C_Q(TOKEN, name, NAME, DISCTYPE)		ISEL_##DISCTYPE(TOKEN, c, q, C, Q, name, NAME)
#define			CASE_R_C(TOKEN, name, NAME, DISCTYPE)		ISEL_##DISCTYPE(TOKEN, r, c, R, C, name, NAME)
#define			CASE_R_Q(TOKEN, name, NAME, DISCTYPE)		ISEL_##DISCTYPE(TOKEN, r, q, R, Q, name, NAME)

#define			CASE_C_RR(TOKEN, name, NAME, DISCTYPE)		ISEL_##DISCTYPE(TOKEN, c, rr, C, RR, name, NAME)

#define			CASE_R_RC(TOKEN, name, NAME, DISCTYPE)		ISEL_##DISCTYPE(TOKEN, r, rc, R, RC, name, NAME)
#define			CASE_R_RQ(TOKEN, name, NAME, DISCTYPE)		ISEL_##DISCTYPE(TOKEN, r, rq, R, RQ, name, NAME)
#define			CASE_R_CR(TOKEN, name, NAME, DISCTYPE)		ISEL_##DISCTYPE(TOKEN, r, cr, R, CR, name, NAME)
#define			CASE_R_CC(TOKEN, name, NAME, DISCTYPE)		ISEL_##DISCTYPE(TOKEN, r, cc, R, CC, name, NAME)
#define			CASE_R_CQ(TOKEN, name, NAME, DISCTYPE)		ISEL_##DISCTYPE(TOKEN, r, cq, R, CQ, name, NAME)
#define			CASE_R_QR(TOKEN, name, NAME, DISCTYPE)		ISEL_##DISCTYPE(TOKEN, r, qr, R, QR, name, NAME)
#define			CASE_R_QC(TOKEN, name, NAME, DISCTYPE)		ISEL_##DISCTYPE(TOKEN, r, qc, R, QC, name, NAME)
#define			CASE_R_QQ(TOKEN, name, NAME, DISCTYPE)		ISEL_##DISCTYPE(TOKEN, r, qq, R, QQ, name, NAME)

#define			CASE_C_QC(TOKEN, name, NAME, DISCTYPE)		ISEL_##DISCTYPE(TOKEN, c, qc, C, QC, name, NAME)

#define			CASE(SIG, NAME, name, DISCTYPE)				CASE_##SIG(NAME, name, NAME, DISCTYPE)
//void			Compile::compile_instruction_select_u	(int f, char side, char op1type, FunctionPointer &function, char (*&umts)(char), DiscontinuityFunction &d)
void			Compile::compile_instruction_select_u	(int f, char side, char op1type, FunctionPointer &function, int &signature, DiscontinuityFunction &d, int &cl_idx, int &cl_disc_idx)
{
	using namespace G2;
	switch(op1type)
	{
	case 'R':
		switch(f)
		{
		CASE_NONE(REAL)
		CASE_NONE(IMAG)
	//	CASE(R_R, CONJUGATE,		conjugate,		C)
		CASE(C_R, POLAR,			polar,			I)
	//	CASE(C_R, CARTESIAN,		cartesian,		C)
		CASE(R_R, DIVIDE,			divide,			I)
		CASE(R_R, MINUS,			minus,			C)
		CASE_R_R(MODULO_PERCENT, percent, PERCENT, C)
		CASE(R_R, INCREMENT,		increment,		C)
		CASE(R_R, DECREMENT,		decrement,		C)
		CASE(R_R, BITWISE_NOT,		bitwise_not,	I)
		case M_BITWISE_SHIFT_LEFT:		 if(side=='<')	function.set(r_r_bitwise_shift_left_l),		signature=SIG_R_R,	d(disc_r_bitwise_shift_left_l_o, false);
									else				function.set(r_r_bitwise_shift_left_r),		signature=SIG_R_R,	d();											return;
		case M_BITWISE_SHIFT_RIGHT:		 if(side=='<')	function.set(r_r_bitwise_shift_right_l),	signature=SIG_R_R,	d(disc_r_bitwise_shift_right_l_o, false);
									else				function.set(r_r_bitwise_shift_right_r),	signature=SIG_R_R,	d();											return;
		case M_FACTORIAL_LOGIC_NOT:		 if(side=='<')	function.set(r_r_logic_not),				signature=SIG_R_R,	d(disc_r_logic_not_o, true);
									else				function.set(r_r_factorial),				signature=SIG_R_R,	d(disc_r_factorial_i, true);			return;
		case M_LOGIC_LESS:				 if(side=='<')	function.set(r_r_logic_less_l),				signature=SIG_R_R,	d(disc_r_logic_less_o, true);
									else				function.set(r_r_logic_less_r),				signature=SIG_R_R,	d(disc_r_logic_less_o, true);			return;
		case M_LOGIC_LESS_EQUAL:		 if(side=='<')	function.set(r_r_logic_less_equal_l),		signature=SIG_R_R,	d(disc_r_logic_less_equal_o, true);
									else				function.set(r_r_logic_less_equal_r),		signature=SIG_R_R,	d(disc_r_logic_less_equal_o, true);		return;
		case M_LOGIC_GREATER:			 if(side=='<')	function.set(r_r_logic_greater_l),			signature=SIG_R_R,	d(disc_r_logic_greater_o, true);
									else				function.set(r_r_logic_greater_r),			signature=SIG_R_R,	d(disc_r_logic_greater_o, true);		return;
		case M_LOGIC_GREATER_EQUAL:		 if(side=='<')	function.set(r_r_logic_greater_equal_l),	signature=SIG_R_R,	d(disc_r_logic_greater_equal_o, true);
									else				function.set(r_r_logic_greater_equal_r),	signature=SIG_R_R,	d(disc_r_logic_greater_equal_o, true);	return;
		CASE(R_R, LOGIC_EQUAL,		logic_equal, 		O)//TODO: all logic (boolean) functions have output disc	DONE
		CASE(R_R, LOGIC_NOT_EQUAL,	logic_not_equal,	O)
		CASE(R_R, BITWISE_AND,		bitwise_and,		O)
		CASE(R_R, BITWISE_NAND,		bitwise_nand,		O)
		CASE(R_R, BITWISE_XOR,		bitwise_xor,		O)
		CASE(R_R, BITWISE_XNOR,		bitwise_xnor,		O)
		CASE_R_R(VERTICAL_BAR, bitwise_or, BITWISE_OR, O)
		CASE(R_R, BITWISE_NOR,		bitwise_nor,		O)
		CASE(R_R, COS,				cos,				C)
		CASE(C_C, ACOS,				acos,				I)
		CASE(R_R, COSH,				cosh,				C)
		CASE(C_C, ACOSH,			acosh,				C)
		CASE(R_R, COSC,				cosc,				I)
		CASE(R_R, SEC,				sec,				I)
		CASE(C_C, ASEC,				asec,				I)
		CASE(R_R, SECH,				sech,				C)
		CASE(C_C, ASECH,			asech,				I)
		CASE(R_R, SIN,				sin,				C)
		CASE(C_C, ASIN,				asin,				I)
		CASE(R_R, SINH,				sinh,				C)
		CASE(R_R, ASINH,			asinh,				C)
		CASE(R_R, SINC,				sinc,				C)
		CASE(R_R, SINHC,			sinhc,				C)
		CASE(R_R, CSC,				csc,				I)
		CASE(C_C, ACSC,				acsc,				I)
		CASE(R_R, CSCH,				csch,				I)
		CASE(R_R, ACSCH,			acsch,				I)
		CASE(R_R, TAN,				tan,				I)
		CASE(R_R, ATAN,				atan,				C)
		CASE(R_R, TANH,				tanh,				C)
		CASE(C_C, ATANH,			atanh,				I)
		CASE(R_R, TANC,				tanc,				I)
		CASE(R_R, COT,				cot,				I)
		CASE(R_R, ACOT,				acot,				I)
		CASE(R_R, COTH,				coth,				I)
		CASE(C_C, ACOTH,			acoth,				I)
		CASE(R_R, EXP,				exp,				C)
		CASE(C_C, LN,				ln,					I)
		CASE(C_C, LOG,				log,				I)
		CASE(C_C, SQRT,				sqrt,				C)
		CASE(R_R, CBRT,				cbrt,				C)
		CASE(R_R, INVSQRT,			invsqrt,			C)
		CASE(R_R, SQ,				sq,					C)
		CASE(R_R, ERF,				erf,				C)
		CASE(R_R, FIB,				fib,				C)
		CASE(R_R, ZETA,				zeta,				I)
		CASE(R_R, STEP,				step,				I)
		CASE(R_R, SGN,				sgn,				I)
		CASE(R_R, RECT,				rect,				I)
		CASE_R_R(TENT,				trgl, TRGL,			C)
		CASE(R_R, CEIL,				ceil,				O)
		CASE(R_R, FLOOR,			floor,				O)
		CASE(R_R, ROUND,			round,				O)
		CASE(R_R, INT,				int,				O)
		CASE(R_R, FRAC,				frac,				I)
		CASE(R_R, ABS,				abs,				C)
		CASE(R_R, ARG,				arg,				I)
		CASE_R_R(RAND,				random, RANDOM,		O)
		CASE_R_R(GAMMA,				tgamma, TGAMMA,		I)
		CASE_R_R(LNGAMMA,			loggamma, LOGGAMMA,	I)
		CASE(R_R, GAUSS,			gauss,				C)
		CASE(R_R, PERMUTATION,		permutation,		C)
		CASE(R_R, COMBINATION,		combination,		C)
		CASE(R_R, SQWV,				sqwv,				O)//TODO: output disc	DONE
		CASE(R_R, TRWV,				trwv,				C)
		CASE(R_R, SAW,				saw,				I)
		CASE_R_R(HYPOT,				abs, ABS,			C)
		CASE(R_R, MANDELBROT,		mandelbrot,			O)//TODO: output disc	DONE
	//	CASE(R_R, MIN,				min,				C)
	//	CASE(R_R, MAX,				max,				C)
		CASE(R_R, BETA,				beta,				I)
		CASE(R_R, BESSEL_J,			bessel_j,			I)//TODO: rename M_BESSEL->M_BESSEL_J, r_r_cyl_bessel_j->r_r_bessel_j	DONE
		CASE(R_R, BESSEL_Y,			bessel_y,			I)//TODO: rename M_NEUMANN->M_BESSEL_Y, r_r_cyl_neumann->r_r_bessel_y	DONE
		CASE(C_R, HANKEL1,			hankel1,			I)
		}
		break;
	case 'c':
		switch(f)
		{
		CASE(R_C, REAL,				real,				C)
		CASE(R_C, IMAG,				imag,				C)
		CASE(C_C, CONJUGATE,		conjugate,			C)
		CASE(C_C, POLAR,			polar,				I)
		CASE(C_C, CARTESIAN,		cartesian,			C)
		CASE(C_C, DIVIDE,			divide,				I)
		CASE(C_C, MINUS,			minus,				C)
		CASE_C_C(MODULO_PERCENT,	percent, PERCENT,	C)
		CASE(C_C, INCREMENT,		increment,			C)
		CASE(C_C, DECREMENT,		decrement,			C)
		CASE(C_C, BITWISE_NOT,		bitwise_not,		I)
		case M_BITWISE_SHIFT_LEFT:		 if(side=='<')	function.set(c_c_bitwise_shift_left_l),		signature=SIG_C_C,		d(disc_c_bitwise_shift_left_l_o, false);
									else				function.set(c_c_bitwise_shift_left_r),		signature=SIG_C_C,		d();											return;
		case M_BITWISE_SHIFT_RIGHT:		 if(side=='<')	function.set(c_c_bitwise_shift_right_l),	signature=SIG_C_C,		d(disc_c_bitwise_shift_right_l_o, false);
									else				function.set(c_c_bitwise_shift_right_r),	signature=SIG_C_C,		d();											return;
		case M_FACTORIAL_LOGIC_NOT:		 if(side=='<')	function.set(r_c_logic_not),				signature=SIG_R_C,		d(disc_r_logic_not_o, true);
									else				function.set(c_c_factorial),				signature=SIG_C_C,		d(disc_c_factorial_i, true);			return;
		case M_LOGIC_LESS:				 if(side=='<')	function.set(r_c_logic_less_l),				signature=SIG_R_C,		d(disc_r_logic_less_o, true);
									else				function.set(r_c_logic_less_r),				signature=SIG_R_C,		d(disc_r_logic_less_o, true);			return;
		case M_LOGIC_LESS_EQUAL:		 if(side=='<')	function.set(r_c_logic_less_equal_l),		signature=SIG_R_C,		d(disc_r_logic_less_equal_o, true);
									else				function.set(r_c_logic_less_equal_r),		signature=SIG_R_C,		d(disc_r_logic_less_equal_o, true);		return;
		case M_LOGIC_GREATER:			 if(side=='<')	function.set(r_c_logic_greater_l),			signature=SIG_R_C,		d(disc_r_logic_greater_o, true);
									else				function.set(r_c_logic_greater_r),			signature=SIG_R_C,		d(disc_r_logic_greater_o, true);		return;
		case M_LOGIC_GREATER_EQUAL:		 if(side=='<')	function.set(r_c_logic_greater_equal_l),	signature=SIG_R_C,		d(disc_r_logic_greater_equal_o, true);
									else				function.set(r_c_logic_greater_equal_r),	signature=SIG_R_C,		d(disc_r_logic_greater_equal_o, true);	return;
		CASE(R_C, LOGIC_EQUAL,		logic_equal,		O)
		CASE(R_C, LOGIC_NOT_EQUAL,	logic_not_equal,	O)
		CASE(C_C, BITWISE_AND,		bitwise_and,		O)
		CASE(C_C, BITWISE_NAND,		bitwise_nand,		O)
		CASE(C_C, BITWISE_XOR,		bitwise_xor,		O)
		CASE(C_C, BITWISE_XNOR,		bitwise_xnor,		O)
		CASE_C_C(VERTICAL_BAR,		bitwise_or, BITWISE_OR, O)
		CASE(C_C, BITWISE_NOR,		bitwise_nor,		O)
		CASE(C_C, COS,				cos,				C)
		CASE(C_C, ACOS,				acos,				I)//disc_c_ln_i?
		CASE(C_C, COSH,				cosh,				C)
		CASE(C_C, ACOSH,			acosh,				C)
		CASE(C_C, COSC,				cosc,				I)
		CASE(C_C, SEC,				sec,				I)
		CASE(C_C, ASEC,				asec,				I)
		CASE(C_C, SECH,				sech,				I)
		CASE(C_C, ASECH,			asech,				I)
		CASE(C_C, SIN,				sin,				C)
		CASE(C_C, ASIN,				asin,				I)
		CASE(C_C, SINH,				sinh,				C)
		CASE(C_C, ASINH,			asinh,				I)

		CASE(C_C, SINC,				sinc,				C)
		CASE(C_C, SINHC,			sinhc,				C)
		CASE(C_C, CSC,				csc,				I)
		CASE(C_C, ACSC,				acsc,				I)
		CASE(C_C, CSCH,				csch,				I)
		CASE(C_C, ACSCH,			acsch,				I)
		CASE(C_C, TAN,				tan,				I)
		CASE(C_C, ATAN,				atan,				I)
		CASE(C_C, TANH,				tanh,				C)
		CASE(C_C, ATANH,			atanh,				I)
		CASE(C_C, TANC,				tanc,				I)
		CASE(C_C, COT,				cot,				I)
		CASE(C_C, ACOT,				acot,				I)
		CASE(C_C, COTH,				coth,				I)
		CASE(C_C, ACOTH,			acoth,				I)
		CASE(C_C, EXP,				exp,				C)
		CASE(C_C, LN,				ln,					I)
		CASE(C_C, LOG,				log,				I)
		CASE(C_C, SQRT,				sqrt,				C)
		CASE(C_C, CBRT,				cbrt,				C)
		CASE(C_C, SQ,				sq,					C)
		CASE(C_C, FIB,				fib,				C)
		CASE(C_C, SGN,				sgn,				I)
		CASE(C_C, STEP,				step,				I)
		CASE(C_C, RECT,				rect,				I)
		CASE_R_C(TENT,				trgl, TRGL,			C)
		CASE(C_C, CEIL,				ceil,				O)
		CASE(C_C, FLOOR,			floor,				O)
		CASE(C_C, ROUND,			round,				O)
		CASE(C_C, INT,				int,				O)
		CASE(C_C, FRAC,				frac,				I)
		CASE(R_C, ABS,				abs,				C)
		CASE(R_C, ARG,				arg,				I)
		CASE_NONE(INVSQRT)
		CASE_NONE(ERF)
		CASE_NONE(ZETA)
		CASE_C_C(RAND,				random, RANDOM,		O)
		CASE_C_C(GAMMA,				tgamma, TGAMMA,		I)
		CASE_NONE(LNGAMMA)
		CASE(C_C, GAUSS,			gauss,				C)
		CASE(C_C, COMBINATION,		combination,		C)
		CASE(C_C, PERMUTATION,		permutation,		C)
		CASE(R_C, SQWV,				sqwv,				O)
		CASE(R_C, TRWV,				trwv,				C)
		CASE(R_C, SAW,				saw,				I)
		CASE(R_C, MANDELBROT,		mandelbrot,			C)
	//	CASE(C_C, MIN,				min,				C)
	//	CASE(C_C, MAX,				max,				C)
		CASE_NONE(BETA)
		CASE_NONE(BESSEL_J)
		CASE_NONE(BESSEL_Y)
		CASE(C_C, HANKEL1,			hankel1,			C)

//		case M_SINC:				function.set(c_c_sinc),					umts=returns_rcq,		d();								return;
//		case M_SINHC:				function.set(c_c_sinhc),					umts=returns_rcq,		d();								return;
//		case M_CSC:					function.set(c_c_csc),						umts=returns_rcq,		d(disc_c_csc_i, true);				return;
//		case M_ACSC:				function.set(c_c_acsc),					umts=returns_ccq,		d(disc_c_acsc_i, true);				return;
//		case M_CSCH:				function.set(c_c_csch),					umts=returns_rcq,		d(disc_c_csch_i, true);				return;
//		case M_ACSCH:				function.set(c_c_acsch),					umts=returns_rcq,		d(disc_c_acsch_i, true);			return;
//		case M_TAN:					function.set(c_c_tan),						umts=returns_rcq,		d(disc_c_tan_i, true);				return;
//		case M_ATAN:				function.set(c_c_atan),					umts=returns_rcq,		d(disc_c_atan_i, true);				return;
//		case M_TANH:				function.set(c_c_tanh),					umts=returns_rcq,		d();				return;
//		case M_ATANH:				function.set(c_c_atanh),					umts=returns_ccq,		d(disc_c_atanh_i, true);			return;
//		case M_TANC:				function.set(c_c_tanc),					umts=returns_rcq,		d(disc_c_tanc_i, true);				return;
//		case M_COT:					function.set(c_c_cot),						umts=returns_rcq,		d(disc_c_cot_i, true);				return;
//		case M_ACOT:				function.set(c_c_acot),					umts=returns_rcq,		d(disc_c_acot_i, true);				return;
//		case M_COTH:				function.set(c_c_coth),					umts=returns_rcq,		d(disc_c_acoth_i, true);			return;
//		case M_ACOTH:				function.set(c_c_acoth),					umts=returns_ccq,		d(disc_c_acoth_i, true);			return;
//		case M_EXP:					function.set(c_c_exp),						umts=returns_rcq,		d();								return;
//		case M_LN:					function.set(c_c_ln),						umts=returns_ccq,		d(disc_c_ln_i, true);				return;
//		case M_LOG:					function.set(c_c_log),						umts=returns_ccq,		d(disc_c_log_i, true);				return;
//		case M_SQRT:				function.set(c_c_sqrt),					umts=returns_ccq,		d();								return;
//		case M_CBRT:				function.set(c_c_cbrt),					umts=returns_rcq,		d();								return;
//		case M_SQ:					function.set(c_c_sq),						umts=returns_rcq,		d();								return;
//		case M_FIB:					function.set(c_c_fib),						umts=returns_rcq,		d();								return;
//		case M_SGN:					function.set(c_c_sgn),						umts=returns_rcq,		d(disc_c_sgn_i, true);				return;
//		case M_STEP:				function.set(c_c_step),					umts=returns_rcq,		d(disc_c_step_i, true);				return;
//		case M_RECT:				function.set(c_c_rect),					umts=returns_rcq,		d(disc_c_rect_i, true);				return;
//		case M_TENT:				function.set(r_c_trgl),					umts=returns_rrr,		d();								return;
//		case M_CEIL:				function.set(c_c_ceil),	umts=returns_rcq,		d(disc_c_ceil_o, false);			return;
//		case M_FLOOR:				function.set(c_c_floor),	umts=returns_rcq,		d(disc_c_floor_o, false);			return;
//		case M_ROUND:				function.set(c_c_round),	umts=returns_rcq,		d(disc_c_round_o, false);			return;
//		case M_INT:					function.set(c_c_int),			umts=returns_rcq,		d(disc_c_int_o, false);				return;
//		case M_FRAC:				function.set(c_c_frac),			umts=returns_rcq,		d(disc_c_frac_i, true);				return;
//		case M_ABS:					function.set(r_c_abs),						umts=returns_rrr,		d();								return;
//		case M_ARG:					function.set(r_c_arg),						umts=returns_rrr,		d(disc_c_arg_i, true);				return;
//		case M_INVSQRT:				function.set();																						return;
//		case M_ERF:					function.set();																						return;
//		case M_ZETA:				function.set();																						return;
//		case M_RAND:				function.set(c_c_random),					umts=returns_rcq,		d(disc_c_random_o, false);			return;
//		case M_GAMMA:				function.set(c_c_tgamma),					umts=returns_rcq,		d(disc_c_tgamma_i, true);			return;
//		case M_LNGAMMA:				function.set(),							umts=returns_rXX,		d();								return;
//		case M_GAUSS:				function.set(c_c_gauss),					umts=returns_rcq,		d();								return;
//		case M_COMBINATION:			function.set(c_c_combination),				umts=returns_rcq,		d();								return;
//		case M_PERMUTATION:			function.set(c_c_permutation),				umts=returns_rcq,		d();								return;
//		case M_SQWV:				function.set(r_c_sqwv),					umts=returns_rcq,		d(disc_c_sqwv_i, true);				return;
//		case M_TRWV:				function.set(r_c_trwv),					umts=returns_rcq,		d(disc_c_trwv_i, true);				return;
//		case M_SAW:					function.set(c_c_saw),						umts=returns_rcq,		d(disc_c_saw_i, true);				return;
//		case M_MANDELBROT:			function.set(r_c_mandelbrot);		umts=returns_rrr,		d();								return;
//	//	case M_MIN:					function.set(c_c_min),						umts=returns_rcq,		d();								return;
//	//	case M_MAX:					function.set(c_c_max),						umts=returns_rcq,		d();								return;
//		case M_BETA:				function.set();																						return;
//		case M_BESSEL_J:				function.set();																						return;
//		case M_BESSEL_Y:				function.set();																						return;
//		case M_HANKEL1:				function.set(c_c_hankel1);					umts=returns_rcq,		d();							return;
		}
		break;
	case 'h':
		switch(f)
		{
		CASE(R_C, REAL,				real,				C)
		CASE(R_C, IMAG,				imag,				C)
		CASE(Q_Q, CONJUGATE,		conjugate,			C)
		CASE(C_Q, POLAR,			polar,				I)
		CASE(Q_Q, CARTESIAN,		cartesian,			C)
		CASE(Q_Q, DIVIDE,			divide,				I)
		CASE(Q_Q, MINUS,			minus,				C)
		CASE_Q_Q(MODULO_PERCENT,	percent, PERCENT,	C)
		CASE(Q_Q, INCREMENT,		increment,			C)
		CASE(Q_Q, DECREMENT,		decrement,			C)
		CASE(Q_Q, BITWISE_NOT,		bitwise_not,		I)
//		case M_REAL:				function.set(r_c_real),					umts=returns_rrr,		d();								return;
//		case M_IMAG:				function.set(r_c_imag),					umts=returns_rrr,		d();								return;
//		case M_CONJUGATE:			function.set(q_q_conjugate),				umts=returns_rcq,		d();								return;
//		case M_POLAR:				function.set(c_q_polar),					umts=returns_ccc,		d(disc_q_polar_i, true);			return;
//		case M_CARTESIAN:			function.set(q_q_cartesian),				umts=returns_rcq,		d();								return;
//		case M_DIVIDE:				function.set(q_q_divide),					umts=returns_rcq,		d(disc_q_divide_i, true);			return;
//		case M_MINUS:				function.set(q_q_minus),					umts=returns_rcq,		d();								return;
//		case M_MODULO_PERCENT:		function.set(q_q_percent),					umts=returns_rcq,		d();								return;
//		case M_INCREMENT:			function.set(q_q_increment),					umts=returns_rcq,		d();								return;
//		case M_DECREMENT:			function.set(q_q_decrement),					umts=returns_rcq,		d();								return;
//		case M_BITWISE_NOT:			function.set(q_q_bitwise_not),				umts=returns_rcq,		d(disc_q_bitwise_not_i, true);		return;
		case M_BITWISE_SHIFT_LEFT:		 if(side=='<')	function.set(q_q_bitwise_shift_left_l),		signature=SIG_Q_Q,		d(disc_q_bitwise_shift_left_l_o, false);
									else				function.set(q_q_bitwise_shift_left_r),		signature=SIG_Q_Q,		d();										return;
		case M_BITWISE_SHIFT_RIGHT:		 if(side=='<')	function.set(q_q_bitwise_shift_right_l),	signature=SIG_Q_Q,		d(disc_q_bitwise_shift_right_l_o, false);
									else				function.set(q_q_bitwise_shift_right_r),	signature=SIG_Q_Q,		d();										return;
		case M_FACTORIAL_LOGIC_NOT:		 if(side=='<')	function.set(r_q_logic_not),				signature=SIG_R_Q,		d(disc_r_logic_not_o, true);
									else				function.set(q_q_factorial),				signature=SIG_Q_Q,		d(disc_q_factorial_i, true);				return;
		case M_LOGIC_LESS:				 if(side=='<')	function.set(r_q_logic_less_l),				signature=SIG_R_Q,		d(disc_r_logic_less_o, true);
									else				function.set(r_q_logic_less_r),				signature=SIG_R_Q,		d(disc_r_logic_less_o, true);				return;
		case M_LOGIC_LESS_EQUAL:		 if(side=='<')	function.set(r_q_logic_less_equal_l),		signature=SIG_R_Q,		d(disc_r_logic_less_equal_o, true);
									else				function.set(r_q_logic_less_equal_r),		signature=SIG_R_Q,		d(disc_r_logic_less_equal_o, true);			return;
		case M_LOGIC_GREATER:			 if(side=='<')	function.set(r_q_logic_greater_l),			signature=SIG_R_Q,		d(disc_r_logic_greater_o, true);
									else				function.set(r_q_logic_greater_r),			signature=SIG_R_Q,		d(disc_r_logic_greater_o, true);			return;
		case M_LOGIC_GREATER_EQUAL:		 if(side=='<')	function.set(r_q_logic_greater_equal_l),	signature=SIG_R_Q,		d(disc_r_logic_greater_equal_o, true);
									else				function.set(r_q_logic_greater_equal_r),	signature=SIG_R_Q,		d(disc_r_logic_greater_equal_o, true);		return;
		CASE(R_Q, LOGIC_EQUAL,		logic_equal,		O)
		CASE(R_Q, LOGIC_NOT_EQUAL,	logic_not_equal,	O)
		CASE(Q_Q, BITWISE_AND,		bitwise_and,		O)
		CASE(Q_Q, BITWISE_NAND,		bitwise_nand,		O)
		CASE(Q_Q, BITWISE_XOR,		bitwise_xor,		O)
		CASE(Q_Q, BITWISE_XNOR,		bitwise_xnor,		O)
		CASE_Q_Q(VERTICAL_BAR,		bitwise_or, BITWISE_OR, O)
		CASE(Q_Q, BITWISE_NOR,		bitwise_nor,		O)
		CASE(Q_Q, COS,				cos,				C)
		CASE(Q_Q, ACOS,				acos,				I)
		CASE(Q_Q, COSH,				cosh,				C)
		CASE(Q_Q, ACOSH,			acosh,				C)
		CASE(Q_Q, COSC,				cosc,				I)
		CASE(Q_Q, SEC,				sec,				I)
		CASE(Q_Q, ASEC,				asec,				I)
		CASE(Q_Q, SECH,				sech,				I)
		CASE(Q_Q, ASECH,			asech,				I)
		CASE(Q_Q, SIN,				sin,				C)
		CASE(Q_Q, ASIN,				asin,				I)
		CASE(Q_Q, SINH,				sinh,				C)
		CASE(Q_Q, ASINH,			asinh,				I)
		CASE(Q_Q, SINC,				sinc,				C)
		CASE(Q_Q, SINHC,			sinhc,				C)
		CASE(Q_Q, CSC,				csc,				I)
		CASE(Q_Q, ACSC,				acsc,				I)
		CASE(Q_Q, CSCH,				csch,				I)
		CASE(Q_Q, ACSCH,			acsch,				I)
		CASE(Q_Q, TAN,				tan,				I)
		CASE(Q_Q, ATAN,				atan,				I)
		CASE(Q_Q, TANH,				tanh,				C)
		CASE(Q_Q, ATANH,			atanh,				I)
		CASE(Q_Q, TANC,				tanc,				I)
		CASE(Q_Q, COT,				cot,				I)
		CASE(Q_Q, ACOT,				acot,				I)
		CASE(Q_Q, COTH,				coth,				I)
		CASE(Q_Q, ACOTH,			acoth,				I)
		CASE(Q_Q, EXP,				exp,				C)
		CASE(Q_Q, LN,				ln,					I)
		CASE(Q_Q, LOG,				log,				I)
		CASE(Q_Q, SQRT,				sqrt,				C)
		CASE(Q_Q, CBRT,				cbrt,				C)
		CASE(Q_Q, SQ,				sq,					C)
		CASE(Q_Q, FIB,				fib,				C)
		CASE(Q_Q, SGN,				sgn,				I)
		CASE(Q_Q, STEP,				step,				I)
		CASE(Q_Q, RECT,				rect,				I)
		CASE_R_Q(TENT,				trgl, TRGL,			C)
		CASE(Q_Q, CEIL,				ceil,				O)
		CASE(Q_Q, FLOOR,			floor,				O)
		CASE(Q_Q, ROUND,			round,				O)
		CASE(Q_Q, INT,				int,				O)
		CASE(Q_Q, FRAC,				frac,				I)
		CASE(R_Q, ABS,				abs,				C)
		CASE(R_Q, ARG,				arg,				I)
		CASE_NONE(INVSQRT)
		CASE_NONE(ERF)
		CASE_NONE(ZETA)
		CASE_Q_Q(RAND,				random, RANDOM,		O)
		CASE_Q_Q(GAMMA,				tgamma, TGAMMA,		I)
		CASE_NONE(LNGAMMA)
		CASE(Q_Q, GAUSS,			gauss,				C)
		CASE(Q_Q, PERMUTATION,		permutation,		C)
		CASE(Q_Q, COMBINATION,		combination,		C)
		CASE(R_Q, SQWV,				sqwv,				O)
		CASE(R_Q, TRWV,				trwv,				C)
		CASE(R_Q, SAW,				saw,				I)
	//	CASE(Q_Q, MIN,				min,				C)
	//	CASE(Q_Q, MAX,				max,				C)
		CASE_NONE(BETA)
		CASE_NONE(BESSEL_J)
		CASE_NONE(BESSEL_Y)
		CASE_NONE(HANKEL1)

//		case M_LOGIC_EQUAL:			function.set(r_q_logic_equal),				umts=returns_rrr,		d(disc_q_logic_equal_i, true);		return;
//		case M_LOGIC_NOT_EQUAL:		function.set(r_q_logic_not_equal),			umts=returns_rrr,		d(disc_q_logic_not_equal_i, true);	return;
//		case M_BITWISE_AND:			function.set(q_q_bitwise_and),				umts=returns_rcq,		d(disc_q_bitwise_and_o, false);		return;
//		case M_BITWISE_NAND:		function.set(q_q_bitwise_nand),			umts=returns_rcq,		d(disc_q_bitwise_or_o, false);		return;
//		case M_BITWISE_XOR:			function.set(q_q_bitwise_xor),				umts=returns_rcq,		d(disc_q_bitwise_xor_o, false);		return;
//		case M_BITWISE_XNOR:		function.set(q_q_bitwise_xnor),			umts=returns_rcq,		d(disc_q_bitwise_xnor_o, false);	return;
//		case M_VERTICAL_BAR:		function.set(q_q_bitwise_or),				umts=returns_rcq,		d(disc_q_bitwise_or_o, false);		return;
//		case M_BITWISE_NOR:			function.set(q_q_bitwise_nor),				umts=returns_rcq,		d(disc_q_bitwise_nor_o, false);		return;
//		case M_COS:					function.set(q_q_cos),						umts=returns_rcq,		d();								return;
//		case M_ACOS:				function.set(q_q_acos),					umts=returns_ccq,		d(disc_q_acos_i, true);				return;
//		case M_COSH:				function.set(q_q_cosh),					umts=returns_rcq,		d();								return;
//		case M_ACOSH:				function.set(q_q_acosh),					umts=returns_ccq,		d();								return;
//		case M_COSC:				function.set(q_q_cosc),					umts=returns_rcq,		d(disc_q_cosc_i, true);				return;
//		case M_SEC:					function.set(q_q_sec),						umts=returns_rcq,		d(disc_q_sec_i, true);				return;
//		case M_ASEC:				function.set(q_q_asec),					umts=returns_ccq,		d(disc_q_asec_i, true);				return;
//		case M_SECH:				function.set(q_q_sech),					umts=returns_rcq,		d(disc_q_sech_i, true);				return;
//		case M_ASECH:				function.set(q_q_asech),					umts=returns_ccq,		d(disc_q_asech_i, true);			return;
//		case M_SIN:					function.set(q_q_sin),						umts=returns_rcq,		d();								return;
//		case M_ASIN:				function.set(q_q_asin),					umts=returns_ccq,		d(disc_q_asin_i, true);				return;
//		case M_SINH:				function.set(q_q_sinh),					umts=returns_rcq,		d();								return;
//		case M_ASINH:				function.set(q_q_asinh),					umts=returns_rcq,		d(disc_q_asinh_i, true);			return;
//		case M_SINC:				function.set(q_q_sinc),					umts=returns_rcq,		d();								return;
//		case M_SINHC:				function.set(q_q_sinhc),					umts=returns_rcq,		d();								return;
//		case M_CSC:					function.set(q_q_csc),						umts=returns_rcq,		d(disc_q_csc_i, true);				return;
//		case M_ACSC:				function.set(q_q_acsc),					umts=returns_ccq,		d(disc_q_acsc_i, true);				return;
//		case M_CSCH:				function.set(q_q_csch),					umts=returns_rcq,		d(disc_q_csch_i, true);				return;
//		case M_ACSCH:				function.set(q_q_acsch),					umts=returns_rcq,		d(disc_q_acsch_i, true);			return;
//		case M_TAN:					function.set(q_q_tan),						umts=returns_rcq,		d(disc_q_tan_i, true);				return;
//		case M_ATAN:				function.set(q_q_atan),					umts=returns_rcq,		d(disc_q_atan_i, true);				return;
//		case M_TANH:				function.set(q_q_tanh),					umts=returns_rcq,		d();				return;
//		case M_ATANH:				function.set(q_q_atanh),					umts=returns_ccq,		d(disc_q_atanh_i, true);			return;
//		case M_TANC:				function.set(q_q_tanc),					umts=returns_rcq,		d(disc_q_tanc_i, true);				return;
//		case M_COT:					function.set(q_q_cot),						umts=returns_rcq,		d(disc_q_cot_i, true);				return;
//		case M_ACOT:				function.set(q_q_acot),					umts=returns_rcq,		d(disc_q_acot_i, true);				return;
//		case M_COTH:				function.set(q_q_coth),					umts=returns_rcq,		d(disc_q_coth_i, true);				return;
//		case M_ACOTH:				function.set(q_q_acoth),					umts=returns_ccq,		d(disc_q_acoth_i, true);			return;
//		case M_EXP:					function.set(q_q_exp),						umts=returns_rcq,		d();								return;
//		case M_LN:					function.set(q_q_ln),						umts=returns_ccq,		d(disc_q_ln_i, true);				return;
//		case M_LOG:					function.set(q_q_log),						umts=returns_ccq,		d(disc_q_log_i, true);				return;
//		case M_SQRT:				function.set(q_q_sqrt),					umts=returns_ccq,		d();								return;
//		case M_CBRT:				function.set(q_q_cbrt),					umts=returns_rcq,		d();								return;
//		case M_SQ:					function.set(q_q_sq),						umts=returns_rcq,		d();								return;
//		case M_FIB:					function.set(q_q_fib),						umts=returns_rcq,		d();								return;
//		case M_SGN:					function.set(q_q_sgn),						umts=returns_rcq,		d(disc_q_sgn_i, true);				return;
//		case M_STEP:				function.set(q_q_step),					umts=returns_rcq,		d(disc_q_step_i, true);				return;
//		case M_RECT:				function.set(q_q_rect),					umts=returns_rcq,		d(disc_q_rect_i, true);				return;
//		case M_TENT:				function.set(r_q_trgl),					umts=returns_rrr,		d();								return;
//		case M_CEIL:				function.set(q_q_ceil),	umts=returns_rcq,		d(disc_q_ceil_o, false);			return;
//		case M_FLOOR:				function.set(q_q_floor),	umts=returns_rcq,		d(disc_q_floor_o, false);			return;
//		case M_ROUND:				function.set(q_q_round),	umts=returns_rcq,		d(disc_q_round_o, false);			return;
//		case M_INT:					function.set(q_q_int),			umts=returns_rcq,		d(disc_q_int_o, false);				return;
//		case M_FRAC:				function.set(q_q_frac),			umts=returns_rcq,		d(disc_q_frac_i, true);				return;
//		case M_ABS:					function.set(r_q_abs),						umts=returns_rrr,		d();								return;
//		case M_ARG:					function.set(r_q_arg),						umts=returns_rrr,		d(disc_q_arg_i, true);				return;
//		case M_INVSQRT:				function.set();																						return;
//		case M_ERF:					function.set();																						return;
//		case M_ZETA:				function.set();																						return;
//		case M_RAND:				function.set(q_q_random),					umts=returns_rcq,		d(disc_q_random_o, false);			return;
//		case M_GAMMA:				function.set(q_q_tgamma),					umts=returns_rcq,		d(disc_q_tgamma_i, true);			return;
//		case M_LNGAMMA:				function.set(),							umts=returns_rXX,		d();								return;
//		case M_GAUSS:				function.set(q_q_gauss),					umts=returns_rcq,		d();								return;
//		case M_PERMUTATION:			function.set(q_q_permutation),				umts=returns_rcq,		d();								return;
//		case M_COMBINATION:			function.set(q_q_combination),				umts=returns_rcq,		d();								return;
//		case M_SQWV:				function.set(r_q_sqwv),					umts=returns_rcq,		d(disc_q_sqwv_i, true);				return;
//		case M_TRWV:				function.set(r_q_trwv),					umts=returns_rcq,		d(disc_q_trwv_i, true);				return;
//		case M_SAW:					function.set(q_q_saw),						umts=returns_rcq,		d(disc_q_saw_i, true);				return;
//	//	case M_MIN:					function.set(q_q_min),						umts=returns_rcq,		d();								return;
//	//	case M_MAX:					function.set(q_q_max),						umts=returns_rcq,		d();								return;
//		case M_BETA:				function.set();																						return;
//		case M_BESSEL_J:				function.set();																						return;
//		case M_BESSEL_Y:				function.set();																						return;
//		case M_HANKEL1:				function.set();																						return;
		}
		break;
	}
}
//void			Compile::compile_instruction_select_b	(int f, char op1type, char op2type, FunctionPointer &function, char (*&bmts)(char, char), DiscontinuityFunction &d)
void			Compile::compile_instruction_select_b	(int f, char op1type, char op2type, FunctionPointer &function, int &signature, DiscontinuityFunction &d, int &cl_idx, int &cl_disc_idx)
{
	using namespace G2;
	switch(op1type)
	{
	case 'R':
		switch(op2type)
		{
		case 'R':
			switch(f)
			{
			CASE_C_CR(POWER,			pow, POW,		I)
		//	case M_POWER:				function.set(c_cr_pow),						bmts=returns_ccq_ccq_qqq,	d(disc_cr_pow_i);					return;
			case M_ASSIGN_DIVIDE:
			CASE(R_RR, DIVIDE,			divide,			I)
			CASE(R_RR, LOGIC_DIVIDES,	logic_divides,	C)
		//	case M_DIVIDE:				function.set(r_rr_divide),					bmts=returns_rcq_ccq_qqq,	d(disc_rr_divide_i);				return;
		//	case M_LOGIC_DIVIDES:		function.set(r_rr_logic_divides),			bmts=returns_rcq_ccq_qqq,	d();	return;
			case M_ASSIGN_MINUS:
			CASE(R_RR, MINUS,			minus,			C)
			CASE(C_RR, PENTATE,			pentate,		I)
			CASE(C_RR, TETRATE,			tetrate,		I)
			CASE(R_RR, POWER_REAL,		power_real,		I)
			case M_ASSIGN_MULTIPLY:
			CASE(R_RR, MULTIPLY,		multiply,		C)
			case M_ASSIGN_PLUS:
			CASE(R_RR, PLUS,			plus,			C)
			CASE(R_RR, LOGIC_AND,		logic_and,		O)
			CASE(R_RR, LOGIC_XOR,		logic_xor,		O)
			CASE(R_RR, LOGIC_OR,		logic_or,		O)
			CASE(R_RR, CONDITION_ZERO,	condition_zero,	I)
			case M_ASSIGN_MOD:
			CASE_R_RR(MODULO_PERCENT,	modulo, MODULO,	I)
			case M_ASSIGN_LEFT:
			CASE(R_RR, BITWISE_SHIFT_LEFT, bitwise_shift_left, I)
			case M_ASSIGN_RIGHT:
			CASE(R_RR, BITWISE_SHIFT_RIGHT, bitwise_shift_right, I)
			CASE(R_RR, LOGIC_LESS,		logic_less,		O)
			CASE(R_RR, LOGIC_LESS_EQUAL, logic_less_equal, O)
			CASE(R_RR, LOGIC_GREATER,	logic_greater,	O)
			CASE(R_RR, LOGIC_GREATER_EQUAL, logic_greater_equal, O)
			CASE(R_RR, LOGIC_EQUAL,		logic_equal,	O)
			CASE(R_RR, LOGIC_NOT_EQUAL,	logic_not_equal, O)
			case M_ASSIGN_AND:
			CASE(R_RR, BITWISE_AND,		bitwise_and,	O)
			CASE(R_RR, BITWISE_NAND,	bitwise_nand,	O)
			case M_ASSIGN_XOR:
			CASE(R_RR, BITWISE_XOR,		bitwise_xor,	O)
			CASE(R_RR, BITWISE_XNOR,	bitwise_xnor,	O)
			case M_ASSIGN_OR:
			CASE_R_RR(VERTICAL_BAR,		bitwise_or, BITWISE_OR, O)
			CASE(R_RR, BITWISE_NOR,		bitwise_nor,	O)
			CASE(C_CR, LOG,				log,			I)
			CASE_R_RR(RAND,				random, RANDOM,	O)
			CASE(R_RR, ATAN,			atan,			I)
			CASE(R_RR, SQWV,			sqwv,			O)
			CASE(R_RR, TRWV,			trwv,			C)
			CASE(R_RR, SAW,				saw,			I)
			CASE(R_RR, HYPOT,			hypot,			C)
			CASE(R_RR, MANDELBROT,		mandelbrot,		C)
			CASE(R_RR, MIN,				min,			C)
			CASE(R_RR, MAX,				max,			C)
			CASE(R_RR, BETA,			beta,			I)
			CASE_R_RR(GAMMA,			tgamma, TGAMMA,	I)
			CASE(R_RR, PERMUTATION,		permutation,	I)
			CASE(R_RR, COMBINATION,		combination,	I)
			CASE(R_RR, BESSEL_J,		bessel_j,		I)
			CASE(R_RR, BESSEL_Y,		bessel_y,		I)
			CASE(C_RR, HANKEL1,			hankel1,		I)
		//	CASE(R_RR, HANKEL1,			hankel1,		I)
			CASE(R_R, ASSIGN,			assign,			C)
//			case M_ASSIGN_MINUS:
//			case M_MINUS:				function.set(r_rr_minus),					bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_PENTATE:				function.set(c_rr_pentate),					bmts=returns_rXX_rXX_XXX,	d(disc_rr_pentate_i);				return;
//			case M_TETRATE:				function.set(c_rr_tetrate),					bmts=returns_ccX_ccX_qXX,	d(disc_rr_tetrate_i);				return;
//			case M_POWER_REAL:			function.set(r_rr_power_real),				bmts=returns_rXX_cXX_qXX,	d(disc_rr_power_real_i);			return;
//			case M_ASSIGN_MULTIPLY:
//			case M_MULTIPLY:			function.set(r_rr_multiply),				bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_ASSIGN_PLUS:
//			case M_PLUS:				function.set(r_rr_plus),					bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_LOGIC_AND:			function.set(r_rr_logic_and),				bmts=returns_rrr_rrr_rrr,	d(disc_rr_logic_and_i);				return;
//			case M_LOGIC_XOR:			function.set(r_rr_logic_xor),				bmts=returns_rrr_rrr_rrr,	d(disc_rr_logic_xor_i);				return;
//			case M_LOGIC_OR:			function.set(r_rr_logic_or),				bmts=returns_rrr_rrr_rrr,	d(disc_rr_logic_or_i);				return;
//			case M_CONDITION_ZERO:		function.set(r_rr_condition_zero),	bmts=returns_rcq_ccq_qqq,	d(disc_rr_condition_zero_i);	return;
//			case M_ASSIGN_MOD:
//			case M_MODULO_PERCENT:		function.set(r_rr_modulo),					bmts=returns_rcq_ccq_qqq,	d(disc_rr_modulo_i);				return;
//			case M_ASSIGN_LEFT:
//			case M_BITWISE_SHIFT_LEFT:	function.set(r_rr_bitwise_shift_left),		bmts=returns_rcq_ccq_qqq,	d(disc_rr_bitwise_shift_left_i);	return;
//			case M_ASSIGN_RIGHT:
//			case M_BITWISE_SHIFT_RIGHT:	function.set(r_rr_bitwise_shift_right),		bmts=returns_rcq_ccq_qqq,	d(disc_rr_bitwise_shift_right_i);	return;
//			case M_LOGIC_LESS:			function.set(r_rr_logic_less),				bmts=returns_rrr_rrr_rrr,	d(disc_rr_logic_less_i);			return;
//			case M_LOGIC_LESS_EQUAL:	function.set(r_rr_logic_less_equal),		bmts=returns_rrr_rrr_rrr,	d(disc_rr_logic_less_equal_i);		return;
//			case M_LOGIC_GREATER:		function.set(r_rr_logic_greater),			bmts=returns_rrr_rrr_rrr,	d(disc_rr_logic_greater_i);			return;
//			case M_LOGIC_GREATER_EQUAL:	function.set(r_rr_logic_greater_equal),		bmts=returns_rrr_rrr_rrr,	d(disc_rr_logic_greater_equal_i);	return;
//			case M_LOGIC_EQUAL:			function.set(r_rr_logic_equal),				bmts=returns_rrr_rrr_rrr,	d(disc_rr_logic_equal_i);			return;
//			case M_LOGIC_NOT_EQUAL:		function.set(r_rr_logic_not_equal),			bmts=returns_rrr_rrr_rrr,	d(disc_rr_logic_not_equal_i);		return;
//			case M_ASSIGN_AND:
//			case M_BITWISE_AND:			function.set(r_rr_bitwise_and),				bmts=returns_rcq_ccq_qqq,	d(disc_rr_bitwise_and_o, false);	return;
//			case M_BITWISE_NAND:		function.set(r_rr_bitwise_nand),			bmts=returns_rcq_ccq_qqq,	d(disc_rr_bitwise_nand_o, false);	return;
//			case M_ASSIGN_XOR:
//			case M_BITWISE_XOR:			function.set(r_rr_bitwise_xor),				bmts=returns_rcq_ccq_qqq,	d(disc_rr_bitwise_xor_o, false);	return;
//			case M_BITWISE_XNOR:		function.set(r_rr_bitwise_xnor),			bmts=returns_rcq_ccq_qqq,	d(disc_rr_bitwise_xnor_o, false);	return;
//			case M_ASSIGN_OR:
//			case M_VERTICAL_BAR:		function.set(r_rr_bitwise_or),				bmts=returns_rcq_ccq_qqq,	d(disc_rr_bitwise_or_o, false);		return;
//			case M_BITWISE_NOR:			function.set(r_rr_bitwise_nor),				bmts=returns_rcq_ccq_qqq,	d(disc_rr_bitwise_nor_o, false);	return;
//			case M_LOG:					function.set(c_cr_log),						bmts=returns_ccq_ccq_qqq,	d(disc_cr_log_i);					return;
//			case M_RAND:				function.set(r_rr_random),					bmts=returns_rcq_ccq_qqq,	d(disc_rr_random_o, false);			return;
//			case M_ATAN:				function.set(r_rr_atan),					bmts=returns_rcq_ccq_qqq,	d(disc_rr_atan_i);					return;
//			case M_SQWV:				function.set(r_rr_sqwv),					bmts=returns_rcq_ccq_qqq,	d(disc_rr_sqwv_o, false);			return;
//			case M_TRWV:				function.set(r_rr_trwv),					bmts=returns_rcq_ccq_qqq,	d(disc_rr_trwv_i);					return;
//			case M_SAW:					function.set(r_rr_saw),						bmts=returns_rcq_ccq_qqq,	d(disc_rr_saw_i);					return;
//			case M_HYPOT:				function.set(r_rr_hypot),					bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_MANDELBROT:			function.set(r_rr_mandelbrot),				bmts=returns_rrr_rrr_rrr,	d();								return;
//			case M_MIN:					function.set(r_rr_min),						bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_MAX:					function.set(r_rr_max),						bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_BETA:				function.set(r_rr_beta),					bmts=returns_rXX_XXX_XXX,	d(disc_rr_beta_i);					return;
//			case M_GAMMA:				function.set(r_rr_tgamma),					bmts=returns_rXX_XXX_XXX,	d(disc_rr_tgamma_i);				return;
//			case M_PERMUTATION:			function.set(r_rr_permutation),				bmts=returns_rcq_ccq_qqq,	d(disc_rr_permutation_i);			return;
//			case M_COMBINATION:			function.set(r_rr_combination),				bmts=returns_rcq_ccq_qqq,	d(disc_rr_combination_i);			return;
//			case M_BESSEL_J:				function.set(r_rr_bessel_j),			bmts=returns_rXX_XXX_XXX,	d(disc_rr_cyl_bessel_j_i);			return;
//			case M_BESSEL_Y:				function.set(r_rr_bessel_y),				bmts=returns_rXX_XXX_XXX,	d(disc_rr_cyl_neumann_i);			return;
//			case M_HANKEL1:				function.set(c_rr_hankel1),					bmts=returns_cXX_XXX_XXX,	d(disc_rr_hankel1_i);				return;
//		//	case M_HANKEL1:				function.set(r_rr_hankel1),					bmts=returns_cXX_XXX_XXX,	d(disc_rr_hankel1_i);				return;
//			case M_ASSIGN:				function.set(r_r_assign),					bmts=returns_rcq_rcq_rcq,	d();								return;
			}
			break;
		case 'c':
			switch(f)
			{
			CASE_C_CC(POWER,			pow, POW,		I)
			case M_ASSIGN_DIVIDE:
			CASE(C_RC, DIVIDE,			divide,			I)
			CASE(R_RC, LOGIC_DIVIDES,	logic_divides,	C)
			case M_ASSIGN_MINUS:
			CASE(C_RC, MINUS,			minus,			C)
			CASE_NONE(PENTATE)
			CASE(C_RC, TETRATE,			tetrate,		I)
			case M_ASSIGN_MULTIPLY:
			CASE(C_RC, MULTIPLY,		multiply,		C)
			case M_ASSIGN_PLUS:
			CASE(C_RC, PLUS,			plus,			C)
			CASE(R_RC, LOGIC_AND,		logic_and,		O)
			CASE(R_RC, LOGIC_XOR,		logic_xor,		O)
			CASE(R_RC, LOGIC_OR,		logic_or,		O)
			CASE(C_RC, CONDITION_ZERO,	condition_zero,	I)
			case M_ASSIGN_MOD:
			CASE_C_RC(MODULO_PERCENT,	modulo, MODULO,	I)
			case M_ASSIGN_LEFT:
			CASE(C_RC, BITWISE_SHIFT_LEFT, bitwise_shift_left, I)
			case M_ASSIGN_RIGHT:
			CASE(C_RC, BITWISE_SHIFT_RIGHT, bitwise_shift_right, I)
			CASE(R_RC, LOGIC_LESS,		logic_less,		O)
			CASE(R_RC, LOGIC_LESS_EQUAL, logic_less_equal, O)
			CASE(R_RC, LOGIC_GREATER,	logic_greater,	O)
			CASE(R_RC, LOGIC_GREATER_EQUAL, logic_greater_equal, O)
			CASE(R_RC, LOGIC_EQUAL,		logic_equal,	O)
			CASE(R_RC, LOGIC_NOT_EQUAL,	logic_not_equal, O)
			case M_ASSIGN_AND:
			CASE(C_RC, BITWISE_AND,		bitwise_and,	O)
			CASE(C_RC, BITWISE_NAND,	bitwise_nand,	O)
			case M_ASSIGN_XOR:
			CASE(C_RC, BITWISE_XOR,		bitwise_xor,	O)
			CASE(C_RC, BITWISE_XNOR,	bitwise_xnor,	O)
			case M_ASSIGN_OR:
			CASE_C_RC(VERTICAL_BAR,		bitwise_or, BITWISE_OR, O)
			CASE(C_RC, BITWISE_NOR,		bitwise_nor,	O)
			CASE(C_CC, LOG,				log,			I)
			CASE_C_CC(RAND,				random, RANDOM,	O)
		//	CASE_C_RC(RAND,				random, RANDOM,	O)
			CASE(C_RC, ATAN,			atan,			I)
			CASE(R_RC, SQWV,			sqwv,			O)
			CASE(R_RC, TRWV,			trwv,			C)
		//	CASE(C_RC, TRWV,			trwv,			I)
		//	CASE(C_CC, TRWV,			trwv,			C)
			CASE(R_RC, SAW,				saw,			I)
		//	CASE(C_RC, MIN,				min,			C)
			CASE(C_CC, MIN,				min,			C)
		//	CASE(C_RC, MAX,				max,			C)
			CASE(C_CC, MAX,				max,			C)
			CASE_NONE(BETA)
			CASE_NONE(GAMMA)
		//	CASE(C_RC, PERMUTATION,		permutation,	I)
			CASE(C_CC, PERMUTATION,		permutation,	I)
		//	CASE(C_RC, COMBINATION,		combination,	I)
			CASE(C_CC, COMBINATION,		combination,	I)
			CASE_NONE(BESSEL_J)
			CASE_NONE(BESSEL_Y)
			CASE_NONE(HANKEL1)
			CASE(C_C, ASSIGN,			assign,			C)

//			case M_POWER:				function.set(c_cc_pow),						bmts=returns_ccq_ccq_qqq,	d(disc_cc_pow_i);					return;
//			case M_ASSIGN_DIVIDE:
//			case M_DIVIDE:				function.set(c_rc_divide),					bmts=returns_rcq_ccq_qqq,	d(disc_rc_divide_i);				return;
//			case M_LOGIC_DIVIDES:		function.set(r_rc_logic_divides),			bmts=returns_rcq_ccq_qqq,	d();	return;
//			case M_ASSIGN_MINUS:
//			case M_MINUS:				function.set(c_rc_minus),					bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_PENTATE:				function.set();																								return;
//			case M_TETRATE:				function.set(c_rc_tetrate),					bmts=returns_ccX_ccX_qXX,	d(disc_rc_tetrate_i);				return;
//			case M_ASSIGN_MULTIPLY:
//			case M_MULTIPLY:			function.set(c_rc_multiply),				bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_ASSIGN_PLUS:
//			case M_PLUS:				function.set(c_rc_plus),					bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_LOGIC_AND:			function.set(r_rc_logic_and),				bmts=returns_rrr_rrr_rrr,	d(disc_rc_logic_and_i);				return;
//			case M_LOGIC_XOR:			function.set(r_rc_logic_xor),				bmts=returns_rrr_rrr_rrr,	d(disc_rc_logic_xor_i);				return;
//			case M_LOGIC_OR:			function.set(r_rc_logic_or),				bmts=returns_rrr_rrr_rrr,	d(disc_rc_logic_or_i);				return;
//			case M_CONDITION_ZERO:		function.set(c_rc_condition_zero),	bmts=returns_rcq_ccq_qqq,	d(disc_rc_condition_zero_i);	return;
//			case M_ASSIGN_MOD:
//			case M_MODULO_PERCENT:		function.set(c_rc_modulo),					bmts=returns_rcq_ccq_qqq,	d(disc_rc_modulo_i);				return;
//			case M_ASSIGN_LEFT:
//			case M_BITWISE_SHIFT_LEFT:	function.set(c_rc_bitwise_shift_left),		bmts=returns_rcq_ccq_qqq,	d(disc_rc_bitwise_shift_left_i);	return;
//			case M_ASSIGN_RIGHT:
//			case M_BITWISE_SHIFT_RIGHT:	function.set(c_rc_bitwise_shift_right),		bmts=returns_rcq_ccq_qqq,	d(disc_rq_bitwise_shift_right_i);	return;
//			case M_LOGIC_LESS:			function.set(r_rc_logic_less),				bmts=returns_rrr_rrr_rrr,	d(disc_rc_logic_less_i);			return;
//			case M_LOGIC_LESS_EQUAL:	function.set(r_rc_logic_less_equal),		bmts=returns_rrr_rrr_rrr,	d(disc_rc_logic_equal_i);			return;
//			case M_LOGIC_GREATER:		function.set(r_rc_logic_greater),			bmts=returns_rrr_rrr_rrr,	d(disc_rc_logic_greater_i);			return;
//			case M_LOGIC_GREATER_EQUAL:	function.set(r_rc_logic_greater_equal),		bmts=returns_rrr_rrr_rrr,	d(disc_rc_logic_greater_equal_i);	return;
//			case M_LOGIC_EQUAL:			function.set(r_rc_logic_equal),				bmts=returns_rrr_rrr_rrr,	d(disc_rc_logic_equal_i);			return;
//			case M_LOGIC_NOT_EQUAL:		function.set(r_rc_logic_not_equal),			bmts=returns_rrr_rrr_rrr,	d(disc_rc_logic_not_equal_i);		return;
//			case M_ASSIGN_AND:
//			case M_BITWISE_AND:			function.set(c_rc_bitwise_and),				bmts=returns_rcq_ccq_qqq,	d(disc_rc_bitwise_and_o, false);	return;
//			case M_BITWISE_NAND:		function.set(c_rc_bitwise_nand),			bmts=returns_rcq_ccq_qqq,	d(disc_rc_bitwise_nand_o, false);	return;
//			case M_ASSIGN_XOR:
//			case M_BITWISE_XOR:			function.set(c_rc_bitwise_xor),				bmts=returns_rcq_ccq_qqq,	d(disc_rc_bitwise_xor_o, false);	return;
//			case M_BITWISE_XNOR:		function.set(c_rc_bitwise_xnor),			bmts=returns_rcq_ccq_qqq,	d(disc_rc_bitwise_xnor_o, false);	return;
//			case M_ASSIGN_OR:
//			case M_VERTICAL_BAR:		function.set(c_rc_bitwise_or),				bmts=returns_rcq_ccq_qqq,	d(disc_rc_bitwise_or_o, false);		return;
//			case M_BITWISE_NOR:			function.set(c_rc_bitwise_nor),				bmts=returns_rcq_ccq_qqq,	d(disc_rc_bitwise_nor_o, false);	return;
//			case M_LOG:					function.set(c_cc_log),						bmts=returns_ccq_ccq_qqq,	d(disc_cc_log_i);					return;
//			case M_RAND:				function.set(c_cc_random),					bmts=returns_rcq_ccq_qqq,	d(disc_cc_random_o, false);			return;
//		//	case M_RAND:				function.set(c_rc_random),					bmts=returns_rcq_ccq_qqq,	d(disc_rc_random_o, false);			return;
//			case M_ATAN:				function.set(c_rc_atan),					bmts=returns_rcq_ccq_qqq,	d(disc_rc_atan_i);					return;
//			case M_SQWV:				function.set(r_rc_sqwv),					bmts=returns_rcq_ccq_qqq,	d(disc_rc_sqwv_i);					return;
//		//	case M_TRWV:				function.set(c_rc_trwv),					bmts=returns_rcq_ccq_qqq,	d(disc_rc_trwv_i);					return;
//			case M_TRWV:				function.set(c_cc_trwv),					bmts=returns_rcq_ccq_qqq,	d(disc_rc_trwv_i);					return;
//			case M_SAW:					function.set(c_rc_saw),						bmts=returns_rcq_ccq_qqq,	d(disc_rc_saw_i);					return;
//		//	case M_MIN:					function.set(c_rc_min),						bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_MIN:					function.set(c_cc_min),						bmts=returns_rcq_ccq_qqq,	d();								return;
//		//	case M_MAX:					function.set(c_rc_max),						bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_MAX:					function.set(c_cc_max),						bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_BETA:				function.set();																								return;
//			case M_GAMMA:				function.set();																								return;
//			case M_PERMUTATION:			function.set(c_cc_permutation),				bmts=returns_rcq_ccq_qqq,	d(disc_rc_permutation_i);			return;
//		//	case M_PERMUTATION:			function.set(c_rc_permutation),				bmts=returns_rcq_ccq_qqq,	d(disc_rc_permutation_i);			return;
//		//	case M_COMBINATION:			function.set(c_rc_combination),				bmts=returns_rcq_ccq_qqq,	d(disc_rc_combination_i);			return;
//			case M_COMBINATION:			function.set(c_cc_combination),				bmts=returns_rcq_ccq_qqq,	d(disc_rc_combination_i);			return;
//			case M_BESSEL_J:				function.set();																								return;
//			case M_BESSEL_Y:				function.set();																								return;
//			case M_HANKEL1:				function.set();																								return;
//			case M_ASSIGN:				function.set(c_c_assign),					bmts=returns_rcq_rcq_rcq,	d();								return;
			}
			break;
		case 'h':
			switch(f)
			{
			CASE_Q_CQ(POWER,			pow, POW,		I)
			case M_ASSIGN_DIVIDE:
			CASE(Q_RQ, DIVIDE,			divide,			I)
			CASE(R_RQ, LOGIC_DIVIDES,	logic_divides,	C)
			case M_ASSIGN_MINUS:
			CASE(Q_RQ, MINUS,			minus,			C)
			CASE_NONE(PENTATE)
			CASE_NONE(TETRATE)
			case M_ASSIGN_MULTIPLY:
			CASE(Q_RQ, MULTIPLY,		multiply,		C)
			case M_ASSIGN_PLUS:
			CASE(Q_RQ, PLUS,			plus,			C)
			CASE(R_RQ, LOGIC_AND,		logic_and,		O)
			CASE(R_RQ, LOGIC_XOR,		logic_xor,		O)
			CASE(R_RQ, LOGIC_OR,		logic_or,		O)
			CASE(Q_RQ, CONDITION_ZERO,	condition_zero,	I)
			case M_ASSIGN_MOD:
			CASE_Q_RQ(MODULO_PERCENT,	modulo, MODULO,	I)
			case M_ASSIGN_LEFT:
			CASE(Q_RQ, BITWISE_SHIFT_LEFT, bitwise_shift_left, I)
			case M_ASSIGN_RIGHT:
			CASE(Q_RQ, BITWISE_SHIFT_RIGHT, bitwise_shift_right, I)
			CASE(R_RQ, LOGIC_LESS,		logic_less,		O)
			CASE(R_RQ, LOGIC_LESS_EQUAL, logic_less_equal, O)
			CASE(R_RQ, LOGIC_GREATER,	logic_greater,	O)
			CASE(R_RQ, LOGIC_GREATER_EQUAL, logic_greater_equal, O)
			CASE(R_RQ, LOGIC_EQUAL,		logic_equal,	O)
			CASE(R_RQ, LOGIC_NOT_EQUAL,	logic_not_equal, O)
			case M_ASSIGN_AND:
			CASE(Q_RQ, BITWISE_AND,		bitwise_and,	O)
			CASE(Q_RQ, BITWISE_NAND,	bitwise_nand,	O)
			case M_ASSIGN_XOR:
			CASE(Q_RQ, BITWISE_XOR,		bitwise_xor,	O)
			CASE(Q_RQ, BITWISE_XNOR,	bitwise_xnor,	O)
			case M_ASSIGN_OR:
			CASE_Q_RQ(VERTICAL_BAR,		bitwise_or, BITWISE_OR, O)
			CASE(Q_RQ, BITWISE_NOR,		bitwise_nor,	O)
			CASE(Q_CQ, LOG,				log,			I)
			CASE_Q_QQ(RAND,				random, RANDOM,	O)
		//	CASE_Q_RQ(RAND,				random, RANDOM,	O)
			CASE(Q_RQ, ATAN,			atan,			I)
			CASE(R_RQ, SQWV,			sqwv,			O)
			CASE(R_RQ, TRWV,			trwv,			C)
		//	CASE(Q_QQ, TRWV,			trwv,			C)
			CASE(R_RQ, SAW,				saw,			I)
		//	CASE(Q_RQ, MIN,				min,			C)
			CASE(Q_QQ, MIN,				min,			C)
		//	CASE(Q_RQ, MAX,				max,			C)
			CASE(Q_QQ, MAX,				max,			C)
			CASE_NONE(BETA)
			CASE_NONE(GAMMA)
		//	CASE(Q_RQ, PERMUTATION,		permutation,	I)
			CASE(Q_QQ, PERMUTATION,		permutation,	I)
		//	CASE(Q_RQ, COMBINATION,		combination,	I)
			CASE(Q_QQ, COMBINATION,		combination,	I)
			CASE_NONE(BESSEL_J)
			CASE_NONE(BESSEL_Y)
			CASE_NONE(HANKEL1)
			CASE(Q_Q, ASSIGN,			assign,			C)

//			case M_POWER:				function.set(q_cq_pow),						bmts=returns_ccq_ccq_qqq,	d(disc_cq_pow_i);					return;
//			case M_ASSIGN_DIVIDE:
//			case M_DIVIDE:				function.set(q_rq_divide),					bmts=returns_rcq_ccq_qqq,	d(disc_rq_divide_i);				return;
//			case M_LOGIC_DIVIDES:		function.set(r_rq_logic_divides),			bmts=returns_rcq_ccq_qqq,	d();	return;
//			case M_ASSIGN_MINUS:
//			case M_MINUS:				function.set(q_rq_minus),					bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_PENTATE:				function.set();																								return;
//			case M_TETRATE:				function.set();																								return;
//			case M_ASSIGN_MULTIPLY:
//			case M_MULTIPLY:			function.set(q_rq_multiply),				bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_ASSIGN_PLUS:
//			case M_PLUS:				function.set(q_rq_plus),					bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_LOGIC_AND:			function.set(r_rq_logic_and),				bmts=returns_rrr_rrr_rrr,	d(disc_rq_logic_and_i);				return;
//			case M_LOGIC_XOR:			function.set(r_rq_logic_xor),				bmts=returns_rrr_rrr_rrr,	d(disc_rq_logic_xor_i);				return;
//			case M_LOGIC_OR:			function.set(r_rq_logic_or),				bmts=returns_rrr_rrr_rrr,	d(disc_rq_logic_or_i);				return;
//			case M_CONDITION_ZERO:		function.set(q_rq_condition_zero),	bmts=returns_rcq_ccq_qqq,	d(disc_rq_condition_zero_i);	return;
//			case M_ASSIGN_MOD:
//			case M_MODULO_PERCENT:		function.set(q_rq_modulo),					bmts=returns_rcq_ccq_qqq,	d(disc_rq_modulo_i);				return;
//			case M_ASSIGN_LEFT:
//			case M_BITWISE_SHIFT_LEFT:	function.set(q_rq_bitwise_shift_left),		bmts=returns_rcq_ccq_qqq,	d(disc_rq_bitwise_shift_left_i);	return;
//			case M_ASSIGN_RIGHT:
//			case M_BITWISE_SHIFT_RIGHT:	function.set(q_rq_bitwise_shift_right),		bmts=returns_rcq_ccq_qqq,	d(disc_rq_bitwise_shift_right_i);	return;
//			case M_LOGIC_LESS:			function.set(r_rq_logic_less),				bmts=returns_rrr_rrr_rrr,	d(disc_rq_logic_less_i);			return;
//			case M_LOGIC_LESS_EQUAL:	function.set(r_rq_logic_less_equal),		bmts=returns_rrr_rrr_rrr,	d(disc_rq_logic_less_equal_i);		return;
//			case M_LOGIC_GREATER:		function.set(r_rq_logic_greater),			bmts=returns_rrr_rrr_rrr,	d(disc_rq_logic_greater_i);			return;
//			case M_LOGIC_GREATER_EQUAL:	function.set(r_rq_logic_greater_equal),		bmts=returns_rrr_rrr_rrr,	d(disc_rq_logic_greater_equal_i);	return;
//			case M_LOGIC_EQUAL:			function.set(r_rq_logic_equal),				bmts=returns_rrr_rrr_rrr,	d(disc_rq_logic_equal_i);			return;
//			case M_LOGIC_NOT_EQUAL:		function.set(r_rq_logic_not_equal),			bmts=returns_rrr_rrr_rrr,	d(disc_rq_logic_not_equal_i);		return;
//			case M_ASSIGN_AND:
//			case M_BITWISE_AND:			function.set(q_rq_bitwise_and),				bmts=returns_rcq_ccq_qqq,	d(disc_rq_bitwise_and_o, false);	return;
//			case M_BITWISE_NAND:		function.set(q_rq_bitwise_nand),			bmts=returns_rcq_ccq_qqq,	d(disc_rq_bitwise_nand_o, false);	return;
//			case M_ASSIGN_XOR:
//			case M_BITWISE_XOR:			function.set(q_rq_bitwise_xor),				bmts=returns_rcq_ccq_qqq,	d(disc_rq_bitwise_xor_o, false);	return;
//			case M_BITWISE_XNOR:		function.set(q_rq_bitwise_xnor),			bmts=returns_rcq_ccq_qqq,	d(disc_rq_bitwise_xnor_o, false);	return;
//			case M_ASSIGN_OR:
//			case M_VERTICAL_BAR:		function.set(q_rq_bitwise_or),				bmts=returns_rcq_ccq_qqq,	d(disc_rq_bitwise_or_o, false);		return;
//			case M_BITWISE_NOR:			function.set(q_rq_bitwise_nor),				bmts=returns_rcq_ccq_qqq,	d(disc_rq_bitwise_nor_o, false);	return;
//			case M_LOG:					function.set(q_cq_log),						bmts=returns_ccq_ccq_qqq,	d(disc_cq_log_i);					return;
//			case M_RAND:				function.set(q_qq_random),					bmts=returns_rcq_ccq_qqq,	d(disc_qq_random_o, false);			return;
//		//	case M_RAND:				function.set(q_rq_random),					bmts=returns_rcq_ccq_qqq,	d(disc_rq_random_o, false);			return;
//			case M_ATAN:				function.set(q_rq_atan),					bmts=returns_rcq_ccq_qqq,	d(disc_rq_atan_i);					return;
//			case M_SQWV:				function.set(r_rq_sqwv),					bmts=returns_rcq_ccq_qqq,	d(disc_rq_sqwv_i);					return;
//		//	case M_TRWV:				function.set(q_rq_trwv),					bmts=returns_rcq_ccq_qqq,	d(disc_rq_trwv_i);					return;
//			case M_TRWV:				function.set(q_qq_trwv),					bmts=returns_rcq_ccq_qqq,	d(disc_rq_trwv_i);					return;//
//			case M_SAW:					function.set(q_rq_saw),						bmts=returns_rcq_ccq_qqq,	d(disc_rq_saw_i);					return;
//		//	case M_MIN:					function.set(q_rq_min),						bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_MIN:					function.set(q_qq_min),						bmts=returns_rcq_ccq_qqq,	d();								return;
//		//	case M_MAX:					function.set(q_rq_max),						bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_MAX:					function.set(q_qq_max),						bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_BETA:				function.set();																								return;
//			case M_GAMMA:				function.set();																								return;
//		//	case M_PERMUTATION:			function.set(q_rq_permutation),				bmts=returns_rcq_ccq_qqq,	d(disc_rq_permutation_i);			return;
//			case M_PERMUTATION:			function.set(q_qq_permutation),				bmts=returns_rcq_ccq_qqq,	d(disc_rq_permutation_i);			return;
//		//	case M_COMBINATION:			function.set(q_rq_combination),				bmts=returns_rcq_ccq_qqq,	d(disc_rq_combination_i);			return;
//			case M_COMBINATION:			function.set(q_qq_combination),				bmts=returns_rcq_ccq_qqq,	d(disc_rq_combination_i);			return;
//			case M_BESSEL_J:				function.set();																								return;
//			case M_BESSEL_Y:				function.set();																								return;
//			case M_HANKEL1:				function.set();																								return;
//			case M_ASSIGN:				function.set(q_q_assign),					bmts=returns_rcq_rcq_rcq,	d();								return;
			}
			break;
		}
		break;
	case 'c':
		switch(op2type)
		{
		case 'R':
			switch(f)
			{
			CASE_C_CR(POWER,			pow, POW,		I)
			case M_ASSIGN_DIVIDE:
			CASE(C_CR, DIVIDE,			divide,			I)
			CASE(R_CR, LOGIC_DIVIDES,	logic_divides,	C)
			case M_ASSIGN_MINUS:
			CASE(C_CR, MINUS,			minus,			C)
			CASE(C_CR, PENTATE,			pentate,		I)
			CASE(C_CR, TETRATE,			tetrate,		I)
			CASE(C_CR, POWER_REAL,		power_real,		I)
			case M_ASSIGN_MULTIPLY:
			CASE(C_CR, MULTIPLY,		multiply,		C)
			case M_ASSIGN_PLUS:
			CASE(C_CR, PLUS,			plus,			C)
			CASE(R_CR, LOGIC_AND,		logic_and,		O)
			CASE(R_CR, LOGIC_XOR,		logic_xor,		O)
			CASE(R_CR, LOGIC_OR,		logic_or,		O)
			CASE(C_CR, CONDITION_ZERO,	condition_zero,	I)
			case M_ASSIGN_MOD:
			CASE_C_CR(MODULO_PERCENT,	modulo, MODULO,	I)
			case M_ASSIGN_LEFT:
			CASE(C_CR, BITWISE_SHIFT_LEFT, bitwise_shift_left, I)
			case M_ASSIGN_RIGHT:
			CASE(C_CR, BITWISE_SHIFT_RIGHT, bitwise_shift_right, I)
			CASE(R_CR, LOGIC_LESS,		logic_less,		O)
			CASE(R_CR, LOGIC_LESS_EQUAL, logic_less_equal, O)
			CASE(R_CR, LOGIC_GREATER,	logic_greater,	O)
			CASE(R_CR, LOGIC_GREATER_EQUAL, logic_greater_equal, O)
			CASE(R_CR, LOGIC_EQUAL,		logic_equal,	O)
			CASE(R_CR, LOGIC_NOT_EQUAL,	logic_not_equal, O)
			case M_ASSIGN_AND:
			CASE(C_CR, BITWISE_AND,		bitwise_and,	O)
			CASE(C_CR, BITWISE_NAND,	bitwise_nand,	O)
			case M_ASSIGN_XOR:
			CASE(C_CR, BITWISE_XOR,		bitwise_xor,	O)
			CASE(C_CR, BITWISE_XNOR,	bitwise_xnor,	O)
			case M_ASSIGN_OR:
			CASE_C_CR(VERTICAL_BAR,		bitwise_or, BITWISE_OR, O)
			CASE(C_CR, BITWISE_NOR,		bitwise_nor,	O)
			CASE(C_CR, LOG,				log,			I)
			CASE_C_CR(RAND,				random, RANDOM,	O)
			CASE(C_CR, ATAN,			atan,			I)
			CASE(R_CR, SQWV,			sqwv,			O)
			CASE(R_CR, TRWV,			trwv,			C)
			CASE(R_CR, SAW,				saw,			I)
			CASE(R_CR, MANDELBROT,		mandelbrot,		C)
			CASE(C_CR, MIN,				min,			C)
			CASE(C_CR, MAX,				max,			C)
			CASE_NONE(BETA)
			CASE_NONE(GAMMA)
			CASE(C_CR, PERMUTATION,		permutation,	I)
			CASE(C_CR, COMBINATION,		combination,	I)
			CASE_NONE(BESSEL_J)
			CASE_NONE(BESSEL_Y)
			CASE_NONE(HANKEL1)
			CASE(R_R, ASSIGN,			assign,			C)

//			case M_POWER:				function.set(c_cr_pow),						bmts=returns_ccq_ccq_qqq,	d(disc_cr_pow_i);					return;
//			case M_ASSIGN_DIVIDE:
//			case M_DIVIDE:				function.set(c_cr_divide),					bmts=returns_rcq_ccq_qqq,	d(disc_cr_divide_i);				return;
//			case M_LOGIC_DIVIDES:		function.set(r_cr_logic_divides),			bmts=returns_rcq_ccq_qqq,	d();	return;
//			case M_ASSIGN_MINUS:
//			case M_MINUS:				function.set(c_cr_minus),					bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_PENTATE:				function.set(c_cr_pentate);					bmts=returns_rXX_rXX_XXX,	d(disc_cr_pentate_i);				return;
//			case M_TETRATE:				function.set(c_cr_tetrate),					bmts=returns_ccX_ccX_qXX,	d(disc_cr_tetrate_i);				return;
//			case M_POWER_REAL:			function.set(c_cr_power_real),				bmts=returns_rXX_cXX_qXX,	d(disc_cr_power_real_i);			return;
//			case M_ASSIGN_MULTIPLY:
//			case M_MULTIPLY:			function.set(c_cr_multiply),				bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_ASSIGN_PLUS:
//			case M_PLUS:				function.set(c_cr_plus),					bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_LOGIC_AND:			function.set(r_cr_logic_and),				bmts=returns_rrr_rrr_rrr,	d(disc_cr_logic_and_i);				return;
//			case M_LOGIC_XOR:			function.set(r_cr_logic_xor),				bmts=returns_rrr_rrr_rrr,	d(disc_cr_logic_xor_i);				return;
//			case M_LOGIC_OR:			function.set(r_cr_logic_or),				bmts=returns_rrr_rrr_rrr,	d(disc_cr_logic_or_i);				return;
//			case M_CONDITION_ZERO:		function.set(c_cr_condition_zero),	bmts=returns_rcq_ccq_qqq,	d(disc_cr_condition_zero_i);	return;
//			case M_ASSIGN_MOD:
//			case M_MODULO_PERCENT:		function.set(c_cr_modulo),					bmts=returns_rcq_ccq_qqq,	d(disc_cr_modulo_i);				return;
//			case M_ASSIGN_LEFT:
//			case M_BITWISE_SHIFT_LEFT:	function.set(c_cr_bitwise_shift_left),		bmts=returns_rcq_ccq_qqq,	d(disc_cr_bitwise_shift_left_i);	return;
//			case M_ASSIGN_RIGHT:
//			case M_BITWISE_SHIFT_RIGHT:	function.set(c_cr_bitwise_shift_right),		bmts=returns_rcq_ccq_qqq,	d(disc_cr_bitwise_shift_right_i);	return;
//			case M_LOGIC_LESS:			function.set(r_cr_logic_less),				bmts=returns_rrr_rrr_rrr,	d(disc_cr_logic_less_i);			return;
//			case M_LOGIC_LESS_EQUAL:	function.set(r_cr_logic_less_equal),		bmts=returns_rrr_rrr_rrr,	d(disc_cr_logic_less_equal_i);		return;
//			case M_LOGIC_GREATER:		function.set(r_cr_logic_greater),			bmts=returns_rrr_rrr_rrr,	d(disc_cr_logic_greater_i);			return;
//			case M_LOGIC_GREATER_EQUAL:	function.set(r_cr_logic_greater_equal),		bmts=returns_rrr_rrr_rrr,	d(disc_cr_logic_greater_equal_i);	return;
//			case M_LOGIC_EQUAL:			function.set(r_cr_logic_equal),				bmts=returns_rrr_rrr_rrr,	d(disc_cr_logic_equal_i);			return;
//			case M_LOGIC_NOT_EQUAL:		function.set(r_cr_logic_not_equal),			bmts=returns_rrr_rrr_rrr,	d(disc_cr_logic_not_equal_i);		return;
//			case M_ASSIGN_AND:
//			case M_BITWISE_AND:			function.set(c_cr_bitwise_and),				bmts=returns_rcq_ccq_qqq,	d(disc_cr_bitwise_and_o, false);	return;
//			case M_BITWISE_NAND:		function.set(c_cr_bitwise_nand),			bmts=returns_rcq_ccq_qqq,	d(disc_cr_bitwise_nand_o, false);	return;
//			case M_ASSIGN_XOR:
//			case M_BITWISE_XOR:			function.set(c_cr_bitwise_xor),				bmts=returns_rcq_ccq_qqq,	d(disc_cr_bitwise_xor_o, false);	return;
//			case M_BITWISE_XNOR:		function.set(c_cr_bitwise_xnor),			bmts=returns_rcq_ccq_qqq,	d(disc_cr_bitwise_xnor_o, false);	return;
//			case M_ASSIGN_OR:
//			case M_VERTICAL_BAR:		function.set(c_cr_bitwise_or),				bmts=returns_rcq_ccq_qqq,	d(disc_cr_bitwise_or_o, false);		return;
//			case M_BITWISE_NOR:			function.set(c_cr_bitwise_nor),				bmts=returns_rcq_ccq_qqq,	d(disc_cr_bitwise_nor_o, false);	return;
//			case M_LOG:					function.set(c_cr_log),						bmts=returns_ccq_ccq_qqq,	d(disc_cr_log_i);					return;
//			case M_RAND:				function.set(c_cr_random),					bmts=returns_rcq_ccq_qqq,	d(disc_cr_random_o, false);			return;
//			case M_ATAN:				function.set(c_cr_atan),					bmts=returns_rcq_ccq_qqq,	d(disc_cr_atan_i);					return;
//			case M_SQWV:				function.set(r_cr_sqwv),					bmts=returns_rcq_ccq_qqq,	d(disc_cr_sqwv_i);					return;
//			case M_TRWV:				function.set(c_cr_trwv),					bmts=returns_rcq_ccq_qqq,	d(disc_cr_trwv_i);					return;
//			case M_SAW:					function.set(c_cr_saw),						bmts=returns_rcq_ccq_qqq,	d(disc_cr_saw_i);					return;
//			case M_MANDELBROT:			function.set(r_cr_mandelbrot),				bmts=returns_rrr_rrr_rrr,	d();								return;
//			case M_MIN:					function.set(c_cr_min),						bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_MAX:					function.set(c_cr_max),						bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_BETA:				function.set();																								return;
//			case M_GAMMA:				function.set();																								return;
//			case M_PERMUTATION:			function.set(c_cr_permutation),				bmts=returns_rcq_ccq_qqq,	d(disc_cr_permutation_i);			return;
//			case M_COMBINATION:			function.set(c_cr_combination),				bmts=returns_rcq_ccq_qqq,	d(disc_cr_combination_i);			return;
//			case M_BESSEL_J:				function.set();																								return;
//			case M_BESSEL_Y:				function.set();																								return;
//			case M_HANKEL1:				function.set();																								return;
//			case M_ASSIGN:				function.set(r_r_assign),					bmts=returns_rcq_rcq_rcq,	d();								return;
			}
			break;
		case 'c':
			switch(f)
			{
			CASE_C_CC(POWER,			pow, POW,		I)
			case M_ASSIGN_DIVIDE:
			CASE(C_CC, DIVIDE,			divide,			I)
			CASE(R_CC, LOGIC_DIVIDES,	logic_divides,	C)
			case M_ASSIGN_MINUS:
			CASE(C_CC, MINUS,			minus,			C)
			CASE_NONE(PENTATE)
			CASE(C_CC, TETRATE,			tetrate,		I)
			case M_ASSIGN_MULTIPLY:
			CASE(C_CC, MULTIPLY,		multiply,		C)
			case M_ASSIGN_PLUS:
			CASE(C_CC, PLUS,			plus,			C)
			CASE(R_CC, LOGIC_AND,		logic_and,		O)
			CASE(R_CC, LOGIC_XOR,		logic_xor,		O)
			CASE(R_CC, LOGIC_OR,		logic_or,		O)
			CASE(C_CC, CONDITION_ZERO,	condition_zero,	I)
			case M_ASSIGN_MOD:
			CASE_C_CC(MODULO_PERCENT,	modulo, MODULO,	I)
			case M_ASSIGN_LEFT:
			CASE(C_CC, BITWISE_SHIFT_LEFT, bitwise_shift_left, I)
			case M_ASSIGN_RIGHT:
			CASE(C_CC, BITWISE_SHIFT_RIGHT, bitwise_shift_right, I)
			CASE(R_CC, LOGIC_LESS,		logic_less,		O)
			CASE(R_CC, LOGIC_LESS_EQUAL, logic_less_equal, O)
			CASE(R_CC, LOGIC_GREATER,	logic_greater,	O)
			CASE(R_CC, LOGIC_GREATER_EQUAL, logic_greater_equal, O)
			CASE(R_CC, LOGIC_EQUAL,		logic_equal,	O)
			CASE(R_CC, LOGIC_NOT_EQUAL,	logic_not_equal, O)
			case M_ASSIGN_AND:
			CASE(C_CC, BITWISE_AND,		bitwise_and,	O)
			CASE(C_CC, BITWISE_NAND,	bitwise_nand,	O)
			case M_ASSIGN_XOR:
			CASE(C_CC, BITWISE_XOR,		bitwise_xor,	O)
			CASE(C_CC, BITWISE_XNOR,	bitwise_xnor,	O)
			case M_ASSIGN_OR:
			CASE_C_CC(VERTICAL_BAR,		bitwise_or, BITWISE_OR, O)
			CASE(C_CC, BITWISE_NOR,		bitwise_nor,	O)
			CASE(C_CC, LOG,				log,			I)
			CASE_C_CC(RAND,				random, RANDOM,	O)
			CASE(C_CC, ATAN,			atan,			I)
			CASE(R_CC, SQWV,			sqwv,			O)
			CASE(R_CC, TRWV,			trwv,			C)
			CASE(R_CC, SAW,				saw,			I)
			CASE(C_CC, MIN,				min,			C)
			CASE(C_CC, MAX,				max,			C)
			CASE_NONE(BETA)
			CASE_NONE(GAMMA)
			CASE(C_CC, PERMUTATION,		permutation,	I)
			CASE(C_CC, COMBINATION,		combination,	I)
			CASE_NONE(BESSEL_J)
			CASE_NONE(BESSEL_Y)
			CASE_NONE(HANKEL1)
			CASE(C_C, ASSIGN,			assign,			C)

//			case M_POWER:				function.set(c_cc_pow),						bmts=returns_ccq_ccq_qqq,	d(disc_cc_pow_i);					return;
//			case M_ASSIGN_DIVIDE:
//			case M_DIVIDE:				function.set(c_cc_divide),					bmts=returns_rcq_ccq_qqq,	d(disc_cc_divide_i);				return;
//			case M_LOGIC_DIVIDES:		function.set(r_cc_logic_divides),			bmts=returns_rcq_ccq_qqq,	d();	return;
//			case M_ASSIGN_MINUS:
//			case M_MINUS:				function.set(c_cc_minus),					bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_PENTATE:				function.set();																								return;
//			case M_TETRATE:				function.set(c_cc_tetrate),					bmts=returns_ccX_ccX_qXX,	d(disc_cc_tetrate_i);				return;
//			case M_ASSIGN_MULTIPLY:
//			case M_MULTIPLY:			function.set(c_cc_multiply),				bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_ASSIGN_PLUS:
//			case M_PLUS:				function.set(c_cc_plus),					bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_LOGIC_AND:			function.set(r_cc_logic_and),				bmts=returns_rrr_rrr_rrr,	d(disc_cc_logic_and_i);				return;
//			case M_LOGIC_XOR:			function.set(r_cc_logic_xor),				bmts=returns_rrr_rrr_rrr,	d(disc_cc_logic_xor_i);				return;
//			case M_LOGIC_OR:			function.set(r_cc_logic_or),				bmts=returns_rrr_rrr_rrr,	d(disc_cc_logic_or_i);				return;
//			case M_CONDITION_ZERO:		function.set(c_cc_condition_zero),	bmts=returns_rcq_ccq_qqq,	d(disc_cc_condition_zero_i);	return;
//			case M_ASSIGN_MOD:
//			case M_MODULO_PERCENT:		function.set(c_cc_modulo),					bmts=returns_rcq_ccq_qqq,	d(disc_cc_modulo_i);				return;
//			case M_ASSIGN_LEFT:
//			case M_BITWISE_SHIFT_LEFT:	function.set(c_cc_bitwise_shift_left),		bmts=returns_rcq_ccq_qqq,	d(disc_cc_bitwise_shift_left_i);	return;
//			case M_ASSIGN_RIGHT:
//			case M_BITWISE_SHIFT_RIGHT:	function.set(c_cc_bitwise_shift_right),		bmts=returns_rcq_ccq_qqq,	d(disc_cc_bitwise_shift_right_i);	return;
//			case M_LOGIC_LESS:			function.set(r_cc_logic_less),				bmts=returns_rrr_rrr_rrr,	d(disc_cc_logic_less_i);			return;
//			case M_LOGIC_LESS_EQUAL:	function.set(r_cc_logic_less_equal),		bmts=returns_rrr_rrr_rrr,	d(disc_cc_logic_less_equal_i);		return;
//			case M_LOGIC_GREATER:		function.set(r_cc_logic_greater),			bmts=returns_rrr_rrr_rrr,	d(disc_cc_logic_greater_i);			return;
//			case M_LOGIC_GREATER_EQUAL:	function.set(r_cc_logic_greater_equal),		bmts=returns_rrr_rrr_rrr,	d(disc_cc_logic_greater_equal_i);	return;
//			case M_LOGIC_EQUAL:			function.set(r_cc_logic_equal),				bmts=returns_rrr_rrr_rrr,	d(disc_cc_logic_equal_i);			return;
//			case M_LOGIC_NOT_EQUAL:		function.set(r_cc_logic_not_equal),			bmts=returns_rrr_rrr_rrr,	d(disc_cc_logic_not_equal_i);		return;
//			case M_ASSIGN_AND:
//			case M_BITWISE_AND:			function.set(c_cc_bitwise_and),				bmts=returns_rcq_ccq_qqq,	d(disc_cc_bitwise_and_o, false);	return;
//			case M_BITWISE_NAND:		function.set(c_cc_bitwise_nand),			bmts=returns_rcq_ccq_qqq,	d(disc_cc_bitwise_nand_o, false);	return;
//			case M_ASSIGN_XOR:
//			case M_BITWISE_XOR:			function.set(c_cc_bitwise_xor),				bmts=returns_rcq_ccq_qqq,	d(disc_cc_bitwise_xor_o, false);	return;
//			case M_BITWISE_XNOR:		function.set(c_cc_bitwise_xnor),			bmts=returns_rcq_ccq_qqq,	d(disc_cc_bitwise_xnor_o, false);	return;
//			case M_ASSIGN_OR:
//			case M_VERTICAL_BAR:		function.set(c_cc_bitwise_or),				bmts=returns_rcq_ccq_qqq,	d(disc_cc_bitwise_or_o, false);		return;
//			case M_BITWISE_NOR:			function.set(c_cc_bitwise_nor),				bmts=returns_rcq_ccq_qqq,	d(disc_cc_bitwise_nor_o, false);	return;
//			case M_LOG:					function.set(c_cc_log),						bmts=returns_ccq_ccq_qqq,	d(disc_cc_log_i);					return;
//			case M_RAND:				function.set(c_cc_random),					bmts=returns_rcq_ccq_qqq,	d(disc_cc_random_o, false);			return;
//			case M_ATAN:				function.set(c_cc_atan),					bmts=returns_rcq_ccq_qqq,	d(disc_cc_atan_i);					return;
//			case M_SQWV:				function.set(r_cc_sqwv),					bmts=returns_rcq_ccq_qqq,	d(disc_cc_sqwv_i);					return;
//			case M_TRWV:				function.set(c_cc_trwv),					bmts=returns_rcq_ccq_qqq,	d(disc_cc_trwv_i);					return;
//			case M_SAW:					function.set(c_cc_saw),						bmts=returns_rcq_ccq_qqq,	d(disc_cc_saw_i);					return;
//			case M_MIN:					function.set(c_cc_min),						bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_MAX:					function.set(c_cc_max),						bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_BETA:				function.set();																								return;
//			case M_GAMMA:				function.set();																								return;
//			case M_PERMUTATION:			function.set(c_cc_permutation),				bmts=returns_rcq_ccq_qqq,	d(disc_cc_permutation_i);			return;
//			case M_COMBINATION:			function.set(c_cc_combination),				bmts=returns_rcq_ccq_qqq,	d(disc_cc_combination_i);			return;
//			case M_BESSEL_J:				function.set();																								return;
//			case M_BESSEL_Y:				function.set();																								return;
//			case M_HANKEL1:				function.set();																								return;
//			case M_ASSIGN:				function.set(c_c_assign),					bmts=returns_rcq_rcq_rcq,	d();								return;
			}
			break;
		case 'h':
			switch(f)
			{
			CASE_Q_CQ(POWER,			pow, POW,		I)
			case M_ASSIGN_DIVIDE:
			CASE(Q_CQ, DIVIDE,			divide,			I)
			CASE(R_CQ, LOGIC_DIVIDES,	logic_divides,	C)
			case M_ASSIGN_MINUS:
			CASE(Q_CQ, MINUS,			minus,			C)
			CASE_NONE(PENTATE)
			CASE_NONE(TETRATE)
			case M_ASSIGN_MULTIPLY:
			CASE(Q_CQ, MULTIPLY,		multiply,		C)
			case M_ASSIGN_PLUS:
			CASE(Q_CQ, PLUS,			plus,			C)
			CASE(R_CQ, LOGIC_AND,		logic_and,		O)
			CASE(R_CQ, LOGIC_XOR,		logic_xor,		O)
			CASE(R_CQ, LOGIC_OR,		logic_or,		O)
			CASE(Q_CQ, CONDITION_ZERO,	condition_zero,	I)
			case M_ASSIGN_MOD:
			CASE_Q_CQ(MODULO_PERCENT,	modulo, MODULO,	I)
			case M_ASSIGN_LEFT:
			CASE(Q_CQ, BITWISE_SHIFT_LEFT, bitwise_shift_left, I)
			case M_ASSIGN_RIGHT:
			CASE(Q_CQ, BITWISE_SHIFT_RIGHT, bitwise_shift_right, I)
			CASE(R_CQ, LOGIC_LESS,		logic_less,		O)
			CASE(R_CQ, LOGIC_LESS_EQUAL, logic_less_equal, O)
			CASE(R_CQ, LOGIC_GREATER,	logic_greater,	O)
			CASE(R_CQ, LOGIC_GREATER_EQUAL, logic_greater_equal, O)
			CASE(R_CQ, LOGIC_EQUAL,		logic_equal,	O)
			CASE(R_CQ, LOGIC_NOT_EQUAL,	logic_not_equal, O)
			case M_ASSIGN_AND:
			CASE(Q_CQ, BITWISE_AND,		bitwise_and,	O)
			CASE(Q_CQ, BITWISE_NAND,	bitwise_nand,	O)
			case M_ASSIGN_XOR:
			CASE(Q_CQ, BITWISE_XOR,		bitwise_xor,	O)
			CASE(Q_CQ, BITWISE_XNOR,	bitwise_xnor,	O)
			case M_ASSIGN_OR:
			CASE_Q_CQ(VERTICAL_BAR,		bitwise_or, BITWISE_OR, O)
			CASE(Q_CQ, BITWISE_NOR,		bitwise_nor,	O)
			CASE(Q_CQ, LOG,				log,			I)
			CASE_Q_QQ(RAND,				random, RANDOM,	O)
		//	CASE_Q_CQ(RAND,				random, RANDOM,	O)
			CASE(Q_CQ, ATAN,			atan,			I)
			CASE(R_CQ, SQWV,			sqwv,			O)
			CASE(R_CQ, TRWV,			trwv,			C)
		//	CASE(R_QQ, TRWV,			trwv,			C)
			CASE(R_CQ, SAW,				saw,			I)
		//	CASE(Q_CQ, MIN,				min,			C)
			CASE(Q_QQ, MIN,				min,			C)
		//	CASE(Q_CQ, MAX,				max,			C)
			CASE(Q_QQ, MAX,				max,			C)
			CASE_NONE(BETA)
			CASE_NONE(GAMMA)
		//	CASE(Q_CQ, PERMUTATION,		permutation,	I)
			CASE(Q_QQ, PERMUTATION,		permutation,	I)
		//	CASE(Q_CQ, COMBINATION,		combination,	I)
			CASE(Q_QQ, COMBINATION,		combination,	I)
			CASE_NONE(BESSEL_J)
			CASE_NONE(BESSEL_Y)
			CASE_NONE(HANKEL1)
			CASE(Q_Q, ASSIGN,			assign,			C)

//			case M_POWER:				function.set(q_cq_pow),						bmts=returns_ccq_ccq_qqq,	d(disc_cq_pow_i);					return;
//			case M_ASSIGN_DIVIDE:
//			case M_DIVIDE:				function.set(q_cq_divide),					bmts=returns_rcq_ccq_qqq,	d(disc_cq_divide_i);				return;
//			case M_LOGIC_DIVIDES:		function.set(r_cq_logic_divides),			bmts=returns_rcq_ccq_qqq,	d();	return;
//			case M_ASSIGN_MINUS:
//			case M_MINUS:				function.set(q_cq_minus),					bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_PENTATE:				function.set();																								return;
//			case M_TETRATE:				function.set();																								return;
//			case M_ASSIGN_MULTIPLY:
//			case M_MULTIPLY:			function.set(q_cq_multiply),				bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_ASSIGN_PLUS:
//			case M_PLUS:				function.set(q_cq_plus),					bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_LOGIC_AND:			function.set(r_cq_logic_and),				bmts=returns_rrr_rrr_rrr,	d(disc_cq_logic_and_i);				return;
//			case M_LOGIC_XOR:			function.set(r_cq_logic_xor),				bmts=returns_rrr_rrr_rrr,	d(disc_cq_logic_xor_i);				return;
//			case M_LOGIC_OR:			function.set(r_cq_logic_or),				bmts=returns_rrr_rrr_rrr,	d(disc_cq_logic_or_i);				return;
//			case M_CONDITION_ZERO:		function.set(q_cq_condition_zero),	bmts=returns_rcq_ccq_qqq,	d(disc_cq_condition_zero_i);	return;
//			case M_ASSIGN_MOD:
//			case M_MODULO_PERCENT:		function.set(q_cq_modulo),					bmts=returns_rcq_ccq_qqq,	d(disc_cq_modulo_i);				return;
//			case M_ASSIGN_LEFT:
//			case M_BITWISE_SHIFT_LEFT:	function.set(q_cq_bitwise_shift_left),		bmts=returns_rcq_ccq_qqq,	d(disc_cq_bitwise_shift_left_i);	return;
//			case M_ASSIGN_RIGHT:
//			case M_BITWISE_SHIFT_RIGHT:	function.set(q_cq_bitwise_shift_right),		bmts=returns_rcq_ccq_qqq,	d(disc_cq_bitwise_shift_right_i);	return;
//			case M_LOGIC_LESS:			function.set(r_cq_logic_less),				bmts=returns_rrr_rrr_rrr,	d(disc_cq_logic_less_i);			return;
//			case M_LOGIC_LESS_EQUAL:	function.set(r_cq_logic_less_equal),		bmts=returns_rrr_rrr_rrr,	d(disc_cq_logic_less_equal_i);		return;
//			case M_LOGIC_GREATER:		function.set(r_cq_logic_greater),			bmts=returns_rrr_rrr_rrr,	d(disc_cq_logic_greater_i);			return;
//			case M_LOGIC_GREATER_EQUAL:	function.set(r_cq_logic_greater_equal),		bmts=returns_rrr_rrr_rrr,	d(disc_cq_logic_greater_equal_i);	return;
//			case M_LOGIC_EQUAL:			function.set(r_cq_logic_equal),				bmts=returns_rrr_rrr_rrr,	d(disc_cq_logic_equal_i);			return;
//			case M_LOGIC_NOT_EQUAL:		function.set(r_cq_logic_not_equal),			bmts=returns_rrr_rrr_rrr,	d(disc_cq_logic_not_equal_i);		return;
//			case M_ASSIGN_AND:
//			case M_BITWISE_AND:			function.set(q_cq_bitwise_and),				bmts=returns_rcq_ccq_qqq,	d(disc_cq_bitwise_and_o, false);	return;
//			case M_BITWISE_NAND:		function.set(q_cq_bitwise_nand),			bmts=returns_rcq_ccq_qqq,	d(disc_cq_bitwise_nand_o, false);	return;
//			case M_ASSIGN_XOR:
//			case M_BITWISE_XOR:			function.set(q_cq_bitwise_xor),				bmts=returns_rcq_ccq_qqq,	d(disc_cq_bitwise_xor_o, false);	return;
//			case M_BITWISE_XNOR:		function.set(q_cq_bitwise_xnor),			bmts=returns_rcq_ccq_qqq,	d(disc_cq_bitwise_xnor_o, false);	return;
//			case M_ASSIGN_OR:
//			case M_VERTICAL_BAR:		function.set(q_cq_bitwise_or),				bmts=returns_rcq_ccq_qqq,	d(disc_cq_bitwise_or_o, false);		return;
//			case M_BITWISE_NOR:			function.set(q_cq_bitwise_nor),				bmts=returns_rcq_ccq_qqq,	d(disc_cq_bitwise_nor_o, false);	return;
//			case M_LOG:					function.set(q_cq_log),						bmts=returns_ccq_ccq_qqq,	d(disc_cq_log_i);					return;
//			case M_RAND:				function.set(q_qq_random),					bmts=returns_rcq_ccq_qqq,	d(disc_qq_random_o, false);			return;
//		//	case M_RAND:				function.set(q_cq_random),					bmts=returns_rcq_ccq_qqq,	d(disc_cq_random_o, false);			return;
//			case M_ATAN:				function.set(q_cq_atan),					bmts=returns_rcq_ccq_qqq,	d(disc_cq_atan_i);					return;
//			case M_SQWV:				function.set(r_cq_sqwv),					bmts=returns_rcq_ccq_qqq,	d(disc_cq_sqwv_i);					return;
//		//	case M_TRWV:				function.set(q_cq_trwv),					bmts=returns_rcq_ccq_qqq,	d(disc_cq_trwv_i);					return;
//			case M_TRWV:				function.set(q_qq_trwv),					bmts=returns_rcq_ccq_qqq,	d(disc_cq_trwv_i);					return;
//			case M_SAW:					function.set(q_cq_saw),						bmts=returns_rcq_ccq_qqq,	d(disc_cq_saw_i);					return;
//		//	case M_MIN:					function.set(q_cq_min),						bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_MIN:					function.set(q_qq_min),						bmts=returns_rcq_ccq_qqq,	d();								return;
//		//	case M_MAX:					function.set(q_cq_max),						bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_MAX:					function.set(q_qq_max),						bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_BETA:				function.set();																								return;
//			case M_GAMMA:				function.set();																								return;
//		//	case M_PERMUTATION:			function.set(q_cq_permutation),				bmts=returns_rcq_ccq_qqq,	d(disc_cq_permutation_i);			return;
//			case M_PERMUTATION:			function.set(q_qq_permutation),				bmts=returns_rcq_ccq_qqq,	d(disc_cq_permutation_i);			return;
//		//	case M_COMBINATION:			function.set(q_cq_combination),				bmts=returns_rcq_ccq_qqq,	d(disc_cq_combination_i);			return;
//			case M_COMBINATION:			function.set(q_qq_combination),				bmts=returns_rcq_ccq_qqq,	d(disc_cq_combination_i);			return;
//			case M_BESSEL_J:				function.set();																								return;
//			case M_BESSEL_Y:				function.set();																								return;
//			case M_HANKEL1:				function.set();																								return;
//			case M_ASSIGN:				function.set(q_q_assign),					bmts=returns_rcq_rcq_rcq,	d();								return;
			}
			break;
		}
	case 'h':
		switch(op2type)
		{
		case 'R':
			switch(f)
			{
			CASE_Q_QR(POWER,			pow, POW,		I)
			case M_ASSIGN_DIVIDE:
			CASE(Q_QR, DIVIDE,			divide,			I)
			CASE(R_QR, LOGIC_DIVIDES,	logic_divides,	C)
			case M_ASSIGN_MINUS:
			CASE(Q_QR, MINUS,			minus,			C)
			CASE_NONE(PENTATE)
			CASE(Q_QR, TETRATE,			tetrate,		I)
			CASE(Q_QR, POWER_REAL,		power_real,		I)
			case M_ASSIGN_MULTIPLY:
			CASE(Q_QR, MULTIPLY,		multiply,		C)
			case M_ASSIGN_PLUS:
			CASE(Q_QR, PLUS,			plus,			C)
			CASE(R_QR, LOGIC_AND,		logic_and,		O)
			CASE(R_QR, LOGIC_XOR,		logic_xor,		O)
			CASE(R_QR, LOGIC_OR,		logic_or,		O)
			CASE(Q_QR, CONDITION_ZERO,	condition_zero,	I)
			case M_ASSIGN_MOD:
			CASE_Q_QR(MODULO_PERCENT,	modulo, MODULO,	I)
			case M_ASSIGN_LEFT:
			CASE(Q_QR, BITWISE_SHIFT_LEFT, bitwise_shift_left, I)
			case M_ASSIGN_RIGHT:
			CASE(Q_QR, BITWISE_SHIFT_RIGHT, bitwise_shift_right, I)
			CASE(R_QR, LOGIC_LESS,		logic_less,		O)
			CASE(R_QR, LOGIC_LESS_EQUAL, logic_less_equal, O)
			CASE(R_QR, LOGIC_GREATER,	logic_greater,	O)
			CASE(R_QR, LOGIC_GREATER_EQUAL, logic_greater_equal, O)
			CASE(R_QR, LOGIC_EQUAL,		logic_equal,	O)
			CASE(R_QR, LOGIC_NOT_EQUAL,	logic_not_equal, O)
			case M_ASSIGN_AND:
			CASE(Q_QR, BITWISE_AND,		bitwise_and,	O)
			CASE(Q_QR, BITWISE_NAND,	bitwise_nand,	O)
			case M_ASSIGN_XOR:
			CASE(Q_QR, BITWISE_XOR,		bitwise_xor,	O)
			CASE(Q_QR, BITWISE_XNOR,	bitwise_xnor,	O)
			case M_ASSIGN_OR:
			CASE_Q_QR(VERTICAL_BAR,		bitwise_or, BITWISE_OR, O)
			CASE(Q_QR, BITWISE_NOR,		bitwise_nor,	O)
			CASE(Q_QC, LOG,				log,			I)
			CASE_Q_QQ(RAND,				random, RANDOM,	O)
		//	CASE_Q_QR(RAND,				random, RANDOM,	O)
			CASE(Q_QR, ATAN,			atan,			I)
			CASE(R_QR, SQWV,			sqwv,			O)
			CASE(R_QR, TRWV,			trwv,			C)
		//	CASE(R_QQ, TRWV,			trwv,			C)
			CASE(R_QR, SAW,				saw,			I)
		//	CASE(Q_QR, MIN,				min,			C)
			CASE(Q_QQ, MIN,				min,			C)
		//	CASE(Q_QR, MAX,				max,			C)
			CASE(Q_QQ, MAX,				max,			C)
			CASE_NONE(BETA)
			CASE_NONE(GAMMA)
		//	CASE(Q_QR, PERMUTATION,		permutation,	I)
			CASE(Q_QQ, PERMUTATION,		permutation,	I)
		//	CASE(Q_QR, COMBINATION,		combination,	I)
			CASE(Q_QQ, COMBINATION,		combination,	I)
			CASE_NONE(BESSEL_J)
			CASE_NONE(BESSEL_Y)
			CASE_NONE(HANKEL1)
			CASE(R_R, ASSIGN,			assign,			C)

//			case M_POWER:				function.set(q_qr_pow),						bmts=returns_ccq_ccq_qqq,	d(disc_qr_pow_i);					return;
//			case M_ASSIGN_DIVIDE:
//			case M_DIVIDE:				function.set(q_qr_divide),					bmts=returns_rcq_ccq_qqq,	d(disc_qr_divide_i);				return;
//			case M_LOGIC_DIVIDES:		function.set(r_qr_logic_divides),			bmts=returns_rcq_ccq_qqq,	d();	return;
//			case M_ASSIGN_MINUS:
//			case M_MINUS:				function.set(q_qr_minus),					bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_PENTATE:				function.set();																								return;
//			case M_TETRATE:				function.set(q_qr_tetrate),					bmts=returns_ccX_ccX_qXX,	d(disc_qr_tetrate_i);				return;
//			case M_POWER_REAL:			function.set(q_qr_power_real),				bmts=returns_rXX_cXX_qXX,	d(disc_rr_power_real_i);			return;
//			case M_ASSIGN_MULTIPLY:
//			case M_MULTIPLY:			function.set(q_qr_multiply),				bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_ASSIGN_PLUS:
//			case M_PLUS:				function.set(q_qr_plus),					bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_LOGIC_AND:			function.set(r_qr_logic_and),				bmts=returns_rrr_rrr_rrr,	d(disc_qr_logic_and_i);				return;
//			case M_LOGIC_XOR:			function.set(r_qr_logic_xor),				bmts=returns_rrr_rrr_rrr,	d(disc_qr_logic_xor_i);				return;
//			case M_LOGIC_OR:			function.set(r_qr_logic_or),				bmts=returns_rrr_rrr_rrr,	d(disc_qr_logic_or_i);				return;
//			case M_CONDITION_ZERO:		function.set(q_qr_condition_zero),	bmts=returns_rcq_ccq_qqq,	d(disc_qr_condition_zero_i);	return;
//			case M_ASSIGN_MOD:
//			case M_MODULO_PERCENT:		function.set(q_qr_modulo),					bmts=returns_rcq_ccq_qqq,	d(disc_qr_modulo_i);				return;
//			case M_ASSIGN_LEFT:
//			case M_BITWISE_SHIFT_LEFT:	function.set(q_qr_bitwise_shift_left),		bmts=returns_rcq_ccq_qqq,	d(disc_qr_bitwise_shift_left_i);	return;
//			case M_ASSIGN_RIGHT:
//			case M_BITWISE_SHIFT_RIGHT:	function.set(q_qr_bitwise_shift_right),		bmts=returns_rcq_ccq_qqq,	d(disc_qr_bitwise_shift_right_i);	return;
//			case M_LOGIC_LESS:			function.set(r_qr_logic_less),				bmts=returns_rrr_rrr_rrr,	d(disc_qr_logic_less_i);			return;
//			case M_LOGIC_LESS_EQUAL:	function.set(r_qr_logic_less_equal),		bmts=returns_rrr_rrr_rrr,	d(disc_qr_logic_less_equal_i);		return;
//			case M_LOGIC_GREATER:		function.set(r_qr_logic_greater),			bmts=returns_rrr_rrr_rrr,	d(disc_qr_logic_greater_i);			return;
//			case M_LOGIC_GREATER_EQUAL:	function.set(r_qr_logic_greater_equal),		bmts=returns_rrr_rrr_rrr,	d(disc_qr_logic_greater_equal_i);	return;
//			case M_LOGIC_EQUAL:			function.set(r_qr_logic_equal),				bmts=returns_rrr_rrr_rrr,	d(disc_qr_logic_equal_i);			return;
//			case M_LOGIC_NOT_EQUAL:		function.set(r_qr_logic_not_equal),			bmts=returns_rrr_rrr_rrr,	d(disc_qr_logic_not_equal_i);		return;
//			case M_ASSIGN_AND:
//			case M_BITWISE_AND:			function.set(q_qr_bitwise_and),				bmts=returns_rcq_ccq_qqq,	d(disc_qr_bitwise_and_o, false);	return;
//			case M_BITWISE_NAND:		function.set(q_qr_bitwise_nand),			bmts=returns_rcq_ccq_qqq,	d(disc_qr_bitwise_nand_o, false);	return;
//			case M_ASSIGN_XOR:
//			case M_BITWISE_XOR:			function.set(q_qr_bitwise_xor),				bmts=returns_rcq_ccq_qqq,	d(disc_qr_bitwise_xor_o, false);	return;
//			case M_BITWISE_XNOR:		function.set(q_qr_bitwise_xnor),			bmts=returns_rcq_ccq_qqq,	d(disc_qr_bitwise_xnor_o, false);	return;
//			case M_ASSIGN_OR:
//			case M_VERTICAL_BAR:		function.set(q_qr_bitwise_or),				bmts=returns_rcq_ccq_qqq,	d(disc_qr_bitwise_or_o, false);		return;
//			case M_BITWISE_NOR:			function.set(q_qr_bitwise_nor),				bmts=returns_rcq_ccq_qqq,	d(disc_qr_bitwise_nor_o, false);	return;
//			case M_LOG:					function.set(q_qc_log),						bmts=returns_ccq_ccq_qqq,	d(disc_qc_log_i);					return;
//			case M_RAND:				function.set(q_qq_random),					bmts=returns_rcq_ccq_qqq,	d(disc_qq_random_o, false);			return;
//		//	case M_RAND:				function.set(q_qr_random),					bmts=returns_rcq_ccq_qqq,	d(disc_qr_random_o, false);			return;
//			case M_ATAN:				function.set(q_qr_atan),					bmts=returns_rcq_ccq_qqq,	d(disc_qr_atan_i);					return;
//			case M_SQWV:				function.set(r_qr_sqwv),					bmts=returns_rcq_ccq_qqq,	d(disc_qr_sqwv_i);					return;
//		//	case M_TRWV:				function.set(q_qr_trwv),					bmts=returns_rcq_ccq_qqq,	d(disc_qr_trwv_i);					return;
//			case M_TRWV:				function.set(q_qq_trwv),					bmts=returns_rcq_ccq_qqq,	d(disc_qr_trwv_i);					return;
//			case M_SAW:					function.set(q_qr_saw),						bmts=returns_rcq_ccq_qqq,	d(disc_qr_saw_i);					return;
//		//	case M_MIN:					function.set(q_qr_min),						bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_MIN:					function.set(q_qq_min),						bmts=returns_rcq_ccq_qqq,	d();								return;
//		//	case M_MAX:					function.set(q_qr_max),						bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_MAX:					function.set(q_qq_max),						bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_BETA:				function.set();																								return;
//			case M_GAMMA:				function.set();																								return;
//		//	case M_PERMUTATION:			function.set(q_qr_permutation),				bmts=returns_rcq_ccq_qqq,	d(disc_qr_permutation_i);			return;
//			case M_PERMUTATION:			function.set(q_qq_permutation),				bmts=returns_rcq_ccq_qqq,	d(disc_qr_permutation_i);			return;
//		//	case M_COMBINATION:			function.set(q_qr_combination),				bmts=returns_rcq_ccq_qqq,	d(disc_qr_combination_i);			return;
//			case M_COMBINATION:			function.set(q_qq_combination),				bmts=returns_rcq_ccq_qqq,	d(disc_qr_combination_i);			return;
//			case M_BESSEL_J:				function.set();																								return;
//			case M_BESSEL_Y:				function.set();																								return;
//			case M_HANKEL1:				function.set();																								return;
//			case M_ASSIGN:				function.set(r_r_assign),					bmts=returns_rcq_rcq_rcq,	d();								return;
			}
			break;
		case 'c':
			switch(f)
			{
			CASE_Q_QC(POWER,			pow, POW,		I)
			case M_ASSIGN_DIVIDE:
			CASE(Q_QC, DIVIDE,			divide,			I)
			CASE(R_QC, LOGIC_DIVIDES,	logic_divides,	C)
			case M_ASSIGN_MINUS:
			CASE(Q_QC, MINUS,			minus,			C)
			CASE_NONE(PENTATE)
			CASE_NONE(TETRATE)
			case M_ASSIGN_MULTIPLY:
			CASE(Q_QC, MULTIPLY,		multiply,		C)
			case M_ASSIGN_PLUS:
			CASE(Q_QC, PLUS,			plus,			C)
			CASE(R_QC, LOGIC_AND,		logic_and,		O)
			CASE(R_QC, LOGIC_XOR,		logic_xor,		O)
			CASE(R_QC, LOGIC_OR,		logic_or,		O)
			CASE(Q_QC, CONDITION_ZERO,	condition_zero,	I)
			case M_ASSIGN_MOD:
			CASE_Q_QC(MODULO_PERCENT,	modulo, MODULO,	I)
			case M_ASSIGN_LEFT:
			CASE(Q_QC, BITWISE_SHIFT_LEFT, bitwise_shift_left, I)
			case M_ASSIGN_RIGHT:
			CASE(Q_QC, BITWISE_SHIFT_RIGHT, bitwise_shift_right, I)
			CASE(R_QC, LOGIC_LESS,		logic_less,		O)
			CASE(R_QC, LOGIC_LESS_EQUAL, logic_less_equal, O)
			CASE(R_QC, LOGIC_GREATER,	logic_greater,	O)
			CASE(R_QC, LOGIC_GREATER_EQUAL, logic_greater_equal, O)
			CASE(R_QC, LOGIC_EQUAL,		logic_equal,	O)
			CASE(R_QC, LOGIC_NOT_EQUAL,	logic_not_equal, O)
			case M_ASSIGN_AND:
			CASE(Q_QC, BITWISE_AND,		bitwise_and,	O)
			CASE(Q_QC, BITWISE_NAND,	bitwise_nand,	O)
			case M_ASSIGN_XOR:
			CASE(Q_QC, BITWISE_XOR,		bitwise_xor,	O)
			CASE(Q_QC, BITWISE_XNOR,	bitwise_xnor,	O)
			case M_ASSIGN_OR:
			CASE_Q_QC(VERTICAL_BAR,		bitwise_or, BITWISE_OR, O)
			CASE(Q_QC, BITWISE_NOR,		bitwise_nor,	O)
			CASE(Q_QC, LOG,				log,			I)
			CASE_Q_QQ(RAND,				random, RANDOM,	O)
		//	CASE_Q_QC(RAND,				random, RANDOM,	O)
			CASE(Q_QC, ATAN,			atan,			I)
			CASE(R_QC, SQWV,			sqwv,			O)
		//	CASE(R_QC, TRWV,			trwv,			C)
			CASE(R_QQ, TRWV,			trwv,			C)
			CASE(R_QC, SAW,				saw,			I)
		//	CASE(Q_QC, MIN,				min,			C)
			CASE(Q_QQ, MIN,				min,			C)
		//	CASE(Q_QC, MAX,				max,			C)
			CASE(Q_QQ, MAX,				max,			C)
			CASE_NONE(BETA)
			CASE_NONE(GAMMA)
		//	CASE(Q_QC, PERMUTATION,		permutation,	I)
			CASE(Q_QQ, PERMUTATION,		permutation,	I)
		//	CASE(Q_QC, COMBINATION,		combination,	I)
			CASE(Q_QQ, COMBINATION,		combination,	I)
			CASE_NONE(BESSEL_J)
			CASE_NONE(BESSEL_Y)
			CASE_NONE(HANKEL1)
			CASE(C_C, ASSIGN,			assign,			C)

//			case M_POWER:				function.set(q_qc_pow),						bmts=returns_ccq_ccq_qqq,	d(disc_qc_pow_i);					return;
//			case M_ASSIGN_DIVIDE:
//			case M_DIVIDE:				function.set(q_qc_divide),					bmts=returns_rcq_ccq_qqq,	d(disc_qc_divide_i);				return;
//			case M_LOGIC_DIVIDES:		function.set(r_qc_logic_divides),			bmts=returns_rcq_ccq_qqq,	d();	return;
//			case M_ASSIGN_MINUS:
//			case M_MINUS:				function.set(q_qc_minus),					bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_PENTATE:				function.set();																								return;
//			case M_TETRATE:				function.set();																								return;
//			case M_ASSIGN_MULTIPLY:
//			case M_MULTIPLY:			function.set(q_qc_multiply),				bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_ASSIGN_PLUS:
//			case M_PLUS:				function.set(q_qc_plus),					bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_LOGIC_AND:			function.set(r_qc_logic_and),				bmts=returns_rrr_rrr_rrr,	d(disc_qc_logic_and_i);				return;
//			case M_LOGIC_XOR:			function.set(r_qc_logic_xor),				bmts=returns_rrr_rrr_rrr,	d(disc_qc_logic_xor_i);				return;
//			case M_LOGIC_OR:			function.set(r_qc_logic_or),				bmts=returns_rrr_rrr_rrr,	d(disc_qc_logic_or_i);				return;
//			case M_CONDITION_ZERO:		function.set(q_qc_condition_zero),	bmts=returns_rcq_ccq_qqq,	d(disc_qc_condition_zero_i);	return;
//			case M_ASSIGN_MOD:
//			case M_MODULO_PERCENT:		function.set(q_qc_modulo),					bmts=returns_rcq_ccq_qqq,	d(disc_qc_modulo_i);				return;
//			case M_ASSIGN_LEFT:
//			case M_BITWISE_SHIFT_LEFT:	function.set(q_qc_bitwise_shift_left),		bmts=returns_rcq_ccq_qqq,	d(disc_qc_bitwise_shift_left_i);	return;
//			case M_ASSIGN_RIGHT:
//			case M_BITWISE_SHIFT_RIGHT:	function.set(q_qc_bitwise_shift_right),		bmts=returns_rcq_ccq_qqq,	d(disc_qc_bitwise_shift_right_i);	return;
//			case M_LOGIC_LESS:			function.set(r_qc_logic_less),				bmts=returns_rrr_rrr_rrr,	d(disc_qc_logic_less_i);			return;
//			case M_LOGIC_LESS_EQUAL:	function.set(r_qc_logic_less_equal),		bmts=returns_rrr_rrr_rrr,	d(disc_qc_logic_less_equal_i);		return;
//			case M_LOGIC_GREATER:		function.set(r_qc_logic_greater),			bmts=returns_rrr_rrr_rrr,	d(disc_qc_logic_greater_i);			return;
//			case M_LOGIC_GREATER_EQUAL:	function.set(r_qc_logic_greater_equal),		bmts=returns_rrr_rrr_rrr,	d(disc_qc_logic_greater_equal_i);	return;
//			case M_LOGIC_EQUAL:			function.set(r_qc_logic_equal),				bmts=returns_rrr_rrr_rrr,	d(disc_qc_logic_equal_i);			return;
//			case M_LOGIC_NOT_EQUAL:		function.set(r_qc_logic_not_equal),			bmts=returns_rrr_rrr_rrr,	d(disc_qc_logic_not_equal_i);		return;
//			case M_ASSIGN_AND:
//			case M_BITWISE_AND:			function.set(q_qc_bitwise_and),				bmts=returns_rcq_ccq_qqq,	d(disc_qc_bitwise_and_o, false);	return;
//			case M_BITWISE_NAND:		function.set(q_qc_bitwise_nand),			bmts=returns_rcq_ccq_qqq,	d(disc_qc_bitwise_nand_o, false);	return;
//			case M_ASSIGN_XOR:
//			case M_BITWISE_XOR:			function.set(q_qc_bitwise_xor),				bmts=returns_rcq_ccq_qqq,	d(disc_qc_bitwise_xor_o, false);	return;
//			case M_BITWISE_XNOR:		function.set(q_qc_bitwise_xnor),			bmts=returns_rcq_ccq_qqq,	d(disc_qc_bitwise_xnor_o, false);	return;
//			case M_ASSIGN_OR:
//			case M_VERTICAL_BAR:		function.set(q_qc_bitwise_or),				bmts=returns_rcq_ccq_qqq,	d(disc_qc_bitwise_or_o, false);		return;
//			case M_BITWISE_NOR:			function.set(q_qc_bitwise_nor),				bmts=returns_rcq_ccq_qqq,	d(disc_qc_bitwise_nor_o, false);	return;
//			case M_LOG:					function.set(q_qc_log),						bmts=returns_ccq_ccq_qqq,	d(disc_qc_log_i);					return;
//			case M_RAND:				function.set(q_qq_random),					bmts=returns_rcq_ccq_qqq,	d(disc_qq_random_o, false);			return;
//		//	case M_RAND:				function.set(q_qc_random),					bmts=returns_rcq_ccq_qqq,	d(disc_qc_random_o, false);			return;
//			case M_ATAN:				function.set(q_qc_atan),					bmts=returns_rcq_ccq_qqq,	d(disc_qc_atan_i);					return;
//			case M_SQWV:				function.set(r_qc_sqwv),					bmts=returns_rcq_ccq_qqq,	d(disc_qc_sqwv_i);					return;
//		//	case M_TRWV:				function.set(q_qc_trwv),					bmts=returns_rcq_ccq_qqq,	d(disc_qc_trwv_i);					return;
//			case M_TRWV:				function.set(q_qq_trwv),					bmts=returns_rcq_ccq_qqq,	d(disc_qc_trwv_i);					return;
//			case M_SAW:					function.set(q_qc_saw),						bmts=returns_rcq_ccq_qqq,	d(disc_qc_saw_i);					return;
//		//	case M_MIN:					function.set(q_qc_min),						bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_MIN:					function.set(q_qq_min),						bmts=returns_rcq_ccq_qqq,	d();								return;
//		//	case M_MAX:					function.set(q_qc_max),						bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_MAX:					function.set(q_qq_max),						bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_BETA:				function.set();																								return;
//			case M_GAMMA:				function.set();																								return;
//		//	case M_PERMUTATION:			function.set(q_qc_permutation),				bmts=returns_rcq_ccq_qqq,	d(disc_qc_permutation_i);			return;
//			case M_PERMUTATION:			function.set(q_qq_permutation),				bmts=returns_rcq_ccq_qqq,	d(disc_qc_permutation_i);			return;
//		//	case M_COMBINATION:			function.set(q_qc_combination),				bmts=returns_rcq_ccq_qqq,	d(disc_qc_combination_i);			return;
//			case M_COMBINATION:			function.set(q_qq_combination),				bmts=returns_rcq_ccq_qqq,	d(disc_qc_combination_i);			return;
//			case M_BESSEL_J:				function.set();																								return;
//			case M_BESSEL_Y:				function.set();																								return;
//			case M_HANKEL1:				function.set();																								return;
//			case M_ASSIGN:				function.set(c_c_assign),					bmts=returns_rcq_rcq_rcq,	d();								return;
			}
			break;
		case 'h':
			switch(f)
			{
			CASE_Q_QQ(POWER,			pow, POW,		I)
			case M_ASSIGN_DIVIDE:
			CASE(Q_QQ, DIVIDE,			divide,			I)
			CASE(R_QQ, LOGIC_DIVIDES,	logic_divides,	C)
			case M_ASSIGN_MINUS:
			CASE(Q_QQ, MINUS,			minus,			C)
			CASE_NONE(PENTATE)
			CASE_NONE(TETRATE)
			case M_ASSIGN_MULTIPLY:
			CASE(Q_QQ, MULTIPLY,		multiply,		C)
			case M_ASSIGN_PLUS:
			CASE(Q_QQ, PLUS,			plus,			C)
			CASE(R_QQ, LOGIC_AND,		logic_and,		O)
			CASE(R_QQ, LOGIC_XOR,		logic_xor,		O)
			CASE(R_QQ, LOGIC_OR,		logic_or,		O)
			CASE(Q_QQ, CONDITION_ZERO,	condition_zero,	I)
			case M_ASSIGN_MOD:
			CASE_Q_QQ(MODULO_PERCENT,	modulo, MODULO,	I)
			case M_ASSIGN_LEFT:
			CASE(Q_QQ, BITWISE_SHIFT_LEFT, bitwise_shift_left, I)
			case M_ASSIGN_RIGHT:
			CASE(Q_QQ, BITWISE_SHIFT_RIGHT, bitwise_shift_right, I)
			CASE(R_QQ, LOGIC_LESS,		logic_less,		O)
			CASE(R_QQ, LOGIC_LESS_EQUAL,logic_less_equal, O)
			CASE(R_QQ, LOGIC_GREATER,	logic_greater,	O)
			CASE(R_QQ, LOGIC_GREATER_EQUAL, logic_greater_equal, O)
			CASE(R_QQ, LOGIC_EQUAL,		logic_equal,	O)
			CASE(R_QQ, LOGIC_NOT_EQUAL,	logic_not_equal, O)
			case M_ASSIGN_AND:
			CASE(Q_QQ, BITWISE_AND,		bitwise_and,	O)
			CASE(Q_QQ, BITWISE_NAND,	bitwise_nand,	O)
			case M_ASSIGN_XOR:
			CASE(Q_QQ, BITWISE_XOR,		bitwise_xor,	O)
			CASE(Q_QQ, BITWISE_XNOR,	bitwise_xnor,	O)
			case M_ASSIGN_OR:
			CASE_Q_QQ(VERTICAL_BAR,		bitwise_or, BITWISE_OR, O)
			CASE(Q_QQ, BITWISE_NOR,		bitwise_nor,	O)
			CASE(Q_QQ, LOG,				log,			I)
			CASE_Q_QQ(RAND,				random, RANDOM, O)
			CASE(Q_QQ, ATAN,			atan,			I)
			CASE(R_QQ, SQWV,			sqwv,			O)
			CASE(R_QQ, TRWV,			trwv,			C)
			CASE(R_QQ, SAW,				saw,			I)
			CASE(Q_QQ, MIN,				min,			C)
			CASE(Q_QQ, MAX,				max,			C)
			CASE_NONE(BETA)
			CASE_NONE(GAMMA)
			CASE(Q_QQ, PERMUTATION,		permutation,	I)
			CASE(Q_QQ, COMBINATION,		combination,	I)
			CASE_NONE(BESSEL_J)
			CASE_NONE(BESSEL_Y)
			CASE_NONE(HANKEL1)
			CASE(Q_Q, ASSIGN,			assign,			C)

//			case M_POWER:				function.set(q_qq_pow),						bmts=returns_ccq_ccq_qqq,	d(disc_qq_pow_i);					return;
//			case M_ASSIGN_DIVIDE:
//			case M_DIVIDE:				function.set(q_qq_divide),					bmts=returns_rcq_ccq_qqq,	d(disc_qq_divide_i);				return;
//			case M_LOGIC_DIVIDES:		function.set(r_qq_logic_divides),			bmts=returns_rcq_ccq_qqq,	d();	return;
//			case M_ASSIGN_MINUS:
//			case M_MINUS:				function.set(q_qq_minus),					bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_PENTATE:				function.set();																								return;
//			case M_TETRATE:				function.set();																								return;
//			case M_ASSIGN_MULTIPLY:
//			case M_MULTIPLY:			function.set(q_qq_multiply),				bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_ASSIGN_PLUS:
//			case M_PLUS:				function.set(q_qq_plus),					bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_LOGIC_AND:			function.set(r_qq_logic_and),				bmts=returns_rrr_rrr_rrr,	d(disc_qq_logic_and_i);				return;
//			case M_LOGIC_XOR:			function.set(r_qq_logic_xor),				bmts=returns_rrr_rrr_rrr,	d(disc_qq_logic_xor_i);				return;
//			case M_LOGIC_OR:			function.set(r_qq_logic_or),				bmts=returns_rrr_rrr_rrr,	d(disc_qq_logic_or_i);				return;
//			case M_CONDITION_ZERO:		function.set(q_qq_condition_zero),	bmts=returns_rcq_ccq_qqq,	d(disc_qq_condition_zero_i);	return;
//			case M_ASSIGN_MOD:
//			case M_MODULO_PERCENT:		function.set(q_qq_modulo),					bmts=returns_rcq_ccq_qqq,	d(disc_qq_modulo_i);				return;
//			case M_ASSIGN_LEFT:
//			case M_BITWISE_SHIFT_LEFT:	function.set(q_qq_bitwise_shift_left),		bmts=returns_rcq_ccq_qqq,	d(disc_qq_bitwise_shift_left_i);	return;
//			case M_ASSIGN_RIGHT:
//			case M_BITWISE_SHIFT_RIGHT:	function.set(q_qq_bitwise_shift_right),		bmts=returns_rcq_ccq_qqq,	d(disc_qq_bitwise_shift_right_i);	return;
//			case M_LOGIC_LESS:			function.set(r_qq_logic_less),				bmts=returns_rrr_rrr_rrr,	d(disc_qq_logic_less_i);			return;
//			case M_LOGIC_LESS_EQUAL:	function.set(r_qq_logic_less_equal),		bmts=returns_rrr_rrr_rrr,	d(disc_qq_logic_less_equal_i);		return;
//			case M_LOGIC_GREATER:		function.set(r_qq_logic_greater),			bmts=returns_rrr_rrr_rrr,	d(disc_qq_logic_greater_i);			return;
//			case M_LOGIC_GREATER_EQUAL:	function.set(r_qq_logic_greater_equal),		bmts=returns_rrr_rrr_rrr,	d(disc_qq_logic_greater_equal_i);	return;
//			case M_LOGIC_EQUAL:			function.set(r_qq_logic_equal),				bmts=returns_rrr_rrr_rrr,	d(disc_qq_logic_equal_i);			return;
//			case M_LOGIC_NOT_EQUAL:		function.set(r_qq_logic_not_equal),			bmts=returns_rrr_rrr_rrr,	d(disc_qq_logic_not_equal_i);		return;
//			case M_ASSIGN_AND:
//			case M_BITWISE_AND:			function.set(q_qq_bitwise_and),				bmts=returns_rcq_ccq_qqq,	d(disc_qq_bitwise_and_o, false);	return;
//			case M_BITWISE_NAND:		function.set(q_qq_bitwise_nand),			bmts=returns_rcq_ccq_qqq,	d(disc_qq_bitwise_nand_o, false);	return;
//			case M_ASSIGN_XOR:
//			case M_BITWISE_XOR:			function.set(q_qq_bitwise_xor),				bmts=returns_rcq_ccq_qqq,	d(disc_qq_bitwise_xor_o, false);	return;
//			case M_BITWISE_XNOR:		function.set(q_qq_bitwise_xnor),			bmts=returns_rcq_ccq_qqq,	d(disc_qq_bitwise_xnor_o, false);	return;
//			case M_ASSIGN_OR:
//			case M_VERTICAL_BAR:		function.set(q_qq_bitwise_or),				bmts=returns_rcq_ccq_qqq,	d(disc_qq_bitwise_or_o, false);		return;
//			case M_BITWISE_NOR:			function.set(q_qq_bitwise_nor),				bmts=returns_rcq_ccq_qqq,	d(disc_qq_bitwise_nor_o, false);	return;
//			case M_LOG:					function.set(q_qq_log),						bmts=returns_ccq_ccq_qqq,	d(disc_qq_log_i);					return;
//			case M_RAND:				function.set(q_qq_random),					bmts=returns_rcq_ccq_qqq,	d(disc_qq_random_o, false);			return;
//			case M_ATAN:				function.set(q_qq_atan),					bmts=returns_rcq_ccq_qqq,	d(disc_qq_atan_i);					return;
//			case M_SQWV:				function.set(r_qq_sqwv),					bmts=returns_rcq_ccq_qqq,	d(disc_qq_sqwv_i);					return;
//			case M_TRWV:				function.set(q_qq_trwv),					bmts=returns_rcq_ccq_qqq,	d(disc_qq_trwv_i);					return;
//			case M_SAW:					function.set(q_qq_saw),						bmts=returns_rcq_ccq_qqq,	d(disc_qq_saw_i);					return;
//			case M_MIN:					function.set(q_qq_min),						bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_MAX:					function.set(q_qq_max),						bmts=returns_rcq_ccq_qqq,	d();								return;
//			case M_BETA:				function.set();																								return;
//			case M_GAMMA:				function.set();																								return;
//			case M_PERMUTATION:			function.set(q_qq_permutation),				bmts=returns_rcq_ccq_qqq,	d(disc_qq_permutation_i);			return;
//			case M_COMBINATION:			function.set(q_qq_combination),				bmts=returns_rcq_ccq_qqq,	d(disc_qq_combination_i);			return;
//			case M_BESSEL_J:				function.set();																								return;
//			case M_BESSEL_Y:				function.set();																								return;
//			case M_HANKEL1:				function.set();																								return;
//			case M_ASSIGN:				function.set(q_q_assign),					bmts=returns_rcq_rcq_rcq,	d();								return;
			}
			break;
		}
		break;
	}
}
void			Compile::compile_instruction_u			(int f, char side, int a1, bool assign)
{
	int result;
	int op=expr->m[a1]._1;
	if(procedural&&term[op].fresh&&(expr->n[op].constant||!assign))
	{
		char mathSet='R';
		expr->insertData(mathSet);//constant but doesn't matter
		result=expr->n.size()-1;
		bool constant=expr->n[result].constant=expr->n[op].constant;
		term=(CompileTerm*)realloc(term, expr->n.size()*sizeof(CompileTerm));
		term[result]=CompileTerm(constant, mathSet, false);
	}
	else
		result=op;
	char op_ms=term[op].mathSet;
	FunctionPointer function;
	int signature=0;
//	char (*umts)(char);
	DiscontinuityFunction d;
	int cl_idx=0, cl_disc_idx=0;
	compile_instruction_select_u(f, side, term[op].mathSet, function, signature, d, cl_idx, cl_disc_idx);
//	compile_instruction_select_u(f, side, term[op].mathSet, function, umts, d);
	if(function.r_r)
	{
		char resultMathSet=returnMathSet_from_signature(signature, term[op].mathSet);
	//	char resultMathSet=umts(term[op].mathSet);
		if(!procedural||!term[result].fresh||(!term[result].constant&&term[result].mathSet>resultMathSet))//'R' < 'c'
			term[result].mathSet=resultMathSet;
		if(term[op].constant||function.type==-1)
		{
			Quat1d x;
			auto LOL_1=&x.r, LOL_2=&x.i, LOL_3=&x.j, LOL_4=&x.k;
			x=expr->data[op];
			switch(function.type)
			{
			case  1:x.r=function.r_r(x.r);													break;//r_r
			case  2:CompRef(x.r, x.i)			=function.c_c(Comp1d(x.r, x.i));			break;//c_c
			case  3:QuatRef(x.r, x.i, x.j, x.k)	=function.q_q(Quat1d(x.r, x.i, x.j, x.k));	break;//q_q
			//case  4:break;//r_rr
			//case  5:break;//c_rc
			//case  6:break;//q_rq
			//case  7:break;//c_cr
			//case  8:break;//c_cc
			//case  9:break;//q_cq
			//case 10:break;//q_qr
			//case 11:break;//q_qc
			//case 12:break;//q_qq
			case 13:CompRef(x.r, x.i)			=function.c_r(x.r);							break;//c_r
			case 14:CompRef(x.r, x.i)			=function.c_q(Quat1d(x.r, x.i, x.j, x.k));	break;//c_q +
			case 15:x.r=function.r_c(Comp1d(x.r, x.i));										break;//r_c +
			case 16:x.r=function.r_q(Quat1d(x.r, x.i, x.j, x.k));							break;//r_q +
			}
			expr->data[result].set(x);
		//	expr->data[result]=function(expr->data[op]);
			term[result].constant=true;
		}
		else
		{
			expr->i.push_back(Instruction(function, op, op_ms, result, resultMathSet, d, cl_idx, cl_disc_idx));
		//	expr->i.push_back(Instruction(function, umts, op, result, d));
		}
		auto &ms=expr->n[result].mathSet;
		ms=maximum(ms, term[result].mathSet);
		expr->resultTerm=result;
	}
	expr->m[a1]._1=result;
}
//int					Compile::compile_instruction_b			(int f, int a1, int a2, bool assign)
//int				Compile::compile_instruction_b			(int f, int _op1, int _op2, bool r_op1, bool _1procVar, bool _2procVar)//r: resultGoesTo_op1=r_op1||op1<op2, 1: resultGoesTo_op1=true, 2: resultGoesTo_op1=false
//int				Compile::compile_instruction_b			(int f, int _op1, int _op2, bool r_op1, bool r_op2)//r: resultGoesTo_op1=r_op1||op1<op2, 1: resultGoesTo_op1=true, 2: resultGoesTo_op1=false
//int				Compile::compile_instruction_b			(int f, int _op1, int _op2, char r_op1)//r: 0: resultGoesTo_op1=op1<op2, 1: resultGoesTo_op1=true, 2: resultGoesTo_op1=false
void			Compile::compile_instruction_b			(int f, int a1, int a2, bool assign)
{
	int op1=expr->m[a1]._1, op2=expr->m[a2]._1, result;
	char op1_ms=term[op1].mathSet, op2_ms=term[op2].mathSet;
	FunctionPointer function;
	int signature=0;
//	char (*bmts)(char, char);
	DiscontinuityFunction d;
	int cl_idx=0, cl_disc_idx=0;
	compile_instruction_select_b(f, op1_ms, op2_ms, function, signature, d, cl_idx, cl_disc_idx);
//	compile_instruction_select_b(f, op1_ms, op2_ms, function, bmts, d);
	if(function.r_rr)
	{
		if(procedural)
		{
			if(term[op1].fresh&&(expr->n[op1].constant||!assign))
			{
				if(term[op2].fresh)
				{
					char mathSet='R';
					expr->insertData(mathSet);
					result=expr->n.size()-1;
					bool constant=expr->n.rbegin()->constant=expr->n[op1].constant&&expr->n[op2].constant;
					term=(CompileTerm*)realloc(term, expr->n.size()*sizeof(CompileTerm));
					term[result]=CompileTerm(constant, mathSet, false);
				}
				else
					result=op2;
			}
			else
				result=op1;
		}
		else
			result=(function.type==5)|(function.type==6)|(function.type==9)?op2:op1;
	//	expr->m[a2]._1=result;

		char resultMathSet=returnMathSet_from_signature(signature, op1_ms, op2_ms);
	//	char resultMathSet=bmts(op1_ms, op2_ms);
		if(!procedural||!term[result].fresh||(!term[result].constant&&term[result].mathSet>resultMathSet))//'R' < 'c'
			term[result].mathSet=resultMathSet;
	//	term[result].mathSet=bmts(term[op1].mathSet, term[op2].mathSet);
		if(term[op1].constant&&term[op2].constant)
		{
			Quat1d x=expr->data[op1], y=expr->data[op2];
			switch(function.type)
			{
		//	case  1:x.r=function.r_r(x.r);															break;//r_r
		//	case  2:CompRef(x.r, x.i)			=function.c_c(Comp1d(x.r, x.i));					break;//c_c
		//	case  3:QuatRef(x.r, x.i, x.j, x.k)	=function.q_q(Quat1d(x.r, x.i, x.j, x.k));			break;//q_q
			case  4:x.r=function.r_rr(x.r, y.r);																		break;//r_rr
			case  5:CompRef(x.r, x.i)			=function.c_rc(x.r, Comp1d(y.r, y.i));									break;//c_rc
			case  6:QuatRef(x.r, x.i, x.j, x.k)	=function.q_rq(x.r, Quat1d(y.r, y.i, y.j, y.k));						break;//q_rq
			case  7:CompRef(x.r, x.i)			=function.c_cr(Comp1d(x.r, x.i), y.r);									break;//c_cr
			case  8:CompRef(x.r, x.i)			=function.c_cc(Comp1d(x.r, x.i), Comp1d(y.r, y.i));						break;//c_cc
			case  9:QuatRef(x.r, x.i, x.j, x.k)	=function.q_cq(Comp1d(x.r, x.i), Quat1d(y.r, y.i, y.j, y.k));			break;//q_cq
			case 10:QuatRef(x.r, x.i, x.j, x.k)	=function.q_qr(Quat1d(x.r, x.i, x.j, x.k), y.r);						break;//q_qr
			case 11:QuatRef(x.r, x.i, x.j, x.k)	=function.q_qc(Quat1d(x.r, x.i, x.j, x.k), Comp1d(y.r, y.i));			break;//q_qc
			case 12:QuatRef(x.r, x.i, x.j, x.k)	=function.q_qq(Quat1d(x.r, x.i, x.j, x.k), Quat1d(y.r, y.i, y.j, y.k));	break;//q_qq
		//	case 13:CompRef(x.r, x.i)			=function.c_r(x.r);									break;//c_r
		//	case 14:CompRef(x.r, x.i)			=function.c_q(Quat1d(x.r, x.i, x.j, x.k));			break;//c_q
		//	case 15:x.r=function.r_c(Comp1d(x.r, x.i));												break;//r_c
		//	case 16:x.r=function.r_q(Quat1d(x.r, x.i, x.j, x.k));									break;//r_q
			case 17:CompRef(x.r, x.i)			=function.c_rr(x.r, y.r);							break;//c_rr
			case 18:x.r=function.r_rc(x.r,							Comp1d(y.r, y.i));				break;//r_rc
			case 19:x.r=function.r_rq(x.r,							Quat1d(y.r, y.i, y.j, y.k));	break;//r_rq
			case 20:x.r=function.r_cr(Comp1d(x.r, x.i),				y.r);							break;//r_cr
			case 21:x.r=function.r_cc(Comp1d(x.r, x.i),				Comp1d(y.r, y.i));				break;//r_cc
			case 22:x.r=function.r_cq(Comp1d(x.r, x.i),				Quat1d(y.r, y.i, y.j, y.k));	break;//r_cq
			case 23:x.r=function.r_qr(Quat1d(x.r, x.i, x.j, x.k),	y.r);							break;//r_qr
			case 24:x.r=function.r_qc(Quat1d(x.r, x.i, x.j, x.k),	Comp1d(y.r, y.i));				break;//r_qc
			case 25:x.r=function.r_qq(Quat1d(x.r, x.i, x.j, x.k),	Quat1d(y.r, y.i, y.j, y.k));	break;//r_qq
			case 26:CompRef(x.r, x.i)=function.r_qq(Quat1d(x.r, x.i, x.j, x.k), Comp1d(y.r, y.i));	break;//c_qc
			}
			expr->data[result].set(x);
			//if(other)
			//	expr->m[a1]._1=expr->m[a2]._1;
			//else
			//	expr->m[a2]._1=expr->m[a1]._1;
		//	expr->data[result]=function(expr->data[op1], expr->data[op2]);
		}
		else
		{
			expr->i.push_back(Instruction(function, op1, op1_ms, op2, op2_ms, result, resultMathSet, d, cl_idx, cl_disc_idx));
			term[result].constant=false;
		//	expr->i.push_back(Instruction(function, bmts, op1, op2, result, d)), term[result].constant=false;
		}
		auto &ms=expr->n[result].mathSet;
		ms=maximum(ms, term[result].mathSet);
		expr->resultTerm=result;
	}
	expr->m[a1]._1=result, expr->m[a2]._0=G2::M_IGNORED;
}
void			Compile::compile_instruction_b2			(int f, int op1, int _op2)
{//M_ASSIGN n[op1]=m[n[_op2]], no m link removal
	int op2=expr->m[_op2]._1;
	int result=op1;//assign only
	FunctionPointer function;
	int signature=0;
//	char (*bmts)(char, char);
	DiscontinuityFunction d;
	int cl_idx=0, cl_disc_idx=0;
	char op1_ms=term[op1].mathSet, op2_ms=term[op2].mathSet, resultMathSet=op2_ms;
	compile_instruction_select_b(f, op1_ms, op2_ms, function, signature, d, cl_idx, cl_disc_idx);
//	compile_instruction_select_b(f, op1_ms, op2_ms, function, bmts, d);
	if(function.r_rr)
	{
		term[result].mathSet=returnMathSet_from_signature(signature, op1_ms, op2_ms);
	//	term[result].mathSet=bmts(term[op1].mathSet, term[op2].mathSet);
		if(term[op1].constant&&term[op2].constant)
		{
			Quat1d x=expr->data[op1], y=expr->data[op2];
			switch(function.type)
			{
			case  1:x.r=function.r_r(x.r);															break;//r_r
			case  2:CompRef(x.r, x.i)				=function.c_c(Comp1d(x.r, x.i));				break;//c_c
			case  3:QuatRef(x.r, x.i, x.j, x.k)	=function.q_q(Quat1d(x.r, x.i, x.j, x.k));			break;//q_q
		//	case  4:x.r=function.r_rr(x.r, y.r);																		break;//r_rr
		//	case  5:CompRef(x.r, x.i)			=function.c_rc(x.r, Comp1d(y.r, y.i));									break;//c_rc
		//	case  6:QuatRef(x.r, x.i, x.j, x.k)	=function.q_rq(x.r, Quat1d(y.r, y.i, y.j, y.k));						break;//q_rq
		//	case  7:CompRef(x.r, x.i)			=function.c_cr(Comp1d(x.r, x.i), y.r);									break;//c_cr
		//	case  8:CompRef(x.r, x.i)			=function.c_cc(Comp1d(x.r, x.i), Comp1d(y.r, y.i));						break;//c_cc
		//	case  9:QuatRef(x.r, x.i, x.j, x.k)	=function.q_cq(Comp1d(x.r, x.i), Quat1d(y.r, y.i, y.j, y.k));			break;//q_cq
		//	case 10:QuatRef(x.r, x.i, x.j, x.k)	=function.q_qr(Quat1d(x.r, x.i, x.j, x.k), y.r);						break;//q_qr
		//	case 11:QuatRef(x.r, x.i, x.j, x.k)	=function.q_qc(Quat1d(x.r, x.i, x.j, x.k), Comp1d(y.r, y.i));			break;//q_qc
		//	case 12:QuatRef(x.r, x.i, x.j, x.k)	=function.q_qq(Quat1d(x.r, x.i, x.j, x.k), Quat1d(y.r, y.i, y.j, y.k));	break;//q_qq
		//	case 13:CompRef(x.r, x.i)			=function.c_r(x.r);									break;//c_r
		//	case 14:CompRef(x.r, x.i)			=function.c_q(Quat1d(x.r, x.i, x.j, x.k));			break;//c_q
		//	case 15:x.r=function.r_c(Comp1d(x.r, x.i));												break;//r_c
		//	case 16:x.r=function.r_q(Quat1d(x.r, x.i, x.j, x.k));									break;//r_q
		//	case 18:x.r=function.r_rc(x.r,							Comp1d(y.r, y.i));				break;//r_rc
		//	case 19:x.r=function.r_rq(x.r,							Quat1d(y.r, y.i, y.j, y.k));	break;//r_rq
		//	case 20:x.r=function.r_cr(Comp1d(x.r, x.i),				y.r);							break;//r_cr
		//	case 21:x.r=function.r_cc(Comp1d(x.r, x.i),				Comp1d(y.r, y.i));				break;//r_cc
		//	case 22:x.r=function.r_cq(Comp1d(x.r, x.i),				Quat1d(y.r, y.i, y.j, y.k));	break;//r_cq
		//	case 23:x.r=function.r_qr(Quat1d(x.r, x.i, x.j, x.k),	y.r);							break;//r_qr
		//	case 24:x.r=function.r_qc(Quat1d(x.r, x.i, x.j, x.k),	Comp1d(y.r, y.i));				break;//r_qc
		//	case 25:x.r=function.r_qq(Quat1d(x.r, x.i, x.j, x.k),	Quat1d(y.r, y.i, y.j, y.k));	break;//r_qq
			}
			expr->data[result].set(x);
		//	expr->data[result]=function(expr->data[op1], expr->data[op2]);
		}
		else
		{
			//if(function.type==5||function.type==6||function.type==9)
			//	std::swap(expr->m[a1]._1, expr->m[a2]._1);
			if(op2!=result)
				expr->i.push_back(Instruction(function, op2, op2_ms, result, resultMathSet, d, cl_idx, cl_disc_idx));
		//	expr->i.push_back(Instruction(function, op1, op1_ms, op2, op2_ms, result, resultMathSet, d));
			term[result].constant=false;
		//	expr->i.push_back(Instruction(function, bmts, op1, op2, result, d)), term[result].constant=false;
		}
		auto &ms=expr->n[result].mathSet;
		ms=maximum(ms, term[result].mathSet);
		expr->resultTerm=result;
	}
}
void			Compile::compile_instruction_condition_111	(int op1, int op2, int op3)
{
	using namespace G2;
	int result=op1;
	char op1_ms=term[op1].mathSet, op2_ms=term[op2].mathSet, op3_ms=term[op3].mathSet, resultMathSet=term[op1].mathSet=maximum(op2_ms, op3_ms);
	if(term[op1].constant&&term[op2].constant&&term[op3].constant)
	{
		expr->data[op1]=expr->data[op1].q_isTrue()?expr->data[op2]:expr->data[op3];
		expr->n[op1].mathSet=term[op1].mathSet;
	}
	else
	{
//		Instruction in=
//		{
//			27,
//			result, op1, op2, op3,
//			resultMathSet, op1_ms, op2_ms, op3_ms,
//			0,
//			DiscontinuityFunction(),
//			std::vector<int>()
//		};
//		expr->i.push_back(in);
		DiscontinuityFunction d;
		expr->i.push_back(Instruction(op1, op1_ms, op2, op2_ms, op3, op3_ms, result, resultMathSet, d, CONDITIONAL_111, DISC_CONDITIONAL_111_I));
		term[op1].constant=false;
	//	expr->i.push_back(Instruction(conditional_111, returns_conditional, op1, op2, op3, result, DiscontinuityFunction(disc_conditional_111_i))), term[result].constant=false;
	}
}
void			Compile::compile_instruction_condition_110	(int op1, int op2)
{
	using namespace G2;
	int result=op2;//
	char op1_ms=term[op1].mathSet, op2_ms=term[op2].mathSet, resultMathSet=op2_ms;//
	term[op1].mathSet=op2_ms;
	if(term[op1].constant&&term[op2].constant)
	{
		expr->data[op1]=expr->data[op1].q_isTrue()?expr->data[op2]:Value();
	//	expr->data[op1]=conditional_110(expr->data[op1], expr->data[op2]);
		expr->n[op1].mathSet=term[op1].mathSet;
	}
	else
	{
		FunctionPointer fp;
		int cl_idx=0;
			 if(op1_ms=='R'){	 if(op2_ms=='R')fp.set(r_rr_conditional_110), cl_idx=R_RR_CONDITIONAL_110;
							else if(op2_ms=='c')fp.set(c_rc_conditional_110), cl_idx=C_RC_CONDITIONAL_110;
							else				fp.set(q_rq_conditional_110), cl_idx=Q_RQ_CONDITIONAL_110;}
		else if(op1_ms=='c'){	 if(op2_ms=='R')fp.set(r_cr_conditional_110), cl_idx=R_CR_CONDITIONAL_110;
							else if(op2_ms=='c')fp.set(c_cc_conditional_110), cl_idx=C_CC_CONDITIONAL_110;
							else				fp.set(q_cq_conditional_110), cl_idx=Q_CQ_CONDITIONAL_110;}
		else if(op1_ms=='h'){	 if(op2_ms=='R')fp.set(r_qr_conditional_110), cl_idx=R_QR_CONDITIONAL_110;
							else if(op2_ms=='c')fp.set(c_qc_conditional_110), cl_idx=C_QC_CONDITIONAL_110;
							else				fp.set(q_qq_conditional_110), cl_idx=Q_QQ_CONDITIONAL_110;}
		DiscontinuityFunction d(disc_conditional_110_i);
		expr->i.push_back(Instruction(fp, op1, op1_ms, op2, op2_ms, result, resultMathSet, d, cl_idx, DISC_CONDITIONAL_110_I));
		term[op1].constant=false;
	//	expr->i.push_back(Instruction(conditional_110, returns_rrr_ccc_qqq, op1, op2, result, DiscontinuityFunction(disc_conditional_110_i))), term[result].constant=false;
	}
}
void			Compile::compile_instruction_condition_101	(int op1, int op3)
{
	using namespace G2;
	int result=op3;
	char op1_ms=term[op1].mathSet, op3_ms=term[op3].mathSet, resultMathSet=op3_ms;
	term[op1].mathSet=op3_ms;
	if(term[op1].constant&&term[op3].constant)
	{
		expr->data[op1]=expr->data[op1].q_isTrue()?Value():expr->data[op3];
		expr->n[op1].mathSet=term[op1].mathSet;
	//	expr->data[result]=conditional_101(expr->data[op1], expr->data[op2]), expr->n[result].mathSet=term[result].mathSet;
	}
	else
	{
		FunctionPointer fp;
		int cl_idx=0;
			 if(op1_ms=='R'){	 if(op3_ms=='R')fp.set(r_rr_conditional_101), cl_idx=R_RR_CONDITIONAL_101;
							else if(op3_ms=='c')fp.set(c_rc_conditional_101), cl_idx=C_RC_CONDITIONAL_101;
							else				fp.set(q_rq_conditional_101), cl_idx=Q_RQ_CONDITIONAL_101;}
		else if(op1_ms=='c'){	 if(op3_ms=='R')fp.set(r_cr_conditional_101), cl_idx=R_CR_CONDITIONAL_101;
							else if(op3_ms=='c')fp.set(c_cc_conditional_101), cl_idx=C_CC_CONDITIONAL_101;
							else				fp.set(q_cq_conditional_101), cl_idx=Q_CQ_CONDITIONAL_101;}
		else if(op1_ms=='h'){	 if(op3_ms=='R')fp.set(r_qr_conditional_101), cl_idx=R_QR_CONDITIONAL_101;
							else if(op3_ms=='c')fp.set(c_qc_conditional_101), cl_idx=C_QC_CONDITIONAL_101;
							else				fp.set(q_qq_conditional_101), cl_idx=Q_QQ_CONDITIONAL_101;}
		DiscontinuityFunction d(disc_conditional_101_i);
		expr->i.push_back(Instruction(fp, op1, op1_ms, op3, op3_ms, result, resultMathSet, d, cl_idx, DISC_CONDITIONAL_101_I));
		term[op1].constant=false;
	//	expr->i.push_back(Instruction(conditional_101, G2::returns_rrr_ccc_qqq, op1, op2, result, DiscontinuityFunction(disc_conditional_101_i))), term[result].constant=false;
	}
}
void			Compile::compile_instruction_condition_100	(int op1)
{
	expr->data[op1]=Value(), term[op1].constant=true, expr->n[op1].mathSet=term[op1].mathSet='R';
}
void			Compile::compile_instruction_condition_011	(int op2, int op3)
{//X link to op3 instead
	using namespace G2;
	int result=op3;
	char op2_ms=term[op2].mathSet, op3_ms=term[op3].mathSet, resultMathSet=term[op2].mathSet=op3_ms;
	if(term[op2].constant&&term[op3].constant)
	{
		expr->data[op2]=expr->data[op3];
		expr->n[op2].mathSet=term[op2].mathSet;
	//	expr->data[op2]=conditional_011(expr->data[op1], expr->data[op2]), expr->n[result].mathSet=term[result].mathSet;
	}
	else
	{
		FunctionPointer fp;
			 if(op3_ms=='R')fp.set(r_r_assign);
		else if(op3_ms=='c')fp.set(c_c_assign);
		else				fp.set(q_q_assign);
		DiscontinuityFunction d;
		expr->i.push_back(Instruction(fp, op2, op2_ms, op3, op3_ms, result, resultMathSet, d, 0, 0));
		term[op2].constant=false;
	//	expr->i.push_back(Instruction(conditional_011, G2::returns_rrr_ccc_qqq, op1, op2, result, DiscontinuityFunction())), term[result].constant=false;
	}
}
void			Compile::compile_instruction_condition_010	(int op1)
{
	expr->data[op1]=Value(), term[op1].constant=true, expr->n[op1].mathSet=term[op1].mathSet='R';
}
//int				Compile::compile_instruction			(int f, char side, int a1, int a2, int a3)
char			Compile::compile_instruction			(int f, char side, int a1, int a2, int a3)
{
	using namespace G2;
	expr->lastInstruction=expr->m[f]._0;
//	bool _1procVar=procedural&&a1!=-1&&!expr->n[expr->m[a1]._1].constant, _2procVar=procedural&&a2!=-1&&!expr->n[expr->m[a2]._1].constant;
	switch(expr->m[f]._0)
	{
	case M_FACTORIAL_LOGIC_NOT:
		if(ub[f]=='u')
			compile_instruction_u(expr->m[f]._0, side, a1);
		else if(ub[f]=='b')
			compile_instruction_u(expr->m[f]._0, '>', a1);//*/
		break;
	case M_BITWISE_NOT:
		if(ub[f]=='u')
			compile_instruction_u(expr->m[f]._0, side, a1);
		else if(ub[f]=='b')
			compile_instruction_u(expr->m[f]._0, '<', a2);
		break;
	case M_PENTATE:
	case M_TETRATE:
	case M_MULTIPLY:
	case M_LOGIC_DIVIDES:
	case M_PLUS:
	case M_LOGIC_AND:
	case M_LOGIC_XOR:
	case M_LOGIC_OR:
	case M_CONDITION_ZERO:
		if(ub[f]=='b')
		{
			compile_instruction_b(expr->m[f]._0, a1, a2);
			return 'b';
		}
		break;
	case M_DIVIDE:
	case M_MINUS:
		if(ub[f]=='u')
		{
			if(side=='<')
				compile_instruction_u(expr->m[f]._0, side, a1);
		}
		else if(ub[f]=='b')
		{
			compile_instruction_b(expr->m[f]._0, a1, a2);
			return 'b';
		}
		break;
	case M_POWER:
		if(ub[f]=='u')
		{
			if(side=='<')
				compile_instruction_u(M_EXP, side, a1);
		}
		else if(ub[f]=='b')
		{
			compile_instruction_b(expr->m[f]._0, a1, a2);
			return 'b';
		}
		break;
	case M_MODULO_PERCENT:
	case M_BITWISE_SHIFT_LEFT:case M_BITWISE_SHIFT_RIGHT:
	case M_LOGIC_LESS:case M_LOGIC_LESS_EQUAL:case M_LOGIC_GREATER:case M_LOGIC_GREATER_EQUAL:
	case M_LOGIC_EQUAL:case M_LOGIC_NOT_EQUAL:
	case M_BITWISE_AND:case M_BITWISE_NAND:
	case M_BITWISE_XOR:case M_BITWISE_XNOR:
	case M_VERTICAL_BAR:case M_BITWISE_NOR:
		if(ub[f]=='u')
			compile_instruction_u(expr->m[f]._0, side, a1);
		else if(ub[f]=='b')
		{
			compile_instruction_b(expr->m[f]._0, a1, a2);
			return 'b';
		}
		break;
	case M_N:
		compile_instruction_b(M_MULTIPLY, a1, a2);
		return 'b';
	case M_QUESTION_MARK://always not procedural
			 if(a1!=-1)	{		 if(a2!=-1)	{		 if(a3!=-1)		compile_instruction_condition_111(	expr->m[a1]._1,	expr->m[a2]._1,	expr->m[a3]._1	), expr->m[a2]._0=M_IGNORED, expr->m[a3]._0=M_IGNORED;
												else				compile_instruction_condition_110(	expr->m[a1]._1,	expr->m[a2]._1					), expr->m[a2]._0=M_IGNORED;							}
							else			{		 if(a3!=-1)		compile_instruction_condition_101(	expr->m[a1]._1,					expr->m[a3]._1	), expr->m[a3]._0=M_IGNORED;
												else				compile_instruction_condition_100(	expr->m[a1]._1									);														}}
		else			{		 if(a2!=-1)	{		 if(a3!=-1)	{	expr->m[a2]=expr->m[a3]; if(a2!=a3)expr->m[a3]._0=M_IGNORED;}//compile_instruction_condition_011(					expr->m[a2]._1, expr->m[a3]._1	), expr->m[a3]._0=M_IGNORED;
												else				compile_instruction_condition_010(					expr->m[a2]._1					);														}
							else			{		 if(a3!=-1)		;
												else				expr->m[f]._0=M_N, expr->m[f]._1=expr->n.size(), expr->insertData('R', Value()), term=(CompileTerm*)realloc(term, expr->n.size()*sizeof(CompileTerm)), term[expr->n.size()-1]=CompileTerm(true, expr->n.rbegin()->mathSet); }}
		//										else				expr->m[f]=::Map(M_N, expr->n.size()), expr->insertData('r', Value()), term=(CompileTerm*)realloc(term, expr->n.size()*sizeof(CompileTerm)), term[expr->n.size()-1]=CompileTerm(true, expr->n.rbegin()->mathSet); }}
		//										else				expr->m[f]=::Map(M_N, expr->n.size()), expr->insertData('R', Value()), term=(CompileTerm*)realloc(term, expr->n.size()*sizeof(CompileTerm)), term[expr->n.size()-1]=CompileTerm(true, expr->n.rbegin()->mathSet); }}
		break;

	case M_ASSIGN://always procedural
		compile_instruction_b2(expr->m[f]._0, expr->m[a1]._1, a2);
		return 'b';
	case M_ASSIGN_MULTIPLY:case M_ASSIGN_DIVIDE:case M_ASSIGN_MOD:
	case M_ASSIGN_PLUS:case M_ASSIGN_MINUS:
	case M_ASSIGN_LEFT:case M_ASSIGN_RIGHT:
	case M_ASSIGN_AND:case M_ASSIGN_XOR:case M_ASSIGN_OR:
		compile_instruction_b(expr->m[f]._0, a1, a2, true);
		return 'b';
	case M_INCREMENT:case M_DECREMENT:
		if(ub[f]=='u')
		{
			if(side=='<')//++a1
				compile_instruction_u(expr->m[f]._0, '_', a1, true);
			else if(side=='>')//a1++
			{
				if(procedural)
				{
					if(term[expr->m[a1]._1].fresh&&!expr->n[expr->m[a1]._1].constant)
					{
						expr->insertData('R');
						term=(CompileTerm*)realloc(term, expr->n.size()*sizeof(CompileTerm));
						term[expr->n.size()-1]=CompileTerm(expr->n.rbegin()->constant, expr->n.rbegin()->mathSet, false);
						compile_instruction_b2(M_ASSIGN, expr->n.size()-1, a1);
						compile_instruction_u(expr->m[f]._0, '_', a1, true);
						expr->m[a1]._1=expr->n.size()-1;
					}
				}
			}
		}
		else if(ub[f]=='b')//a1 ++ a2
			compile_instruction_u(expr->m[f]._0, '_', a2, true);
		break;
	default:
		break;
	}
//	if(expr->m[f]._0>M_FUNCTION_START&&expr->m[f]._0<M_USER_FUNCTION_START)//default function
	if(expr->m[f]._0>M_FSTART&&expr->m[f]._0<M_USER_FUNCTION)//default function
	{
		if(a1!=-1)
		{
			if(ub[f]=='b')//n	f	n
			{
				if(a3!=-1)//binary		a1	f(	a2,	a3)			//a3 is gone but commas are handled outside compile_execute, outdated?
					compile_instruction_b(expr->m[f]._0, a2, a3);
				else//unary		a1	f	a2
					compile_instruction_u(expr->m[f]._0, side, a2);
			}
			else if(side=='<')//f	n
			{
				if(a2!=-1)//binary		f(	a1,	a2)			//a2 is gone but commas are handled outside compile_execute, outdated?
					compile_instruction_b(expr->m[f]._0, a1, a2);
				else//unary		f	a1
					compile_instruction_u(expr->m[f]._0, side, a1);
			}
			else if(side=='>')//	n	f
			{
				compile_instruction_f_def(f);
				return 'd';
			}
		}
		else//f
		{
			compile_instruction_f_def(f);
			return 'd';
		}
	}
	return 0;
}

//int				Compile::compile_instruction_userFunctionCall(int function, std::vector<int> const &args)
//void			Compile::compile_instruction_userFunctionCall(int function, std::vector<int> const &args)
//void			Compile::compile_instruction_userFunctionCall(int function, std::vector<int> const &args, int n_result)
int				Compile::compile_instruction_userFunctionCall(int function, std::vector<int> const &args, bool recursiveCall)
{
	int result;
//	bool new_repositry=false;
	if(!args.size())
	{
		char mathSet='R';
		expr->insertData(mathSet);
		result=expr->n.size()-1;
	//	bool constant=expr->n[result].constant=expr->n[op].constant;
		term=(CompileTerm*)realloc(term, expr->n.size()*sizeof(CompileTerm));
		term[result]=CompileTerm(true, mathSet, false);
	//	term[result]=CompileTerm(constant, mathSet, false);
	//	new_repositry=true;
	}
	else if(procedural)
	{
		bool t=true;
		for(unsigned k=0;k<args.size();++k)
		{
			int arg=args[k];
			if(!term[arg].fresh)
			{
				result=arg;
				t=false;
				break;
			}
		}
		if(t)
		{
			char mathSet='R';
			expr->insertData(mathSet);
			result=expr->n.size()-1;
		//	bool constant=expr->n[result].constant=expr->n[op].constant;
			term=(CompileTerm*)realloc(term, expr->n.size()*sizeof(CompileTerm));
			term[result]=CompileTerm(true, mathSet, false);
		//	term[result]=CompileTerm(constant, mathSet, false);
		//	new_repositry=true;
		}
		for(unsigned k=0;k<args.size();++k)
		{
			int arg=args[k];
			if(!term[arg].constant)
			{
				term[result].constant=false;
				break;
			}
		}
	}
	else
	{
		result=args[0];
		for(int k=1, kEnd=args.size();k<kEnd;++k)
			if(result>args[k])
				result=args[k];
	}
	//	result=args[0];
	
//	char resultMathSet=userFunctionDefinitions[function].resultMathSet;
	char resultMathSet=recursiveCall?predictedMathSet:userFunctionDefinitions[function].resultMathSet;
	if(!procedural||!term[result].fresh||(!term[result].constant&&term[result].mathSet>resultMathSet))//'R' < 'c'
		term[result].mathSet=resultMathSet;
	expr->resultTerm=result;
	auto &ms=expr->n[expr->resultTerm].mathSet;
	ms=maximum(ms, term[result].mathSet);
//	if(!recursiveCall)
//		term[result].mathSet=userFunctionDefinitions[function].resultMathSet;//
	bool result_constant=true;
	for(unsigned k=0;k<args.size();++k)
	{
		int arg=args[k];
		if(!term[arg].constant)
		{
			result_constant=false;
			break;
		}
	}
	Instruction in(function, args, result);
	if(result_constant)
	{
		if(recursiveCall)
		{
			expr->i.push_back(in);
			term[result].constant=false;
		}
		else
			Solve_UserFunction(*expr, in, true)(0);
	}
	else
		expr->i.push_back(in);
/*	if(result_constant)
		Solve_UserFunction(*expr, in)();
	else
		expr->i.push_back(in);//*/
/*	{
		expr->i.push_back(in);
		if(new_repositry)
			term[result].constant=false;
	}//*/
	return result;
}
int				Compile::compile_instruction_branch_if		(int n_condition){	expr->i.push_back(Instruction('b', n_condition));	return expr->i.size()-1;}
int				Compile::compile_instruction_branch_if_not	(int n_condition){	expr->i.push_back(Instruction('B', n_condition));	return expr->i.size()-1;}
int				Compile::compile_instruction_jump			(){					expr->i.push_back(Instruction());					return expr->i.size()-1;}
int				Compile::compile_instruction_return			(int n_result){		expr->i.push_back(Instruction(n_result));			return expr->i.size()-1;}
int				Compile::compile_instruction_return			(Value const &x)
{
	expr->insertData(x.j||x.k?'h':x.i?'c':'R', x);
	term=(CompileTerm*)realloc(term, expr->n.size()*sizeof(CompileTerm));
	term[expr->n.size()-1]=CompileTerm(true, expr->n.rbegin()->mathSet);
	expr->i.push_back(Instruction(expr->n.size()-1));
	return expr->i.size()-1;
}
void			Compile::compile_instruction_assign			(int dst, int src){compile_instruction_b(G2::M_ASSIGN, dst, src, true);}
void			Compile::compile_instruction_assign_value	(int dst, Value const &x)
{
	expr->insertData(x.j||x.k?'h':x.i?'c':'R', x);
	term=(CompileTerm*)realloc(term, expr->n.size()*sizeof(CompileTerm));
	term[expr->n.size()-1]=CompileTerm(true, expr->n.rbegin()->mathSet);
	compile_instruction_b(G2::M_ASSIGN, dst, expr->n.size()-1, true);
}

void			Compile::compile_execute				(int i, int f, int v)
{
	using namespace G2;
	std::list<int> pN;
	for(bool repeat=true;repeat;)
	{
		pN.clear();
		repeat=false;
		int lastF=-1;
		bool startField=true, binaryField=true;
		for(int k=i, kPeak, peak=0;k<f;++k)
		{
			auto &S=expr->m[k]._0;
			if(S==M_N)
			{
				pN.push_back(k);
				ub[k]='b';
				if(startField)
					startField=false;
				else if(peak)
					ub[kPeak]='b';
				peak=0, binaryField=true;
			}
			else if(S)
			{
				if(binaryField&&peak<bi_mass(S))
					kPeak=k, peak=bi_mass(S);
				ub[k]='u';
			//	if(S>M_FUNCTION_START)
				if(S>M_FSTART)
				{
					lastF=k;
					binaryField=false;
				}
			}
		}
		if((!pN.size()&&lastF>-1)||(pN.size()&&lastF>*pN.rbegin()))
			compile_instruction_f_def(lastF), pN.push_back(lastF), ub[lastF]='b', repeat=true;
	}
	if(pN.size())
	{
		for(auto it=pN.begin();;)
		{
			int lOp=-1, rOp=-1;
			for(int k=*it-1;k>=i;--k)
			{
				if(expr->m[k]._0)
				{
					lOp=k;
					break;
				}
			}
			for(int k=*it+1;k<f;++k)
			{
				if(expr->m[k]._0)
				{
					if(v!=1||prec(M_COS, 'u', expr->m[k]._0, ub[k])=='>')//v==1: lazy call last arg f,[..]..	compare right op precedence to a function
						rOp=k;
					break;
				}
			}
			char s;
				 if(lOp>=0)	{		 if(rOp>=0)	s=prec(expr->m[lOp]._0, ub[lOp], expr->m[rOp]._0, ub[rOp]);
								else			s='<';}
			else			{		 if(rOp>=0)	s='>';
								else			s='_';}
			if(s=='<')
			{
				switch(ub[lOp])
				{
				case 'u'://			- [n]
					compile_instruction(lOp, '<', *it);
					expr->m[lOp]._0=M_IGNORED;
					continue;
				case 'b'://			n + [n]
					{
						auto itPrev=it; --itPrev;
						if(compile_instruction(lOp, '_', *itPrev, *it)=='b')//called compile_instruction_b, a2 is gone
							pN.erase(it--);
						if(lOp!=*it)
							expr->m[lOp]._0=M_IGNORED;
					}
					continue;
				}
			}
			else if(s=='>')
			{
				switch(ub[rOp])
				{
				case 'u'://			[n] !
					if((int)compile_instruction(rOp, '>', *it)==-1)//called compile_instruction_f_def
						ub[rOp]='b';
					else
						expr->m[rOp]._0=M_IGNORED;
					continue;
				case 'b'://			[n] + n
					++it;
					continue;
				}
			}
			else if(s=='_')
			{
				expr->m[i]=expr->m[*it];
				if(*it!=i)
					expr->m[*it]._0=M_IGNORED;
				break;
			}
		}
	}
	else
	{
		for(int k=i;k<f;++k)
			expr->m[k]._0=M_IGNORED;
		if(i==expr->m.size())
			expr->m.push_back(::Map(0, 0, M_N, expr->n.size()));
		else
			expr->m[i]=::Map(0, 0, M_N, expr->n.size());
		expr->insertData('R', Value()), term=(CompileTerm*)realloc(term, expr->n.size()*sizeof(CompileTerm)), term[expr->n.size()-1]=CompileTerm(true, expr->n.rbegin()->mathSet);
	//	expr->m[i]=::Map(M_N, expr->n.size()), expr->insertData('R', Value()), term=(CompileTerm*)realloc(term, expr->n.size()*sizeof(CompileTerm)), term[expr->n.size()-1]=CompileTerm(true, expr->n.rbegin()->mathSet);
	}
}
void			Compile::compile_abs					(int i, int f, int v)
{
	if(verticalBarAbs)
	{
		bool field=false, active=false;
		for(int k=i, first=-1;k<f;++k)
		{
			if(expr->m[k]._0==G2::M_VERTICAL_BAR)
			{
				if(field)
				{
					compile_execute(first+1, k, 0);
					compile_instruction(first, '<', first+1);
					expr->m[first]._0=expr->m[k]._0=G2::M_IGNORED;
				}
				else
					first=k;
				field=!field;
			}
			else if(field&&expr->m[k]._0)
				active=true;
		}
	}
	compile_execute(i, f, v);
}
//void Compile::compile_inline_if(int i, int f, int v, bool SIMD)
void Compile::compile_inline_if(int i, int f, int v)
{
	using namespace G2;
	for(;;)
	{
		bool match_colon=false;
		int topLevel=0, i1=-1, f1=-1;
		{
			bool top=false;
			for(int k=i, level=0;k<f;++k)
			{
				auto &S=expr->m[k]._0;
				if(S==M_QUESTION_MARK)
				{
					++level;
					if(topLevel<=level)
						topLevel=level, i1=k, top=true, match_colon=false;
				}
				else if(S==M_COLON)
				{
					--level;
					if(top)
						f1=k, top=false, match_colon=true;
				}
			}
		}
		if(topLevel)
		{
			int i0=f, f2=-1;
			for(int k=i1-1;k>=i;--k)//find range 1 start: [..]?..:..
			{
				if(expr->m[k]._0==M_QUESTION_MARK||expr->m[k]._0==M_COLON)
				{
					if(k!=i1-1)
						i0=k+1;
					break;
				}
				if(k==i)
				{
					i0=k;
					break;
				}
			}
			if(f1!=-1)
			{
				for(int k=f1+1, mLevel=0;k<=f;++k)//find range 3 end: ..?..:[..]
				{
					if(k==f||expr->m[k]._0==M_QUESTION_MARK||expr->m[k]._0==M_COLON)
					{
						if(k!=f1+1)
							f2=k;
						break;
					}
				}
			}
			else
				f1=f2=f;
			if(procedural)
			{
				bool expr1=false;
				for(int k=i0;k<i1;++k)
				{
					if(expr->m[k]._0)
					{
						expr1=true;
						break;
					}
				}
				if(expr1)
				{
					if(expr->n[expr->m[i0+1]._1].constant)
					{
						if(expr->data[expr->m[i0+1]._1].q_isTrue())
					//	if(expr->n[expr->m[i0+1]._1].val.q_isTrue())
						{
							for(int k=i1+1;k<f1;++k)//just expr2
							{
								if(expr->m[k]._0)
								{
									compile_abs(i1+1, f1, 0);
									break;
								}
							}
							for(int k=f1+1;k<f2;++k)
								expr->m[k]=M_IGNORED;
						}
						else
						{
							for(int k=i1+1;k<f1;++k)//just expr3
								expr->m[k]=M_IGNORED;
							for(int k=f1+1;k<f2;++k)
							{
								if(expr->m[k]._0)
								{
									compile_abs(f1+1, f2, 0);
									break;
								}
							}
						}
					}
					else
					{
						bool expr2=false, expr3=false;
						for(int k=i1+1;k<f1;++k)
						{
							if(expr->m[k]._0)
							{
								expr2=true;
								break;
							}
						}
						for(int k=f1+1;k<f2;++k)
						{
							if(expr->m[k]._0)
							{
								expr3=true;
								break;
							}
						}
						if(expr2||expr3)
						{
						/*	expr1 ? expr2 : expr3

								<expr1>
								branch condition if
								<expr3>
								assign expr3
								jump end
							if:
								<expr2>
								assign expr2
							end:
							*/
							compile_abs(i0, i1, 0);//expr1
							int i_branch=compile_instruction_branch_if(expr->m[i0]._1);
							if(expr3)
							{
								compile_abs(f1+1, f2, 0);//expr3
								compile_instruction_assign(expr->m[i0]._1, expr->m[f1+1]._1);//n[i0]=n[f1+1]
							//	compile_instruction_assign(i0, f1+1);//n[i0]=n[f1+1]
							}
							else
								compile_instruction_assign_value(expr->m[i0]._1, Value());//n[i0]=0;
							//	compile_instruction_assign_value(i0, Value());//n[i0]=0;
							int i_jump=compile_instruction_jump();
							expr->i[i_branch].op2=expr->i.size();
							if(expr2)
							{
								compile_abs(i1+1, f1, 0);//expr2
								compile_instruction_assign(expr->m[i0]._1, expr->m[i1+1]._1);//n[i0]=n[i1+1]
							//	compile_instruction_assign(i0, i1+1);//n[i0]=n[i1+1]
							}
							else
								compile_instruction_assign_value(expr->m[i0]._1, Value());//n[i0]=0;
							//	compile_instruction_assign_value(i0, Value());//n[i0]=0;
							expr->i[i_jump].op1=expr->i.size();
							expr->m[i1+1]._0=M_IGNORED, expr->m[f1+1]._0=M_IGNORED;
#if 0//branch if not
						/*	expr1 ? expr2 : expr3

								<expr1>
								branch !condition else
								<expr2>
								assign expr2
								jump end
							else:
								<expr3>
								assign expr3
							end:
							*/
							compile_abs(i0, i1, 0);//expr1
							int i_branch=compile_instruction_branch_if_not(expr->m[i0]._1);
							if(expr2)
							{
								compile_abs(i1+1, f1, 0);//expr2
								compile_instruction_assign(expr.m[i0]._1, expr.m[i1+1]._1);//n[i0]=n[i1+1]
							//	compile_instruction_assign(i0, i1+1);//n[i0]=n[i1+1]
							}
							else
								compile_instruction_assign_value(expr.m[i0]._1, Value());//n[i0]=0;
							//	compile_instruction_assign_value(i0, Value());//n[i0]=0;
							int i_jump=compile_instruction_jump();
							expr->i[i_branch].op2=expr->i.size();
							if(expr3)
							{
								compile_abs(f1+1, f2, 0);//expr3
								compile_instruction_assign(expr.m[i0]._1, expr.m[f1+1]._1);//n[i0]=n[f1+1]
							//	compile_instruction_assign(i0, f1+1);//n[i0]=n[f1+1]
							}
							else
								compile_instruction_assign_value(expr.m[i0]._1, Value());//n[i0]=0;
							//	compile_instruction_assign_value(i0, Value());//n[i0]=0;
							expr->i[i_jump].op1=expr->i.size();
							expr->m[i1+1]._0=M_IGNORED, expr->m[f1+1]._0=M_IGNORED;
#endif
						}
						else
							compile_instruction(i1, '_', -1, -1, -1);
						//	compile_instruction_assign_value(i0, Value());//
					}
				}
				else if(inline_if_default_true)//condition (expr1) is empty: default true (like for loop condition)
				{
					bool expr2=false;
					for(int k=i1+1;k<f1;++k)//just expr2
					{
						if(expr->m[k]._0)
						{
							expr2=true;
							break;
						}
					}
					if(expr2)
						compile_abs(i1+1, f1, 0);
					else
						compile_instruction(i1, '_', -1, -1, -1);
					for(int k=f1+1;k<f2;++k)
						expr->m[k]=M_IGNORED;
				}
				else//default false when condition (expr1) is empty
				{
					bool expr3=false;
					for(int k=i1+1;k<f1;++k)//just expr3
						expr->m[k]=M_IGNORED;
					for(int k=f1+1;k<f2;++k)
					{
						if(expr->m[k]._0)
						{
							expr3=true;
							break;
						}
					}
					if(expr3)
						compile_abs(f1+1, f2, 0);
					else
						compile_instruction(i1, '_', -1, -1, -1);
				}
				expr->m[i1]._0=M_IGNORED, expr->m[f1]._0=M_IGNORED;
			}
			else
			{
				int r0=-1, r1=-1, r2=-1;
				for(int k=i0;k<i1;++k)
				{
					if(expr->m[k]._0)
					{
						compile_abs(r0=i0, i1, 0);
						break;
					}
				}
				for(int k=i1+1;k<f1;++k)
				{
					if(expr->m[k]._0)
					{
						compile_abs(r1=i1+1, f1, 0);
						break;
					}
				}
				for(int k=f1+1;k<f2;++k)
				{
					if(expr->m[k]._0)
					{
						compile_abs(r2=f1+1, f2, 0);
						break;
					}
				}
				compile_instruction(i1, '_', r0, r1, r2);
				if(expr->m[i1]._0!=M_N)
					expr->m[i1]._0=M_IGNORED;
				if(match_colon)
					expr->m[f1]._0=M_IGNORED;
			}
		}
		else
			break;
	}
	compile_abs(i, f, v);
}

void Compile::compile_assignment(int i, int f)
{
	using namespace G2;
	bool notAssignExpr=true;
	for(int k=f-1;k>=i;--k)
	{
		auto &S=expr->m[k]._0;
		if(S>M_PROCEDURAL_ASSIGN_START&&S<M_PROCEDURAL_ASSIGN_END)
		{
			compile_inline_if(k+1, f, 0);
		//	compile_inline_if(k+1, f, 0, false);
			int lastA=k;
			for(int k2=k-1;k2>=i;--k2)
			{
				auto &S2=expr->m[k2]._0;
				if(S2>M_PROCEDURAL_ASSIGN_START&&S2<M_PROCEDURAL_ASSIGN_END)
				{
					compile_inline_if(k2+1, lastA, 0);
				//	compile_inline_if(k2+1, lastA, 0, false);
					compile_instruction(k2, '_', k2+1, lastA+1);
					S2=M_IGNORED;
					lastA=k2;
				}
			}
			compile_inline_if(i, lastA, 0);
		//	compile_inline_if(i, lastA, 0, false);
			compile_instruction(k, '_', i, lastA+1);
			S=M_IGNORED;
			notAssignExpr=false;
			break;
		}
	}
	if(notAssignExpr)
		compile_inline_if(i, f, 0);
	//	compile_inline_if(i, f, 0, false);
}
void Compile::compile_expression_local(int _i, int _f)
{
	using namespace G2;
	for(;;)
	{
		int topLevel=0, i=-1, f=-1;
		{
			bool peak=false;
			for(int k=_i, level=0;k<_f;++k)
		//	for(int k=0, kEnd=expr->m.size(), level=0;k<kEnd;++k)
			{
				if(expr->m[k]._0==M_LPR)
				{
					++level;
					if(level>=topLevel)
						topLevel=level, i=k+1, peak=true;
				}
				else if(expr->m[k]._0==M_RPR)
				{
					--level;
					if(peak)
						f=k, peak=false;
				}
			}
		}
			 if(i==-1)	i=_i;//0;
		else			expr->m[i-1]._0=M_IGNORED;
			 if(f==-1)	f=_f;//expr->m.size();
		else			expr->m[f  ]._0=M_IGNORED;
	//	if(topLevel>0&&i-2>=0&&expr->m[i-2]._0>M_FUNCTION_START)
		if(topLevel>0&&i-2>=0&&expr->m[i-2]._0>M_FSTART)//call level
		{
			auto &S=expr->m[i-2]._0;
			std::vector<int> commas;
			bool notVoidCall=false;
			for(int k=i;k<f;++k)
			{
				auto &S2=expr->m[k]._0;
				if(S2==M_COMMA)
					commas.push_back(k);
				notVoidCall|=S2!=M_IGNORED;
			}
			int exprNArgs=commas.size()+notVoidCall;
		//	if(S<M_USER_FUNCTION_START)
			if(S<M_USER_FUNCTION)//default function call level
			{
				int signature=default_overload(S);
				if(signature&(1<<exprNArgs))//default overload match
				{
					int start=i;
					for(int k=0, kEnd=commas.size();k<kEnd;++k)
					{
						expr->m[commas[k]]._0=M_IGNORED;
						compile_assignment(start, commas[k]);
						start=commas[k]+1;
					}
					compile_assignment(start, f);
					if(commas.size()==2)//ternary function
						compile_instruction(i-2, '<', i, commas[0]+1, commas[1]+1), expr->m[commas[0]+1]=expr->m[commas[1]+1]=M_IGNORED;
					else if(commas.size())//binary function
						compile_instruction(i-2, '<', i, commas[0]+1), expr->m[commas[0]+1]=M_IGNORED;
					else//unary function
						compile_instruction(i-2, '<', i);
					S=M_IGNORED;
				}
				else//syntax error, no such overload
				{
					if(exprNArgs>signature)//too many args: highlight unexpected args
						expr->insertSyntaxError(expr->m[commas[signature>0x3]].pos, expr->m[f].pos);
					else//highlight arglist contents
						expr->insertSyntaxError(expr->m[i].pos, expr->m[f].pos);
					expr->i.clear();
					expr->valid=false;
					return;
				}
			}
			else//user function call level
			{
				int name_id=expr->m[i-2]._1, d_match=-1;
				for(int d=0, dEnd=userFunctionDefinitions.size();d<dEnd&&expr->lineNo>=userFunctionDefinitions[d].lineNo;++d)//linearly through all user definitions		sorted by lineNo
			//	for(int d=0, dEnd=userFunctionDefinitions.size();d<dEnd&&expr->lineNo>userFunctionDefinitions[d].lineNo;++d)
				{
					auto &definition=userFunctionDefinitions[d];
					if(definition.valid&&name_id==definition.name_id&&exprNArgs==definition.nArgs)//find overload instance
						d_match=d;
				}
				if(d_match!=-1)
				{
					std::vector<int> args(exprNArgs);
					if(notVoidCall)//compile args
					{
						int start=i;
						for(int k3=0, k3End=exprNArgs-1;k3<k3End;++k3)
						{
							expr->m[commas[k3]]._0=M_IGNORED;//before compile in case start==commas[k3]
							compile_assignment(start, commas[k3]);
							args[k3]=expr->m[start]._1, expr->m[start]._0=M_IGNORED;
							start=commas[k3]+1;
						}
						compile_assignment(start, f);
						args[exprNArgs-1]=expr->m[start]._1, expr->m[start]._0=M_IGNORED;
					}
					bool recursiveCall=&userFunctionDefinitions[d_match]==expr;
					recursiveFunction|=recursiveCall;
					int result=compile_instruction_userFunctionCall(d_match, args, recursiveCall);
				//	int result=compile_instruction_userFunctionCall(d_match, args, &userFunctionDefinitions[d_match]==expr);
					S=M_N, expr->m[i-2]._1=result;
				}
				else//syntax error, no such overload
				{
					expr->insertSyntaxError(expr->m[i].pos, expr->m[f].pos);//arglist
				//	expr->insertSyntaxError(expr->m[k]._1);//mark the error		function call level: highlight id, parentheses and arglist
					expr->i.clear();
					expr->valid=false;
					return;
				}
			}
		}
		else//not a call level
		{
			int start=i;
			for(int k=i;k<f;++k)
			{
				if(expr->m[k]._0==M_COMMA)
				{
					compile_assignment(start, k);
					expr->m[start]._0=M_IGNORED, expr->m[k]._0=M_IGNORED;
					start=k+1;
				}
			}
			compile_assignment(start, f);
			if(start!=i)
				expr->m[i]=expr->m[start], expr->m[start]._0=M_IGNORED;
		}
		if(topLevel==0)
			break;
	}
}
bool Compile::compile_exprStatement(int &k, int mEnd)
//void Compile::compile_exprStatement(int &k, int mEnd)
{
	for(int k2=k, level=0;k2<mEnd;++k2)
	{
		auto &S=expr->m[k2]._0;
		if(S==G2::M_SEMICOLON)
		{
			compile_expression_local(k, k2);
			S=G2::M_IGNORED;//in case k==k2 & execute leaves a link: semicolon statements are expendable
			k=k2;
			return true;
		}
		else if(S>G2::M_PROCEDURAL_START&&S<G2::M_PROCEDURAL_ASSIGN_START)
	//	else if(S>G2::M_PROCEDURAL_START&&S<G2::M_PROCEDURAL_END)//X expression_local calls compile_assignments
		{
			//syntax error
			auto &S2=expr->m[k2-(k<k2-1)];
			expr->insertSyntaxError(expr->m[k].pos, S2.pos+S2.len);
			k=k2-1;
			break;
		}
		else if(S==G2::M_LPR)
			++level;
		else if(S==G2::M_RPR)
			--level;
		else if(S==G2::M_COMMA&&!level)
		{
			compile_expression_local(k, k2);
			S=G2::M_IGNORED;//in case k==k2 & execute leaves a link: semicolon statements are expendable
			k=k2+1;
		}
	}
	return false;
}
bool Compile::comp_seek_allowNewline(int &k, int mEnd, int S)
{
	for(int k2=k;k2<mEnd;++k2)
	{
		auto &S2=expr->m[k2]._0;
		if(S2==S)
		{
			k=k2;
			return true;
		}
		if(S2!=G2::M_IGNORED)
			return false;
	}
	return false;
}
bool Compile::comp_seek_condition(int &k, int mEnd, int &conditionStart, int &conditionEnd)
{
	using namespace G2;
	int k2=k;
	if(comp_seek_allowNewline(k2, mEnd, M_LPR))
	{
		conditionStart=++k2;
		for(int level=1;k2<mEnd;++k2)//profile condition
		{
			auto &S=expr->m[k2]._0;
			if(S==M_LPR)
				++level;
			else if(S==M_RPR)
			{
				--level;
				if(!level)
				{
					conditionEnd=k2;
					k=k2;
					return true;
				}
			}
			else if(S==M_SEMICOLON)
				return false;
		}
	}
	return false;
}
bool Compile::comp_seek_forHeader(int &k, int mEnd, int &headerStart, int &sc1, int &sc2, int &headerEnd)
{
	using namespace G2;
	int k2=k;
	if(comp_seek_allowNewline(k2, mEnd, M_LPR))
	{
		headerStart=++k2;
		for(int level=1, nSemicolons=0;k2<mEnd;++k2)
		{
			auto &S=expr->m[k2]._0;
			if(S==M_LPR)
				++level;
			else if(S==M_RPR)
			{
				--level;
				if(!level)
				{
					if(nSemicolons==2)
					{
						k=headerEnd=k2;
						return true;
					}
					return false;
				}
			}
			else if(S==M_SEMICOLON)
			{
				if(level==1)
				{
					++nSemicolons;
					if(nSemicolons==1)
						sc1=k2;
					else
						sc2=k2;
				}
				else return false;
			}
		}
	}
	return false;
}
bool Compile::comp_seek_block(int &k, int mEnd, int &blockStart, int &blockEnd)
{
	using namespace G2;
	if(comp_seek_allowNewline(k, mEnd, M_LBRACE))
	{
		blockStart=k+1;
		for(int k2=blockStart, level=1;k2<mEnd;++k2)//profile block
		{
			auto &S=expr->m[k2]._0;
			if(S==M_LBRACE)
				++level;
			else if(S==M_RBRACE)
			{
				--level;
				if(!level)
				{
					k=blockEnd=k2;
					return true;
				}
			}
		}
	}
	return false;
}
/*struct CompileBlockInfo
{
	int k;
	int mStart, mEnd;
	CompileBlockInfo(int k, int mStart, int mEnd):k(k), mStart(mStart), mEnd(mEnd){}
};//*/
std::stack<std::pair<int, std::vector<int>>> Compile::loopInfo;
//std::stack<int> Compile::loopStart, Compile::loopEnd;
//int Compile::n_lastResult;
//std::stack<int> loopStart, loopEnd;
//int n_lastResult;
void	compile_break_address(std::stack<std::pair<int, std::vector<int>>> &loopInfo, Expression *expr)
{
	int end=expr->i.size();//1
	auto &it=loopInfo.top().second;
	for(unsigned k3=0;k3<it.size();++k3)
	{
		int &i_break=it[k3];
		expr->i[i_break].result=end;
	}
}
void Compile::compile_statement(int &k, int mEnd)
{
	using namespace G2;
	switch(expr->m[k]._0)
	{
	case M_IF:
		{
			int k2=k+1;
			int conditionStart, conditionEnd;
			if(comp_seek_condition(k2, mEnd, conditionStart, conditionEnd))
			{
			/*	<condition>
				branch !condition end
				<if body>
			end:

				<condition>
				branch !condition else
				<if body>
				jump end
			else:
				<else body>
			end:
				*/
				compile_expression_local(conditionStart, conditionEnd);//<condition>
				int n_condition=expr->m[conditionStart]._1;
			//	int n_condition;
			//	compile_expression_local(n_condition=conditionStart, conditionEnd);	//n_condition: op1, i_branch: result	if(ex.n[ex.i[i].op1].ndr[v].r_isTrue())i=ex.i[i].result;

				int i_branch=compile_instruction_branch_if_not(n_condition);
				{
					int blockStart=0, blockEnd=0;
					k2=conditionEnd+1;
					if(comp_seek_block(k2, mEnd, blockStart, blockEnd))//<if body>
					{
						compile_block(blockStart, blockEnd);						//double recursive for now
						expr->m[blockStart-1]._0=M_IGNORED, expr->m[blockEnd]._0=M_IGNORED;
					}
					else
						compile_statement(k2, mEnd);
					//	compile_exprStatement(start, mEnd);
				}
				if(comp_seek_allowNewline(k2, mEnd, M_ELSE))
				{
					int i_jump=compile_instruction_jump();
					expr->i[i_branch].result=expr->i.size();//branch to else
				//	expr->i[i_branch].op2=expr->i.size();
					++k2;
					compile_statement(k2, mEnd);//<else body> statement						//recursive for now
					expr->i[i_jump].result=expr->i.size();//jump to end
				//	expr->i[i_jump].op2=expr->i.size();
				}
				else
					expr->i[i_branch].result=expr->i.size();//branch to end
				//	expr->i[i_branch].op2=expr->i.size();
				k=k2;
			}
			else//syntax error: expected condition
				expr->insertSyntaxError(expr->m[k].pos, expr->m[k].pos+2);
		}
		break;
	case M_FOR:
		{
			int k2=k+1;
			int headerStart, sc1, sc2, headerEnd;
			if(comp_seek_forHeader(k2, mEnd, headerStart, sc1, sc2, headerEnd))
			{
			/*		<start>
					jump condition
				loop:
					<body>
					<increment>
				condition:
					<condition>
					branch condition loop
				*/
			/*		<start>						//empty condition
				loop:
					<body>
					<increment>
					jump loop
				*/
				if(headerStart<sc1)
					compile_expression_local(headerStart, sc1);//<start>
				bool conditionNotEmpty=sc1+1<sc2;
				int i_jump=-1;
				if(conditionNotEmpty)
					i_jump=compile_instruction_jump();
			//	int i_jump=compile_instruction_jump();//jump to condition below		//i_jump: result		i=expr->i[i].result;		//i_jump: op1		i=expr->i[i].op1;
				int i_loop=expr->i.size();
				loopInfo.push(std::pair<int, std::vector<int>>(i_loop, std::vector<int>()));
				{
					int blockStart=0, blockEnd=0;
					k2=headerEnd+1;
					if(comp_seek_block(k2, mEnd, blockStart, blockEnd))//<body>
						compile_block(blockStart, blockEnd);					//double recursive for now
					else
						compile_statement(k2, mEnd);
					//	compile_exprStatement(start, mEnd);
				}
				compile_expression_local(sc2+1, headerEnd);//<increment>
				if(conditionNotEmpty)
				{
					expr->i[i_jump].result=expr->i.size();
				//	expr->i[i_jump].op1=expr->i.size();
					compile_expression_local(sc1+1, sc2);//<condition>
					int n_condition=expr->m[sc1+1]._1;
				//	int n_condition;
				//	compile_expression_local(n_condition=sc1+1, sc2);

					int i_branch=compile_instruction_branch_if(n_condition);
					expr->i[i_branch].result=i_loop;
				//	expr->i[i_branch].op2=i_loop;
				}
				else
				{
					i_jump=compile_instruction_jump();
					expr->i[i_jump].result=i_loop;
				}
				compile_break_address(loopInfo, expr);
				k=k2;
				loopInfo.pop();
			}
			else//expected for header
				expr->insertSyntaxError(expr->m[k].pos, expr->m[k].pos+3);
		}
		break;
	case M_WHILE:
		{
			int k2=k+1;
			int conditionStart, conditionEnd;
			if(comp_seek_condition(k2, mEnd, conditionStart, conditionEnd))
			{
			/*		jump condition
				loop:
					<body>
				condition:
					<condition>
					branch condition loop
				*/
			/*	loop:							//empty condition (syntax error in c/c++)
					<body>
					jump loop
				*/
				bool conditionNotEmpty=conditionStart<conditionEnd;
				int i_jump=-1;
				if(conditionNotEmpty)
					i_jump=compile_instruction_jump();
			//	int i_jump=compile_instruction_jump();
				int i_loop=expr->i.size();
				loopInfo.push(std::pair<int, std::vector<int>>(i_loop, std::vector<int>()));
				{
					int blockStart=0, blockEnd=0;
					k2=conditionEnd+1;
					if(comp_seek_block(k2, mEnd, blockStart, blockEnd))//<body>
						compile_block(blockStart, blockEnd);						//double recursive for now
					else
						compile_statement(k2, mEnd);
					//	compile_exprStatement(conditionEnd, mEnd);
				}
				if(conditionNotEmpty)
					expr->i[i_jump].result=expr->i.size();
			//	expr->i[i_jump].op1=expr->i.size();
				
				if(conditionNotEmpty)
				{
					compile_expression_local(conditionStart, conditionEnd);//<condition>
					int n_condition=expr->m[conditionStart]._1;
				//	int n_condition;
				//	compile_expression_local(n_condition=conditionStart, conditionEnd);

					int i_branch=compile_instruction_branch_if(n_condition);
					expr->i[i_branch].result=i_loop;
				//	expr->i[i_branch].op2=i_loop;
				}
				else
				{
					i_jump=compile_instruction_jump();
					expr->i[i_jump].result=i_loop;
				}
				compile_break_address(loopInfo, expr);
				k=k2;
				loopInfo.pop();
			}
			else//expected condition
				expr->insertSyntaxError(expr->m[k].pos, expr->m[k].pos+5);
		}
		break;
	case M_DO:
		{
			int k2=k+1;
			int blockStart=0, blockEnd=0;
			bool block=comp_seek_block(k2, mEnd, blockStart, blockEnd);
			if(block)
				expr->m[blockStart-1]._0=M_IGNORED, expr->m[blockEnd]._0=M_IGNORED;
			if(comp_seek_allowNewline(k2, mEnd, M_WHILE))
			{
				int conditionStart, conditionEnd;
				++k2;
				if(comp_seek_condition(k2, mEnd, conditionStart, conditionEnd))
				{
				/*	loop:
						<body>
						<condition>
						branch condition loop
					*/
				/*	loop:							//empty condition (syntax error in c/c++)
						<body>
						jump loop
					*/
					int i_loop=expr->i.size();
					loopInfo.push(std::pair<int, std::vector<int>>(i_loop, std::vector<int>()));
					if(block)//<body>
						compile_block(blockStart, blockEnd);				//double recursive for now
					else
					{
						k2=conditionEnd+1;
						compile_statement(k2, mEnd);
					//	compile_exprStatement(conditionEnd, mEnd);
					}
					
					bool conditionNotEmpty=conditionStart<conditionEnd;
					if(conditionNotEmpty)
					{
						compile_expression_local(conditionStart, conditionEnd);//<conditions>
						int n_condition=expr->m[conditionStart]._1;
					//	int n_condition;
					//	compile_expression_local(n_condition=conditionStart, conditionEnd);

						int i_branch=compile_instruction_branch_if(n_condition);
						expr->i[i_branch].result=i_loop;
					//	expr->i[i_branch].op2=i_loop;
					}
					else
					{
						int i_jump=compile_instruction_jump();
						expr->i[i_jump].result=i_loop;
					}
					compile_break_address(loopInfo, expr);
					k=k2;
				}
				else//expected condition
					expr->insertSyntaxError(expr->m[k].pos, expr->m[k2].pos+5);
			}
			else//expected do
				expr->insertSyntaxError(expr->m[k].pos, expr->m[blockEnd].pos);
		}
		break;
	case M_LBRACE:
		{
		//	int k2=k+1;//X comp_seek_block: seek '{' (in this case: again) then close with '}'
			int k2=k;
			int blockStart=0, blockEnd=0;
			if(comp_seek_block(k2, mEnd, blockStart, blockEnd))
			{
				compile_block(blockStart, blockEnd);				//double recursive for now
				k=k2;
			}
			else//unmatched brace, unreachable
				expr->insertSyntaxError(expr->m[k].pos, expr->m[k].pos+1);
		}
		break;
	case M_CONTINUE:
		if(loopInfo.size())
		{
			int i_jump=compile_instruction_jump();
			expr->i[i_jump]=loopInfo.top().first;
		}
		else//continue not expected here
			expr->insertSyntaxError(expr->m[k].pos, expr->m[k].pos+8);
	/*	if(loopStart.size())
		{
			int i_jump=compile_instruction_jump();
			expr->i[i_jump]=loopStart.top();
		}
		else//continue not expected here
			expr->insertSyntaxError(expr->m[k].pos, expr->m[k].pos+8);//*/
		break;
	case M_BREAK:
		if(loopInfo.size())
		{
			int i_jump=compile_instruction_jump();
			loopInfo.top().second.push_back(i_jump);
		}
		else//break not expected here
			expr->insertSyntaxError(expr->m[k].pos, expr->m[k].pos+5);
	/*	if(loopStart.size())
		{
			int i_jump=compile_instruction_jump();
			loopEnd.push(i_jump);//when loop is compiled	if(loopEnd.size())expr->i[loopEnd.top()].op1=expr->i.size(), loopEnd.pop();
		}
		else//break not expected here
			expr->insertSyntaxError(expr->m[k].pos, expr->m[k].pos+5);//*/
		break;
	case M_RETURN:
		if(k+1<mEnd&&expr->m[k+1]._0==M_SEMICOLON)//return;		default return 0;
			compile_instruction_return(Value());
		else
		{
			int k2=k+1;
			if(compile_exprStatement(k2, mEnd))
			{
				compile_instruction_return(expr->m[k+1]._1);
				char mathSet=term[expr->m[k+1]._1].mathSet;
			//	char mathSet=expr->n[expr->m[k+1]._1].mathSet;//X not updated
				if(predictedMathSet<mathSet)
					expr->resultMathSet=predictedMathSet=mathSet;
				else
					expr->resultMathSet=predictedMathSet;//just to initialize?
			}//else syntax error: marked & ignored
			k=k2;

		//	++k;
		//	compile_exprStatement(k, mEnd);
		//	compile_instruction_return(k);
		}
		break;
	default:
	/*	{
			int start=k;
			compile_exprStatement(k, mEnd);
			n_lastResult=expr->m[start]._1;//when function is done compiling
		}//*/
		compile_exprStatement(k, mEnd);
	//	n_lastResult=k;//X
		break;
	}
}
void Compile::compile_block(int mStart, int mEnd)
{
//	std::stack<CompileBlockInfo> s;
	for(int k=mStart;k<mEnd;++k)
		compile_statement(k, mEnd);
}
void Compile::compile_function(Expression &expr)
{
	procedural=true, recursiveFunction=false;
//	expr.resultMathSet=predictedMathSet='R';
	predictedMathSet='R';
	Compile::expr=&expr;
	if(expr.m.size())
	{
		ub=new char[expr.m.size()];
		memset(ub, 0, expr.m.size()*sizeof(char));
	}
	else
		ub=nullptr;
	if(expr.n.size())
	{
		term=(CompileTerm*)malloc(expr.n.size()*sizeof(CompileTerm));
		for(unsigned n=0;n<expr.n.size();++n)
		{
			auto &t=expr.n[n];
			term[n]=CompileTerm(t.constant, t.mathSet);
		}
	}
	else
		term=nullptr;


	auto map=expr.m;
	int dataSize=expr.n.size();
	int f=expr.m.size()-1;
	compile_block(1, f);
	if(recursiveFunction&&predictedMathSet>'R')//'R' < 'c'
	{
		expr.m=map;
		delete[] ub;
		if(expr.m.size())
		{
			ub=new char[expr.m.size()];
			memset(ub, 0, expr.m.size()*sizeof(char));
		}
		else
			ub=nullptr;
		expr.n.resize(dataSize), expr.data.resize(dataSize);
		if(dataSize)
		{
			term=(CompileTerm*)realloc(term, dataSize*sizeof(CompileTerm));
			for(unsigned n=0;n<expr.n.size();++n)
			{
				auto &t=expr.n[n];
				term[n]=CompileTerm(t.constant, t.mathSet);
			}
		}
		else
			term=nullptr;
		expr.i.clear();
		compile_block(1, f);
	}
//	compile_block(1, expr.m.size()-1);

//	compile_block(0, expr.m.size());
//	int k=0;
//	compile_statement(k, expr.m.size());

	delete[] ub;

	if(expr.i.size())
	{
		bool noBranches=true;
		for(unsigned k=0;k<expr.i.size();++k)
		{
			auto &in=expr.i[k];
			if(in.type>13&&in.type!='c')
			{
				noBranches=false;
				break;
			}
		}
		if(noBranches)//last instruction affects predictedMathType
		{
			auto &lastInstr=*expr.i.rbegin();
			if(lastInstr.type<4)
			{
				char mathSet=term[lastInstr.result].mathSet;
				if(predictedMathSet>mathSet)
				//	expr.resultMathSet=predictedMathSet=mathSet;
					predictedMathSet=mathSet;
			}
			else if(lastInstr.type=='c')
			{
				auto &definition=userFunctionDefinitions[lastInstr.op1];
				if(&definition!=&expr)//not a recursive call
				{
					char mathSet=definition.resultMathSet;
					predictedMathSet=maximum(predictedMathSet, mathSet);
					//if(predictedMathSet>mathSet)
					////	expr.resultMathSet=predictedMathSet=mathSet;
					//	predictedMathSet=mathSet;
				}
			}//'r' handled in compile_statement
		}
	}
	else
		compile_instruction_return(Value());
	free(term);
}

int				Compile::expressionResultLogicType()
{
	using namespace G2;
//	if(expr->lastInstruction>M_FUNCTION_START)																return 0;
	if(expr->lastInstruction>M_FSTART)																		return 0;
	switch(expr->lastInstruction)
	{
	case M_FACTORIAL_LOGIC_NOT:	case M_BITWISE_NOT:															return 0;
	case M_PLUS:				case M_MINUS:																return 0;
	case M_BITWISE_AND:			case M_BITWISE_NAND:														return 0;
	case M_BITWISE_XOR:			case M_BITWISE_XNOR:														return 0;
	case M_VERTICAL_BAR:		case M_BITWISE_NOR:															return 0;
	case M_MULTIPLY:			case M_DIVIDE:case M_MODULO_PERCENT:										return 0;
	case M_LOGIC_DIVIDES:																					return 1;
	case M_BITWISE_SHIFT_LEFT:	case M_BITWISE_SHIFT_RIGHT:													return 0;
	case M_POWER:																							return 0;
	case M_TETRATE:																							return 0;
	case M_PENTATE:																							return 0;
	case M_LOGIC_LESS:			case M_LOGIC_LESS_EQUAL:case M_LOGIC_GREATER:case M_LOGIC_GREATER_EQUAL:	return 1;
	case M_LOGIC_EQUAL:																						return 2;
	case M_LOGIC_NOT_EQUAL:																					return 3;
	case M_LOGIC_AND:																						return 1;
	case M_LOGIC_XOR:																						return 1;
	case M_LOGIC_OR:																						return 1;
	case M_CONDITION_ZERO:																					return 0;
	case M_S_EQUAL_ASSIGN:																					return 2;
	case M_S_NOT_EQUAL:																						return 3;
	case M_S_LESS:case M_S_LESS_EQUAL:case M_S_GREATER:case M_S_GREATER_EQUAL:								return 1;
	}
																											return 0;//
}
int Compile::default_overload(int S)//each bit marks correponding overload
{
	using namespace G2;
	if(S>M_FSTART&&S<M_BFSTART)//unary function
		return 0x02;//B0010		only 1 arg overload (no 0 arg overload)
	if(S<M_USER_FUNCTION)
		return 0x06;//B0110		1 arg & 2 arg overloads
/*	switch(S)
	{
	case M_COS:case M_ACOS:case M_COSH:case M_ACOSH:case M_COSC:
	case M_SEC:case M_ASEC:case M_SECH:case M_ASECH:
	case M_SIN:case M_ASIN:case M_SINH:case M_ASINH:case M_SINC:case M_SINHC:
	case M_CSC:case M_ACSC:case M_CSCH:case M_ACSCH:
	case M_TAN:			   case M_TANH:case M_ATANH:case M_TANC:
	case M_COT:case M_ACOT:case M_COTH:case M_ACOTH:
	case M_EXP:case M_LN:case M_SQRT:case M_CBRT:case M_INVSQRT:
	case M_GAUSS:case M_ERF:case M_FIB:case M_ZETA:
	case M_STEP:case M_SGN:case M_RECT:case M_TENT:
	case M_CEIL:case M_FLOOR:case M_ROUND:
	case M_ABS:case M_ARG:case M_REAL:case M_IMAG:case M_CONJUGATE:case M_POLAR:case M_CARTESIAN:
		return 0x02;//B0010		only 1 arg overload (no 0 arg overload)

	case M_RAND:
	case M_ATAN:
	case M_LOG:
	case M_BETA:case M_GAMMA:case M_PERMUTATION:case M_COMBINATION:
	case M_BESSEL_J:case M_BESSEL_Y:case M_HANKEL1:
	case M_SQWV:case M_TRWV:case M_SAW:case M_MIN:case M_MAX:
		return 0x06;//B0110		1 arg & 2 arg overloads
	}//*/
	return 0;
}
void Compile::compile_expression_global(Expression &expr)
{
	using namespace G2;
	procedural=false;//, recursiveFunction=false;
	Compile::expr=&expr;
	if(expr.m.size())
	{
		ub=new char[expr.m.size()];
		memset(ub, 0, expr.m.size()*sizeof(char));
	}
	else
		ub=0;
	if(expr.n.size())
	{
		term=(CompileTerm*)malloc(expr.n.size()*sizeof(CompileTerm));
		for(unsigned n=0;n<expr.n.size();++n)
		{
			auto &t=expr.n[n];
			term[n]=CompileTerm(t.constant, t.mathSet);
		}
	}
	else
		term=0;
	for(;;)
	{
		int topLevel=0, i=-1, f=-1;
		{
			bool peak=false;
			for(int k=0, kEnd=expr.m.size(), level=0;k<kEnd;++k)
			{
				if(expr.m[k]._0==M_LPR)
				{
					++level;
					if(level>=topLevel)
						topLevel=level, i=k+1, peak=true;
				}
				else if(expr.m[k]._0==M_RPR)
				{
					--level;
					if(peak)
						f=k, peak=false;
				}
			}
		}
			 if(i==-1)	i=0;
		else			expr.m[i-1]._0=M_IGNORED;
			 if(f==-1)	f=expr.m.size();
		else			expr.m[f  ]._0=M_IGNORED;
		bool callLevel=topLevel>0&&i-2>=0&&expr.m[i-2]._0>M_FSTART;
		for(int k=i;k<f;++k)//parse lazy calls
		{
			auto &S=expr.m[k]._0;				//sys 20150525 overloads and redefinitions
			if(S==M_USER_FUNCTION)//user function lazy call
			{
				std::vector<int> commas;
				int k2=k+1;
				for(;k2<f&&expr.m[k2]._0<M_FSTART;++k2)
					if(expr.m[k2]._0==M_COMMA)
						commas.push_back(k2);
				if(commas.size())//lazy call
				{
					int name_id=expr.m[k]._1, d_match=-1, exprNArgs=commas.size()+1;
					for(int d=0, dEnd=userFunctionDefinitions.size(), max_nArgs=-1;d<dEnd&&expr.lineNo>userFunctionDefinitions[d].lineNo;++d)//linearly through all user definitions
					{
						auto &definition=userFunctionDefinitions[d];
						if(definition.valid&&name_id==definition.name_id&&exprNArgs>=definition.nArgs&&max_nArgs<=definition.nArgs)
							d_match=d, max_nArgs=definition.nArgs;
					}
					if(d_match!=-1&&(!callLevel||exprNArgs==userFunctionDefinitions[d_match].nArgs))
				//	if(d_match!=-1)
					{
						std::vector<int> args(commas.size()+1);
						{
							int start=k;
							for(int k3=0, k3End=exprNArgs-1;k3<k3End;++k3)
							{
								expr.m[commas[k3]]._0=M_IGNORED;
								compile_assignment(start, commas[k3]);
								args[k3]=expr.m[start]._1, expr.m[start]._0=M_IGNORED;
								start=commas[k3]+1;
							}
							compile_assignment(start, f);
							args[exprNArgs-1]=expr.m[start]._1, expr.m[start]._0=M_IGNORED;
						}
						int result=compile_instruction_userFunctionCall(d_match, args);
						S=M_N, expr.m[k]._1=result;
					}
					else//syntax error, no such overload
					{
						expr.insertSyntaxError(expr.m[k].pos, expr.m[*commas.rbegin()].pos+1);//function id and 'arglist'
				//		expr.insertSyntaxError(expr.m[k].pos, expr.m[k].len);//mark the error		lazy call mismatch: highlight function id
				//	//	expr.insertSyntaxError(expr.m[k]._1);
						expr.i.clear();
						expr.valid=false;
						return;
					}
				}
			}
		//	else if(S>M_BINARY_FUNCTION_START)
			else if(S>M_BFSTART)//default binary function lazy call
			{
				std::vector<int> commas;
				int k2=k+1;
				for(;k2<f&&expr.m[k2]._0<M_FSTART;++k2)
					if(expr.m[k2]._0==M_COMMA)
						commas.push_back(k2);
				if(commas.size())
				{
					if(callLevel||commas.size()==1)
					{
						compile_inline_if(k+1, *commas.begin(), 0);
						int a2_start=commas[0]+1, a2_end=commas.size()>1?commas[1]:f;
						if(a2_start==a2_end)
							compile_instruction(k, '<', k+1);
						else
						{
							compile_inline_if(a2_start, a2_end, 3);
						//	compile_inline_if(commas[0]+1, commas.size()>1?commas[1]:f, 3);
						//	compile_inline_if(k+1, *commas.begin(), 0, true);
						//	compile_inline_if(commas[0]+1, commas.size()>1?commas[1]:f, 3, true);
							compile_instruction(k, '<', k+1, commas[0]+1);
						}
						S=G2::M_IGNORED, expr.m[commas[0]]._0=M_IGNORED;
					}
					else
					{
						expr.insertSyntaxError(expr.m[commas[1]].pos, expr.m[commas[1]].pos+1);//unexpected comma(s)
						expr.i.clear();
						expr.valid=false;
						return;
					}
				}
				else if(!(default_overload(S)&0x03))//no unary/void overload: call mismatch		later?
				{
				//	expr.insertSyntaxError(expr.m[k].pos, expr.m[k].len);//call mismatch, [function id]		default function: missing arguments get default values
					expr.i.clear();
					expr.valid=false;
					return;
				}
			}
		}
		if(callLevel)
	//	if(topLevel>0&&i-2>=0&&expr.m[i-2]._0>M_FSTART)
		{
			auto &S=expr.m[i-2]._0;
			std::vector<int> commas;
			bool notVoidCall=false;
			for(int k=i;k<f;++k)
			{
				auto &S2=expr.m[k]._0;
				if(S2==M_COMMA)
					commas.push_back(k);
				notVoidCall|=S2!=M_IGNORED;
			}
			int exprNArgs=commas.size()+notVoidCall;
			if(S<M_USER_FUNCTION)//default function call level
			{
				int signature=default_overload(S);
				if(signature&(1<<exprNArgs))//default overload match
				{
					int start=i;
					for(int k=0, kEnd=commas.size();k<kEnd;++k)
					{
						expr.m[commas[k]]._0=M_IGNORED;
						compile_inline_if(start, commas[k], 0);
					//	compile_inline_if(start+1, commas[k], 0, true);
						start=commas[k]+1;
					}
					compile_inline_if(start, f, 0);
				//	compile_inline_if(start+1, f, 0, true);
					if(exprNArgs==1)
						compile_instruction(i-2, '<', i);
					else if(exprNArgs==2)
						compile_instruction(i-2, '<', i, commas[0]+1);
					S=G2::M_IGNORED;
				}
				else//syntax error, no such overload
				{
					if(exprNArgs>signature)//too many args: highlight unexpected args
						expr.insertSyntaxError(expr.m[commas[signature>0x3]].pos, expr.m[f].pos);
				//	else//highlight arglist contents
				//		expr.insertSyntaxError(expr.m[i].pos, expr.m[f].pos);		//default function: missing args get default values
					//	expr.insertSyntaxError(expr.m[i-(i==f)].pos, expr.m[f].pos);
					expr.i.clear();
					expr.valid=false;
					return;
				}
			}
			else//user function call level
			{
				int name_id=expr.m[i-2]._1, d_match=-1;
				for(int d=0, dEnd=userFunctionDefinitions.size();d<dEnd&&expr.lineNo>userFunctionDefinitions[d].lineNo;++d)//linearly through all user definitions
				{
					auto &definition=userFunctionDefinitions[d];
					if(definition.valid&&name_id==definition.name_id&&exprNArgs==definition.nArgs)//find overload instance		sorted by lineNo
						d_match=d;
				}
				if(d_match!=-1)
				{
					std::vector<int> args(commas.size()+notVoidCall);
					if(notVoidCall)//compile args
					{
						int start=i;
						for(int k3=0, k3End=exprNArgs-1;k3<k3End;++k3)
						{
							expr.m[commas[k3]]._0=M_IGNORED;
							compile_assignment(start, commas[k3]);
							args[k3]=expr.m[start]._1, expr.m[start]._0=M_IGNORED;
							start=commas[k3]+1;
						}
						compile_assignment(start, f);
						args[exprNArgs-1]=expr.m[start]._1, expr.m[start]._0=M_IGNORED;
					}
					int result=compile_instruction_userFunctionCall(d_match, args);
					S=M_N, expr.m[i-2]._1=result;
				}
				else//syntax error, no such overload
				{
					expr.insertSyntaxError(expr.m[i-(i==f)].pos, expr.m[f].pos);//arglist
				//	expr.insertSyntaxError(expr.m[k]._1);//mark the error		function call level: highlight id, parentheses and arglist
					expr.i.clear();
					expr.valid=false;
					return;
				}
			}
		}
		else//not a call	or ground level
		{
			for(int k=i;k<f;++k)
			{
				if(expr.m[k]._0==M_COMMA)//unexpected comma
				{
					expr.insertSyntaxError(expr.m[k].pos, expr.m[k].pos+1);//just the comma
				//	expr.insertSyntaxError(expr.m[k]._1);//mark the error		unexpected commas: highlight
					if(omitExprWithUnexpectedCommas)
					{
						expr.i.clear();
						expr.valid=false;
						return;
					}
					expr.m[k]._0=M_IGNORED;
				//	break;
				}
			}
			compile_inline_if(i, f, 0);
		//	compile_inline_if(i, f, 0, true);
		}
		if(topLevel<=0)
			break;
	}
	delete[] ub;
	if(expr.n.size())
	{
		if(expr.i.size())
			expr.resultTerm=expr.i.rbegin()->result;
		predictedMathSet=term[expr.resultTerm].mathSet;
	}
	free(term);
}

//bool& operator|=(bool &a, bool const &b){return a=a|b;}
bool disc_conditional(Value const &x0, Value const &x1)
{
	using namespace G2;
	return _1d_zero_in_range(x0.r, x1.r)||_1d_zero_in_range(x0.i, x1.i)||_1d_zero_in_range(x0.j, x1.j)||_1d_zero_in_range(x0.k, x1.k);
}
void disc_1d_in_u	(Expression &ex, Instruction &in, int offset, int disc_offset, int x1, int x2, int y1, int y2, int z1, int z2, int Xplaces, int Yplaces, int Zplaces)
{
	auto &op1=ex.n[in.op1];
	auto &op1_r=op1.r, &op1_i=op1.i, &op1_j=op1.j, &op1_k=op1.k;
	switch(in.type)
	{
	case 1://r
		for(int v=x1, vEnd=x2-1;v<vEnd;++v)
			if(in.d.ud_i(Value(op1_r[v]), Value(op1_r[v+1])))
				ex.discontinuities[v]=true;
//			ex.discontinuities[v]|=in.d.ud_i(Value(op1_r[v]), Value(op1_r[v+1]));//no viable overloaded '|='
//		{
//			auto &condition=ex.discontinuities[v];//non-const lvalue reference to type 'bit_reference<...>' cannot bind to a temporary '__bit_reference<...>'
//			condition=condition||in.d.ud_i(Value(op1_r[v]), Value(op1_r[v+1]));
//		}
		break;
	case 2://c
		for(int v=x1, vEnd=x2-1;v<vEnd;++v)
			if(in.d.ud_i(Value(op1_r[v], op1_i[v]), Value(op1_r[v+1], op1_i[v+1])))
				ex.discontinuities[v]=true;
//		{
//			auto &condition=ex.discontinuities[v];
//			condition=condition||in.d.ud_i(Value(op1_r[v], op1_i[v]), Value(op1_r[v+1], op1_i[v+1]));
//		}
		break;
	case 3://q
		for(int v=x1, vEnd=x2-1;v<vEnd;++v)
			if(in.d.ud_i(Value(op1_r[v], op1_i[v], op1_j[v], op1_k[v]), Value(op1_r[v+1], op1_i[v+1], op1_j[v+1], op1_k[v+1])))
				ex.discontinuities[v]=true;
//		{
//			auto &condition=ex.discontinuities[v];
//			condition=condition||in.d.ud_i(Value(op1_r[v], op1_i[v], op1_j[v], op1_k[v]), Value(op1_r[v+1], op1_i[v+1], op1_j[v+1], op1_k[v+1]));
//		}
		break;
	}
}
void disc_1d_in_b	(Expression &ex, Instruction &in, int offset, int disc_offset, int x1, int x2, int y1, int y2, int z1, int z2, int Xplaces, int Yplaces, int Zplaces)
{
	auto &op1=ex.n[in.op1], &op2=ex.n[in.op2];
	auto &op1_r=op1.r, &op1_i=op1.i, &op1_j=op1.j, &op1_k=op1.k;
	auto &op2_r=op2.r, &op2_i=op2.i, &op2_j=op2.j, &op2_k=op2.k;
	switch(in.type)
	{
	case 4://rr
		for(int v=x1, vEnd=x2-1;v<vEnd;++v)
			if(in.d.bd_i(Value(op1_r[v]), Value(op2_r[v]), Value(op1_r[v+1]), Value(op2_r[v+1])))
				ex.discontinuities[v]=true;
//		{
//			auto &condition=ex.discontinuities[v];
//			condition=condition||in.d.bd_i(Value(op1_r[v]), Value(op2_r[v]), Value(op1_r[v+1]), Value(op2_r[v+1]));
//		}
		break;
	case 5://rc
		for(int v=x1, vEnd=x2-1;v<vEnd;++v)
			if(in.d.bd_i(Value(op1_r[v]), Value(op2_r[v], op2_i[v]), Value(op1_r[v+1]), Value(op2_r[v+1], op2_i[v+1])))
				ex.discontinuities[v]=true;
//		{
//			auto &condition=ex.discontinuities[v];
//			condition=condition||in.d.bd_i(Value(op1_r[v]), Value(op2_r[v], op2_i[v]), Value(op1_r[v+1]), Value(op2_r[v+1], op2_i[v+1]));
//		}
		break;
	case 6://rq
		for(int v=x1, vEnd=x2-1;v<vEnd;++v)
			if(in.d.bd_i(Value(op1_r[v]), Value(op2_r[v], op2_i[v], op2_j[v], op2_k[v]), Value(op1_r[v+1]), Value(op2_r[v+1], op2_i[v+1], op2_j[v+1], op2_k[v+1])))
				ex.discontinuities[v]=true;
//		{
//			auto &condition=ex.discontinuities[v];
//			condition=condition||in.d.bd_i(Value(op1_r[v]), Value(op2_r[v], op2_i[v], op2_j[v], op2_k[v]), Value(op1_r[v+1]), Value(op2_r[v+1], op2_i[v+1], op2_j[v+1], op2_k[v+1]));
//		}
		break;
	case 7://cr
		for(int v=x1, vEnd=x2-1;v<vEnd;++v)
			if(in.d.bd_i(Value(op1_r[v], op1_i[v]), Value(op2_r[v]), Value(op1_r[v+1], op1_i[v+1]), Value(op2_r[v+1])))
				ex.discontinuities[v]=true;
//		{
//			auto &condition=ex.discontinuities[v];
//			condition=condition||in.d.bd_i(Value(op1_r[v], op1_i[v]), Value(op2_r[v]), Value(op1_r[v+1], op1_i[v+1]), Value(op2_r[v+1]));
//		}
		break;
	case 8://cc
		for(int v=x1, vEnd=x2-1;v<vEnd;++v)
			if(in.d.bd_i(Value(op1_r[v], op1_i[v]), Value(op2_r[v], op2_i[v]), Value(op1_r[v+1], op1_i[v+1]), Value(op2_r[v+1], op2_i[v+1])))
				ex.discontinuities[v]=true;
//		{
//			auto &condition=ex.discontinuities[v];
//			condition=condition||in.d.bd_i(Value(op1_r[v], op1_i[v]), Value(op2_r[v], op2_i[v]), Value(op1_r[v+1], op1_i[v+1]), Value(op2_r[v+1], op2_i[v+1]));
//		}
		break;
	case 9://cq
		for(int v=x1, vEnd=x2-1;v<vEnd;++v)
			if(in.d.bd_i(Value(op1_r[v], op1_i[v]), Value(op2_r[v], op2_i[v], op2_j[v], op2_k[v]), Value(op1_r[v+1], op1_i[v+1]), Value(op2_r[v+1], op2_i[v+1], op2_j[v+1], op2_k[v+1])))
				ex.discontinuities[v]=true;
//		{
//			auto &condition=ex.discontinuities[v];
//			condition=condition||in.d.bd_i(Value(op1_r[v], op1_i[v]), Value(op2_r[v], op2_i[v], op2_j[v], op2_k[v]), Value(op1_r[v+1], op1_i[v+1]), Value(op2_r[v+1], op2_i[v+1], op2_j[v+1], op2_k[v+1]));
//		}
		break;
	case 10://qr
		for(int v=x1, vEnd=x2-1;v<vEnd;++v)
			if(in.d.bd_i(Value(op1_r[v], op1_i[v], op1_j[v], op1_k[v]), Value(op2_r[v]), Value(op1_r[v+1], op1_i[v+1], op1_j[v+1], op1_k[v+1]), Value(op2_r[v+1])))
				ex.discontinuities[v]=true;
//		{
//			auto &condition=ex.discontinuities[v];
//			condition=condition||in.d.bd_i(Value(op1_r[v], op1_i[v], op1_j[v], op1_k[v]), Value(op2_r[v]), Value(op1_r[v+1], op1_i[v+1], op1_j[v+1], op1_k[v+1]), Value(op2_r[v+1]));
//		}
		break;
	case 11://qc
		for(int v=x1, vEnd=x2-1;v<vEnd;++v)
			if(in.d.bd_i(Value(op1_r[v], op1_i[v], op1_j[v], op1_k[v]), Value(op2_r[v], op2_i[v]), Value(op1_r[v+1], op1_i[v+1], op1_j[v+1], op1_k[v+1]), Value(op2_r[v+1], op2_i[v+1])))
				ex.discontinuities[v]=true;
//		{
//			auto &condition=ex.discontinuities[v];
//			condition=condition||in.d.bd_i(Value(op1_r[v], op1_i[v], op1_j[v], op1_k[v]), Value(op2_r[v], op2_i[v]), Value(op1_r[v+1], op1_i[v+1], op1_j[v+1], op1_k[v+1]), Value(op2_r[v+1], op2_i[v+1]));
//		}
		break;
	case 12://qq
		for(int v=x1, vEnd=x2-1;v<vEnd;++v)
			if(in.d.bd_i(Value(op1_r[v], op1_i[v], op1_j[v], op1_k[v]), Value(op2_r[v], op2_i[v], op2_j[v], op2_k[v]),
				Value(op1_r[v+1], op1_i[v+1], op1_j[v+1], op1_k[v+1]), Value(op2_r[v+1], op2_i[v+1], op2_j[v+1], op2_k[v+1])))
				ex.discontinuities[v]=true;
//		{
//			auto &condition=ex.discontinuities[v];
//			condition=condition||in.d.bd_i(Value(op1_r[v], op1_i[v], op1_j[v], op1_k[v]), Value(op2_r[v], op2_i[v], op2_j[v], op2_k[v]),
//				Value(op1_r[v+1], op1_i[v+1], op1_j[v+1], op1_k[v+1]), Value(op2_r[v+1], op2_i[v+1], op2_j[v+1], op2_k[v+1]));
//		}
		break;
	//case 13://a ? b : c
	//	break;
	}
}
void disc_1d_in_t	(Expression &ex, Instruction &in, int offset, int disc_offset, int x1, int x2, int y1, int y2, int z1, int z2, int Xplaces, int Yplaces, int Zplaces)
{
	auto &op1=ex.n[in.op1];
	auto &op1_r=op1.r, &op1_i=op1.i, &op1_j=op1.j, &op1_k=op1.k;
	switch(in.type)
	{
	case 13://a ? b : c
		switch(maximum(in.op2_ms, in.op3_ms))
		{
		case 'R':
			for(int v=x1, vEnd=x2-1;v<vEnd;++v)
				if(disc_conditional(Value(op1_r[v]), Value(op1_r[v+1])))
					ex.discontinuities[v]=true;
//			{
//				auto &condition=ex.discontinuities[v];
//				condition=condition||disc_conditional(Value(op1_r[v]), Value(op1_r[v+1]));
//			}
			break;
		case 'c':
			for(int v=x1, vEnd=x2-1;v<vEnd;++v)
				if(disc_conditional(Value(op1_r[v], op1_i[v]), Value(op1_r[v+1], op1_i[v+1])))
					ex.discontinuities[v]=true;
//			{
//				auto &condition=ex.discontinuities[v];
//				condition=condition||disc_conditional(Value(op1_r[v], op1_i[v]), Value(op1_r[v+1], op1_i[v+1]));
//			}
			break;
		case 'h':
			for(int v=x1, vEnd=x2-1;v<vEnd;++v)
				if(disc_conditional(Value(op1_r[v], op1_i[v], op1_j[v], op1_k[v]), Value(op1_r[v+1], op1_i[v+1], op1_j[v+1], op1_k[v+1])))
					ex.discontinuities[v]=true;
//			{
//				auto &condition=ex.discontinuities[v];
//				condition=condition||disc_conditional(Value(op1_r[v], op1_i[v], op1_j[v], op1_k[v]), Value(op1_r[v+1], op1_i[v+1], op1_j[v+1], op1_k[v+1]));
//			}
		break;
		}
	}
}
void disc_1d_out	(Expression &ex, Instruction &in, int offset, int disc_offset, int x1, int x2, int y1, int y2, int z1, int z2, int Xplaces, int Yplaces, int Zplaces)
{
	bool other=in.type==5||in.type==6||in.type==9;
	auto &op1=ex.n[other?in.op2:in.op1];
	auto &op1_r=op1.r, &op1_i=op1.i, &op1_j=op1.j, &op1_k=op1.k;
	switch(other?in.op2_ms:in.op1_ms)
	{
	case 'R':
		for(int v=x1;v<x2-1;++v)
			if(in.d.d_o(Value(op1_r[v]), Value(op1_r[v+1])))
				ex.discontinuities[v]=true;
//		{
//			auto &condition=ex.discontinuities[v];
//			condition=condition||in.d.d_o(Value(op1_r[v]), Value(op1_r[v+1]));
//		}
		break;
	case 'c':
		for(int v=x1;v<x2-1;++v)
			if(in.d.d_o(Value(op1_r[v], op1_i[v]), Value(op1_r[v+1], op1_i[v+1])))
				ex.discontinuities[v]=true;
//		{
//			auto &condition=ex.discontinuities[v];
//			condition=condition||in.d.d_o(Value(op1_r[v], op1_i[v]), Value(op1_r[v+1], op1_i[v+1]));
//		}
		break;
	case 'h':
		for(int v=x1;v<x2-1;++v)
			if(in.d.d_o(Value(op1_r[v], op1_i[v], op1_j[v], op1_k[v]), Value(op1_r[v+1], op1_i[v+1], op1_j[v+1], op1_k[v+1])))
				ex.discontinuities[v]=true;
//		{
//			auto &condition=ex.discontinuities[v];
//			condition=condition||in.d.d_o(Value(op1_r[v], op1_i[v], op1_j[v], op1_k[v]), Value(op1_r[v+1], op1_i[v+1], op1_j[v+1], op1_k[v+1]));
//		}
		break;
	}
}

void disc_i2d_in_u	(Expression &ex, Instruction &in, int offset, int disc_offset, int x1, int x2, int y1, int y2, int z1, int z2, int aXplaces, int aYplaces, int Zplaces)
{
	auto &op1=ex.n[in.op1];
	auto &op1_r=op1.r, &op1_i=op1.i, &op1_j=op1.j, &op1_k=op1.k;
	int Xplaces=aXplaces-2, Yplaces=aYplaces-2;
	for(int y=y1;y<y2;++y)
	{
		for(int x=x1;x<x2-1;++x)
		{
			unsigned v=(Xplaces+2)*y+x;
			//if(v>=op1.ndr.size()||(Xplaces+1)*y+x>=ex.discontinuities.size())
			//	int LOL_1=0;
			if(in.d.ud_i(
				Value(op1_r[v  ], op1_i.size()?op1_i[v  ]:0, op1_j.size()?op1_j[v  ]:0, op1_k.size()?op1_k[v  ]:0),
				Value(op1_r[v+1], op1_i.size()?op1_i[v+1]:0, op1_j.size()?op1_j[v+1]:0, op1_k.size()?op1_k[v+1]:0)))
				ex.discontinuities[(Xplaces+1)*y+x]=true;
//			auto &condition=ex.discontinuities[(Xplaces+1)*y+x];
//			condition=condition||in.d.ud_i(
//				Value(op1_r[v  ], op1_i.size()?op1_i[v  ]:0, op1_j.size()?op1_j[v  ]:0, op1_k.size()?op1_k[v  ]:0),
//				Value(op1_r[v+1], op1_i.size()?op1_i[v+1]:0, op1_j.size()?op1_j[v+1]:0, op1_k.size()?op1_k[v+1]:0));
		}
	}
	for(int y=y1;y<y2-1;++y)
	{
		for(int x=x1;x<x2;++x)
		{
			unsigned v=(Xplaces+2)*y+x;
			if(in.d.ud_i(
				Value(op1_r[v			], op1_i.size()?op1_i[v				]:0, op1_j.size()?op1_j[v			]:0, op1_k.size()?op1_k[v			]:0),
				Value(op1_r[v+Xplaces+2	], op1_i.size()?op1_i[v+Xplaces+2	]:0, op1_j.size()?op1_j[v+Xplaces+2	]:0, op1_k.size()?op1_k[v+Xplaces+2	]:0)))
				ex.discontinuities[disc_offset+v]=true;
//			auto &condition=ex.discontinuities[disc_offset+v];
//			condition=condition||in.d.ud_i(
//				Value(op1_r[v			], op1_i.size()?op1_i[v				]:0, op1_j.size()?op1_j[v			]:0, op1_k.size()?op1_k[v			]:0),
//				Value(op1_r[v+Xplaces+2	], op1_i.size()?op1_i[v+Xplaces+2	]:0, op1_j.size()?op1_j[v+Xplaces+2	]:0, op1_k.size()?op1_k[v+Xplaces+2	]:0));
		}
	}
}
void disc_i2d_in_b	(Expression &ex, Instruction &in, int offset, int disc_offset, int x1, int x2, int y1, int y2, int z1, int z2, int aXplaces, int aYplaces, int Zplaces)
{
	auto &op1=ex.n[in.op1], &op2=ex.n[in.op2];
	auto &op1_r=op1.r, &op1_i=op1.i, &op1_j=op1.j, &op1_k=op1.k;
	auto &op2_r=op2.r, &op2_i=op2.i, &op2_j=op2.j, &op2_k=op2.k;
	int Xplaces=aXplaces-2, Yplaces=aYplaces-2;
	for(int y=y1;y<y2;++y)
	{
		for(int x=x1;x<x2-1;++x)
		{
			unsigned v=(Xplaces+2)*y+x;
			if(in.d.bd_i(
				Value(op1_r[v], op1_i.size()?op1_i[v]:0, op1_j.size()?op1_j[v]:0, op1_k.size()?op1_k[v]:0),
				Value(op2_r[v], op2_i.size()?op2_i[v]:0, op2_j.size()?op2_j[v]:0, op2_k.size()?op2_k[v]:0),
				Value(op1_r[v+1], op1_i.size()?op1_i[v+1]:0, op1_j.size()?op1_j[v+1]:0, op1_k.size()?op1_k[v+1]:0),
				Value(op2_r[v+1], op2_i.size()?op2_i[v+1]:0, op2_j.size()?op2_j[v+1]:0, op2_k.size()?op2_k[v+1]:0)))
				ex.discontinuities[(Xplaces+1)*y+x]=true;
//			auto &condition=ex.discontinuities[(Xplaces+1)*y+x];
//			condition=condition||in.d.bd_i(
//				Value(op1_r[v], op1_i.size()?op1_i[v]:0, op1_j.size()?op1_j[v]:0, op1_k.size()?op1_k[v]:0),
//				Value(op2_r[v], op2_i.size()?op2_i[v]:0, op2_j.size()?op2_j[v]:0, op2_k.size()?op2_k[v]:0),
//				Value(op1_r[v+1], op1_i.size()?op1_i[v+1]:0, op1_j.size()?op1_j[v+1]:0, op1_k.size()?op1_k[v+1]:0),
//				Value(op2_r[v+1], op2_i.size()?op2_i[v+1]:0, op2_j.size()?op2_j[v+1]:0, op2_k.size()?op2_k[v+1]:0));
		}
	}
	for(int y=y1;y<y2-1;++y)
	{
		for(int x=x1;x<x2;++x)
		{
			unsigned v=(Xplaces+2)*y+x;
			if(in.d.bd_i(
				Value(op1_r[v], op1_i.size()?op1_i[v]:0, op1_j.size()?op1_j[v]:0, op1_k.size()?op1_k[v]:0),
				Value(op2_r[v], op2_i.size()?op2_i[v]:0, op2_j.size()?op2_j[v]:0, op2_k.size()?op2_k[v]:0),
				Value(op1_r[v+Xplaces+2], op1_i.size()?op1_i[v+Xplaces+2]:0, op1_j.size()?op1_j[v+Xplaces+2]:0, op1_k.size()?op1_k[v+Xplaces+2]:0),
				Value(op2_r[v+Xplaces+2], op2_i.size()?op2_i[v+Xplaces+2]:0, op2_j.size()?op2_j[v+Xplaces+2]:0, op2_k.size()?op2_k[v+Xplaces+2]:0)))
				ex.discontinuities[disc_offset+v]=true;
		}
	}
}
void disc_i2d_in_t	(Expression &ex, Instruction &in, int offset, int disc_offset, int x1, int x2, int y1, int y2, int z1, int z2, int aXplaces, int aYplaces, int Zplaces)
{
	auto &op1=ex.n[in.op1];
	auto &op1_r=op1.r, &op1_i=op1.i, &op1_j=op1.j, &op1_k=op1.k;
	int Xplaces=aXplaces-2, Yplaces=aYplaces-2;
	for(int y=y1;y<y2;++y)
	{
		for(int x=x1;x<x2-1;++x)
		{
			unsigned v=(Xplaces+2)*y+x;
			if(disc_conditional(
				Value(op1_r[v  ], op1_i.size()?op1_i[v  ]:0, op1_j.size()?op1_j[v  ]:0, op1_k.size()?op1_k[v  ]:0),
				Value(op1_r[v+1], op1_i.size()?op1_i[v+1]:0, op1_j.size()?op1_j[v+1]:0, op1_k.size()?op1_k[v+1]:0)))
				ex.discontinuities[(Xplaces+1)*y+x]=true;
		}
	}
	for(int y=y1;y<y2-1;++y)
	{
		for(int x=x1;x<x2;++x)
		{
			unsigned v=(Xplaces+2)*y+x;
			if(disc_conditional(
				Value(op1_r[v			], op1_i.size()?op1_i[v				]:0, op1_j.size()?op1_j[v			]:0, op1_k.size()?op1_k[v			]:0),
				Value(op1_r[v+Xplaces+2	], op1_i.size()?op1_i[v+Xplaces+2	]:0, op1_j.size()?op1_j[v+Xplaces+2	]:0, op1_k.size()?op1_k[v+Xplaces+2	]:0)))
				ex.discontinuities[disc_offset+v]=true;
		}
	}
}
void disc_i2d_out	(Expression &ex, Instruction &in, int offset, int disc_offset, int x1, int x2, int y1, int y2, int z1, int z2, int aXplaces, int aYplaces, int Zplaces)
{
	bool other=in.type==5||in.type==6||in.type==9;
	auto &op1=ex.n[other?in.op2:in.op1];
	auto &op1_r=op1.r, &op1_i=op1.i, &op1_j=op1.j, &op1_k=op1.k;
	int Xplaces=aXplaces-2, Yplaces=aYplaces-2;
	for(int y=y1;y<y2;++y)
	{
		for(int x=x1;x<x2-1;++x)
		{
			unsigned v=(Xplaces+2)*y+x;
			if(in.d.d_o(
				Value(op1_r[v  ], op1_i.size()?op1_i[v  ]:0, op1_j.size()?op1_j[v  ]:0, op1_k.size()?op1_k[v  ]:0),
				Value(op1_r[v+1], op1_i.size()?op1_i[v+1]:0, op1_j.size()?op1_j[v+1]:0, op1_k.size()?op1_k[v+1]:0)))
				ex.discontinuities[(Xplaces+1)*y+x]=true;
		}
	}
	for(int y=y1;y<y2-1;++y)
	{
		for(int x=x1;x<x2;++x)
		{
			unsigned v=(Xplaces+2)*y+x;
			if(in.d.d_o(
				Value(op1_r[v			], op1_i.size()?op1_i[v				]:0, op1_j.size()?op1_j[v			]:0, op1_k.size()?op1_k[v			]:0),
				Value(op1_r[v+Xplaces+2	], op1_i.size()?op1_i[v+Xplaces+2	]:0, op1_j.size()?op1_j[v+Xplaces+2	]:0, op1_k.size()?op1_k[v+Xplaces+2	]:0)))
				ex.discontinuities[disc_offset+v]=true;
		}
	}
}

void disc_2d_in_u	(Expression &ex, Instruction &in, int offset, int disc_offset, int x1, int x2, int y1, int y2, int z1, int z2, int Xplaces, int Yplaces, int Zplaces)
{
	auto &op1=ex.n[in.op1];
	auto &op1_r=op1.r, &op1_i=op1.i, &op1_j=op1.j, &op1_k=op1.k;
//	unsigned yDiscOffset=(Xplaces-1)*Yplaces;
	for(int y=y1;y<y2;++y)
	{
		for(int x=x1;x<x2-1;++x)
		{
			unsigned v=Xplaces*y+x;
			if(in.d.ud_i(
				Value(op1_r[v  ], op1_i.size()?op1_i[v  ]:0, op1_j.size()?op1_j[v  ]:0, op1_k.size()?op1_k[v  ]:0),
				Value(op1_r[v+1], op1_i.size()?op1_i[v+1]:0, op1_j.size()?op1_j[v+1]:0, op1_k.size()?op1_k[v+1]:0)))
				ex.discontinuities[(Xplaces-1)*y+x]=true;
		}
	}
	for(int x=x1;x<x2;++x)
	{
		for(int y=y1;y<y2-1;++y)
		{
			unsigned v=Xplaces*y+x;
			if(in.d.ud_i(
				Value(op1_r[v			], op1_i.size()?op1_i[v			]:0, op1_j.size()?op1_j[v			]:0, op1_k.size()?op1_k[v			]:0),
				Value(op1_r[v+Xplaces	], op1_i.size()?op1_i[v+Xplaces	]:0, op1_j.size()?op1_j[v+Xplaces	]:0, op1_k.size()?op1_k[v+Xplaces	]:0)))
				ex.discontinuities[disc_offset+(Yplaces-1)*x+y]=true;
		}
	}
}
void disc_2d_in_b	(Expression &ex, Instruction &in, int offset, int disc_offset, int x1, int x2, int y1, int y2, int z1, int z2, int Xplaces, int Yplaces, int Zplaces)
{
	auto &op1=ex.n[in.op1], &op2=ex.n[in.op2];
	auto &op1_r=op1.r, &op1_i=op1.i, &op1_j=op1.j, &op1_k=op1.k;
	auto &op2_r=op2.r, &op2_i=op2.i, &op2_j=op2.j, &op2_k=op2.k;
//	unsigned yDiscOffset=(Xplaces-1)*Yplaces;
	for(int y=y1;y<y2;++y)
	{
		for(int x=x1;x<x2-1;++x)
		{
			unsigned v=Xplaces*y+x;
			if(in.d.bd_i(
				Value(op1_r[v  ], op1_i.size()?op1_i[v  ]:0, op1_j.size()?op1_j[v  ]:0, op1_k.size()?op1_k[v  ]:0),
				Value(op2_r[v  ], op2_i.size()?op2_i[v  ]:0, op2_j.size()?op2_j[v  ]:0, op2_k.size()?op2_k[v  ]:0),
				Value(op1_r[v+1], op1_i.size()?op1_i[v+1]:0, op1_j.size()?op1_j[v+1]:0, op1_k.size()?op1_k[v+1]:0),
				Value(op2_r[v+1], op2_i.size()?op2_i[v+1]:0, op2_j.size()?op2_j[v+1]:0, op2_k.size()?op2_k[v+1]:0)))
				ex.discontinuities[(Xplaces-1)*y+x]=true;
		}
	}
	for(int x=x1;x<x2;++x)
	{
		for(int y=y1;y<y2-1;++y)
		{
			unsigned v=Xplaces*y+x;
			if(in.d.bd_i(
				Value(op1_r[v], op1_i.size()?op1_i[v]:0, op1_j.size()?op1_j[v]:0, op1_k.size()?op1_k[v]:0),
				Value(op2_r[v], op2_i.size()?op2_i[v]:0, op2_j.size()?op2_j[v]:0, op2_k.size()?op2_k[v]:0),
				Value(op1_r[v+Xplaces], op1_i.size()?op1_i[v+Xplaces]:0, op1_j.size()?op1_j[v+Xplaces]:0, op1_k.size()?op1_k[v+Xplaces]:0),
				Value(op2_r[v+Xplaces], op2_i.size()?op2_i[v+Xplaces]:0, op2_j.size()?op2_j[v+Xplaces]:0, op2_k.size()?op2_k[v+Xplaces]:0)))
				ex.discontinuities[disc_offset+(Yplaces-1)*x+y]=true;
		}
	}
}
void disc_2d_in_t	(Expression &ex, Instruction &in, int offset, int disc_offset, int x1, int x2, int y1, int y2, int z1, int z2, int Xplaces, int Yplaces, int Zplaces)
{
	auto &op1=ex.n[in.op1];
	auto &op1_r=op1.r, &op1_i=op1.i, &op1_j=op1.j, &op1_k=op1.k;
//	unsigned yDiscOffset=(Xplaces-1)*Yplaces;
	for(int y=y1;y<y2;++y)
	{
		for(int x=x1;x<x2-1;++x)
		{
			unsigned v=Xplaces*y+x;
			if(disc_conditional(
				Value(op1_r[v  ], op1_i.size()?op1_i[v  ]:0, op1_j.size()?op1_j[v  ]:0, op1_k.size()?op1_k[v  ]:0),
				Value(op1_r[v+1], op1_i.size()?op1_i[v+1]:0, op1_j.size()?op1_j[v+1]:0, op1_k.size()?op1_k[v+1]:0)))
				ex.discontinuities[(Xplaces-1)*y+x]=true;
		}
	}
	for(int x=x1;x<x2;++x)
	{
		for(int y=y1;y<y2-1;++y)
		{
			unsigned v=Xplaces*y+x;
			if(disc_conditional(
				Value(op1_r[v], op1_i.size()?op1_i[v]:0, op1_j.size()?op1_j[v]:0, op1_k.size()?op1_k[v]:0),
				Value(op1_r[v+Xplaces], op1_i.size()?op1_i[v+Xplaces]:0, op1_j.size()?op1_j[v+Xplaces]:0, op1_k.size()?op1_k[v+Xplaces]:0)))
				ex.discontinuities[disc_offset+(Yplaces-1)*x+y]=true;
		}
	}
}
void disc_2d_out	(Expression &ex, Instruction &in, int offset, int disc_offset, int x1, int x2, int y1, int y2, int z1, int z2, int Xplaces, int Yplaces, int Zplaces)
{
	auto &op1=ex.n[in.result];
	auto &op1_r=op1.r, &op1_i=op1.i, &op1_j=op1.j, &op1_k=op1.k;
//	unsigned yDiscOffset=(Xplaces-1)*Yplaces;
	for(int y=y1;y<y2;++y)
	{
		for(int x=x1;x<x2-1;++x)
		{
			unsigned v=Xplaces*y+x;
			if(in.d.d_o(
				Value(op1_r[v  ], op1_i.size()?op1_i[v  ]:0, op1_j.size()?op1_j[v  ]:0, op1_k.size()?op1_k[v  ]:0),
				Value(op1_r[v+1], op1_i.size()?op1_i[v+1]:0, op1_j.size()?op1_j[v+1]:0, op1_k.size()?op1_k[v+1]:0)))
				ex.discontinuities[(Xplaces-1)*y+x]=true;
		}
	}
	for(int x=x1;x<x2;++x)
	{
		for(int y=y1;y<y2-1;++y)
		{
			unsigned v=Xplaces*y+x;
			if(in.d.d_o(
				Value(op1_r[v], op1_i.size()?op1_i[v]:0, op1_j.size()?op1_j[v]:0, op1_k.size()?op1_k[v]:0),
				Value(op1_r[v+Xplaces], op1_i.size()?op1_i[v+Xplaces]:0, op1_j.size()?op1_j[v+Xplaces]:0, op1_k.size()?op1_k[v+Xplaces]:0)))
				ex.discontinuities[disc_offset+(Yplaces-1)*x+y]=true;
		}
	}
}

void disc_l2d_X_in_u(Expression &ex, Instruction &in, int offset, int disc_offset, int x1, int x2, int y1, int y2, int z1, int z2, int Xplaces, int Yplaces, int Zplaces)
{
	auto &op1=ex.n[in.op1];
	auto &op1_r=op1.r, &op1_i=op1.i, &op1_j=op1.j, &op1_k=op1.k;
	for(int y=y1;y<y2;++y)
	{
		for(int x=x1;x<x2-1;++x)
		{
			unsigned v=Xplaces*y+x;
			if(in.d.ud_i(
				Value(op1_r[v  ], op1_i.size()?op1_i[v  ]:0, op1_j.size()?op1_j[v  ]:0, op1_k.size()?op1_k[v  ]:0),
				Value(op1_r[v+1], op1_i.size()?op1_i[v+1]:0, op1_j.size()?op1_j[v+1]:0, op1_k.size()?op1_k[v+1]:0)))
				ex.discontinuities[(Xplaces-1)*y+x]=true;
		//	if(in.d.ud_i(op1.ndr[v], op1.ndr[v+1]);
		}
	}
}
void disc_l2d_Y_in_u(Expression &ex, Instruction &in, int offset, int disc_offset, int y1, int y2, int x1, int x2, int z1, int z2, int Yplaces, int Xplaces, int Zplaces)//x and y swapped
{
	auto &op1=ex.n[in.op1];
	auto &op1_r=op1.r, &op1_i=op1.i, &op1_j=op1.j, &op1_k=op1.k;
	for(int x=x1;x<x2;++x)
	{
		for(int y=y1;y<y2-1;++y)
		{
			unsigned v=offset+Yplaces*x+y;
			if(in.d.ud_i(
				Value(op1_r[v  ], op1_i.size()?op1_i[v  ]:0, op1_j.size()?op1_j[v  ]:0, op1_k.size()?op1_k[v  ]:0),
				Value(op1_r[v+1], op1_i.size()?op1_i[v+1]:0, op1_j.size()?op1_j[v+1]:0, op1_k.size()?op1_k[v+1]:0)))
				ex.discontinuities[disc_offset+(Yplaces-1)*x+y]=true;
		}
	}
}
void disc_l2d_X_in_b(Expression &ex, Instruction &in, int offset, int disc_offset, int x1, int x2, int y1, int y2, int z1, int z2, int Xplaces, int Yplaces, int Zplaces)
{
	auto &op1=ex.n[in.op1], &op2=ex.n[in.op2];
	auto &op1_r=op1.r, &op1_i=op1.i, &op1_j=op1.j, &op1_k=op1.k;
	auto &op2_r=op2.r, &op2_i=op2.i, &op2_j=op2.j, &op2_k=op2.k;
	for(int y=y1;y<y2;++y)
	{
		for(int x=x1;x<x2-1;++x)
		{
			unsigned v=Xplaces*y+x;
			if(in.d.bd_i(
				Value(op1_r[v  ], op1_i.size()?op1_i[v  ]:0, op1_j.size()?op1_j[v  ]:0, op1_k.size()?op1_k[v  ]:0),
				Value(op2_r[v  ], op2_i.size()?op2_i[v  ]:0, op2_j.size()?op2_j[v  ]:0, op2_k.size()?op2_k[v  ]:0),
				Value(op1_r[v+1], op1_i.size()?op1_i[v+1]:0, op1_j.size()?op1_j[v+1]:0, op1_k.size()?op1_k[v+1]:0),
				Value(op2_r[v+1], op2_i.size()?op2_i[v+1]:0, op2_j.size()?op2_j[v+1]:0, op2_k.size()?op2_k[v+1]:0)))
				ex.discontinuities[(Xplaces-1)*y+x]=true;
		//	condition=condition||in.d.bd_i(op1.ndr[v], op2.ndr[v], op1.ndr[v+1], op2.ndr[v+1]);
		}
	}
}
void disc_l2d_Y_in_b(Expression &ex, Instruction &in, int offset, int disc_offset, int y1, int y2, int x1, int x2, int z1, int z2, int Yplaces, int Xplaces, int Zplaces)//x and y swapped
{
	auto &op1=ex.n[in.op1], &op2=ex.n[in.op2];
	auto &op1_r=op1.r, &op1_i=op1.i, &op1_j=op1.j, &op1_k=op1.k;
	auto &op2_r=op2.r, &op2_i=op2.i, &op2_j=op2.j, &op2_k=op2.k;
	for(int x=x1;x<x2;++x)
	{
		for(int y=y1;y<y2-1;++y)
		{
			unsigned v=offset+Yplaces*x+y;
			if(in.d.bd_i(
				Value(op1_r[v  ], op1_i.size()?op1_i[v  ]:0, op1_j.size()?op1_j[v  ]:0, op1_k.size()?op1_k[v  ]:0),
				Value(op2_r[v  ], op2_i.size()?op2_i[v  ]:0, op2_j.size()?op2_j[v  ]:0, op2_k.size()?op2_k[v  ]:0),
				Value(op1_r[v+1], op1_i.size()?op1_i[v+1]:0, op1_j.size()?op1_j[v+1]:0, op1_k.size()?op1_k[v+1]:0),
				Value(op2_r[v+1], op2_i.size()?op2_i[v+1]:0, op2_j.size()?op2_j[v+1]:0, op2_k.size()?op2_k[v+1]:0)))
				ex.discontinuities[disc_offset+(Yplaces-1)*x+y]=true;
		//	condition=condition||in.d.bd_i(op1.ndr[v], op2.ndr[v], op1.ndr[v+1], op2.ndr[v+1]);
		}
	}
}
void disc_l2d_X_in_t(Expression &ex, Instruction &in, int offset, int disc_offset, int x1, int x2, int y1, int y2, int z1, int z2, int Xplaces, int Yplaces, int Zplaces)
{
	auto &op1=ex.n[in.op1];
	auto &op1_r=op1.r, &op1_i=op1.i, &op1_j=op1.j, &op1_k=op1.k;
	for(int y=y1;y<y2;++y)
	{
		for(int x=x1;x<x2-1;++x)
		{
			unsigned v=Xplaces*y+x;
			if(disc_conditional(
				Value(op1_r[v  ], op1_i.size()?op1_i[v  ]:0, op1_j.size()?op1_j[v  ]:0, op1_k.size()?op1_k[v  ]:0),
				Value(op1_r[v+1], op1_i.size()?op1_i[v+1]:0, op1_j.size()?op1_j[v+1]:0, op1_k.size()?op1_k[v+1]:0)))
				ex.discontinuities[(Xplaces-1)*y+x]=true;
		//	condition=condition||in.d.td_i(op1.ndr[v], op2.ndr[v], op3.ndr[v], op1.ndr[v+1], op2.ndr[v+1], op3.ndr[v+1]);
		}
	}
}
void disc_l2d_Y_in_t(Expression &ex, Instruction &in, int offset, int disc_offset, int y1, int y2, int x1, int x2, int z1, int z2, int Yplaces, int Xplaces, int Zplaces)//x and y swapped
{
	auto &op1=ex.n[in.op1];
	auto &op1_r=op1.r, &op1_i=op1.i, &op1_j=op1.j, &op1_k=op1.k;
	for(int x=x1;x<x2;++x)
	{
		for(int y=y1;y<y2-1;++y)
		{
			unsigned v=offset+Yplaces*x+y;
			if(disc_conditional(
				Value(op1_r[v  ], op1_i.size()?op1_i[v  ]:0, op1_j.size()?op1_j[v  ]:0, op1_k.size()?op1_k[v  ]:0),
				Value(op1_r[v+1], op1_i.size()?op1_i[v+1]:0, op1_j.size()?op1_j[v+1]:0, op1_k.size()?op1_k[v+1]:0)))
				ex.discontinuities[disc_offset+(Yplaces-1)*x+y]=true;
		//	condition=in.d.td_i(op1.ndr[v], op2.ndr[v], op3.ndr[v], op1.ndr[v+1], op2.ndr[v+1], op3.ndr[v+1]);
		}
	}
}
void disc_l2d_X_out	(Expression &ex, Instruction &in, int offset, int disc_offset, int x1, int x2, int y1, int y2, int z1, int z2, int Xplaces, int Yplaces, int Zplaces)
{
	auto &op1=ex.n[in.result];
	auto &op1_r=op1.r, &op1_i=op1.i, &op1_j=op1.j, &op1_k=op1.k;
	for(int y=y1;y<y2;++y)
	{
		for(int x=x1;x<x2-1;++x)
		{
			unsigned v=Xplaces*y+x;
			if(in.d.d_o(
				Value(op1_r[v  ], op1_i.size()?op1_i[v  ]:0, op1_j.size()?op1_j[v  ]:0, op1_k.size()?op1_k[v  ]:0),
				Value(op1_r[v+1], op1_i.size()?op1_i[v+1]:0, op1_j.size()?op1_j[v+1]:0, op1_k.size()?op1_k[v+1]:0)))
				ex.discontinuities[(Xplaces-1)*y+x]=true;
		//	condition=condition||in.d.d_o(result.ndr[v], result.ndr[v+1]);
		}
	}
}
void disc_l2d_Y_out	(Expression &ex, Instruction &in, int offset, int disc_offset, int y1, int y2, int x1, int x2, int z1, int z2, int Yplaces, int Xplaces, int Zplaces)//x and y swapped
{
	auto &op1=ex.n[in.result];
	auto &op1_r=op1.r, &op1_i=op1.i, &op1_j=op1.j, &op1_k=op1.k;
	for(int x=x1;x<x2;++x)
	{
		for(int y=y1;y<y2-1;++y)
		{
			unsigned v=offset+Yplaces*x+y;
			if(in.d.d_o(
				Value(op1_r[v  ], op1_i.size()?op1_i[v  ]:0, op1_j.size()?op1_j[v  ]:0, op1_k.size()?op1_k[v  ]:0),
				Value(op1_r[v+1], op1_i.size()?op1_i[v+1]:0, op1_j.size()?op1_j[v+1]:0, op1_k.size()?op1_k[v+1]:0)))
				ex.discontinuities[disc_offset+(Yplaces-1)*x+y]=true;
		//	condition=in.d.d_o(result.ndr[v], result.ndr[v+1]);
		}
	}
}
typedef void (*Disc_fn)(Expression &ex, Instruction &in, int offset, int disc_offset, int x1, int x2, int y1, int y2, int z1, int z2, int Xplaces, int Yplaces, int Zplaces);
inline int ceil_half(int x){return (x>>1)+(x&1);}
void solve_disc(Expression &ex, int offset, int x1, int x2, int y1, int y2, int z1, int z2, int Xplaces, int Yplaces, int Zplaces, Disc_fn disc_in_u, Disc_fn disc_in_b, Disc_fn disc_in_t, Disc_fn disc_out, int disc_offset, bool skip_last_instr)
{
	int dx=x2-x1, dy=y2-y1, dz=z2-z1, workSize=dx*dy*dz;
//	int ndrSize=Xplaces*Yplaces*Zplaces;
	int ndrSize=ex.n[0].r.size();
	for(int i=0, iEnd=ex.i.size()-skip_last_instr;i<iEnd;++i)
	{
		auto &in=ex.i[i];
		//for(int kn=0;kn<ex.n.size();++kn)
		//	ndr_to_clipboard_2d((double*)ex.n[kn].r, Xplaces0, Yplaces);//
		if(in.d.disc_in)
		{
			if(in.type<=3)
			{
				if(disc_in_u)
					disc_in_u(ex, in, offset, disc_offset, x1, x2, y1, y2, z1, z2, Xplaces, Yplaces, Zplaces);
			}
			else if(in.type<=26)
			{
				if(disc_in_b)
					disc_in_b(ex, in, offset, disc_offset, x1, x2, y1, y2, z1, z2, Xplaces, Yplaces, Zplaces);
			}
			else if(in.type==27&&disc_in_t!=0)
				disc_in_t(ex, in, offset, disc_offset, x1, x2, y1, y2, z1, z2, Xplaces, Yplaces, Zplaces);
		}
		switch(in.type)
		{
		case 'c':
			{
				Solve_UserFunction uf(ex, in, false);
				for(int k=0;k<workSize;++k)
				{
					int x=x1+k%dx, y=y1+(k/dx)%dy, z=z1+k/(dy*dx), idx=offset+Xplaces*(Yplaces*z+y)+x;
					uf(idx);
				}
			}
			break;
		case 1://r_r
			{
				auto &res=ex.n[in.result], &op1=ex.n[in.op1];
				for(int k=0;k<workSize;++k)
				{
					int x=x1+k%dx, y=y1+(k/dx)%dy, z=z1+k/(dy*dx), idx=offset+Xplaces*(Yplaces*z+y)+x;
					res.r[idx]=in.r_r(op1.r[idx]);
				}
			}
			break;
		case 2://c_c
			{
				auto &res=ex.n[in.result], &op1=ex.n[in.op1];
				if(res.i.size()!=ndrSize)
					res.i.resize(ndrSize);
				for(int k=0;k<workSize;++k)
				{
					int x=x1+k%dx, y=y1+(k/dx)%dy, z=z1+k/(dy*dx), idx=offset+Xplaces*(Yplaces*z+y)+x;
					CompRef(res.r[idx], res.i[idx])=in.c_c(Comp1d(op1.r[idx], op1.i[idx]));
				}
			}
			break;
		case 3://q_q
			{
				auto &res=ex.n[in.result], &op1=ex.n[in.op1];
				if(res.i.size()!=ndrSize)
				{
					res.i.resize(ndrSize);
					if(res.j.size()!=ndrSize)
						res.j.resize(ndrSize), op1.k.resize(ndrSize);
				}
				for(int k=0;k<workSize;++k)
				{
					int x=x1+k%dx, y=y1+(k/dx)%dy, z=z1+k/(dy*dx), idx=offset+Xplaces*(Yplaces*z+y)+x;
					QuatRef(res.r[idx], res.i[idx], res.j[idx], res.j[idx])=
						in.q_q(Quat1d(op1.r[idx], op1.i[idx], op1.j[idx], op1.k[idx]));
				}
			}
			break;
		case 4://r_rr
			{
				auto &res=ex.n[in.result], &op1=ex.n[in.op1], &op2=ex.n[in.op2];
				for(int k=0;k<workSize;++k)
				{
					int x=x1+k%dx, y=y1+(k/dx)%dy, z=z1+k/(dy*dx), idx=offset+Xplaces*(Yplaces*z+y)+x;
					res.r[idx]=in.r_rr(op1.r[idx], op2.r[idx]);
				}
			}
			break;
		case 5://c_rc
			{
				auto &res=ex.n[in.result], &op1=ex.n[in.op1], &op2=ex.n[in.op2];
				if(res.i.size()!=ndrSize)
					res.i.resize(ndrSize);
				for(int k=0;k<workSize;++k)
				{
					int x=x1+k%dx, y=y1+(k/dx)%dy, z=z1+k/(dy*dx), idx=offset+Xplaces*(Yplaces*z+y)+x;
					CompRef(res.r[idx], res.i[idx])=in.c_rc(op1.r[idx], Comp1d(op2.r[idx], op2.i[idx]));
				}
			}
			break;
		case 6://q_rq
			{
				auto &res=ex.n[in.result], &op1=ex.n[in.op1], &op2=ex.n[in.op2];
				if(res.i.size()!=ndrSize)
				{
					res.i.resize(ndrSize);
					if(res.j.size()!=ndrSize)
						res.j.resize(ndrSize), op2.k.resize(ndrSize);
				}
				for(int k=0;k<workSize;++k)
				{
					int x=x1+k%dx, y=y1+(k/dx)%dy, z=z1+k/(dy*dx), idx=offset+Xplaces*(Yplaces*z+y)+x;
					QuatRef(res.r[idx], res.i[idx], res.j[idx], res.k[idx])=
						in.q_rq(op1.r[idx], Quat1d(op2.r[idx], op2.i[idx], op2.j[idx], op2.k[idx]));
				}
			}
			break;
		case 7://c_cr
			{
				auto &res=ex.n[in.result], &op1=ex.n[in.op1], &op2=ex.n[in.op2];
				if(res.i.size()!=ndrSize)
					res.i.resize(ndrSize);
				for(int k=0;k<workSize;++k)
				{
					int x=x1+k%dx, y=y1+(k/dx)%dy, z=z1+k/(dy*dx), idx=offset+Xplaces*(Yplaces*z+y)+x;
					CompRef(res.r[idx], res.i[idx])=in.c_cr(Comp1d(op1.r[idx], op1.i[idx]), op2.r[idx]);
				}
			}
			break;
		case 8://c_cc
			{
				auto &res=ex.n[in.result], &op1=ex.n[in.op1], &op2=ex.n[in.op2];
				if(res.i.size()!=ndrSize)
					res.i.resize(ndrSize);
				for(int k=0;k<workSize;++k)
				{
					int x=x1+k%dx, y=y1+(k/dx)%dy, z=z1+k/(dy*dx), idx=offset+Xplaces*(Yplaces*z+y)+x;
					CompRef(res.r[idx], res.i[idx])=in.c_cc(Comp1d(op1.r[idx], op1.i[idx]), Comp1d(op2.r[idx], op2.i[idx]));
				}
			}
			break;
		case 9://q_cq
			{
				auto &res=ex.n[in.result], &op1=ex.n[in.op1], &op2=ex.n[in.op2];
				if(res.i.size()!=ndrSize)
				{
					res.i.resize(ndrSize);
					if(res.j.size()!=ndrSize)
						res.j.resize(ndrSize), op2.k.resize(ndrSize);
				}
				for(int k=0;k<workSize;++k)
				{
					int x=x1+k%dx, y=y1+(k/dx)%dy, z=z1+k/(dy*dx), idx=offset+Xplaces*(Yplaces*z+y)+x;
					QuatRef(res.r[idx], res.i[idx], res.j[idx], res.k[idx])=
						in.q_cq(Comp1d(op1.r[idx], op1.i[idx]), Quat1d(op2.r[idx], op2.i[idx], op2.j[idx], op2.k[idx]));
				}
			}
			break;
		case 10://q_qr
			{
				auto &res=ex.n[in.result], &op1=ex.n[in.op1], &op2=ex.n[in.op2];
				if(res.i.size()!=ndrSize)
				{
					res.i.resize(ndrSize);
					if(res.j.size()!=ndrSize)
						res.j.resize(ndrSize), op1.k.resize(ndrSize);
				}
				for(int k=0;k<workSize;++k)
				{
					int x=x1+k%dx, y=y1+(k/dx)%dy, z=z1+k/(dy*dx), idx=offset+Xplaces*(Yplaces*z+y)+x;
					QuatRef(res.r[idx], res.i[idx], res.j[idx], res.k[idx])=
						in.q_qr(Quat1d(op1.r[idx], op1.i[idx], op1.j[idx], op1.k[idx]), op2.r[idx]);
				}
			}
			break;
		case 11://q_qc
			{
				auto &res=ex.n[in.result], &op1=ex.n[in.op1], &op2=ex.n[in.op2];
				if(res.i.size()!=ndrSize)
				{
					res.i.resize(ndrSize);
					if(res.j.size()!=ndrSize)
						res.j.resize(ndrSize), op1.k.resize(ndrSize);
				}
				for(int k=0;k<workSize;++k)
				{
					int x=x1+k%dx, y=y1+(k/dx)%dy, z=z1+k/(dy*dx), idx=offset+Xplaces*(Yplaces*z+y)+x;
					QuatRef(res.r[idx], res.i[idx], res.j[idx], res.k[idx])=
						in.q_qc(Quat1d(op1.r[idx], op1.i[idx], op1.j[idx], op1.k[idx]), Comp1d(op2.r[idx], op2.i[idx]));
				}
			}
			break;
		case 12://q_qq
			{
				auto &res=ex.n[in.result], &op1=ex.n[in.op1], &op2=ex.n[in.op2];
				if(res.i.size()!=ndrSize)
				{
					res.i.resize(ndrSize);
					if(res.j.size()!=ndrSize)
						res.j.resize(ndrSize), op1.k.resize(ndrSize);
				}
				for(int k=0;k<workSize;++k)
				{
					int x=x1+k%dx, y=y1+(k/dx)%dy, z=z1+k/(dy*dx), idx=offset+Xplaces*(Yplaces*z+y)+x;
					QuatRef(res.r[idx], res.i[idx], res.j[idx], res.k[idx])=
						in.q_qq(Quat1d(op1.r[idx], op1.i[idx], op1.j[idx], op1.k[idx]), Quat1d(op2.r[idx], op2.i[idx], op2.j[idx], op2.k[idx]));
				}
			}
			break;

		case 13://c_r
			{
				auto &res=ex.n[in.result], &op1=ex.n[in.op1];
				for(int k=0;k<workSize;++k)
				{
					int x=x1+k%dx, y=y1+(k/dx)%dy, z=z1+k/(dy*dx), idx=offset+Xplaces*(Yplaces*z+y)+x;
					CompRef(res.r[idx], res.i[idx])=in.c_r(op1.r[idx]);
				}
			}
			break;
		case 14://c_q
			{
				auto &res=ex.n[in.result], &op1=ex.n[in.op1];
				for(int k=0;k<workSize;++k)
				{
					int x=x1+k%dx, y=y1+(k/dx)%dy, z=z1+k/(dy*dx), idx=offset+Xplaces*(Yplaces*z+y)+x;
					CompRef(res.r[idx], res.i[idx])=in.c_q(Quat1d(op1.r[idx], op1.i[idx], op1.j[idx], op1.k[idx]));
				}
			}
			break;

		case 15://r_c
			{
				auto &res=ex.n[in.result], &op1=ex.n[in.op1];
				for(int k=0;k<workSize;++k)
				{
					int x=x1+k%dx, y=y1+(k/dx)%dy, z=z1+k/(dy*dx), idx=offset+Xplaces*(Yplaces*z+y)+x;
					res.r[idx]=in.r_c(Comp1d(op1.r[idx], op1.i[idx]));
				}
			}
			break;
		case 16://r_q
			{
				auto &res=ex.n[in.result], &op1=ex.n[in.op1];
				for(int k=0;k<workSize;++k)
				{
					int x=x1+k%dx, y=y1+(k/dx)%dy, z=z1+k/(dy*dx), idx=offset+Xplaces*(Yplaces*z+y)+x;
					res.r[idx]=in.r_q(Quat1d(op1.r[idx], op1.i[idx], op1.j[idx], op1.k[idx]));
				}
			}
			break;

		case 17://c_rr
			{
				auto &res=ex.n[in.result], &op1=ex.n[in.op1], &op2=ex.n[in.op2];
				for(int k=0;k<workSize;++k)
				{
					int x=x1+k%dx, y=y1+(k/dx)%dy, z=z1+k/(dy*dx), idx=offset+Xplaces*(Yplaces*z+y)+x;
					CompRef(res.r[idx], res.i[idx])=in.c_rr(op1.r[idx], op2.r[idx]);
				}
			}
			break;

		case 18://r_rc
			{
				auto &res=ex.n[in.result], &op1=ex.n[in.op1], &op2=ex.n[in.op2];
				for(int k=0;k<workSize;++k)
				{
					int x=x1+k%dx, y=y1+(k/dx)%dy, z=z1+k/(dy*dx), idx=offset+Xplaces*(Yplaces*z+y)+x;
					res.r[idx]=in.r_rc(op1.r[idx], Comp1d(op2.r[idx], op2.i[idx]));
				}
			}
			break;
		case 19://r_rq
			{
				auto &res=ex.n[in.result], &op1=ex.n[in.op1], &op2=ex.n[in.op2];
				for(int k=0;k<workSize;++k)
				{
					int x=x1+k%dx, y=y1+(k/dx)%dy, z=z1+k/(dy*dx), idx=offset+Xplaces*(Yplaces*z+y)+x;
					res.r[idx]=in.r_rq(op1.r[idx], Quat1d(op2.r[idx], op2.i[idx], op2.j[idx], op2.k[idx]));
				}
			}
			break;
		case 20://r_cr
			{
				auto &res=ex.n[in.result], &op1=ex.n[in.op1], &op2=ex.n[in.op2];
				for(int k=0;k<workSize;++k)
				{
					int x=x1+k%dx, y=y1+(k/dx)%dy, z=z1+k/(dy*dx), idx=offset+Xplaces*(Yplaces*z+y)+x;
					res.r[idx]=in.r_cr(Comp1d(op1.r[idx], op1.i[idx]), op2.r[idx]);
				}
			}
			break;
		case 21://r_cc
			{
				auto &res=ex.n[in.result], &op1=ex.n[in.op1], &op2=ex.n[in.op2];
				for(int k=0;k<workSize;++k)
				{
					int x=x1+k%dx, y=y1+(k/dx)%dy, z=z1+k/(dy*dx), idx=offset+Xplaces*(Yplaces*z+y)+x;
					res.r[idx]=in.r_cc(Comp1d(op1.r[idx], op1.i[idx]), Comp1d(op2.r[idx], op2.i[idx]));
				}
			}
			break;
		case 22://r_cq
			{
				auto &res=ex.n[in.result], &op1=ex.n[in.op1], &op2=ex.n[in.op2];
				for(int k=0;k<workSize;++k)
				{
					int x=x1+k%dx, y=y1+(k/dx)%dy, z=z1+k/(dy*dx), idx=offset+Xplaces*(Yplaces*z+y)+x;
					res.r[idx]=in.r_cq(Comp1d(op1.r[idx], op1.i[idx]), Quat1d(op2.r[idx], op2.i[idx], op2.j[idx], op2.k[idx]));
				}
			}
			break;
		case 23://r_qr
			{
				auto &res=ex.n[in.result], &op1=ex.n[in.op1], &op2=ex.n[in.op2];
				for(int k=0;k<workSize;++k)
				{
					int x=x1+k%dx, y=y1+(k/dx)%dy, z=z1+k/(dy*dx), idx=offset+Xplaces*(Yplaces*z+y)+x;
					res.r[idx]=in.r_qr(Quat1d(op1.r[idx], op1.i[idx], op1.j[idx], op1.k[idx]), op2.r[idx]);
				}
			}
			break;
		case 24://r_qc
			{
				auto &res=ex.n[in.result], &op1=ex.n[in.op1], &op2=ex.n[in.op2];
				for(int k=0;k<workSize;++k)
				{
					int x=x1+k%dx, y=y1+(k/dx)%dy, z=z1+k/(dy*dx), idx=offset+Xplaces*(Yplaces*z+y)+x;
					res.r[idx]=in.r_qc(Quat1d(op1.r[idx], op1.i[idx], op1.j[idx], op1.k[idx]), Comp1d(op2.r[idx], op2.i[idx]));
				}
			}
			break;
		case 25://r_qq
			{
				auto &res=ex.n[in.result], &op1=ex.n[in.op1], &op2=ex.n[in.op2];
				for(int k=0;k<workSize;++k)
				{
					int x=x1+k%dx, y=y1+(k/dx)%dy, z=z1+k/(dy*dx), idx=offset+Xplaces*(Yplaces*z+y)+x;
					res.r[idx]=in.r_qq(Quat1d(op1.r[idx], op1.i[idx], op1.j[idx], op1.k[idx]), Quat1d(op2.r[idx], op2.i[idx], op2.j[idx], op2.k[idx]));
				}
			}
			break;

		case 26://c_qc
			{
				auto &res=ex.n[in.result], &op1=ex.n[in.op1], &op2=ex.n[in.op2];
				for(int k=0;k<workSize;++k)
				{
					int x=x1+k%dx, y=y1+(k/dx)%dy, z=z1+k/(dy*dx), idx=offset+Xplaces*(Yplaces*z+y)+x;
					CompRef(res.r[idx], res.i[idx])=in.c_qc(Quat1d(op1.r[idx], op1.i[idx], op1.j[idx], op1.k[idx]), Comp1d(op2.r[idx], op2.i[idx]));
				}
			}
			break;

		case 27://a ? b : c
			{
				auto &res=ex.n[in.result], &op1=ex.n[in.op1], &op2=ex.n[in.op2], &op3=ex.n[in.op3];
				char r_ms=maximum(in.op2_ms, in.op3_ms), op_ms=(in.op2_ms=='c')+2*(in.op2_ms=='h')+3*(in.op3_ms=='c')+6*(in.op3_ms=='h');
				if(r_ms>='c')
				{
					if(res.i.size()!=ndrSize)
						res.i.resize(ndrSize);
					if(r_ms=='h'&&res.j.size()!=ndrSize)
						res.j.resize(ndrSize), res.k.resize(ndrSize);
				}
				switch(op_ms)
				{
				case 0://r_rr
					for(int k=0;k<workSize;++k)
					{
						int x=x1+k%dx, y=y1+(k/dx)%dy, z=z1+k/(dy*dx), idx=offset+Xplaces*(Yplaces*z+y)+x;
						auto &X=op1.r[idx], &Y=op2.r[idx], &Z=op3.r[idx];
						res.r[idx]=X?Y:Z;
					}
					break;
				case 1://c_rc
					for(int k=0;k<workSize;++k)
					{
						int x=x1+k%dx, y=y1+(k/dx)%dy, z=z1+k/(dy*dx), idx=offset+Xplaces*(Yplaces*z+y)+x;
						auto X=Comp1d(op1.r[idx], op1.i[idx]);
						auto Y=op2.r[idx];
						auto Z=Comp1d(op3.r[idx], op3.i[idx]);
						Comp1d(res.r[idx], res.i[idx])=X.c_is_true()?Y:Z;
					}
					break;
				case 2://q_rq
					for(int k=0;k<workSize;++k)
					{
						int x=x1+k%dx, y=y1+(k/dx)%dy, z=z1+k/(dy*dx), idx=offset+Xplaces*(Yplaces*z+y)+x;
						auto X=Quat1d(op1.r[idx], op1.i[idx], op1.j[idx], op1.k[idx]);
						auto Y=op2.r[idx];
						auto Z=Quat1d(op3.r[idx], op3.i[idx], op3.j[idx], op3.k[idx]);
						Quat1d(res.r[idx], res.i[idx], res.j[idx], res.k[idx])=X.q_is_true()?Y:Z;
					}
					break;
				case 3://c_cr
					for(int k=0;k<workSize;++k)
					{
						int x=x1+k%dx, y=y1+(k/dx)%dy, z=z1+k/(dy*dx), idx=offset+Xplaces*(Yplaces*z+y)+x;
						auto X=Comp1d(op1.r[idx], op1.i[idx]);
						auto Y=Comp1d(op2.r[idx], op2.i[idx]);
						auto Z=op3.r[idx];
						Comp1d(res.r[idx], res.i[idx])=X.c_is_true()?Y:Z;
					}
					break;
				case 4://c_cc
					for(int k=0;k<workSize;++k)
					{
						int x=x1+k%dx, y=y1+(k/dx)%dy, z=z1+k/(dy*dx), idx=offset+Xplaces*(Yplaces*z+y)+x;
						auto X=Comp1d(op1.r[idx], op1.i[idx]);
						auto Y=Comp1d(op2.r[idx], op2.i[idx]);
						auto Z=Comp1d(op3.r[idx], op3.i[idx]);
						Comp1d(res.r[idx], res.i[idx])=X.c_is_true()?Y:Z;
					}
					break;
				case 5://q_cq
					for(int k=0;k<workSize;++k)
					{
						int x=x1+k%dx, y=y1+(k/dx)%dy, z=z1+k/(dy*dx), idx=offset+Xplaces*(Yplaces*z+y)+x;
						auto X=Quat1d(op1.r[idx], op1.i[idx], op1.j[idx], op1.k[idx]);
						auto Y=Comp1d(op2.r[idx], op2.i[idx]);
						auto Z=Quat1d(op3.r[idx], op3.i[idx], op3.j[idx], op3.k[idx]);
						Quat1d(res.r[idx], res.i[idx], res.j[idx], res.k[idx])=X.q_is_true()?Y:Z;
					}
					break;
				case 6://q_qr
					for(int k=0;k<workSize;++k)
					{
						int x=x1+k%dx, y=y1+(k/dx)%dy, z=z1+k/(dy*dx), idx=offset+Xplaces*(Yplaces*z+y)+x;
						auto X=Quat1d(op1.r[idx], op1.i[idx], op1.j[idx], op1.k[idx]);
						auto Y=Quat1d(op2.r[idx], op2.i[idx], op2.j[idx], op2.k[idx]);
						auto Z=op3.r[idx];
						Quat1d(res.r[idx], res.i[idx], res.j[idx], res.k[idx])=X.q_is_true()?Y:Z;
					}
					break;
				case 7://q_qc
					for(int k=0;k<workSize;++k)
					{
						int x=x1+k%dx, y=y1+(k/dx)%dy, z=z1+k/(dy*dx), idx=offset+Xplaces*(Yplaces*z+y)+x;
						auto X=Quat1d(op1.r[idx], op1.i[idx], op1.j[idx], op1.k[idx]);
						auto Y=Quat1d(op2.r[idx], op2.i[idx], op2.j[idx], op2.k[idx]);
						auto Z=Comp1d(op3.r[idx], op3.i[idx]);
						Quat1d(res.r[idx], res.i[idx], res.j[idx], res.k[idx])=X.q_is_true()?Y:Z;
					}
					break;
				case 8://q_qq
					for(int k=0;k<workSize;++k)
					{
						int x=x1+k%dx, y=y1+(k/dx)%dy, z=z1+k/(dy*dx), idx=offset+Xplaces*(Yplaces*z+y)+x;
						auto X=Quat1d(op1.r[idx], op1.i[idx], op1.j[idx], op1.k[idx]);
						auto Y=Quat1d(op2.r[idx], op2.i[idx], op2.j[idx], op2.k[idx]);
						auto Z=Quat1d(op3.r[idx], op3.i[idx], op3.j[idx], op3.k[idx]);
						Quat1d(res.r[idx], res.i[idx], res.j[idx], res.k[idx])=X.q_is_true()?Y:Z;
					}
					break;
				}
			}
			break;
		}
		if(in.d.disc_out&&disc_out!=0)
			disc_out(ex, in, offset, disc_offset, x1, x2, y1, y2, z1, z2, Xplaces, Yplaces, Zplaces);
		//for(int kn=0;kn<ex.n.size();++kn)
		//	ndr_to_clipboard_2d((double*)ex.n[kn].r, Xplaces0, Yplaces);//
	}
//	if(ex.i.size())
//		_mm_empty();
}
inline void subtract_NDRs_component(int workSize, int x1, int dx, int y1, int dy, int Xplaces, std::vector<double> *op1)
{
	for(int k=0;k<workSize;++k)
	{
		int x=x1+k%dx, y=y1+k/dx, idx=Xplaces*y+x;
		(*op1)[idx]=G2::r_r_minus((*op1)[idx]);
	}
}
inline void subtract_NDRs_component(int workSize, int x1, int dx, int y1, int dy, int Xplaces, std::vector<double> *op1, std::vector<double> *op2)
{
//	BinaryMinus bm(x1, dx, y1, dy, Xplaces, op1, op2);
	for(int k=0;k<workSize;++k)
	{
		int x=x1+k%dx, y=y1+k/dx, idx=Xplaces*y+x;
		(*op1)[idx]=G2::r_rr_minus((*op1)[idx], (*op2)[idx]);
	}
}
void subtract_NDRs(Expression &ex, int x1, int x2, int y1, int y2, int Xplaces, int Yplaces)
{
	auto &in=*ex.i.rbegin();
	auto &op1=ex.n[in.op1];
	x1>>=1, x2=ceil_half(x2), Xplaces=ceil_half(Xplaces);
	int dx=x2-x1, dy=y2-y1, workSize=dx*dy;
	if(in.type<=3||in.type=='c')//unary == or !=	can't be 'c'
	{
		subtract_NDRs_component(workSize, x1, dx, y1, dy, Xplaces, &op1.r);
		if(in.op1_ms>='c')
		{
			subtract_NDRs_component(workSize, x1, dx, y1, dy, Xplaces, &op1.i);
			if(in.op1_ms=='h')
			{
				subtract_NDRs_component(workSize, x1, dx, y1, dy, Xplaces, &op1.j);
				subtract_NDRs_component(workSize, x1, dx, y1, dy, Xplaces, &op1.k);
			}
		}
	}
	else//binary
	{
		auto &op2=ex.n[in.op2];
		subtract_NDRs_component(workSize, x1, dx, y1, dy, Xplaces, &op1.r, &op2.r);
		if(in.op1_ms>='c'&&in.op2_ms>='c')
			subtract_NDRs_component(workSize, x1, dx, y1, dy, Xplaces, &op1.i, &op2.i);
		else if(in.op2_ms>='c')
		{
			subtract_NDRs_component(workSize, x1, dx, y1, dy, Xplaces, &op2.i);
			op1.i=std::move(op2.i);
		}
		if(in.op1_ms=='h'&&in.op2_ms=='h')
		{
			subtract_NDRs_component(workSize, x1, dx, y1, dy, Xplaces, &op1.j, &op2.j);
			subtract_NDRs_component(workSize, x1, dx, y1, dy, Xplaces, &op1.k, &op2.k);
		}
		else if(in.op2_ms=='h')
		{
			subtract_NDRs_component(workSize, x1, dx, y1, dy, Xplaces, &op1.j);
			subtract_NDRs_component(workSize, x1, dx, y1, dy, Xplaces, &op1.k);
			op1.j=std::move(op2.j);
			op1.k=std::move(op2.k);
		}
	}
}

struct		Button
{
	int x1, x2, y1, y2;
	const char *label;
	int label_width;
	Button(){memset(this, 0, sizeof(Button));}
	Button(int x, int y, int w, int h, const char *label, int label_width):x1(x), x2(x+w), y1(y), y2(y+h), label(label), label_width(label_width){}
	void set(int x, int y, int w, int h, const char *label, int label_width){x1=x, y1=y, x2=x+w, y2=y+h, this->label=label, this->label_width=label_width;}
	void set(int x, int y, int w, int h){x1=x, y1=y, x2=x+w, y2=y+h;}
	bool click(int x, int y){return x>=x1&&x<x2&&y>=y1&&y<y2;}
	bool click(vec2 pos){return pos.x>=x1&&pos.x<x2&&pos.y>=y1&&pos.y<y2;}
	void draw_border()
	{
		GL2_2D::draw_rectangle_hollow(x1, x2, y1, y2);
	}
	void print_label()
	{
		print((x1+x2-label_width)>>1, (int)(y1+y2-fontH)>>1, label);
	}
} reset_button(w>>1, 0, w>>1, h/10, "Reset", 38), copy_button(-1, -1, -1, -1, "Copy", 33);
enum		TouchRegion
{
	TOUCH_MISS,
	DRAG_CURSOR, DRAG_SELCUR,
	_2D_DRAG,
	BOTTOM_LEFT_MOVE, BOTTOM_RIGHT_TURN, BUTTON_RESET
};
struct		TouchInfo
{
	int idx;
	vec2 pos, delta;
	int region;
	vec2 objPos;
	TouchInfo(){memset(this, 0, sizeof(TouchInfo));}
	TouchInfo(int idx, vec2 const &pos, int region, vec2 const &objPos):idx(idx), pos(pos), region(region), objPos(objPos){}
	void move_to(float x, float y)
	{
		vec2 new_pos(x, y);
		delta=new_pos-pos;
		pos=new_pos;
	}
};
std::vector<TouchInfo> touchInfo;
vec2 new_pos;
ivec4 getCursorBox(vec2 const &cursorPos)
{
	return ivec4(cursorPos.x-fontH*2, cursorPos.x+fontH*2, cursorPos.y+fontH, cursorPos.y+fontH*5);
	//return ivec4(cursorPos.x-fontH, cursorPos.x+fontH, cursorPos.y+fontH*2, cursorPos.y+fontH*4);
	//float fontH_2=fontH*0.5f;
	//return ivec4(cursorPos.x-fontH_2, cursorPos.x+fontH_2, cursorPos.y-fontH_2, cursorPos.y+fontH+fontH_2);
}
void draw_touch_pointers()
{
	glLineWidth(preferred_line_width);
	for(int k=0, kEnd=touchInfo.size();k<kEnd;++k)//draw touch pointers
	{
		auto &ti=touchInfo[k];
		GL2_2D::draw_line(ti.pos.x-200, ti.pos.y, ti.pos.x+200, ti.pos.y);
		GL2_2D::draw_line(ti.pos.x, ti.pos.y-200, ti.pos.x, ti.pos.y+200);
		//draw_line(X0, Y0, ti.pos.x, ti.pos.y);
	}
}
void print_touch_pointers(int line_from_bottom)
{
	for(int k=0, kEnd=touchInfo.size();k<kEnd;++k)//print touch pointers
	{
		auto &ti=touchInfo[k];
		//print(0, h-fontH*(line_from_bottom+k), "idx %d, x %f, y %f, action %d", ti.idx, ti.pos.x, ti.pos.y, ti.action);
		print(0, h-fontH*(line_from_bottom+k), "idx %d, x %f, y %f", ti.idx, ti.pos.x, ti.pos.y);
	}
}

template<int _size>void append(char *&a, int &alen, char (&b)[_size], int &blen)
{
	a=(char*)realloc(a, (alen+blen+1)*sizeof(char));
	memcpy(&a[alen], b, (blen+1)*sizeof(char));
	alen+=blen;
}
void print_contextHelp(const char **helpMsg, short msg_count, short dx)
{
	short X0=w>>1, Y0=h>>1, dy=msg_count*18, dx_2=dx>>1, dy_2=dy>>1;
	short xs=X0-dx_2, ys=Y0-dy_2;
//	Rectangle(ghMemDC, xs-1, ys-1, X0+dx_2, Y0+dy_2);
	pen_color=0xFF000000, brush_color=0xFFFFFFFF, GL2_2D::draw_rectangle(xs-1, X0+dx_2, ys-1, Y0+dy_2);
	//set_color(0xFFFFFFFF), draw_rectangle(xs-1, X0+dx_2, ys-1, Y0+dy_2);
	//set_color(0xFF000000), draw_rectangle_hollow(xs-1, X0+dx_2, ys-1, Y0+dy_2);
	for(short k=0;k<msg_count;++k)
		print(xs, ys+18*k, helpMsg[k]);
}
void debug_check()
{
#ifdef DEBUG
//	std::vector<double> a(100, 0);
//	if(a.size()!=100)
//		return;
//			int *a=(int*)malloc(1);
//			if(!a)
//				return;
//			free(a);
#endif
}
namespace	modes
{
	bool active=false, ready=false;
	unsigned const _2dCheckColor=0xFFE0E0E0,//0xFFD0D0D0	0xFFEFEFEF
		_3dGridColor=0xFFD0D0D0;
	int const N_MODES=13;
	std::vector<int> nExpr(N_MODES, 0);
	struct		Label
	{
		std::string label;
		int exNo;
		Label(std::string const &label, int exNo):label(label), exNo(exNo){}
	};
	struct		CLabel:public Label
	{
		bool isTime;
		double value;
		CLabel(std::string const &label, int exNo):Label(label, exNo), isTime(true){}
		CLabel(std::string const &label, int exNo, double value):Label(label, exNo), value(value), isTime(false){}
	};
	class		Labels
	{
		virtual void LOL(int, std::string const&, int, double)=0;
	public:
		std::vector<CLabel> Clabels;
		void fill(int exNo)
		{
		//	for(auto &v:expr[exNo].variables)
			auto & ex_var=expr[exNo].variables;
			for(unsigned k=0;k<ex_var.size();++k)
			{
				auto &v=ex_var[k];
				switch(v.mathSet)
				{
				case 'R':
					LOL(v.varTypeR, v.name, exNo, v.val.r);
					break;
				case 'c':
					LOL(v.varTypeR, "Re("+v.name+")", exNo, v.val.r);
					LOL(v.varTypeI, "Im("+v.name+")", exNo, v.val.i);
					break;
				case 'h':
					LOL(v.varTypeR, "Re("+v.name+")", exNo, v.val.r);
					LOL(v.varTypeI, "I("+v.name+")", exNo, v.val.i);
					LOL(v.varTypeJ, "J("+v.name+")", exNo, v.val.j);
					LOL(v.varTypeK, "K("+v.name+")", exNo, v.val.k);
					break;
				}
			}
		}
	};
	class		Labels_0D:public Labels
	{
		std::unordered_set<int> exprs;
		void LOL(int LOL_1, std::string const &str, int exNo, double value)
		{
			switch(LOL_1)
			{
			case 't':Clabels.push_back(CLabel(str, exNo)), exprs.insert(exNo);if(exprs.size()>1)colored=1;break;
			case 'c':Clabels.push_back(CLabel(str, exNo, value)), exprs.insert(exNo);if(exprs.size()>1)colored=1;break;
			}
		}
	public:
		int colored;
		void clear(){Clabels.clear(), colored=0, exprs.clear();}
	};
	class		Labels_1D:public Labels
	{
		void LOL(int LOL_1, std::string const &str, int exNo, double value)
		{
			switch(LOL_1)
			{
			case 'x':Xlabels.push_back(Label(str, exNo));break;
			case 't':Clabels.push_back(CLabel(str, exNo));break;
			case 'c':Clabels.push_back(CLabel(str, exNo, value));break;
			}
		}
	public:
		std::vector<Label> Xlabels;
		void clear(){Xlabels.clear(), Clabels.clear();}
	};
	class		Labels_2D:public Labels
	{
		void LOL(int LOL_1, std::string const &str, int exNo, double value)
		{
			switch(LOL_1)
			{
			case 'x':Xlabels.push_back(Label(str, exNo));break;
			case 'y':Ylabels.push_back(Label(str, exNo));break;
			case 't':Clabels.push_back(CLabel(str, exNo));break;
			case 'c':Clabels.push_back(CLabel(str, exNo, value));break;
			}
		}
	public:
		std::vector<Label> Xlabels, Ylabels;
		void clear(){Xlabels.clear(), Ylabels.clear(), Clabels.clear();}
	};
	class		Labels_3D:public Labels
	{
		void LOL(int LOL_1, std::string const &str, int exNo, double value)
		{
			switch(LOL_1)
			{
			case 'x':Xlabels.push_back(Label(str, exNo));break;
			case 'y':Ylabels.push_back(Label(str, exNo));break;
			case 'z':Zlabels.push_back(Label(str, exNo));break;
			case 't':Clabels.push_back(CLabel(str, exNo));break;
			case 'c':Clabels.push_back(CLabel(str, exNo, value));break;
			}
		}
	public:
		std::vector<Label> Xlabels, Ylabels, Zlabels;
		void clear(){Xlabels.clear(), Ylabels.clear(), Zlabels.clear(), Clabels.clear();}
	};

//	typedef double real;
	typedef float real;
	struct Triangle
	{
		double X1, Y1, Z1, X2, Y2, Z2, X3, Y3, Z3;
		Triangle(double X1, double Y1, double Z1, double X2, double Y2, double Z2, double X3, double Y3, double Z3):X1(X1), Y1(Y1), Z1(Z1), X2(X2), Y2(Y2), Z2(Z2), X3(X3), Y3(Y3), Z3(Z3){}
	};
	struct Stick
	{
		double X1, Y1, Z1, X2, Y2, Z2;
		Stick(double X1, double Y1, double Z1, double X2, double Y2, double Z2):X1(X1), Y1(Y1), Z1(Z1), X2(X2), Y2(Y2), Z2(Z2){}
	};
	struct		_3D
	{//_3D_API:public _3D
		int bx1, bx2, by1, by2, bw, bh;
		Camera cam;
		_3D(real camx, real camy, real camz, real ax, real ay, real tanfov):cam(camx, camy, camz, ax, ay, tanfov){}
		void setDimensions(int x, int y, int w, int h)
		{
			bx1=x, by1=y, bx2=x+w, by2=h, bw=w, bh=h, X0=w>>1, Y0=h>>1;
			clipX1=x, clipY1=y, clipX2=x+w-50, clipY2=y+h-24;
		}
		void teleport(real camx, real camy, real camz, real ax, real ay, real tanfov)
		{
			cam.teleport(vec3(camx, camy, camz), ax, ay, tanfov);
		}
		void teleport_degrees(real camx, real camy, real camz, real ax_deg, real ay_deg, real tanfov)
		{
			cam.teleport(vec3(camx, camy, camz), ax_deg*torad, ay_deg*torad, tanfov);
		}
		//void move(float dx, float dy)
		//{
		//	cam.moveByForward(dy);
		//	cam.moveByRight(dx);
		//}
	private:
		void line_A_coeff_x(real Xcp1, real Ycp1, real Zcp1, real Xcp2, real Ycp2, real Zcp2, real &a, real &b)
		{
			auto &tanfov=cam.tanfov;
			real t=(Xcp2-Xcp1)*Zcp1-Xcp1*(Zcp2-Zcp1);
			a=(Zcp1-Zcp2)*tanfov/(X0*t), b=((Zcp2-Zcp1)*tanfov+Xcp2-Xcp1)/t;
		}
		void line_A_coeff_y(real Xcp1, real Ycp1, real Zcp1, real Xcp2, real Ycp2, real Zcp2, real &a, real &b)
		{
			auto &tanfov=cam.tanfov;
			real t=(Ycp2-Ycp1)*Zcp1-Ycp1*(Zcp2-Zcp1);
			a=(Zcp1-Zcp2)*tanfov/(X0*t), b=((Zcp2-Zcp1)*Y0*tanfov/X0+Ycp2-Ycp1)/t;
		}
		real linearY(real x1, real y1, real x2, real y2, real x){return (y2-y1)/(x2-x1)*(x-x1)+y1;}
		real linearX(real x1, real y1, real x2, real y2, real y){return (x2-x1)/(y2-y1)*(y-y1)+x1;}
		vec3 p0, cp1, cp2;
		vec2 s1, s2;
	public:
		int lineColor;//rgb
	private:
		bool isNanOrInf(real x){return (((int*)&x)[1]&0x7FF00000)==0x7FF00000;}
		void doPoint1(vec3 const &p){cam.world2camscreen(p, cp1, s1);}
		//world coordinates -> camera coordinates
		void point_world_camera(vec3 const &d, vec3 &cp){cam.relworld2cam(d, cp);}
		//camera coordinates -> screen coordinates
		void point_camera_screen(vec3 const &cp, vec2 &s){cam.cam2screen(cp, s);}
		//world coordinates -> screen coordinates		with depth info
		void point_world_screen(vec3 const &d, vec3 &cp, vec2 &s){cam.relworld2camscreen(d, cp, s);}
	public:
		void moveTo(real x, real y, real z){p0.set(x, y, z);}
		void lineTo(real x, real y, real z){line(p0, vec3(x, y, z));}
		void line(vec3 const &p1, vec3 const &p2)
		{
			GL2_3D::push_line_segment(p1, p2, lineColor);
		//	push_line(vec3(x1, y1, z1), vec3(x2, y2, z2));
		}

	//public:
	//	void triangle_halfTransparent(Triangle &Tr, int color)
	//	{
	//		GL2_3D::push_triangle(vec3(Tr.X1, Tr.Y1, Tr.Z1), vec3(Tr.X2, Tr.Y2, Tr.Z2), vec3(Tr.X3, Tr.Y3, Tr.Z3), 0x7FFFFFFF&color);
	//	}
	//	void triangle_halfTransparent(real x1, real y1, real z1, real x2, real y2, real z2, real x3, real y3, real z3, int color)
	//	{
	//		GL2_3D::push_triangle(vec3(x1, y1, z1), vec3(x2, y2, z2), vec3(x3, y3, z3), 0x7FFFFFFF&color);
	//	}
	public:
		void label(real x, real y, real z, char const *format, ...)
		{
			vec3 cp;
			cam.world2cam(vec3(x, y, z), cp);
			if(cp.z>0)
			{
				vec2 s;
				cam.cam2screen(cp, s);
				va_list args;
				va_start(args, format);
				if(abs(s.x)<1e6&&abs(s.y)<1e6&&vsnprintf(g_buf, g_buf_size, format, args)>0)
					insert_label(cp.z, Text((int)floor(s.x), (int)floor(s.y), TRANSPARENT, g_buf));//-0.5 truncated as 0
				va_end(args);
			}
		}
	private:
		int clipX1, clipY1, clipX2, clipY2;
		//	->1->2->		+depth info
		bool arrowLabel_2dLine(real &x1, real &y1, real &x2, real &y2)
		{
			if(y1==y2)
			{
				if(y2<clipY1||y2>clipY2)
					return false;
				if(x1>x2)
				{
					real tx=x2, ty=y2;
					y2=linearY(x1, y1, x2, y2, clipX1), x2=clipX1;
					y1=linearY(x1, y1, tx, ty, clipX2), x1=clipX2;
				}
				else
				{
					real tx=x2, ty=y2;
					y2=linearY(x1, y1, x2, y2, clipX2), x2=clipX2;
					y1=linearY(x1, y1, tx, ty, clipX1), x1=clipX1;
				}
			}
			else
			{
				if(y1>y2)							//	<-	<-	Y1	<-	<-	Y2	<-	<-
				{
					real tx=x2, ty=y2;
					x2=linearX(x1, y1, x2, y2, clipY1), y2=clipY1;
					x1=linearX(x1, y1, tx, ty, clipY2), y1=clipY2;
				}
				else								//	->	->	Y1	->	->	Y2	->	->
				{
					real tx=x2, ty=y2;
					x2=linearX(x1, y1, x2, y2, clipY2), y2=clipY2;
					x1=linearX(x1, y1, tx, ty, clipY1), y1=clipY1;
				}
				if(x2<clipX1)
				{
					if(x1<clipX1)
						return false;
					else if(x1>clipX2)
					{
						real tx=x2, ty=y2;
						y2=linearY(x1, y1, x2, y2, clipX1), x2=clipX1;
						y1=linearY(x1, y1, tx, ty, clipX2), x1=clipX2;
					}
					else
						y2=linearY(x1, y1, x2, y2, clipX1), x2=clipX1;
				}
				else if(x2>clipX2)
				{
					if(x1<clipX1)
					{
						real tx=x2, ty=y2;
						y2=linearY(x1, y1, x2, y2, clipX2), x2=clipX2;
						y1=linearY(x1, y1, tx, ty, clipX1), x1=clipX1;
					}
					else if(x1>clipX2)
						return false;
					else
						y2=linearY(x1, y1, x2, y2, clipX2), x2=clipX2;
				}
				else
				{
					if(x1<clipX1)
						y1=linearY(x1, y1, x2, y2, clipX1), x1=clipX1;
					else if(x1>clipX2)
						y1=linearY(x1, y1, x2, y2, clipX2), x1=clipX2;
				}
			}
			return true;
		}
		//	->1->2		+depth info
		bool arrowLabel_2dRay(real &x1, real &y1, real &x2, real &y2)
		{
			if(y1==y2)
			{
				if(y2<clipY1||y2>clipY2)
					return false;
				if(x1>x2)
				{
					real tx=x2, ty=y2;
					y2=linearY(x1, y1, x2, y2, clipX1), x2=clipX1;
					y1=linearY(x1, y1, tx, ty, clipX2), x1=clipX2;
				}
				else
				{
					real tx=x2, ty=y2;
					y2=linearY(x1, y1, x2, y2, clipX2), x2=clipX2;
					y1=linearY(x1, y1, tx, ty, clipX1), x1=clipX1;
				}
			}
			else
			{
				if(y1>y2)
				{
					if(y2<clipY1)					//	y2	<-	Y1	<-	<-	Y2	<-	<-
					{
						real tx=x2, ty=y2;
						x2=linearX(x1, y1, x2, y2, clipY1), y2=clipY1;
						x1=linearX(x1, y1, tx, ty, clipY2), y1=clipY2;
					}
					else if(y2>clipY2)				//			Y1			Y2	y2	<-
						return false;
					else							//			Y1	y2	<-	Y2	<-	<-
						x1=linearX(x1, y1, x2, y2, clipY2), y1=clipY2;
				}
				else
				{
					if(y2<clipY1)					//	->	y2	Y1			Y2
						return false;
					else if(y2>clipY2)				//	->	->	Y1	->	->	Y2	->	y2
					{
						real tx=x2, ty=y2;
						x2=linearX(x1, y1, x2, y2, clipY2), y2=clipY2;
						x1=linearX(x1, y1, tx, ty, clipY1), y1=clipY1;
					}
					else							//	->	->	Y1	->	y2	Y2
						x1=linearX(x1, y1, x2, y2, clipY1), y1=clipY1;
				}
				if(x2<clipX1)
				{
					if(x1<clipX1)
						return false;
					else if(x1>clipX2)
					{
						real tx=x2, ty=y2;
						y2=linearY(x1, y1, x2, y2, clipX1), x2=clipX1;
						y1=linearY(x1, y1, tx, ty, clipX2), x1=clipX2;
					}
					else
						y2=linearY(x1, y1, x2, y2, clipX1), x2=clipX1;
				}
				else if(x2>clipX2)
				{
					if(x1<clipX1)
					{
						real tx=x2, ty=y2;
						y2=linearY(x1, y1, x2, y2, clipX2), x2=clipX2;
						y1=linearY(x1, y1, tx, ty, clipX1), x1=clipX1;
					}
					else if(x1>clipX2)
						return false;
					else
						y2=linearY(x1, y1, x2, y2, clipX2), x2=clipX2;
				}
				else
				{
					if(x1<clipX1)
						y1=linearY(x1, y1, x2, y2, clipX1), x1=clipX1;
					else if(x1>clipX2)
						y1=linearY(x1, y1, x2, y2, clipX2), x1=clipX2;
				}
			}
			return true;
		}
		//	1 2<-		+depth info
		bool arrowLabel_2dExtrapolateRay(real &Xs1, real &Ys1, real &Xs2, real &Ys2)
		{
			real dx=Xs2-Xs1, dy=Ys2-Ys1, r=G2::_sqrt2*(abs(Xs2)+abs(Ys2)+2*(X0+Y0))/(abs(dx)+abs(dy));
			return arrowLabel_2dRay(Xs1=Xs2+r*dx, Ys1=Ys2+r*dy, Xs2, Ys2);
		}
		void arrowLabel_finishPoint1(real &x1, real &y1, real Xcp1, real Ycp1, real Zcp1, real &x2, real &y2, real Xcp2, real Ycp2, real Zcp2, int &Xs, int &Ys, real &A)
		{
			real dx=x2-x1, dy=y2-y1, a, b;
			if(abs(dx)>abs(dy))
			{
				line_A_coeff_x(Xcp1, Ycp1, Zcp1, Xcp2, Ycp2, Zcp2, a, b);
				A=a*x1+b;
			}
			else
			{
				line_A_coeff_y(Xcp1, Ycp1, Zcp1, Xcp2, Ycp2, Zcp2, a, b);
				A=a*y1+b;
			}
			Xs=int(x1), Ys=int(y1);
		}
		void arrowLabel_finishPoint2(real &x1, real &y1, real Xcp1, real Ycp1, real Zcp1, real &x2, real &y2, real Xcp2, real Ycp2, real Zcp2, int &Xs, int &Ys, real &A)
		{
			real dx=x2-x1, dy=y2-y1, a, b;
			if(abs(dx)>abs(dy))
			{
				line_A_coeff_x(Xcp1, Ycp1, Zcp1, Xcp2, Ycp2, Zcp2, a, b);
				A=a*x2+b;
			}
			else
			{
				line_A_coeff_y(Xcp1, Ycp1, Zcp1, Xcp2, Ycp2, Zcp2, a, b);
				A=a*y2+b;
			}
			Xs=int(x2), Ys=int(y2);
		}
		bool arrowLabel_infiniteRay(vec3 const &p1, vec3 const &p2, int &Xs, int &Ys, real &A)
		{
			bool valid=false;
			vec3 cp1;		cam.world2cam(p1, cp1);
			vec3 cpInf;	cam.relworld2cam(p2-p1, cpInf);
			if(cpInf.z>0)
			{
				vec2 sInf;		cam.cam2screen(cpInf, sInf);
				if(cp1.z>0)		//		|	P1	->Inf
				{
					vec3 cp2;	cam.world2cam(p2, cp2);
					vec2 s1;	cam.cam2screen(cp1, s1);
					if(arrowLabel_2dRay(s1.x, s1.y, sInf.x, sInf.y))
					{
						arrowLabel_finishPoint2(s1.x, s1.y, cp1.x, cp1.y, cp1.z, sInf.x, sInf.y, cp2.x, cp2.y, cp2.z, Xs, Ys, A);
						return true;
					}
				}
				else if(cp1.z==0)//		|P1|	->Inf
				{
					vec3 cp2;		cam.world2cam(p2, cp2);//P2
					cp1+=cp1-cp2;//P1 behind
					vec2 s1;		cam.cam2screen(cp1, s1);
					if(arrowLabel_2dExtrapolateRay(s1.x, s1.y, sInf.x, sInf.y))
					{
						arrowLabel_finishPoint2(s1.x, s1.y, cp1.x, cp1.y, cp1.z, sInf.x, sInf.y, cp2.x, cp2.y, cp2.z, Xs, Ys, A);
						return true;
					}
				}
				else			//P1	|	->Inf
				{
					vec3 cp2;		cam.world2cam(p2, cp2);
					vec2 s1;		cam.cam2screen(cp1, s1);
					if(arrowLabel_2dExtrapolateRay(s1.x, s1.y, sInf.x, sInf.y))
					{
						arrowLabel_finishPoint2(s1.x, s1.y, cp1.x, cp1.y, cp1.z, sInf.x, sInf.y, cp2.x, cp2.y, cp2.z, Xs, Ys, A);
						return true;
					}
				}
			}
			else if(cpInf.z==0)
			{
				if(cp1.z>0)		//		|	//P1->Inf
				{
					vec3 cp2;
					vec2 s2, s1;
					cam.world2camscreen(p2, cp2, s2);
					cam.cam2screen(cp1, s1);
					if(arrowLabel_2dLine(s1.x, s1.y, s2.x, s2.y))
					{
						arrowLabel_finishPoint2(s1.x, s1.y, cp1.x, cp1.y, cp1.z, s2.x, s2.y, cp2.x, cp2.y, cp2.z, Xs, Ys, A);
						return true;
					}
				}
			}
			else
			{
				vec2 sInf;		cam.cam2screen(cpInf, sInf);
				if(cp1.z>0)		//Inf<-	|	P1
				{
					vec3 cp2;	cam.world2cam(p2, cp2);
					vec2 s1;	cam.cam2screen(cp1, s1);
					if(arrowLabel_2dRay(s1.x, s1.y, sInf.x, sInf.y))
					{
						arrowLabel_finishPoint1(s1.x, s1.y, cp1.x, cp1.y, cp1.z, sInf.x, sInf.y, cp2.x, cp2.y, cp2.z, Xs, Ys, A);
						return true;
					}
				}
				else if(cp1.z==0)//Inf<-	|P1|
				{
					vec3 cp2;	cam.world2cam(p2, cp2);
					cp1+=cp1-cp2;
					vec2 s1;	cam.cam2screen(cp1, s1);
					if(arrowLabel_2dExtrapolateRay(s1.x, s1.y, sInf.x, sInf.y))
					{
						arrowLabel_finishPoint1(s1.x, s1.y, cp1.x, cp1.y, cp1.z, sInf.x, sInf.y, cp2.x, cp2.y, cp2.z, Xs, Ys, A);
						return true;
					}
				}
				else			//Inf<-	P1	|
				{
					vec3 cp2;	cam.world2cam(p2, cp2);
					vec2 s1;	cam.cam2screen(cp1, s1);
					if(arrowLabel_2dExtrapolateRay(s1.x, s1.y, sInf.x, sInf.y))
					{
						arrowLabel_finishPoint1(s1.x, s1.y, cp1.x, cp1.y, cp1.z, sInf.x, sInf.y, cp2.x, cp2.y, cp2.z, Xs, Ys, A);
						return true;
					}
				}
			}
			return false;
		}
	public:
		void arrowLabel(real x1, real y1, real z1, real x2, real y2, real z2, char const *format)
		{
			int Xs, Ys;
			real A;
			if(arrowLabel_infiniteRay(vec3(x1, y1, z1), vec3(x2, y2, z2), Xs, Ys, A))
				insert_label(1/A, Text(Xs, Ys, TRANSPARENT, format));
		}
		void arrowLabelsColor(real x1, real y1, real z1, real x2, real y2, real z2, std::vector<Label> &labels)
		{
			int Xs, Ys;
			real A;
			if(arrowLabel_infiniteRay(vec3(x1, y1, z1), vec3(x2, y2, z2), Xs, Ys, A))
			{
				for(unsigned k=0;k<labels.size();++k)
				{
					auto &label=labels[k];
					insert_label(1/A, Text(Xs, Ys, TRANSPARENT, expr[label.exNo].color, label.label));
				//	LOL_text[A].push_back(Text(Xs, Ys, TRANSPARENT, expr[label.exNo].color, label.label));
					Ys+=fontH;
				}
			}
		}
		void arrowLabelsColor(real x1, real y1, real z1, real x2, real y2, real z2, char const *const *labels, int const *colors, unsigned nLabels)
		{
			int Xs, Ys;
			real A;
			if(arrowLabel_infiniteRay(vec3(x1, y1, z1), vec3(x2, y2, z2), Xs, Ys, A))
			{
				for(unsigned l=0;l<nLabels;++l)
				{
					insert_label(1/A, Text(Xs, Ys, TRANSPARENT, colors[l], labels[l]));
				//	LOL_text[A].push_back(Text(Xs, Ys, TRANSPARENT, colors[l], labels[l]));
					Ys+=fontH;
				}
			}
		}
		void arrowLabels(real x1, real y1, real z1, real x2, real y2, real z2, std::vector<Label> &labels)
		{
			int Xs, Ys;
			real A;
			if(arrowLabel_infiniteRay(vec3(x1, y1, z1), vec3(x2, y2, z2), Xs, Ys, A))
			{
				//	for(auto &label:labels)
				for(unsigned k=0;k<labels.size();++k)
				{
					auto &label=labels[k];
					insert_label(1/A, Text(Xs, Ys, TRANSPARENT, label.label));
				//	LOL_text[A].push_back(Text(Xs, Ys, TRANSPARENT, label.label));
					Ys+=fontH;
				}
			}
		}
	//private:
	//	void point(real x, real y, real z)
	//	{
	//		push_point(vec3(x, y, z));
	//	}
	public:
		void point(real x, real y, real z, int Rcolor)
		{
		//	this->color=Rcolor, this->transparent=false;
		//	this->line_width=4;
			GL2_3D::push_point(vec3(x, y, z), Rcolor);
		}
		void point(real x, real y, real z, int Rcolor, int Icolor, int Jcolor, int Kcolor)
		{
		//	this->color=Rcolor, this->transparent=false, this->line_width=2;
			GL2_3D::push_point(vec3(x, y, z), Rcolor);//
		}

		//calculate the screen coordinates of the point (x, y, z) as integers
		//returns true if can be cast to integers
		//bool pointCoordinates(real x, real y, real z, int &Xs, int &Ys)
		//{
		//	vec3 cp;
		//	cam.world2cam(vec3(x, y, z), cp);
		//	if(cp.z>0)
		//	{
		//		real Acp=1/cp.z;
		//		vec2 s;
		//		cam.cam2screen(cp, s);
		//		if(s.x>-1e6&&s.x<1e6&&s.y>-1e6&&s.y<1e6)
		//		{
		//			Xs=(int)floor(s.x), Ys=(int)floor(s.y);
		//			return true;
		//		}
		//	}
		//	return false;
		//}

		//calculate the screen coordinates of the point (x, y, z) as doubles
		//returns true if can be cast to integers
		//bool pointCoordinates(real x, real y, real z, real &Xs, real &Ys)
		//{
		//	dvec3 cp;
		//	cam.world2cam(dvec3(x, y, z), cp);
		//	if(cp.z>0)
		//	{
		//		double Acp=1/cp.z;
		//		dvec2 s;
		//		cam.cam2screen(cp, s);
		//		Xs=s.x, Ys=s.y;
		//		if(Xs>-1e6&&Xs<1e6&&Ys>-1e6&&Ys<1e6)
		//			return true;
		//	}
		//	return false;
		//}
	private:
		struct Text
		{
			int x, y;
			bool bkMode;
			bool enable_color;	int color;
			std::string str;
			Text(int x, int y, bool bkMode, std::string &str)			:x(x), y(y), bkMode(bkMode), enable_color(false), str(str){}
			Text(int x, int y, bool bkMode, const char *a)				:x(x), y(y), bkMode(bkMode), enable_color(false), str(a){}
			Text(int x, int y, bool bkMode, int color, std::string &str):x(x), y(y), bkMode(bkMode), enable_color(true), color(color), str(str){}
			Text(int x, int y, bool bkMode, int color, const char *a)	:x(x), y(y), bkMode(bkMode), enable_color(true), color(color), str(a){}
		};
		typedef std::pair<real, Text> ZT;
		std::vector<ZT> LOL_text;//closest to farthest
		void insert_label(real z, Text const &t)
		{
			int k=0;
			for(int size=LOL_text.size();k<size&&z<=LOL_text[k].first;++k);
			LOL_text.insert(LOL_text.begin()+k, std::make_pair(z, t));
		}
	//	std::map<real, std::list<Text>> LOL_text;//closest to farthest

	public:
		//print text hud style in screen coordinates, always appears on top of text in 3d
		void textIn2D(int xs, int ys, int bkMode, char const *format, ...)
		{
			va_list args;
			va_start(args, format);
			int buflen=vsnprintf(g_buf, g_buf_size, format, args);
			va_end(args);
			if(buflen>0)
				insert_label(0, Text(xs, ys, bkMode, g_buf));
			//	LOL_text[_HUGE].push_back(Text(xs, ys, bkMode, g_buf));
		}
		void textIn2D(int xs, int ys, int bkMode, int color, char const *format, ...)
		{
			va_list args;
			va_start(args, format);
			int buflen=vsnprintf(g_buf, g_buf_size, format, args);
			va_end(args);
			if(buflen>0)
				insert_label(0, Text(xs, ys, bkMode, color, g_buf));
			//	LOL_text[_HUGE].push_back(Text(xs, ys, bkMode, color, g_buf));
		}

		//calculate the screen coordinates of the point (x, y, z) and writes the distance in Zcp1
		//as integers
		//returns true if can be cast to integers
		bool pointCoordinates3dText(real x, real y, real z, int &Xs, int &Ys)
		{
			vec3 cp;
			cam.world2cam(vec3(x, y, z), cp);
			if(cp.z>0)
			{
				real Acp=1/cp.z;
				vec2 s;
				cam.cam2screen(cp, s);
				if(s.x>-1e6&&s.x<1e6&&s.y>-1e6&&s.y<1e6)
				{
					Xs=(int)floor(s.x), Ys=(int)floor(s.y);
					return true;
				}
			}
			return false;
		}

		//print text at (x, y, z) in 3d space coordinates
		void textIn3D(real x, real y, real z, int bkMode, const char *format, ...)
		{
			cam.world2camscreen(vec3(x, y, z), cp1, s1);
			if(cp1.z>0&&abs(s1.x)<1e6&&abs(s1.y)<1e6)
			{
				va_list args;
				va_start(args, format);
				int buflen=vsnprintf(g_buf, g_buf_size, format, args);
				va_end(args);
				if(buflen>0)
					insert_label(cp1.z, Text((int)floor(s1.x), (int)floor(s1.y), bkMode, g_buf));
			}
		}

		//print text at (xs, ys) in screen coordinates, given inverse of the distance (A)
		void textIn3D_screenCoord(int xs, int ys, real A, int bkMode, char *format, ...)
		{
			if(A>0)
			{
				va_list args;
				va_start(args, format);
				int buflen=vsnprintf(g_buf, g_buf_size, format, args);
				va_end(args);
				if(buflen>0)
					insert_label(1/A, Text(xs, ys, bkMode, g_buf));
			}
		}

		//print text at (xs, ys) in screen coordinates, after using a function that writes the point distance Zcp in (Zcp1)
		void textIn3D_screenCoord(int xs, int ys, int bkMode, char *format, ...)
		{
			if(cp1.z>0)
			{
				va_list args;
				va_start(args, format);
				int buflen=vsnprintf(g_buf, g_buf_size, format, args);
				va_end(args);
				if(buflen>0)
					insert_label(cp1.z, Text(xs, ys, bkMode, g_buf));
			}
		}

		//show the text that was printed in 3d space
		void text_show()
		{
			int bkMode=getBkMode();
			for(int k=0, size=LOL_text.size();k<size;++k)
			{
				auto &vk=LOL_text[k];
				auto &text=vk.second;
				setBkMode(text.bkMode);
				if(text.enable_color)
					setTextColor(text.color);
				print(text.x, text.y, text.str.c_str(), text.str.size());
				if(text.enable_color)
					setTextColor(0);
			}
			setBkMode(bkMode);
			LOL_text.clear();
		}
		void text_dump(){LOL_text.clear();}
	};//_3D

	int colorFunction_r(double &r)
	{
		if(r!=r)
			return 0x7F7F7F;
		else if(r==_HUGE)
			return 0xFFED7F11;//arg=0
		else if(r==-_HUGE)
			return 0xFF117FED;//arg=pi
		double mag=255/G2::_pi*atan(std::abs(r)), cosx=r>0?1:-1, sinx=0;
	//	return unsigned char(mag*1.866025403784439)<<16|unsigned char(mag*0.133974596215561);
		const double cos_pi_6=0.866025403784439, sin_pi_6=0.5;
		return (unsigned char)(mag*(1+cosx*cos_pi_6))<<16|(unsigned char)(mag)<<8|(unsigned char)(mag*(1+cosx*-cos_pi_6));
	//	return unsigned char(mag*(1+cosx*cos_pi_6-sinx*sin_pi_6))<<16|unsigned char(mag*(1+sinx))<<8|unsigned char(mag*(1+cosx*-cos_pi_6-sinx*sin_pi_6));//*/

	/*	if(r!=r)
			return 0x7F7F7F;
		else if(r==_HUGE)
		{
			double mag=255/G2::_pi*atan(std::abs(r)), arg=(0);
			return unsigned char(mag*(1+cos(arg+G2::_pi/6)))<<16|unsigned char(mag*(1+cos(arg-G2::_pi/2)))<<8|unsigned char(mag*(1+cos(arg+G2::_pi*5/6)));
		}
		else if(r==-_HUGE)
		{
			double mag=255/G2::_pi*atan(std::abs(r)), arg=(G2::_pi);
			return unsigned char(mag*(1+cos(arg+G2::_pi/6)))<<16|unsigned char(mag*(1+cos(arg-G2::_pi/2)))<<8|unsigned char(mag*(1+cos(arg+G2::_pi*5/6)));
		}
		double mag=255/G2::_pi*atan(std::abs(r)), arg=atan2(0, r);
		return unsigned char(mag*(1+cos(arg+G2::_pi/6)))<<16|unsigned char(mag*(1+cos(arg-G2::_pi/2)))<<8|unsigned char(mag*(1+cos(arg+G2::_pi*5/6)));//*/
	}
	int colorFunction_i(double &i)
	{
		if(i!=i)
			return 0x7F7F7F;
		else if(i==_HUGE)
			return 0xFF3FFF3F;//arg=pi/2
		else if(i==-_HUGE)
			return 0xFFBF00BF;//arg=-pi/2
		double mag=255/G2::_pi*atan(abs(i)), cosx=0, sinx=i>0?1:-1;
	//	return unsigned char(mag*0.5)<<16|unsigned char(mag*2)<<8|unsigned char(mag*0.5);
		const double cos_pi_6=0.866025403784439, sin_pi_6=0.5;
		return (unsigned char)(mag*(1-sin_pi_6))<<16|(unsigned char)(mag*(1+1))<<8|(unsigned char)(mag*(1-sin_pi_6));//*/

	/*	if(i!=i)
			return 0x7F7F7F;
		else if(i==_HUGE)
		{
			double mag=255/G2::_pi*atan(std::abs(i)), arg=(G2::_pi/2);
			return unsigned char(mag*(1+cos(arg+G2::_pi/6)))<<16|unsigned char(mag*(1+cos(arg-G2::_pi/2)))<<8|unsigned char(mag*(1+cos(arg+G2::_pi*5/6)));
		}
		else if(i==-_HUGE)
		{
			double mag=255/G2::_pi*atan(std::abs(i)), arg=(-G2::_pi/2);
			return unsigned char(mag*(1+cos(arg+G2::_pi/6)))<<16|unsigned char(mag*(1+cos(arg-G2::_pi/2)))<<8|unsigned char(mag*(1+cos(arg+G2::_pi*5/6)));
		}
		double mag=255/G2::_pi*atan(std::abs(i)), arg=atan2(i, 0);
		return unsigned char(mag*(1+cos(arg+G2::_pi/6)))<<16|unsigned char(mag*(1+cos(arg-G2::_pi/2)))<<8|unsigned char(mag*(1+cos(arg+G2::_pi*5/6)));//*/
	}
	int colorFunction(double &r, double &i)
	{
		if(r!=r||i!=i)
			return 0x7F7F7F;
		if(r==_HUGE)
		{
			if(i==_HUGE||i==-_HUGE)
				return 0xFFFFFFFF;
			else
				return 0xFFED7F11;//arg=0
		}
		else if(r==-_HUGE)
		{
			if(i==_HUGE||i==-_HUGE)
				return 0xFFFFFFFF;
			else
				return 0xFF117FED;//arg=pi
		}
		else if(i==_HUGE)
			return 0xFF3FFF3F;//arg=pi/2
		else if(i==-_HUGE)
			return 0xFFBF00BF;//arg=-pi/2
		const double cos_pi_6=0.866025403784439, sin_pi_6=0.5;
		double hyp=sqrt(r*r+i*i), mag=255/G2::_pi*atan(hyp), cosx=r/hyp, sinx=i/hyp;
		double red=mag*(1+cosx*cos_pi_6-sinx*sin_pi_6), green=mag*(1+sinx), blue=mag*(1+cosx*-cos_pi_6-sinx*sin_pi_6);
		int color=(unsigned char)(mag*(1+cosx*cos_pi_6-sinx*sin_pi_6))<<16|(unsigned char)(mag*(1+sinx))<<8|(unsigned char)(mag*(1+cosx*-cos_pi_6-sinx*sin_pi_6));//argb
		return color;
	//	return unsigned char(mag*(1+cosx*cos_pi_6-sinx*sin_pi_6))<<16;
	}
	void colorFunction_cases(double r, double i, int &c)
	{
		if(r!=r||i!=i)
			c=0x7F7F7F;
		if(r==_HUGE)
		{
			if(i==_HUGE||i==-_HUGE)
				c=0xFFFFFFFF;
			else
				c=0xFFED7F11;//arg=0
		}
		else if(r==-_HUGE)
		{
			if(i==_HUGE||i==-_HUGE)
				c=0xFFFFFFFF;
			else
				c=0xFF117FED;//arg=pi
		}
		else if(i==_HUGE)
			c=0xFF3FFF3F;//arg=pi/2
		else if(i==-_HUGE)
			c=0xFFBF00BF;//arg=-pi/2
	}
	int colorFunction(Value &x)
	{
		if(x.r!=x.r||x.i!=x.i)
			return 0x7F7F7F;
		if(x.r==_HUGE)
		{
			if(x.i==_HUGE||x.i==-_HUGE)
				return 0xFFFFFFFF;
			else
				return 0xFFED7F11;//arg=0
		}
		else if(x.r==-_HUGE)
		{
			if(x.i==_HUGE||x.i==-_HUGE)
				return 0xFFFFFFFF;
			else
				return 0xFF117FED;//arg=pi
		}
		else if(x.i==_HUGE)
			return 0xFF3FFF3F;//arg=pi/2
		else if(x.i==-_HUGE)
			return 0xFFBF00BF;//arg=-pi/2
		const double cos_pi_6=0.866025403784439, sin_pi_6=0.5;//, cos_pi3_2=0, sin_pi3_2=-1, cos_pi5_6=-cos_pi_6, sin_pi5_6=sin_pi_6;
		double _1_hyp=inv_sqrt(x.r*x.r+x.i*x.i), mag=255/G2::_pi*atan(1/_1_hyp), cosx=x.r*_1_hyp, sinx=x.i*_1_hyp;
	//	double hyp=sqrt(x.r*x.r+x.i*x.i), mag=255/G2::_pi*atan(hyp), cosx=x.r/hyp, sinx=x.i/hyp;
		return (unsigned char)(mag*(1+cosx*cos_pi_6-sinx*sin_pi_6))<<16|(unsigned char)(mag*(1+sinx))<<8|(unsigned char)(mag*(1+cosx*-cos_pi_6-sinx*sin_pi_6));
	//	return (unsigned char)(mag*(1+cosx*cos_pi_6-sinx*sin_pi_6))<<16;
	//	return (unsigned char)(mag*(1+cosx*cos_pi_6-sinx*sin_pi_6))<<16|(unsigned char)(mag*(1+cosx*cos_pi3_2-sinx*sin_pi3_2))<<8|(unsigned char)(mag*(1+cosx*cos_pi5_6-sinx*sin_pi5_6));
	/*	if(x.r!=x.r||x.i!=x.i)
			return 0x7F7F7F;
		if(x.r==_HUGE)
		{
			if(x.i==_HUGE||x.i==-_HUGE)
				return 0xFFFFFFFF;
			else
			{
				double mag=127.5, arg=(0);
				return unsigned char(mag*(1+cos(arg+G2::_pi/6)))<<16|unsigned char(mag*(1+cos(arg-G2::_pi/2)))<<8|unsigned char(mag*(1+cos(arg+G2::_pi*5/6)));
			}
		}
		else if(x.r==-_HUGE)
		{
			if(x.i==_HUGE||x.i==-_HUGE)
				return 0xFFFFFFFF;
			else
			{
				double mag=-127.5, arg=(G2::_pi);
				return unsigned char(mag*(1+cos(arg+G2::_pi/6)))<<16|unsigned char(mag*(1+cos(arg-G2::_pi/2)))<<8|unsigned char(mag*(1+cos(arg+G2::_pi*5/6)));
			}
		}
		else if(x.i==_HUGE)
		{
			double mag=255/G2::_pi*atan(sqrt(x.r*x.r+x.i*x.i)), arg=(G2::_pi/2);
			return unsigned char(mag*(1+cos(arg+G2::_pi/6)))<<16|unsigned char(mag*(1+cos(arg-G2::_pi/2)))<<8|unsigned char(mag*(1+cos(arg+G2::_pi*5/6)));
		}
		else if(x.i==-_HUGE)
		{
			double mag=255/G2::_pi*atan(sqrt(x.r*x.r+x.i*x.i)), arg=(-G2::_pi/2);
			return unsigned char(mag*(1+cos(arg+G2::_pi/6)))<<16|unsigned char(mag*(1+cos(arg-G2::_pi/2)))<<8|unsigned char(mag*(1+cos(arg+G2::_pi*5/6)));
		}
		mag=255/G2::_pi*atan(sqrt(x.r*x.r+x.i*x.i)); double arg=atan2(x.i, x.r);//600ms
		int v1=unsigned char(mag*(1+cos(arg+G2::_pi/6)))<<16|unsigned char(mag*(1+cos(arg-G2::_pi/2)))<<8|unsigned char(mag*(1+cos(arg+G2::_pi*5/6)));//*/
	}
	void colorFunction_q(Value const &x, int &r, int &i, int &j, int &k)
	{
		const double f=255*2/G2::_pi;
		((unsigned char*)&r)[x.r<0]=(unsigned char)(atan(abs(x.r))*f);
		((unsigned char*)&i)[x.i<0]=(unsigned char)(atan(abs(x.i))*f);
		((unsigned char*)&j)[x.j<0]=(unsigned char)(atan(abs(x.j))*f);
		((unsigned char*)&k)[x.k<0]=(unsigned char)(atan(abs(x.k))*f);
		//r=unsigned char(f*atan(abs(x.r)))<<(16&-(x.r<0));
		//i=unsigned char(f*atan(abs(x.i)))<<(16&-(x.i<0));
		//j=unsigned char(f*atan(abs(x.j)))<<(16&-(x.j<0));
		//k=unsigned char(f*atan(abs(x.k)))<<(16&-(x.k<0));
		//r=x.r>=0?unsigned char(255*atan(x.r)*2/G2::_pi):unsigned char(255*atan(-x.r)*2/G2::_pi)<<16;
		//i=x.i>=0?unsigned char(255*atan(x.i)*2/G2::_pi):unsigned char(255*atan(-x.i)*2/G2::_pi)<<16;
		//j=x.j>=0?unsigned char(255*atan(x.j)*2/G2::_pi):unsigned char(255*atan(-x.j)*2/G2::_pi)<<16;
		//k=x.k>=0?unsigned char(255*atan(x.k)*2/G2::_pi):unsigned char(255*atan(-x.k)*2/G2::_pi)<<16;
	}

	double		Xstart, Xsample, Ystart, Yend, Ysample, Zstart, Zsample;
	int			Xplaces, Yplaces, Zplaces, ndrSize;
	double		*p;
	double		T;//time from count start
	int			x, y, z;
	double		fx(){return Xstart+x*Xsample;}
	double		fy(){return Ystart+y*Ysample;}
	double		fz(){return Zstart+z*Zsample;}
	double		fc(){return *p;}
	double		fy2(){return Yend-y*Ysample;}
	typedef double (*F_fn)();
	void		choose_fill_fn		(Variable &var, F_fn &f, int component, bool &LOL_1_const)
	{
		switch((&var.varTypeR)[component])
		{
		case 'x':f=&fx, LOL_1_const=0;break;
		case 'y':f=&fy, LOL_1_const=0;break;
		case 'z':f=&fz, LOL_1_const=0;break;
		case 'c':f=&fc, p=&var.val.r+component;break;
		case 't':f=&fc, p=&T;break;
		}
	};
	void		choose_fill_fn_2di	(Variable &var, F_fn &f, int component, bool &LOL_1_const)
	{//2di: inverted y
		switch((&var.varTypeR)[component])
		{
		case 'x':f=&fx, LOL_1_const=0;break;
		case 'y':f=&fy2, LOL_1_const=0;break;//
		case 'z':f=&fz, LOL_1_const=0;break;
		case 'c':f=&fc, p=&var.val.r+component;break;
		case 't':f=&fc, p=&T;break;
		}
	};
	
	double XXstart, XXstep, XYstart, XYstep, YXstart, YXstep, YYstart, YYstep;
	double fxx(){return XXstart+x*XXstep;}
	double fxy(){return XYstart+y*XYstep;}
	double fyx(){return YXstart+y*YXstep;}//x and y swapped
	double fyy(){return YYstart+x*YYstep;}
	//double fyx(){return YXstart+x*YXstep;}
	//double fyy(){return YYstart+y*YYstep;}
	void		choose_fill_fn_l2d_X(Variable &var, F_fn &f, int component, bool &LOL_1_const)
	{
		switch((&var.varTypeR)[component])
		{
		case 'x':f=&fxx, LOL_1_const=0;break;
		case 'y':f=&fxy, LOL_1_const=0;break;
		case 'c':f=&fc, p=&var.val.r+component;break;
		case 't':f=&fc, p=&T;break;
		}
	}
	void		choose_fill_fn_l2d_Y(Variable &var, F_fn &fb, int component, bool &LOL_1_const)
	{
		switch((&var.varTypeR)[component])
		{
		case 'x':fb=&fyx, LOL_1_const=0;break;
		case 'y':fb=&fyy, LOL_1_const=0;break;
		case 'c':fb=&fc, p=&var.val.r+component;break;
		case 't':fb=&fc, p=&T;break;
		}
	}
	void		resize_terms(Expression &ex, int ndrSize)
	{
		//ceil_half(Xplaces), Xplaces<<=1;
		//int ndrSize=Xplaces*Yplaces*Zplaces;
	//	int ndrSize_2=ndrSize>>1;
	//	int ndrSize_2=(ndrSize>>1)+(ndrSize&1);
		for(int kn=0, knEnd=ex.n.size();kn<knEnd;++kn)
		{
			auto &n=ex.n[kn];
			n.r.resize(ndrSize);
			if(n.mathSet>='c')
			{
				n.i.resize(ndrSize);
				if(n.mathSet=='h')
				{
					n.j.resize(ndrSize);
					n.k.resize(ndrSize);
				}
			}
		}
	}
	void		initialize_const_comp(Expression &ex, int kn, double val, int component, int offset, int x1, int x2, int y1, int y2, int z1, int z2, int Xplaces, int Yplaces, int kzStep)
	{
		auto &n=ex.n[kn];
		auto &ndr=(&n.r)[component];
		auto _First=ndr.begin()+offset+x1, _Last=ndr.begin()+offset+x2;
		for(int kz=Xplaces*Yplaces*z1, kzEnd=Xplaces*Yplaces*z2;kz<kzEnd;kz+=kzStep)
			for(int ky=kz+Xplaces*y1, kyEnd=kz+Xplaces*y2;ky<kyEnd;ky+=Xplaces)
				std::fill(_First+ky, _Last+ky, val);
	}
	void		initialize_component(Expression &ex, int kn, decltype(&choose_fill_fn) choose_fn, int component,
		int offset, int x1, int x2, int y1, int y2, int z1, int z2, int Xplaces, int Yplaces, int kzStep)
	{//x1, x2 multiples of two
		auto &n=ex.n[kn];
		auto &var=ex.variables[n.varNo];
		bool const_comp=true;
		double (*f)();
		choose_fn(var, f, component, const_comp);
		if(const_comp)
			initialize_const_comp(ex, kn, f(), component, offset, x1, x2, y1, y2, z1, z2, Xplaces, Yplaces, kzStep);
		else
		{
			auto &ndr=(&n.r)[component];
			z=z1;
			for(int kz=z1*kzStep;z<z2;++z, kz+=kzStep)
			{
				y=y1;
				for(int ky=kz+Xplaces*y1;y<y2;++y, ky+=Xplaces)
					for(x=x1;x<x2;++x)
						ndr[offset+ky+x]=f();
			}
		}
	}
	void		fill_range(Expression &ex, decltype(&choose_fill_fn) choose_fn,
		int offset, int x1, int x2, int y1, int y2, int z1, int z2, int Xplaces, int Yplaces, int Zplaces, int kzStep)
	{//x1, x2 multiples of two
	//	offset+=offset&1, x1-=x1&1, x2+=x2&1, Xplaces+=Xplaces&1, kzStep=Xplaces*Yplaces;
		for(int kn=0, knEnd=ex.n.size();kn<knEnd;++kn)
		{
			auto &n=ex.n[kn];
			if(n.constant)
			{
				initialize_const_comp(ex, kn, ex.data[kn].r, 0, offset, x1, x2, y1, y2, z1, z2, Xplaces, Yplaces, kzStep);
				if(n.mathSet>='c')
				{
					initialize_const_comp(ex, kn, ex.data[kn].i, 1, offset, x1, x2, y1, y2, z1, z2, Xplaces, Yplaces, kzStep);
					if(n.mathSet=='h')
					{
						initialize_const_comp(ex, kn, ex.data[kn].j, 2, offset, x1, x2, y1, y2, z1, z2, Xplaces, Yplaces, kzStep);
						initialize_const_comp(ex, kn, ex.data[kn].k, 3, offset, x1, x2, y1, y2, z1, z2, Xplaces, Yplaces, kzStep);
					}
					else
					{
						if(n.j.size())
							memset(&n.j[0], 0, n.j.size()*sizeof(double));
						if(n.k.size())
							memset(&n.k[0], 0, n.k.size()*sizeof(double));
					}
				}
				else if(n.i.size())
					memset(&n.i[0], 0, n.i.size()*sizeof(double));
			}
			else//the term is a variable
			{
				auto &var=ex.variables[n.varNo];
				initialize_component(ex, kn, choose_fn, 0, offset, x1, x2, y1, y2, z1, z2, Xplaces, Yplaces, kzStep);
				if(var.mathSet>='c')
				{
					initialize_component(ex, kn, choose_fn, 1, offset, x1, x2, y1, y2, z1, z2, Xplaces, Yplaces, kzStep);
					if(var.mathSet=='h')
					{
						initialize_component(ex, kn, choose_fn, 2, offset, x1, x2, y1, y2, z1, z2, Xplaces, Yplaces, kzStep);
						initialize_component(ex, kn, choose_fn, 3, offset, x1, x2, y1, y2, z1, z2, Xplaces, Yplaces, kzStep);
					}
					else
					{
						if(n.j.size())
							memset(&n.j[0], 0, n.j.size()*sizeof(double));
						if(n.k.size())
							memset(&n.k[0], 0, n.k.size()*sizeof(double));
					}
				}
				else if(n.i.size())
					memset(&n.i[0], 0, n.i.size()*sizeof(double));
			}
		}
		//for(int kn=0;kn<ex.n.size();++kn)
		//	ndr_to_clipboard_2d((double*)ex.n[kn].r.p, Xplaces, Yplaces);//
	}

	//inline double current_time()
	//{
	//	static timespec t1={0, 0};
	//	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t1);
	//	return t1.tv_sec+1e-9*t1.tv_nsec;
	//}
	class		Solve
	{
	public:
		double
			Tstart,//current time
			T_elapsed,//time from previous call to sync
			T;//time from count start
		void reset(){Tstart=now_seconds(), T_elapsed=0;}
		void synchronize(){T=T_elapsed+now_seconds()-Tstart;}
		void pause(){T_elapsed=T;}
		void resume(){Tstart=now_seconds();}
	};
	class		Solve_0D:public Solve
	{
	public:
		void full(Expression &ex)
		{
			modes::T=T;
			modes::Xplaces=1, modes::Yplaces=1, modes::Zplaces=1, modes::ndrSize=1;
			resize_terms(ex, 2);
			fill_range(ex, choose_fill_fn, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1);
			solve_disc(ex, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1,	0, 0, 0, 0, 0, 0);
		//	::solve_disc(ex, 0, 0, 2, 0, 1, 0, 1, 2, 1, 1,	0, 0, 0, 0, 0, 0);
		}
	};
	
	void solve_zerocross_1d(Expression &ex, int x1, int x2)
	{
		auto &n_res=ex.n[ex.resultTerm];//
		auto &ndr_r=n_res.r, &ndr_i=n_res.i, &ndr_j=n_res.j, &ndr_k=n_res.k;
		switch(ex.resultMathSet)
		{
		case 'R':
			for(int x=x1, vEnd=x2-2;x<vEnd;++x)//{x1,x2}={0,Xoffset2} or {Xplaces-Xoffset, Xplaces}		ndr.size()==Xplaces+2
		//	for(int x=0, vEnd=ndr.size()-2;x<vEnd;++x)//ndr.size()==Xplaces+2
			{
				auto &V0=ndr_r[x], &V1=ndr_r[x+1], &V2=ndr_r[x+2];//V0 <= cross(V0, V1, V2)
				if(V1==0)
					V0=true;
				else if(!ex.discontinuities[x]&&std::signbit(V0)!=std::signbit(V1))//cross at L?
				{
					double d_min=V0/(V0-V1);
					if(!ex.discontinuities[x+1]&&std::signbit(V1)!=std::signbit(V2))//cross at R?
					{
						double d=V2/(V2-V1);
						d_min=(std::abs(d_min+d)+std::abs(d_min-d))/2;
					}
					V0=d_min;
				}
				else
				{
					if(!ex.discontinuities[x+1]&&std::signbit(V1)!=std::signbit(V2))//cross at R?
						V0=V2/(V2-V1);
					else
						V0=false;
				}
			}
			break;
		case 'c':
			{
				bool Tshade=false;//trans-pixel shade
				double Tx;
				if(!ex.discontinuities[x1])
				{
					CompRef V0(ndr_r[x1], ndr_i[x1]), V1(ndr_r[x1+1], ndr_i[x1+1]);
					double dr=V1.r-V0.r, di=V1.i-V0.i;
					double Tx=-.5*(V0.r*dr+V0.i*di)/(dr*dr+di*di);
					if(Tx>=0&&Tx<1)
					{
						double rx=V0.r+dr*Tx, ix=V0.i+di*Tx;
						Tshade=rx*rx+ix*ix<.25;
					}
				}
				for(int v=x1, vEnd=x2-2;v<vEnd;++v)
			//	for(int v=0, vEnd=ndr.size()-2;v<vEnd;++v)//the only difference w/ full
				{
					CompRef V0(ndr_r[v], ndr_i[v]), V1(ndr_r[v+1], ndr_i[v+1]), V2(ndr_r[v+2], ndr_i[v+2]);
				//	auto &V0=ndr[v], &V1=ndr[v+1], &V2=ndr[v+2];
					if(V1.r==0&&V1.i==0)
						V0.r=true;
					else
					{
						bool shade=false;
						double Rx=0;
						if(!ex.discontinuities[v+1])
						{
							double dr=V2.r-V1.r, di=V2.i-V1.i;
							double Rx=-.5*(V1.r*dr+V1.i*di)/(dr*dr+di*di);
							if(Rx>=0&&Rx<1)
							{
								double rx=V1.r+dr*Rx, ix=V1.i+di*Rx;
								shade=rx*rx+ix*ix<.25;//why 0.25?
							}
						}
						if(shade)//cross at R?
						{
							if(Tshade)//cross at L?		LR
								V0.r=(std::abs(Tx+Rx)+std::abs(Tx-Rx))/2;
							//	V0.r=(Tx+Rx+std::abs(Tx-Rx))/2;
							else//	R
								V0.r=Rx;
							Tx=1-Rx;
						}
						else
						{
							if(Tshade)
								V0.r=Tx;
							else
								V0.r=false;
						}
						Tshade=shade;
					}
				}
			}
			break;
		case 'h':
			{
				bool Tshade=false;//trans-pixel shade
				double Tx;
				if(!ex.discontinuities[x1])
				{
					QuatRef V0(ndr_r[x1], ndr_i[x1], ndr_j[x1], ndr_k[x1]), V1(ndr_r[x1+1], ndr_i[x1+1], ndr_j[x1+1], ndr_k[x1+1]);
				//	auto &V0=ndr[x1], &V1=ndr[x1+1];
					double dr=V1.r-V0.r, di=V1.i-V0.i, dj=V1.j-V0.j, dk=V1.k-V0.k;
					double Tx=-.5*(V0.r*dr+V0.i*di+V0.j*dj+V0.k*dk)/(dr*dr+di*di+dj*dj+dk*dk);
					if(Tx>=0&&Tx<1)
					{
						double rx=V0.r+dr*Tx, ix=V0.i+di*Tx, jx=V1.j+dj*Tx, kx=V1.k+dk*Tx;
						Tshade=rx*rx+ix*ix+jx*jx+kx*kx<.25;//why 0.25?
					}
				}
				for(int v=x1, vEnd=x2-2;v<vEnd;++v)
				{
					QuatRef V0(ndr_r[v], ndr_i[v], ndr_j[v], ndr_k[v]),
						V1(ndr_r[v+1], ndr_i[v+1], ndr_j[v+1], ndr_k[v+1]),
						V2(ndr_r[v+2], ndr_i[v+2], ndr_j[v+2], ndr_k[v+2]);
				//	auto &V0=ndr[v], &V1=ndr[v+1], &V2=ndr[v+2];
					if(V1.r==0&&V1.i==0)
						V0.r=true;
					else
					{
						bool shade=false;
						double Rx=0;
						if(!ex.discontinuities[v])
						{
							double dr=V2.r-V1.r, di=V2.i-V1.i, dj=V2.j-V1.j, dk=V2.k-V1.k;
							double Rx=-.5*(V1.r*dr+V1.i*di+V0.j*dj+V0.k*dk)/(dr*dr+di*di+dj*dj+dk*dk);
							if(Rx>=0&&Rx<1)
							{
								double rx=V1.r+dr*Rx, ix=V1.i+di*Rx, jx=V1.j+dj*Rx, kx=V1.k+dk*Rx;
								shade=rx*rx+ix*ix+jx*jx+kx*kx<.25;
							}
						}
						if(shade)//cross at R?
						{
							if(Tshade)//cross at L?		LR
								V0.r=(std::abs(Tx+Rx)+std::abs(Tx-Rx))/2;
							//	V0=(Tx+Rx+std::abs(Tx-Rx))/2;
							else//	R
								V0.r=Rx;
							Tx=1-Rx;
						}
						else
						{
							if(Tshade)
								V0.r=Tx;
							else
								V0.r=false;
						}
						Tshade=shade;
					}
				}
			}
			break;
		}
	}
	class		Solve_1D_Implicit:public Solve
	{
	public:
		double Xsample, Xstart, Xstart_s, *aXstart;
		int Xplaces;
	private:
		//int x;
		//double *p;
		//double fx(){return *aXstart+x*Xsample;}
		////double fx(){return Xstart+x*Xsample;}
		//double fc(){return *p;}
		int sa[6];//XsrcStart, XsrcEnd, XdestStart		+padded version
		int ra[4];//XsolveStart, XsolveEnd				+padded version
		//void LOL_1(Variable &variables, double (Solve_1D_Implicit::*&f)(), int varType, int &LOL_1_const)
		//{
		//	switch(varType)
		//	{
		//	case 'x':f=&Solve_1D_Implicit::fx, LOL_1_const=0;break;
		//	case 'c':f=&Solve_1D_Implicit::fc, p=&variables.val.r;break;
		//	case 't':f=&Solve_1D_Implicit::fc, p=&T;break;
		//	}
		//}
	public:
		void full_resize(double VX, double DX, int Xplaces)
		{
			this->Xplaces=Xplaces;
			Xsample=DX/Xplaces;
			Xstart_s=VX-DX/2, Xstart=Xstart_s+Xsample;
		//	Xstart=VX-DX/2, Xstart_s=Xstart-Xsample;
		}
	//	void full_resize(double VX, double DX, int Xplaces){this->Xplaces=Xplaces, this->Xstart=VX-DX/2, this->Xsample=DX/Xplaces;}
		void full(Expression &ex)
		{
			int aXplaces;
			if(ex.resultLogicType>=2)
				aXstart=&Xstart_s, aXplaces=Xplaces+2;
			else
				aXstart=&Xstart, aXplaces=Xplaces;
			modes::Xstart=*aXstart, modes::Xsample=Xsample, modes::T=T;
			modes::Xplaces=aXplaces, modes::Yplaces=1, modes::Zplaces=1, modes::ndrSize=aXplaces;
			resize_terms(ex, aXplaces);
			fill_range(ex, choose_fill_fn, 0, 0, aXplaces, 0, 1, 0, 1, aXplaces, 1, 1, aXplaces);
			switch(ex.resultLogicType)
			{
			case 1://&& ## || < <= > >=		logic/inequality
				::solve_disc(ex, 0, 0, aXplaces, 0, 1, 0, 1, aXplaces, 1, 1,	0, 0, 0, 0, 0, 0);
				break;
			//ex.resultLogicType>=2		zero cross anti-aliasing
			case 2://=		equation
			case 3://!=		anti-equation
				{
					int Xplaces1=Xplaces+1, Xplaces2=Xplaces+2;
					ex.discontinuities.assign(Xplaces2, false);
					unsigned ndrSize=aXplaces;

					::solve_disc(ex, 0, 0, aXplaces, 0, 1, 0, 1, aXplaces, 1, 1,	disc_1d_in_u, disc_1d_in_b, disc_1d_in_t, disc_1d_out,		0, true);//
				//	::solve_disc(ex, 0, 0, aXplaces, 0, 1, 0, 1, aXplaces, 1, 1,	0, 0, 0, 0,		0, true);//
					subtract_NDRs(ex, 0, aXplaces, 0, 1, aXplaces, 1);
				//	auto &n_res=ex.n[ex.resultTerm];//
				//	int ndrSize=n_res.r.size()<<1;
				//	auto ndr_r=(double*)n_res.r.p, ndr_i=(double*)n_res.i.p, ndr_j=(double*)n_res.j.p, ndr_k=(double*)n_res.k.p;
					solve_zerocross_1d(ex, 0, ndrSize);
				}
				break;
			}
		}
	private:
		void shift_		(std::vector<double> &ndr, int *a, int)
		{
			int XSstart=a[0], XSend=a[1], XDstart=a[2];
			std::copy(ndr.begin()+XSstart, ndr.begin()+XSend, ndr.begin()+XDstart);
		}
		void shift_r	(std::vector<double> &ndr, int *a, int aXplaces)
		{
			int XSstart=a[0], XSend=a[1], XDstart=a[2];
			auto rb=ndr.rbegin();
		//	auto rb=std::reverse_iterator<double*>(ndr+aXplaces);
			std::copy(rb+aXplaces-1-XSstart, rb+aXplaces-1-XSend, rb+aXplaces-1-XDstart);
		//	auto rb=rbegin(ndr, aXplaces);
		//	std::copy(rb+aXplaces-1-XSstart, rb+aXplaces-1-XSend, rb+aXplaces-1-XDstart);
		}
		typedef void (modes::Solve_1D_Implicit::*Shift_fn)(std::vector<double>&, int*, int);

		Shift_fn shift;
	public:
		void partial_bounds(double VX, double DX, int Xoffset)
		{
			Xsample=DX/Xplaces;
			Xstart_s=VX-DX/2, Xstart=Xstart_s+Xsample;
		//	Xstart=VX-DX/2, Xstart_s=Xstart-Xsample;
			int Xoffset2=std::abs(Xoffset)+2;
			int Xplaces2=Xplaces+2;
		//	this->Xstart=VX-DX/2, this->Xsample=DX/Xplaces;
				 if(Xoffset>0)					shift=(Shift_fn)&Solve_1D_Implicit::shift_,	sa[0]=Xoffset,				sa[1]=Xplaces,	sa[2]=0,				ra[0]=Xplaces-Xoffset,		ra[1]=Xplaces,
																					sa[3]=Xoffset,				sa[4]=Xplaces2,	sa[5]=0,				ra[2]=Xplaces2-Xoffset2,	ra[3]=Xplaces2;
			else if(Xoffset<0)Xoffset=-Xoffset,	shift=(Shift_fn)&Solve_1D_Implicit::shift_r,	sa[0]=Xplaces-1-Xoffset,	sa[1]=-1,		sa[2]=Xplaces-1,		ra[0]=0,					ra[1]=Xoffset,
																					sa[3]=Xplaces2-1-Xoffset,	sa[4]=-1,		sa[5]=Xplaces2-1,		ra[2]=0,					ra[3]=Xoffset2;
		//		 if(Xoffset>0)					shift=&Solve_1D_Implicit::shift_,	sa[0]=Xoffset,				sa[1]=Xplaces,	sa[2]=0,				ra[0]=Xplaces-Xoffset-1,	ra[1]=Xplaces;
		//	else if(Xoffset<0)Xoffset=-Xoffset,	shift=&Solve_1D_Implicit::shift_r,	sa[0]=Xplaces-1-Xoffset,	sa[1]=-1,		sa[2]=Xplaces-1,		ra[0]=0,					ra[1]=Xoffset+1;
		}
		void partial(Expression &ex)
		{
			int saOffset, aXplaces;
			int x1, x2;
			double overlap[2];//because value ndr size = logic ndr size + 2
			{
				auto &ndr=ex.n[ex.resultTerm].r;
				if(ex.resultLogicType==1)
					saOffset=0, aXplaces=Xplaces,	x1=ra[0], x2=ra[1];
				else
				{
					saOffset=3, aXplaces=Xplaces+2,	x1=ra[2], x2=ra[3];
					if(x1==0)
						overlap[0]=ndr[0], overlap[1]=ndr[1];
				}
				(this->*shift)(ndr, sa+saOffset, aXplaces);
			}

		//	auto &ndr0=ex.n[0].ndr;
		//	(this->*shift)(ndr0);
		//	int x1=ra[0], x2=ra[1]+(ex.resultLogicType>=2);

		//	double transient;
		//	if(ex.resultLogicType>=2&&x2!=ndr0.size())//= != equation, ndr.size()=Xplaces+1, shift right: {Xplaces-1-Xoffset, -1, Xplaces-1}, solve left: {0, Xoffset}
		//		transient=ndr0[x2-1];
			
			modes::Xstart=*aXstart, modes::Xsample=Xsample, modes::T=T;
			modes::Xplaces=aXplaces, modes::Yplaces=1, modes::Zplaces=1, modes::ndrSize=aXplaces;
			fill_range(ex, choose_fill_fn, 0, x1, x2, 0, 1, 0, 1, aXplaces, 1, 1, aXplaces);
			switch(ex.resultLogicType)
			{
			case 1://&& ## || < <= > >=
				solve_disc(ex, 0, x1, x2, 0, 1, 0, 1, Xplaces, 1, 1,	0, 0, 0, 0, 0, false);
				break;
			//ex.resultLogicType>=2		anti-aliasing
			case 2://=
			case 3://!=
				{
					int Xplaces1=Xplaces+1, Xplaces2=Xplaces+2;
					for(int x=x1, xEnd=x2-1;x<xEnd;++x)
						ex.discontinuities[x]=false;

					unsigned ndrSize=aXplaces;
					solve_disc(ex, 0, x1, x2, 0, 1, 0, 1, Xplaces, 1, 1,	disc_1d_in_u, disc_1d_in_b, disc_1d_in_t, disc_1d_out, 0, true);//
					subtract_NDRs(ex, x1, x2, 0, 1, Xplaces, 1);
					auto &ndr_r=ex.n[ex.resultTerm].r;
					solve_zerocross_1d(ex, x1, x2);
					if(x1==0)
						ndr_r[x2-2]=overlap[0], ndr_r[x2-1]=overlap[1];
				}
				break;
			}
		}
	};
	class		Solve_1D:public Solve
	{
	public:
		double Xstart, Xsample;
		int Xplaces;
		bool enable_disc;
		Disc_fn disc_in_u, disc_in_b, disc_in_t, disc_out;
		Solve_1D(bool enable_disc):enable_disc(enable_disc),
			disc_in_u(enable_disc?disc_1d_in_u:0), disc_in_b(enable_disc?disc_1d_in_b:0), disc_in_t(enable_disc?disc_1d_in_t:0), disc_out(enable_disc?disc_1d_out:0)
		{}
	private:
		//int x;
		//double *p;
		//double fx(){return Xstart+x*Xsample;}
		//double fc(){return *p;}
		int sa[3], ra[2];
		//void LOL_1(Variable &variables, double (modes::Solve_1D::*&f)(), int varType, int &LOL_1_const)
		//{
		//	switch(varType)
		//	{
		//	case 'x':f=&Solve_1D::fx, LOL_1_const=0;break;
		//	case 'c':f=&Solve_1D::fc, p=&variables.val.r;break;
		//	case 't':f=&Solve_1D::fc, p=&T;break;
		//	}
		//}
	public:
		void full_resize(double VX, double DX, int Xplaces){this->Xplaces=Xplaces, this->Xstart=VX-DX/2, this->Xsample=DX/Xplaces;}
		void full(Expression &ex)
		{
			modes::Xstart=Xstart, modes::Xsample=Xsample, modes::T=T;
			modes::Xplaces=Xplaces, modes::Yplaces=1, modes::Zplaces=1, modes::ndrSize=Xplaces;
			resize_terms(ex, Xplaces);
			fill_range(ex, choose_fill_fn, 0, 0, Xplaces, 0, 1, 0, 1, Xplaces, 1, 1, Xplaces);
		//	ndr_to_clipboard_1d(ex, Xplaces);//

			if(enable_disc)
				ex.discontinuities.assign(Xplaces-1, false);
			::solve_disc(ex, 0, 0, Xplaces, 0, 1, 0, 1, Xplaces, 1, 1,		disc_in_u, disc_in_b, disc_in_t, disc_out, 0, false);//
		//	ndr_to_clipboard_1d(ex, Xplaces);//
		}
	private:
		void shift_		(std::vector<double> &ndr, std::vector<bool> &discontinuities)
		{
			int &XSstart=sa[0], &XSend=sa[1], &XDstart=sa[2];
			std::copy(ndr.begin()+XSstart, ndr.begin()+XSend, ndr.begin()+XDstart);
			if(enable_disc&&XSstart<Xplaces-1)
			{
				auto begin=discontinuities.begin();
				std::copy(begin+XSstart, begin+XSend-1, begin+XDstart);
			}
		}
		void shift_r	(std::vector<double> &ndr, std::vector<bool> &discontinuities)
		{
			int XSstart=sa[0], XSend=sa[1], XDstart=sa[2];
			{
				auto rb=ndr.rbegin();
			//	auto rb=std::reverse_iterator<double*>(ndr+Xplaces);
			//	auto rb=rbegin(ndr, Xplaces);
				std::copy(rb+Xplaces-1-XSstart, rb+Xplaces-1-XSend, rb+Xplaces-1-XDstart);
			}
			if(enable_disc&&XSstart>0)
			{
				auto rbegin=discontinuities.rbegin();
				std::copy(rbegin+Xplaces-2-(XSstart-1), rbegin+Xplaces-2-XSend, rbegin+Xplaces-2-(XDstart-1));
			}
		}
		typedef void(modes::Solve_1D::*Shift_fn)(std::vector<double>&, std::vector<bool>&);
		Shift_fn shift;
	public:
		void partial_bounds	(double VX, double DX, int Xoffset)
		{
			this->Xstart=VX-DX/2, this->Xsample=DX/Xplaces;
				 if(Xoffset>0)					shift=(Shift_fn)&Solve_1D::shift_,		sa[0]=Xoffset,				sa[1]=Xplaces,	sa[2]=0,				++Xoffset,	ra[0]=Xplaces-Xoffset,	ra[1]=Xplaces;
			else if(Xoffset<0)Xoffset=-Xoffset,	shift=(Shift_fn)&Solve_1D::shift_r,	sa[0]=Xplaces-1-Xoffset,	sa[1]=-1,		sa[2]=Xplaces-1,		++Xoffset,	ra[0]=0,				ra[1]=Xoffset;
		}
		void partial(Expression &ex)
		{
			(this->*shift)(ex.n[ex.resultTerm].r, ex.discontinuities);
			int x1=ra[0], x2=ra[1];
			modes::Xstart=Xstart, modes::Xsample=Xsample, modes::T=T;
			modes::Xplaces=Xplaces, modes::Yplaces=1, modes::Zplaces=1, modes::ndrSize=Xplaces;
			fill_range(ex, choose_fill_fn, 0, x1, x2, 0, 1, 0, 1, Xplaces, 1, 1, Xplaces);
			if(enable_disc)
			{
			//	std::fill(ex.discontinuities.begin()+x1, ex.discontinuities.begin()+x2-1, false);
				for(int v=x1;v<x2-1;++v)
					ex.discontinuities[v]=false;
			}
			::solve_disc(ex, 0, x1, x2, 0, 1, 0, 1, Xplaces, 1, 1,		disc_in_u, disc_in_b, disc_in_t, disc_out, 0, false);//
		//	::solve_disc(ex, 0, x1, x2, 0, 1, 0, 1, Xplaces, 1, 1,		0, 0, 0, 0, 0, false);//
		}
	};
	double exColorRA, exColorRB, exColorGA, exColorGB, exColorBA, exColorBB;
	void DimDIBPixelUnchecked_mono				(int **prgb, int x, int y, double a)
	{
		auto p=(unsigned char*)((*prgb)+(y-1)*w+x-1);
	//	auto p=(unsigned char*)&prgb[0][(y-1)*w+x-1];
	//	auto p=(unsigned char*)&(*prgb)[(y-1)*w+x-1];
		p[0]=(unsigned char)(p[0]*a), p[1]=(unsigned char)(p[1]*a), p[2]=(unsigned char)(p[2]*a);
	}
	void DimDIBPixelUnchecked_mono_complement	(int **prgb, int x, int y, double a)
	{
		auto p=(unsigned char*)&(*prgb)[(y-1)*w+x-1];
		p[0]-=(unsigned char)(a*p[0])/2, p[1]-=(unsigned char)(a*p[1])/2, p[2]-=(unsigned char)(a*p[2])/2;
	}
	void DimDIBPixelUnchecked_color				(int **prgb, int x, int y, double a)
	{
		auto p=(unsigned char*)&(*prgb)[(y-1)*w+x-1];
			
	//	p[0]=(unsigned char(exColorBA*a)+exColorBB)*p[0]>>8;
	//	p[1]=(unsigned char(exColorGA*a)+exColorGB)*p[1]>>8;
	//	p[2]=(unsigned char(exColorRA*a)+exColorRB)*p[2]>>8;
		p[0]=(unsigned char)((exColorBA*a+exColorBB)*p[0]);
		p[1]=(unsigned char)((exColorGA*a+exColorGB)*p[1]);
		p[2]=(unsigned char)((exColorRA*a+exColorRB)*p[2]);
	}
	void DimDIBPixelUnchecked_color_complement	(int **prgb, int x, int y, double a)
	{
		a=1-a;
		auto p=(unsigned char*)&(*prgb)[(y-1)*w+x-1];
			
	//	p[0]=(unsigned char(exColorBA*a)+exColorBB)*p[0]>>8;
	//	p[1]=(unsigned char(exColorGA*a)+exColorGB)*p[1]>>8;
	//	p[2]=(unsigned char(exColorRA*a)+exColorRB)*p[2]>>8;
		p[0]=(unsigned char)((exColorBA*a+exColorBB)*p[0]);
		p[1]=(unsigned char)((exColorGA*a+exColorGB)*p[1]);
		p[2]=(unsigned char)((exColorRA*a+exColorRB)*p[2]);
	}
	void solve_zerocross_2d(Expression &ex, int **prgb, int x1, int x2, int y1, int y2, int Xplaces, int Yplaces)
	{//inflated_range
		switch(ex.resultLogicType)
		{
		case 1://logic/inequality	just evaluate
			{
				auto &ndr=ex.n[ex.resultTerm].r;
				double A0, A1, A2;
				if(nExpr[11]==1)
					A0=A1=A2=.5;
				else
				{
					auto pp=(unsigned char*)&ex.color;
					A0=(0xFF-pp[0])/510., A1=(0xFF-pp[1])/510., A2=(0xFF-pp[2])/510.;
				//	A0=.5-pp[0]/510., A1=.5-pp[1]/510., A2=.5-pp[2]/510.;
				}
				for(int y=y1;y<y2;++y)
				{
					for(int p=w*y+x1, pn=Xplaces*y+x1, pEnd=w*y+x2;p<pEnd;++p, ++pn)
					{
						auto pp=(unsigned char*)&(*prgb)[p];
						auto &a=ndr[pn];
						pp[0]-=(unsigned char)(A0*a*pp[0]);
						pp[1]-=(unsigned char)(A1*a*pp[1]);
						pp[2]-=(unsigned char)(A2*a*pp[2]);//warning C4244
					//	pp[3]=127;//
					}
				}
			}
			break;
		//ex.resultLogicType>=2		zero cross curve, anti-aliasing
		case 2:
			//zero cross	_/\_		= equation
		case 3:
			//				_  _
			//zero cross	 \/			!= anti-equation?
			{
				int Xplaces1=Xplaces+1, Yplaces1=Yplaces+1,
					Xplaces2=Xplaces+2, Yplaces2=Yplaces+2;
				unsigned yDiscOffset=Xplaces1*Yplaces2;
				switch(ex.resultMathSet)
				{
				case 'R':
					if(x2-x1>=3&&y2-y1>=3)//redundant check		Xoffset>0||Yoffset>0	x2-x1>2&&y2-y1>2
					{
						int x11=x1+1, y11=y1+1, x2_1=x2-1, y2_1=y2-1;
						auto &ndr=ex.n[ex.resultTerm].r;

						const double aa_thickness=1,//
				
							_1_aa_thickness=1/aa_thickness;
						const int aa_bound=int(std::round(aa_thickness));

						//crossings
						int XCsize=Xplaces1*Yplaces2, YCsize=Xplaces2*Yplaces1,
							Xplaces3=Xplaces2+aa_bound;
						std::vector<double>
							Xcross(XCsize, -1),//._
							Ycross(YCsize, -1);//!
						std::vector<char> shade(Xplaces3*(Yplaces2+aa_bound));
						{
							// _	top left
							//|_!
							auto&v00=ndr[Xplaces2*y1	+x1], &v01=ndr[Xplaces2*y1	+x11],
								&v10=ndr[Xplaces2*y11	+x1], &v11=ndr[Xplaces2*y11	+x11];
							auto&corner=shade[Xplaces3*y11+x11];
							if(std::signbit(v00)!=std::signbit(v01)&&!ex.discontinuities[				Xplaces1*y1	+x1])
								corner=true, Xcross[Xplaces1*y1		+x1]=(0-v00)/(v01-v00);
							if(std::signbit(v10)!=std::signbit(v11)&&!ex.discontinuities[				Xplaces1*y11+x1])
								corner=true, Xcross[Xplaces1*y11	+x1]=(0-v10)/(v11-v10);
							if(std::signbit(v00)!=std::signbit(v10)&&!ex.discontinuities[yDiscOffset+	Xplaces2*y1	+x1])
								corner=true, Ycross[Xplaces2*y1		+x1]=(0-v00)/(v10-v00);
							if(std::signbit(v01)!=std::signbit(v11)&&!ex.discontinuities[yDiscOffset+	Xplaces2*y1+x11])
								corner=true, Ycross[Xplaces2*y1		+x11]=(0-v01)/(v11-v01);
							if(v11==0)
								corner=true, Xcross[Xplaces1*y11	+x1]=Ycross[Xplaces2*y1		+x11]=0.5;
						}
						{
							// _	top right
							//._|
							auto&v00=ndr[Xplaces2*y1	+x2_1-1], &v01=ndr[Xplaces2*y1	+x2_1],
								&v10=ndr[Xplaces2*y11	+x2_1-1], &v11=ndr[Xplaces2*y11	+x2_1];
							auto&corner=shade[Xplaces3*y11+x2_1-1];
							if(std::signbit(v00)!=std::signbit(v01)&&!ex.discontinuities[				Xplaces1*y1		+x2_1-1])
								corner=true, Xcross[Xplaces1*y1		+x2_1-1]=(0-v00)/(v01-v00);
							if(std::signbit(v10)!=std::signbit(v11)&&!ex.discontinuities[				Xplaces1*y11	+x2_1-1])
								corner=true, Xcross[Xplaces1*y11	+x2_1-1]=(0-v10)/(v11-v10);
							if(std::signbit(v01)!=std::signbit(v11)&&!ex.discontinuities[yDiscOffset+	Xplaces2*y11	+x2_1])
								corner=true, Ycross[Xplaces2*y11	+x2_1]=(0-v01)/(v11-v01);
							if(v10==0)
								corner=true, Xcross[Xplaces1*y11	+x2_1-1]=Ycross[Xplaces2*y11	+x2_1]=0.5;
						}
						{
							//  .	bottom left
							//|_|
							auto&v00=ndr[Xplaces2*(y2_1-1)	+x1], &v01=ndr[Xplaces2*(y2_1-1)+x11],
								&v10=ndr[Xplaces2* y2_1		+x1], &v11=ndr[Xplaces2* y2_1	+x11];
							auto&corner=shade[Xplaces3*(y2_1-1)+x11];
							if(std::signbit(v10)!=std::signbit(v11)&&!ex.discontinuities[				Xplaces1* y2_1		+x1])
								corner=true, Xcross[Xplaces1*y2_1		+x1]=(0-v10)/(v11-v10);
							if(std::signbit(v00)!=std::signbit(v10)&&!ex.discontinuities[yDiscOffset+	Xplaces2*(y2_1-1)	+x1])
								corner=true, Ycross[Xplaces2*(y2_1-1)	+x1]=(0-v00)/(v10-v00);
							if(std::signbit(v01)!=std::signbit(v11)&&!ex.discontinuities[yDiscOffset+	Xplaces2*(y2_1-1)	+x11])
								corner=true, Ycross[Xplaces2*(y2_1-1)	+x11]=(0-v01)/(v11-v01);
							if(v01==0)
								corner=true, Ycross[Xplaces2*(y2_1-1)	+x11]=0.5;
						}
						for(int x=x11, xEnd=x2_1-1;x<xEnd;++x)
						{
							// _	upper row
							//._!
							auto&v00=ndr[Xplaces2*y1	+x], &v01=ndr[Xplaces2*y1	+x+1],
								&v10=ndr[Xplaces2*y11	+x], &v11=ndr[Xplaces2*y11	+x+1];
							auto&p0=shade[Xplaces3*y11+x], &p1=shade[Xplaces3*y11+x+1];
							if(std::signbit(v00)!=std::signbit(v01)&&!ex.discontinuities[				Xplaces1*y1		+x])
								p0=p1=true, Xcross[Xplaces1*y1	+x]=(0-v00)/(v01-v00);
							if(std::signbit(v10)!=std::signbit(v11)&&!ex.discontinuities[				Xplaces1*y11	+x])
								p0=p1=true, Xcross[Xplaces1*y11	+x]=(0-v10)/(v11-v10);
							if(std::signbit(v01)!=std::signbit(v11)&&!ex.discontinuities[yDiscOffset+	Xplaces2*y1		+x+1])
								p0=p1=true, Ycross[Xplaces2*y1	+x+1]=(0-v01)/(v11-v01);
							if(v10==0)
								p0=true, Xcross[Xplaces1*y11	+x]=0.5;
							if(v11==0)
								p1=true, Xcross[Xplaces1*y11	+x]=0.5;
						}
						for(int y=y11, yEnd=y2_1-1;y<yEnd;++y)
						{
							//  .	left column
							//|_!
							auto&v00=ndr[Xplaces2* y   +x1], &v01=ndr[Xplaces2* y   +x11],
								&v10=ndr[Xplaces2*(y+1)+x1], &v11=ndr[Xplaces2*(y+1)+x11];
							auto&p0=shade[Xplaces3* y   +x11],
								&p1=shade[Xplaces3*(y+1)+x11];
							if(std::signbit(v10)!=std::signbit(v11)&&!ex.discontinuities[				Xplaces1*(y+1)	+x1])
								p0=p1=true, Xcross[Xplaces1*(y+1)	+x1]=(0-v10)/(v11-v10);
							if(std::signbit(v00)!=std::signbit(v10)&&!ex.discontinuities[yDiscOffset	+Xplaces2*y		+x1])
								p0=p1=true, Ycross[Xplaces2*y	+x1]=(0-v00)/(v10-v00);
							if(std::signbit(v01)!=std::signbit(v11)&&!ex.discontinuities[yDiscOffset	+Xplaces2*y		+x11])
								p0=p1=true, Ycross[Xplaces2*y	+x11]=(0-v01)/(v11-v01);
							if(v01==0)
								p0=true, Ycross[Xplaces2*y	+x11]=0.5;
							if(v11==0)
								p1=true, Ycross[Xplaces2*y	+x11]=0.5;
						}
			
						auto aa_straight=[&](double d){return d>aa_thickness?1:d*_1_aa_thickness;};
						auto aa_close=[&](double x, double y)->double//	!_
						{
							double d=x*y*inv_sqrt(x*x+y*y);
						//	double d=std::abs(x*y)*inv_sqrt(x*x+y*y);
							return d>aa_thickness?1:d*_1_aa_thickness;
						};
						auto aa_middle=[&](double a, double b)->double//	|. |
						{
							double b_a=b-a, d=a*inv_sqrt(1+b_a*b_a);
							return d>aa_thickness?1:d*_1_aa_thickness;
						};
						auto aa_far=[&](double x, double y)->double//	.7
						{
							if(x==1.||y==1.)
								return 1.;
							double x_1=x-1, y_1=y-1;
							double d=std::abs(x*y-1)*inv_sqrt(x_1*x_1+y_1*y_1);
							return d>aa_thickness?1:d*_1_aa_thickness;
						};

						//{
						//	auto p=(unsigned char*)&ex.color;
						//	exColorBA=1-p[0], exColorBB=p[0];
						//	exColorGA=1-p[1], exColorGB=p[1];
						//	exColorRA=1-p[2], exColorRB=p[2];
						//}
						{
							auto p=(unsigned char*)&ex.color;
							double a=p[0]/255.;
							exColorBA=1-a, exColorBB=a;
							a=p[1]/255.;
							exColorGA=1-a, exColorGB=a;
							a=p[2]/255.;
							exColorRA=1-a, exColorRB=a;
						}
						auto DimDIBPixelUnchecked=ex.resultLogicType==2?
								nExpr[11]==1?&DimDIBPixelUnchecked_mono				:&DimDIBPixelUnchecked_color
							:	nExpr[11]==1?&DimDIBPixelUnchecked_mono_complement	:&DimDIBPixelUnchecked_color_complement;
						for(int y=y1+aa_bound, yEnd=y2_1;y<yEnd;++y)
						{
						//	for(int x=x1+aa_bound, xEnd=minimum(x2_1, w+1);x<xEnd;++x)
							for(int x=x1+aa_bound, xEnd=x2_1;x<xEnd;++x)
							{
							//	double a=.5+.5*tanh(ndr[Xplaces2*y+x]);
							////	auto a=.5+.5*tanh(Xcross[Xplaces1*y+x]);
							////	auto a=.5+.5*tanh(Ycross[Xplaces2*y+x]);
							////	auto a=shade[sp];
							//	auto p=(unsigned char*)&(*prgb)[w*(y-1)+x-1];
							////	p[0]=p[1]=p[2]=0xFF*a;
							//	p[0]*=a, p[1]*=a, p[2]*=a;

								int ndrP=Xplaces2*y+x;
								int sp=Xplaces3*y+x;
								{
									auto							&v01=ndr[ndrP			+1],
										&v10=ndr[ndrP+Xplaces2],	&v11=ndr[ndrP+Xplaces2	+1];
									//auto&v00=ndr[ndrP			],	&v01=ndr[ndrP			+1],
									//	&v10=ndr[ndrP+Xplaces2	],	&v11=ndr[ndrP+Xplaces2	+1];
									//if(v00==0)
									//{
									//	Xcross[Xplaces1*(y+1)+x]=0;
									//	shade[sp]=true;
									//}
									if(v10==0)
									{
										Xcross[Xplaces1*(y+1)+x]=0.5;
										shade[sp			]=true, shade[sp			+1]=true;
										shade[sp+Xplaces3	]=true, shade[sp+Xplaces3	+1]=true;
										shade[sp+Xplaces3*2	]=true, shade[sp+Xplaces3*2	+1]=true;
									}
								//	if(v10==0||std::signbit(v10)!=std::signbit(v11)&&!ex.discontinuities[Xplaces1*(y+1)+x])
									else if(std::signbit(v10)!=std::signbit(v11)&&!ex.discontinuities[Xplaces1*(y+1)+x])
									{
										Xcross[Xplaces1*(y+1)+x]=(0-v10)/(v11-v10);
										shade[sp			]=true, shade[sp			+1]=true;
										shade[sp+Xplaces3	]=true, shade[sp+Xplaces3	+1]=true;
										shade[sp+Xplaces3*2	]=true, shade[sp+Xplaces3*2	+1]=true;
									}
									if(v01==0)
									{
										Ycross[ndrP+1]=0.5;
										shade[sp			]=true, shade[sp			+1]=true, shade[sp			+2]=true;
										shade[sp+Xplaces3	]=true, shade[sp+Xplaces3	+1]=true, shade[sp+Xplaces3	+2]=true;
									}
								//	if(v01==0||std::signbit(v01)!=std::signbit(v11)&&!ex.discontinuities[yDiscOffset+ndrP+1])
									else if(std::signbit(v01)!=std::signbit(v11)&&!ex.discontinuities[yDiscOffset+ndrP+1])
									{
										Ycross[ndrP+1]=(0-v01)/(v11-v01);
										shade[sp			]=true, shade[sp			+1]=true, shade[sp			+2]=true;
										shade[sp+Xplaces3	]=true, shade[sp+Xplaces3	+1]=true, shade[sp+Xplaces3	+2]=true;
									}
								}
								if(shade[sp])
								{
									//	 	Dx
									//	Lx	+	Rx
									//		Ux  '
									auto		&Dx=Ycross[Xplaces2*(y-1)+x],
										&Lx=Xcross[Xplaces1*y+x-1],		&Rx=Xcross[Xplaces1*y+x],
												&Ux=Ycross[Xplaces2* y   +x];
									double d, d_min;
									if(Ux!=-1)
									{
										if(Dx!=-1)
										{
											if(Lx!=-1)
											{
												if(Rx!=-1)	//udlr	>=2 lines	\+\	/+/
												{
													d_min=aa_close(Rx, Ux);
													if(d_min>(d=aa_close(Rx, 1-Dx)))
														d_min=d;
													if(d_min>(d=aa_close(1-Lx, 1-Dx)))
														d_min=d;
													if(d_min>(d=aa_close(1-Lx, Ux)))
														d_min=d;
													(*DimDIBPixelUnchecked)(prgb, x, y, d_min);
												}
												else		//udl	>=2 lines	<+
												{
													d_min=aa_close(1-Lx, 1-Dx);
													if(d_min>(d=aa_close(1-Lx, Ux)))
														d_min=d;
													(*DimDIBPixelUnchecked)(prgb, x, y, d_min);
												}
											}
											else
											{
												if(Rx!=-1)	//ud r	>=2 lines	+>
												{
													d_min=aa_close(Rx, Ux);
													if(d_min>(d=aa_close(Rx, 1-Dx)))
														d_min=d;
													(*DimDIBPixelUnchecked)(prgb, x, y, d_min);
												}
												else		//ud				_F_
												{
													d_min=Ux;
													if(d_min>(d=1-Dx))
														d_min=d;
													(*DimDIBPixelUnchecked)(prgb, x, y, d_min);
												}
											}
										}
										else
										{
											if(Lx!=-1)
											{
												if(Rx!=-1)	//u lr	>=2 lines	\+/
												{
													d_min=aa_close(Rx, 1-Dx);
													if(d_min>(d=aa_close(1-Lx, 1-Dx)))
														d_min=d;
													(*DimDIBPixelUnchecked)(prgb, x, y, d_min);
												}
												else		//u l	>=1 line	\+
													(*DimDIBPixelUnchecked)(prgb, x, y, aa_close(1-Lx, Ux));
											}
											else
											{
												if(Rx!=-1)	//u  r	>=1 line	+/
													(*DimDIBPixelUnchecked)(prgb, x, y, aa_close(Rx, Ux));
												else		//u		>=1 line	_+_
												{
													auto &LUx=Ycross[Xplaces2* y   +x-1], &RUx=Ycross[Xplaces2* y   +x+1];
													d_min=Ux;
													if(LUx!=-1&&LUx<Ux&&d_min>(d=aa_middle(Ux, LUx)))
														d_min=d;
													if(RUx!=-1&&RUx<Ux&&d_min>(d=aa_middle(Ux, RUx)))
														d_min=d;
													(*DimDIBPixelUnchecked)(prgb, x, y, d_min);
												}
											}
										}
									}
									else
									{
										if(Dx!=-1)
										{
											if(Lx!=-1)
											{
												if(Rx!=-1)	// dlr	>=2 lines	/+\ stop
												{
													d_min=aa_close(Rx, Ux);
													if(d_min>(d=aa_close(1-Lx, Ux)))
														d_min=d;
													(*DimDIBPixelUnchecked)(prgb, x, y, d_min);
												}
												else		// dl	>=1 line	/+
													(*DimDIBPixelUnchecked)(prgb, x, y, aa_close(1-Lx, 1-Dx));
											}
											else
											{
												if(Rx!=-1)	// d r	>=1 line	+\ stop
													(DimDIBPixelUnchecked)(prgb, x, y, aa_close(Rx, 1-Dx));
												else		// d	>=1 line	F
												{
													auto &LDx=Ycross[Xplaces2*(y-1)+x-1], &RDx=Ycross[Xplaces2*(y-1)+x+1];
													d_min=1-Dx;
													if(LDx!=-1&&LDx>Dx&&d_min>(d=aa_middle(1-Dx, 1-LDx)))
														d_min=d;
													if(RDx!=-1&&RDx>Dx&&d_min>(d=aa_middle(1-Dx, 1-RDx)))
														d_min=d;
													(*DimDIBPixelUnchecked)(prgb, x, y, d_min);
												}
											}
										}
										else
										{
											if(Lx!=-1)
											{
												if(Rx!=-1)	//  lr	>=1 line	|+|		//>=1 line	[-]
												{
													d_min=Rx;
													if(d_min>(d=1-Lx))
														d_min=d;
													(*DimDIBPixelUnchecked)(prgb, x, y, d_min);
												}
												else		//  l	>=1 line	|+
												{
													auto &DLx=Xcross[Xplaces1*(y-1)+x-1], &ULx=Xcross[Xplaces1*(y+1)+x-1];
													d_min=1-Lx;
													if(DLx!=-1&&DLx>Lx&&d_min>(d=aa_middle(1-Lx, 1-DLx)))
														d_min=d;
													if(ULx!=-1&&ULx>Lx&&d_min>(d=aa_middle(1-Lx, 1-ULx)))
														d_min=d;
													(*DimDIBPixelUnchecked)(prgb, x, y, d_min);
												}
											}
											else
											{
												if(Rx!=-1)	//   r	>=1 line	+|
												{
													auto &DRx=Xcross[Xplaces1*(y-1)+x  ], &URx=Xcross[Xplaces1*(y+1)+x  ];
													d_min=Rx;
													if(DRx!=-1&&DRx<Rx&&d_min>(d=aa_middle(Rx, DRx)))
														d_min=d;
													if(URx!=-1&&URx<Rx&&d_min>(d=aa_middle(Rx, URx)))
														d_min=d;
													(*DimDIBPixelUnchecked)(prgb, x, y, d_min);
												}
												else		//		>=0 lines
												{
													auto
														&DLx=Xcross[Xplaces1*(y-1)+x-1],	&LDx=Ycross[Xplaces2*(y-1)+x-1],//	 _    _
														&DRx=Xcross[Xplaces1*(y-1)+x  ],	&RDx=Ycross[Xplaces2*(y-1)+x+1],//	| 1  2 | D
														&ULx=Xcross[Xplaces1*(y+1)+x-1],	&LUx=Ycross[Xplaces2* y   +x-1],//	    .
														&URx=Xcross[Xplaces1*(y+1)+x  ],	&RUx=Ycross[Xplaces2* y   +x+1];//	|_3  4_| U
													d_min=_HUGE;
													if(DLx!=-1&&LDx!=-1&&d_min>(d=aa_far(1-DLx, 1-LDx)))//1
														d_min=d;
													if(DRx!=-1&&RDx!=-1&&d_min>(d=aa_far(  DRx, 1-RDx)))//2
														d_min=d;
													if(ULx!=-1&&LUx!=-1&&d_min>(d=aa_far(1-ULx,   LUx)))//3
														d_min=d;
													if(URx!=-1&&RUx!=-1&&d_min>(d=aa_far(  URx,   RUx)))//4
														d_min=d;
													if(d_min!=_HUGE)
														(*DimDIBPixelUnchecked)(prgb, x, y, d_min>aa_thickness?1:d_min*_1_aa_thickness);
												}
											}
										}
									}
								}
								else
									(*DimDIBPixelUnchecked)(prgb, x, y, 1);//*/
							}
						}
					}
					break;
				case 'c':
					break;
				case 'h':
					break;
				}
				break;
			}
		}
	//	bitmap_to_clipboard(*prgb, w, h);//
	}
	class		Solve_2D_Implicit:public Solve
	{
	public:
	//	unsigned texture_id;
	//	HBITMAP hBitmap;
		int *rgb, *g_rgb, **prgb;
		Solve_2D_Implicit():rgb((int*)malloc(1)), g_rgb((int*)malloc(1)){}
	//	Solve_2D_Implicit():texture_id(0), rgb(0), g_rgb(0){}
		double
			Xsample, Xstart, Xstart_s, *aXstart,
			Ysample, Yend, Yend_s, *aYend;
		int ndrSize, Xplaces, Yplaces;
	private:
		//int y, x;
		//double *p;
		//double fx(){return *aXstart	+x*Xsample;}
		//double fy(){return *aYend	-y*Ysample;}
		//double fc(){return *p;}
		int sa[12],//shift arguments: XSrcStart XSrcEnd XDstStart YSrcStart YSrcEnd YDstStart		x resultLogicType{1, 2/3}
			ra[16];//range arguments: x1 x2 y1 y2	up to 2 rectangles		x resultLogicType{1, 2/3}
		//void LOL_1(Variable &variables, double (modes::Solve_2D_Implicit::*&f)(), int varType, int &LOL_1_const)
		//{
		//	switch(varType)
		//	{
		//	case 'x':f=&Solve_2D_Implicit::fx, LOL_1_const=0;break;
		//	case 'y':f=&Solve_2D_Implicit::fy, LOL_1_const=0;break;
		//	case 'c':f=&Solve_2D_Implicit::fc, p=&variables.val.r;break;
		//	case 't':f=&Solve_2D_Implicit::fc, p=&T;break;
		//	}
		//}

	//	unsigned char exColorRA, exColorRB, exColorGA, exColorGB, exColorBA, exColorBB;
		double exColorRA, exColorRB, exColorGA, exColorGB, exColorBA, exColorBB;
		void DimDIBPixelUnchecked_mono				(int x, int y, double a)
		{
		//	auto p=(unsigned char*)&prgb[0][(y-1)*w+x-1];
			auto p=(unsigned char*)&(*prgb)[(y-1)*w+x-1];
			p[0]=(unsigned char)(p[0]*a), p[1]=(unsigned char)(p[1]*a), p[2]=(unsigned char)(p[2]*a);
		}
		void DimDIBPixelUnchecked_mono_complement	(int x, int y, double a)
		{
			auto p=(unsigned char*)&(*prgb)[(y-1)*w+x-1];
			p[0]-=(unsigned char)(a*p[0])/2, p[1]-=(unsigned char)(a*p[1])/2, p[2]-=(unsigned char)(a*p[2])/2;
		}
		void DimDIBPixelUnchecked_color				(int x, int y, double a)
		{
			auto p=(unsigned char*)&(*prgb)[(y-1)*w+x-1];
			
		//	p[0]=(unsigned char(exColorBA*a)+exColorBB)*p[0]>>8;
		//	p[1]=(unsigned char(exColorGA*a)+exColorGB)*p[1]>>8;
		//	p[2]=(unsigned char(exColorRA*a)+exColorRB)*p[2]>>8;
			p[0]=(unsigned char)((exColorBA*a+exColorBB)*p[0]);
			p[1]=(unsigned char)((exColorGA*a+exColorGB)*p[1]);
			p[2]=(unsigned char)((exColorRA*a+exColorRB)*p[2]);
		}
		void DimDIBPixelUnchecked_color_complement	(int x, int y, double a)
		{
			a=1-a;
			auto p=(unsigned char*)&(*prgb)[(y-1)*w+x-1];
			
		//	p[0]=(unsigned char(exColorBA*a)+exColorBB)*p[0]>>8;
		//	p[1]=(unsigned char(exColorGA*a)+exColorGB)*p[1]>>8;
		//	p[2]=(unsigned char(exColorRA*a)+exColorRB)*p[2]>>8;
			p[0]=(unsigned char)((exColorBA*a+exColorBB)*p[0]);
			p[1]=(unsigned char)((exColorGA*a+exColorGB)*p[1]);
			p[2]=(unsigned char)((exColorRA*a+exColorRB)*p[2]);
		}
	public:
		void full_resize(double VX, double DX, double VY, double DY, int Xplaces, int Yplaces)
		{
			this->Xplaces=Xplaces, this->Yplaces=Yplaces, ndrSize=Xplaces*Yplaces;
			Xsample=DX/Xplaces;
			Xstart_s=VX-DX/2, Xstart=Xstart_s+Xsample;
		//	Xstart=VX-DX/2, Xstart_s=Xstart-Xsample;
			Ysample=DY/Yplaces, Yend=VY+DY/2, Yend_s=Yend+Ysample;

			int err=0;
//			if(!texture_id)
//				glGenTextures(1, &texture_id), err=glGetError();
			rgb=(int*)realloc(rgb, ndrSize*sizeof(int));
			g_rgb=(int*)realloc(g_rgb, ndrSize*sizeof(int));
			for(int k=0;k<ndrSize;++k)
				rgb[k]=0x7FFFFFFF;//white with alpha=half
		//	memset(rgb, 127, ndrSize*sizeof(int));
//			glBindTexture(GL_TEXTURE_2D, texture_id), err=glGetError();
//			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST), err=glGetError();
//			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST), err=glGetError();
//			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Xplaces, Yplaces, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgb), err=glGetError();

//			DeleteObject(hBitmap);
//			BITMAPINFO bmpInfo={{sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB, 0, 0, 0, 0, 0}};
//			hBitmap=CreateDIBSection(0, &bmpInfo, DIB_RGB_COLORS, (void**)&rgb, 0, 0);
		}
		void full(Expression &ex)
		{
			int aXplaces, aYplaces;
			if(ex.resultLogicType>=2)
				aXstart=&Xstart_s, aYend=&Yend_s, aXplaces=Xplaces+2, aYplaces=Yplaces+2;
			else
				aXstart=&Xstart, aYend=&Yend, aXplaces=Xplaces, aYplaces=Yplaces;
			int aNDRsize=aXplaces*aYplaces;

			modes::Xstart=*aXstart, modes::Xsample=Xsample, modes::Yend=*aYend, modes::Ysample=Ysample, modes::T=T;
			modes::Xplaces=aXplaces, modes::Yplaces=aYplaces, modes::Zplaces=1, modes::ndrSize=aNDRsize;

			resize_terms(ex, aNDRsize);
			fill_range(ex, choose_fill_fn_2di, 0, 0, aXplaces, 0, aYplaces, 0, 1, aXplaces, aYplaces, 1, aNDRsize);

			switch(ex.resultLogicType)
			{
			case 1://&& || ## < > <= >=		logic expresion / inequality		logic
				//for(int kn=0;kn<ex.n.size();++kn)
				//	ndr_to_clipboard_2d((double*)ex.n[kn].r.p, Xplaces, Yplaces);//
				solve_disc(ex, 0, 0, Xplaces, 0, Yplaces, 0, 1, Xplaces, Yplaces, 1,	0, 0, 0, 0, 0, 0);
				break;

				//= !=		equation	ex.resultLogicType>=2		zero cross curve, anti-aliasing
			case 2:
				//zero cross	_/\_
			case 3:
				//				_  _
				//zero cross	 \/
				{
					int Xplaces1=Xplaces+1, Yplaces1=Yplaces+1,
						Xplaces2=Xplaces+2, Yplaces2=Yplaces+2;

					unsigned yDiscOffset=Yplaces2*Xplaces1;
					ex.discontinuities.assign(yDiscOffset+Yplaces1*Xplaces2, false);

					//if(ex.n[0].r.size()==(aXplaces*aYplaces>>1))
					//	ndr_to_clipboard_2d((double*)ex.n[0].r.p, aXplaces, aYplaces);//
					//if(ex.n[1].r.size()==(aXplaces*aYplaces>>1))
					//	ndr_to_clipboard_2d((double*)ex.n[1].r.p, aXplaces, aYplaces);//
					
					::solve_disc(ex, 0, 0, aXplaces, 0, aYplaces, 0, 1, aXplaces, aYplaces, 1,		disc_i2d_in_u, disc_i2d_in_b, disc_i2d_in_t, disc_i2d_out, yDiscOffset, true);
				//	::solve_disc(ex, 0, 0, aXplaces, 0, aYplaces, 0, 1, aXplaces, aYplaces, 1,		0, 0, 0, 0, 0, true);//

					//if(ex.n[0].r.size()==(aXplaces*aYplaces>>1))
					//	ndr_to_clipboard_2d((double*)ex.n[0].r.p, aXplaces, aYplaces);//

					subtract_NDRs(ex, 0, aXplaces, 0, aYplaces, aXplaces, aYplaces);
				//	subtract_NDRs(ex, 0, aXplaces, 0, aYplaces, Xplaces, Yplaces);

					//if(ex.n[0].r.size()==(aXplaces*aYplaces>>1))
					//	ndr_to_clipboard_2d((double*)ex.n[0].r.p, aXplaces, aYplaces);//
				}
				break;
			}
		}
		void draw(Expression &ex)
		{
			//1: && || ## < > <= >=		logic expresion / inequality		logic
			//2: =	3: !=				equation							zero cross curve, anti-aliasing		inflate by 2

			int inflate=(ex.resultLogicType>=2)<<1;
			modes::exColorRA=exColorRA, modes::exColorRB=exColorRB, modes::exColorGA=exColorGA, modes::exColorGB=exColorGB, modes::exColorBA=exColorBA, modes::exColorBB=exColorBB;
			solve_zerocross_2d(ex, prgb, 0, w+inflate, 0, Yplaces+inflate, Xplaces, Yplaces);//inflated range
		//	solve_zerocross_2d(ex, prgb, 0, Xplaces+inflate, 0, Yplaces+inflate, Xplaces, Yplaces);//inflated range
			//if(ex.resultLogicType==1)
			//	solve_zerocross_2d(ex, prgb, 0, Xplaces, 0, Yplaces, Xplaces, Yplaces);
			//else
			//	solve_zerocross_2d(ex, prgb, 0, Xplaces+2, 0, Yplaces+2, Xplaces, Yplaces);
		}
	private:
		void drawCheckboard_range(int color, double VX, double DX, double VY, double DY, double Xstep, double Ystep, int *rDims, int clearScreen)
		{
			int x1=rDims[0]-(Xplaces-w), x2=rDims[1]+Xplaces-w, y1=rDims[2], y2=rDims[3];

//			HRGN hRgn=CreateRectRgn(x1, y1, x2, y2);
//			SelectClipRgn(ghMemDC, hRgn);

			set_color(0xFFFFFFFF), GL2_2D::draw_rectangle(x1, x2, y1, y2);
//			HPEN hWpen=(HPEN)SelectObject(ghMemDC, CreatePen(PS_SOLID, 1, 0xFFFFFF));//bgr
//			HBRUSH hWbrush=(HBRUSH)SelectObject(ghMemDC, CreateSolidBrush(0xFFFFFF));
//			Rectangle(ghMemDC, x1, y1, x2, y2);
//			DeleteObject(SelectObject(ghMemDC, hWpen));
//			DeleteObject(SelectObject(ghMemDC, hWbrush));

		//	if(!clearScreen)
		//	{
				set_color(color);
				//hPen=(HPEN)SelectObject(ghMemDC, hPen), hBrush=(HBRUSH)SelectObject(ghMemDC, hBrush);
				double Ystart=VY-DY/2, Yend=VY+DY/2, Ystepx2=Ystep*2, Xstart=VX-DX/2, Xend=VX+DX/2, Xstepx2=Xstep*2;
				for(double y=ceil((Yend-y1*DY/h)/Ystepx2)*Ystepx2, yEnd=floor((Yend-y2*DY/h)/Ystep)*Ystep;y>yEnd;y-=Ystepx2)
				{
					for(double x=floor((Xstart+x1*DX/w)/Xstepx2)*Xstepx2, xEnd=ceil((Xstart+x2*DX/w)/Xstep)*Xstep;x<xEnd;x+=Xstepx2)
					{
					//	double
						float
							ax1=(x+Xstep-Xstart)/DX*w, ay1=(Yend-y		)/DY*h, ax2=(x+Xstepx2	-Xstart)/DX*w, ay2=(Yend-y+Ystep	)/DY*h,
							bx1=(x		-Xstart)/DX*w, by1=(Yend-y+Ystep)/DY*h, bx2=(x+Xstep	-Xstart)/DX*w, by2=(Yend-y+Ystepx2	)/DY*h;
						GL2_2D::draw_rectangle(ax1, ax2, ay1, ay2);
						GL2_2D::draw_rectangle(bx1, bx2, by1, by2);
					//	Rectangle(ghMemDC, int(ax1)-(ax1<0), int(ay1)-(ay1<0)+1, int(ax2)-(ax2<0), int(ay2)-(ay2<0)+1);
					//	Rectangle(ghMemDC, int(bx1)-(bx1<0), int(by1)-(by1<0)+1, int(bx2)-(bx2<0), int(by2)-(by2<0)+1);
					}
				}
				//hPen=(HPEN)SelectObject(ghMemDC, hPen), hBrush=(HBRUSH)SelectObject(ghMemDC, hBrush);
		//	}

//			SelectClipRgn(ghMemDC, 0);
//			DeleteObject(hRgn);
		}
	public:
		void shiftAndCheckboard(int color, double VX, double DX, double VY, double DY, double Xstep, double Ystep, int clearScreen)
		{
		//	bitmap_to_clipboard(rgb, w, h);//
			if(shift==&Solve_2D_Implicit::shift_)
			{
				int XSstart=sa[0], XSend=sa[1], XDstart=sa[2], YSstart=sa[3], YSend=sa[4], YDstart=sa[5];
				if(XSend==Xplaces)
					XSend=w;
				auto _First=rgb+XSstart, _Last=rgb+XSend, _Dest=rgb+XDstart;
				for(int ky=w*YSstart, kyEnd=w*YSend, kyStep=(ky<kyEnd?1:-1)*w, ky2=w*YDstart;ky!=kyEnd;ky+=kyStep, ky2+=kyStep)
					std::copy(_First+ky, _Last+ky, _Dest+ky2);
				//for(int ky=Xplaces*YSstart, kyEnd=Xplaces*YSend, kyStep=(ky<kyEnd?1:-1)*Xplaces, ky2=Xplaces*YDstart;ky!=kyEnd;ky+=kyStep, ky2+=kyStep)
				//	std::copy(_First+ky, _Last+ky, _Dest+ky2);
			}
			else if(shift==&Solve_2D_Implicit::shift_r)
			{
				int XSstart=sa[0], XSend=sa[1], XDstart=sa[2], YSstart=sa[3], YSend=sa[4], YDstart=sa[5];
				auto rb=std::reverse_iterator<int*>(rgb+w*h);
				auto _First=rb+w-1-XSstart, _Last=rb+w-1-XSend, _Dest=rb+w-1-XDstart;
				for(int ky=w*(h-1-YSstart), kyEnd=w*(h-1-YSend), kyStep=(ky<kyEnd?1:-1)*w, ky2=w*(h-1-YDstart);ky!=kyEnd;ky+=kyStep, ky2+=kyStep)
					std::copy(_First+ky,_Last+ky, _Dest+ky2);
				//int XSstart=sa[0], XSend=sa[1], XDstart=sa[2], YSstart=sa[3], YSend=sa[4], YDstart=sa[5];
				//auto rb=std::reverse_iterator<int*>(rgb+Xplaces*Yplaces);
				//auto _First=rb+Xplaces-1-XSstart, _Last=rb+Xplaces-1-XSend, _Dest=rb+Xplaces-1-XDstart;
				//for(int ky=Xplaces*(Yplaces-1-YSstart), kyEnd=Xplaces*(Yplaces-1-YSend), kyStep=(ky<kyEnd?1:-1)*Xplaces, ky2=Xplaces*(Yplaces-1-YDstart);ky!=kyEnd;ky+=kyStep, ky2+=kyStep)
				//	std::copy(_First+ky,_Last+ky, _Dest+ky2);
			}
		//	bitmap_to_clipboard(rgb, w, h);//
			drawCheckboard_range(color, VX, DX, VY, DY, Xstep, Ystep, ra, clearScreen);
			if(partial==&Solve_2D_Implicit::partial_2)
				drawCheckboard_range(color, VX, DX, VY, DY, Xstep, Ystep, ra+4, clearScreen);
		//	bitmap_to_clipboard(rgb, w, h);//
		}
	private:
		void shift_		(std::vector<double> &ndr, int *a, int aXplaces, int Yplaces)
		{
			int XSstart=a[0], XSend=a[1], XDstart=a[2], YSstart=a[3], YSend=a[4], YDstart=a[5];
		//	int XSstart=sa[0], XSend=sa[1], XDstart=sa[2], YSstart=sa[3], YSend=sa[4], YDstart=sa[5];
			auto _First=ndr.begin()+XSstart, _Last=ndr.begin()+XSend, _Dest=ndr.begin()+XDstart;
			for(int ky=aXplaces*YSstart, kyEnd=aXplaces*YSend, kyStep=(ky<kyEnd?1:-1)*aXplaces, ky2=aXplaces*YDstart;ky!=kyEnd;ky+=kyStep, ky2+=kyStep)
				std::copy(_First+ky, _Last+ky, _Dest+ky2);
		}
		void shift_r	(std::vector<double> &ndr, int *a, int aXplaces, int aYplaces)
		{
			int XSstart=a[0], XSend=a[1], XDstart=a[2], YSstart=a[3], YSend=a[4], YDstart=a[5];
		//	int XSstart=sa[0], XSend=sa[1], XDstart=sa[2], YSstart=sa[3], YSend=sa[4], YDstart=sa[5];
			auto rb=ndr.rbegin();
		//	auto rb=std::reverse_iterator<double*>(ndr+aXplaces*aYplaces);
		//	auto rb=rbegin(ndr, aXplaces*aYplaces);
			auto _First=rb+aXplaces-1-XSstart, _Last=rb+aXplaces-1-XSend, _Dest=rb+aXplaces-1-XDstart;
			for(int ky=aXplaces*(aYplaces-1-YSstart), kyEnd=aXplaces*(aYplaces-1-YSend), kyStep=(ky<kyEnd?1:-1)*aXplaces, ky2=aXplaces*(aYplaces-1-YDstart);ky!=kyEnd;ky+=kyStep, ky2+=kyStep)
				std::copy(_First+ky,_Last+ky, _Dest+ky2);
		}
		typedef void(modes::Solve_2D_Implicit::*Shift_fn)(std::vector<double>&, int*, int, int);
		Shift_fn shift;
		void solve_range	(Expression &ex, int *a)
		{
			int aXplaces, aYplaces;
			int x1, x2, y1, y2;
			if(ex.resultLogicType==1)
				aXstart=&Xstart,	aYend=&Yend,	aXplaces=Xplaces,	aYplaces=Yplaces,		x1=a[0], x2=a[1], y1=a[2], y2=a[3];
			else
				aXstart=&Xstart_s,	aYend=&Yend_s,	aXplaces=Xplaces+2,	aYplaces=Yplaces+2,		x1=a[8], x2=a[9], y1=a[10], y2=a[11];//x1=a[16], x2=a[17], y1=a[18], y2=a[19];//x1=a[8], x2=a[9], y1=a[10], y2=a[11];
			modes::Xstart=*aXstart, modes::Xsample=Xsample, modes::Yend=*aYend, modes::Ysample=Ysample, modes::T=T;
			modes::Xplaces=aXplaces, modes::Yplaces=aYplaces, modes::Zplaces=1, modes::ndrSize=aXplaces*aYplaces;
			fill_range(ex, choose_fill_fn_2di, 0, x1, x2, y1, y2, 0, 1, aXplaces, aYplaces, 1, modes::ndrSize);
			if(ex.resultLogicType==1)
				solve_disc(ex, 0, x1, x2, y1, y2, 0, 1, Xplaces, Yplaces, 1,	0, 0, 0, 0, 0, 0);
			else
			{
				int Xplaces1=Xplaces+1, Yplaces1=Yplaces+1,
					Xplaces2=Xplaces+2, Yplaces2=Yplaces+2;

				unsigned yDiscOffset=Xplaces1*Yplaces2;
				for(int y=y1;y<y2;++y)
				//	for(auto it=ex.discontinuities.begin()+Xplaces1*y+x1, itEnd=ex.discontinuities.begin()+Xplaces1*y+x2-1;it!=itEnd;++it)
				//		*it=false;
					for(int x=x1;x<x2-1;++x)
						ex.discontinuities[Xplaces1*y+x]=false;
				for(int y=y1;y<y2-1;++y)
				//	for(auto it=ex.discontinuities.begin()+yDiscOffset+Xplaces2*y+x1, itEnd=ex.discontinuities.begin()+Xplaces2*y+x2;it!=itEnd;++it)
				//		*it=false;
					for(int x=x1;x<x2;++x)
						ex.discontinuities[yDiscOffset+Xplaces2*y+x]=false;
			//	ndr_to_clipboard_2d(ex, aXplaces, aYplaces);//
			//	ndr_to_clipboard_2d((double*)ex.n[ex.resultTerm].r.p, aXplaces, aYplaces);//
				::solve_disc(ex, 0, x1, x2, y1, y2, 0, 1, Xplaces2, Yplaces, 1, disc_i2d_in_u, disc_i2d_in_b, disc_i2d_in_t, disc_i2d_out, yDiscOffset, true);
			//	::solve_disc(ex, 0, x1, x2, y1, y2, 0, 1, aXplaces, aYplaces, 1,		0, 0, 0, 0, 0, true);//
			//	ndr_to_clipboard_2d((double*)ex.n[ex.resultTerm].r.p, aXplaces, aYplaces);//
				subtract_NDRs(ex, x1, x2, y1, y2, aXplaces, aYplaces);
				//::solve_disc(ex, 0, x1, x2, y1, y2, 0, 1, Xplaces2, Yplaces, 1,		0, 0, 0, 0, 0, true);//
				//subtract_NDRs(ex, x1, x2, y1, y2, Xplaces, Yplaces);
			//	ndr_to_clipboard_2d(ex, aXplaces, aYplaces);//
			//	ndr_to_clipboard_2d((double*)ex.n[ex.resultTerm].r.p, aXplaces, aYplaces);//
			}
			modes::exColorRA=exColorRA, modes::exColorRB=exColorRB, modes::exColorGA=exColorGA, modes::exColorGB=exColorGB, modes::exColorBA=exColorBA, modes::exColorBB=exColorBB;
			//auto ndr=(double*)ex.n[ex.resultTerm].r.p;//
			//x1=0, x2=aXplaces, y1=0, y2=aYplaces;//
			//for(int y=y1+1, yEnd=y2-1;y<yEnd;++y)//
			//{
			//	for(int x=x1+1, xEnd=x2-1;x<xEnd;++x)//
			//	{
			//		double a=.5+.5*tanh(ndr[(Xplaces+2)*y+x]);//
			//		auto p=(unsigned char*)&(*prgb)[w*(y-1)+x-1];//
			//		p[0]=p[1]=p[2]=0xFF*a;
			//	//	p[0]*=a, p[1]*=a, p[2]*=a;
			//	}
			//}
			solve_zerocross_2d(ex, prgb, x1==0?x1:x1-(Xplaces-w), x2==aXplaces?x2:x2+Xplaces-w, y1, y2, Xplaces, Yplaces);
		//	ndr_to_clipboard_2d(ex, aXplaces, aYplaces);//
		//	ndr_to_clipboard_2d((double*)ex.n[ex.resultTerm].r.p, aXplaces, aYplaces);//
		//	bitmap_to_clipboard(rgb, w, h);//
		}
		void partial_		(Expression &ex){}
		void partial_1		(Expression &ex)
		{
			int saOffset, aXplaces, aYplaces;
			if(ex.resultLogicType==1)
				saOffset=0, aXplaces=Xplaces, aYplaces=Yplaces;
			else
				saOffset=6, aXplaces=Xplaces+2, aYplaces=Yplaces+2;
			(this->*shift)(ex.n[ex.resultTerm].r, sa+saOffset, aXplaces, aYplaces);
			solve_range(ex, ra);
		}
		void partial_2		(Expression &ex)
		{
			int saOffset, aXplaces, aYplaces;
			if(ex.resultLogicType==1)
				saOffset=0, aXplaces=Xplaces, aYplaces=Yplaces;
			else
				saOffset=6, aXplaces=Xplaces+2, aYplaces=Yplaces+2;
			(this->*shift)(ex.n[ex.resultTerm].r, sa+saOffset, aXplaces, aYplaces);
			solve_range(ex, ra), solve_range(ex, ra+4);
		}
	public:
		void partial_bounds(double VX, double DX, double VY, double DY, int Xoffset, int Yoffset)
		{
			Xsample=DX/Xplaces;
			Xstart_s=VX-DX/2, Xstart=Xstart_s+Xsample;
		//	Xstart=VX-DX/2, Xstart_s=Xstart-Xsample;
			Ysample=DY/Yplaces, Yend=VY+DY/2, Yend_s=Yend+Ysample;
			partial=&Solve_2D_Implicit::partial_;
			int Xoffset2=std::abs(Xoffset)+2, Yoffset2=std::abs(Yoffset)+2;
			int Xplaces2=Xplaces+2, Yplaces2=Yplaces+2;
				 if(Xoffset>0){							 if(Yoffset>0)					shift=&Solve_2D_Implicit::shift_,	sa[0]=Xoffset,				sa[1]=Xplaces,	sa[2]=0,			sa[3]=Yplaces-1-Yoffset,	sa[4]=-1,			sa[5]=Yplaces-1,			partial=&Solve_2D_Implicit::partial_2,	ra[0]=0,					ra[1]=Xplaces,		ra[ 2]=0,					ra[ 3]=Yoffset,							ra[ 4]=Xplaces-Xoffset,		ra[ 5]=Xplaces,		ra[ 6]=Yoffset,				ra[ 7]=Yplaces,
																															sa[6]=Xoffset,				sa[7]=Xplaces2,	sa[8]=0,			sa[9]=Yplaces2-1-Yoffset,	sa[10]=-1,			sa[11]=Yplaces2-1,													ra[8]=0,					ra[9]=Xplaces2,		ra[10]=0,					ra[11]=Yoffset2,						ra[12]=Xplaces2-Xoffset2,	ra[13]=Xplaces2,	ra[14]=Yoffset,				ra[15]=Yplaces2;

													else if(Yoffset<0)Yoffset=-Yoffset,	shift=&Solve_2D_Implicit::shift_,	sa[0]=Xoffset,				sa[1]=Xplaces,	sa[2]=0,			sa[3]=Yoffset,				sa[4]=Yplaces,		sa[5]=0,					partial=&Solve_2D_Implicit::partial_2,	ra[0]=Xplaces-Xoffset,		ra[1]=Xplaces,		ra[ 2]=0,					ra[ 3]=Yplaces-Yoffset,					ra[ 4]=0,					ra[ 5]=Xplaces,		ra[ 6]=Yplaces-Yoffset,		ra[ 7]=Yplaces,
																															sa[6]=Xoffset,				sa[7]=Xplaces2,	sa[8]=0,			sa[9]=Yoffset,				sa[10]=Yplaces2,	sa[11]=0,															ra[8]=Xplaces2-Xoffset2,	ra[9]=Xplaces2,		ra[10]=0,					ra[11]=Yplaces2-Yoffset,				ra[12]=0,					ra[13]=Xplaces2,	ra[14]=Yplaces2-Yoffset2,	ra[15]=Yplaces2;

													else								shift=&Solve_2D_Implicit::shift_,	sa[0]=Xoffset,				sa[1]=Xplaces,	sa[2]=0,			sa[3]=Yoffset,				sa[4]=Yplaces,		sa[5]=0,					partial=&Solve_2D_Implicit::partial_1,	ra[0]=Xplaces-Xoffset,		ra[1]=Xplaces,		ra[ 2]=0,					ra[ 3]=Yplaces,
																															sa[6]=Xoffset,				sa[7]=Xplaces2,	sa[8]=0,			sa[9]=Yoffset,				sa[10]=Yplaces2,	sa[11]=0,															ra[8]=Xplaces2-Xoffset2,	ra[9]=Xplaces2,		ra[10]=0,					ra[11]=Yplaces2;}

			else if(Xoffset<0){Xoffset=-Xoffset;		 if(Yoffset>0)					shift=&Solve_2D_Implicit::shift_r,	sa[0]=Xplaces-1-Xoffset,	sa[1]=-1,		sa[2]=Xplaces-1,	sa[3]=Yplaces-1-Yoffset,	sa[4]=-1,			sa[5]=Yplaces-1,			partial=&Solve_2D_Implicit::partial_2,	ra[0]=0,					ra[1]=Xplaces,		ra[ 2]=0,					ra[ 3]=Yoffset,							ra[ 4]=0,					ra[ 5]=Xoffset,		ra[ 6]=Yoffset,				ra[ 7]=Yplaces,
																															sa[6]=Xplaces2-1-Xoffset,	sa[7]=-1,		sa[8]=Xplaces2-1,	sa[9]=Yplaces2-1-Yoffset,	sa[10]=-1,			sa[11]=Yplaces2-1,													ra[8]=0,					ra[9]=Xplaces2,		ra[10]=0,					ra[11]=Yoffset2,						ra[12]=0,					ra[13]=Xoffset2,	ra[14]=Yoffset,				ra[15]=Yplaces2;

													else if(Yoffset<0)Yoffset=-Yoffset,	shift=&Solve_2D_Implicit::shift_r,	sa[0]=Xplaces-1-Xoffset,	sa[1]=-1,		sa[2]=Xplaces-1,	sa[3]=Yoffset,				sa[4]=Yplaces,		sa[5]=0,					partial=&Solve_2D_Implicit::partial_2,	ra[0]=0,					ra[1]=Xoffset,		ra[ 2]=0,					ra[ 3]=Yplaces-Yoffset,					ra[ 4]=0,					ra[ 5]=Xplaces,		ra[ 6]=Yplaces-Yoffset,		ra[ 7]=Yplaces,
																															sa[6]=Xplaces2-1-Xoffset,	sa[7]=-1,		sa[8]=Xplaces2-1,	sa[9]=Yoffset,				sa[10]=Yplaces2,	sa[11]=0,															ra[8]=0,					ra[9]=Xoffset2,		ra[10]=0,					ra[11]=Yplaces2-Yoffset,				ra[12]=0,					ra[13]=Xplaces2,	ra[14]=Yplaces2-Yoffset2,	ra[15]=Yplaces2;

													else								shift=&Solve_2D_Implicit::shift_r,	sa[0]=Xplaces-1-Xoffset,	sa[1]=-1,		sa[2]=Xplaces-1,	sa[3]=Yoffset,				sa[4]=Yplaces,		sa[5]=0,					partial=&Solve_2D_Implicit::partial_1,	ra[0]=0,					ra[1]=Xoffset,		ra[ 2]=0,					ra[ 3]=Yplaces,
																															sa[6]=Xplaces2-1-Xoffset,	sa[7]=-1,		sa[8]=Xplaces2-1,	sa[9]=Yoffset,				sa[10]=Yplaces2,	sa[11]=0,															ra[8]=0,					ra[9]=Xoffset2,		ra[10]=0,					ra[11]=Yplaces2;}

			else{										 if(Yoffset>0)					shift=&Solve_2D_Implicit::shift_,	sa[0]=Xoffset,				sa[1]=Xplaces,	sa[2]=0,			sa[3]=Yplaces-1-Yoffset,	sa[4]=-1,			sa[5]=Yplaces-1,			partial=&Solve_2D_Implicit::partial_1,	ra[0]=0,					ra[1]=Xplaces,		ra[ 2]=0,					ra[ 3]=Yoffset,
																															sa[6]=Xoffset,				sa[7]=Xplaces2,	sa[8]=0,			sa[9]=Yplaces2-1-Yoffset,	sa[10]=-1,			sa[11]=Yplaces2-1,													ra[8]=0,					ra[9]=Xplaces2,		ra[10]=0,					ra[11]=Yoffset2;

													else if(Yoffset<0)Yoffset=-Yoffset,	shift=&Solve_2D_Implicit::shift_,	sa[0]=Xoffset,				sa[1]=Xplaces,	sa[2]=0,			sa[3]=Yoffset,				sa[4]=Yplaces,		sa[5]=0,					partial=&Solve_2D_Implicit::partial_1,	ra[0]=0,					ra[1]=Xplaces,		ra[ 2]=Yplaces-Yoffset,		ra[ 3]=Yplaces,
																															sa[6]=Xoffset,				sa[7]=Xplaces2,	sa[8]=0,			sa[9]=Yoffset,				sa[10]=Yplaces2,	sa[11]=0,															ra[8]=0,					ra[9]=Xplaces2,		ra[10]=Yplaces2-Yoffset2,	ra[11]=Yplaces2;
													else;}
		}
	//	decltype(&Solve_2D_Implicit::partial_) partial;
		typedef void(modes::Solve_2D_Implicit::*Partial_fn)(Expression&);
		Partial_fn partial;
	};
	class		Solve_2D:public Solve
	{
	public:
		double Xstart, Xsample, Ystart, Ysample;
		int ndrSize, Xplaces, Yplaces;
		bool enable_disc;
		Disc_fn disc_in_u, disc_in_b, disc_in_t, disc_out;
		Solve_2D(bool enable_disc):enable_disc(enable_disc),
			disc_in_u(enable_disc?disc_2d_in_u:0), disc_in_b(enable_disc?disc_2d_in_b:0), disc_in_t(enable_disc?disc_2d_in_t:0), disc_out(enable_disc?disc_2d_out:0)
		{}
	private:
		//int y, x;
		//double *p;
		//double fx(){return Xstart+x*Xsample;}
		//double fy(){return Ystart+y*Ysample;}
		//double fc(){return *p;}
		int sa[6], ra[8];
		//void LOL_1(Variable &variables, double (modes::Solve_2D::*&f)(), int varType, int &LOL_1_const)
		//{
		//	switch(varType)
		//	{
		//	case 'x':f=&Solve_2D::fx, LOL_1_const=0;break;
		//	case 'y':f=&Solve_2D::fy, LOL_1_const=0;break;
		//	case 'c':f=&Solve_2D::fc, p=&variables.val.r;break;
		//	case 't':f=&Solve_2D::fc, p=&T;break;
		//	}
		//}
	public:
		void full_resize(double VX, double DX, double VY, double DY, int Xplaces, int Yplaces)
		{
			this->Xplaces=Xplaces, this->Yplaces=Yplaces, ndrSize=Xplaces*Yplaces, this->Xstart=VX-DX/2, this->Xsample=DX/Xplaces, this->Ystart=VY-DY/2, this->Ysample=DY/Yplaces;
		}
		void full(Expression &ex)
		{
			modes::Xstart=Xstart, modes::Xsample=Xsample, modes::Ystart=Ystart, modes::Ysample=Ysample, modes::T=T;
			modes::Xplaces=Xplaces, modes::Yplaces=Yplaces, modes::Zplaces=1, modes::ndrSize=ndrSize;

			resize_terms(ex, ndrSize);
			fill_range(ex, choose_fill_fn, 0, 0, Xplaces, 0, Yplaces, 0, 1, Xplaces, Yplaces, 1, ndrSize);

			//Xdiscs:	(Xplaces-1) * Yplaces
			//Ydiscs:	(Yplaces-1) * Xplaces
			unsigned yDiscOffset=(Xplaces-1)*Yplaces;
			if(enable_disc)
				ex.discontinuities.assign(yDiscOffset+(Yplaces-1)*Xplaces, false);
			::solve_disc(ex, 0, 0, Xplaces, 0, Yplaces, 0, 1, Xplaces, Yplaces, 1,		disc_in_u, disc_in_b, disc_in_t, disc_out, yDiscOffset, false);
		//	::solve_disc(ex, 0, 0, Xplaces, 0, Yplaces, 0, 1, Xplaces, Yplaces, 1,		0, 0, 0, 0, 0, false);//
		}
	private:
		void shift_		(std::vector<double> &ndr, std::vector<bool> &discontinuities)
		{
			int &XSstart=sa[0], &XSend=sa[1], &XDstart=sa[2], &YSstart=sa[3], &YSend=sa[4], &YDstart=sa[5];
			{
				auto _First=ndr.begin()+XSstart, _Last=ndr.begin()+XSend, _Dest=ndr.begin()+XDstart;
				for(int ys=Xplaces*YSstart, ysEnd=Xplaces*YSend, yStep=(ys<ysEnd?1:-1)*Xplaces, yd=Xplaces*YDstart;ys!=ysEnd;ys+=yStep, yd+=yStep)
					std::copy(_First+ys, _Last+ys, _Dest+yd);
			}
			if(enable_disc)
			{
				if(XSstart<Xplaces-1)
				{
					auto _First=discontinuities.begin()+XSstart, _Last=discontinuities.begin()+XSend-1, _Dest=discontinuities.begin()+XDstart;
					for(int ys=YSstart*(Xplaces-1), ysEnd=YSend*(Xplaces-1), yStep=(ys<ysEnd?1:-1)*(Xplaces-1), yd=YDstart*(Xplaces-1);ys!=ysEnd;ys+=yStep, yd+=yStep)
						std::copy(_First+ys, _Last+ys, _Dest+yd);
				}
				if(YSstart<YSend)
				{
					if(YSstart<Yplaces-1)
					{
						unsigned yDiscOffset=(Xplaces-1)*Yplaces;
						auto _First=discontinuities.begin()+yDiscOffset+YSstart, _Last=discontinuities.begin()+yDiscOffset+YSend-1, _Dest=discontinuities.begin()+yDiscOffset+YDstart;
						for(int xs=XSstart*(Yplaces-1), xsEnd=XSend*(Yplaces-1), xStep=(xs<xsEnd?1:-1)*(Yplaces-1), xd=XDstart*(Yplaces-1);xs!=xsEnd;xs+=xStep, xd+=xStep)
							std::copy(_First+xs, _Last+xs, _Dest+xd);
					}
				}
				else
				{
					if(YSstart>0)
					{
						auto _First=discontinuities.rbegin()+Yplaces-2-(YSstart-1), _Last=discontinuities.rbegin()+Yplaces-2-YSend, _Dest=discontinuities.rbegin()+Yplaces-2-(YDstart-1);
						for(int xs=(Xplaces-1-XSstart)*(Yplaces-1), xsEnd=(Xplaces-1-XSend)*(Yplaces-1), xStep=(xs<xsEnd?1:-1)*(Yplaces-1), xd=(Xplaces-1-XDstart)*(Yplaces-1);xs!=xsEnd;xs+=xStep, xd+=xStep)
							std::copy(_First+xs, _Last+xs, _Dest+xd);
					}
				}
			}
		}
		void shift_r	(std::vector<double> &ndr, std::vector<bool> &discontinuities)
		{
			int XSstart=sa[0], XSend=sa[1], XDstart=sa[2], YSstart=sa[3], YSend=sa[4], YDstart=sa[5];//98 -1 99		0 100 0
			//if(YSstart>YSend)//98 -1 99		96 -1 99
			//	int LOL_1=0;
			//for(int ky=YSstart, ky2=YDstart, kyEnd=YSend, kyStep=ky<kyEnd?1:-1;ky<kyEnd;++ky, ++ky2)
			//{
			//	for(int kx=XSstart, kx2=XDstart, kxEnd=XSend;kx>kxEnd;--kx, --kx2)
			//	{
			//		ndr[Xplaces*ky2+kx2]=ndr[Xplaces*ky+kx];
			//		//if(expr.size()&&expr[0].n.size()&&expr[0].n[0].r.size())//
			//		//{
			//		//	auto &n=expr[0].n[0];
			//		//	AVector_v2d test=n.r;
			//		//	_aligned_free(n.r.p), n.r.p=0;
			//		//	n.r=test;
			//		//}
			//	}
			//}
			//{
			//	auto _First=ndr+XSstart, _Last=ndr+XSend, _Dest=ndr+XDstart;
			//	for(int ys=Xplaces*YSstart, ysEnd=Xplaces*YSend, yStep=(ys<ysEnd?1:-1)*Xplaces, yd=Xplaces*YDstart;ys!=ysEnd;ys+=yStep, yd+=yStep)
			//		memmove(_Dest+yd, _First+ys, XSend-XSstart+ys);
			//}
			{
				auto rb=ndr.rbegin();
			//	auto rb=std::reverse_iterator<double*>(ndr+ndrSize);
				auto _First=rb+Xplaces-1-XSstart, _Last=rb+Xplaces-1-XSend, _Dest=rb+Xplaces-1-XDstart;
				for(int ky=Xplaces*(Yplaces-1-YSstart), kyEnd=Xplaces*(Yplaces-1-YSend), kyStep=(ky<kyEnd?1:-1)*Xplaces, ky2=Xplaces*(Yplaces-1-YDstart);ky!=kyEnd;ky+=kyStep, ky2+=kyStep)
				{
					std::copy(_First+ky, _Last+ky, _Dest+ky2);
					//auto start=&*(_First+ky), end=&*(_Last+ky), dst=&*(_Dest+ky2);
					//for(int kx=-1, kEnd=end-start;kx>=kEnd;--kx)
					//{
					//	dst[kx]=start[kx];
					////	if(ky==0&&kx==-1)
					//	if(ky==4300&&kx==-89)
					//		int LOL_1=0;
					//	if(expr.size()&&expr[0].n.size()&&expr[0].n[0].r.size())//
					//	{
					//		auto &n=expr[0].n[0];
					//		AVector_v2d test=n.r;
					//		_aligned_free(n.r.p), n.r.p=0;
					//		n.r=test;
					//	}
					//}
				}
			}
			if(enable_disc)
			{
				if(XSstart>0)
				{
					unsigned xDiscROffset=Xplaces*(Yplaces-1);
					auto _First=discontinuities.rbegin()+xDiscROffset+Xplaces-2-(XSstart-1), _Last=discontinuities.rbegin()+xDiscROffset+Xplaces-2-XSend, _Dest=discontinuities.rbegin()+xDiscROffset+Xplaces-2-(XDstart-1);
					for(int ys=(Yplaces-1-YSstart)*(Xplaces-1), ysEnd=(Yplaces-1-YSend)*(Xplaces-1), yStep=(ys<ysEnd?1:-1)*(Xplaces-1), yd=(Yplaces-1-YDstart)*(Xplaces-1);ys!=ysEnd;ys+=yStep, yd+=yStep)
						std::copy(_First+ys, _Last+ys, _Dest+yd);
				}
				if(YSstart<YSend)
				{
					if(YSstart<Yplaces-1)
					{
						unsigned yDiscOffset=(Xplaces-1)*Yplaces;
						auto _First=discontinuities.begin()+yDiscOffset+YSstart, _Last=discontinuities.begin()+yDiscOffset+YSend-1, _Dest=discontinuities.begin()+yDiscOffset+YDstart;
						for(int xs=XSstart*(Yplaces-1), xsEnd=XSend*(Yplaces-1), xStep=(xs<xsEnd?1:-1)*(Yplaces-1), xd=XDstart*(Yplaces-1);xs!=xsEnd;xs+=xStep, xd+=xStep)
							std::copy(_First+xs, _Last+xs, _Dest+xd);
					}
				}
				else
				{
					if(YSstart>0)
					{
						auto _First=discontinuities.rbegin()+Yplaces-2-(YSstart-1), _Last=discontinuities.rbegin()+Yplaces-2-YSend, _Dest=discontinuities.rbegin()+Yplaces-2-(YDstart-1);
						for(int xs=(Xplaces-1-XSstart)*(Yplaces-1), xsEnd=(Xplaces-1-XSend)*(Yplaces-1), xStep=(xs<xsEnd?1:-1)*(Yplaces-1), xd=(Xplaces-1-XDstart)*(Yplaces-1);xs!=xsEnd;xs+=xStep, xd+=xStep)
							std::copy(_First+xs, _Last+xs, _Dest+xd);
					}
				}
			}
		}
	//	decltype(&Solve_2D::shift_) shift;
		typedef void (modes::Solve_2D::*Shift_fn)(std::vector<double>&, std::vector<bool>&);
		Shift_fn shift;
		void solve_range	(Expression &ex, int *a)
		{
			int &x1=a[0], &x2=a[1], &y1=a[2], &y2=a[3];
			modes::Xstart=Xstart, modes::Xsample=Xsample, modes::Ystart=Ystart, modes::Ysample=Ysample, modes::T=T;
			modes::Xplaces=Xplaces, modes::Yplaces=Yplaces, modes::Zplaces=1, modes::ndrSize=ndrSize;
			fill_range(ex, choose_fill_fn, 0, x1, x2, y1, y2, 0, 1, Xplaces, Yplaces, 1, Xplaces*Yplaces);
			unsigned yDiscOffset=(Xplaces-1)*Yplaces;
			if(enable_disc)
			{
				for(int y=y1;y<y2;++y)
					for(int x=x1;x<x2-1;++x)
						ex.discontinuities[(Xplaces-1)*y+x]=false;
				for(int x=x1;x<x2;++x)
					for(int y=y1;y<y2-1;++y)
						ex.discontinuities[yDiscOffset+(Yplaces-1)*x+y]=false;
			}
			::solve_disc(ex, 0, x1, x2, y1, y2, 0, 1, Xplaces, Yplaces, 1,		disc_in_u, disc_in_b, disc_in_t, disc_out, yDiscOffset, false);
		}
		void partial_		(Expression&){}
		void partial_1		(Expression &ex)
		{
			(this->*shift)(ex.n[ex.resultTerm].r, ex.discontinuities);
			solve_range(ex, ra);
		}
		void partial_2		(Expression &ex)
		{
			(this->*shift)(ex.n[ex.resultTerm].r, ex.discontinuities);
			solve_range(ex, ra);
			solve_range(ex, ra+4);
		}
	public:
		void partial_bounds	(double VX, double DX, double VY, double DY, int Xoffset, int Yoffset)
		{
			this->Xstart=VX-DX/2, this->Xsample=DX/Xplaces, this->Ystart=VY-DY/2, this->Ysample=DY/Yplaces;
			partial=&Solve_2D::partial_;
				 if(Xoffset>0){							 if(Yoffset>0)					shift=&Solve_2D::shift_,	sa[0]=Xoffset,				sa[1]=Xplaces,	sa[2]=0,			sa[3]=Yoffset,				sa[4]=Yplaces,	sa[5]=0,					partial=&Solve_2D::partial_2,	++Xoffset, ++Yoffset,	ra[0]=Xplaces-Xoffset,	ra[1]=Xplaces,	ra[2]=0,				ra[3]=Yplaces-Yoffset,			ra[4]=0,				ra[5]=Xplaces,	ra[6]=Yplaces-Yoffset,	ra[7]=Yplaces;
													else if(Yoffset<0)Yoffset=-Yoffset,	shift=&Solve_2D::shift_,	sa[0]=Xoffset,				sa[1]=Xplaces,	sa[2]=0,			sa[3]=Yplaces-1-Yoffset,	sa[4]=-1,		sa[5]=Yplaces-1,			partial=&Solve_2D::partial_2,	++Xoffset, ++Yoffset,	ra[0]=0,				ra[1]=Xplaces,	ra[2]=0,				ra[3]=Yoffset,					ra[4]=Xplaces-Xoffset,	ra[5]=Xplaces,	ra[6]=Yoffset,			ra[7]=Yplaces;
													else								shift=&Solve_2D::shift_,	sa[0]=Xoffset,				sa[1]=Xplaces,	sa[2]=0,			sa[3]=Yoffset,				sa[4]=Yplaces,	sa[5]=0,					partial=&Solve_2D::partial_1,	++Xoffset, ++Yoffset,	ra[0]=Xplaces-Xoffset,	ra[1]=Xplaces,	ra[2]=0,				ra[3]=Yplaces;}
			else if(Xoffset<0){Xoffset=-Xoffset;		 if(Yoffset>0)					shift=&Solve_2D::shift_r,	sa[0]=Xplaces-1-Xoffset,	sa[1]=-1,		sa[2]=Xplaces-1,	sa[3]=Yoffset,				sa[4]=Yplaces,	sa[5]=0,					partial=&Solve_2D::partial_2,	++Xoffset, ++Yoffset,	ra[0]=0,				ra[1]=Xoffset,	ra[2]=0,				ra[3]=Yplaces-Yoffset,			ra[4]=0,				ra[5]=Xplaces,	ra[6]=Yplaces-Yoffset,	ra[7]=Yplaces;
													else if(Yoffset<0)Yoffset=-Yoffset,	shift=&Solve_2D::shift_r,	sa[0]=Xplaces-1-Xoffset,	sa[1]=-1,		sa[2]=Xplaces-1,	sa[3]=Yplaces-1-Yoffset,	sa[4]=-1,		sa[5]=Yplaces-1,			partial=&Solve_2D::partial_2,	++Xoffset, ++Yoffset,	ra[0]=0,				ra[1]=Xplaces,	ra[2]=0,				ra[3]=Yoffset,					ra[4]=0,				ra[5]=Xoffset,	ra[6]=Yoffset,			ra[7]=Yplaces;
													else								shift=&Solve_2D::shift_r,	sa[0]=Xplaces-1-Xoffset,	sa[1]=-1,		sa[2]=Xplaces-1,	sa[3]=Yoffset,				sa[4]=Yplaces,	sa[5]=0,					partial=&Solve_2D::partial_1,	++Xoffset, ++Yoffset,	ra[0]=0,				ra[1]=Xoffset,	ra[2]=0,				ra[3]=Yplaces;}
			else{										 if(Yoffset>0)					shift=&Solve_2D::shift_,	sa[0]=Xoffset,				sa[1]=Xplaces,	sa[2]=0,			sa[3]=Yoffset,				sa[4]=Yplaces,	sa[5]=0,					partial=&Solve_2D::partial_1,	++Xoffset, ++Yoffset,	ra[0]=0,				ra[1]=Xplaces,	ra[2]=Yplaces-Yoffset,	ra[3]=Yplaces;
													else if(Yoffset<0)Yoffset=-Yoffset,	shift=&Solve_2D::shift_,	sa[0]=Xoffset,				sa[1]=Xplaces,	sa[2]=0,			sa[3]=Yplaces-1-Yoffset,	sa[4]=-1,		sa[5]=Yplaces-1,			partial=&Solve_2D::partial_1,	++Xoffset, ++Yoffset,	ra[0]=0,				ra[1]=Xplaces,	ra[2]=0,				ra[3]=Yoffset;
													else;}
		}
	//	decltype(&Solve_2D::partial_) partial;
		typedef void(modes::Solve_2D::*Partial_fn)(Expression&);
		Partial_fn partial;
	};
	class		Solve_C2D:public Solve
	{
	public:
		int *rgb;
		Solve_C2D():rgb((int*)malloc(1)){}
		double Xstart, Xsample, Yend, Ysample;
		int ndrSize, Xplaces, Yplaces;
	private:
		//int y, x;
		//double *p;
		//double fx(){return Xstart	+x*Xsample;}
		//double fy(){return Yend		-y*Ysample;}
		//double fc(){return *p;}
		int sa[6], ra[8];
		//void LOL_1(Variable &variables, double (modes::Solve_C2D::*&f)(), int varType, int &LOL_1_const)
		//{
		//	switch(varType)
		//	{
		//	case 'x':f=&Solve_C2D::fx, LOL_1_const=0;break;
		//	case 'y':f=&Solve_C2D::fy, LOL_1_const=0;break;
		//	case 'c':f=&Solve_C2D::fc, p=&variables.val.r;break;
		//	case 't':f=&Solve_C2D::fc, p=&T;break;
		//	}
		//}
	public:
		void full_resize(double VX, double DX, double VY, double DY, int Xplaces, int Yplaces)
		{
			this->Xplaces=Xplaces, this->Yplaces=Yplaces, ndrSize=Xplaces*Yplaces, this->Xstart=VX-DX/2, this->Xsample=DX/Xplaces, this->Yend=VY+DY/2, this->Ysample=DY/Yplaces;
			rgb=(int*)realloc(rgb, Xplaces*Yplaces*sizeof(int));
		}
		void full(Expression &ex)
		{
			modes::Xstart=Xstart, modes::Xsample=Xsample, modes::Yend=Yend, modes::Ysample=Ysample, modes::T=T;
			modes::Xplaces=Xplaces, modes::Yplaces=Yplaces, modes::Zplaces=1, modes::ndrSize=ndrSize;

			resize_terms(ex, ndrSize);
			prof_add("resize");
			fill_range(ex, choose_fill_fn_2di, 0, 0, Xplaces, 0, Yplaces, 0, 1, Xplaces, Yplaces, 1, ndrSize);
			prof_add("fill full");
			solve_disc(ex, 0, 0, Xplaces, 0, Yplaces, 0, 1, Xplaces, Yplaces, 1,	0, 0, 0, 0, 0, 0);
			prof_add("solve full");
		}
		void updateRGB(Term &n)
		{
		//	Concurrency::parallel_transform(ndr.begin(), ndr.end(), rgb, (int(*)(Value&))colorFunction);
			auto prgb=&rgb;

			//Concurrency::parallel_for(0u, n.r.size(), [&](int k)
			for(int k=0;k<n.r.size();++k)
			{
				(*prgb)[k]=colorFunction(n.r[k], n.i[k]);
			//	colorFunction(n.r[k], n.i[k], (*prgb)[k<<1], (*prgb)[(k<<1)+1]);
			}//);
			prof_add("rgb");
		}
//	private:
		void shift_		(std::vector<double> &ndr_r, std::vector<double> &ndr_i)
		{
			int XSstart=sa[0], XSend=sa[1], XDstart=sa[2], YSstart=sa[3], YSend=sa[4], YDstart=sa[5];
			auto _First=ndr_r.begin()+XSstart, _Last=ndr_r.begin()+XSend, _Dest=ndr_r.begin()+XDstart;
			auto _First2=ndr_i.begin()+XSstart, _Last2=ndr_i.begin()+XSend, _Dest2=ndr_i.begin()+XDstart;
			auto _First3=rgb+XSstart, _Last3=rgb+XSend, _Dest3=rgb+XDstart;
			for(int ky=Xplaces*YSstart, kyEnd=Xplaces*YSend, kyStep=conditional_negate(Xplaces, ky>kyEnd), ky2=Xplaces*YDstart;ky!=kyEnd;ky+=kyStep, ky2+=kyStep)
			{
				std::copy(_First+ky, _Last+ky, _Dest+ky2);
				std::copy(_First2+ky, _Last2+ky, _Dest2+ky2);
				std::copy(_First3+ky, _Last3+ky, _Dest3+ky2);
			}
			prof_add("shift");
		}
		void shift_r	(std::vector<double> &ndr_r, std::vector<double> &ndr_i)
		{
			int ndrSize=Xplaces*Yplaces;
			int XSstart=sa[0], XSend=sa[1], XDstart=sa[2], YSstart=sa[3], YSend=sa[4], YDstart=sa[5];
			auto rb_r=ndr_r.rbegin(), rb_i=ndr_i.rbegin();
		//	auto rb_r=std::reverse_iterator<double*>(ndr_r+ndrSize), rb_i=std::reverse_iterator<double*>(ndr_i+ndrSize);
			auto _First=rb_r+Xplaces-1-XSstart, _Last=rb_r+Xplaces-1-XSend, _Dest=rb_r+Xplaces-1-XDstart;
			auto _First2=rb_i+Xplaces-1-XSstart, _Last2=rb_i+Xplaces-1-XSend, _Dest2=rb_i+Xplaces-1-XDstart;
			auto rb_rgb=std::reverse_iterator<int*>(rgb+ndrSize);
			auto _First3=rb_rgb+Xplaces-1-XSstart, _Last3=rb_rgb+Xplaces-1-XSend, _Dest3=rb_rgb+Xplaces-1-XDstart;
			for(int ky=Xplaces*(Yplaces-1-YSstart), kyEnd=Xplaces*(Yplaces-1-YSend), kyStep=conditional_negate(Xplaces, ky>kyEnd), ky2=Xplaces*(Yplaces-1-YDstart);ky!=kyEnd;ky+=kyStep, ky2+=kyStep)
			{
				std::copy(_First+ky,_Last+ky, _Dest+ky2);
				std::copy(_First2+ky,_Last2+ky, _Dest2+ky2);
				std::copy(_First3+ky,_Last3+ky, _Dest3+ky2);
			}
			prof_add("rev shift");
		}
	//	decltype(&Solve_C2D::shift_) shift;
		typedef void(modes::Solve_C2D::*Shift_fn)(std::vector<double>&, std::vector<double>&);
		Shift_fn shift;
	//	void solve_range	(Expression &ex, int *a, bool contourOnly)
		void solve_range	(Expression &ex, int *a, int contourOnly)
		{
			int x1=a[0], x2=a[1], y1=a[2], y2=a[3];
			modes::Xstart=Xstart, modes::Xsample=Xsample, modes::Yend=Yend, modes::Ysample=Ysample, modes::T=T;
			modes::Xplaces=Xplaces, modes::Yplaces=Yplaces, modes::Zplaces=1, modes::ndrSize=ndrSize;
			fill_range(ex, choose_fill_fn_2di, 0, x1, x2, y1, y2, 0, 1, Xplaces, Yplaces, 1, Xplaces*Yplaces);
			prof_add("fill");
			solve_disc(ex, 0, x1, x2, y1, y2, 0, 1, Xplaces, Yplaces, 1,	0, 0, 0, 0, 0, 0);
			prof_add("solve");
			auto &n=ex.n[ex.resultTerm];
			auto &ndr_r=n.r, &ndr_i=n.i;
#ifdef DEBUG
			if(contourOnly==2)
			{
				for(int ky=Xplaces*y1, kyEnd=Xplaces*y2;ky<kyEnd;ky+=Xplaces)
					for(int kx=ky+x1, kxEnd=ky+x2;kx<kxEnd;++kx)
						rgb[kx]=colorFunction(ndr_r[kx], ndr_i[kx]);
			}
			else
#endif
			if(contourOnly)
			{
				for(int ky=Xplaces*y1, kyEnd=Xplaces*y2;ky<kyEnd;ky+=Xplaces)
					for(int kx=ky+x1, kxEnd=ky+x2;kx<kxEnd;++kx)
						rgb[kx]=0xFFFFFF;
			}
			else
			{
				for(int ky=Xplaces*y1, kyEnd=Xplaces*y2;ky<kyEnd;ky+=Xplaces)
					for(int kx=ky+x1, kxEnd=ky+x2;kx<kxEnd;++kx)
						rgb[kx]=colorFunction(ndr_r[kx], ndr_i[kx]);
			}
			prof_add("color");
		}
		void partial_		(Expression &ex, bool contourOnly){}
		void partial_1		(Expression &ex, bool contourOnly)
		{
			sa[3]=Yplaces-1-sa[3], sa[4]=Yplaces-1-sa[4], sa[5]=Yplaces-1-sa[5];
			auto &n=ex.n[ex.resultTerm];
			(this->*shift)(n.r, n.i);
			int	temp=ra[2];	ra[2]=Yplaces-ra[3], ra[3]=Yplaces-temp;
			solve_range(ex, ra, contourOnly);
		}
		void partial_2		(Expression &ex, bool contourOnly)
		{
			sa[3]=Yplaces-1-sa[3], sa[4]=Yplaces-1-sa[4], sa[5]=Yplaces-1-sa[5];
			auto &n=ex.n[ex.resultTerm];
			(this->*shift)(n.r, n.i);
			int	temp=ra[2];	ra[2]=Yplaces-ra[3], ra[3]=Yplaces-temp;
				temp=ra[6], ra[6]=Yplaces-ra[7], ra[7]=Yplaces-temp;
			solve_range(ex, ra, contourOnly), solve_range(ex, ra+4, contourOnly);
		}
	public:
		void partial_bounds(double VX, double DX, double VY, double DY, int Xoffset, int Yoffset)
		{
			this->Xstart=VX-DX/2, this->Xsample=DX/Xplaces, this->Yend=VY+DY/2, this->Ysample=DY/Yplaces;
			partial=&Solve_C2D::partial_;
				 if(Xoffset>0){							 if(Yoffset>0)					shift=&Solve_C2D::shift_,	sa[0]=Xoffset,				sa[1]=Xplaces,	sa[2]=0,			sa[3]=Yoffset,				sa[4]=Yplaces,	sa[5]=0,					partial=&Solve_C2D::partial_2,	ra[0]=Xplaces-Xoffset,	ra[1]=Xplaces,	ra[2]=0,				ra[3]=Yplaces-Yoffset,			ra[4]=0,				ra[5]=Xplaces,	ra[6]=Yplaces-Yoffset,	ra[7]=Yplaces;
													else if(Yoffset<0)Yoffset=-Yoffset,	shift=&Solve_C2D::shift_,	sa[0]=Xoffset,				sa[1]=Xplaces,	sa[2]=0,			sa[3]=Yplaces-1-Yoffset,	sa[4]=-1,		sa[5]=Yplaces-1,			partial=&Solve_C2D::partial_2,	ra[0]=0,				ra[1]=Xplaces,	ra[2]=0,				ra[3]=Yoffset,					ra[4]=Xplaces-Xoffset,	ra[5]=Xplaces,	ra[6]=Yoffset,			ra[7]=Yplaces;
													else								shift=&Solve_C2D::shift_,	sa[0]=Xoffset,				sa[1]=Xplaces,	sa[2]=0,			sa[3]=Yoffset,				sa[4]=Yplaces,	sa[5]=0,					partial=&Solve_C2D::partial_1,	ra[0]=Xplaces-Xoffset,	ra[1]=Xplaces,	ra[2]=0,				ra[3]=Yplaces;}
			else if(Xoffset<0){Xoffset=-Xoffset;		 if(Yoffset>0)					shift=&Solve_C2D::shift_r,	sa[0]=Xplaces-1-Xoffset,	sa[1]=-1,		sa[2]=Xplaces-1,	sa[3]=Yoffset,				sa[4]=Yplaces,	sa[5]=0,					partial=&Solve_C2D::partial_2,	ra[0]=0,				ra[1]=Xoffset,	ra[2]=0,				ra[3]=Yplaces-Yoffset,			ra[4]=0,				ra[5]=Xplaces,	ra[6]=Yplaces-Yoffset,	ra[7]=Yplaces;
													else if(Yoffset<0)Yoffset=-Yoffset,	shift=&Solve_C2D::shift_r,	sa[0]=Xplaces-1-Xoffset,	sa[1]=-1,		sa[2]=Xplaces-1,	sa[3]=Yplaces-1-Yoffset,	sa[4]=-1,		sa[5]=Yplaces-1,			partial=&Solve_C2D::partial_2,	ra[0]=0,				ra[1]=Xplaces,	ra[2]=0,				ra[3]=Yoffset,					ra[4]=0,				ra[5]=Xoffset,	ra[6]=Yoffset,			ra[7]=Yplaces;
													else								shift=&Solve_C2D::shift_r,	sa[0]=Xplaces-1-Xoffset,	sa[1]=-1,		sa[2]=Xplaces-1,	sa[3]=Yoffset,				sa[4]=Yplaces,	sa[5]=0,					partial=&Solve_C2D::partial_1,	ra[0]=0,				ra[1]=Xoffset,	ra[2]=0,				ra[3]=Yplaces;}
			else{										 if(Yoffset>0)					shift=&Solve_C2D::shift_,	sa[0]=Xoffset,				sa[1]=Xplaces,	sa[2]=0,			sa[3]=Yoffset,				sa[4]=Yplaces,	sa[5]=0,					partial=&Solve_C2D::partial_1,	ra[0]=0,				ra[1]=Xplaces,	ra[2]=Yplaces-Yoffset,	ra[3]=Yplaces;
													else if(Yoffset<0)Yoffset=-Yoffset,	shift=&Solve_C2D::shift_,	sa[0]=Xoffset,				sa[1]=Xplaces,	sa[2]=0,			sa[3]=Yplaces-1-Yoffset,	sa[4]=-1,		sa[5]=Yplaces-1,			partial=&Solve_C2D::partial_1,	ra[0]=0,				ra[1]=Xplaces,	ra[2]=0,				ra[3]=Yoffset;
													else;}
		}
	//	decltype(&Solve_C2D::partial_) partial;
		void (modes::Solve_C2D::*partial)(Expression&, bool);
	};
	class		Solve_L2D:public Solve
	{
	public:
		double XXstart, XXstep, XYstart, XYstep, YXstart, YXstep, YYstart, YYstep;
		int Xplaces, Yplaces, nX, nY, old_nX, old_nY, ndrSize;
		bool enable_disc;
		Disc_fn
			disc_X_in_u, disc_X_in_b, disc_X_in_t, disc_X_out,
			disc_Y_in_u, disc_Y_in_b, disc_Y_in_t, disc_Y_out;
		Solve_L2D(bool enable_disc):enable_disc(enable_disc),
			disc_X_in_u(enable_disc?disc_l2d_X_in_u:0), disc_X_in_b(enable_disc?disc_l2d_X_in_b:0), disc_X_in_t(enable_disc?disc_l2d_X_in_t:0), disc_X_out(enable_disc?disc_l2d_X_out:0),
			disc_Y_in_u(enable_disc?disc_l2d_Y_in_u:0), disc_Y_in_b(enable_disc?disc_l2d_Y_in_b:0), disc_Y_in_t(enable_disc?disc_l2d_Y_in_t:0), disc_Y_out(enable_disc?disc_l2d_Y_out:0)
		{}
	private:
		//int x, y;
		//double *p;
		//double fxx(){return XXstart+x*XXstep;}
		//double fxy(){return XYstart+y*XYstep;}
		//double fyx(){return YXstart+x*YXstep;}
		//double fyy(){return YYstart+y*YYstep;}
		//double fc(){return *p;}
		int sa[12], ra[16];
		//typedef double(modes::Solve_L2D::*F_fn)();
		//void LOL_1(Variable &variables, std::remove_reference<F_fn>::type &f, std::remove_reference<F_fn>::type &fb, int varType, int &LOL_1_const)
		//{
		//	switch(varType)
		//	{
		//	case 'x':	f=&Solve_L2D::fxx, fb=&Solve_L2D::fyx, LOL_1_const=0;break;
		//	case 'y':	f=&Solve_L2D::fxy, fb=&Solve_L2D::fyy, LOL_1_const=0;break;
		//	case 'c':fb=f=&Solve_L2D::fc, p=&variables.val.r;break;
		//	case 't':fb=f=&Solve_L2D::fc, p=&T;break;
		//	}
		//}
		//void LOL_2(Variable &variables, std::remove_reference<F_fn>::type &f, int varType, int &LOL_1_const)
		//{
		//	switch(varType)
		//	{
		//	case 'x':f=&Solve_L2D::fxx, LOL_1_const=0;break;
		//	case 'y':f=&Solve_L2D::fxy, LOL_1_const=0;break;
		//	case 'c':f=&Solve_L2D::fc, p=&variables.val.r;break;
		//	case 't':f=&Solve_L2D::fc, p=&T;break;
		//	}
		//}
		//void LOL_3(Variable &variables, std::remove_reference<F_fn>::type &fb, int varType, int &LOL_1_const)
		//{
		//	switch(varType)
		//	{
		//	case 'x':fb=&Solve_L2D::fyx, LOL_1_const=0;break;
		//	case 'y':fb=&Solve_L2D::fyy, LOL_1_const=0;break;
		//	case 'c':fb=&Solve_L2D::fc, p=&variables.val.r;break;
		//	case 't':fb=&Solve_L2D::fc, p=&T;break;
		//	}
		//}
	public:
		void full_resize(double VX, double DX, double VY, double DY, double Xsample, double Ysample, int Xplaces, int Yplaces)
		{
		/*	XYstep=YXstep=step;
			XXstart=VX-DX/2, XXend=VX+DX/2, XYstart=ceil((VY-DY/2)/XYstep)*XYstep, XYend=floor((VY+DY/2)/XYstep)*XYstep;
			YYstart=VY-DY/2, YYend=VY+DY/2, YXstart=ceil((VX-DX/2)/YXstep)*YXstep, YXend=floor((VX+DX/2)/YXstep)*YXstep;
			nX=int(floor(YYend/XYstep)-floor(YYstart/XYstep)), nY=int(floor(XXend/YXstep)-floor(XXstart/YXstep));//*/
			this->Xplaces=Xplaces, this->Yplaces=Yplaces;
			XYstep=Ysample, YXstep=Xsample;
			XXstart=VX-DX/2, XXstep=DX/Xplaces;
			YYstart=VY-DY/2, YYstep=DY/Yplaces;
			int Xstart=int(ceil(YYstart/XYstep)), Ystart=int(ceil(XXstart/YXstep));
			XYstart=Xstart*XYstep;
			YXstart=Ystart*YXstep;
			nX=int(ceil((VY+DY/2)/XYstep))-Xstart, nY=int(ceil((VX+DX/2)/YXstep))-Ystart, ndrSize=Xplaces*nX+Yplaces*nY;//*/
		/*	XYstep=YXstep=step;
			XXstart=VX-DX/2, XXstep=DX/Xplaces;
			YYstart=VY-DY/2, YYstep=DY/Yplaces;
			XYstart=ceil(YYstart/XYstep)*XYstep;
			YXstart=ceil(XXstart/YXstep)*YXstep;
			nX=int(ceil((VY+DY/2)/XYstep)-ceil(YYstart/XYstep)), nY=int(ceil((VX+DX/2)/YXstep)-ceil(XXstart/YXstep)), ndrSize=Xplaces*nX+Yplaces*nY;*/
		//	Xdiscs.clear(), Ydiscs.clear();
		}
		void full(Expression &ex)
		{
			unsigned yPos=Xplaces*nX;
			modes::XXstart=XXstart, modes::XXstep=XXstep, modes::XYstart=XYstart, modes::XYstep=XYstep, modes::T=T;
			modes::YXstart=YXstart, modes::YXstep=YXstep, modes::YYstart=YYstart, modes::YYstep=YYstep;
			modes::Xplaces=Xplaces, modes::Yplaces=Yplaces, modes::Zplaces=Zplaces, modes::ndrSize=ndrSize;
			resize_terms(ex, ndrSize);
			fill_range(ex, choose_fill_fn_l2d_X, 0,		0, Xplaces, 0, nX, 0, 1, Xplaces, nX, 1, yPos);
			fill_range(ex, choose_fill_fn_l2d_Y, yPos,	0, Yplaces, 0, nY, 0, 1, Yplaces, nY, 1, Yplaces*nY);
			//Xdiscs:	(Xplaces-1) * nX
			//Ydiscs:	(Yplaces-1) * nY
			unsigned yDiscOffset=(Xplaces-1)*nX;
			if(enable_disc)
				ex.discontinuities.assign(yDiscOffset+(Yplaces-1)*nY, false);
			::solve_disc(ex, 0,				0, Xplaces, 0, nX, 0, 1, Xplaces, nX, 1,	disc_X_in_u, disc_X_in_b, disc_X_in_t, disc_X_out, 0, false);
			::solve_disc(ex, Xplaces*nX,	0, Yplaces, 0, nY, 0, 1, Yplaces, nY, 1,	disc_Y_in_u, disc_Y_in_b, disc_Y_in_t, disc_Y_out, (Xplaces-1)*nX, false);
		}
	private:
		void shift_		(std::vector<double>&, std::vector<double>&, std::vector<bool>&){}
		void shift_x	(std::vector<double> &ndr_r, std::vector<double> &ndr_i, std::vector<bool> &discontinuities)
		{
			int &XSstart=sa[0], &XSend=sa[1], &XDstart=sa[2], &YSstart=sa[3], &YSend=sa[4], &YDstart=sa[5];
			{
				auto _First=ndr_r.begin()+XSstart, _Last=ndr_r.begin()+XSend, _Dest=ndr_r.begin()+XDstart;
				auto _First2=ndr_i.begin()+XSstart, _Last2=ndr_i.begin()+XSend, _Dest2=ndr_i.begin()+XDstart;
				for(int ys=Xplaces*YSstart, ysEnd=Xplaces*YSend, yStep=(ys<ysEnd?1:-1)*Xplaces, yd=Xplaces*YDstart;ys!=ysEnd;ys+=yStep, yd+=yStep)
				{
					std::copy(_First+ys, _Last+ys, _Dest+yd);
					std::copy(_First2+ys, _Last2+ys, _Dest2+yd);
				}
			}
			if(XSstart<Xplaces-1)
			{
				auto _First=discontinuities.begin()+XSstart, _Last=discontinuities.begin()+XSend-1, _Dest=discontinuities.begin()+XDstart;
				for(int ys=YSstart*(Xplaces-1), ysEnd=YSend*(Xplaces-1), yStep=(ys<ysEnd?1:-1)*(Xplaces-1), yd=YDstart*(Xplaces-1);ys!=ysEnd;ys+=yStep, yd+=yStep)
					std::copy(_First+ys, _Last+ys, _Dest+yd);
			}
		}
		void shift_xr	(std::vector<double> &ndr_r, std::vector<double> &ndr_i, std::vector<bool> &discontinuities)
		{
			int &XSstart=sa[0], &XSend=sa[1], &XDstart=sa[2], &YSstart=sa[3], &YSend=sa[4], &YDstart=sa[5];
			{
				auto rb_r=ndr_r.rbegin(), rb_i=ndr_i.rbegin();
			//	auto rb_r=std::reverse_iterator<double*>(ndr_r+ndrSize), rb_i=std::reverse_iterator<double*>(ndr_i+ndrSize);
				auto _First=rb_r+Yplaces*nY+Xplaces-1-XSstart, _Last=rb_r+Yplaces*nY+Xplaces-1-XSend, _Dest=rb_r+Yplaces*nY+Xplaces-1-XDstart;
				auto _First2=rb_i+Yplaces*nY+Xplaces-1-XSstart, _Last2=rb_i+Yplaces*nY+Xplaces-1-XSend, _Dest2=rb_i+Yplaces*nY+Xplaces-1-XDstart;
				for(int ys=Xplaces*(nX-1-YSstart), ysEnd=Xplaces*(nX-1-YSend), yStep=Xplaces*(ys<ysEnd?1:-1), yd=Xplaces*(nX-1-YDstart);ys!=ysEnd;ys+=yStep, yd+=yStep)
				{
					std::copy(_First+ys,_Last+ys, _Dest+yd);
					std::copy(_First2+ys,_Last2+ys, _Dest2+yd);
				}
			}
			if(XSstart>0)
			{
				unsigned xDiscROffset=(Yplaces-1)*nY;
				auto _First=discontinuities.rbegin()+Xplaces-2-(XSstart-1), _Last=discontinuities.rbegin()+Xplaces-2-XSend, _Dest=discontinuities.rbegin()+Xplaces-2-(XDstart-1);
				for(int ys=(nX-1-YSstart)*(Xplaces-1), ysEnd=(nX-1-YSend)*(Xplaces-1), yStep=(ys<ysEnd?1:-1)*(Xplaces-1), yd=(nX-1-YDstart)*(Xplaces-1);ys!=ysEnd;ys+=yStep, yd+=yStep)
					std::copy(_First+ys, _Last+ys, _Dest+yd);
			}
		}
		void shift_y	(std::vector<double> &ndr_r, std::vector<double> &ndr_i, std::vector<bool> &discontinuities)
		{
			int &XSstart=sa[6], &XSend=sa[7], &XDstart=sa[8], &YSstart=sa[9], &YSend=sa[10], &YDstart=sa[11];
			{
				auto _First=ndr_r.begin()+Xplaces*nX+YSstart, _Last=ndr_r.begin()+Xplaces*nX+YSend, _Dest=ndr_r.begin()+Xplaces*nX+YDstart;
				auto _First2=ndr_i.begin()+Xplaces*nX+YSstart, _Last2=ndr_i.begin()+Xplaces*nX+YSend, _Dest2=ndr_i.begin()+Xplaces*nX+YDstart;
				for(int xs=Yplaces*XSstart, xsEnd=Yplaces*XSend, xStep=(xs<xsEnd?1:-1)*Yplaces, xd=Yplaces*XDstart;xs!=xsEnd;xs+=xStep, xd+=xStep)
				{
					std::copy(_First+xs, _Last+xs, _Dest+xd);
					std::copy(_First2+xs, _Last2+xs, _Dest2+xd);
				}
			}
			if(YSstart<Yplaces-1)
			{
				unsigned yDiscOffset=(Xplaces-1)*nX;
				auto _First=discontinuities.begin()+yDiscOffset+YSstart, _Last=discontinuities.begin()+yDiscOffset+YSend-1, _Dest=discontinuities.begin()+yDiscOffset+YDstart;
				for(int xs=(Yplaces-1)*XSstart, xsEnd=(Yplaces-1)*XSend, xStep=(xs<xsEnd?1:-1)*(Yplaces-1), xd=(Yplaces-1)*XDstart;xs!=xsEnd;xs+=xStep, xd+=xStep)
					std::copy(_First+xs, _Last+xs, _Dest+xd);
			}
		}
		void shift_yr	(std::vector<double> &ndr_r, std::vector<double> &ndr_i, std::vector<bool> &discontinuities)
		{
			int &XSstart=sa[6], &XSend=sa[7], &XDstart=sa[8], &YSstart=sa[9], &YSend=sa[10], &YDstart=sa[11];
			{
				auto rb_r=ndr_r.rbegin(), rb_i=ndr_i.rbegin();
			//	auto rb_r=std::reverse_iterator<double*>(ndr_r+ndrSize), rb_i=std::reverse_iterator<double*>(ndr_i+ndrSize);
				auto _First=rb_r+Yplaces-1-YSstart, _Last=rb_r+Yplaces-1-YSend, _Dest=rb_r+Yplaces-1-YDstart;
				auto _First2=rb_i+Yplaces-1-YSstart, _Last2=rb_i+Yplaces-1-YSend, _Dest2=rb_i+Yplaces-1-YDstart;
				for(int xs=Yplaces*(nY-1-XSstart), xsEnd=Yplaces*(nY-1-XSend), xStep=(xs<xsEnd?1:-1)*Yplaces, xd=Yplaces*(nY-1-XDstart);xs!=xsEnd;xs+=xStep, xd+=xStep)
				{
					std::copy(_First+xs, _Last+xs, _Dest+xd);
					std::copy(_First2+xs, _Last2+xs, _Dest2+xd);
				}
			}
			if(YSstart>0)
			{
				auto _First=discontinuities.rbegin()+Yplaces-2-(YSstart-1), _Last=discontinuities.rbegin()+Yplaces-2-YSend, _Dest=discontinuities.rbegin()+Yplaces-2-(YDstart-1);
				for(int xs=(Yplaces-1)*(nY-1-XSstart), xsEnd=(Yplaces-1)*(nY-1-XSend), xStep=(xs<xsEnd?1:-1)*(Yplaces-1), xd=(Yplaces-1)*(nY-1-XDstart);xs!=xsEnd;xs+=xStep, xd+=xStep)
					std::copy(_First+xs, _Last+xs, _Dest+xd);
			}
		}
	//	decltype(&Solve_L2D::shift_) shift_X, shift_Y;
		typedef void(modes::Solve_L2D::*Shift_fn)(std::vector<double>&, std::vector<double>&, std::vector<bool>&);
		Shift_fn shift_X, shift_Y;
		void modify_	(Term&, std::vector<bool>&){}
		void modify_Yi	(Term &n, std::vector<bool> &discontinuities)
		{
			n.r.erase(n.r.begin()+Xplaces*old_nX, n.r.begin()+Yplaces);//not halved
			n.i.erase(n.i.begin()+Xplaces*old_nX, n.i.begin()+Yplaces);

			unsigned yDiscOffset=(Xplaces-1)*old_nX;
			discontinuities.erase(
				discontinuities.begin()+yDiscOffset,
				discontinuities.begin()+yDiscOffset+Yplaces-1);
		}
		void modify_YI	(Term &n, std::vector<bool> &discontinuities)
		{
			n.r.insert(n.r.begin()+Xplaces*old_nX, Yplaces, 0);
			n.i.insert(n.i.begin()+Xplaces*old_nX, Yplaces, 0);

			unsigned yDiscOffset=(Xplaces-1)*old_nX;
			discontinuities.insert(discontinuities.begin()+yDiscOffset, Yplaces-1, false);
		}
		void modify_Yf	(Term &n, std::vector<bool> &discontinuities)
		{
			n.r.erase(n.r.begin()+Xplaces*old_nX+Yplaces*(old_nY-1), n.r.begin()+Yplaces);
			n.i.erase(n.i.begin()+Xplaces*old_nX+Yplaces*(old_nY-1), n.i.begin()+Yplaces);

			unsigned yDiscOffset=(Xplaces-1)*old_nX;
			discontinuities.erase(
				discontinuities.begin()+yDiscOffset+(Yplaces-1)*(old_nY-1),
				discontinuities.begin()+yDiscOffset+(Yplaces-1)*old_nY);
		}
		void modify_YF	(Term &n, std::vector<bool> &discontinuities)
		{
			n.r.insert(n.r.begin()+Xplaces*old_nX+Yplaces*old_nY, Yplaces, 0);
			n.i.insert(n.i.begin()+Xplaces*old_nX+Yplaces*old_nY, Yplaces, 0);

			discontinuities.insert(discontinuities.end(), Yplaces-1, false);
		}
		void modify_Xi	(Term &n, std::vector<bool> &discontinuities)
		{
			n.r.erase(n.r.begin(), n.r.begin()+Xplaces);
			n.i.erase(n.i.begin(), n.i.begin()+Xplaces);

			discontinuities.erase(
				discontinuities.begin(),
				discontinuities.begin()+Xplaces-1);
		}
		void modify_XI	(Term &n, std::vector<bool> &discontinuities)
		{
			n.r.insert(n.r.begin(), Xplaces, 0);
			n.i.insert(n.i.begin(), Xplaces, 0);

			discontinuities.insert(discontinuities.begin(), Xplaces-1, false);
		}
		void modify_Xf	(Term &n, std::vector<bool> &discontinuities)
		{
			n.r.erase(n.r.begin()+Xplaces*(old_nX-1), n.r.begin()+Xplaces);
			n.i.erase(n.i.begin()+Xplaces*(old_nX-1), n.i.begin()+Xplaces);

			discontinuities.erase(
				discontinuities.begin()+(Xplaces-1)*(old_nX-1),
				discontinuities.begin()+(Xplaces-1)*old_nX);
		}
		void modify_XF	(Term &n, std::vector<bool> &discontinuities)
		{
		//	ndr_to_clipboard_l2d(expr[0].n[expr[0].resultTerm], Xplaces, old_nX, Yplaces, old_nY);//
			n.r.insert(n.r.begin()+Xplaces*old_nX, Xplaces, 0);
			n.i.insert(n.i.begin()+Xplaces*old_nX, Xplaces, 0);
		//	ndr_to_clipboard_l2d(expr[0].n[expr[0].resultTerm], Xplaces, nX, Yplaces, nY);//

			discontinuities.insert(discontinuities.begin()+(Xplaces-1)*old_nX, Xplaces-1, false);
		}
		typedef void(modes::Solve_L2D::*Modify_fn)(Term&, std::vector<bool>&);
		Modify_fn modify_X, modify_Y;
		void solve_range_X(Expression &ex, int *a)
		{
			int &x1=a[0], &x2=a[1], &y1=a[2], &y2=a[3];
			modes::XXstart=XXstart, modes::XXstep=XXstep, modes::XYstart=XYstart, modes::XYstep=XYstep, modes::T=T;
			fill_range(ex, choose_fill_fn_l2d_X, 0, x1, x2, y1, y2, 0, 1, Xplaces, nX, 1, Xplaces*nX);
			if(enable_disc)
				for(int y=y1;y<y2;++y)
					for(int x=x1;x<x2-1;++x)
						ex.discontinuities[(Xplaces-1)*y+x]=false;
			::solve_disc(ex, 0, x1, x2, y1, y2, 0, 1, Xplaces, nY, 1,		disc_X_in_u, disc_X_in_b, disc_X_in_t, disc_X_out, 0, false);
		}
		void solve_range_Y(Expression &ex, int *a)
		{
			int &x1=a[0], &x2=a[1], &y1=a[2], &y2=a[3];
			modes::YXstart=YXstart, modes::YXstep=YXstep, modes::YYstart=YYstart, modes::YYstep=YYstep, modes::T=T;
			fill_range(ex, choose_fill_fn_l2d_Y, Xplaces*nX, y1, y2, x1, x2, 0, 1, Yplaces, nY, 1, Yplaces*nY);
			int yPos=Xplaces*nX, yDiscPos=(Xplaces-1)*nX;
			if(enable_disc)
				for(int x=x1;x<x2;++x)
					for(int y=y1;y<y2-1;++y)
						ex.discontinuities[yDiscPos+(Yplaces-1)*x+y]=false;
			::solve_disc(ex, yPos, y1, y2, x1, x2, 0, 1, Yplaces, nY, 1,	disc_Y_in_u, disc_Y_in_b, disc_Y_in_t, disc_Y_out, yDiscPos, false);//x and y swapped
		}
		void range_		(Expression&){}
		void range_xx	(Expression &ex){solve_range_X(ex, ra), solve_range_X(ex, ra+4);}
		void range_x	(Expression &ex){solve_range_X(ex, ra);}
		void range_yy	(Expression &ex){solve_range_Y(ex, ra+8), solve_range_Y(ex, ra+12);}
		void range_y	(Expression &ex){solve_range_Y(ex, ra+8);}
	//	decltype(&Solve_L2D::range_) range_X, range_Y;
		typedef void(modes::Solve_L2D::*Range_fn)(Expression&);
		Range_fn range_X, range_Y;
	public:
		void partial_bounds(double VX, double DX, double VY, double DY, int Xoffset, int Yoffset)
		{
			int XDstart, XDend, YDstart, YDend;
			{
				old_nX=nX, old_nY=nY;
				int old_Xstart=int(ceil(YYstart/XYstep)), old_Ystart=int(ceil(XXstart/YXstep));

				XXstart=VX-DX/2, XXstep=DX/Xplaces;
				YYstart=VY-DY/2, YYstep=DY/Yplaces;
				int Xstart=int(ceil(YYstart/XYstep)), Ystart=int(ceil(XXstart/YXstep));
				XYstart=Xstart*XYstep;
				YXstart=Ystart*YXstep;
				nX=int(ceil((VY+DY/2)/XYstep))-Xstart, nY=int(ceil((VX+DX/2)/YXstep))-Ystart;

				XDstart=abs(Xstart-old_Xstart), XDend=abs(Xstart+nX-(old_Xstart+old_nX)), YDstart=abs(Ystart-old_Ystart), YDend=abs(Ystart+nY-(old_Ystart+old_nY));//*/
			}

		/*	shift_Y
				shift_																	!nY
				shift_y																	nY	Yoffset>0
				shift_yr																nY	Yoffset<0

			shift_X
				shift_																	!nX
				shift_x																	nX	Xoffset>0
				shift_xr																nX	Xoffset<0

			modify_Y
				modify_																	YDstart==	YDend		||	Xoffset==0
				modify_Yi																YDstart>	YDend		&&	Xoffset>0
				modify_YF																YDstart<	YDend		&&	Xoffset>0
				modify_YI																YDstart>	YDend		&&	Xoffset<0
				modify_Yf																YDstart<	YDend		&&	Xoffset<0

			modify_X
				modify_																	XDstart==	XDend		||	Yoffset==0
				modify_Xi																XDstart>	XDend		&&	Yoffset>0
				modify_XF																XDstart<	XDend		&&	Yoffset>0
				modify_XI																XDstart>	XDend		&&	Yoffset<0
				modify_Xf																XDstart<	XDend		&&	Yoffset<0

			sa[0], sa[1], sa[2]
				Xoffset,			Xplaces,	0										Xoffset>0	&&	nX
				Xplaces-1-Xoffset,	-1,			Xplaces-1								Xoffset<0	&&	nX
				0,					Xplaces,	0										Xoffset==0	&&	nX	&&	(Yoffset>0	&&	(XDstart>	XDend	&&	XDstart-1	||	XDstart)	||	Yoffset<0	&&	(XDstart<	XDend	&&	XDend-1	||	XDend))

			sa[3], sa[4], sa[5]
				XDstart,			nX,			0										Xoffset!=0	&&	Yoffset>0	&&	nX	&&	XDstart==	XDend	||	Yoffset>0	&&	XDstart==	XDend	&&	XDstart
				XDstart-1,			nX,			0										Xoffset!=0	&&	Yoffset>0	&&	nX	&&	XDstart>	XDend	||	Yoffset>0	&&	XDstart>	XDend	&&	XDstart-1
				XDstart,			nX-1,		0										Xoffset!=0	&&	Yoffset>0	&&	nX	&&	XDstart<	XDend	||	Yoffset>0	&&	XDstart<	XDend	&&	XDstart
				nX-1-XDend,			-1,			nX-1									Xoffset!=0	&&	Yoffset<0	&&	nX	&&	XDstart==	XDend	||	Yoffset<0	&&	XDstart==	XDend	&&	XDend
				nX-1-XDend,			0,			nX-1									Xoffset!=0	&&	Yoffset<0	&&	nX	&&	XDstart>	XDend	||	Yoffset<0	&&	XDstart>	XDend	&&	XDend
				nX-1-(XDend-1),		-1,			nX-1									Xoffset!=0	&&	Yoffset<0	&&	nX	&&	XDstart<	XDend	||	Yoffset<0	&&	XDstart<	XDend	&&	XDend-1

			sa[6], sa[7], sa[8]
				YDstart,			nY,			0										Yoffset!=0	&&	Xoffset>0	&&	nY	&&	YDstart==	YDend	||	Xoffset>0	&&	YDstart==	YDend	&&	YDstart
				YDstart-1,			nY,			0										Yoffset!=0	&&	Xoffset>0	&&	nY	&&	YDstart>	YDend	||	Xoffset>0	&&	YDstart>	YDend	&&	YDstart-1
				YDstart,			nY-1,		0										Yoffset!=0	&&	Xoffset>0	&&	nY	&&	YDstart<	YDend	||	Xoffset>0	&&	YDstart<	YDend	&&	YDstart
				nY-1-YDend,			-1,			nY-1									Yoffset!=0	&&	Xoffset<0	&&	nY	&&	YDstart==	YDend	||	Xoffset<0	&&	YDstart==	YDend	&&	YDend
				nY-1-YDend,			0,			nY-1									Yoffset!=0	&&	Xoffset<0	&&	nY	&&	YDstart>	YDend	||	Xoffset<0	&&	YDstart>	YDend	&&	YDend
				nY-1-(YDend-1),		-1,			nY-1									Yoffset!=0	&&	Xoffset<0	&&	nY	&&	YDstart<	YDend	||	Xoffset<0	&&	YDstart<	YDend	&&	YDend-1

			sa[9], sa[10], sa[11]
				Yoffset,			Yplaces,	0										Yoffset>0	&&	nY
				Yplaces-1-Yoffset,	-1,			Yplaces-1								Yoffset<0	&&	nY
				0,					Yplaces,	0										Yoffset==0	&&	nY	&&	(Xoffset>0	&&	(YDstart>	YDend	&&	YDstart-1	||	YDstart)	||	Xoffset<0	&&	(YDstart<	YDend	&&	YDend-1	||	YDend))

			ra[0], ra[1], ra[2], ra[3], ra[4], ra[5], ra[6], ra[7]
				Xplaces-Xoffset,	Xplaces,	0,					nX-XDend		0,					Xplaces,	nX-XDend,			nX					nX	&&	Xoffset>0	&&	Yoffset>0	&&	XDend
				0,					Xplaces,	0,					XDstart			Xplaces-Xoffset,	Xplaces,	XDstart,			nX					nX	&&	Xoffset>0	&&	Yoffset<0	&&	XDstart
				Xplaces-Xoffset,	Xplaces,	0,					nX																						nX	&&	Xoffset>0	&&	(Yoffset>0	&&	!XDend	||	Yoffset<0	&&	!XDstart	||	!Yoffset)
				0,					Xoffset,	0,					nX-XDend		0,					Xplaces,	nX-XDend,			nX					nX	&&	Xoffset<0	&&	Yoffset>0	&&	XDend
				0,					Xplaces,	0,					XDstart			0,					Xoffset,	XDstart,			nX					nX	&&	Xoffset<0	&&	Yoffset<0	&&	XDstart
				0,					Xoffset,	0,					nX																						nX	&&	Xoffset<0	&&	(Yoffset>0	&&	!XDend	||	Yoffset<0	&&	!XDstart	||	!Yoffset)
				0,					Xplaces,	nX-XDend,			nX																								!Xoffset	&&	Yoffset>0	&&	XDend
				0,					Xplaces,	0,					XDstart																							!Xoffset	&&	Yoffset<0	&&	XDstart

			ra[8], ra[9], ra[10], ra[11], ra[12], ra[13], ra[14], ra[15]
				0,					nY-YDend,	Yplaces-Yoffset,	Yplaces			nY-YDend,			nY,			0,					Yplaces				nY	&&	Xoffset>0	&&	Yoffset>0	&&	YDend
				0,					nY,			Yplaces-Yoffset,	Yplaces																					nY	&&	Xoffset>0	&&	Yoffset>0	&&	!YDend
				0,					nY-YDend,	0,					Yoffset			nY-YDend,			nY,			0,					Yplaces				nY	&&	Xoffset>0	&&	Yoffset<0	&&	YDend
				0,					nY,			0,					Yoffset																					nY	&&	Xoffset>0	&&	Yoffset<0	&&	!YDend
				nY-YDend,			nY,			0,					Yplaces																							Xoffset>0	&&	!Yoffset	&&	YDend
				0,					YDstart,	0,					Yplaces			YDstart,			nY,			Yplaces-Yoffset,	Yplaces				nY	&&	Xoffset<0	&&	Yoffset>0	&&	YDstart
				0,					nY,			Yplaces-Yoffset,	Yplaces																					nY	&&	Xoffset<0	&&	Yoffset>0	&&	!YDstart
				0,					YDstart,	0,					Yplaces			YDstart,			nY,			0,					Yoffset				nY	&&	Xoffset<0	&&	Yoffset<0	&&	YDstart
				0,					nY,			0,					Yplaces																					nY	&&	Xoffset<0	&&	Yoffset<0	&&	!YDstart
				0,					YDstart,	0,					Yplaces																							Xoffset<0	&&	!Yoffset	&&	YDstart
				0,					nY,			Yplaces-Yoffset,	Yplaces																					nY	&&	!Xoffset	&&	Yoffset>0
				0,					nY,			0,					Yoffset																					nY	&&	!Xoffset	&&	Yoffset<0
			*/
				 if(Xoffset>0)	{						 if(Yoffset>0)	{						 if(nY)	{	shift_Y=(Shift_fn)&Solve_L2D::shift_y;		 if(YDstart==	YDend)				modify_Y=&Solve_L2D::modify_,																sa[6]=YDstart,				sa[7]=nY,			sa[8]=0,			sa[9]=Yoffset,				sa[10]=Yplaces,	sa[11]=0;
																																					else if(YDstart>	YDend)				modify_Y=&Solve_L2D::modify_Yi,															sa[6]=YDstart-1,			sa[7]=nY,			sa[8]=0,			sa[9]=Yoffset,				sa[10]=Yplaces,	sa[11]=0;
																																					else if(YDstart<	YDend)				modify_Y=&Solve_L2D::modify_YF,															sa[6]=YDstart,				sa[7]=nY-1,			sa[8]=0,			sa[9]=Yoffset,				sa[10]=Yplaces,	sa[11]=0;
																																																																																																							if(YDend)	range_Y=&Solve_L2D::range_yy,		++Yoffset,	ra[8]=0,				ra[9]=nY-YDend,	ra[10]=Yplaces-Yoffset,	ra[11]=Yplaces,		ra[12]=nY-YDend,		ra[13]=nY,		ra[14]=0,				ra[15]=Yplaces;
																																																																																																						else			range_Y=&Solve_L2D::range_y,		++Yoffset,	ra[8]=0,				ra[9]=nY,		ra[10]=Yplaces-Yoffset,	ra[11]=Yplaces;																							}
																							else		{	shift_Y=(Shift_fn)&Solve_L2D::shift_;		 if(YDstart==	YDend)				modify_Y=&Solve_L2D::modify_;
																																					else if(YDstart>	YDend)				modify_Y=&Solve_L2D::modify_Yi;
																																					else if(YDstart<	YDend)				modify_Y=&Solve_L2D::modify_YF;																																																				range_Y=&Solve_L2D::range_;																																												}
																								 if(nX)	{	shift_X=(Shift_fn)&Solve_L2D::shift_x;		 if(XDstart==	XDend)		modify_X=&Solve_L2D::modify_,																		sa[0]=Xoffset,				sa[1]=Xplaces,		sa[2]=0,			sa[3]=XDstart,				sa[4]=nX,		sa[5]=0;
																																					else if(XDstart>	XDend)		modify_X=&Solve_L2D::modify_Xi,																	sa[0]=Xoffset,				sa[1]=Xplaces,		sa[2]=0,			sa[3]=XDstart-1,			sa[4]=nX,		sa[5]=0;
																																					else if(XDstart<	XDend)		modify_X=&Solve_L2D::modify_XF,																	sa[0]=Xoffset,				sa[1]=Xplaces,		sa[2]=0,			sa[3]=XDstart,				sa[4]=nX-1,		sa[5]=0;
																																																																																																							if(XDend)	range_X=&Solve_L2D::range_xx,		++Xoffset,	ra[0]=Xplaces-Xoffset,	ra[1]=Xplaces,	ra[2]=0,				ra[3]=nX-XDend,		ra[4]=0,				ra[5]=Xplaces,	ra[6]=nX-XDend,			ra[7]=nX;
																																																																																																						else			range_X=&Solve_L2D::range_x,		++Xoffset,	ra[0]=Xplaces-Xoffset,	ra[1]=Xplaces,	ra[2]=0,				ra[3]=nX;																								}
																							else		{	shift_X=(Shift_fn)&Solve_L2D::shift_;		 if(XDstart==	XDend)		modify_X=&Solve_L2D::modify_;
																																					else if(XDstart>	XDend)		modify_X=&Solve_L2D::modify_Xi;
																																					else if(XDstart<	XDend)		modify_X=&Solve_L2D::modify_XF;																																																						range_X=&Solve_L2D::range_;																																												}}
													else if(Yoffset<0)	{Yoffset=-Yoffset;		 if(nY)	{	shift_Y=(Shift_fn)&Solve_L2D::shift_yr;		 if(YDstart==	YDend)				modify_Y=&Solve_L2D::modify_,																sa[6]=YDstart,				sa[7]=nY,			sa[8]=0,			sa[9]=Yplaces-1-Yoffset,	sa[10]=-1,		sa[11]=Yplaces-1;
																																					else if(YDstart>	YDend)				modify_Y=&Solve_L2D::modify_Yi,															sa[6]=YDstart-1,			sa[7]=nY,			sa[8]=0,			sa[9]=Yplaces-1-Yoffset,	sa[10]=-1,		sa[11]=Yplaces-1;
																																					else if(YDstart<	YDend)				modify_Y=&Solve_L2D::modify_YF,															sa[6]=YDstart,				sa[7]=nY-1,			sa[8]=0,			sa[9]=Yplaces-1-Yoffset,	sa[10]=-1,		sa[11]=Yplaces-1;
																																																																																																							if(YDend)	range_Y=&Solve_L2D::range_yy,		++Yoffset,	ra[8]=0,				ra[9]=nY-YDend,	ra[10]=0,				ra[11]=Yoffset,		ra[12]=nY-YDend,		ra[13]=nY,		ra[14]=0,				ra[15]=Yplaces;
																																																																																																						else			range_Y=&Solve_L2D::range_y,		++Yoffset,	ra[8]=0,				ra[9]=nY,		ra[10]=0,				ra[11]=Yoffset;																							}
																							else		{	shift_Y=(Shift_fn)&Solve_L2D::shift_;		 if(YDstart==	YDend)				modify_Y=&Solve_L2D::modify_;
																																					else if(YDstart>	YDend)				modify_Y=&Solve_L2D::modify_Yi;
																																					else if(YDstart<	YDend)				modify_Y=&Solve_L2D::modify_YF;																																																				range_Y=&Solve_L2D::range_;																																												}
																								 if(nX)	{	shift_X=(Shift_fn)&Solve_L2D::shift_x;		 if(XDstart==	XDend)		modify_X=&Solve_L2D::modify_,																		sa[0]=Xoffset,				sa[1]=Xplaces,		sa[2]=0,			sa[3]=nX-1-XDend,			sa[4]=-1,		sa[5]=nX-1;
																																					else if(XDstart>	XDend)		modify_X=&Solve_L2D::modify_XI,																	sa[0]=Xoffset,				sa[1]=Xplaces,		sa[2]=0,			sa[3]=nX-1-XDend,			sa[4]=0,		sa[5]=nX-1;
																																					else if(XDstart<	XDend)		modify_X=&Solve_L2D::modify_Xf,																	sa[0]=Xoffset,				sa[1]=Xplaces,		sa[2]=0,			sa[3]=nX-1-(XDend-1),		sa[4]=-1,		sa[5]=nX-1;
																																																																																																							if(XDstart)	range_X=&Solve_L2D::range_xx,		++Xoffset,	ra[0]=0,				ra[1]=Xplaces,	ra[2]=0,				ra[3]=XDstart,		ra[4]=Xplaces-Xoffset,	ra[5]=Xplaces,	ra[6]=XDstart,			ra[7]=nX;
																																																																																																						else			range_X=&Solve_L2D::range_x,		++Xoffset,	ra[0]=Xplaces-Xoffset,	ra[1]=Xplaces,	ra[2]=0,				ra[3]=nX;																								}
																							else		{	shift_X=(Shift_fn)&Solve_L2D::shift_;		 if(XDstart==	XDend)		modify_X=&Solve_L2D::modify_;
																																					else if(XDstart>	XDend)		modify_X=&Solve_L2D::modify_XI;
																																					else if(XDstart<	XDend)		modify_X=&Solve_L2D::modify_Xf;																																																					range_X=&Solve_L2D::range_;																																												}}
													else				{																												modify_X=&Solve_L2D::modify_;				 if(nX)			shift_X=&Solve_L2D::shift_x,	sa[0]=Xoffset,				sa[1]=Xplaces,		sa[2]=0,			sa[3]=0,					sa[4]=nX,		sa[5]=0,						range_X=&Solve_L2D::range_x,		++Xoffset,	ra[0]=Xplaces-Xoffset,	ra[1]=Xplaces,	ra[2]=0,				ra[3]=nX;
																																																								else				shift_X=&Solve_L2D::shift_,																																						range_X=&Solve_L2D::range_;
																																						 if(YDstart==	YDend)	{			modify_Y=&Solve_L2D::modify_;			 if(YDstart)	shift_Y=&Solve_L2D::shift_y,	sa[6]=YDstart,				sa[7]=nY,			sa[8]=0,			sa[9]=0,					sa[10]=Yplaces,	sa[11]=0;
																																																								else				shift_Y=&Solve_L2D::shift_;																																																																																									}
																																					else if(YDstart>	YDend)	{			modify_Y=&Solve_L2D::modify_Yi;			 if(YDstart-1)	shift_Y=&Solve_L2D::shift_y,	sa[6]=YDstart-1,			sa[7]=nY,			sa[8]=0,			sa[9]=0,					sa[10]=Yplaces,	sa[11]=0;
																																																								else				shift_Y=&Solve_L2D::shift_;																																																																																									}
																																					else if(YDstart<	YDend)	{			modify_Y=&Solve_L2D::modify_YF;			 if(YDstart)	shift_Y=&Solve_L2D::shift_y,	sa[6]=YDstart,				sa[7]=nY-1,			sa[8]=0,			sa[9]=0,					sa[10]=Yplaces,	sa[11]=0;
																																																								else				shift_Y=&Solve_L2D::shift_;																																																																																									}
																																																																																																							 if(YDend)	range_Y=&Solve_L2D::range_y,					ra[8]=nY-YDend,			ra[9]=nY,		ra[10]=0,				ra[11]=Yplaces;
																																																																																																						else			range_Y=&Solve_L2D::range_;																																												}}
			else if(Xoffset<0)	{Xoffset=-Xoffset;		 if(Yoffset>0)	{						 if(nY)	{	shift_Y=(Shift_fn)&Solve_L2D::shift_y;		 if(YDstart==	YDend)				modify_Y=&Solve_L2D::modify_,																sa[6]=nY-1-YDend,			sa[7]=-1,			sa[8]=nY-1,			sa[9]=Yoffset,				sa[10]=Yplaces,	sa[11]=0;
																																					else if(YDstart>	YDend)				modify_Y=&Solve_L2D::modify_YI,															sa[6]=nY-1-YDend,			sa[7]=0,			sa[8]=nY-1,			sa[9]=Yoffset,				sa[10]=Yplaces,	sa[11]=0;
																																					else if(YDstart<	YDend)				modify_Y=&Solve_L2D::modify_Yf,															sa[6]=nY-1-(YDend-1),		sa[7]=-1,			sa[8]=nY-1,			sa[9]=Yoffset,				sa[10]=Yplaces,	sa[11]=0;																																																									
																																																																																																							 if(YDstart)range_Y=&Solve_L2D::range_yy,		++Yoffset,	ra[8]=0,				ra[9]=YDstart,	ra[10]=0,				ra[11]=Yplaces,		ra[12]=YDstart,			ra[13]=nY,		ra[14]=Yplaces-Yoffset,	ra[15]=Yplaces;
																																																																																																						else			range_Y=&Solve_L2D::range_y,		++Yoffset,	ra[8]=0,				ra[9]=nY,		ra[10]=Yplaces-Yoffset,	ra[11]=Yplaces;																							}
																							else		{	shift_Y=(Shift_fn)&Solve_L2D::shift_;		 if(YDstart==	YDend)				modify_Y=&Solve_L2D::modify_;
																																					else if(YDstart>	YDend)				modify_Y=&Solve_L2D::modify_YI;
																																					else if(YDstart<	YDend)				modify_Y=&Solve_L2D::modify_Yf;																																																			range_Y=&Solve_L2D::range_;																																												}
																								 if(nX)	{	shift_X=(Shift_fn)&Solve_L2D::shift_xr;		 if(XDstart==	XDend)		modify_X=&Solve_L2D::modify_,																		sa[0]=Xplaces-1-Xoffset,	sa[1]=-1,			sa[2]=Xplaces-1,	sa[3]=XDstart,				sa[4]=nX,		sa[5]=0;
																																					else if(XDstart>	XDend)		modify_X=&Solve_L2D::modify_Xi,																	sa[0]=Xplaces-1-Xoffset,	sa[1]=-1,			sa[2]=Xplaces-1,	sa[3]=XDstart-1,			sa[4]=nX,		sa[5]=0;
																																					else if(XDstart<	XDend)		modify_X=&Solve_L2D::modify_XF,																	sa[0]=Xplaces-1-Xoffset,	sa[1]=-1,			sa[2]=Xplaces-1,	sa[3]=XDstart,				sa[4]=nX-1,		sa[5]=0;																																																									
																																																																																																							 if(XDend)	range_X=&Solve_L2D::range_xx,		++Xoffset,	ra[0]=0,				ra[1]=Xoffset,	ra[2]=0,				ra[3]=nX-XDend,		ra[4]=0,				ra[5]=Xplaces,	ra[6]=nX-XDend,			ra[7]=nX;
																																																																																																						else			range_X=&Solve_L2D::range_x,		++Xoffset,	ra[0]=0,				ra[1]=Xoffset,	ra[2]=0,				ra[3]=nX;																								}
																							else		{	shift_X=(Shift_fn)&Solve_L2D::shift_;		 if(XDstart==	XDend)		modify_X=&Solve_L2D::modify_;
																																					else if(XDstart>	XDend)		modify_X=&Solve_L2D::modify_Xi;
																																					else if(XDstart<	XDend)		modify_X=&Solve_L2D::modify_XF;																																																					range_X=&Solve_L2D::range_;																																												}}
													else if(Yoffset<0)	{Yoffset=-Yoffset;		 if(nY)	{	shift_Y=(Shift_fn)&Solve_L2D::shift_yr;		 if(YDstart==	YDend)				modify_Y=&Solve_L2D::modify_,																sa[6]=nY-1-YDend,			sa[7]=-1,			sa[8]=nY-1,			sa[9]=Yplaces-1-Yoffset,	sa[10]=-1,		sa[11]=Yplaces-1;
																																					else if(YDstart>	YDend)				modify_Y=&Solve_L2D::modify_YI,															sa[6]=nY-1-YDend,			sa[7]=0,			sa[8]=nY-1,			sa[9]=Yplaces-1-Yoffset,	sa[10]=-1,		sa[11]=Yplaces-1;
																																					else if(YDstart<	YDend)				modify_Y=&Solve_L2D::modify_Yf,															sa[6]=nY-1-(YDend-1),		sa[7]=-1,			sa[8]=nY-1,			sa[9]=Yplaces-1-Yoffset,	sa[10]=-1,		sa[11]=Yplaces-1;																																																							
																																																																																																							 if(YDstart)range_Y=&Solve_L2D::range_yy,		++Yoffset,	ra[8]=0,				ra[9]=YDstart,	ra[10]=0,				ra[11]=Yplaces,		ra[12]=YDstart,			ra[13]=nY,		ra[14]=0,				ra[15]=Yoffset;
																																																																																																						else			range_Y=&Solve_L2D::range_y,					ra[8]=0,				ra[9]=nY,		ra[10]=0,				ra[11]=Yplaces;																							}
																							else		{	shift_Y=(Shift_fn)&Solve_L2D::shift_;		 if(YDstart==	YDend)				modify_Y=&Solve_L2D::modify_;
																																					else if(YDstart>	YDend)				modify_Y=&Solve_L2D::modify_YI;
																																					else if(YDstart<	YDend)				modify_Y=&Solve_L2D::modify_Yf;																																																			range_Y=&Solve_L2D::range_;																																												}
																								 if(nX)	{	shift_X=(Shift_fn)&Solve_L2D::shift_xr;		 if(XDstart==	XDend)		modify_X=&Solve_L2D::modify_,																		sa[0]=Xplaces-1-Xoffset,	sa[1]=-1,			sa[2]=Xplaces-1,	sa[3]=nX-1-XDend,			sa[4]=-1,		sa[5]=nX-1;
																																					else if(XDstart>	XDend)		modify_X=&Solve_L2D::modify_XI,																	sa[0]=Xplaces-1-Xoffset,	sa[1]=-1,			sa[2]=Xplaces-1,	sa[3]=nX-1-XDend,			sa[4]=0,		sa[5]=nX-1;
																																					else if(XDstart<	XDend)		modify_X=&Solve_L2D::modify_Xf,																	sa[0]=Xplaces-1-Xoffset,	sa[1]=-1,			sa[2]=Xplaces-1,	sa[3]=nX-1-(XDend-1),		sa[4]=-1,		sa[5]=nX-1;																																																									
																																																																																																							 if(XDstart)range_X=&Solve_L2D::range_xx,		++Xoffset,	ra[0]=0,				ra[1]=Xplaces,	ra[2]=0,				ra[3]=XDstart,		ra[4]=0,				ra[5]=Xoffset,	ra[6]=XDstart,			ra[7]=nX;
																																																																																																						else			range_X=&Solve_L2D::range_x,		++Xoffset,	ra[0]=0,				ra[1]=Xoffset,	ra[2]=0,				ra[3]=nX;																								}
																							else		{	shift_X=(Shift_fn)&Solve_L2D::shift_;		 if(XDstart==	XDend)		modify_X=&Solve_L2D::modify_;
																																					else if(XDstart>	XDend)		modify_X=&Solve_L2D::modify_XI;
																																					else if(XDstart<	XDend)		modify_X=&Solve_L2D::modify_Xf;																																																					range_X=&Solve_L2D::range_;																																												}}
													else				{																										modify_X=&Solve_L2D::modify_;					 if(nX)			shift_X=&Solve_L2D::shift_xr,	sa[0]=Xplaces-1-Xoffset,	sa[1]=-1,			sa[2]=Xplaces-1,	sa[3]=0,					sa[4]=nX,		sa[5]=0,						range_X=&Solve_L2D::range_x,		++Xoffset,	ra[0]=0,				ra[1]=Xoffset,	ra[2]=0,				ra[3]=nX;
																																																							else				shift_X=&Solve_L2D::shift_,																																						range_X=&Solve_L2D::range_;
																																						 if(YDstart==	YDend)	{			modify_Y=&Solve_L2D::modify_;		 if(YDend)		shift_Y=&Solve_L2D::shift_y,	sa[6]=nY-1-YDend,			sa[7]=-1,			sa[8]=nY-1,			sa[9]=0,					sa[10]=Yplaces,	sa[11]=0;
																																																							else				shift_Y=&Solve_L2D::shift_;																																																																																									}
																																					else if(YDstart>	YDend)	{			modify_Y=&Solve_L2D::modify_YI;		 if(YDend)		shift_Y=&Solve_L2D::shift_y,	sa[6]=nY-1-YDend,			sa[7]=0,			sa[8]=nY-1,			sa[9]=0,					sa[10]=Yplaces,	sa[11]=0;
																																																							else				shift_Y=&Solve_L2D::shift_;																																																																																									}
																																					else if(YDstart<	YDend)	{			modify_Y=&Solve_L2D::modify_Yf;		 if(YDend-1)	shift_Y=&Solve_L2D::shift_y,	sa[6]=nY-1-(YDend-1),		sa[7]=-1,			sa[8]=nY-1,			sa[9]=0,					sa[10]=Yplaces,	sa[11]=0;
																																																							else				shift_Y=&Solve_L2D::shift_;																																																																																									}
																																																																																																							 if(YDstart)range_Y=&Solve_L2D::range_y,					ra[8]=0,				ra[9]=YDstart,	ra[10]=0,				ra[11]=Yplaces;
																																																																																																						else			range_Y=&Solve_L2D::range_;																																												}}
			else				{																																						modify_Y=&Solve_L2D::modify_;
															if(Yoffset>0)	{																																					 if(nY)			shift_Y=&Solve_L2D::shift_y,	sa[6]=0,					sa[7]=nY,			sa[8]=0,			sa[9]=Yoffset,				sa[10]=Yplaces,	sa[11]=0,						range_Y=&Solve_L2D::range_y,		++Yoffset,	ra[8]=0,				ra[9]=nY,		ra[10]=Yplaces-Yoffset,	ra[11]=Yplaces;
																																																							else				shift_Y=&Solve_L2D::shift_,																																					range_Y=&Solve_L2D::range_;
																																						 if( XDstart==	XDend)	{	modify_X=&Solve_L2D::modify_;				 if(XDstart)	shift_X=&Solve_L2D::shift_x,	sa[0]=0,					sa[1]=Xplaces,		sa[2]=0,			sa[3]=XDstart,				sa[4]=nX,		sa[5]=0;
																																																							else				shift_X=&Solve_L2D::shift_;																																																																																									}
																																					else if( XDstart>	XDend)	{	modify_X=&Solve_L2D::modify_Xi;				 if(XDstart-1)	shift_X=&Solve_L2D::shift_x,	sa[0]=0,					sa[1]=Xplaces,		sa[2]=0,			sa[3]=XDstart-1,			sa[4]=nX,		sa[5]=0;
																																																							else				shift_X=&Solve_L2D::shift_;																																																																																									}
																																					else if( XDstart<	XDend)	{	modify_X=&Solve_L2D::modify_XF;				 if(XDstart)	shift_X=&Solve_L2D::shift_x,	sa[0]=0,					sa[1]=Xplaces,		sa[2]=0,			sa[3]=XDstart,				sa[4]=nX-1,		sa[5]=0;
																																																							else				shift_X=&Solve_L2D::shift_;																																																																																									}
																																																																																																							 if(XDend)	range_X=&Solve_L2D::range_x,					ra[0]=0,				ra[1]=Xplaces,	ra[2]=nX-XDend,			ra[3]=nX;
																																																																																																						else			range_X=&Solve_L2D::range_;																																												}
													else if(Yoffset<0)	{Yoffset=-Yoffset;																																		 if(nY)			shift_Y=&Solve_L2D::shift_yr,	sa[6]=0,					sa[7]=nY,			sa[8]=0,			sa[9]=Yplaces-1-Yoffset,	sa[10]=-1,		sa[11]=Yplaces-1,				range_Y=&Solve_L2D::range_y,		++Yoffset,	ra[8]=0,				ra[9]=nY,		ra[10]=0,				ra[11]=Yoffset;
																																																							else				shift_Y=&Solve_L2D::shift_,																																					range_Y=&Solve_L2D::range_;
																																						 if( XDstart==	XDend)	{	modify_X=&Solve_L2D::modify_;				 if(XDend)		shift_X=&Solve_L2D::shift_x,	sa[0]=0,					sa[1]=Xplaces,		sa[2]=0,			sa[3]=nX-1-XDend,			sa[4]=-1,		sa[5]=nX-1;
																																																							else				shift_X=&Solve_L2D::shift_;																																																																																									}
																																					else if( XDstart>	XDend)	{	modify_X=&Solve_L2D::modify_XI;				 if(XDend)		shift_X=&Solve_L2D::shift_x,	sa[0]=0,					sa[1]=Xplaces,		sa[2]=0,			sa[3]=nX-1-XDend,			sa[4]=0,		sa[5]=nX-1;
																																																							else				shift_X=&Solve_L2D::shift_;																																																																																									}
																																					else if( XDstart<	XDend)	{	modify_X=&Solve_L2D::modify_Xf;				 if(XDend-1)	shift_X=&Solve_L2D::shift_x,	sa[0]=0,					sa[1]=Xplaces,		sa[2]=0,			sa[3]=nX-1-(XDend-1),		sa[4]=-1,		sa[5]=nX-1;
																																																							else				shift_X=&Solve_L2D::shift_;																																																																																									}
																																																																																																							 if(XDstart)range_X=&Solve_L2D::range_x,					ra[0]=0,				ra[1]=Xplaces,	ra[2]=0,				ra[3]=XDstart;
																																																																																																						else			range_X=&Solve_L2D::range_;																																												}}
			
			partial=modify_X==&Solve_L2D::modify_&&modify_Y==&Solve_L2D::modify_?&Solve_L2D::partial_:&Solve_L2D::partial_m;
		}
	private:
		void partial_(Expression &ex)
		{
			auto &n=ex.n[ex.resultTerm];
			(this->*shift_X)(n.r, n.i, ex.discontinuities);
			(this->*shift_Y)(n.r, n.i, ex.discontinuities);
			
		//	ndr_to_clipboard_l2d(ex.n[ex.resultTerm], Xplaces, nX, Yplaces, nY);//
			(this->*range_X)(ex);
			(this->*range_Y)(ex);
		}
		void partial_m(Expression &ex)
		{
			auto &n=ex.n[ex.resultTerm];
		//	ndr_to_clipboard_l2d(ex.n[ex.resultTerm], Xplaces, old_nX, Yplaces, old_nY);//
			(this->*modify_Y)(n, ex.discontinuities);
			(this->*modify_X)(n, ex.discontinuities);
		//	ndr_to_clipboard_l2d(ex.n[ex.resultTerm], Xplaces, nX, Yplaces, nY);//
			resize_terms(ex, ndrSize=Xplaces*nX+Yplaces*nY);
							
			(this->*shift_X)(n.r, n.i, ex.discontinuities);
			(this->*shift_Y)(n.r, n.i, ex.discontinuities);
		//	ndr_to_clipboard_l2d(ex.n[ex.resultTerm], Xplaces, nX, Yplaces, nY);//
			
			(this->*range_X)(ex);
			(this->*range_Y)(ex);
		}
	public:
	//	decltype(&Solve_L2D::partial_) partial;
		void (modes::Solve_L2D::*partial)(Expression&);
	};
	class		Solve_3D:public Solve
	{
	public:
		int *ndr_rgb,
			*ndr_rgb_i, *ndr_rgb_j, *ndr_rgb_k;
		Solve_3D():ndr_rgb(0), ndr_rgb_i(0), ndr_rgb_j(0), ndr_rgb_k(0){}
		double Xstart, Xsample, Ystart, Ysample, Zstart, Zsample;
		int Xplaces, Yplaces, Zplaces, ndrSize;
	private:
		//int x, y, z;
		//double *p;
		////double x(){return Xstart+(.5+x)*Xsample;}
		////double y(){return Ystart+(.5+y)*Ysample;}
		////double z(){return Zstart+(.5+z)*Zsample;}
		//double fx(){return Xstart+x*Xsample;}
		//double fy(){return Ystart+y*Ysample;}
		//double fz(){return Zstart+z*Zsample;}
		//double fc(){return *p;}
		int sa[9], ra[12];
		//void LOL_1(Variable &variables, std::remove_reference<double(modes::Solve_3D::*)()>::type &f, int varType, int &LOL_1_const)
		//{
		//	switch(varType)
		//	{
		//	case 'x':f=&Solve_3D::fx, LOL_1_const=0;break;
		//	case 'y':f=&Solve_3D::fy, LOL_1_const=0;break;
		//	case 'z':f=&Solve_3D::fz, LOL_1_const=0;break;
		//	case 'c':f=&Solve_3D::fc, p=&variables.val.r;break;
		//	case 't':f=&Solve_3D::fc, p=&T;break;
		//	}
		//};
	public:
		void full_resize(double VX, double DX, double VY, double DY, double VZ, double DZ, int Xplaces, int Yplaces, int Zplaces)
		{
			this->Xplaces=Xplaces, this->Yplaces=Yplaces, this->Zplaces=Zplaces, ndrSize=Xplaces*Yplaces*Zplaces, kzStep=Yplaces*Xplaces;
			Xstart=VX-DX/2, Xsample=DX/Xplaces, Ystart=VY-DY/2, Ysample=DY/Yplaces, Zstart=VZ-DZ/2, Zsample=DZ/Zplaces;
			ndr_rgb=(int*)realloc(ndr_rgb, ndrSize*sizeof(int));
			ndr_rgb_i=(int*)realloc(ndr_rgb_i, ndrSize*sizeof(int));
			ndr_rgb_j=(int*)realloc(ndr_rgb_j, ndrSize*sizeof(int));
			ndr_rgb_k=(int*)realloc(ndr_rgb_k, ndrSize*sizeof(int));
		}
		void full(Expression &ex)
		{
			modes::Xstart=Xstart, modes::Xsample=Xsample, modes::Ystart=Ystart, modes::Ysample=Ysample, modes::Zstart=Zstart, modes::Zsample=Zsample, modes::T=T;
			modes::Xplaces=Xplaces, modes::Yplaces=Yplaces, modes::Zplaces=Zplaces, modes::ndrSize=ndrSize;

			resize_terms(ex, ndrSize);
			fill_range(ex, choose_fill_fn, 0, 0, Xplaces, 0, Yplaces, 0, Zplaces, Xplaces, Yplaces, Zplaces, Xplaces*Yplaces);
			solve_disc(ex, 0, 0, Xplaces, 0, Yplaces, 0, Zplaces, Xplaces, Yplaces, Zplaces,	0, 0, 0, 0, 0, 0);
		}
		void updateRGB(Expression &ex)
	//	void updateRGB(Expression::Term &n)
		{
			auto &n=ex.n[ex.resultTerm];
			auto &ndr_r=n.r, &ndr_i=n.i, &ndr_j=n.j, &ndr_k=n.k;
			if(ex.resultMathSet=='R')
			{
				//std::transform(ndr_r.begin(), ndr_r.begin()+ndrSize, ndr_rgb, colorFunction_r(x));
				std::transform(ndr_r.begin(), ndr_r.begin()+ndrSize, ndr_rgb, [&](double x){return colorFunction_r(x);});
			}
			else if(ex.resultMathSet=='c')
			{
				for(int k=0;k<ndrSize;++k)
					ndr_rgb[k]=colorFunction(ndr_r[k], ndr_i[k]);
			}
			else
			{
				for(int k=0;k<ndrSize;++k)
					colorFunction_q(Value(ndr_r[k], ndr_i[k], ndr_j[k], ndr_k[k]), ndr_rgb[k], ndr_rgb_i[k], ndr_rgb_j[k], ndr_rgb_k[k]);
			}
		}
	private:
		int kzStep;
		void shift_component(std::vector<double> &ndr)
		{
			int XSstart=sa[0], XSend=sa[1], XDstart=sa[2], YSstart=sa[3], YSend=sa[4], YDstart=sa[5], ZSstart=sa[6], ZSend=sa[7], ZDstart=sa[8];
			auto _First=ndr.begin()+XSstart, _Last=ndr.begin()+XSend, _Dest=ndr.begin()+XDstart;
			for(int kz=Xplaces*Yplaces*ZSstart, kzEnd=Xplaces*Yplaces*ZSend, kzStep=(kz<kzEnd?1:-1)*Xplaces*Yplaces, kz2=Xplaces*Yplaces*ZDstart;kz!=kzEnd;kz+=kzStep, kz2+=kzStep)
				for(int ky=kz+Xplaces*YSstart, kyEnd=kz+Xplaces*YSend, kyStep=(ky<kyEnd?1:-1)*Xplaces, ky2=kz2+Xplaces*YDstart;ky!=kyEnd;ky+=kyStep, ky2+=kyStep)
					std::copy(_First+ky, _Last+ky, _Dest+ky2);
		}
		void shift_		(Expression &ex)
		{
			auto &n=ex.n[ex.resultTerm];
			shift_component(n.r);
			if(ex.resultMathSet>='c')
			{
				shift_component(n.i);
				if(ex.resultMathSet=='h')
				{
					shift_component(n.j);
					shift_component(n.k);
				}
			}
		}
		void shift_r_component(std::vector<double> &ndr)
		{
			int XSstart=sa[0], XSend=sa[1], XDstart=sa[2], YSstart=sa[3], YSend=sa[4], YDstart=sa[5], ZSstart=sa[6], ZSend=sa[7], ZDstart=sa[8];
			auto rb=ndr.rbegin();
		//	auto rb=std::reverse_iterator<double*>(ndr+ndrSize);
			auto _First=rb+Xplaces-1-XSstart, _Last=rb+Xplaces-1-XSend, _Dest=rb+Xplaces-1-XDstart;
			for(int kz=Xplaces*Yplaces*(Zplaces-1-ZSstart), kzEnd=Xplaces*Yplaces*(Zplaces-1-ZSend), kzStep=(kz<kzEnd?1:-1)*Xplaces*Yplaces, kz2=Xplaces*Yplaces*(Zplaces-1-ZDstart);kz!=kzEnd;kz+=kzStep, kz2+=kzStep)
				for(int ky=kz+Xplaces*(Yplaces-1-YSstart), kyEnd=kz+Xplaces*(Yplaces-1-YSend), kyStep=(ky<kyEnd?1:-1)*Xplaces, ky2=kz2+Xplaces*(Yplaces-1-YDstart);ky!=kyEnd;ky+=kyStep, ky2+=kyStep)
					std::copy(_First+ky, _Last+ky, _Dest+ky2);
		}
		void shift_r	(Expression &ex)
		{
			auto &n=ex.n[ex.resultTerm];
			shift_r_component(n.r);
			if(ex.resultMathSet>='c')
			{
				shift_r_component(n.i);
				if(ex.resultMathSet=='h')
				{
					shift_r_component(n.j);
					shift_r_component(n.k);
				}
			}
		}
		void (modes::Solve_3D::*shift)(Expression&);
		void solve_range	(Expression &ex, int *a)
		{
			int x1=a[0], x2=a[1], y1=a[2], y2=a[3], z1=a[4], z2=a[5];
			modes::Xstart=Xstart, modes::Xsample=Xsample, modes::Ystart=Ystart, modes::Ysample=Ysample, modes::Zstart=Zstart, modes::Zsample=Zsample, modes::T=T;
			modes::Xplaces=Xplaces, modes::Yplaces=Yplaces, modes::Zplaces=Zplaces, modes::ndrSize=ndrSize;
			fill_range(ex, choose_fill_fn, 0, x1, x2, y1, y2, z1, z2, Xplaces, Yplaces, Zplaces, kzStep);
			solve_disc(ex, 0, x1, x2, y1, y2, z1, z2, Xplaces, Yplaces, Zplaces,	0, 0, 0, 0, 0, 0);
		}
	public:
		void partial_bounds	(double VX, double DX, double VY, double DY, double VZ, double DZ, int Xoffset, int Yoffset, int Zoffset)
		{
			partial=&Solve_3D::partial_;
			Xstart=VX-DX/2, Xsample=DX/Xplaces, Ystart=VY-DY/2, Xsample=DY/Yplaces, Zstart=VZ-DZ/2, Xsample=DZ/Zplaces;
				 if(Xoffset>0){							 if(Yoffset>0){							 if(Zoffset>0);
																							else if(Zoffset<0);
																							else								shift=&Solve_3D::shift_,	sa[0]=Xoffset,				sa[1]=Xplaces,	sa[2]=0,			sa[3]=Yoffset,				sa[4]=Yplaces,	sa[5]=0,			sa[6]=Zoffset,				sa[7]=Zplaces,	sa[8]=0,					partial=&Solve_3D::partial_2,	ra[0]=0,				ra[1]=Xplaces,	ra[2]=Yplaces-Yoffset,	ra[3]=Yplaces, ra[4]=0,					ra[5]=Zplaces,				ra[6]=Xplaces-Xoffset,	ra[7]=Xplaces, ra[8]=0,					ra[9]=Yplaces-Yoffset,	ra[10]=0,				ra[11]=Zplaces;			}
													else if(Yoffset<0){Yoffset=-Yoffset;		 if(Zoffset>0);
																							else if(Zoffset<0);
																							else								shift=&Solve_3D::shift_,	sa[0]=Xoffset,				sa[1]=Xplaces,	sa[2]=0,			sa[3]=Yplaces-1-Yoffset,	sa[4]=-1,		sa[5]=Yplaces-1,	sa[6]=Zoffset,				sa[7]=Zplaces,	sa[8]=0,					partial=&Solve_3D::partial_2,	ra[0]=0,				ra[1]=Xplaces,	ra[2]=0,				ra[3]=Yoffset, ra[4]=0,					ra[5]=Zplaces,				ra[6]=Xplaces-Xoffset,	ra[7]=Xplaces, ra[8]=Yoffset,			ra[9]=Yplaces,			ra[10]=0,				ra[11]=Zplaces;			}
													else{										 if(Zoffset>0)					shift=&Solve_3D::shift_,	sa[0]=Xoffset,				sa[1]=Xplaces,	sa[2]=0,			sa[3]=Yoffset,				sa[4]=Yplaces,	sa[5]=0,			sa[6]=Zoffset,				sa[7]=Zplaces,	sa[8]=0,					partial=&Solve_3D::partial_2,	ra[0]=0,				ra[1]=Xplaces,	ra[2]=0,				ra[3]=Yplaces, ra[4]=Zplaces-Zoffset,	ra[5]=Zplaces,				ra[6]=Xplaces-Xoffset,	ra[7]=Xplaces, ra[8]=0,					ra[9]=Yplaces,			ra[10]=0,				ra[11]=Zplaces-Zoffset;
																							else if(Zoffset<0)Zoffset=-Zoffset,	shift=&Solve_3D::shift_,	sa[0]=Xoffset,				sa[1]=Xplaces,	sa[2]=0,			sa[3]=Yoffset,				sa[4]=Yplaces,	sa[5]=0,			sa[6]=Zplaces-1-Zoffset,	sa[7]=-1,		sa[8]=Zplaces-1,			partial=&Solve_3D::partial_2,	ra[0]=0,				ra[1]=Xplaces,	ra[2]=0,				ra[3]=Yplaces, ra[4]=0,					ra[5]=Zoffset,				ra[6]=Xplaces-Xoffset,	ra[7]=Xplaces, ra[8]=0,					ra[9]=Yplaces,			ra[10]=Zoffset,			ra[11]=Zplaces;
																							else								shift=&Solve_3D::shift_,	sa[0]=Xoffset,				sa[1]=Xplaces,	sa[2]=0,			sa[3]=Yoffset,				sa[4]=Yplaces,	sa[5]=0,			sa[6]=Zoffset,				sa[7]=Zplaces,	sa[8]=0,					partial=&Solve_3D::partial_1,	ra[0]=Xplaces-Xoffset,	ra[1]=Xplaces,	ra[2]=0,				ra[3]=Yplaces, ra[4]=0,					ra[5]=Zplaces;																																						}}
			else if(Xoffset<0){Xoffset=-Xoffset;		 if(Yoffset>0){							 if(Zoffset>0);
																							else if(Zoffset<0);
																							else								shift=&Solve_3D::shift_r,	sa[0]=Xplaces-1-Xoffset,	sa[1]=-1,		sa[2]=Xplaces-1,	sa[3]=Yoffset,				sa[4]=Yplaces,	sa[5]=0,			sa[6]=Zoffset,				sa[7]=Zplaces,	sa[8]=0,					partial=&Solve_3D::partial_2,	ra[0]=0,				ra[1]=Xplaces,	ra[2]=Yplaces-Yoffset,	ra[3]=Yplaces, ra[4]=0,					ra[5]=Zplaces,				ra[6]=0,				ra[7]=Xoffset, ra[8]=0,					ra[9]=Yplaces-Yoffset,	ra[10]=0,				ra[11]=Zplaces;			}
													else if(Yoffset<0){Yoffset=-Yoffset;		 if(Zoffset>0);
																							else if(Zoffset<0);
																							else								shift=&Solve_3D::shift_r,	sa[0]=Xplaces-1-Xoffset,	sa[1]=-1,		sa[2]=Xplaces-1,	sa[3]=Yplaces-1-Yoffset,	sa[4]=-1,		sa[5]=Yplaces-1,	sa[6]=Zoffset,				sa[7]=Zplaces,	sa[8]=0,					partial=&Solve_3D::partial_2,	ra[0]=0,				ra[1]=Xplaces,	ra[2]=0,				ra[3]=Yoffset, ra[4]=0,					ra[5]=Zplaces,				ra[6]=0,				ra[7]=Xoffset, ra[8]=Yoffset,			ra[9]=Yplaces,			ra[10]=0,				ra[11]=Zplaces;			}
													else{										 if(Zoffset>0)					shift=&Solve_3D::shift_r,	sa[0]=Xplaces-1-Xoffset,	sa[1]=-1,		sa[2]=Xplaces-1,	sa[3]=Yoffset,				sa[4]=Yplaces,	sa[5]=0,			sa[6]=Zoffset,				sa[7]=Zplaces,	sa[8]=0,					partial=&Solve_3D::partial_2,	ra[0]=0,				ra[1]=Xplaces,	ra[2]=0,				ra[3]=Yplaces, ra[4]=Zplaces-Zoffset,	ra[5]=Zplaces,				ra[6]=0,				ra[7]=Xoffset, ra[8]=0,					ra[9]=Yplaces,			ra[10]=0,				ra[11]=Zplaces-Zoffset;
																							else if(Zoffset<0)Zoffset=-Zoffset,	shift=&Solve_3D::shift_r,	sa[0]=Xplaces-1-Xoffset,	sa[1]=-1,		sa[2]=Xplaces-1,	sa[3]=Yoffset,				sa[4]=Yplaces,	sa[5]=0,			sa[6]=Zplaces-Zoffset-1,	sa[7]=-1,		sa[8]=Zplaces-1,			partial=&Solve_3D::partial_2,	ra[0]=0,				ra[1]=Xplaces,	ra[2]=0,				ra[3]=Yplaces, ra[4]=0,					ra[5]=Zoffset,				ra[6]=0,				ra[7]=Xoffset, ra[8]=0,					ra[9]=Yplaces,			ra[10]=Zoffset,			ra[11]=Zplaces;
																							else								shift=&Solve_3D::shift_r,	sa[0]=Xplaces-1-Xoffset,	sa[1]=-1,		sa[2]=Xplaces-1,	sa[3]=Yoffset,				sa[4]=Yplaces,	sa[5]=0,			sa[6]=Zoffset,				sa[7]=Zplaces,	sa[8]=0,					partial=&Solve_3D::partial_1,	ra[0]=0,				ra[1]=Xoffset,	ra[2]=0,				ra[3]=Yplaces, ra[4]=0,					ra[5]=Zplaces;																																						}}
			else{										if(Yoffset>0){							 if(Zoffset>0)					shift=&Solve_3D::shift_,	sa[0]=Xoffset,				sa[1]=Xplaces,	sa[2]=0,			sa[3]=Yoffset,				sa[4]=Yplaces,	sa[5]=0,			sa[6]=Zoffset,				sa[7]=Zplaces,	sa[8]=0,					partial=&Solve_3D::partial_2,	ra[0]=0,				ra[1]=Xplaces,	ra[2]=0,				ra[3]=Yplaces, ra[4]=Zplaces-Zoffset,	ra[5]=Zplaces,				ra[6]=0,				ra[7]=Xplaces, ra[8]=Yplaces-Yoffset,	ra[9]=Yplaces,			ra[10]=0,				ra[11]=Zplaces-Zoffset;
																							else if(Zoffset<0)Zoffset=-Zoffset,	shift=&Solve_3D::shift_,	sa[0]=Xoffset,				sa[1]=Xplaces,	sa[2]=0,			sa[3]=Yoffset,				sa[4]=Yplaces,	sa[5]=0,			sa[6]=Zplaces-Zoffset-1,	sa[7]=-1,		sa[8]=Zplaces-1,			partial=&Solve_3D::partial_2,	ra[0]=0,				ra[1]=Xplaces,	ra[2]=0,				ra[3]=Yplaces, ra[4]=0,					ra[5]=Zoffset,				ra[6]=0,				ra[7]=Xplaces, ra[8]=Yplaces-Yoffset,	ra[9]=Yplaces,			ra[10]=Zoffset,			ra[11]=Zplaces;
																							else								shift=&Solve_3D::shift_,	sa[0]=Xoffset,				sa[1]=Xplaces,	sa[2]=0,			sa[3]=Yoffset,				sa[4]=Yplaces,	sa[5]=0,			sa[6]=Zoffset,				sa[7]=Zplaces,	sa[8]=0,					partial=&Solve_3D::partial_1,	ra[0]=0,				ra[1]=Xplaces,	ra[2]=Yplaces-Yoffset,	ra[3]=Yplaces, ra[4]=0,					ra[5]=Zplaces;																																						}
													else if(Yoffset<0){Yoffset=-Yoffset;		 if(Zoffset>0)					shift=&Solve_3D::shift_,	sa[0]=Xoffset,				sa[1]=Xplaces,	sa[2]=0,			sa[3]=Yplaces-1-Yoffset,	sa[4]=-1,		sa[5]=Yplaces-1,	sa[6]=Zoffset,				sa[7]=Zplaces,	sa[8]=0,					partial=&Solve_3D::partial_2,	ra[0]=0,				ra[1]=Xplaces,	ra[2]=0,				ra[3]=Yplaces, ra[4]=Zplaces-Zoffset,	ra[5]=Zplaces,				ra[6]=0,				ra[7]=Xplaces, ra[8]=0,					ra[9]=Yplaces,			ra[10]=Zplaces-Zoffset,	ra[11]=Zplaces;
																							else if(Zoffset<0)Zoffset=-Zoffset,	shift=&Solve_3D::shift_,	sa[0]=Xoffset,				sa[1]=Xplaces,	sa[2]=0,			sa[3]=Yplaces-1-Yoffset,	sa[4]=-1,		sa[5]=Yplaces-1,	sa[6]=Zplaces-Zoffset-1,	sa[7]=-1,		sa[8]=Zplaces-1,			partial=&Solve_3D::partial_2,	ra[0]=0,				ra[1]=Xplaces,	ra[2]=0,				ra[3]=Yplaces, ra[4]=0,					ra[5]=Zoffset,				ra[6]=0,				ra[7]=Xplaces, ra[8]=0,					ra[9]=Yoffset,			ra[10]=Zplaces-Zoffset,	ra[11]=Zplaces;
																							else								shift=&Solve_3D::shift_,	sa[0]=Xoffset,				sa[1]=Xplaces,	sa[2]=0,			sa[3]=Yplaces-1-Yoffset,	sa[4]=-1,		sa[5]=Yplaces-1,	sa[6]=Zoffset,				sa[7]=Zplaces,	sa[8]=0,					partial=&Solve_3D::partial_1,	ra[0]=0,				ra[1]=Xplaces,	ra[2]=0,				ra[3]=Yoffset, ra[4]=0,					ra[5]=Zplaces;																																						}
													else{										 if(Zoffset>0)					shift=&Solve_3D::shift_,	sa[0]=Xoffset,				sa[1]=Xplaces,	sa[2]=0,			sa[3]=Yoffset,				sa[4]=Yplaces,	sa[5]=0,			sa[6]=Zoffset,				sa[7]=Zplaces,	sa[8]=0,					partial=&Solve_3D::partial_1,	ra[0]=0,				ra[1]=Xplaces,	ra[2]=0,				ra[3]=Yplaces, ra[4]=Zplaces-Zoffset,	ra[5]=Zplaces;
																							else if(Zoffset<0)Zoffset=-Zoffset,	shift=&Solve_3D::shift_,	sa[0]=Xoffset,				sa[1]=Xplaces,	sa[2]=0,			sa[3]=Yoffset,				sa[4]=Yplaces,	sa[5]=0,			sa[6]=Zplaces-Zoffset-1,	sa[7]=-1,		sa[8]=Zplaces-1,			partial=&Solve_3D::partial_1,	ra[0]=0,				ra[1]=Xplaces,	ra[2]=0,				ra[3]=Yplaces, ra[4]=0,					ra[5]=Zoffset;
																							else;}}
		}
	private:
		void partial_		(Expression &ex){}
		void partial_1		(Expression &ex){(this->*shift)(ex), solve_range(ex, ra);}
		void partial_2		(Expression &ex){(this->*shift)(ex), solve_range(ex, ra), solve_range(ex, ra+6);}
	public:
		void (modes::Solve_3D::*partial)(Expression&);
	};
	
	void derive_step(double DV, int v, double &Vstep, int &prec)
	{
		double t=grid_step*DV/v;
		int t2=floor_log10(t);
		Vstep=_10pow(t2), prec=-clamp_negative(t2);
	//	Vstep=exp(t2*G2::_ln10), prec=t2<0?int(-t2):0;
		switch(int(t/Vstep))
		{
		case 1:Vstep*=1;break;
		case 2:Vstep*=2;break;
		case 3:Vstep*=2;break;
		case 4:Vstep*=2;break;
		case 5:Vstep*=5;break;
		case 6:Vstep*=5;break;
		case 7:Vstep*=5;break;
		case 8:Vstep*=5;break;
		case 9:Vstep*=5;break;
		default:Vstep*=6;break;
	//	default:Vstep*=1;break;
		}
	}
	void derive_step_noprec(double DV, int v, double &Vstep)
	{
		double t=grid_step*DV/v;
		Vstep=_10pow(floor_log10(t));
	//	Vstep=exp(floor_log10(t)*G2::_ln10);
		switch(int(t/Vstep))
		{
		case 1:Vstep*=1;break;
		case 2:Vstep*=2;break;
		case 3:Vstep*=2;break;
		case 4:Vstep*=2;break;
		case 5:Vstep*=5;break;
		case 6:Vstep*=5;break;
		case 7:Vstep*=5;break;
		case 8:Vstep*=5;break;
		case 9:Vstep*=5;break;
		default:Vstep*=6;break;
	//	default:Vstep*=1;break;
		}
	}
	void derive_step_2D(double const &DX, double const &AR_Y, int w, double &Xstep, double &Ystep, int &Xprec, int &Yprec)
	{
		derive_step(DX, w, Xstep, Xprec);
		if(AR_Y==1)
			Ystep=Xstep;
		else
			derive_step(DX/AR_Y, w, Ystep, Yprec);
	}
	void derive_step_3D(double const &DX, double const &AR_Y, double const &AR_Z, int w, double &Xstep, double &Ystep, double &Zstep)
	{
		int prec=0;
		derive_step_noprec(DX, w, Xstep);
		if(AR_Y==1)
			Ystep=Xstep;
		else
			derive_step_noprec(DX/AR_Y, w, Ystep);
		if(AR_Z==1)
			Zstep=Xstep;
		else
			derive_step_noprec(DX/AR_Z, w, Zstep);
	}
	void _3dMode_DrawGridNAxes(_3D &_3d, double &VX, double &VY, double &VZ, double &DX, double Xstep, double Ystep, double Zstep, double AR_Y, double AR_Z)
	{
		_3d.lineColor=_3dGridColor;
		real
			Xstart=VX-DX/2, Xend=VX+DX/2,
			Ystart=VY-DX/2, Yend=VY+DX/2,
			Zstart=VZ-DX/2, Zend=VZ+DX/2,
			Yst_r=AR_Y*Ystep, Zst_r=AR_Z*Zstep;
		for(real x=(floor(Xstart/Xstep)+1)*Xstep, xEnd=floor(Xend/Xstep)*Xstep;x<xEnd;x+=Xstep)//floor+1: xo grid, ceil: L grid at reset
			_3d.line(vec3(x, Ystart, VZ), vec3(x, Yend, VZ)), _3d.line(vec3(x, VY, Zstart), vec3(x, VY, Zend));
		//	_3d.line(x, Ystart, VZ, x, Yend, VZ), _3d.line(x, VY, Zstart, x, VY, Zend);
		for(real y=(floor(Ystart/Yst_r)+1)*Yst_r, yEnd=floor(Yend/Yst_r)*Yst_r;y<yEnd;y+=Yst_r)
			_3d.line(vec3(Xstart, y, VZ), vec3(Xend, y, VZ)), _3d.line(vec3(VX, y, Zstart), vec3(VX, y, Zend));
		//	_3d.line(Xstart, y, VZ, Xend, y, VZ), _3d.line(VX, y, Zstart, VX, y, Zend);
		for(real z=(floor(Zstart/Zst_r)+1)*Zst_r, zEnd=floor(Zend/Zst_r)*Zst_r;z<zEnd;z+=Zst_r)
			_3d.line(vec3(Xstart, VY, z), vec3(Xend, VY, z)), _3d.line(vec3(VX, Ystart, z), vec3(VX, Yend, z));
		//	_3d.line(Xstart, VY, z, Xend, VY, z), _3d.line(VX, Ystart, z, VX, Yend, z);
		_3d.lineColor=0;
		vec3 zero;
		_3d.line(zero, vec3(10, 0, 0));
		_3d.line(zero, vec3(0, AR_Y*10, 0));
		_3d.line(zero, vec3(0, 0, AR_Z*10));
//		_3d.line(0, 0, 0, 10,	0,			0);
//		_3d.line(0, 0, 0, 0,	AR_Y*10,	0);
//		_3d.line(0, 0, 0, 0,	0,			AR_Z*10);
	}
	void _3dMode_FrameStart(_3D &_3d, double &VX, double &VY, double &VZ, double &DX, double Xstep, double Ystep, double Zstep, double AR_Y, double AR_Z)
	{
		//_3d.newFrame();
		_3dMode_DrawGridNAxes(_3d, VX, VY, VZ, DX, Xstep, Ystep, Zstep, AR_Y, AR_Z);
	}
	void _2dMode_DrawCheckboard(int color, double const &VX, double const &VY, double const &DX, double const &DY, double const &Xstep, double const &Ystep)
	{
		set_color(color);
		{
			double Ystart=VY-DY/2, Yend=VY+DY/2, Ystepx2=Ystep*2, Xstart=VX-DX/2, Xend=VX+DX/2, Xstepx2=Xstep*2;
			for(double y=ceil(Yend/Ystepx2)*Ystepx2, yEnd=floor(Ystart/Ystep)*Ystep-Ystepx2;y>yEnd&&y-Ystepx2!=y;y-=Ystepx2)
			{
				for(double x=floor(Xstart/Xstepx2)*Xstepx2, xEnd=ceil (Xend/Xstep)*Xstep;x<xEnd&&x+Xstepx2!=x;x+=Xstepx2)
				{
					double
						ax1=(x		-Xstart)/DX*w, ay1=(Yend-y		)/DY*h, ax2=(x+Xstep	-Xstart)/DX*w, ay2=(Yend-y-Ystep	)/DY*h,
						bx1=(x+Xstep-Xstart)/DX*w, by1=(Yend-y-Ystep)/DY*h, bx2=(x+Xstepx2	-Xstart)/DX*w, by2=(Yend-y-Ystepx2	)/DY*h;
					GL2_2D::draw_rectangle(ax1, ax2, ay1, ay2);
					GL2_2D::draw_rectangle(bx1, bx2, by1, by2);
				}
			}
		}
	}
	void _2dMode_NumberAxes(double const &VX, double const &VY, double const &DX, double const &DY, double const &Xstep, double const &Ystep, int prec, int &H, int &V, int &VT)
	{
		Font::change(0xFF000000, 0xFFFFFFFF, preferred_fontH);
		H=int(VY-DY/2>0?h:VY+DY/2<0?-1:h*(VY/DY+.5)); int HT=H+(H>h-30?-18:2); V=int(VX-DX/2>0?-1:VX+DX/2<0?w:w*(-VX+DX/2)/DX), VT=V+int(V>w-24-prec*8?-24-prec*8:2);
		for(double x=floor((VX-DX/2)/Xstep)*Xstep, xEnd=ceil((VX+DX/2)/Xstep)*Xstep, Xstep_2=Xstep/2;x<xEnd&&x+Xstep!=x;x+=Xstep)
		{
			if(x>-Xstep_2&&x<Xstep_2)
				continue;
			double X=w*(x-(VX-DX/2))/DX;
			print(int(X)-(X<0)+2, HT, "%g", x);
		}
		for(double y=ceil((VY+DY/2)/Ystep)*Ystep, yEnd=floor((VY-DY/2)/Ystep)*Ystep, Ystep_2=Ystep/2;y>yEnd&&y-Ystep!=y;y-=Ystep)
		{
			if(y>-Ystep_2&&y<Ystep_2)
				continue;
			double Y=h*((VY+DY/2)-y)/DY;
			print(VT, int(Y)-(Y<0)+2, "%g", y);
		}
	}

	class		Mode
	{
	public:
		static const int arrow_label_offset_X=50, const_label_offset_X=150;
		bool ready;
		bool toSolve;
		int bpx, bpy, bw, bh;
		bool time_variance, paused;
		Mode():ready(false), toSolve(false), bpx(0), bpy(0), bw(0), bh(0), time_variance(false), paused(false){}

		virtual void enter()=0;//mode becomes active or inactive
		virtual void exit()=0;
		virtual void pause()=0;//mode goes away
		virtual void resume()=0;
		virtual void appDeactivate()=0;//application deactivated
		virtual void appActivate()=0;

		virtual void setDimensions(int x, int y, int w, int h)=0;
	//	virtual void messagePaint(int x, int y, int w, int h)=0;
		virtual void messageTimer()=0;
		virtual int inputTouchDown(int idx)=0;
		virtual void inputTouchUp(int idx)=0;
		virtual void inputTouchMove(int idx)=0;
	//	virtual int inputLButtonDown(int lParam)=0;
	//	virtual int inputMouseMove(int lParam)=0;
	//	virtual void inputLButtonUp(int lParam)=0;
	//	virtual int inputMouseWheel(int wParam)=0;
	//	virtual int inputKeyDown(int wParam, int lParam)=0;
	//	virtual int inputKeyUp(int wParam)=0;
		virtual void a_draw()=0;
		virtual void i_draw()=0;
	} *mode, *old_mode=nullptr;
	class		_2D_Mode:public Mode
	{//ti2d		t1d, t1d_h, c2d, l2d, ti2d
	public:
		int shiftOnly;//0 full, 1 partial, 2 no change yet
		
		double VX, VY, DX, AR_Y, Xstep, Ystep;
		int prec;
		Solve &solver;

		bool timer, drag, m_bypass;
		int kp;
		_2D_Mode(Solve &solver):
			shiftOnly(2),
			VX(0), VY(0), DX(20), AR_Y(1),
			solver(solver),
			timer(false), drag(false), m_bypass(false), kp(0)
		{}
		void enter()
		{
//			if(kb[VK_LBUTTON])
//			{
//				ShowCursor(0);
//				GetCursorPos(&mouseP0);
//				SetCursorPos(centerP.x, centerP.y);
//			//	SetCapture(ghWnd);
//				drag=1;
//			}
//			kp=kb[VK_UP]+kb[VK_DOWN]+kb[VK_LEFT]+kb[VK_RIGHT]
//				+kb[VK_ADD]+kb[VK_SUBTRACT]//numpad
//				+kb[VK_OEM_PLUS]+kb[VK_OEM_MINUS]
//				+kb[VK_RETURN]+kb[VK_BACK];
//			if(!time_variance&&kp&&!timer)
//				SetTimer(ghWnd, 0, 10, 0), timer=true;
		}
		void exit()
		{
			drag=0;
//			if(drag)
//			{
//				drag=0;
//			//	ReleaseCapture();
//				SetCursorPos(mouseP0.x, mouseP0.y);
//				ShowCursor(1);
//			}
//			if(!time_variance&&timer)
//				KillTimer(ghWnd, 0), timer=false;
		}
		void pause()
		{
			if(time_variance&&!paused)
			{
				solver.pause();
			//	KillTimer(ghWnd, 0);
				paused=true;
			}
		}
		void resume()
		{
			if(time_variance)
			{
			//	SetTimer(ghWnd, 0, 10, 0);
				solver.resume();
				paused=false;
			}
		}
		void appDeactivate()
		{
			drag=0;
//			if(drag)
//			{
//				ReleaseCapture();
//				SetCursorPos(mouseP0.x, mouseP0.y);
//				ShowCursor(1);
//				drag=0;
//			}
//			kp=0, timer=false;
//			if(!time_variance)
//				KillTimer(ghWnd, 0);
		}
		void appActivate()
		{
//			kp=kb[VK_UP]+kb[VK_DOWN]+kb[VK_LEFT]+kb[VK_RIGHT]
//				+kb[VK_ADD]+kb[VK_SUBTRACT]//numpad
//				+kb[VK_OEM_PLUS]+kb[VK_OEM_MINUS]
//				+kb[VK_RETURN]+kb[VK_BACK];
//			if(!time_variance&&kp&&!timer)
//				SetTimer(ghWnd, 0, 10, 0), timer=true;
		}
		
		void setDimensions(int x, int y, int w, int h)
		{
			bpx=x, bpy=y, bw=w, bh=h;
			function1();
			toSolve=true, shiftOnly=0;
			ready=true;
		}

		void function1(){derive_step_2D(DX, AR_Y, w, Xstep, Ystep, prec, prec);}
	};
	class		_3D_Mode:public Mode
	{
	public:
		int shiftOnly;//0 full, 1 partial, 2 no change yet
		
		double VX, VY, VZ, DX, AR_Y, AR_Z, Xstep, Ystep, Zstep;
		int prec;
		Solve &solver;

		bool timer, drag, m_bypass, shift;
		int kp;

		_3D_Mode(Solve &solver):
			shiftOnly(2),
			VX(0), VY(0), VZ(0), DX(20), AR_Y(1), AR_Z(1),
			solver(solver),
			timer(false), m_bypass(false), drag(false), shift(false), kp(0)
		{}

		void enter()
		{
//			if(kb[VK_LBUTTON])
//			{
//				ShowCursor(0);
//				GetCursorPos(&mouseP0);
//				SetCursorPos(centerP.x, centerP.y);
//			//	SetCapture(ghWnd);
//				drag=1;
//			}
//			kp=kb['W']+kb['A']+kb['S']+kb['D']+kb[VK_UP]+kb[VK_DOWN]+kb[VK_LEFT]+kb[VK_RIGHT]
//				+kb[VK_ADD]+kb[VK_SUBTRACT]//numpad
//				+kb[VK_OEM_PLUS]+kb[VK_OEM_MINUS]
//				+kb[VK_RETURN]+kb[VK_BACK];
//			if(!time_variance&&kp&&!timer)
//				SetTimer(ghWnd, 0, 10, 0), timer=true;
		}
		void exit()
		{
			drag=0, shift=0;
//			if(drag||shift)
//			{
//				drag=0, shift=0;
//			//	ReleaseCapture();
//				SetCursorPos(mouseP0.x, mouseP0.y);
//				ShowCursor(1);
//			}
//			if(!time_variance&&timer)
//				KillTimer(ghWnd, 0), timer=false;
		}
		void pause()
		{
			if(time_variance&&!paused)
			{
				solver.pause();
			//	KillTimer(ghWnd, 0);
				paused=true;
			}
		}
		void resume()
		{
			if(time_variance)
			{
			//	SetTimer(ghWnd, 0, 10, 0);
				solver.resume();
				paused=false;
			}
		}
		void appDeactivate()
		{
			drag=0;
//			if(drag)
//			{
//				ReleaseCapture();
//				SetCursorPos(mouseP0.x, mouseP0.y);
//				ShowCursor(1);
//				drag=0;
//			}
//			kp=0, timer=false;
//			if(!time_variance)
//				KillTimer(ghWnd, 0);
		}
		void appActivate()
		{
//			kp=kb['W']+kb['A']+kb['S']+kb['D']+kb[VK_UP]+kb[VK_DOWN]+kb[VK_LEFT]+kb[VK_RIGHT]
//				+kb[VK_ADD]+kb[VK_SUBTRACT]//numpad
//				+kb[VK_OEM_PLUS]+kb[VK_OEM_MINUS]
//				+kb[VK_RETURN]+kb[VK_BACK];
//			if(!time_variance&&kp&&!timer)
//				SetTimer(ghWnd, 0, 10, 0), timer=true;
		}

		void messagePaint(int x, int y, int w, int h)
		{
			setDimensions(x, y, w, h);//t1d_c, t2d, t2d_h, c3d
			if(!time_variance&&!timer)
				a_draw();
		}

		void function1(){derive_step_3D(DX, AR_Y, AR_Z, w, Xstep, Ystep, Zstep);}
	};

	void lpf_1d_real(Term &n)
	{
		auto &ndr_r=n.r;
		int N=n.r.size()<<1, T=11;
		double sum(0);
		std::vector<double> t0(T);
		for(int k=-T/2;k<T/2;++k)
			sum+=t0[(k+T)%T]=ndr_r[(k+N)%N];
		std::vector<double> t1(t0);
		for(int k=0;k<N-T/2;++k)
		{
			t0[k%T]=ndr_r[k];
			ndr_r[k]=sum/double(T);
			sum+=ndr_r[(k+T/2)%N]-t0[(k-T/2+T)%T];
		}
		for(int k=N-T/2;k<N;++k)
			ndr_r[k]=sum/double(T), sum+=t1[(k-(N-T/2)+N+T)%T]-t0[(k-T/2+T)%T];
	}
	void lpf_1d_complex(Term &n)
	{
		auto &ndr_r=n.r, &ndr_i=n.i;
		int N=n.r.size()<<1, T=11;
		std::complex<double> sum(0);
		std::vector<std::complex<double>> t0(T);
		for(int k=-T/2;k<T/2;++k)
			sum+=t0[(k+T)%T]=std::complex<double>(ndr_r[(k+N)%N], ndr_i[(k+N)%N]);
		std::vector<std::complex<double>> t1(t0);
		for(int k=0;k<N-T/2;++k)
		{
			t0[k%T]=std::complex<double>(ndr_r[k], ndr_i[k]);
			auto temp=sum/double(T);
			ndr_r[k]=temp.real(), ndr_i[k]=temp.imag();
			sum+=std::complex<double>(ndr_r[(k+T/2)%N], ndr_i[(k+T/2)%N])-t0[(k-T/2+T)%T];
		}
		for(int k=N-T/2;k<N;++k)
		{
			std::complex<double> temp=sum/double(T);
			ndr_r[k]-=temp.real(), ndr_i[k]-=temp.imag(), sum+=t1[(k-(N-T/2)+N+T)%T]-t0[(k-T/2+T)%T];
		}
	}
	void hpf_1d_real(Term &n)
	{
		auto &ndr_r=n.r;
		int N=n.r.size()<<1, T=11;
		double sum(0);
		std::vector<double> t0(T);
		for(int k=-T/2;k<T/2;++k)
			sum+=t0[(k+T)%T]=ndr_r[(k+N)%N];
		std::vector<double> t1(t0);
		for(int k=0;k<N-T/2;++k)
		{
			t0[k%T]=ndr_r[k];
			ndr_r[k]-=sum/double(T);
			sum+=ndr_r[(k+T/2)%N]-t0[(k-T/2+T)%T];
		}
		for(int k=N-T/2;k<N;++k)
			ndr_r[k]-=sum/double(T), sum+=t1[(k-(N-T/2)+N+T)%T]-t0[(k-T/2+T)%T];
	}
	void hpf_1d_complex(Term &n)
	{
		auto &ndr_r=n.r, &ndr_i=n.i;
		int N=n.r.size()<<1, T=11;
		std::complex<double> sum(0);
		std::vector<std::complex<double>> t0(T);
		for(int k=-int(T)/2;k<T/2;++k)
			sum+=t0[k%T]=std::complex<double>(ndr_r[k%N], ndr_i[k%N]);
		std::vector<std::complex<double>> t1(t0);
		for(int k=0;k<N-T/2;++k)
		{
			t0[k%T]=std::complex<double>(ndr_r[k], ndr_i[k]);
			auto temp=sum/double(T);
			ndr_r[k]-=temp.real(), ndr_i[k]-=temp.imag();
			sum+=std::complex<double>(ndr_r[(k+T/2)%N], ndr_i[(k+T/2)%N])-t0[(k-T/2)%T];
		}
		for(int k=N-T/2;k<N;++k)
		{
			auto temp=sum/double(T);
			ndr_r[k]-=temp.real(), ndr_i[k]-=temp.imag(), sum+=t1[(k-(N-T/2))%T]-t0[(k-T/2)%T];
		}
	}
	void lpf_2d(Term &n, int Xplaces, int Yplaces)
	{
		auto &ndr_r=n.r, &ndr_i=n.i;
	/*	int N=n.r.size(), T=11, T_2=T>>1;
		std::complex<double> sum;
		std::vector<decltype(sum)> t0(T*T);
		for(int ky=-T_2;ky<T_2;++ky)//initialize the kernel
		{
			for(int kx=-T_2;kx<T_2;++kx)
			{
				int idx=(ky+Yplaces)%Yplaces*Xplaces+(kx+Xplaces)%Xplaces;
				sum+=t0[(ky+T)%T*T+(kx+T)%T]=std::complex<double>(ndr_r[idx], ndr_i[idx]);
			}
		}
		for(int ky=0;ky<Yplaces;++ky)
		{
			int ry1=ky-T_2+Yplaces, ry2=ky+T_2+Yplaces;
			for(int kx=0;kx<Xplaces;++kx)
			{
				int rx2=kx+T_2+Xplaces;
			//	int rx1=kx-T_2+Xplaces, rx2=kx+T_2+Xplaces;
				for(int ky2=ry1, ty=0;ty<T;++ky2, ++ty)
				{
					int r_idx=ky2%Yplaces*Xplaces+rx2%Xplaces,
						k_idx=ty*T+T-1;
					sum+=
				}
			}
		}//*/
		int ndrSize=n.r.size()<<1;
		std::vector<double> r=ndr_r, i=ndr_i;
	//	std::vector<double> r(ndr_r, ndr_r+ndrSize), i(ndr_i, ndr_i+ndrSize);
		const double normal=1./9;
		for(int ky=0;ky<Yplaces;++ky)
		{
			for(int kx=0;kx<Xplaces;++kx)
			{
				int idx=ky*Xplaces+kx;
				int ky_1=ky-1+Yplaces, ky1=ky+1,
					kx_1=kx-1+Xplaces, kx1=kx+1;
				ky_1-=Yplaces&-(ky_1>=Yplaces), ky1-=Yplaces&-(ky1>=Yplaces);
				kx_1-=Xplaces&-(kx_1>=Xplaces), kx1-=Xplaces&-(kx1>=Xplaces);
					ky_1*=Xplaces;
				int ky_0=ky*Xplaces;
					ky1*=Xplaces;
				ndr_r[idx]=(
					r[ky_1+kx_1]+r[ky_1+kx]+r[ky_1+kx1]+
					r[ky_0+kx_1]+r[ky_0+kx]+r[ky_0+kx1]+
					r[ky1 +kx_1]+r[ky1 +kx]+r[ky1 +kx1])*normal;
				ndr_i[idx]=(
					i[ky_1+kx_1]+i[ky_1+kx]+i[ky_1+kx1]+
					i[ky_0+kx_1]+i[ky_0+kx]+i[ky_0+kx1]+
					i[ky1 +kx_1]+i[ky1 +kx]+i[ky1 +kx1])*normal;
				//ndr_r[ky*Xplaces+kx]=(
				//	r[(ky-1+Yplaces)%Yplaces*Xplaces+(kx-1+Xplaces)%Xplaces]+r[(ky-1+Yplaces)%Yplaces*Xplaces+(kx+Xplaces)%Xplaces]+r[(ky-1+Yplaces)%Yplaces*Xplaces+(kx+1+Xplaces)%Xplaces]+
				//	r[(ky  +Yplaces)%Yplaces*Xplaces+(kx-1+Xplaces)%Xplaces]+r[(ky  +Yplaces)%Yplaces*Xplaces+(kx+Xplaces)%Xplaces]+r[(ky  +Yplaces)%Yplaces*Xplaces+(kx+1+Xplaces)%Xplaces]+
				//	r[(ky+1+Yplaces)%Yplaces*Xplaces+(kx-1+Xplaces)%Xplaces]+r[(ky+1+Yplaces)%Yplaces*Xplaces+(kx+Xplaces)%Xplaces]+r[(ky+1+Yplaces)%Yplaces*Xplaces+(kx+1+Xplaces)%Xplaces])*0.11111111111111111111;
				//ndr_i[ky*Xplaces+kx]=(
				//	i[(ky-1+Yplaces)%Yplaces*Xplaces+(kx-1+Xplaces)%Xplaces]+i[(ky-1+Yplaces)%Yplaces*Xplaces+(kx+Xplaces)%Xplaces]+i[(ky-1+Yplaces)%Yplaces*Xplaces+(kx+1+Xplaces)%Xplaces]+
				//	i[(ky  +Yplaces)%Yplaces*Xplaces+(kx-1+Xplaces)%Xplaces]+i[(ky  +Yplaces)%Yplaces*Xplaces+(kx+Xplaces)%Xplaces]+i[(ky  +Yplaces)%Yplaces*Xplaces+(kx+1+Xplaces)%Xplaces]+
				//	i[(ky+1+Yplaces)%Yplaces*Xplaces+(kx-1+Xplaces)%Xplaces]+i[(ky+1+Yplaces)%Yplaces*Xplaces+(kx+Xplaces)%Xplaces]+i[(ky+1+Yplaces)%Yplaces*Xplaces+(kx+1+Xplaces)%Xplaces])*0.11111111111111111111;
			}
		}//*/
	/*	std::vector<decltype(sum)> t1(t0);
		for(int k=0;k<N-T/2;++k)
			t0[k%T]=std::complex<double>(ndr_r[k], ndr_i[k]), ndr[k]=sum/double(T), sum+=(decltype(sum))ndr[(k+T/2)%N]-t0[(k-T/2)%T];
		for(int k=N-T/2;k<N;++k)
		{
			auto temp=sum/double(T);
			ndr_r[k]=temp.real(), ndr_i[k]=temp.imag(), sum+=t1[(k-(N-T/2))%T]-t0[(k-T/2)%T];
		}//*/
	}
	void hpf_2d(Term &n, int Xplaces, int Yplaces)
	{
		auto &ndr_r=n.r, &ndr_i=n.i;
		int ndrSize=n.r.size()<<1;
		std::vector<double> r=ndr_r, i=ndr_i;
	//	std::vector<double> r(ndr_r, ndr_r+ndrSize), i(ndr_i, ndr_i+ndrSize);
		const double normal=1./4;
		for(int ky=0;ky<Yplaces;++ky)
		{
			for(int kx=0;kx<Xplaces;++kx)
			{
				int idx=ky*Xplaces+kx;
				int ky_1=ky-1+Yplaces, ky1=ky+1,
					kx_1=kx-1+Xplaces, kx1=kx+1;
				ky_1-=Yplaces&-(ky_1>=Yplaces), ky1-=Yplaces&-(ky1>=Yplaces);
				kx_1-=Xplaces&-(kx_1>=Xplaces), kx1-=Xplaces&-(kx1>=Xplaces);
					ky_1*=Xplaces;
				int ky_0=ky*Xplaces;
					ky1*=Xplaces;
				ndr_r[idx]=(	   -r[ky_1+kx]
					-r[ky_0+kx_1]+8*r[ky_0+kx]-r[ky_0+kx1]+
								   -r[ky1 +kx])*0.25;
				ndr_i[idx]=(	   -i[ky_1+kx]
					-i[ky_0+kx_1]+8*i[ky_0+kx]-i[ky_0+kx1]+
								   -i[ky1 +kx])*0.25;
				//ndr_r[ky*Xplaces+kx]=(										   -r[(ky-1+Yplaces)%Yplaces*Xplaces+(kx+Xplaces)%Xplaces]
				//	-r[(ky  +Yplaces)%Yplaces*Xplaces+(kx-1+Xplaces)%Xplaces]+8*r[(ky  +Yplaces)%Yplaces*Xplaces+(kx+Xplaces)%Xplaces]-r[(ky  +Yplaces)%Yplaces*Xplaces+(kx+1+Xplaces)%Xplaces]+
				//															   -r[(ky+1+Yplaces)%Yplaces*Xplaces+(kx+Xplaces)%Xplaces])*0.25;
				//ndr_i[ky*Xplaces+kx]=(										   -i[(ky-1+Yplaces)%Yplaces*Xplaces+(kx+Xplaces)%Xplaces]
				//	-i[(ky  +Yplaces)%Yplaces*Xplaces+(kx-1+Xplaces)%Xplaces]+8*i[(ky  +Yplaces)%Yplaces*Xplaces+(kx+Xplaces)%Xplaces]-i[(ky  +Yplaces)%Yplaces*Xplaces+(kx+1+Xplaces)%Xplaces]+
				//															   -i[(ky+1+Yplaces)%Yplaces*Xplaces+(kx+Xplaces)%Xplaces])*0.25;
			}
		}
	}

	class		Numeric_0D:public Mode
	{
		std::string text;
	//	TextBox tb;
	public:
		int toPrint;
		Solve_0D solver;
		int base;
		
		Labels_0D labels;
		static const int modes[], nmodes;

		bool drag;

		Numeric_0D():toPrint(false), drag(false), base(10){}
		void enter()
		{
//			tb.active=true;
//			tb.setDimensions(bpx, bpy, bw, bh);
//			if(kb[VK_LBUTTON])
//			{
//				if(tb.switchToWithMouseOn(oldMouse))
//					a_draw();
//			}
		}
		void exit()
		{
//			if(kb[VK_LBUTTON])
//			{
//			//	ReleaseCapture();
//				tb.inputLButtonUp();
//			}
//			tb.active=false;
//			tb.setDimensions(bpx+bw/2, bpy, bw/2, bh);
		}
		void pause()
		{
			if(time_variance&&!paused)
			{
				solver.pause();
			//	KillTimer(ghWnd, 0);
				paused=true;
			}
		}
		void resume()
		{
			if(!toSolve)//toCheck?
			{
				time_variance=0;

				labels.clear();

				text.clear();
			//	tb.text=(char*)realloc(tb.text, ((tb.textlen=0)+1)*sizeof(char));
//				const int aSize=1024;
//				char a[aSize];
				int alen=0;

				solver.synchronize();
				for(unsigned k=0;k<expr.size();++k)
				{
					auto &ex=expr[k];
					if(ex.rmode[0]==1)
					{
						time_variance|=ex.nITD;
						labels.fill(k);

						alen=0;
						print_value(ex.n[ex.resultTerm], 0, alen, ex.resultMathSet, base);
					//	print_value(ex.n[ex.resultTerm], 0, a, alen, ex.resultMathSet, base);
						text+=std::string(g_buf, g_buf+alen);
					//	append(tb.text, tb.textlen, a, alen);
						for(++k;k<expr.size();++k)
						{
							auto &ex2=expr[k];
							if(ex2.rmode[0]==1)
							{
								time_variance|=ex2.nITD;
								labels.fill(k);

								alen=snprintf(g_buf, g_buf_size, "\n");
							//	alen=sprintf_s(a, "\r\n");
								print_value(ex2.n[ex2.resultTerm], 0, alen, ex2.resultMathSet, base);
							//	print_value(ex2.n[ex2.resultTerm], 0, a, alen, ex2.resultMathSet, base);

								text+=std::string(g_buf, g_buf+alen);
							//	append(tb.text, tb.textlen, a, alen);
							}
						}
						break;
					}
				}
			}
			if(time_variance)
			{
			//	SetTimer(ghWnd, 0, 10, 0);
				solver.resume();
				paused=false;
			}
		}
		void appDeactivate()
		{
			drag=0;
//			if(drag)
//			{
//				drag=0;
//				ReleaseCapture();
//				tb.inputLButtonUp();
//			}
		}
		void appActivate(){}

		void setDimensions(int x, int y, int w, int h)
		{
			bpx=x, bpy=y, bw=w, bh=h, ready=true;
//			if(active)
//				tb.setDimensions(bpx, bpy, bw, bh);
//			else
//				tb.setDimensions(bpx+bw/2, bpy, bw/2, bh);
		}
//		void messagePaint(int x, int y, int w, int h)
//		{
//			setDimensions(x, y, w, h);
//			if(!time_variance)
//				a_draw();
//		}
		void messageTimer(){a_draw();}
		bool click_cursor(vec2 const &pos, vec2 const &cursorPos)
		{
			ivec4 rect=getCursorBox(cursorPos);
			return pos.x>=rect.x1&&pos.x<rect.x2&&pos.y>=rect.y1&&pos.y<rect.y2;
			//float fontH_2=fontH*0.5f;
			//return x>=cursorPos.x-fontH_2&&x<cursorPos.x+fontH_2&&y>=cursorPos.y-fontH_2&&y<cursorPos.y+fontH+fontH_2;
		}
		int inputTouchDown(int idx)
		{
			auto &ti=touchInfo[idx];
			auto &pos=ti.pos, &objPos=ti.objPos;
			if(cursor==selcur)
			{
				if(!touchInfo.size())
				{
					cursorAtPointer(pos, cursor);
					selcur=cursor;
					return DRAG_CURSOR;
				}
				if(touchInfo.size()==1)
				{
					cursorAtPointer(pos, selcur);
					return DRAG_SELCUR;
				}
			}
			else
			{
				if(copy_button.click(pos))//copy selection
				{
					JNIEnv *env=nullptr;
					int envStat=jvm->GetEnv((void**)&env, JNI_VERSION_1_6);
					bool toDetach=false;
					if(envStat==JNI_EDETACHED)
						envStat=jvm->AttachCurrentThread(&env, 0), toDetach=true;
					if(envStat==JNI_OK)
					{
						int selStart=minimum(cursor, selcur), selEnd=maximum(cursor, selcur);
						jstring str=env->NewString((const unsigned short*)&text[selStart], selEnd-selStart);

						jclass mainActivity=env->FindClass("com/example/grapher2/MainActivity");
						jmethodID textToClipboard=env->GetStaticMethodID(mainActivity, "textToClipboard", "(Ljava/lang/String;)V");
						env->CallStaticVoidMethod(mainActivity, textToClipboard, str);
					}
					if(toDetach)
						jvm->DetachCurrentThread();
					cursor=selcur=0;
					return TOUCH_MISS;
				}
				if(click_cursor(pos, cursorPos))
				{
#ifndef SELECTION_FOLLOW_CURSOR_EXACT
					objPos=pos-cursorPos;
#endif
					return DRAG_CURSOR;
				}
				if(click_cursor(pos, selcurPos))
				{
#ifndef SELECTION_FOLLOW_CURSOR_EXACT
					objPos=pos-selcurPos;
#endif
					return DRAG_SELCUR;
				}
				cursor=selcur=0;//miss
			}
			return TOUCH_MISS;
		}
		void inputTouchUp(int idx)
		{
		}
		void inputTouchMove(int idx)
		{
		}

		void print()
		{
			text.clear();
		//	tb.text=(char*)realloc(tb.text, ((tb.textlen=0)+1)*sizeof(char));
			const int aSize=1024;
			char a[aSize];
			if(!paused)
				solver.synchronize();
			for(unsigned k=0;k<expr.size();++k)
			{
				auto &ex=expr[k];
				if(ex.rmode[0]==1)
				{
					int alen=0;
					print_value(ex.n[ex.resultTerm], 0, alen, ex.resultMathSet, base);
				//	print_value(ex.n[ex.resultTerm], 0, a, alen, ex.resultMathSet, base);
					text+=std::string(g_buf, g_buf+alen);
				//	append(tb.text, tb.textlen, a, alen);
					for(++k;k<expr.size();++k)
					{
						auto &ex=expr[k];
						if(ex.rmode[0]==1)
						{
							alen=snprintf(g_buf, g_buf_size, "\n");
						//	alen=sprintf_s(a, "\r\n");
							print_value(ex.n[ex.resultTerm], 0, alen, ex.resultMathSet, base);
						//	print_value(ex.n[ex.resultTerm], 0, a, alen, ex.resultMathSet, base);
							text+=std::string(g_buf, g_buf+alen);
						//	append(tb.text, tb.textlen, a, alen);
						}
					}
					break;
				}
			}
		}
		void cursorAtPointer(vec2 const &p, int &cursor)
		{
			int x=p.x, y=p.y;
			int cursorL=int((y-tb_offset+textPos.y)/tb_fontH);
			cursor=0;
			int textlen=text.size();
			for(int k=0, line=0;k<textlen;++k)
			{
				if(text[k]=='\n')
				{
					if(line==cursorL)
						break;
					++line;
					cursor=k+1;
				}
			}
			int lstart=cursor;
			float textW=0;
			const float tb_px=tb_fontW*inv16;
			for(;text[cursor]!='\n'&&cursor<textlen;++cursor)
			{
				auto &c=text[cursor];
				if(c>=32&&c<127)
				{
					float charW=getCharWidth(c);
				//	float charW=Text::char_width[c]*tb_px;
					if(textPos.x+x<textW+charW*0.5f)
						break;
					textW+=charW;
				}
			}
		}
		void tb_draw()
		{
			int err=0;
			int X0=w>>1, Y0=h>>1;
			for(int k=0, kEnd=touchInfo.size();k<kEnd;++k)		//select text
			{
				auto &ti=touchInfo[k];
#ifdef SELECTION_FOLLOW_CURSOR_EXACT
				vec2 pos=ti.pos;
#else
				vec2 pos=ti.pos-ti.objPos;//xy=cursor+objPos, cursor=xy-objPos
#endif
				if(ti.region==DRAG_CURSOR)
					cursorAtPointer(pos, cursor);
				else if(ti.region==DRAG_SELCUR)
					cursorAtPointer(pos, selcur);
			}

			int xpos=inputBoxOn?w>>1:0;
			Font f;
			Font::change(0xFF000000, 0xFFFFFFFF, tb_fontH, tb_fontW);
			int nLines=0;
			float paragraph_height=0, paragraph_width=0;
			int length=text.size();
			float x=0;
			int bkMode=getBkMode(), bkColor=getBkColor();
			for(int k=0, k2=0, selStart=minimum(cursor, selcur), selEnd=maximum(cursor, selcur);k2<=length;++k2)
			{
				if(k2==selStart)
				{
					x+=print_array(xpos+x, tb_offset+paragraph_height, &text[0]+k, k2-k, 0);
					setBkColor(inputBoxOn?0xFF969696:0xFFFFFF96);
				//	setBkMode(OPAQUE), setBkColor(inputBoxOn?0xFF969696:0xFFFFFF96);
					k=k2;
					if(cursor<selcur)
						cursorPos.set(x, tb_offset+paragraph_height);
					else
						selcurPos.set(x, tb_offset+paragraph_height);
				}
				if(k2==selEnd)
				{
					x+=print_array(xpos+x, tb_offset+paragraph_height, &text[0]+k, k2-k, 0);
					setBkColor(bkColor);
					k=k2;
					if(cursor<selcur)
						selcurPos.set(x, tb_offset+paragraph_height);
					else
						cursorPos.set(x, tb_offset+paragraph_height);

				}
				if(k2==length|text[k2]=='\n')
				{
					print_array(xpos+x, tb_offset+paragraph_height, &text[0]+k, k2-k, 0);
					++nLines, paragraph_height=ceil(fontH*nLines);
					paragraph_width=maximum(paragraph_width, x);
					k=k2+(text[k2]=='\n'), x=0;
					k2=k;
				}
			}

			//set_2d_color(0xFFE0E0E0), draw_touch_pointers();

			if(!inputBoxOn&&cursor!=selcur)		//draw copy button
			{
				set_color(0xFF000000);
				float fontH_2=fontH*0.5f;
				GL2_2D::draw_line(cursorPos.x, cursorPos.y, cursorPos.x, cursorPos.y+fontH);		//draw cursors
				GL2_2D::draw_rectangle_hollow(getCursorBox(cursorPos));
				GL2_2D::draw_rectangle_hollow(getCursorBox(selcurPos));
				float yStart=minimum(cursorPos.y, selcurPos.y);
				float buttonH=(float)h*0.1f;
				if(yStart>h/10)
					copy_button.set(X0, yStart-buttonH, X0, buttonH);
				else
				{
					float yEnd=maximum(cursorPos.y, selcurPos.y);
					copy_button.set(X0, yEnd, X0, buttonH);
				}
				copy_button.draw_border();
				//draw_rectangle_hollow(copy_button.x1, copy_button.x2, copy_button.y1, copy_button.y2);
			}

			Font::change(0xFF000000, 0xFFFFFFFF, true, preferred_fontH);
			if(!inputBoxOn&&cursor!=selcur)		//print copy button
				copy_button.print_label();
	//		print_touch_pointers(2);
		}
		void tb_draw_corresponding(bool colored)
		{
			int err=0;
			err=glGetError();
			if(err)
				return;
		//	glUseProgram(Text::program), err=glGetError();		//print text
			int xpos=w>>1;
		//	int xpos=inputBoxOn?w>>1:0;
		//	Font f;
		//	Font::change(0xFF000000, 0xFFFFFFFF, true, tb_fontH, tb_fontW);
			int nLines=0, length=text.size();
			float paragraph_width=0, paragraph_height=0, x=0;
			int bkColor=getBkColor();
			unsigned e=0, eSize=expr.size();
			for(;e<eSize;++e)
			{
				auto &ex=expr[e];
				if(ex.rmode[0]==1)
				{
					nLines=ex.endLineNo, paragraph_height=fontH*ex.endLineNo;
					if(colored)
						setTextColor(ex.color);
					break;
				}
			}
			for(int k=0, k2=0, selStart=minimum(cursor, selcur), selEnd=maximum(cursor, selcur);k2<=length;++k2)
			{
				if(k2==selStart)
				{
					if(k<k2)
						x+=print_array(xpos+x, tb_offset+paragraph_height, &text[0]+k, k2-k, 0);
					setBkColor(0xFF969696);//selection color
					k=k2;
				}
				if(k2==selEnd)
				{
					if(k<k2)
						x+=print_array(xpos+x, tb_offset+paragraph_height, &text[0]+k, k2-k, 0);
					setBkColor(bkColor);
					k=k2;

				}
				if(k2==length|text[k2]=='\n')
				{
					if(k<k2)
						print_array(xpos+x, tb_offset+paragraph_height, &text[0]+k, k2-k, 0);
					++nLines, paragraph_height=ceil(fontH*nLines);
					paragraph_width=maximum(paragraph_width, x);
					k=k2+(text[k2]=='\n'), x=0;
					k2=k;
					if(k2==0||text[k2-1]!='\\')//find next n0d expression
					{
						++e;
						for(;e<eSize;++e)
						{
							auto &ex=expr[e];
							if(ex.rmode[0]==1)
							{
								nLines=ex.endLineNo, paragraph_height=fontH*ex.endLineNo;
								if(colored)
									setTextColor(ex.color);
								break;
							}
						}
					}
				}
			}

			pen_color=0xFFE0E0E0;
			draw_touch_pointers();

			if(!inputBoxOn&&cursor!=selcur)		//draw copy button
			{
				pen_color=0xFF000000;
				float fontH_2=fontH*0.5f;
				GL2_2D::draw_line(cursorPos.x, cursorPos.y, cursorPos.x, cursorPos.y+fontH);		//draw cursors
				GL2_2D::draw_rectangle_hollow(getCursorBox(cursorPos));
				GL2_2D::draw_rectangle_hollow(getCursorBox(selcurPos));
				float yStart=minimum(cursorPos.y, selcurPos.y);
				float buttonH=(float)h*0.1f;
				if(yStart>h/10)
					copy_button.set(X0, yStart-buttonH, X0, buttonH);
				else
				{
					float yEnd=maximum(cursorPos.y, selcurPos.y);
					copy_button.set(X0, yEnd, X0, buttonH);
				}
				copy_button.draw_border();
			}

			Font::change(0xFF000000, 0xFFFFFFFF, true, preferred_fontH);
			if(!inputBoxOn&&cursor!=selcur)//print copy button
				copy_button.print_label();
			print_touch_pointers(2);
		}
		void draw()
		{
			if(toSolve)
			{
				auto old_time_variance=time_variance;
				time_variance=false;
			//	std::fill(nExpr.begin(), nExpr.end(), 0);
				labels.clear();
			//	print();
				text.clear();
			//	tb.text=(char*)realloc(tb.text, ((tb.textlen=0)+1)*sizeof(char));
//				const int aSize=1024;
//				char a[aSize];
				int alen;
				if(!paused)
					solver.synchronize();
				for(unsigned k=0;k<expr.size();++k)
				{
					auto &ex=expr[k];
				//	++nExpr[ex.rmode[0]];
					if(ex.rmode[0]==1)
					{
						time_variance|=ex.nITD;
						solver.full(ex);
						labels.fill(k);
						alen=0;
						print_value(ex.n[ex.resultTerm], 0, alen, ex.resultMathSet, base);
					//	print_value(ex.n[ex.resultTerm], 0, a, alen, ex.resultMathSet, base);
						text+=std::string(g_buf, g_buf+alen);
					//	append(tb.text, tb.textlen, a, alen);
						for(++k;k<expr.size();++k)
						{
							auto &ex=expr[k];
						//	++nExpr[ex.rmode[0]];
							if(ex.rmode[0]==1)
							{
								time_variance|=ex.nITD;
								solver.full(ex);
								labels.fill(k);
								alen=snprintf(g_buf, g_buf_size, "\n");
							//	alen=sprintf_s(a, "\r\n");
								print_value(ex.n[ex.resultTerm], 0, alen, ex.resultMathSet, base);
							//	print_value(ex.n[ex.resultTerm], 0, a, alen, ex.resultMathSet, base);
								text+=std::string(g_buf, g_buf+alen);
							//	append(tb.text, tb.textlen, a, alen);
							}
						}
						break;
					}
				}//*/
//				if(time_variance)
//				{
//					if(!paused)
//						SetTimer(ghWnd, 0, 10, 0);
//				}
//				else if(old_time_variance)
//					KillTimer(ghWnd, 0);
				toSolve=false;
			//	if(tb.cursor>tb.textlen)
					cursor=selcur=0;
//					tb.cursor=tb.selcur=0;
			}
			else if(time_variance)
			{
			//	print();
				text.clear();
			//	tb.text=(char*)realloc(tb.text, ((tb.textlen=0)+1)*sizeof(char));
				const int aSize=1024;
				char a[aSize];
				int alen;
				if(!paused)
					solver.synchronize();
				for(unsigned k=0;k<expr.size();++k)
				{
					auto &ex=expr[k];
					if(ex.rmode[0]==1)
					{
						if(ex.nITD)
							solver.full(ex);
						print_value(ex.n[ex.resultTerm], 0, alen=0, ex.resultMathSet, base);
					//	print_value(ex.n[ex.resultTerm], 0, a, alen=0, ex.resultMathSet, base);
						text+=std::string(g_buf, g_buf+alen);
					//	append(tb.text, tb.textlen, a, alen);
						for(++k;k<expr.size();++k)
						{
							auto &ex=expr[k];
							if(ex.rmode[0]==1)
							{
								if(ex.nITD)
									solver.full(ex);
								alen=snprintf(g_buf, g_buf_size, "\n");
							//	alen=sprintf_s(a, aSize, "\r\n");
								print_value(ex.n[ex.resultTerm], 0, alen, ex.resultMathSet, base);
							//	print_value(ex.n[ex.resultTerm], 0, a, alen, ex.resultMathSet, base);
								text+=std::string(g_buf, g_buf+alen);
							//	append(tb.text, tb.textlen, a, alen);
							}
						}
						break;
					}
				}//*/
				if(cursor>=text.size()||selcur>=text.size())
					cursor=selcur=0;
				//if(tb.cursor>tb.textlen)
				//	tb.cursor=tb.selcur=0;
			}
			else if(toPrint)
				print();
			toPrint=false;
			if(active)
			//	tb.setDimensions(bpx, bpy, bw, bh);
				tb_draw();
			else
			{
				int err=0;
				Font font;
				Font::change(0xFF777777, 0xFFFFFFFF, tb_fontH, tb_fontW);
				tb_draw_corresponding(labels.colored);
//				if(labels.colored)
//					tb.draw_corresponding_color(modes, nmodes, &::itb);
//				else
//					tb.draw_corresponding(modes, nmodes, &::itb);
				font.revert();
				setTextColor(0xFF000000);
			}
			{
				int Ys=0;
				auto colorCondition=nExpr[1]>1;
				int textColor;
				if(colorCondition)
					textColor=getTextColor();
				for(int kl=0, klEnd=labels.Clabels.size();kl<klEnd;++kl)
				{
					auto &label=labels.Clabels[kl];
					if(colorCondition)
						setTextColor(expr[label.exNo].color);
					::print(w-const_label_offset_X, Ys, "%s=%g", label.label.c_str(), label.isTime?solver.T:label.value);
					Ys+=fontH;
				}
				if(colorCondition)
					setTextColor(textColor);
			}
			switch(base)
			{
			case 2:
				::print(w-const_label_offset_X, h-16, "binary");
				break;
			case 8:
				::print(w-const_label_offset_X, h-16, "octal");
				break;
			case 16:
				::print(w-const_label_offset_X, h-16, "hexadecimal");
				break;
			}
		}
		void i_draw();
		void a_draw();
	} n0d;
	const int		Numeric_0D::modes[]={1}, Numeric_0D::nmodes=sizeof(modes)>>2;
	void			Numeric_0D::i_draw()
	{
		if(modes::ready)
		{
			draw();
			//if(showLastModeOnIdle)
			//	std::copy(rgb, rgb+w*h, modeRGB);
		}
		else
		{
		//	std::copy(modeRGB, modeRGB+w*h, rgb);
		}
		if(showBenchmark)
			Performance(0, h>>1);//
		//	Performance(0, h-32);//
	}
	void			Numeric_0D::a_draw()
	{
		set_color(0xFFFFFFFF), GL2_2D::draw_rectangle(bpx-1, bpy-1, bw+1, bh+1);

		draw();

		if(contextHelp)
		{
			const char *help[]=
			{
				"1: binary",
				"2: octal",
				"3/0: decimal",
				"4: hexadecimal",
				"Esc: back to text editor"
			};
			print_contextHelp(help, sizeof(help)>>2, 154);
		}
		if(showBenchmark)
			Performance(0, h);//
	//	BitBlt(ghDC, 0, 0, w, h, ghMemDC, 0, 0, SRCCOPY);
	}
	class		Implicit_1D:public Mode
	{
	public:
		int shiftOnly;//0 full, 1 partial, 2 no change yet
		int Xoffset, exprRemoved;

		int Xplaces;
		double VX, DX, step;
		int prec;
		Solve_1D_Implicit solver;
		
	//	_2D _2d;
		bool clearScreen;
		Labels_1D labels;
		static const int modes[], nmodes;
	//	HPEN__ *hPen;
	//	HBRUSH__ *hBrush;

		bool timer, drag, m_bypass;
		int kp;

		Implicit_1D():
			VX(0), DX(20),
			timer(false), drag(false), m_bypass(false), kp(0),
			shiftOnly(2), Xoffset(0), exprRemoved(0),
			clearScreen(false)
		{
		//	hPen=CreatePen(PS_SOLID, 1, _2dCheckColor), hBrush=CreateSolidBrush(_2dCheckColor);
		}
	//	~Implicit_1D(){DeleteObject(hPen), DeleteObject(hBrush);}

		void enter()
		{
//			if(kb[VK_LBUTTON])
//			{
//				ShowCursor(0);
//				GetCursorPos(&mouseP0);
//				SetCursorPos(centerP.x, centerP.y);
//			//	SetCapture(ghWnd);
//				drag=1;
//			}
//			kp=kb[VK_UP]+kb[VK_DOWN]+kb[VK_LEFT]+kb[VK_RIGHT]
//				+kb[VK_ADD]+kb[VK_SUBTRACT]//numpad
//				+kb[VK_OEM_PLUS]+kb[VK_OEM_MINUS]
//				+kb[VK_RETURN]+kb[VK_BACK];
//			if(!time_variance&&kp&&!timer)
//				SetTimer(ghWnd, 0, 10, 0), timer=true;
		}
		void exit()
		{
			drag=0;
//			if(drag)
//			{
//				drag=0;
//			//	ReleaseCapture();
//				SetCursorPos(mouseP0.x, mouseP0.y);
//				ShowCursor(1);
//			}
//			if(!time_variance&&timer)
//				KillTimer(ghWnd, 0), timer=false;
		}
		void pause()
		{
			if(time_variance&&!paused)
			{
				solver.pause();
			//	KillTimer(ghWnd, 0);
				paused=true;
			}
		}
		void resume()
		{
			if(time_variance)
			{
			//	SetTimer(ghWnd, 0, 10, 0);
				solver.resume();
				paused=false;
			}
		}
		void appDeactivate()
		{
			drag=0;
//			if(drag)
//			{
//				ReleaseCapture();
//				SetCursorPos(mouseP0.x, mouseP0.y);
//				ShowCursor(1);
//				drag=0;
//			}
//			kp=0, timer=false;
//			if(!time_variance)
//				KillTimer(ghWnd, 0);
		}
		void appActivate()
		{
//			kp=kb[VK_UP]+kb[VK_DOWN]+kb[VK_LEFT]+kb[VK_RIGHT]
//				+kb[VK_ADD]+kb[VK_SUBTRACT]//numpad
//				+kb[VK_OEM_PLUS]+kb[VK_OEM_MINUS]
//				+kb[VK_RETURN]+kb[VK_BACK];
//			if(!time_variance&&kp&&!timer)
//				SetTimer(ghWnd, 0, 10, 0), timer=true;
		}
		
		void setDimensions(int x, int y, int w, int h)
		{
			bpx=x, bpy=y, bw=w, bh=h;
			function1();
			toSolve=true, shiftOnly=0;
			ready=true;
		}
//		void messagePaint(int x, int y, int w, int h)
//		{
//			setDimensions(x, y, w, h);
//			toSolve=true, shiftOnly=0;
//			if(!time_variance&&!timer)
//				a_draw();
//		}
		void messageTimer()
		{
//			if(_2d_drag_graph_not_window)
//			{
//				if(kb[VK_LEFT		]){	VX+=DX/w*10, toSolve=true; if(shiftOnly)shiftOnly=1, Xoffset+=10;}
//				if(kb[VK_RIGHT		]){	VX-=DX/w*10, toSolve=true; if(shiftOnly)shiftOnly=1, Xoffset-=10;}
//				if(kb[VK_UP			]){	itb.tpy+=10;	if(itb.tpy>itb.th-h	)itb.tpy=itb.th-h;}//VY-=DX/w*10;
//				if(kb[VK_DOWN		]){	itb.tpy-=10;	if(itb.tpy<0		)itb.tpy=0;}//VY+=DX/w*10;
//			}
//			else
//			{
//				if(kb[VK_LEFT		]){	VX-=DX*10/w, toSolve=true; if(shiftOnly)shiftOnly=1, Xoffset-=10;}
//				if(kb[VK_RIGHT		]){	VX+=DX*10/w, toSolve=true; if(shiftOnly)shiftOnly=1, Xoffset+=10;}
//				if(kb[VK_UP			]){	itb.tpy-=10;	if(itb.tpy<0		)itb.tpy=0;}//VY+=DX*10/w;
//				if(kb[VK_DOWN		]){	itb.tpy+=10;	if(itb.tpy>itb.th-h	)itb.tpy=itb.th-h;}//VY-=DX*10/w;
//			}
//			if(kb[VK_ADD		]||kb[VK_OEM_PLUS	]||kb[VK_RETURN	])	DX/=1.05, function1(), toSolve=true, shiftOnly=0;
//			if(kb[VK_SUBTRACT	]||kb[VK_OEM_MINUS	]||kb[VK_BACK	])	DX*=1.05, function1(), toSolve=true, shiftOnly=0;
			a_draw();
//			if(!time_variance&&!kp)
//				KillTimer(ghWnd, 0), timer=false;
		}
		int inputTouchDown(int idx)
		{
			auto &ti=touchInfo[idx];
			if(ti.pos.y<h/10)
			{
				DX=20, function1();
				VX=0;
				toSolve=true, shiftOnly=0;
				return TOUCH_MISS;
			}
			return _2D_DRAG;
		}
		void inputTouchUp(int idx)
		{
		}
		void inputTouchMove(int idx)
		{
			auto &ti=touchInfo[idx];
			if(touchInfo.size()==1)//shift
			{
				if(ti.region==_2D_DRAG)
				{
					double Xr=DX/w;
					int dx=-ti.delta.x;
					VX+=dx*Xr;
				//	VY+=ti.delta.y*Xr/AR_Y;
					toSolve=true;
					if(shiftOnly)
						shiftOnly=1, Xoffset+=dx;
				}
			}
			else if(touchInfo.size()==2)//zoom
			{
				if(idx==1)//wait till both pointers are updated
				{
					auto &ti2=touchInfo[!idx];
					if(ti.region==_2D_DRAG&&ti2.region==_2D_DRAG)
					{
						auto &a=ti.pos, &b=ti2.pos, a0=a-ti.delta, b0=b-ti2.delta;
						float d=(a-b).magnitude(), d0=(a0-b0).magnitude();
						float diff=d0-d;
						double Xr=DX/w;
						DX+=diff*Xr;
						vec2 ab=0.5f*(a+b), ab0=0.5f*(a0+b0), dab=ab0-ab;
						VX+=dab.x*Xr;
					//	VY-=dab.y*Xr/AR_Y;
						function1();
						toSolve=true, shiftOnly=0;
					}
				}
			}
		}

		void function1(){derive_step(DX, w, step, prec);}
		void draw()
		{
			double Xr=w/DX;
			if(!clearScreen)
			{
				//hPen=(HPEN__*)SelectObject(ghMemDC, hPen), hBrush=(HBRUSH__*)SelectObject(ghMemDC, hBrush);
				set_color(_2dCheckColor);
				{
					double Xstart=VX-DX/2, Xstepx2=2*step;
					for(double x=floor(Xstart/Xstepx2)*Xstepx2, xEnd=ceil((VX+DX/2)/step)*step;x<xEnd;x+=Xstepx2)
					{
						double x1=(x-Xstart)*Xr, x2=(x+step-Xstart)*Xr;
						GL2_2D::draw_rectangle(x1, x2, 0, h);
//						Rectangle(ghMemDC, int(x1)-(x1<0), 0, int(x2)-(x2<0), h);
					}
				}
				//hPen=(HPEN__*)SelectObject(ghMemDC, hPen), hBrush=(HBRUSH__*)SelectObject(ghMemDC, hBrush);
			}
			if(toSolve)
			{
				if(shiftOnly==1&&abs(Xoffset)<Xplaces)
				{
					if(Xoffset)
					{
						solver.partial_bounds(VX, DX, Xoffset);
						solver.synchronize();
						for(int ke=0, keEnd=expr.size();ke<keEnd;++ke)
						{
							auto &ex=expr[ke];
							if(ex.rmode[0]==10)
							{
								if(ex.nITD)
									solver.full(ex);
								else
									solver.partial(ex);
							}
							//	(solver.*(ex.nITD?&Solve_1D_Implicit::full:&Solve_1D_Implicit::partial))(ex);
						}
					}
				}
				else
				{
					auto old_time_variance=time_variance;
					time_variance=false;
					labels.clear();
					solver.full_resize(VX, DX, Xplaces=w);
					if(!paused)
						solver.synchronize();
					for(unsigned e=0;e<expr.size();++e)
					{
						auto &ex=expr[e];
						if(ex.rmode[0]==10)
						{
							time_variance|=ex.nITD;
							labels.fill(e);
							solver.full(ex);
						}
					}
//					if(time_variance)
//					{
//						if(!paused)
//							SetTimer(ghWnd, 0, 10, 0);
//					}
//					else if(old_time_variance&&!timer)
//						KillTimer(ghWnd, 0);
				}
				toSolve=false, shiftOnly=2, Xoffset=0;
				exprRemoved=false;
			}
			else if(exprRemoved)
			{
				auto old_time_variance=time_variance;
				time_variance=false;
				labels.clear();
				for(unsigned e=0;e<expr.size();++e)
				{
					auto &ex=expr[e];
					if(ex.rmode[0]==10)
					{
						time_variance|=ex.nITD;
						labels.fill(e);
					}
				}
				//if(old_time_variance&&!time_variance&&!timer)
				//	KillTimer(ghWnd, 0);
				exprRemoved=false;
			}
			else if(time_variance)
			{
				if(!paused)
					solver.synchronize();
				for(int ke=0, keEnd=expr.size();ke<keEnd;++ke)
				{
					auto &ex=expr[ke];
					if(ex.rmode[0]==10&&ex.nITD)
						solver.full(ex);
				}
			}
			int colorCondition=nExpr[10]>1;
			if(!clearScreen)
			{
				int H=(h>>1)+1;
			//	int H=h>>1;
			//	int H=h*3>>2;
			//	int H=!drag&&((short*)&oldMouse)[1]>h>>1?h>>2:h*3>>2;
				int bkMode=setBkMode(TRANSPARENT);
				for(double x=floor(solver.Xstart/step)*step, xEnd=ceil((VX+DX/2)/step)*step, step_2=step/2;x<xEnd;x+=step)
				{
					int linelen=x>-step_2&&x<step_2?
							snprintf(g_buf, g_buf_size, "0")
						:	snprintf(g_buf, g_buf_size, "%g", x);
					double X=(x-solver.Xstart)*Xr;
					print_array((int)floor(X)+2, H, g_buf, linelen, 0);
				}
				{
					int textColor;
					if(colorCondition)
						textColor=getTextColor();
					int Ys=H+(H>h-46?-34:18)-2, Ys0=Ys;
					for(int kl=0, klEnd=labels.Xlabels.size();kl<klEnd;++kl)
					{
						auto &label=labels.Xlabels[kl];
						if(colorCondition)
							setTextColor(expr[label.exNo].color);
						print_array(w-arrow_label_offset_X, Ys, label.label.c_str(), label.label.size(), 0), Ys+=fontH;
					}
					Ys=Ys0>h/3?0:h-16*labels.Clabels.size();
					for(int kl=0, klEnd=labels.Clabels.size();kl<klEnd;++kl)
					{
						auto &label=labels.Clabels[kl];
						if(colorCondition)
							setTextColor(expr[label.exNo].color);
						print(w-const_label_offset_X, Ys, "%s=%g", label.label.c_str(), label.isTime?solver.T:label.value);
						Ys+=fontH;
					}
					if(colorCondition)
						setTextColor(textColor);
				}
				setBkMode(bkMode);
			}
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_DST_COLOR);
		//	glBlendFunc(GL_SRC_COLOR, GL_DST_COLOR);
		//	draw_line(0, 0, w, h);//
			for(int e=0, eSize=expr.size();e<eSize;++e)//draw the solutions
			{
				auto &ex=expr[e];
				if(ex.rmode[0]==10)
				{
					set_color(ex.resultLogicType==1?0xFF7F7F7F:0xFF000000);
					auto &ndr=ex.n[ex.resultTerm].r;
					int y_eStart=ex.lineNo*fontH, y_eEnd=(ex.endLineNo+1)*fontH;
				//	int y_eStart=ex.lineNo*fontH-itb.tpy, y_eEnd=(ex.endLineNo+1)*fontH;
					y_eStart=clamp_positive(y_eStart);
				//	auto color=(unsigned char*)&ex.color;
					if(colorCondition)
						set_color(ex.color);

					int aXplaces=w+((ex.resultLogicType>=2)<<1);
					for(int x=0;x<w;++x)
					{
						if(ndr.size()!=aXplaces)
							return;
						if(ndr[x]>0.5)
							GL2_2D::draw_line(x, y_eStart, x, h);
						//	draw_line(x, y_eStart, x, y_eEnd);
					}
//					if(ex.resultLogicType==2)//= equations aa		several color
//					{
//						for(int x=0;x<w;++x)
//							if(ndr[x]>0.5)
//								draw_line(x, y_eStart, x, y_eEnd);
//					}
//					else if(ex.resultLogicType==3)//!=		aa inverted		several color
//					{
//					}
//					else//&& ## || < <= > >= logic/inequality		several color
//					{
//					}
				}
			}
			glDisable(GL_BLEND);
		}
		void i_draw();
		void a_draw();
	} ti1d;
	const int		Implicit_1D::modes[]={10}, Implicit_1D::nmodes=sizeof(modes)>>2;
	void			Implicit_1D::i_draw()
	{
		if(modes::ready)
		{
			draw();
//			if(showLastModeOnIdle)
//				std::copy(rgb, rgb+w*h, modeRGB);
		}
		else
		{
		//	std::copy(modeRGB, modeRGB+w*h, rgb);
		}
		if(showBenchmark)
			Performance(0, h>>1);//
	}
	void			Implicit_1D::a_draw()
	{
	//	Rectangle(ghMemDC, bpx-1, bpy-1, bw+1, bh+1);

		draw();
		
		if(contextHelp)
		{
			const char *help[]=
			{
				"left/right/drag: move",
				"+/-/enter/backspace/wheel: zoom",
				"E: reset scale",
				"R: reset scale & view",
				"C: toggle clear screen",
				"Esc: back to text editor"
			};
			print_contextHelp(help, sizeof(help)>>2, 218);
		}
		if(showBenchmark&&!clearScreen)
			Performance(0, h-32);//
	//	BitBlt(ghDC, 0, 0, w, h, ghMemDC, 0, 0, SRCCOPY);
	}

	bool		difference_t1d_t1dc();
	class		Transverse_1D:public _2D_Mode
	{
	public:
		int Xoffset, exprRemoved;

		int Xplaces;
		Solve_1D solver;
		std::list<int> operations;
		bool drawImag;
		
		int contourOn, contourOnly;
		//contours[e]=contour, contour[y level (y/step)]=points, *points.begin()=x pos
		std::map<unsigned, std::unordered_map<int, std::list<double>>> contours;

	//	_2D _2d;
		bool clearScreen;
		Labels_1D labels;
		static const int modes[], nmodes;
	//	HPEN__ *hPen;
	//	HBRUSH__ *hBrush;

		friend bool difference_t1d_t1dc();

		Transverse_1D():
			solver(true), _2D_Mode(solver),
			Xoffset(0), exprRemoved(0),
			drawImag(false),
			contourOnly(false),
			clearScreen(false)
		{
		//	hPen=CreatePen(PS_SOLID, 1, _2dCheckColor), hBrush=CreateSolidBrush(_2dCheckColor);
		}
	//	~Transverse_1D(){DeleteObject(hPen), DeleteObject(hBrush);}
		
		void doContour(unsigned e, double Xs, double Ystart, double Yend, double Ystep, unsigned nySteps)
		{
			auto &ndr=expr[e].n[expr[e].resultTerm].r;
			auto &contour=contours[e];
			double YLstart=std::ceil((Ystart-10*Xs)/Ystep)*Ystep, YLend=std::ceil((Yend+25*Xs)/Ystep)*Ystep;
			for(unsigned v=0, vEnd=ndr.size()-1;v<vEnd;++v)
			{
				auto &V1=ndr[v], &V2=ndr[v+1];
				if(V1==V1&&V2==V2)
				{
					double Y1, Y2;
					if(V1<V2)
						Y1=V1, Y2=V2;
					else
						Y1=V2, Y2=V1;
					for(int y=int(std::floor(maximum(Y1, YLstart)/Ystep)), yEnd=int(std::floor(minimum(Y2, YLend)/Ystep));y<=yEnd;++y)
					{
						double YL=Ystep*y;
						if(Y1<=YL&&YL<Y2)
							contour[y].push_back(solver.Xstart+Xs*(v+(YL-V1)/(V2-V1)));
					}
				}
			}
		}
		void differentiate						(Term &n)
		{
			auto &ndr_r=n.r;
			double step=DX/Xplaces, _1_step=1/step;
			for(int k=0;k<Xplaces-1;++k)
				ndr_r[k]=(ndr_r[k+1]-ndr_r[k])*_1_step;
			ndr_r[n.r.size()-1]/=-step;
		}
		void integrate							(Term &n)
		{
		/*	double step=DX/Xplaces;
			auto &V0=ndr[0].r;
			double sum=V0!=V0?0:V0;
			ndr[0].r=sum*step;
			for(unsigned k=1;k<ndr.size();++k)
			{
				auto &V=ndr[k].r;
				sum+=V!=V?0:V;
				ndr[k].r=sum*step;
			}//*/
			auto &ndr_r=n.r;
			double step=DX/Xplaces;
			double sum=0;
			for(int k=0;k<Xplaces;++k)
			{
				sum+=ndr_r[k];
				ndr_r[k]=sum*step;
			}
		}
	//	fftw_complex *fft_in, *fft_out;
	//	fftw_plan fft_p, ifft_p;
		unsigned fft_N;
		double fft_sqrt_N;
		void discreteFourrierTransform			(Term &n)
		{
			if(n.i.size()!=n.r.size())
				n.i.resize(n.r.size());
		//	fft_1d_forward(n.r, n.i, Xplaces, fft_N, fft_sqrt_N, fft_in, fft_out, fft_p, ifft_p);
		}
		void inverseDiscreteFourrierTransform	(Term &n)
		{
			if(n.i.size()!=n.r.size())
				n.i.resize(n.r.size());
		//	fft_1d_inverse(n.r, n.i, Xplaces, fft_N, fft_sqrt_N, fft_in, fft_out, fft_p, ifft_p);
		}
		void lowPassFilter						(Term &n){lpf_1d_real(n);}
		void highPassFilter						(Term &n){hpf_1d_real(n);}

		void messageTimer()
		{
//			if(_2d_drag_graph_not_window)
//			{
//				if(kb[VK_LEFT		]){	VX+=10*DX/w, toSolve=true; if(shiftOnly)shiftOnly=1, Xoffset+=10;}
//				if(kb[VK_RIGHT		]){	VX-=10*DX/w, toSolve=true; if(shiftOnly)shiftOnly=1, Xoffset-=10;}
//				if(kb[VK_UP			])	VY-=10*DX/(w*AR_Y);
//				if(kb[VK_DOWN		])	VY+=10*DX/(w*AR_Y);
//			}
//			else
//			{
//				if(kb[VK_LEFT		]){	VX-=10*DX/w, toSolve=true; if(shiftOnly)shiftOnly=1, Xoffset-=10;}
//				if(kb[VK_RIGHT		]){	VX+=10*DX/w, toSolve=true; if(shiftOnly)shiftOnly=1, Xoffset+=10;}
//				if(kb[VK_UP			])	VY+=10*DX/(w*AR_Y);
//				if(kb[VK_DOWN		])	VY-=10*DX/(w*AR_Y);
//			}
//			if(kb[VK_ADD		]||kb[VK_RETURN	]||kb[VK_OEM_PLUS	])
//			{
//				if(kb['X'])
//					DX/=1.05, AR_Y/=1.05;
//				else if(kb['Y'])
//					AR_Y*=1.05;
//				else
//					DX/=1.05;
//				function1(), toSolve=true, shiftOnly=0;
//			}
//			if(kb[VK_SUBTRACT	]||kb[VK_BACK	]||kb[VK_OEM_MINUS	])
//			{
//				if(kb['X'])
//					DX*=1.05, AR_Y*=1.05;
//				else if(kb['Y'])
//					AR_Y/=1.05;
//				else
//					DX*=1.05;
//				function1(), toSolve=true, shiftOnly=0;
//			}
			a_draw();
//			if(!time_variance&&!kp)
//				KillTimer(ghWnd, 0), timer=false;
		}
		int inputTouchDown(int idx)
		{
			auto &ti=touchInfo[idx];
			if(ti.pos.y<h/10)
			{
				DX=20, AR_Y=1, function1();
				VX=VY=0;
				toSolve=true, shiftOnly=0;
				return TOUCH_MISS;
			}
			return _2D_DRAG;
		}
		void inputTouchUp(int idx)
		{
		}
		void inputTouchMove(int idx)
		{
//			TouchInfo *ti1=0, *ti2=0;
//			for(int k=0, kEnd=touchInfo.size();k<kEnd;++k)
//			{
//			}
			auto &ti=touchInfo[idx];
			if(touchInfo.size()==1)//shift
			{
				if(ti.region==_2D_DRAG)
				{
					double Xr=DX/w;
					int dx=-ti.delta.x;
					VX+=dx*Xr;
					VY+=ti.delta.y*Xr/AR_Y;
//					int dx=ti.pos.x-new_pos.x;
//					VX+=dx*Xr;
//					VY+=(new_pos.y-ti.pos.y)*Xr/AR_Y;
					toSolve=true;
					if(shiftOnly)
						shiftOnly=1, Xoffset+=dx;
				}
			}
			else if(touchInfo.size()==2)//zoom
			{
				if(idx==1)//wait till both pointers are updated
				{
					auto &ti2=touchInfo[!idx];
					if(ti.region==_2D_DRAG&&ti2.region==_2D_DRAG)
					{
						auto &a=ti.pos, &b=ti2.pos, a0=a-ti.delta, b0=b-ti2.delta;
						float d=(a-b).magnitude(), d0=(a0-b0).magnitude();
						float diff=d0-d;
						double Xr=DX/w;
						DX+=diff*Xr;
						vec2 ab=0.5f*(a+b), ab0=0.5f*(a0+b0), dab=ab0-ab;
						VX+=dab.x*Xr, VY-=dab.y*Xr/AR_Y;
						function1();
						toSolve=true, shiftOnly=0;

//						auto &old_pos=ti.pos, &pivot=ti2.pos;
//						vec2 old_d=old_pos-pivot, new_d=new_pos-pivot;
//						double zoomx=old_d.x/new_d.x, zoomy=old_d.y/new_d.y;
//						double zoom=0.5*(zoomx+zoomy);//sqrt(zoomx*zoomx+zoomy*zoomy);
//						double Xr=DX/w;
//						double dx=(pivot.x-(w>>1))*Xr, dy=((h>>1)-pivot.y)*Xr/AR_Y;
//						DX*=zoom;
//					//	AR_Y*=zoomy;
//						VX+=dx-dx*zoom;
//						VY+=dy-dy*zoom;
//					//	function1();
//						toSolve=true, shiftOnly=0;
					}
				}
			}
		}

		void draw_component(Expression &ex, std::vector<double> &ndr, int yoffset, double Yend, double Yr)
		{
			int nthreads0=nthreads;
			if(ndr.size()!=Xplaces)//
			{
			//	debug_check();//
				return;//
			}
			GL2_2D::curve_begin();
			for(int v=0, vEnd=Xplaces-1;v<vEnd;++v)
			{
				if(toSolve||ndr.size()!=Xplaces)//
				{
				//	debug_check();//
					return;//
				}
				GL2_2D::curve_point(bpx+v, Yr*(Yend-ndr[v])+yoffset);
				GL2_2D::continuous=!ex.discontinuities[v];
			}
			GL2_2D::draw_curve();
		}
		void draw()
		{
			double DY=DX*h/(w*AR_Y);
			if(DY<=0)
				DY=1;
			_2dMode_DrawCheckboard(_2dCheckColor, VX, VY, DX, DY, Xstep, Ystep);
			if(toSolve)
			{
				if(!operations.size()&&shiftOnly==1&&abs(Xoffset)<Xplaces)
				{
					if(Xoffset)
					{
						solver.partial_bounds(VX, DX, Xoffset);
						solver.synchronize();
					//	for(auto &ex:expr)
						for(int ke=0, keEnd=expr.size();ke<keEnd;++ke)
						{
							auto &ex=expr[ke];
							if(ex.rmode[0]==2)
							{
								if(ex.nITD)
									solver.full(ex);
								else
									solver.partial(ex);
							}
							//	(solver.*(ex.nITD?&Solve_1D::full:&Solve_1D::partial))(ex);
						}
					}
				}
				else
				{
					auto old_time_variance=time_variance;
					time_variance=false;
					labels.clear();
					solver.full_resize(VX, DX, Xplaces=w);
					if(!paused)
						solver.synchronize();
					for(unsigned e=0;e<expr.size();++e)
					{
						auto &ex=expr[e];
						if(ex.rmode[0]==2)
						{
							time_variance|=ex.nITD;
							labels.fill(e);
							solver.full(ex);
							for(auto it=operations.begin();it!=operations.end();++it)
							{
								auto &operation=*it;
								switch(operation)
								{
								case 1:differentiate					(ex.n[ex.resultTerm]);break;
								case 2:integrate						(ex.n[ex.resultTerm]);break;
								case 3:discreteFourrierTransform		(ex.n[ex.resultTerm]);break;
								case 4:inverseDiscreteFourrierTransform	(ex.n[ex.resultTerm]);break;
								case 5:lowPassFilter					(ex.n[ex.resultTerm]);break;
								case 6:highPassFilter					(ex.n[ex.resultTerm]);break;
								}
							}
						}
					}
//					if(time_variance)
//					{
//						if(!paused)
//							SetTimer(ghWnd, 0, 10, 0);
//					}
//					else if(old_time_variance&&!timer)
//						KillTimer(ghWnd, 0);
				}
				if(contourOn)
				{
					contours.clear();
					double Xs=DX/Xplaces;//, DY=DX*h/(w*AR_Y);
					for(unsigned e=0;e<expr.size();++e)
						if(expr[e].rmode[0]==2)
							doContour(e, Xs, VY-DY/2, VY+DY/2, Ystep, 10);
				}
				toSolve=false, shiftOnly=2, Xoffset=0;
			}
			else if(exprRemoved)
			{
				auto old_time_variance=time_variance;
				time_variance=false;
				labels.clear();
				for(unsigned e=0;e<expr.size();++e)
				{
					auto &ex=expr[e];
					if(ex.rmode[0]==2)
					{
						time_variance|=ex.nITD;
						labels.fill(e);
					}
				}
//				if(old_time_variance&&!time_variance&&!timer)
//					KillTimer(ghWnd, 0);
				exprRemoved=false;
			}
			else if(time_variance)
			{
				if(!paused)
					solver.synchronize();
				for(int ke=0, keEnd=expr.size();ke<keEnd;++ke)
				{
					auto &ex=expr[ke];
					if(ex.rmode[0]==2&&ex.nITD)
					{
						solver.full(ex);
						for(auto it=operations.begin();it!=operations.end();++it)
						{
							auto &operation=*it;
							switch(operation)
							{
							case 1:differentiate					(ex.n[ex.resultTerm]);break;
							case 2:integrate						(ex.n[ex.resultTerm]);break;
							case 3:discreteFourrierTransform		(ex.n[ex.resultTerm]);break;
							case 4:inverseDiscreteFourrierTransform	(ex.n[ex.resultTerm]);break;
							case 5:lowPassFilter					(ex.n[ex.resultTerm]);break;
							case 6:highPassFilter					(ex.n[ex.resultTerm]);break;
							}
						}
					}
				}
				if(contourOn)
				{
					double Xs=DX/Xplaces;//, DY=DX*h/(w*AR_Y);
					for(unsigned e=0;e<expr.size();++e)
					{
						auto &ex=expr[e];
						if(ex.rmode[0]==2&&ex.nITD)
							doContour(e, Xs, VY-DY/2, VY+DY/2, Ystep, 10);
					}
				}
			}
			else if(nExpr[3]&&difference_t1d_t1dc())
			{
				for(unsigned e=0;e<expr.size();++e)
				{
					auto &ex=expr[e];
					if(ex.rmode[0]==2&&!ex.nITD)
					{
						solver.full(ex);
						for(auto it=operations.begin();it!=operations.end();++it)
						{
							auto &operation=*it;
							switch(operation)
							{
							case 1:differentiate					(ex.n[ex.resultTerm]);break;
							case 2:integrate						(ex.n[ex.resultTerm]);break;
							case 3:discreteFourrierTransform		(ex.n[ex.resultTerm]);break;
							case 4:inverseDiscreteFourrierTransform	(ex.n[ex.resultTerm]);break;
							case 5:lowPassFilter					(ex.n[ex.resultTerm]);break;
							case 6:highPassFilter					(ex.n[ex.resultTerm]);break;
							}
						}
					}
				}
			}
			int colorCondition=nExpr[2]>1;
			if(!clearScreen)
			{
				int bkMode=getBkMode();
				int H=0, V=0, VT=0;
				_2dMode_NumberAxes(VX, VY, DX, DY, Xstep, Ystep, prec, H, V, VT);
				GL2_2D::draw_line(0, H, w, H), GL2_2D::draw_line(V, 0, V, h);
				{
					int textColor;
					if(colorCondition)
						textColor=getTextColor();
					int Ys=H+(H>h-46?-34:18), Ys0=Ys;
					for(int kl=0, klEnd=labels.Xlabels.size();kl<klEnd;++kl)
					{
						auto &label=labels.Xlabels[kl];
						if(colorCondition)
							setTextColor(expr[label.exNo].color);
						print_array(w-arrow_label_offset_X, Ys, label.label.c_str(), label.label.size(), 0), Ys+=fontH;
					}
					Ys=Ys0>h/3?0:h-16*labels.Clabels.size();
					for(int kl=0, klEnd=labels.Clabels.size();kl<klEnd;++kl)
					{
						auto &label=labels.Clabels[kl];
						setTextColor(0xFF000000|(expr[label.exNo].color&-colorCondition));
						print(w-const_label_offset_X, Ys, "%s=%g", label.label.c_str(), label.isTime?solver.T:label.value);
						Ys+=fontH;
					}
					if(colorCondition)
						setTextColor(textColor);
				}
				setBkMode(bkMode);
			}
			if(toSolve)//
			{
				debug_check();//
				return;//
			}
		//	HPEN hGPen=0;
			int colored=0;
			double Yend=VY+DY/2, Yr=h/DY;
			if(!contourOnly)//draw the curves
			{
				if(operations.size())
				{
					for(int ke=0, keEnd=expr.size();ke<keEnd;++ke)
					{
						auto &ex=expr[ke];
						if(ex.rmode[0]==2)
						{
							pen_color=0xFF000000|(ex.color&-colorCondition);
							auto &n=ex.n[ex.resultTerm];
							draw_component(ex, n.r, 0, Yend, Yr);
							if(drawImag)
							{
								draw_component(ex, n.i, 0, Yend, Yr);
								draw_component(ex, n.i, 1, Yend, Yr);
							}
						}
					}
				}
				else//curve with discontinuities
				{
					for(int ke=0, keEnd=expr.size();ke<keEnd;++ke)
					{
						auto &ex=expr[ke];
						if(ex.rmode[0]==2)
						{
							pen_color=0xFF000000|(ex.color&-colorCondition);
							draw_component(ex, ex.n[ex.resultTerm].r, 0, Yend, Yr);
						}
					}
				}
			}
			if(contourOn)//draw contour
			{
				int mrX1=-60, mrX2=60, mrY1=-60, mrY2=60;
				double Xstart=VX-DX/2;
				double Ystart=VY-DY/2, Yend=VY+DY/2;
				double Xs=DX/Xplaces, YLstart=std::ceil((Ystart-10*Xs)/Ystep)*Ystep;
				for(unsigned e=0;e<expr.size();++e)
				{
					auto &ex=expr[e];
					if(ex.rmode[0]==2)
					{
						if(colorCondition)
							set_color(ex.color);
						auto &contour=contours[e];
						for(auto cit=contour.begin();cit!=contour.end();++cit)
						{
							auto &L=*cit;
							double Y=L.first*Ystep;
							int y=(int)floor((Yend-Y)*h/DY);
							for(auto xit=L.second.begin();xit!=L.second.end();++xit)
							{
								auto &X=*xit;
								int x=(int)floor((X-Xstart)*w/DX);
								GL2_2D::draw_line(x-5, y, x+5, y);
								GL2_2D::draw_line(x, y-5, x, y+5);
								if(active)
									print(x, y, "%g, %g", X, Y);
							}
						}
					}
				}
				{
					if(active)
					{
						if(drag)
						{
							int w_2=w>>1, h_2=h>>1;
							set_color(0xFF000000);
							GL2_2D::draw_rectangle_hollow(w_2+mrX1, w_2+mrX2, h_2+mrY1, h_2+mrY2);
						}
					}
				}
			}
			{
				int k=0, Y=h-operations.size()*16;
				for(auto it=operations.begin();it!=operations.end();++it)
				{
					auto &operation=*it;
					char const *a=0;
					switch(operation)
					{
					case 1:a="%d: Differentiate";break;
					case 2:a="%d: Integrate";break;
					case 3:a="%d: DFT";break;
					case 4:a="%d: IDFT";break;
					case 5:a="%d: LPF";break;
					case 6:a="%d: HPF";break;
					}
					print(w-const_label_offset_X, Y, a, k);
					++k, Y+=16;
				}
			}
		}
		void i_draw();
		void a_draw();
	} t1d;
	const int		Transverse_1D::modes[]={2}, Transverse_1D::nmodes=sizeof(modes)>>2;
	void			Transverse_1D::i_draw()
	{
		if(modes::ready)
		{
			draw();
//			if(showLastModeOnIdle)
//				std::copy(rgb, rgb+w*h, modeRGB);
		}
		else
		{
//			std::copy(modeRGB, modeRGB+w*h, rgb);
		}
		if(showBenchmark)
			Performance(0, h>>1);//
		//	Performance(0, h-32);//
	}
	void			Transverse_1D::a_draw()
	{
	//	Rectangle(ghMemDC, bpx-1, bpy-1, bw+1, bh+1);

		draw();
		
		if(contextHelp)
		{
			const char *help[]=
			{
				"arrows/drag: move",
				"+/-/enter/backspace/wheel: zoom",
				"X/Y +/-/enter/backspace/wheel: scale x/y",
				"E: reset scale",
				"R: reset scale & view",
				"C: toggle clear screen",
				"1: differentiate",
				"2: integrate",
				"3: DFT",
				"4: Inverse DFT",
				"5: LPF",
				"6: HPF",
				"0: reset operations",
				"`: contour",
				"Esc: back to text editor"
			};
			print_contextHelp(help, sizeof(help)>>2, 265);
		}
	/*	{
			const char *helpMsg[]={"1: binary", "2: octal", "3/0: decimal", "4: hexadecimal"};
			const int LOL_1=sizeof(helpMsg)/sizeof(void*);
			short X0=w>>1, Y0=h>>1, dx=107, dy=68;
			short xs=X0-dx, ys=Y0-dy;
			Rectangle(ghMemDC, xs-1, ys-1, X0+dx, Y0+dy);
			GUIPrint(ghMemDC, xs, ys, "1: binary");
			GUIPrint(ghMemDC, xs, ys+18, "2: octal");
			GUIPrint(ghMemDC, xs, ys+18*2, "3/0: decimal");
			GUIPrint(ghMemDC, xs, ys+18*3, "4: hexadecimal");
		}//*/
		if(showBenchmark&&!clearScreen)
			Performance(0, h);//
		//	Performance(0, h-32);//
	//	BitBlt(ghDC, 0, 0, w, h, ghMemDC, 0, 0, SRCCOPY);
	}
	namespace contour
	{
		struct Double_x_i
		{
			double x, i;
			Double_x_i(double x, double i):x(x), i(i){}
		};
		struct Double_x_r
		{
			double x, r;
			Double_x_r(double x, double r):x(x), r(r){}
		};
	}
	class		Transverse_1D_C:public _3D_Mode
	{
	public:
		int Xoffset, exprRemoved;
		int X0, Y0;

		int Xplaces;
		int KXplaces;
		Solve_1D solver;
		std::list<int> operations;

		bool contourOn, contourOnly;
		//Rcontours[e]=Rcontour, Rcontour[r level (r/rStep)]=points, *points.begin()=(x,i)
		std::map<unsigned, std::unordered_map<int, std::list<contour::Double_x_i>>> Rcontours;
		//Icontours[e]=Icontour, Icontour[i level (i/iStep)]=points, *points.begin()=(x,r)
		std::map<unsigned, std::unordered_map<int, std::list<contour::Double_x_r>>> Icontours;
		
		_3D _3d;
		bool clearScreen, kb_VK_F6_msg;
		Labels_1D labels;
		int gridColor;
		static const int modes[], nmodes;
		
		friend bool difference_t1d_t1dc();

		Transverse_1D_C():
			solver(true), _3D_Mode(solver),
			_3d(4, 4, 4, 225*G2::_pi/180, 324.7356103172454*G2::_pi/180, 1),
			gridColor(_3dGridColor),
			Xoffset(0), exprRemoved(0),
		//	fft_N(0),
			contourOnly(false),
			clearScreen(false), kb_VK_F6_msg(false)
		{}
		~Transverse_1D_C()
		{
//			if(fft_N)
//			{
//				fftw_destroy_plan(fft_p), fftw_destroy_plan(ifft_p);
//				fftw_free(fft_in), fftw_free(fft_out);
//			}
		}
		
		void doContour(unsigned e, double Xs, double Rstart, double Rend, double Rstep, unsigned nrSteps, double Istart, double Iend, double Istep, unsigned niSteps)
		{
			using namespace contour;
			auto &ex=expr[e];
			auto &n=ex.n[ex.resultTerm];
			auto &ndr_r=n.r, &ndr_i=n.i;
			auto &Rcontour=Rcontours[e];
			auto &Icontour=Icontours[e];
			Rcontour.clear(), Icontour.clear();
			double RLstart=std::ceil((Rstart-10*Xs)/Rstep)*Rstep, RLend=std::ceil((Rend+25*Xs)/Rstep)*Rstep;
			double ILstart=std::ceil((Istart-10*Xs)/Istep)*Istep, ILend=std::ceil((Iend+25*Xs)/Istep)*Istep;
			for(unsigned v=0, vEnd=Xplaces-1;v<vEnd;++v)
			{
				CompRef V1(ndr_r[v], ndr_i[v]), V2(ndr_r[v+1], ndr_i[v+1]);
				if(V1.r==V1.r&&V2.r==V2.r)
				{
					double R1, R2;
					if(V1.r<V2.r)
						R1=V1.r, R2=V2.r;
					else
						R1=V2.r, R2=V1.r;
					for(int r=int(std::floor(maximum(R1, RLstart)/Rstep)), rEnd=int(std::floor(minimum(R2, RLend)/Rstep));r<=rEnd;++r)
					{
						double RL=Rstep*r;
						if(R1<=RL&&RL<R2)
						{
							double M=(RL-V1.r)/(V2.r-V1.r);
							Rcontour[r].push_back(Double_x_i(solver.Xstart+Xs*(v+M), V1.i+(V2.i-V1.i)*M));
						}
					}
				}
				if(V1.i==V1.i&&V2.i==V2.i)
				{
					double I1, I2;
					if(V1.i<V2.i)
						I1=V1.i, I2=V2.i;
					else
						I1=V2.i, I2=V1.i;
					for(int i=int(std::floor(maximum(I1, ILstart)/Istep)), iEnd=int(std::floor(minimum(I2, ILend)/Istep));i<=iEnd;++i)
					{
						double IL=Istep*i;
						if(I1<=IL&&IL<I2)
						{
							double M=(IL-V1.i)/(V2.i-V1.i);
							Icontour[i].push_back(Double_x_r(solver.Xstart+Xs*(v+M), V1.r+(V2.r-V1.r)*M));
						}
					}
				}
			}
		}
		void differentiate						(Term &n)
		{
			auto &ndr_r=n.r, &ndr_i=n.i;
			double step=DX/Xplaces;
			for(int k=0;k<Xplaces-1;++k)
			{
				CompRef V0(ndr_r[k], ndr_i[k]), V1(ndr_r[k+1], ndr_i[k+1]);
				V0=(V1-V0)/step;
			}
			ndr_r[n.r.size()-1]/=-step, ndr_i[n.i.size()-1]/=-step;
		}
		void integrate							(Term &n)
		{
			auto &ndr_r=n.r, &ndr_i=n.i;
			double step=DX/Xplaces;
			std::complex<double> sum;
			for(int k=0;k<Xplaces;++k)
			{
				sum+=std::complex<double>(ndr_r[k], ndr_i[k]);
				auto temp=sum*step;
				ndr_r[k]=temp.real(), ndr_i[k]=temp.imag();
			}
		}
	//	fftw_complex *fft_in, *fft_out;
	//	fftw_plan fft_p, ifft_p;
	//	unsigned fft_N;
	//	double fft_sqrt_N;
		void discreteFourrierTransform			(Term &n)
		{
			if(n.i.size()!=n.r.size())
				n.i.resize(n.r.size());
		//	fft_1d_forward(n.r, n.i, Xplaces, fft_N, fft_sqrt_N, fft_in, fft_out, fft_p, ifft_p);//
		}
		void inverseDiscreteFourrierTransform	(Term &n)
		{
			if(n.i.size()!=n.r.size())
				n.i.resize(n.r.size());
		//	fft_1d_inverse(n.r, n.i, Xplaces, fft_N, fft_sqrt_N, fft_in, fft_out, fft_p, ifft_p);//
		}
		void lowPassFilter						(Term &n){lpf_1d_complex(n);}
		void highPassFilter						(Term &n){hpf_1d_complex(n);}

		void setDimensions(int x, int y, int w, int h)
		{
			bpx=x, bpy=y, bw=w, bh=h, X0=bpx+bw/2, Y0=bpy+bh/2;
		//	_3d.setDimensions(x, y, w, h);
			double old_Ystep=Ystep, old_Zstep=Zstep;
			function1();
			if(!toSolve&&contourOn&&(old_Ystep!=Ystep||old_Zstep!=Zstep))
			{
				Rcontours.clear(), Icontours.clear();
				double Xs=DX/Xplaces, DY=DX/AR_Y, DZ=DX/AR_Z;
				for(unsigned e=0;e<expr.size();++e)
					if(expr[e].rmode[0]==2||expr[e].rmode[0]==3)
						doContour(e, Xs, VY-DY/2, VY+DY/2, Ystep, 10, VZ-DZ/2, VZ+DZ/2, Zstep, 10);
			}
			ready=true;
		}
		void messageTimer()
		{
//				 if(kb[VK_SHIFT	]){	 if(kb['W'])	_3d.moveForwardFast();
//									 if(kb['A'])	_3d.moveLeftFast();
//									 if(kb['S'])	_3d.moveBackFast();
//									 if(kb['D'])	_3d.moveRightFast();
//									 if(kb['T'])	_3d.camz+=10*_3d.dcam;
//									 if(kb['G'])	_3d.camz-=10*_3d.dcam;}
//			else				  {	 if(kb['W'])	_3d.moveForward();
//									 if(kb['A'])	_3d.moveLeft();
//									 if(kb['S'])	_3d.moveBack();
//									 if(kb['D'])	_3d.moveRight();
//									 if(kb['T'])	_3d.camz+=_3d.dcam;
//									 if(kb['G'])	_3d.camz-=_3d.dcam;}
//			double dVD=KXplaces*DX/Xplaces;
//			if(kb['X'])
//			{
//				if(kb['Y'])
//				{
//					if(kb['Z'])
//					{
//					}
//					else		//xy
//					{
//						if(kb[VK_UP])		VY+=dVD, _3d.camy+=_3d_shift_move_cam*dVD;
//						if(kb[VK_DOWN])		VY-=dVD, _3d.camy-=_3d_shift_move_cam*dVD;
//						if(kb[VK_RIGHT]){	VX+=dVD, _3d.camx+=_3d_shift_move_cam*dVD, toSolve=true;	if(shiftOnly)shiftOnly=1, Xoffset+=KXplaces;}
//						if(kb[VK_LEFT]){	VX-=dVD, _3d.camx-=_3d_shift_move_cam*dVD, toSolve=true;	if(shiftOnly)shiftOnly=1, Xoffset-=KXplaces;}
//					}
//				}
//				else
//				{
//					if(kb['Z'])	//xz
//					{
//						if(kb[VK_UP])		VZ+=dVD, _3d.camz+=_3d_shift_move_cam*dVD;
//						if(kb[VK_DOWN])		VZ-=dVD, _3d.camz-=_3d_shift_move_cam*dVD;
//						if(kb[VK_RIGHT]){	VX+=dVD, _3d.camx+=_3d_shift_move_cam*dVD, toSolve=true;	if(shiftOnly)shiftOnly=1, Xoffset+=KXplaces;}
//						if(kb[VK_LEFT]){	VX-=dVD, _3d.camx-=_3d_shift_move_cam*dVD, toSolve=true;	if(shiftOnly)shiftOnly=1, Xoffset-=KXplaces;}
//					}
//					else		//x
//					{
//						if(kb[VK_UP]||kb[VK_RIGHT]){	VX+=dVD, _3d.camx+=_3d_shift_move_cam*dVD, toSolve=true;	if(shiftOnly)shiftOnly=1, Xoffset+=KXplaces;}
//						if(kb[VK_DOWN]||kb[VK_LEFT]){	VX-=dVD, _3d.camx-=_3d_shift_move_cam*dVD, toSolve=true;	if(shiftOnly)shiftOnly=1, Xoffset-=KXplaces;}
//					}
//				}
//			}
//			else
//			{
//				if(kb['Y'])
//				{
//					if(kb['Z'])	//yz
//					{
//						if(kb[VK_UP])		VZ+=dVD, _3d.camz+=_3d_shift_move_cam*dVD;
//						if(kb[VK_DOWN])		VZ-=dVD, _3d.camz-=_3d_shift_move_cam*dVD;
//						if(kb[VK_RIGHT])	VY+=dVD, _3d.camy+=_3d_shift_move_cam*dVD;
//						if(kb[VK_LEFT])		VY-=dVD, _3d.camy-=_3d_shift_move_cam*dVD;
//					}
//					else		//y
//					{
//						if(kb[VK_UP]||kb[VK_RIGHT])		VY+=dVD, _3d.camy+=_3d_shift_move_cam*dVD;
//						if(kb[VK_DOWN]||kb[VK_LEFT])	VY-=dVD, _3d.camy-=_3d_shift_move_cam*dVD;
//					}
//				}
//				else
//				{
//					if(kb['Z'])	//z
//					{
//						if(kb[VK_UP]||kb[VK_RIGHT])		VZ+=dVD, _3d.camx+=_3d_shift_move_cam*dVD;
//						if(kb[VK_DOWN]||kb[VK_LEFT])	VZ-=dVD, _3d.camx-=_3d_shift_move_cam*dVD;
//					}
//					else
//					{
//						if(kb[VK_UP])	_3d.rotateUp();
//						if(kb[VK_DOWN])	_3d.rotateDown();
//						if(kb[VK_RIGHT])_3d.rotateRight();
//						if(kb[VK_LEFT])	_3d.rotateLeft();
//					}
//				}
//			}
//			if(kb[VK_ADD]||kb[VK_RETURN]||kb[VK_OEM_PLUS])
//			{
//				if(kb[VK_MENU])//alt+
//				{
//					if(_3d_zoom_move_cam)//zoom in
//						DX/=1.1, _3d.camx=VX+(_3d.camx-VX)/1.1, _3d.camy=VY+(_3d.camy-VY)/1.1, _3d.camz=VZ+(_3d.camz-VZ)/1.1, _3d.dcam/=1.1;
//					else//zoom out
//						DX*=1.1;
//					function1();
//					toSolve=true, shiftOnly=0;
//				}
//				else if(kb['X'])//x+ stretch (zoom in (stretch), compress y, z)
//				{
//					if(_3d_stretch_move_cam)
//						_3d.camx=VX+(_3d.camx-VX)/1.1;
//					else
//						_3d.camx/=1.1;
//					_3d.camy/=1.1, _3d.camz/=1.1, _3d.dcam/=1.1;
//					DX/=1.1, AR_Y/=1.1, AR_Z/=1.1, function1();
//					VY/=1.1, VZ/=1.1;
//					toSolve=true, shiftOnly=0;
//				}
//				else if(kb['Y'])//y+ stretch
//				{
//					if(_3d_stretch_move_cam)
//						_3d.camy+=VY*0.1;
//					VY*=1.1;//move cube
//					AR_Y*=1.1, function1();
//					if(contourOn)
//						toSolve=true, shiftOnly=0;
//				}
//				else if(kb['Z'])//z+ stretch
//				{
//					if(_3d_stretch_move_cam)
//						_3d.camy+=VZ*0.1;
//					VZ*=1.1;//move cube
//					AR_Z*=1.1, function1();
//					if(contourOn)
//						toSolve=true, shiftOnly=0;
//				}
//				else if(!kb[VK_CONTROL])
//					_3d.zoomIn();
//			}
//			if(kb[VK_SUBTRACT]||kb[VK_BACK]||kb[VK_OEM_MINUS])
//			{
//				if(kb[VK_MENU])//alt-
//					DX/=1.1, function1(), toSolve=true, shiftOnly=0;
//				else if(kb['X'])//x- compress (zoom out (compress), stretch y z)
//				{
//					if(_3d_stretch_move_cam)
//						_3d.camx=VX+(_3d.camx-VX)*1.1;
//					else
//						_3d.camx*=1.1;
//					_3d.camy*=1.1, _3d.camz*=1.1, _3d.dcam*=1.1;
//					DX*=1.1, AR_Y*=1.1, AR_Z*=1.1, function1();
//					VY*=1.1, VZ*=1.1;
//					toSolve=true, shiftOnly=0;
//				}
//				else if(kb['Y'])//y- compress
//				{
//					if(_3d_stretch_move_cam)
//						_3d.camy-=VY/11.;
//					VY/=1.1;//move cube
//					AR_Y/=1.1, function1();
//					if(contourOn)
//						toSolve=true, shiftOnly=0;
//				}
//				else if(kb['Z'])//z- compress
//				{
//					if(_3d_stretch_move_cam)
//						_3d.camy-=VZ/11.;
//					VZ/=1.1;//move cube
//					AR_Z/=1.1, function1();
//					if(contourOn)
//						toSolve=true, shiftOnly=0;
//				}
//				else if(!kb[VK_CONTROL])
//					_3d.zoomOut();
//			}
			a_draw();
//			if(!time_variance&&!kp)
//				KillTimer(ghWnd, 0), timer=false;
		}
		int inputTouchDown(int idx)
		{
			auto &ti=touchInfo[idx];
			auto &pos=ti.pos;
			int X0=w>>1, Y0=h>>1;
			if(pos.y>Y0)
			{
				if(pos.x<X0)
					return BOTTOM_LEFT_MOVE;
				return BOTTOM_RIGHT_TURN;
			}
			if(reset_button.click(pos))
			{
				_3d.cam.dcam=.04;
				DX=20, AR_Y=1, AR_Z=1, function1();
				VX=VY=VZ=0;
				_3d.teleport_degrees(4, 4, 4, 225, 324.7356103172454, 1);
				return BUTTON_RESET;
			}
			return TOUCH_MISS;
		}
		void inputTouchUp(int idx)
		{
		}
		void inputTouchMove(int idx)
		{
			auto &ti=touchInfo[idx];
				if(ti.region==BOTTOM_RIGHT_TURN)
					_3d.cam.turnBy(ti.delta.x*5/w, ti.delta.y*5/w);
				//_3d.turn_by(float(new_pos.x-ti.pos.x)*5/w, float(new_pos.y-ti.pos.y)*5/w);
		}

		void draw()
		{
			int X0=w>>1, Y0=h>>1;
			for(int k=0, kEnd=touchInfo.size();k<kEnd;++k)//move camera
			{
				auto &ti=touchInfo[k];
				if(ti.region==BOTTOM_LEFT_MOVE)
				{
					int X00=w>>2, Y00=h*3>>2;
					float dx=float(ti.pos.x-X00)/w, dy=-float(ti.pos.y-Y00)/w;
					//float dx=float(ti.pos.x-X00)*0.1f/w, dy=float(ti.pos.y-Y00)*0.1f/w;

					_3d.cam.move(dx, dy);
					break;
				}
			}
			if(toSolve)
			{
				if(!operations.size()&&shiftOnly==1&&abs(Xoffset)<Xplaces)
				{
					if(Xoffset)
					{
						solver.partial_bounds(VX, DX, Xoffset);
						if(time_variance)
							solver.synchronize();
						for(int ke=0, keEnd=expr.size();ke<keEnd;++ke)
						{
							auto &ex=expr[ke];
							if(ex.rmode[0]==2||ex.rmode[0]==3)
							{
								if(ex.nITD)
									solver.full(ex);
								else
									solver.partial(ex);
							}
							//	(solver.*(ex.nITD?&Solve_1D::full:&Solve_1D::partial))(ex);
						}
					}
				}
				else
				{
					int old_time_variance=time_variance;
					time_variance=0;
					labels.clear();
					solver.full_resize(VX, DX, Xplaces=1000), KXplaces=Xplaces/100?Xplaces/100:1;
					if(!paused)
						solver.synchronize();
					for(unsigned e=0;e<expr.size();++e)
					{
						auto &ex=expr[e];
						if(ex.rmode[0]==2||ex.rmode[0]==3)
						{
							time_variance|=ex.nITD;
							labels.fill(e);
							solver.full(ex);
							for(auto it=operations.begin();it!=operations.end();++it)
							{
								auto &operation=*it;
								switch(operation)
								{
								case 1:differentiate					(ex.n[ex.resultTerm]);break;
								case 2:integrate						(ex.n[ex.resultTerm]);break;
								case 3:discreteFourrierTransform		(ex.n[ex.resultTerm]);break;
								case 4:inverseDiscreteFourrierTransform	(ex.n[ex.resultTerm]);break;
								case 5:lowPassFilter					(ex.n[ex.resultTerm]);break;
								case 6:highPassFilter					(ex.n[ex.resultTerm]);break;
								}
							}
						}
					}
//					if(time_variance)
//					{
//						if(!paused)
//							SetTimer(ghWnd, 0, 10, 0);
//					}
//					else if(old_time_variance&&!timer)
//						KillTimer(ghWnd, 0);
				}
				if(contourOn)
				{
					Rcontours.clear(), Icontours.clear();
					double Xs=DX/Xplaces, DY=DX/AR_Y, DZ=DX/AR_Z;
					for(unsigned e=0;e<expr.size();++e)
						if(expr[e].rmode[0]==2||expr[e].rmode[0]==3)
							doContour(e, Xs, VY-DY/2, VY+DY/2, Ystep, 10, VZ-DZ/2, VZ+DZ/2, Zstep, 10);
				}
				toSolve=false, shiftOnly=2, Xoffset=0;
				exprRemoved=false;
			}
			else if(exprRemoved)
			{
				auto old_time_variance=time_variance;
				time_variance=false;
				labels.clear();
				for(unsigned e=0;e<expr.size();++e)
				{
					auto &ex=expr[e];
					if(ex.rmode[0]==2||ex.rmode[0]==3)
					{
						time_variance|=ex.nITD;
						labels.fill(e);
					}
				}
//				if(old_time_variance&&!time_variance&&!timer)
//					KillTimer(ghWnd, 0);
				exprRemoved=false;
			}
			else if(time_variance)
			{
				if(!paused)
					solver.synchronize();
				for(int ke=0, keEnd=expr.size();ke<keEnd;++ke)
				{
					auto &ex=expr[ke];
					if((ex.rmode[0]==2||ex.rmode[0]==3)&&ex.nITD)
					{
						solver.full(ex);
						for(auto it=operations.begin();it!=operations.end();++it)
						{
							auto &operation=*it;
							switch(operation)
							{
							case 1:differentiate					(ex.n[ex.resultTerm]);break;
							case 2:integrate						(ex.n[ex.resultTerm]);break;
							case 3:discreteFourrierTransform		(ex.n[ex.resultTerm]);break;
							case 4:inverseDiscreteFourrierTransform	(ex.n[ex.resultTerm]);break;
							case 5:lowPassFilter					(ex.n[ex.resultTerm]);break;
							case 6:highPassFilter					(ex.n[ex.resultTerm]);break;
							}
						}
					}
				}
				if(contourOn)
				{
					double Xs=DX/Xplaces, DY=DX/AR_Y, DZ=DX/AR_Z;//contour
					for(unsigned e=0;e<expr.size();++e)
					{
						auto &ex=expr[e];
						if((ex.rmode[0]==2||ex.rmode[0]==3)&&ex.nITD)
							doContour(e, Xs, VY-DY/2, VY+DY/2, Ystep, 10, VZ-DZ/2, VZ+DZ/2, Zstep, 10);
					}
				}
			}
			else if(difference_t1d_t1dc())
			{
				for(unsigned e=0;e<expr.size();++e)
				{
					auto &ex=expr[e];
					if(ex.rmode[0]==2&&!ex.nITD)
					{
						solver.full(ex);
						for(auto it=operations.begin();it!=operations.end();++it)
						{
							auto &operation=*it;
							switch(operation)
							{
							case 1:differentiate					(ex.n[ex.resultTerm]);break;
							case 2:integrate						(ex.n[ex.resultTerm]);break;
							case 3:discreteFourrierTransform		(ex.n[ex.resultTerm]);break;
							case 4:inverseDiscreteFourrierTransform	(ex.n[ex.resultTerm]);break;
							case 5:lowPassFilter					(ex.n[ex.resultTerm]);break;
							case 6:highPassFilter					(ex.n[ex.resultTerm]);break;
							}
						}
					}
				}
			}

			GL2_3D::begin();
		//	_3d.begin();//
			_3dMode_DrawGridNAxes(_3d, VX, VY, VZ, DX, Xstep, Ystep, Zstep, AR_Y, AR_Z);
		//	_3dMode_FrameStart(_3d, VX, VY, VZ, DX, Xstep, Ystep, Zstep, AR_Y, AR_Z);
			int colorCondition=nExpr[2]+nExpr[3]>1;
#if 1
			if(!contourOnly)//draw the curves
			{
				double Xstart=VX-DX/2, Xstep=DX/Xplaces;
				if(operations.size())
				{
					for(int ke=0, keEnd=expr.size();ke<keEnd;++ke)
					{
						auto &ex=expr[ke];
						switch(ex.rmode[0])
						{
						case 2:
						case 3:
							{
								_3d.lineColor=0xFF000000|(ex.color&-colorCondition);
								auto &n=ex.n[ex.resultTerm];
								auto &ndr_r=n.r, &ndr_i=n.i;
								if(ndr_r.size()!=Xplaces)
									continue;
								GL2_3D::curve_start(_3d.lineColor);
								if(n.i.size()==Xplaces)
								{
									for(int x=0;x<Xplaces;++x)
										GL2_3D::curve_point(vec3(Xstart+x*Xstep, AR_Y*ndr_r[x], AR_Z*ndr_i[x]));
								}
								else
								{
									for(int x=0;x<Xplaces;++x)
										GL2_3D::curve_point(vec3(Xstart+x*Xstep, AR_Y*ndr_r[x], 0));
								}
							}
							break;
						}
					}
				}
				else
				{
					for(int ke=0, keEnd=expr.size();ke<keEnd;++ke)
					{
						auto &ex=expr[ke];
						switch(ex.rmode[0])
						{
						case 2:
						case 3:
							{
								_3d.lineColor=0xFF000000|(ex.color&-colorCondition);
								auto &n=ex.n[ex.resultTerm];
								auto &ndr_r=n.r, &ndr_i=n.i;
								int nthreads0=nthreads;//
								if(ndr_r.size()!=Xplaces)
									continue;
								GL2_3D::curve_start(_3d.lineColor);
								if(n.i.size()==Xplaces)
								{
									for(int x=0, xEnd=Xplaces-1;x<xEnd;++x)
									{
										if(ndr_r.size()!=Xplaces||ndr_i.size()!=Xplaces)//
											return;//
										GL2_3D::curve_point(vec3(Xstart+x*Xstep, AR_Y*ndr_r[x], AR_Z*ndr_i[x]));
										if(ex.discontinuities[x])
											GL2_3D::curve_start(_3d.lineColor);
									}
									GL2_3D::curve_point(vec3(Xstart+(Xplaces-1)*Xstep, AR_Y*ndr_r[Xplaces-1], AR_Z*ndr_i[Xplaces-1]));
								}
								else
								{
									for(int x=0, xEnd=Xplaces-1;x<xEnd;++x)
									{
										if(ndr_r.size()!=Xplaces)//
											return;//
										GL2_3D::curve_point(vec3(Xstart+x*Xstep, AR_Y*ndr_r[x], 0));
										if(ex.discontinuities[x])
											GL2_3D::curve_start(_3d.lineColor);
									}
									GL2_3D::curve_point(vec3(Xstart+(Xplaces-1)*Xstep, AR_Y*ndr_r[Xplaces-1], 0));
								}
							}
							break;
						}
					}
				}
			}
			if(contourOn)//draw contour
			{
				int mrX1=-60, mrX2=60, mrY1=-60, mrY2=60;
			//	int mx=((short*)&oldMouse)[0], my=((short*)&oldMouse)[1];
				double cross=5*DX/w;
				for(unsigned e=0;e<expr.size();++e)
				{
					auto &ex=expr[e];
					if(ex.rmode[0]==2||ex.rmode[0]==3)
					{
						_3d.lineColor=0xFF000000|(ex.color&-colorCondition);
						auto &Rcontour=Rcontours[e];
						for(auto cit=Rcontour.begin();cit!=Rcontour.end();++cit)
						{
							auto &L=*cit;
							double RL=AR_Y*Ystep*L.first;
						//	double RL=Ystep*L.first;
							for(auto xit=L.second.begin();xit!=L.second.end();++xit)
							{
								auto &X_I=*xit;
								_3d.line(vec3(X_I.x-cross	, RL			, AR_Z* X_I.i		), vec3(X_I.x+cross	, RL			, AR_Z* X_I.i		));
							//	_3d.line(vec3(X_I.x			, RL-AR_Y*cross	, AR_Z* X_I.i		), vec3(X_I.x		, RL+AR_Y*cross	, AR_Z* X_I.i		));
								_3d.line(vec3(X_I.x			, RL			, AR_Z*(X_I.i-cross)), vec3(X_I.x		, RL			, AR_Z*(X_I.i+cross)));
								int x, y;
								if(active&&_3d.pointCoordinates3dText(X_I.x, RL, X_I.i, x, y))
									print(x, y, "%g, %g + %g i", X_I.x, RL, X_I.i);
							}
						}
						auto Icontour=Icontours[e];
						for(auto lit=Icontour.begin();lit!=Icontour.end();++lit)
						{
							auto &L=*lit;
							double IL=AR_Z*Zstep*L.first;
						//	double IL=Zstep*L.first;
							for(auto xit=L.second.begin();xit!=L.second.end();++xit)
							{
								auto &X_R=*xit;
								_3d.line(vec3(X_R.x-cross	, AR_Y* X_R.r		, IL			), vec3(X_R.x+cross	, AR_Y* X_R.r		, IL));
								_3d.line(vec3(X_R.x			, AR_Y*(X_R.r-cross), IL			), vec3(X_R.x		, AR_Y*(X_R.r+cross), IL));
							//	_3d.line(vec3(X_R.x			, AR_Y* X_R.r		, IL-AR_Z*cross	), vec3(X_R.x		, AR_Y* X_R.r		, IL+AR_Z*cross));
								int x, y;
								if(active&&_3d.pointCoordinates3dText(X_R.x, X_R.r, IL, x, y))
									print(x, y, "%g, %g + %g i", X_R.x, X_R.r, IL);
							}
						}
//						if(colorCondition)
//							_3d.lineColor=0;
					}
				}
				if(active)
				{
					if(drag)
					{
						int X0=w/2, Y0=h/2;
						GL2_2D::draw_rectangle_hollow(X0+mrX1, X0+mrX2, Y0+mrY1, Y0+mrY2);
					}
				}
			}
			GL2_3D::end();
			GL2_3D::draw(_3d.cam);
#endif
			if(!clearScreen)
			{
				int bkMode=setBkMode(TRANSPARENT);
				for(double X=floor((VX-DX/2)/Xstep)*Xstep+Xstep, Xend=ceil((VX+DX/2)/Xstep)*Xstep;X<Xend;X+=Xstep)
					if(abs(X)>Xstep/2)
						_3d.label(X, VY, VZ, "%g", X);
				if(AR_Y==1)
				{
					for(double Y=floor((VY-DX/2)/Ystep)*Ystep+Ystep, Yend=ceil((VY+DX/2)/Ystep)*Ystep;Y<Yend;Y+=Ystep)
						if(Y<-Ystep/2||Y>Ystep/2)
							_3d.label(VX, Y, VZ, "%g", Y);
				}
				else
				{
					double aYstep=AR_Y*Ystep, Ycore=floor((VY-DX/2)/aYstep);
					for(double Y=Ycore*aYstep+aYstep, Yend=ceil((VY+DX/2)/aYstep)*aYstep, aY=Ycore*Ystep+Ystep;Y<Yend;Y+=aYstep, aY+=Ystep)
						if(Y<-aYstep/2||Y>aYstep/2)
							_3d.label(VX, Y, VZ, "%g", aY);
				}
				if(AR_Z==1)
				{
					for(double Z=floor((VZ-DX/2)/Zstep)*Zstep+Zstep, Zend=ceil((VZ+DX/2)/Zstep)*Zstep;Z<Zend;Z+=Zstep)
						if(Z<-Zstep/2||Z>Zstep/2)
							_3d.label(VX, VY, Z, "%g", Z);
				}
				else
				{
					double aZstep=AR_Z*Zstep, Zcore=floor((VZ-DX/2)/aZstep);
					for(double Z=Zcore*aZstep+aZstep, Zend=ceil((VZ+DX/2)/aZstep)*aZstep, aZ=Zcore*Zstep+Zstep;Z<Zend;Z+=aZstep, aZ+=Zstep)
						if(Z<-aZstep/2||Z>aZstep/2)
							_3d.label(VX, VY, Z, "%g", aZ);
				}
				if(colorCondition)
					_3d.arrowLabelsColor(0, 0, 0, 1, 0, 0, labels.Xlabels);
				else
					_3d.arrowLabels		(0, 0, 0, 1, 0, 0, labels.Xlabels);
				{
					int textColor=txtColor;
//					int textColor;
//					if(colorCondition)
//						textColor=GetTextColor(ghMemDC);
					int Ys=0;
					for(int kl=0, klEnd=labels.Clabels.size();kl<klEnd;++kl)
					{
						auto &label=labels.Clabels[kl];
						setTextColor(0xFF000000|(expr[label.exNo].color&-colorCondition));
						print(w-const_label_offset_X, Ys, "%s=%g", label.label.c_str(), label.isTime?solver.T:label.value);
						Ys+=fontH;
					}
					setTextColor(textColor);
				}
				_3d.arrowLabel(0, 0, 0, 0, 1, 0, "Re()");
				_3d.arrowLabel(0, 0, 0, 0, 0, 1, "Im()");
				setBkMode(bkMode);
				{
					int k=0, Y=h-operations.size()*16;
					for(auto it=operations.begin();it!=operations.end();++it)
					{
						auto &operation=*it;
						char const *a=0;
						switch(operation)
						{
						case 1:a="%d: Differentiate";break;
						case 2:a="%d: Integrate";break;
						case 3:a="%d: DFT";break;
						case 4:a="%d: IDFT";break;
						case 5:a="%d: LPF";break;
						case 6:a="%d: HPF";break;
						}
						print(w-const_label_offset_X, Y, a, k);
					//	GUIPrint(ghMemDC, w-const_label_offset_X, Y, a, k);
						++k, Y+=16;
					}
				}
			}
			_3d.text_show();
//			if(kb_VK_F6_msg||kb[VK_F6])
//			{
//				int kb_mode=SetBkMode(ghMemDC, OPAQUE);
//				kb_VK_F6_msg=false;
//				if(!_3d_stretch_move_cam&!_3d_shift_move_cam&!_3d_zoom_move_cam)
//					GUIPrint(ghMemDC, 0, h-18*3, "move cam: [-] scale, [-] shift, [-] zoom");
//				else if(!_3d_shift_move_cam&!_3d_zoom_move_cam)
//					GUIPrint(ghMemDC, 0, h-18*3, "move cam: [v] scale, [-] shift, [-] zoom");
//				else if(!_3d_zoom_move_cam)
//					GUIPrint(ghMemDC, 0, h-18*3, "move cam: [v] scale, [v] shift, [-] zoom");
//				else
//					GUIPrint(ghMemDC, 0, h-18*3, "move cam: [v] scale, [v] shift, [v] zoom");
//				SetBkMode(ghMemDC, kb_mode);
//			}
		}
		void i_draw();
		void a_draw();
	} t1d_c;
	const int		Transverse_1D_C::modes[]={2, 3}, Transverse_1D_C::nmodes=sizeof(modes)>>2;
	void			Transverse_1D_C::i_draw()
	{
		if(modes::ready)
		{
			draw();
//			if(showLastModeOnIdle)
//				std::copy(rgb, rgb+w*h, modeRGB);
		}
		else
		{
		//	std::copy(modeRGB, modeRGB+w*h, rgb);
		}
		if(showBenchmark)
			Performance(0, h>>1);//
		//	Performance(0, h-32);//
	}
	void			Transverse_1D_C::a_draw()
	{
	//	Rectangle(ghMemDC, bpx-1, bpy-1, bw+1, bh+1);

		draw();
		
		if(contextHelp)
		{
			const char *help[]=
			{
				"W/A/S/D/T/G: move",
				"arrows: turn",
				"+/-/enter/backspace/wheel: change FOV",
				"X/Y/Z arrows/mouse move: shift x/y/z",
				"X/Y/Z +/-/enter/backspace/wheel: scale x/y/z",
				"alt +/-/enter/backspace/wheel: zoom",
				"E: reset scale",
				"R: reset scale & view",
				"C: toggle clear screen",
				"1: differentiate",
				"2: integrate",
				"3: DFT",
				"4: Inverse DFT",
				"5: LPF",
				"6: HPF",
				"0: reset operations",
				"`: contour",
				"Esc: back to text editor"
			};
			print_contextHelp(help, sizeof(help)>>2, 294);
		}
		if(showBenchmark&&!clearScreen)
			Performance(0, h);//
		//	Performance(0, h-32);//
	//	BitBlt(ghDC, 0, 0, w, h, ghMemDC, 0, 0, SRCCOPY);
	}
	bool		difference_t1d_t1dc(){return t1d.VX!=t1d_c.VX||t1d.DX!=t1d_c.DX||t1d.Xplaces!=t1d_c.Xplaces;}

	namespace contour
	{
		struct Double_x_i_j_k
		{
			double x, i, j, k;
			Double_x_i_j_k(double x, double i, double j, double k):x(x), i(i), j(j), k(k){}
		};
		struct Double_x_r_j_k
		{
			double x, r, j, k;
			Double_x_r_j_k(double x, double r, double j, double k):x(x), r(r), j(j), k(k){}
		};
		struct Double_x_r_i_k
		{
			double x, r, i, k;
			Double_x_r_i_k(double x, double r, double i, double k):x(x), r(r), i(i), k(k){}
		};
		struct Double_x_r_i_j
		{
			double x, r, i, j;
			Double_x_r_i_j(double x, double r, double i, double j):x(x), r(r), i(i), j(j){}
		};
	}
	class		Transverse_1D_H:public _2D_Mode
	{
	public:
		int Xoffset;

		int Xplaces;
		Solve_1D solver;
		std::list<int> operations;

		bool contourOn, contourOnly;
		std::map<unsigned, std::unordered_map<int, std::list<contour::Double_x_i_j_k>>> Rcontours;
		//Rcontours[e]=Rcontour, Rcontour[r level (r/step)]=points, *points.begin()=(x,i,j,k)
		std::map<unsigned, std::unordered_map<int, std::list<contour::Double_x_r_j_k>>> Icontours;
		//Icontours[e]=Icontour, Icontour[i level (i/step)]=points, *points.begin()=(x,r,j,k)
		std::map<unsigned, std::unordered_map<int, std::list<contour::Double_x_r_i_k>>> Jcontours;
		//Jcontours[e]=Jcontour, Jcontour[j level (j/step)]=points, *points.begin()=(x,r,i,k)
		std::map<unsigned, std::unordered_map<int, std::list<contour::Double_x_r_i_j>>> Kcontours;
		//Kcontours[e]=Kcontour, Kcontour[k level (k/step)]=points, *points.begin()=(x,r,i,j)
		
	//	_2D _2d;
		bool clearScreen;
		Labels_1D labels;
		static const int rColor=0, iColor=0xFFEF0000, jColor=0xFF00EF00, kColor=0xFF0000EF;
	//	HPEN__ *hPenGrid, *hPenR, *hPenI, *hPenJ, *hPenK;
	//	HBRUSH__ *hBrush;
		static const int modes[], nmodes;


		Transverse_1D_H():
			solver(true), _2D_Mode(solver),
			Xoffset(0),
			clearScreen(false)
		{
		//	hPenGrid=CreatePen(PS_SOLID, 1, _2dCheckColor), hBrush=CreateSolidBrush(_2dCheckColor);
		//	hPenR=CreatePen(PS_SOLID, 1, rColor), hPenI=CreatePen(PS_SOLID, 1, iColor), hPenJ=CreatePen(PS_SOLID, 1, jColor), hPenK=CreatePen(PS_SOLID, 1, kColor);
		}
//		~Transverse_1D_H()
//		{
//			DeleteObject(hPenGrid), DeleteObject(hBrush);
//			DeleteObject(hPenR), DeleteObject(hPenI), DeleteObject(hPenJ), DeleteObject(hPenK);
//		}
		
		void doContour(unsigned e, double Xs, double Ystart, double Yend, double Ystep, unsigned nySteps)
		{
			using namespace contour;
			auto &n=expr[e].n[expr[e].resultTerm];
			auto &ndr_r=n.r, &ndr_i=n.i, &ndr_j=n.j, &ndr_k=n.k;
			auto &Rcontour=Rcontours[e];
			auto &Icontour=Icontours[e];
			auto &Jcontour=Jcontours[e];
			auto &Kcontour=Kcontours[e];
			double YLstart=std::ceil((Ystart-10*Xs)/Ystep)*Ystep, YLend=std::ceil((Yend+25*Xs)/Ystep)*Ystep;
			for(int v=0;v<Xplaces-1;++v)
			{
				QuatRef V1(ndr_r[v], ndr_i[v], ndr_j[v], ndr_k[v]), V2(ndr_r[v+1], ndr_i[v+1], ndr_j[v+1], ndr_k[v+1]);
				if(V1.r==V1.r&&V2.r==V2.r)
				{
					double R1, R2;
					if(V1.r<V2.r)
						R1=V1.r, R2=V2.r;
					else
						R1=V2.r, R2=V1.r;
					for(int r=int(std::floor(maximum(R1, YLstart)/Ystep)), rEnd=int(std::floor(minimum(R2, YLend)/Ystep));r<=rEnd;++r)
					{
						double RL=Ystep*r;
						if(R1<=RL&&RL<R2)
						{
							double M=(RL-V1.r)/(V2.r-V1.r);
						//	double M=(RL-V1.r)/(V2-V1.r);
							Rcontour[r].push_back(Double_x_i_j_k(solver.Xstart+Xs*(v+M), V1.i+(V2.i-V1.i)*M, V1.j+(V2.j-V1.j)*M, V1.k+(V2.k-V1.k)*M));
						}
					}
				}
				if(V1.i==V1.i&&V2.i==V2.i)
				{
					double I1, I2;
					if(V1.i<V2.i)
						I1=V1.i, I2=V2.i;
					else
						I1=V2.i, I2=V1.i;
					for(int i=int(std::floor(maximum(I1, YLstart)/Ystep)), iEnd=int(std::floor(minimum(I2, YLend)/Ystep));i<=iEnd;++i)
					{
						double IL=Ystep*i;
						if(I1<=IL&&IL<I2)
						{
							double M=(IL-V1.i)/(V2.i-V1.i);
						//	double M=(IL-V1.i)/(V2-V1.i);
							Icontour[i].push_back(Double_x_r_j_k(solver.Xstart+Xs*(v+M), V1.r+(V2.r-V1.r)*M, V1.j+(V2.j-V1.j)*M, V1.k+(V2.k-V1.k)*M));
						}
					}
				}
				if(V1.j==V1.j&&V2.j==V2.j)
				{
					double J1, J2;
					if(V1.j<V2.j)
						J1=V1.j, J2=V2.j;
					else
						J1=V2.j, J2=V1.j;
					for(int j=int(std::floor(maximum(J1, YLstart)/Ystep)), jEnd=int(std::floor(minimum(J2, YLend)/Ystep));j<=jEnd;++j)
					{
						double JL=Ystep*j;
						if(J1<=JL&&JL<J2)
						{
							double M=(JL-V1.j)/(V2.j-V1.j);
							Jcontour[j].push_back(Double_x_r_i_k(solver.Xstart+Xs*(v+M), V1.r+(V2.r-V1.r)*M, V1.i+(V2.i-V1.i)*M, V1.k+(V2.k-V1.k)*M));
						}
					}
				}
				if(V1.k==V1.k&&V2.k==V2.k)
				{
					double K1, K2;
					if(V1.k<V2.k)
						K1=V1.k, K2=V2.k;
					else
						K1=V2.k, K2=V1.k;
					for(int k=int(std::floor(maximum(K1, YLstart)/Ystep)), kEnd=int(std::floor(minimum(K2, YLend)/Ystep));k<=kEnd;++k)
					{
						double KL=Ystep*k;
						if(K1<=KL&&KL<K2)
						{
							double M=(KL-V1.k)/(V2.k-V1.k);
							Kcontour[k].push_back(Double_x_r_i_j(solver.Xstart+Xs*(v+M), V1.r+(V2.r-V1.r)*M, V1.i+(V2.i-V1.i)*M, V1.j+(V2.j-V1.j)*M));
						}
					}
				}
			}
		}
		void differentiate						(Term &n)
		{
			auto &ndr_r=n.r, &ndr_i=n.i, &ndr_j=n.j, &ndr_k=n.k;
			double step=DX/Xplaces, _1_step=1/step;
			for(int k=0;k<Xplaces-1;++k)
				ndr_r[k]=(ndr_r[k+1]-ndr_r[k])*_1_step, ndr_i[k]=(ndr_i[k+1]-ndr_i[k])*_1_step, ndr_j[k]=(ndr_j[k+1]-ndr_j[k])*_1_step, ndr_k[k]=(ndr_k[k+1]-ndr_k[k])*_1_step;
			//	ndr[k]=((boost::math::quaternion<double>)ndr[k+1]-(boost::math::quaternion<double>)ndr[k])/step;
			ndr_r[Xplaces-1]*=-_1_step, ndr_i[Xplaces-1]*=-_1_step, ndr_j[Xplaces-1]*=-_1_step, ndr_k[Xplaces-1]*=-_1_step;
			//ndr.rbegin()->r/=-step, ndr.rbegin()->i/=-step, ndr.rbegin()->j/=-step, ndr.rbegin()->k/=-step;
		}
		void integrate							(Term &n)
		{
			auto &ndr_r=n.r, &ndr_i=n.i, &ndr_j=n.j, &ndr_k=n.k;
			double step=DX/Xplaces;
			Quat1d sum;
			for(int k=0;k<Xplaces;++k)
			{
				sum+=Quat1d(ndr_r[k], ndr_i[k], ndr_j[k], ndr_k[k]);
				QuatRef(ndr_r[k], ndr_i[k], ndr_j[k], ndr_k[k])=sum*step;
			}
		}
		void discreteFourrierTransform			(Term &n)
		{
			auto &ndr_r=n.r, &ndr_i=n.i, &ndr_j=n.j, &ndr_k=n.k;
			std::vector<Quat1d> t0(Xplaces);
			for(int k=0;k<Xplaces;++k)
				t0[k]=Quat1d(ndr_r[k], ndr_i[k], ndr_j[k], ndr_k[k]);
			std::vector<Quat1d> t1(Xplaces);
			for(unsigned k=0;k<t0.size();++k)
			{
				Quat1d p=-2*G2::_pi*Quat1d(0, 1)*double(k)/double(t0.size());
				for(unsigned k2=0;k2<t0.size();++k2)
					t1[k]+=t0[k2]*exp(p*double(k2));
				t1[k]/=t0.size();
			}
			for(int k=0;k<Xplaces;++k)
				QuatRef(ndr_r[k], ndr_i[k], ndr_j[k], ndr_k[k])=t1[k];//*/
		}
		void inverseDiscreteFourrierTransform	(Term &n)
		{
			auto &ndr_r=n.r, &ndr_i=n.i, &ndr_j=n.j, &ndr_k=n.k;
			std::vector<Quat1d> t0(Xplaces);
			for(int k=0;k<Xplaces;++k)
				t0[k]=Quat1d(ndr_r[k], ndr_i[k], ndr_j[k], ndr_k[k]);
			std::vector<Quat1d> t1(Xplaces);
			for(int k=0;k<Xplaces;++k)
			{
				Quat1d p=2*G2::_pi*Quat1d(0, 1)*double(k)/double(t0.size());
				for(unsigned k2=0;k2<t0.size();++k2)
					t1[k]+=t0[k2]*exp(p*double(k2));
			}
			for(int k=0;k<Xplaces;++k)
				QuatRef(ndr_r[k], ndr_i[k], ndr_j[k], ndr_k[k])=t1[k];
		}
		void lowPassFilter						(Term &n)
		{
			auto &ndr_r=n.r, &ndr_i=n.i, &ndr_j=n.j, &ndr_k=n.k;
			unsigned N=Xplaces, T=11;
			Quat1d sum;
			std::vector<Quat1d> t0(T);
			for(unsigned k=-int(T)/2;k<T/2;++k)
				sum+=t0[k%T]=Quat1d(ndr_r[k%N], ndr_i[k%N], ndr_j[k%N], ndr_k[k%N]);
			std::vector<Quat1d> t1(t0);
			for(unsigned k=0;k<N-T/2;++k)
			{
				t0[k%T]=Quat1d(ndr_r[k], ndr_i[k], ndr_j[k], ndr_k[k]);
				QuatRef(ndr_r[k], ndr_i[k], ndr_j[k], ndr_k[k])=sum/double(T);
				int idx=(k+T/2)%N;
				sum+=Quat1d(ndr_r[idx], ndr_i[idx], ndr_j[idx], ndr_k[idx])-t0[(k-T/2)%T];
			}
			for(unsigned k=N-T/2;k<N;++k)
				QuatRef(ndr_r[k], ndr_i[k], ndr_j[k], ndr_k[k])=sum/double(T), sum+=t1[(k-(N-T/2))%T]-t0[(k-T/2)%T];
		}
		void highPassFilter						(Term &n)
		{
			auto &ndr_r=n.r, &ndr_i=n.i, &ndr_j=n.j, &ndr_k=n.k;
			unsigned N=Xplaces, T=11;
			Quat1d sum;
			std::vector<Quat1d> t0(T);
			for(unsigned k=-int(T)/2;k<T/2;++k)
				sum+=t0[k%T]=Quat1d(ndr_r[k%N], ndr_i[k%N], ndr_j[k%N], ndr_k[k%N]);
			std::vector<Quat1d> t1(t0);
			for(unsigned k=0;k<N-T/2;++k)
			{
				t0[k%T]=Quat1d(ndr_r[k], ndr_i[k], ndr_j[k], ndr_k[k]);
				QuatRef(ndr_r[k], ndr_i[k], ndr_j[k], ndr_k[k])-=sum/double(T);
				int idx=(k+T/2)%N;
				sum+=Quat1d(ndr_r[idx], ndr_i[idx], ndr_j[idx], ndr_k[idx])-t0[(k-T/2)%T];
			}
			for(unsigned k=N-T/2;k<N;++k)
				QuatRef(ndr_r[k], ndr_i[k], ndr_j[k], ndr_k[k])-=sum/double(T), sum+=t1[(k-(N-T/2))%T]-t0[(k-T/2)%T];
		}

		void messageTimer()
		{
//			if(_2d_drag_graph_not_window)
//			{
//				if(kb[VK_LEFT		])	VX+=10*DX/ w	, toSolve=true; if(shiftOnly)shiftOnly=1, Xoffset+=10;
//				if(kb[VK_RIGHT		])	VX-=10*DX/ w	, toSolve=true; if(shiftOnly)shiftOnly=1, Xoffset-=10;
//				if(kb[VK_UP			])	VY-=10*DX/(w*AR_Y);
//				if(kb[VK_DOWN		])	VY+=10*DX/(w*AR_Y);
//			}
//			else
//			{
//				if(kb[VK_LEFT		])	VX-=10*DX/ w	, toSolve=true; if(shiftOnly)shiftOnly=1, Xoffset-=10;
//				if(kb[VK_RIGHT		])	VX+=10*DX/ w	, toSolve=true; if(shiftOnly)shiftOnly=1, Xoffset+=10;
//				if(kb[VK_UP			])	VY+=10*DX/(w*AR_Y);
//				if(kb[VK_DOWN		])	VY-=10*DX/(w*AR_Y);
//			}
//			if(kb[VK_ADD		]||kb[VK_RETURN	]||kb[VK_OEM_PLUS	])
//			{
//				if(kb['X'])
//					DX/=1.05, AR_Y/=1.05;
//				else if(kb['Y'])
//					AR_Y*=1.05;
//				else
//					DX/=1.05;
//				function1(), toSolve=true, shiftOnly=0;
//			}
//			if(kb[VK_SUBTRACT	]||kb[VK_BACK	]||kb[VK_OEM_MINUS	])
//			{
//				if(kb['X'])
//					DX*=1.05, AR_Y*=1.05;
//				else if(kb['Y'])
//					AR_Y/=1.05;
//				else
//					DX*=1.05;
//				function1(), toSolve=true, shiftOnly=0;
//			}
			a_draw();
//			if(!time_variance&&!kp)
//				KillTimer(ghWnd, 0), timer=false;
		}
		int inputTouchDown(int idx)
		{
			return _2D_DRAG;
		}
		void inputTouchUp(int idx)
		{
		}
		void inputTouchMove(int idx)
		{
			auto &ti=touchInfo[idx];
			if(touchInfo.size()==1)//shift
			{
				if(ti.region==_2D_DRAG)
				{
					double Xr=DX/w;
					int dx=-ti.delta.x;
					VX+=dx*Xr;
					VY+=ti.delta.y*Xr/AR_Y;
					toSolve=true;
					if(shiftOnly)
						shiftOnly=1, Xoffset+=dx;
				}
			}
			else if(touchInfo.size()==2)//zoom
			{
				if(idx==1)//wait till both pointers are updated
				{
					auto &ti2=touchInfo[!idx];
					if(ti.region==_2D_DRAG&&ti2.region==_2D_DRAG)
					{
						auto &a=ti.pos, &b=ti2.pos, a0=a-ti.delta, b0=b-ti2.delta;
						float d=(a-b).magnitude(), d0=(a0-b0).magnitude();
						float diff=d0-d;
						double Xr=DX/w;
						DX+=diff*Xr;
						vec2 ab=0.5f*(a+b), ab0=0.5f*(a0+b0), dab=ab0-ab;
						VX+=dab.x*Xr, VY-=dab.y*Xr/AR_Y;
						function1();
						toSolve=true, shiftOnly=0;
					}
				}
			}
		}

		void draw_component(Expression &ex, std::vector<double> &ndr, int yoffset, double Yend, double Yr)
		{
			GL2_2D::curve_begin();
			for(int v=0, vEnd=Xplaces-1;v<vEnd;++v)
			{
				if(ndr.size()!=Xplaces)
					return;
				GL2_2D::curve_point(bpx+v, Yr*(Yend-ndr[v])+yoffset);
				GL2_2D::continuous=!ex.discontinuities[v];
			}
			GL2_2D::draw_curve();
		}
		void draw_component_continuous(Expression &ex, std::vector<double> &ndr, int yoffset, double Yend, double Yr)
		{
			GL2_2D::curve_begin();
			GL2_2D::continuous=true;
			for(int v=0, vEnd=Xplaces-1;v<vEnd;++v)
			{
				if(ndr.size()!=Xplaces)
					return;
				GL2_2D::curve_point(bpx+v, Yr*(Yend-ndr[v])+yoffset);
			}
			GL2_2D::draw_curve();
		}
		void draw()
		{
			double DY=DX*h/(w*AR_Y);
			if(DY<=0)
				DY=1;
			_2dMode_DrawCheckboard(_2dCheckColor, VX, VY, DX, DY, Xstep, Ystep);
			auto &ex=expr[cursorEx];
			{
				bool changed=false;
				if(toSolve)
				{
					if(!operations.size()&&shiftOnly==1&&abs(Xplaces)<Xoffset)
					{
						if(Xoffset)
						{
							if(ex.nITD)
							{
								solver.synchronize();
								solver.full(ex);
							}
							else
							{
								solver.partial_bounds(VX, DX, Xoffset);
								solver.partial(ex);
							}
						}
					}
					else
					{
						auto old_time_variance=time_variance;
						time_variance=false;
						labels.clear();

						Xplaces=w;
						solver.full_resize(VX, DX, Xplaces);
						time_variance|=ex.nITD;
						labels.fill(cursorEx);
						if(!paused)
							solver.synchronize();
						solver.full(ex);
						for(auto it=operations.begin();it!=operations.end();++it)
						{
							auto &operation=*it;
							switch(operation)
							{
							case 1:differentiate					(ex.n[ex.resultTerm]);break;
							case 2:integrate						(ex.n[ex.resultTerm]);break;
							case 3:discreteFourrierTransform		(ex.n[ex.resultTerm]);break;
							case 4:inverseDiscreteFourrierTransform	(ex.n[ex.resultTerm]);break;
							case 5:lowPassFilter					(ex.n[ex.resultTerm]);break;
							case 6:highPassFilter					(ex.n[ex.resultTerm]);break;
							}
						}
//						if(time_variance)
//						{
//							if(!paused)
//								SetTimer(ghWnd, 0, 10, 0);
//						}
//						else if(old_time_variance&&!timer)
//							KillTimer(ghWnd, 0);
					}
					changed=true;
					toSolve=false, shiftOnly=2, Xoffset=0;
				}
				else if(time_variance)
				{
					if(!paused)
						solver.synchronize();
					solver.full(ex);
					for(auto it=operations.begin();it!=operations.end();++it)
					{
						auto &operation=*it;
						switch(operation)
						{
						case 1:differentiate					(ex.n[ex.resultTerm]);break;
						case 2:integrate						(ex.n[ex.resultTerm]);break;
						case 3:discreteFourrierTransform		(ex.n[ex.resultTerm]);break;
						case 4:inverseDiscreteFourrierTransform	(ex.n[ex.resultTerm]);break;
						case 5:lowPassFilter					(ex.n[ex.resultTerm]);break;
						case 6:highPassFilter					(ex.n[ex.resultTerm]);break;
						}
					}
					changed=true;
				}
				if(changed&&contourOn)
				{
					Rcontours.clear(), Icontours.clear(), Jcontours.clear(), Kcontours.clear();
					double DY=DX*h/(w*AR_Y);
					doContour(cursorEx, DX/Xplaces, VY-DY/2, VY+DY/2, Ystep, 10);
				}
			}
			if(!clearScreen)
			{
				int bkMode=getBkMode();
				int H=0, V=0, VT=0;
				_2dMode_NumberAxes(VX, VY, DX, DY, Xstep, Ystep, prec, H, V, VT);
				GL2_2D::draw_line(0, H, w, H), GL2_2D::draw_line(V, 0, V, h);
				{
					int textColor=getTextColor(), Xs=VT+24+8*prec>w-24-prec*8?VT-24-8*prec:VT+24+8*prec, Ys=0;
					setTextColor(rColor), print(Xs, Ys, "R()", 3), Ys+=fontH;
					setTextColor(iColor), print(Xs, Ys, "I()", 3), Ys+=fontH;
					setTextColor(jColor), print(Xs, Ys, "J()", 3), Ys+=fontH;
					setTextColor(kColor), print(Xs, Ys, "K()", 3);
					setTextColor(textColor);
				}
				{
					int Ys=H+(H>h-46?-34:18), Ys0=Ys;
					for(int kl=0, klEnd=labels.Xlabels.size();kl<klEnd;++kl)
					{
						auto &label=labels.Xlabels[kl];
						print_array(w-arrow_label_offset_X, Ys, label.label.c_str(), label.label.size()), Ys+=fontH;
					}
					Ys=Ys0>h/3?0:h-16*labels.Clabels.size();
					for(int kl=0, klEnd=labels.Clabels.size();kl<klEnd;++kl)
					{
						auto &label=labels.Clabels[kl];
						print(w-const_label_offset_X, Ys, "%s=%g", label.label.c_str(), label.isTime?solver.T:label.value), Ys+=fontH;
					}
				}
				setBkMode(bkMode);
			}
			if(!contourOnly)//draw the curves
			{
				double Yend=VY+DY/2, Yr=h/DY;
				auto &n=ex.n[ex.resultTerm];
				int colors[]={rColor, iColor, jColor, kColor};
				if(operations.size())
				{
					for(unsigned c=0;c<4;++c)
					{
						set_color(colors[c]);
						draw_component_continuous(ex, (&n.r)[c], 0, Yend, Yr);
					}
				}
				else
				{
					for(unsigned c=0;c<4;++c)
					{
						set_color(colors[c]);
						draw_component(ex, (&n.r)[c], 0, Yend, Yr);
					}
				}
				int k=0, Y=h-operations.size()*16;
				for(auto it=operations.begin();it!=operations.end();++it)
				{
					auto &operation=*it;
					char const *a=0;
					switch(operation)
					{
					case 1:a="%d: Differentiate";	break;
					case 2:a="%d: Integrate";		break;
					case 3:a="%d: DFT";				break;
					case 4:a="%d: IDFT";			break;
					case 5:a="%d: LPF";				break;
					case 6:a="%d: HPF";				break;
					}
					print(w-const_label_offset_X, Y, a, k);
					++k, Y+=16;
				}
			}
			if(contourOn)//mark the crossings
			{
				int mrX1=-60, mrX2=60, mrY1=-60, mrY2=60;
				double Xstart=VX-DX/2, Yend=VY+DY/2;
				set_color(rColor);
				auto &Rcontour=Rcontours[cursorEx];
				for(auto cit=Rcontour.begin();cit!=Rcontour.end();++cit)
				{
					auto &L=*cit;
					double R=Ystep*L.first;
					int y=(int)floor((Yend-R)*h/DY);
					for(auto xit=L.second.begin();xit!=L.second.end();++xit)
					{
						auto &X=*xit;
						int x=int((X.x-Xstart)*w/DX);
						GL2_2D::draw_line(x-5, y, x+5, y);
						GL2_2D::draw_line(x, y-5, x, y+5);
						if(active)
							print(x, y, "%g, %g+%gi+%gj+%gk", X.x, R, X.i, X.j, X.k);
					}
				}
				set_color(iColor);
				auto &Icontour=Icontours[cursorEx];
				for(auto cit=Icontour.begin();cit!=Icontour.end();++cit)
				{
					auto &L=*cit;
					double I=Ystep*L.first;
					int y=(int)floor((Yend-I)*h/DY);
					for(auto xit=L.second.begin();xit!=L.second.end();++xit)
					{
						auto &X=*xit;
						int x=int((X.x-Xstart)*w/DX);
						GL2_2D::draw_line(x-5, y, x+5, y);
						GL2_2D::draw_line(x, y-5, x, y+5);
						if(active)
							print(x, y, "%g, %g+%gi+%gj+%gk", X.x, X.r, I, X.j, X.k);
					}
				}
				set_color(jColor);
				auto &Jcontour=Jcontours[cursorEx];
				for(auto cit=Jcontour.begin();cit!=Jcontour.end();++cit)
				{
					auto &L=*cit;
					double J=Ystep*L.first;
					int y=(int)floor((Yend-J)*h/DY);
					for(auto xit=L.second.begin();xit!=L.second.end();++xit)
					{
						auto &X=*xit;
						int x=int((X.x-Xstart)*w/DX);
						GL2_2D::draw_line(x-5, y, x+5, y);
						GL2_2D::draw_line(x, y-5, x, y+5);
						if(active)
							print(x, y, "%g, %g+%gi+%gj+%gk", X.x, X.r, X.i, J, X.k);
					}
				}
				set_color(kColor);
				auto &Kcontour=Kcontours[cursorEx];
				for(auto cit=Kcontour.begin();cit!=Kcontour.end();++cit)
				{
					auto &L=*cit;
					double K=Ystep*L.first;
					int y=(int)floor((Yend-K)*h/DY);
					for(auto xit=L.second.begin();xit!=L.second.end();++xit)
					{
						auto &X=*xit;
						int x=int((X.x-Xstart)*w/DX);
						GL2_2D::draw_line(x-5, y, x+5, y);
						GL2_2D::draw_line(x, y-5, x, y+5);
						if(active)
							print(x, y, "%g, %g+%gi+%gj+%gk", X.x, X.r, X.i, X.j, K);
					}
				}
				if(active)
				{
					if(drag)
					{
						int w_2=w/2, h_2=h/2;
						GL2_2D::draw_rectangle_hollow(w_2+mrX1, w_2+mrX2, h_2+mrY1, h_2+mrY2);
					}
				}
			}
		}
		void i_draw();
		void a_draw();
	} t1d_h;
	const int Transverse_1D_H::modes[]={4}, Transverse_1D_H::nmodes=sizeof(modes)>>2;
	void			Transverse_1D_H::i_draw()
	{
		if(modes::ready)
		{
			draw();
//			if(showLastModeOnIdle)
//				std::copy(rgb, rgb+w*h, modeRGB);
		}
		else
		{
			//std::copy(modeRGB, modeRGB+w*h, rgb);
		}
		if(showBenchmark)
			Performance(0, h>>1);//
	}
	void			Transverse_1D_H::a_draw()
	{
	//	Rectangle(ghMemDC, bpx-1, bpy-1, bw+1, bh+1);

		draw();
		
		if(contextHelp)
		{
			const char *help[]=
			{
				"arrows/drag: move",
				"+/-/enter/backspace/wheel: zoom",
				"X/Y +/-/enter/backspace/wheel: scale x/y",
				"E: reset scale",
				"R: reset scale & view",
				"C: toggle clear screen",
				"1: differentiate",
				"2: integrate",
				"3: DFT",
				"4: Inverse DFT",
				"5: LPF",
				"6: HPF",
				"0: reset operations",
				"`: contour",
				"Esc: back to text editor"
			};
			print_contextHelp(help, sizeof(help)>>2, 247);
		}
		if(showBenchmark&&!clearScreen)
			Performance(0, h);//
	//	BitBlt(ghDC, 0, 0, w, h, ghMemDC, 0, 0, SRCCOPY);
	}
	class		Implicit_2D:public _2D_Mode
	{
	public:
		int Xoffset, Yoffset, exprRemoved;

		int Xplaces, Yplaces;
		Solve_2D_Implicit solver;
		
	//	_2D _2d;
		bool clearScreen;
		Labels_2D labels;
		static const int modes[], nmodes;
	//	HPEN hPen;
	//	HBRUSH hBrush;

		Implicit_2D():
			_2D_Mode(solver),
			Xoffset(0), Yoffset(0), exprRemoved(0),
			clearScreen(false)
		{
		//	hPen=CreatePen(PS_SOLID, 1, _2dCheckColor), hBrush=CreateSolidBrush(_2dCheckColor);
		}
	//	~Implicit_2D(){DeleteObject(hPen), DeleteObject(hBrush);}

		void messageTimer(){a_draw();}
		int inputTouchDown(int idx){return _2D_DRAG;}
		void inputTouchUp(int idx){}
		void inputTouchMove(int idx)
		{
			auto &ti=touchInfo[idx];
			if(touchInfo.size()==1)//shift
			{
				if(ti.region==_2D_DRAG)
				{
					double Xr=DX/w;
					int dx=-ti.delta.x;
					VX+=dx*Xr;
					VY+=ti.delta.y*Xr/AR_Y;
					toSolve=true;
					if(shiftOnly)
						shiftOnly=1, Xoffset+=dx;
				}
			}
			else if(touchInfo.size()==2)//zoom
			{
				if(idx==1)//wait till both pointers are updated
				{
					auto &ti2=touchInfo[!idx];
					if(ti.region==_2D_DRAG&&ti2.region==_2D_DRAG)
					{
						auto &a=ti.pos, &b=ti2.pos, a0=a-ti.delta, b0=b-ti2.delta;
						float d=(a-b).magnitude(), d0=(a0-b0).magnitude();
						float diff=d0-d;
						double Xr=DX/w;
						DX+=diff*Xr;
						vec2 ab=0.5f*(a+b), ab0=0.5f*(a0+b0), dab=ab0-ab;
						VX+=dab.x*Xr, VY-=dab.y*Xr/AR_Y;
						function1();
						toSolve=true, shiftOnly=0;
					}
				}
			}
		}

		void draw()
		{
			double DY=DX*h/(w*AR_Y);
			if(DY<=0)
				DY=1;
			double Xr=w/DX;
			if(toSolve)
			{
				if(shiftOnly==1&&abs(Xoffset)<Xplaces&&abs(Yoffset)<Yplaces)
				{
					if(Xoffset||Yoffset)
					{
						solver.partial_bounds(VX, DX, VY, DY, Xoffset, Yoffset);
					//	solver.hBitmap=(HBITMAP)SelectObject(ghMemDC, solver.hBitmap);
						solver.shiftAndCheckboard(_2dCheckColor, VX, DX, VY, DY, Xstep, Ystep, clearScreen);
						solver.prgb=&solver.rgb;
						for(int ke=0, keEnd=expr.size();ke<keEnd;++ke)
						{
							auto &ex=expr[ke];
							if(ex.rmode[0]==11&&!ex.nITD)
								(solver.*solver.partial)(ex);
						}
					//	solver.hBitmap=(HBITMAP)SelectObject(ghMemDC, solver.hBitmap);
						std::copy(solver.rgb, solver.rgb+w*h, solver.g_rgb);
					//	std::copy(solver.rgb, solver.rgb+w*h, rgb);
						solver.prgb=&solver.g_rgb;
					//	solver.prgb=&rgb;
						solver.synchronize();
						for(int ke=0, keEnd=expr.size();ke<keEnd;++ke)
						{
							auto &ex=expr[ke];
							if(ex.rmode[0]==11&&ex.nITD)
								solver.full(ex), solver.draw(ex);
						}
					}
				}
				else
				{
					auto old_time_variance=time_variance;
					time_variance=false;
					labels.clear();
					solver.full_resize(VX, DX, VY, DY, Xplaces=w, Yplaces=h);
					solver.prgb=&solver.rgb;
					for(unsigned e=0;e<expr.size();++e)
					{
						auto &ex=expr[e];
						if(ex.rmode[0]==11&&!ex.nITD)
						{
							labels.fill(e);
							solver.full(ex), solver.draw(ex);
						}
					}
					std::copy(solver.rgb, solver.rgb+w*h, solver.g_rgb);
					solver.prgb=&solver.g_rgb;
				//	std::copy(solver.rgb, solver.rgb+w*h, rgb);
				//	solver.prgb=&rgb;
					if(!paused)
						solver.synchronize();
					for(unsigned e=0;e<expr.size();++e)
					{
						auto &ex=expr[e];
						if(ex.rmode[0]==11&&ex.nITD)
					//	if(ex.rmode[0]==11&&(time_variance|=ex.nITD))
						{
							labels.fill(e);
							solver.full(ex), solver.draw(ex);
							time_variance=true;
						}
					}
//					if(time_variance)
//					{
//						if(!paused)
//							SetTimer(ghWnd, 0, 10, 0);
//					}
//					else if(old_time_variance&&!timer)
//						KillTimer(ghWnd, 0);
				}
				toSolve=false, shiftOnly=2, Xoffset=0, Yoffset=0;
				exprRemoved=false;
			}
			else if(exprRemoved)//expression(s) was removed
			{
				_2dMode_DrawCheckboard(_2dCheckColor, VX, VY, DX, DY, Xstep, Ystep);
				solver.prgb=&solver.rgb;

				auto old_time_variance=time_variance;
				time_variance=false;
				labels.clear();
				for(unsigned e=0;e<expr.size();++e)
				{
					auto &ex=expr[e];
					if(ex.rmode[0]==11&&!ex.nITD)
					{
						labels.fill(e);
						solver.draw(ex);
					}
				}
			//	solver.hBitmap=(HBITMAP)SelectObject(ghMemDC, solver.hBitmap);
				std::copy(solver.rgb, solver.rgb+w*h, solver.g_rgb);
				solver.prgb=&solver.g_rgb;
//				std::copy(solver.rgb, solver.rgb+w*h, rgb);
//				solver.prgb=&rgb;
				for(unsigned e=0;e<expr.size();++e)
				{
					auto &ex=expr[e];
					if(ex.rmode[0]==11&&ex.nITD)
					{
						labels.fill(e);
						solver.draw(ex);
						time_variance=true;
					}
				}
//				if(old_time_variance&&!time_variance&&!timer)
//					KillTimer(ghWnd, 0);
				exprRemoved=false;
			}
			else if(time_variance)
			{
				std::copy(solver.rgb, solver.rgb+w*h, solver.g_rgb);
			//	std::copy(solver.rgb, solver.rgb+w*h, rgb);
				if(!paused)
					solver.synchronize();
				for(int ke=0, keEnd=expr.size();ke<keEnd;++ke)
				{
					auto &ex=expr[ke];
					if(ex.rmode[0]==11&&ex.nITD)
						solver.full(ex), solver.draw(ex);
				}
			}
			else
				std::copy(solver.rgb, solver.rgb+w*h, solver.g_rgb);
			//	std::copy(solver.rgb, solver.rgb+w*h, rgb);

			_2dMode_DrawCheckboard(_2dCheckColor, VX, VY, DX, DY, Xstep, Ystep);//draw grid and time-fixed expressions on solver bitmap
//			for(int k=0;k<solver.ndrSize;++k)//set alpha to half
//				((unsigned char*)&solver.g_rgb[k])[3]=127;
			display_texture(0, w, 0, h, solver.g_rgb, w, h);//display results

			int colorCondition=nExpr[11]>1;
			if(!clearScreen)
			{
				int bkMode=getBkMode();
			//	int H=int(VY-DY/2>0?h:VY+DY/2<0?-1:h*(VY/DY+.5)), HT=int(H>h-30?-18:2), V=int(VX-DX/2>0?-1:VX+DX/2<0?w:w*(-VX+DX/2)/DX), VT=int(V>w-24-prec*8?-24-prec*8:2);
				int H=0, V=0, VT=0;
				_2dMode_NumberAxes(VX, VY, DX, DY, Xstep, Ystep, prec, H, V, VT);
				{
					int textColor=getTextColor();
					int Ys=H+(H>h-46?-34:18)-2, Ys0=Ys;
					for(int kl=0, klEnd=labels.Xlabels.size();kl<klEnd;++kl)
					{
						auto &label=labels.Xlabels[kl];
						setTextColor(0xFF000000|(expr[label.exNo].color&-colorCondition));
						print_array(w-arrow_label_offset_X, Ys, label.label.c_str(), label.label.size()), Ys+=fontH;
					}
					int Xs=	VT+24+8*prec>w-24-prec*8
						?	VT-24-8*prec
						:	VT+24+8*prec;
					//int Xs=	V+VT+24+8*prec>w-24-prec*8
					//	?	V+VT-24-8*prec
					//	:	V+VT+24+8*prec;
					Ys=0;
					for(int kl=0, klEnd=labels.Ylabels.size();kl<klEnd;++kl)
					{
						auto &label=labels.Ylabels[kl];
						setTextColor(0xFF000000|(expr[label.exNo].color&-colorCondition));
						print_array(Xs, Ys, label.label.c_str(), label.label.size()), Ys+=fontH;
					}
					Ys=Ys0>h/3?0:h-16*labels.Clabels.size();
					for(int kl=0, klEnd=labels.Clabels.size();kl<klEnd;++kl)
					{
						auto &label=labels.Clabels[kl];
						setTextColor(0xFF000000|(expr[label.exNo].color&-colorCondition));
						print(w-const_label_offset_X, Ys, "%s=%g", label.label.c_str(), label.isTime?solver.T:label.value);
						Ys+=fontH;
					}
				}
				setBkMode(bkMode);
			}
		}
		void i_draw();
		void a_draw();
	} ti2d;
	const int		Implicit_2D::modes[]={11}, Implicit_2D::nmodes=sizeof(modes)>>2;
	void			Implicit_2D::i_draw()
	{
		if(modes::ready)
		{
			draw();
//			if(showLastModeOnIdle)
//				std::copy(rgb, rgb+w*h, modeRGB);
		}
		else
		{
		//	std::copy(modeRGB, modeRGB+w*h, rgb);
		}
		if(showBenchmark)
			Performance(0, h>>1);//
	}
	void			Implicit_2D::a_draw()
	{
	//	Rectangle(ghMemDC, bpx-1, bpy-1, bw+1, bh+1);

		draw();
		
		if(contextHelp)
		{
			const char *help[]=
			{
				"arrows/drag: move",
				"+/-/enter/backspace/wheel: zoom",
				"x/y +/-/enter/backspace/wheel: scale x/y",
				"E: reset scale",
				"R: reset scale & view",
				"C: toggle clear screen",
				"`: contour",
				"Esc: back to text editor"
			};
			print_contextHelp(help, sizeof(help)>>2, 265);
		}
		if(showBenchmark&&!clearScreen)
			Performance(0, h);//
	//	BitBlt(ghDC, 0, 0, w, h, ghMemDC, 0, 0, SRCCOPY);
	}
	namespace contour
	{
		struct Double_X_Y
		{
			double x, y;
			Double_X_Y(double x, double y):x(x), y(y){}
		};
		struct Double_X_Y_V
		{
			double X, Y, V;
			Double_X_Y_V(double X, double Y, double V):X(X), Y(Y), V(V){}
		};
		void solve_quadratic(double a, double b, double c, std::complex<double> &r1, std::complex<double> &r2)
		{
			if(a==0)
				r1=-c/b, r2=_HUGE;
			else
			{
				if(std::abs(b)/std::abs(a)>=1e10&&std::abs(c)/std::abs(a)>=1e10)
					r1=-c/b, r2=-b/a;
				else
				{
					b/=a, c/=a;
					double first=-b;
					std::complex<double> disc=std::sqrt(b*b-4*c);
					r1=.5*(first+disc), r2=.5*(first-disc);
				}
			}
		}
	}
	class		Transverse_2D:public _3D_Mode
	{
	public:
		int Xoffset, Yoffset, exprRemoved;
		int X0, Y0;

		int Xplaces, Yplaces;
		double XshiftPoint, YshiftPoint;
		double XsamplePos, YsamplePos;

		Solve_2D solver;
		std::list<int> operations;

		bool contourOn, contourOnly, contourFlat;
		std::map<unsigned, std::unordered_map<int, std::list<std::pair<contour::Double_X_Y, contour::Double_X_Y>>>>
			contours;//contours[e]=contour, contour[z level (z/step)]=sticks, *stick.begin()=((x,y),(x,y))
		
		bool clearScreen, kb_VK_F6_msg;
		Labels_2D labels;
		_3D _3d;
		int gridColor;
		static const int modes[], nmodes;

		Transverse_2D():
			solver(true), _3D_Mode(solver),
			Xplaces(100), Yplaces(100),
			XshiftPoint(0), YshiftPoint(0),
			XsamplePos(0), YsamplePos(0),
			_3d(4, 4, 4, 225*G2::_pi/180, 324.7356103172454*G2::_pi/180, 1),
			gridColor(_3dGridColor),
			Xoffset(0), Yoffset(0),
			contourOn(false), contourOnly(false), contourFlat(false),
			clearScreen(false), kb_VK_F6_msg(false)
		{}

		void doContour(unsigned e, double Xs, double Vstart, double Vend, double Vstep, unsigned nzSteps)
		{
			using namespace contour;
			auto &ex=expr[e];
			auto &ndr_r=ex.n[ex.resultTerm].r;
			auto &contour=contours[e];
			unsigned yDiscOffset=(Xplaces-1)*Yplaces;
			double X0, X1, Y0, Y1, V00, V01, V10, V11;
			auto getPos=[&](double &t, double Vx, Double_X_Y_V &A, Double_X_Y_V &B)->bool
			{
				double
					Xd=B.X-A.X, X1a=X1-A.X, Xa0=A.X-X0,
					Yd=B.Y-A.Y, Y1a=Y1-A.Y, Ya0=A.Y-Y0,
					
					A0=(V01-V00)*Xd, B0=X1a*V00+Xa0*V01,
					A1=(V11-V10)*Xd, B1=X1a*V10+Xa0*V11,
					
					a=(A1-A0)*Yd, b=(B1-B0)*Yd+Y1a*A0+Ya0*A1, c=Y1a*B0+Ya0*B1-(X1-X0)*(Y1-Y0)*Vx;

				if(a==0&&b==0)
				{
					t=(Vx-A.V)/(B.V-A.V);
					return true;
				}
				std::complex<double> r1, r2;
				modes::contour::solve_quadratic(a, b, c, r1, r2);
				if(r1.real()>=0&&r1.real()<=1)
				{
					t=r1.real();
					return true;
				}
				if(r2.real()>=0&&r2.real()<=1)
				{
					t=r2.real();
					return true;
				}
				if(r1.real()>=-1e-5&&r1.real()<=1+1e-5)
				{
					t=r1.real();
					return true;
				}
				if(r2.real()>=-1e-5&&r2.real()<=1+1e-5)
				{
					t=r2.real();
					return true;
				}
				if(r1.real()>=-1e-1&&r1.real()<=1+1e-1)
				{
					t=r1.real();
					return true;
				}
				if(r2.real()>=-1e-1&&r2.real()<=1+1e-1)
				{
					t=r2.real();
					return true;
				}
				return false;
			};
			auto cutTrgl=[&](Double_X_Y_V &A, Double_X_Y_V &B, Double_X_Y_V &C)
			{
				modes::contour::Double_X_Y_V *_1, *_2, *_3;//ordered by z in acscending order
				if(A.V<B.V)//ab
				{
						 if(B.V<C.V)	_1=&A, _2=&B, _3=&C;
					else if(A.V<C.V)	_1=&A, _2=&C, _3=&B;
					else				_1=&C, _2=&A, _3=&B;
				}
				else//ba
				{
						 if(A.V<C.V)	_1=&B, _2=&A, _3=&C;
					else if(B.V<C.V)	_1=&B, _2=&C, _3=&A;
					else				_1=&C, _2=&B, _3=&A;
				}
				//for V1 -> V2
				for(double v=std::floor((_1->V<Vstart?Vstart:_1->V)/Vstep), vEnd=std::floor((_2->V>Vend?Vend:_2->V)/Vstep);v<=vEnd;++v)
				{
					double VL=Vstep*v;
					if(_1->V<=VL&&VL<_2->V)
					{
						double M12, M13;
						if(getPos(M12, VL, *_1, *_2)&&getPos(M13, VL, *_1, *_3))
						{
							contour[int(v)].push_back(std::pair<modes::contour::Double_X_Y, modes::contour::Double_X_Y>(
								modes::contour::Double_X_Y(_1->X+M12*(_2->X-_1->X), _1->Y+M12*(_2->Y-_1->Y)),
								modes::contour::Double_X_Y(_1->X+M13*(_3->X-_1->X), _1->Y+M13*(_3->Y-_1->Y))
								));
						}
					}
				}
				//for V2 -> V3
				for(double v=std::floor((_2->V<Vstart?Vstart:_2->V)/Vstep), vEnd=std::floor((_3->V>Vend?Vend:_3->V)/Vstep);v<=vEnd;++v)
				{
					double VL=Vstep*v;
					if(_2->V<=VL&&VL<_3->V)
					{
						double M32, M31;
						if(getPos(M32, VL, *_2, *_3)&&getPos(M31, VL, *_1, *_3))
						{
							contour[int(v)].push_back(std::pair<modes::contour::Double_X_Y, modes::contour::Double_X_Y>(
								modes::contour::Double_X_Y(_2->X+M32*(_3->X-_2->X), _2->Y+M32*(_3->Y-_2->Y)),
								modes::contour::Double_X_Y(_1->X+M31*(_3->X-_1->X), _1->Y+M31*(_3->Y-_1->Y))
								));
						}
					}
				}
			};
			double Xstart=solver.Xstart, Ystart=AR_Y*solver.Ystart;
			for(unsigned vy=0, vyEnd=Yplaces-1;vy<vyEnd;++vy)
			{
				Y0=Ystart+Xs*vy, Y1=Ystart+Xs*(vy+1);
				for(unsigned vx=0, vxEnd=Xplaces-1;vx<vxEnd;++vx)
				{
					X0=solver.Xstart+Xs*vx, X1=solver.Xstart+Xs*(vx+1);
					double Xm=solver.Xstart+Xs*(vx+.5), Ym=Ystart+Xs*(vy+.5);

					Double_X_Y_V//yx
						P00(X0, Y0, V00=ndr_r[Xplaces* vy   +vx  ]), P01(X1, Y0, V01=ndr_r[Xplaces* vy   +vx+1]),
						P10(X0, Y1, V10=ndr_r[Xplaces*(vy+1)+vx  ]), P11(X1, Y1, V11=ndr_r[Xplaces*(vy+1)+vx+1]),
						
						Pmm(Xm, Ym, .25*(V00+V01+V10+V11));

					if(!ex.discontinuities[(Xplaces-1)*vy+vx]&&!ex.discontinuities[yDiscOffset+(Yplaces-1)*(vx+1)+vy]&&!ex.discontinuities[(Xplaces-1)*(vy+1)+vx]&&!ex.discontinuities[yDiscOffset+(Yplaces-1)*vx+vy])
					{
						cutTrgl(P00, Pmm, P01);
						cutTrgl(P01, Pmm, P11);
						cutTrgl(P11, Pmm, P10);
						cutTrgl(P10, Pmm, P00);
					}
				/*	if(!ex.discontinuities[(Xplaces-1)*vy+vx])
						cutTrgl(P00, Pmm, P01);
					if(!ex.discontinuities[yDiscOffset+(Yplaces-1)*(vx+1)+vy])
						cutTrgl(P01, Pmm, P11);
					if(!ex.discontinuities[(Xplaces-1)*(vy+1)+vx])
						cutTrgl(P11, Pmm, P10);
					if(!ex.discontinuities[yDiscOffset+(Yplaces-1)*vx+vy])
						cutTrgl(P10, Pmm, P00);//*/
				}
			}//*/
		}
		void differentiate_x					(Term &n)
		{
			auto &ndr_r=n.r;
			double _1_step=Xplaces/DX;
			for(int ky=0;ky<Yplaces;++ky)
			{
				for(int kx=0;kx<Xplaces-1;++kx)
					ndr_r[Xplaces*ky+kx]=(ndr_r[Xplaces*ky+kx+1]-ndr_r[Xplaces*ky+kx])*_1_step;
				ndr_r[Xplaces*ky+Xplaces-1]*=-_1_step;
			}
			ndr_r[Xplaces-1]*=-_1_step;
		}
		void differentiate_y					(Term &n)
		{
			auto &ndr_r=n.r;
			double _1_step=Xplaces/DX;
			for(int ky=0;ky<Yplaces-1;++ky)
				for(int kx=0;kx<Xplaces;++kx)
					ndr_r[Xplaces*ky+kx]=(ndr_r[Xplaces*(ky+1)+kx]-ndr_r[Xplaces*ky+kx])*_1_step;
			for(int kx=0;kx<Xplaces;++kx)
				ndr_r[Xplaces*(Yplaces-1)+kx]*=-_1_step;
			ndr_r[Xplaces-1]*=-_1_step;
		}
		void integrate_x						(Term &n)
		{
			auto &ndr_r=n.r;
			double step=DX/Xplaces;
			double sum=0;
			for(int ky=0;ky<Yplaces;++ky)
			{
				for(int kx=0;kx<Xplaces;++kx)
				{
					auto &v=ndr_r[Xplaces*ky+kx];
					v=(sum+=v)*step;
				}
				sum=0;
			}
		}
		void integrate_y						(Term &n)
		{
			auto &ndr_r=n.r;
			double step=DX/Xplaces;
			std::vector<double> sum(Xplaces, 0);
			for(int ky=0;ky<Yplaces;++ky)
				for(int kx=0;kx<Xplaces;++kx)
				{
					auto &v=ndr_r[Xplaces*ky+kx];
					v=(sum[kx]+=v)*step;
				}
		}
	//	fftw_complex *fft_in, *fft_out;
	//	fftw_plan fft_p, ifft_p;
		unsigned fft_N0, fft_N1;
		double fft_sqrt_N;
		void discreteFourrierTransform			(Term &n)
		{
			if(n.i.size()!=n.r.size())
				n.i.resize(n.r.size());
		//	fft_2d_forward((double*)n.r.p, (double*)n.i.p, Xplaces, Yplaces, fft_N0, fft_N1, fft_sqrt_N, fft_in, fft_out, fft_p, ifft_p);
		}
		void inverseDiscreteFourrierTransform	(Term &n)
		{
			if(n.i.size()!=n.r.size())
				n.i.resize(n.r.size());
		//	fft_2d_inverse((double*)n.r.p, (double*)n.i.p, Xplaces, Yplaces, fft_N0, fft_N1, fft_sqrt_N, fft_in, fft_out, fft_p, ifft_p);
		}
		void lowPassFilter						(Term &n){lpf_2d(n, Xplaces, Yplaces);}
		void highPassFilter						(Term &n){hpf_2d(n, Xplaces, Yplaces);}

		void setDimensions(int x, int y, int w, int h)
		{
			bpx=x, bpy=y, bw=w, bh=h, X0=bpx+bw/2, Y0=bpy+bh/2;
			_3d.setDimensions(x, y, w, h);
			double old_Zstep=Zstep;
			function1();
			if(!toSolve&&contourOn&&old_Zstep!=Zstep)
			{
				contours.clear();
				double Xs=DX/Xplaces, DZ=DX/AR_Z;
				for(unsigned e=0;e<expr.size();++e)
					if(expr[e].rmode[0]==5)
						doContour(e, Xs, VZ-DZ/2, VZ+DZ/2, Zstep, 10);
			}
			ready=true;
		}
		void shiftNDR(double &DshiftPoint, double Dsample, double &DsamplePos, double &VD, int &Doffset, double ammount)
		{
			double newDsamplePos=std::floor((DshiftPoint+=ammount)/Dsample);
			if(newDsamplePos!=DsamplePos)
			{
				toSolve=true;
				if(shiftOnly)
					shiftOnly=1, Doffset+=int(newDsamplePos-DsamplePos);
				DsamplePos=newDsamplePos, VD=newDsamplePos*Dsample;
			}
		}
		void shiftNDRupdate(double DshiftPoint, double Dsample, double &DsamplePos, double &VD)
		{
			double newDsamplePos=std::floor(DshiftPoint/Dsample);
			if(newDsamplePos!=DsamplePos)
				DsamplePos=newDsamplePos, VD=newDsamplePos*Dsample;
		}
		void messageTimer()
		{
			a_draw();
//			if(!time_variance&&!kp)
//				KillTimer(ghWnd, 0), timer=false;
		}
		int inputTouchDown(int idx)
		{
			auto &ti=touchInfo[idx];
			auto &pos=ti.pos;
			int X0=w>>1, Y0=h>>1;
			if(pos.y>Y0)
			{
				if(pos.x<X0)
					return BOTTOM_LEFT_MOVE;
				return BOTTOM_RIGHT_TURN;
			}
			if(reset_button.click(pos))
			{
				_3d.cam.reset();
				DX=20, AR_Y=1, AR_Z=1, function1();
				VX=VY=VZ=0;
			//	_3d.teleport_degrees(4, 4, 4, 225, 324.7356103172454, 1);
				return BUTTON_RESET;
			}
			return TOUCH_MISS;
		}
		void inputTouchUp(int idx)
		{
		}
		void inputTouchMove(int idx)
		{
			auto &ti=touchInfo[idx];
			if(ti.region==BOTTOM_RIGHT_TURN)
				_3d.cam.turnBy(ti.delta.x*5/w, ti.delta.y*5/w);
		}

		void draw()
		{
			int X0=w>>1, Y0=h>>1;
			for(int k=0, kEnd=touchInfo.size();k<kEnd;++k)//move camera
			{
				auto &ti=touchInfo[k];
				if(ti.region==BOTTOM_LEFT_MOVE)
				{
					int X00=w>>2, Y00=h*3>>2;
					float dx=float(ti.pos.x-X00)/w, dy=-float(ti.pos.y-Y00)/w;

					_3d.cam.move(dx, dy);
					break;
				}
			}
			double DY=DX/AR_Y, DZ=DX/AR_Z;
			if(toSolve)
			{
				if(!operations.size()&&shiftOnly==1&&abs(Xoffset)<Xplaces&&abs(Yoffset)<Yplaces)
				{
					if(Xoffset||Yoffset)
					{
					//	GUIPrint(ghMemDC, w-200, h/2, "Xoffset=%d", Xoffset);//
					//	GUIPrint(ghMemDC, w-200, h/2+18, "Yoffset=%d", Yoffset);//
						solver.partial_bounds(VX, DX, VY/AR_Y, DY, Xoffset, Yoffset);
						solver.synchronize();
						for(int ke=0, keEnd=expr.size();ke<keEnd;++ke)
						{
							auto &ex=expr[ke];
							if(ex.rmode[0]==5)
								(solver.*(ex.nITD?&Solve_2D::full:solver.partial))(ex);
						}
					}
				}
				else
				{
					auto old_time_variance=time_variance;
					time_variance=false;
					labels.clear();
					solver.full_resize(VX, DX, VY/AR_Y, DY, Xplaces, Yplaces);
					XsamplePos=std::floor(XshiftPoint/solver.Xsample);
					YsamplePos=std::floor(YshiftPoint/(solver.Ysample*AR_Y));
				//	solver.full_resize(VX, DX, VY/AR_Y, DY, Xplaces=100, Yplaces=100), KXplaces=Xplaces/100?Xplaces/100:1, KYplaces=Yplaces/100?Yplaces/100:1;
					if(!paused)
						solver.synchronize();
					for(unsigned e=0;e<expr.size();++e)
					{
						auto &ex=expr[e];
						if(ex.rmode[0]==5)
						{
							time_variance|=ex.nITD;
							labels.fill(e);
							solver.full(ex);
							for(auto it=operations.begin();it!=operations.end();++it)
							{
								auto &operation=*it;
								switch(operation)
								{
								case  1:differentiate_x					(ex.n[ex.resultTerm]);break;
								case  2:differentiate_y					(ex.n[ex.resultTerm]);break;
								case  3:integrate_x						(ex.n[ex.resultTerm]);break;
								case  4:integrate_y						(ex.n[ex.resultTerm]);break;
								case  5:discreteFourrierTransform		(ex.n[ex.resultTerm]);break;
								case  6:inverseDiscreteFourrierTransform(ex.n[ex.resultTerm]);break;
								case  7:lowPassFilter					(ex.n[ex.resultTerm]);break;
								case  8:highPassFilter					(ex.n[ex.resultTerm]);break;
								}
							}
						}
					}
//					if(time_variance)
//					{
//						if(!paused)
//							SetTimer(ghWnd, 0, 10, 0);
//					}
//					else if(old_time_variance&&!timer)
//						KillTimer(ghWnd, 0);
				}
				if(contourOn)
				{
					contours.clear();
					double Xs=DX/Xplaces, DZ=DX/AR_Z;
					for(unsigned e=0;e<expr.size();++e)
						if(expr[e].rmode[0]==5)
							doContour(e, Xs, VZ/AR_Z-DZ/2, VZ/AR_Z+DZ/2, Zstep, 10);
				}
				toSolve=false, shiftOnly=2, Xoffset=0, Yoffset=0;
				exprRemoved=false;
			}
			else if(time_variance)
			{
				if(!paused)
					solver.synchronize();
				for(int ke=0, keEnd=expr.size();ke<keEnd;++ke)
				{
					auto &ex=expr[ke];
					if(ex.rmode[0]==5&&ex.nITD)
					{
						solver.full(ex);
						for(auto it=operations.begin();it!=operations.end();++it)
						{
							auto &operation=*it;
							switch(operation)
							{
							case  1:differentiate_x					(ex.n[ex.resultTerm]);break;
							case  2:differentiate_y					(ex.n[ex.resultTerm]);break;
							case  3:integrate_x						(ex.n[ex.resultTerm]);break;
							case  4:integrate_y						(ex.n[ex.resultTerm]);break;
							case  5:discreteFourrierTransform		(ex.n[ex.resultTerm]);break;
							case  6:inverseDiscreteFourrierTransform(ex.n[ex.resultTerm]);break;
							case  7:lowPassFilter					(ex.n[ex.resultTerm]);break;
							case  8:highPassFilter					(ex.n[ex.resultTerm]);break;
							}
						}
					}
				}
				if(contourOn)
				{
					double Xs=DX/Xplaces, DZ=DX/AR_Z;
					for(unsigned e=0;e<expr.size();++e)
					{
						auto &ex=expr[e];
						if(ex.rmode[0]==5&&ex.nITD)
						{
							contours[e].clear();
							doContour(e, Xs, VZ/AR_Z-DZ/2, VZ/AR_Z+DZ/2, Zstep, 10);
						}
					}
				}
			}
			else if(exprRemoved)
			{
				auto old_time_variance=time_variance;
				time_variance=false;
				labels.clear();
				for(unsigned e=0;e<expr.size();++e)
				{
					auto &ex=expr[e];
					if(ex.rmode[0]==5)
					{
						time_variance|=ex.nITD;
						labels.fill(e);
					}
				}
//				if(old_time_variance&&!time_variance&&!timer)
//					KillTimer(ghWnd, 0);
				exprRemoved=false;
			}

			GL2_3D::begin();
			_3dMode_FrameStart(_3d, VX, VY, VZ, DX, Xstep, Ystep, Zstep, AR_Y, AR_Z);
			int colorCondition=nExpr[5]>1;
			double Xstart=solver.Xstart, Xr=solver.Xsample, Ystart=AR_Y*solver.Ystart, Yr=AR_Y*solver.Ysample;
			int ndrSize=Xplaces*Yplaces;
			if(!contourOnly)//draw the surfaces
			{
				if(operations.size())
				{
					for(int ke=0, keEnd=expr.size();ke<keEnd;++ke)
					{
						auto &ex=expr[ke];
						if(ex.rmode[0]==5)
						{
							_3d.lineColor=0xFF000000|(ex.color&-colorCondition);
							auto &ndr=ex.n[ex.resultTerm].r;
							for(int y=0;y<Yplaces;++y)
							{
								GL2_3D::curve_start(_3d.lineColor);
								for(int x=0;x<Xplaces;++x)
								{
									if(ndr.size()!=ndrSize)
										return;
									GL2_3D::curve_point(vec3(Xstart+x*Xr, Ystart+y*Yr, AR_Z*ndr[Xplaces*y+x]));
								}
							}
							for(int x=0;x<Xplaces;++x)
							{
								GL2_3D::curve_start(_3d.lineColor);
								for(int y=0;y<Yplaces;++y)
								{
									if(ndr.size()!=ndrSize)
										return;
									GL2_3D::curve_point(vec3(Xstart+x*Xr, Ystart+y*Yr, AR_Z*ndr[Xplaces*y+x]));
								}
							}
						}
					}
				}
				else
				{
					unsigned yDiscOffset=(Xplaces-1)*Yplaces;
					for(int ke=0, keEnd=expr.size();ke<keEnd;++ke)
					{
						auto &ex=expr[ke];
						if(ex.rmode[0]==5)
						{
							_3d.lineColor=0xFF000000|(ex.color&-colorCondition);
							auto &ndr=ex.n[ex.resultTerm].r;
							for(int y=0;y<Yplaces;++y)
							{
								GL2_3D::curve_start(_3d.lineColor);
								for(int x=0;x<Xplaces-1;++x)
								{
									if(ndr.size()!=ndrSize)
										return;
									GL2_3D::curve_point(vec3(Xstart+x*Xr, Ystart+y*Yr, AR_Z*ndr[Xplaces*y+x]));
									if(ex.discontinuities[(Xplaces-1)*y+x])
										GL2_3D::curve_start(_3d.lineColor);
								}
								GL2_3D::curve_point(vec3(Xstart+(Xplaces-1)*Xr, Ystart+y*Yr, AR_Z*ndr[Xplaces*y+(Xplaces-1)]));
							}
							for(int x=0;x<Xplaces;++x)
							{
								GL2_3D::curve_start(_3d.lineColor);
								for(int y=0;y<Yplaces-1;++y)
								{
									if(ndr.size()!=ndrSize)
										return;
									GL2_3D::curve_point(vec3(Xstart+x*Xr, Ystart+y*Yr, AR_Z*ndr[Xplaces*y+x]));
									if(ex.discontinuities[yDiscOffset+(Yplaces-1)*x+y])
										GL2_3D::curve_start(_3d.lineColor);
								}
								GL2_3D::curve_point(vec3(Xstart+x*Xr, Ystart+(Yplaces-1)*Yr, AR_Z*ndr[Xplaces*(Yplaces-1)+x]));
							}
						}
					}
				}
			}
			if(contourOn)//draw the contour
			{
			//	double Xstart=VX-DX/2;
			//	double Ystart=VY/AR_Y-DY/2;
			//	double Zstart=VZ/AR_Z-DX/2, Zend=VZ/AR_Z+DX/2;
				for(unsigned e=0;e<expr.size();++e)
				{
					auto &ex=expr[e];
					if(ex.rmode[0]==5)
					{
						if(colorCondition)
							_3d.lineColor=ex.color;
						auto &contour=contours[e];
						for(auto cit=contour.begin();cit!=contour.end();++cit)
						{
							auto &L=*cit;
							double Z=contourFlat?VZ:L.first*Zstep;
							for(auto xit=L.second.begin();xit!=L.second.end();++xit)
							{
								auto &p=*xit;
								GL2_3D::push_line_segment(vec3(p.first.x, p.first.y, AR_Z*Z), vec3(p.second.x, p.second.y, AR_Z*Z), _3d.lineColor);
							}
						}
					}
				}
			}
			if(!clearScreen)
			{
				{
					int bkMode=setBkMode(TRANSPARENT);
					for(double X=floor((VX-DX/2)/Xstep)*Xstep+Xstep, Xend=ceil((VX+DX/2)/Xstep)*Xstep;X<Xend;X+=Xstep)
						if(abs(X)>Xstep/2)
							_3d.label(X, VY, VZ, "%g", X);
					if(AR_Y==1)
					{
						for(double Y=floor((VY-DX/2)/Ystep)*Ystep+Ystep, Yend=ceil((VY+DX/2)/Ystep)*Ystep;Y<Yend;Y+=Ystep)
							if(Y<-Ystep/2||Y>Ystep/2)
								_3d.label(VX, Y, VZ, "%g", Y);
					}
					else
					{
						double aYstep=AR_Y*Ystep, Ycore=floor((VY-DX/2)/aYstep);
						for(double Y=Ycore*aYstep+aYstep, Yend=ceil((VY+DX/2)/aYstep)*aYstep, aY=Ycore*Ystep+Ystep;Y<Yend;Y+=aYstep, aY+=Ystep)
							if(Y<-aYstep/2||Y>aYstep/2)
								_3d.label(VX, Y, VZ, "%g", aY);
					}
					if(AR_Z==1)
					{
						for(double Z=floor((VZ-DX/2)/Zstep)*Zstep+Zstep, Zend=ceil((VZ+DX/2)/Zstep)*Zstep;Z<Zend;Z+=Zstep)
							if(Z<-Zstep/2||Z>Zstep/2)
								_3d.label(VX, VY, Z, "%g", Z);
					}
					else
					{
						double aZstep=AR_Z*Zstep, Zcore=floor((VZ-DX/2)/aZstep);
						for(double Z=Zcore*aZstep+aZstep, Zend=ceil((VZ+DX/2)/aZstep)*aZstep, aZ=Zcore*Zstep+Zstep;Z<Zend;Z+=aZstep, aZ+=Zstep)
							if(Z<-aZstep/2||Z>aZstep/2)
								_3d.label(VX, VY, Z, "%g", aZ);
					}
					{
						auto al=colorCondition?(void (_3D::*)(real, real, real, real, real, real, std::vector<Label>&))&_3D::arrowLabelsColor:&_3D::arrowLabels;
					//	void (_3D_API::*al)(double, double, double, double, double, double, std::vector<Label>&);
					//	if(colorCondition)
					//		al=&_3D_API::arrowLabelsColor;
					//	else
					//		al=&_3D_API::arrowLabels;
					//	auto LOL=colorCondition?&_3D_API::arrowLabelsColor:&_3D_API::arrowLabels;
						(_3d.*al)(0, 0, 0, 1, 0, 0, labels.Xlabels);
						(_3d.*al)(0, 0, 0, 0, 1, 0, labels.Ylabels);
					}
					{
						int textColor=getTextColor();
						int Ys=0;
						for(int kl=0, klEnd=labels.Clabels.size();kl<klEnd;++kl)
						{
							auto &label=labels.Clabels[kl];
							setTextColor(0xFF000000|(expr[label.exNo].color&-colorCondition));
							print(w-const_label_offset_X, Ys, "%s=%g", label.label.c_str(), label.isTime?solver.T:label.value);
							Ys+=fontH;
						}
					}
					setBkMode(bkMode);
				}
				{
					int k=0, Y=h-operations.size()*16;
					for(auto it=operations.begin();it!=operations.end();++it)
					{
						auto &operation=*it;
						char const *a=0;
						switch(operation)
						{
						case 1:a="%d: Differentiate X";	break;
						case 2:a="%d: Differentiate Y";	break;
						case 3:a="%d: Integrate X";		break;
						case 4:a="%d: Integrate Y";		break;
						case 5:a="%d: DFT";				break;
						case 6:a="%d: IDFT";			break;
						case 7:a="%d: LPF";				break;
						case 8:a="%d: HPF";				break;
						case 9:a="%d: HT";				break;
						case 10:a="%d: IHT";			break;
						}
						_3d.textIn2D(w-const_label_offset_X, Y, OPAQUE, a, k);
						++k, Y+=16;
					}
				}
				_3d.text_show();
			}
			GL2_3D::end();
			GL2_3D::draw(_3d.cam);
		}
		void i_draw();
		void a_draw();
	} t2d;
	const int		Transverse_2D::modes[]={5}, Transverse_2D::nmodes=sizeof(modes)>>2;
	void			Transverse_2D::i_draw()
	{
		if(modes::ready)
		{
			draw();
//			if(showLastModeOnIdle)
//				std::copy(rgb, rgb+w*h, modeRGB);
		}
		else
		{
		//	std::copy(modeRGB, modeRGB+w*h, rgb);
		}
		if(showBenchmark)
			Performance(0, h>>1);//
	}
	void			Transverse_2D::a_draw()
	{
	//	Rectangle(ghMemDC, bpx-1, bpy-1, bw+1, bh+1);

		draw();
		
		if(contextHelp)
		{
			const char *help[]=
			{
				"W/A/S/D/T/G: move",
				"arrows: turn",
				"+/-/enter/backspace/wheel: change FOV",
				"X/Y/Z arrows/mouse move: shift x/y/z",
				"X/Y/Z +/-/enter/backspace/wheel: scale x/y/z",
				"alt +/-/enter/backspace/wheel: zoom",
				"shift +/-/enter/backspace/wheel: change resolution",
				"E: reset scale",
				"R: reset scale & view",
				"C: toggle clear screen",
				"1: differentiate x",
				"2: differentiate y",
				"3: integrate x",
				"4: integrate y",
				"5: DFT",
				"6: Inverse DFT",
				"7: LPF",
				"8: HPF",
				"0: reset operations",
				"`: contour",
				"Esc: back to text editor"
			};
			print_contextHelp(help, sizeof(help)>>2, 330);
		}
		if(showBenchmark&&!clearScreen)
			Performance(0, h);//
	//	BitBlt(ghDC, 0, 0, w, h, ghMemDC, 0, 0, SRCCOPY);
	}
	namespace contour
	{
		struct Int_x_y_z_Double_Z
		{
			int x, y, c;
			double Z;
			Int_x_y_z_Double_Z(int x, int y, int c, double Z):x(x), y(y), c(c), Z(Z){}
		};
	}
	int drag_counter=0, disp_x=0, disp_y=0;//debug counter
	class		Color_2D:public _2D_Mode
	{
	public:
		int Xoffset, Yoffset;

		int Xplaces, Yplaces;
		Solve_C2D solver;
		std::list<int> operations;

		bool contourOn, contourOnly;//, contourFlat;
		
		bool clearScreen;
		Labels_2D labels;
	//	HPEN__ *hPen;
	//	HBRUSH__ *hBrush;
		static const int modes[], nmodes;

		Color_2D():
			_2D_Mode(solver),
			Xoffset(0), Yoffset(0),
			clearScreen(false),
			contourOnly(false)//, contourFlat(true)
		{
		//	hPen=CreatePen(PS_SOLID, 1, 0xFFEFEFEF), hBrush=CreateSolidBrush(0xFFEFEFEF);
		}
		
		void doContour_component_range(std::vector<double> &ndr, double Xs, double Zstart, double Zend, double Zstep, unsigned nzSteps, unsigned const component, int x1, int x2, int y1, int y2)
		{
			int dx=x2-x1, dy=y2-y1, x11=x1+1, y11=y1+1, x2_1=x2-1, y2_1=y2-1;
			if(dx>=3&&dy>=3)
			{
				const double aa_thickness=1,
				
					_1_aa_thickness=1/aa_thickness;
				const int aa_bound=int(std::round(aa_thickness));

				//crossings
				int Xplaces_1=Xplaces-1, Yplaces_1=Yplaces-1,
					XCsize=Xplaces_1*Yplaces, YCsize=Yplaces_1*Xplaces, ndrSize=Xplaces*Yplaces,
					Xplaces1=Xplaces+aa_bound;
				std::vector<double>
					Xcross(XCsize, -1),//._
					Ycross(YCsize, -1);//!
				std::vector<char> shade(Xplaces1*(Yplaces+aa_bound));
				{
					// _	top left
					//|_!
					auto&v00=ndr[Xplaces*y1	+x1], &v01=ndr[Xplaces*y1	+x11],
						&v10=ndr[Xplaces*y11+x1], &v11=ndr[Xplaces*y11+x11];
					auto&corner=shade[Xplaces1*y11+x11];
					if(std::signbit(v00)!=std::signbit(v01))
						corner=true, Xcross[Xplaces_1*y1+x1]=(0-v00)/(v01-v00);		//zero cross
					if(std::signbit(v10)!=std::signbit(v11))
						corner=true, Xcross[Xplaces_1*y11+x1]=(0-v10)/(v11-v10);
					if(std::signbit(v00)!=std::signbit(v10))
						corner=true, Ycross[Xplaces*y1+x1]=(0-v00)/(v10-v00);
					if(std::signbit(v01)!=std::signbit(v11))
						corner=true, Ycross[Xplaces*y1+x11]=(0-v01)/(v11-v01);
				}
				{
					// _	top right
					//._|
					auto&v00=ndr[Xplaces*y1	+x2_1-1], &v01=ndr[Xplaces*y1	+x2_1],
						&v10=ndr[Xplaces*y11+x2_1-1], &v11=ndr[Xplaces*y11+x2_1];
					auto&corner=shade[Xplaces1*y11+x2_1-1];
					if(std::signbit(v00)!=std::signbit(v01))
						corner=true, Xcross[Xplaces_1*y1	+x2_1-1]=(0-v00)/(v01-v00);		//zero cross
					if(std::signbit(v10)!=std::signbit(v11))
						corner=true, Xcross[Xplaces_1*y11	+x2_1-1]=(0-v10)/(v11-v10);
					if(std::signbit(v01)!=std::signbit(v11))
						corner=true, Ycross[Xplaces*y11+	x2_1]=(0-v01)/(v11-v01);
				}
				{
					//  .	bottom left
					//|_|
					auto&v00=ndr[Xplaces*(y2_1-1)	+x1], &v01=ndr[Xplaces*(y2_1-1)	+x11],
						&v10=ndr[Xplaces* y2_1		+x1], &v11=ndr[Xplaces* y2_1		+x11];
					auto&corner=shade[Xplaces1*(y2_1-1)+x11];
					if(std::signbit(v10)!=std::signbit(v11))
						corner=true, Xcross[Xplaces_1*y2_1	+x1]=(0-v10)/(v11-v10);		//zero cross
					if(std::signbit(v00)!=std::signbit(v10))
						corner=true, Ycross[Xplaces*(y2_1-1)	+x1]=(0-v00)/(v10-v00);
					if(std::signbit(v01)!=std::signbit(v11))
						corner=true, Ycross[Xplaces*(y2_1-1)	+x11]=(0-v01)/(v11-v01);
				}
				for(int x=x11, xEnd=x2_1-1;x<xEnd;++x)
				{
					// _	upper row
					//._!
					auto&v00=ndr[Xplaces*y1	+x], &v01=ndr[Xplaces*y1	+x+1],
						&v10=ndr[Xplaces*y11+x], &v11=ndr[Xplaces*y11	+x+1];
					auto&p0=shade[Xplaces1*y11+x], &p1=shade[Xplaces1*y11+x+1];
					if(std::signbit(v00)!=std::signbit(v01))
						p0=p1=true, Xcross[Xplaces_1*y1	+x]=(0-v00)/(v01-v00);		//zero cross
					if(std::signbit(v10)!=std::signbit(v11))
						p0=p1=true, Xcross[Xplaces_1*y11+x]=(0-v10)/(v11-v10);
					if(std::signbit(v01)!=std::signbit(v11))
						p0=p1=true, Ycross[Xplaces*y1	+x+1]=(0-v01)/(v11-v01);
				}
				for(int y=y11, yEnd=y2_1-1;y<yEnd;++y)
				{
					//  .	left column
					//|_!
					auto&v00=ndr[Xplaces* y   +x1], &v01=ndr[Xplaces* y   +x11],
						&v10=ndr[Xplaces*(y+1)+x1], &v11=ndr[Xplaces*(y+1)+x11];
					auto&p0=shade[Xplaces1*y+x11], &p1=shade[Xplaces1*(y+1)+x11];
					if(std::signbit(v10)!=std::signbit(v11))
						p0=p1=true, Xcross[Xplaces_1*(y+1)	+x1]=(0-v10)/(v11-v10);		//zero cross
					if(std::signbit(v00)!=std::signbit(v10))
						p0=p1=true, Ycross[Xplaces*y	+x1]=(0-v00)/(v10-v00);
					if(std::signbit(v01)!=std::signbit(v11))
						p0=p1=true, Ycross[Xplaces*y	+x11]=(0-v01)/(v11-v01);
				}
				auto aa_straight=[&](double d){return d>aa_thickness?1:d*_1_aa_thickness;};
				auto aa_close=[&](double x, double y)->double
				{
					double d=x*y*inv_sqrt(x*x+y*y);
				//	double d=std::abs(x*y)*inv_sqrt(x*x+y*y);
					return d>aa_thickness?1:d*_1_aa_thickness;
				};
				auto aa_middle=[&](double a, double b)->double//|. |
				{
					double b_a=b-a, d=a*inv_sqrt(1+b_a*b_a);
					return d>aa_thickness?1:d*_1_aa_thickness;
				};
				auto aa_far=[&](double x, double y)->double
				{
					double x_1=x-1, y_1=y-1;
					double d=std::abs(x*y-1)*inv_sqrt(x_1*x_1+y_1*y_1);
					return d>aa_thickness?1:d*_1_aa_thickness;
				};
				int complement=component?0xFF00FF00:0xFF0000FF;
			//	int complement=component?0xFFFFFFFF:0xFF000000;
				int *&_rgb=solver.rgb;
				auto ApplyDIBPixelUnchecked=[&](int x, int y, double a)
				{
					auto p=(unsigned char*)&_rgb[y*w+x];
				//	auto p=(unsigned char*)&solver.rgb[y*w+x];
					*(int*)p^=complement;
					p[0]=(unsigned char)(p[0]*a), p[1]=(unsigned char)(p[1]*a), p[2]=(unsigned char)(p[2]*a);
					*(int*)p^=complement;
				};
				for(int y=y1+aa_bound, yEnd=y2_1;y<yEnd;++y)
				{
					for(int x=x1+aa_bound, xEnd=x2_1;x<xEnd;++x)
					{
						int ndrP=Xplaces*y+x;
						int sp=Xplaces1*y+x;
						{
							auto						&v01=ndr[ndrP			+1],
								&v10=ndr[ndrP+Xplaces], &v11=ndr[ndrP+Xplaces	+1];
							if(std::signbit(v10)!=std::signbit(v11))
							{
								Xcross[Xplaces_1*(y+1)+x]=(0-v10)/(v11-v10);	//zero cross
								shade[sp				]=true, shade[sp			+1	]=true;
								shade[sp+Xplaces1		]=true, shade[sp+Xplaces1	+1	]=true;
								shade[sp+Xplaces1*2		]=true, shade[sp+Xplaces1*2	+1	]=true;
							}
							if(std::signbit(v01)!=std::signbit(v11))
							{
								Ycross[ndrP+1]=(0-v01)/(v11-v01);
								shade[sp				]=true, shade[sp			+1	]=true, shade[sp			+2	]=true;
								shade[sp+Xplaces1		]=true, shade[sp+Xplaces1	+1	]=true, shade[sp+Xplaces1	+2	]=true;
							}
						}
						if(shade[sp])
						{
							//	 	Dx
							//	Lx	+	Rx
							//		Ux  '
							auto		&Dx=Ycross[Xplaces*(y-1)+x],
								&Lx=Xcross[Xplaces_1*y+x-1],	&Rx=Xcross[Xplaces_1*y+x],
										&Ux=Ycross[Xplaces* y   +x];
							if(Ux!=-1)
							{
								if(Dx!=-1)
								{
									if(Lx!=-1)
									{
										if(Rx!=-1)	//udlr	>=2 lines	\+\	/+/
										{
											double d, d_min=aa_close(Rx, Ux);
											if(d_min>(d=aa_close(Rx, 1-Dx)))
												d_min=d;
											if(d_min>(d=aa_close(1-Lx, 1-Dx)))
												d_min=d;
											if(d_min>(d=aa_close(1-Lx, Ux)))
												d_min=d;
											ApplyDIBPixelUnchecked(x, y, d_min);
										}
										else		//udl	>=2 lines	<+
										{
											double d, d_min=aa_close(1-Lx, 1-Dx);
											if(d_min>(d=aa_close(1-Lx, Ux)))
												d_min=d;
											ApplyDIBPixelUnchecked(x, y, d_min);
										}
									}
									else
									{
										if(Rx!=-1)	//ud r	>=2 lines	+>
										{
											double d, d_min=aa_close(Rx, Ux);
											if(d_min>(d=aa_close(Rx, 1-Dx)))
												d_min=d;
											ApplyDIBPixelUnchecked(x, y, d_min);
										}
										else		//ud				_F_
										{
											double d, d_min=Ux;
											if(d_min>(d=1-Dx))
												d_min=d;
											ApplyDIBPixelUnchecked(x, y, d_min);
										}
									}
								}
								else
								{
									if(Lx!=-1)
									{
										if(Rx!=-1)	//u lr	>=2 lines	\+/
										{
											double d, d_min=aa_close(Rx, 1-Dx);
											if(d_min>(d=aa_close(1-Lx, 1-Dx)))
												d_min=d;
											ApplyDIBPixelUnchecked(x, y, d_min);
										}
										else		//u l	>=1 line	\+
											ApplyDIBPixelUnchecked(x, y, aa_close(1-Lx, Ux));
									}
									else
									{
										if(Rx!=-1)	//u  r	>=1 line	+/
											ApplyDIBPixelUnchecked(x, y, aa_close(Rx, Ux));
										else		//u		>=1 line	_+_
										{
											auto &LUx=Ycross[Xplaces* y   +x-1], &RUx=Ycross[Xplaces* y   +x+1];
											double d, d_min=Ux;
											if(LUx!=-1&&LUx<Ux&&d_min>(d=aa_middle(Ux, LUx)))
												d_min=d;
											if(RUx!=-1&&RUx<Ux&&d_min>(d=aa_middle(Ux, RUx)))
												d_min=d;
											ApplyDIBPixelUnchecked(x, y, d_min);
										}
									}
								}
							}
							else
							{
								if(Dx!=-1)
								{
									if(Lx!=-1)
									{
										if(Rx!=-1)	// dlr	>=2 lines	/+\ stop
										{
											double d, d_min=aa_close(Rx, Ux);
											if(d_min>(d=aa_close(1-Lx, Ux)))
												d_min=d;
											ApplyDIBPixelUnchecked(x, y, d_min);
										}
										else		// dl	>=1 line	/+
											ApplyDIBPixelUnchecked(x, y, aa_close(1-Lx, 1-Dx));
									}
									else
									{
										if(Rx!=-1)	// d r	>=1 line	+\ stop
											ApplyDIBPixelUnchecked(x, y, aa_close(Rx, 1-Dx));
										else		// d	>=1 line	F
										{
											auto &LDx=Ycross[Xplaces*(y-1)+x-1], &RDx=Ycross[Xplaces*(y-1)+x+1];
											double d, d_min=1-Dx;
											if(LDx!=-1&&LDx>Dx&&d_min>(d=aa_middle(1-Dx, 1-LDx)))
												d_min=d;
											if(RDx!=-1&&RDx>Dx&&d_min>(d=aa_middle(1-Dx, 1-RDx)))
												d_min=d;
											ApplyDIBPixelUnchecked(x, y, d_min);
										}
									}
								}
								else
								{
									if(Lx!=-1)
									{
										if(Rx!=-1)	//  lr	>=1 line	|+|
										{
											double d, d_min=Rx;
											if(d_min>(d=1-Lx))
												d_min=d;
											ApplyDIBPixelUnchecked(x, y, d_min);
										}
										else		//  l	>=1 line	|+
										{
											auto &DLx=Xcross[Xplaces_1*(y-1)+x-1], &ULx=Xcross[Xplaces_1*(y+1)+x-1];
											double d, d_min=1-Lx;
											if(DLx!=-1&&DLx>Lx&&d_min>(d=aa_middle(1-Lx, 1-DLx)))
												d_min=d;
											if(ULx!=-1&&ULx>Lx&&d_min>(d=aa_middle(1-Lx, 1-ULx)))
												d_min=d;
											ApplyDIBPixelUnchecked(x, y, d_min);
										}
									}
									else
									{
										if(Rx!=-1)	//   r	>=1 line	+|
										{
											auto &DRx=Xcross[Xplaces_1*(y-1)+x  ], &URx=Xcross[Xplaces_1*(y+1)+x  ];
											double d, d_min=Rx;
											if(DRx!=-1&&DRx<Rx&&d_min>(d=aa_middle(Rx, DRx)))
												d_min=d;
											if(URx!=-1&&URx<Rx&&d_min>(d=aa_middle(Rx, URx)))
												d_min=d;
											ApplyDIBPixelUnchecked(x, y, d_min);
										}
										else		//		>=0 lines
										{
											auto
												&DLx=Xcross[Xplaces_1*(y-1)+x-1],	&LDx=Ycross[Xplaces*(y-1)+x-1],//	 _    _
												&DRx=Xcross[Xplaces_1*(y-1)+x  ],	&RDx=Ycross[Xplaces*(y-1)+x+1],//	| 1  2 | D
												&ULx=Xcross[Xplaces_1*(y+1)+x-1],	&LUx=Ycross[Xplaces* y   +x-1],//	    .
												&URx=Xcross[Xplaces_1*(y+1)+x  ],	&RUx=Ycross[Xplaces* y   +x+1];//	|_3  4_| U
											double d, d_min=_HUGE;
											if(DLx!=-1&&LDx!=-1&&d_min>(d=aa_far(1-DLx, 1-LDx)))//1
												d_min=d;
											if(DRx!=-1&&RDx!=-1&&d_min>(d=aa_far(  DRx, 1-RDx)))//2
												d_min=d;
											if(ULx!=-1&&LUx!=-1&&d_min>(d=aa_far(1-ULx,   LUx)))//3
												d_min=d;
											if(URx!=-1&&RUx!=-1&&d_min>(d=aa_far(  URx,   RUx)))//4
												d_min=d;
											if(d_min!=_HUGE)
												ApplyDIBPixelUnchecked(x, y, d_min>aa_thickness?1:d_min*_1_aa_thickness);
										}
									}
								}
							}
						}
					}
				}
			}
		}
		void doContour_component_partial(std::vector<double> &ndr, double Xs, double Zstart, double Zend, double Zstep, unsigned nzSteps, unsigned const component, int Xoffset, int Yoffset)
		{
			int Xoffset2=std::abs(Xoffset)+2, Yoffset2=std::abs(Yoffset)+2;
				 if(Xoffset>0){							 if(Yoffset>0)					doContour_component_range(ndr, Xs, Zstart, Zend, Zstep, nzSteps, component,	0,					Xplaces,	0,					Yoffset2),			doContour_component_range(ndr, Xs, Zstart, Zend, Zstep, nzSteps, component,	Xplaces-Xoffset2,	Xplaces,	Yoffset,			Yplaces);
													else if(Yoffset<0)Yoffset=-Yoffset,	doContour_component_range(ndr, Xs, Zstart, Zend, Zstep, nzSteps, component,	Xplaces-Xoffset2,	Xplaces,	0,					Yplaces-Yoffset),	doContour_component_range(ndr, Xs, Zstart, Zend, Zstep, nzSteps, component,	0,					Xplaces,	Yplaces-Yoffset2,	Yplaces);
													else								doContour_component_range(ndr, Xs, Zstart, Zend, Zstep, nzSteps, component,	Xplaces-Xoffset2,	Xplaces,	0,					Yplaces);}
			else if(Xoffset<0){Xoffset=-Xoffset;		 if(Yoffset>0)					doContour_component_range(ndr, Xs, Zstart, Zend, Zstep, nzSteps, component,	0,					Xplaces,	0,					Yoffset2),			doContour_component_range(ndr, Xs, Zstart, Zend, Zstep, nzSteps, component,	0,					Xoffset2,	Yoffset,			Yplaces);
													else if(Yoffset<0)Yoffset=-Yoffset,	doContour_component_range(ndr, Xs, Zstart, Zend, Zstep, nzSteps, component,	0,					Xoffset2,	0,					Yplaces-Yoffset),	doContour_component_range(ndr, Xs, Zstart, Zend, Zstep, nzSteps, component,	0,					Xplaces,	Yplaces-Yoffset2,	Yplaces);
													else								doContour_component_range(ndr, Xs, Zstart, Zend, Zstep, nzSteps, component,	0,					Xoffset2,	0,					Yplaces);}
			else{										 if(Yoffset>0)					doContour_component_range(ndr, Xs, Zstart, Zend, Zstep, nzSteps, component,	0,					Xplaces,	0,					Yoffset2);
													else if(Yoffset<0)Yoffset=-Yoffset,	doContour_component_range(ndr, Xs, Zstart, Zend, Zstep, nzSteps, component,	0,					Xplaces,	Yplaces-Yoffset2,	Yplaces);
													else;}
		/*		 if(Xoffset>0){							 if(Yoffset>0)					doContour_component_range(ndr, contour, Xs, Zstart, Zend, Zstep, nzSteps, component,	0,					Xplaces,	0,					Yoffset2),			doContour_component_range(ndr, contour, Xs, Zstart, Zend, Zstep, nzSteps, component,	Xplaces-Xoffset2,	Xplaces,	Yoffset,			Yplaces);
													else if(Yoffset<0)Yoffset=-Yoffset,	doContour_component_range(ndr, contour, Xs, Zstart, Zend, Zstep, nzSteps, component,	Xplaces-Xoffset2,	Xplaces,	0,					Yplaces-Yoffset),	doContour_component_range(ndr, contour, Xs, Zstart, Zend, Zstep, nzSteps, component,	0,					Xplaces,	Yplaces-Yoffset2,	Yplaces);
													else								doContour_component_range(ndr, contour, Xs, Zstart, Zend, Zstep, nzSteps, component,	Xplaces-Xoffset2,	Xplaces,	0,					Yplaces);}
			else if(Xoffset<0){Xoffset=-Xoffset;		 if(Yoffset>0)					doContour_component_range(ndr, contour, Xs, Zstart, Zend, Zstep, nzSteps, component,	0,					Xplaces,	0,					Yoffset2),			doContour_component_range(ndr, contour, Xs, Zstart, Zend, Zstep, nzSteps, component,	0,					Xoffset2,	Yoffset,			Yplaces);
													else if(Yoffset<0)Yoffset=-Yoffset,	doContour_component_range(ndr, contour, Xs, Zstart, Zend, Zstep, nzSteps, component,	0,					Xoffset2,	0,					Yplaces-Yoffset),	doContour_component_range(ndr, contour, Xs, Zstart, Zend, Zstep, nzSteps, component,	0,					Xplaces,	Yplaces-Yoffset2,	Yplaces);
													else								doContour_component_range(ndr, contour, Xs, Zstart, Zend, Zstep, nzSteps, component,	0,					Xoffset2,	0,					Yplaces);}
			else{										 if(Yoffset>0)					doContour_component_range(ndr, contour, Xs, Zstart, Zend, Zstep, nzSteps, component,	0,					Xplaces,	0,					Yoffset2);
													else if(Yoffset<0)Yoffset=-Yoffset,	doContour_component_range(ndr, contour, Xs, Zstart, Zend, Zstep, nzSteps, component,	0,					Xplaces,	Yplaces-Yoffset2,	Yplaces);
													else;}//*/
		}
		void doContour_partial(unsigned e, double Xs, double Zstart, double Zend, double Zstep, unsigned nzSteps, int Xoffset, int Yoffset)
		{
			auto &n=expr[e].n[expr[e].resultTerm];
			doContour_component_partial(n.r, Xs, Zstart, Zend, Zstep, nzSteps, 0, Xoffset, Yoffset);
			doContour_component_partial(n.i, Xs, Zstart, Zend, Zstep, nzSteps, 1, Xoffset, Yoffset);
		}
		void doContour(unsigned e, double Xs, double Zstart, double Zend, double Zstep, unsigned nzSteps)
		{
			auto &n=expr[e].n[expr[e].resultTerm];
			doContour_component_range(n.r, Xs, Zstart, Zend, Zstep, nzSteps, 0, 0, Xplaces, 0, Yplaces);
			doContour_component_range(n.i, Xs, Zstart, Zend, Zstep, nzSteps, 1, 0, Xplaces, 0, Yplaces);
		}
		void differentiate_x					(Term &n)
		{
			auto &ndr_r=n.r, &ndr_i=n.i;
			double _1_step=Xplaces/DX;
			for(int ky=0;ky<Yplaces;++ky)
			{
				for(int kx=0;kx<Xplaces-1;++kx)
				{
					int idx=Xplaces*ky+kx;
					CompRef(ndr_r[idx], ndr_i[idx])=(Comp1d(ndr_r[idx+1], ndr_i[idx+1])-Comp1d(ndr_r[idx], ndr_i[idx]))*_1_step;
				}
				int idx=Xplaces*ky+Xplaces-1;
				CompRef(ndr_r[idx], ndr_i[idx])*=-_1_step;
			}
			int idx=Xplaces*Yplaces-1;
			CompRef(ndr_r[idx], ndr_i[idx])*=-_1_step;
		}
		void differentiate_y					(Term &n)
		{
			auto &ndr_r=n.r, &ndr_i=n.i;
			double _1_step=Xplaces/DX;
			for(int ky=0;ky<Yplaces-1;++ky)
				for(int kx=0;kx<Xplaces;++kx)
				{
					int idx=Xplaces*ky+kx;
					CompRef(ndr_r[idx], ndr_i[idx])=(Comp1d(ndr_r[idx+Xplaces], ndr_i[idx+Xplaces])-Comp1d(ndr_r[idx], ndr_i[idx]))*_1_step;
				}
			for(int kx=0;kx<Xplaces;++kx)
			{
				int idx=Xplaces*(Yplaces-1)+kx;
				CompRef(ndr_r[idx], ndr_i[idx])*=-_1_step;
			}
			int idx=Xplaces*Yplaces-1;
			CompRef(ndr_r[idx], ndr_i[idx])*=-_1_step;
		}
		void integrate_x						(Term &n)
		{
			auto &ndr_r=n.r, &ndr_i=n.i;
			double step=DX/Xplaces;
			Comp1d sum;
			for(int ky=0;ky<Yplaces;++ky)
			{
				for(int kx=0;kx<Xplaces;++kx)
				{
					int idx=Xplaces*ky+kx;
					CompRef(ndr_r[idx], ndr_i[idx])=(sum+=Comp1d(ndr_r[idx], ndr_i[idx]))*step;
				}
				sum=0;
			}
		}
		void integrate_y						(Term &n)
		{
			auto &ndr_r=n.r, &ndr_i=n.i;
			double step=DX/Xplaces;
			std::vector<Comp1d> sum(Xplaces);
			for(int ky=Yplaces-1;ky>=0;--ky)
				for(int kx=0;kx<Xplaces;++kx)
				{
					int idx=Xplaces*ky+kx;
					CompRef(ndr_r[idx], ndr_i[idx])=(sum[kx]+=Comp1d(ndr_r[idx], ndr_i[idx]))*step;
				}
		}
		void discreteFourrierTransform			(Term &n)
		{
		//	fft_2d_forward((double*)n.r.p, (double*)n.i.p, Xplaces, Yplaces, fft_N0, fft_N1, fft_sqrt_N, fft_in, fft_out, fft_p, ifft_p);
		}
		void inverseDiscreteFourrierTransform	(Term &n)
		{
		//	fft_2d_inverse((double*)n.r.p, (double*)n.i.p, Xplaces, Yplaces, fft_N0, fft_N1, fft_sqrt_N, fft_in, fft_out, fft_p, ifft_p);
		}
		void lowPassFilter						(Term &n){lpf_2d(n, Xplaces, Yplaces);}
		void highPassFilter						(Term &n){hpf_2d(n, Xplaces, Yplaces);}
		void HemalyTransform					(Term &n)
		{
			auto &ndr_r=n.r, &ndr_i=n.i;
			std::vector<Comp1d> t0(ndrSize), t1(ndrSize);
			for(int k=0;k<ndrSize;++k)
				t0[k]=Comp1d(ndr_r[k], ndr_i[k]);
			auto sqrtsize=::sqrt(double(Xplaces*Yplaces));
			int Xplaces3=Xplaces*Xplaces*Xplaces, Yplaces3=Yplaces*Yplaces*Yplaces;
		//	auto Xplaces3_2=sqrt(Xplaces3), Yplaces3_2=sqrt(Yplaces3);
			for(int kv=0;kv<Yplaces;++kv)
			{
				for(int ku=0;ku<Xplaces;++ku)
				{
					auto &Fuv=t1[Xplaces*kv+ku];

					Comp1d//reversible with noise
						pu=-2*G2::_pi*Comp1d(0, 1)*(double(ku-Xplaces/2)/Xplaces3),
						pv=-2*G2::_pi*Comp1d(0, 1)*(double(kv-Yplaces/2)/Yplaces3);
					for(int ky=0;ky<Yplaces;++ky)
					{
						double py=ky-Yplaces/2; Comp1d ty=pv*(py*py*py);
						for(int kx=0;kx<Xplaces;++kx)
						{
							double px=kx-Xplaces/2;
							Fuv+=t0[Xplaces*ky+kx]*exp(pu*(px*px*px)+ty);
						}
					}
					Fuv/=sqrtsize;
				}
			}
			for(int k=0;k<ndrSize;++k)
				CompRef(ndr_r[k], ndr_i[k])=t1[k];
		}
		void inverseHemalyTransform				(Term &n)
		{
			auto &ndr_r=n.r, &ndr_i=n.i;
			std::vector<Comp1d> t0(ndrSize), t1(ndrSize);
			for(int k=0;k<ndrSize;++k)
				t0[k]=Comp1d(ndr_r[k], ndr_i[k]);
			auto sqrtsize=::sqrt(double(Xplaces*Yplaces));
			int Xplaces3=Xplaces*Xplaces*Xplaces, Yplaces3=Yplaces*Yplaces*Yplaces;
		//	auto Xplaces3_2=sqrt(Xplaces3), Yplaces3_2=sqrt(Yplaces3);
			for(int ky=0;ky<Yplaces;++ky)
			{
				for(int kx=0;kx<Xplaces;++kx)
				{
					auto &Fxy=t1[Xplaces*ky+kx];

					Comp1d//reversible with noise
						px=2*G2::_pi*Comp1d(0, 1)*(double(kx-Xplaces/2)*double(kx-Xplaces/2)*double(kx-Xplaces/2)/Xplaces3),
						py=2*G2::_pi*Comp1d(0, 1)*(double(ky-Yplaces/2)*double(ky-Yplaces/2)*double(ky-Yplaces/2)/Yplaces3);
					for(int kv=0;kv<Yplaces;++kv)
					{
						Comp1d tv=py*double(kv-Yplaces/2);
						for(int ku=0;ku<Xplaces;++ku)
							Fxy+=t0[Xplaces*kv+ku]*exp(px*double(ku-Xplaces/2)+tv);
					}
					Fxy/=sqrtsize;
				}
			}
			for(int k=0;k<ndrSize;++k)
				CompRef(ndr_r[k], ndr_i[k])=t1[k];
		}

		void messageTimer()
		{
//			if(_2d_drag_graph_not_window)
//			{
//				if(kb[VK_LEFT		]){	VX+=10*DX/ Xplaces		, toSolve=true; if(shiftOnly)shiftOnly=1, Xoffset+=10;}
//				if(kb[VK_RIGHT		]){	VX-=10*DX/ Xplaces		, toSolve=true; if(shiftOnly)shiftOnly=1, Xoffset-=10;}
//				if(kb[VK_UP			]){	VY-=10*DX/(Xplaces*AR_Y), toSolve=true; if(shiftOnly)shiftOnly=1, Yoffset-=10;}
//				if(kb[VK_DOWN		]){	VY+=10*DX/(Xplaces*AR_Y), toSolve=true; if(shiftOnly)shiftOnly=1, Yoffset+=10;}
//			}
//			else
//			{
//				if(kb[VK_LEFT		]){	VX-=10*DX/ Xplaces		, toSolve=true; if(shiftOnly)shiftOnly=1, Xoffset-=10;}
//				if(kb[VK_RIGHT		]){	VX+=10*DX/ Xplaces		, toSolve=true; if(shiftOnly)shiftOnly=1, Xoffset+=10;}
//				if(kb[VK_UP			]){	VY+=10*DX/(Xplaces*AR_Y), toSolve=true; if(shiftOnly)shiftOnly=1, Yoffset+=10;}
//				if(kb[VK_DOWN		]){	VY-=10*DX/(Xplaces*AR_Y), toSolve=true; if(shiftOnly)shiftOnly=1, Yoffset-=10;}
//			}
//			if(kb[VK_ADD		]||kb[VK_RETURN	]||kb[VK_OEM_PLUS	])
//			{
//				if(kb['X'])
//					DX/=1.1, AR_Y/=1.1;
//				else if(kb['Y'])
//					AR_Y*=1.1;
//				else
//					DX/=1.1;
//				function1(), toSolve=true, shiftOnly=0;
//			}
//			if(kb[VK_SUBTRACT	]||kb[VK_BACK	]||kb[VK_OEM_MINUS	])
//			{
//				if(kb['X'])
//					DX*=1.1, AR_Y*=1.1;
//				else if(kb['Y'])
//					AR_Y/=1.1;
//				else
//					DX*=1.1;
//				function1(), toSolve=true, shiftOnly=0;
//			}
			a_draw();
//			if(!time_variance&&!kp)
//				KillTimer(ghWnd, 0), timer=false;
		}
		int inputTouchDown(int idx){return _2D_DRAG;}
		void inputTouchUp(int idx)
		{
			drag_counter=0, disp_x=0, disp_y=0;//DEBUG
		}
		void inputTouchMove(int idx)
		{
			auto &ti=touchInfo[idx];
			if(touchInfo.size()==1)//shift
			{
				if(ti.region==_2D_DRAG)//C2D DRAG IS BROKEN
				{
					double Xr=DX/w;
					int dx=-ti.delta.x, dy=-ti.delta.y;
					VX+=dx*Xr;
					VY+=-dy*Xr/AR_Y;
					disp_x+=dx, disp_y-=dy;
					toSolve=true;
					if(shiftOnly)
						shiftOnly=1, Xoffset+=dx, Yoffset-=dy;
				}
			}
			else if(touchInfo.size()==2)//zoom
			{
				if(idx==1)//wait till both pointers are updated
				{
					auto &ti2=touchInfo[!idx];
					if(ti.region==_2D_DRAG&&ti2.region==_2D_DRAG)
					{
						auto &a=ti.pos, &b=ti2.pos, a0=a-ti.delta, b0=b-ti2.delta;
						float d=(a-b).magnitude(), d0=(a0-b0).magnitude();
						float diff=d0-d;
						double Xr=DX/w;
						DX+=diff*Xr;
						vec2 ab=0.5f*(a+b), ab0=0.5f*(a0+b0), dab=ab0-ab;
						VX+=dab.x*Xr, VY-=dab.y*Xr/AR_Y;
						function1();
						toSolve=true, shiftOnly=0;
					}
				}
			}
		}

		void draw()
		{
			static unsigned gl_texture=0;
			int full_solve=0;
			double DY=DX*h/(w*AR_Y);
			if(DY<=0)
				DY=1;
			auto &ex=expr[cursorEx];
			{
				bool changed=false;
				if(toSolve)
				{
					if(!paused)
						solver.synchronize();
					Xplaces=w, Yplaces=h;
					generate_glcl_texture(gl_texture, Xplaces, Yplaces);
					cl_solve_c2d(ex, VX, DX, VY, DY, Xplaces, Yplaces, solver.T, gl_texture);//TODO: partial solve
#if 0
					if(!operations.size()&&shiftOnly==1&&abs(Xoffset)<Xplaces&&abs(Yoffset)<Yplaces)//C2D DRAG IS BROKEN
					{
						if(Xoffset||Yoffset)
						{
							++drag_counter;//
							solver.partial_bounds(VX, DX, VY, DY, Xoffset, Yoffset);
							solver.synchronize();
							if(ex.nITD)
							{
								solver.full(ex);
								if(contourOnly)
									memset(solver.rgb, 0xFF, Xplaces*Yplaces*sizeof(int));
								else
									solver.updateRGB(ex.n[ex.resultTerm]);
							}
							else
							{
								(solver.*(solver.partial))(ex, contourOnly);
								//int ra[]={0, w>>1, 0, h>>2};//corner solve: correct
								//solver.solve_range(ex, ra, 2);
							}
							if(contourOn&&abs(Xoffset)+2<Xplaces&&abs(Yoffset)+2<Yplaces)
							{
							//	Rcontours.clear(), Icontours.clear();
								double Xs=DX/Xplaces, DZ=DX, Zstep=Xstep;
								doContour_partial(cursorEx, Xs, -DZ/2, +DZ/2, Zstep, 10, Xoffset, Yoffset);
							}
							else changed=true;
						}
					}
					else//c2d full solve
					{
						full_solve=1;
						auto old_time_variance=time_variance;
						time_variance=false;
						labels.clear();
						solver.full_resize(VX, DX, VY, DY, Xplaces=w, Yplaces=h);
						if(!paused)
							solver.synchronize();
						time_variance|=ex.nITD;
						labels.fill(cursorEx);
						solver.full(ex);
						for(auto it=operations.begin();it!=operations.end();++it)
						{
							auto &operation=*it;
							switch(operation)
							{
							case  1:differentiate_x					(ex.n[ex.resultTerm]);break;
							case  2:differentiate_y					(ex.n[ex.resultTerm]);break;
							case  3:integrate_x						(ex.n[ex.resultTerm]);break;
							case  4:integrate_y						(ex.n[ex.resultTerm]);break;
							case  5:discreteFourrierTransform		(ex.n[ex.resultTerm]);break;
							case  6:inverseDiscreteFourrierTransform(ex.n[ex.resultTerm]);break;
							case  7:lowPassFilter					(ex.n[ex.resultTerm]);break;
							case  8:highPassFilter					(ex.n[ex.resultTerm]);break;
							case  9:HemalyTransform					(ex.n[ex.resultTerm]);break;
							case 10:inverseHemalyTransform			(ex.n[ex.resultTerm]);break;
							}
						}
						if(contourOnly)
							memset(solver.rgb, 0xFF, Xplaces*Yplaces*sizeof(int));
						else
							solver.updateRGB(ex.n[ex.resultTerm]);
//						if(time_variance)
//						{
//							if(!paused)
//								SetTimer(ghWnd, 0, 10, 0);
//						}
//						else if(old_time_variance&&!timer)
//							KillTimer(ghWnd, 0);
						changed=true;
					}
				//	changed=true;
#endif
					toSolve=false, shiftOnly=2, Xoffset=0, Yoffset=0;
				}
				else if(time_variance)
				{
					if(!paused)
						solver.synchronize();
					solver.full(ex);
					for(auto it=operations.begin();it!=operations.end();++it)
					{
						auto &operation=*it;
						switch(operation)
						{
						case  1:differentiate_x					(ex.n[ex.resultTerm]);break;
						case  2:differentiate_y					(ex.n[ex.resultTerm]);break;
						case  3:integrate_x						(ex.n[ex.resultTerm]);break;
						case  4:integrate_y						(ex.n[ex.resultTerm]);break;
						case  5:discreteFourrierTransform		(ex.n[ex.resultTerm]);break;
						case  6:inverseDiscreteFourrierTransform(ex.n[ex.resultTerm]);break;
						case  7:lowPassFilter					(ex.n[ex.resultTerm]);break;
						case  8:highPassFilter					(ex.n[ex.resultTerm]);break;
						case  9:HemalyTransform					(ex.n[ex.resultTerm]);break;
						case 10:inverseHemalyTransform			(ex.n[ex.resultTerm]);break;
						}
					}
					if(contourOnly)
						memset(solver.rgb, 0xFF, Xplaces*Yplaces*sizeof(int));
					else
						solver.updateRGB(ex.n[ex.resultTerm]);
					changed=true;
				}
				if(changed&&contourOn)
				{
				//	Rcontours.clear(), Icontours.clear();
					double Xs=DX/Xplaces, DZ=DX, Zstep=Xstep;
					doContour(cursorEx, Xs, -DZ/2, +DZ/2, Zstep, 10);
				}
			}
			cl_finish();

		//	read_gl_texture(gl_texture, w, h, rgb);//DEBUG correct
		//	debug_printrgb(rgb, w, h, 512);
		//	display_texture(0, w, 0, h, rgb, w, h);//DEBUG correct
			if(cl_gl_interop)
				display_gl_texture(gl_texture);//draw the solution
			else
				display_texture(0, w, 0, h, rgb, w, h);
		//	display_texture(0, w, 0, h, solver.rgb, w, h);
		//	if(!contourOnly)
//			for(int ky=0;ky<h;++ky)//Xplaces=w+(w&1)
//				std::copy(solver.rgb+Xplaces*ky, solver.rgb+Xplaces*ky+w, rgb+w*ky);
			prof_add("display");

			if(!clearScreen)
			{
			//	GUIPrint(ghMemDC, w/2, h/2, "LOL_1");
				{
					double
						Xstart=VX-DX*0.5, Xend=VX+DX*0.5, inv_Xsample=w/DX,//inv_Xsample: unit size in pixels
						Ystart=VY-DY*0.5, Yend=VY+DY*0.5, inv_Ysample=h/DY;
					int H=int(Ystart>0?h:Yend<0?-1:Yend*inv_Ysample),//x-axis y-position, (V, H) is the origin
						HT=int(H>h-30?-18:2),
						V=int(Xstart>0?-1:Xend<0?w:-Xstart*inv_Xsample),//y-axis x-position
						VT=int(V>w-24-prec*8?-24-prec*8:2);
					int bkMode=setBkMode(TRANSPARENT);
					for(double x=floor(Xstart/Xstep)*Xstep, xEnd=ceil(Xend/Xstep)*Xstep, Xstep_2=Xstep*0.5;x<xEnd;x+=Xstep)
					{
						if(x>-Xstep_2&&x<Xstep_2)
							continue;
						double X=(x-Xstart)*inv_Xsample;
						print((int)X+2, H+HT, "%g", x);
					}
					for(double y=ceil(Yend/Ystep)*Ystep, yEnd=floor(Ystart/Ystep)*Ystep, Ystep_2=Ystep*0.5;y>yEnd;y-=Ystep)
					{
						if(y>-Ystep_2&&y<Ystep_2)
							continue;
						double Y=(Yend-y)*inv_Ysample;
						print(V+VT, (int)Y+2, "%g", y);
					}
					{
						int Ys=H+(H>h-46?-34:18);
						for(int kl=0, klEnd=labels.Xlabels.size();kl<klEnd;++kl)
						{
							auto &label=labels.Xlabels[kl];
							print_array(w-arrow_label_offset_X, Ys, label.label.c_str(), label.label.size()), Ys+=fontH;
						}
						int Xs=	V+VT+24+8*prec>w-24-prec*8
							?	V+VT-24-8*prec
							:	V+VT+24+8*prec;
						Ys=0;
						for(int kl=0, klEnd=labels.Ylabels.size();kl<klEnd;++kl)
						{
							auto &label=labels.Ylabels[kl];
							print_array(Xs, Ys, label.label.c_str(), label.label.size()), Ys+=fontH;
						}
						Ys=0;
						for(int kl=0, klEnd=labels.Clabels.size();kl<klEnd;++kl)
						{
							auto &label=labels.Clabels[kl];
							print(w-const_label_offset_X,  Ys, "%s=%g", label.label.c_str(), label.isTime?solver.T:label.value);
							Ys+=fontH;
						}
					}
					setBkMode(bkMode);
					prof_add("label");

#if 0				//DEBUG INFO
					auto &n=ex.n[ex.resultTerm];
					int ox=0, oy=0;
					for(int kx=1;kx<Xplaces;++kx)//find minimum real part
					{
						auto &v0=n.r[ox], &v1=n.r[kx];
						if(abs(v0)>abs(v1))
							ox=kx;
					}
					for(int ky=1;ky<Yplaces;++ky)//find minimum imaginary part
					{
						auto &v0=n.i[Xplaces*oy], &v1=n.i[Xplaces*ky];
						if(abs(v0)>abs(v1))
							oy=ky;
					}
					int cx=0, cy=0;
					for(int ky=0;ky<h;++ky)//find darkest spot in image
					{
						for(int kx=0;kx<w;++kx)
						{
							auto c0=(unsigned char*)(solver.rgb+w*cy+cx), c1=(unsigned char*)(solver.rgb+w*ky+kx);
							int
								abssq0=(int)c0[0]*c0[0]+(int)c0[1]*c0[1]+(int)c0[2]*c0[2],
								abssq1=(int)c1[0]*c1[0]+(int)c1[1]*c1[1]+(int)c1[2]*c1[2];
							if(abssq0>abssq1)
								cx=kx, cy=ky;
						}
					}
					print(0, h>>2, "V(%g, %g)", VX, VY);
					print(0, (h>>2)+fontH, "(%g, %g)%d", disp_x/inv_Xsample, disp_y/inv_Ysample, drag_counter);
					int c=pen_color;
					GL2_2D::draw_line(0, H, w, H), GL2_2D::draw_line(V, 0, V, h);//black crosshair: correct origin
					GL2_2D::draw_rectangle_hollow(ivec4(V-50, V+50, H-50, H+50));
					pen_color=0xFF0000FF;//red
					GL2_2D::draw_line(0, h>>1, w, h>>1), GL2_2D::draw_line(w>>1, 0, w>>1, h);//red crosshair: screen center
					pen_color=0xFFFF0000;//blue
					GL2_2D::draw_line(0, oy, w, oy), GL2_2D::draw_line(ox, 0, ox, h);//blue crosshair: ndr origin
					GL2_2D::draw_rectangle_hollow(ivec4(ox-50, ox+50, oy-50, oy+50));
					pen_color=0xFF00FF00;//green
					GL2_2D::draw_line(0, cy, w, cy), GL2_2D::draw_line(cx, 0, cx, h);//green crosshair: rgb origin
					GL2_2D::draw_rectangle_hollow(ivec4(cx-50, cx+50, cy-50, cy+50));
					int bkMode2=setBkMode(TRANSPARENT), txtColor=setTextColor(0xFF000000);
					print(V, H, "correct");
					setTextColor(0xFFFF0000);
					print(ox, oy, "ndr");
					setTextColor(0xFF00FF00);
					print(cx, cy, "rgb");
					setBkMode(bkMode2);
					pen_color=c;
					prof_add("debug info");
#endif
				}
				{
					int k=0, Y=h-operations.size()*16;
					for(auto it=operations.begin();it!=operations.end();++it)
					{
						auto &operation=*it;
						char const *a=0;
						switch(operation)
						{
						case 1:a="%d: Differentiate X";break;
						case 2:a="%d: Differentiate Y";break;
						case 3:a="%d: Integrate X";break;
						case 4:a="%d: Integrate Y";break;
						case 5:a="%d: DFT";break;
						case 6:a="%d: IDFT";break;
						case 7:a="%d: LPF";break;
						case 8:a="%d: HPF";break;
						case 9:a="%d: HT";break;
						case 10:a="%d: IHT";break;
						}
						print(w-const_label_offset_X, Y, a, k);
						++k, Y+=16;
					}
				}
			}
			static std::vector<ProfInfo> prof2;//
			if(full_solve)//
				prof2=prof;//

			prof_print();

			prof=prof2;//
			prof_print(h>>2);//
			prof_start();
		}
		void i_draw();
		void a_draw();
	} c2d;
	const int		Color_2D::modes[]={6}, Color_2D::nmodes=sizeof(modes)>>2;
	void			Color_2D::i_draw()
	{
		if(modes::ready)
		{
			draw();
//			if(showLastModeOnIdle)
//				std::copy(rgb, rgb+w*h, modeRGB);
		}
		else
		{
//			std::copy(modeRGB, modeRGB+w*h, rgb);
		}
		if(showBenchmark)
			Performance(0, h>>1);//
	}
	void			Color_2D::a_draw()
	{
	//	Rectangle(ghMemDC, bpx-1, bpy-1, bw+1, bh+1);
		
		draw();
		
		if(contextHelp)
		{
			const char *help[]=
			{
				"arrows/drag: move",
				"+/-/enter/backspace/wheel: zoom",
				"X/Y +/-/enter/backspace/wheel: scale x/y",
				"E: reset scale",
				"R: reset scale & view",
				"C: toggle clear screen",
				"1: differentiate x",
				"2: differentiate y",
				"3: integrate x",
				"4: integrate y",
				"5: DFT",
				"6: Inverse DFT",
				"7: LPF",
				"8: HPF",
				"H: HT",
				"J: Inverse HT",
				"0: reset operations",
				"`: contour",
				"Esc: back to text editor"
			};
			print_contextHelp(help, sizeof(help)>>2, 270);
		}
		if(showBenchmark&&!clearScreen)
			Performance(0, h);//
	//	BitBlt(ghDC, 0, 0, w, h, ghMemDC, 0, 0, SRCCOPY);
	}
	class		Longitudinal_2D:public _2D_Mode
	{
	public:
		int Xoffset, Yoffset, exprRemoved;

		int Xplaces, Yplaces, old_nX, old_nY;
		double T;
		Solve_L2D solver;
		
		bool clearScreen;
		Labels_2D labels;
	//	_2D_L2D _2d;
		static const int modes[], nmodes;
	//	HPEN__ *hPen;
	//	HBRUSH__ *hBrush;

		Longitudinal_2D():
			solver(true), _2D_Mode(solver),
			Xoffset(0), Yoffset(0), exprRemoved(0),
			clearScreen(false)
		{
		//	hPen=CreatePen(PS_SOLID, 1, _3dGridColor);
		}
	//	~Longitudinal_2D(){DeleteObject(hPen);}

		void messageTimer()
		{
//			if(_2d_drag_graph_not_window)
//			{
//				if(kb[VK_LEFT		]){	VX+=10*DX/ w		, toSolve=true; if(shiftOnly)shiftOnly=1, Xoffset+=10;}
//				if(kb[VK_RIGHT		]){	VX-=10*DX/ w		, toSolve=true; if(shiftOnly)shiftOnly=1, Xoffset-=10;}
//				if(kb[VK_UP			]){	VY-=10*DX/(w*AR_Y)	, toSolve=true; if(shiftOnly)shiftOnly=1, Yoffset-=10;}
//				if(kb[VK_DOWN		]){	VY+=10*DX/(w*AR_Y)	, toSolve=true; if(shiftOnly)shiftOnly=1, Yoffset+=10;}
//			}
//			else
//			{
//				if(kb[VK_LEFT		]){	VX-=10*DX/ w		, toSolve=true; if(shiftOnly)shiftOnly=1, Xoffset-=10;}
//				if(kb[VK_RIGHT		]){	VX+=10*DX/ w		, toSolve=true; if(shiftOnly)shiftOnly=1, Xoffset+=10;}
//				if(kb[VK_UP			]){	VY+=10*DX/(w*AR_Y)	, toSolve=true; if(shiftOnly)shiftOnly=1, Yoffset+=10;}
//				if(kb[VK_DOWN		]){	VY-=10*DX/(w*AR_Y)	, toSolve=true; if(shiftOnly)shiftOnly=1, Yoffset-=10;}
//			}
//			if(kb[VK_ADD		]||kb[VK_RETURN	]||kb[VK_OEM_PLUS	])
//			{
//				if(kb['X'])
//					DX/=1.05, AR_Y/=1.05;
//				else if(kb['Y'])
//					AR_Y*=1.05;
//				else
//					DX/=1.05;
//				//	DX/=1.1, DY=DX*h/w;
//				function1(), toSolve=true, shiftOnly=0;
//			}
//			if(kb[VK_SUBTRACT	]||kb[VK_BACK	]||kb[VK_OEM_MINUS	])
//			{
//				if(kb['X'])
//					DX*=1.05, AR_Y*=1.05;
//				else if(kb['Y'])
//					AR_Y/=1.05;
//				else
//					DX*=1.05;
//				function1(), toSolve=true, shiftOnly=0;
//			//	DX*=1.1, DY=DX*h/w, function1(), toSolve=true, shiftOnly=0;
//			}
			a_draw();
//			if(!time_variance&&!kp)
//				KillTimer(ghWnd, 0), timer=false;
		}
		int inputTouchDown(int idx)
		{
			return _2D_DRAG;
		}
		void inputTouchUp(int idx)
		{
		}
		void inputTouchMove(int idx)
		{
			auto &ti=touchInfo[idx];
			if(touchInfo.size()==1)//shift
			{
				if(ti.region==_2D_DRAG)
				{
					double Xr=DX/w;
					int dx=-ti.delta.x;
					VX+=dx*Xr;
					VY+=ti.delta.y*Xr/AR_Y;
					toSolve=true;
					if(shiftOnly)
						shiftOnly=1, Xoffset+=dx, Yoffset+=ti.delta.y;
				}
			}
			else if(touchInfo.size()==2)//zoom
			{
				if(idx==1)//wait till both pointers are updated
				{
					auto &ti2=touchInfo[!idx];
					if(ti.region==_2D_DRAG&&ti2.region==_2D_DRAG)
					{
						auto &a=ti.pos, &b=ti2.pos, a0=a-ti.delta, b0=b-ti2.delta;
						float d=(a-b).magnitude(), d0=(a0-b0).magnitude();
						float diff=d0-d;
						double Xr=DX/w;
						DX+=diff*Xr;
						vec2 ab=0.5f*(a+b), ab0=0.5f*(a0+b0), dab=ab0-ab;
						VX+=dab.x*Xr, VY-=dab.y*Xr/AR_Y;
						function1();
						toSolve=true, shiftOnly=0;
					}
				}
			}
		}

		void draw()
		{
			double DY=DX*h/(w*AR_Y);
			if(DY<=0)
				DY=1;
			{//draw the grid
				set_color(_3dGridColor);
				double Ystart=VY-DY/2, Yend=VY+DY/2, Xstart=VX-DX/2, Xend=VX+DX/2;
				for(double x=floor(Xstart/Xstep)*Xstep, xEnd=ceil(Xend/Xstep)*Xstep;x<xEnd;x+=Xstep)
				{
					int X=(int)floor((x-Xstart)/DX*w);
					GL2_2D::draw_line(X, 0, X, h);
				}
				for(double y=ceil(Yend/Ystep)*Ystep, yEnd=floor(Ystart/Ystep)*Ystep-Ystep;y>yEnd;y-=Ystep)
				{
					int Y=(int)floor((Yend-y)/DY*h);
					GL2_2D::draw_line(0, Y, w, Y);
				}
			}
			if(toSolve)
			{
				if(_dangerous_code&&//
					shiftOnly==1&&abs(Xoffset)<Xplaces&&abs(Yoffset)<Yplaces)
				{
					if(Xoffset||Yoffset)
					{
						solver.partial_bounds(VX, DX, VY, DY, Xoffset, Yoffset);
						solver.synchronize();
						for(int ke=0, keEnd=expr.size();ke<keEnd;++ke)
						{
							auto &ex=expr[ke];
							if(ex.rmode[0]==7)
								(solver.*(ex.nITD?&Solve_L2D::full:solver.partial))(ex);
						}
					}
				}
				else
				{
					auto old_time_variance=time_variance;
					time_variance=false;
					labels.clear();
					Xplaces=w, Yplaces=h;
					function1();
					solver.full_resize(VX, DX, VY, DY, Xstep, Ystep, Xplaces, Yplaces);
					if(!paused)
						solver.synchronize();
					for(unsigned e=0;e<expr.size();++e)
					{
						auto &ex=expr[e];
						if(ex.rmode[0]==7)
						{
							time_variance|=ex.nITD;
							labels.fill(e);
							solver.full(ex);
						}
					}
//					if(time_variance)
//					{
//						if(!paused)
//							SetTimer(ghWnd, 0, 10, 0);
//					}
//					else if(old_time_variance&&!timer)
//						KillTimer(ghWnd, 0);
				}
				toSolve=false, shiftOnly=2, Xoffset=Yoffset=0;
				exprRemoved=false;
			}
			else if(exprRemoved)
			{
				auto old_time_variance=time_variance;
				time_variance=false;
				labels.clear();
				for(unsigned e=0;e<expr.size();++e)
				{
					auto &ex=expr[e];
					if(ex.rmode[0]==7)
					{
						time_variance|=ex.nITD;
						labels.fill(e);
					}
				}
//				if(old_time_variance&&!time_variance&&!timer)
//					KillTimer(ghWnd, 0);
				exprRemoved=false;
			}
			else if(time_variance)
			{
				if(!paused)
					solver.synchronize();
				for(int ke=0, keEnd=expr.size();ke<keEnd;++ke)
				{
					auto &ex=expr[ke];
					if(ex.rmode[0]==7&&ex.nITD)
						solver.full(ex);
				}
			}
			int colorCondition=nExpr[7]>1;
			if(!clearScreen)
			{
			//	double LOL_1=h*(VY+DY/2)/DY;//242.99999999999997
			//	double LOL_2=h*(VY/DY+.5);//243.00000000000000
				int H=int(VY-DY/2>0?h:VY+DY/2<0?-1:h*(VY/DY+.5)), HT=H+(H>h-30?-18:2), V=int(VX-DX/2>0?-1:VX+DX/2<0?w:w*(-VX+DX/2)/DX), VT=int(V>w-24-prec*8?-24-prec*8:2);
				int bkMode=setBkMode(TRANSPARENT);
				for(double x=floor((VX-DX/2)/Xstep)*Xstep, xEnd=ceil((VX+DX/2)/Xstep)*Xstep;x<xEnd;x+=Xstep)
				{
					if(x>-Xstep/2&&x<Xstep/2)
						continue;
					double X=(x-(VX-DX/2))/DX*w;
					print(X+2, HT, "%g", x);
				}
				for(double y=ceil((VY+DY/2)/Ystep)*Ystep, yEnd=floor((VY-DY/2)/Ystep)*Ystep;y>yEnd;y-=Ystep)
				{
					if(y>-Ystep/2&&y<Ystep/2)
						continue;
					double Y=((VY+DY/2)-y)/DY*h;
					print(V+VT, Y+2, "%g", y);
				}
				GL2_2D::draw_line(0, H, w, H), GL2_2D::draw_line(V, 0, V, h);
				{
					int textColor=getTextColor();
					int Ys=H+(H>h-46?-34:18);
					for(int kl=0, klEnd=labels.Xlabels.size();kl<klEnd;++kl)
					{
						auto &label=labels.Xlabels[kl];
						setTextColor(0xFF000000|(expr[label.exNo].color&-colorCondition));
						print_array(w-arrow_label_offset_X, Ys, label.label.c_str(), label.label.size()), Ys+=fontH;
					}
					int Xs=	V+VT+24+8*prec>w-24-prec*8
						?	V+VT-24-8*prec
						:	V+VT+24+8*prec;
					Ys=0;
					for(int kl=0, klEnd=labels.Ylabels.size();kl<klEnd;++kl)
					{
						auto &label=labels.Ylabels[kl];
						setTextColor(0xFF000000|(expr[label.exNo].color&-colorCondition));
						print_array(Xs, Ys, label.label.c_str(), label.label.size()), Ys+=fontH;
					}
					Ys=0;
					for(int kl=0, klEnd=labels.Clabels.size();kl<klEnd;++kl)
					{
						auto &label=labels.Clabels[kl];
						setTextColor(0xFF000000|(expr[label.exNo].color&-colorCondition));
						print(w-const_label_offset_X, Ys, "%s=%g", label.label.c_str(), label.isTime?solver.T:label.value);
						Ys+=fontH;
					}
				}
				setBkMode(bkMode);
			}
			double XXstart=solver.XXstart, YYend=VY+DY/2;
			double Xr=w/DX, Yr=h/DY;
			int colored=0;
			int ndrSize=solver.ndrSize;//Xplaces*solver.nX+Yplaces*solver.nY;
			unsigned yDiscOffset=(Xplaces-1)*solver.nX;
			for(int ke=0, keEnd=expr.size();ke<keEnd;++ke)//draw the curves/lines
			{
				auto &ex=expr[ke];
				if(ex.rmode[0]==7)
				{
					set_color(0xFF000000|(ex.color&-colorCondition));
					auto &n=ex.n[ex.resultTerm];
					auto &ndr_r=n.r, &ndr_i=n.i;

					for(int y=0;y<solver.nX;++y)//horizontal projected lines
					{
						GL2_2D::curve_begin();
						for(int x=0;x<Xplaces-1;++x)
						{
							int idx=Xplaces*y+x;
							if(ndr_r.size()!=ndrSize||ndr_i.size()!=ndrSize)//
								return;//
							CompRef V(ndr_r[idx], ndr_i[idx]);
							GL2_2D::curve_point(Xr*(V.r-XXstart), Yr*(YYend-V.i));
							GL2_2D::continuous=!ex.discontinuities[(Xplaces-1)*y+x];
						}
						int idx=Xplaces*y+Xplaces-1;
						CompRef V(ndr_r[idx], ndr_i[idx]);
						GL2_2D::curve_point(Xr*(V.r-XXstart), Yr*(YYend-V.i));
						GL2_2D::draw_curve();
					}
					int yPos=Xplaces*solver.nX;
					for(int x=0;x<solver.nY;++x)//vertical projected lines
					{
						GL2_2D::curve_begin();
						for(int y=0;y<Yplaces-1;++y)
						{
							int idx=yPos+Yplaces*x+y;
							if(ndr_r.size()!=ndrSize||ndr_i.size()!=ndrSize)//
								return;//
							CompRef V(ndr_r[idx], ndr_i[idx]);
							GL2_2D::curve_point(Xr*(V.r-XXstart), Yr*(YYend-V.i));
						}
						int idx=yPos+Yplaces*x+Yplaces-1;
						CompRef V(ndr_r[idx], ndr_i[idx]);
						GL2_2D::curve_point(Xr*(V.r-XXstart), Yr*(YYend-V.i));
						GL2_2D::draw_curve();
					}
				}
			}
		}
		void i_draw();
		void a_draw();
	} l2d;
	const int		Longitudinal_2D::modes[]={7}, Longitudinal_2D::nmodes=sizeof(modes)>>2;
	void			Longitudinal_2D::i_draw()
	{
		if(modes::ready)
		{
			draw();
//			if(showLastModeOnIdle)
//				std::copy(rgb, rgb+w*h, modeRGB);
		}
		else
		{
		//	std::copy(modeRGB, modeRGB+w*h, rgb);
		}
		if(showBenchmark)
			Performance(0, h>>1);//
	}
	void			Longitudinal_2D::a_draw()
	{
	//	Rectangle(ghMemDC, bpx-1, bpy-1, bw+1, bh+1);
		
		draw();
		
		if(contextHelp)
		{
			const char *help[]=
			{
				"arrows/drag: move",
				"+/-/enter/backspace/wheel: zoom",
				"X/Y +/-/enter/backspace/wheel: scale x/y",
				"E: reset scale",
				"R: reset scale & view",
				"C: toggle clear screen",
				"Esc: back to text editor"
			};
			print_contextHelp(help, sizeof(help)>>2, 270);
		}
		if(showBenchmark&&!clearScreen)
			Performance(0, h);//
	//	BitBlt(ghDC, 0, 0, w, h, ghMemDC, 0, 0, SRCCOPY);
	}
	class		Transverse_2D_H:public _3D_Mode
	{
	public:
		int Xoffset, Yoffset;
		int X0, Y0;

		int Xplaces, Yplaces;
		double XshiftPoint, YshiftPoint;
		double XsamplePos, YsamplePos;

		double Xstart, Xr, Ystart, Yr;
		Solve_2D solver;
		std::list<int> operations;
		
		bool contourOn, contourOnly, contourFlat;
		std::map<unsigned, std::unordered_map<int, std::list<std::pair<contour::Double_X_Y, contour::Double_X_Y>>>>
			Rcontours,//contours[e]=contour, contour[r level (r/step)]=sticks, *stick.begin()=((x,y),(x,y))
			Icontours,//contours[e]=contour, contour[i level (i/step)]=sticks, *stick.begin()=((x,y),(x,y))
			Jcontours,//contours[e]=contour, contour[j level (j/step)]=sticks, *stick.begin()=((x,y),(x,y))
			Kcontours;//contours[e]=contour, contour[k level (k/step)]=sticks, *stick.begin()=((x,y),(x,y))
		
		bool clearScreen, kb_VK_F6_msg;
		Labels_2D labels;
		_3D _3d;
		static const int modes[], nmodes;
		int gridColor, rColor, iColor, jColor, kColor;

		Transverse_2D_H():
			solver(true), _3D_Mode(solver),
			Xplaces(100), Yplaces(100),
			XshiftPoint(0), YshiftPoint(0),
			XsamplePos(0), YsamplePos(0),
			_3d(4, 4, 4, 225*G2::_pi/180, 324.7356103172454*G2::_pi/180, 1),
			gridColor(_3dGridColor), rColor(0), iColor(0xFF0000EF), jColor(0xFF00EF00), kColor(0xFFEF0000),//rgb
			Xoffset(0), Yoffset(0),
			contourOnly(false),
			clearScreen(false), kb_VK_F6_msg(false)
		{}

		void doContour_component(unsigned e, unsigned c, double Xs, double Zstart, double Zend, double Zstep, unsigned nzSteps)
		{
			using namespace contour;
			auto &ex=expr[e];
			auto &ndr=(&ex.n[ex.resultTerm].r)[c];
			auto contour=&Rcontours[e];
			switch(c)
			{
			case 0:contour=&Rcontours[e];break;
			case 1:contour=&Icontours[e];break;
			case 2:contour=&Jcontours[e];break;
			case 3:contour=&Kcontours[e];break;
			}
			auto cutTrgl=[&](double &Xa, double &Ya, double &Za, double &Xb, double &Yb, double &Zb, double &Xc, double &Yc, double &Zc)
			{
				double X1, Y1, Z1, X2, Y2, Z2, X3, Y3, Z3;//ordered by z in acscending order
				if(Za<Zb)
				{
					if(Zb<Zc)
						X1=Xa, Y1=Ya, Z1=Za, X2=Xb, Y2=Yb, Z2=Zb, X3=Xc, Y3=Yc, Z3=Zc;
					else if(Za<Zc)
						X1=Xa, Y1=Ya, Z1=Za, X2=Xc, Y2=Yc, Z2=Zc, X3=Xb, Y3=Yb, Z3=Zb;
					else
						X1=Xc, Y1=Yc, Z1=Zc, X2=Xa, Y2=Ya, Z2=Za, X3=Xb, Y3=Yb, Z3=Zb;
				}
				else if(Zb<Zc)
				{
					if(Za<Zc)
						X1=Xb, Y1=Yb, Z1=Zb, X2=Xa, Y2=Ya, Z2=Za, X3=Xc, Y3=Yc, Z3=Zc;
					else
						X1=Xb, Y1=Yb, Z1=Zb, X2=Xc, Y2=Yc, Z2=Zc, X3=Xa, Y3=Ya, Z3=Za;
				}
				else
					X1=Xc, Y1=Yc, Z1=Zc, X2=Xb, Y2=Yb, Z2=Zb, X3=Xa, Y3=Ya, Z3=Za;
				double X13, Y13, Z13;
				{
					double Zr=(Z2-Z1)/(Z3-Z1);
					X13=X1+(X3-X1)*Zr, Y13=Y1+(Y3-Y1)*Zr, Z13=Z2;
				}
				//for Z1 -> Z2
				for(double z=std::floor((Z1<Zstart?Zstart:Z1)/Zstep), zEnd=std::floor((Z2>Zend?Zend:Z2)/Zstep);z<=zEnd;++z)
				{
					double ZL=Zstep*z;
					if(Z1<=ZL&&ZL<Z2)
					{
						double M=(ZL-Z1)/(Z2-Z1);
						contour->operator[](int(z)).push_back(std::pair<modes::contour::Double_X_Y, modes::contour::Double_X_Y>(
							modes::contour::Double_X_Y(X1+M*(X2-X1), Y1+M*(Y2-Y1)),
							modes::contour::Double_X_Y(X1+M*(X13-X1), Y1+M*(Y13-Y1))
							));
					}
				}
				//for Z2 -> Z3
				for(double z=std::floor((Z2<Zstart?Zstart:Z2)/Zstep), zEnd=std::floor((Z3>Zend?Zend:Z3)/Zstep);z<=zEnd;++z)
				{
					double ZL=Zstep*z;
					if(Z2<=ZL&&ZL<Z3)
					{
						double M=(ZL-Z2)/(Z3-Z2);
						contour->operator[](int(z)).push_back(std::pair<modes::contour::Double_X_Y, modes::contour::Double_X_Y>(
							modes::contour::Double_X_Y(X2+M*(X3-X2), Y2+M*(Y3-Y2)),
							modes::contour::Double_X_Y(X13+M*(X3-X13), Y13+M*(Y3-Y13))
							));
					}
				}
			};
			double Xstart=solver.Xstart, Ystart=AR_Y*solver.Ystart;
			for(unsigned vy=0, vyEnd=Yplaces-1;vy<vyEnd;++vy)
			{
				for(unsigned vx=0, vxEnd=Xplaces-1;vx<vxEnd;++vx)
				{
					auto
						X00=Xstart+Xs* vx    , Y00=Ystart+Xs* vy    , &Z00=ndr[Xplaces* vy    +vx   ],
						X01=Xstart+Xs* vx    , Y01=Ystart+Xs*(vy+1 ), &Z01=ndr[Xplaces*(vy+1 )+vx   ],
						X10=Xstart+Xs*(vx+1 ), Y10=Ystart+Xs* vy    , &Z10=ndr[Xplaces* vy    +vx+1 ],
						X11=Xstart+Xs*(vx+1 ), Y11=Ystart+Xs*(vy+1 ), &Z11=ndr[Xplaces*(vy+1 )+vx+1 ],
						Xx =Xstart+Xs*(vx+.5), Yx =Ystart+Xs*(vy+.5),  Zx =.25*(Z00+Z01+Z10+Z11);
					cutTrgl(X00, Y00, Z00, Xx, Yx, Zx, X01, Y01, Z01);
					cutTrgl(X01, Y01, Z01, Xx, Yx, Zx, X11, Y11, Z11);
					cutTrgl(X11, Y11, Z11, Xx, Yx, Zx, X10, Y10, Z10);
					cutTrgl(X10, Y10, Z10, Xx, Yx, Zx, X00, Y00, Z00);
				}
			}
		}
		void doContour(unsigned e, double Xs, double Zstart, double Zend, double Zstep, unsigned nzSteps)
		{
			doContour_component(e, 0, Xs, Zstart, Zend, Zstep, nzSteps);
			doContour_component(e, 1, Xs, Zstart, Zend, Zstep, nzSteps);
			doContour_component(e, 2, Xs, Zstart, Zend, Zstep, nzSteps);
			doContour_component(e, 3, Xs, Zstart, Zend, Zstep, nzSteps);
		}

		void setDimensions(int x, int y, int w, int h)
		{
			bpx=x, bpy=y, bw=w, bh=h, X0=bpx+bw/2, Y0=bpy+bh/2;
			_3d.setDimensions(x, y, w, h);
			double old_Zstep=Zstep;
			function1();
			if(!toSolve&&contourOn&&old_Zstep!=Zstep)
			{
				Rcontours.clear(), Icontours.clear(), Jcontours.clear(), Kcontours.clear();
				double Xs=DX/Xplaces, DZ=DX/AR_Z;
				doContour(cursorEx, Xs, VZ-DZ/2, VZ+DZ/2, Zstep, 10);
			}
			ready=true;
		}
		void shiftNDR(double &DshiftPoint, double Dsample, double &DsamplePos, double &VD, int &Doffset, double ammount)
		{
			double newDsamplePos=std::floor((DshiftPoint+=ammount)/Dsample);
			if(newDsamplePos!=DsamplePos)
			{
				toSolve=true;
				if(shiftOnly)
					shiftOnly=1, Doffset+=int(newDsamplePos-DsamplePos);
				DsamplePos=newDsamplePos, VD=newDsamplePos*Dsample;
			}
			DsamplePos=newDsamplePos;
		}
		void shiftNDRupdate(double DshiftPoint, double Dsample, double &DsamplePos, double &VD)
		{
			double newDsamplePos=std::floor(DshiftPoint/Dsample);
			if(newDsamplePos!=DsamplePos)
				DsamplePos=newDsamplePos, VD=newDsamplePos*Dsample;
		}
		void messageTimer()
		{
			a_draw();
		}
		int inputTouchDown(int idx)
		{
			auto &ti=touchInfo[idx];
			auto &pos=ti.pos;
			int X0=w>>1, Y0=h>>1;
			if(pos.y>Y0)
			{
				if(pos.x<X0)
					return BOTTOM_LEFT_MOVE;
				return BOTTOM_RIGHT_TURN;
			}
			if(reset_button.click(pos))
			{
				_3d.cam.reset();
				DX=20, AR_Y=1, AR_Z=1, function1();
				VX=VY=VZ=0;
			//	_3d.teleport_degrees(4, 4, 4, 225, 324.7356103172454, 1);
				return BUTTON_RESET;
			}
			return TOUCH_MISS;
		}
		void inputTouchUp(int idx)
		{
		}
		void inputTouchMove(int idx)
		{
			auto &ti=touchInfo[idx];
			if(ti.region==BOTTOM_RIGHT_TURN)
				_3d.cam.turnBy(ti.delta.x*5/w, ti.delta.y*5/w);
		}

		void draw()
		{
			int X0=w>>1, Y0=h>>1;
			for(int k=0, kEnd=touchInfo.size();k<kEnd;++k)//move camera
			{
				auto &ti=touchInfo[k];
				if(ti.region==BOTTOM_LEFT_MOVE)
				{
					int X00=w>>2, Y00=h*3>>2;
					float dx=float(ti.pos.x-X00)/w, dy=-float(ti.pos.y-Y00)/w;
					_3d.cam.move(dx, dy);
					break;
				}
			}
			double DY=DX/AR_Y, DZ=DX/AR_Z;
			auto &ex=expr[cursorEx];
			{
				bool changed=false;
				if(toSolve)
				{
					if(shiftOnly==1&&abs(Xoffset)<Xplaces&&abs(Yoffset)<Yplaces)
					{
						if(Xoffset||Yoffset)
						{
							solver.partial_bounds(VX, DX, VY/AR_Y, DY, Xoffset, Yoffset);
							solver.synchronize();
							(solver.*(ex.nITD?&Solve_2D::full:solver.partial))(ex);
						}
					}
					else
					{
						auto old_time_variance=time_variance;
						time_variance=false;
						labels.clear();
						solver.full_resize(VX, DX, VY/AR_Y, DY, Xplaces, Yplaces);
						XsamplePos=std::floor(XshiftPoint/solver.Xsample);
						YsamplePos=std::floor(YshiftPoint/solver.Ysample);
					//	solver.full_resize(VX, DX, VY/AR_Y, DY, Xplaces=100, Yplaces=100), KXplaces=Xplaces/100?Xplaces/100:1, KYplaces=Yplaces/100?Yplaces/100:1;
						time_variance|=ex.nITD;
						labels.fill(cursorEx);
						if(!paused)
							solver.synchronize();
						solver.full(ex);
//						if(time_variance)
//						{
//							if(!paused)
//								SetTimer(ghWnd, 0, 10, 0);
//						}
//						else if(old_time_variance&&!timer)
//							KillTimer(ghWnd, 0);
					}
					changed=true;
					toSolve=false, shiftOnly=2, Xoffset=Yoffset=0;
				}
				else if(time_variance)
				{
					if(!paused)
						solver.synchronize();
					solver.full(ex);
					changed=true;
				}
				if(changed&&contourOn)
				{
					Rcontours.clear(), Icontours.clear(), Jcontours.clear(), Kcontours.clear();
					double Xs=DX/Xplaces, DZ=DX/AR_Z;
					doContour(cursorEx, Xs, VZ-DZ/2, VZ+DZ/2, Zstep, 10);
				}
			}

			GL2_3D::begin();
			_3dMode_FrameStart(_3d, VX, VY, VZ, DX, Xstep, Ystep, Zstep, AR_Y, AR_Z);
			double Xstart=solver.Xstart, Xsample=solver.Xsample, Ystart=AR_Y*solver.Ystart, Ysample=AR_Y*solver.Ysample;
			if(!contourOnly)//draw the surfaces
			{
				int ndrSize=solver.ndrSize;
				double Yend=VY+DY/2;
				auto &n=ex.n[ex.resultTerm];
				int lColor[]={rColor, iColor, jColor, kColor};
				if(operations.size())
				{
					for(unsigned c=0;c<4;++c)
					{
						auto &ndr=(&n.r)[c];
						_3d.lineColor=lColor[c];
						for(int y=0;y<Yplaces;++y)
						{
							GL2_3D::curve_start(_3d.lineColor);
							for(int x=0;x<Xplaces;++x)
							{
								if(ndr.size()!=ndrSize)
									return;
								GL2_3D::curve_point(vec3(Xstart+x*Xsample, Ystart+y*Ysample, AR_Z*ndr[Xplaces*y+x]));
							}
						}
						for(int x=0;x<Xplaces;++x)
						{
							GL2_3D::curve_start(_3d.lineColor);
							for(int y=0;y<Yplaces;++y)
							{
								if(ndr.size()!=ndrSize)
									return;
								GL2_3D::curve_point(vec3(Xstart+x*Xsample, Ystart+y*Ysample, AR_Z*ndr[Xplaces*y+x]));
							}
						}
					}
				}
				else
				{
					unsigned yDiscOffset=(Xplaces-1)*Yplaces;
				//	auto &Xdisc=solver.Xdiscs[cursorEx], &Ydisc=solver.Ydiscs[cursorEx];
					for(unsigned c=0;c<4;++c)
					{
						auto &ndr=(&n.r)[c];
						_3d.lineColor=lColor[c];
						for(int y=0;y<Yplaces;++y)
						{
							GL2_3D::curve_start(_3d.lineColor);
							for(int x=0;x<Xplaces-1;++x)
							{
								if(ndr.size()!=ndrSize)
									return;
								GL2_3D::curve_point(vec3(Xstart+x*Xsample, Ystart+y*Ysample, AR_Z*ndr[Xplaces*y+x]));
								if(ex.discontinuities[(Xplaces-1)*y+x])
									GL2_3D::curve_start(_3d.lineColor);
							}
							GL2_3D::curve_point(vec3(Xstart+(Xplaces-1)*Xsample, Ystart+y*Ysample, AR_Z*ndr[Xplaces*y+(Xplaces-1)]));
						}
						for(int x=0;x<Xplaces;++x)
						{
							GL2_3D::curve_start(_3d.lineColor);
							for(int y=0;y<Yplaces-1;++y)
							{
								if(ndr.size()!=ndrSize)
									return;
								GL2_3D::curve_point(vec3(Xstart+x*Xsample, Ystart+y*Ysample, AR_Z*ndr[Xplaces*y+x]));
								if(ex.discontinuities[yDiscOffset+(Yplaces-1)*x+y])
									GL2_3D::curve_start(_3d.lineColor);
							}
							GL2_3D::curve_point(vec3(Xstart+x*Xsample, Ystart+(Yplaces-1)*Ysample, AR_Z*ndr[Xplaces*(Yplaces-1)+x]));
						}
					}
				}
				_3d.lineColor=0;
			}
			if(contourOn)//draw the contour
			{
				double Xstart=VX-DX/2;
				double Ystart=VY-DY/2;
				double Zstart=VZ-DX/2, Zend=VZ+DX/2;

				for(int c=0;c<4;++c)
				{
					_3d.lineColor=(&rColor)[c];
					auto &contour=(&Rcontours)[c][cursorEx];
					for(auto cit=contour.begin();cit!=contour.end();++cit)
					{
						auto &L=*cit;
						double Z=contourFlat?VZ:L.first*Zstep;
						for(auto pit=L.second.begin();pit!=L.second.end();++pit)
						{
							auto &p=*pit;
							GL2_3D::push_line_segment(vec3(p.first.x, p.first.y, AR_Z*Z), vec3(p.second.x, p.second.y, AR_Z*Z), _3d.lineColor);
						//	_3d.line(vec3(p.first.x, p.first.y, AR_Z*Z), vec3(p.second.x, p.second.y, AR_Z*Z));
						}
					}
				}
			}
			GL2_3D::end();
			GL2_3D::draw(_3d.cam);
			if(!clearScreen)
			{
				int bkMode=setBkMode(TRANSPARENT);
				for(double X=floor((VX-DX/2)/Xstep)*Xstep+Xstep, Xend=ceil((VX+DX/2)/Xstep)*Xstep;X<Xend;X+=Xstep)
					if(abs(X)>Xstep/2)
						_3d.label(X, VY, VZ, "%g", X);
				if(AR_Y==1)
				{
					for(double Y=floor((VY-DX/2)/Ystep)*Ystep+Ystep, Yend=ceil((VY+DX/2)/Ystep)*Ystep;Y<Yend;Y+=Ystep)
						if(Y<-Ystep/2||Y>Ystep/2)
							_3d.label(VX, Y, VZ, "%g", Y);
				}
				else
				{
					double aYstep=AR_Y*Ystep, Ycore=floor((VY-DX/2)/aYstep);
					for(double Y=Ycore*aYstep+aYstep, Yend=ceil((VY+DX/2)/aYstep)*aYstep, aY=Ycore*Ystep+Ystep;Y<Yend;Y+=aYstep, aY+=Ystep)
						if(Y<-aYstep/2||Y>aYstep/2)
							_3d.label(VX, Y, VZ, "%g", aY);
				}
				if(AR_Z==1)
				{
					for(double Z=floor((VZ-DX/2)/Zstep)*Zstep+Zstep, Zend=ceil((VZ+DX/2)/Zstep)*Zstep;Z<Zend;Z+=Zstep)
						if(Z<-Zstep/2||Z>Zstep/2)
							_3d.label(VX, VY, Z, "%g", Z);
				}
				else
				{
					double aZstep=AR_Z*Zstep, Zcore=floor((VZ-DX/2)/aZstep);
					for(double Z=Zcore*aZstep+aZstep, Zend=ceil((VZ+DX/2)/aZstep)*aZstep, aZ=Zcore*Zstep+Zstep;Z<Zend;Z+=aZstep, aZ+=Zstep)
						if(Z<-aZstep/2||Z>aZstep/2)
							_3d.label(VX, VY, Z, "%g", aZ);
				}
				_3d.arrowLabels(0, 0, 0, 1, 0, 0, labels.Xlabels);
				_3d.arrowLabels(0, 0, 0, 0, 1, 0, labels.Ylabels);
				char const *const OpLabels[]={"R()", "I()", "J()", "K()"};
				int const OpLabelColors[]={rColor, kColor, jColor, iColor};//bgr in windows API
				_3d.arrowLabelsColor(0, 0, 0, 0, 0, 1, OpLabels, OpLabelColors, sizeof(OpLabels)/sizeof(OpLabels[0]));
				{
					int Ys=0;
					for(int kl=0, klEnd=labels.Clabels.size();kl<klEnd;++kl)
					{
						auto &label=labels.Clabels[kl];
						print(w-const_label_offset_X, Ys, "%s=%g", label.label.c_str(), label.isTime?solver.T:label.value);
						Ys+=fontH;
					}
				}
				setBkMode(bkMode);
				_3d.text_show();
			}
		}
		void i_draw();
		void a_draw();
	} t2d_h;
	const int Transverse_2D_H::modes[]={8}, Transverse_2D_H::nmodes=sizeof(modes)>>2;
	void			Transverse_2D_H::i_draw()
	{
		if(modes::ready)
		{
			draw();
//			if(showLastModeOnIdle)
//				std::copy(rgb, rgb+w*h, modeRGB);
		}
		else
		{
		//	std::copy(modeRGB, modeRGB+w*h, rgb);
		}
		if(showBenchmark)
			Performance(0, h>>1);//
	}
	void			Transverse_2D_H::a_draw()
	{
	//	Rectangle(ghMemDC, bpx-1, bpy-1, bw+1, bh+1);

		draw();
		
		if(contextHelp)
		{
			const char *help[]=
			{
				"W/A/S/D/T/G: move",
				"arrows: turn",
				"+/-/enter/backspace/wheel: change FOV",
				"X/Y/Z arrows/mouse move: shift x/y/z",
				"X/Y/Z +/-/enter/backspace/wheel: scale x/y/z",
				"alt +/-/enter/backspace/wheel: zoom",
				"shift +/-/enter/backspace/wheel: change resolution",
				"E: reset scale",
				"R: reset scale & view",
				"C: toggle clear screen",
				"`: contour",
				"Esc: back to text editor"
			};
			print_contextHelp(help, sizeof(help)>>2, 330);
		}
		if(!clearScreen)
			Performance(0, h);//
	//	BitBlt(ghDC, 0, 0, w, h, ghMemDC, 0, 0, SRCCOPY);
	}
	namespace contour
	{
		struct Double_X_Y_Z_V
		{
			double X, Y, Z, V;
			Double_X_Y_Z_V(){}
			Double_X_Y_Z_V(double X, double Y, double Z, double V):X(X), Y(Y), Z(Z), V(V){}
			void operator()(double X, double Y, double Z, double V){this->X=X, this->Y=Y, this->Z=Z, this->V=V;}
		};
		struct Double_X_Y_Z
		{
			double X, Y, Z;
			Double_X_Y_Z(double X, double Y, double Z):X(X), Y(Y), Z(Z){}
		};
		void solve_cubic(double a, double b, double c, double d, double &r1, std::complex<double> &r2, std::complex<double> &r3)
		{
			//http://easycalculation.com/algebra/learn-cubic-equation.php
			//http://stackoverflow.com/questions/13328676/c-solving-cubic-equations
			if(a==0)
			{
				r1=_HUGE;
				solve_quadratic(b, c, d, r2, r3);
			}
			else if(d==0)
			{
				r1=0;
				solve_quadratic(a, b, c, r2, r3);
			}
			else
			{
				b/=a, c/=a, d/=a;
				double disc, q, r, dum1, s, t, term1, r13;
				q=(3.0*c-(b*b))/9.0;
				r=-(27.0*d)+b*(9.0*c-2.0*(b*b));
				r/=54.0;
				disc=q*q*q+r*r;
				term1=b/3;
				if(disc>0)
				{
					s=r+std::sqrt(disc);
					s=s<0?-std::pow(-s, 1./3):std::pow(s, 1./3);
					t=r-std::sqrt(disc);
					t=t<0?-std::pow(-t, 1./3):std::pow(t, 1./3);
					r1=-term1+s+t;//The first root is always real
					term1+=(s+t)/2;
					double term2=G2::_sqrt3*(-t+s)/2;
					r2=std::complex<double>(-term1, term2);
					r3=std::complex<double>(-term1, -term2);
				}
				else if(disc==0)//The remaining options are all real
				{
					r13=r<0?-std::pow(-r, 1./3):std::pow(r, 1./3);
					r1=-term1+2*r13;
					r3=r2=-(r13+term1);//at least two are equal
				}
				else//Only option left is that all roots are real and unequal (to get here, q < 0)
				{
					q = -q;
					dum1 = q*q*q;
					dum1 = std::acos(r/std::sqrt(dum1));
					r13 = 2.0*std::sqrt(q);
					r1=-term1+r13*std::cos(dum1/3);
					r2=-term1+r13*std::cos((dum1+2*G2::_pi)/3);
					r3=-term1+r13*std::cos((dum1+4*G2::_pi)/3);
				}
			}
		}
	}
	int n_components_in_current_expr()
	{
		int nComponents=0;
		switch(expr[cursorEx].resultMathSet)//function
		{
		case 'R':nComponents=1;break;
		case 'c':nComponents=2;break;
		case 'h':nComponents=4;break;
		}
		return nComponents;
	}
	void apply_kernel_comp(Term &n, int component, const double *kernel, int Xplaces, int Yplaces, int Zplaces, int ndrSize)
	{
		auto &ndr=(&n.r)[component];
		std::vector<double> temp(ndrSize);
		auto &t=temp;
		int XYplaces=Xplaces*Yplaces;
		for(int kz=0;kz<Zplaces;++kz)
		{
			for(int ky=0;ky<Yplaces;++ky)
			{
				for(int kx=0;kx<Xplaces;++kx)	//27n^3 muls
				{
					int idx=Yplaces*(Xplaces*kz+ky)+kx;
					int kz_1=kz-1+Zplaces, kz1=kz+1,
						ky_1=ky-1+Yplaces, ky1=ky+1,
						kx_1=kz-1+Xplaces, kx1=kx+1;
					kz_1-=Zplaces&-(kz_1>=Zplaces), kz1-=Zplaces&-(kz1>=Zplaces);
					ky_1-=Yplaces&-(ky_1>=Yplaces), ky1-=Yplaces&-(ky1>=Yplaces);
					kx_1-=Xplaces&-(kx_1>=Xplaces), kx1-=Xplaces&-(kx1>=Xplaces);
						kz_1*=XYplaces, ky_1*=Xplaces;
					int kz_0=kz*XYplaces, ky_0=ky*Xplaces;
						kz1*=XYplaces, ky1*=Xplaces;
					t[idx]=
						kernel[ 0]*ndr[kz_1+ky_1+kx_1]+kernel[ 1]*ndr[kz_1+ky_1+kx]+kernel[ 2]*ndr[kz_1+ky_1+kx1]+
						kernel[ 3]*ndr[kz_1+ky_0+kx_1]+kernel[ 4]*ndr[kz_1+ky_0+kx]+kernel[ 5]*ndr[kz_1+ky_0+kx1]+
						kernel[ 6]*ndr[kz_1+ky1 +kx_1]+kernel[ 7]*ndr[kz_1+ky1 +kx]+kernel[ 8]*ndr[kz_1+ky1 +kx1]+

						kernel[ 9]*ndr[kz_0+ky_1+kx_1]+kernel[10]*ndr[kz_0+ky_1+kx]+kernel[11]*ndr[kz_0+ky_1+kx1]+
						kernel[12]*ndr[kz_0+ky_0+kx_1]+kernel[13]*ndr[kz_0+ky_0+kx]+kernel[14]*ndr[kz_0+ky_0+kx1]+
						kernel[15]*ndr[kz_0+ky1 +kx_1]+kernel[16]*ndr[kz_0+ky1 +kx]+kernel[17]*ndr[kz_0+ky1 +kx1]+

						kernel[18]*ndr[kz1 +ky_1+kx_1]+kernel[19]*ndr[kz1 +ky_1+kx]+kernel[20]*ndr[kz1 +ky_1+kx1]+
						kernel[21]*ndr[kz1 +ky_0+kx_1]+kernel[22]*ndr[kz1 +ky_0+kx]+kernel[23]*ndr[kz1 +ky_0+kx1]+
						kernel[24]*ndr[kz1 +ky1 +kx_1]+kernel[25]*ndr[kz1 +ky1 +kx]+kernel[26]*ndr[kz1 +ky1 +kx1];
				}
			}
		}
		(&n.r)[component]=std::move(temp);
	}
	void apply_kernel(const double *kernel, int Xplaces, int Yplaces, int Zplaces, int ndrSize)
	{
		int nComponents=n_components_in_current_expr();
		for(int c=0;c<nComponents;++c)
			apply_kernel_comp(expr[cursorEx].n[expr[cursorEx].resultTerm], c, kernel, Xplaces, Yplaces, Zplaces, ndrSize);
	}
	class		Color_3D:public _3D_Mode
	{
	public:
		int Xoffset, Yoffset, Zoffset;
		int X0, Y0;

		int Xplaces, Yplaces, Zplaces;
		double XshiftPoint, YshiftPoint, ZshiftPoint;
		double XsamplePos, YsamplePos, ZsamplePos;

		Solve_3D solver;
		std::list<int> operations;

		bool contourOn, contourOnly, contourFlat;
		std::map<unsigned, std::unordered_map<int, std::list<Triangle>>>
			Rcontours,//contours[e]=contour, contour[r level (r/step)]=triangles, *triangles.begin()=triangle
			Icontours,//contours[e]=contour, contour[i level (i/step)]=triangles, *triangles.begin()=triangle
			Jcontours,//contours[e]=contour, contour[j level (j/step)]=triangles, *triangles.begin()=triangle
			Kcontours;//contours[e]=contour, contour[k level (k/step)]=triangles, *triangles.begin()=triangle
		std::map<unsigned, std::unordered_map<int, std::list<Stick>>>
			Rlines,//contours[e]=contour, contour[r level (r/step)]=sticks, *sticks.begin()=stick
			Ilines,//contours[e]=contour, contour[i level (i/step)]=sticks, *sticks.begin()=stick
			Jlines,//contours[e]=contour, contour[j level (j/step)]=sticks, *sticks.begin()=stick
			Klines;//contours[e]=contour, contour[k level (k/step)]=sticks, *sticks.begin()=stick
		
		bool clearScreen, kb_VK_F6_msg;
		Labels_3D labels;
		static const int rColor=0, iColor=0xFFEF0000, jColor=0xFF00EF00, kColor=0xFF0000EF;
		_3D _3d;
		int gridColor;
		static const int modes[], nmodes;

		Color_3D():
			_3D_Mode(solver),
			Xplaces(10), Yplaces(10), Zplaces(10),
			XshiftPoint(0), YshiftPoint(0), ZshiftPoint(0),
			XsamplePos(0), YsamplePos(0), ZsamplePos(0),
			_3d(20, 20, 20, 225*G2::_pi/180, 324.7356103172454*G2::_pi/180, 1),
			gridColor(_3dGridColor),
			Xoffset(0), Yoffset(0),
			contourOn(false), contourOnly(false), contourFlat(false),
			clearScreen(false), kb_VK_F6_msg(false)
		{}
		
		void draw_contour(std::map<unsigned, std::unordered_map<int, std::list<Triangle>>> &contours, std::map<unsigned, std::unordered_map<int, std::list<Stick>>> &lines, unsigned lineColor)
		{
			auto &contour=contours[cursorEx];
			for(auto cit=contour.begin();cit!=contour.end();++cit)
			{
				auto &L=*cit;
				double RL=10*L.first*Xstep;
				int color=colorFunction_r(RL);
				if(!color)
					color=0xFFD0D0D0;
				for(auto pit=L.second.begin();pit!=L.second.end();++pit)
				{
					auto &p=*pit;
					GL2_3D::push_triangle(vec3(p.X1, p.Y1, p.Z1), vec3(p.X2, p.Y2, p.Z2), vec3(p.X3, p.Y3, p.Z3), 0x7FFFFFFF&color);
				//	_3d.triangle_halfTransparent(p, color);
				}
			}
			_3d.lineColor=lineColor;
			//for(auto &L:contours[cursorEx])//contour mesh
			//{
			//	for(auto &p:L.second)
			//	{
			//		GL2_3D::push_line_segment(vec3(p.X1, p.Y1, p.Z1), vec3(p.X2, p.Y2, p.Z2), lineColor);
			//		GL2_3D::push_line_segment(vec3(p.X2, p.Y2, p.Z2), vec3(p.X3, p.Y3, p.Z3), lineColor);
			//		GL2_3D::push_line_segment(vec3(p.X3, p.Y3, p.Z3), vec3(p.X1, p.Y1, p.Z1), lineColor);
			//	}
			//}
			auto &line=lines[cursorEx];
			for(auto vit=line.begin();vit!=line.end();++vit)
			{
				auto &Vlevel=*vit;
				for(auto sit=Vlevel.second.begin();sit!=Vlevel.second.end();++sit)
				{
					auto &s=*sit;
					GL2_3D::push_line_segment(vec3(s.X1, s.Y1, s.Z1), vec3(s.X2, s.Y2, s.Z2), lineColor);
				//	_3d.line(s.X1, s.Y1, s.Z1, s.X2, s.Y2, s.Z2);
				}
			}
		}
		void draw_contourGrid(double Xs)
		{
			unsigned e=cursorEx, c=0;
			using namespace contour;
			auto &ex=expr[e];
			auto &ndr=(&ex.n[ex.resultTerm].r)[c];
			_3d.lineColor=0xFFD0D0D0;

			double X0, X1, Y0, Y1, Z0, Z1, V000, V001, V010, V011, V100, V101, V110, V111;
			auto interpolate=[&](double Xa, double Ya, double Za)->double
			{
				double dX0=Xa-X0, dX1=X1-Xa, dY0=(Ya-Y0), dY1=(Y1-Ya);
				return
					(	(Z1-Za)*	(	dY1*	(	dX1*V000+
													dX0*V001	)+
										dY0*	(	dX1*V010+
													dX0*V011	))+
						(Za-Z0)*	(	dY1*	(	dX1*V100+
													dX0*V101	)+
										dY0*	(	dX1*V110+
													dX0*V111	))
					)/(Z1-Z0)/(Y1-Y0)/(X1-X0);
			};

			auto cutTrap=[&](Double_X_Y_Z_V &A, Double_X_Y_Z_V &B, Double_X_Y_Z_V &C, Double_X_Y_Z_V &D)
			{
				GL2_3D::push_line_segment(vec3(A.X, A.Y, A.Z), vec3(B.X, B.Y, B.Z), _3d.lineColor);
				GL2_3D::push_line_segment(vec3(B.X, B.Y, B.Z), vec3(C.X, C.Y, C.Z), _3d.lineColor);
				GL2_3D::push_line_segment(vec3(C.X, C.Y, C.Z), vec3(D.X, D.Y, D.Z), _3d.lineColor);
				GL2_3D::push_line_segment(vec3(D.X, D.Y, D.Z), vec3(A.X, A.Y, A.Z), _3d.lineColor);
			};
			auto print=[&](Double_X_Y_Z_V &P){_3d.textIn3D(P.X, P.Y, P.Z, OPAQUE, "%g", P.V);};
			for(unsigned vz=0, vzEnd=Zplaces-1;vz<vzEnd;++vz)
			{
				for(unsigned vy=0, vyEnd=Yplaces-1;vy<vyEnd;++vy)
				{
					for(unsigned vx=0, vxEnd=Xplaces-1;vx<vxEnd;++vx)
					{
						X0=solver.Xstart+Xs*vx, X1=solver.Xstart+Xs*(vx+1);
						Y0=solver.Ystart+Xs*vy, Y1=solver.Ystart+Xs*(vy+1);
						Z0=solver.Zstart+Xs*vz, Z1=solver.Zstart+Xs*(vz+1);
						double Xm=solver.Xstart+Xs*(vx+.5), Ym=solver.Ystart+Xs*(vy+.5), Zm=solver.Zstart+Xs*(vz+.5);
							
						Double_X_Y_Z_V
							P000(X0, Y0, Z0, V000=ndr[Xplaces*(Yplaces* vz   +vy  )+vx  ]),
							P001(X1, Y0, Z0, V001=ndr[Xplaces*(Yplaces* vz   +vy  )+vx+1]),
							P010(X0, Y1, Z0, V010=ndr[Xplaces*(Yplaces* vz   +vy+1)+vx  ]),
							P011(X1, Y1, Z0, V011=ndr[Xplaces*(Yplaces* vz   +vy+1)+vx+1]),
							P100(X0, Y0, Z1, V100=ndr[Xplaces*(Yplaces*(vz+1)+vy  )+vx  ]),
							P101(X1, Y0, Z1, V101=ndr[Xplaces*(Yplaces*(vz+1)+vy  )+vx+1]),
							P110(X0, Y1, Z1, V110=ndr[Xplaces*(Yplaces*(vz+1)+vy+1)+vx  ]),
							P111(X1, Y1, Z1, V111=ndr[Xplaces*(Yplaces*(vz+1)+vy+1)+vx+1]),

							P00m(Xm, Y0, Z0, interpolate(Xm, Y0, Z0)),
							P01m(Xm, Y1, Z0, interpolate(Xm, Y1, Z0)),
							P10m(Xm, Y0, Z1, interpolate(Xm, Y0, Z1)),
							P11m(Xm, Y1, Z1, interpolate(Xm, Y1, Z1)),

							P0m0(X0, Ym, Z0, interpolate(X0, Ym, Z0)),
							P0m1(X1, Ym, Z0, interpolate(X1, Ym, Z0)),
							P1m0(X0, Ym, Z1, interpolate(X0, Ym, Z1)),
							P1m1(X1, Ym, Z1, interpolate(X1, Ym, Z1)),

							Pm00(X0, Y0, Zm, interpolate(X0, Y0, Zm)),
							Pm01(X1, Y0, Zm, interpolate(X1, Y0, Zm)),
							Pm10(X0, Y1, Zm, interpolate(X0, Y1, Zm)),
							Pm11(X1, Y1, Zm, interpolate(X1, Y1, Zm)),

							P0mm(Xm, Ym, Z0, interpolate(Xm, Ym, Z0)),
							P1mm(Xm, Ym, Z1, interpolate(Xm, Ym, Z1)),

							Pm0m(Xm, Y0, Zm, interpolate(Xm, Y0, Zm)),
							Pm1m(Xm, Y1, Zm, interpolate(Xm, Y1, Zm)),

							Pmm0(X0, Ym, Zm, interpolate(X0, Ym, Zm)),
							Pmm1(X1, Ym, Zm, interpolate(X1, Ym, Zm)),

							Pmmm(Xm, Ym, Zm, interpolate(Xm, Ym, Zm));
						
						print(P00m), print(P01m), print(P10m), print(P11m);
						print(P0m0), print(P0m1), print(P1m0), print(P1m1);
						print(Pm00), print(Pm01), print(Pm10), print(Pm11);
						print(P0mm), print(P1mm);
						print(Pm0m), print(Pm1m);
						print(Pmm0), print(Pmm1);
						print(Pmmm);
						
						cutTrap(Pm00, Pm0m, P000, Pmmm);
						cutTrap(Pm0m, P00m, P000, Pmmm);
						cutTrap(P00m, P0mm, P000, Pmmm);
						cutTrap(P0mm, P0m0, P000, Pmmm);
						cutTrap(P0m0, Pmm0, P000, Pmmm);
						cutTrap(Pmm0, Pm00, P000, Pmmm);
						
						cutTrap(Pm01, Pm0m, P001, Pmmm);
						cutTrap(Pm0m, P00m, P001, Pmmm);
						cutTrap(P00m, P0mm, P001, Pmmm);
						cutTrap(P0mm, P0m1, P001, Pmmm);
						cutTrap(P0m1, Pmm1, P001, Pmmm);
						cutTrap(Pmm1, Pm01, P001, Pmmm);
						
						cutTrap(Pm10, Pm1m, P010, Pmmm);
						cutTrap(Pm1m, P01m, P010, Pmmm);
						cutTrap(P01m, P0mm, P010, Pmmm);
						cutTrap(P0mm, P0m0, P010, Pmmm);
						cutTrap(P0m0, Pmm0, P010, Pmmm);
						cutTrap(Pmm0, Pm10, P010, Pmmm);
						
						cutTrap(Pm11, Pm1m, P011, Pmmm);
						cutTrap(Pm1m, P01m, P011, Pmmm);
						cutTrap(P01m, P0mm, P011, Pmmm);
						cutTrap(P0mm, P0m1, P011, Pmmm);
						cutTrap(P0m1, Pmm1, P011, Pmmm);
						cutTrap(Pmm1, Pm11, P011, Pmmm);
						
						cutTrap(Pm00, Pm0m, P100, Pmmm);
						cutTrap(Pm0m, P10m, P100, Pmmm);
						cutTrap(P10m, P1mm, P100, Pmmm);
						cutTrap(P1mm, P1m0, P100, Pmmm);
						cutTrap(P1m0, Pmm0, P100, Pmmm);
						cutTrap(Pmm0, Pm00, P100, Pmmm);
						
						cutTrap(Pm01, Pm0m, P101, Pmmm);
						cutTrap(Pm0m, P10m, P101, Pmmm);
						cutTrap(P10m, P1mm, P101, Pmmm);
						cutTrap(P1mm, P1m1, P101, Pmmm);
						cutTrap(P1m1, Pmm1, P101, Pmmm);
						cutTrap(Pmm1, Pm01, P101, Pmmm);
						
						cutTrap(Pm10, Pm1m, P110, Pmmm);
						cutTrap(Pm1m, P11m, P110, Pmmm);
						cutTrap(P11m, P1mm, P110, Pmmm);
						cutTrap(P1mm, P1m0, P110, Pmmm);
						cutTrap(P1m0, Pmm0, P110, Pmmm);
						cutTrap(Pmm0, Pm10, P110, Pmmm);
						
						cutTrap(Pm11, Pm1m, P111, Pmmm);
						cutTrap(Pm1m, P11m, P111, Pmmm);
						cutTrap(P11m, P1mm, P111, Pmmm);
						cutTrap(P1mm, P1m1, P111, Pmmm);
						cutTrap(P1m1, Pmm1, P111, Pmmm);
						cutTrap(Pmm1, Pm11, P111, Pmmm);
					}
				}
			}
		}
		void doContour_component(unsigned e, unsigned c, double Xs, double Vstart, double Vend, double Vstep, unsigned nvSteps)//cursorEx, comp, DX/Xplaces, -DX/2, +DX/2, Xstep, 2
		{
			using namespace contour;
			auto &ndr=(&expr[e].n[expr[e].resultTerm].r)[c];
			decltype(&Rcontours[e]) contour=0;
			switch(c)
			{
			case 0:contour=&Rcontours[e];break;
			case 1:contour=&Icontours[e];break;
			case 2:contour=&Jcontours[e];break;
			case 3:contour=&Kcontours[e];break;
			}
			double X0, X1, Y0, Y1, Z0, Z1, V000, V001, V010, V011, V100, V101, V110, V111;
			auto getPos=[&](double &t, double Vx, Double_X_Y_Z_V &A, Double_X_Y_Z_V &B)->bool
			{
				double
					Xd=B.X-A.X, X1a=X1-A.X, Xa0=A.X-X0,
					Yd=B.Y-A.Y, Y1a=Y1-A.Y, Ya0=A.Y-Y0,
					Zd=B.Z-A.Z, Z1a=Z1-A.Z, Za0=A.Z-Z0,
					
					A00=(V001-V000)*Xd, B00=X1a*V000+Xa0*V001,
					A01=(V011-V010)*Xd, B01=X1a*V010+Xa0*V011,
					A10=(V101-V100)*Xd, B10=X1a*V100+Xa0*V101,
					A11=(V111-V110)*Xd, B11=X1a*V110+Xa0*V111,
					
					C0=(A01-A00)*Yd, D0=(B01-B00)*Yd+Y1a*A00+Ya0*A01, E0=Y1a*B00+Ya0*B01,
					C1=(A11-A10)*Yd, D1=(B11-B10)*Yd+Y1a*A10+Ya0*A11, E1=Y1a*B10+Ya0*B11,
					
					a=(C1-C0)*Zd, b=(D1-D0)*Zd+Z1a*C0+Za0*C1, c=(E1-E0)*Zd+Z1a*D0+Za0*D1, d=Z1a*E0+Za0*E1-(X1-X0)*(Y1-Y0)*(Z1-Z0)*Vx;

			//	if(A.Z==Z0&&B.Z==Z0)
			//		int LOL_1=0;
				if(a==0&&b==0&&c==0)
				{
					t=(Vx-A.V)/(B.V-A.V);
					return true;
				}
				double r1;
				std::complex<double> r2, r3;
				modes::contour::solve_cubic(a, b, c, d, r1, r2, r3);
				if(r1>=0&&r1<=1)
				{
					t=r1;
					return true;
				}
				if(r2.real()>=0&&r2.real()<=1)//what
				{
					t=r2.real();
					return true;
				}
				if(r3.real()>=0&&r3.real()<=1)
				{
					t=r3.real();
					return true;
				}
				if(r1>=-1e-5&&r1<=1+1e-5)
				{
					t=r1;
					return true;
				}
				if(r2.real()>=-1e-5&&r2.real()<=1+1e-5)//what
				{
					t=r2.real();
					return true;
				}
				if(r3.real()>=-1e-5&&r3.real()<=1+1e-5)
				{
					t=r3.real();
					return true;
				}
				if(r1>=-1e-1&&r1<=1+1e-1)
				{
					t=r1;
					return true;
				}
				if(r2.real()>=-1e-1&&r2.real()<=1+1e-1)//what
				{
					t=r2.real();
					return true;
				}
				if(r3.real()>=-1e-1&&r3.real()<=1+1e-1)
				{
					t=r3.real();
					return true;
				}
				return false;
			};
			auto cutTrap=[&](Double_X_Y_Z_V &A, Double_X_Y_Z_V &B, Double_X_Y_Z_V &C, Double_X_Y_Z_V &D)
			{
				modes::contour::Double_X_Y_Z_V *_1, *_2, *_3, *_4;//sorted by V in ascending order
				if(A.V<B.V)//ab
				{
					if(B.V<C.V)//abc
					{
							 if(C.V<D.V)	_1=&A, _2=&B, _3=&C, _4=&D;
						else if(B.V<D.V)	_1=&A, _2=&B, _3=&D, _4=&C;
						else if(A.V<D.V)	_1=&A, _2=&D, _3=&B, _4=&C;
						else				_1=&D, _2=&A, _3=&B, _4=&C;
					}
					else if(A.V<C.V)//acb
					{
							 if(B.V<D.V)	_1=&A, _2=&C, _3=&B, _4=&D;
						else if(C.V<D.V)	_1=&A, _2=&C, _3=&D, _4=&B;
						else if(A.V<D.V)	_1=&A, _2=&D, _3=&C, _4=&B;
						else				_1=&D, _2=&A, _3=&C, _4=&B;
					}
					else//cab
					{
							 if(B.V<D.V)	_1=&C, _2=&A, _3=&B, _4=&D;
						else if(A.V<D.V)	_1=&C, _2=&A, _3=&D, _4=&B;
						else if(C.V<D.V)	_1=&C, _2=&D, _3=&A, _4=&B;
						else				_1=&D, _2=&C, _3=&A, _4=&B;
					}
				}
				else//ba
				{
					if(A.V<C.V)//bac
					{
							 if(C.V<D.V)	_1=&B, _2=&A, _3=&C, _4=&D;
						else if(A.V<D.V)	_1=&B, _2=&A, _3=&D, _4=&C;
						else if(B.V<D.V)	_1=&B, _2=&D, _3=&A, _4=&C;
						else				_1=&D, _2=&B, _3=&A, _4=&C;
					}
					else if(B.V<C.V)//bca
					{
							 if(A.V<D.V)	_1=&B, _2=&C, _3=&A, _4=&D;
						else if(C.V<D.V)	_1=&B, _2=&C, _3=&D, _4=&A;
						else if(B.V<D.V)	_1=&B, _2=&D, _3=&C, _4=&A;
						else				_1=&D, _2=&B, _3=&C, _4=&A;
					}
					else//cba
					{
							 if(A.V<D.V)	_1=&C, _2=&B, _3=&A, _4=&D;
						else if(B.V<D.V)	_1=&C, _2=&B, _3=&D, _4=&A;
						else if(C.V<D.V)	_1=&C, _2=&D, _3=&B, _4=&A;
						else				_1=&D, _2=&C, _3=&B, _4=&A;
					}
				}
				for(double v=std::floor((_1->V<Vstart?Vstart:_1->V)/Vstep), vEnd=std::floor((_2->V>Vend?Vend:_2->V)/Vstep);v<=vEnd;++v)
				{
					double VL=Vstep*v;
					if(_1->V<=VL&&VL<_2->V)
					{
						double M12, M13, M14;
						if(getPos(M12, VL, *_1, *_2)&&getPos(M13, VL, *_1, *_3)&&getPos(M14, VL, *_1, *_4))
						{
							contour->operator[](int(v)).push_back(Triangle(
								_1->X+M12*(_2->X-_1->X), _1->Y+M12*(_2->Y-_1->Y), _1->Z+M12*(_2->Z-_1->Z),
								_1->X+M13*(_3->X-_1->X), _1->Y+M13*(_3->Y-_1->Y), _1->Z+M13*(_3->Z-_1->Z),
								_1->X+M14*(_4->X-_1->X), _1->Y+M14*(_4->Y-_1->Y), _1->Z+M14*(_4->Z-_1->Z)
								));
						}
					}
				}
				for(double v=std::floor((_2->V<Vstart?Vstart:_2->V)/Vstep), vEnd=std::floor((_3->V>Vend?Vend:_3->V)/Vstep);v<=vEnd;++v)
				{
					double VL=Vstep*v;
					if(_2->V<=VL&&VL<_3->V)
					{
						double M13, M23, M24, M14;
						if(getPos(M13, VL, *_1, *_3)&&getPos(M23, VL, *_2, *_3)&&getPos(M24, VL, *_2, *_4)&&getPos(M14, VL, *_1, *_4))
						{
							double
								X13=_1->X+M13*(_3->X-_1->X), Y13=_1->Y+M13*(_3->Y-_1->Y), Z13=_1->Z+M13*(_3->Z-_1->Z),
								X23=_2->X+M23*(_3->X-_2->X), Y23=_2->Y+M23*(_3->Y-_2->Y), Z23=_2->Z+M23*(_3->Z-_2->Z),
								X24=_2->X+M24*(_4->X-_2->X), Y24=_2->Y+M24*(_4->Y-_2->Y), Z24=_2->Z+M24*(_4->Z-_2->Z),
								X14=_1->X+M14*(_4->X-_1->X), Y14=_1->Y+M14*(_4->Y-_1->Y), Z14=_1->Z+M14*(_4->Z-_1->Z);
							contour->operator[](int(v)).push_back(Triangle(X13, Y13, Z13, X23, Y23, Z23, X24, Y24, Z24));
							contour->operator[](int(v)).push_back(Triangle(X13, Y13, Z13, X14, Y14, Z14, X24, Y24, Z24));
						}
					}
				}
				for(double v=std::floor((_3->V<Vstart?Vstart:_3->V)/Vstep), vEnd=std::floor((_4->V>Vend?Vend:_4->V)/Vstep);v<=vEnd;++v)
				{
					double VL=Vstep*v;
					if(_3->V<=VL&&VL<_4->V)
					{
						double M14, M24, M34;
						if(getPos(M14, VL, *_1, *_4)&&getPos(M24, VL, *_2, *_4)&&getPos(M34, VL, *_3, *_4))
						{
							contour->operator[](int(v)).push_back(Triangle(
								_1->X+M14*(_4->X-_1->X), _1->Y+M14*(_4->Y-_1->Y), _1->Z+M14*(_4->Z-_1->Z),
								_2->X+M24*(_4->X-_2->X), _2->Y+M24*(_4->Y-_2->Y), _2->Z+M24*(_4->Z-_2->Z),
								_3->X+M34*(_4->X-_3->X), _3->Y+M34*(_4->Y-_3->Y), _3->Z+M34*(_4->Z-_3->Z)
								));
						}
					}
				}
			};
			double dispYstart=AR_Y*solver.Ystart, dispZstart=AR_Z*solver.Zstart;
			for(unsigned vz=0, vzEnd=Zplaces-1;vz<vzEnd;++vz)
			{
				for(unsigned vy=0, vyEnd=Yplaces-1;vy<vyEnd;++vy)
				{
					for(unsigned vx=0, vxEnd=Xplaces-1;vx<vxEnd;++vx)
					{
						X0=solver.Xstart+Xs*vx, X1=solver.Xstart+Xs*(vx+1);
						Y0=   dispYstart+Xs*vy, Y1=   dispYstart+Xs*(vy+1);
						Z0=   dispZstart+Xs*vz, Z1=   dispZstart+Xs*(vz+1);
						double Xm=solver.Xstart+Xs*(vx+.5), Ym=dispYstart+Xs*(vy+.5), Zm=dispZstart+Xs*(vz+.5);		//display env units		faster

					//	X0=solver.Xstart+Xs*vx, X1=solver.Xstart+Xs*(vx+1);
					//	Y0=AR_Y*solver.Ystart+Xs*vy, Y1=AR_Y*solver.Ystart+Xs*(vy+1);
					//	Z0=AR_Z*solver.Zstart+Xs*vz, Z1=AR_Z*solver.Zstart+Xs*(vz+1);
					//	double Xm=solver.Xstart+Xs*(vx+.5), Ym=AR_Y*solver.Ystart+Xs*(vy+.5), Zm=AR_Z*solver.Zstart+Xs*(vz+.5);		//display env units		faster

					//	X0=solver.Xstart+Xs*vx, X1=solver.Xstart+Xs*(vx+1);
					//	Y0=solver.Ystart+Xs/AR_Y*vy, Y1=solver.Ystart+Xs/AR_Y*(vy+1);
					//	Z0=solver.Zstart+Xs/AR_Z*vz, Z1=solver.Zstart+Xs/AR_Z*(vz+1);
					//	double Xm=solver.Xstart+Xs*(vx+.5), Ym=solver.Ystart+Xs/AR_Y*(vy+.5), Zm=solver.Zstart+Xs/AR_Z*(vz+.5);		//math env units		multiply by gain at draw
							
						Double_X_Y_Z_V//zyx matrix notation
							P000(X0, Y0, Z0, V000=ndr[Xplaces*(Yplaces* vz   +vy  )+vx  ]),
							P001(X1, Y0, Z0, V001=ndr[Xplaces*(Yplaces* vz   +vy  )+vx+1]),
							P010(X0, Y1, Z0, V010=ndr[Xplaces*(Yplaces* vz   +vy+1)+vx  ]),
							P011(X1, Y1, Z0, V011=ndr[Xplaces*(Yplaces* vz   +vy+1)+vx+1]),
							P100(X0, Y0, Z1, V100=ndr[Xplaces*(Yplaces*(vz+1)+vy  )+vx  ]),
							P101(X1, Y0, Z1, V101=ndr[Xplaces*(Yplaces*(vz+1)+vy  )+vx+1]),
							P110(X0, Y1, Z1, V110=ndr[Xplaces*(Yplaces*(vz+1)+vy+1)+vx  ]),
							P111(X1, Y1, Z1, V111=ndr[Xplaces*(Yplaces*(vz+1)+vy+1)+vx+1]),
							
							P00m(Xm, Y0, Z0, .5*(V000+V001)),
							P01m(Xm, Y1, Z0, .5*(V010+V011)),
							P10m(Xm, Y0, Z1, .5*(V100+V101)),
							P11m(Xm, Y1, Z1, .5*(V110+V111)),

							P0m0(X0, Ym, Z0, .5*(V000+V010)),
							P0m1(X1, Ym, Z0, .5*(V001+V011)),
							P1m0(X0, Ym, Z1, .5*(V100+V110)),
							P1m1(X1, Ym, Z1, .5*(V101+V111)),

							Pm00(X0, Y0, Zm, .5*(V000+V100)),
							Pm01(X1, Y0, Zm, .5*(V001+V101)),
							Pm10(X0, Y1, Zm, .5*(V010+V110)),
							Pm11(X1, Y1, Zm, .5*(V011+V111)),

							P0mm(Xm, Ym, Z0, .25*(V000+V001+V010+V011)),
							P1mm(Xm, Ym, Z1, .25*(V100+V101+V110+V111)),

							Pm0m(Xm, Y0, Zm, .25*(V000+V001+V100+V101)),
							Pm1m(Xm, Y1, Zm, .25*(V010+V011+V110+V111)),

							Pmm0(X0, Ym, Zm, .25*(V000+V010+V100+V110)),
							Pmm1(X1, Ym, Zm, .25*(V001+V011+V101+V111)),

							Pmmm(Xm, Ym, Zm, .125*(V000+V001+V010+V011+V100+V101+V110+V111));

						cutTrap(Pm00, Pm0m, P000, Pmmm);
						cutTrap(Pm0m, P00m, P000, Pmmm);
						cutTrap(P00m, P0mm, P000, Pmmm);
						cutTrap(P0mm, P0m0, P000, Pmmm);
						cutTrap(P0m0, Pmm0, P000, Pmmm);
						cutTrap(Pmm0, Pm00, P000, Pmmm);
						
						cutTrap(Pm01, Pm0m, P001, Pmmm);
						cutTrap(Pm0m, P00m, P001, Pmmm);
						cutTrap(P00m, P0mm, P001, Pmmm);
						cutTrap(P0mm, P0m1, P001, Pmmm);
						cutTrap(P0m1, Pmm1, P001, Pmmm);
						cutTrap(Pmm1, Pm01, P001, Pmmm);
						
						cutTrap(Pm10, Pm1m, P010, Pmmm);
						cutTrap(Pm1m, P01m, P010, Pmmm);
						cutTrap(P01m, P0mm, P010, Pmmm);
						cutTrap(P0mm, P0m0, P010, Pmmm);
						cutTrap(P0m0, Pmm0, P010, Pmmm);
						cutTrap(Pmm0, Pm10, P010, Pmmm);
						
						cutTrap(Pm11, Pm1m, P011, Pmmm);
						cutTrap(Pm1m, P01m, P011, Pmmm);
						cutTrap(P01m, P0mm, P011, Pmmm);
						cutTrap(P0mm, P0m1, P011, Pmmm);
						cutTrap(P0m1, Pmm1, P011, Pmmm);
						cutTrap(Pmm1, Pm11, P011, Pmmm);
						
						cutTrap(Pm00, Pm0m, P100, Pmmm);
						cutTrap(Pm0m, P10m, P100, Pmmm);
						cutTrap(P10m, P1mm, P100, Pmmm);
						cutTrap(P1mm, P1m0, P100, Pmmm);
						cutTrap(P1m0, Pmm0, P100, Pmmm);
						cutTrap(Pmm0, Pm00, P100, Pmmm);
						
						cutTrap(Pm01, Pm0m, P101, Pmmm);
						cutTrap(Pm0m, P10m, P101, Pmmm);
						cutTrap(P10m, P1mm, P101, Pmmm);
						cutTrap(P1mm, P1m1, P101, Pmmm);
						cutTrap(P1m1, Pmm1, P101, Pmmm);
						cutTrap(Pmm1, Pm01, P101, Pmmm);
						
						cutTrap(Pm10, Pm1m, P110, Pmmm);
						cutTrap(Pm1m, P11m, P110, Pmmm);
						cutTrap(P11m, P1mm, P110, Pmmm);
						cutTrap(P1mm, P1m0, P110, Pmmm);
						cutTrap(P1m0, Pmm0, P110, Pmmm);
						cutTrap(Pmm0, Pm10, P110, Pmmm);
						
						cutTrap(Pm11, Pm1m, P111, Pmmm);
						cutTrap(Pm1m, P11m, P111, Pmmm);
						cutTrap(P11m, P1mm, P111, Pmmm);
						cutTrap(P1mm, P1m1, P111, Pmmm);
						cutTrap(P1m1, Pmm1, P111, Pmmm);
						cutTrap(Pmm1, Pm11, P111, Pmmm);
					}
				}
			}
			decltype(&Rlines[e]) lines=0;
			switch(e)
			{
			case 0:lines=&Rlines[e];break;
			case 1:lines=&Ilines[e];break;
			case 2:lines=&Jlines[e];break;
			case 3:lines=&Klines[e];break;
			}
			for(auto cit=contour->begin();cit!=contour->end();++cit)
			{
				auto &level=*cit;
				for(auto tit=level.second.begin();tit!=level.second.end();++tit)
				{
					auto &T=*tit;
					double *X1, *Y1, *V1, *X2, *Y2, *V2, *X3, *Y3, *V3;
					if(T.Z1<T.Z2)//12
					{
							 if(T.Z2<T.Z3)	X1=&T.X1, Y1=&T.Y1, V1=&T.Z1, X2=&T.X2, Y2=&T.Y2, V2=&T.Z2, X3=&T.X3, Y3=&T.Y3, V3=&T.Z3;
						else if(T.Z1<T.Z3)	X1=&T.X1, Y1=&T.Y1, V1=&T.Z1, X2=&T.X3, Y2=&T.Y3, V2=&T.Z3, X3=&T.X2, Y3=&T.Y2, V3=&T.Z2;
						else				X1=&T.X3, Y1=&T.Y3, V1=&T.Z3, X2=&T.X1, Y2=&T.Y1, V2=&T.Z1, X3=&T.X2, Y3=&T.Y2, V3=&T.Z2;
					}
					else//21
					{
							 if(T.Z1<T.Z3)	X1=&T.X2, Y1=&T.Y2, V1=&T.Z2, X2=&T.X1, Y2=&T.Y1, V2=&T.Z1, X3=&T.X3, Y3=&T.Y3, V3=&T.Z3;
						else if(T.Z2<T.Z3)	X1=&T.X2, Y1=&T.Y2, V1=&T.Z2, X2=&T.X3, Y2=&T.Y3, V2=&T.Z3, X3=&T.X1, Y3=&T.Y1, V3=&T.Z1;
						else				X1=&T.X3, Y1=&T.Y3, V1=&T.Z3, X2=&T.X2, Y2=&T.Y2, V2=&T.Z2, X3=&T.X1, Y3=&T.Y1, V3=&T.Z1;
					}
					double X13, Y13;
					{
						double Zr=(*V2-*V1)/(*V3-*V1);
						X13=*X1+Zr*(*X3-*X1), Y13=*Y1+Zr*(*Y3-*Y1);//V13=V2;
					}
					//for V1 -> V2
					for(double v=std::floor((*V1<Vstart?Vstart:*V1)/Vstep), zEnd=std::floor((*V2>Vend?Vend:*V2)/Vstep);v<=zEnd;++v)
					{
						double ZL=Vstep*v;
						if(*V1<=ZL&&ZL<*V2)
						{
							double M=(ZL-*V1)/(*V2-*V1);
							lines->operator[](level.first).push_back(Stick(*X1+M*(*X2-*X1), *Y1+M*(*Y2-*Y1), ZL, *X1+M*(X13-*X1), *Y1+M*(Y13-*Y1), ZL));
						}
					}
					//for V2 -> V3
					for(double v=std::floor((*V2<Vstart?Vstart:*V2)/Vstep), zEnd=std::floor((*V3>Vend?Vend:*V3)/Vstep);v<=zEnd;++v)
					{
						double ZL=Vstep*v;
						if(*V2<=ZL&&ZL<*V3)
						{
							double M=(ZL-*V2)/(*V3-*V2);
							lines->operator[](level.first).push_back(Stick(*X2+M*(*X3-*X2), *Y2+M*(*Y3-*Y2), ZL, X13+M*(*X3-X13), Y13+M*(*Y3-Y13), ZL));
						}
					}
				}
			}
		}
		void doContour(unsigned e, double Xs, double Vstart, double Vend, double Vstep, unsigned nvSteps)
		{
			Vstep*=10;//
			switch(expr[e].resultMathSet)
			{
			case 'R':
				doContour_component(e, 0, Xs, Vstart, Vend, Vstep, nvSteps);
				break;
			case 'c':
				doContour_component(e, 0, Xs, Vstart, Vend, Vstep, nvSteps);
				doContour_component(e, 1, Xs, Vstart, Vend, Vstep, nvSteps);
				break;
			case 'h':
				doContour_component(e, 0, Xs, Vstart, Vend, Vstep, nvSteps);
				doContour_component(e, 1, Xs, Vstart, Vend, Vstep, nvSteps);
				doContour_component(e, 2, Xs, Vstart, Vend, Vstep, nvSteps);
				doContour_component(e, 3, Xs, Vstart, Vend, Vstep, nvSteps);
				break;
			}
		}
		void doOperations						(Term &n)
		{
			for(auto it=operations.begin();it!=operations.end();++it)
			{
				auto &op=*it;
				switch(op)
				{
				case  1:differentiate_xyz					(n);break;
				case  2:differentiate_x						(n);break;
				case  3:differentiate_y						(n);break;
				case  4:differentiate_z						(n);break;
				case  5:differentiate_xy					(n);break;
				case  6:differentiate_yz					(n);break;
				case  7:differentiate_xz					(n);break;
				case  8:integrate_xyz						(n);break;
				case  9:integrate_x							(n);break;
				case 10:integrate_y							(n);break;
				case 11:integrate_z							(n);break;
				case 12:integrate_xy						(n);break;
				case 13:integrate_yz						(n);break;
				case 14:integrate_xz						(n);break;
				case 15:discreteFourrierTransform			(n);break;
				case 16:inverseDiscreteFourrierTransform	(n);break;
				case 17:lowPassFilter_xyz					(n);break;
				case 18:lowPassFilter_x						(n);break;
				case 19:lowPassFilter_y						(n);break;
				case 20:lowPassFilter_z						(n);break;
				case 21:lowPassFilter_xy					(n);break;
				case 22:lowPassFilter_yz					(n);break;
				case 23:lowPassFilter_xz					(n);break;
				case 24:highPassFilter_xyz					(n);break;
				case 25:highPassFilter_x					(n);break;
				case 26:highPassFilter_y					(n);break;
				case 27:highPassFilter_z					(n);break;
				case 28:highPassFilter_xy					(n);break;
				case 29:highPassFilter_yz					(n);break;
				case 30:highPassFilter_xz					(n);break;
				}
			}
		}
		void differentiate_xyz					(Term &n)
		{
			double step=DX/Xplaces;
			double kernel[]=
			{	//y=-1		y=0					y=1
				0, 0, 0,	0, 0,       0,		0, 0,      0,//z=-1
				0, 0, 0,	0, -3/step, 1,		0, 1/step, 0,//z=0
				0, 0, 0,	0, 1/step,  0,		0, 0,      0 //z=1
			};
			apply_kernel(kernel, Xplaces, Yplaces, Zplaces, ndrSize);
		}
		void differentiate_x					(Term &n)
		{
			double step=DX/Xplaces;
			double kernel[]=
			{	//y=-1		y=0						y=1
				0, 0, 0,	0, 0,       0,			0, 0, 0,//z=-1
				0, 0, 0,	0, -1/step, 1/step,		0, 0, 0,//z=0
				0, 0, 0,	0, 0,       0,			0, 0, 0 //z=1
			};
			apply_kernel(kernel, Xplaces, Yplaces, Zplaces, ndrSize);
		}
		void differentiate_y					(Term &n)
		{
			double step=DX/Xplaces;
			double kernel[]=
			{	//y=-1		y=0						y=1
				0, 0, 0,	0, 0,       0,		0, 0,      0,//z=-1
				0, 0, 0,	0, -1/step, 0,		0, 1/step, 0,//z=0
				0, 0, 0,	0, 0,       0,		0, 0,      0 //z=1
			};
			apply_kernel(kernel, Xplaces, Yplaces, Zplaces, ndrSize);
		}
		void differentiate_z					(Term &n)
		{
			double step=DX/Xplaces;
			double kernel[]=
			{	//y=-1		y=0						y=1
				0, 0, 0,	0, 0,       0,		0, 0, 0,//z=-1
				0, 0, 0,	0, -1/step, 0,		0, 0, 0,//z=0
				0, 0, 0,	0, 1/step,  0,		0, 0, 0 //z=1
			};
			apply_kernel(kernel, Xplaces, Yplaces, Zplaces, ndrSize);
		}
		void differentiate_xy					(Term &n)
		{
			double step=DX/Xplaces;
			double kernel[]=
			{	//y=-1		y=0						y=1
				0, 0, 0,	0, 0,       0,			0, 0,      0,//z=-1
				0, 0, 0,	0, -2/step, 1/step,		0, 1/step, 0,//z=0
				0, 0, 0,	0, 0,       0,			0, 0,      0 //z=1
			};
			apply_kernel(kernel, Xplaces, Yplaces, Zplaces, ndrSize);
		}
		void differentiate_yz					(Term &n)
		{
			double step=DX/Xplaces;
			double kernel[]=
			{	//y=-1		y=0						y=1
				0, 0, 0,	0, 0,       0,		0, 0,      0,//z=-1
				0, 0, 0,	0, -2/step, 0,		0, 1/step, 0,//z=0
				0, 0, 0,	0, 1/step,  0,		0, 0,      0 //z=1
			};
			apply_kernel(kernel, Xplaces, Yplaces, Zplaces, ndrSize);
		}
		void differentiate_xz					(Term &n)
		{
			double step=DX/Xplaces;
			double kernel[]=
			{	//y=-1		y=0						y=1
				0, 0, 0,	0, 0,       0,			0, 0, 0,//z=-1
				0, 0, 0,	0, -2/step, 1/step,		0, 0, 0,//z=0
				0, 0, 0,	0, 1/step,  0,			0, 0, 0 //z=1
			};
			apply_kernel(kernel, Xplaces, Yplaces, Zplaces, ndrSize);
		}
		void integrate_xyz						(Term &n)
		{
			double step=DX/Xplaces;
			int nComponents=n_components_in_current_expr();
			std::vector<double> Zsum[4], ZYsum[4];
			double ZYXsum[4]={0};
			int ZsumSize=Xplaces*Yplaces;
			for(int c=0;c<nComponents;++c)
			{
				auto &ndr=(&n.r)[c];
				Zsum[c].resize(ZsumSize), ZYsum[c].resize(Xplaces);
				for(int vz=0;vz<Zplaces;++vz)
				{
					for(int vy=0;vy<Yplaces;++vy)
					{
						for(int vx=0;vx<Xplaces;++vx)
						{
							auto &V000=ndr[Xplaces*(Yplaces*vz+vy)+vx];
							V000=(ZYXsum[c]+=ZYsum[c][vx]+=Zsum[c][Xplaces*vy+vx]+=V000)*step;
						}
						ZYXsum[c]=0;
					}
					std::fill(ZYsum[c].begin(), ZYsum[c].end(), 0);
				}
			}//*/
		}
		void integrate_x						(Term &n)
		{
			double step=DX/Xplaces;
			int nComponents=n_components_in_current_expr();
			double Xsum[4]={0};
			for(int c=0;c<nComponents;++c)
			{
				auto &ndr=(&n.r)[c];
				for(int vz=0;vz<Zplaces;++vz)
				{
					for(int vy=0;vy<Yplaces;++vy)
					{
						for(int vx=0;vx<Xplaces;++vx)
						{
							auto &V000=ndr[Xplaces*(Yplaces*vz+vy)+vx];
							V000=(Xsum[c]+=V000)*step;
						}
						Xsum[c]=0;
					}
				}
			}//*/
		}
		void integrate_y						(Term &n)
		{
			int nComponents=n_components_in_current_expr();
			std::vector<double> Ysum[4];
			for(int c=0;c<nComponents;++c)
				Ysum[c].resize(Xplaces);
			double step=DX/Xplaces;
			for(int c=0;c<nComponents;++c)
			{
				auto &ndr=(&n.r)[c];
				for(int vz=0;vz<Zplaces;++vz)
				{
					for(int vy=0;vy<Yplaces;++vy)
					{
						for(int vx=0;vx<Xplaces;++vx)
						{
							auto &V000=ndr[Xplaces*(Yplaces*vz+vy)+vx];
							V000=(Ysum[c][vx]+=V000)*step;
						}
					}
					std::fill(Ysum[c].begin(), Ysum[c].end(), 0);
				}
			}
		}
		void integrate_z						(Term &n)
		{
			int nComponents=n_components_in_current_expr();
			std::vector<double> Zsum[4];
			int ZsumSize=Xplaces*Yplaces;
			for(int c=0;c<nComponents;++c)
				Zsum[c].resize(ZsumSize);
			double step=DX/Xplaces;
			for(int c=0;c<nComponents;++c)
			{
				auto &ndr=(&n.r)[c];
				for(int vz=0;vz<Zplaces;++vz)
				{
					for(int vy=0;vy<Yplaces;++vy)
					{
						for(int vx=0;vx<Xplaces;++vx)
						{
							auto &V000=ndr[Xplaces*(Yplaces*vz+vy)+vx];
							V000=(Zsum[c][Xplaces*vy+vx]+=V000)*step;
						}
					}
				}
			}
		}
		void integrate_xy						(Term &n)
		{
			int nComponents=n_components_in_current_expr();
			std::vector<double> Ysum[4];
			double YXsum[4]={0};
			int ZsumSize=Xplaces*Yplaces;
			for(int c=0;c<nComponents;++c)
				Ysum[c].resize(Xplaces);
			double step=DX/Xplaces;
			for(int c=0;c<nComponents;++c)
			{
				auto &ndr=(&n.r)[c];
				for(int vz=0;vz<Zplaces;++vz)
				{
					for(int vy=0;vy<Yplaces;++vy)
					{
						for(int vx=0;vx<Xplaces;++vx)
						{
							auto &V000=ndr[Xplaces*(Yplaces*vz+vy)+vx];
							V000=(YXsum[c]+=Ysum[c][vx]+=V000)*step;
						}
						for(int c=0;c<nComponents;++c)
							YXsum[c]=0;
					}
					std::fill(Ysum[c].begin(), Ysum[c].end(), 0);
				}
			}
		}
		void integrate_yz						(Term &n)
		{
			int nComponents=n_components_in_current_expr();
			std::vector<double> Zsum[4], ZYsum[4];
			int ZsumSize=Xplaces*Yplaces;
			for(int c=0;c<nComponents;++c)
			{
				auto &ndr=(&n.r)[c];
				Zsum[c].resize(ZsumSize), ZYsum[c].resize(Xplaces);
				double step=DX/Xplaces;
				for(int vz=0;vz<Zplaces;++vz)
				{
					for(int vy=0;vy<Yplaces;++vy)
					{
						for(int vx=0;vx<Xplaces;++vx)
						{
							auto &V000=ndr[Xplaces*(Yplaces*vz+vy)+vx];
							V000=(ZYsum[c][vx]+=Zsum[c][Xplaces*vy+vx]+=V000)*step;
						}
					}
					std::fill(ZYsum[c].begin(), ZYsum[c].end(), 0);
				}
			}
		}
		void integrate_xz						(Term &n)
		{
			int nComponents=n_components_in_current_expr();
			std::vector<double> Zsum[4];
			double ZXsum[4]={0};
			int ZsumSize=Xplaces*Yplaces;
			for(int c=0;c<nComponents;++c)
			{
				auto &ndr=(&n.r)[c];
				Zsum[c].resize(ZsumSize);
				double step=DX/Xplaces;
				for(int vz=0;vz<Zplaces;++vz)
				{
					for(int vy=0;vy<Yplaces;++vy)
					{
						for(int vx=0;vx<Xplaces;++vx)
						{
							auto &V000=ndr[Xplaces*(Yplaces*vz+vy)+vx];
							V000=(ZXsum[c]+=Zsum[c][Xplaces*vy+vx]+=V000)*step;
						}
						for(int c=0;c<nComponents;++c)
							ZXsum[c]=0;
					}
				}
			}
		}
	//	fftw_complex *fft_in, *fft_out;
	//	fftw_plan fft_p, ifft_p;
	//	unsigned fft_N0, fft_N1, fft_N2;
	//	double fft_sqrt_N;
		void discreteFourrierTransform			(Term &n)
		{
//			int ndrSize=Xplaces*Yplaces*Zplaces;
//			if(fft_N0!=Zplaces||fft_N1!=Yplaces||fft_N2!=Xplaces)
//			{
//				if(fft_N0||fft_N1||fft_N2)
//				{
//					fftw_destroy_plan(fft_p), fftw_destroy_plan(ifft_p);
//					fftw_free(fft_in), fftw_free(fft_out);
//				}
//				fft_N0=Zplaces, fft_N1=Yplaces, fft_N2=Xplaces;
//				fft_in=(fftw_complex*)fftw_malloc(ndrSize*sizeof(fftw_complex)), fft_out=(fftw_complex*)fftw_malloc(ndrSize*sizeof(fftw_complex));
//				fft_sqrt_N=std::sqrt((double)ndrSize);
//				fft_p=fftw_plan_dft_3d(fft_N0, fft_N1, fft_N2, fft_in, fft_out, FFTW_FORWARD, FFTW_ESTIMATE);
//				ifft_p=fftw_plan_dft_3d(fft_N0, fft_N1, fft_N2, fft_in, fft_out, FFTW_BACKWARD, FFTW_ESTIMATE);
//			}
//			auto ndr_r=(double*)n.r.p, ndr_i=(double*)n.i.p;
//			for(int k=0;k<ndrSize;++k)
//			{
//				unsigned x=k%Xplaces, y=k%Zplaces/Xplaces, z=k/Xplaces/Zplaces;
//				int sign=1-((x%2^y%2^z%2)<<1);
//				fft_in[k][0]=ndr_r[k]*sign, fft_in[k][1]=ndr_i[k]*sign;
//			}
//			fftw_execute(fft_p);
//			for(int k=0;k<ndrSize;++k)
//				ndr_r[k]=fft_out[k][0]/fft_sqrt_N, ndr_i[k]=fft_out[k][1]/fft_sqrt_N;
		}
		void inverseDiscreteFourrierTransform	(Term &n)
		{
//			int ndrSize=Xplaces*Yplaces;
//			if(fft_N0!=Zplaces||fft_N1!=Yplaces||fft_N2!=Xplaces)
//			{
//				if(fft_N0||fft_N1||fft_N2)
//				{
//					fftw_destroy_plan(fft_p), fftw_destroy_plan(ifft_p);
//					fftw_free(fft_in), fftw_free(fft_out);
//				}
//				fft_N0=Zplaces, fft_N1=Yplaces, fft_N2=Xplaces;
//				fft_in=(fftw_complex*)fftw_malloc(ndrSize*sizeof(fftw_complex)), fft_out=(fftw_complex*)fftw_malloc(ndrSize*sizeof(fftw_complex));
//				fft_sqrt_N=std::sqrt((double)ndrSize);
//				fft_p=fftw_plan_dft_3d(fft_N0, fft_N1, fft_N2, fft_in, fft_out, FFTW_FORWARD, FFTW_ESTIMATE);
//				ifft_p=fftw_plan_dft_3d(fft_N0, fft_N1, fft_N2, fft_in, fft_out, FFTW_BACKWARD, FFTW_ESTIMATE);
//			}
//			auto ndr_r=(double*)n.r.p, ndr_i=(double*)n.i.p;
//			for(int k=0;k<ndrSize;++k)
//				fft_in[k][0]=ndr_r[k], fft_in[k][1]=ndr_i[k];
//			fftw_execute(ifft_p);
//			for(int k=0;k<ndrSize;++k)
//			{
//				unsigned x=k%Xplaces, y=k%Zplaces/Xplaces, z=k/Xplaces/Zplaces;
//				double gain=(1-((x%2^y%2^z%2)<<1))/fft_sqrt_N;
//				ndr_r[k]=fft_out[k][0]*gain, ndr_i[k]=fft_out[k][1]*gain;
//			}
		}
		void lowPassFilter_xyz					(Term &n)
		{
			const double t=1./27;
			const double kernel[]=
			{	//y=-1		y=0			y=1
				t, t, t,	t, t, t,	t, t, t,//z=-1
				t, t, t,	t, t, t,	t, t, t,//z=0
				t, t, t,	t, t, t,	t, t, t //z=1
			};
			apply_kernel(kernel, Xplaces, Yplaces, Zplaces, ndrSize);
		}
		void lowPassFilter_x					(Term &n)
		{
			const double t=1./3;
			const double kernel[]=
			{	//y=-1				y=0					y=1
				0, 0, 0,	0, 0, 0,	0, 0, 0,//z=-1
				0, 0, 0,	t, t, t,	0, 0, 0,//z=0
				0, 0, 0,	0, 0, 0,	0, 0, 0 //z=1
			};
			apply_kernel(kernel, Xplaces, Yplaces, Zplaces, ndrSize);
		}
		void lowPassFilter_y					(Term &n)
		{
			const double t=1./3;
			const double kernel[]=
			{	//y=-1		y=0			y=1
				0, 0, 0,	0, 0, 0,	0, 0, 0,//z=-1
				0, t, 0,	0, t, 0,	0, t, 0,//z=0
				0, 0, 0,	0, 0, 0,	0, 0, 0 //z=1
			};
			apply_kernel(kernel, Xplaces, Yplaces, Zplaces, ndrSize);
		}
		void lowPassFilter_z					(Term &n)
		{
			const double t=1./3;
			const double kernel[]=
			{	//y=-1		y=0			y=1
				0, 0, 0,	0, t, 0,	0, 0, 0,//z=-1
				0, 0, 0,	0, t, 0,	0, 0, 0,//z=0
				0, 0, 0,	0, t, 0,	0, 0, 0 //z=1
			};
			apply_kernel(kernel, Xplaces, Yplaces, Zplaces, ndrSize);
		}
		void lowPassFilter_xy					(Term &n)
		{
			const double t=1./5;
			const double kernel[]=
			{	//y=-1		y=0			y=1
				0, 0, 0,	0, 0, 0,	0, 0, 0,//z=-1
				0, t, 0,	t, t, t,	0, t, 0,//z=0
				0, 0, 0,	0, 0, 0,	0, 0, 0 //z=1
			};
			apply_kernel(kernel, Xplaces, Yplaces, Zplaces, ndrSize);
		}
		void lowPassFilter_yz					(Term &n)
		{
			const double t=1./5;
			const double kernel[]=
			{	//y=-1		y=0			y=1
				0, 0, 0,	0, t, 0,	0, 0, 0,//z=-1
				0, t, 0,	0, t, 0,	0, t, 0,//z=0
				0, 0, 0,	0, t, 0,	0, 0, 0 //z=1
			};
			apply_kernel(kernel, Xplaces, Yplaces, Zplaces, ndrSize);
		}
		void lowPassFilter_xz					(Term &n)
		{
			const double t=1./5;
			const double kernel[]=
			{	//y=-1		y=0			y=1
				0, 0, 0,	0, t, 0,	0, 0, 0,//z=-1
				0, 0, 0,	t, t, t,	0, 0, 0,//z=0
				0, 0, 0,	0, t, 0,	0, 0, 0 //z=1
			};
			apply_kernel(kernel, Xplaces, Yplaces, Zplaces, ndrSize);
		}
		void highPassFilter_xyz					(Term &n)
		{
			const double t=-1./6;
			const double kernel[]=
			{	//y=-1		y=0				y=1
				0, 0, 0,	0, t,    0,		0, 0, 0,//z=-1
				0, t, 0,	t, -6*t, t,		0, t, 0,//z=0
				0, 0, 0,	0, t,    0,		0, 0, 0 //z=1
			};
			apply_kernel(kernel, Xplaces, Yplaces, Zplaces, ndrSize);
		}
		void highPassFilter_x					(Term &n)
		{
			const double t=-1./2;
			const double kernel[]=
			{	//y=-1		y=0				y=1
				0, 0, 0,	0, 0,    0,		0, 0, 0,//z=-1
				0, 0, 0,	t, -2*t, t,		0, 0, 0,//z=0
				0, 0, 0,	0, 0,    0,		0, 0, 0 //z=1
			};
			apply_kernel(kernel, Xplaces, Yplaces, Zplaces, ndrSize);
		}
		void highPassFilter_y					(Term &n)
		{
			const double t=-1./2;
			const double kernel[]=
			{	//y=-1		y=0				y=1
				0, 0, 0,	0, 0,    0,		0, 0, 0,//z=-1
				0, t, 0,	0, -2*t, 0,		0, t, 0,//z=0
				0, 0, 0,	0, 0,    0,		0, 0, 0 //z=1
			};
			apply_kernel(kernel, Xplaces, Yplaces, Zplaces, ndrSize);
		}
		void highPassFilter_z					(Term &n)
		{
			const double t=-1./2;
			const double kernel[]=
			{	//y=-1		y=0				y=1
				0, 0, 0,	0, t,    0,		0, 0, 0,//z=-1
				0, 0, 0,	0, -2*t, 0,		0, 0, 0,//z=0
				0, 0, 0,	0, t,    0,		0, 0, 0 //z=1
			};
			apply_kernel(kernel, Xplaces, Yplaces, Zplaces, ndrSize);
		}
		void highPassFilter_xy					(Term &n)
		{
			const double t=-1./4;
			const double kernel[]=
			{	//y=-1		y=0				y=1
				0, 0, 0,	0, 0,    0,		0, 0, 0,//z=-1
				0, t, 0,	t, -4*t, t,		0, t, 0,//z=0
				0, 0, 0,	0, 0,    0,		0, 0, 0 //z=1
			};
			apply_kernel(kernel, Xplaces, Yplaces, Zplaces, ndrSize);
		}
		void highPassFilter_yz					(Term &n)
		{
			const double t=-1./4;
			const double kernel[]=
			{	//y=-1		y=0				y=1
				0, 0, 0,	0, t,    0,		0, 0, 0,//z=-1
				0, t, 0,	0, -4*t, 0,		0, t, 0,//z=0
				0, 0, 0,	0, t,    0,		0, 0, 0 //z=1
			};
			apply_kernel(kernel, Xplaces, Yplaces, Zplaces, ndrSize);
		}
		void highPassFilter_xz					(Term &n)
		{
			const double t=-1./4;
			const double kernel[]=
			{	//y=-1		y=0				y=1
				0, 0, 0,	0, t,    0,		0, 0, 0,//z=-1
				0, 0, 0,	t, -4*t, t,		0, 0, 0,//z=0
				0, 0, 0,	0, t,    0,		0, 0, 0 //z=1
			};
			apply_kernel(kernel, Xplaces, Yplaces, Zplaces, ndrSize);
		}

		void setDimensions(int x, int y, int w, int h)
		{
			bpx=x, bpy=y, bw=w, bh=h, X0=bpx+bw/2, Y0=bpy+bh/2;
			_3d.setDimensions(x, y, w, h);
			double old_Xstep=Xstep;
			function1();
			if(!toSolve&&contourOn&&old_Xstep!=Xstep)
			{
				Rcontours.clear(), Icontours.clear(), Jcontours.clear(), Kcontours.clear();
				Rlines.clear(), Ilines.clear(), Jlines.clear(), Klines.clear();
				doContour(cursorEx, DX/Xplaces, -DX/2, +DX/2, Xstep, 2);
			}
			ready=true;
		}
		void shiftNDR(double &DshiftPoint, double Dsample, double &DsamplePos, double &VD, int &Doffset, double ammount)
		{
			double newDsamplePos=std::floor((DshiftPoint+=ammount)/Dsample);
			if(newDsamplePos!=DsamplePos)
			{
				toSolve=true;
				if(shiftOnly)
					shiftOnly=1, Doffset+=int(newDsamplePos-DsamplePos);
				DsamplePos=newDsamplePos, VD=newDsamplePos*Dsample;
			}
			DsamplePos=newDsamplePos;
		}
		void shiftNDRupdate(double DshiftPoint, double Dsample, double &DsamplePos, double &VD)
		{
			double newDsamplePos=std::floor(DshiftPoint/Dsample);
			if(newDsamplePos!=DsamplePos)
				DsamplePos=newDsamplePos, VD=newDsamplePos*Dsample;
		}
		void messageTimer()
		{
			a_draw();
//			if(!time_variance&&!kp)
//				KillTimer(ghWnd, 0), timer=false;
		}
		int inputTouchDown(int idx)
		{
			auto &ti=touchInfo[idx];
			auto &pos=ti.pos;
			int X0=w>>1, Y0=h>>1;
			if(pos.y>Y0)
			{
				if(pos.x<X0)
					return BOTTOM_LEFT_MOVE;
				return BOTTOM_RIGHT_TURN;
			}
			if(reset_button.click(pos))
			{
				_3d.cam.reset();
				DX=20, AR_Y=1, AR_Z=1, function1();
				VX=VY=VZ=0;
				_3d.teleport_degrees(4, 4, 4, 225, 324.7356103172454, 1);
				return BUTTON_RESET;
			}
			return TOUCH_MISS;
		}
		void inputTouchUp(int idx)
		{
		}
		void inputTouchMove(int idx)
		{
			auto &ti=touchInfo[idx];
			if(ti.region==BOTTOM_RIGHT_TURN)
				_3d.cam.turnBy(ti.delta.x*5/w, ti.delta.y*5/w);
		}

		void draw()
		{
			int X0=w>>1, Y0=h>>1;
			for(int k=0, kEnd=touchInfo.size();k<kEnd;++k)//move camera
			{
				auto &ti=touchInfo[k];
				if(ti.region==BOTTOM_LEFT_MOVE)
				{
					int X00=w>>2, Y00=h*3>>2;
					float dx=float(ti.pos.x-X00)/w, dy=-float(ti.pos.y-Y00)/w;

					_3d.cam.move(dx, dy);
					break;
				}
			}
			double DY=DX/AR_Y, DZ=DX/AR_Z;
			auto &ex=expr[cursorEx];
			{
				bool changed=false;
				if(toSolve)
				{
					if(!operations.size()&&shiftOnly==1&&abs(Xoffset)<Xplaces&&abs(Yoffset)<Yplaces&&abs(Zoffset)<Zplaces)
					{
						solver.partial_bounds(VX, DX, VY/AR_Y, DY, VZ/AR_Z, DZ, Xoffset, Yoffset, Zoffset);
						solver.synchronize();
						(solver.*(ex.nITD?&Solve_3D::full:solver.partial))(ex);
					}
					else
					{
						auto old_time_variance=time_variance;
						time_variance=false;
						labels.clear();
					//	Xplaces=Yplaces=Zplaces=
					//		50//2//4//8//10//50
					//		;
						solver.full_resize(VX, DX, VY/AR_Y, DY, VZ/AR_Z, DZ, Xplaces, Yplaces, Zplaces);
						XsamplePos=std::floor(XshiftPoint/solver.Xsample);
						YsamplePos=std::floor(YshiftPoint/solver.Ysample);
						ZsamplePos=std::floor(ZshiftPoint/solver.Zsample);

					//	KXplaces=Xplaces/100?Xplaces/100:1, KYplaces=Yplaces/100?Yplaces/100:1, KZplaces=Zplaces/100?Zplaces/100:1;
						time_variance|=ex.nITD;
						labels.fill(cursorEx);
						if(!paused)
							solver.synchronize();
						solver.full(ex);
						doOperations(ex.n[ex.resultTerm]);
//						if(time_variance)
//						{
//							if(!paused)
//								SetTimer(ghWnd, 0, 10, 0);
//						}
//						else if(old_time_variance&&!timer)
//							KillTimer(ghWnd, 0);
					}
					toSolve=false, shiftOnly=2, Xoffset=Yoffset=Zoffset=0;
					changed=true;
				}
				else if(time_variance)
				{
					if(!paused)
						solver.synchronize();
					solver.full(ex);
					doOperations(ex.n[ex.resultTerm]);
					changed=true;
				}
				if(changed)
				{
					solver.updateRGB(ex);
					if(contourOn)
					{
						Rcontours.clear(), Icontours.clear(), Jcontours.clear(), Kcontours.clear();
						Rlines.clear(), Ilines.clear(), Jlines.clear(), Klines.clear();
						doContour(cursorEx, DX/Xplaces, -DX/2, +DX/2, Xstep, 2);
					}
				}
			}

		//	_3d.newFrame();
			GL2_3D::begin();
			if(!clearScreen)
				_3dMode_DrawGridNAxes(_3d, VX, VY, VZ, DX, Xstep, Ystep, Zstep, AR_Y, AR_Z);
			if(!contourOnly)//draw the points
			{
				double Xstart=VX-DX/2, Xr=DX/Xplaces, Ystart=VY-DX/2, Ysample=DX/Yplaces, Zstart=VZ-DX/2, Zr=DX/Zplaces;
				unsigned v=0;
				switch(ex.resultMathSet)
				{
				case 'R':case 'c':
					for(int k=0;k<Zplaces;++k)
						for(int k2=0;k2<Yplaces;++k2)
							for(int k3=0;k3<Xplaces;++k3, ++v)
								_3d.point(Xstart+Xr*k3, Ystart+Ysample*k2, Zstart+Zr*k, solver.ndr_rgb[v]);
					break;
				case 'h':
					for(int k=0;k<Zplaces;++k)
						for(int k2=0;k2<Yplaces;++k2)
							for(int k3=0;k3<Xplaces;++k3, ++v)
								_3d.point(Xstart+Xr*k3, Ystart+Ysample*k2, Zstart+Zr*k, solver.ndr_rgb[v], solver.ndr_rgb_i[v], solver.ndr_rgb_j[v], solver.ndr_rgb_k[v]);
					break;
				}
			}
			if(contourOn)//draw the contour
			{
				switch(ex.resultMathSet)
				{
				case 'R':
					draw_contour(Rcontours, Rlines, rColor);
					break;
				case 'c':
					draw_contour(Rcontours, Rlines, rColor);
					draw_contour(Icontours, Ilines, iColor);
					break;
				case 'h':
					draw_contour(Rcontours, Rlines, rColor);
					draw_contour(Icontours, Ilines, iColor);
					draw_contour(Icontours, Ilines, jColor);
					draw_contour(Icontours, Ilines, kColor);
					break;
				}
			}
			GL2_3D::end();
			GL2_3D::draw(_3d.cam);
			if(!clearScreen)
			{
				int bkMode=setBkMode(TRANSPARENT);
				for(double X=floor((VX-DX/2)/Xstep)*Xstep+Xstep, Xend=ceil((VX+DX/2)/Xstep)*Xstep;X<Xend;X+=Xstep)
					if(abs(X)>Xstep/2)
						_3d.label(X, VY, VZ, "%g", X);
				if(AR_Y==1)
				{
					for(double Y=floor((VY-DX/2)/Ystep)*Ystep+Ystep, Yend=ceil((VY+DX/2)/Ystep)*Ystep;Y<Yend;Y+=Ystep)
						if(Y<-Ystep/2||Y>Ystep/2)
							_3d.label(VX, Y, VZ, "%g", Y);
				}
				else
				{
					double aYstep=AR_Y*Ystep, Ycore=floor((VY-DX/2)/aYstep);
					for(double Y=Ycore*aYstep+aYstep, Yend=ceil((VY+DX/2)/aYstep)*aYstep, aY=Ycore*Ystep+Ystep;Y<Yend;Y+=aYstep, aY+=Ystep)
						if(Y<-aYstep/2||Y>aYstep/2)
							_3d.label(VX, Y, VZ, "%g", aY);
				}
				if(AR_Z==1)
				{
					for(double Z=floor((VZ-DX/2)/Zstep)*Zstep+Zstep, Zend=ceil((VZ+DX/2)/Zstep)*Zstep;Z<Zend;Z+=Zstep)
						if(Z<-Zstep/2||Z>Zstep/2)
							_3d.label(VX, VY, Z, "%g", Z);
				}
				else
				{
					double aZstep=AR_Z*Zstep, Zcore=floor((VZ-DX/2)/aZstep);
					for(double Z=Zcore*aZstep+aZstep, Zend=ceil((VZ+DX/2)/aZstep)*aZstep, aZ=Zcore*Zstep+Zstep;Z<Zend;Z+=aZstep, aZ+=Zstep)
						if(Z<-aZstep/2||Z>aZstep/2)
							_3d.label(VX, VY, Z, "%g", aZ);
				}
				_3d.arrowLabels(0, 0, 0, 1, 0, 0, labels.Xlabels);
				_3d.arrowLabels(0, 0, 0, 0, 1, 0, labels.Ylabels);
				_3d.arrowLabels(0, 0, 0, 0, 0, 1, labels.Zlabels);
				{
					int Ys=0;
					for(int kl=0, klEnd=labels.Clabels.size();kl<klEnd;++kl)
					{
						auto &label=labels.Clabels[kl];
						print(w-const_label_offset_X, Ys, "%s=%g", label.label.c_str(), label.isTime?solver.T:label.value);
					//	int linelen=sprintf_s(g_buf, "%s=%g", label.label.c_str(), label.isTime?solver.T:label.value);
					//	TextOutA(ghMemDC, w-const_label_offset_X, Ys, g_buf, linelen);
						Ys+=fontH;
					}
				}
				{
					int k=0, Y=h-operations.size()*16;
					for(auto it=operations.begin();it!=operations.end();++it)
					{
						auto &operation=*it;
						char const *a=0;
						switch(operation)
						{
						case  1:a="%d: Differentiate XYZ";	break;
						case  2:a="%d: Differentiate X";	break;
						case  3:a="%d: Differentiate Y";	break;
						case  4:a="%d: Differentiate Z";	break;
						case  5:a="%d: Differentiate XY";	break;
						case  6:a="%d: Differentiate YZ";	break;
						case  7:a="%d: Differentiate XZ";	break;
						case  8:a="%d: Integrate XYZ";		break;
						case  9:a="%d: Integrate X";		break;
						case 10:a="%d: Integrate Y";		break;
						case 11:a="%d: Integrate Z";		break;
						case 12:a="%d: Integrate XY";		break;
						case 13:a="%d: Integrate YZ";		break;
						case 14:a="%d: Integrate XZ";		break;
						case 15:a="%d: DFT";				break;
						case 16:a="%d: IDFT";				break;
						case 17:a="%d: LPF";				break;
						case 18:a="%d: LPF X";				break;
						case 19:a="%d: LPF Y";				break;
						case 20:a="%d: LPF Z";				break;
						case 21:a="%d: LPF XY";				break;
						case 22:a="%d: LPF YZ";				break;
						case 23:a="%d: LPF XZ";				break;
						case 24:a="%d: HPF";				break;
						case 25:a="%d: HPF X";				break;
						case 26:a="%d: HPF Y";				break;
						case 27:a="%d: HPF Z";				break;
						case 28:a="%d: HPF XY";				break;
						case 29:a="%d: HPF YZ";				break;
						case 30:a="%d: HPF XZ";				break;
						}
						_3d.textIn2D(w-const_label_offset_X, Y, OPAQUE, a, k);
						++k, Y+=16;
					}
				}
				setBkMode(bkMode);
				_3d.text_show();
			}
			else
				_3d.text_show();
			//	_3d.text_dump();
//			if(kb_VK_F6_msg||kb[VK_F6])
//			{
//				int kb_mode=SetBkMode(ghMemDC, OPAQUE);
//				kb_VK_F6_msg=false;
//				if(!_3d_stretch_move_cam&!_3d_shift_move_cam&!_3d_zoom_move_cam)
//					GUIPrint(ghMemDC, 0, h-18*3, "move cam: [-] scale, [-] shift, [-] zoom");
//				else if(!_3d_shift_move_cam&!_3d_zoom_move_cam)
//					GUIPrint(ghMemDC, 0, h-18*3, "move cam: [v] scale, [-] shift, [-] zoom");
//				else if(!_3d_zoom_move_cam)
//					GUIPrint(ghMemDC, 0, h-18*3, "move cam: [v] scale, [v] shift, [-] zoom");
//				else
//					GUIPrint(ghMemDC, 0, h-18*3, "move cam: [v] scale, [v] shift, [v] zoom");
//				SetBkMode(ghMemDC, kb_mode);
//			}
		}
		void i_draw();
		void a_draw();
	} c3d;
	const int Color_3D::modes[]={9}, Color_3D::nmodes=sizeof(modes)>>2;
	void			Color_3D::i_draw()
	{
		if(modes::ready)
		{
			draw();
//			if(showLastModeOnIdle)
//				std::copy(rgb, rgb+w*h, modeRGB);
		}
		else
		{
		//	std::copy(modeRGB, modeRGB+w*h, rgb);
		}
		if(showBenchmark)
			Performance(0, h>>1);//
	}
	void			Color_3D::a_draw()
	{
	//	Rectangle(ghMemDC, bpx-1, bpy-1, bw+1, bh+1);

		draw();
		
		if(contextHelp)
		{
			const char *help[]=
			{
				"W/A/S/D/T/G: move",
				"arrows: turn",
				"+/-/enter/backspace/wheel: change FOV",
				"X/Y/Z arrows/mouse move: shift x/y/z",
				"X/Y/Z +/-/enter/backspace/wheel: scale x/y/z",
				"alt +/-/enter/backspace/wheel: zoom",
				"shift +/-/enter/backspace/wheel: change resolution",
				"E: reset scale",
				"R: reset scale & view",
				"C: toggle clear screen",
				"1: differentiate xyz",
				"X/Y/Z 1: differentiate x/y/z",
				"2: integrate xyz",
				"X/Y/Z 2: integrate xyz",
				"3: DFT",
				"4: Inverse DFT",
				"5: LPF",
				"X/Y/Z 5: LPF x/y/z",
				"6: HPF",
				"X/Y/Z 6: HPF x/y/z",
				"0: reset operations",
				"`: contour",
				"Esc: back to text editor"
			};
			print_contextHelp(help, sizeof(help)>>2, 330);
		}
		if(showBenchmark&&!clearScreen)
			Performance(0, h);//
	//	BitBlt(ghDC, 0, 0, w, h, ghMemDC, 0, 0, SRCCOPY);
	}

	void (*paint)();
	void paint_0()
	{
		modes::n0d	.ready=false;
		modes::t1d	.ready=false, modes::t1d.toSolve=true, modes::t1d.shiftOnly=0;
		modes::t1d_c.ready=false;
		modes::t1d_h.ready=false, modes::t1d_h.toSolve=true, modes::t1d_h.shiftOnly=0;
		modes::t2d	.ready=false;
		modes::c2d	.ready=false, modes::c2d.toSolve=true, modes::c2d.shiftOnly=0;
		modes::l2d	.ready=false, modes::l2d.toSolve=true, modes::l2d.shiftOnly=0;
		modes::t2d_h.ready=false;
		modes::c3d	.ready=false;
		modes::ti1d	.ready=false;
		modes::ti2d	.ready=false;
	}
	void paint_n0d()
	{
		modes::n0d.setDimensions(0, 0, w, h);
		
		modes::t1d	.ready=false, modes::t1d.toSolve=true, modes::t1d.shiftOnly=0;
		modes::t1d_c.ready=false;
		modes::t1d_h.ready=false, modes::t1d_h.toSolve=true, modes::t1d_h.shiftOnly=0;
		modes::t2d	.ready=false;
		modes::c2d	.ready=false, modes::c2d.toSolve=true, modes::c2d.shiftOnly=0;
		modes::l2d	.ready=false, modes::l2d.toSolve=true, modes::l2d.shiftOnly=0;
		modes::t2d_h.ready=false;
		modes::c3d	.ready=false;
		modes::ti1d	.ready=false;
		modes::ti2d	.ready=false;
	}
	void paint_t1d()
	{
		modes::n0d.ready=false;

		modes::t1d.setDimensions(0, 0, w, h);
		
		modes::t1d_c.ready=false;
		modes::t1d_h.ready=false, modes::t1d_h.toSolve=true, modes::t1d_h.shiftOnly=0;
		modes::t2d	.ready=false;
		modes::c2d	.ready=false, modes::c2d.toSolve=true, modes::c2d.shiftOnly=0;
		modes::l2d	.ready=false, modes::l2d.toSolve=true, modes::l2d.shiftOnly=0;
		modes::t2d_h.ready=false;
		modes::c3d	.ready=false;
		modes::ti1d	.ready=false;
		modes::ti2d	.ready=false;
	}
	void paint_t1d_c()
	{
		modes::n0d.ready=false;
		modes::t1d.ready=false, modes::t1d.toSolve=true, modes::t1d.shiftOnly=0;

		modes::t1d_c.setDimensions(0, 0, w, h);
		
		modes::t1d_h.ready=false, modes::t1d_h.toSolve=true, modes::t1d_h.shiftOnly=0;
		modes::t2d	.ready=false;
		modes::c2d	.ready=false, modes::c2d.toSolve=true, modes::c2d.shiftOnly=0;
		modes::l2d	.ready=false, modes::l2d.toSolve=true, modes::l2d.shiftOnly=0;
		modes::t2d_h.ready=false;
		modes::c3d	.ready=false;
		modes::ti1d	.ready=false;
		modes::ti2d	.ready=false;
	}
	void paint_t1d_h()
	{
		modes::n0d	.ready=false;
		modes::t1d	.ready=false, modes::t1d.toSolve=true, modes::t1d.shiftOnly=0;
		modes::t1d_c.ready=false;

		modes::t1d_h.setDimensions(0, 0, w, h);
		
		modes::t2d	.ready=false;
		modes::c2d	.ready=false, modes::c2d.toSolve=true, modes::c2d.shiftOnly=0;
		modes::l2d	.ready=false, modes::l2d.toSolve=true, modes::l2d.shiftOnly=0;
		modes::t2d_h.ready=false;
		modes::c3d	.ready=false;
		modes::ti1d	.ready=false;
		modes::ti2d	.ready=false;
	}
	void paint_t2d()
	{
		modes::n0d	.ready=false;
		modes::t1d	.ready=false, modes::t1d.toSolve=true, modes::t1d.shiftOnly=0;
		modes::t1d_c.ready=false;
		modes::t1d_h.ready=false, modes::t1d_h.toSolve=true, modes::t1d_h.shiftOnly=0;

		modes::t2d.setDimensions(0, 0, w, h);
		
		modes::c2d	.ready=false, modes::c2d.toSolve=true, modes::c2d.shiftOnly=0;
		modes::l2d	.ready=false, modes::l2d.toSolve=true, modes::l2d.shiftOnly=0;
		modes::t2d_h.ready=false;
		modes::c3d	.ready=false;
		modes::ti1d	.ready=false;
		modes::ti2d	.ready=false;
	}
	void paint_c2d()
	{
		modes::n0d	.ready=false;
		modes::t1d	.ready=false, modes::t1d.toSolve=true, modes::t1d.shiftOnly=0;
		modes::t1d_c.ready=false;
		modes::t1d_h.ready=false, modes::t1d_h.toSolve=true, modes::t1d_h.shiftOnly=0;
		modes::t2d	.ready=false;

		modes::c2d.setDimensions(0, 0, w, h);
		
		modes::l2d	.ready=false, modes::l2d.toSolve=true, modes::l2d.shiftOnly=0;
		modes::t2d_h.ready=false;
		modes::c3d	.ready=false;
		modes::ti1d	.ready=false;
		modes::ti2d	.ready=false;
	}
	void paint_l2d()
	{
		modes::n0d	.ready=false;
		modes::t1d	.ready=false, modes::t1d.toSolve=true, modes::t1d.shiftOnly=0;
		modes::t1d_c.ready=false;
		modes::t1d_h.ready=false, modes::t1d_h.toSolve=true, modes::t1d_h.shiftOnly=0;
		modes::t2d	.ready=false;
		modes::c2d	.ready=false, modes::c2d.toSolve=true, modes::c2d.shiftOnly=0;

		modes::l2d.setDimensions(0, 0, w, h);

		modes::t2d_h.ready=false;
		modes::c3d	.ready=false;
		modes::ti1d	.ready=false;
		modes::ti2d	.ready=false;
	}
	void paint_t2d_h()
	{
		modes::n0d	.ready=false;
		modes::t1d	.ready=false, modes::t1d.toSolve=true, modes::t1d.shiftOnly=0;
		modes::t1d_c.ready=false;
		modes::t1d_h.ready=false, modes::t1d_h.toSolve=true, modes::t1d_h.shiftOnly=0;
		modes::t2d	.ready=false;
		modes::c2d	.ready=false, modes::c2d.toSolve=true, modes::c2d.shiftOnly=0;
		modes::l2d	.ready=false, modes::l2d.toSolve=true, modes::l2d.shiftOnly=0;

		modes::t2d_h.setDimensions(0, 0, w, h);

		modes::c3d	.ready=false;
		modes::ti1d	.ready=false;
		modes::ti2d	.ready=false;
	}
	void paint_c3d()
	{
		modes::n0d	.ready=false;
		modes::t1d	.ready=false, modes::t1d.toSolve=true, modes::t1d.shiftOnly=0;
		modes::t1d_c.ready=false;
		modes::t1d_h.ready=false, modes::t1d_h.toSolve=true, modes::t1d_h.shiftOnly=0;
		modes::t2d	.ready=false;
		modes::c2d	.ready=false, modes::c2d.toSolve=true, modes::c2d.shiftOnly=0;
		modes::l2d	.ready=false, modes::l2d.toSolve=true, modes::l2d.shiftOnly=0;
		modes::t2d_h.ready=false;

		modes::c3d.setDimensions(0, 0, w, h);
		
		modes::ti1d.ready=false;
		modes::ti2d.ready=false;
	}
	void paint_ti1d()
	{
		modes::n0d	.ready=false;
		modes::t1d	.ready=false, modes::t1d.toSolve=true, modes::t1d.shiftOnly=0;
		modes::t1d_c.ready=false;
		modes::t1d_h.ready=false, modes::t1d_h.toSolve=true, modes::t1d_h.shiftOnly=0;
		modes::t2d	.ready=false;
		modes::c2d	.ready=false, modes::c2d.toSolve=true, modes::c2d.shiftOnly=0;
		modes::l2d	.ready=false, modes::l2d.toSolve=true, modes::l2d.shiftOnly=0;
		modes::t2d_h.ready=false;
		modes::c3d	.ready=false;

		modes::ti1d.setDimensions(0, 0, w, h);

		modes::ti2d.ready=false;
	}
	void paint_ti2d()
	{
		modes::n0d	.ready=false;
		modes::t1d	.ready=false, modes::t1d.toSolve=true, modes::t1d.shiftOnly=0;
		modes::t1d_c.ready=false;
		modes::t1d_h.ready=false, modes::t1d_h.toSolve=true, modes::t1d_h.shiftOnly=0;
		modes::t2d	.ready=false;
		modes::c2d	.ready=false, modes::c2d.toSolve=true, modes::c2d.shiftOnly=0;
		modes::l2d	.ready=false, modes::l2d.toSolve=true, modes::l2d.shiftOnly=0;
		modes::t2d_h.ready=false;
		modes::c3d	.ready=false;
		modes::ti1d	.ready=false;

		modes::ti2d.setDimensions(0, 0, w, h);
	}
}
void render()
{
//	memset(rgb, 0xFF, w*h*sizeof(int));

//	int inputTextBkMode=OPAQUE;
	if(expr.size()||userFunctionDefinitions.size())//
	{
		static bool toResume=false;

		cursorB=0, cursorEx=0;
		for(int bEnd=bounds.size()-1;cursorB<bEnd&&bounds[cursorB].first<=tb_cursor;++cursorB)
			cursorEx+=bounds[cursorB].second=='e';
//		for(int bEnd=bounds.size()-1;cursorB<bEnd&&bounds[cursorB].first<=itb.cursor;++cursorB)
//			cursorEx+=bounds[cursorB].second=='e';

		if(bounds[cursorB].second&1)//expression or clear
		{
			auto &ex=expr[cursorEx];
			switch(ex.rmode[0])//current mode
			{
			case 0://no expression
				modes::ready=false;
				if(showLastModeOnIdle)
				{
					if(modes::mode&&!modes::mode->paused)
						modes::mode->pause();
				}
				else
					modes::mode=nullptr, modes::paint=modes::paint_0;
				std::fill(modes::nExpr.begin(), modes::nExpr.end(), 0);//crappy code
				for(int ke=0, keEnd=expr.size();ke<keEnd;++ke)
				{
					auto &ex=expr[ke];
					++modes::nExpr[ex.rmode[0]];
				}
				break;
			case 1://numeric 0d
				modes::ready=true;
				modes::mode=&modes::n0d, modes::paint=modes::paint_n0d;
				modes::n0d.toPrint=true;
				break;
			case 2://transverse 1d
				modes::ready=true;
				modes::mode=&modes::t1d, modes::paint=modes::paint_t1d;
				break;
			case 3://transverse 1d complex
				modes::ready=true;
				modes::mode=&modes::t1d_c, modes::paint=modes::paint_t1d_c;
				if(modes::mode!=modes::old_mode)
					modes::t1d.ready=false;
				break;
			case 4://transverse 1d quaternion
				modes::ready=true;
				modes::mode=&modes::t1d_h, modes::paint=modes::paint_t1d_h;
				if(cursorEx!=prevCursorEx||!ex.n[ex.resultTerm].r.size())
					modes::t1d_h.toSolve=true, modes::t1d_h.shiftOnly=0;
				break;
			case 5://transverse 2d
				modes::ready=true;
				modes::mode=&modes::t2d, modes::paint=modes::paint_t2d;
				break;
			case 6://color 2d
				modes::ready=true;
				modes::mode=&modes::c2d, modes::paint=modes::paint_c2d;
				if(cursorEx!=prevCursorEx||!ex.n[ex.resultTerm].r.size())
					modes::c2d.toSolve=true, modes::c2d.shiftOnly=0;
				break;
			case 7://longitudinal 2d
				modes::ready=true;
				modes::mode=&modes::l2d, modes::paint=modes::paint_l2d;
				break;
			case 8://transverse 2d quaternion
				modes::ready=true;
				modes::mode=&modes::t2d_h, modes::paint=modes::paint_t2d_h;
				if(cursorEx!=prevCursorEx||!ex.n[ex.resultTerm].r.size())
					modes::t2d_h.toSolve=true, modes::t2d_h.shiftOnly=0;
				break;
			case 9://color 3d
				modes::ready=true;
				modes::mode=&modes::c3d, modes::paint=modes::paint_c3d;
				if(cursorEx!=prevCursorEx||!ex.n[ex.resultTerm].r.size())
					modes::c3d.toSolve=true, modes::c3d.shiftOnly=0;
				break;
			case 10://transverse implicit 1d
				modes::ready=true;
				modes::mode=&modes::ti1d, modes::paint=modes::paint_ti1d;
				break;
			case 11://transverse implicit 2d
				modes::ready=true;
				modes::mode=&modes::ti2d, modes::paint=modes::paint_ti2d;
				break;
			}
		}
		else//cursor in user function
		{
			modes::ready=false;
			if(showLastModeOnIdle)
			{
				if(modes::mode&&!modes::mode->paused)
					modes::mode->pause();
			}
			else
				modes::mode=nullptr, modes::paint=modes::paint_0;
			std::fill(modes::nExpr.begin(), modes::nExpr.end(), 0);//crappy code
			for(int ke=0, keEnd=expr.size();ke<keEnd;++ke)
			{
				auto &ex=expr[ke];
				++modes::nExpr[ex.rmode[0]];
			}
		}
		if(modes::mode!=modes::old_mode)
		{
			if(modes::old_mode)
			{
				if(modes::mode&&(modes::mode->bw!=modes::old_mode->bw||modes::mode->bh!=modes::old_mode->bh))//up, esc, resize, esc, down CRASH
					modes::mode->setDimensions(0, 0, w, h);//
				modes::old_mode->pause();
			}
			if(modes::mode)
				modes::mode->resume();
			modes::old_mode=modes::mode;//
		}
		if(modes::ready)
		{
			if(modes::mode->paused)
				modes::mode->resume();
			if(!modes::mode->ready)
				modes::mode->setDimensions(0, 0, w, h);
		//	modes::mode->i_draw();
		}
		else if(showLastModeOnIdle&&modes::mode!=nullptr)//
			modes::mode->i_draw();//
		prevCursorEx=cursorEx;
#ifdef BUILD_1_7
		if(bounds[cursorB].second&1&&var_menu)
		{
			auto &ex=expr[cursorEx];
			bool start_anew=!puis.size()||cursorEx!=var_menu_idx;
			if(var_menu_idx!=cursorEx)
				puis.clear(), var_menu_idx=cursorEx;
			if(ex.nx+ex.nZ+ex.nQ)
			{
				int vui_h_2=vui_height>>1;
				for(int kv=0, kp=0, kvEnd=ex.variables.size();kv<kvEnd;++kv, ++kp)
				{
					auto &var=ex.variables[kv];
					int y=h-(vui_h_2+kp*vui_height);
					draw_var_ui(ex, kv, 0, kp, start_anew, y);
					if(var.mathSet>='c')
					{
						++kp, y=h-(vui_h_2+kp*vui_height);
						draw_var_ui(ex, kv, 1, kp, start_anew, y);
						if(var.mathSet=='h')
						{
							++kp, y=h-(vui_h_2+kp*vui_height);
							draw_var_ui(ex, kv, 2, kp, start_anew, y);
							++kp, y=h-(vui_h_2+kp*vui_height);
							draw_var_ui(ex, kv, 3, kp, start_anew, y);
						}
					}
				}
			}
		}
#endif
	}
	if(contextHelp)
	{
//		if(var_menu)
//		{
//			const char *help[]=
//			{
//				"number +/-: increment/decrement Nth parameter",
//				"    (starting from 1)",
//				"number 0: reset Nth parameter",
//				"number enter: enter value for Nth parameter",
//				"number space: select type of Nth parameter",
//				"number S: select scale type for Nth parameter",
//				"F2/right click: close parameter menu"
//			};
//			print_contextHelp(help, sizeof(help)>>2, 315);
//		}
//		else
		{
			const char *help[]=
			{
				"Esc: interactive mode",
				"F1: context help",
				"F2/right click: parameter menu",
				"F7: toggle benchmark",
				"F10: toggle show last mode on idle",
				"F11: fullscreen",
				"Ctrl +/-/wheel: change font size",
				"Ctrl 0: reset font size",
				"Ctrl up/down: scroll",
				"Ctrl C/X: copy/cut text",
				"Ctrl Shift C/X: copy/cut text with results",
				"Ctrl V: paste text"
			};
			print_contextHelp(help, sizeof(help)>>2, 256);
		}
	}
//#ifdef BUILD_1_7
//	BitBlt(ghDC, 0, 0, w, h, ghMemDC, 0, 0, SRCCOPY);
//#endif
}
extern "C" JNIEXPORT jstring JNICALL Java_com_example_grapher2_GL2JNILib_init(JNIEnv *env, jclass obj,  jint w, jint h, jint extra_info)
{//called on start & resume
	++init_counter;
	::w=w, ::h=h, X0=w>>1, Y0=h>>1, landscape=w>h;
	env->GetJavaVM(&jvm);
//	reset_button.set(w>>1, 0, w>>1, h/10, "Reset", 38);
	//ButtonReset::set(w>>1, 0, w>>1, h/10);
	set_font_size(10);
	gl_initiate();
	cl_initiate();
	//printGLString("Version", GL_VERSION);
	//printGLString("Vendor", GL_VENDOR);
	//printGLString("Renderer", GL_RENDERER);
	//printGLString("Extensions", GL_EXTENSIONS);
	//LOGI("setupGraphics(%d, %d)", w, h);
	//if(!Text::initialize())
	//{
	//	LOGE("Could not create text program.");
	//	broken=1;
	//	return env->NewStringUTF(g_buf);
	//}
	//if(!_2D::initialize())
	//{
	//	LOGE("Could not create 2D program.");
	//	broken=2;
	//	return env->NewStringUTF(g_buf);
	//}
	//if(!_3D::initialize())
	//{
	//	LOGE("Could not create 3D solid program.");
	//	broken=3;
	//	return env->NewStringUTF(g_buf);
	//}
	//glViewport(0, 0, w, h);
	//checkGlError("glViewport");
	return env->NewString((const unsigned short*)L"", 0);
}
extern "C" JNIEXPORT void JNICALL Java_com_example_grapher2_GL2JNILib_step(JNIEnv *env, jclass obj, jint cursor)
{
	if(nthreads>0)
		return;
	++nthreads;
	if(tb_cursor!=cursor)
	{
		tb_cursor=cursor;
		render();
	}
	glViewport(0, 0, w, h);		CHECK();// Set the viewport
	//if(inputBoxOn)
	//	glClearColor(half, 1, 1, 1);
	//else
		glClearColor(1, 1, 1, 1);
	CHECK();
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);	CHECK();// Clear the color buffer

	if(modes::mode)
	{
		if(inputBoxOn)
			modes::mode->i_draw();
		else
			modes::mode->a_draw();
	}
	if(inputBoxOn)//apply bluish tint
	{
		int c1=pen_color, c2=brush_color;
		set_color(0x40FF4040);//0x20FF8080
		GL2_2D::draw_rectangle(0, w, 0, h);
		CHECK();
		pen_color=c1, brush_color=c2;
	}
//	cl_step();
	print_if_error();
//	glUseProgram(Text::program);// Use the program object
//	CHECK();
//	if(text.size())
//		tb_draw();
//	else
//		mode_draw();
	--nthreads;
}
void push_buttons(TouchInfo &ti)
{
//	if(reset_button.click(ti.pos.x, ti.pos.y))
//	{
//		_3d.reset_cam();
//	}
}
enum TouchType
{
	ACTION_DOWN, ACTION_UP, ACTION_MOVE, ACTION_CANCEL, ACTION_OUTSIDE, ACTION_POINTER_DOWN, ACTION_POINTER_UP
};
extern "C" JNIEXPORT unsigned char JNICALL Java_com_example_grapher2_GL2JNILib_touch(JNIEnv *env, jclass obj, jfloat x, jfloat y, jint msg, jint idx)
{
	switch(msg)
	{
	case ACTION_DOWN://0
	case ACTION_POINTER_DOWN://5
		if(idx<=touchInfo.size())
		{
			vec2 pos(x, y), objPos;
			//int region=click(pos, objPos);
			//int region=click(env, obj, pos, objPos);
			int region=0;
			touchInfo.insert(touchInfo.begin()+idx, TouchInfo(idx, pos, region, objPos));
			if(modes::active&&modes::mode)
				touchInfo[idx].region=modes::mode->inputTouchDown(idx);
			//push_buttons(touchInfo[idx]);
		}
		break;
	case ACTION_UP://1
	case ACTION_POINTER_UP://6
	case ACTION_OUTSIDE://4
	case ACTION_CANCEL://3
		if(idx<touchInfo.size())
		{
			if(modes::active&&modes::mode)
				modes::mode->inputTouchUp(idx);
			touchInfo.erase(touchInfo.begin()+idx);
		}
		break;
	case ACTION_MOVE://2
		if(idx<touchInfo.size())
		{
			auto &ti=touchInfo[idx];
			ti.move_to(x, y);
			if(modes::active&&modes::mode)
				modes::mode->inputTouchMove(idx);

//			new_pos.set(x, y);
//			if(modes::active&&modes::mode)
//				modes::mode->inputTouchMove(idx);
//			ti.pos.set(x, y);
		}
		break;
	}
	return false;
}
//extern "C" JNIEXPORT void JNICALL Java_com_example_grapher2_GL2JNILib_changeText(JNIEnv *env, jclass obj, jstring jstr, jint start, jint before, jint count, jint cursor)
extern "C" JNIEXPORT int JNICALL Java_com_example_grapher2_GL2JNILib_changeText(JNIEnv *env, jclass obj, jstring jstr, jint start, jint before, jint count)
{
	int quit=0;
	++nthreads;
	auto a=env->GetStringChars(jstr, nullptr);//utf16
	if(before)
		text.erase(text.begin()+start, text.begin()+start+before);
	if(count)
		text.insert(text.begin()+start, a+start, a+start+count);
	env->ReleaseStringChars(jstr, a);
	
	replaceText_update(start, before, count);


	if(exprRemoveEnd<exprInsertEnd)
		expr.insert(expr.begin()+exprRemoveEnd, exprInsertEnd-exprRemoveEnd, Expression());
	else if(exprRemoveEnd>exprInsertEnd)
		expr.erase(expr.begin()+exprInsertEnd, expr.begin()+exprRemoveEnd);
	if(functionRemoveEnd<functionInsertEnd)
		userFunctionDefinitions.insert(userFunctionDefinitions.begin()+functionRemoveEnd, functionInsertEnd-functionRemoveEnd, Expression());
	else if(functionRemoveEnd>functionInsertEnd)
		userFunctionDefinitions.erase(userFunctionDefinitions.begin()+functionInsertEnd, userFunctionDefinitions.begin()+functionRemoveEnd);

	char const			// 0				   1				   2				   3				   4				   5				   6				   7				   8				   9				   10				   11				   12
						// 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7
						//																   s ! " # $ % & ' ( ) * + , - . / 0 1 2 3 4 5 6 7 8 9 : ; < = > ? @ A B C D E F G H I J K L M N O P Q R S T U V W X Y Z [ \ ] ^ _ ` a b c d e f g h i j k l m n o p q r s t u v w x y z { | } ~ del
		*isAlphanumeric	="\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\1\1\1\1\1\1\1\1\1\1\0\0\0\0\0\0\0\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\0\0\0\0\1\0\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\0\0\0\0\0",
		*isLetter		="\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\0\0\0\0\1\0\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\0\0\0\0\0",
		*isHexadecimal	="\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\1\1\1\1\1\1\1\1\1\1\0\0\0\0\0\0\0\1\1\1\1\1\1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\1\1\1\1\1\1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
	text.insert(text.end(), 10, ' ');//"          ";

	int kStart=boundChangeStart==0?0:bounds[boundChangeStart-1].first;
	int comment=0;
	for(;comment!=allComments.size()&&allComments[comment].first<kStart;++comment);
	int lineNo=lineChangeStart=getLineNo(0, 0, kStart);

	std::vector<UFVariableName> ufVarNames;
	std::stack<char> scopeLevel;//'{', 'f' for, '(' for(, ')' for()
	int forHeaderPLevel=0;

	int e=exprChangeStart, function=functionChangeStart;
	for(int bound=boundChangeStart;bound<boundInsertEnd;++bound)//bound loop
	{
		int kStart=bound?bounds[bound-1].first:0, kEnd=bounds[bound].first;
		bool exprBound=bounds[bound].second=='e';
		int old_rmode;
		Expression *it;
		if(exprBound)
		{
			it=&expr[e];
			old_rmode=it->rmode.size()?it->rmode[0]:0;
			it->free();
		//	puis.clear();
		}
		else
		{
			it=&userFunctionDefinitions[function];
			*it=Expression();
			ufVarNames.clear();

			//parse function header		correct header syntax checked by profiler
			bool unnamed=true;
			char state='f', mathSet='R';//'f' looking for function name, 'a' reading arg names, 's' arg mathSet specifier
			int i=-1, f=-1;
			for(int k=kStart, kcEnd=comment!=allComments.size()?allComments[comment].first:kEnd;k<kEnd;++k)
			{
				if(k>=kcEnd)//skip comments
				{
					lineNo=getLineNo(allComments[comment].first, lineNo, allComments[comment].second);
					k=allComments[comment].second-1;
					++comment;
					kcEnd=comment!=allComments.size()?allComments[comment].first:text.size();
					continue;
				}
				if(isLetter[text[k]])
				{
					for(int k2=(i=k)+1;;++k2)
					{
						if(!isAlphanumeric[text[k2]])
						{
							if(state=='f')
							{
								it->name_id=userFunctionNames.addName(&text[0], i, k2)->name_id;
								unnamed=false;
							}
							else if(state=='a'||state=='s')
							{
								f=k2;
								if(state!='s'&&k2-i==1&&(text[i]=='r'||text[i]=='R'||text[i]=='c'||text[i]=='C'||text[i]=='h'||text[i]=='H'||text[i]=='q'||text[i]=='Q'))
								{
									switch(text[i])
									{
										case 'r':case 'R':mathSet='R';break;
										case 'c':case 'C':mathSet='c';break;
										case 'h':case 'H':
										case 'q':case 'Q':mathSet='h';break;
									}
									state='s';
								}
								else
								{
									ufVarNames.push_back(UFVariableName(&text[0]+i, f-i, 0, it->data.size()));
									it->insertFVar(mathSet);
								//	it->insertData(mathSet, Value());
									++it->nArgs;
									mathSet='R';
								}
							}
							k=k2-1;
							break;
						}
					}
				}
				else if(text[k]=='(')
					state='a';
				else if(text[k]==',')
				{
					if(state=='a')
						continue;
					else if(state=='s')//not a specifier but a variable
					{
						ufVarNames.push_back(UFVariableName(&text[0]+i, f-i, 0, it->data.size()));
						it->insertFVar(mathSet);
						//	it->insertData(mathSet, 0);
						continue;
					}
					else if(state=='f')//unreachable
						break;
				}
				else if(text[k]==')')//header end
				{
					for(++k;text[k]!='{';++k)
					{
						bool newline=text[k]=='\r';
						lineNo+=newline||text[k]=='\n', k+=newline&&text[k+1]=='\n';//robust
					}
					kStart=k;//definition lexing starts with { or CRASH
					//	kStart=k+1;
					break;
				}
				else if(text[k]=='{')//header end
				{
					kStart=k;
					break;
				}
				else if(text[k]!=' '&&text[k]!='\t')//unreachable
					break;
			}
			if(unnamed)
				it->name_id=-1;
		}
		it->lineNo=lineNo, it->boundNo=bound;
		bool notForLoopHeaderScope=true;
		for(int k=kStart, kcEnd=comment!=allComments.size()?allComments[comment].first:kEnd;k<kEnd;++k)//lexer loop
		{
			if(k>=kcEnd)//skip comments
			{
				lineNo=getLineNo(allComments[comment].first, lineNo, allComments[comment].second);
				k=allComments[comment].second-1;
				++comment;
				kcEnd=comment!=allComments.size()?allComments[comment].first:text.size();
				continue;
			}
			{//user function call
				NameTreeNode* node=nullptr;
				int nameEnd;
				if(exprBound)
					node=userFunctionNames.matchName_free(&text[0], k, kEnd, nameEnd);
				else//user function bound, get token call identifier
				{
					if((k-1<0||!isAlphanumeric[text[k-1]])&&isLetter[text[k]])
					{
						for(int f=k+1;f<=kEnd;++f)
						{
							if(!isAlphanumeric[text[f]])
							{
								node=userFunctionNames.matchName(&text[0], k, f, nameEnd);
								break;
							}
						}
					}
				}
				if(node)
				{
					bool match=false;
					for(int d=0, dEnd=userFunctionDefinitions.size();d<dEnd;++d)
					{
						auto &definition=userFunctionDefinitions[d];
						if(definition.m.size()&&definition.m.rbegin()->pos>k)
							break;
						if(definition.name_id==node->name_id)
						{
							match=true;
							break;
						}
					}
					if(match)
					{
						it->insertMap(k, nameEnd-k, G2::M_USER_FUNCTION, node->name_id);
						k=nameEnd-1;
						continue;
					}
					else
						userFunctionNames.removeName(&text[0], k, nameEnd);
				}
			}
			using namespace G2;
			switch(text[k])
			{
			case '(':
				if(!exprBound)
				{
					auto &scope=scopeLevel.top();
					if(scope=='f')
						scope='(', forHeaderPLevel=1;
					else if(scope=='(')
						++forHeaderPLevel;
				}
				it->insertMap(k, 1, M_LPR);
			//	it->insertMap(M_LPR);
				continue;
			case ')':
				if(!exprBound)
				{
					auto &scope=scopeLevel.top();
					if(scope=='(')
					{
						--forHeaderPLevel;
						if(!forHeaderPLevel)
							scope=')';
					}
				}
				it->insertMap(k, 1, M_RPR);
			//	it->insertMap(M_RPR);
				continue;
			case '{':
				if(!exprBound)
				{
					if(!scopeLevel.size()||scopeLevel.top()=='{')
						scopeLevel.push('{');
					else
						scopeLevel.top()='{';
					it->insertMap(k, 1, M_LBRACE);
				//	it->insertMap(M_LBRACE);
					continue;
				}
				continue;//exprBound
			case '}':
				if(!exprBound)
				{
					scopeLevel.pop();
					for(;scopeLevel.size()&&scopeLevel.top()==')';)
						scopeLevel.pop();
					if(ufVarNames.size())
					{
						int level=scopeLevel.size();
						for(int n=ufVarNames.size()-1;;--n)
						{
							if(n<0||ufVarNames[n].scopeLevel<=level)
							{
								ufVarNames.erase(ufVarNames.begin()+n+1, ufVarNames.end());
								break;
							}
						}
					}
					it->insertMap(k, 1, M_RBRACE);
				//	it->insertMap(M_RBRACE);
					continue;
				}
				continue;//exprBound
			case ',':
				it->insertMap(k, 1, M_COMMA);
			//	it->insertMap(M_COMMA);
				continue;
			case ';':
				if(!exprBound)
				{
					auto &scope=scopeLevel.top();
					if(scope==')'||scope=='f')
					{
						scopeLevel.pop();
						for(;scopeLevel.size()&&scopeLevel.top()==')';)
							scopeLevel.pop();
						if(ufVarNames.size())
						{
							int level=scopeLevel.size();
							for(int n=ufVarNames.size()-1;;--n)
							{
								if(n<0||ufVarNames[n].scopeLevel<=level)
								{
									ufVarNames.erase(ufVarNames.begin()+n+1, ufVarNames.end());
									break;
								}
							}
						}
					}
					it->insertMap(k, 1, M_SEMICOLON);
				//	it->insertMap(M_SEMICOLON);
					continue;
				}
				continue;//exprBound
				//	break;
			case '0'://hex/oct
				if((text[k+1]=='x'||text[k+1]=='X')&&(isHexadecimal[text[k+2]]||text[k+2]=='.'))
				{
					bool number=false;
					for(int k2=k+2;k2<=text.size();++k2)
					{
						if(isHexadecimal[text[k2]])
							number=true;
						else if(text[k2]!='.'&&(!commasInNumbers||text[k2]!=','))
						{
							if(number&&(exprBound||!isAlphanumeric[text[k2]]))
							{
								bool E_notation=(text[k2]=='p'||text[k2]=='P')//p notation	c99, java5
									&&(isHexadecimal[text[k2+1]]||text[k2+1]=='.'
									   ||((text[k2+1]=='+'||text[k2+1]=='-')&&(isHexadecimal[text[k2+2]]||text[k2+2]=='.')));
								if(!E_notation)
									for(;text[k2-1]==',';--k2);
								double p=1;
								for(int k4=k+2;k4<k2;++k4)
								{
									if(text[k4]=='.')
									{
										for(int k5=k4+1;k5<k2;++k5)
											if(text[k5]!='.'&&text[k5]!=',')
												p/=16;
										break;
									}
								}
								double val=0;
								for(int k4=k2-1;k4>=k+2;--k4)
								{
									switch(text[k4])
									{
										case '0':						break;
										case '1':			val+=   p;	break;
										case '2':			val+= 2*p;	break;
										case '3':			val+= 3*p;	break;
										case '4':			val+= 4*p;	break;
										case '5':			val+= 5*p;	break;
										case '6':			val+= 6*p;	break;
										case '7':			val+= 7*p;	break;
										case '8':			val+= 8*p;	break;
										case '9':			val+= 9*p;	break;
										case 'a':case 'A':	val+=10*p;	break;
										case 'b':case 'B':	val+=11*p;	break;
										case 'c':case 'C':	val+=12*p;	break;
										case 'd':case 'D':	val+=13*p;	break;
										case 'e':case 'E':	val+=14*p;	break;
										case 'f':case 'F':	val+=15*p;	break;
										default:						continue;
									}
									p*=16;
								}
								if(E_notation)
								{
									int sign=text[k2+1]=='-'?-1:1;
									bool number=false;
									for(int k3=k2+1+(text[k2+1]=='+'||text[k2+1]=='-'), k4=k3;k4<=text.size();++k4)
									{
										if(isHexadecimal[text[k4]])
											number=true;
										else if(text[k4]!='.')
										{
											if(number)
											{
												double p=1;
												for(int k5=k3;k5<k4;++k5)
												{
													if(text[k5]=='.')
													{
														for(int k6=k5+1;k6<k4;++k6)
															if(text[k6]!='.')
																p/=16;
														break;
													}
												}
												double val2=0;
												for(int k5=k4-1;k5>=k3;--k5)
												{
													if(text[k5]!='.')
													{
														switch(text[k5])
														{
															case '0':						break;
															case '1':			val2+=   p;	break;
															case '2':			val2+= 2*p;	break;
															case '3':			val2+= 3*p;	break;
															case '4':			val2+= 4*p;	break;
															case '5':			val2+= 5*p;	break;
															case '6':			val2+= 6*p;	break;
															case '7':			val2+= 7*p;	break;
															case '8':			val2+= 8*p;	break;
															case '9':			val2+= 9*p;	break;
															case 'a':case 'A':	val2+=10*p;	break;
															case 'b':case 'B':	val2+=11*p;	break;
															case 'c':case 'C':	val2+=12*p;	break;
															case 'd':case 'D':	val2+=13*p;	break;
															case 'e':case 'E':	val2+=14*p;	break;
															case 'f':case 'F':	val2+=15*p;	break;
															default:						continue;
														}
														p*=16;
													}
												}
												const double logBase=G2::_ln2;//power of 2		c99
												val*=::exp(sign*val2*logBase);
												k2=k4;
											}
											break;
										}
									}
								}
								it->insertMapData(k, k2-k, 'R', val);
							//	it->insertMapData('R', val);
							//	it->insertData('R', val);
								k=k2-1;
							}
							break;
						}
					}
					continue;
				}
				else if(text[k+1]>='0'&&text[k+1]<='7')
				{
					for(int k2=k+2;k2<=text.size();++k2)
					{
						if(text[k2]=='8'||text[k2]=='9')//default to decimal
							break;
						if((text[k2]<'0'||text[k2]>'7')&&text[k2]!='.'&&(!commasInNumbers||text[k2]!=','))
						{
							if(exprBound||!isAlphanumeric[text[k2]])
							{
								bool E_notation=(text[k2]=='e'||text[k2]=='E')
									&&((text[k2+1]>='0'&&text[k2+1]<='7')||text[k2+1]=='.'
									||((text[k2+1]=='+'||text[k2+1]=='-')&&((text[k2+2]>='0'&&text[k2+2]<='7')||text[k2+2]=='.')));
								if(!E_notation)
									for(;text[k2-1]==',';--k2);
								double p=1;
								for(int k4=k+1;k4<k2;++k4)
								{
									if(text[k4]=='.')
									{
										for(int k5=k4+1;k5<k2;++k5)
											if(text[k5]>='0'&&text[k5]<='7')
												p/=8;
										break;
									}
								}
								double val=0;
								for(int k4=k2-1;k4>=k+1;--k4)
									if(text[k4]>='0'&&text[k4]<='7')
										val+=(text[k4]-'0')*p, p*=8;
								if(E_notation)
								{
									int sign=text[k2+1]=='-'?-1:1;
									bool number=false;
									for(int k3=k2+1+(text[k2+1]=='+'||text[k2+1]=='-'), k4=k3;k4<=text.size();++k4)
									{
										if(text[k4]>='0'&&text[k4]<='7')
											number=true;
										else if(text[k4]!='.')
										{
											if(number)
											{
												double p=1;
												for(int k5=k3;k5<k4;++k5)
												{
													if(text[k5]=='.')
													{
														for(int k6=k5+1;k6<k4;++k6)
															if(text[k6]!='.')
																p/=8;
														break;
													}
												}
												double val2=0;
												for(int k5=k4-1;k5>=k3;--k5)
													if(text[k5]!='.')
														val2+=(text[k5]-'0')*p, p*=8;
												const double logBase=G2::_ln8;
												val*=exp(sign*val2*logBase);
												k2=k4;
											}
											break;
										}
									}
								}
								it->insertMapData(k, k2-k, 'R', val);
							//	it->insertMapData('R', val);
							//	it->insertData('R', val);
								k=k2-1;
							}
							break;
						}
					}
					continue;
				}
			case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':case '.':
			{
				bool number=false;
				for(int k2=k;k2<=text.size();++k2)
				{
					if(text[k2]>='0'&&text[k2]<='9')
						number=true;
					else if(text[k2]!='.'&&(!commasInNumbers||text[k2]!=','))
					{
						if(number&&(exprBound||!isAlphanumeric[text[k2]]))
						{
							bool E_notation=(text[k2]=='e'||text[k2]=='E')
								&&((text[k2+1]>='0'&&text[k2+1]<='9')||text[k2+1]=='.'
								   ||((text[k2+1]=='+'||text[k2+1]=='-')&&((text[k2+2]>='0'&&text[k2+2]<='9')||text[k2+2]=='.')));
							if(!E_notation)
								for(;text[k2-1]==',';--k2);
							double p=1;
							for(int k4=k;k4<k2;++k4)
							{
								if(text[k4]=='.')
								{
									for(int k5=k4+1;k5<k2;++k5)
										if(text[k5]>='0'&&text[k5]<='9')
											p/=10;
									break;
								}
							}
							double val=0;
							for(int k4=k2-1;k4>=k;--k4)
								if(text[k4]>='0'&&text[k4]<='9')
									val+=(text[k4]-'0')*p, p*=10;
							if(E_notation)
							{
								int sign=text[k2+1]=='-'?-1:1;
								number=false;
								for(int k3=k2+1+(text[k2+1]=='+'||text[k2+1]=='-'), k4=k3;k4<=text.size();++k4)
								{
									if(text[k4]>='0'&&text[k4]<='9')
										number=true;
									else if(text[k4]!='.')
									{
										if(number)
										{
											double p=1;
											for(int k5=k3;k5<k4;++k5)
											{
												if(text[k5]=='.')
												{
													for(int k6=k5+1;k6<k4;++k6)
														if(text[k6]!='.')
															p/=10;
													break;
												}
											}
											double val2=0;
											for(int k5=k4-1;k5>=k3;--k5)
												if(text[k5]!='.')
													val2+=(text[k5]-'0')*p, p*=10;
											const double logBase=G2::_ln10;
											val*=exp(sign*val2*logBase);
											k2=k4;
										}
										break;
									}
								}
							}
							it->insertMapData(k, k2-k, 'R', val);
						//	it->insertMapData('R', val);
						//	it->insertData('R', val);
							k=k2-1;
						}
						break;
					}
				}
			}
				continue;
			case '_':			 if(text[k+1]=='a'||text[k+1]=='A'){		 if(text[k+2]=='t'||text[k+2]=='T'){		 if(text[k+3]=='m'||text[k+3]=='M'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMapData	(k, 4, 'R', _atm				);	k+=3;	continue;}	   }}}
							else if(text[k+1]=='b'||text[k+1]=='B'){		 if(text[k+2]=='b'||text[k+2]=='B'){		 if(text[k+3]=='r'||text[k+3]=='R'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMapData	(k, 4, 'R', _bbr				);	k+=3;	continue;}	   }}}
							else if(text[k+1]=='c'||text[k+1]=='C'){																																																																																																				if(exprBound||!isAlphanumeric[text[k+2]]){	it->insertMapData	(k, 2, 'R', _c					);	++k;	continue;}	     }
							else if(text[k+1]=='e'||text[k+1]=='E'){		 if((text[k+2]=='l'||text[k+2]=='L')		&&(text[k+3]=='e'||text[k+3]=='E')){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMapData	(k, 4, 'R', _ele				);	k+=3;	continue;}	    }
																		else								   {																																																																																									if(exprBound||!isAlphanumeric[text[k+2]]){	it->insertMapData	(k, 2, 'R', _e					);	++k;	continue;}	    }}
							else if(text[k+1]=='g'				  ){																																																																																																				if(exprBound||!isAlphanumeric[text[k+2]]){	it->insertMapData	(k, 2, 'R', _g					);	++k;	continue;}	     }
							else if(text[k+1]=='G'				  ){																																																																																																				if(exprBound||!isAlphanumeric[text[k+2]]){	it->insertMapData	(k, 2, 'R', _G					);	++k;	continue;}	     }
							else if(text[k+1]=='h'||text[k+1]=='H'){																																																																																																				if(exprBound||!isAlphanumeric[text[k+2]]){	it->insertMapData	(k, 2, 'R', _h					);	++k;	continue;}	     }
							else if(text[k+1]=='m'				  ){		 if(text[k+2]=='a'||text[k+2]=='A'){		 if(text[k+3]=='g'||text[k+3]=='G'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMapData	(k, 4, 'R', _mag				);	k+=3;	continue;}	   }}
																		else if(text[k+2]=='e'||text[k+2]=='E'){																																																																																									if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMapData	(k, 3, 'R', _me					);	k+=2;	continue;}	    }
																		else if(text[k+2]=='n'||text[k+2]=='N'){																																																																																									if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMapData	(k, 3, 'R', _mn					);	k+=2;	continue;}	    }
																		else if(text[k+2]=='p'||text[k+2]=='P'){																																																																																									if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMapData	(k, 3, 'R', _mp					);	k+=2;	continue;}	    }}
							else if(				text[k+1]=='M'){		 if(text[k+2]=='a'||text[k+2]=='A'){		 if(text[k+3]=='g'||text[k+3]=='G'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMapData	(k, 4, 'R', _mag				);	k+=3;	continue;}	   }}
																		else if(text[k+2]=='e'||text[k+2]=='E'){																																																																																									if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMapData	(k, 3, 'R', _Me					);	k+=2;	continue;}	    }
																		else if(text[k+2]=='n'||text[k+2]=='N'){																																																																																									if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMapData	(k, 3, 'R', _mn					);	k+=2;	continue;}	    }
																		else if(text[k+2]=='p'||text[k+2]=='P'){																																																																																									if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMapData	(k, 3, 'R', _mp					);	k+=2;	continue;}	    }
																		else if(text[k+2]=='s'||text[k+2]=='S'){																																																																																									if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMapData	(k, 3, 'R', _Ms					);	k+=2;	continue;}	    }}
							else if(text[k+1]=='n'||text[k+1]=='N'){		 if(text[k+2]=='a'||text[k+2]=='A'){																																																																																									if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMapData	(k, 3, 'R', _Na					);	k+=2;	continue;}	    }}
							else if(text[k+1]=='p'||text[k+1]=='P'){		 if(text[k+2]=='h'||text[k+2]=='H'){		 if(text[k+3]=='i'||text[k+3]=='I'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMapData	(k, 4, 'R', _phi				);	k+=3;	continue;}	   }}
																		else if(text[k+2]=='i'||text[k+2]=='I'){																																																																																									if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMapData	(k, 3, 'R', G2::_pi				);	k+=2;	continue;}	    }}
							else if(text[k+1]=='q'||text[k+1]=='Q'){																																																																																																				if(exprBound||!isAlphanumeric[text[k+2]]){	it->insertMapData	(k, 2, 'R', _q					);	++k;	continue;}	     }
							else if(text[k+1]=='r'||text[k+1]=='R'){		 if((text[k+2]=='a'||text[k+2]=='A')		&&(text[k+3]=='n'||text[k+3]=='N')			&&(text[k+4]=='d'||text[k+4]=='D')){																																																																			if(exprBound||!isAlphanumeric[text[k+2]]){	it->insertMapData	(k, 2, 'R', rand()				);	++k;	continue;}	    }
																		else								   {																																																																																									if(exprBound||!isAlphanumeric[text[k+2]]){	it->insertMapData	(k, 2, 'R', _R					);	++k;	continue;}	    }}	break;
			case 'i':			 if(text[k+1]=='f'&&!exprBound	  ){																																																																																																															it->insertMap		(k, 2, M_IF						);	++k;	continue;		 }
							else if(text[k+1]=='m'||text[k+1]=='M'){																																																																																																				if(exprBound||!isAlphanumeric[text[k+2]]){	it->insertMap		(k, 2, M_IMAG					);	++k;	continue;}		 }
							else if(text[k+1]=='n'||text[k+1]=='N'){		 if(text[k+2]=='d'||text[k+2]=='D'){																																																																																									if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMapData	(k, 3, 'R', _qnan				);	k+=2;	continue;}		 }
																		else if(text[k+2]=='f'||text[k+2]=='F'){																																																																																									if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMapData	(k, 3, 'R', _HUGE				);	k+=2;	continue;}		 }
																		else if((text[k+2]=='v'||text[k+2]=='V')		&&(text[k+3]=='s'||text[k+3]=='S')			&&(text[k+4]=='q'||text[k+4]=='Q')			&&(text[k+5]=='r'||text[k+5]=='R')			&&(text[k+6]=='t'||text[k+6]=='T')){																																													if(exprBound||!isAlphanumeric[text[k+7]]){	it->insertMap		(k, 7, M_INVSQRT				);	k+=6;	continue;}		}
																		else if(text[k+2]=='t'||text[k+2]=='T'){																																																																																									if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMap		(k, 3, M_INT					);	k+=2;	continue;}		}}
							else								   {																																																																																																				if(exprBound||!isAlphanumeric[text[k+1]]){	it->insertMapData	(k, 1, 'c', 0, 1				);			continue;}		 }	break;
					 case 'I':	 if(text[k+1]=='m'||text[k+1]=='M'){																																																																																																				if(exprBound||!isAlphanumeric[text[k+2]]){	it->insertMap		(k, 2, M_IMAG					);	++k;	continue;}		 }
							else if(text[k+1]=='n'||text[k+1]=='N'){		 if(text[k+2]=='d'||text[k+2]=='D'){																																																																																									if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMapData	(k, 3, 'R', _qnan				);	k+=2;	continue;}		 }
																		else if(text[k+2]=='f'||text[k+2]=='F'){																																																																																									if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMapData	(k, 3, 'R', _HUGE				);	k+=2;	continue;}		 }
																		else if((text[k+2]=='v'||text[k+2]=='V')			&&(text[k+3]=='s'||text[k+3]=='S')			&&(text[k+4]=='q'||text[k+4]=='Q')			&&(text[k+5]=='r'||text[k+5]=='R')			&&(text[k+6]=='t'||text[k+6]=='T')){																																												if(exprBound||!isAlphanumeric[text[k+7]]){	it->insertMap		(k, 7, M_INVSQRT				);	k+=6;	continue;}		 }}	break;
			case 'j':																																																																																																																if(exprBound||!isAlphanumeric[text[k+1]]){	it->insertMapData	(k, 1, 'h', 0, 0, 1				);			continue;}			break;
					 case 'J':																																																																																																														if(exprBound||!isAlphanumeric[text[k+1]]){	it->insertMap		(k, 1, M_BESSEL_J					);			continue;}			break;
			case 'k':																																																																																																																if(exprBound||!isAlphanumeric[text[k+1]]){	it->insertMapData	(k, 1, 'h', 0, 0, 0, 1			);			continue;}
			case 'x':			 if(exprBound)						{																																																																																																															it->insertRVar		(k, 1, &text[k], 's'			);			continue;		  }	break;
			case 'y':			 if(exprBound)						{																																																																																																															it->insertRVar		(k, 1, &text[k], 's'			);			continue;		  }	break;
			case 'Y':																																																																																																																if(exprBound||!isAlphanumeric[text[k+1]]){	it->insertMap		(k, 1, M_BESSEL_Y				);}			continue;
			case 'z':			 if((text[k+1]=='e'||text[k+1]=='E')		&&(text[k+2]=='t'||text[k+2]=='T')			&&(text[k+3]=='a'||text[k+3]=='A')){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_ZETA					);	k+=3;	continue;}		  }
							else if(exprBound)					   {																																																																																																															it->insertRVar		(k, 1, &text[k], 's'			);			continue;		  }	break;
			case 'Z':			 if((text[k+1]=='e'||text[k+1]=='E')		&&(text[k+2]=='t'||text[k+2]=='T')			&&(text[k+3]=='a'||text[k+3]=='A')){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_ZETA					);	k+=3;	continue;}		  }
							else if(exprBound)					   {																																																																																																															it->insertCVar		(k, 1, &text[k]					);			continue;		  }	break;
			case '\'':
				if(exprBound)
				{
					bool hit=false;
					for(int k2=k+1;k2<=text.size();++k2)
					{
						if(!isAlphanumeric[text[k2]])
						{
							it->insertRVar(k+1, k2-(k+1), &text[k+1], 's');
						//	it->insertRVar(&text[k+1], k2-(k+1), 's');
							k=text[k2]=='\''?k2:k2-1;
							hit=true;
							break;
						}
					}
					if(hit)
						continue;
				}
				break;
			case 'Q':			 if(exprBound)						{																																																																																																															it->insertHVar		(k, 1, &text[k]					);			continue;		 }	break;
			case '+':			 if(text[k+1]=='='&&!exprBound	  ){																																																																																																															it->insertMap		(k, 1, M_ASSIGN_PLUS			);	++k;	continue;		 }
							else if(text[k+1]=='+'&&!exprBound	  ){																																																																																																															it->insertMap		(k, 2, M_INCREMENT				);	++k;	continue;		 }
							else								   {																																																																																																															it->insertMap		(k, 1, M_PLUS					);			continue;		 }	break;
			case '-':			 if(text[k+1]=='='&&!exprBound	  ){																																																																																																															it->insertMap		(k, 2, M_ASSIGN_MINUS			);	++k;	continue;		 }
							else if(text[k+1]=='-'&&!exprBound	  ){																																																																																																															it->insertMap		(k, 2, M_DECREMENT				);	++k;	continue;		 }
							else								   {																																																																																																															it->insertMap		(k, 1, M_MINUS					);			continue;		 }	break;
			case '*':			 if(text[k+1]=='*'				  ){																																																																																																															it->insertMap		(k, 2, M_POWER_REAL				);	++k;	continue;		 }
							else if(text[k+1]=='='&&!exprBound	  ){																																																																																																															it->insertMap		(k, 2, M_ASSIGN_MULTIPLY		);	++k;	continue;		 }
							else								   {																																																																																																															it->insertMap		(k, 1, M_MULTIPLY				);			continue;		 }	break;
			case '/':			 if(text[k+1]=='='&&!exprBound	  ){																																																																																																															it->insertMap		(k, 2, M_ASSIGN_DIVIDE			);	++k;	continue;		 }
							else								   {																																																																																																															it->insertMap		(k, 1, M_DIVIDE					);			continue;		 }	break;
			case '%':			 if(text[k+1]=='='&&!exprBound	  ){																																																																																																															it->insertMap		(k, 2, M_ASSIGN_MOD				);	++k;	continue;		 }
							else								   {									   																																																																																																						it->insertMap		(k, 1, M_MODULO_PERCENT			);			continue;		 }	break;
			case '@':																					   																																																																																																						it->insertMap		(k, 1, M_LOGIC_DIVIDES			);			continue;
			case '^':			 if(text[k+1]=='^'				  ){		 if(text[k+2]=='^'				  ){																																																																																																				it->insertMap		(k, 3, M_PENTATE				);	k+=2;	continue;	    }
																		else								   {																																																																																																				it->insertMap		(k, 2, M_TETRATE				);	++k;	continue;	    }}
							else if(text[k+1]=='~'				  ){																																																																																																															it->insertMap		(k, 2, M_BITWISE_XNOR			);	++k;	continue;	     }
							else								   {																																																																																																															it->insertMap		(k, 1, M_POWER					);			continue;	     }	break;
			case '!':			 if(text[k+1]=='='				  ){																																																																																																															it->insertMap		(k, 2, M_LOGIC_NOT_EQUAL		);	++k;	continue;		 }
							else								   {																																																																																																															it->insertMap		(k, 1, M_FACTORIAL_LOGIC_NOT	);			continue;		 }	break;
			case '~':			 if(text[k+1]=='&'				  ){																																																																																																															it->insertMap		(k, 2, M_BITWISE_NAND			);	++k;	continue;		 }
							else if(text[k+1]=='#'				  ){																																																																																																															it->insertMap		(k, 2, M_BITWISE_XNOR			);	++k;	continue;		 }
							else if(text[k+1]=='|'				  ){																																																																																																															it->insertMap		(k, 2, M_BITWISE_NOR			);	++k;	continue;		 }
							else								   {																																																																																																															it->insertMap		(k, 1, M_BITWISE_NOT			);			continue;		 }	break;
			case '&':			 if(text[k+1]=='&'				  ){																																																																																																															it->insertMap		(k, 2, M_LOGIC_AND				);	++k;	continue;	     }
							else if(text[k+1]=='~'				  ){																																																																																																															it->insertMap		(k, 2, M_BITWISE_NOR			);	++k;	continue;	     }
							else if(text[k+1]=='='&&!exprBound	  ){																																																																																																															it->insertMap		(k, 2, M_ASSIGN_AND				);	++k;	continue;	     }
							else								   {																																																																																																															it->insertMap		(k, 1, M_BITWISE_AND			);			continue;		 }	break;
			case '#':			 if(text[k+1]=='#'				  ){																																																																																																															it->insertMap		(k, 2, M_LOGIC_XOR				);	++k;	continue;	     }
							else if(text[k+1]=='~'				  ){																																																																																																															it->insertMap		(k, 2, M_BITWISE_XNOR			);	++k;	continue;	     }
							else if(text[k+1]=='='&&!exprBound	  ){																																																																																																															it->insertMap		(k, 2, M_ASSIGN_XOR				);	++k;	continue;	     }
							else								   {																																																																																																															it->insertMap		(k, 1, M_BITWISE_XOR			);			continue;		 }	break;
			case '|':			 if(text[k+1]=='|'				  ){																																																																																																															it->insertMap		(k, 2, M_LOGIC_OR				);	++k;	continue;	     }
							else if(text[k+1]=='~'				  ){																																																																																																															it->insertMap		(k, 2, M_BITWISE_NAND			);	++k;	continue;	     }
							else if(text[k+1]=='='&&!exprBound	  ){																																																																																																															it->insertMap		(k, 2, M_ASSIGN_OR				);	++k;	continue;	     }
							else								   {																																																																																																															it->insertMap		(k, 1, M_VERTICAL_BAR			);			continue;	     }	break;
			case '<':			 if(text[k+1]=='<'				  ){		 if(text[k+2]=='='&&!exprBound	  ){																																																																																																				it->insertMap		(k, 3, M_ASSIGN_LEFT			);	k+=2;	continue;		}
																		else								   {																																																																																																				it->insertMap		(k, 2, M_BITWISE_SHIFT_LEFT		);	++k;	continue;	    }}
							else if(text[k+1]=='='				  ){																																																																																																															it->insertMap		(k, 2, M_LOGIC_LESS_EQUAL		);	++k;	continue;	     }
							else								   {																																																																																																															it->insertMap		(k, 1, M_LOGIC_LESS				);			continue;	     }	break;
			case '>':			 if(text[k+1]=='>'				  ){		 if(text[k+2]=='='&&!exprBound	  ){																																																																																																				it->insertMap		(k, 3, M_ASSIGN_RIGHT			);	k+=2;	continue;		}
																		else								   {																																																																																																				it->insertMap		(k, 2, M_BITWISE_SHIFT_RIGHT	);	++k;	continue;	    }}
							else if(text[k+1]=='='				  ){																																																																																																															it->insertMap		(k, 2, M_LOGIC_GREATER_EQUAL	);	++k;	continue;	     }
							else								   {																																																																																																															it->insertMap		(k, 1, M_LOGIC_GREATER			);			continue;	     }	break;
			case '=':			 if(text[k+1]=='='				  ){																																																																																																															it->insertMap		(k, 2, M_LOGIC_EQUAL			);	++k;	continue;	     }
							else if(!exprBound					  ){																																																																																																															it->insertMap		(k, 1, M_ASSIGN					);			continue;		 }	break;
			case '?':			 if(text[k+1]=='?'				  ){																																																																																																															it->insertMap		(k, 2, M_CONDITION_ZERO		);	++k;	continue;		 }
							else																																																																																																																								it->insertMap		(k, 1, M_QUESTION_MARK			);			continue;			break;
			case ':':																																																																																																																											it->insertMap		(k, 1, M_COLON					);			continue;
			case 'a':case 'A':	 if(text[k+1]=='b'||text[k+1]=='B'){		 if(text[k+2]=='s'||text[k+2]=='S'){																																																																																									if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMap		(k, 3, M_ABS					);	k+=2;	continue;}		}}
							else if(text[k+1]=='c'||text[k+1]=='C'){		 if(text[k+2]=='h'||text[k+2]=='H'){																																																																																									if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMap		(k, 3, M_ACOSH					);	k+=2;	continue;}	    }
																		else if(text[k+2]=='o'||text[k+2]=='O'){		 if(text[k+3]=='s'||text[k+3]=='S'){		 if(text[k+4]=='e'||text[k+4]=='E'){		 if(text[k+5]=='c'||text[k+5]=='C'){		 if(text[k+6]=='h'||text[k+6]=='H'){																																													if(exprBound||!isAlphanumeric[text[k+7]]){	it->insertMap		(k, 7, M_ACSCH					);	k+=6;	continue;}	}
																																																														else								   {																																													if(exprBound||!isAlphanumeric[text[k+6]]){	it->insertMap		(k, 6, M_ACSC					);	k+=5;	continue;}	}}}
																																								else if(text[k+4]=='h'||text[k+4]=='H'){																																																																			if(exprBound||!isAlphanumeric[text[k+5]]){	it->insertMap		(k, 5, M_ACOSH					);	k+=4;	continue;}	  }
																																								else								   {																																																																			if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_ACOS					);	k+=3;	continue;}	  }}
																														 if(text[k+3]=='t'||text[k+3]=='T'){		 if(text[k+4]=='h'||text[k+4]=='H'){																																																																			if(exprBound||!isAlphanumeric[text[k+5]]){	it->insertMap		(k, 5, M_ACOTH					);	k+=4;	continue;}	  }
																																								else								   {																																																																			if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_ACOT					);	k+=3;	continue;}	  }}}
																		else if(text[k+2]=='s'||text[k+2]=='S'){		 if(text[k+3]=='c'||text[k+3]=='C'){		 if(text[k+4]=='h'||text[k+4]=='H'){																																																																			if(exprBound||!isAlphanumeric[text[k+5]]){	it->insertMap		(k, 5, M_ACSCH					);	k+=4;	continue;}	  }
																																								else								   {																																																																			if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_ACSC					);	k+=3;	continue;}	  }}}}
							else if(text[k+1]=='r'||text[k+1]=='R'){		 if(text[k+2]=='g'||text[k+2]=='G'){																																																																																									if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMap		(k, 3, M_ARG					);	k+=2;	continue;}	    }}
							else if(text[k+1]=='s'||text[k+1]=='S'){		 if(text[k+2]=='e'||text[k+2]=='E'){		 if(text[k+3]=='c'||text[k+3]=='C'){		 if(text[k+4]=='h'||text[k+4]=='H'){																																																																			if(exprBound||!isAlphanumeric[text[k+5]]){	it->insertMap		(k, 5, M_ASECH					);	k+=4;	continue;}	  }
																																								else								   {																																																																			if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_ASEC					);	k+=3;	continue;}	  }}}
																		else if(text[k+2]=='h'||text[k+2]=='H'){																																																																																									if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMap		(k, 3, M_ASINH					);	k+=2;	continue;}	    }
																		else if(text[k+2]=='i'||text[k+2]=='I'){		 if(text[k+3]=='n'||text[k+3]=='N'){		 if(text[k+4]=='h'||text[k+4]=='H'){																																																																			if(exprBound||!isAlphanumeric[text[k+5]]){	it->insertMap		(k, 5, M_ASINH					);	k+=4;	continue;}	  }
																																								else								   {																																																																			if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_ASIN					);	k+=3;	continue;}	  }}}}
							else if(text[k+1]=='t'||text[k+1]=='T'){		 if(text[k+2]=='a'||text[k+2]=='A'){		 if(text[k+3]=='n'||text[k+3]=='N'){		 if(text[k+4]=='h'||text[k+4]=='H'){																																																																			if(exprBound||!isAlphanumeric[text[k+5]]){	it->insertMap		(k, 5, M_ATANH					);	k+=4;	continue;}	  }
																																								else								   {																																																																			if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_ATAN					);	k+=3;	continue;}	  }}}
																		else if(text[k+2]=='h'||text[k+2]=='H'){																																																																																									if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMap		(k, 3, M_ATANH					);	k+=2;	continue;}	    }}	break;
			case 'b':			 if(text[k+1]=='e'||text[k+1]=='E'){		 if(text[k+2]=='s'||text[k+2]=='S'){		 if(text[k+3]=='s'||text[k+3]=='S'){		 if(text[k+4]=='e'||text[k+4]=='E'){		 if(text[k+5]=='l'||text[k+5]=='L'){																																																								if(exprBound||!isAlphanumeric[text[k+6]]){	it->insertMap		(k, 6, M_BESSEL_J					);	k+=5;	continue;}	 }}}}
																			 if(text[k+2]=='t'||text[k+2]=='T'){		 if(text[k+3]=='a'||text[k+3]=='A'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_BETA					);	k+=3;	continue;}	   }}}
							else if(text[k+1]=='r'				  ){		 if(text[k+2]=='e'				  ){		 if(text[k+3]=='a'				  ){		 if(text[k+4]=='k'&&!exprBound	  ){																																																																														it->insertMap		(k, 5, M_BREAK					);	k+=4;	continue;	  }}}}
				else if(text[k+1]=='0'||text[k+1]=='1'||text[k+1]=='.')//binary
				{
					bool number=false;
					for(int k2=k+1;k2<=text.size();++k2)
					{
						if(text[k2]>='2'&&text[k2]<='9')//default to decimal
							break;
						if(text[k2]=='0'||text[k2]=='1')
							number=true;
						else if(text[k2]!='.'&&(!commasInNumbers||text[k2]!=','))
						{
							if(number&&(exprBound||!isAlphanumeric[text[k+3]]))
							{
								bool E_notation=(text[k2]=='e'||text[k2]=='E')
									&&((text[k2+1]>='0'&&text[k2+1]<='1')||text[k2+1]=='.'
									   ||((text[k2+1]=='+'||text[k2+1]=='-')&&((text[k2+2]>='0'&&text[k2+2]<='1')||text[k2+2]=='.')));
								if(!E_notation)
									for(;text[k2-1]==',';--k2);
								double p=1;
								for(int k4=k+1;k4<k2;++k4)
								{
									if(text[k4]=='.')
									{
										for(int k5=k4+1;k5<k2;++k5)
											if(text[k5]>='0'&&text[k5]<='1')
												p/=2;
										break;
									}
								}
								double val=0;
								for(int k4=k2-1;k4>=k+1;--k4)
									if(text[k4]>='0'&&text[k4]<='1')
										val+=(text[k4]-'0')*p, p*=2;
								if(E_notation)
								{
									int sign=text[k2+1]=='-'?-1:1;
									number=false;
									for(int k3=k2+1+(text[k2+1]=='+'||text[k2+1]=='-'), k4=k3;k4<=text.size();++k4)
									{
										if(text[k4]>='0'&&text[k4]<='1')
											number=true;
										else if(text[k4]!='.')
										{
											if(number)
											{
												double p=1;
												for(int k5=k3;k5<k4;++k5)
												{
													if(text[k5]=='.')
													{
														for(int k6=k5+1;k6<k4;++k6)
															if(text[k6]!='.')
																p/=2;
														break;
													}
												}
												double val2=0;
												for(int k5=k4-1;k5>=k3;--k5)
													if(text[k5]!='.')
														val2+=(text[k5]-'0')*p, p*=2;
												const double logBase=G2::_ln2;
												val*=exp(sign*val2*logBase);
												k2=k4;
											}
											break;
										}
									}
								}
								it->insertMapData(k, k2-k, 'R', val);
							//	it->insertMapData('R', val);
							//	it->insertData('R', val);
								k=k2-1;
							}
							break;
						}
					}
					continue;
				}
				break;
			case 'B':			 if(text[k+1]=='e'||text[k+1]=='E'){		 if(text[k+2]=='s'||text[k+2]=='S'){		 if(text[k+3]=='s'||text[k+3]=='S'){		 if(text[k+4]=='e'||text[k+4]=='E'){		 if(text[k+5]=='l'||text[k+5]=='L'){																																																								if(exprBound||!isAlphanumeric[text[k+6]]){	it->insertMap		(k, 6, M_BESSEL_J					);	k+=5;	continue;}	 }}}}
																			 if(text[k+2]=='t'||text[k+2]=='T'){		 if(text[k+3]=='a'||text[k+3]=='A'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_BETA					);	k+=3;	continue;}	   }}}
				else if(text[k+1]=='0'||text[k+1]=='1'||text[k+1]=='.')//binary
				{
					bool number=false;
					for(int k2=k+1;k2<=text.size();++k2)
					{
						if(text[k2]>='2'&&text[k2]<='9')//default to decimal
							break;
						if(text[k2]=='0'||text[k2]=='1')
							number=true;
						else if(text[k2]!='.'&&(!commasInNumbers||text[k2]!=','))
						{
							if(number&&(exprBound||!isAlphanumeric[text[k+3]]))
							{
								bool E_notation=(text[k2]=='e'||text[k2]=='E')
									&&((text[k2+1]>='0'&&text[k2+1]<='1')||text[k2+1]=='.'
									   ||((text[k2+1]=='+'||text[k2+1]=='-')&&((text[k2+2]>='0'&&text[k2+2]<='1')||text[k2+2]=='.')));
								if(!E_notation)
									for(;text[k2-1]==',';--k2);
								double p=1;
								for(int k4=k+1;k4<k2;++k4)
								{
									if(text[k4]=='.')
									{
										for(int k5=k4+1;k5<k2;++k5)
											if(text[k5]>='0'&&text[k5]<='1')
												p/=2;
										break;
									}
								}
								double val=0;
								for(int k4=k2-1;k4>=k+1;--k4)
									if(text[k4]>='0'&&text[k4]<='1')
										val+=(text[k4]-'0')*p, p*=2;
								if(E_notation)
								{
									int sign=text[k2+1]=='-'?-1:1;
									number=false;
									for(int k3=k2+1+(text[k2+1]=='+'||text[k2+1]=='-'), k4=k3;k4<=text.size();++k4)
									{
										if(text[k4]>='0'&&text[k4]<='1')
											number=true;
										else if(text[k4]!='.')
										{
											if(number)
											{
												double p=1;
												for(int k5=k3;k5<k4;++k5)
												{
													if(text[k5]=='.')
													{
														for(int k6=k5+1;k6<k4;++k6)
															if(text[k6]!='.')
																p/=2;
														break;
													}
												}
												double val2=0;
												for(int k5=k4-1;k5>=k3;--k5)
													if(text[k5]!='.')
														val2+=(text[k5]-'0')*p, p*=2;
												const double logBase=G2::_ln2;
												val*=exp(sign*val2*logBase);
												k2=k4;
											}
											break;
										}
									}
								}
								it->insertMapData(k, k2-k, 'R', val);
								//	it->insertMapData('R', val);
								//	it->insertData('R', val);
								k=k2-1;
							}
							break;
						}
					}
					continue;
				}
				break;
			case 'c':			 if(text[k+1]=='a'||text[k+1]=='A'){		 if(text[k+2]=='r'||text[k+2]=='R'){		 if(text[k+3]=='t'||text[k+3]=='T'){		 if(text[k+4]=='e'||text[k+4]=='E'){		 if(text[k+5]=='s'||text[k+5]=='S'){		 if(text[k+6]=='i'||text[k+6]=='I'){		 if(text[k+7]=='a'||text[k+7]=='A'){		 if(text[k+8]=='n'||text[k+8]=='N'){																							if(exprBound||!isAlphanumeric[text[k+9]]){	it->insertMap		(k, 9, M_CARTESIAN				);	k+=8;	continue;} }}}}}}}}
							else if(text[k+1]=='b'||text[k+1]=='B'){		 if(text[k+2]=='r'||text[k+2]=='R'){		 if(text[k+3]=='t'||text[k+3]=='T'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_CBRT					);	k+=3;	continue;}	   }}}
							else if(text[k+1]=='e'||text[k+1]=='E'){		 if(text[k+2]=='i'||text[k+2]=='I'){		 if(text[k+3]=='l'||text[k+3]=='L'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_CEIL					);	k+=3;	continue;}	   }}}
							else if(text[k+1]=='h'||text[k+1]=='H'){																																																																																																				if(exprBound||!isAlphanumeric[text[k+2]]){	it->insertMap		(k, 2, M_COSH					);	++k;	continue;}	     }
							else if(text[k+1]=='o'				  ){		 if(text[k+2]=='m'||text[k+2]=='M'){		 if(text[k+3]=='b'||text[k+3]=='B'){		 if(text[k+4]=='i'||text[k+4]=='I'){		 if(text[k+5]=='n'||text[k+5]=='N'){		 if(text[k+6]=='a'||text[k+6]=='A'){		 if(text[k+7]=='t'||text[k+7]=='T'){		 if(text[k+8]=='i'||text[k+8]=='I'){		 if(text[k+9]=='o'||text[k+9]=='O'){		 if(text[k+10]=='n'||text[k+10]=='N'){	if(exprBound||!isAlphanumeric[text[k+11]]){	it->insertMap		(k, 11, M_COMBINATION			);	k+=10;	continue;}}}}}}}}}}
																		else if(text[k+2]=='n'				  ){		 if(text[k+3]=='j'||text[k+3]=='J'){		 if(text[k+4]=='u'||text[k+4]=='U'){		 if(text[k+5]=='g'||text[k+5]=='G'){		 if(text[k+6]=='a'||text[k+6]=='A'){		 if(text[k+7]=='t'||text[k+7]=='T'){		 if(text[k+8]=='e'||text[k+8]=='E'){																							if(exprBound||!isAlphanumeric[text[k+9]]){	it->insertMap		(k, 9, M_CONJUGATE				);	k+=8;	continue;} }}}}}}
																													else if(text[k+3]=='t'							 &&text[k+4]=='i'							 &&text[k+5]=='n'							 &&text[k+6]=='u'							 &&text[k+7]=='e'&&!exprBound	  ){																																													it->insertMap		(k, 8, M_CONTINUE				);	k+=7;	continue;     }}
																		else if(				text[k+2]=='N'){		 if(text[k+3]=='j'||text[k+3]=='J'){		 if(text[k+4]=='u'||text[k+4]=='U'){		 if(text[k+5]=='g'||text[k+5]=='G'){		 if(text[k+6]=='a'||text[k+6]=='A'){		 if(text[k+7]=='t'||text[k+7]=='T'){		 if(text[k+8]=='e'||text[k+8]=='E'){																							if(exprBound||!isAlphanumeric[text[k+9]]){	it->insertMap		(k, 9, M_CONJUGATE				);	k+=8;	continue;} }}}}}}}
																		else if(text[k+2]=='s'||text[k+2]=='S'){		 if(text[k+3]=='c'||text[k+3]=='C'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_COSC					);	k+=3;	continue;}	   }
																													else if(text[k+3]=='e'||text[k+3]=='E'){		 if(text[k+4]=='c'||text[k+4]=='C'){		 if(text[k+5]=='h'||text[k+5]=='H'){																																																								if(exprBound||!isAlphanumeric[text[k+6]]){	it->insertMap		(k, 6, M_CSCH					);	k+=5;	continue;}	 }
																																																			else								   {																																																								if(exprBound||!isAlphanumeric[text[k+5]]){	it->insertMap		(k, 5, M_CSC					);	k+=4;	continue;}	 }}}
																													else if(text[k+3]=='h'||text[k+3]=='H'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_COSH					);	k+=3;	continue;}	   }
																													else								   {																																																																														if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMap		(k, 3, M_COS					);	k+=2;	continue;}	   }}
																		else if(text[k+2]=='t'||text[k+2]=='T'){		 if(text[k+3]=='h'||text[k+3]=='H'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_COTH					);	k+=3;	continue;}	   }
																													else								   {																																																																														if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMap		(k, 3, M_COT					);	k+=2;	continue;}	   }}}
							else if(				text[k+1]=='O'){		 if(text[k+2]=='m'||text[k+2]=='M'){		 if(text[k+3]=='b'||text[k+3]=='B'){		 if(text[k+4]=='i'||text[k+4]=='I'){		 if(text[k+5]=='n'||text[k+5]=='N'){		 if(text[k+6]=='a'||text[k+6]=='A'){		 if(text[k+7]=='t'||text[k+7]=='T'){		 if(text[k+8]=='i'||text[k+8]=='I'){		 if(text[k+9]=='o'||text[k+9]=='O'){		 if(text[k+10]=='n'||text[k+10]=='N'){	if(exprBound||!isAlphanumeric[text[k+11]]){	it->insertMap		(k, 11, M_COMBINATION			);	k+=10;	continue;}}}}}}}}}}
																		else if(text[k+2]=='n'||text[k+2]=='N'){		 if(text[k+3]=='j'||text[k+3]=='J'){		 if(text[k+4]=='u'||text[k+4]=='U'){		 if(text[k+5]=='g'||text[k+5]=='G'){		 if(text[k+6]=='a'||text[k+6]=='A'){		 if(text[k+7]=='t'||text[k+7]=='T'){		 if(text[k+8]=='e'||text[k+8]=='E'){																							if(exprBound||!isAlphanumeric[text[k+9]]){	it->insertMap		(k, 9, M_CONJUGATE				);	k+=8;	continue;} }}}}}}}
																		else if(text[k+2]=='s'||text[k+2]=='S'){		 if(text[k+3]=='c'||text[k+3]=='C'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_COSC					);	k+=3;	continue;}	   }
																													else if(text[k+3]=='e'||text[k+3]=='E'){		 if(text[k+4]=='c'||text[k+4]=='C'){		 if(text[k+5]=='h'||text[k+5]=='H'){																																																								if(exprBound||!isAlphanumeric[text[k+6]]){	it->insertMap		(k, 6, M_CSCH					);	k+=5;	continue;}	 }
																																																			else								   {																																																								if(exprBound||!isAlphanumeric[text[k+5]]){	it->insertMap		(k, 5, M_CSC					);	k+=4;	continue;}	 }}}
																													else if(text[k+3]=='h'||text[k+3]=='H'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_COSH					);	k+=3;	continue;}	   }
																													else								   {																																																																														if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMap		(k, 3, M_COS					);	k+=2;	continue;}	   }}
																		else if(text[k+2]=='t'||text[k+2]=='T'){		 if(text[k+3]=='h'||text[k+3]=='H'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_COTH					);	k+=3;	continue;}	   }
																													else								   {																																																																														if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMap		(k, 3, M_COT					);	k+=2;	continue;}	   }}}
							else if(text[k+1]=='s'||text[k+1]=='S'){		 if(text[k+2]=='c'||text[k+2]=='C'){		 if(text[k+3]=='h'||text[k+3]=='H'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_CSCH					);	k+=3;	continue;}	   }
																													else								   {																																																																														if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMap		(k, 3, M_CSC					);	k+=2;	continue;}	   }}}	break;
					 case 'C':	 if(text[k+1]=='a'||text[k+1]=='A'){		 if(text[k+2]=='r'||text[k+2]=='R'){		 if(text[k+3]=='t'||text[k+3]=='T'){		 if(text[k+4]=='e'||text[k+4]=='E'){		 if(text[k+5]=='s'||text[k+5]=='S'){		 if(text[k+6]=='i'||text[k+6]=='I'){		 if(text[k+7]=='a'||text[k+7]=='A'){		 if(text[k+8]=='n'||text[k+8]=='N'){																							if(exprBound||!isAlphanumeric[text[k+9]]){	it->insertMap		(k, 9, M_CARTESIAN				);	k+=8;	continue;} }}}}}}}}
							else if(text[k+1]=='b'||text[k+1]=='B'){		 if(text[k+2]=='r'||text[k+2]=='R'){		 if(text[k+3]=='t'||text[k+3]=='T'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_CBRT					);	k+=3;	continue;}	   }}}
							else if(text[k+1]=='e'||text[k+1]=='E'){		 if(text[k+2]=='i'||text[k+2]=='I'){		 if(text[k+3]=='l'||text[k+3]=='L'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_CEIL					);	k+=3;	continue;}	   }}}
							else if(text[k+1]=='h'||text[k+1]=='H'){																																																																																																				if(exprBound||!isAlphanumeric[text[k+2]]){	it->insertMap		(k, 2, M_COSH					);	++k;	continue;}	     }
							else if(text[k+1]=='o'||text[k+1]=='O'){		 if(text[k+2]=='m'||text[k+2]=='M'){		 if(text[k+3]=='b'||text[k+3]=='B'){		 if(text[k+4]=='i'||text[k+4]=='I'){		 if(text[k+5]=='n'||text[k+5]=='N'){		 if(text[k+6]=='a'||text[k+6]=='A'){		 if(text[k+7]=='t'||text[k+7]=='T'){		 if(text[k+8]=='i'||text[k+8]=='I'){		 if(text[k+9]=='o'||text[k+9]=='O'){		 if(text[k+10]=='n'||text[k+10]=='N'){	if(exprBound||!isAlphanumeric[text[k+11]]){	it->insertMap		(k, 11, M_COMBINATION			);	k+=10;	continue;}}}}}}}}}}
																		else if(text[k+2]=='n'||text[k+2]=='N'){		 if(text[k+3]=='j'||text[k+3]=='J'){		 if(text[k+4]=='u'||text[k+4]=='U'){		 if(text[k+5]=='g'||text[k+5]=='G'){		 if(text[k+6]=='a'||text[k+6]=='A'){		 if(text[k+7]=='t'||text[k+7]=='T'){		 if(text[k+8]=='e'||text[k+8]=='E'){																							if(exprBound||!isAlphanumeric[text[k+9]]){	it->insertMap		(k, 9, M_CONJUGATE				);	k+=8;	continue;} }}}}}}}
																		else if(text[k+2]=='s'||text[k+2]=='S'){		 if(text[k+3]=='c'||text[k+3]=='C'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_COSC					);	k+=3;	continue;}	   }
																													else if(text[k+3]=='e'||text[k+3]=='E'){		 if(text[k+4]=='c'||text[k+4]=='C'){		 if(text[k+5]=='h'||text[k+5]=='H'){																																																								if(exprBound||!isAlphanumeric[text[k+6]]){	it->insertMap		(k, 6, M_CSCH					);	k+=5;	continue;}	 }
																																																			else								   {																																																								if(exprBound||!isAlphanumeric[text[k+5]]){	it->insertMap		(k, 5, M_CSC					);	k+=4;	continue;}	 }}}
																													else if(text[k+3]=='h'||text[k+3]=='H'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_COSH					);	k+=3;	continue;}	   }
																													else								   {																																																																														if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMap		(k, 3, M_COS					);	k+=2;	continue;}	   }}
																		else if(text[k+2]=='t'||text[k+2]=='T'){		 if(text[k+3]=='h'||text[k+3]=='H'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_COTH					);	k+=3;	continue;}	   }
																													else								   {																																																																														if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMap		(k, 3, M_COT					);	k+=2;	continue;}	   }}}
							else if(text[k+1]=='s'||text[k+1]=='S'){		 if(text[k+2]=='c'||text[k+2]=='C'){		 if(text[k+3]=='h'||text[k+3]=='H'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_CSCH					);	k+=3;	continue;}	   }
																													else								   {																																																																														if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMap		(k, 3, M_CSC					);	k+=2;	continue;}	   }}}	break;
			case 'd':			 if(text[k+1]=='o'&&!exprBound	  ){																																																																																																															it->insertMap		(k, 2, M_DO						);	++k;	continue;		 }	break;
			case 'e':			 if(text[k+1]=='l'				  ){		 if(text[k+2]=='s'							 &&text[k+3]=='e'&&!exprBound){																																																																																											it->insertMap		(k, 4, M_ELSE					);	k+=3;	continue;	    }}
							else if(text[k+1]=='r'||text[k+1]=='R'){		 if(text[k+2]=='f'||text[k+2]=='F'){																																																																																									if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMap		(k, 3, M_ERF					);	k+=2;	continue;}	    }}
							else if(text[k+1]=='x'||text[k+1]=='X'){	 	 if((text[k+2]=='i'||text[k+2]=='I')		 &&(text[k+3]=='t'||text[k+3]=='T'))		quit=42, abort();
																		else if(text[k+2]=='p'||text[k+2]=='P'){																																																																																									if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMap		(k, 3, M_EXP					);	k+=2;	continue;}	     }}
							else								   {																																																																																																				if(exprBound||!isAlphanumeric[text[k+1]]){	it->insertMapData	(k, 1, 'R', _e					);			continue;}		 }	break;
			case 'E':			 if(text[k+1]=='r'||text[k+1]=='R'){		 if(text[k+2]=='f'||text[k+2]=='F'){																																																																																									if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMap		(k, 3, M_ERF					);	k+=2;	continue;}	    }}
							else if((text[k+1]=='x'||text[k+1]=='X')		 &&(text[k+2]=='p'||text[k+2]=='P')){																																																																																									if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMap		(k, 3, M_EXP					);	k+=2;	continue;}	     }
							else								   {																																																																																																				if(exprBound||!isAlphanumeric[text[k+1]]){	it->insertMapData	(k, 1, 'R', _e					);			continue;}		 }	break;
			case 'f':			 if(text[k+1]=='i'||text[k+1]=='I'){		 if(text[k+2]=='b'||text[k+2]=='B'){																																																																																									if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMap		(k, 3, M_FIB					);	k+=2;	continue;}		}}
								 if(text[k+1]=='a'&&!exprBound		){		 if(text[k+2]=='l'							 &&text[k+3]=='s'							 &&text[k+4]=='e'					)																																																																														it->insertMapData	(k, 5, 'R', 0					);	k+=4;	continue;		 }
							else if(text[k+1]=='l'||text[k+1]=='L'){		 if(text[k+2]=='o'||text[k+2]=='O'){		 if(text[k+3]=='o'||text[k+3]=='O'){		 if(text[k+4]=='r'||text[k+4]=='R'){																																																																			if(exprBound||!isAlphanumeric[text[k+5]]){	it->insertMap		(k, 5, M_FLOOR					);	k+=4;	continue;}	  }}}}
				else if(text[k+1]=='o'				  ){		 if(text[k+2]=='r'&&!exprBound	  ){
						auto &scope=scopeLevel.top();
						if(scope=='{'||scope==')')
							scopeLevel.push('f');
						it->insertMap(k, 3, M_FOR);
						//it->insertMap(M_FOR);
						k+=2;
						continue;
					}}
							else if((text[k+1]=='r'||text[k+1]=='R')		&&(text[k+2]=='a'||text[k+2]=='A')		&&(text[k+3]=='c'||text[k+3]=='C')){																																																																															if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_FRAC					);	k+=3;	continue;}		 }
				break;
			case 'F':			 if(text[k+1]=='i'||text[k+1]=='I'){		 if(text[k+2]=='b'||text[k+2]=='B'){																																																																																									if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMap		(k, 3, M_FIB					);	k+=2;	continue;}		}}
							else if(text[k+1]=='l'||text[k+1]=='L'){		 if(text[k+2]=='o'||text[k+2]=='O'){		 if(text[k+3]=='o'||text[k+3]=='O'){		 if(text[k+4]=='r'||text[k+4]=='R'){																																																																			if(exprBound||!isAlphanumeric[text[k+5]]){	it->insertMap		(k, 5, M_FLOOR					);	k+=4;	continue;}	  }}}}
							else if((text[k+1]=='r'||text[k+1]=='R')		&&(text[k+2]=='a'||text[k+2]=='A')			&&(text[k+2]=='c'||text[k+2]=='C')){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_FRAC					);	k+=3;	continue;}		 }	break;
			case 'g':case 'G':	 if(text[k+1]=='a'||text[k+1]=='A'){		 if(text[k+2]=='m'||text[k+2]=='M'){		 if(text[k+3]=='m'||text[k+3]=='M'){		 if(text[k+4]=='a'||text[k+4]=='A'){																																																																			if(exprBound||!isAlphanumeric[text[k+5]]){	it->insertMap		(k, 5, M_GAMMA					);	k+=4;	continue;}	  }}}
																			 if(text[k+2]=='u'||text[k+2]=='U'){		 if(text[k+3]=='s'||text[k+3]=='S'){		 if(text[k+4]=='s'||text[k+4]=='S'){																																																																			if(exprBound||!isAlphanumeric[text[k+5]]){	it->insertMap		(k, 5, M_GAUSS					);	k+=4;	continue;}	  }}}}	break;
			case 'h':case 'H':	 if(text[k+1]=='a'||text[k+1]=='A'){		 if(text[k+2]=='n'||text[k+2]=='N'){		 if(text[k+3]=='k'||text[k+3]=='K'){		 if(text[k+4]=='e'||text[k+4]=='E'){		 if(text[k+5]=='l'||text[k+5]=='L'){																																																								if(exprBound||!isAlphanumeric[text[k+6]]){	it->insertMap		(k, 6, M_HANKEL1				);	k+=5;	continue;}	 }}}}}
							else if(text[k+1]=='y'||text[k+1]=='Y'){		 if(text[k+2]=='p'||text[k+2]=='P'){		 if(text[k+3]=='o'||text[k+3]=='O'){		 if(text[k+4]=='t'||text[k+4]=='T'){																																																																			if(exprBound||!isAlphanumeric[text[k+5]]){	it->insertMap		(k, 6, M_HYPOT					);	k+=4;	continue;}	  }}}}	break;
			case 'l':case 'L':	 if(text[k+1]=='n'||text[k+1]=='N'){		 if((text[k+2]=='g'||text[k+2]=='G')		 &&(text[k+3]=='a'||text[k+3]=='A')			 &&(text[k+4]=='m'||text[k+4]=='M')			 &&(text[k+5]=='m'||text[k+5]=='M')			 &&(text[k+6]=='a'||text[k+6]=='A')){																																													if(exprBound||!isAlphanumeric[text[k+7]]){	it->insertMap		(k, 7, M_LNGAMMA				);	k+=6;	continue;}	}
																		else								   {																																																																																									if(exprBound||!isAlphanumeric[text[k+2]]){	it->insertMap		(k, 2, M_LN						);	++k;	continue;}	    }}
							else if(text[k+1]=='o'||text[k+1]=='O'){		 if(text[k+2]=='g'||text[k+2]=='G'){																																																																																									if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMap		(k, 3, M_LOG					);	k+=2;	continue;}	    }}	break;
			case 'm':case 'M':	 if(text[k+1]=='a'||text[k+1]=='A'){		 if(text[k+2]=='g'||text[k+2]=='G'){																																																																																									if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMap		(k, 3, M_ABS					);	k+=2;	continue;}	    }
																		else if(text[k+2]=='n'||text[k+2]=='N'){		if((text[k+3]=='d'||text[k+3]=='D')			&&(text[k+4]=='e'||text[k+4]=='E')			&&(text[k+5]=='l'||text[k+5]=='L')			&&(text[k+6]=='b'||text[k+6]=='B')			&&(text[k+7]=='r'||text[k+7]=='R')			&&(text[k+8]=='o'||text[k+8]=='O')			&&(text[k+9]=='t'||text[k+9]=='T'))													if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMap		(k, 10, M_MANDELBROT			);	k+=9;	continue;}		}
																		else if(text[k+2]=='x'||text[k+2]=='X'){																																																																																									if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMap		(k, 3, M_MAX					);	k+=2;	continue;}	    }}
							else if((text[k+1]=='i'||text[k+1]=='I')		 &&(text[k+2]=='n'||text[k+2]=='N')){																																																																																									if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMap		(k, 3, M_MIN					);	k+=2;	continue;}	     }	break;
			case 'n':case 'N':	 if(text[k+1]=='a'||text[k+1]=='A'){		 if(text[k+2]=='n'||text[k+2]=='N'){																																																																																									if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMapData	(k, 3, 'R', _qnan				);	k+=2;	continue;}	    }}
							else if(text[k+1]=='e'||text[k+1]=='E'){		 if(text[k+2]=='u'||text[k+2]=='U'){		 if(text[k+3]=='m'||text[k+3]=='M'){		 if(text[k+4]=='a'||text[k+4]=='A'){		 if(text[k+5]=='n'||text[k+5]=='N'){		 if(text[k+6]=='n'||text[k+6]=='N'){																																													if(exprBound||!isAlphanumeric[text[k+7]]){	it->insertMap		(k, 7, M_BESSEL_Y				);	k+=6;	continue;}	}}}}}}	break;
			case 'p':case 'P':	 if(text[k+1]=='e'||text[k+1]=='E'){		 if((text[k+2]=='r'||text[k+2]=='R')		 &&(text[k+3]=='m'||text[k+3]=='M')			 &&(text[k+4]=='u'||text[k+4]=='U')			 &&(text[k+5]=='t'||text[k+5]=='T')			 &&(text[k+6]=='a'||text[k+6]=='A')			 &&(text[k+7]=='t'||text[k+7]=='T')			 &&(text[k+8]=='i'||text[k+8]=='I')			 &&(text[k+9]=='o'||text[k+9]=='O')			 &&(text[k+10]=='n'||text[k+10]=='N')){	if(exprBound||!isAlphanumeric[text[k+11]]){	it->insertMap		(k, 11, M_PERMUTATION			);	k+=10;	continue;}		}}
							else if(text[k+1]=='i'||text[k+1]=='I'){																																																																																																				if(exprBound||!isAlphanumeric[text[k+2]]){	it->insertMapData	(k, 2, 'R', G2::_pi				);	++k;	continue;}		 }
							else if(text[k+1]=='o'||text[k+1]=='O'){		 if(text[k+2]=='l'||text[k+2]=='L'){		 if(text[k+3]=='a'||text[k+3]=='A'){		 if(text[k+4]=='r'||text[k+4]=='R'){																																																																			if(exprBound||!isAlphanumeric[text[k+5]]){	it->insertMap		(k, 5, M_POLAR					);	k+=4;	continue;}	  }}}}	break;
			case 'r':			 if(text[k+1]=='a'||text[k+1]=='A'){		 if(text[k+2]=='n'||text[k+2]=='N'){		 if(text[k+3]=='d'||text[k+3]=='D'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_RAND					);	k+=3;	continue;}	   }}}
							else if(text[k+1]=='e'				  ){		 if((text[k+2]=='c'||text[k+2]=='C')		 &&(text[k+3]=='t'||text[k+3]=='T')){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_RECT					);	k+=3;	continue;}	    }
																		else if(text[k+2]=='t'							 &&text[k+3]=='u'							  &&text[k+4]=='r'							 &&text[k+5]=='n'&&!exprBound	  ){																																																																			it->insertMap		(k, 6, M_RETURN					);	k+=5;	continue;	    }
																		else								   {																																																																																									if(exprBound||!isAlphanumeric[text[k+2]]){	it->insertMap		(k, 2, M_REAL					);	++k;	continue;}	    }}
							else if(				text[k+1]=='E'){		 if((text[k+2]=='c'||text[k+2]=='C')		 &&(text[k+3]=='t'||text[k+3]=='T')){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_RECT					);	k+=3;	continue;}	    }
																		else								   {																																																																																									if(exprBound||!isAlphanumeric[text[k+2]]){	it->insertMap		(k, 2, M_REAL					);	++k;	continue;}	    }}
							else if(text[k+1]=='o'||text[k+1]=='O'){		 if(text[k+2]=='u'||text[k+2]=='U'){		 if(text[k+3]=='n'||text[k+3]=='N'){		 if(text[k+4]=='d'||text[k+4]=='D'){																																																																			if(exprBound||!isAlphanumeric[text[k+5]]){	it->insertMap		(k, 5, M_ROUND					);	k+=4;	continue;}	  }}}}	break;
					 case 'R':	 if(text[k+1]=='a'||text[k+1]=='A'){		 if(text[k+2]=='n'||text[k+2]=='N'){		 if(text[k+3]=='d'||text[k+3]=='D'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_RAND					);	k+=3;	continue;}	   }}}
							else if(text[k+1]=='e'||text[k+1]=='E'){		 if((text[k+2]=='c'||text[k+2]=='C')		 &&(text[k+3]=='t'||text[k+3]=='T')){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_RECT					);	k+=3;	continue;}	    }
																		else								   {																																																																																									if(exprBound||!isAlphanumeric[text[k+2]]){	it->insertMap		(k, 2, M_REAL					);	++k;	continue;}	    }}
							else if(text[k+1]=='o'||text[k+1]=='O'){		 if(text[k+2]=='u'||text[k+2]=='U'){		 if(text[k+3]=='n'||text[k+3]=='N'){		 if(text[k+4]=='d'||text[k+4]=='D'){																																																																			if(exprBound||!isAlphanumeric[text[k+5]]){	it->insertMap		(k, 5, M_ROUND					);	k+=4;	continue;}	  }}}}	break;
			case 's':case 'S':	 if(text[k+1]=='a'||text[k+1]=='A'){		 if(text[k+2]=='w'||text[k+2]=='W'){																																																																																									if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMap		(k, 3, M_SAW					);	k+=2;	continue;}		}}
							else if(text[k+1]=='e'||text[k+1]=='E'){		 if(text[k+2]=='c'||text[k+2]=='C'){		 if(text[k+3]=='h'||text[k+3]=='H'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_SECH					);	k+=3;	continue;}	   }
																													else								   {																																																																														if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMap		(k, 3, M_SEC					);	k+=2;	continue;}	   }}}
							else if(text[k+1]=='g'||text[k+1]=='G'){		 if(text[k+2]=='n'||text[k+2]=='N'){																																																																																									if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMap		(k, 3, M_SGN					);	k+=2;	continue;}		}}
							else if(text[k+1]=='h'||text[k+1]=='H'){																																																																																																				if(exprBound||!isAlphanumeric[text[k+2]]){	it->insertMap		(k, 2, M_SINH					);	++k;	continue;}	     }
							else if(text[k+1]=='i'||text[k+1]=='I'){		 if(text[k+2]=='n'||text[k+2]=='N'){		 if(text[k+3]=='c'||text[k+3]=='C'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 3, M_SINC					);	k+=3;	continue;}	   }
																													else if(text[k+3]=='h'||text[k+3]=='H'){		 if(text[k+4]=='c'||text[k+4]=='C'){																																																																			if(exprBound||!isAlphanumeric[text[k+5]]){	it->insertMap		(k, 5, M_SINHC					);	k+=4;	continue;}	  }
																																								else								   {																																																																			if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_SINH					);	k+=3;	continue;}	  }}
																													else								   {																																																																														if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMap		(k, 3, M_SIN					);	k+=2;	continue;}	   }}}
							else if(text[k+1]=='q'||text[k+1]=='Q'){		 if(text[k+2]=='r'||text[k+2]=='R'){		 if(text[k+3]=='t'||text[k+3]=='T'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_SQRT					);	k+=3;	continue;}	   }}
																		else if(text[k+2]=='w'||text[k+2]=='W'){		 if(text[k+3]=='v'||text[k+3]=='V'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_SQWV					);	k+=3;	continue;}	   }}
																		else								   {																																																																																									if(exprBound||!isAlphanumeric[text[k+2]]){	it->insertMap		(k, 4, M_SQ						);	++k;	continue;}		}}
							else if(text[k+1]=='t'||text[k+1]=='T'){		 if(text[k+2]=='e'||text[k+2]=='E'){		 if(text[k+3]=='p'||text[k+3]=='P'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_STEP					);	k+=3;	continue;}	   }}}	break;
			case 't':			 if(text[k+1]=='a'||text[k+1]=='A'){		 if(text[k+2]=='n'||text[k+2]=='N'){		 if(text[k+3]=='c'||text[k+3]=='C'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_TANC					);	k+=3;	continue;}	   }
																													else if(text[k+3]=='h'||text[k+3]=='H'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_TANH					);	k+=3;	continue;}	   }
																													else								   {																																																																														if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMap		(k, 3, M_TAN					);	k+=2;	continue;}	   }}}
							else if(text[k+1]=='a'||text[k+1]=='A'){		 if(text[k+2]=='n'||text[k+2]=='N'){		 if(text[k+3]=='c'||text[k+3]=='C'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_TANC					);	k+=3;	continue;}	   }
																													else if(text[k+3]=='h'||text[k+3]=='H'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_TANH					);	k+=3;	continue;}	   }
																													else								   {																																																																														if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMap		(k, 3, M_TAN					);	k+=2;	continue;}	   }}}
							else if(text[k+1]=='e'||text[k+1]=='E'){		 if(text[k+2]=='n'||text[k+2]=='N'){		 if(text[k+3]=='t'||text[k+3]=='T'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_TENT					);	k+=3;	continue;}	   }}}
							else if(text[k+1]=='h'||text[k+1]=='H'){																																																																																																				if(exprBound||!isAlphanumeric[text[k+2]]){	it->insertMap		(k, 2, M_TANH					);	++k;	continue;}	     }
							else if(text[k+1]=='r'				  ){		 if(text[k+2]=='g'||text[k+2]=='G'){		 if(text[k+3]=='l'||text[k+3]=='L'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_TENT					);	k+=3;	continue;}	   }}
																		else if(text[k+2]=='u'&&!exprBound		){		 if(text[k+3]=='e'					)																																																																																									it->insertMapData	(k, 4, 'R', 1.					);	k+=3;	continue;	    }
																		else if(text[k+2]=='w'||text[k+2]=='W'){		 if(text[k+3]=='v'||text[k+3]=='V'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_TRWV					);	k+=3;	continue;}	   }}}
							else if(				text[k+1]=='R'){		 if(text[k+2]=='g'||text[k+2]=='G'){		 if(text[k+3]=='l'||text[k+3]=='L'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_TENT					);	k+=3;	continue;}	   }}
																		else if(text[k+2]=='w'||text[k+2]=='W'){		 if(text[k+3]=='v'||text[k+3]=='V'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_TRWV					);	k+=3;	continue;}	   }}}
							else if(exprBound)					   {																																																																																																															it->insertRVar		(k, 1, &text[k], 't'			);			continue;		 }	break;
					case 'T':	 if(text[k+1]=='a'||text[k+1]=='A'){		 if(text[k+2]=='n'||text[k+2]=='N'){		 if(text[k+3]=='c'||text[k+3]=='C'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_TANC					);	k+=3;	continue;}	   }
																													else if(text[k+3]=='h'||text[k+3]=='H'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_TANH					);	k+=3;	continue;}	   }
																													else								   {																																																																														if(exprBound||!isAlphanumeric[text[k+3]]){	it->insertMap		(k, 3, M_TAN					);	k+=2;	continue;}	   }}}
							else if(text[k+1]=='e'||text[k+1]=='E'){		 if(text[k+2]=='n'||text[k+2]=='N'){		 if(text[k+3]=='t'||text[k+3]=='T'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_TENT					);	k+=3;	continue;}	   }}}
							else if(text[k+1]=='h'||text[k+1]=='H'){																																																																																																				if(exprBound||!isAlphanumeric[text[k+2]]){	it->insertMap		(k, 2, M_TANH					);	++k;	continue;}	     }
							else if(text[k+1]=='r'||text[k+1]=='R'){		 if(text[k+2]=='g'||text[k+2]=='G'){		 if(text[k+3]=='l'||text[k+3]=='L'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_TENT					);	k+=3;	continue;}	   }}
																		else if(text[k+2]=='w'||text[k+2]=='W'){		 if(text[k+3]=='v'||text[k+3]=='V'){																																																																														if(exprBound||!isAlphanumeric[text[k+4]]){	it->insertMap		(k, 4, M_TRWV					);	k+=3;	continue;}	   }}}
							else if(exprBound)					   {																																																																																																															it->insertRVar		(k, 1, &text[k], 't'			);			continue;		 }	break;
			case 'w':			 if(text[k+1]=='e'||text[k+1]=='E'){		 if(text[k+2]=='b'||text[k+2]=='B'){		 if(text[k+3]=='e'||text[k+3]=='E'){		 if(text[k+4]=='r'||text[k+4]=='R'){																																																																			if(exprBound||!isAlphanumeric[text[k+5]]){	it->insertMap		(k, 5, M_BESSEL_Y				);	k+=4;	continue;}	  }}}}
							else if(text[k+1]=='h'				  ){		 if(text[k+2]=='i'				  ){		 if(text[k+3]=='l'				  ){		 if(text[k+4]=='e'&&!exprBound	  ){																																																																														it->insertMap		(k, 5, M_WHILE					);	k+=4;	continue;	  }}}}	break;
			case 'W':			 if((text[k+1]=='e'||text[k+1]=='E')		&&(text[k+2]=='b'||text[k+2]=='B')			&&(text[k+3]=='e'||text[k+3]=='E')			&&(text[k+4]=='r'||text[k+4]=='R')){																																																																			if(exprBound||!isAlphanumeric[text[k+5]]){	it->insertMap		(k, 5, M_BESSEL_Y				);	k+=4;	continue;}	     }
							else if(exprBound)					   {																																																																																																															it->insertCVar		(k, 5, &text[k]					);			continue;		 }	break;
			case '\r':
				k+=text[k+1]=='\n';
				++lineNo;
				continue;
			case '\n'://robust	unix/mac
				++lineNo;
				continue;
			}
			if(!exprBound)//user function token variable names
			{
				if((k-1<0||!isAlphanumeric[text[k-1]])&&isAlphanumeric[text[k]])
				{
					for(int f=k+1;f<=kEnd;++f)//match name
					{
						if(!isAlphanumeric[text[f]])
						{
							bool noMatch=true;
							int nameLen=f-k;
							std::string str;
							utf16ToAscii(&text[k], nameLen, str);
							for(int n=0, nEnd=ufVarNames.size();n<nEnd;++n)
							{
								auto &name=ufVarNames[n].name;
								if(name.size()==nameLen&&name==str)//match, add reference
							//	if(name.size()==nameLen&&!name.compare(0, name.size(), &text[k], nameLen))//match, add reference
								{
									it->insertMap(k, f-k, G2::M_N, ufVarNames[n].data_idx);
									//	it->insertMap(G2::M_N, ufVarNames[n].data_idx);
									noMatch=false;
									break;
								}
							}
							if(noMatch)//add new name and repositry
							{
								char mathSet='R';
								if(nameLen==1&&(text[k]=='r'||text[k]=='R'||text[k]=='c'||text[k]=='C'||text[k]=='h'||text[k]=='H'||text[k]=='q'||text[k]=='Q'))
								{
									int f2=f;
									switch(text[k])
									{
										case 'r':case 'R':mathSet='R';break;
										case 'c':case 'C':mathSet='c';break;
										case 'h':case 'H':
										case 'q':case 'Q':mathSet='h';break;
									}
									for(;text[f2]==' '||text[f2]=='\t'||text[f2]=='\r'||text[f2]=='\n';++f2)//skip white space
										lineNo+=text[k]=='\n';
									if(isLetter[text[f2]])
									{
										k=f2;
										for(;isAlphanumeric[text[f2]];++f2);
										f=f2, nameLen=f-k;
									}
								}
								ufVarNames.push_back(UFVariableName(&text[0]+k, nameLen, scopeLevel.size(), it->data.size()));
								it->insertFVar(mathSet);
								it->insertMap(k, f-k, G2::M_N, ufVarNames.rbegin()->data_idx);
								//	it->insertMap(G2::M_N, ufVarNames.rbegin()->data_idx);
								//	it->insertFVar('R');
								//	it->insertData('R', 0);//variables are initialized with 0, mathSet is updated by values assigned
							}
							k=f-1;
							break;
						}
					}
				}
			}
		}//end lexer loop
		it->endLineNo=lineNo-((bound==0?0:bounds[bound-1].first)<kEnd&&text[kEnd-1]=='\n');
		if(exprBound)
		{
			auto &map=it->m;
			if(map.size())
			{
				using namespace G2;
				{
					int min=0, end=0;
					for(unsigned k=0;k<map.size();++k)
					{
						if(map[k]._0==M_LPR)
							++end;
						else if(map[k]._0==M_RPR)
						{
							--end;
							if(min>end)
								min=end;
						}
					}
					if(min<0)
					{
						int pos=map[0].pos;
						int inc=-min;
						map.resize(map.size()+inc);
						for(int k=map.size()-1;k>=inc;--k)
							map[k]=map[k-inc];
						for(int k=0;k<inc;++k)
							map[k]=::Map(pos, 0, M_LPR);
						//	map[k]=::Map(M_LPR);
					}
					if(end-min>0)
					{
						int pos=map.rbegin()->pos+map.rbegin()->len;
						int inc=end-min;
						map.resize(map.size()+inc);
						for(unsigned k=map.size()-inc;k<map.size();++k)
							map[k]=::Map(pos, 0, M_RPR);
						//	map[k]=::Map(M_RPR);
					}
				}
			}
			if(map.size())
			{
				Compile::compile_expression_global(*it);
				it->resultLogicType=Compile::expressionResultLogicType();
				if(!it->n.size())//no expression
					it->rmode[0]=0;
				else
				{
					it->resultMathSet=Compile::predictedMathSet;
					switch(it->nISD)
					{
					case 0://numeric 0d
						{
							it->rmode[0]=1;
							modes::n0d.toSolve=true, modes::n0d.solver.reset();
							auto &n=it->n[it->resultTerm];
							n.r.push_back(it->data[it->resultTerm].r);
							if(it->resultMathSet>='c')
							{
								n.i.push_back(it->data[it->resultTerm].i);
								if(it->resultMathSet=='h')
								{
									n.j.push_back(it->data[it->resultTerm].j);
									n.k.push_back(it->data[it->resultTerm].k);
								}
							}
							it->setColor_random();
						}
						break;
					case 1:
						if(it->i.size()&&it->resultLogicType)//transverse implicit 1d	//bool expr
						{
							it->rmode[0]=10;
							modes::ti1d.toSolve=true, modes::ti1d.shiftOnly=0, modes::ti1d.solver.reset();
							modes::ti1d.ready=false;
							it->setColor_random();
						}
						else
						{
							switch(Compile::predictedMathSet)
							{
							case 'R'://transverse 1d
								it->rmode[0]=2;
								modes::t1d.toSolve=true, modes::t1d.shiftOnly=0, modes::t1d.solver.reset();
								modes::t1d.ready=false;
								it->setColor_random();
								break;
							case 'c'://transverse 1d complex
								it->rmode[0]=3;
								modes::t1d_c.toSolve=true, modes::t1d_c.shiftOnly=0, modes::t1d_c.solver.reset();
								modes::t1d_c.ready=false;
								it->setColor_random();
								break;
							case 'h'://transverse 1d quaternion
								it->rmode[0]=4;
								it->rmode.push_back(0);
								modes::t1d_h.toSolve=true, modes::t1d_h.shiftOnly=0, modes::t1d_h.solver.reset();
								modes::t1d_h.ready=false;
								it->setColor_black();
								break;
							}
						}
						break;
					case 2:
						if(it->i.size()&&it->resultLogicType)//transverse implicit 2d
						{
							it->rmode[0]=11;
							modes::ti2d.toSolve=true, modes::ti2d.shiftOnly=0, modes::ti2d.solver.reset();
							modes::ti2d.ready=false;
							it->setColor_random();
						}
						else
						{
							switch(Compile::predictedMathSet)
							{
							case 'R'://transverse 2d
								it->rmode[0]=5;
								modes::t2d.toSolve=true, modes::t2d.shiftOnly=0, modes::t2d.solver.reset();
								modes::t2d.ready=false;
								it->setColor_random();
								break;
							case 'c':
								if(it->nZ==1)//longitudinal 2d
								{
									it->rmode[0]=7;
									modes::l2d.toSolve=true, modes::l2d.shiftOnly=0, modes::l2d.solver.reset();
									modes::l2d.ready=false;
									it->setColor_random();
								}
								else//color 2d
								{
									it->rmode[0]=6;
									it->rmode.push_back(0);
									modes::c2d.toSolve=true, modes::c2d.shiftOnly=0, modes::c2d.solver.reset();
									modes::c2d.ready=false;
									it->setColor_black();
								}
								break;
							case 'h'://transverse 2d quaternion
								it->rmode[0]=8;
								it->rmode.push_back(0);
								modes::t2d_h.toSolve=true, modes::t2d_h.shiftOnly=0, modes::t2d_h.solver.reset();
								modes::t2d_h.ready=false;
								it->setColor_black();
								break;
							}
						}
						break;
					case 3://color 3d
#if 0
						if(it->i.size()&&it->resultLogicType)
						{
							it->rmode[0]=9;
							it->rmode.push_back(0);
							modes::c3d.toSolve=true, modes::c3d.shiftOnly=0, modes::c3d.solver.reset();
							modes::c3d.ready=false;
							it->setColor_black();
						}
						else
#endif
						{
							it->rmode[0]=9;
							it->rmode.push_back(0);
							modes::c3d.toSolve=true, modes::c3d.shiftOnly=0, modes::c3d.solver.reset();
							modes::c3d.ready=false;
							it->setColor_black();
						}
						break;
					}
				}
			}
			switch(old_rmode)//all modes showing multiple expressions except n0d
			{
				case  2:modes::t1d	.exprRemoved|=it->rmode[0]!=old_rmode;	break;
				case  3:modes::t1d_c.exprRemoved|=it->rmode[0]!=old_rmode;	break;
				case  5:modes::t2d	.exprRemoved|=it->rmode[0]!=old_rmode;	break;
				case  7:modes::l2d	.exprRemoved|=it->rmode[0]!=old_rmode;	break;
				case 10:modes::ti1d	.exprRemoved|=it->rmode[0]!=old_rmode;	break;
				case 11:modes::ti2d	.exprRemoved|=it->rmode[0]!=old_rmode;	break;
			}
			++e;
		}
		else
		{
			Compile::compile_function(*it);
			it->resultMathSet=Compile::predictedMathSet;
			//	expression_to_clipboard(*it);
			++function;
		}
	}//end bound loop
	std::fill(modes::nExpr.begin(), modes::nExpr.end(), 0);
	for(int ke=0, keEnd=expr.size();ke<keEnd;++ke)
	{
		auto &ex=expr[ke];
		++modes::nExpr[ex.rmode[0]];
	}

	int lineChange=lineInsertEnd-lineRemoveEnd;
	for(int b=boundInsertEnd, bEnd=bounds.size();b<bEnd;++b)
	{
		auto &bound=bounds[b];
		if(bound.second=='e')
			expr[e].lineNo+=lineChange, expr[e].endLineNo+=lineChange, ++e;
		else if(bound.second=='f')
			userFunctionDefinitions[function].lineNo+=lineChange, userFunctionDefinitions[function].endLineNo+=lineChange, ++function;
	}

	text.erase(text.end()-10, text.end());
	lineChangeStart=lineRemoveEnd=lineInsertEnd=0;
	boundChangeStart=boundRemoveEnd=boundInsertEnd=0;
	exprChangeStart=exprRemoveEnd=exprInsertEnd=0;
	functionChangeStart=functionRemoveEnd=functionInsertEnd=0;

	render();
	--nthreads;
	return quit;
}
extern "C" JNIEXPORT void JNICALL Java_com_example_grapher2_GL2JNILib_toggleInputBox(JNIEnv *env, jclass obj)
{
	inputBoxOn=!inputBoxOn;
	modes::active=!modes::active;
}
extern "C" JNIEXPORT void JNICALL Java_com_example_grapher2_GL2JNILib_resume(JNIEnv *env, jclass obj)
{
//	toSend=true;
}
extern "C" JNIEXPORT void JNICALL Java_com_example_grapher2_GL2JNILib_switchOrientation(JNIEnv *env, jclass obj, jboolean landscape)
{
	::landscape=landscape;
}
extern "C" JNIEXPORT void JNICALL Java_com_example_grapher2_GL2JNILib_inputKeyDown(JNIEnv *env, jclass obj, jint keyCode)
{
//	LOL_1=keyCode;
}
extern "C" JNIEXPORT void JNICALL Java_com_example_grapher2_GL2JNILib_inputKeyUp(JNIEnv *env, jclass obj, jint keyCode)
{
}

//#include <jni.h>
//#include <string>

//extern "C" JNIEXPORT jstring JNICALL Java_com_example_grapher2_MainActivity_stringFromJNI( JNIEnv *env, jobject /* this */)
//{
//	std::string hello="Hello from C++";
//	return env->NewStringUTF(hello.c_str());
//}